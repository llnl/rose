#include <featureTests.h>
#ifdef ROSE_ENABLE_BINARY_ANALYSIS
#include <sage3basic.h>                                 // needed for `::SgProject`
#include <Rose/BinaryAnalysis/Partitioner2/EngineBinary.h>

#include <Rose/As.h>
#include <Rose/AST/Traversal.h>
#include <Rose/BinaryAnalysis/Architecture/Base.h>
#include <Rose/BinaryAnalysis/BinaryLoader.h>
#include <Rose/BinaryAnalysis/ByteCode/Cil.h>
#include <Rose/BinaryAnalysis/Debugger/Linux.h>
#include <Rose/BinaryAnalysis/Disassembler/Aarch32.h>
#include <Rose/BinaryAnalysis/Disassembler/Aarch64.h>
#include <Rose/BinaryAnalysis/Disassembler/M68k.h>
#include <Rose/BinaryAnalysis/Disassembler/Mips.h>
#include <Rose/BinaryAnalysis/Disassembler/Powerpc.h>
#include <Rose/BinaryAnalysis/Disassembler/X86.h>
#include <Rose/BinaryAnalysis/Dwarf/Exception.h>
#include <Rose/BinaryAnalysis/MemoryMap.h>
#include <Rose/BinaryAnalysis/Partitioner2/BasicBlock.h>
#include <Rose/BinaryAnalysis/Partitioner2/DataBlock.h>
#include <Rose/BinaryAnalysis/Partitioner2/Function.h>
#include <Rose/BinaryAnalysis/Partitioner2/IndirectControlFlow.h>
#include <Rose/BinaryAnalysis/Partitioner2/Modules.h>
#include <Rose/BinaryAnalysis/Partitioner2/ModulesElf.h>
#include <Rose/BinaryAnalysis/Partitioner2/ModulesLinux.h>
#include <Rose/BinaryAnalysis/Partitioner2/ModulesM68k.h>
#include <Rose/BinaryAnalysis/Partitioner2/ModulesMips.h>
#include <Rose/BinaryAnalysis/Partitioner2/ModulesPe.h>
#include <Rose/BinaryAnalysis/Partitioner2/ModulesPowerpc.h>
#include <Rose/BinaryAnalysis/Partitioner2/ModulesX86.h>
#include <Rose/BinaryAnalysis/Partitioner2/Partitioner.h>
#include <Rose/BinaryAnalysis/Partitioner2/Semantics.h>
#include <Rose/BinaryAnalysis/Partitioner2/Thunk.h>
#include <Rose/BinaryAnalysis/Partitioner2/Utility.h>
#include <Rose/BinaryAnalysis/RelativeVirtualAddress.h>
#include <Rose/BinaryAnalysis/SRecord.h>
#include <Rose/BinaryAnalysis/SymbolicExpression.h>
#include <Rose/BinaryAnalysis/VxworksTerminal.h>
#include <Rose/CommandLine.h>
#include <Rose/Diagnostics.h>
#include <Rose/Progress.h>

#include <BinaryVxcoreParser.h>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <rose_getline.h>
#include <rose_strtoull.h>
#include <Sawyer/FileSystem.h>
#include <Sawyer/GraphAlgorithm.h>
#include <Sawyer/GraphTraversal.h>
#include <Sawyer/Stopwatch.h>

using namespace Rose::Diagnostics;

namespace Rose {
namespace BinaryAnalysis {
namespace Partitioner2 {

EngineBinary::EngineBinary(const Settings &settings)
    : Engine("binary", settings) {
    init();
}

EngineBinary::~EngineBinary() {}

EngineBinary::Ptr
EngineBinary::instance() {
    return instance(Settings());
}

EngineBinary::Ptr
EngineBinary::instance(const Settings &settings) {
    return Ptr(new EngineBinary(settings));
}

EngineBinary::Ptr
EngineBinary::factory() {
    return Ptr(new EngineBinary(Settings()));
}

bool
EngineBinary::matchFactory(const Sawyer::CommandLine::ParserResult &, const std::vector<std::string> &) const {
    // EngineBinary is the default. It treats all inputs as binaries even if they are something else. Therefore,
    // this engine class is usually the first one registered and therefore the last one matched.
    return true;
}

Engine::Ptr
EngineBinary::instanceFromFactory(const Settings &settings) {
    ASSERT_require(isFactory());
    return instance(settings);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Utility functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void
EngineBinary::init() {
    functionMatcherThunks_ = ThunkPredicates::functionMatcherThunks();
    functionSplittingThunks_ = ThunkPredicates::allThunks();
}

void
EngineBinary::reset() {
    binaryLoader_ = BinaryLoader::Ptr();
}

// Increment the address as far as possible while avoiding overflow.
static Address
incrementAddress(Address va, Address amount, Address maxaddr) {
    if (maxaddr - va < amount)
        return maxaddr;
    return va + amount;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Top-level, do everything functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SgAsmBlock*
EngineBinary::frontend(const std::vector<std::string> &args, const std::string &purpose, const std::string &description) {
    try {
        std::vector<std::string> specimenNames = parseCommandLine(args, purpose, description).unreachedArgs();
        if (specimenNames.empty())
            throw std::runtime_error("no binary specimen specified; see --help");
        return buildAst(specimenNames);
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

std::list<Sawyer::CommandLine::SwitchGroup>
EngineBinary::commandLineSwitches() {
    std::list<Sawyer::CommandLine::SwitchGroup> retval = Super::commandLineSwitches();
    retval.push_back(engineSwitches(settings().engine));
    retval.push_back(loaderSwitches(settings().loader));
    retval.push_back(disassemblerSwitches(settings().disassembler));
    retval.push_back(partitionerSwitches(settings().partitioner));
    retval.push_back(IndirectControlFlow::commandLineSwitches(settings().icf));
    retval.push_back(astConstructionSwitches(settings().astConstruction));
    return retval;
}

// class method
Sawyer::CommandLine::SwitchGroup
EngineBinary::engineSwitches(EngineSettings &settings) {
    using namespace Sawyer::CommandLine;

    SwitchGroup sg("Binary engine switches");
    sg.name("binary-engine");
    sg.doc("These switches are used by Rose::BinaryAnalysis::Partitioner2::EngineBinary to control the overall operation of "
           "the engine. Additional switches control different aspects of the engine's behavior such as loading, partitioning, "
           "building the AST, etc.");

    sg.insert(Switch("config")
              .argument("names", listParser(anyParser(settings.configurationNames), ":"))
              .explosiveLists(true)
              .whichValue(SAVE_ALL)
              .doc("Configuration files or directories containing configuration files that control the instruction decoding "
                   "and partitioning process. If a directory is specified, it is searched recursively for files whose names have "
                   "either a \".json\" or a \".yaml\" extension.   Each file is parsed and used to configure the partitioner. "
                   "This switch may appear more than once and/or a comma-separated list of names can be specified. This switch "
                   "has no effect if the input is a ROSE Binary Analysis (RBA) file since the partitioner is constructed and "
                   "initialized from the RBA file instead.\n\n" + Configuration::fileFormatDoc()));
    return sg;
}

// class method
Sawyer::CommandLine::SwitchGroup
EngineBinary::loaderSwitches(LoaderSettings &settings) {
    using namespace Sawyer::CommandLine;

    SwitchGroup sg("Binary loading switches");
    sg.name("binary-load");
    sg.doc("These switches are used by Rose::BinaryAnalysis::Partitioner2::EngineBinary to control how a binary specimen "
           "is mapped into a virtual address space. These switches have no effect if the input is a ROSE Binary Analysis (RBA) "
           "file, since the loader steps in such an input have already been completed.");

    sg.insert(Switch("remove-zeros")
              .argument("size", nonNegativeIntegerParser(settings.deExecuteZerosThreshold), "128")
              .doc("This switch causes execute permission to be removed from sequences of contiguous zero bytes. The "
                   "switch argument is the minimum number of consecutive zeros that will trigger the removal, and "
                   "defaults to 128.  An argument of zero disables the removal.  Each interval of zeros is narrowed "
                   "according to the @s{remove-zeros-narrow} switch before execute permission is removed. When this switch "
                   "is not specified at all, this tool assumes a value of " +
                   StringUtility::plural(settings.deExecuteZerosThreshold, "bytes") + "."));
    sg.insert(Switch("remove-zeros-narrow")
              .argument("@v{begin},@v{end}",
                        listParser(nonNegativeIntegerParser(settings.deExecuteZerosLeaveAtFront))
                        ->nextMember(nonNegativeIntegerParser(settings.deExecuteZerosLeaveAtBack))
                        ->exactly(2))
              .doc("If @s{remove-zeros} is active then each interval of zeros detected by that analysis is narrowed by "
                   "removing @v{begin} and @v{end} bytes from the interval before execute permission is removed. This "
                   "effectively makes the analysis less greedy and less apt to remove zeros from the ends and beginnings "
                   "of adjacent instructions.  The default is to narrow each interval by " +
                   StringUtility::plural(settings.deExecuteZerosLeaveAtFront, "bytes") + " at the beginning and " +
                   StringUtility::plural(settings.deExecuteZerosLeaveAtBack, "bytes") + " at the end."));

    sg.insert(Switch("executable")
              .intrinsicValue(true, settings.memoryIsExecutable)
              .doc("This switch is for backward compatibility. It is equivalent to adding \"meta:0:perm+x\" to the end "
                   "of the command line.\n\n"
                   "Adds execute permission to the entire memory map, aside from regions excluded by @s{remove-zeros}. "
                   "The executable bit determines whether the partitioner is allowed to make instructions at some address, "
                   "so using this switch is an easy way to make the disassembler think that all of memory may contain "
                   "instructions.  The default is to not add executable permission to all of memory."));
    sg.insert(Switch("no-executable")
              .key("executable")
              .intrinsicValue(false, settings.memoryIsExecutable)
              .hidden(true));

    sg.insert(Switch("data")
              .argument("state", enumParser<MemoryDataAdjustment>(settings.memoryDataAdjustment)
                        ->with("constant", DATA_IS_CONSTANT)
                        ->with("initialized", DATA_IS_INITIALIZED)
                        ->with("no-change", DATA_NO_CHANGE))
              .doc("Globally adjusts the memory map to influence how the partitioner treats reads from concrete memory "
                   "addresses.  The values for @v{state} are one of these words:"

                   "@named{constant}{Causes write access to be removed from all memory segments and the partitioner treats "
                   "memory reads as returning a concrete value." +
                   std::string(DATA_IS_CONSTANT==settings.memoryDataAdjustment?" This is the default.":"") + "}"

                   "@named{initialized}{Causes the initialized bit to be added to all memory segments and the partitioner "
                   "treats reads from such addresses to return a concrete value, plus if the address is writable, "
                   "indeterminate values." +
                   std::string(DATA_IS_INITIALIZED==settings.memoryDataAdjustment?" This is the default.":"") + "}"

                   "@named{no-change}{Causes the engine to not change data access bits for memory." +
                   std::string(DATA_NO_CHANGE==settings.memoryDataAdjustment?" This is the default.":"") + "}"

                   "One of the things influenced by these access flags is indirect jumps, like x86 \"jmp [@v{addr}]\". If "
                   "@v{addr} is constant memory, then the \"jmp\" has a single constant successor; if @v{addr} is "
                   "non-constant but initialized, then the \"jmp\" will have a single constant successor and indeterminate "
                   "successors; otherwise it will have only indeterminate successors."));

    sg.insert(Switch("link-objects")
              .intrinsicValue(true, settings.linkObjectFiles)
              .doc("Object files (\".o\" files) typically don't contain information about how the object is mapped into "
                   "virtual memory, and thus machine instructions are not found. This switch causes the linker to be run "
                   "on all the object files mentioned on the command line and, if successful, the output file is "
                   "analyzed instead of the objects.  The linker command is specified with the @s{linker} switch. The "
                   "@s{no-link-objects} switch disables linking object files. The default is to " +
                   std::string(settings.linkObjectFiles ? "" : "not ") + "perform the link."));
    sg.insert(Switch("no-link-objects")
              .key("link-objects")
              .intrinsicValue(false, settings.linkObjectFiles)
              .hidden(true));

    sg.insert(Switch("link-archives")
              .intrinsicValue(true, settings.linkStaticArchives)
              .doc("Library archives (\".a\" files) contain object files that typically don't contain information about "
                   "how the object is mapped into virtual memory, and thus machine instructions are not found. This switch "
                   "causes the linker to be run on all the library archives mentioned on the command line and, if successful, "
                   "the output file is analyzed instead of the archives.  The linker command is specified with the @s{linker} "
                   "switch. The @s{no-link-archives} switch disables linking archives. The default is to " +
                   std::string(settings.linkStaticArchives ? "" : "not ") + "perform the link."));
    sg.insert(Switch("no-link-archives")
              .key("link-archives")
              .intrinsicValue(false, settings.linkStaticArchives)
              .hidden(true));

    sg.insert(Switch("linker")
              .argument("command", anyParser(settings.linker))
              .doc("Shell command that runs the linker if object files and/or library archives are being linked.  The command "
                   "should include two special variables: \"%o\" is replaced by the name of the output file, and \"%f\" is "
                   "replaced by a space-separated list of input files (object files and/or library archives) in the same order "
                   "that they were specified on the command-line.  Since the substituted files names are properly escaped using "
                   "Bourne shell syntax, the \"%o\" and \"%f\" should not appear in quotes. If the linker fails, the object "
                   "and archive files are processed without linking.  The default link command is \"" +
                   StringUtility::cEscape(settings.linker) + "\"."));

    sg.insert(Switch("env-erase-name")
              .argument("variable", anyParser(settings.envEraseNames))
              .whichValue(SAVE_ALL)
              .doc("Remove the specified variable from the environment when processing specimens with the \"run:\" schema. This "
                   "switch may appear multiple times to remove multiple variables. The default is to not erase any variables. "
                   "See also, @s{env-erase-pattern}."));

    sg.insert(Switch("env-erase-pattern")
              .argument("regular-expression", anyParser(settings.envErasePatterns))
              .whichValue(SAVE_ALL)
              .doc("Remove variables whose names match the specified regular expression when processing specimens with the "
                   "\"run:\" schema. You must specify \"^\" and/or \"$\" if you want the regular expression anchored to the "
                   "beginning and/or end of names. This switch may appear multiple times to supply multiple regular expressions. "
                   "See also, @s{env-erase-name}."));

    sg.insert(Switch("env-insert")
              .argument("name=value", anyParser(settings.envInsert))
              .whichValue(SAVE_ALL)
              .doc("Add the specified variable and value to the environment when processing specimens with the \"run:\" schema. "
                   "Insertions occur after all environment variable erasures. This switch may appear multiple times to specify "
                   "multiple environment variables."));

    return sg;
}

// Helper for the --isa switch
class IsaLister: public Sawyer::CommandLine::SwitchAction {
public:
    using Ptr = Sawyer::SharedPointer<IsaLister>;

    static Ptr instance() {
        return Ptr(new IsaLister);
    }

protected:
    void operator()(const Sawyer::CommandLine::ParserResult &cmdline) {
        if (cmdline.have("isa") && cmdline.parsed("isa", 0).string() == "list") {
            for (const std::string &name: Architecture::registeredNames())
                std::cout <<name <<"\n";
            exit(0);
        }
    }
};

// class method
Sawyer::CommandLine::SwitchGroup
EngineBinary::disassemblerSwitches(DisassemblerSettings &settings) {
    using namespace Sawyer::CommandLine;

    SwitchGroup sg("Binary disassembling switches");
    sg.name("binary-dis");
    sg.doc("These switches are used by Rose::BinaryAnalysis::Partitioner2::EngineBinary to control how the disassembler "
           "decodes instructions at given addresses. The disassembler's only purpose is to decode individual instructions at "
           "given addresses; it is not responsible for determining what addresses of the virtual address space are decoded. "
           "These switches have no effect if the input is a ROSE Binary Analysis (RBA) file, since the disassembler steps "
           "in such an input have already been completed.");

    Rose::CommandLine::insertBooleanSwitch(sg, "disassemble", settings.doDisassemble,
                                           "Perform the disassemble, partition, and post-analysis steps. Otherwise only parse "
                                           "the container (if any) and save the raw, un-disassembled bytes.");

    sg.insert(Switch("isa")
              .argument("architecture", anyParser(settings.isaName))
              .action(IsaLister::instance())
              .doc("Name of instruction set architecture.  If no name is specified then the architecture is obtained from "
                   "the binary container (ELF, PE). For a list of supported architectuers, say \"@s{isa}=list\"."));

    // Not called here because we don't want to do this until after the architecture shared libraries are loaded.
    // StringUtility::joinEnglish(Architecture::registeredNames()));

    return sg;
}

// class method
Sawyer::CommandLine::SwitchGroup
EngineBinary::partitionerSwitches(PartitionerSettings &settings) {
    using namespace Sawyer::CommandLine;
    SwitchGroup sg("Binary partitioning switches");
    sg.name("binary-part");
    sg.doc("These switches are used by Rose::BinaryAnalysis::Partitioner2::EngineBinary to control how virtual memory "
           "is organized into basic blocks and functions. While the disassembler knows how to decode a machine instruction to an "
           "internal representation, the partitioner knows where to decode. These switches have no effect if the input is a ROSE "
           "Binary Analysis (RBA) file, since the partitioner steps in such an input have already been completed.");

    sg.insert(Switch("function-at")
              .argument("addresses", listParser(nonNegativeIntegerParser(settings.functionStartingVas)))
              .whichValue(SAVE_ALL)
              .explosiveLists(true)
              .doc("List of addresses where recursive disassembly should start in addition to addresses discovered by "
                   "other methods. A function entry point will be insterted at each address listed by this switch. "
                   "This switch may appear multiple times, each of which may have multiple comma-separated addresses."));

    sg.insert(Switch("use-semantics")
              .intrinsicValue(true, settings.base.usingSemantics)
              .doc("The partitioner can either use quick and naive methods of determining instruction characteristics, or "
                   "it can use slower but more accurate methods, such as symbolic semantics.  This switch enables use of "
                   "the slower symbolic semantics, or the feature can be disabled with @s{no-use-semantics}. Furthermore, "
                   "instruction semantics will use an SMT solver if one is specified, which can make the analysis even "
                   "slower. The default is to " + std::string(settings.base.usingSemantics?"":"not ") +
                   "use semantics."));
    sg.insert(Switch("no-use-semantics")
              .key("use-semantics")
              .intrinsicValue(false, settings.base.usingSemantics)
              .hidden(true));

    sg.insert(Switch("ignore-unknown")
              .intrinsicValue(true, settings.base.ignoringUnknownInsns)
              .doc("If set, then any machine instructions that cannot be decoded due to ROSE having an incomplete disassembler "
                   "are treated as if they were no-ops for the purpose of extending a basic block. Although they will still show "
                   "up as \"unknown\" in the assembly listing, they will not cause a basic block to be terminated. If this "
                   "feature is disabled (@s{no-ignore-unknown}) then such instructions terminate a basic block. The default "
                   "is that unknown instructions " +
                   std::string(settings.base.ignoringUnknownInsns ? "are ignored." : "terminate basic blocks.")));
    sg.insert(Switch("no-ignore-unknown")
              .key("ignore-unknown")
              .intrinsicValue(false, settings.base.ignoringUnknownInsns)
              .hidden(true));

    sg.insert(Switch("semantic-memory")
              .argument("type", enumParser<SemanticMemoryParadigm>(settings.semanticMemoryParadigm)
                        ->with("list", LIST_BASED_MEMORY)
                        ->with("map", MAP_BASED_MEMORY))
              .doc("The partitioner can switch between storing semantic memory states in a list versus a map.  The @v{type} "
                   "should be one of these words:"

                   "@named{list}{List-based memory stores memory cells (essentially address+value pairs) in a reverse "
                   "chronological list and uses an SMT solver (when one is configured and enabled) to solve aliasing "
                   "equations.  The number of symbolic expression comparisons (either within ROSE or using an SMT solver) "
                   "is linear with the size of the memory cell list.}"

                   "@named{map}{Map-based memory stores memory cells in a container hashed by address expression. Aliasing "
                   "equations are not solved even when an SMT solver is available. One cell aliases another only if their "
                   "address expressions are identical. This approach is faster but less precise.}"

                   "The default is to use the " +
                   std::string(LIST_BASED_MEMORY == settings.semanticMemoryParadigm ? "list" : "map") +
                   "-based paradigm."));

    sg.insert(Switch("follow-ghost-edges")
              .intrinsicValue(true, settings.followingGhostEdges)
              .doc("A \"ghost edge\" is a control flow graph (CFG) edge that would be present if the CFG-building analysis "
                   "looked only at individual instructions, but would be absent when the analysis looks at coarser units "
                   "of code.  For instance, consider the following x86 assembly code:"

                   "@numbered{mov eax, 0}"              // 1
                   "@numbered{cmp eax, 0}"              // 2
                   "@numbered{jne 5}"                   // 3
                   "@numbered{nop}"                     // 4
                   "@numbered{hlt}"                     // 5

                   "If the analysis looks only at instruction 3, then it appears to have two CFG successors: instructions "
                   "4 and 5. But if the analysis looks at the first three instructions collectively it will ascertain that "
                   "instruction 3 has an opaque predicate, that the only valid CFG successor is instruction 4, and that the "
                   "edge from 3 to 5 is a \"ghost\". In fact, if there are no other incoming edges to these instructions, "
                   "then instructions 1 through 4 will form a basic block with the (unconditional) branch in the interior. "
                   "The ability to look at larger units of code than single instructions is enabled with the @s{use-semantics} "
                   "switch.\n\n"

                   "This @s{follow-ghost-edges} switch causes the ghost edges to be added back into the CFG as real edges, which "
                   "might force a basic block to end. For instance, in this example, turning on @s{follow-ghost-edges} will "
                   "force the first basic block to end with the \"jne\" instruction. The @s{no-follow-ghost-edges} switch turns "
                   "this feature off. By default, this feature is " +
                   std::string(settings.followingGhostEdges?"enabled":"disabled") + "."));
    sg.insert(Switch("no-follow-ghost-edges")
              .key("follow-ghost-edges")
              .intrinsicValue(false, settings.followingGhostEdges)
              .hidden(true));

    sg.insert(Switch("allow-discontiguous-blocks")
              .intrinsicValue(true, settings.discontiguousBlocks)
              .doc("This setting allows basic blocks to contain instructions that are discontiguous in memory as long as "
                   "the other requirements for a basic block are still met. Discontiguous blocks can be formed when a "
                   "compiler fails to optimize away an opaque predicate for a conditional branch, or when basic blocks "
                   "are scattered in memory by the introduction of unconditional jumps.  The @s{no-allow-discontiguous-blocks} "
                   "switch disables this feature and can slightly improve partitioner performance by avoiding cases where "
                   "an unconditional branch initially creates a larger basic block which is later discovered to be "
                   "multiple blocks.  The default is to " +
                   std::string(settings.discontiguousBlocks?"":"not ") +
                   "allow discontiguous basic blocks."));
    sg.insert(Switch("no-allow-discontiguous-blocks")
              .key("allow-discontiguous-blocks")
              .intrinsicValue(false, settings.discontiguousBlocks)
              .hidden(true));

    sg.insert(Switch("max-bblock-size")
              .argument("n", nonNegativeIntegerParser(settings.maxBasicBlockSize))
              .doc("Limit the size of basic blocks to @v{n} instructions. If a basic block would contain more than @v{n} "
                   "instructions then it is split into multiple basic blocks.  Limiting the block size is useful in order "
                   "to prevent long analysis times for intra-basic block semantics, but reduces the amount of information "
                   "available to some analyses. If @v{n} is zero then no limit is enforced.  The default is " +
                   StringUtility::numberToString(settings.maxBasicBlockSize) + "."));

    sg.insert(Switch("ip-rewrite")
              .argument("old", nonNegativeIntegerParser(settings.ipRewrites))
              .argument("new", nonNegativeIntegerParser(settings.ipRewrites))
              .whichValue(SAVE_ALL)
              .doc("Rewrite the global control flow graph so that any edges that would have gone to address @v{old} instead "
                   "go to @v{new}. This switch may appear multiple times."));

    sg.insert(Switch("find-function-padding")
              .intrinsicValue(true, settings.findingFunctionPadding)
              .doc("Cause each built-in and user-defined function padding analysis to run. The purpose of these "
                   "analyzers is to look for padding such as zero bytes and certain instructions like no-ops that occur "
                   "prior to the lowest address of a function and attach them to the function as static data.  The "
                   "@s{no-find-function-padding} switch turns this off.  The default is to " +
                   std::string(settings.findingFunctionPadding?"":"not ") + "search for padding."));
    sg.insert(Switch("no-find-function-padding")
              .key("find-function-padding")
              .intrinsicValue(false, settings.findingFunctionPadding)
              .hidden(true));

    sg.insert(Switch("find-dead-code")
              .intrinsicValue(true, settings.findingDeadCode)
              .doc("If ghost edges are being discovered (see @s{follow-ghost-edges} for the definition of \"ghost "
                   "edge\") and are not being inserted into the global control flow graph (controlled by "
                   "@s{follow-ghost-edges}) then the target address of the ghost edge might not be used as a code "
                   "address during the instruction discovery phase. This switch, @s{find-dead-code}, will cause the "
                   "target addresses of ghost edges to be used to discover more instructions even though the ghost "
                   "edges don't appear in the control flow graph. The @s{no-find-dead-code} switch turns this off. "
                   "The default is that this feature is " +
                   std::string(settings.findingDeadCode?"enabled":"disabled") + "."));
    sg.insert(Switch("no-find-dead-code")
              .key("find-dead-code")
              .intrinsicValue(false, settings.findingDeadCode)
              .hidden(true));

    sg.insert(Switch("find-thunks")
              .intrinsicValue(true, settings.findingThunks)
              .doc("Search for common thunk patterns in areas of executable memory that have not been previously "
                   "discovered to contain other functions.  When this switch is enabled, the function-searching callbacks "
                   "include patterns to match thunks.  This switch does not cause the thunk's instructions to be "
                   "detached as a separate function from the thunk's target function; that's handled by the "
                   "@s{split-thunks} switch.  The @s{no-find-thunks} switch turns thunk searching off. The default "
                   "is to " + std::string(settings.findingThunks ? "" : "not ") + "search for thunks."));
    sg.insert(Switch("no-find-thunks")
              .key("find-thunks")
              .intrinsicValue(false, settings.findingThunks)
              .hidden(true));

    sg.insert(Switch("split-thunks")
              .intrinsicValue(true, settings.splittingThunks)
              .doc("Look for common thunk patterns at the start of existing functions and split off those thunk "
                   "instructions to their own separate function.  The @s{no-split-thunks} switch turns this feature "
                   "off.  The default is to " + std::string(settings.splittingThunks?"":"not ") + "split thunks into their own "
                   "functions."));
    sg.insert(Switch("no-split-thunks")
              .key("split-thunks")
              .intrinsicValue(false, settings.splittingThunks)
              .hidden(true));

    sg.insert(Switch("pe-scrambler")
              .argument("dispatcher_address", nonNegativeIntegerParser(settings.peScramblerDispatcherVa))
              .doc("Simulate the action of the PEScrambler dispatch function in order to rewrite CFG edges.  Any edges "
                   "that go into the specified @v{dispatcher_address} are immediately rewritten so they appear to go "
                   "instead to the function contained in the dispatcher table which normally immediately follows the "
                   "dispatcher function.  The dispatcher function is quite easy to find in a call graph because nearly "
                   "everything calls it -- it will likely have far and away more callers than anything else.  Setting the "
                   "address to zero disables this module (which is the default)."));

    sg.insert(Switch("intra-function-code")
              .argument("npasses", nonNegativeIntegerParser(settings.findingIntraFunctionCode), "10")
              .doc("Near the end of processing, a pass is made over the entire address space to find executable memory "
                   "that doesn't yet belong to any known function but is surrounded by a single function. A basic block "
                   "is created for each such region after which a recursive basic block discover phase ensues in order "
                   "find additional blocks reachable by the control flow. This process is repeated up to @v{npasses} "
                   "times, or until no new addresses are found. For backward compatibility, this switch also acts as "
                   "a boolean: @s{intra-function-code} and @s{no-intra-function-code} are equivalent to setting the "
                   "number of passes to ten and zero, respectively. The default is " +
                   StringUtility::plural(settings.findingIntraFunctionCode, "passes") + "."));
    sg.insert(Switch("no-intra-function-code")
              .key("intra-function-code")
              .intrinsicValue((size_t)0, settings.findingIntraFunctionCode)
              .hidden(true));

    sg.insert(Switch("intra-function-data")
              .intrinsicValue(true, settings.findingIntraFunctionData)
              .doc("Near the end of processing, if there are regions of unused memory that are immediately preceded and "
                   "followed by the same function then add that region of memory to that function as a static data block. "
                   "The @s{no-intra-function-data} switch turns this feature off.  The default is to " +
                   std::string(settings.findingIntraFunctionData?"":"not ") + "perform this analysis."));
    sg.insert(Switch("no-intra-function-data")
              .key("intra-function-data")
              .intrinsicValue(false, settings.findingIntraFunctionData)
              .hidden(true));

    sg.insert(Switch("inter-function-calls")
              .intrinsicValue(true, settings.findingInterFunctionCalls)
              .doc("Near the end of processing look for function calls that occur in the executable regions between "
                   "existing functions, and turn the call targets into functions. The analysis uses instruction semantics "
                   "to find the call sites rather than looking for architecture-specific call instructions, and attempts "
                   "to prune away things that are not legitimate calls. The @s{no-inter-function-calls} switch turns this "
                   "feature off. The default is to " + std::string(settings.findingInterFunctionCalls?"":"not ") +
                   "perform this analysis."));
    sg.insert(Switch("no-inter-function-calls")
              .key("inter-function-calls")
              .intrinsicValue(false, settings.findingInterFunctionCalls)
              .hidden(true));

    sg.insert(Switch("called-functions")
              .intrinsicValue(true, settings.findingFunctionCallFunctions)
              .doc("Look for function call instructions or sequences of instructions with similar behavior and assume "
                   "that the target address is the entry point for a function under most circumstances. The "
                   "@s{no-called-functions} switch turns this feature off, which can be useful when analyzing virtual "
                   "memory where targets of call-like sequences have not been initialized (such as in object files). "
                   "The default is to " + std::string(settings.findingFunctionCallFunctions?"":"not ") + "perform this "
                   "analysis."));
    sg.insert(Switch("no-called-functions")
              .key("called-functions")
              .intrinsicValue(false, settings.findingFunctionCallFunctions)
              .hidden(true));

    sg.insert(Switch("entry-functions")
              .intrinsicValue(true, settings.findingEntryFunctions)
              .doc("Create functions at the program entry point(s). The @s{no-entry-functions} switch turns this feature "
                   "off. The default is to " + std::string(settings.findingEntryFunctions?"":"not ") + "perform this analysis."));
    sg.insert(Switch("no-entry-functions")
              .key("entry-functions")
              .intrinsicValue(false, settings.findingEntryFunctions)
              .hidden(true));

    sg.insert(Switch("error-functions")
              .intrinsicValue(true, settings.findingErrorFunctions)
              .doc("Create functions based on error handling information that might be present in the container's tables. "
                   "The @s{no-error-functions} switch turns this feature off. The default is to " +
                   std::string(settings.findingErrorFunctions?"":"not ") + "perform this analysis."));
    sg.insert(Switch("no-error-functions")
              .key("error-functions")
              .intrinsicValue(false, settings.findingErrorFunctions)
              .hidden(true));

    sg.insert(Switch("import-functions")
              .intrinsicValue(true, settings.findingImportFunctions)
              .doc("Create functions based on import information that might be present in the container's tables. The "
                   "@s{no-import-functions} switch turns this feature off. The default is to " +
                   std::string(settings.findingImportFunctions?"":"not ") + "perform this analysis."));
    sg.insert(Switch("no-import-functions")
              .key("import-functions")
              .intrinsicValue(false, settings.findingImportFunctions)
              .hidden(true));

    sg.insert(Switch("export-functions")
              .intrinsicValue(true, settings.findingExportFunctions)
              .doc("Create functions based on export information that might be present in the container's tables. The "
                   "@s{no-export-functions} switch turns this feature off. The default is to " +
                   std::string(settings.findingExportFunctions?"":"not ") + "perform this analysis."));
    sg.insert(Switch("no-export-functions")
              .key("export-functions")
              .intrinsicValue(false, settings.findingExportFunctions)
              .hidden(true));

    sg.insert(Switch("symbol-functions")
              .intrinsicValue(true, settings.findingSymbolFunctions)
              .doc("Create functions based on symbol tables that might be present in the container. The @s{no-symbol-functions} "
                   "switch turns this feature off. The default is to " + std::string(settings.findingSymbolFunctions?"":"not ") +
                   "perform this analysis."));
    sg.insert(Switch("no-symbol-functions")
              .key("symbol-functions")
              .intrinsicValue(false, settings.findingSymbolFunctions)
              .hidden(true));

    sg.insert(Switch("data-functions")
              .intrinsicValue(true, settings.findingDataFunctionPointers)
              .doc("Scan non-executable areas of memory to find pointers to functions.  This analysis can be disabled "
                   "with @s{no-data-functions}. The default is to " + std::string(settings.findingDataFunctionPointers?"":"not ") +
                   "perform this analysis."));
    sg.insert(Switch("no-data-functions")
              .key("data-functions")
              .intrinsicValue(false, settings.findingDataFunctionPointers)
              .hidden(true));

    sg.insert(Switch("code-functions")
              .intrinsicValue(true, settings.findingCodeFunctionPointers)
              .doc("Scan instructions to find pointers to functions. This analysis can be disabled with "
                   "@s{no-code-functions}. The default is to " + std::string(settings.findingCodeFunctionPointers?"":"not ") +
                   "perform this analysis."));
    sg.insert(Switch("no-code-functions")
              .key("code-functions")
              .intrinsicValue(false, settings.findingCodeFunctionPointers)
              .hidden(true));

    sg.insert(Switch("interrupt-vector")
              .argument("addresses", addressIntervalParser(settings.interruptVector))
              .doc("A table containing addresses of functions invoked for various kinds of interrupts. " +
                   AddressIntervalParser::docString() + " The length and contents of the table is architecture "
                   "specific, and the disassembler will use available information about the architecture to decode the "
                   "table.  If a single address is specified, then the length of the table is architecture dependent, "
                   "otherwise the entire table is read."));

    sg.insert(Switch("name-constants")
              .argument("addresses", addressIntervalParser(settings.namingConstants), "all")
              .doc("Scans the instructions and gives labels to constants that refer to entities that have that address "
                   "and also have a name.  For instance, if a constant refers to the beginning of a file section then "
                   "the constant will be labeled so it has the same name as the section. The argument for this switch is the "
                   "range of integer values that can be labeled, defaulting to all addresses. The @s{no-name-constants} switch "
                   "turns this feature off. The default is to " +
                   (settings.namingConstants ?
                    "try to label constants within " + StringUtility::addrToString(settings.namingConstants) + "." :
                    "not perform this labeling.")));
    sg.insert(Switch("no-name-constants")
              .key("name-constants")
              .intrinsicValue(AddressInterval(), settings.namingConstants)
              .hidden(true));

    sg.insert(Switch("name-strings")
              .argument("addresses", addressIntervalParser(settings.namingStrings), "all")
              .doc("Scans the instructions and gives labels to constants that refer to the beginning of string literals. "
                   "The label is usually the first few characters of the string. The argument for this switch is the "
                   "range of integer values that should be tested as string pointers, defaulting to all addresses. The "
                   "@s{no-name-strings} switch turns this feature off. The default is to " +
                   (settings.namingStrings.isEmpty() ? "not perform this labeling." :
                    ("try to label constants within " + StringUtility::addrToString(settings.namingStrings) + "."))));
    sg.insert(Switch("no-name-strings")
              .key("name-strings")
              .intrinsicValue(AddressInterval(), settings.namingStrings)
              .hidden(true));

    sg.insert(Switch("name-syscalls")
              .intrinsicValue(true, settings.namingSyscalls)
              .doc("Scans all instructions and tries to give names to system calls.  The names are assigned as comments "
                   "to the instruction that performs the system call. The system call names are parsed from the Linux header "
                   "files on the system running the analysis (not necessarily where ROSE was compiled); this can be adjusted "
                   "with the @s{syscall-header} switch.  The @s{no-name-syscalls} turns this feature off. The default is to " +
                   std::string(settings.namingSyscalls?"":"not ") + "do this step."));
    sg.insert(Switch("no-name-syscalls")
              .key("name-syscalls")
              .intrinsicValue(false, settings.namingSyscalls)
              .hidden(true));

    sg.insert(Switch("syscall-header")
              .argument("filename", anyParser(settings.syscallHeader))
              .doc("Name of the header file from which to obtain the system call ID-name mapping. The default is to look "
                   "in standard places such as /usr/include/asm/unistd_32.h."));

    sg.insert(Switch("post-analysis")
              .intrinsicValue(true, settings.doingPostAnalysis)
              .doc("Run all enabled post-partitioning analysis functions.  For instance, calculate stack deltas for each "
                   "instruction, and may-return analysis for each function.  The individual analyses are enabled and "
                   "disabled separately with other @s{post-*}{noerror} switches. Some of these analyses will only work if "
                   "instruction semantics are enabled (see @s{use-semantics}).  The @s{no-post-analysis} switch turns "
                   "this off, although analysis will still be performed where it is needed for partitioning.  The "
                   "default is to " + std::string(settings.doingPostAnalysis?"":"not ") + "perform the post analysis phase."));
    sg.insert(Switch("no-post-analysis")
              .key("post-analysis")
              .intrinsicValue(false, settings.doingPostAnalysis)
              .hidden(true));

    sg.insert(Switch("post-function-noop")
              .intrinsicValue(true, settings.doingPostFunctionNoop)
              .doc("Run a function no-op analysis for each function if post-partitioning analysis is enabled with the "
                   "@s{post-analysis} switch. This analysis tries to determine whether each function is effectively a no-op. "
                   "Functions that are no-ops are given names (if they don't already have one) that's indicative of "
                   "being a no-op. The @s{no-post-function-noop} switch disables this analysis. The default is that "
                   "this analysis is " + std::string(settings.doingPostFunctionNoop?"enabled":"disabled") + "."));
    sg.insert(Switch("no-post-function-noop")
              .key("post-function-noop")
              .intrinsicValue(false, settings.doingPostFunctionNoop)
              .hidden(true));

    sg.insert(Switch("post-may-return")
              .intrinsicValue(true, settings.doingPostFunctionMayReturn)
              .doc("Run the may-return analysis for each function if post-partitioning analysis is enabled with the "
                   "@s{post-analysis} switch. This analysis tries to quickly determine if a function might return a "
                   "value to the caller.  The @s{no-post-may-return} switch disables this analysis. The default is that "
                   "this analysis is " + std::string(settings.doingPostFunctionMayReturn?"enabled":"disabled") + "."));
    sg.insert(Switch("no-post-may-return")
              .key("post-may-return")
              .intrinsicValue(false, settings.doingPostFunctionMayReturn)
              .hidden(true));

    sg.insert(Switch("post-stack-delta")
              .intrinsicValue(true, settings.doingPostFunctionStackDelta)
              .doc("Run the stack-delta analysis for each function if post-partitioning analysis is enabled with the "
                   "@s{post-analysis} switch.  This is a data-flow analysis that tries to determine whether the function "
                   "has a constant net effect on the stack pointer and what that effect is.  For instance, when a caller "
                   "is reponsible for cleaning up function call arguments on a 32-bit architecture with a downward-growing "
                   "stack then the stack delta is usually +4, representing the fact that the called function popped the "
                   "return address from the stack.  The @s{no-post-stack-delta} switch disables this analysis. The default "
                   "is that this analysis is " + std::string(settings.doingPostFunctionStackDelta?"enabled":"disabled") + "."));
    sg.insert(Switch("no-post-stack-delta")
              .key("post-stack-delta")
              .intrinsicValue(false, settings.doingPostFunctionStackDelta)
              .hidden(true));

    sg.insert(Switch("post-calling-convention")
              .intrinsicValue(true, settings.doingPostCallingConvention)
              .doc("Run the calling-convention analysis for each function if post-partitioning analysis is enabled with the "
                   "@s{post-analysis} switch. This relatively expensive analysis uses use-def, stack-delta, memory variable "
                   "discovery, and data-flow to determine characteristics of the function and then matches it against a "
                   "dictionary of calling conventions appropriate for the architecture.  The @s{no-post-calling-convention} "
                   "disables this analysis. The default is that this analysis is " +
                   std::string(settings.doingPostCallingConvention?"enabled":"disabled") + "."));
    sg.insert(Switch("no-post-calling-convention")
              .key("post-calling-convention")
              .intrinsicValue(false, settings.doingPostCallingConvention)
              .hidden(true));

    sg.insert(Switch("functions-return")
              .argument("how", enumParser<FunctionReturnAnalysis>(settings.functionReturnAnalysis)
                        ->with("always", MAYRETURN_ALWAYS_YES)
                        ->with("never", MAYRETURN_ALWAYS_NO)
                        ->with("yes", MAYRETURN_DEFAULT_YES)
                        ->with("no", MAYRETURN_DEFAULT_NO))
              .doc("Determines how function may-return analysis is performed. This analysis returns true if a call to the "
                   "function has a possibility of returning, false if the call has no possibility of returning, or "
                   "indeterminate if the analysis cannot decide.  The partitioner will attempt to disassemble instructions "
                   "at the fall-through address of a call if the call has a possibility of returning, otherwise that address "
                   "will be disassembled only if it can be reached by some other mechanism.\n\n"

                   "This switch accepts one of these four words:"
                   "@named{always}{Assume that all function calls may return without ever running the may-return analysis.}"
                   "@named{never}{Assume that all functions cannot return to the caller and never run the may-return analysis.}"
                   "@named{yes}{Assume a function returns if the may-return analysis cannot decide. This is the default.}"
                   "@named{no}{Assume a function does not return if the may-return analysis cannot decide.}"));

    sg.insert(Switch("functions-return-sort")
              .argument("n", nonNegativeIntegerParser(settings.functionReturnAnalysisMaxSorts))
              .doc("If function return analysis is occurring (@s{functions-return}) then functions are sorted according to "
                   "their depth in the global control flow graph, arbitrarily removing cycles. The functions are analyzed "
                   "starting at the leaves in order to minimize forward dependencies. For large specimens, this sorting "
                   "might occur often and is expensive. Therefore, the sorting is limited to the specified number of "
                   "occurrences, after which unsorted lists are used. The default is " +
                   StringUtility::plural(settings.functionReturnAnalysisMaxSorts, "sorting operations") + "."));

    sg.insert(Switch("call-branch")
              .intrinsicValue(true, settings.base.checkingCallBranch)
              .doc("When determining whether a basic block is a function call, also check whether the callee discards "
                   "the return address. If so, then the apparent call is perhaps not a true function call.  The "
                   "@s{no-call-branch} switch disables this analysis. The default is that this analysis is " +
                   std::string(settings.base.checkingCallBranch ? "enabled" : "disabled") + "."));
    sg.insert(Switch("no-call-branch")
              .key("call-branch")
              .intrinsicValue(false, settings.base.checkingCallBranch)
              .hidden(true));

    sg.insert(Switch("demangle-names")
              .intrinsicValue(true, settings.demangleNames)
              .doc("Causes all function names to be run through a demangler process to undo the name mangling that "
                   "normally happens with some compilers.  The @s{no-demangle-names} switch disables this step. The "
                   "default is to " + std::string(settings.demangleNames ? "" : "not ") + "do this step."));
    sg.insert(Switch("no-demangle-names")
              .key("demangle-names")
              .intrinsicValue(false, settings.demangleNames)
              .hidden(true));

    Rose::CommandLine::insertBooleanSwitch(sg, "naive-jump-tables", settings.naiveJumpTables,
                                           "Perform a pattern-based anaylsis in an attempt to discover indirect control flow "
                                           "edges emanating from a basic block. This analysis is simple and fast, and if it "
                                           "detects a jump table then its results are used in preference to more expensive "
                                           "analysis. This switch should be turned off in order to use the more expensive "
                                           "dataflow-based jump table analysis.");

    return sg;
}

// class method
Sawyer::CommandLine::SwitchGroup
EngineBinary::astConstructionSwitches(AstConstructionSettings &settings) {
    using namespace Sawyer::CommandLine;
    SwitchGroup sg("Binary AST construction switches");
    sg.name("binary-ast");
    sg.doc("These switches are used by Rose::BinaryAnalysis::Partitioner2::EngineBinary to control how an abstract syntax "
           "tree (AST) is constructed from the partitioned instructions.");

    sg.insert(Switch("allow-empty-global-block")
              .intrinsicValue(true, settings.allowEmptyGlobalBlock)
              .doc("Allows creation of an empty AST if the partitioner does not find any functions. The "
                   "@s{no-allow-empty-global-block} switch causes a null AST to be returned instead. The default is to " +
                   std::string(settings.allowEmptyGlobalBlock ? "create an empty " : "not create an ") + "AST."));
    sg.insert(Switch("no-allow-empty-global-block")
              .key("allow-empty-global-block")
              .intrinsicValue(false, settings.allowEmptyGlobalBlock)
              .hidden(true));

    sg.insert(Switch("allow-empty-functions")
              .intrinsicValue(true, settings.allowFunctionWithNoBasicBlocks)
              .doc("Allows creation of an AST that has functions with no instructions. This can happen, for instance, when "
                   "an analysis indicated that a particular virtual address is the start of a function but no memory is "
                   "mapped at that address. This is common for things like functions from shared libraries that have not "
                   "been linked in before the analysis starts.  The @s{no-allow-empty-functions} will instead elide all "
                   "empty functions from the AST. The default is to " +
                   std::string(settings.allowFunctionWithNoBasicBlocks ? "allow " : "elide ") + "empty functions."));
    sg.insert(Switch("no-allow-empty-functions")
              .key("allow-empty-functions")
              .intrinsicValue(false, settings.allowFunctionWithNoBasicBlocks)
              .hidden(true));

    sg.insert(Switch("allow-empty-basic-blocks")
              .intrinsicValue(true, settings.allowEmptyBasicBlocks)
              .doc("Allows creation of an AST that has basic blocks with no instructions. This can happen when an analysis "
                   "indicates that a basic block exists at a particular virtual address but no memory is mapped at that "
                   "address. The @s{no-allow-empty-basic-blocks} will instead elide all empty blocks from the AST. The "
                   "default is to " + std::string(settings.allowEmptyBasicBlocks ? "allow " : "elide ") + "empty blocks."));
    sg.insert(Switch("no-allow-empty-basic-blocks")
              .key("allow-empty-basic-blocks")
              .intrinsicValue(false, settings.allowEmptyBasicBlocks)
              .hidden(true));

    sg.insert(Switch("copy-instructions")
              .intrinsicValue(true, settings.copyAllInstructions)
              .doc("Causes all instructions to be deep-copied from the partitioner's instruction provider into the AST. "
                   "Although this slows down AST construction and increases memory since SageIII nodes are not garbage "
                   "collected, copying instructions ensures that the AST is a tree. Turning off the copying with the "
                   "@s{no-copy-instructions} switch will result in the AST being a lattice if the partitioner has "
                   "determined that two or more functions contain the same basic block, and therefore the same instructions. "
                   "The default is to " + std::string(settings.copyAllInstructions ? "" : "not ") + "copy instructions.\n\n"

                   "Note that within the partitioner data structures it's the basic blocks that are shared when two or "
                   "more functions point to the same block, but within the AST, sharing is at the instruction "
                   "(SgAsmInstruction) level. It was designed this way because it's common to store function-specific "
                   "analysis results as attributes attached to basic blocks (SgAsmBlock nodes) in the AST and sharing at "
                   "the block level would break those programs. Users that store analysis results by attaching them to "
                   "partitioner basic blocks (Partitioner2::BasicBlock) should be aware that those blocks can be shared "
                   "among functions."));
    sg.insert(Switch("no-copy-instructions")
              .key("copy-instructions")
              .intrinsicValue(false, settings.copyAllInstructions)
              .hidden(true));

    return sg;
}

std::pair<std::string, std::string>
EngineBinary::specimenNameDocumentation() {
    return {
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        "Binary specimens",
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        "The Rose::BinaryAnalysis::Partitioner2::EngineBinary class recognizes the following command-line positional arguments "
        "for specifying binary specimens:"

        "@bullet{If the name does not match any of the following patterns then it is assumed to be the name of a "
        "file containing a specimen that is a binary container format such as ELF or PE.}"

        "@bullet{If the name begins with the string \"map:\" then it is treated as a memory map resource string that "
        "adjusts a memory map by inserting part of a file. " + MemoryMap::insertFileDocumentation() + "}"

        "@bullet{If the name begins with the string \"data:\" then its data portion is parsed as a byte sequence "
        "which is then inserted into the memory map. " + MemoryMap::insertDataDocumentation() + "}"

        "@bullet{If the name begins with the string \"proc:\" then it is treated as a process resource string that "
        "adjusts a memory map by reading the process' memory. " + MemoryMap::insertProcessDocumentation() + "}"

        "@bullet{If the name begins with the string \"run:\" then it is first treated like a normal file by ROSE's "
        "\"frontend\" function, and then during a second pass it will be loaded natively under a debugger, run until "
        "a mapped executable address is reached, and then its memory is copied into ROSE's memory map possibly "
        "overwriting existing parts of the map.  This can be useful when the user wants accurate information about "
        "how that native loader links in shared objects since ROSE's linker doesn't always have identical behavior. "
        "The syntax of this form is \"run:@v{options}:@v{filename}\" where @v{options} is a comma-separated list of "
        "options that control the finer details. The following options are recognized:"

        "@named{replace}{This option causes the memory map to be entirely replaced with the process map rather than "
        "the default behavior of the process map augmenting the map created by the ROSE loader.  This can be useful "
        "if ROSE's internal loader resulted in wrong addresses, although symbols will then probably also be pointing to "
        "those wrong addresses and will be dangling when those addresses are removed from the map.}"

        "@named{aslr}{Turns on address space layout randomization. ASLR is an exploitation counter measure on Linux "
        "systems, but ROSE tools normally disable it in order to have reproducible results during analysis. The \"aslr\" "
        "option causes ROSE to re-enable address space randomization. Although \"noaslr\" is the default, it can also be "
        "explicitly specified.}"

        "}"                                         // end of @bullet

        "@bullet{If the file name begins with the string \"srec:\" then it is treated as Motorola S-Record format. "
        "Mapping attributes are stored after the first column and before the second; the file name appears after the "
        "second colon.  The only mapping attributes supported at this time are permissions, specified as an equal "
        "sign ('=') followed by zero or more of the letters \"r\", \"w\", and \"x\" to signify read, write, and "
        "execute permissions. If no letters are present after the equal sign, then the memory has no permissions; "
        "if the equal sign itself is also missing then the segments are given read, write, and execute permission.}"

        "@bullet{If the name begins with the string \"vxcore:\" then it is treated as a special VxWorks core dump "
        "in a format defined by ROSE. The complete specification has the syntax \"vxcore:[@v{memory_attributes}]"
        ":[@v{file_attributes}]:@v{file_name}\". The parts in square brackets are optional. The only file attribute "
        "recognized at this time is \"version=@v{v}\" where @v{v} is a version number which must be 1 or 2, defaulting "
        "to 1. For version 1, the only memory attribute is an equal sign (\"=\") followed by zero of more of the letters "
        "\"r\" (read), \"w\" (write), and \"x\" (execute) to specify the mapping permissions, defaulting to read, write, and "
        "execute. Version 2 has no memory attributes.}"

        "@bullet{If the name begins with the string \"vxworks:\" then it is treated as a resource that adjusts a memory map "
        "by downloading memory contents over TCP/IP from a VxWorks terminal over a telnet connection. " +
        VxworksTerminal::locatorStringDocumentation(VxworksTerminal::Settings()) + "}"

        "@bullet{If the name begins with the string \"meta:\" then it adjusts meta information about the memory "
        "map, such as permissions. " + MemoryMap::adjustMapDocumentation() + "}"

        "@bullet{If the name ends with \".srec\" and doesn't match the previous list of prefixes then it is assumed "
        "to be a text file containing Motorola S-Records and will be parsed as such and loaded into the memory map "
        "with read, write, and execute permissions.}"

        "@bullet{If the name ends with \".rba\" and doesn't match the previous list of prefixes then it is assumed "
        "to be a ROSE Binary Analysis file that contains a serialized partitioner and an optional AST. Use of an "
        "RBA file to describe a specimen precludes the use of any other inputs for that specimen. Furthermore, since "
        "an RBA file represents a fully parsed and disassembled specimen, the command-line switches that control "
        "the parsing and disassembly are ignored.}"

        "When more than one mechanism is used to load a single coherent specimen, the normal names are processed first "
        "by passing them all to ROSE's \"frontend\" function, which results in an initial memory map.  The other names "
        "are then processed in the order they appear, possibly overwriting parts of the map. RBA files have special "
        "handling described above."
    };
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Binary container parsing
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool
EngineBinary::isNonContainer(const std::string &name) {
    return (boost::starts_with(name, "map:")  ||        // map file directly into MemoryMap
            boost::starts_with(name, "data:") ||        // map data directly into MemoryMap
            boost::starts_with(name, "proc:") ||        // map process memory into MemoryMap
            boost::starts_with(name, "run:")  ||        // run a process in a debugger, then map into MemoryMap
            boost::starts_with(name, "srec:") ||        // Motorola S-Record format
            boost::ends_with(name, ".srec")   ||        // Motorola S-Record format
            boost::starts_with(name, "vxcore:") ||      // Jim Leek's format of a VxWorks core dump
            boost::starts_with(name, "vxworks:") ||     // use telnet to connect to a VxWorks terminal
            boost::starts_with(name, "meta:") ||        // Adjust meta information about the map
            isRbaFile(name));                           // ROSE Binary Analysis file
}

bool
EngineBinary::areContainersParsed() const {
    return interpretation() != nullptr;
}

SgAsmInterpretation*
EngineBinary::parseContainers(const std::vector<std::string> &fileNames) {
    try {
        interpretation(nullptr);

        // Don't clear the memory map because the user might have configured it already.
        //memoryMap(MemoryMap::Ptr());

        checkSettings();

        // Prune away things we recognize as not being binary containers.
        std::vector<boost::filesystem::path> containerFiles;
        for (const std::string &fileName: fileNames) {
            if (boost::starts_with(fileName, "run:") && fileName.size()>4) {
                static size_t colon1 = 3;
                size_t colon2 = fileName.find(':', colon1+1);
                if (colon2 == std::string::npos) {
                    // [Robb Matzke 2017-07-24]: deprecated: use two colons for consistency with other schemas
                    containerFiles.push_back(fileName.substr(colon1+1));
                } else {
                    containerFiles.push_back(fileName.substr(colon2+1));
                }
            } else if (!isNonContainer(fileName)) {
                containerFiles.push_back(fileName);
            }
        }

        // Try to link all the .o and .a files
        bool linkerFailed = false;
        Sawyer::FileSystem::TemporaryFile linkerOutput;
        if (!settings().loader.linker.empty()) {
            std::vector<boost::filesystem::path> filesToLink, nonLinkedFiles;
            for (const boost::filesystem::path &file: containerFiles) {
                if (settings().loader.linkObjectFiles && ModulesElf::isObjectFile(file)) {
                    filesToLink.push_back(file);
                } else if (settings().loader.linkStaticArchives && ModulesElf::isStaticArchive(file)) {
                    filesToLink.push_back(file);
                } else {
                    nonLinkedFiles.push_back(file);
                }
            }
            if (!filesToLink.empty()) {
                linkerOutput.stream().close();          // will be written by linker command
                if (ModulesElf::tryLink(settings().loader.linker, linkerOutput.name().string(), filesToLink, mlog[WARN])) {
                    containerFiles = nonLinkedFiles;
                    containerFiles.push_back(linkerOutput.name().string());
                } else {
                    mlog[ERROR] <<"linking objects and/or archives failed; falling back to internal (incomplete) linker\n";
                    filesToLink.clear();
                    linkerFailed = true;
                }
            }
        }

        // If we have *.a files that couldn't be linked, extract all their members and replace the archive file with the list
        // of object files.
        Sawyer::FileSystem::TemporaryDirectory tempDir;
        if (linkerFailed) {
            std::vector<boost::filesystem::path> expandedList;
            for (const boost::filesystem::path &file: containerFiles) {
                if (ModulesElf::isStaticArchive(file)) {
                    std::vector<boost::filesystem::path> objects = ModulesElf::extractStaticArchive(tempDir.name(), file);
                    if (objects.empty())
                        mlog[WARN] <<"empty static archive \"" <<StringUtility::cEscape(file.string()) <<"\"\n";
                    for (const boost::filesystem::path &objectFile: objects)
                        expandedList.push_back(objectFile.string());
                } else {
                    expandedList.push_back(file);
                }
            }
            containerFiles = expandedList;
        }

        // Process through ROSE's frontend()
        if (!containerFiles.empty()) {
            SgProject *project = roseFrontendReplacement(containerFiles);
            ASSERT_not_null(project);                       // an exception should have been thrown

            std::vector<SgAsmInterpretation*> interps = AST::Traversal::findDescendantsTyped<SgAsmInterpretation>(project);
            if (interps.empty())
                throw std::runtime_error("a binary specimen container must have at least one SgAsmInterpretation");
            interpretation(interps.back());    // windows PE is always after DOS
            ASSERT_require(areContainersParsed());
        }

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

// Replacement for ::frontend, which is a complete mess, in order create a project containing multiple files. Nothing
// special happens to any of the input file names--that should have already been done by this point. All the fileNames
// are expected to be names of existing container (ELF, PE, etc) files that will result in an AST (non-container files
// typically are loaded into virtual memory and have no AST since they have very little structure).
SgProject*
EngineBinary::roseFrontendReplacement(const std::vector<boost::filesystem::path> &fileNames) {
    ASSERT_forbid(fileNames.empty());

    // We want to move all binary file processing to the Rose:: namespace (do we?) [Rasmussen 2024.05.17].
    // Called by testPartitioner2_x86-elf-exe

    SgBinaryComposite* binaryComposite = new SgBinaryComposite;
    SgAsmGenericFileList* fileList = binaryComposite->get_genericFileList();
    SgAsmInterpretationList* interpList = binaryComposite->get_interpretations();

    // Create the SgAsmGenericFiles (not a type of SgFile), one per fileName, and add them to a SgAsmGenericFileList node. Each
    // SgAsmGenericFile has one or more file headers (e.g., ELF files have one, PE files have two).
    for (const boost::filesystem::path &fileName: fileNames) {
        SAWYER_MESG(mlog[TRACE]) <<"parsing " <<fileName <<"\n";
        SgAsmGenericFile* file = SgAsmExecutableFileFormat::parseBinaryFormat(fileName.string().c_str());
        ASSERT_not_null(file);
#ifdef ROSE_HAVE_LIBDWARF
        try {
            Dwarf::parse(file);
        } catch (const Dwarf::Exception &e) {
            mlog[ERROR] <<"DWARF parsing failed: " <<e.what() <<"\n";
        }
#endif
        fileList->get_files().push_back(file);
        file->set_parent(fileList);
    }
    SAWYER_MESG(mlog[DEBUG]) <<"parsed " <<StringUtility::plural(fileList->get_files().size(), "container files") <<"\n";

    // FIXME[Robb Matzke 2019-01-29]: A SgBinaryComposite represents many files, not just one, so some of these settings
    //                                don't make much sense.
    binaryComposite->set_sourceFileNameWithPath(boost::filesystem::absolute(fileNames[0]).string()); // best we can do
    binaryComposite->set_sourceFileNameWithoutPath(fileNames[0].filename().string());                // best we can do
    binaryComposite->initializeSourcePosition(fileNames[0].string());                                // best we can do
    binaryComposite->set_originalCommandLineArgumentList(std::vector<std::string>(1, fileNames[0].string())); // best we can do
    ASSERT_not_null(binaryComposite->get_file_info());

    // Create one or more SgAsmInterpretation nodes. If all the SgAsmGenericFile objects are ELF files, then there's one
    // SgAsmInterpretation that points to them all. If all the SgAsmGenericFile objects are PE files, then there's two
    // SgAsmInterpretation nodes: one for all the DOS parts of the files, and one for all the PE parts of the files.
    std::vector<std::pair<SgAsmExecutableFileFormat::ExecFamily, SgAsmInterpretation*>> interpretations;
    for (SgAsmGenericFile *file: fileList->get_files()) {
        SgAsmGenericHeaderList *headerList = file->get_headers();
        ASSERT_not_null(headerList);
        for (SgAsmGenericHeader *header: headerList->get_headers()) {
            SgAsmGenericFormat *format = header->get_executableFormat();
            ASSERT_not_null(format);

            // Find or create the interpretation that holds this family of headers.
            SgAsmInterpretation *interpretation = nullptr;
            for (size_t i = 0; i < interpretations.size() && !interpretation; ++i) {
                if (interpretations[i].first == format->get_family())
                    interpretation = interpretations[i].second;
            }
            if (!interpretation) {
                interpretation = new SgAsmInterpretation;
                interpretations.push_back(std::make_pair(format->get_family(), interpretation));
            }

            // Add the header to the interpretation. This isn't an AST parent/child link, so don't set the parent ptr.
            SgAsmGenericHeaderList *interpHeaders = interpretation->get_headers();
            ASSERT_not_null(interpHeaders);
            interpHeaders->get_headers().push_back(header);
        }
    }
    SAWYER_MESG(mlog[DEBUG]) <<"created " <<StringUtility::plural(interpretations.size(), "interpretation nodes") <<"\n";

    // Put all the interpretations in the interp list
    for (size_t i=0; i<interpretations.size(); ++i) {
        SgAsmInterpretation *interpretation = interpretations[i].second;
        interpList->get_interpretations().push_back(interpretation);
        interpretation->set_parent(interpList);
    }
    ASSERT_require(interpList->get_interpretations().size() == interpretations.size());

    // The project
    SgProject *project = new SgProject;
    project->get_fileList().push_back(binaryComposite);
    binaryComposite->set_parent(project);

    return project;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Memory map creation (loading)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BinaryLoader::Ptr
EngineBinary::obtainLoader(const BinaryLoader::Ptr &hint) {
    if (!binaryLoader_ && interpretation()) {
        if ((binaryLoader_ = BinaryLoader::lookup(interpretation()))) {
            binaryLoader_ = binaryLoader_->clone();
        }
    }

    if (!binaryLoader_ && hint)
        binaryLoader_ = hint;

    if (!binaryLoader_)
        throw std::runtime_error("no binary loader found and none specified");

    return binaryLoader_;
}

BinaryLoader::Ptr
EngineBinary::obtainLoader() {
    return obtainLoader(BinaryLoader::Ptr());
}

void
EngineBinary::loadContainers(const std::vector<std::string> & /*fileNames*/) {
    // Load the interpretation if it hasn't been already
    if (interpretation() && (!interpretation()->get_map() || interpretation()->get_map()->isEmpty())) {
        interpretation()->set_map(memoryMap());
        obtainLoader();
        binaryLoader_->load(interpretation());
    }

    // Get a map from the now-loaded interpretation, or use an empty map if the interp isn't mapped
    memoryMap(MemoryMap::instance());
    if (interpretation() && interpretation()->get_map()) {
        *memoryMap() = *interpretation()->get_map();
    }
}

void
EngineBinary::loadNonContainers(const std::vector<std::string> &fileNames) {
    MemoryMap::Ptr map = memoryMap();
    ASSERT_not_null(map);
    for (const std::string &fileName: fileNames) {
        if (boost::starts_with(fileName, "map:")) {
            std::string resource = fileName.substr(3);  // remove "map", leaving colon and rest of string
            map->insertFile(resource);
        } else if (boost::starts_with(fileName, "vxworks:")) {
            VxworksTerminal::Settings dflt;
            dflt.where = AddressInterval::whole();      // indicates that addresses are required for parseLocatorString
            auto vxworks = VxworksTerminal::instance(VxworksTerminal::parseLocatorString(fileName.substr(7), dflt).orThrow());
            if (const auto error = vxworks->open())
                throw Rose::Exception("open failed for \"" + StringUtility::cEscape(fileName) + "\": " + *error);
            vxworks->download(map, "VxWorks " + vxworks->settings().host).orThrow<Rose::Exception>();
        } else if (boost::starts_with(fileName, "data:")) {
            std::string resource = fileName.substr(4);  // remove "data", leaving colon and the rest of the string
            map->insertData(resource);
        } else if (boost::starts_with(fileName, "meta:")) {
            std::string resource = fileName.substr(4);  // "remove "meta", leaving the colon and the rest of the string
            map->adjustMap(resource);
        } else if (boost::starts_with(fileName, "proc:")) {
            std::string resource = fileName.substr(4);  // remove "proc", leaving colon and the rest of the string
            map->insertProcess(resource);
        } else if (boost::starts_with(fileName, "run:")) {
            // Split resource as "run:OPTIONS:EXECUTABLE"
#ifdef ROSE_ENABLE_DEBUGGER_LINUX
            static const size_t colon1 = 3;             // index of first colon in fileName
            const size_t colon2 = fileName.find(':', colon1+1); // index of second colon in FileName
            if (std::string::npos == colon2)
                throw Rose::Exception("two colons are required in \"run\" resource \"" + StringUtility::cEscape(fileName) + "\"");
            std::string optionsStr = fileName.substr(colon1+1, colon2-(colon1+1)); // between the first two colons
            std::string exeName = fileName.substr(colon2+1);                       // after the second colon

            // Parse options
            bool doReplace = false;                                                // "replace" word was present?
            bool aslr = false;                                                     // allow address space layout randomization?
            std::vector<std::string> options;
            boost::split(options, optionsStr, boost::is_any_of(","));
            for (const std::string &option: options) {
                if (option.empty()) {
                } else if ("replace" == option) {
                    doReplace = true;
                } else if ("aslr" == option) {
                    aslr = true;
                } else if ("noaslr" == option) {
                    aslr = false;
                } else {
                    throw Rose::Exception("option \"" + StringUtility::cEscape(option) + "\" not recognized"
                                          " in resource \"" + StringUtility::cEscape(fileName) + "\"");
                }
            }

            Debugger::Linux::Specimen subordinate(exeName);
            subordinate.flags()
                .set(Debugger::Linux::Flag::CLOSE_FILES)
                .set(Debugger::Linux::Flag::REDIRECT_INPUT)
                .set(Debugger::Linux::Flag::REDIRECT_OUTPUT)
                .set(Debugger::Linux::Flag::REDIRECT_ERROR);
            subordinate.randomizedAddresses(aslr);

            for (const std::string &name: settings().loader.envEraseNames)
                subordinate.eraseEnvironmentVariable(name);
            for (const boost::regex &re: settings().loader.envErasePatterns)
                subordinate.eraseMatchingEnvironmentVariables(re);
            for (const std::string &var: settings().loader.envInsert) {
                size_t eq = var.find('=');
                if (std::string::npos == eq)
                    throw std::runtime_error("no '=' in NAME=VALUE: \"" + StringUtility::cEscape(var) + "\"");
                if (eq == 0)
                    throw std::runtime_error("empty name in NAME=VALUE: \"" + StringUtility::cEscape(var) + "\"");
                subordinate.insertEnvironmentVariable(var.substr(0, eq), var.substr(eq+1));
            }
            auto debugger = Debugger::Linux::instance(subordinate);

            // Set breakpoints for all executable addresses in the memory map created by the Linux kernel. Since we're doing
            // this before the first instruction executes, no shared libraries have been loaded yet. However, the dynamic
            // linker itself is present as are the vdso and vsyscall segments.  We don't want to set breakpoints in anything
            // that the dynamic linker might call because the whole purpose of the "run:" URL is to get an accurate memory map
            // of the process after shared libraries are loaded. We assume that the kernel has loaded the executable at the
            // lowest address.
            MemoryMap::Ptr procMap = MemoryMap::instance();
            procMap->insertProcess(*debugger->processId(), MemoryMap::Attach::NO);
            procMap->require(MemoryMap::EXECUTABLE).keep();
            if (procMap->isEmpty())
                throw std::runtime_error(exeName + " has no executable addresses");
            std::string name = procMap->segments().begin()->name(); // lowest segment is always part of the main executable
            for (const MemoryMap::Node &node: procMap->nodes()) {
                if (node.value().name() == name)        // usually just one match; names are like "proc:123(/bin/ls)"
                    debugger->setBreakPoint(node.key());
            }

            debugger->runToBreakPoint(Debugger::ThreadId::unspecified());
            if (debugger->isTerminated())
                throw std::runtime_error(exeName + " " + debugger->howTerminated() + " without reaching a breakpoint");
            if (doReplace)
                map->clear();
            map->insertProcess(*debugger->processId(), MemoryMap::Attach::NO);
            debugger->terminate();
#else
            throw std::runtime_error("\"run:\" loader schema is not available in this configuration of ROSE");
#endif
        } else if (boost::starts_with(fileName, "srec:") || boost::ends_with(fileName, ".srec")) {
            std::string resource;                       // name of file to open
            unsigned perms = MemoryMap::READABLE | MemoryMap::WRITABLE | MemoryMap::EXECUTABLE;

            if (boost::starts_with(fileName, "srec:")) {
                // Format is "srec:[=PERMS]:FILENAME" where PERMS are the letters "r", "w", and/or "x"
                std::vector<std::string> parts = StringUtility::split(":", fileName, 3);
                if (parts.size() != 3)
                    throw std::runtime_error("second ':' expected in \"srec\" URI (expected \"srec:[=PERMS]:FILENAME\")");
                resource = parts[2];

                // Permissions, like "=rw". Lack of '=...' means default permissions; nothing after '=' means no permissions
                // (e.g., "srec:=:filename").
                if (!parts[1].empty()) {
                    if ('=' != parts[1][0])
                        throw std::runtime_error("expected \"=PERMS\" in \"srec:\" URI");
                    perms = 0;
                    for (size_t i=1; i<parts[1].size(); ++i) {
                        switch (parts[1][i]) {
                            case 'r': perms |= MemoryMap::READABLE; break;
                            case 'w': perms |= MemoryMap::WRITABLE; break;
                            case 'x': perms |= MemoryMap::EXECUTABLE; break;
                            default:
                                throw std::runtime_error("invalid permission character '" +
                                                         StringUtility::cEscape(parts[1].substr(i, 1)) +
                                                         "' in \"srec:\" URI");
                                break;
                        }
                    }
                }
            } else {
                resource = fileName;
            }

            // Parse and load the S-Record file
            if (resource.size()!=strlen(resource.c_str())) {
                throw std::runtime_error("file name contains internal NUL characters: \"" +
                                         StringUtility::cEscape(resource) + "\"");
            }
            std::ifstream input(resource.c_str());
            if (!input.good()) {
                throw std::runtime_error("cannot open Motorola S-Record file: \"" +
                                         StringUtility::cEscape(resource) + "\"");
            }
            std::vector<SRecord> srecs = SRecord::parse(input);
            for (size_t i=0; i<srecs.size(); ++i) {
                if (!srecs[i].error().empty())
                    mlog[ERROR] <<resource <<":" <<(i+1) <<": S-Record: " <<srecs[i].error() <<"\n";
            }
            SRecord::load(srecs, map, 1, perms, resource, MemoryMap::Clobber::NO);
        } else if (boost::starts_with(fileName, "vxcore:")) {
            // format is "vxcore:[MEMORY_ATTRS]:[FILE_ATTRS]:FILE_NAME
            loadVxCore(fileName.substr(7));             // the part after "vxcore:"
        }
    }
}

void
EngineBinary::loadVxCore(const std::string &spec) {
    VxcoreParser parser;
    boost::filesystem::path fileName = parser.parseUrl(spec);
    parser.parse(fileName, memoryMap());
    if (settings().disassembler.isaName.empty())
        settings().disassembler.isaName = parser.isaName();
}

MemoryMap::Ptr
EngineBinary::loadSpecimens(const std::vector<std::string> &fileNames) {
    try {
        if (!areContainersParsed()) {
            parseContainers(fileNames);
        }
        if (!memoryMap()) {
            memoryMap(MemoryMap::instance());
        }
        loadContainers(fileNames);
        loadNonContainers(fileNames);
        adjustMemoryMap();
        memoryMap()->shrinkUnshare();
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

Partitioner::Ptr
EngineBinary::createTunedPartitioner() {
    checkCreatePartitionerPrerequisites();
    Partitioner::Ptr partitioner = createBarePartitioner();

    for (FunctionPrologueMatcher::Ptr &matcher: architecture()->functionPrologueMatchers(sharedFromThis()))
        partitioner->functionPrologueMatchers().push_back(matcher);

    for (BasicBlockCallback::Ptr &callback: architecture()->basicBlockCreationHooks(sharedFromThis()))
        partitioner->basicBlockCallbacks().append(callback);



    Disassembler::Base::Ptr decoder = architecture()->newInstructionDecoder();
    if (as<Disassembler::M68k>(decoder))
        partitioner->basicBlockCallbacks().append(libcStartMain_ = ModulesLinux::LibcStartMain::instance());

    if (as<Disassembler::X86>(decoder)) {
        partitioner->basicBlockCallbacks().append(ModulesLinux::SyscallSuccessors::instance(partitioner,
                                                                                            settings().partitioner.syscallHeader));
        partitioner->basicBlockCallbacks().append(libcStartMain_ = ModulesLinux::LibcStartMain::instance());
    }

    return partitioner;
}

Partitioner::Ptr
EngineBinary::createPartitionerFromAst(SgAsmInterpretation *interp) {
    ASSERT_not_null(interp);
    interpretation(interp);
    memoryMap(MemoryMap::Ptr());
    loadSpecimens(std::vector<std::string>());
    Partitioner::Ptr partitioner = createTunedPartitioner();

    // Cache all the instructions so they're available by address in O(log N) time in the future.
    for (SgAsmInstruction *insn: AST::Traversal::findDescendantsTyped<SgAsmInstruction>(interp))
        partitioner->instructionProvider().insert(insn);

    // Create and attach basic blocks
    for (SgAsmNode *node: AST::Traversal::findDescendantsTyped<SgAsmNode>(interp)) {
        SgAsmBlock *blockAst = isSgAsmBlock(node);
        if (!blockAst || !blockAst->hasInstructions())
            continue;
        BasicBlock::Ptr bblock = BasicBlock::instance(blockAst->get_address(), partitioner);
        bblock->comment(blockAst->get_comment());

        // Instructions
        const SgAsmStatementPtrList &stmts = blockAst->get_statementList();
        for (SgAsmStatementPtrList::const_iterator si=stmts.begin(); si!=stmts.end(); ++si) {
            if (SgAsmInstruction *insn = isSgAsmInstruction(*si))
                bblock->append(partitioner, insn);
        }

        // Successors
        const SgAsmIntegerValuePtrList &successors = blockAst->get_successors();
        for (SgAsmIntegerValueExpression *ival: successors)
            bblock->insertSuccessor(ival->get_absoluteValue(), ival->get_significantBits());
        if (!blockAst->get_successorsComplete()) {
            size_t nbits = partitioner->instructionProvider().instructionPointerRegister().nBits();
            bblock->insertSuccessor(Semantics::SValue::instance_undefined(nbits));
        }

        partitioner->attachBasicBlock(bblock);
    }

    // Create and attach functions
    for (SgAsmFunction *funcAst: AST::Traversal::findDescendantsTyped<SgAsmFunction>(interp)) {
        if (0!=(funcAst->get_reason() & SgAsmFunction::FUNC_LEFTOVERS))
            continue;                                   // this isn't really a true function
        Function::Ptr function = Function::instance(funcAst->get_entryVa(), funcAst->get_name());
        function->comment(funcAst->get_comment());
        function->reasons(funcAst->get_reason());
        function->reasonComment(funcAst->get_reasonComment());

        for (SgAsmBlock *blockAst: AST::Traversal::findDescendantsTyped<SgAsmBlock>(funcAst)) {
            if (blockAst->hasInstructions())
                function->insertBasicBlock(blockAst->get_address());
        }

        for (SgAsmStaticData *dataAst: AST::Traversal::findDescendantsTyped<SgAsmStaticData>(funcAst)) {
            DataBlock::Ptr dblock = DataBlock::instanceBytes(dataAst->get_address(), dataAst->get_size());
            partitioner->attachDataBlock(dblock);
            function->insertDataBlock(dblock);
        }

        partitioner->attachFunction(function);
    }

    return partitioner;
}

Partitioner::Ptr
EngineBinary::createPartitioner() {
    return createTunedPartitioner();
}

void EngineBinary::runPartitionerInit(const Partitioner::Ptr &partitioner) {
    Sawyer::Message::Stream where(mlog[WHERE]);

    if (hasCilCodeSection()) {
        settings().partitioner.findingImportFunctions = false; // Don't find "_Cor_CorExeMain"
        settings().partitioner.findingEntryFunctions = false; // Do not find "_start"
        settings().partitioner.findingInterFunctionCalls = false; // chases memory willy nilly looking for instructions
        settings().partitioner.demangleNames = false; // throwing exception

        // what about the following:
        //   create: settings().partitioner.makingPrologueFunctions = false;
        //   findingFunctionPadding, findingDeadCode, findingIntraFunctionCode, findingIntraFunctionData, 
        //   findingFunctionCallFunctions, findingErrorFunctions, findingExportFunctions, findingSymbolFunctions,
        //   doingPostAnalysis, doingPost...
        //   functionReturnAnalysis, findingDataFunctionPointers, findingCodeFunctionPointers, findingThunks, splittingThunks,
        //   namingSyscalls, demangleNames
    }

    SAWYER_MESG(where) <<"labeling addresses\n";
    labelAddresses(partitioner, partitioner->configuration());

    SAWYER_MESG(where) <<"marking configured basic blocks\n";
    makeConfiguredDataBlocks(partitioner, partitioner->configuration());

    SAWYER_MESG(where) <<"marking configured functions\n";
    makeConfiguredFunctions(partitioner, partitioner->configuration());

    SAWYER_MESG(where) <<"marking ELF/PE container functions\n";
    makeContainerFunctions(partitioner, interpretation());

    SAWYER_MESG(where) <<"marking interrupt functions\n";
    makeInterruptVectorFunctions(partitioner, settings().partitioner.interruptVector);

    SAWYER_MESG(where) <<"marking user-defined functions\n";
    makeUserFunctions(partitioner, settings().partitioner.functionStartingVas);
}

void
EngineBinary::runPartitionerRecursive(const Partitioner::Ptr &partitioner) {
    Sawyer::Message::Stream where(mlog[WHERE]);

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

    // Make the semantics machine state available
    state_ = state;

    auto ops = InstructionSemantics::SymbolicSemantics::RiscOperators::instanceFromState(state, solver);
    InstructionSemantics::BaseSemantics::RiscOperatorsPtr baseOps = ops;

    // Decode and partition any CIL byte code (sections with name "CLR Runtime Header")
    SAWYER_MESG(where) <<"decoding and partitioning CIL byte code\n";
    bool foundCilSection = partitionCilSections(partitioner);
    if (foundCilSection) return;

    // Start discovering instructions and forming them into basic blocks and functions
    SAWYER_MESG(where) <<"discovering and populating functions\n";
    discoverFunctions(partitioner);

    // Try to attach basic blocks to functions
    SAWYER_MESG(where) <<"marking function call targets\n";
    if (settings().partitioner.findingFunctionCallFunctions) {
        SAWYER_MESG(where) <<"finding called functions\n";
        makeCalledFunctions(partitioner);
    }

    SAWYER_MESG(where) <<"discovering basic blocks for marked functions\n";
    attachBlocksToFunctions(partitioner);

    // Additional work
    if (settings().partitioner.findingDeadCode) {
        SAWYER_MESG(where) <<"attaching dead code to functions\n";
        attachDeadCodeToFunctions(partitioner);
    }
    if (settings().partitioner.findingFunctionPadding) {
        SAWYER_MESG(where) <<"attaching function padding\n";
        attachPaddingToFunctions(partitioner);
    }
    if (settings().partitioner.findingIntraFunctionCode > 0) {
        // WHERE message is emitted in the call
        attachAllSurroundedCodeToFunctions(partitioner);
    }
    if (settings().partitioner.findingIntraFunctionData) {
        SAWYER_MESG(where) <<"searching for inter-function code\n";
        attachSurroundedDataToFunctions(partitioner);
    }

    // Another pass to attach blocks to functions
    SAWYER_MESG(where) <<"discovering basic blocks for marked functions\n";
    attachBlocksToFunctions(partitioner);
}

void
EngineBinary::runPartitionerFinal(const Partitioner::Ptr &partitioner) {
    Sawyer::Message::Stream where(mlog[WHERE]);

    IndirectControlFlow::reanalyzeSomeBlocks(settings().icf, partitioner);

    if (settings().partitioner.splittingThunks) {
        // Splitting thunks off the front of a basic block causes the rest of the basic block to be discarded and then
        // rediscovered. This might also create additional blocks due to the fact that opaque predicate analysis runs only on
        // single blocks at a time -- splitting the block may have broken the opaque predicate.
        SAWYER_MESG(where) <<"splitting thunks from functions\n";
        splitThunkFunctions(partitioner, functionSplittingThunks_);
        discoverBasicBlocks(partitioner);
    }

    // Perform a final pass over all functions.
    SAWYER_MESG(where) <<"discovering basic blocks for marked functions\n";
    attachBlocksToFunctions(partitioner);

    if (interpretation()) {
        if (settings().partitioner.findingImportFunctions) { // TODO: consider settings().partitioner.namingImports
            SAWYER_MESG(where) <<"naming imports\n";
            ModulesPe::nameImportThunks(partitioner, interpretation());
            ModulesPowerpc::nameImportThunks(partitioner, interpretation());
        }
    }
    if (settings().partitioner.namingConstants) {
        SAWYER_MESG(where) <<"naming constants\n";
        Modules::nameConstants(partitioner, settings().partitioner.namingConstants);
    }
    if (!settings().partitioner.namingStrings.isEmpty()) {
        SAWYER_MESG(where) <<"naming strings\n";
        Modules::nameStrings(partitioner, settings().partitioner.namingStrings);
    }
    if (settings().partitioner.namingSyscalls) {
        SAWYER_MESG(where) <<"naming system calls\n";
        ModulesLinux::nameSystemCalls(partitioner, settings().partitioner.syscallHeader);
    }
    if (settings().partitioner.demangleNames) {
        SAWYER_MESG(where) <<"demangling names\n";
        Modules::demangleFunctionNames(partitioner);
    }
    if (SgBinaryComposite *bc = SageInterface::getEnclosingNode<SgBinaryComposite>(interpretation())) {
        // [Robb Matzke 2020-02-11]: This only works if ROSE was configured with external DWARF and ELF libraries.
        SAWYER_MESG(where) <<"mapping source locations\n";
        partitioner->sourceLocations().insertFromDebug(bc);
    }
    if (libcStartMain_)
        libcStartMain_->nameMainFunction(partitioner);
}

bool
EngineBinary::hasCilCodeSection() {
    if (auto interp{interpretation()}) {
        for (SgAsmGenericHeader* header : interp->get_headers()->get_headers()) {
            auto sections = header->get_sectionsByName("CLR Runtime Header");
            return sections.size() > 0;
        }
    }
    return false;
}

bool
EngineBinary::partitionCilSections(const Partitioner::Ptr &partitioner) {
    SgAsmCilMetadataRoot* mdr{nullptr};
    if (auto interp{interpretation()}) {
        for (SgAsmGenericHeader* header : interp->get_headers()->get_headers()) {
            auto sections = header->get_sectionsByName("CLR Runtime Header");
            for (SgAsmGenericSection* section : sections) {
                if (auto cliHeader = isSgAsmCliHeader(section)) {
                    if (mdr != nullptr) {
                        mlog[WARN] << "multiple CIL code sections seen by partitionCilSections\n";
                    }
                    mdr = cliHeader->get_metadataRoot();
                }
            }
        }
    }

    // If there is a "CLR Runtime Header" section, it's CIL byte code
    if (mdr) {
      // Don't do post analysis on a dotnet specimen
      settings().partitioner.doingPostAnalysis = false;

      ByteCode::CilContainer cilContainer{mdr};
      cilContainer.partition(partitioner);

      // Make the ByteCode analysis class available

      auto nss = cilContainer.namespaces();
      ASSERT_require(nss.size() > 0);

      auto ns = nss[0];
      auto cls = ns->classes();
      ASSERT_require(cls.size() > 0);

      analysisClass_ = cls[0];

      return true;
    }

    return false; // No CIL sections encountered
}

Partitioner::Ptr
EngineBinary::partition(const std::vector<std::string> &fileNames) {
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
            if (!areSpecimensLoaded())
                loadSpecimens(fileNames);
            obtainArchitecture();
            Partitioner::Ptr partitioner = createPartitioner();
            runPartitioner(partitioner);
            if (!partitioner->interpretation())
                partitioner->interpretation(interpretation());
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
//                                      Partitioner mid-level operations
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<Function::Ptr>
EngineBinary::makeEntryFunctions(const Partitioner::Ptr &partitioner, SgAsmInterpretation *interp) {
    ASSERT_not_null(partitioner);
    std::vector<Function::Ptr> retval;
    if (interp) {
        for (SgAsmGenericHeader *fileHeader: interp->get_headers()->get_headers()) {
            for (const RelativeVirtualAddress &rva: fileHeader->get_entryRvas()) {
                Address va = rva.rva() + fileHeader->get_baseVa();
                Function::Ptr function = Function::instance(va, "_start", SgAsmFunction::FUNC_ENTRY_POINT);
                insertUnique(retval, partitioner->attachOrMergeFunction(function), sortFunctionsByAddress);
                ASSERT_require2(function->address() == va, function->printableName());
            }
        }
    }
    return retval;
}

std::vector<Function::Ptr>
EngineBinary::makeErrorHandlingFunctions(const Partitioner::Ptr &partitioner, SgAsmInterpretation *interp) {
    ASSERT_not_null(partitioner);
    std::vector<Function::Ptr> retval;
    if (interp) {
        for (const Function::Ptr &function: ModulesElf::findErrorHandlingFunctions(interp))
            insertUnique(retval, partitioner->attachOrMergeFunction(function), sortFunctionsByAddress);
    }
    return retval;
}

std::vector<Function::Ptr>
EngineBinary::makeImportFunctions(const Partitioner::Ptr &partitioner, SgAsmInterpretation *interp) {
    ASSERT_not_null(partitioner);
    std::vector<Function::Ptr> retval;
    if (interp) {
        // Windows PE imports
        ModulesPe::rebaseImportAddressTables(partitioner, ModulesPe::getImportIndex(partitioner, interp));
        for (const Function::Ptr &function: ModulesPe::findImportFunctions(partitioner, interp)) {
            Address va = function->address();
            insertUnique(retval, partitioner->attachOrMergeFunction(function), sortFunctionsByAddress);
            ASSERT_always_require2(function->address() == va, function->printableName());
        }

        // ELF imports
        for (const Function::Ptr &function: ModulesElf::findPltFunctions(partitioner, interp))
            insertUnique(retval, partitioner->attachOrMergeFunction(function), sortFunctionsByAddress);
    }
    return retval;
}

std::vector<Function::Ptr>
EngineBinary::makeExportFunctions(const Partitioner::Ptr &partitioner, SgAsmInterpretation *interp) {
    ASSERT_not_null(partitioner);
    std::vector<Function::Ptr> retval;
    if (interp) {
        for (const Function::Ptr &function: ModulesPe::findExportFunctions(partitioner, interp))
            insertUnique(retval, partitioner->attachOrMergeFunction(function), sortFunctionsByAddress);
    }
    return retval;
}

std::vector<Function::Ptr>
EngineBinary::makeSymbolFunctions(const Partitioner::Ptr &partitioner, SgAsmInterpretation *interp) {
    ASSERT_not_null(partitioner);
    std::vector<Function::Ptr> retval;
    if (interp) {
        for (const Function::Ptr &function: Modules::findSymbolFunctions(partitioner, interp))
            insertUnique(retval, partitioner->attachOrMergeFunction(function), sortFunctionsByAddress);
    }
    return retval;
}

std::vector<Function::Ptr>
EngineBinary::makeContainerFunctions(const Partitioner::Ptr &partitioner, SgAsmInterpretation *interp) {
    std::vector<Function::Ptr> retval;
    Sawyer::Message::Stream where(mlog[WHERE]);

    if (settings().partitioner.findingImportFunctions) {
        SAWYER_MESG(where) <<"making import functions\n";
        for (const Function::Ptr &function: makeImportFunctions(partitioner, interp)) {
            if (auto exists = getOrInsertUnique(retval, function, sortFunctionsByAddress))
                (*exists)->insertReasons(SgAsmFunction::FUNC_IMPORT);
        }
    }
    if (settings().partitioner.findingExportFunctions) {
        SAWYER_MESG(where) <<"making export functions\n";
        for (const Function::Ptr &function: makeExportFunctions(partitioner, interp)) {
            if (auto exists = getOrInsertUnique(retval, function, sortFunctionsByAddress))
                (*exists)->insertReasons(SgAsmFunction::FUNC_EXPORT);
        }
    }
    if (settings().partitioner.findingSymbolFunctions) {
        SAWYER_MESG(where) <<"making symbol table functions\n";
        for (const Function::Ptr &function: makeSymbolFunctions(partitioner, interp)) {
            if (auto exists = getOrInsertUnique(retval, function, sortFunctionsByAddress))
                (*exists)->insertReasons(SgAsmFunction::FUNC_SYMBOL);
        }
    }
    if (settings().partitioner.findingEntryFunctions) {
        SAWYER_MESG(where) <<"making entry point functions\n";
        for (const Function::Ptr &function: makeEntryFunctions(partitioner, interp)) {
            if (auto exists = getOrInsertUnique(retval, function, sortFunctionsByAddress))
                (*exists)->insertReasons(SgAsmFunction::FUNC_ENTRY_POINT);
        }
    }
    if (settings().partitioner.findingErrorFunctions) {
        SAWYER_MESG(where) <<"making error-handling functions\n";
        for (const Function::Ptr &function: makeErrorHandlingFunctions(partitioner, interp)) {
            if (auto exists = getOrInsertUnique(retval, function, sortFunctionsByAddress))
                (*exists)->insertReasons(SgAsmFunction::FUNC_EH_FRAME);
        }
    }
    return retval;
}

std::vector<Function::Ptr>
EngineBinary::makeInterruptVectorFunctions(const Partitioner::Ptr &partitioner, const AddressInterval &interruptVector) {
    ASSERT_not_null(partitioner);
    std::vector<Function::Ptr> functions;
    if (interruptVector.isEmpty()) {
        return functions;
    }
    Disassembler::Base::Ptr disassembler = architecture()->newInstructionDecoder();
    if (!disassembler) {
        disassembler = partitioner->instructionProvider().disassembler();
    }    
    if (!disassembler) {
        throw std::runtime_error("cannot decode interrupt vector without architecture information");
    } else if (as<Disassembler::M68k>(disassembler)) {
        for (const Function::Ptr &f: ModulesM68k::findInterruptFunctions(partitioner, interruptVector.least()))
            insertUnique(functions, partitioner->attachOrMergeFunction(f), sortFunctionsByAddress);
    } else if (1 == interruptVector.size()) {
        throw std::runtime_error("cannot determine interrupt vector size for architecture");
    } else {
        size_t ptrSize = partitioner->instructionProvider().instructionPointerRegister().nBits();
        ASSERT_require2(ptrSize % 8 == 0, "instruction pointer register size is strange");
        size_t bytesPerPointer = ptrSize / 8;
        size_t nPointers = interruptVector.size() / bytesPerPointer;
        ByteOrder::Endianness byteOrder = partitioner->instructionProvider().defaultByteOrder();

        for (size_t i=0; i<nPointers; ++i) {
            Address elmtVa = interruptVector.least() + i*bytesPerPointer;
            uint32_t functionVa;
            if (4 == partitioner->memoryMap()->at(elmtVa).limit(4).read((uint8_t*)&functionVa).size()) {
                functionVa = ByteOrder::diskToHost(byteOrder, functionVa);
                std::string name = "interrupt_" + StringUtility::numberToString(i) + "_handler";
                Function::Ptr function = Function::instance(functionVa, name, SgAsmFunction::FUNC_EXCEPTION_HANDLER);
                if (Sawyer::Optional<Function::Ptr> found = getUnique(functions, function, sortFunctionsByAddress)) {
                    // Multiple vector entries point to the same function, so give it a rather generic name
                    found.get()->name("interrupt_vector_function");
                } else {
                    insertUnique(functions, partitioner->attachOrMergeFunction(function), sortFunctionsByAddress);
                }
            }
        }
    }
    return functions;
}

std::vector<Function::Ptr>
EngineBinary::makeUserFunctions(const Partitioner::Ptr &partitioner, const std::vector<Address> &vas) {
    ASSERT_not_null(partitioner);
    std::vector<Function::Ptr> retval;
    for (Address va: vas) {
        Function::Ptr function = Function::instance(va, SgAsmFunction::FUNC_CMDLINE);
        insertUnique(retval, partitioner->attachOrMergeFunction(function), sortFunctionsByAddress);
    }
    return retval;
}

void
EngineBinary::discoverBasicBlocks(const Partitioner::Ptr &partitioner) {
    while (makeNextBasicBlock(partitioner)) /*void*/;
}

Function::Ptr
EngineBinary::makeNextDataReferencedFunction(const Partitioner::ConstPtr &partitioner, Address &readVa /*in,out*/) {
    ASSERT_not_null(partitioner);
    const Address wordSize = partitioner->instructionProvider().instructionPointerRegister().nBits() / 8;
    ASSERT_require2(wordSize>0 && wordSize<=8, StringUtility::numberToString(wordSize)+"-byte words not implemented yet");
    const Address maxaddr = partitioner->memoryMap()->hull().greatest();

    while (readVa < maxaddr &&
           partitioner->memoryMap()->atOrAfter(readVa)
           .require(MemoryMap::READABLE).prohibit(MemoryMap::EXECUTABLE|MemoryMap::WRITABLE)
           .next().assignTo(readVa)) {

        // Addresses must be aligned on a word boundary
        if (Address misaligned = readVa % wordSize) {
            readVa = incrementAddress(readVa, wordSize-misaligned, maxaddr);
            continue;
        }

        // Convert raw memory to native address
        // FIXME[Robb P. Matzke 2014-12-08]: assuming little endian
        ASSERT_require(wordSize<=8);
        uint8_t raw[8];
        if (partitioner->memoryMap()->at(readVa).limit(wordSize)
            .require(MemoryMap::READABLE).prohibit(MemoryMap::EXECUTABLE|MemoryMap::WRITABLE)
            .read(raw).size()!=wordSize) {
            readVa = incrementAddress(readVa, wordSize, maxaddr);
            continue;
        }
        Address targetVa = 0;
        for (size_t i=0; i<wordSize; ++i)
            targetVa |= raw[i] << (8*i);

        // Sanity checks
        SgAsmInstruction *insn = partitioner->discoverInstruction(targetVa);
        if (!insn || architecture()->isUnknown(insn)) {
            readVa = incrementAddress(readVa, wordSize, maxaddr);
            continue;                                   // no instruction
        }
        AddressInterval insnInterval = AddressInterval::baseSize(insn->get_address(), insn->get_size());
        if (!partitioner->instructionsOverlapping(insnInterval).empty()) {
            readVa = incrementAddress(readVa, wordSize, maxaddr);
            continue;                                   // would overlap with existing instruction
        }

        // All seems okay, so make a function there
        // FIXME[Robb P. Matzke 2014-12-08]: USERDEF is not the best, most descriptive reason, but it's what we have for now
        mlog[INFO] <<"possible code address " <<StringUtility::addrToString(targetVa)
                   <<" found at read-only address " <<StringUtility::addrToString(readVa) <<"\n";
        readVa = incrementAddress(readVa, wordSize, maxaddr);
        Function::Ptr function = Function::instance(targetVa, SgAsmFunction::FUNC_SCAN_RO_DATA);
        function->reasonComment("at ro-data address " + StringUtility::addrToString(readVa));
        return function;
    }
    readVa = maxaddr;
    return Function::Ptr();
}

Function::Ptr
EngineBinary::makeNextCodeReferencedFunction(const Partitioner::ConstPtr &partitioner) {
    ASSERT_not_null(partitioner);
    // As basic blocks are inserted into the CFG their instructions go into a set to be examined by this function. Once this
    // function examines them, it moves them to an already-examined set.
    Address constant = 0;
    while (codeFunctionPointers() && codeFunctionPointers()->nextConstant(partitioner).assignTo(constant)) {
        Address srcVa = codeFunctionPointers()->inProgress();
        SgAsmInstruction *srcInsn = partitioner->instructionProvider()[srcVa];
        ASSERT_not_null(srcInsn);

        SgAsmInstruction *targetInsn = partitioner->discoverInstruction(constant);
        if (!targetInsn || architecture()->isUnknown(targetInsn))
            continue;                                   // no instruction

        AddressInterval insnInterval = AddressInterval::baseSize(targetInsn->get_address(), targetInsn->get_size());
        if (!partitioner->instructionsOverlapping(insnInterval).empty())
            continue;                                   // would overlap with existing instruction

        // All seems okay, so make a function there
        // FIXME[Robb P Matzke 2017-04-13]: USERDEF is not the best, most descriptive reason, but it's what we have for now
        mlog[INFO] <<"possible code address " <<StringUtility::addrToString(constant)
                   <<" found in instruction at " <<StringUtility::addrToString(srcVa) <<"\n";
        Function::Ptr function = Function::instance(constant, SgAsmFunction::FUNC_INSN_RO_DATA);

        function->reasonComment("from " + srcInsn->toString() + ", ro-data address " + StringUtility::addrToString(constant));
        return function;
    }
    return Function::Ptr();
}

std::vector<Function::Ptr>
EngineBinary::makeCalledFunctions(const Partitioner::Ptr &partitioner) {
    ASSERT_not_null(partitioner);
    std::vector<Function::Ptr> retval;
    for (const Function::Ptr &function: partitioner->discoverCalledFunctions())
        insertUnique(retval, partitioner->attachOrMergeFunction(function), sortFunctionsByAddress);
    return retval;
}

std::vector<Function::Ptr>
EngineBinary::makeNextPrologueFunction(const Partitioner::Ptr &partitioner, Address startVa) {
    return makeNextPrologueFunction(partitioner, startVa, startVa);
}

std::vector<Function::Ptr>
EngineBinary::makeNextPrologueFunction(const Partitioner::Ptr &partitioner, Address startVa, Address &lastSearchedVa) {
    ASSERT_not_null(partitioner);
    std::vector<Function::Ptr> functions = partitioner->nextFunctionPrologue(startVa, lastSearchedVa /*out*/);
    for (const Function::Ptr &function: functions)
        partitioner->attachOrMergeFunction(function);
    return functions;
}

std::vector<Function::Ptr>
EngineBinary::makeFunctionFromInterFunctionCalls(const Partitioner::Ptr &partitioner, Address &startVa /*in,out*/) {
    ASSERT_not_null(partitioner);
    static const Address MAX_ADDR(-1);
    static const std::vector<Function::Ptr> NO_FUNCTIONS;
    static const char *me = "makeFunctionFromInterFunctionCalls: ";
    Sawyer::Message::Stream debug(mlog[DEBUG]);
    SAWYER_MESG(debug) <<me <<"(startVa = " <<StringUtility::addrToString(startVa) <<")\n";

    // Avoid creating large basic blocks since this can drastically slow down instruction semantics. Large basic blocks are a
    // real possibility here because we're likely to be interpreting data areas as code. We're not creating any permanent basic
    // blocks in this analysis, so limiting the size here has no effect on which blocks are ultimately added to the control
    // flow graph.  The smaller the limit, the more likely that a multi-instruction call will get split into two blocks and not
    // detected. Multi-instruction calls are an obfuscation technique.
    Sawyer::TemporaryCallback<BasicBlockCallback::Ptr>
        tcb(partitioner->basicBlockCallbacks(), Modules::BasicBlockSizeLimiter::instance(20)); // arbitrary

    while (AddressInterval unusedVas = partitioner->aum().nextUnused(startVa)) {

        // The unused interval must have executable addresses, otherwise skip to the next unused interval.
        AddressInterval unusedExecutableVas = memoryMap()->within(unusedVas).require(MemoryMap::EXECUTABLE).available();
        if (unusedExecutableVas.isEmpty()) {
            if (unusedVas.greatest() == MAX_ADDR) {
                startVa = MAX_ADDR;
                return NO_FUNCTIONS;
            } else {
                startVa = unusedVas.greatest() + 1;
                continue;
            }
        }
        startVa = unusedExecutableVas.least();
        SAWYER_MESG(debug) <<me <<"examining interval " <<StringUtility::addrToString(unusedExecutableVas.least())
                           <<".." <<StringUtility::addrToString(unusedExecutableVas.greatest()) <<"\n";

        while (startVa <= unusedExecutableVas.greatest()) {
            // Discover the basic block. It's possible that the partitioner already knows about this block location but just
            // hasn't tried looking for its instructions yet.  I don't think this happens within the stock engine because it
            // tries to recursively discover all basic blocks before it starts scanning things that might be data. But users
            // might call this before they've processed all the outstanding placeholders.  Consider the following hypothetical
            // user's partitioner state
            //          B1: push ebp            ; this block's insns are discovered
            //              mov ebp, esp
            //              test eax, eax
            //              je B3
            //          B2: inc eax             ; this block's insns are discovered
            //          B3: ???                 ; this block is known, but no instructions discovered yet
            //              ???                 ; arbitrary number of blocks
            //          Bn: push ebp            ; this block's insns are discovered
            //              mov ebp, esp
            // To this analysis, the region (B3+1)..Bn is ambiguous. It could be code or data or some of both. Ideally, we
            // should have used the recursive disassembly to process this area already. We'll just pretend it's all unknown and
            // process it with a linear sweep like normal. This is probably fine for normal code since linear and recursive are
            // mostly the same and we'll eventually discover the correct blocks anyway when we finally do the recursive (this
            // analysis doesn't actually create blocks in this region -- it only looks for call sites).
            BasicBlock::Ptr bb;
            if (partitioner->placeholderExists(startVa)) {
#if 0 // DEBUGGING [Robb P Matzke 2016-06-30]
                if (mlog[WARN]) {
                    mlog[WARN] <<me <<"va " <<StringUtility::addrToString(startVa) <<" has ambiguous disposition:\n";
                    static bool emitted = false;
                    if (!emitted) {
                        mlog[WARN] <<"  it is a basic block (with undiscovered instructions),\n"
                                   <<"  and it is absent from the address usage map\n"
                                   <<"  this analysis should be called after all pending instructions are discovered\n";
                        mlog[WARN] <<"  interval " <<StringUtility::addrToString(startVa)
                                   <<".." <<StringUtility::addrToString(unusedExecutableVas.greatest())
                                   <<" treated as unknown\n";
                        mlog[WARN] <<"  the CFG contains "
                                   <<StringUtility::plural(partitioner->undiscoveredVertex()->nInEdges(), "blocks")
                                   <<" lacking instructions\n";
                        emitted = true;
                    }
                }
#endif
            } else {
                bb = partitioner->discoverBasicBlock(startVa);
            }

            // Increment the startVa to be the next unused address. We can't just use the fall-through address of the basic
            // block we just discovered because it might not be contiguous in memory.  Watch out for overflow since it's
            // possible that startVa is already the last address in the address space (in which case we leave startVa as is and
            // return).
            if (bb == nullptr || bb->nInstructions() == 0) {
                if (startVa == MAX_ADDR)
                    return NO_FUNCTIONS;
                ++startVa;
                continue;
            }
            if (debug) {
                debug <<me <<bb->printableName() <<"\n";
                for (SgAsmInstruction *insn: bb->instructions())
                    debug <<me <<"  " <<partitioner->unparse(insn) <<"\n";
            }
            AddressIntervalSet bbVas = bb->insnAddresses();
            if (!bbVas.leastNonExistent(bb->address()).assignTo(startVa)) // address of first hole, or following address
                startVa = MAX_ADDR;                                       // bb ends at max address

            // Basic block sanity checks because we're not sure that we're actually disassembling real code. The basic block
            // should not overlap with anything (basic block, data block, function) already attached to the CFG.
            if (partitioner->aum().anyExists(bbVas)) {
                SAWYER_MESG(debug) <<me <<"candidate basic block overlaps with another; skipping\n";
                continue;
            }

            if (!partitioner->basicBlockIsFunctionCall(bb, Precision::LOW)) {
                SAWYER_MESG(debug) <<me <<"candidate basic block is not a function call; skipping\n";
                continue;
            }

            // Look at the basic block successors to find those which appear to be function calls. Note that the edge types are
            // probably all E_NORMAL at this point rather than E_FUNCTION_CALL, and the call-return edges are not yet present.
            std::set<Address> candidateFunctionVas; // entry addresses for potential new functions
            BasicBlock::Successors successors = partitioner->basicBlockSuccessors(bb, Precision::LOW);
            for (const BasicBlock::Successor &succ: successors) {
                if (auto targetVa = succ.expr()->toUnsigned()) {
                    if (*targetVa == bb->fallthroughVa()) {
                        SAWYER_MESG(debug) <<me <<"successor " <<StringUtility::addrToString(*targetVa) <<" is fall-through\n";
                    } else if (partitioner->functionExists(*targetVa)) {
                        // We already know about this function, so move on -- nothing to see here.
                        SAWYER_MESG(debug) <<me <<"successor " <<StringUtility::addrToString(*targetVa) <<" already exists\n";
                    } else if (partitioner->aum().exists(*targetVa)) {
                        // Target is inside some basic block, data block, or function but not a function entry. The basic block
                        // we just discovered is probably bogus, therefore ignore everything about it.
                        candidateFunctionVas.clear();
                        SAWYER_MESG(debug) <<me <<"successor " <<StringUtility::addrToString(*targetVa) <<" has a conflict\n";
                        break;
                    } else if (!memoryMap()->at(*targetVa).require(MemoryMap::EXECUTABLE).exists()) {
                        // Target is in an unmapped area or is not executable. The basic block we just discovered is probably
                        // bogus, therefore ignore everything about it.
                        SAWYER_MESG(debug) <<me <<"successor " <<StringUtility::addrToString(*targetVa) <<" not executable\n";
                        candidateFunctionVas.clear();
                        break;
                    } else {
                        // Looks good.
                        candidateFunctionVas.insert(*targetVa);
                    }
                }
            }

            // Create new functions containing only the entry blocks. We'll discover the rest of their blocks later by
            // recursively following their control flow.
            if (!candidateFunctionVas.empty()) {
                std::vector<Function::Ptr> newFunctions;
                for (Address functionVa: candidateFunctionVas) {
                    Function::Ptr newFunction = Function::instance(functionVa, SgAsmFunction::FUNC_CALL_INSN);
                    newFunction->reasonComment("from " + bb->instructions().back()->toString());
                    newFunctions.push_back(partitioner->attachOrMergeFunction(newFunction));
                    SAWYER_MESG(debug) <<me <<"created " <<newFunction->printableName() <<" from " <<bb->printableName() <<"\n";
                }
                return newFunctions;
            }

            // Avoid overflow or infinite loop
            if (startVa == MAX_ADDR)
                return NO_FUNCTIONS;
        }
    }
    return NO_FUNCTIONS;
}

void
EngineBinary::discoverFunctions(const Partitioner::Ptr &partitioner) {
    ASSERT_not_null(partitioner);
    Address nextPrologueVa = 0;                         // where to search for function prologues
    Address nextInterFunctionCallVa = 0;                // where to search for inter-function call instructions
    Address nextReadAddr = 0;                           // where to look for read-only function addresses

    while (1) {
        // Find as many basic blocks as possible by recursively following the CFG as we build it.
        discoverBasicBlocks(partitioner);

        // Resolve indirect control flow
        if (IndirectControlFlow::analyzeAllBlocks(settings().icf, partitioner))
            continue;

        // No pending basic blocks, so look for a function prologue. This creates a pending basic block for the function's entry
        // block, so go back and look for more basic blocks again.
        if (!hasCilCodeSection() && !partitioner->memoryMap()->isEmpty()) {
            if (nextPrologueVa < partitioner->memoryMap()->hull().greatest()) {
                std::vector<Function::Ptr> newFunctions =
                    makeNextPrologueFunction(partitioner, nextPrologueVa, nextPrologueVa /*out*/);
                if (nextPrologueVa < partitioner->memoryMap()->hull().greatest())
                    ++nextPrologueVa;
                if (!newFunctions.empty())
                    continue;
            }
        }

        // Scan inter-function code areas to find basic blocks that look reasonable and process them with instruction semantics to
        // find calls to functions that we don't know about yet.
        if (settings().partitioner.findingInterFunctionCalls) {
            std::vector<Function::Ptr> newFunctions =
                makeFunctionFromInterFunctionCalls(partitioner, nextInterFunctionCallVa /*in,out*/);
            if (!newFunctions.empty())
                continue;
        }

        // Try looking at literal constants inside existing instructions to find possible pointers to new functions
        if (settings().partitioner.findingCodeFunctionPointers) {
            if (Function::Ptr function = makeNextCodeReferencedFunction(partitioner)) {
                partitioner->attachFunction(function);
                continue;
            }
        }

        // Try looking for a function address mentioned in read-only memory
        if (settings().partitioner.findingDataFunctionPointers) {
            if (Function::Ptr function = makeNextDataReferencedFunction(partitioner, nextReadAddr /*in,out*/)) {
                partitioner->attachFunction(function);
                continue;
            }
        }

        // Nothing more to do
        break;
    }
}

std::set<Address>
EngineBinary::attachDeadCodeToFunction(const Partitioner::Ptr &partitioner, const Function::Ptr &function, size_t maxIterations) {
    ASSERT_not_null(partitioner);
    ASSERT_not_null(function);
    std::set<Address> retval;

    for (size_t i=0; i<maxIterations; ++i) {
        // Find ghost edges
        std::set<Address> ghosts = partitioner->functionGhostSuccessors(function);
        if (ghosts.empty())
            break;

        // Insert placeholders for all ghost edge targets
        partitioner->detachFunction(function);          // so we can modify its basic block ownership list
        for (Address ghost: ghosts) {
            if (retval.find(ghost)==retval.end()) {
                partitioner->insertPlaceholder(ghost);  // ensure a basic block gets created here
                function->insertBasicBlock(ghost);      // the function will own this basic block
                retval.insert(ghost);
            }
        }

        // If we're about to do more iterations then we should recursively discover instructions for pending basic blocks. Once
        // we've done that we should traverse the function's CFG to see if some of those new basic blocks are reachable and
        // should also be attached to the function.
        if (i+1 < maxIterations) {
            while (makeNextBasicBlock(partitioner)) /*void*/;
            partitioner->discoverFunctionBasicBlocks(function);
        }
    }

    partitioner->attachFunction(function);               // no-op if still attached
    return retval;
}

DataBlock::Ptr
EngineBinary::attachPaddingToFunction(const Partitioner::Ptr &partitioner, const Function::Ptr &function) {
    ASSERT_not_null(partitioner);
    ASSERT_not_null(function);
    if (DataBlock::Ptr padding = partitioner->matchFunctionPadding(function)) {
        partitioner->attachDataBlockToFunction(padding, function);
        return padding;
    }
    return DataBlock::Ptr();
}

std::vector<DataBlock::Ptr>
EngineBinary::attachPaddingToFunctions(const Partitioner::Ptr &partitioner) {
    ASSERT_not_null(partitioner);
    std::vector<DataBlock::Ptr> retval;
    for (const Function::Ptr &function: partitioner->functions()) {
        if (DataBlock::Ptr padding = attachPaddingToFunction(partitioner, function))
            insertUnique(retval, padding, sortDataBlocks);
    }
    return retval;
}

size_t
EngineBinary::attachAllSurroundedCodeToFunctions(const Partitioner::Ptr &partitioner) {
    Sawyer::Message::Stream where(mlog[WHERE]);
    size_t retval = 0;
    for (size_t i = 0; i < settings().partitioner.findingIntraFunctionCode; ++i) {
        SAWYER_MESG(where) <<"searching for intra-function code (pass " <<(i+1) <<")\n";
        size_t n = attachSurroundedCodeToFunctions(partitioner);
        if (0 == n)
            break;
        retval += n;
        discoverBasicBlocks(partitioner);
        if (settings().partitioner.findingFunctionCallFunctions)
            makeCalledFunctions(partitioner);
        attachBlocksToFunctions(partitioner);
    }
    return retval;
}

// Assumes that each unused address interaval that's surrounded by a single function begins coincident with the beginning of an
// as yet undiscovered basic block and adds a basic block placeholder to the surrounding function.  This could be further
// improved by testing to see if the candidate address looks like a valid basic block.
size_t
EngineBinary::attachSurroundedCodeToFunctions(const Partitioner::Ptr &partitioner) {
    ASSERT_not_null(partitioner);
    size_t nNewBlocks = 0;
    if (partitioner->aum().isEmpty())
        return 0;
    Address va = partitioner->aum().hull().least() + 1;
    while (va < partitioner->aum().hull().greatest()) {
        // Find an address interval that's unused and also executable.
        AddressInterval unusedAum = partitioner->aum().nextUnused(va);
        if (!unusedAum || unusedAum.greatest() > partitioner->aum().hull().greatest())
            break;
        AddressInterval interval = partitioner->memoryMap()->within(unusedAum).require(MemoryMap::EXECUTABLE).available();
        if (interval == unusedAum) {
            // Is this interval immediately surrounded by a single function?
            typedef std::vector<Function::Ptr> Functions;
            Functions beforeFuncs = partitioner->functionsOverlapping(interval.least()-1);
            Functions afterFuncs = partitioner->functionsOverlapping(interval.greatest()+1);
            Functions enclosingFuncs(beforeFuncs.size());
            Functions::iterator final = std::set_intersection(beforeFuncs.begin(), beforeFuncs.end(),
                                                              afterFuncs.begin(), afterFuncs.end(), enclosingFuncs.begin());
            enclosingFuncs.resize(final-enclosingFuncs.begin());
            if (1 == enclosingFuncs.size()) {
                Function::Ptr function = enclosingFuncs[0];

                // Add the address to the function
                mlog[DEBUG] <<"attachSurroundedCodeToFunctions: basic block " <<StringUtility::addrToString(interval.least())
                            <<" is attached now to function " <<function->printableName() <<"\n";
                partitioner->detachFunction(function);
                if (function->insertBasicBlock(interval.least()))
                    ++nNewBlocks;
                partitioner->attachFunction(function);
            }
        }

        // Advance to next unused interval
        if (unusedAum.greatest() > partitioner->aum().hull().greatest())
            break;                                      // prevent possible overflow
        va = unusedAum.greatest() + 1;
    }
    return nNewBlocks;
}

void
EngineBinary::attachBlocksToFunctions(const Partitioner::Ptr &partitioner) {
    ASSERT_not_null(partitioner);
    std::vector<Function::Ptr> retval;
    for (const Function::Ptr &function: partitioner->functions()) {
        partitioner->detachFunction(function);           // must be detached in order to modify block ownership
        partitioner->discoverFunctionBasicBlocks(function);
        partitioner->attachFunction(function);
    }
}

// Finds dead code and adds it to the function to which it seems to belong.
std::set<Address>
EngineBinary::attachDeadCodeToFunctions(const Partitioner::Ptr &partitioner, size_t maxIterations) {
    ASSERT_not_null(partitioner);
    std::set<Address> retval;
    for (const Function::Ptr &function: partitioner->functions()) {
        std::set<Address> deadVas = attachDeadCodeToFunction(partitioner, function, maxIterations);
        retval.insert(deadVas.begin(), deadVas.end());
    }
    return retval;
}

std::vector<DataBlock::Ptr>
EngineBinary::attachSurroundedDataToFunctions(const Partitioner::Ptr &partitioner) {
    ASSERT_not_null(partitioner);

    // Find executable addresses that are not yet used in the CFG/AUM
    AddressIntervalSet executableSpace;
    for (const MemoryMap::Node &node: partitioner->memoryMap()->nodes()) {
        if ((node.value().accessibility() & MemoryMap::EXECUTABLE) != 0)
            executableSpace.insert(node.key());
    }
    AddressIntervalSet unused = partitioner->aum().unusedExtent(executableSpace);

    // Iterate over the large unused address intervals and find their surrounding functions
    std::vector<DataBlock::Ptr> retval;
    for (const AddressInterval &interval: unused.intervals()) {
        if (interval.least()<=executableSpace.least() || interval.greatest()>=executableSpace.greatest())
            continue;
        typedef std::vector<Function::Ptr> Functions;
        Functions beforeFuncs = partitioner->functionsOverlapping(interval.least()-1);
        Functions afterFuncs = partitioner->functionsOverlapping(interval.greatest()+1);

        // What functions are in both sets?
        Functions enclosingFuncs(beforeFuncs.size());
        Functions::iterator final = std::set_intersection(beforeFuncs.begin(), beforeFuncs.end(),
                                                          afterFuncs.begin(), afterFuncs.end(), enclosingFuncs.begin());
        enclosingFuncs.resize(final-enclosingFuncs.begin());

        // Add the data block to all enclosing functions
        if (!enclosingFuncs.empty()) {
            DataBlock::Ptr dblock = DataBlock::instanceBytes(interval.least(), interval.size());
            dblock->comment("data encapsulated by function");
            for (const Function::Ptr &function: enclosingFuncs) {
                dblock = partitioner->attachDataBlockToFunction(dblock, function);
                insertUnique(retval, dblock, sortDataBlocks);
            }
        }
    }
    return retval;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Partitioner low-level stuff
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Return true if a new CFG edge was added.
bool
EngineBinary::makeNextCallReturnEdge(const Partitioner::Ptr &partitioner, boost::logic::tribool assumeReturns) {
    ASSERT_not_null(partitioner);
    Sawyer::Container::DistinctList<Address> &workList = basicBlockWorkList()->pendingCallReturn();
    while (!workList.isEmpty()) {
        Address va = workList.popBack();
        ControlFlowGraph::VertexIterator caller = partitioner->findPlaceholder(va);

        // Some sanity checks because it could be possible for this list to be out-of-date if the user monkeyed with the
        // partitioner's CFG adjuster.
        if (caller == partitioner->cfg().vertices().end()) {
            SAWYER_MESG(mlog[WARN]) <<StringUtility::addrToString(va) <<" was present on the basic block worklist "
                                    <<"but not in the CFG\n";
            continue;
        }
        BasicBlock::Ptr bb = caller->value().bblock();
        if (!bb)
            continue;
        if (!partitioner->basicBlockIsFunctionCall(bb))
            continue;
        if (hasCallReturnEdges(caller))
            continue;

        // If the new vertex lacks a call-return edge (tested above) and its callee has positive or indeterminate may-return
        // then we may need to add a call-return edge depending on whether assumeCallReturns is true.
        boost::logic::tribool mayReturn;
        Confidence confidence = PROVED;
        switch (settings().partitioner.functionReturnAnalysis) {
            case MAYRETURN_ALWAYS_NO:
                mayReturn = false;
                confidence = ASSUMED;
                break;
            case MAYRETURN_ALWAYS_YES:
                mayReturn = true;
                confidence = ASSUMED;
                break;
            case MAYRETURN_DEFAULT_YES:
            case MAYRETURN_DEFAULT_NO:
                mayReturn = hasAnyCalleeReturn(partitioner, caller);
                if (boost::logic::indeterminate(mayReturn)) {
                    mayReturn = assumeReturns;
                    confidence = ASSUMED;
                }
                break;
        }

        if (mayReturn) {
            size_t nBits = partitioner->instructionProvider().instructionPointerRegister().nBits();
            partitioner->detachBasicBlock(bb);
            bb->insertSuccessor(bb->fallthroughVa(), nBits, E_CALL_RETURN, confidence);
            partitioner->attachBasicBlock(caller, bb);
            return true;
        } else if (!mayReturn) {
            return false;
        } else {
            // We're not sure yet whether a call-return edge should be inserted, so save vertex for later.
            ASSERT_require(boost::logic::indeterminate(mayReturn));
            basicBlockWorkList()->processedCallReturn().pushBack(va);
        }
    }
    return false;
}

// Discover a basic block's instructions for some placeholder that has no basic block yet.
BasicBlock::Ptr
EngineBinary::makeNextBasicBlockFromPlaceholder(const Partitioner::Ptr &partitioner) {
    ASSERT_not_null(partitioner);

    // Pick the first (as LIFO) item from the undiscovered worklist. Make sure the item is truly undiscovered
    while (!basicBlockWorkList()->undiscovered().isEmpty()) {
        Address va = basicBlockWorkList()->undiscovered().popBack();
        ControlFlowGraph::VertexIterator placeholder = partitioner->findPlaceholder(va);
        if (placeholder == partitioner->cfg().vertices().end()) {
            mlog[WARN] <<"makeNextBasicBlockFromPlaceholder: block " <<StringUtility::addrToString(va)
                       <<" was on the undiscovered worklist but not in the CFG\n";
            continue;
        }
        ASSERT_require(placeholder->value().type() == V_BASIC_BLOCK);
        if (placeholder->value().bblock()) {
            SAWYER_MESG(mlog[DEBUG]) <<"makeNextBasicBlockFromPlacholder: block " <<StringUtility::addrToString(va)
                                     <<" was on the undiscovered worklist but is already discovered\n";
            continue;
        }
        BasicBlock::Ptr bb = partitioner->discoverBasicBlock(placeholder);
        partitioner->attachBasicBlock(placeholder, bb);
        return bb;
    }

    // The user probably monkeyed with basic blocks without notifying this engine, so just consume a block that we don't know
    // about. The order in which such placeholder blocks are discovered is arbitrary.
    if (partitioner->undiscoveredVertex()->nInEdges() > 0) {
        mlog[WARN] <<"partitioner engine undiscovered worklist is empty but CFG undiscovered vertex is not\n";
        ControlFlowGraph::VertexIterator placeholder = partitioner->undiscoveredVertex()->inEdges().begin()->source();
        ASSERT_require(placeholder->value().type() == V_BASIC_BLOCK);
        BasicBlock::Ptr bb = partitioner->discoverBasicBlock(placeholder);
        partitioner->attachBasicBlock(placeholder, bb);
        return bb;
    }

    // Nothing to discover
    return BasicBlock::Ptr();
}

// make a new basic block for an arbitrary placeholder
BasicBlock::Ptr
EngineBinary::makeNextBasicBlock(const Partitioner::Ptr &partitioner) {
    ASSERT_not_null(basicBlockWorkList());

    while (1) {
        // If there's an undiscovered basic block then discover it.
        if (BasicBlock::Ptr bb = makeNextBasicBlockFromPlaceholder(partitioner))
            return bb;

        // If there's a function call that needs a new call-return edge then add such an edge, but only if we know the callee
        // has a positive may-return analysis. If we don't yet know with certainty whether the call may return or will never
        // return then move the vertex to the processedCallReturn list to save it for later (this happens as part of
        // makeNextCallReturnEdge().
        if (!basicBlockWorkList()->pendingCallReturn().isEmpty()) {
            makeNextCallReturnEdge(partitioner, boost::logic::indeterminate);
            continue;
        }

        // We've added call-return edges everywhere possible, but may have delayed adding them to blocks where the analysis was
        // indeterminate. If so, sort all those blocks approximately by their height in the global CFG and run may-return
        // analysis on each one
        if (!basicBlockWorkList()->processedCallReturn().isEmpty() || !basicBlockWorkList()->finalCallReturn().isEmpty()) {
            ASSERT_require(basicBlockWorkList()->pendingCallReturn().isEmpty());
            if (!basicBlockWorkList()->processedCallReturn().isEmpty())
                basicBlockWorkList()->moveAndSortCallReturn(partitioner);
            while (!basicBlockWorkList()->finalCallReturn().isEmpty()) {
                basicBlockWorkList()->pendingCallReturn().pushBack(basicBlockWorkList()->finalCallReturn().popFront());
                makeNextCallReturnEdge(partitioner, partitioner->assumeFunctionsReturn());
            }
            continue;
        }

        // Nothing to do
        break;
    }

    // Nothing more to do; all lists are empty
    ASSERT_require(basicBlockWorkList()->undiscovered().isEmpty());
    ASSERT_require(basicBlockWorkList()->pendingCallReturn().isEmpty());
    ASSERT_require(basicBlockWorkList()->processedCallReturn().isEmpty());
    ASSERT_require(basicBlockWorkList()->finalCallReturn().isEmpty());
    return BasicBlock::Ptr();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Build AST
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SgAsmBlock*
EngineBinary::buildAst(const std::vector<std::string> &fileNames) {
    try {
        Partitioner::Ptr partitioner = partition(fileNames);
        return Modules::buildAst(partitioner, interpretation(), settings().astConstruction);
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
//                                      Settings and Properties
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BinaryLoader::Ptr
EngineBinary::binaryLoader() const {
    return binaryLoader_;
}

void
EngineBinary::binaryLoader(const BinaryLoader::Ptr &loader) {
    binaryLoader_ = loader;
}

ThunkPredicates::Ptr
EngineBinary::functionMatcherThunks() const {
    return functionMatcherThunks_;
}

void
EngineBinary::functionMatcherThunks(const ThunkPredicates::Ptr &p) {
    functionMatcherThunks_ = p;
}

ThunkPredicates::Ptr
EngineBinary::functionSplittingThunks() const {
    return functionSplittingThunks_;
}

void
EngineBinary::functionSplittingThunks(const ThunkPredicates::Ptr &p) {
    functionSplittingThunks_ = p;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Python API support
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef ROSE_ENABLE_PYTHON_API

template<class T>
std::vector<T>
pythonListToVector(boost::python::list &list) {
    std::vector<T> retval;
    for (int i = 0; i < len(list); ++i)
        retval.push_back(boost::python::extract<T>(list[i]));
    return retval;
}

Partitioner::Ptr
EngineBinary::pythonParseVector(boost::python::list &pyArgs, const std::string &purpose, const std::string &description) {
    reset();
    std::vector<std::string> args = pythonListToVector<std::string>(pyArgs);
    std::vector<std::string> specimenNames = parseCommandLine(args, purpose, description).unreachedArgs();
    return partition(specimenNames);
}

Partitioner::Ptr
EngineBinary::pythonParseSingle(const std::string &specimen, const std::string &purpose, const std::string &description) {
    return partition(std::vector<std::string>(1, specimen));
}

#endif

} // namespace
} // namespace
} // namespace

#endif
