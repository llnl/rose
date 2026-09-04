#ifndef ROSE_BinaryAnalysis_Partitioner2_Engine_H
#define ROSE_BinaryAnalysis_Partitioner2_Engine_H
#include <featureTests.h>
#ifdef ROSE_ENABLE_BINARY_ANALYSIS

#include <Rose/BasicTypes.h>
#include <Rose/BinaryAnalysis/Architecture/BasicTypes.h>
#include <Rose/BinaryAnalysis/ByteCode/Analysis.h>
#include <Rose/BinaryAnalysis/Partitioner2/Exception.h>
#include <Rose/BinaryAnalysis/Partitioner2/Modules.h>

#include <Sawyer/DistinctList.h>
#include <Sawyer/SharedObject.h>
#include <Sawyer/SharedPointer.h>

namespace Rose {
namespace BinaryAnalysis {
namespace Partitioner2 {

/** Base class for engines driving the partitioner.
 *
 * The @ref Partitioner2::Engine is an abstract base class whose derived classes implement the algorithms for creating functions,
 * basic blocks, instructions, control flow graphs (CFGs), address usage maps (AUMs), static data blocks, and everything else that
 * can be stored in a @ref Partitioner2::Partitioner "Partitioner".
 *
 * Although subclasses can be instantiated directly, it is often easier to do so through the base class's factory methods. For
 * instance, a tool that operates on various kinds of specimens would want to decide what engine subclass to instantiate based on
 * the types of specimens that are being analyzed. The ROSE library defines a few sublcasses, and the factory mechanism allows
 * external tools to register their own subclasses.
 *
 * @section Rose_BinaryAnalysis_Partitioner2_Engine_factories Factories
 *
 * Using the engine factories is a two-step process: first the subclasses are registered with the ROSE library, then the ROSE
 * library chooses which engine should be instantiated and does so.
 *
 * A subclass is registered by instantiating a "factory instance" of the subclass and calling @ref registerFactory, which takes
 * shared ownership of the factory instance. Additional member functions such as @ref deregisterFactory and @ref registeredFactories
 * can be used to manipulate the factory list. The engine types that are built into ROSE are registered automatically and appear at
 * the beginning of the list.
 *
 * To directly instantiate an instance of a subclass without going through the factory mechanism, one calls the static @c instance
 * method of the subclass. The @c instance method allocates a new instance in the heap and returns a shared-ownership pointer to the
 * new instance.  This is how almost all binary analysis works in ROSE.
 *
 * On the other hand, to instantiate an engine using the factory method, one calls the @ref Engine::forge static member
 * function. The words "forge" and "factory" both start with "f" as a reminder that they go together in this API. The @ref forge
 * method will scan the list of registered factories from the end toward the beginning and the first factory whose @ref matchFactory
 * predicate returns true will be the type of engine to be used. The engine actually returned is not the registered factory
 * directly, but the result of calling @ref instanceFromFactory on the registered factory.
 *
 * @section Rose_BinaryAnalysis_Partitioner2_Engine_commandlineparsing Command-line parsing
 *
 * Every engine instance has a @ref settings property. Currently this is shared by all engine types, but in the future we may make
 * this more extensible. Every engine supports ROSE's command-line parsing (@ref Sawyer::CommandLine) to modify its settings from
 * the command-line. There is a chicken-or-egg dilemma with command-line parsing: in order to choose the correct engine type, we
 * need to find the command-line positional arguments that describe the specimen; in order to find the positional arguments, we need
 * to be able to accurately parse the command-line switches; in order to parse the engine-specific command-line switches, we need to
 * know which engine will be used for the parsing.
 *
 * The solution that ROSE uses isn't perfect, but works well in practice. Each engine type is able to register its command-line
 * switches as a switch group within a command-line parser. As long as the switches don't conflict with each other, or can be
 * disambiguated to not conflict, we're able to parse the command-line switches as the union of all the switches from all known
 * engine types. Switches can be disambiguated by giving each engine type its own switch group name (i.e., optional switch
 * prefix). For example, the JVM engine might use "--[jvm-]foo" while the binary machine engine uses "--[binary-]foo".
 *
 * In order to choose a matching factory from the list of factories, ROSE constructs a temporary command-line parser that handles
 * the switches from all known engines and binds the parser to the registered factories (i.e., the presence of a switch modifies
 * the settings in the factory instance). ROSE then parses the command-line to find the specimen positional arguments which it uses
 * to choose a factory instance. Once the factory instance is chosen, ROSE instantiates a regular instance from the factory and
 * constructs a new command-line parser with just that engine's switches, and bound to the engine being returned. Later, when the
 * tool parses the command-line for real and applies the parse result, the returned engine's settings are updated.
 *
 * Although this mechanism for handling engine-specific command-line switches is not perfect, we think it will provide a means by
 * which tools can define their own command-line switches, and other factories besides @ref Partitioner2::Engine can operate in
 * similar ways.
 *
 * @section Rose_BinaryAnalysis_Partitioner2_Engine_basic Basic usage
 *
 * An engine drives the process of building a @ref Partitioner2::Partitioner which is then used as an efficient means of obtaining
 * information that's needed during analysis. Although every engine can operate in a different way, there's a common API that's
 * intended to be quite general. The engine API is divided into abstraction layers. The most abstract layer consists of functions
 * that do the most things in one step, from parsing of command-lines to generation of the @ref Partitioner2::Partitioner
 * "Partitioner" and/or abstract syntax tree (AST).  The next layer of less abstract functions does smaller parts, such as parsing
 * containers like ELF or PE, loading parts of specimen files into memory, finding related specimens like shared libraries, linking
 * specimens together into a common address space, organizing instructions into basic blocks and functions, creating an AST, etc.
 * Finally, at the least level of abstraction, many of the functions are specific enough to be defined only in subclasses.
 *
 * Here's an example of basic usage of the engine to instantiate and populate a @ref Partitioner2::Partitioner "Partitioner" that
 * can be used for analysis. First, the tool needs to define the necessary classes, which it can do by including either the
 * top-level "Partitioner2.h" header, or the headers for the individual Partitioner2 types.
 *
 * @code
 *   #include <rose.h>
 *   #include <Rose/BinaryAnalysis/Partitioner2.h>
 *   using namespace Rose;
 *   namespace P2 = Rose::BinaryAnalysis::Partitioner2;
 * @endcode
 *
 * Most tools, after initializing the ROSE library, will create a command-line parser to parse the tool's own switches.
 *
 * @code
 *   int main(int argc, char *argv[]) {
 *       ROSE_INITIALIZE;
 *       std::string purpose = "disassembles a binary specimen";
 *       std::string description =
 *           "This tool disassembles the specified specimen and presents the "
 *           "results as a pseudo assembly listing, that is, a listing intended "
 *           "for human consumption rather than assembly.";
 *
 *       ToolSettings settings;
 *       Sawyer::CommandLine::Parser parser = buildSwitchParser(settings);
 * @endcode
 *
 * In order for the tool to choose the correct engine, it needs to be able to find the positional command-line arguments that
 * describe the specimen. The tool could do all its own command-line parsing and adjust the values in a @ref Engine::Settings object
 * which it ultimately copies into an engine instance, or it could use ROSE's command-line parsing mechanism. We'll show the latter
 * since we already started creating such a parser above.
 *
 * Since we're using ROSE to parse the command-line, we pass the command-line and our partial command-line parser to the @ref
 * Partitioner2::Engine::forge "forge" method like this:
 *
 * @code
 *       P2::Engine::Ptr engine = P2::Engine::forge(argc, argv, parser);
 * @endcode
 *
 * When the forge call returns, it also adjusts the parser so it knows how to parse the command-line switches that are specific to
 * the returned engine type, and those switches are bound to the settings in that returned engine instance. Thus the tool is now
 * able to parse the command-line, update the settings in the engine, and obtain the positional arguments that describe the
 * specimen.
 *
 * @code
 *       std::vector<std::string> specimen = parser.parse(argc, argv).apply().unreachedArgs();
 * @endcode
 *
 * Finally, now that the engine has been obtained, the tool can cause the engine to produce a fully initialized @ref Partitioner, or
 * do any of the other things that an engine is designed to do.
 *
 * @code
 *       P2::Partitioner::Ptr partitioner = engine.partition(specimen);
 * @endcode
 *
 * On the other hand, if you perform your own command-line parser then you'll have the engine settings and the specimen arguments
 * already parsed, in which case you can create the engine with fewer steps:
 *
 * @code
 *  int main(int argc, char *argv[]) {
 *      P2::Engine::Settings settings;
 *      std::vector<std::string> specimen = parseCommandLine(settings);
 *      auto engine = P2::Engine::forge(specimen);
 *      auto partitioner = engine->partition(specimen);
 * @endcode
 *
 * @section Rose_BinaryAnalysis_Partitioner2_Engine_sa See also
 *
 * See also @ref Partitioner2::EngineBinary, @ref Partitioner2::EngineJvm, and the documentation for non-ROSE engines. */
class Engine: public Sawyer::SharedObject, public Sawyer::SharedFromThis<Engine> {
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                  Internal data structures
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
    /** Shared ownership pointer. */
    using Ptr = EnginePtr;

    //--------------------------------------------------------------------------------------------------------------------------
public:
    /** Settings for the engine.
     *
     *  The engine is configured by adjusting these settings, usually shortly after the engine is created. */
    struct Settings {
        LoaderSettings loader;                          /**< Settings used during specimen loading. */
        DisassemblerSettings disassembler;              /**< Settings for creating the disassembler. */
        PartitionerSettings partitioner;                /**< Settings for creating a partitioner. */
        IndirectControlFlow::Settings icf;              /**< Settings for indirect control flow recovery. */
        EngineSettings engine;                          /**< Settings that control engine behavior. */
        JvmSettings engineJvm;                          /**< Settings that control behavior specific to EngineJvm. */
        AstConstructionSettings astConstruction;        /**< Settings for constructing the AST. */

#ifdef ROSE_ENABLE_BOOST_SERIALIZATION
    private:
        friend class boost::serialization::access;
        template<class S> void serialize(S&, unsigned version);
#endif

    public:
        Settings();
        ~Settings();
    };

    //--------------------------------------------------------------------------------------------------------------------------
public:
    /** Errors from the engine. */
    class Exception: public Partitioner2::Exception {
    public:
        // WARNING: Defined in Engine.C with different behavior
        // ~Exception() throw();
        ~Exception();

        /** Construct an exception with a message string. */
        explicit Exception(const std::string&);
    };

    //--------------------------------------------------------------------------------------------------------------------------
public:
    /** How to parse positional command-line arguments. */
    class PositionalArgumentParser {
    public:
        virtual ~PositionalArgumentParser();
        PositionalArgumentParser();

        /** Return specimen from positional arguments.
         *
         *  Given a list of positional arguments from the command-line (i.e., the stuff that comes after the switches),
         *  return those positional arguments that represent the specimen and which are used to choose the engine subclass.
         *
         *  The default implementation in the @c Engine base class just returns all input arguments. */
        virtual std::vector<std::string> specimen(const std::vector<std::string>&) const = 0;
    };

    /** Return all positional arguments as the specimen. */
    class AllPositionalArguments: public PositionalArgumentParser {
    public:
        virtual std::vector<std::string> specimen(const std::vector<std::string>&) const override;
    };

    /** Up to first N arguments are the specimen. */
    class FirstPositionalArguments: public PositionalArgumentParser {
        size_t n_;
    public:
        /** Constructor returning up to @p n arguments. */
        explicit FirstPositionalArguments(size_t n);
        virtual std::vector<std::string> specimen(const std::vector<std::string>&) const override;
    };

    /** All but the last N arguments are the specimen. */
    class AllButLastArguments: public PositionalArgumentParser {
        size_t n_;
    public:
        /** Constructor returning all but last @p n arguments. */
        explicit AllButLastArguments(size_t n);
        virtual std::vector<std::string> specimen(const std::vector<std::string>&) const override;
    };

    /** Nth group of arguments are the specimen. */
    class GroupedPositionalArguments: public PositionalArgumentParser {
        size_t n_ = 0;
    public:
        /** Constructor returning first group of arguments. */
        GroupedPositionalArguments();
        /** Constructor returning nth group of arguments. */
        explicit GroupedPositionalArguments(size_t);
        virtual std::vector<std::string> specimen(const std::vector<std::string>&) const override;
    };

    //--------------------------------------------------------------------------------------------------------------------------
protected:
    // Engine callback for handling instructions added to basic blocks.  This is called when a basic block is discovered,
    // before it's attached to a partitioner, so it shouldn't really be modifying any state in the engine, but rather only
    // preparing the basic block to be processed.
    class BasicBlockFinalizer: public BasicBlockCallback {
        typedef Sawyer::Container::Map<Address /*target*/, std::vector<Address> /*sources*/> WorkList;
    public:
        ~BasicBlockFinalizer();
    protected:
        BasicBlockFinalizer();
    public:
        static Ptr instance();
        virtual bool operator()(bool chain, const Args &args) override;
    private:
        void fixFunctionReturnEdge(const Args&);
        void fixFunctionCallEdges(const Args&);
        void addPossibleIndeterminateEdge(const Args&);
    };

    //--------------------------------------------------------------------------------------------------------------------------
private:
    // Basic blocks that need to be worked on next. These lists are adjusted whenever a new basic block (or placeholder) is
    // inserted or erased from the CFG.
    class BasicBlockWorkList: public CfgAdjustmentCallback {
        // The following lists are used for adding outgoing E_CALL_RETURN edges to basic blocks based on whether the basic
        // block is a call to a function that might return.  When a new basic block is inserted into the CFG (or a previous
        // block is removed, modified, and re-inserted), the operator() is called and conditionally inserts the block into the
        // "pendingCallReturn" list (if the block is a function call that lacks an E_CALL_RETURN edge and the function is known
        // to return or the analysis was incomplete).
        //
        // When we run out of other ways to create basic blocks, we process the pendingCallReturn list from back to front. If
        // the back block (which gets popped) has a positive may-return result then an E_CALL_RETURN edge is added to the CFG
        // and the normal recursive BB discovery is resumed. Otherwise if the analysis is incomplete the basic block is moved
        // to the processedCallReturn list.  The entire pendingCallReturn list is processed before proceeding.
        //
        // If there is no more pendingCallReturn work to be done, then the processedCallReturn blocks are moved to the
        // finalCallReturn list and finalCallReturn is sorted by approximate CFG height (i.e., leafs first). The contents
        // of the finalCallReturn list is then analyzed and the result (or the default may-return value for failed analyses)
        // is used to decide whether a new CFG edge should be created, possibly adding new basic block addresses to the
        // list of undiscovered blocks.
        //
        Sawyer::Container::DistinctList<Address> pendingCallReturn_;   // blocks that might need an E_CALL_RETURN edge
        Sawyer::Container::DistinctList<Address> processedCallReturn_; // call sites whose may-return was indeterminate
        Sawyer::Container::DistinctList<Address> finalCallReturn_;     // indeterminate call sites awaiting final analysis

        Sawyer::Container::DistinctList<Address> undiscovered_;        // undiscovered basic block list (last-in-first-out)
        EnginePtr engine_;                                             // engine to which this callback belongs
        size_t maxSorts_;                                              // max sorts before using unsorted lists
    public:
        ~BasicBlockWorkList();
    protected:
        BasicBlockWorkList(const EnginePtr &engine, size_t maxSorts);
    public:
        using Ptr = Sawyer::SharedPointer<BasicBlockWorkList>;
        static Ptr instance(const EnginePtr &engine, size_t maxSorts);
        virtual bool operator()(bool chain, const AttachedBasicBlock &args) override;
        virtual bool operator()(bool chain, const DetachedBasicBlock &args) override;
        Sawyer::Container::DistinctList<Address>& pendingCallReturn();
        Sawyer::Container::DistinctList<Address>& processedCallReturn();
        Sawyer::Container::DistinctList<Address>& finalCallReturn();
        Sawyer::Container::DistinctList<Address>& undiscovered();
        void moveAndSortCallReturn(const PartitionerConstPtr&);
    };

    //--------------------------------------------------------------------------------------------------------------------------
protected:
    // A work list providing constants from instructions that are part of the CFG.
    class CodeConstants: public CfgAdjustmentCallback {
    public:
        using Ptr = Sawyer::SharedPointer<CodeConstants>;

    private:
        std::set<Address> toBeExamined_;                // instructions waiting to be examined
        std::set<Address> wasExamined_;                 // instructions we've already examined
        Address inProgress_;                            // instruction that is currently in progress
        std::vector<Address> constants_;                // constants for the instruction in progress

    public:
        ~CodeConstants();
    protected:
        CodeConstants();

    public:
        static Ptr instance();

        // Address of instruction being examined.
        Address inProgress();

        // Possibly insert more instructions into the work list when a basic block is added to the CFG
        virtual bool operator()(bool chain, const AttachedBasicBlock &attached) override;

        // Possibly remove instructions from the worklist when a basic block is removed from the CFG
        virtual bool operator()(bool chain, const DetachedBasicBlock &detached) override;

        // Return the next available constant if any.
        Sawyer::Optional<Address> nextConstant(const PartitionerConstPtr &partitioner);
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                  Data members
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
using ClassRepository = std::map<std::string, ByteCode::Class::Ptr>;

private:
    std::string name_;                                  // factory name
    Settings settings_;                                 // Settings for the partitioner.
    SgAsmInterpretation *interp_;                       // interpretation set by loadSpecimen
    MemoryMapPtr map_;                                  // memory map initialized by load()
    BasicBlockWorkList::Ptr basicBlockWorkList_;        // what blocks to work on next
    CodeConstants::Ptr codeFunctionPointers_;           // generates constants that are found in instruction ASTs
    ProgressPtr progress_;                              // optional progress reporting
    std::vector<std::string> specimen_;                 // list of additional command line arguments (often file names)

protected:
    ByteCode::Class::Ptr analysisClass_;                // the ByteCode analysis class used for partitioning
    Architecture::BaseConstPtr architecture_;           // architecture-specific information
    InstructionSemantics::BaseSemantics::StatePtr state_; // the semantics machine state
    ClassRepository classes_;                           // Repository of ByteCode Classes

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                  Construction and destruction
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
    virtual ~Engine();

protected:
    /** Default constructor.  Constructor is deleted and class noncopyable. */
    Engine() = delete;
    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

protected:
    /** Allocating `instance` constructors are implemented by the non-abstract subclasses. */
    Engine(const std::string &name, const Settings &settings);

public:
    // [Robb Matzke 2023-03-03]: deprecated.
    // This used to create a binary engine, so we leave it in place for a while for improved backward compatibility
    static EngineBinaryPtr instance() ROSE_DEPRECATED("use Engine::forge or EngineBinary::instance");

private:
    void init();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                  Command-line processing
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:

    /** Command-line switches for a particular engine.
     *
     *  Returns the list of switch groups that declare the command-line switches specific to a particular engine. Since every @c
     *  Engine subclass needs its own particular switches (possibly in addition to the base class switches), this is implemented in
     *  each subclass that needs switches. The base class returns a list of switch groups that are applicable to all engines,
     *  although the subclasses can refine this list, and the subclass implementations should augment what the base implementation
     *  returns.
     *
     *  In order to implement the "--help" switch to show the man page, we need a way to include the switch documentation for all
     *  possible engine subclasses at once. Therefore, the returned command line switch groups must have names and prefixes that
     *  are unique across all subclasses, and the descriptions should refer to the name of the subclass. For instance, the @ref
     *  EngineBinary class, which returns many switch groups, will name the switch groups like "binary-load", "binary-dis",
     *  "binary-part", "binary-ast", etc. and will make it clear in each group description that these switches are intended for the
     *  binary engine.
     *
     *  See @ref allCommandLineSwitches for details about how the \"--help\" man page is constructed. */
    virtual std::list<Sawyer::CommandLine::SwitchGroup> commandLineSwitches();

    /** List of command-line switches for all engines.
     *
     *  This function is used to construct the man page for a tool. It invokes @ref commandLineSwitches for the base class and every
     *  registered subclass in the opposite order they were registered (i.e., the same order they're matched). It then traverses the
     *  list and removes any group that has the same name or prefix as an earlier group. The final list is returned. */
    std::list<Sawyer::CommandLine::SwitchGroup> allCommandLineSwitches();

    /** Documentation about how the specimen is specified.
     *
     *  The documentation string that's returned is expected to be used in a command-line parser description and thus may contain
     *  special formatting constructs. For most engine subclasses, this will be a description of those command-line positional
     *  arguments that describe the specimen. For instance, the @ref EngineJvm subclass would probably document that the specimen
     *  consists of one or more file names ending with the string \".class\".
     *
     *  In order to support the --help switch that generates the man page, it must be possible to include the documentation for
     *  all subclasses concurrently. Therefore, each subclass returns both a section title and the section documentation string.
     *  The section title and documentation string should make it clear that this part of the documentation applies only to that
     *  particular subclass. */
    virtual std::pair<std::string/*title*/, std::string /*doc*/> specimenNameDocumentation() = 0;

    /** Documentation for all specimen specifications.
     *
     *  This function calls the @ref specimenNameDocumentation for all registered subclasses in the reverse order their factories
     *  were registered, which is the same order they're matched. It then combines (and warns) documentation that has the same title
     *  and returns the (possibly modified) list. The list consists of pairs where the first of each pair is the unique section
     *  title and the second is the documentation tht goes into that section. */
    static std::list<std::pair<std::string /*title*/, std::string /*doc*/>> allSpecimenNameDocumentation();

    /** Add switches and sections to command-line parser.
     *
     *  The switches and special documentation sections for the particular parser are added to the specified command-line parser.
     *
     *  Note that if you intend to create a parser that recognizes more than one engine type, it is better to use @ref
     *  addAllToParser since that function is better for this purpose. If you want to include only a few engines, consider
     *  clearing the engine factory list and replacing it with only those you want before calling @ref addAllToParser. */
    virtual void addToParser(Sawyer::CommandLine::Parser&);

    /** Add switches and sections to command-line parser.
     *
     *  This function adds switch groups and descriptions for all available engine subclases, thus producing a parser that's
     *  suitable for generating a man page. It uses @ref allCommandLineSwitches and @ref allSpecimenNameDocumentation to acheive
     *  this goal. */
    void addAllToParser(Sawyer::CommandLine::Parser&);

    /** Creates a command-line parser.
     *
     *  This creates and returns a command-line parser that contains the switch groups and documentation sections for all registered
     *  engines. No other switches are included in the parser, particularly the switches defined by @ref
     *  Rose::CommandLine::genericSwitches.
     *
     *  The @p purpose should be a single line string that will be shown in the title of the man page and should
     *  not start with an upper-case letter, a hyphen, white space, or the name of the command. E.g., a disassembler tool might
     *  specify the purpose as "disassembles a binary specimen".
     *
     *  The @p description is a full, multi-line description written in the Sawyer markup language where "@" characters have
     *  special meaning. */
    virtual Sawyer::CommandLine::Parser commandLineParser(const std::string &purpose, const std::string &description);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                  Factories
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
    /** Register an engine as a factory.
     *
     *  The specified engine is added to the end of a list of engine prototypical factory objects. When a new engine is
     *  needed, this list is scanned in reverse order until one of the @c matchFactory predicates for the prototypical object
     *  returns true, at which time a new copy of that object is created by passing the lookup arguments to its virtual @c
     *  instanceFromFactory constructor.
     *
     *  Thread safety: This method is thread safe. */
    static void registerFactory(const EnginePtr &factory);

    /** Remove a concrete engine factory from the registry.
     *
     *  The last occurrence of the specified factory is removed from the list of registered factories. This function returns
     *  true if a factory was removed, and false if no registered factories match.
     *
     *  Thread safety: This method is thread safe. */
    static bool deregisterFactory(const EnginePtr &factory);

    /** List of all registered factories.
     *
     *  The returned list contains the registered factories in the order they were registereed, which is the reverse order
     *  of how they're searched.
     *
     *  Thread safety: This method is thread safe. */
    static std::vector<EnginePtr> registeredFactories();

    /** Creates a suitable engine based on the specimen.
     *
     *  Scans the @ref registeredFactories list in the reverse order looking for a factory whose @ref matchFactory predicate returns
     *  true. The first factory whose predicate returns true is used to create and return a new concrete engine object by invoking
     *  the factory's virtual @c instanceFromFactory constructor with the first argument of this function.
     *
     *  The version that takes a string or vector of strings, the "specimen", returns a new engine from the first factory that says
     *  it can handle those strings. For instance, if the @ref EngineJvm can handle a specimen whose name is a class file having the
     *  extension \".class\".
     *
     *  The versions that takes a command-line switch parser and an optional command-line positional argument parser and attempts to
     *  parse the command-line to obtain the specimen, which is then passed to the version that takes a specimen. The incoming
     *  parser should not have definitions for switches that adjust the engine settings--they will be added automatically to the
     *  parser before returning. The returned parser will be augmented with switches for all engines so that \"--help\" and friends
     *  work propertly, however only the switches applicable to the returned engine will be linked to the returned engine (switches
     *  for other engine types are linked to their respective factories as a holding area, although they're never used for
     *  anything).
     *
     *  If settings are specified, then they are the defaults to be adjusted by the command-line parser. These defaults will also
     *  show up in the man page.
     *
     *  Thread safety: This method is thread safe.
     *
     * @{ */

    //---------------------------------------------------------
    // These operate on specimens
    //---------------------------------------------------------

    static EnginePtr forge(const std::vector<std::string> &specimen);
    static EnginePtr forge(const std::string &specimen);

    //---------------------------------------------------------
    // These operate on arguments as std::vector<std::string>
    //---------------------------------------------------------

    // all args
    static EnginePtr forge(const std::vector<std::string> &arguments, Sawyer::CommandLine::Parser&,
                           const PositionalArgumentParser&, const Settings&);

    // default settings
    static EnginePtr forge(const std::vector<std::string> &arguments, Sawyer::CommandLine::Parser&,
                           const PositionalArgumentParser&);

    // default positional parser
    static EnginePtr forge(const std::vector<std::string> &arguments, Sawyer::CommandLine::Parser&, const Settings&);

    // default positional parser and settings
    static EnginePtr forge(const std::vector<std::string> &arguments, Sawyer::CommandLine::Parser&);

    //---------------------------------------------------------
    // These operate on arguments as argc and argv
    //---------------------------------------------------------

    // all args
    static EnginePtr forge(int argc, char *argv[], Sawyer::CommandLine::Parser&, const PositionalArgumentParser&, const Settings&);

    // default settings
    static EnginePtr forge(int argc, char *argv[], Sawyer::CommandLine::Parser&, const PositionalArgumentParser&);

    // default positional parser
    static EnginePtr forge(int argc, char *argv[], Sawyer::CommandLine::Parser&, const Settings&);

    // default positional parser and settings
    static EnginePtr forge(int argc, char *argv[], Sawyer::CommandLine::Parser&);
    /** @} */

    /** Predicate for matching a concrete engine factory by parser result and specimen. */
    virtual bool matchFactory(const Sawyer::CommandLine::ParserResult &result, const std::vector<std::string> &specimen) const = 0;

    /** Virtual constructor for factories.
     *
     *  This creates a new object by calling the class method @c instance for the class of which @c this is a type. All
     *  arguments are passed to @c instance. */
    virtual EnginePtr instanceFromFactory(const Settings&) = 0;

    /** Returns true if this object is a factory.
     *
     *  Factories are created by the @c factory class methods rather than the usual @c instance class methods. A factory
     *  object should only be used to create other (non-factory) objects by registering it as a factory and eventually
     *  calling (directly or indirectly) its @ref instanceFromFactory object method. */
    bool isFactory() const;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                  Top-level, do everything functions
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
    SgAsmBlock* frontend(int argc, char *argv[],
                         const std::string &purpose, const std::string &description);
    virtual SgAsmBlock* frontend(const std::vector<std::string> &args,
                                 const std::string &purpose, const std::string &description) = 0;
    /** @} */

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                  Basic top-level steps
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
    /** Reset the engine to its initial state.
     *
     *  This does not reset the settings properties since that can be done easily by constructing a new engine.  It only resets the
     *  interpretation, binary loader, and memory map so all the top-level steps get executed again. This is a useful way to re-use
     *  the same partitioner to process multiple specimens. */
    virtual void reset();

    /** Parse the command-line.
     *
     *  This method parses the command-line and uses it to update this engine's settings.  Since a command line is usually more
     *  than just engine-related switches, the more usual approach is for the user to obtain engine-related command-line switch
     *  declarations and parse the command-line in user code.
     *
     *  This function automatically applies the command-line when it's successfully parsed, thereby updating this engine's settings.
     *  If something goes wrong with the command-line then an `std::runtime_error` is thrown.
     *
     *  The command-line can be provided as a typical @c argc and @c argv pair, or as a vector of arguments. In the latter case, the
     *  vector should not include `argv[0]` or `argv[argc]` (which is always a null pointer).
     *
     *  The @p purpose should be a single line string that will be shown in the title of the man page and should
     *  not start with an upper-case letter, a hyphen, white space, or the name of the command. E.g., a disassembler tool might
     *  specify the purpose as "disassembles a binary specimen".
     *
     *  The @p description is a full, multi-line description written in the Sawyer markup language where "@" characters have
     *  special meaning.
     *
     *  If the tool requires additional switches, an opportunity to adjust the parser, or other special handling, it can call
     *  @ref commandLineParser to obtain a parser and then call its @c parse and @c apply methods explicitly.
     *
     *  If an `std::runtime_exception` occurs and the @ref EngineSettings::exitOnError property is set, then the exception is
     *  caught, its text is emitted to the partitioner's fatal error stream, and `exit(1)` is invoked.
     *
     * @{ */
    Sawyer::CommandLine::ParserResult parseCommandLine(int argc, char *argv[],
                                                       const std::string &purpose, const std::string &description) /*final*/;
    virtual Sawyer::CommandLine::ParserResult parseCommandLine(const std::vector<std::string> &args,
                                                               const std::string &purpose, const std::string &description);
    /** @} */

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
    virtual SgAsmBlock* buildAst(const std::vector<std::string> &fileNames = std::vector<std::string>()) = 0;
    SgAsmBlock *buildAst(const std::string &fileName) /*final*/;
    /** @} */

    // [Robb Matzke 2023-03-03]: deprecated
    // Save a partitioner and AST to a file.
    //
    // The specified partitioner and the binary analysis components of the AST are saved into the specified file, which is
    // created if it doesn't exist and truncated if it does exist. The name should end with a ".rba" extension. The file can
    // be loaded by passing its name to the @ref partition function or by calling @ref loadPartitioner.
    virtual void savePartitioner(const PartitionerConstPtr&, const boost::filesystem::path&, Serialization::Format = Serialization::BINARY)
        ROSE_DEPRECATED("use Partitioner::saveAsRbaFile");

    // [Robb Matzke 2023-03-03]: deprecated
    // Load a partitioner and an AST from a file.
    //
    // The specified RBA file is opened and read to create a new @ref Partitioner object and associated AST. The @ref
    // partition function also understands how to open RBA files.
    virtual PartitionerPtr loadPartitioner(const boost::filesystem::path&, Serialization::Format = Serialization::BINARY)
        ROSE_DEPRECATED("use Partitioner::instanceFromRbaFile");

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                  Command-line parsing
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
    /** Parse specimen binary containers.
     *
     *  Parses the ELF and PE binary containers to create an abstract syntax tree (AST).  If @p fileNames contains names that
     *  are recognized as raw data or other non-containers then they are skipped over at this stage but processed during the
     *  @ref loadSpecimens stage.
     *
     *  This method tries to determine the specimen architecture. It also resets the interpretation to be the return value (see
     *  below), and clears the memory map.
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
    virtual SgAsmInterpretation* parseContainers(const std::vector<std::string> &fileNames) = 0;
    SgAsmInterpretation* parseContainers(const std::string &fileName) /*final*/;
    /** @} */

    /** Load and/or link interpretation.
     *
     *  Loads and/or links the engine's interpretation according to the engine's binary loader with these steps:
     *
     *  @li Clears any existing memory map in the engine.
     *
     *  @li If the binary containers have not been parsed (@ref areContainersParsed returns false, i.e., engine has a null
     *      binary interpretation) then @ref parseContainers is called with the same arguments.
     *
     *  @li If binary containers are present but the chosen binary interpretation's memory map is null or empty, then
     *      initialize the memory map.
     *
     *  @li Continue initializing the memory map by processing all non-container arguments.
     *
     *  Returns a reference to the engine's memory map.
     *
     *  If an `std::runtime_exception` occurs and the @ref EngineSettings::exitOnError property is set, then the exception is
     *  caught, its text is emitted to the partitioner's fatal error stream, and `exit(1)` is invoked.
     *
     * @{ */
    virtual MemoryMapPtr loadSpecimens(const std::vector<std::string> &fileNames = std::vector<std::string>()) = 0;
    MemoryMapPtr loadSpecimens(const std::string &fileName) /*final*/;
    /** @} */

    /** Partition instructions into basic blocks and functions.
     *
     *  Disassembles and organizes instructions into basic blocks and functions with these steps:
     *
     *  @li If the specimen is not loaded (@ref areSpecimensLoaded) then call @ref loadSpecimens. The no-argument version of
     *  this function requires that specimens have already been loaded.
     *
     *  @li Determine the architecture for the specimen by calling @ref obtainArchitecture.
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
    virtual PartitionerPtr partition(const std::vector<std::string> &fileNames = std::vector<std::string>()) = 0;
    PartitionerPtr partition(const std::string &fileName) /*final*/;
    /** @} */

    /** Check settings after command-line is processed.
     *
     *  This does some checks and further configuration immediately after processing the command line. It's also called by most
     *  of the top-level operations.
     *
     *  If an ISA name is specified in the settings and no architecture has been set yet, then an architecture is chosen. */
    virtual void checkSettings();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                  Container parsing
    //
    // top-level: parseContainers
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
    /** Determine whether a specimen is an RBA file.
     *
     *  Returns true if the name looks like a ROSE Binary Analysis file. Such files are not intended to be passed to ROSE's
     *  global @c ::frontend function but may be passed to this Engine's @ref Engine::frontend method. */
    virtual bool isRbaFile(const std::string&);

    /** Determine whether a specimen name is a non-container.
     *
     *  Certain strings are recognized as special instructions for how to adjust a memory map and are not intended to be passed
     *  to ROSE's @c frontend function.  This predicate returns true for such strings. */
    virtual bool isNonContainer(const std::string&) = 0;

    /** Returns true if containers are parsed.
     *
     *  Specifically, returns true if the engine has a non-null interpretation.  If it has a null interpretation then
     *  @ref parseContainers might have already been called but no binary containers specified, in which case calling it again
     *  with the same file names will have no effect. */
    virtual bool areContainersParsed() const = 0;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                  Load specimens
    //
    // top-level: loadSpecimens
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
    /** Returns true if specimens are loaded.
     *
     *  Specifically, returns true if the memory map is non-empty. */
    virtual bool areSpecimensLoaded() const;

    /** Adjust memory map post-loading.
     *
     *  Make adjustments to the memory map after the specimen is loaded. */
    virtual void adjustMemoryMap();

    /** Property: memory map
     *
     *  Returns the memory map resulting from the @ref loadSpecimens step.  This is a combination of the memory map created by the
     *  BinaryLoader and stored in the interpretation, and the application of any memory map resources (non-container
     *  arguments). During partitioning operations the memory map comes from the partitioner itself.  See @ref loadSpecimens.
     *
     * @{ */
    MemoryMapPtr memoryMap() const /*final*/;
    virtual void memoryMap(const MemoryMapPtr&);
    /** @} */

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                  Architecture
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
    /** Determine the architecture.
     *
     *  Chooses an architecture based on one of the following (in this order):
     *
     *  @li If this engine's @ref architecture property is non-null, then return that architecture.
     *
     *  @li If this engine's ISA name setting is non-empty, then use an architecture that handles that name.
     *
     *  @li If a binary container was parsed (@ref areContainersParsed returns true and @ref interpretation is non-null) then
     *      try to choose an architecture based on the interpretation.
     *
     *  @li If a @p hint is supplied, then use it.
     *
     *  @li Fail by throwing a @ref Architecture::NotFound error.
     *
     *  In any case, the @ref architecture property is set to this method's return value.
     *
     * @{ */
    virtual Architecture::BaseConstPtr obtainArchitecture();
    virtual Architecture::BaseConstPtr obtainArchitecture(const Architecture::BaseConstPtr &hint);
    /** @} */

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                  Partitioner high-level functions
    //
    // top-level: partition
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
    /** Check that we have everything necessary to create a partitioner. */
    virtual void checkCreatePartitionerPrerequisites() const;

    /** Create a bare partitioner.
     *
     *  A bare partitioner, as far as the engine is concerned, is one that has characteristics that are common across all
     *  architectures but which is missing all architecture-specific functionality.  Using the partitioner's own constructor is
     *  not quite the same--that would produce an even more bare partitioner! */
    virtual PartitionerPtr createBarePartitioner();

    /** Create partitioner.
     *
     *  This is the method usually called to create a new partitioner. */
    virtual PartitionerPtr createPartitioner() = 0;

    /** Finds interesting things to work on initially.
     *
     *  Seeds the partitioner with addresses and functions where recursive disassembly should begin. */
    virtual void runPartitionerInit(const PartitionerPtr&) = 0;

    /** Runs the recursive part of partioning.
     *
     *  This is the long-running guts of the partitioner. */
    virtual void runPartitionerRecursive(const PartitionerPtr&) = 0;

    /** Runs the final parts of partitioning.
     *
     *  This does anything necessary after the main part of partitioning is finished. For instance, it might give names to some
     *  functions that don't have names yet. */
    virtual void runPartitionerFinal(const PartitionerPtr&) = 0;

    /** Partitions instructions into basic blocks and functions.
     *
     *  This method is a wrapper around a number of lower-level partitioning steps that uses the specified interpretation to
     *  instantiate functions and then uses the specified partitioner to discover basic blocks and use the CFG to assign basic
     *  blocks to functions.  It is often overridden by subclasses. */
    virtual void runPartitioner(const PartitionerPtr&);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                  Partitioner mid-level functions
    //
    // These are the functions called by the partitioner high-level stuff.  These are sometimes overridden in subclasses,
    // although it is more likely that the high-level stuff is overridden.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
    /** Label addresses.
     *
     *  Labels addresses according to symbols, etc.  Address labels are used for things like giving an unnamed function a name
     *  when it's attached to the partitioner's CFG/AUM. */
    virtual void labelAddresses(const PartitionerPtr&, const Configuration&);

    /** Make data blocks based on configuration.
     *
     *  NOTE: for now, all this does is label the datablock addresses. FIXME[Robb P. Matzke 2015-05-12] */
    virtual std::vector<DataBlockPtr> makeConfiguredDataBlocks(const PartitionerPtr&, const Configuration&);

    /** Make functions based on configuration information.
     *
     *  Uses the supplied function configuration information to make functions. */
    virtual std::vector<FunctionPtr> makeConfiguredFunctions(const PartitionerPtr&, const Configuration&);

    /** Runs various analysis passes.
     *
     *  Runs each analysis over all functions to ensure that results are cached.  This should typically be done after functions
     *  are discovered and before the final AST is generated, otherwise the AST will not contain cached results for functions
     *  and blocks for which an analysis was not performed. */
    virtual void updateAnalysisResults(const PartitionerPtr&);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                  Partitioner low-level functions
    //
    // These are functions that a subclass seldom overrides, and maybe even shouldn't override because of their complexity or
    // the way the interact with one another.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                  Build AST
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
    // Used internally by ROSE's ::frontend disassemble instructions to build the AST that goes under each SgAsmInterpretation.
    static void disassembleForRoseFrontend(SgAsmInterpretation*);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                  Settings and properties
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
    /** Property: Name.
     *
     *  The name of the engine comes from the engine factory, will be empty if not a factory.
     *
     * @{ */
    const std::string& name() const /*final*/;
    void name(const std::string&);
    /** @} */

    /** Property: Architecture.
     *
     *  The non-null object representing architecture-specific information is returned, or an @ref Architecture::NotFound exception
     *  is thrown if there is no architecture and none can be determined. */
    Architecture::BaseConstPtr architecture();

    /** Property: All settings.
     *
     *  Returns a reference to the engine settings structures.  Alternatively, some settings also have a corresponding engine
     *  member function to query or adjust the setting directly.
     *
     * @{ */
    const Settings& settings() const /*final*/;
    Settings& settings() /*final*/;
    void settings(const Settings&) /*final*/;
    /** @} */

    /** Property: BasicBlock work list.
     *
     *  This property holds the list of what blocks to work on next.
     *
     * @{ */
    BasicBlockWorkList::Ptr basicBlockWorkList() const /*final*/;
    void basicBlockWorkList(const BasicBlockWorkList::Ptr&) /*final*/;

    /** Property: Instruction AST constants.
     *
     *  This property holds constants that are found in instruction ASTs.
     *
     * @{ */
    CodeConstants::Ptr codeFunctionPointers() const /*final*/;
    void codeFunctionPointers(const CodeConstants::Ptr&) /*final*/;
    /** @} */

    /** Property: interpretation
     *
     *  The interpretation which is being analyzed. The interpretation is chosen when an ELF or PE container is parsed, and the
     *  user can set it to something else if desired. For instance, parsing a PE file will set the interpretation to PE, but
     *  the user can reset it to DOS to disassemble the DOS part of the executable.
     *
     * @{ */
    SgAsmInterpretation* interpretation() const /*final*/;
    virtual void interpretation(SgAsmInterpretation*);
    /** @} */

    /** Property: progress reporting.
     *
     *  The optional object to receive progress reports.
     *
     * @{ */
    ProgressPtr progress() const /*final*/;
    virtual void progress(const ProgressPtr&);
    /** @} */

    /** Property: specimen.
     *
     *  The specimen is a list of additional command line arguments. It is often a list of file names.
     *
     * @{ */
    const std::vector<std::string>& specimen() const /*final*/;
    virtual void specimen(const std::vector<std::string>&);
    /** @} */

    /** Property: state.
     *
     *  The semantic machine state.
     *
     * @{ */
    InstructionSemantics::BaseSemantics::StatePtr state();
    /** @} */

    /** Property: ByteCode class used for partitioning.
     *
     *  After partitioning this class makes the partitioning results available for analysis.
     *
     * @{ */
    ByteCode::Class::Ptr analysisClass();
    /** @} */

    /** Property: classes.
     *
     *  Repository of ByteCode Classes.
     *
     * @{ */
    ClassRepository classes() const;

    /** Retrieve a ByteCode class from the repository by name */
    ByteCode::Class::Ptr classByName(const std::string &name) const;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                  Internal stuff
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
protected:
    // Similar to ::frontend but a lot less complicated.
    virtual SgProject* roseFrontendReplacement(const std::vector<boost::filesystem::path> &fileNames) = 0;
};

} // namespace
} // namespace
} // namespace

#endif
#endif
