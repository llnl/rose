#ifndef ROSE_BinaryAnalysis_InstructionSemantics_BaseSemantics_State_H
#define ROSE_BinaryAnalysis_InstructionSemantics_BaseSemantics_State_H
#include <featureTests.h>
#ifdef ROSE_ENABLE_BINARY_ANALYSIS
#include <Rose/BinaryAnalysis/InstructionSemantics/BaseSemantics/BasicTypes.h>
#include <Rose/BinaryAnalysis/InstructionSemantics/BaseSemantics/State.h>
#include <Rose/BinaryAnalysis/InstructionSemantics/BaseSemantics/SValue.h>

#include <Rose/BinaryAnalysis/RegisterDescriptor.h>
#include <Combinatorics.h>                              // rose

#include <boost/enable_shared_from_this.hpp>

#ifdef ROSE_ENABLE_BOOST_SERIALIZATION
#include <boost/serialization/access.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/shared_ptr.hpp>
#endif

namespace Rose {
namespace BinaryAnalysis {
namespace InstructionSemantics {
namespace BaseSemantics {

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      State
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Base class for semantics machine states.
 *
 *  Binary semantic analysis usually progresses one instruction at a time--one starts with an initial state and the act of
 *  processing an instruction modifies the state.  The State is the base class class for the semantic states of various instruction
 *  semantic policies.
 *
 *  A state contains storage, in the form of @ref AddressSpace objects, for all the machine registers and memory. It normally has
 *  one register address space and one memory address space, although any number of address spaces are allowed. Additional address
 *  spaces can be used for such things as temporary variables and constants used during semantic analysis.
 *
 *  States refer to their address spaces using shared-ownership smart pointers, but one can also make deep copies of the state
 *  using it's @ref clone method. Deep copies generally copy only those parts of the state that are mutable, thus the individual
 *  symbolic expressions for a symbolic state are not copied. But since symbolic expressions are immutable, the lack of copying
 *  at that level is irrelevant. Many analyses keep a copy of the machine state for each instruction or each CFG vertex.
 *
 *  This BaseSemantics::State is an abstract class that defines the interface.  See the Rose::BinaryAnalysis::InstructionSemantics
 *  namespace for an overview of how the parts fit together.  */
class State: public boost::enable_shared_from_this<State> {
public:
    /** Shared-ownership pointer. */
    using Ptr = StatePtr;

private:
    SValuePtr protoval_;                                // Initial value used to create additional values as needed.
    std::vector<AddressSpacePtr> addressSpaces_;        // All address spaces for this semantic state.
    RegisterStatePtr registers_;                        // All machine register values for this semantic state.
    MemoryStatePtr memory_;                             // All memory for this semantic state.
    RegisterStatePtr interrupts_;                       // Whether interrupts occurred.
    SValuePtr returnValue_;                             // Result returned by the root method, if any.
    bool terminated_ = false;                           // No further instructions execute on this path.

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Serialization.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef ROSE_ENABLE_BOOST_SERIALIZATION
private:
    friend class boost::serialization::access;

    template<class S>
    void serialize(S &s, const unsigned version) {
        ASSERT_require2(version >= 2, version);
        s & BOOST_SERIALIZATION_NVP(protoval_);
        s & BOOST_SERIALIZATION_NVP(addressSpaces_);
        s & BOOST_SERIALIZATION_NVP(registers_);
        s & BOOST_SERIALIZATION_NVP(memory_);
        s & BOOST_SERIALIZATION_NVP(interrupts_);
        s & BOOST_SERIALIZATION_NVP(returnValue_);
        s & BOOST_SERIALIZATION_NVP(terminated_);
    }
#endif


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Real constructors.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
protected:
    // needed for serialization
    State();

    State(const RegisterStatePtr &registers, const MemoryStatePtr &memory, const RegisterStatePtr &interrupts, const FrameStatePtr &frame);
    State(const RegisterStatePtr &registers, const MemoryStatePtr &memory, const RegisterStatePtr &interrupts);
    State(const RegisterStatePtr &registers, const MemoryStatePtr &memory);

    // deep-copy the registers and memory
    State(const State &other);

public:
    virtual ~State();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Static allocating constructors
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
    /** Instantiate a new state object with specified register, memory, interrupt, and frame address spaces. */
    static StatePtr instance(const RegisterStatePtr &registers, const MemoryStatePtr &memory,
                             const RegisterStatePtr &interrupts, const FrameStatePtr &frame);

    /** Instantiate a new state object with specified register, memory, and interrupt address spaces. */
    static StatePtr instance(const RegisterStatePtr &registers, const MemoryStatePtr &memory, const RegisterStatePtr &interrupts);

    /** Instantiate a new state object with specified register and memory address spaces. */
    static StatePtr instance(const RegisterStatePtr &registers, const MemoryStatePtr &memory);

    /** Instantiate a new copy of an existing state.
     *
     *  The existing state is deep-copied to form the new returned state. Immutable parts of the state (such as symbolic
     *  expressions) are not actually copied since both states can safely refer to the same immutable data. See also, @ref clone. */
    static StatePtr instance(const StatePtr &other);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Virtual constructors.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
    /** Virtual constructor.
     *
     *  Constructs a new instance of a state from an existing state without copying the existing state. The register and memory
     *  address spaces will be set to the specified states and the optional @ref interruptState will be a null pointer. */
    virtual StatePtr create(const RegisterStatePtr &registers, const MemoryStatePtr &memory) const;

    /** Virtual copy constructor.
     *
     *  Allocates a new state object which is a deep copy of this state. States must be copyable objects because many analyses
     *  depend on being able to make a copy of the entire semantic state at each machine instruction, at each CFG vertex, etc. */
    virtual StatePtr clone() const;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Dynamic pointer casts.  No-op since this is the base class.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
    static StatePtr promote(const StatePtr&);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Properties of a state.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
    /** Property: Prototypical value.
     *
     *  The protoval is used to construct other values via its virtual constructors. For instance, if the protoval is of class
     *  @ref Rose::BinaryAnalysis::InstructionSemantics::SymbolicSemantics::SValue then new values that need to be created as
     *  part of operations performed on this state will also be symbolic values. Similarly, if the protoval is a value of type
     *  @ref Rose::BinaryAnalysis::InstructionSemantics::ConcreteSemantics::SValue then new values will be concrete values using
     *  that same type. This allows the state to support any user-defined value type even if it wasn't linked as part of the
     *  original ROSE library source code. One just has to be sure that the user-defined `SValue` type implements the correct
     *  virtual constructors named `create` and `clone`. */
    SValuePtr protoval() const;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Address space declaration and searching.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
    /** Insert an address space into this state.
     *
     *  The specified address space, which must not be null, is appended to the list of address spaces for this state. A state
     *  can have any number of address spaces, but it usually has one register space and one memory space. It is also permissible
     *  for a state to have multiple address spaces with the same @ref AddressSpace::purpose "purpose" property but which serve
     *  slightly different purposes, in which case functions that look up a space by its purpose generally return the first such
     *  space that's found. */
    void insertAddressSpace(const AddressSpacePtr&);

    /** The list of all address spaces. */
    const std::vector<AddressSpacePtr>& addressSpaces() const;

    /** Find the first address space with the specified purpose.
     *
     *  Returns a pointer to the first address space in the @ref addressSpaces such that its purpose is as specified. If no such
     *  address space exists, then the null pointer is returned. */
    AddressSpacePtr findFirstAddressSpace(AddressSpacePurpose) const;

    /** Find the first address space with the specified purpose and name.
     *
     *  Returns a pointer to the first address space in the @ref addressSpaces such that its purpose and name are as specified. If
     *  no such address space exists, then the null pointer is returned. */
    AddressSpacePtr findFirstAddressSpace(AddressSpacePurpose, const std::string &name) const;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Low-level operations on address spaces.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
    /** Read a value from an address space.
     *
     *  Reads a value from the specified address space at the specified address. If the address space does not hold a value for that
     *  address then the specified default value is written to the address space and returned.  The default value also determines
     *  how many bytes are read from the address space.
     *
     *  The `addrOps` and `valOps` arguments provide the operators that might be needed for the address and value, respectively.
     *  For instance, when reading multiple bytes from a memory address space, the address operators are used to increment the
     *  starting address and the value operators are used to concatenate the individual bytes. Oftentimes the address and value
     *  operators are the same.
     *
     *  The specified address space need not be an address space that's owned by either of the provided operator arguments, although
     *  it usually is owned by one or the other.
     *
     *  See also @ref peek, which has no side effects. */
    virtual SValuePtr read(const AddressSpacePtr&, const AddressSpaceAddress&, const SValuePtr &dflt,
                           RiscOperators &addrOps, RiscOperators &valOps);

    /** Peek at a value in an address space.
     *
     *  This is similar to the @ref read method except it has no side effects. See that method for more complete documentation. */
    virtual SValuePtr peek(const AddressSpacePtr&, const AddressSpaceAddress&, const SValuePtr &dflt,
                           RiscOperators &addrOps, RiscOperators &valOps);

    /** Write a value to an address space.
     *
     *  The specified value is written to the address space at the specified address.
     *
     *  The `addrOps` and `valOps` arguments provide the operators that might be needed for the address and value, respectively.
     *  For instance, when writing multiple bytes to a memory address space, the address operators are used to increment the
     *  starting address and the value operators are used to extract the individual byte values. Oftentimes the address and value
     *  operators are the same.
     *
     *  The specified address space need not be an address space that's owned by either of the provided operator arguments, although
     *  it usually is owned by one or the other. */
    virtual void write(const AddressSpacePtr&, const AddressSpaceAddress&, const SValuePtr &value,
                       RiscOperators &addrOps, RiscOperators &valOps);

    /** Merge operation for data flow analysis.
     *
     *  Merges the @p other state into this state. Returns true if this state changed, false otherwise.  This method usually
     *  isn't overridden in subclasses since all the base implementation does is invoke the merge operation on each of the
     *  address spaces.
     *
     *  Every address space of `other` is merged into this state as follows: if this state has an address space with the same
     *  purpose and name the first such address space is chosen to be the target and the other address space is merged into the
     *  target. Otherwise, when this state has no matching target, the other address space is deep-copied into this state.  Address
     *  spaces in this state that were not used as targets for any step are not modified by this function.
     *
     *  Returns true if this state was modified in any way, and false if no modifications occurred. */
    virtual bool merge(const StatePtr &other, RiscOperators *addrOps, RiscOperators *valOps);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Operations on address spaces found by searching.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
    /** Initialize state.
     *
     *  All attached address spaces are cleared by calling their @ref AddressSpace::clear "clear" method, removing all addresses and
     *  values. The address spaces themselves are not removed from this state. */
    virtual void clear();

    /** Initialize all registers to zero.
     *
     *  Calls the @ref BaseSemantics::RegisterState::zero method on each address space that inherits from @ref
     *  BaseSemantics::RegisterState and whose purpose is @ref AddressSpace::Purpose "REGISTERS". This causes the values for each
     *  register to be set to a concrete zero value. */
    void zeroRegisters();

    /** Clear all memory locations.
     *
     *  Calls the @ref BaseSemantics::MemoryState::clear method on each address space that inherits from @ref
     *  BaseSemantics::MemoryState. This causes the addresses and values to be removed from the memory states but does not remove
     *  the memory from this state. */
    void clearMemory();

    /** Property: Register state.
     *
     *  Returns the first address space of type @ref BaseSemantics::RegisterState whose purpose is @ref AddressSpace::Purpose
     *  "REGISTERS" from this state's list of address spaces. If there is no such address space then a null pointer is returned. */
    RegisterStatePtr registerState() const;

    /** Property: Memory state.
     *
     *  Returns the first address space of type @ref BaseSemantics::MemoryState whose purpose is @ref AddressSpace::Purpose
     *  "AddressSpace::Purpose::MEMORY" from this state's list of address spaces. If there is no such address space then a null
     *  pointer is returned. */
    MemoryStatePtr memoryState() const;

    /** Property: Interrupt state.
     *
     *  Returns the first address space of type @ref BaseSemantics::RegisterState whose purpose is @ref
     *  BaseSemantics::AddressSpace::Purpose "INTERRUPTS" from this state's list of address spaces. If there is no such address
     *  space then a null pointer is returned.
     *
     *  The interrupt state is modeled as bit flags specifying whether the interrupt has been raised. Interrupts have concrete major
     *  and minor numbers that correspond to the @ref RegisterDescriptor major and minor properties. It is permissible for this
     *  state to have a null interrupt state, in which case the @ref BaseSemantics::RiscOperators do something else (such as throw
     *  an exception). */
    RegisterStatePtr interruptState() const;

    /** Property: Interrupt state.
     *
     *  If the specified state is non-null, then this function replaces the first address space of type @ref
     *  BaseSemantics::RegisterState whose purpose is @ref BaseSemantics::AddressSpace::Purpose "INTERRUPTS" from this state's list
     *  of address spaces with the specified address space. If there is no such existing address space then the specified space is
     *  appended to the list of address spaces.
     *
     *  If the specified state is null, then the first address space of type @ref BaseSemantics::RegisterState whose purpose is @ref
     *  BaseSemantics::AddressSpace::Purpose "INTERRUPTS" is removed from this state's list of address spaces. If there is no such
     *  existing state then this function does nothing. */
    void interruptState(const RegisterStatePtr&);

    /** Tests whether an interrupt state is present.
     *
     *  A state may have an interrupt state (similar to the register and memory states). This function returns true if and only if
     *  the interrupt state is present.
     *
     *  See also @ref interruptState, which returns a null pointer if the interrupt state is not present. */
    bool hasInterruptState() const;

    /** Property: Frame state.
     *
     *  Returns the first address space of type @ref BaseSemantics::FrameState whose purpose is @ref
     *  BaseSemantics::AddressSpace::Purpose "FRAMES" from this state's list of address spaces. If there is no such address
     *  space then a null pointer is returned.
     *
     *  The frame state contains an operand stack and a local variables array. */
    FrameStatePtr frameState() const;

    /** Tests whether an frame state is present.
     *
     *  A state may have an frame state. This function returns true if and only if the frame state is present.
     *
     *  See also @ref frameState, which returns a null pointer if the frame state is not present. */
    bool hasFrameState() const;

    /** Property: terminated.
     *
     *  The state is terminated when the root method returns. */
    void terminate();
    void terminate(SValuePtr returnValue);

    /** Tests whether a state is terminated. */
    bool isTerminated() const;

    /** Property: returnValue.
     *
     *  The value returned when the root method returns. */
    SValuePtr returnValue() const;

    virtual SValuePtr readLocal(size_t index);
    virtual void writeLocal(size_t index, const SValuePtr &value);

    /** Peek at an operand value.
     *
     *  Calls @ref peekOperand on the address space returned by @ref frameState, which must be non-null. */
    virtual SValuePtr peekOperand(size_t depth = 0);

    /** Push an operand value.
     *
     *  Calls @ref pushOperand on the address space returned by @ref frameState, which must be non-null. */
    virtual void pushOperand(const SValuePtr &value);

    /** Pop an operand value.
     *
     *  Calls @ref popOperand on the address space returned by @ref frameState, which must be non-null. */
    virtual SValuePtr popOperand();

    /** Pushes a method invocation frame. */
    virtual void pushFrame(const FrameStatePtr &frame);

    /** Pops and returns the current method invocation frame. */
    virtual FrameStatePtr popFrame();

    /** Returns the current method invocation frame. */
    virtual FrameStatePtr currentFrame() const;

    /** Read a value from a register.
     *
     *  Calls @ref read on the address space returned by @ref registerState, which must be non-null. */
    virtual SValuePtr readRegister(RegisterDescriptor desc, const SValuePtr &dflt, RiscOperators *ops);

    /** Read register without side effects.
     *
     *  Calls @ref peek on the address space returned by @ref registerState, which must be non-null. */
    virtual SValuePtr peekRegister(RegisterDescriptor desc, const SValuePtr &dflt, RiscOperators *ops);

    /** Write a value to a register.
     *
     *  Calls @ref write on the address space returned by @ref registerState, which must be non-null. */
    virtual void writeRegister(RegisterDescriptor desc, const SValuePtr &value, RiscOperators *ops);

    /** Read a value from memory.
     *
     *  Calls @ref read on the address space returned by @ref memoryState, which must be non-null. */
    virtual SValuePtr readMemory(const SValuePtr &address, const SValuePtr &dflt,
                                 RiscOperators *addrOps, RiscOperators *valOps);

    /** Read from memory without side effects.
     *
     *  Calls @ref peek on the address space returned by @ref memoryState, which must be non-null. */
    virtual SValuePtr peekMemory(const SValuePtr &address, const SValuePtr &dflt, RiscOperators *addrOps, RiscOperators *valOps);

    /** Write a value to memory.
     *
     *  Calls @ref write on the address space returned by @ref memoryState, which must be non-null. */
    virtual void writeMemory(const SValuePtr &addr, const SValuePtr &value, RiscOperators *addrOps, RiscOperators *valOps);

    /** Read an interrupt state.
     *
     *  Calls @ref read on the address space returned by @ref interruptState, if any. If there is no interrupt state then this
     *  function returns null.
     *
     *  The default value `dflt` value will be written to the interrupt state if that interrupt has no previous value. The default
     *  value and the return value (when non-null) are always Boolean values (single bits) to indicate whether that interrupt is
     *  raised. */
    virtual SValuePtr readInterrupt(unsigned major, unsigned minor, const SValuePtr &dflt, RiscOperators *valOps);

    /** Read an interrupt state without side effects.
     *
     *  Calls @ref peek on the address space returned by @ref interruptState, if any. If there is no interrupt state then this
     *  function returns null.
     *
     *  The default value `dflt` value will be written to the interrupt state if that interrupt has no previous value. The default
     *  value and the return value (when non-null) are always Boolean values (single bits) to indicate whether that interrupt is
     *  raised. */
    virtual SValuePtr peekInterrupt(unsigned major, unsigned minor, const SValuePtr &dflt, RiscOperators *valOps);

    /** Write an interrupt state.
     *
     *  Calls @ref write on the address space returned by @ref interruptState, if any. If there is no interrupt state then this
     *  function returns false.
     *
     *  The `value` argument is Boolean (single bit) to indicate whether the interrupt has been raised. */
    virtual bool writeInterrupt(unsigned major, unsigned minor, const SValuePtr &value, RiscOperators *valOps);

    /** Raise an interrupt.
     *
     *  Raises an interrupt by writing a concrete true value to the address space returned by @ref interruptState, if any, and
     *  returns the interupt state's previous value. If there is no interrupt state then this function returns null. */
    SValuePtr raiseInterrupt(unsigned major, unsigned minor, RiscOperators *valOps);

    /** Clear an interrupt.
     *
     *  Clears an interrupt by writing a concrete false value to the address space returned by @ref interruptState, if any. If
     *  the interrupt state exists then the return value is the old Boolean value for that interrupt; otherwise this function
     *  returns a null pointer. */
    SValuePtr clearInterrupt(unsigned major, unsigned minor, RiscOperators *valOps);

    /** Test an interrupt.
     *
     *  Returns true if this state has an interrupt address space returned by @ref interruptState, and the specified interrupt in
     *  that substate has a concrete true value. Returns false in all other cases.
     *
     *  If the caller needs to handle values other than concrete true and false (such as unknown states) then use @ref readInterrupt
     *  or @ref peekInterrupt instead. */
    bool isInterruptDefinitelyRaised(unsigned major, unsigned minor, RiscOperators *valOps);

    /** Test an interrupt.
     *
     *  Returns true if this state has an interrupt sub-state returned by @ref interruptState and the specified interrupt in that
     *  substate has a concrete false value. Returns false in all other cases.
     *
     *  If the caller needs to handle values other than concrete true and false (such as unknown states) then use @ref readInterrupt
     *  or @ref peekInterrupt instead. */
    bool isInterruptDefinitelyClear(unsigned major, unsigned minor, RiscOperators *valOps);

    /** Compute a hash of the state.
     *
     *  The hash is computed across all address spaces that are part of this state. That typically includes register values,
     *  memory addresses and values, and interrupts. */
    virtual void hash(Combinatorics::Hasher&, RiscOperators *addrOps, RiscOperators *valOps) const;

    /** Print the register contents.
     *
     *  This method emits one line per register and contains the register name and its value. It gets the register state by calling
     *  @ref registerState.
     *
     * @{ */
    void printRegisters(std::ostream &stream, const std::string &prefix = "");
    virtual void printRegisters(std::ostream &stream, Formatter &fmt) const;
    /** @} */

    /** Print memory contents.
     *
     *  This simply calls the MemoryState::print method for the address space returned by @ref memoryState, if any.
     *
     * @{ */
    void printMemory(std::ostream &stream, const std::string &prefix = "") const;
    virtual void printMemory(std::ostream &stream, Formatter &fmt) const;
    /** @} */

    /** Print interrupt states.
     *
     *  This method emits one line per interrupt and contains the interrupt major and minor numbers and a Boolean expression
     *  indicating whether the interrupt is currently raised. It uses the address space returned by @ref interruptState, if any.
     *
     * @{ */
    void printInterrupts(std::ostream&, const std::string &prefix = "");
    virtual void printInterrupts(std::ostream &stream, Formatter &fmt) const;
    /** @} */

    /** Print the state.
     *
     *  This emits a multi-line string describing the contents of all address spaces. This typically includes an address space
     *  for the registers and another representing the memory. Other address spaces may be present also.
     *
     * @{ */
    void print(std::ostream &stream, const std::string &prefix = "") const;
    virtual void print(std::ostream&, Formatter&) const;
    /** @} */

    /** Convert the state to a string for debugging.
     *
     *  This is intended mainly for debugging so that you can easily print the state from within a debugger. */
    std::string toString() const;

    /** State with formatter. See with_formatter(). */
    class WithFormatter {
        StatePtr obj;
        Formatter &fmt;
    public:
        WithFormatter(const StatePtr &obj, Formatter &fmt): obj(obj), fmt(fmt) {}
        void print(std::ostream &stream) const { obj->print(stream, fmt); }
    };

    /** Used for printing states with formatting. The usual way to use this is:
     * @code
     *  StatePtr obj = ...;
     *  Formatter fmt = ...;
     *  std::cout <<"The value is: " <<(*obj+fmt) <<"\n";
     * @endcode
     *
     * Since specifying a line prefix string for indentation purposes is such a common use case, the
     * indentation can be given instead of a format, as in the following code that indents the
     * prefixes each line of the expression with four spaces.
     *
     * @code
     *  std::cout <<"Current state:\n" <<*(obj + "    ");
     * @endcode
     * @{ */
    WithFormatter with_format(Formatter &fmt) { return WithFormatter(shared_from_this(), fmt); }
    WithFormatter operator+(Formatter &fmt) { return with_format(fmt); }
    WithFormatter operator+(const std::string &linePrefix);
    /** @} */

    /** Merge operation for data flow analysis.
     *
     *  Merges the @p other state into this state. Returns true if this state changed, false otherwise.  This method usually
     *  isn't overridden in subclasses since all the base implementation does is invoke the merge operation on the memory state
     *  and register state. */
    virtual bool merge(const StatePtr &other, RiscOperators *ops);
};

std::ostream& operator<<(std::ostream&, const State&);
std::ostream& operator<<(std::ostream&, const State::WithFormatter&);

} // namespace
} // namespace
} // namespace
} // namespace

#ifdef ROSE_ENABLE_BOOST_SERIALIZATION
BOOST_CLASS_VERSION(Rose::BinaryAnalysis::InstructionSemantics::BaseSemantics::State, 2);
BOOST_CLASS_EXPORT_KEY(Rose::BinaryAnalysis::InstructionSemantics::BaseSemantics::State);
#endif

#endif
#endif
