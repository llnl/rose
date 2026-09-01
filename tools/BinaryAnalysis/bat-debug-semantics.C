static const char *purpose = "decode and symbolically execute a single instruction";
static const char *description =
    "Given an architecture and bytes for one or more machine instructions on the command-line, decode the bytes and "
    "symbolically execute the resulting instructions, printing information about the entire process. The bytes are "
    "always specified as hexadecimal values, with or without a \"0x\" prefix.";

#include <rose.h>
#include <batSupport.h>

#include <Rose/BinaryAnalysis/AddressInterval.h>
#include <Rose/BinaryAnalysis/Architecture/Base.h>
#include <Rose/BinaryAnalysis/Disassembler/Base.h>
#include <Rose/BinaryAnalysis/InstructionSemantics/ConcreteSemantics.h>
#include <Rose/BinaryAnalysis/InstructionSemantics/IntervalSemantics.h>
#include <Rose/BinaryAnalysis/InstructionSemantics/NullSemantics.h>
#include <Rose/BinaryAnalysis/InstructionSemantics/PartialSymbolicSemantics.h>
#include <Rose/BinaryAnalysis/InstructionSemantics/SymbolicSemantics.h>
#include <Rose/BinaryAnalysis/InstructionSemantics/TraceSemantics.h>
#include <Rose/BinaryAnalysis/MemoryMap.h>
#include <Rose/BinaryAnalysis/Partitioner2/Partitioner.h>
#include <Rose/CommandLine.h>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem.hpp>

using namespace Rose;
using namespace Rose::BinaryAnalysis;
using namespace Rose::BinaryAnalysis::InstructionSemantics;
using namespace Sawyer::Message::Common;
namespace P2 = Rose::BinaryAnalysis::Partitioner2;
namespace IS = Rose::BinaryAnalysis::InstructionSemantics;

Sawyer::Message::Facility mlog;

// Settings parsed from command-line switches
struct Settings {
    Sawyer::Optional<std::string> valueClassName;
    Sawyer::Optional<std::string> rstateClassName;
    Sawyer::Optional<std::string> mstateClassName;
    std::string opsClassName = "symbolic";              // name of RiscOperators class, abbreviated
    bool useMemoryMap = false;
    SymbolicSemantics::DefinersMode computingDefiners = SymbolicSemantics::TRACK_NO_DEFINERS;
    SymbolicSemantics::WritersMode computingWriters = SymbolicSemantics::TRACK_NO_WRITERS;
};

// Parse command-line switches and return positional arguments, or exit failure.
static std::vector<std::string>
parseCommandLine(int argc, char *argv[], Settings &settings) {
    using namespace Sawyer::CommandLine;

    //----- Generic switches -----
    SwitchGroup genSwitches = Rose::CommandLine::genericSwitches();
    genSwitches.name("global");

    //----- Semantics class switches -----
    SwitchGroup sem("Semantics class switches");
    sem.name("class");
    sem.doc("These switches control which classes are used when constructing the instruction semantics framework.");

    sem.insert(Switch("semantics")
               .argument("class", anyParser(settings.opsClassName))
               .doc("Name of the class that represents the semantic RiscOperators.  This switch is required and provides "
                    "the default values for @s{value}, @s{rstate}, and @s{mstate}.  Use \"@s{semantics} list\" to get a "
                    "list of recognized names."));

    sem.insert(Switch("value")
               .argument("class", anyParser(settings.valueClassName))
               .doc("Name of the class that represents semantic values. If not specified, use the default value class for "
                    "the semantics class specified with @s{semantics}. Use \"@s{value} list\" to get a list of "
                    "recognized names."));

    sem.insert(Switch("rstate")
               .argument("class", anyParser(settings.rstateClassName))
               .doc("Name of the class that represents the register state.  If not specified, use the default register "
                    "class for the semantics class specified with @s{semantics}. Use \"@s{rstate} list\" to get a list "
                    "of recognized names."));

    sem.insert(Switch("mstate")
               .argument("class", anyParser(settings.mstateClassName))
               .doc("Name of the class that represents the memory state.  If not specified, use the default memory class "
                    "for the semantics class specified with @s{semantics}. Use \"@s{mstate} list\" to get a list "
                    "of recognized names."));

    //----- Semantics control switches -----
    SwitchGroup ctl("Semantics control switches");
    ctl.name("ctl");
    ctl.doc("These switches control various operational characteristics of the instruction semantics framework. The "
            "applicability of some of these switches depends on the classes used to construct the framework.");

    CommandLine::insertBooleanSwitch(ctl, "use-memory-map", settings.useMemoryMap,
                                     "Adds the specimen's memory map to the memory state and/or RISC operators if they support "
                                     "using a memory map. For most domains that allow this, it essentially initializes memory "
                                     "based on values loaded from the executable.");

    ctl.insert(Switch("track-definers")
               .argument("how", enumParser<SymbolicSemantics::DefinersMode>(settings.computingDefiners)
                         ->with("none", SymbolicSemantics::TRACK_NO_DEFINERS)
                         ->with("last", SymbolicSemantics::TRACK_LATEST_DEFINER)
                         ->with("all", SymbolicSemantics::TRACK_ALL_DEFINERS))
               .doc("Controls whether a domain that supports tracking value definers does so.  For instance, the "
                    "symbolic domain can keep a list of defining instructions for each symbolic value. Turning on any "
                    "form of definers also turns on the output of such definers.  The @v{how} can be one of these words: "
                    "@named{none}{Do not compute definers.}"
                    "@named{last}{Track only the latest defining instruction per value.}"
                    "@named{all}{Track all defining instructions per value.}"));

    ctl.insert(Switch("track-writers")
               .argument("how", enumParser<SymbolicSemantics::WritersMode>(settings.computingWriters)
                         ->with("none", SymbolicSemantics::TRACK_NO_WRITERS)
                         ->with("last", SymbolicSemantics::TRACK_LATEST_WRITER)
                         ->with("all", SymbolicSemantics::TRACK_ALL_WRITERS))
               .doc("Controls whether a domain that supports tracking writers does so.  This means that each location "
                    "(registers and memory) will have a list of instructions that wrote to that location. The allowed "
                    "arguments are:"
                    "@named{none}{Do not track writers.}"
                    "@named{last}{Track only the latest writing instruction per location.}"
                    "@named{all}{Track all writing instructions per location.}"));

    //----- Parse -----
    Parser parser = Rose::CommandLine::createEmptyParser(purpose, description);
    parser.errorStream(::mlog[FATAL]);
    parser.doc("Synopsis", "@prop{programName} [@v{switches}] @v{isa} @v{bytes}...");
    parser.with(sem);
    parser.with(ctl);
    parser.with(genSwitches);
    std::vector<std::string> args = parser.parse(argc, argv).apply().unreachedArgs();

    if (args.size() < 2) {
        ::mlog[FATAL] <<"incorrect usage; see --help\n";
        exit(1);
    }
    return args;
}

// The first argument is an optional starting address of the form "=N" where N is a hexadecimal value without the leading "0x"
// and the bytes are hexidecimal without the leading "0x".
template<class StringIterator>
static MemoryMap::Ptr
parseBytes(StringIterator string, StringIterator end) {
    std::vector<uint8_t> bytes;
    Sawyer::Optional<Address> va;

    for (/*void*/; string != end; ++string) {
        if (!string->empty() && '=' == (*string)[0]) {
            if (va) {
                ::mlog[FATAL] <<"cannot parse \"" <<StringUtility::cEscape(*string) <<"\" because only one address can be specified\n";
                exit(1);
            } else if (auto addr = StringUtility::toNumber<Address>(string->substr(1))) {
                va = addr.ok();
            } else {
                ::mlog[FATAL] <<"cannot parse \"" <<StringUtility::cEscape(string->substr(1)) <<"\""
                              <<" as an address: " <<addr.unwrapError() <<"\n";
                exit(1);
            }
        } else if (auto byte = StringUtility::toNumber<uint8_t>((boost::starts_with(*string, "0x") ? "" : "0x") + *string)) {
            bytes.push_back(*byte);
        } else {
            ::mlog[FATAL] <<"cannot parse \"" <<StringUtility::cEscape(*string) <<"\" as byte value: " <<byte.unwrapError() <<"\n";
        }
    }

    std::reverse(bytes.begin(), bytes.end());

    auto map = MemoryMap::instance();
    map->insert(AddressInterval::baseSize(va.orElse(0), bytes.size()),
                MemoryMap::Segment(MemoryMap::AllocatingBuffer::instance(bytes.size()),
                                   0, MemoryMap::READ_EXECUTE, "instructions"));
    map->at(va.orElse(0)).write(bytes);
    return map;
}

static SmtSolver::Ptr
makeSolver(const Settings&) {
    return SmtSolver::instance(Rose::CommandLine::genericSwitchArgs.smtSolver);
}

static BaseSemantics::SValue::Ptr
makeProtoVal(const Settings &settings) {
    const std::string className = settings.valueClassName.orElse(settings.opsClassName);

    if (className == "list") {
        std::cout <<"value class names:\n"
                  <<"  concrete         Rose::BinaryAnalysis::InstructionSemantics::ConcreteSemantics::SValue\n"
                  <<"  interval         Rose::BinaryAnalysis::InstructionSemantics::IntervalSemantics::SValue\n"
                  <<"  null             Rose::BinaryAnalysis::InstructionSemantics::NullSemantics::SValue\n"
                  <<"  partial          Rose::BinaryAnalysis::InstructionSemantics::PartialSymbolicSemantics::SValue\n"
                  <<"  partitioner2     Rose::BinaryAnalysis::Partitioner2::Semantics::SValue\n"
                  <<"  symbolic         Rose::BinaryAnalysis::InstructionSemantics::SymbolicSemantics::SValue\n";
        return BaseSemantics::SValue::Ptr();
    } else if (className == "concrete") {
        return ConcreteSemantics::SValue::instance();
    } else if (className == "interval") {
        return IntervalSemantics::SValue::instance();
    } else if (className == "null") {
        return NullSemantics::SValue::instance();
    } else if (className == "partial") {
        return PartialSymbolicSemantics::SValue::instance();
    } else if (className == "partitioner2") {
        return P2::Semantics::SValue::instance();
    } else if (className == "symbolic") {
        return SymbolicSemantics::SValue::instance();
    } else {
        throw std::runtime_error("unrecognized value class \"" + className + "\"; see --value=list\n");
    }
}

static BaseSemantics::RegisterState::Ptr
makeRegisterState(const Settings &settings, const BaseSemantics::SValue::Ptr &protoval, const RegisterDictionary::Ptr &regdict) {
    const std::string className = settings.rstateClassName.orElse(settings.opsClassName);

    if (className == "list") {
        std::cout <<"register state class names:\n"
                  <<"  concrete         Rose::BinaryAnalysis::InstructionSemantics::BaseSemantics::RegisterStateGeneric\n"
                  <<"  generic          Rose::BinaryAnalysis::InstructionSemantics::BaseSemantics::RegisterStateGeneric\n"
                  <<"  interval         Rose::BinaryAnalysis::InstructionSemantics::BaseSemantics::RegisterStateGeneric\n"
                  <<"  null             Rose::BinaryAnalysis::InstructionSemantics::NullSemantics::RegisterState\n"
                  <<"  partial          Rose::BinaryAnalysis::InstructionSemantics::BaseSemantics::RegisterStateGeneric\n"
                  <<"  partitioner2     Rose::BinaryAnalysis::Partitioner2::Semantics::RegisterState\n"
                  <<"  symbolic         Rose::BinaryAnalysis::InstructionSemantics::BaseSemantics::RegisterStateGeneric\n";
        return BaseSemantics::RegisterState::Ptr();
    } else if (className == "concrete") {
        return BaseSemantics::RegisterStateGeneric::instance(protoval, regdict);
    } else if (className == "generic") {
        return BaseSemantics::RegisterStateGeneric::instance(protoval, regdict);
    } else if (className == "interval") {
        return BaseSemantics::RegisterStateGeneric::instance(protoval, regdict);
    } else if (className == "null") {
        return NullSemantics::RegisterState::instance(protoval, regdict);
    } else if (className == "partial") {
        return BaseSemantics::RegisterStateGeneric::instance(protoval, regdict);
    } else if (className == "partitioner2") {
        return P2::Semantics::RegisterState::instance(protoval, regdict);
    } else if (className == "symbolic") {
        return BaseSemantics::RegisterStateGeneric::instance(protoval, regdict);
    } else {
        throw std::runtime_error("unrecognized register state class name \"" + className + "\"; see --rstate=list\n");
    }
}

static BaseSemantics::MemoryState::Ptr
makeMemoryState(const Settings &settings, const P2::Partitioner::ConstPtr &partitioner, const BaseSemantics::SValue::Ptr &protoval,
                const BaseSemantics::SValue::Ptr &protoaddr, const RegisterDictionary::Ptr &regdict) {
    const std::string className = settings.mstateClassName.orElse(settings.opsClassName);
    if (className == "list") {
        std::cout <<"memory state class names:\n"
                  <<"  concrete         Rose::BinaryAnalysis::InstructionSemantics::ConcreteSemantics::MemoryState\n"
                  <<"  interval         Rose::BinaryAnalysis::InstructionSemantics::IntervalSemantics::MemoryState\n"
                  <<"  null             Rose::BinaryAnalysis::InstructionSemantics::NullSemantics::MemoryState\n"
                  <<"  partial          Rose::BinaryAnalysis::InstructionSemantics::PartialSymbolicSemantics default\n"
                  <<"  p2-list          Rose::BinaryAnalysis::Partitioner2::Semantics::MemoryListState\n"
                  <<"  p2-map           Rose::BinaryAnalysis::Partitioner2::Semantics::MemoryMapState\n"
                  <<"  symbolic-list    Rose::BinaryAnalysis::InstructionSemantics::SymbolicSemantics::MemoryListState\n"
                  <<"  symbolic-map     Rose::BinaryAnalysis::InstructionSemantics::SymbolicSemantics::MemoryMapState\n";
        return BaseSemantics::MemoryState::Ptr();
    } else if (className == "concrete") {
        return ConcreteSemantics::MemoryState::instance(protoval, protoaddr);
    } else if (className == "interval") {
        return IntervalSemantics::MemoryState::instance(protoval, protoaddr);
    } else if (className == "null") {
        return NullSemantics::MemoryState::instance(protoval, protoaddr);
    } else if (className == "partial") {
        BaseSemantics::RiscOperators::Ptr ops = PartialSymbolicSemantics::RiscOperators::instanceFromRegisters(regdict);
        return ops->currentState()->memoryState();
    } else if (className == "p2-list" || className == "partitioner2") {
        P2::Semantics::MemoryListState::Ptr m = P2::Semantics::MemoryListState::instance(protoval, protoaddr);
        m->memoryMap(partitioner->memoryMap()->shallowCopy());
        return m;
    } else if (className == "p2-map") {
        P2::Semantics::MemoryMapState::Ptr m = P2::Semantics::MemoryMapState::instance(protoval, protoaddr);
        m->memoryMap(partitioner->memoryMap()->shallowCopy());
        return m;
    } else if (className == "symbolic-list" || className == "symbolic") {
        return SymbolicSemantics::MemoryListState::instance(protoval, protoaddr);
    } else if (className == "symbolic-map") {
        return SymbolicSemantics::MemoryMapState::instance(protoval, protoaddr);
    } else {
        throw std::runtime_error("unrecognized memory state class name \"" + className + "\"; see --mstate=list\n");
    }
}

static BaseSemantics::RiscOperators::Ptr
makeRiscOperators(const Settings &settings, const P2::Partitioner::ConstPtr &partitioner) {
    const std::string &className = settings.opsClassName;
    if ("list" == className) {
        std::cout <<"semantic class names:\n"
                  <<"  concrete         Rose::BinaryAnalysis::InstructionSemantics::ConcreteSemantics::RiscOperators\n"
                  <<"  interval         Rose::BinaryAnalysis::InstructionSemantics::IntervalSemantics::RiscOperators\n"
                  <<"  null             Rose::BinaryAnalysis::InstructionSemantics::NullSemantics::RiscOperators\n"
                  <<"  partial          Rose::BinaryAnalysis::InstructionSemantics::PartialSymbolicSemantics::RiscOperators\n"
                  <<"  partitioner2     Rose::BinaryAnalysis::Partitioner2::Semantics::RiscOperators\n"
                  <<"  symbolic         Rose::BinaryAnalysis::InstructionSemantics::SymbolicSemantics::RiscOperators\n";
        exit(0);
    }

    Architecture::Base::ConstPtr arch = partitioner->architecture();
    SmtSolver::Ptr solver = makeSolver(settings);
    RegisterDictionary::Ptr regdict = arch->registerDictionary();
    BaseSemantics::SValue::Ptr protoval = makeProtoVal(settings);
    BaseSemantics::RegisterState::Ptr rstate = makeRegisterState(settings, protoval, regdict);
    BaseSemantics::MemoryState::Ptr mstate = makeMemoryState(settings, partitioner, protoval, protoval, regdict);

    BaseSemantics::RegisterState::Ptr istate = makeRegisterState(settings, protoval, arch->interruptDictionary());
    istate->purpose(BaseSemantics::AddressSpace::Purpose::INTERRUPTS);

    BaseSemantics::State::Ptr state = BaseSemantics::State::instance(rstate, mstate, istate);

    if (className == "concrete") {
        return ConcreteSemantics::RiscOperators::instanceFromState(state, solver);
    } else if (className == "interval") {
        return IntervalSemantics::RiscOperators::instanceFromState(state, solver);
    } else if (className == "null") {
        return NullSemantics::RiscOperators::instanceFromState(state, solver);
    } else if (className == "partial") {
        PartialSymbolicSemantics::RiscOperators::Ptr ops = PartialSymbolicSemantics::RiscOperators::instanceFromState(state, solver);
        if (settings.useMemoryMap)
            ops->set_memory_map(partitioner->memoryMap()->shallowCopy());
        return ops;
    } else if (className == "partitioner2") {
        return P2::Semantics::RiscOperators::instance(state, solver);
    } else if (className == "symbolic") {
        SymbolicSemantics::RiscOperators::Ptr ops = SymbolicSemantics::RiscOperators::instanceFromState(state, solver);
        ops->computingDefiners(settings.computingDefiners);
        ops->computingRegisterWriters(settings.computingWriters);
        ops->computingMemoryWriters(settings.computingWriters);
        return ops;
    } else {
        throw std::runtime_error("unrecognized semantic class name \"" + className + "\"; see --semantics=list\n");
    }
}

static void
printAst(std::ostream &out, SgAsmInstruction *insn, const std::string &prefix) {
    struct Visitor: AstPrePostProcessing {
        std::ostream &out;
        size_t depth = 0;
        const std::string &prefix;

        Visitor(std::ostream &out, const std::string &prefix)
            : out(out), prefix(prefix) {}

        void preOrderVisit(SgNode *node) override {
            out <<prefix <<"|";
            for (size_t i = 0; i < depth; ++i)
                out <<"   |";
            ++depth;
            out <<node->class_name();
            if (auto insn = isSgAsmInstruction(node))
                out <<" " <<insn->toString();
            if (auto rre = isSgAsmRegisterReferenceExpression(node))
                out <<" " <<rre->get_descriptor().toString();
            if (auto ive = isSgAsmIntegerValueExpression(node))
                out <<" " <<StringUtility::toHex2(ive->get_absoluteValue(), ive->get_significantBits());
            out <<"\n";
        }

        void postOrderVisit(SgNode*) override {
            --depth;
        }
    };
    Visitor(out, prefix).traverse(insn);
}

int
main(int argc, char *argv[]) {
    ROSE_INITIALIZE;
    Diagnostics::initAndRegister(&::mlog, "tool");
    ::mlog.comment("debugging semantics");
    Bat::checkRoseVersionNumber(MINIMUM_ROSE_LIBRARY_VERSION, ::mlog[FATAL]);
    Bat::registerSelfTests();

    // Parse command-line
    Settings settings;
    std::vector<std::string> args = parseCommandLine(argc, argv, settings);
    ASSERT_require(args.size() >= 2);
    std::string isa = args[0];
    MemoryMap::Ptr memory = parseBytes(args.begin()+1, args.end());

    // Create the decoder and semantics
    auto architecture = Architecture::findByName(isa).orThrow();
    P2::Partitioner::Ptr partitioner = P2::Partitioner::instance(architecture, memory);

    auto innerOps = makeRiscOperators(settings, partitioner);
    auto ops = IS::TraceSemantics::RiscOperators::instance(innerOps);
    IS::BaseSemantics::Dispatcher::Ptr cpu = partitioner->newDispatcher(ops);

    auto state = innerOps->currentState();

    // Push a new method frame
    auto frame = IS::BaseSemantics::FrameState::instance(state->protoval(), Sawyer::Nothing(), ByteCode::Method::Ptr());
    state->pushFrame(frame);

    // Decode and process the instruction
    size_t va = memory->hull().least();
    while (SgAsmInstruction *insn = partitioner->instructionProvider()[va]) {
        std::cerr <<partitioner->unparse(insn) <<"\n";
        printAst(std::cout, insn, "");
        if (cpu) {
            cpu->processInstruction(insn);
            std::cerr <<*ops->currentState();
        }
        va += insn->get_size();
    }
}
