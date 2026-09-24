#ifndef ROSE_BinaryAnalysis_InstructionSemantics_BaseSemantics_Dispatcher_H
#define ROSE_BinaryAnalysis_InstructionSemantics_BaseSemantics_Dispatcher_H
#include <featureTests.h>
#ifdef ROSE_ENABLE_BINARY_ANALYSIS

#include <Rose/BinaryAnalysis/BasicTypes.h>
#include <Rose/BinaryAnalysis/ByteCode/Analysis.h>
#include <Rose/BinaryAnalysis/InstructionSemantics/BaseSemantics/RiscOperators.h>

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
//                                      Instruction Dispatcher
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Functor that knows how to dispatch a single kind of instruction. */
class InsnProcessor {
public:
    virtual ~InsnProcessor() {}
    virtual void process(const DispatcherPtr &dispatcher, SgAsmInstruction *insn) = 0;
};
    
/** Dispatches instructions through the RISC layer.
 *
 *  The dispatcher is the instruction semantics entity that translates a high-level architecture-dependent instruction into a
 *  sequence of RISC operators whose interface is defined by ROSE. These classes are the key in ROSE's ability to connect a
 *  variety of instruction set architectures to a variety of semantic domains.
 *
 *  Each dispatcher contains a table indexed by the machine instruction "kind" (e.g., SgAsmMipsInstruction::get_kind()). The
 *  table stores functors derived from the abstract InsnProcessor class.  (FIXME: The functors are not currently reference
 *  counted; they are owned by the dispatcher and deleted when the dispatcher is destroyed. [Robb Matzke 2013-03-04])
 *
 *  Dispatcher objects are allocated on the heap and reference counted.  The BaseSemantics::Dispatcher is an abstract class
 *  that defines the interface.  See the Rose::BinaryAnalysis::InstructionSemantics namespace for an overview of how the parts
 *  fit together. */
class Dispatcher: public boost::enable_shared_from_this<Dispatcher> {
public:
    /** Shared-ownership pointer. */
    using Ptr = DispatcherPtr;

private:
    Architecture::BaseConstPtr architecture_; // Required architecture
    RiscOperatorsPtr operators_;
    uint32_t nextObjectId_ = 1; // next unique object identifier

    /** Non-owning pointer to the ByteCode class repository maintained by the Engine. */
    const ByteCode::ClassRepository* classes_ = nullptr;

protected:
    bool autoResetInstructionPointer_ = true;           /**< Reset instruction pointer register for each instruction. */

    // Dispatchers keep a table of all the kinds of instructions they can handle.  The lookup key is typically some sort of
    // instruction identifier, such as from SgAsmX86Instruction::get_kind(), and comes from the iprocKey() virtual method.
    typedef std::vector<InsnProcessor*> InsnProcessors;
    InsnProcessors iproc_table;

public:
    virtual int8_t   asS1(const SgAsmExpression* expr);
    virtual uint8_t  asU1(const SgAsmExpression* expr);
    virtual int16_t  asS2(const SgAsmExpression* expr);
    virtual uint16_t asU2(const SgAsmExpression* expr);
    virtual int32_t  asS4(const SgAsmExpression* expr);

    // Make a constant number_
    virtual SValuePtr makeConstant(ValueKind kind, int64_t value);

#ifdef ROSE_ENABLE_BOOST_SERIALIZATION
private:
    friend class boost::serialization::access;

    template<class S>
    void serializeCommon(S &s, const unsigned version) {
        ASSERT_always_require(version >= 2);
        s & boost::serialization::make_nvp("operators", operators_); // for backward compatibility
        s & BOOST_SERIALIZATION_NVP(nextObjectId_);
        s & BOOST_SERIALIZATION_NVP(autoResetInstructionPointer_);
        //s & iproc_table; -- not saved
    }

    template<class S>
    void save(S &s, const unsigned version) const {
        const_cast<Dispatcher*>(this)->serializeCommon(s, version);
        ASSERT_not_null(architecture_);
        const std::string architecture = Architecture::name(architecture_);
        s & BOOST_SERIALIZATION_NVP(architecture);
    }

    template<class S>
    void load(S &s, const unsigned version) {
        serializeCommon(s, version);
        std::string architecture;
        s & BOOST_SERIALIZATION_NVP(architecture);
        architecture_ = Architecture::findByName(architecture).orThrow();
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER();
#endif
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Real constructors
protected:
    Dispatcher();                                       // used only by boost::serialization

    // Prototypical constructor
    explicit Dispatcher(const Architecture::BaseConstPtr&);

    Dispatcher(const Architecture::BaseConstPtr&, const RiscOperatorsPtr&);

public:
    virtual ~Dispatcher();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Static allocating constructors. None since this is an abstract class


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Virtual constructors
public:
    /** Virtual constructor. */
    virtual DispatcherPtr create(const RiscOperatorsPtr &ops) const = 0;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Methods to process instructions
public:
    /** Process a single instruction.
     *
     *  This is the function that most users will call. */
    virtual void processInstruction(SgAsmInstruction *insn);

    /** Process a delay slot.
     *
     *  This is called when one instruction must temporarily interrupt its own processing to process some other instruction. It can
     *  be called recursively as often as needed, such as on architectures where there are multiple delay slots for some
     *  instructions.  Users don't normally call this--it's intended to be called by the instruction semantics processing layers. */
    virtual void processDelaySlot(SgAsmInstruction *delayInsn);

protected:
    // The part of processing instructions that's common to both `processInstruction` and `processDelaySlot`.
    virtual void processCommon();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Instruction processor table operations
public:
    /** Lookup the processor for an instruction.  Looks up the functor that has been registered to process the given
     *  instruction. Returns the null pointer if the instruction cannot be processed. Instruction processor objects are
     *  managed by the caller; the instruction itself is only used for the duration of this call. */
    virtual InsnProcessor *iprocLookup(SgAsmInstruction *insn);

    /** Replace an instruction processor with another.  The processor for the specified instruction is replaced with the
     *  specified processor, which may be the null pointer.  Instruction processor objects are managed by the caller; the
     *  instruction itself is only used for the duration of this call. */
    virtual void iprocReplace(SgAsmInstruction *insn, InsnProcessor *iproc);

    /** Given an instruction, return the InsnProcessor key that can be used as an index into the iproc_table. */
    virtual int iprocKey(SgAsmInstruction*) const = 0;

    /** Set an iproc table entry to the specified value.
     *
     *  The @p iproc object will become owned by this dispatcher and deleted when this dispatcher is destroyed. */
    virtual void iprocSet(int key, InsnProcessor *iproc);

    /** Obtain an iproc table entry for the specified key. */
    virtual InsnProcessor *iprocGet(int key);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Convenience methods that defer the call to some member object
public:
    /** Property: Architecture.
     *
     *  Non-null pointer to architecture-specific information. */
    Architecture::BaseConstPtr architecture() const;

    /** Property: RISC operators.
     *
     *  The RISC operators also contain the current and initial state on which they operate.
     *
     * @{ */
    virtual RiscOperatorsPtr operators() const;
    virtual void operators(const RiscOperatorsPtr&);
    /** @} */

    /** Get a pointer to the state object. The state is stored in the RISC operators object, so this is just here for
     *  convenience. */
    virtual StatePtr currentState() const;

    /** Return the prototypical value.  The prototypical value comes from the RISC operators object. */
    virtual SValuePtr protoval() const;

    /** Returns the instruction that is being processed.
     *
     *  The instruction comes from the @ref RiscOperators::currentInstruction "currentInstruction" method of the RiscOperators
     *  object. */
    virtual SgAsmInstruction* currentInstruction() const;

    /** Returns a pointer to the ByteCode::Class repository.
     *
     *  The repository is maintained by the Engine. */
    const ByteCode::ClassRepository& classRepository() const;
    void classRepository(const ByteCode::ClassRepository *repo);

    /** Return a new undefined semantic value.
     *
     * @{ */
    virtual SValuePtr undefined_(size_t nbits) const;
    virtual SValuePtr unspecified_(size_t nbits) const;
    /** @} */

    /** Return a semantic value representing a number. */
    virtual SValuePtr number_(size_t nbits, uint64_t number) const;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Methods related to registers
public:
    /** Property: Register dictionary.
     *
     *  The register dictionary translates register descriptors to names and vice versa and provides descriptors for common
     *  registers such as instruction and stack pointer registers.
     *
     *  This function is simply a wrapper that obtains the register dictionary from the architecture. */
    RegisterDictionaryPtr registerDictionary() const /*final*/;

    /** Lookup a register by name.  This dispatcher's register dictionary is consulted and the specified register is located by
     *  name.  If a bit width is specified (@p nbits) then it must match the size of register that was found.  If a valid
     *  register cannot be found then either an exception is thrown or an invalid register is returned depending on whether
     *  @p allowMissing is false or true, respectively. */
    virtual RegisterDescriptor findRegister(const std::string &regname, size_t nbits=0, bool allowMissing=false) const;

    /** Property: Width of memory addresses in bits.
     *
     *  This property defines the width of memory addresses. All memory reads and writes (and any other defined memory
     *  operations) should pass address expressions that are this width. */
    size_t addressWidth() const;

    /** Returns the instruction pointer register. */
    virtual RegisterDescriptor instructionPointerRegister() const;

    /** Returns the stack pointer register. */
    virtual RegisterDescriptor stackPointerRegister() const;

    /** Returns the stack call frame register. */
    virtual RegisterDescriptor stackFrameRegister() const;

    /** Returns the function call return address register. */
    virtual RegisterDescriptor callReturnRegister() const;

    /** Property: Reset instruction pointer register for each instruction.
     *
     *  If this property is set, then each time an instruction is processed, the first thing that happens is that the
     *  instruction pointer register is reset to the concrete address of the instruction.
     *
     * @{ */
    bool autoResetInstructionPointer() const { return autoResetInstructionPointer_; }
    void autoResetInstructionPointer(bool b) { autoResetInstructionPointer_ = b; }
    /** @} */
    
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Miscellaneous methods that tend to be the same for most dispatchers
public:
    /** Initialize the state.
     *
     *  Some architectures benefit from having their initial state initialized in a certain way. For instance, on x86/amd64 the
     *  segment registers CS, DS, and SS typically refer to the entire machine memory and can be initialized to have a zero
     *  base address. */
    virtual void initializeState(const StatePtr&);

    /** Sets non-owning pointer to the ByteCode class repository maintained by the Engine. */
    void classes(const ByteCode::ClassRepository *classes);

    /** Allocates a new unique object identifier. */
    uint32_t allocateObjectId();

    /** Completes a return from the current method.
     *
     *  Pops the current frame, pushes @p result onto the caller's operand stack
     *  when non-null, and restores the caller's instruction pointer. A frame
     *  without a return address is treated as the root analysis frame.
     */
    void completeReturn(BaseSemantics::RiscOperators*,
                        const BaseSemantics::SValuePtr result = BaseSemantics::SValuePtr());

    /** Update the instruction pointer register.
     *
     *  Causes the instruction pointer register to point to the address following the specified instruction.  Since every
     *  instruction has a concrete address, we could simply set the instruction pointer to that concrete address. However, some
     *  analyses depend on having an instruction pointer value that's built up by processing one instruction after
     *  another.  Therefore, if we can recognize the register state implementation and determine that the instruction pointer
     *  registers' value is already stored, we'll increment that value, which might result in a concrete value depending on the
     *  semantic domain. Otherwise we just explicitly assign a new concrete value to that register. */
    virtual void advanceInstructionPointer(SgAsmInstruction*);

    /** Returns a register descriptor for the segment part of a memory reference expression.  Many architectures don't use
     *  segment registers (they have a flat virtual address space), in which case the returned register descriptor's is_valid()
     *  method returns false. */
    virtual RegisterDescriptor segmentRegister(SgAsmMemoryReferenceExpression*);

    /** Increment all auto-increment registers in the expression.  This method traverses the expression and increments each
     *  the register of each register reference expression that has a positive adjustment value.  If the same register is
     *  encountered multiple times then it is incremented multiple times. */
    virtual void incrementRegisters(SgAsmExpression*);

    /** Decrement all auto-decrement registers in the expression.  This method traverses the expression and increments each
     *  the register of each register reference expression that has a negative adjustment value.  If the same register is
     *  encountered multiple times then it is decremented multiple times. */
    virtual void decrementRegisters(SgAsmExpression*);

    /** Update registers for pre-add expressions.
     *
     *  For each SgAsmBinaryAddPreupdate, add the lhs and rhs operands and assign the sum to the lhs, which must be a register
     *  reference expression. */
    virtual void preUpdate(SgAsmExpression*, const BaseSemantics::SValuePtr &enabled);

    /** Update registers for post-add expressions.
     *
     *  For each SgAsmBinaryAddPostupdate, add the lhs and rhs operands and assign the sum to the lhs, which must be a register
     *  reference expression. */
    virtual void postUpdate(SgAsmExpression*, const BaseSemantics::SValuePtr &enabled);

    /** Returns a memory address by evaluating the address expression.  The address expression can be either a constant or an
     *  expression containing operators and constants.  If @p nbits is non-zero then the result is sign extended or truncated
     *  to the specified width, otherwise the returned SValue is the natural width of the expression. */
    virtual SValuePtr effectiveAddress(SgAsmExpression*, size_t nbits=0);

    /** Reads an R-value expression.  The expression can be a constant, register reference, or memory reference.  The width of
     *  the returned value is specified by the @p value_nbits argument, and if this argument is zero then the width of the
     *  expression type is used.  The width of the address passed to lower-level memory access functions is specified by @p
     *  addr_nbits.  If @p addr_nbits is zero then the natural width of the effective address is passed to lower level
     *  functions. */
    virtual SValuePtr read(SgAsmExpression*, size_t value_nbits=0, size_t addr_nbits=0);

    /** Writes to an L-value expression. The expression can be a register or memory reference.  The width of the address passed
     *  to lower-level memory access functions is specified by @p addr_nbits.  If @p addr_nbits is zero then the natural width
     *  of the effective address is passed to lower level functions. */
    virtual void write(SgAsmExpression*, const SValuePtr &value, size_t addr_nbits=0);
};

} // namespace
} // namespace
} // namespace
} // namespace

#ifdef ROSE_ENABLE_BOOST_SERIALIZATION
BOOST_CLASS_EXPORT_KEY(Rose::BinaryAnalysis::InstructionSemantics::BaseSemantics::Dispatcher);
BOOST_CLASS_VERSION(Rose::BinaryAnalysis::InstructionSemantics::BaseSemantics::Dispatcher, 2);
#endif

#endif
#endif
