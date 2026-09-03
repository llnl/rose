#ifndef ROSE_BinaryAnalysis_InstructionSemantics_BaseSemantics_FrameState_H
#define ROSE_BinaryAnalysis_InstructionSemantics_BaseSemantics_FrameState_H
#include <featureTests.h>
#ifdef ROSE_ENABLE_BINARY_ANALYSIS

#include <Rose/BinaryAnalysis/InstructionSemantics/BaseSemantics/BasicTypes.h>
#include <Rose/BinaryAnalysis/InstructionSemantics/BaseSemantics/AddressSpace.h>
#include <Rose/BinaryAnalysis/InstructionSemantics/BaseSemantics/MemoryState.h>
#include <Rose/BinaryAnalysis/ByteCode/Analysis.h>

namespace Rose {
namespace BinaryAnalysis {
namespace InstructionSemantics {
namespace BaseSemantics {

/** Shared-ownership pointer for frame memory state. */
using FrameStatePtr = boost::shared_ptr<class FrameState>;

/** Purely symbolic memory state.
 *
 *  This memory state stores Jvm operands and local variables.
 */

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Frame State
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Jvm Frames, see section 2.6.

 * A frame is used to store data and partial results, as well as to perform dynamic linking, return values for methods,
 * and dispatch exceptions.

 * A new frame is created each time a method is invoked. A frame is destroyed when its method invocation completes,
 * whether that completion is normal or abrupt (it throws an uncaught exception). Frames are allocated from the Java
 * Virtual Machine stack (§2.5.2) of the thread creating the frame. Each frame has its own array of local variables (§2.6.1),
 * its own operand stack (§2.6.2), and a reference to the run-time constant pool (§2.5.5) of the class of the current method.

 * A frame may be extended with additional implementation-specific information, such as debugging information.

 * The sizes of the local variable array and the operand stack are determined at compile-time and are supplied along with the
 * code for the method associated with the frame (§4.7.3). Thus the size of the frame data structure depends only on the
 * implementation of the Java Virtual Machine, and the memory for these structures can be allocated simultaneously on
 * method invocation.

 * Only one frame, the frame for the executing method, is active at any point in a given thread of control. This frame is
 * referred to as the current frame, and its method is known as the current method. The class in which the current method is
 * defined is the current class. Operations on local variables and the operand stack are typically with reference to the
 * current frame.

 * A frame ceases to be current if its method invokes another method or if its method completes. When a method is invoked,
 * a new frame is created and becomes current when control transfers to the new method. On method return, the current frame
 * passes back the result of its method invocation, if any, to the previous frame. The current frame is then discarded as the
 * previous frame becomes the current one.

 * Note that a frame created by a thread is local to that thread and cannot be referenced by any other thread.
 */

/** The set of all frames and their values. FrameState objects are allocated on the heap and reference counted.  The
 *  See the Rose::BinaryAnalysis::InstructionSemantics namespace for an overview of how the various memory state
 *  parts fit together.*/
class FrameState: public MemoryState {
public:
    /** Base type. */
    using Super = MemoryState;

    /** Shared-ownership pointer. */
    using Ptr = FrameStatePtr;

private:
    size_t frameId_ = 0; // unique frame identifier
    Sawyer::Optional<Address> returnAddress_ = Sawyer::Nothing(); // caller resume address
    std::vector<SValuePtr> stack_;  // operand stack for the frame
    std::vector<SValuePtr> locals_; // local variables for the frame
    ByteCode::Method::Ptr method_;  // method associated with this invocation frame

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Serialization
#ifdef ROSE_ENABLE_BOOST_SERIALIZATION
private:
    friend class boost::serialization::access;

    template<class S>
    void serialize(S &s, const unsigned /*version*/) {
        s & BOOST_SERIALIZATION_BASE_OBJECT_NVP(MemoryState);
        s & BOOST_SERIALIZATION_NVP(frameId_);
        s & BOOST_SERIALIZATION_NVP(returnAddress_);
        s & BOOST_SERIALIZATION_NVP(stack_);
        s & BOOST_SERIALIZATION_NVP(locals_);
        s & BOOST_SERIALIZATION_NVP(method_);
    }
#endif

public:
    ~FrameState();

protected:
    FrameState(); // for serialization

    // All memory states should be heap allocated; use instance(), create(), or clone() instead.
    explicit FrameState(const SValue::Ptr &valProtoval, const Sawyer::Optional<Address> &, const ByteCode::Method::Ptr &);
    explicit FrameState(const FrameState &other);

public:
    /** Instantiate a new empty frame state on the heap. */
    static FrameStatePtr instance(const SValuePtr &, const Sawyer::Optional<Address> &, const ByteCode::Method::Ptr &);

public:
    // documented in base class
    MemoryStatePtr create(const SValuePtr &addrProtoval, const SValuePtr &valProtoval) const override;

    // documented in base class
    AddressSpacePtr clone() const override;

    /** Convert pointer to a FrameState pointer.
     *
     *  Converts @p x to a FrameStatePtr and asserts that it is non-null. */
    static FrameStatePtr promote(const AddressSpacePtr&);

public:
    void clear() override;
    void clearFrame();

    bool merge(const AddressSpacePtr &other, RiscOperators *addrOps, RiscOperators *valOps) override;

    SValuePtr readMemory(const SValuePtr &address, const SValuePtr &dflt,
                         RiscOperators *addrOps, RiscOperators *valOps) override;

    void writeMemory(const SValuePtr &address, const SValuePtr &value,
                     RiscOperators *addrOps, RiscOperators *valOps) override;

    SValuePtr peekMemory(const SValuePtr &address, const SValuePtr &dflt,
                         RiscOperators *addrOps, RiscOperators *valOps) override;

    SValuePtr readLocal(size_t index) const override;
    void writeLocal(size_t index, const SValuePtr &value) override;

    SValuePtr peekOperand(size_t depth = 0) override;
    SValuePtr popOperand() override;
    void pushOperand(const SValuePtr &value) override;

public:
    void hash(Combinatorics::Hasher&, RiscOperators* addrOps, RiscOperators* valOps) const override;
    void print(std::ostream&, Formatter&) const override;

    ByteCode::Method::Ptr method() const;

    SValuePtr createArgument(const std::string &descriptor, size_t argIdx);
    static std::string argumentDescriptor(const std::string &descriptor, size_t argIdx);

    const Sawyer::Optional<Address>& returnAddress() const;
    void returnAddress(Address);

    size_t frameId() const;
    void frameId(size_t);
    static size_t newFrameId();

    std::string frameLabel() const;
    std::string frameName() const;

    // Short form of frameLabel
    std::string label() const override;

    static bool isCategory1(const SValuePtr&);
    static bool isCategory2(const SValuePtr&);
    static bool isCategory2Tail(const SValuePtr&);

private:
    /** Initialize the frame with a receiver and arguments based on the method descriptor */
    void initializeForRootFrame();

    /** Invalidate a range of local-variable slots.
     *
     *  Any overlapping category-2 value (long/double) is completely removed,
     *  including its head and tail slots, to preserve JVM slot semantics.
     */
    void clearLocalRange(size_t begin, size_t nSlots);
};
    
} // namespace
} // namespace
} // namespace
} // namespace

#ifdef ROSE_ENABLE_BOOST_SERIALIZATION
BOOST_CLASS_EXPORT_KEY(Rose::BinaryAnalysis::InstructionSemantics::BaseSemantics::FrameState);
#endif

#endif
#endif
