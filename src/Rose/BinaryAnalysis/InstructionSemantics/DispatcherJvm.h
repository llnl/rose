#ifndef ROSE_BinaryAnalysis_InstructionSemantics_DispatcherJvm_H
#define ROSE_BinaryAnalysis_InstructionSemantics_DispatcherJvm_H
#include <featureTests.h>
#ifdef ROSE_ENABLE_BINARY_ANALYSIS

#include <Rose/BinaryAnalysis/InstructionSemantics/DescriptorParser.h>
#include <Rose/BinaryAnalysis/InstructionSemantics/BaseSemantics/Dispatcher.h>
#include <Rose/BinaryAnalysis/InstructionSemantics/BaseSemantics/FrameState.h>

#ifdef ROSE_HAVE_BOOST_SERIALIZATION_LIB
#include <boost/serialization/access.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/split_member.hpp>
#endif

namespace Rose {
namespace BinaryAnalysis {
namespace InstructionSemantics {

/** Shared-ownership pointer to a JVM instruction dispatcher. */
using DispatcherJvmPtr = boost::shared_ptr<class DispatcherJvm>;

/** Dispatches JVM instructions through the semantics layer.
 *
 *  The instruction is lowered to a set of a few dozen operations that operate on values and states in a semantic domain. */
class DispatcherJvm: public BaseSemantics::Dispatcher {
public:
    /** Base type. */
    using Super = BaseSemantics::Dispatcher;

    /** Shared-ownership pointer. */
    using Ptr = DispatcherJvmPtr;

public:
    /** Cached register.
     *
     *  This register is cached so that there are not so many calls to look up registers by name.
     *
     * @{ */
    const RegisterDescriptor REG_PC, REG_SP, REG_FP;
    /** @} */

#ifdef ROSE_HAVE_BOOST_SERIALIZATION_LIB
private:
    friend class boost::serialization::access;

    template<class S>
    void save(S &s, const unsigned /*version*/) const {
        s & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Super);
    }

    template<class S>
    void load(S &s, const unsigned /*version*/) {
        s & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Super);
        initializeDispatchTable();
        initializeMemoryState();
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER();
#endif

public:
    ~DispatcherJvm();
protected:
    DispatcherJvm() = delete;
    explicit DispatcherJvm(const Architecture::BaseConstPtr&);
    DispatcherJvm(const Architecture::BaseConstPtr&, const BaseSemantics::RiscOperatorsPtr&);

public:
    /** Construct a prototypical dispatcher.
     *
     *  The only thing this dispatcher can be used for is to create another dispatcher with the virtual @ref create method. */
    static Ptr instance(const Architecture::BaseConstPtr&);

    /** Constructor. */
    static Ptr instance(const Architecture::BaseConstPtr&, const BaseSemantics::RiscOperatorsPtr&);

    /** Dynamic cast to DispatcherJvmPtr with assertion. */
    static Ptr promote(const BaseSemantics::DispatcherPtr&);

public:
    // documented in the base class
    virtual BaseSemantics::DispatcherPtr create(const BaseSemantics::RiscOperatorsPtr&) const override;

    virtual int iprocKey(SgAsmInstruction*) const override;
    virtual RegisterDescriptor instructionPointerRegister() const override;

    /** Completes a return from the current JVM method.
     *
     *  Pops the current frame, pushes @p result onto the caller's operand stack
     *  when non-null, and restores the caller's instruction pointer. A frame
     *  without a return address is treated as the root analysis frame.
     */
    void completeReturn(BaseSemantics::RiscOperators*,
                        const BaseSemantics::SValuePtr result = BaseSemantics::SValuePtr());

    /** Records an error in the semantic analysis */
    void recordSemanticError(const std::string &msg);

    /** Returns the bit width for a JVM value kind. */
    static size_t nBitsForKind(BaseSemantics::ValueKind);

    /** Returns the constant pool */
    static SgAsmJvmConstantPool* constantPool(BaseSemantics::RiscOperators *ops);

    /** Finds and returns the descriptor for a method from the constant pool at the given index */
    static std::string methodDescriptor(SgAsmJvmConstantPool *pool, size_t index);

    /** Finds and returns the descriptor for a field from the constant pool at the given index */
    static std::string fieldDescriptor(SgAsmJvmConstantPool *pool, size_t index);

    /** Creates a synthetic JVM value having the specified descriptor type and unknown value. */
    static BaseSemantics::SValuePtr
    syntheticValue(const BaseSemantics::SValuePtr &protoval,
                   const DescriptorType &type,
                   const std::string &symbolName = "");

    /** Creates a synthetic JVM object reference with an unknown reference value. */
    static BaseSemantics::SValuePtr
    syntheticObjectReference(const BaseSemantics::SValuePtr &protoval,
                             const std::string &descriptor,
                             const std::string &symbolName = "");

    /** Creates a synthetic JVM array reference with an unknown reference and length. */
    static BaseSemantics::SValuePtr
    syntheticArrayReference(const BaseSemantics::SValuePtr &protoval,
                            const std::string &descriptor,
                            const std::string &symbolName = "");

    /** Initializes local variables in advance of a method invocation */
    static void initializeInvocationLocals(BaseSemantics::RiscOperators *ops,
                                           const BaseSemantics::FrameState::Ptr &frame,
                                           const std::string &descriptor, bool hasReceiver);

private:
    // Initializes the dispatch table that handles each kind of instruction
    void initializeDispatchTable();

    // Initializes memory state, such as the default byte order
    void initializeMemoryState();
};

} // namespace
} // namespace
} // namespace

#ifdef ROSE_HAVE_BOOST_SERIALIZATION_LIB
BOOST_CLASS_EXPORT_KEY(Rose::BinaryAnalysis::InstructionSemantics::DispatcherJvm);
#endif

#endif
#endif
