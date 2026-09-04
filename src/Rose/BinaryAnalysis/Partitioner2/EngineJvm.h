#ifndef ROSE_BinaryAnalysis_Partitioner2_EngineJvm_H
#define ROSE_BinaryAnalysis_Partitioner2_EngineJvm_H
#include <featureTests.h>
#ifdef ROSE_ENABLE_BINARY_ANALYSIS

#include <Rose/BinaryAnalysis/ByteCode/Jvm.h>
#include <Rose/BinaryAnalysis/Partitioner2/Engine.h>
#include <Rose/BinaryAnalysis/Partitioner2/ModulesJvm.h>

#include <Sawyer/ProgressBar.h>

#include <memory>
#include <utility>

namespace Rose {
namespace BinaryAnalysis {
namespace Partitioner2 {

/** Engine for Java Virtual Machine (JVM) specimens.
 *
 *  This engine is reponsible for creating a partitioner for a specimen that has byte code for the Java Virtual Machine.
 *
 *  This engine provides an @ref instance static member function that instantiates an engine of this type on the heap and returns a
 *  shared-ownership pointer to the instance. Refer to the base class, @ref Partitioner2::Engine, to learn how to instantiate
 *  engines from factories. */
class EngineJvm: public Engine {
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                  Types
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
    /** Shared ownership pointer. */
    using Ptr = EngineJvmPtr;

    using Super = Engine;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Data members
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
private:
    struct ArchiveProgress {
        enum class Kind { JAR, WAR };

        Kind kind = Kind::JAR;
        ModulesJvm::Zipper *parentWar = nullptr;
        size_t nClassFiles = 0;
        size_t nClassesHandled = 0;
        size_t nJarFiles = 0;
        size_t nJarsHandled = 0;
        std::vector<ModulesJvm::Zipper*> childJars;
        std::unique_ptr<Sawyer::ProgressBar<size_t>> classProgressBar;
        std::unique_ptr<Sawyer::ProgressBar<size_t>> jarProgressBar;
    };

    // Mapping of function names to virtual address
    std::map<std::string, Address> functions_;

    // Mapping of unresolved (added to partitioner) function names to virtual address
    std::map<std::string, Address> unresolvedFunctions_;

    // Listing of open jar files (maybe should be Zippers with
    std::vector<ModulesJvm::Zipper*> jars_; // Zipper owns SgAsmGenericFile*, ie, Zipper{gf}, yeah, will have buffer
                                            // Zipper.find(className)

    // Multi-release JAR handling currently chooses the highest META-INF/versions/N class present in an archive instead of
    // selecting for a configured Java runtime version. This differs from JVM semantics, but keeps eagerly loaded specimens
    // self-consistent until runtime-version selection is exposed.
    std::map<ModulesJvm::Zipper*, std::map<std::string, std::string>> archivePreferredClassFiles_;

    // Progress totals for loaded zip containers.
    std::map<ModulesJvm::Zipper*, ArchiveProgress> archiveProgress_;

    static constexpr Address vaDefaultIncrement{4*1024};

    Address nextFunctionVa_;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
    /** Default constructor.  Constructor is deleted and class noncopyable. */
    EngineJvm() = delete;
    EngineJvm(const EngineJvm&) = delete;
    EngineJvm& operator=(const EngineJvm&) = delete;

protected:
    explicit EngineJvm(const Settings&);

public:
    ~EngineJvm();

    /** Allocating constructor. */
    static Ptr instance();

    /** Allocating constructor with settings. */
    static Ptr instance(const Settings&);

    /** Allocate a factory. */
    static Ptr factory();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                  Overrides documented in the base class
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
    /** Predicate for matching a concrete engine factory by command-line parser result and specimen. */
    bool matchFactory(const Sawyer::CommandLine::ParserResult &result, const std::vector<std::string> &specimen) const override;

    EnginePtr instanceFromFactory(const Settings&) override;

    std::list<Sawyer::CommandLine::SwitchGroup> commandLineSwitches() override;
    std::pair<std::string, std::string> specimenNameDocumentation() override;

    static Ptr promote(const Engine::Ptr&);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                  The very top-level use case
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
    /** Most basic usage of the partitioner.
     *
     *  This method does everything from parsing the command-line to generating an abstract syntax tree. If all is successful,
     *  then an abstract syntax tree is returned.  The return value is a SgAsmBlock node that contains all the detected
     *  functions. If the specimen consisted of an ELF or PE container then the parent nodes of the returned AST will lead
     *  eventually to an SgProject node.
     *
     *  The command-line can be provided as a typical @c argc and @c argv pair, or as a vector of arguments. In the latter case, the
     *  vector should not include `argv[0]` or `argv[argc]` (which is always a null pointer).
     *
     *  The command-line supports a "--help" (or "-h") switch to describe all other switches and arguments, essentially
     *  generating output like a Unix man(1) page.
     *
     *  The @p purpose should be a single line string that will be shown in the title of the man page and should
     *  not start with an upper-case letter, a hyphen, white space, or the name of the command. E.g., a disassembler tool might
     *  specify the purpose as "disassembles a binary specimen".
     *
     *  The @p description is a full, multi-line description written in the Sawyer markup language where "@" characters have
     *  special meaning.
     *
     *  If an `std::runtime_exception` occurs and the @ref EngineSettings::exitOnError property is set, then the exception is
     *  caught, its text is emitted to the partitioner's fatal error stream, and `exit(1)` is invoked.
     *
     * @{ */
    virtual SgAsmBlock* frontend(const std::vector<std::string> &args,
                                 const std::string &purpose, const std::string &description) override;
    /** @} */

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                  Basic top-level steps
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                  Command-line parsing
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
    virtual void checkSettings() override;

    /** Command-line switches related to the JVM engine behavior.
     *
     *  The switches are configured to adjust the specified settings object when parsed. */
    static Sawyer::CommandLine::SwitchGroup jvmSwitches(JvmSettings&);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                  Container parsing
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
    /** Parse specimen binary containers.
     *
     *  Parses the ELF and PE binary containers to create an abstract syntax tree (AST).  If @p fileNames contains names that
     *  are recognized as raw data or other non-containers then they are skipped over at this stage but processed during the
     *  @ref loadSpecimens stage.
     *
     *  This method tries to allocate a disassembler if none is set and an ISA name is specified in the settings, otherwise the
     *  disassembler is chosen later.  It also resets the interpretation to be the return value (see below), and clears the
     *  memory map.
     *
     *  Returns a binary interpretation (perhaps one of many). ELF files have only one interpretation; PE files have a DOS and
     *  a PE interpretation and this method will return the PE interpretation. The user may, at this point, select a different
     *  interpretation. If the list of names has nothing suitable for ROSE's @c frontend function (the thing that does the
     *  container parsing) then the null pointer is returned.
     *
     *  If an `std::runtime_exception` occurs and the @ref EngineSettings::exitOnError property is set, then the exception is
     *  caught, its text is emitted to the partitioner's fatal error stream, and `exit(1)` is invoked.
     *
     * @{ */
    virtual SgAsmInterpretation* parseContainers(const std::vector<std::string> &fileNames) override;

    /** Determine whether a specimen name is a non-container.
     *
     *  Certain strings are recognized as special instructions for how to adjust a memory map and are not intended to be passed
     *  to ROSE's @c frontend function.  This predicate returns true for such strings. */
    virtual bool isNonContainer(const std::string&) override;

    /** Returns true if containers are parsed.
     *
     *  Specifically, returns true if the engine has a non-null interpretation.  If it has a null interpretation then
     *  @ref parseContainers might have already been called but no binary containers specified, in which case calling it again
     *  with the same file names will have no effect. */
    virtual bool areContainersParsed() const override;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                  Load specimens
    //
    // top-level: loadSpecimens
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:

    /** Returns true if specimens are loaded.
     *
     *  Specifically, returns true if the memory map is non-empty. */
    virtual bool areSpecimensLoaded() const override;

    virtual MemoryMapPtr loadSpecimens(const std::vector<std::string> &fileNames = std::vector<std::string>()) override;

    /** Load a war file by opening its contents
     *
     *  Stores the location of class and jar files found as offsets into the war file
     *
     *  Returns true if war file is loaded
     *
     * @{ */
    bool loadWarFile(const std::string &);

    /** Load a jar file by opening its contents
     *
     *  Stores the location of class files found as offsets into the jar file
     *
     *  Returns true if jar file is loaded
     *
     * @{ */
    bool loadJarFile(const std::string &);

    /** Load a jar file from a zipped container by opening its contents
     *
     *  Returns true if jar file is loaded
     *
     * @{ */
    bool loadJarFile(const std::string &, ModulesJvm::Zipper* zip);

    /** Load a class file by parsing its contents at the given address.
     *
     *  Returns the AST node representing the file.
     *
     * @{ */
    Address loadClassFile(boost::filesystem::path, SgAsmGenericFileList*, Address);

    /** Recursively find and load all super classes starting at the given address.
     *
     *  Returns the address of the last preloaded class.
     *
     * @{ */
    Address loadBaseClassAndInterfaces(const std::string &, SgAsmGenericFileList*, Address);

    /** Load classes discoverable from the file list starting at the given address.
     *
     *  Loads all classes and superclasses discovered by examining instructions.
     *
     *  Returns the next available address.
     *
     * @{ */
    Address loadDiscoverableClasses(SgAsmGenericFileList*, Address);

    /** Load class and super class starting at the given address.
     *
     *  Loads the class and super class from the file header and adds new files to the file list.
     *
     *  Returns the next available address.
     *
     * @{ */
    Address loadClass(uint16_t, SgAsmJvmConstantPool*, SgAsmGenericFileList*, Address);

    /** Path to the given fully qualified class name (FQCN).
     *
     *  Returns the path to the class file (with .class extension) by searching loaded jar files.
     *  If the class file can't be found, an empty path is returned.
     *
     * @{ */
    boost::filesystem::path pathToClass(const std::string &fqcn);

    /** Discover function calls (invoke instructions) made from a method.
     *
     *  Stores results in a map of fully resolved function names to unique virtual addresses.  Also
     *  stores the class names of the discovered functions.
     *
     * @{ */
    void discoverFunctionCalls(SgAsmJvmMethod*, SgAsmJvmConstantPool*, std::map<std::string,Address> &, std::set<std::string> &);

private:
    void registerWarProgress(ModulesJvm::Zipper*);
    void registerJarProgress(ModulesJvm::Zipper*, ModulesJvm::Zipper *parentWar = nullptr);
    const std::map<std::string, std::string>& preferredArchiveClassFiles(ModulesJvm::Zipper*);
    Address loadArchiveClassFiles(ModulesJvm::Zipper*, SgAsmGenericFileList*, Address);
    void beginArchiveClassProgress(ModulesJvm::Zipper*);
    void beginWarJarProgress(ModulesJvm::Zipper*);
    void noteWarJarHandled(ModulesJvm::Zipper*);
    void noteArchiveClassHandled(ModulesJvm::Zipper*);
    void updateLoaderProgress(size_t current, size_t total) const;
    void createClassProgressBar(ArchiveProgress&, const std::string&) const;
    void createJarProgressBar(ArchiveProgress&, const std::string&) const;

public:
    /** Partition instructions into basic blocks and functions.
     *
     *  Disassembles and organizes instructions into basic blocks and functions with these steps:
     *
     *  @li If the specimen is not loaded (@ref areSpecimensLoaded) then call @ref loadSpecimens. The no-argument version of
     *  this function requires that specimens have already been loaded.
     *
     *  @li Obtain a disassembler.
     *
     *  @li Create a partitioner by calling @ref createPartitioner.
     *
     *  @li Run the partitioner by calling @ref runPartitioner.
     *
     *  Returns the partitioner that was used and which contains the results.
     *
     *  If an `std::runtime_exception` occurs and the @ref EngineSettings::exitOnError property is set, then the exception is
     *  caught, its text is emitted to the partitioner's fatal error stream, and `exit(1)` is invoked.
     *
     * @{ */
    virtual PartitionerPtr partition(const std::vector<std::string> &fileNames = std::vector<std::string>()) override;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Disassembler
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Partitioner high-level functions
    //
    // top-level: partition
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:

    /** Check that we have everything necessary to create a partitioner. */
    virtual void checkCreatePartitionerPrerequisites() const override;

    /** Create a partitioner for JVM.
     *
     *  Returns a partitioner that is tuned to operate on a JVM instruction set architecture. The @ref memoryMap must be assigned
     *  already, either explicitly or as the result of earlier steps. */
    PartitionerPtr createJvmTunedPartitioner();

    virtual PartitionerPtr createPartitioner() override;
    virtual void runPartitionerInit(const PartitionerPtr&) override;
    virtual void runPartitionerRecursive(const PartitionerPtr&) override;
    virtual void runPartitionerFinal(const PartitionerPtr&) override;


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Partitioner mid-level functions
    //
    // These are the functions called by the partitioner high-level stuff.  These are sometimes overridden in subclasses,
    // although it is more likely that the high-level stuff is overridden.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Partitioner low-level functions
    //
    // These are functions that a subclass seldom overrides, and maybe even shouldn't override because of their complexity or
    // the way the interact with one another.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Build AST
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
    /** Obtain an abstract syntax tree.
     *
     *  Constructs a new abstract syntax tree (AST) from partitioner information with these steps:
     *
     *  @li If the partitioner has not been run yet, then do that now with the same arguments.  The zero-argument version
     *      invokes the zero-argument @ref partition, which requires that the specimen has already been loaded by @ref
     *      loadSpecimens.
     *
     *  @li Call Modules::buildAst to build the AST.
     *
     *  If an `std::runtime_exception` occurs and the @ref EngineSettings::exitOnError property is set, then the exception is
     *  caught, its text is emitted to the partitioner's fatal error stream, and `exit(1)` is invoked.
     *
     * @{ */
    virtual SgAsmBlock* buildAst(const std::vector<std::string> &fileNames = std::vector<std::string>()) override;
    /** @} */

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                  Settings and properties
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
    /** Setting: JVM class path
     *
     *  The class path is the path that the Java runtime environment searches for classes and other resource files.
     *
     * @{ */
    const std::vector<std::string>& classPath() const;

    /** Property: Instruction set architecture name.
     *
     *  The instruction set architecture name is used to obtain a disassembler and overrides the disassembler that would
     *  otherwise be found by examining the binary container.
     *
     * @{ */
    const std::string& isaName() const /*final*/;
    virtual void isaName(const std::string&);
    /** @} */

    /** Property: Starting addresses for disassembly.
     *
     *  This is a list of addresses where functions will be created in addition to those functions discovered by examining the
     *  binary container.
     *
     * @{ */
    const std::vector<Address>& functionStartingVas() const /*final*/;
    std::vector<Address>& functionStartingVas() /*final*/;
    /** @} */

    /** Replacement for ::frontend for Jvm files only */
    SgProject* roseFrontendReplacement(const std::vector<std::string> &fileNames);

    Architecture::BaseConstPtr obtainArchitecture() override;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                  Internal stuff
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
protected:
    // Similar to ::frontend but a lot less complicated.
    virtual SgProject* roseFrontendReplacement(const std::vector<boost::filesystem::path> &fileNames) override;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      JVM Module
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Disassembly and partitioning utility functions for JVM. */
namespace ModulesJvm {

/** True if named file exists and is a Java class file.
 *
 *  Class files have a file name with a ".class" extension */
bool isJavaClassFile(const boost::filesystem::path&);

/** True if named file exists and is a Java jar file.
 *
 *  Jar files have a file name with a ".jar" extension */
bool isJavaJarFile(const boost::filesystem::path&);

/** True if named file exists and is a Java war file.
 *
 *  War files have a file name with a ".war" extension */
bool isJavaWarFile(const boost::filesystem::path&);

/**
 * @brief Searchs the classpath setting for the path to a fully qualified class name.
 *
 * This function takes a fully qualified class name (FQCN) and converts it into
 * a corresponding filesystem path by replacing the namespace separators ('.')
 * with directory separators appropriate for the operating system. It then
 * searches for the class in the classpath setting. If found, the resulting
 * path can be used to locate the class file within a directory structure, either
 * in the file system or within a jar file. If the class can't be located, an empty
 * path is returned.
 *
 * @param fqcn The fully qualified class name, e.g., "com.example.MyClass".
 * @param cp The classpath. A ':' separated list of paths to jar files or directories".
 * @return A boost::filesystem::path representing the location of the class file,
 *         e.g., "com/example/MyClass.class".
 */
boost::filesystem::path pathToClassFile(const std::string &fqcn, const std::vector<std::string> &cp);

/**
 * @brief Searchs the path for the presence of a fully qualified class name.
 *
 *  Searches all classes and jar files found in the path for the fully qualified class
 *  name (FQCN).  Namespace separators in the class name may be '.' or '/'.
 *  Path separator in the classpath is ':'.
 *
 * @param fqcn The fully qualified class name, e.g., "com.example.MyClass".
 * @param path Path to a jar file, class file, or directory.
 * @return true if the class is found.
 *
 **/
bool present(const std::string &fqcn, const std::string &path);

} // namespace ModulesJvm

} // namespace
} // namespace
} // namespace

#endif
#endif
