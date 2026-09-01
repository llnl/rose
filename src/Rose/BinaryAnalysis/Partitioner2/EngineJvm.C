#include <featureTests.h>
#ifdef ROSE_ENABLE_BINARY_ANALYSIS
#include <sage3basic.h>                                 // needed for `::CommandlineProcessing`
#include <Rose/BinaryAnalysis/Partitioner2/EngineJvm.h>

#include <Rose/BinaryAnalysis/Architecture/Base.h>
#include <Rose/BinaryAnalysis/ByteCode/Jvm.h>
#include <Rose/BinaryAnalysis/Disassembler/Jvm.h>
#include <Rose/BinaryAnalysis/Partitioner2/BasicBlock.h>
#include <Rose/BinaryAnalysis/Partitioner2/Configuration.h>
#include <Rose/BinaryAnalysis/Partitioner2/ModulesJvm.h>
#include <Rose/BinaryAnalysis/Partitioner2/Partitioner.h>
#include <Rose/CommandLine.h>
#include <Rose/Progress.h>

#include <SgAsmBlock.h>
#include <SgAsmGenericFile.h>
#include <SgAsmGenericFileList.h>
#include <SgAsmGenericFormat.h>
#include <SgAsmGenericHeader.h>
#include <SgAsmGenericHeaderList.h>
#include <SgAsmInstructionList.h>
#include <SgAsmIntegerValueExpression.h>
#include <SgAsmInterpretation.h>
#include <SgAsmInterpretationList.h>
#include <SgAsmJvmConstantPool.h>
#include <SgAsmJvmConstantPoolEntry.h>
#include <SgAsmJvmFileHeader.h>
#include <SgAsmJvmInstruction.h>
#include <SgAsmJvmMethod.h>

#include <Cxx_GrammarDowncast.h>

#include <Sawyer/FileSystem.h>
#include <boost/filesystem.hpp>
#include <ROSE_UNUSED.h>

using namespace Rose::Diagnostics;
using AddressSegment = Sawyer::Container::AddressSegment<Rose::BinaryAnalysis::Address,uint8_t>;
using opcode = Rose::BinaryAnalysis::JvmInstructionKind;

namespace fs = boost::filesystem;

namespace Rose {
namespace BinaryAnalysis {
namespace Partitioner2 {

static bool
isArchiveClassFile(const ModulesJvm::FileStat &file) {
    return CommandlineProcessing::isJavaClassFile(file.filename());
}

static bool
isArchiveJarFile(const ModulesJvm::FileStat &file) {
    return CommandlineProcessing::isJavaJarFile(file.filename());
}

static size_t
countArchiveFiles(const ModulesJvm::Zipper *zip, bool (*predicate)(const ModulesJvm::FileStat&)) {
    ASSERT_not_null(zip);

    size_t nFiles = 0;
    for (const ModulesJvm::FileStat &file: zip->files()) {
        if (predicate(file))
            ++nFiles;
    }
    return nFiles;
}

struct ArchiveClassSelection {
    std::string filename;
    unsigned version = 0;
};

static bool
stripPrefix(std::string &s, const std::string &prefix) {
    if (s.substr(0, prefix.size()) == prefix) {
        s = s.substr(prefix.size());
        return true;
    }
    return false;
}

static bool
parseJavaVersion(const std::string &s, size_t begin, size_t end, unsigned &version) {
    ASSERT_require(begin <= end);
    if (begin == end)
        return false;

    for (size_t i = begin; i < end; ++i) {
        if (!std::isdigit(static_cast<unsigned char>(s[i])))
            return false;
    }

    const unsigned long parsed = std::strtoul(s.substr(begin, end - begin).c_str(), nullptr, 10);
    if (parsed > std::numeric_limits<unsigned>::max())
        return false;

    version = static_cast<unsigned>(parsed);
    return true;
}

static std::string
archiveClassName(const std::string &filename, unsigned *version = nullptr) {
    if (version)
        *version = 0;

    const fs::path filePath(filename);
    std::string className = (filePath.parent_path() / filePath.stem()).string();

    stripPrefix(className, "WEB-INF/classes/");

    const std::string multiReleasePrefix = "META-INF/versions/";
    if (className.substr(0, multiReleasePrefix.size()) == multiReleasePrefix) {
        const size_t versionBegin = multiReleasePrefix.size();
        const size_t versionEnd = className.find('/', versionBegin);
        unsigned parsedVersion = 0;
        if (versionEnd != std::string::npos && versionEnd + 1 < className.size() &&
            parseJavaVersion(className, versionBegin, versionEnd, parsedVersion)) {
            className = className.substr(versionEnd + 1);
            if (version)
                *version = parsedVersion;
        }
    }

    return className;
}

static std::map<std::string, ArchiveClassSelection>
selectArchiveClassFiles(const ModulesJvm::Zipper *zip) {
    ASSERT_not_null(zip);

    std::map<std::string, ArchiveClassSelection> selected;
    for (const ModulesJvm::FileStat &file: zip->files()) {
        if (!isArchiveClassFile(file))
            continue;

        unsigned version = 0;
        const std::string className = archiveClassName(file.filename(), &version);
        const auto found = selected.find(className);
        if (found == selected.end() || version > found->second.version)
            selected[className] = ArchiveClassSelection{file.filename(), version};
    }

    return selected;
}

const std::map<std::string, std::string>&
EngineJvm::preferredArchiveClassFiles(ModulesJvm::Zipper *zip) {
    ASSERT_not_null(zip);

    auto cached = archivePreferredClassFiles_.find(zip);
    if (cached == archivePreferredClassFiles_.end()) {
        std::map<std::string, std::string> preferred;
        const std::map<std::string, ArchiveClassSelection> selected = selectArchiveClassFiles(zip);
        for (const auto &entry: selected)
            preferred[entry.first] = entry.second.filename;
        cached = archivePreferredClassFiles_.insert(std::make_pair(zip, preferred)).first;
    }

    return cached->second;
}

EngineJvm::EngineJvm(const Settings &settings)
    : Engine("JVM", settings), nextFunctionVa_{static_cast<Address>(-1)} {
}

EngineJvm::~EngineJvm() {}

EngineJvm::Ptr
EngineJvm::instance() {
  return instance(Settings{});
}

EngineJvm::Ptr
EngineJvm::instance(const Settings &settings) {
    auto engine = Ptr(new EngineJvm(settings));

    // Turn off all post function analysis regardless of what user requested
    engine->settings().partitioner.doingPostAnalysis = false;
    engine->settings().partitioner.doingPostFunctionNoop = false;
    engine->settings().partitioner.doingPostFunctionMayReturn = false;
    engine->settings().partitioner.doingPostFunctionStackDelta = false;
    engine->settings().partitioner.doingPostCallingConvention = false;

    return engine;
}

EngineJvm::Ptr
EngineJvm::promote(const Engine::Ptr &x) {
    EngineJvm::Ptr retval = as<EngineJvm>(x);
    ASSERT_not_null(retval);
    return retval;
}

EngineJvm::Ptr
EngineJvm::factory() {
    return Ptr(new EngineJvm(Settings()));
}

bool
EngineJvm::matchFactory(const Sawyer::CommandLine::ParserResult &result, const std::vector<std::string> &specimen) const {
    // This function called before reading command line and is const, so must create a classpath
    std::vector<std::string> localClassPath;

    if (result.have("classpath")) {
        for (auto path : result.parsed("classpath")) {
            localClassPath.push_back(path.string());
        }
    }

    // If classpath is empty, default to "."
    if (localClassPath.empty()) {
        localClassPath.push_back(".");
    }

    // All files in the specimen must be JVM related (.jar, .class or fully qualified classname, e.g., org.example.MainClass)
    for (auto s: specimen) {
        if (CommandlineProcessing::isJavaJvmFile(s)) {
            continue; // found a jar or class file
        }
        else if (ModulesJvm::pathToClassFile(s, localClassPath) .empty() == false) {
            continue; // found a class file for the fully qualified classname
        }
        else {
          return false;
        }
    }

    // Use another engine type if there is no specimen
    return specimen.size() > 0;
}

Engine::Ptr
EngineJvm::instanceFromFactory(const Settings &settings) {
    ASSERT_require(isFactory());
    return instance(settings);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Utility functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void
EngineJvm::updateLoaderProgress(size_t current, size_t total) const {
    if (total > 0) {
        if (Progress::Ptr p = progress())
            p->update(Progress::Report("", (double)current, (double)total));
    }
}

void
EngineJvm::createClassProgressBar(ArchiveProgress &info, const std::string &name) const {
    if (info.nClassFiles > 0 && !info.classProgressBar) {
        info.classProgressBar.reset(new Sawyer::ProgressBar<size_t>(info.nClassFiles, mlog[MARCH], name));
        info.classProgressBar->suffix(" classes");
        info.classProgressBar->domain(0, info.nClassFiles);
    }
}

void
EngineJvm::createJarProgressBar(ArchiveProgress &info, const std::string &name) const {
    if (info.nJarFiles > 0 && !info.jarProgressBar) {
        info.jarProgressBar.reset(new Sawyer::ProgressBar<size_t>(info.nJarFiles, mlog[MARCH], name));
        info.jarProgressBar->suffix(" jars");
        info.jarProgressBar->domain(0, info.nJarFiles);
    }
}

void
EngineJvm::registerWarProgress(ModulesJvm::Zipper *zip) {
    ASSERT_not_null(zip);

    ArchiveProgress info;
    info.kind = ArchiveProgress::Kind::WAR;
    info.nClassFiles = countArchiveFiles(zip, isArchiveClassFile);
    info.nJarFiles = countArchiveFiles(zip, isArchiveJarFile);

    archiveProgress_.emplace(zip, std::move(info));
}

void
EngineJvm::registerJarProgress(ModulesJvm::Zipper *zip, ModulesJvm::Zipper *parentWar) {
    ASSERT_not_null(zip);

    ArchiveProgress info;
    info.kind = ArchiveProgress::Kind::JAR;
    info.parentWar = parentWar;
    info.nClassFiles = countArchiveFiles(zip, isArchiveClassFile);

    archiveProgress_.emplace(zip, std::move(info));

    if (parentWar) {
        auto parent = archiveProgress_.find(parentWar);
        if (parent != archiveProgress_.end() && parent->second.kind == ArchiveProgress::Kind::WAR)
            parent->second.childJars.push_back(zip);
    }
}

void
EngineJvm::beginArchiveClassProgress(ModulesJvm::Zipper *zip) {
    auto found = archiveProgress_.find(zip);
    if (found == archiveProgress_.end())
        return;

    ArchiveProgress &info = found->second;
    switch (info.kind) {
        case ArchiveProgress::Kind::JAR:
            createClassProgressBar(info, info.parentWar ? "jvm-load.war.jars.jar.classes" : "jvm-load.jar.classes");
            break;

        case ArchiveProgress::Kind::WAR:
            createClassProgressBar(info, "jvm-load.war.classes");
            break;
    }

    updateLoaderProgress(info.nClassesHandled, info.nClassFiles);
    if (info.classProgressBar)
        info.classProgressBar->value(info.nClassesHandled, info.nClassFiles);
}

void
EngineJvm::beginWarJarProgress(ModulesJvm::Zipper *zip) {
    auto found = archiveProgress_.find(zip);
    if (found == archiveProgress_.end())
        return;

    ArchiveProgress &info = found->second;
    createJarProgressBar(info, "jvm-load.war.jars");
    updateLoaderProgress(info.nJarsHandled, info.nJarFiles);
    if (info.jarProgressBar)
        info.jarProgressBar->value(info.nJarsHandled, info.nJarFiles);
}

void
EngineJvm::noteWarJarHandled(ModulesJvm::Zipper *zip) {
    auto found = archiveProgress_.find(zip);
    if (found != archiveProgress_.end()) {
        ++found->second.nJarsHandled;
        updateLoaderProgress(found->second.nJarsHandled, found->second.nJarFiles);
        if (found->second.jarProgressBar) {
            found->second.jarProgressBar->value(found->second.nJarsHandled, found->second.nJarFiles);
            if (found->second.nJarsHandled >= found->second.nJarFiles)
                found->second.jarProgressBar.reset();
        }
    }
}

void
EngineJvm::noteArchiveClassHandled(ModulesJvm::Zipper *zip) {
    auto found = archiveProgress_.find(zip);
    if (found == archiveProgress_.end())
        return;

    ArchiveProgress &info = found->second;
    ++info.nClassesHandled;

    switch (info.kind) {
        case ArchiveProgress::Kind::JAR:
            updateLoaderProgress(info.nClassesHandled, info.nClassFiles);
            if (info.classProgressBar) {
                info.classProgressBar->value(info.nClassesHandled, info.nClassFiles);
                if (info.nClassesHandled >= info.nClassFiles)
                    info.classProgressBar.reset();
            }
            break;

        case ArchiveProgress::Kind::WAR:
            updateLoaderProgress(info.nClassesHandled, info.nClassFiles);
            if (info.classProgressBar) {
                info.classProgressBar->value(info.nClassesHandled, info.nClassFiles);
                if (info.nClassesHandled >= info.nClassFiles)
                    info.classProgressBar.reset();
            }
            break;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Top-level, do everything functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SgAsmBlock*
EngineJvm::frontend(const std::vector<std::string> &args, const std::string &purpose, const std::string &description) {
    try {
        std::vector<std::string> specimen = parseCommandLine(args, purpose, description).unreachedArgs();
        if (specimen.empty()) {
            throw std::runtime_error("no binary specimen specified; see --help");
        }

        // Parse specimen files to ultimately obtain a project
        if (!areContainersParsed()) {
            parseContainers(specimen);
        }

        auto interp = interpretation();

        // TODO: This will probably fail, remove this method (used to call buildAst which should be deprecated)
        ASSERT_not_null(interp->get_globalBlock());
        return interp->get_globalBlock();

    } catch (const std::runtime_error &e) {
        if (settings().engine.exitOnError) {
            mlog[FATAL] <<e.what() <<"\n";
            exit(1);
        } else {
            throw;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Command-line parsing
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void
EngineJvm::checkSettings() {
    Super::checkSettings();

    // If classPath is empty, default to "."
    if (classPath().size() == 0) {
        settings().engineJvm.classPath.push_back(".");
    }

    // Probably need to support the following options
    // 1. Specific *.class
    // 2. Directories containing *.class
    // 3. Jar files
    //    a. directories containing jar files
    //    b. single jar files, *.jar
    // 4. ZIP archives (ooch)
    // 5. Class directories

    // Load all discoverable jar files in classPath
    for (auto path : classPath()) {
        if (CommandlineProcessing::isJavaJarFile(path)) {
            // load the jar file
            ASSERT_require(EngineJvm::loadJarFile(path));
        }
        else if (path == ".") {
            // load all jar files found in current directory
            fs::path dir = fs::current_path();
            for (const auto &entry : fs::directory_iterator(dir)) {
                if (fs::is_regular_file(entry)) {
                    const fs::path &filePath = entry.path();
                    if (filePath.extension() == ".jar") {
                        ASSERT_require(EngineJvm::loadJarFile(filePath.string()));
                    }
                }
            }
        }
    }
}

std::list<Sawyer::CommandLine::SwitchGroup>
EngineJvm::commandLineSwitches() {
    std::list<Sawyer::CommandLine::SwitchGroup> retval = Super::commandLineSwitches();
    retval.push_back(jvmSwitches(settings().engineJvm));
    return retval;
}

// class method
Sawyer::CommandLine::SwitchGroup
EngineJvm::jvmSwitches(JvmSettings &settings) {
    using namespace Sawyer::CommandLine;

    SwitchGroup sg("JVM engine switches");
    sg.name("jvm-engine");
    sg.doc("These switches are used by Rose::BinaryAnalysis::Partitioner2::EngineJvm to control the overall operation of "
           "the engine. Additional switches control different aspects of the engine's behavior such as loading, partitioning, "
           "building the AST, etc.");

    CommandLine::insertBooleanSwitch(sg, "load-all-classes", settings.loadAllClasses,
                                     "Load all classes from jar files (or other containers) obtained from the command line. "
                                     "This doesn't affect the loading of classes from system libraries, which may be ignored. "
                                     "Default behavior is to load classes lazily, as needed.");

    CommandLine::insertBooleanSwitch(sg, "load-all-jars", settings.loadAllJars,
                                     "Load all jar files encountered while processing JVM files. "
                                     "This doesn't affect the loading of jar files from system libraries, which may be ignored. "
                                     "Default behavior is to load jar files lazily, as needed.");

    sg.insert(Switch("classpath")
              .argument("classpath", listParser(anyParser(settings.classPath), ":"))
              .explosiveLists(true)
              .whichValue(SAVE_ALL)
              .doc("The location of class files is searched in the directories specified by (and in the order of) "
                   "the classpath option. Jar files must be specified by name with the .jar extension. Relative paths "
                   "may be used but directories are not searched recursively. The default classpath is the current directory. "
                   "This switch may appear more than once and/or a comma-separated list of names can be specified. This switch "
                   "has no effect if the input is a ROSE Binary Analysis (RBA) file since the partitioner is constructed and "
                   "initialized from the RBA file instead.\n\n" + Configuration::fileFormatDoc()));

    return sg;
}

std::pair<std::string, std::string>
EngineJvm::specimenNameDocumentation() {
    return {
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        "JVM specimens",
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        "The Rose::BinaryAnalysis::Partitioner2::EngineJvm class recognizes the following command-line positional arguments "
        "for specifying Java Virtual Machine specimens:"

        "@bullet{Any positional argument which looks like a file name having a \".class\" extension.}"
    };
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Partitioner high-level functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Binary container parsing
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool
EngineJvm::isNonContainer(const std::string &name) {
  if (isRbaFile(name)) { // ROSE Binary Analysis file
      return true;
  }
  return false;
}

bool
EngineJvm::areContainersParsed() const {
    return interpretation() != nullptr;
}

SgAsmInterpretation*
EngineJvm::parseContainers(const std::vector<std::string> &specimen) {
    try {
        interpretation(nullptr);
        memoryMap(MemoryMap::Ptr());
        checkSettings();

        // Prune away things we don't recognize as Java files.
        std::vector<fs::path> jvmFiles{};
        for (const std::string &s: specimen) {
            if (ModulesJvm::isJavaWarFile(s)) {
                if (settings().engineJvm.loadAllClasses) {
                    jvmFiles.push_back(s);
                } else {
                    loadWarFile(s);
                }
            }
            else if (ModulesJvm::isJavaJarFile(s)) {
                if (settings().engineJvm.loadAllClasses) {
                    jvmFiles.push_back(s);
                } else {
                    loadJarFile(s);
                }
            }
            else if (ModulesJvm::isJavaClassFile(s)) {
                jvmFiles.push_back(s);
            }
            else {
                // Search for fully qualified class name
                auto path = pathToClass(s);
                if (path.extension() == ".class") {
                    jvmFiles.push_back(path);
                }
            }
        }

        // Process through ROSE's frontend()
        if (!jvmFiles.empty()) {
            SgProject* project = roseFrontendReplacement(jvmFiles);
            ASSERT_not_null(project);

            std::vector<SgAsmInterpretation*> interps = SageInterface::querySubTree<SgAsmInterpretation>(project);
            if (interps.size() != 1) {
                throw std::runtime_error("a Jvm specimen container must have one (and only one) SgAsmInterpretation");
            }
            interpretation(interps.back());
            ASSERT_require(areContainersParsed());
        }
        else {
          mlog[WARN] << "No Java class files in specimen\n";
        }

        ASSERT_require(!areSpecimensLoaded());
        return interpretation();
    } catch (const std::runtime_error &e) {
        if (settings().engine.exitOnError) {
            mlog[FATAL] <<e.what() <<"\n";
            exit(1);
        } else {
            throw;
        }
    }
}

fs::path
EngineJvm::pathToClass(const std::string &fqcn) {
    namespace bfs = boost::filesystem;
    fs::path classFilePath{};

    std::string className{fqcn};
    std::replace(className.begin(), className.end(), '.', '/');
    std::string classFileName = className + ".class";

    // Search order used by JVM loader:
    //
    // Loads from:
    //   1. The CLASSPATH environment variable (if set), or
    //   2. The -cp / -classpath option on the java or javac command, or
    //   3. Current directory (.) if nothing else is specified
    //
    // Note that -cp / -classpath overrides CLASSPATH
    //

    // However, let's search jars_ first as a jar file could be from command line (require -classpath instead?)
    for (auto zip : jars_) {
        const auto &preferred = preferredArchiveClassFiles(zip);
        const auto selected = preferred.find(className);
        if (selected != preferred.end()) {
            classFilePath = fs::path{selected->second};
            break;
        }

        if (zip->present(classFileName)) {
            classFilePath = fs::path{classFileName};
            break;
        }
    }

    // Search classpath
    if (!bfs::exists(classFilePath)) {
        for (auto cp : classPath()) {
            // 1. Search cp as directory
            fs::path fullPath{cp + "/" + classFileName};
            if (bfs::exists(fullPath)) {
                classFilePath = fullPath;
                break;
            }
        }
    }

    return classFilePath;
}

// Load war file by opening its contents, assumed to be in the file system
bool
EngineJvm::loadWarFile(const std::string &filename) {
    if (!CommandlineProcessing::isJavaWarFile(filename)) {
        return false;
        
    }
    SAWYER_MESG(mlog[TRACE]) << "loading war file " << filename <<"\n";

    auto gfWar = new SgAsmGenericFile{};
    gfWar->parse(filename); /* this loads war file into memory, does no reading of file */

    // Unzip war file but do not decompress any contents
    auto zipWar = new ModulesJvm::Zipper(gfWar);

    // Store WAR zipper for unified archive access (enables lazy loading)
    jars_.push_back(zipWar);
    registerWarProgress(zipWar);

    // Load files contained in the war file
    for (auto file : zipWar->files()) {
        if (settings().engineJvm.loadAllJars && isArchiveJarFile(file)) {
            loadJarFile(file.filename(), zipWar);
        }
        // Class files in WAR will be loaded via normal mechanisms
        // The zipWar is already in jars_, so loadClassFile() will find them
    }

    return true;
}

Address
EngineJvm::loadArchiveClassFiles(ModulesJvm::Zipper *zip, SgAsmGenericFileList* fileList, Address baseVa) {
    ASSERT_not_null(zip);

    const std::map<std::string, std::string> &selected = preferredArchiveClassFiles(zip);

    for (auto file : zip->files()) {
        if (isArchiveClassFile(file)) {
            const std::string className = archiveClassName(file.filename());
            const auto selectedClass = selected.find(className);
            const bool isSelected = selectedClass != selected.end() && selectedClass->second == file.filename();

            // Skip if this class has already been loaded
            // (prevents loading same class from both WAR and nested JARs)
            if (isSelected && classes_.find(className) == classes_.end()) {
//TODO::add zip as a parameter to loadClassFile so faster to find
                baseVa = loadClassFile(file.filename(), fileList, baseVa);
            }
            noteArchiveClassHandled(zip);
        }
    }

    return baseVa;
}

bool
EngineJvm::loadJarFile(const std::string &filename, ModulesJvm::Zipper* zip) {
    if (!CommandlineProcessing::isJavaJarFile(filename)) {
        return false;
    }
    SAWYER_MESG(mlog[TRACE]) << "loading jar file " << filename <<"\n";

    // Decode the file found in the zipper contents
    size_t nbytes{0};
    unsigned char* bytes = zip->decode(filename, nbytes);
    ASSERT_require2(nbytes >= 4, "A jar file found in a zip file has insufficient bytes\n");

    // Check jar content for magic 0xCAFEBABE
    bool magic = (bytes[0] == 'P' && bytes[1] == 'K' && bytes[2] == 0x03 && bytes[3] == 0x04);
    if (!magic) {
        throw std::runtime_error("magic number incorrect for jar file contents");
    }

    /* Make jar file contents available but don't decompress contents */
    auto gf = new SgAsmGenericFile{};
    gf->set_name(filename);
    gf->set_data(SgFileContentList(bytes, nbytes));

    auto zipJar = new ModulesJvm::Zipper(gf);
    jars_.push_back(zipJar);
    registerJarProgress(zipJar, zip);

    return true;
}

bool
EngineJvm::loadJarFile(const std::string &filename) {
    if (!CommandlineProcessing::isJavaJarFile(filename)) {
        return false;
    }
    SAWYER_MESG(mlog[TRACE]) << "loading jar file " << filename <<"\n";

    /* Make jar file contents available but don't decompress contents */
    auto gf = new SgAsmGenericFile{};
    gf->parse(filename); /* this loads jar file into memory, does no reading of file */

    auto zipJar = new ModulesJvm::Zipper(gf);
    jars_.push_back(zipJar);
    registerJarProgress(zipJar);

    return true;
}

// Load class and super classes
Address
EngineJvm::loadClass(uint16_t classIndex, SgAsmJvmConstantPool* pool, SgAsmGenericFileList* fileList, Address baseVa) {
    std::string superName{};
    std::string className{ByteCode::constantPoolEntryName(classIndex, pool)};

    // Don't load classes twice
    if (classes_.find(className) != classes_.end()) {
        return baseVa;
    }

    // Don't load classes from java/lang/... or bootstrap_methods
    if (ByteCode::JvmContainer::isJvmSystemReserved(className)) {
        return baseVa;
    }

    // But if path exists, load the file
    auto path = pathToClass(className);
    if (fs::exists(path)) {
        baseVa = loadClassFile(path, fileList, baseVa);
    }

    return baseVa;
}

Address
EngineJvm::loadClassFile(fs::path path, SgAsmGenericFileList* fileList, Address baseVa) {
#if 0
   // Temporary NOTES for fs::path usage
   std::cout << "Root Name: " << p.root_name() << "\n";
   std::cout << "Root Directory: " << p.root_directory() << "\n";
   std::cout << "Relative Path: " << p.relative_path() << "\n";
   std::cout << "Filename: " << p.filename() << "\n";
   std::cout << "Extension: " << p.extension() << "\n";

Root Name: C:
Root Directory: \
Relative Path: Users\Example\document.txt
Filename: document.txt
Extension: .txt

Root Directory: /
Parent Path: /home/user
Filename: document.txt
Extension: .txt

a. linux returns "" for root_name()
b. stem() is same for both, e.g., filename() without extension

NOTES:
  1. --load-all-classes: load all classes from encountered jar files
  2. If path.root_directory() then path.extension() == class
  3. If path.root_directory() is "/"
      a: then path/file must exist
      b: then path.extension() == class (TODO:windows runs JVM too)
  4. If positional argument "gov.llnl.T13_Overload" is classname
     a: then look for class in classpath: what if "gov/llnl/T13_Overload.class" exists?
     b: what if "gov/llnl/T13_Overload.class" exists in path, i.e. not in a jar file?
        - change "gov.llnl.T13_Overload" to "gov/llnl/T13_Overload.class" and look
        - this likely should happen before this call because ::loadClassFile should have been discovered?
#endif

    size_t nbytes{0};
    unsigned char* bytes{nullptr};

    std::string className = archiveClassName(path.string());

    // Check to see if the class has already been processed
    if (classes_.find(className) != classes_.end()) {
        return baseVa;
    }

    // Look for the class in the collection of jar files, if it is not in the file system
    if (!fs::exists(path)) {
        for (ModulesJvm::Zipper* zip : jars_) {
            if (!zip->present(path.string())) continue;

            bytes = zip->decode(path.string(), nbytes);
            ASSERT_require2(nbytes > 4, "A class file found in a zip file has insufficient bytes\n");

            // Check class content for magic 0xCAFEBABE
            bool magic = (bytes[0] == 0xCA && bytes[1] == 0xFE &&
                          bytes[2] == 0xBA && bytes[3] == 0xBE);
            if (!magic) {
                delete[] bytes; bytes = nullptr;
                throw std::runtime_error("magic number incorrect for class file contents");
            }
            break;
        }
        if (nbytes < 4) {
            // class not found in a jar file
            return baseVa;
        }
    }

    auto gf = new SgAsmGenericFile{};

    // Complete initialization of gf
    if (bytes != nullptr) {
        gf->set_name(path.string());
        gf->set_data(SgFileContentList(bytes, nbytes));
    }
    else {
        // Load file by parsing from file system
        gf->parse(path.string()); // this loads file into memory, does no reading of file
    }

    auto jfh = new SgAsmJvmFileHeader(gf);

    // Save the address for constructing the ByteCode::Class
    jfh->set_baseVa(baseVa);

    // Check AST
    ASSERT_require(jfh == gf->get_header(SgAsmGenericFile::FAMILY_JVM));
    ASSERT_require(jfh->get_parent() == gf);

    jfh->parse();

    auto pool = jfh->get_constant_pool();

    // Check the class name now that the path has been loaded
    std::string parsedClassName = ByteCode::constantPoolEntryName(jfh->get_this_class(), pool);
    if (className != parsedClassName)
        className = parsedClassName;

    if (classes_.find(className) != classes_.end()) {
        SageInterface::deleteAST(gf);
        return baseVa;
    }

    classes_[className] = gf;

    fileList->get_files().push_back(gf);
    gf->set_parent(fileList);

    // Increase base virtual address for the next class
    baseVa += gf->get_originalSize() + vaDefaultIncrement;
    baseVa -= baseVa % vaDefaultIncrement;

    // Decode instructions for usage downstream
    std::set<std::string> discoveredClasses{};
    auto disassembler = Architecture::findByName("jvm").orThrow()->newInstructionDecoder();
    for (auto sgMethod: jfh->get_method_table()->get_methods()) {
        ByteCode::JvmMethod method{className, jfh->get_baseVa(), jfh, sgMethod};
        method.decode(disassembler);
        discoverFunctionCalls(sgMethod, jfh->get_constant_pool(), functions_, discoveredClasses);
    }

    // Find and load super classes
    baseVa = loadSuperClasses(className, fileList, baseVa);

    // Load classes discovered during function call search
    for (auto discovered: discoveredClasses) {
        auto p = pathToClass(discovered);
        if (fs::exists(p)) {
            baseVa = loadClassFile(p, fileList, baseVa);
        }
    }

    return baseVa;
}

Address
EngineJvm::loadDiscoverableClasses(SgAsmGenericFileList* fileList, Address baseVa) {
    for (auto file: fileList->get_files()) {
        auto jfh = dynamic_cast<SgAsmJvmFileHeader*>(file->get_header(SgAsmGenericFile::FAMILY_JVM));
        auto pool = jfh->get_constant_pool();

        for (auto sgMethod: jfh->get_method_table()->get_methods()) {
          // Examine instructions for classes
            for (auto insn: sgMethod->get_instruction_list()->get_instructions()) {
              switch (isSgAsmJvmInstruction(insn)->get_kind()) {
                case opcode::checkcast: // class, array, or interface type
                case opcode::instanceof: // class, array, or interface type
                  // TODO: don't try to load an array
                case opcode::new_: // can reference a class or interface type
                  if (auto expr = isSgAsmIntegerValueExpression(insn->get_operandList()->get_operands()[0])) {
                    uint16_t classIndex = static_cast<uint16_t>(expr->get_value());
                    baseVa = loadClass(classIndex, pool, fileList, baseVa);
                  }
                  break;
                case opcode::getfield: // fetch field from object
                case opcode::getstatic: // get static field from class
                case opcode::putfield: // set field in object
                case opcode::putstatic: // set static field in class
                  if (auto expr = isSgAsmIntegerValueExpression(insn->get_operandList()->get_operands()[0])) {
                    auto fieldEntry = pool->get_entry(expr->get_value());
                    if (fieldEntry->get_tag() == SgAsmJvmConstantPoolEntry::CONSTANT_Fieldref) {
                      uint16_t classIndex = fieldEntry->get_class_index();
                      baseVa = loadClass(classIndex, pool, fileList, baseVa);
                    }
                  }
                  break;
                default: ;
              }
            }
        }
    }

    return baseVa;
}

Address
EngineJvm::loadSuperClasses(const std::string &className, SgAsmGenericFileList* fileList, Address baseVa) {
    // Make sure the class has been processed and the SgAsmGenericFile for it exists
    if (classes_.find(className) == classes_.end()) {
        return baseVa;
    }
    SgAsmGenericFile* file = classes_[className];

    auto jfh = dynamic_cast<SgAsmJvmFileHeader*>(file->get_header(SgAsmGenericFile::FAMILY_JVM));
    auto pool = jfh->get_constant_pool();

    // Load interfaces
    for (auto interface: jfh->get_interfaces()) {
        std::string interfaceName = ByteCode::constantPoolEntryName(interface, pool);
        if (ByteCode::JvmContainer::isJvmSystemReserved(interfaceName)) continue; // ignore Java system files

        auto interfacePath = pathToClass(interfaceName);
        if (!fs::exists(interfacePath)) {
            // Revert to interface name to check for class in a jar
            interfacePath = interfaceName + ".class";
        }
        baseVa = loadClassFile(interfacePath, fileList, baseVa);
    }

    // There may not be a super class (e.g., module-info.class)
    if (jfh->get_super_class() < 1) return baseVa;

    std::string superName = ByteCode::constantPoolEntryName(jfh->get_super_class(), pool);
    if (ByteCode::JvmContainer::isJvmSystemReserved(superName)) return baseVa; // ignore Java system files

    return loadClassFile(pathToClass(superName), fileList, baseVa);
}

void
EngineJvm::discoverFunctionCalls(SgAsmJvmMethod* sgMethod, SgAsmJvmConstantPool* pool, std::map<std::string,Address> &fnm,
                                 std::set<std::string> &classes) {
    for (auto insn: sgMethod->get_instruction_list()->get_instructions()) {
        switch (isSgAsmJvmInstruction(insn)->get_kind()) {
          case opcode::invokedynamic:
          case opcode::invokeinterface:
          case opcode::invokespecial:
          case opcode::invokestatic:
          case opcode::invokevirtual:
            if (auto expr = isSgAsmIntegerValueExpression(insn->get_operandList()->get_operands()[0])) {
              std::string functionName = ByteCode::constantPoolEntryName(expr->get_value(), pool);
              if (fnm.find(functionName) == fnm.end()) {
                fnm[functionName] = nextFunctionVa_;
                nextFunctionVa_ -= 1024;

                // Also store corresponding class
                auto pos = functionName.find_first_of("::");
                if (pos != std::string::npos) {
                  classes.emplace(functionName.substr(0,pos));
                }
              }
            }
            break;
          default: ;
        }
    }
}

// Replacement for ::frontend for Jvm files only
SgProject*
EngineJvm::roseFrontendReplacement(const std::vector<std::string> &fileNames) {
    std::vector<fs::path> paths{};

    // Load jar files from classpath (do this early so jars_ can be used)
    for (auto path : classPath()) {
       loadJarFile(path);
    }

    // Save file names from command line
    for (auto name : fileNames) {
      paths.push_back(fs::path{name});
    }

    return roseFrontendReplacement(paths);
}


// Replacement for ::frontend, which is a complete mess, in order create a project containing multiple files. Nothing
// special happens to any of the input file names--that should have already been done by this point. All the fileNames
// are expected to be names of existing Java class or jar files.
SgProject*
EngineJvm::roseFrontendReplacement(const std::vector<fs::path> &paths) {
    ASSERT_forbid(paths.empty());

    // Create an SgJvmComposite isa SgBinaryComposite isa SgFile
    auto jvmComposite = new SgJvmComposite;
    auto fileList = jvmComposite->get_genericFileList();

// TODO: try to remove this initialization
//       Perhaps SgFile::doSetupForConstructor(argv, project); // (but we are post constructor!)
    jvmComposite->set_sourceFileNameWithPath(fs::absolute(paths[0]).string()); // best we can do
    jvmComposite->set_sourceFileNameWithoutPath(paths[0].filename().string()); // best we can do
    jvmComposite->initializeSourcePosition(paths[0].string());                 // best we can do
    jvmComposite->set_originalCommandLineArgumentList(std::vector<std::string>(1, paths[0].string())); // best we can do
    ASSERT_not_null(jvmComposite->get_file_info());

    // Load class files starting at this virtual address
    Address baseVa = 0;

    // Loop over all paths, loading war, jar files and classes
    for (auto path : paths) {
        if (ModulesJvm::isJavaWarFile(path)) {
            // Load war file found in the file system and all contained jar files, if requested
            loadWarFile(path.string());
        }
        else if (ModulesJvm::isJavaJarFile(path)) {
            // Load jar file found in the file system
            loadJarFile(path.string());
        }
        else if (ModulesJvm::isJavaClassFile(path)) {
            // Attempt to load class from file system
            baseVa = loadClassFile(path, fileList, baseVa);
        }
    }

    // Load all classes from encountered jar files, if requested
    if (settings().engineJvm.loadAllClasses) {
        Progress::Ptr p = progress();
        ProgressTask loadTask(p, "jvm-load");

        for (auto zip : jars_) {
            auto found = archiveProgress_.find(zip);
            if (found == archiveProgress_.end())
                continue;

            ArchiveProgress &info = found->second;
            switch (info.kind) {
                case ArchiveProgress::Kind::WAR: {
                    ProgressTask warTask(p, "war");

                    if (info.nClassFiles > 0) {
                        ProgressTask classesTask(p, "classes");
                        beginArchiveClassProgress(zip);
                        baseVa = loadArchiveClassFiles(zip, fileList, baseVa);
                    }

                    if (settings().engineJvm.loadAllJars && info.nJarFiles > 0) {
                        ProgressTask jarsTask(p, "jars");
                        beginWarJarProgress(zip);
                        for (ModulesJvm::Zipper *childJar: info.childJars) {
                            {
                                ProgressTask jarTask(p, "jar");
                                ProgressTask classesTask(p, "classes");
                                beginArchiveClassProgress(childJar);
                                baseVa = loadArchiveClassFiles(childJar, fileList, baseVa);
                            }
                            noteWarJarHandled(zip);
                        }
                    }
                    break;
                }

                case ArchiveProgress::Kind::JAR:
                    if (!info.parentWar) {
                        ProgressTask jarTask(p, "jar");
                        ProgressTask classesTask(p, "classes");
                        beginArchiveClassProgress(zip);
                        baseVa = loadArchiveClassFiles(zip, fileList, baseVa);
                    }
                    break;
            }
        }
    }

    // Scan instructions for classes
    baseVa = loadDiscoverableClasses(fileList, baseVa);

    // Finally, create the _one_ interpretation
    auto interp = new SgAsmInterpretation;
    auto interpList = jvmComposite->get_interpretations();

    // Add the interpretation to the list
    interpList->get_interpretations().push_back(interp);
    interp->set_parent(interpList);

    // Add files to the interpretation
    auto interpHeaders = interp->get_headers();
    for (SgAsmGenericFile* file: fileList->get_files()) {
        SgAsmGenericHeaderList* headerList = file->get_headers();
        ASSERT_not_null(headerList);
        for (SgAsmGenericHeader *header: headerList->get_headers()) {
            // Add the header to the interpretation. This isn't an AST parent/child link, so don't set the parent ptr.
            interpHeaders->get_headers().push_back(header);
        }
    }

    // The project
    SgProject* project = new SgProject;
    project->get_fileList().push_back(jvmComposite);
    jvmComposite->set_parent(project);

    // Project is JVM only (for now at least) and should not be compiled
    project->set_Jvm_only(true);
    project->skipfinalCompileStep(true);

    return project;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Memory map creation (loading)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool
EngineJvm::areSpecimensLoaded() const {
    // map will be empty for JVM specimens
    return memoryMap()!=nullptr;
}

MemoryMap::Ptr
EngineJvm::loadSpecimens(const std::vector<std::string> &s) {
    try {
        if (!areContainersParsed()) {
            parseContainers(s);
        }
        if (!memoryMap()) {
            memoryMap(MemoryMap::instance());
        }
        // No loading required for JVM files just return the MemoryMap.
        return memoryMap();
    } catch (const std::runtime_error &e) {
        if (settings().engine.exitOnError) {
            mlog[FATAL] <<e.what() <<"\n";
            exit(1);
        } else {
            throw;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Disassembler creation
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Partitioner high-level functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void
EngineJvm::checkCreatePartitionerPrerequisites() const {}

Partitioner::Ptr
EngineJvm::createPartitioner() {
    return createBarePartitioner();
}

void
EngineJvm::runPartitionerInit(const Partitioner::Ptr &partitioner) {
    Sawyer::Message::Stream where(mlog[WHERE]);
    SAWYER_MESG(where) <<"EngineJvm::runPartitionerInit\n";

    // Assume the JVM will return from a function call
    partitioner->autoAddCallReturnEdges(true);
}

void
EngineJvm::runPartitionerRecursive(const Partitioner::Ptr &partitioner) {
    Sawyer::Message::Stream where(mlog[WHERE]);
    ASSERT_not_null(interpretation());
    SgAsmGenericHeaderList *interpHeaders = interpretation()->get_headers();
    ASSERT_not_null(interpHeaders);

    // Create the symbolic execution state
    //
    Architecture::Base::ConstPtr arch = partitioner->architecture();
    SmtSolver::Ptr solver = SmtSolver::instance("none");
    RegisterDictionary::Ptr regdict = arch->registerDictionary();

    BaseSemantics::SValue::Ptr protoval = InstructionSemantics::SymbolicSemantics::SValue::instance();

    BaseSemantics::RegisterState::Ptr rstate = BaseSemantics::RegisterStateGeneric::instance(protoval, regdict);
    BaseSemantics::RegisterState::Ptr istate = BaseSemantics::RegisterStateGeneric::instance(protoval, regdict);
    istate->purpose(BaseSemantics::AddressSpace::Purpose::INTERRUPTS);

    BaseSemantics::MemoryState::Ptr mstate = InstructionSemantics::SymbolicSemantics::MemoryListState::instance(protoval, protoval);

    auto state = BaseSemantics::State::instance(rstate, mstate, istate);
    ASSERT_require(state);

    // Make the symbolic execution state available
    state_ = state;

    // Attach empty functions as targets for invoke of "java/" or bootstrap_method functions (reserved names)
    auto rit = functions_.rbegin();
    while (rit != functions_.rend()) {
        Address va = rit->second;
        std::string name = rit->first;
        if (ByteCode::JvmContainer::isJvmSystemReserved(name)) {
            auto function = Partitioner2::Function::instance(va, name);
            auto block = Partitioner2::BasicBlock::instance(va, partitioner);

            function->insertBasicBlock(va);
            partitioner->attachBasicBlock(block);
            partitioner->attachFunction(function);

            rit++;
        }
        else {
            unresolvedFunctions_[name] = va;
            functions_.erase(std::next(rit).base());
        }
    }

    {
        Progress::Ptr p = partitioner->progress();
        ProgressTask partitionTask(p, "partition");
        ProgressTask headersTask(p, "headers");
        const size_t nHeaders = interpHeaders->get_headers().size();
        size_t nHeadersHandled = 0;
        std::unique_ptr<Sawyer::ProgressBar<size_t>> headerProgress;

        if (nHeaders > 0) {
            headerProgress.reset(new Sawyer::ProgressBar<size_t>(nHeaders, mlog[MARCH], "jvm-partition"));
            headerProgress->suffix(" headers");
            headerProgress->domain(0, nHeaders);
        }
        if (p) {
            if (nHeaders > 0) {
                p->update(Progress::Report("", 0.0, (double)nHeaders));
            } else {
                p->update(Progress::Report("", 1.0, 1.0));
            }
        }

        // Discover functions in the reverse order of headers because classes are loaded from derived to base
        // and functions should be "discovered" from the base class to derived.  Then the Partitioner2::Function
        // (e.g.) Base::<init> will be available for derived classes to call when they are created.
        for (auto rit = interpHeaders->get_headers().rbegin(); rit != interpHeaders->get_headers().rend(); rit++) {
            auto header = *rit;

            auto jfh = isSgAsmJvmFileHeader(header);
            ASSERT_not_null(jfh);

            auto pool = jfh->get_constant_pool();
            ASSERT_not_null(pool);

            ByteCode::Namespace::Ptr ns = ByteCode::Namespace::instance();

            auto classIndex = jfh->get_this_class();
            std::string className = ByteCode::constantPoolEntryName(classIndex, pool);
            ByteCode::JvmClass::Ptr jvmClass = ByteCode::JvmClass::instance(className, ns, jfh);

            // Make the ByteCode analysis class available
            analysisClass_ = jvmClass;

            // Start discovering instructions and forming them into basic blocks and functions
            SAWYER_MESG(where) <<"discovering and populating functions\n";

            jvmClass->partition(partitioner, functions_, p);

            ++nHeadersHandled;
            if (p)
                p->update(Progress::Report("", (double)nHeadersHandled, (double)nHeaders));
            if (headerProgress) {
                headerProgress->value(nHeadersHandled, nHeaders);
                if (nHeadersHandled >= nHeaders)
                    headerProgress.reset();
            }
        }
    }
}

void
EngineJvm::runPartitionerFinal(const Partitioner::Ptr &partitioner) {
    Sawyer::Message::Stream where(mlog[WHERE]);
    SAWYER_MESG(where) <<"EngineJvm::runPartitionerFinal\n";

    // Primary task is to transfer call edges to the indeterminate vertex.
    // First, for efficiency create a map of function names to addresses.
    std::map<std::string,Address> functionNameMap;
    for (auto function: partitioner->functions()) {
      functionNameMap[function->name()] = function->address();
    }

    // By this point all of the functions should have been found
    for (auto pair: unresolvedFunctions_) {
        Address va = pair.second;
        std::string name = pair.first;
        if (functionNameMap.find(name) == functionNameMap.end()) {
          mlog[WARN] << "unresolved function: " << name << " address: " << StringUtility::addrToString(va) <<"\n";
        }
    }

  // Mapping of basic blocks (with edge to indeterminate) to callee basic block
  std::map<BasicBlock::Ptr, Address> sourceBlocks;

  auto indeterminate = partitioner->indeterminateVertex();
  for (auto incoming: indeterminate->inEdges()) {
    if (incoming.value().type() == E_FUNCTION_CALL) {
      auto source = incoming.source();
      if (source->value().type() == V_BASIC_BLOCK) {
        auto value = source->value();
        if (BasicBlock::Ptr bb = source->value().bblock()) {
          SgAsmInstruction* last = bb->instructions().back();
          std::string functionName = last->get_comment();
          mlog[TRACE] << "... indeterminate incoming: " << functionName << " at " << StringUtility::addrToString(bb->address()) << std::endl;
          if (functionNameMap.find(functionName) != functionNameMap.end()) {
            Address functionVa = functionNameMap[functionName];
            sourceBlocks[bb] = functionVa;
          }
        }
      }
    }
  }

  std::cout << "\n";

  // Replace successor edge to the indeterminate node with a new edge to the discovered function.
  // The source basic block must be detached before the successor edge can be replaced.
  for (auto itr = sourceBlocks.begin(); itr != sourceBlocks.end(); itr++) {
    BasicBlock::Ptr bb = itr->first;
    Address va = itr->second;
    auto detached = partitioner->detachBasicBlock(bb);
    mlog[TRACE] << "replacing successor (to indeterminate) from bb(va): " << StringUtility::addrToString(bb->address())
                << " : to " << StringUtility::addrToString(va) << "\n";
    detached->clearSuccessors();
    detached->insertSuccessor(va, 64, EdgeType::E_FUNCTION_CALL, Confidence::PROVED);
    partitioner->attachBasicBlock(detached);
  }

  // partitioner->dumpCfg(std::cout, "Worker:", true, false);

}

Partitioner::Ptr
EngineJvm::partition(const std::vector<std::string> &fileNames) {
    try {
        for (const std::string &fileName: fileNames) {
            if (isRbaFile(fileName) && fileNames.size() != 1)
                throw Exception("specifying an RBA file excludes all other inputs");
        }
        if (fileNames.size() == 1 && isRbaFile(fileNames[0])) {
            auto partitioner = Partitioner::instanceFromRbaFile(fileNames[0], Serialization::BINARY);
            interpretation(partitioner->interpretation());
            return partitioner;
        } else {
            if (!areSpecimensLoaded()) {
                loadSpecimens(fileNames); }
            obtainArchitecture();
            Partitioner::Ptr partitioner = createPartitioner();
            runPartitioner(partitioner);
            return partitioner;
        }
    } catch (const std::runtime_error &e) {
        if (settings().engine.exitOnError) {
            mlog[FATAL] <<e.what() <<"\n";
            exit(1);
        } else {
            throw;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Partitioner mid-level operations
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void
EngineJvm::discoverBasicBlocks(const PartitionerPtr& partitioner, const ByteCode::Method* m) {
    //TODO: Fix this mess please.

    const ByteCode::JvmMethod* method{dynamic_cast<const ByteCode::JvmMethod*>(m)};
    const SgAsmInstructionList* instructions{method->instructions()};
    Address startVa{method->code().offset()};
    Address va{startVa};

    // Not sure where this goes (I suppose this discovers basic blocks), at least part of it, as it disassembles
    method->decode(architecture()->newInstructionDecoder());

    //TODO:
    // 1. create basic block
    // 2. add vertices
    BasicBlockPtr bb = BasicBlock::instance(va, partitioner);
    //TODO: bb should be attached after instructions are added
    partitioner->attachBasicBlock(bb);

    for (auto insn: instructions->get_instructions()) {
        bb->append(partitioner, insn);
        va += insn->get_size();

        //TODO: void insertSuccessor(Address va, size_t nBits, EdgeType type=E_NORMAL, Confidence confidence=ASSUMED);

    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Partitioner low-level stuff
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Build AST
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//TODO:: remove this method
SgAsmBlock*
EngineJvm::buildAst(const std::vector<std::string> &fileNames) {
    ROSE_UNUSED(fileNames);
#ifdef IMPLEMENT_BUILD_AST
  using std::cout;
  using namespace ModulesJvm;
//TODO: More ...
  std::string filename{fileNames[0]};

//TODO::jar files
  if (isJavaClassFile(filename)) {
  }
  else {
    return nullptr;
  }

  auto specimen = fileNames;
  MemoryMap::Ptr map = loadSpecimens(specimen);

  auto gf = new SgAsmGenericFile{};
  gf->parse(filename); /* this loads file into memory, does no reading of file */
  auto header = new SgAsmJvmFileHeader(gf);

  // Check AST
  ASSERT_require(header == gf->get_header(SgAsmGenericFile::FAMILY_JVM));
  ASSERT_require(header->get_parent() == gf);

  header->parse();
#ifdef DEBUG_ON
  cout << "\n --- JVM file header ---\n";
  header->dump(stdout, "    jfh:", 0);

  cout << "\n---------- JVM Analysis -----------------\n\n";
#endif

  // explicit JvmClass(std::shared_ptr<Namespace> ns, SgAsmJvmFileHeader* jfh);
//TODO: Integrate with analysisClass()
  Rose::BinaryAnalysis::ByteCode::JvmClass* jvmClass = new ByteCode::JvmClass(/*namespace:TODO*/nullptr,header);

#ifdef DEBUG_ON
  cout << "class '" << jvmClass->name() << "'" << endl;
  cout << "----------------\n";
  cout << "   super: " << jvmClass->super_name() << "\n\n";

  cout << "constant pool\n";
  cout << "-----------\n";
  jvmClass->constant_pool()->dump(stdout, "", 1);
  cout << "-----------\n\n";

  if (jvmClass->interfaces().size() > 0) {
    cout << "interfaces\n";
    cout << "-----------\n";
    for (auto interface : jvmClass->interfaces()) {
      cout << "   interface: " << interface->name() << endl;
    }
    cout << "-----------\n\n";
  }

  if (jvmClass->fields().size() > 0) {
    cout << "fields\n";
    cout << "-----------\n";
    for (auto field : jvmClass->fields()) {
      cout << "   field: " << field->name() << endl;
    }
    cout << "-----------\n\n";
  }

  if (jvmClass->attributes().size() > 0) {
    cout << "attributes\n";
    cout << "-----------\n";
    for (auto attribute : jvmClass->attributes()) {
      cout << "   attribute: " << attribute->name() << endl;
    }
    cout << "-----------\n\n";
  }
#endif

  for (auto method : jvmClass->methods()) {
#ifdef DEBUG_ON
    cout << "method '" << method->name() << endl;
    cout << "-----------\n";
#endif

#ifdef DEBUG_ON
    for (auto insn : method->instructions()->get_instructions()) {
      cout << "   : " << insn->get_anyKind() << ": " << insn->get_mnemonic() << ": '"
           << insn->description() << "' " << " size:" << insn->get_size()
           << " va:" << insn->get_address();
      if (insn->terminatesBasicBlock()) cout << " :terminates";
      cout << endl;
    }
    cout << "-----------\n\n";
#endif
  }

  // Bonus
#ifdef DEBUG_ON
  cout << "--- strings ---\n";
  for (auto str : jvmClass->strings()) {
    cout << "   " << str << endl;
  }
  cout << "-----------\n\n";
#endif

  // Dump diagnostics from the partition
#ifdef DEBUG_ON
  for (auto method : jvmClass->methods()) {
    cout << "\nmethod: " << method->name() << endl;
    for (auto block : method->blocks()) {
      cout << "--------------block------------\n";
      for (auto insn : block->instructions()) {
        auto va = insn->get_address();
        cout << "... insn: " << insn << " va:" << va << " :" << insn->get_mnemonic()
             << " nOperands:" << insn->nOperands()
             << " terminates:" << insn->terminatesBasicBlock() << endl;
        for (auto op : insn->get_operandList()->get_operands()) {
          if (op->asUnsigned()) {
            cout << "      unsigned operand:" << *(op->asUnsigned()) << endl;
          }
          else if (op->asSigned()) {
            cout << "       signed operand:" << *(op->asSigned()) << endl;
          }
        }
      }
      // Explore block methods
      cout << "      :nInstructions:" << block->nInstructions() << endl;
      cout << "      :address:" << block->address() << endl;
      cout << "      :fallthroughVa:" << block->fallthroughVa() << endl;
      cout << "      :isEmpty:" << block->isEmpty() << endl;
      cout << "      :nDataBlocks:" << block->nDataBlocks() << endl;
      if (block->isFunctionCall().isCached()) cout << "      :isFunctionCall:" << block->isFunctionCall().get() << endl;
      if (block->isFunctionReturn().isCached()) cout << "      :isFunctionReturn:" << block->isFunctionReturn().get() << endl;
      if (block->successors().isCached()) cout << "      :#successors:" << block->successors().get().size() << endl;
    }
  }
#endif

  // Create graphviz DOT file
  jvmClass->digraph();
#endif // IMPLEMENT_BUILD_AST (or maybe not)

  return nullptr;
}

const std::vector<std::string>&
EngineJvm::classPath() const {
    return settings().engineJvm.classPath;
}

const std::string&
EngineJvm::isaName() const {
    return settings().disassembler.isaName;
}

void
EngineJvm::isaName(const std::string &s) {
    settings().disassembler.isaName = s;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Settings and Properties
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Architecture::BaseConstPtr
EngineJvm::obtainArchitecture() {
    if (!architecture_) {
        architecture_ = Architecture::findByName("jvm").orThrow();
    }
    return architecture_;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      JVM Module
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Disassembly and partitioning utility functions for JVM. */
namespace ModulesJvm {

/** Returns true if named file exists and is a Java class file */
bool isJavaClassFile(const fs::path &file) {
    if (!fs::exists(file) || file.extension() != ".class") {
        return false;
    }

    MemoryMap::Ptr map = MemoryMap::instance();
    if (0 == map->insertFile(file.string(), 0)) {
        return false; // file cannot be mmap'd
    }

    uint8_t magic[4];
    if (4 == map->at(0).limit(4).read(magic).size()) {
        // All JVM files have magic number, 0xCAFEBABE
        if (magic[0] == 0xCA && magic[1] == 0xFE && magic[2] == 0xBA && magic[3] == 0xBE) {
            return true;
        }
    }
    return false;
}

/** Returns true if named file exists and is a Java jar file */
bool isJavaJarFile(const fs::path &file) {
    if (!fs::exists(file) || file.extension() != ".jar") {
        return false;
    }

    // Check file for magic number, 0xCAFEBABE
    MemoryMap::Ptr map = MemoryMap::instance();
    if (0 == map->insertFile(file.string(), 0)) {
        return false; // file cannot be mmap'd
    }

    uint8_t magic[4];
    if (4 == map->at(0).limit(4).read(magic).size()) {
        if (magic[0] == 'P' && magic[1] == 'K' && magic[2] == 0x03 && magic[3] == 0x04) {
            return true;
        }
    }
    return false;
}

/** Returns true if named file exists and is a Java war file */
bool isJavaWarFile(const fs::path &file) {
    if (!fs::exists(file) || file.extension() != ".war") {
        return false;
    }

    // Check file for magic number, 0xCAFEBABE
    MemoryMap::Ptr map = MemoryMap::instance();
    if (0 == map->insertFile(file.string(), 0)) {
        return false; // file cannot be mmap'd
    }

    uint8_t magic[4];
    if (4 == map->at(0).limit(4).read(magic).size()) {
        if (magic[0] == 'P' && magic[1] == 'K' && magic[2] == 0x03 && magic[3] == 0x04) {
            return true;
        }
    }
    return false;
}

/** Searchs the classpath setting for the path to a fully qualified class name */
fs::path pathToClassFile(const std::string &fqcn, const std::vector<std::string> &classpath) {
    fs::path classFilePath{};

    // Form internal representation of the FQCN (ok if fqcn already converted)
    std::string className{fqcn};
    std::replace(className.begin(), className.end(), '.', '/');
    className += ".class";

    // Search classpath for the class location
    std::string path;
    for (auto p : classpath) {
        fs::path path{p};
        if (CommandlineProcessing::isJavaJarFile(p) && ModulesJvm::present(fqcn, p)) {
            return path; // found in the jar
        }
        else if (p == ".") {
            fs::path dir = fs::current_path();
            classFilePath = dir / className;

            if (fs::exists(classFilePath)) {
              return classFilePath; // success
            }

            // Class not directly found in current directory, look in jar files
            for (const auto &entry : fs::directory_iterator(dir)) {
                if (fs::is_regular_file(entry)) {
                    const fs::path &filePath = entry.path();
                    if (filePath.extension() == ".jar") {
                        // Warning: this search is expensive because it opens, searches, then deletes jar files
                        if (ModulesJvm::present(fqcn, filePath.string())) {
                            return classFilePath;
                        }
                    }
                }
            }
        }
        else {
          classFilePath = path / className;
        }

        if (fs::exists(classFilePath)) {
            return classFilePath; // success
        }
    }

    return fs::path{};
}

/**  Searchs the path for the presence of a fully qualified class name.
 *
 *   A relatively expensive operation because there is no resuse of results.
 */
bool
present(const std::string &fqcn, const std::string &path) {
    bool result{false};

    // Form internal representation of the FQCN (ok if fqcn already converted)
    std::string className{fqcn};
    std::replace(className.begin(), className.end(), '.', '/');
    className += ".class";

    if (ModulesJvm::isJavaJarFile(path)) {
        // Search the jar file for the class
        auto gf = new SgAsmGenericFile{};
        gf->parse(path); /* this loads jar file into memory, does no reading of file */
        auto zip = new ModulesJvm::Zipper(gf);
        if (zip->present(className)) {
            result = true;
        }
        // cleanup
        delete zip; delete gf;
    }

    return result;
}

} // namespace ModulesJvm

} // namespace
} // namespace
} // namespace

#endif
