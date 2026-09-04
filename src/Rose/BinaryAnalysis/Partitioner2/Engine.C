#include <featureTests.h>
#ifdef ROSE_ENABLE_BINARY_ANALYSIS
#include <Rose/BinaryAnalysis/Partitioner2/Engine.h>

// Concrete engine pre-registered as factories
#include <Rose/BinaryAnalysis/Partitioner2/EngineBinary.h>
#include <Rose/BinaryAnalysis/Partitioner2/EngineJvm.h>

#include <Rose/AST/Traversal.h>
#include <Rose/BinaryAnalysis/Architecture/Base.h>
#include <Rose/BinaryAnalysis/BinaryLoader.h>
#include <Rose/BinaryAnalysis/Disassembler.h>
#include <Rose/BinaryAnalysis/Partitioner2/BasicBlock.h>
#include <Rose/BinaryAnalysis/Partitioner2/Configuration.h>
#include <Rose/BinaryAnalysis/Partitioner2/DataBlock.h>
#include <Rose/BinaryAnalysis/Partitioner2/Modules.h>
#include <Rose/BinaryAnalysis/Partitioner2/ModulesElf.h>
#include <Rose/BinaryAnalysis/Partitioner2/ModulesPe.h>
#include <Rose/BinaryAnalysis/Partitioner2/Partitioner.h>
#include <Rose/CommandLine.h>
#include <Rose/Initialize.h>

#include <SgAsmBlock.h>
#include <SgAsmFunction.h>
#include <SgAsmGenericFile.h>
#include <SgAsmInstruction.h>
#include <SgAsmIntegerValueExpression.h>
#include <SgAsmInterpretation.h>

#include <Cxx_GrammarDowncast.h>

#include <Sawyer/FileSystem.h>
#include <Sawyer/GraphAlgorithm.h>
#include <Sawyer/Set.h>

#include <boost/range/adaptor/reversed.hpp>

using namespace Rose::Diagnostics;

namespace Rose {
namespace BinaryAnalysis {
namespace Partitioner2 {

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Static utility functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef std::pair<Address, size_t> AddressOrder;

static bool
isSecondZero(const AddressOrder &a) {
    return 0 == a.second;
}

static bool
sortBySecond(const AddressOrder &a, const AddressOrder &b) {
    return a.second < b.second;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Engine::Settings
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Engine::Settings::~Settings() {}

Engine::Settings::Settings() {}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Engine::Exception
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Engine::Exception::~Exception() {}

Engine::Exception::Exception(const std::string &mesg)
    : Partitioner2::Exception(mesg) {}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Engine::PositionalArgumentParser and subclasses
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Engine::PositionalArgumentParser::~PositionalArgumentParser() {}

Engine::PositionalArgumentParser::PositionalArgumentParser() {}

//------------------------------------------------------------------------------------------------------------------------
std::vector<std::string>
Engine::AllPositionalArguments::specimen(const std::vector<std::string> &args) const {
    return args;
}

//------------------------------------------------------------------------------------------------------------------------
Engine::FirstPositionalArguments::FirstPositionalArguments(size_t n)
    : n_(n) {}

std::vector<std::string>
Engine::FirstPositionalArguments::specimen(const std::vector<std::string> &args) const {
    std::vector<std::string> retval;
    for (size_t i = 0; i < std::min(args.size(), n_); ++i)
        retval.push_back(args[i]);
    return retval;
}

//------------------------------------------------------------------------------------------------------------------------
Engine::AllButLastArguments::AllButLastArguments(size_t n)
    : n_(n) {}

std::vector<std::string>
Engine::AllButLastArguments::specimen(const std::vector<std::string> &args) const {
    std::vector<std::string> retval;
    for (size_t i = 0; i + n_ < args.size(); ++i)
        retval.push_back(args[i]);
    return retval;
}

//------------------------------------------------------------------------------------------------------------------------
Engine::GroupedPositionalArguments::GroupedPositionalArguments()
    : n_(1) {}

Engine::GroupedPositionalArguments::GroupedPositionalArguments(size_t n)
    : n_(n) {}

std::vector<std::string>
Engine::GroupedPositionalArguments::specimen(const std::vector<std::string> &args) const {
    std::vector<std::string> retval;
    for (const std::string &arg: args) {
        if (retval.size() < n_ && arg != "--") {
            retval.push_back(arg);
        } else {
            break;
        }
    }
    return retval;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Engine::BasicBlockFinalizer
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Engine::BasicBlockFinalizer::~BasicBlockFinalizer() {}

Engine::BasicBlockFinalizer::BasicBlockFinalizer() {}

Engine::BasicBlockFinalizer::Ptr
Engine::BasicBlockFinalizer::instance() {
    return Ptr(new BasicBlockFinalizer);
}

// Called every time an instructionis added to a basic block.
bool
Engine::BasicBlockFinalizer::operator()(bool chain, const Args &args) {
    if (chain) {
        BasicBlock::Ptr bb = args.bblock;
        ASSERT_not_null(bb);
        ASSERT_require(bb->nInstructions() > 0);

        fixFunctionReturnEdge(args);
        fixFunctionCallEdges(args);
        addPossibleIndeterminateEdge(args);
    }
    return chain;
}

// If the block is a function return (e.g., ends with an x86 RET instruction) to an indeterminate location, then that successor
// type should be E_FUNCTION_RETURN instead of E_NORMAL.
void
Engine::BasicBlockFinalizer::fixFunctionReturnEdge(const Args &args) {
    if (args.partitioner->basicBlockIsFunctionReturn(args.bblock)) {
        bool hadCorrectEdge = false, edgeModified = false;
        BasicBlock::Successors successors = args.partitioner->basicBlockSuccessors(args.bblock);
        for (size_t i = 0; i < successors.size(); ++i) {
            if (!successors[i].expr()->isConcrete() ||
                (successors[i].expr()->get_expression()->flags() & SymbolicExpression::Node::INDETERMINATE) != 0) {
                if (successors[i].type() == E_FUNCTION_RETURN) {
                    hadCorrectEdge = true;
                    break;
                } else if (successors[i].type() == E_NORMAL && !edgeModified) {
                    successors[i].type(E_FUNCTION_RETURN);
                    edgeModified = true;
                }
            }
        }
        if (!hadCorrectEdge && edgeModified) {
            args.bblock->clearSuccessors();
            args.bblock->successors(successors);
            SAWYER_MESG(mlog[DEBUG]) <<args.bblock->printableName() <<": fixed function return edge type\n";
        }
    }
}

// If the block is a function call (e.g., ends with an x86 CALL instruction) then change all E_NORMAL edges to E_FUNCTION_CALL
// edges.
void
Engine::BasicBlockFinalizer::fixFunctionCallEdges(const Args &args) {
    if (args.partitioner->basicBlockIsFunctionCall(args.bblock)) {
        BasicBlock::Successors successors = args.partitioner->basicBlockSuccessors(args.bblock);
        bool changed = false;
        for (BasicBlock::Successor &successor: successors) {
            if (successor.type() == E_NORMAL) {
                successor.type(E_FUNCTION_CALL);
                changed = true;
            }
        }
        if (changed) {
            args.bblock->clearSuccessors();
            args.bblock->successors(successors);
            SAWYER_MESG(mlog[DEBUG]) <<args.bblock->printableName() <<": fixed function call edge(s) type\n";
        }
    }
}

// Should we add an indeterminate CFG edge from this basic block?  For instance, a "JMP [ADDR]" instruction should get an
// indeterminate edge if ADDR is a writable region of memory. There are two situations: ADDR is non-writable, in which case
// RiscOperators::peekMemory would have returned a free variable to indicate an indeterminate value, or ADDR is writable but
// its MemoryMap::INITIALIZED bit is set to indicate it has a valid value already, in which case RiscOperators::peekMemory
// would have returned the value stored there but also marked the value as being INDETERMINATE.  The
// SymbolicExpression::TreeNode::INDETERMINATE bit in the expression should have been carried along so that things like "MOV
// EAX, [ADDR]; JMP EAX" will behave the same as "JMP [ADDR]".
void
Engine::BasicBlockFinalizer::addPossibleIndeterminateEdge(const Args &args) {
    BasicBlockSemantics sem = args.bblock->semantics();
    if (sem.finalState() == nullptr)
        return;
    ASSERT_not_null(sem.operators);

    bool addIndeterminateEdge = false;
    size_t addrWidth = 0;
    for (const BasicBlock::Successor &successor: args.partitioner->basicBlockSuccessors(args.bblock)) {
        if (!successor.expr()->isConcrete()) {          // BB already has an indeterminate successor?
            addIndeterminateEdge = false;
            break;
        } else if (!addIndeterminateEdge &&
                   (successor.expr()->get_expression()->flags() & SymbolicExpression::Node::INDETERMINATE) != 0) {
            addIndeterminateEdge = true;
            addrWidth = successor.expr()->nBits();
        }
    }

    // Add an edge
    if (addIndeterminateEdge) {
        ASSERT_require(addrWidth != 0);
        BaseSemantics::SValue::Ptr addr = sem.operators->undefined_(addrWidth);
        EdgeType type = args.partitioner->basicBlockIsFunctionReturn(args.bblock) ? E_FUNCTION_RETURN : E_NORMAL;
        args.bblock->insertSuccessor(addr, type);
        SAWYER_MESG(mlog[DEBUG]) <<args.bblock->printableName()
                                 <<": added indeterminate successor for initialized, non-constant memory read\n";
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Engine::BasicBlockWorkList
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Engine::BasicBlockWorkList::~BasicBlockWorkList() {}

Engine::BasicBlockWorkList::BasicBlockWorkList(const Engine::Ptr &engine, size_t maxSorts)
    : engine_(engine), maxSorts_(maxSorts) {}

Engine::BasicBlockWorkList::Ptr
Engine::BasicBlockWorkList::instance(const Engine::Ptr &engine, size_t maxSorts) {
    return Ptr(new BasicBlockWorkList(engine, maxSorts));
}

// Add basic block to worklist(s)
bool
Engine::BasicBlockWorkList::operator()(bool chain, const AttachedBasicBlock &args) {
    if (chain) {
        ASSERT_not_null(args.partitioner);
        Partitioner::ConstPtr partitioner = args.partitioner;

        // Basic block that is not yet discovered. We could use the special "undiscovered" CFG vertex, but there is no ordering
        // guarantee for its incoming edges.  We want to process undiscovered vertices in a depth-first manner, which is why we
        // maintain our own list instead.  The reason for depth-first discovery is that some analyses are recursive in nature
        // and we want to try to have children discovered and analyzed before we try to analyze the parent.  For instance,
        // may-return analysis for one vertex probably depends on the may-return analysis of its successors.
        if (args.bblock == nullptr) {
            undiscovered_.pushBack(args.startVa);
            return chain;
        }

        // If a new function call is inserted and it has no E_CALL_RETURN edge and at least one of its callees has an
        // indeterminate value for its may-return analysis, then add this block to the list of blocks for which we may need to
        // later add a call-return edge. The engine can be configured to also just assume that all function calls may return
        // (or never return).
        if (partitioner->basicBlockIsFunctionCall(args.bblock)) {
            ControlFlowGraph::ConstVertexIterator placeholder = partitioner->findPlaceholder(args.startVa);
            boost::logic::tribool mayReturn;
            switch (engine_->settings().partitioner.functionReturnAnalysis) {
                case MAYRETURN_ALWAYS_YES:
                    mayReturn = true;
                    break;
                case MAYRETURN_ALWAYS_NO:
                    mayReturn = false;
                    break;
                case MAYRETURN_DEFAULT_YES:
                case MAYRETURN_DEFAULT_NO: {
                    ASSERT_require(placeholder != partitioner->cfg().vertices().end());
                    mayReturn = hasAnyCalleeReturn(partitioner, placeholder);
                    break;
                }
            }
            if (!hasCallReturnEdges(placeholder) && (mayReturn || boost::logic::indeterminate(mayReturn)))
                pendingCallReturn_.pushBack(args.startVa);
        }
    }
    return chain;
}

// Remove basic block from worklist(s). We probably don't really need to erase things since the items are revalidated when
// they're consumed from the list, and avoiding an erase will keep the list in its original order when higher level functions
// make a minor adjustment with a detach-adjust-attach sequence.  On the other hand, perhaps such adjustments SHOULD move the
// block to the top of the stack.
bool
Engine::BasicBlockWorkList::operator()(bool chain, const DetachedBasicBlock &args) {
    if (chain) {
        pendingCallReturn_.erase(args.startVa);
        processedCallReturn_.erase(args.startVa);
        finalCallReturn_.erase(args.startVa);
        undiscovered_.erase(args.startVa);
    }
    return chain;
}

Sawyer::Container::DistinctList<Address>&
Engine::BasicBlockWorkList::pendingCallReturn() {
    return pendingCallReturn_;
}

Sawyer::Container::DistinctList<Address>&
Engine::BasicBlockWorkList::processedCallReturn() {
    return processedCallReturn_;
}

Sawyer::Container::DistinctList<Address>&
Engine::BasicBlockWorkList::finalCallReturn() {
    return finalCallReturn_;
}

Sawyer::Container::DistinctList<Address>&
Engine::BasicBlockWorkList::undiscovered() {
    return undiscovered_;
}

// Move pendingCallReturn items into the finalCallReturn list and (re)sort finalCallReturn items according to the CFG so that
// descendents appear after their ancestors (i.e., descendents will be processed first since we always use popBack).  This is a
// fairly expensive operation: O((V+E) ln N) where V and E are the number of edges in the CFG and N is the number of addresses
// in the combined lists.
void
Engine::BasicBlockWorkList::moveAndSortCallReturn(const Partitioner::ConstPtr &partitioner) {
    using namespace Sawyer::Container::Algorithm;       // graph traversals

    if (processedCallReturn().isEmpty())
        return;                                         // nothing to move, and finalCallReturn list was previously sorted

    if (maxSorts_ == 0) {
        for (Address va: processedCallReturn_.items())
            finalCallReturn().pushBack(va);
        processedCallReturn_.clear();

    } else {
        if (0 == --maxSorts_)
            mlog[WARN] <<"may-return sort limit reached; reverting to unsorted analysis\n";

        // Get the list of virtual addresses that need to be processed
        std::vector<AddressOrder> pending;
        pending.reserve(finalCallReturn_.size() + processedCallReturn_.size());
        for (Address va: finalCallReturn_.items())
            pending.push_back(AddressOrder(va, (size_t)0));
        for (Address va: processedCallReturn_.items())
            pending.push_back(AddressOrder(va, (size_t)0));
        finalCallReturn_.clear();
        processedCallReturn_.clear();

        // Find the CFG vertex for each pending address and insert its "order" value. Blocks that are leaves (after arbitrarily
        // breaking cycles) have lower numbers than blocks higher up in the global CFG.
        std::vector<size_t> order = graphDependentOrder(partitioner->cfg());
        for (AddressOrder &pair: pending) {
            ControlFlowGraph::ConstVertexIterator vertex = partitioner->findPlaceholder(pair.first);
            if (vertex != partitioner->cfg().vertices().end() && vertex->value().type() == V_BASIC_BLOCK) {
                pair.second = order[vertex->id()];
            }
        }

        // Sort the pending addresses based on their calculated "order", skipping those that aren't CFG basic blocks, and save
        // the result.
        pending.erase(std::remove_if(pending.begin(), pending.end(), isSecondZero), pending.end());
        std::sort(pending.begin(), pending.end(), sortBySecond);
        for (const AddressOrder &pair: pending)
            finalCallReturn().pushBack(pair.first);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Engine::CodeConstants
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Engine::CodeConstants::~CodeConstants() {}

Engine::CodeConstants::CodeConstants()
    : inProgress_(0) {}

Engine::CodeConstants::Ptr
Engine::CodeConstants::instance() {
    return Ptr(new CodeConstants);
}

Address
Engine::CodeConstants::inProgress() {
    return inProgress_;
}

// Add new basic block's instructions to list of instruction addresses to process
bool
Engine::CodeConstants::operator()(bool chain, const AttachedBasicBlock &attached) {
    if (chain && attached.bblock) {
        for (SgAsmInstruction *insn: attached.bblock->instructions()) {
            if (wasExamined_.find(insn->get_address()) == wasExamined_.end())
                toBeExamined_.insert(insn->get_address());
        }
    }
    return chain;
}

// Remove basic block's instructions from list of instructions to process
bool
Engine::CodeConstants::operator()(bool chain, const DetachedBasicBlock &detached) {
    if (chain && detached.bblock) {
        for (SgAsmInstruction *insn: detached.bblock->instructions()) {
            toBeExamined_.erase(insn->get_address());
            if (insn->get_address() == inProgress_)
                constants_.clear();
        }
    }
    return chain;
}

// Return the next constant from the next instruction
Sawyer::Optional<Address>
Engine::CodeConstants::nextConstant(const Partitioner::ConstPtr &partitioner) {
    if (!constants_.empty()) {
        Address constant = constants_.back();
        constants_.pop_back();
        return constant;
    }

    while (!toBeExamined_.empty()) {
        inProgress_ = *toBeExamined_.begin();
        toBeExamined_.erase(inProgress_);
        if (SgAsmInstruction *insn = partitioner->instructionExists(inProgress_).insn()) {

            std::set<Address> constants;
            AST::Traversal::forwardPre<SgAsmIntegerValueExpression>(insn, [&constants](SgAsmIntegerValueExpression *ival) {
                if (ival->get_significantBits() <= 64)
                    constants.insert(ival->get_absoluteValue());
            });
            constants_ = std::vector<Address>(constants.begin(), constants.end());
        }

        if (!constants_.empty()) {
            Address constant = constants_.back();
            constants_.pop_back();
            return constant;
        }
    }

    return Sawyer::Nothing();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Engine construction
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Engine::Engine(const std::string &name, const Settings &settings)
    : name_{name}, settings_{settings}, interp_{nullptr}, progress_{Progress::instance()} {
    init();
}

Engine::~Engine() {}

// [Robb Matzke 2023-03-03]: deprecated
// class method
EngineBinary::Ptr
Engine::instance() {
    return EngineBinary::instance();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Factories
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static SAWYER_THREAD_TRAITS::Mutex registryMutex;
static std::vector<Engine::Ptr> registry;

static void
initRegistryHelper() {
    SAWYER_THREAD_TRAITS::LockGuard lock(registryMutex);
    registry.push_back(EngineBinary::factory());
    registry.push_back(EngineJvm::factory());
}

static void
initRegistry() {
    static boost::once_flag registryInitFlag = BOOST_ONCE_INIT;
    boost::call_once(&initRegistryHelper, registryInitFlag);
}

void
Engine::registerFactory(const Engine::Ptr &factory) {
    ASSERT_not_null(factory);
    ASSERT_require(factory->isFactory());
    initRegistry();
    SAWYER_THREAD_TRAITS::LockGuard lock(registryMutex);
    registry.push_back(factory);
}

bool
Engine::deregisterFactory(const Ptr &factory) {
    ASSERT_not_null(factory);
    ASSERT_require(factory->isFactory());
    SAWYER_THREAD_TRAITS::LockGuard lock(registryMutex);
    for (auto iter = registry.rbegin(); iter != registry.rend(); ++iter) {
        if (*iter == factory) {
            registry.erase(std::next(iter).base());
            return true;
        }
    }
    return false;
}

std::vector<Engine::Ptr>
Engine::registeredFactories() {
    initRegistry();
    std::vector<Ptr> retval;
    SAWYER_THREAD_TRAITS::LockGuard lock(registryMutex);
    retval.reserve(registry.size());
    for (const Ptr &factory: registry)
        retval.push_back(factory);
    return retval;
}

bool
Engine::isFactory() const {
    return specimen_.empty();
}

//------------------------------------------------------------------------------------------------------------------
// forge methods that operate on a list of specimens
//------------------------------------------------------------------------------------------------------------------

Engine::Ptr
Engine::forge(const std::vector<std::string> &specimen) {
    const auto factories = registeredFactories();
    for (const Engine::Ptr &factory: boost::adaptors::reverse(factories)) {
        if (factory->matchFactory(Sawyer::CommandLine::ParserResult{}, specimen)) {
            Engine::Ptr engine = factory->instanceFromFactory(Settings());
            engine->specimen(specimen);
            return engine;
        }
    }
    return {};
}

Engine::Ptr
Engine::forge(const std::string &specimen) {
  return forge(std::vector<std::string>{specimen});
}

//------------------------------------------------------------------------------------------------------------------
// forge methods that operate on a command-line represented as std::vector<std::string>
//------------------------------------------------------------------------------------------------------------------

// This is the base implementation that takes all arguments
Engine::Ptr
Engine::forge(const std::vector<std::string> &args, Sawyer::CommandLine::Parser &switchParser,
              const PositionalArgumentParser &positionalParser, const Settings &settings) {
    const auto factories = registeredFactories();
    for (const Engine::Ptr &factory: boost::adaptors::reverse(factories)) {
        // Figure out what arguments describe the specimen
        auto tempSwitchParser = switchParser;
        factory->addAllToParser(tempSwitchParser);
        auto parserResult = tempSwitchParser.parse(args);
        std::vector<std::string> specimen = positionalParser.specimen(parserResult.unreachedArgs());

        // If the factory can handle the situation, use it to create a new engine and add the engine's command-line parsing to the
        // supplied switch parser. Save the specimen in the engine, although it can be adjusted after the return.
        if (factory->matchFactory(parserResult, specimen)) {
            Engine::Ptr engine = factory->instanceFromFactory(settings);
            engine->addAllToParser(switchParser);
            engine->specimen(specimen);
            return engine;
        }
    }
    return {};
}

// Default settings
Engine::Ptr
Engine::forge(const std::vector<std::string> &args, Sawyer::CommandLine::Parser &switchParser,
              const PositionalArgumentParser &positionalParser) {
    return forge(args, switchParser, positionalParser, Settings());
}
// Default positional argument parser
Engine::Ptr
Engine::forge(const std::vector<std::string> &args, Sawyer::CommandLine::Parser &switchParser, const Settings &settings) {
    return forge(args, switchParser, AllPositionalArguments(), settings);
}

// Default settings and positional argument parser
Engine::Ptr
Engine::forge(const std::vector<std::string> &args, Sawyer::CommandLine::Parser &switchParser) {
    return forge(args, switchParser, AllPositionalArguments(), Settings());
}

//------------------------------------------------------------------------------------------------------------------
// forge methods that operate on a command-line represented as int and char*[]
//------------------------------------------------------------------------------------------------------------------

// Base implementation
Engine::Ptr
Engine::forge(int argc, char *argv[], Sawyer::CommandLine::Parser &switchParser, const PositionalArgumentParser &positionalParser,
              const Settings &settings) {
    std::vector<std::string> args(argv+1, argv+argc);
    return forge(args, switchParser, positionalParser, settings);
}

// Default settings
Engine::Ptr
Engine::forge(int argc, char *argv[], Sawyer::CommandLine::Parser &switchParser, const PositionalArgumentParser &positionalParser) {
    return forge(argc, argv, switchParser, positionalParser, Settings());
}

// Default positional argument parser
Engine::Ptr
Engine::forge(int argc, char *argv[], Sawyer::CommandLine::Parser &switchParser, const Settings &settings) {
    return forge(argc, argv, switchParser, AllPositionalArguments(), settings);
}

// Default settings and positional argument parser
Engine::Ptr
Engine::forge(int argc, char *argv[], Sawyer::CommandLine::Parser &switchParser) {
    return forge(argc, argv, switchParser, AllPositionalArguments(), Settings());
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Construction helper functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void
Engine::init() {
    ASSERT_require(map_ == nullptr);
    Rose::initialize(nullptr);
    basicBlockWorkList_ = BasicBlockWorkList::instance(sharedFromThis(), settings().partitioner.functionReturnAnalysisMaxSorts);
#if ROSE_PARTITIONER_EXPENSIVE_CHECKS == 1
    static bool emitted = false;
    if (!emitted) {
        emitted = true;
        mlog[WARN] <<"ROSE_PARTITIONER_EXPENSIVE_CHECKS is enabled\n";
    }
#endif
}

void
Engine::reset() {
    interpretation(nullptr);
    architecture_ = Architecture::Base::ConstPtr();
    map_ = MemoryMap::Ptr();
    basicBlockWorkList_ = BasicBlockWorkList::instance(sharedFromThis(), settings().partitioner.functionReturnAnalysisMaxSorts);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Top-level, do everything functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SgAsmBlock*
Engine::frontend(int argc, char *argv[], const std::string &purpose, const std::string &description) {
    std::vector<std::string> args;
    for (int i=1; i<argc; ++i)
        args.push_back(argv[i]);
    return frontend(args, purpose, description);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Command-line parsing
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::list<Sawyer::CommandLine::SwitchGroup>
Engine::commandLineSwitches() {
    return {};
}

std::list<Sawyer::CommandLine::SwitchGroup>
Engine::allCommandLineSwitches() {
    std::list<Sawyer::CommandLine::SwitchGroup> retval;

    // Gather switch groups for `this`. It's important to grab these because they're linked to this object (parsing those switches
    // updates this object's settings).
    {
        const auto list = commandLineSwitches();
        retval.insert(retval.end(), list.begin(), list.end());
    }

    // Gather all switch groups for factories. It's okay if the factory has switch groups that are duplicates of this object's
    // switch groups because we'll erase the duplicates next.
    const auto factories = registeredFactories();
    for (const Engine::Ptr &factory: boost::adaptors::reverse(factories)) {
        const auto list = factory->commandLineSwitches();
        retval.insert(retval.end(), list.begin(), list.end());
    }

    // Erase duplicates, keeping only the first occurrance of each switch group.
    Sawyer::Container::Set<std::string> titles, names;
    for (auto iter = retval.begin(); iter != retval.end(); /*void*/) {
        if (titles.exists(iter->title()) || names.exists(iter->name())) {
            iter = retval.erase(iter);
        } else {
            titles.insert(iter->title());
            names.insert(iter->name());
            ++iter;
        }
    }

    return retval;
}

std::list<std::pair<std::string, std::string>>
Engine::allSpecimenNameDocumentation() {
    using List = std::list<std::pair<std::string, std::string>>;

    // Accumulate all documentation
    List retval;
    const auto factories = registeredFactories();
    for (const Engine::Ptr &factory: boost::adaptors::reverse(factories))
        retval.push_back(factory->specimenNameDocumentation());

    // Combine duplicate sections
    std::map<std::string, List::iterator> titles;
    for (List::iterator iter = retval.begin(); iter != retval.end(); /*void*/) {
        auto have = titles.find(iter->first);
        if (have == titles.end()) {
            titles[iter->first] = iter;
            ++iter;
        } else {
            have->second->second += "\n\n" + iter->second;
            iter = retval.erase(iter);
        }
    }
    return retval;
}

void
Engine::addToParser(Sawyer::CommandLine::Parser &parser) {
    for (const Sawyer::CommandLine::SwitchGroup &sg: commandLineSwitches())
        parser.with(sg);

    std::pair<std::string, std::string> pair = specimenNameDocumentation();
    parser.doc(pair.first, pair.second);
}

void
Engine::addAllToParser(Sawyer::CommandLine::Parser &parser) {
    for (const Sawyer::CommandLine::SwitchGroup &sg: allCommandLineSwitches())
        parser.with(sg);

    std::string doc;
    for (const std::pair<std::string, std::string> &pair: allSpecimenNameDocumentation())
        doc += "@named{" + pair.first + "}{" + pair.second + "}";
    parser.doc("How to name specimens", doc);
}

Sawyer::CommandLine::Parser
Engine::commandLineParser(const std::string &purpose, const std::string &description) {
    using namespace Sawyer::CommandLine;
    Parser parser =
        CommandLine::createEmptyParser(purpose.empty() ? std::string("analyze binary specimen") : purpose, description);
    parser.groupNameSeparator("-");                     // ROSE defaults to ":", which is sort of ugly
    parser.doc("Synopsis", "@prop{programName} [@v{switches}] @v{specimen_names}");
    addAllToParser(parser);
    return parser;
}

Sawyer::CommandLine::ParserResult
Engine::parseCommandLine(int argc, char *argv[], const std::string &purpose, const std::string &description) {
    try {
        std::vector<std::string> args;
        for (int i=1; i<argc; ++i)
            args.push_back(argv[i]);
        return parseCommandLine(args, purpose, description);
    } catch (const std::runtime_error &e) {
        if (settings().engine.exitOnError) {
            mlog[FATAL] <<e.what() <<"\n";
            exit(1);
        } else {
            throw;
        }
    }
}

Sawyer::CommandLine::ParserResult
Engine::parseCommandLine(const std::vector<std::string> &args, const std::string &purpose, const std::string &description) {
    return commandLineParser(purpose, description)
        .with(Rose::CommandLine::genericSwitches())
        .parse(args)
        .apply();
}

void
Engine::checkSettings() {
    if (!architecture_ && !settings().disassembler.isaName.empty())
        architecture_ = obtainArchitecture();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Binary container parsing
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool
Engine::isRbaFile(const std::string &name) {
    return boost::ends_with(name, ".rba");
}

SgAsmInterpretation*
Engine::parseContainers(const std::string &fileName) {
    return parseContainers(std::vector<std::string>(1, fileName));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Memory map creation (loading)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool
Engine::areSpecimensLoaded() const {
    return map_!=nullptr && !map_->isEmpty();
}

void
Engine::adjustMemoryMap() {
    ASSERT_not_null(map_);
    if (settings().loader.memoryIsExecutable)
        map_->any().changeAccess(MemoryMap::EXECUTABLE, 0);
    Modules::deExecuteZeros(map_/*in,out*/, settings().loader.deExecuteZerosThreshold,
                            settings().loader.deExecuteZerosLeaveAtFront, settings().loader.deExecuteZerosLeaveAtBack);

    switch (settings().loader.memoryDataAdjustment) {
        case DATA_IS_CONSTANT:
            map_->any().changeAccess(0, MemoryMap::WRITABLE);
            break;
        case DATA_IS_INITIALIZED:
            map_->any().changeAccess(MemoryMap::INITIALIZED, 0);
            break;
        case DATA_NO_CHANGE:
            break;
    }
}

MemoryMap::Ptr
Engine::memoryMap() const {
    return map_;
}

void
Engine::memoryMap(const MemoryMap::Ptr &m) {
    map_ = m;
}

MemoryMap::Ptr
Engine::loadSpecimens(const std::string &fileName) {
    return loadSpecimens(std::vector<std::string>(1, fileName));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Architecture stuff
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Architecture::Base::ConstPtr
Engine::architecture() {
    if (!architecture_) {
        obtainArchitecture();
    }
    ASSERT_not_null(architecture_);
    return architecture_;
}

Architecture::Base::ConstPtr
Engine::obtainArchitecture() {
    return obtainArchitecture(Architecture::Base::ConstPtr());
}

Architecture::Base::ConstPtr
Engine::obtainArchitecture(const Architecture::Base::ConstPtr &hint) {
    if (!architecture_ && !settings().disassembler.isaName.empty())
        architecture_ = Architecture::findByName(settings().disassembler.isaName).orThrow();

    if (!architecture_ && interpretation())
        architecture_ = Architecture::findByInterpretation(interpretation()).orThrow();

    if (!architecture_ && hint)
        architecture_ = hint;

    if (!architecture_)
        throw Architecture::NotFound("no architecture found and none specified");

    return architecture_;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Partitioner high-level functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void
Engine::checkCreatePartitionerPrerequisites() const {
    if (!map_ || map_->isEmpty())
        mlog[WARN] <<"Engine::createBarePartitioner: using an empty memory map\n";
}

Partitioner::Ptr
Engine::createBarePartitioner() {
    Sawyer::Message::Stream info(mlog[MARCH]);

    checkCreatePartitionerPrerequisites();
    auto partitioner = Partitioner::instance(architecture(), memoryMap());
    if (partitioner->memoryMap() && partitioner->memoryMap()->byteOrder() == ByteOrder::ORDER_UNSPECIFIED)
        partitioner->memoryMap()->byteOrder(architecture()->byteOrder());
    partitioner->settings(settings().partitioner.base);
    partitioner->progress(progress());

    // Load configuration files
    if (!settings().engine.configurationNames.empty()) {
        Sawyer::Stopwatch timer;
        info <<"loading configuration files";
        for (const std::string &configName: settings().engine.configurationNames)
            partitioner->configuration().loadFromFile(configName);
        info <<"; took " <<timer <<"\n";
    }

    // Build the may-return blacklist and/or whitelist.  This could be made specific to the type of interpretation being
    // processed, but there's so few functions that we'll just plop them all into the lists.
//TODO: Not for JVM?
    ModulesPe::buildMayReturnLists(partitioner);
    ModulesElf::buildMayReturnLists(partitioner);

    // Make sure the basicBlockWorkList gets updated when the partitioner's CFG is adjusted.
    ASSERT_not_null(basicBlockWorkList());
    partitioner->cfgAdjustmentCallbacks().prepend(basicBlockWorkList());

    // Make sure the stream of constants found in instruction ASTs is updated whenever the CFG is adjusted.
    if (settings().partitioner.findingCodeFunctionPointers) {
        codeFunctionPointers(CodeConstants::instance());
        partitioner->cfgAdjustmentCallbacks().prepend(codeFunctionPointers());
    }

    // Perform some finalization whenever a basic block is created.  For instance, this figures out whether we should add an
    // extra indeterminate edge for indirect jump instructions that go through initialized but writable memory.
    partitioner->basicBlockCallbacks().append(BasicBlockFinalizer::instance());

    // If the may-return analysis is run and cannot decide whether a function may return, should we assume that it may or
    // cannot return?  The engine decides whether to actually invoke the analysis -- this just sets what to do if it's
    // invoked.
    switch (settings().partitioner.functionReturnAnalysis) {
        case MAYRETURN_ALWAYS_YES:
        case MAYRETURN_DEFAULT_YES:
            partitioner->assumeFunctionsReturn(true);
            break;
        case MAYRETURN_ALWAYS_NO:
        case MAYRETURN_DEFAULT_NO:
            partitioner->assumeFunctionsReturn(false);
    }

    // Should the partitioner favor list-based or map-based containers for semantic memory states?
    partitioner->semanticMemoryParadigm(settings().partitioner.semanticMemoryParadigm);

    // Miscellaneous settings
    if (!settings().partitioner.ipRewrites.empty()) {
        std::vector<Modules::IpRewriter::AddressPair> rewrites;
        for (size_t i = 0; i+1 < settings().partitioner.ipRewrites.size(); i += 2)
            rewrites.push_back(std::make_pair(settings().partitioner.ipRewrites[i+0], settings().partitioner.ipRewrites[i+1]));
        partitioner->basicBlockCallbacks().append(Modules::IpRewriter::instance(rewrites));
    }
    if (settings().partitioner.followingGhostEdges)
        partitioner->basicBlockCallbacks().append(Modules::AddGhostSuccessors::instance());
    if (!settings().partitioner.discontiguousBlocks)
        partitioner->basicBlockCallbacks().append(Modules::PreventDiscontiguousBlocks::instance());
    if (settings().partitioner.maxBasicBlockSize > 0)
        partitioner->basicBlockCallbacks().append(Modules::BasicBlockSizeLimiter::instance(settings().partitioner.maxBasicBlockSize));

    // PEScrambler descrambler
    if (settings().partitioner.peScramblerDispatcherVa) {
        ModulesPe::PeDescrambler::Ptr cb = ModulesPe::PeDescrambler::instance(settings().partitioner.peScramblerDispatcherVa);
        cb->nameKeyAddresses(partitioner);              // give names to certain PEScrambler things
        partitioner->basicBlockCallbacks().append(cb);
        partitioner->attachFunction(Function::instance(settings().partitioner.peScramblerDispatcherVa,
                                                       partitioner->addressName(settings().partitioner.peScramblerDispatcherVa),
                                                       SgAsmFunction::FUNC_PESCRAMBLER_DISPATCH));
    }

    return partitioner;
}

void
Engine::runPartitioner(const Partitioner::Ptr &partitioner) {
    ASSERT_not_null(partitioner);
    Sawyer::Message::Stream info(mlog[INFO]);
    Sawyer::Stopwatch timer;
    info <<"disassembling and partitioning";
    runPartitionerInit(partitioner);
    runPartitionerRecursive(partitioner);
    runPartitionerFinal(partitioner);
    info <<"; took " <<timer <<"\n";

    if (settings().partitioner.doingPostAnalysis)
        updateAnalysisResults(partitioner);

    // Make sure solver statistics are accumulated into the class
    if (SmtSolverPtr solver = partitioner->smtSolver())
        solver->resetStatistics();
}

Partitioner::Ptr
Engine::partition(const std::string &fileName) {
    return partition(std::vector<std::string>(1, fileName));
}

void
Engine::savePartitioner(const Partitioner::ConstPtr &partitioner, const boost::filesystem::path &name,
                        Serialization::Format fmt) {
    partitioner->saveAsRbaFile(name, fmt);
}

Partitioner::Ptr
Engine::loadPartitioner(const boost::filesystem::path &name, Serialization::Format fmt) {
    Partitioner::Ptr partitioner = Partitioner::instanceFromRbaFile(name, fmt);
    interpretation(partitioner->interpretation());
    return partitioner;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Partitioner mid-level operations
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void
Engine::labelAddresses(const Partitioner::Ptr &partitioner, const Configuration &configuration) {
    ASSERT_not_null(partitioner);
    Modules::labelSymbolAddresses(partitioner, interpretation());

    for (const AddressConfiguration &c: configuration.addresses().values()) {
        if (!c.name().empty())
            partitioner->addressName(c.address(), c.name());
    }
}

std::vector<DataBlock::Ptr>
Engine::makeConfiguredDataBlocks(const Partitioner::Ptr &partitioner, const Configuration &configuration) {
    // FIXME[Robb P. Matzke 2015-05-12]: This just adds labels to addresses right now.
    ASSERT_not_null(partitioner);
    for (const DataBlockConfiguration &dconfig: configuration.dataBlocks().values()) {
        if (!dconfig.name().empty())
            partitioner->addressName(dconfig.address(), dconfig.name());
    }
    return std::vector<DataBlock::Ptr>();
}

std::vector<Function::Ptr>
Engine::makeConfiguredFunctions(const Partitioner::Ptr &partitioner, const Configuration &configuration) {
    ASSERT_not_null(partitioner);
    std::vector<Function::Ptr> retval;
    for (const FunctionConfiguration &fconfig: configuration.functionConfigurationsByAddress().values()) {
        Address entryVa = 0;
        if (fconfig.address().assignTo(entryVa)) {
            Function::Ptr function = Function::instance(entryVa, fconfig.name(), SgAsmFunction::FUNC_CONFIGURED);
            function->comment(fconfig.comment());
            insertUnique(retval, partitioner->attachOrMergeFunction(function), sortFunctionsByAddress);
        }
    }
    return retval;
}

void
Engine::updateAnalysisResults(const Partitioner::Ptr &partitioner) {
    ASSERT_not_null(partitioner);
    Sawyer::Message::Stream info(mlog[INFO]);
    Sawyer::Stopwatch timer;
    info <<"post partition analysis";
    std::string separator = ": ";

    if (settings().partitioner.doingPostFunctionNoop) {
        info <<separator <<"func-no-op";
        separator = ", ";
        Modules::nameNoopFunctions(partitioner);
    }

    if (settings().partitioner.doingPostFunctionMayReturn) {
        info <<separator <<"may-return";
        separator = ", ";
        partitioner->allFunctionMayReturn();
    }

    if (settings().partitioner.doingPostFunctionStackDelta) {
        info <<separator <<"stack-delta";
        separator = ", ";
        partitioner->allFunctionStackDelta();
    }

    if (settings().partitioner.doingPostCallingConvention) {
        info <<separator <<"call-conv";
        separator = ", ";
        // Calling convention analysis uses a default convention to break recursion cycles in the CG.
        const CallingConvention::Dictionary &ccDict = partitioner->instructionProvider().callingConventions();
        CallingConvention::Definition::Ptr dfltCcDef;
        if (!ccDict.empty())
            dfltCcDef = ccDict[0];
        partitioner->allFunctionCallingConventionDefinition(dfltCcDef);
    }

    info <<"; total " <<timer <<"\n";
}

// class method called by ROSE's ::frontend to disassemble instructions.
void
Engine::disassembleForRoseFrontend(SgAsmInterpretation *interp) {
    ASSERT_not_null(interp);
    ASSERT_require(interp->get_globalBlock() == nullptr);

    if (interp->get_map() == nullptr) {
        mlog[WARN] <<"no virtual memory to disassemble for";
        for (SgAsmGenericFile *file: interp->get_files())
            mlog[WARN] <<" \"" <<StringUtility::cEscape(file->get_name()) <<"\"";
        mlog[WARN] <<"\n";
        return;
    }

    //TODO: Warning, using EngineBinary explicitly. Base engine instance on interpretation instead?
    Ptr engine = EngineBinary::instance();

    engine->memoryMap(interp->get_map()->shallowCopy()); // copied so we can make local changes
    engine->adjustMemoryMap();
    engine->interpretation(interp);

    if (SgAsmBlock *gblock = engine->buildAst()) {
        interp->set_globalBlock(gblock);
        interp->set_map(engine->memoryMap());
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Partitioner low-level stuff
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Build AST
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SgAsmBlock*
Engine::buildAst(const std::string &fileName) {
    return buildAst(std::vector<std::string>{fileName});
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Settings and Properties
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Progress::Ptr
Engine::progress() const {
    return progress_;
}

void
Engine::progress(const Progress::Ptr &progress) {
    progress_ = progress;
}

const std::string&
Engine::name() const /*final*/ {
    return name_;
}

void
Engine::name(const std::string &n) {
    name_ = n;
}

const Engine::Settings&
Engine::settings() const /*final*/ {
    return settings_;
}

Engine::Settings&
Engine::settings() /*final*/ {
    return settings_;
}

void
Engine::settings(const Settings &s) {
    settings_ = s;
}

Engine::BasicBlockWorkList::Ptr
Engine::basicBlockWorkList() const /*final*/ {
    return basicBlockWorkList_;
}

void
Engine::basicBlockWorkList(const BasicBlockWorkList::Ptr &l) /*final*/ {
    basicBlockWorkList_ = l;
}

Engine::CodeConstants::Ptr
Engine::codeFunctionPointers() const /*final*/ {
    return codeFunctionPointers_;
}

void
Engine::codeFunctionPointers(const CodeConstants::Ptr &c) /*final*/ {
    codeFunctionPointers_ = c;
}

SgAsmInterpretation*
Engine::interpretation() const /*final*/ {
    return interp_;
}

void
Engine::interpretation(SgAsmInterpretation *interp) {
    interp_ = interp;
}

const std::vector<std::string>&
Engine::specimen() const /*final*/ {
    return specimen_;
}

void
Engine::specimen(const std::vector<std::string> &specimen) {
    specimen_ = specimen;
}

InstructionSemantics::BaseSemantics::StatePtr
Engine::state() {
    return state_;
}

ByteCode::Class::Ptr
Engine::analysisClass() {
    ASSERT_not_null(analysisClass_);
    return analysisClass_;
}

Engine::ClassRepository
Engine::classes() const {
    return classes_;
}

ByteCode::Class::Ptr
Engine::classByName(const std::string &name) const {
    auto found = classes_.find(name);
    ASSERT_require2(found != classes_.end(), "class not found in repository");
    return found->second;
}

} // namespace
} // namespace
} // namespace

#endif
