#include <featureTests.h>
#ifdef ROSE_ENABLE_BINARY_ANALYSIS
#include <Rose/BinaryAnalysis/InstructionSemantics/DispatcherJvm.h>

#include <Rose/As.h>
#include <Rose/Affirm.h>
#include <Rose/BinaryAnalysis/Architecture/Base.h>
#include <Rose/BinaryAnalysis/ByteCode/Jvm.h>
#include <Rose/BinaryAnalysis/InstructionEnumsJvm.h>
#include <Rose/BinaryAnalysis/InstructionSemantics.h>
#include <Rose/BinaryAnalysis/InstructionSemantics/DescriptorParser.h>
#include <Rose/BinaryAnalysis/InstructionSemantics/BaseSemantics/Dispatcher.h>
#include <Rose/BinaryAnalysis/InstructionSemantics/BaseSemantics/Exception.h>
#include <Rose/BinaryAnalysis/InstructionSemantics/BaseSemantics/MemoryState.h>
#include <Rose/BinaryAnalysis/InstructionSemantics/BaseSemantics/RiscOperators.h>
#include <Rose/BinaryAnalysis/InstructionSemantics/BaseSemantics/State.h>
#include <Rose/BinaryAnalysis/InstructionSemantics/BaseSemantics/SValue.h>
#include <Rose/StringUtility/Diagnostics.h>

#include <SgAsmExpression.h>
#include <SgAsmIntegerValueExpression.h>
#include <SgAsmJvmInstruction.h>
#include <SgAsmOperandList.h>
#include <SgAsmValueExpression.h>

#include <sageInterface.h>
#include <Cxx_GrammarDowncast.h>

#define MOVE_ON 1

using namespace Rose::BinaryAnalysis::InstructionSemantics::BaseSemantics;
using namespace Sawyer::Message::Common;

namespace Rose {
namespace BinaryAnalysis {
namespace InstructionSemantics {

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Functors that handle individual JVM instruction kinds
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Jvm {

// An intermediate class that reduces the amount of typing in all that follows.  Its `process` method does some up-front checking,
// dynamic casting, and pointer dereferencing and then calls the `p` method that does the real work.
class P: public BaseSemantics::InsnProcessor {
public:
    using D = DispatcherJvm*;
    using Ops = RiscOperators*;
    using I = SgAsmJvmInstruction*;
    using Args = const SgAsmExpressionPtrList&;

    virtual void p(D, Ops, I, Args) = 0;

    // Perform the semantics for the specified instruction.
    virtual void process(const Dispatcher::Ptr &dispatcher_, SgAsmInstruction *insn_) override {
        DispatcherJvmPtr dispatcher = DispatcherJvm::promote(dispatcher_);
        RiscOperators::Ptr operators = dispatcher->operators();
        SgAsmJvmInstruction* insn = isSgAsmJvmInstruction(insn_);
        ASSERT_not_null(insn);
        ASSERT_require(insn == operators->currentInstruction());

        // Update the program counter/instruction pointer (PC) to point to the fall-through instruction before we process the
        // instruction, so that the semantics for individual instructions (like branches) can override this choice by assigning a
        // new value to PC.  However, we must be careful of PC-relative addressing and remember that we already incremented the PC.
        dispatcher->advanceInstructionPointer(insn);
        SgAsmExpressionPtrList &operands = insn->get_operandList()->get_operands();

        operators->comment("executing instruction core");
        p(dispatcher.get(), operators.get(), insn, operands);
    }

    void assert_args(I insn, Args args, size_t nargs) {
        if (args.size()!=nargs) {
            std::string mesg = "instruction must have " + StringUtility::plural(nargs, "arguments");
            throw BaseSemantics::Exception(mesg, insn);
        }
    }
};


// Auto-completed JVM instruction semantics skeleton for ROSE-style JVM semantics.
// Source for instruction descriptions: Java SE 26 JVM Specification, Chapter 6.5.
// Generated from the user's partial jvmSemantics.C and completed for the requested opcodes.
//
// The small JvmSemantics adapter functions referenced below intentionally isolate
// JVM-specific operations that depend on your surrounding framework: constant-pool
// access, object/array modeling, exception modeling, method return/branch effects,
// and category-1/category-2 stack manipulation.  Bind these helpers to the
// existing ROSE JVM semantics state where those facilities live.

namespace JvmSemantics {
    // Adapter declarations expected from the surrounding JVM semantics framework.
    using D = DispatcherJvm*;
    using Ops = RiscOperators*;
    using I = SgAsmJvmInstruction*;

    namespace MyDomain {

        class SValue: public BaseSemantics::SValue {
            SgType* type_ = nullptr;
            uint64_t arrayLength_ = 0;  // Track array length

        public:
            using Ptr = Sawyer::SharedPointer<SValue>;

            SgType* get_type() const { return type_; }
            void set_type(SgType* t) { type_ = t; }

            static Ptr promote(const InstructionSemantics::BaseSemantics::SValue::Ptr &v) {
                Ptr retval = as<SValue>(v);
                ASSERT_not_null(retval);
                return retval;
            }

            uint64_t getArrayLength() const { return arrayLength_; }
            void setArrayLength(uint64_t len) { arrayLength_ = len; }
        };

        BaseSemantics::SValue::Ptr arrayLength(Ops ops, BaseSemantics::SValue::Ptr arrayref) {
            auto myArray = MyDomain::SValue::promote(arrayref);
            if (myArray) {
                return ops->number_(32, myArray->getArrayLength());
            }
            throw std::runtime_error("Array reference is not a MyDomain::SValue");
        }

    class State: public Rose::BinaryAnalysis::InstructionSemantics::SymbolicSemantics::State {
    public:
        using Super = InstructionSemantics::SymbolicSemantics::State;
        using Ptr = boost::shared_ptr<State>;

    private:
        std::set<uint64_t> ownedMonitors_; // Track owned object addresses

    protected:
        // Real constructors
        State(const BaseSemantics::RegisterState::Ptr &registers,
              const BaseSemantics::MemoryState::Ptr &memory)
            : Super(registers, memory) {}

        State(const State &other)
            : Super(other), ownedMonitors_(other.ownedMonitors_) {}

    public:
        // Static allocating constructors
        static Ptr instance(const BaseSemantics::RegisterState::Ptr &registers,
                           const BaseSemantics::MemoryState::Ptr &memory) {
            return Ptr(new State(registers, memory));
        }

        static Ptr instance(const Ptr &other) {
            return Ptr(new State(*other));
        }

        // Virtual constructors
        virtual BaseSemantics::State::Ptr create(
            const BaseSemantics::RegisterState::Ptr &registers,
            const BaseSemantics::MemoryState::Ptr &memory) const override {
            return instance(registers, memory);
        }

        virtual BaseSemantics::State::Ptr clone() const override {
            return Ptr(new State(*this));
        }

        // Dynamic pointer cast
        static Ptr promote(const BaseSemantics::State::Ptr &x) {
            Ptr retval = Rose::as<State>(x);
            ASSERT_not_null(retval);
            return retval;
        }

        // Monitor ownership tracking
        bool ownsMonitor(const SValue::Ptr& objectref) {
            auto addr = objectref->toUnsigned();
            return addr && ownedMonitors_.find(*addr) != ownedMonitors_.end();
        }

        void acquireMonitor(const SValue::Ptr& objectref) {
            auto addr = objectref->toUnsigned();
            if (addr) ownedMonitors_.insert(*addr);
        }

        void releaseMonitor(const SValue::Ptr& objectref) {
            auto addr = objectref->toUnsigned();
            if (addr) ownedMonitors_.erase(*addr);
        }
    };

    } // namespace MyDomain

    uint8_t asU1(const SgAsmExpression* expr) {
        auto ivExpr = isSgAsmIntegerValueExpression(expr);
        ASSERT_not_null(ivExpr);
        return static_cast<uint8_t>(ivExpr->get_value());
    }

    using Args = std::vector<SgAsmExpression*>;

    SValue::Ptr nullReference(Ops ops);
    SValue::Ptr rem(Ops ops, SValue::Ptr a, SValue::Ptr b);
    SValue::Ptr convert(Ops ops, const char *op, SValue::Ptr a);

    void methodReturn(Ops ops, I insn);
    void methodReturn(Ops ops, I insn, SValue::Ptr value);

    void branch_goto_w(Ops ops, I insn, Args args);
    void branch_if_acmpeq(Ops ops, I insn, Args args);
    void branch_if_acmpne(Ops ops, I insn, Args args);
    void branch_if_icmpeq(Ops ops, I insn, Args args);
    void branch_if_icmpne(Ops ops, I insn, Args args);
    void branch_if_icmplt(Ops ops, I insn, Args args);
    void branch_if_icmpge(Ops ops, I insn, Args args);
    void branch_if_icmpgt(Ops ops, I insn, Args args);
    void branch_if_icmple(Ops ops, I insn, Args args);
    void branch_ifnonnull(Ops ops, I insn, Args args);
    void branch_ifnull(Ops ops, I insn, Args args);
    void branch_jsr(Ops ops, I insn, Args args);
    void branch_jsr_w(Ops ops, I insn, Args args);
    void branch_lookupswitch(Ops ops, I insn, Args args);
    void branch_tableswitch(Ops ops, I insn, Args args);
    void execute_anewarray(Ops ops, I insn, Args args);
    void execute_athrow(Ops ops, I insn, Args args);
    void execute_checkcast(Ops ops, I insn, Args args);
    void execute_instanceof(Ops ops, I insn, Args args);
    void execute_invokedynamic(Ops ops, I insn, Args args);
    void execute_monitorenter(Ops ops, I insn, Args args);
    void execute_monitorexit(Ops ops, I insn, Args args);
    void execute_multianewarray(Ops ops, I insn, Args args);
    void execute_newarray(Ops ops, I insn, Args args);
    void execute_putstatic(Ops ops, I insn, Args args);


    // Runtime-exception hooks. These are intended to be executable checks in the
    // concrete JVM semantics state, not documentation-only comments. A symbolic
    // implementation can fork states or emit path predicates; a concrete
    // implementation should raise/model the named Java exception.
    void throwIfNull(Ops ops, const char *exceptionName, SValue::Ptr ref);
    void throwIfArrayIndexOutOfBounds(Ops ops, SValue::Ptr arrayref, SValue::Ptr index);
    void throwIfArrayStoreNotCompatible(Ops ops, SValue::Ptr arrayref, SValue::Ptr value);
    void throwIfIntegerDivisorZero(Ops ops, SValue::Ptr divisor);
    void throwIfLongDivisorZero(Ops ops, SValue::Ptr divisor);
    void throwIfNegativeArraySize(Ops ops, SValue::Ptr count);
    void throwIfClassCastFails(Ops ops, I insn, Args args, SValue::Ptr objectref);
    void throwIfMonitorOwnershipInvalid(Ops ops, SValue::Ptr objectref);

    SValue::Ptr arrayLength(Ops ops, SValue::Ptr arrayref);
} // namespace JvmSemantics


// -----------------------------------------------------------------------------
// Default helper definitions.
//
// These definitions remove the unresolved adapter declarations from the generated
// file.  The simple stack/local/arithmetic cases are implemented directly using
// the ROSE-style `Ops` interface already used in the original file.  Operations
// that require JVM class-file state, constant-pool resolution, heap/array state,
// monitor ownership, or CFG successor construction call `jvmUnsupported` rather
// than silently doing the wrong thing.
// -----------------------------------------------------------------------------

namespace JvmSemantics {

    [[noreturn]] static void jvmUnsupported(const char *name) {
        throw std::logic_error(std::string("JVM semantic helper not bound to framework state: ") + name);
    }

    SValue::Ptr nullReference(Ops ops) {
        // The JVMS does not mandate a concrete representation for null.  This
        // default model uses a zero-valued reference-sized symbolic value.
        auto nullRef = ops->number_(32, 0);
        ASSERT_not_null(nullRef);

        nullRef->kind(ValueKind::ObjectReference);
        nullRef->symbolName("null");

        return nullRef;
    }

    SgAsmJvmConstantPool* constantPool(Ops ops) {
        auto state = ops->currentState();
        ASSERT_not_null(state);

        auto frame = state->currentFrame();
        ASSERT_not_null(frame);

        auto jvmMethod = ByteCode::JvmMethod::promote(frame->method());
        ASSERT_not_null(jvmMethod);

        auto pool = jvmMethod->constant_pool();
        ASSERT_not_null(pool);

        return pool;
    }

    SgAsmJvmConstantPoolEntry* constantPoolEntry(Ops ops, size_t index) {
        return constantPool(ops)->get_entry(index);
    }

    std::string constantPoolUtf8(Ops ops, size_t index) {
        auto entry = constantPoolEntry(ops, index);
        ASSERT_require(entry->get_tag() == SgAsmJvmConstantPoolEntry::CONSTANT_Utf8);

        auto pool = constantPool(ops);
        ASSERT_not_null(pool);

        return pool->get_utf8_string(index);
    }

    std::string className(Ops ops, size_t index) {
        auto entry = constantPoolEntry(ops, index);
        ASSERT_require(entry && entry->get_tag() == SgAsmJvmConstantPoolEntry::CONSTANT_Fieldref);

        entry = constantPoolEntry(ops, entry->get_class_index());
        return constantPoolUtf8(ops, entry->get_name_index());
    }

    std::string fieldName(Ops ops, size_t index) {
        auto entry = constantPoolEntry(ops, index);
        ASSERT_require(entry && entry->get_tag() == SgAsmJvmConstantPoolEntry::CONSTANT_Fieldref);

        entry = constantPoolEntry(ops, entry->get_name_and_type_index());
        return constantPoolUtf8(ops, entry->get_name_index());
    }

    std::string nameAndTypeDescriptor(Ops ops, size_t index) {
        auto entry = constantPoolEntry(ops, index);
        ASSERT_require(entry && entry->get_tag() == SgAsmJvmConstantPoolEntry::CONSTANT_NameAndType);

        return constantPoolUtf8(ops, entry->get_descriptor_index());
    }

    std::string descriptor(Ops ops, size_t index) {
        auto entry = constantPoolEntry(ops, index);
        ASSERT_require(entry && entry->get_tag() == SgAsmJvmConstantPoolEntry::CONSTANT_Fieldref);

        entry = constantPoolEntry(ops, entry->get_name_and_type_index());
        ASSERT_require(entry && entry->get_tag() == SgAsmJvmConstantPoolEntry::CONSTANT_NameAndType);

        return constantPoolUtf8(ops, entry->get_descriptor_index());
    }

    std::string methodDescriptor(Ops ops, size_t index) {
        auto entry = constantPoolEntry(ops, index);
        auto nameTypeIdx = entry->get_name_and_type_index();

        entry = constantPoolEntry(ops, nameTypeIdx);
        ASSERT_require(entry->get_tag() == SgAsmJvmConstantPoolEntry::CONSTANT_NameAndType);

        auto descIdx = entry->get_descriptor_index();
        entry = constantPoolEntry(ops, descIdx);
        ASSERT_require(entry->get_tag() == SgAsmJvmConstantPoolEntry::CONSTANT_Utf8);

        return constantPoolUtf8(ops, descIdx);
    }

    template<class UnaryFunc>
    void
    doUnaryOp(Ops ops, ValueKind kind, UnaryFunc func) {
        auto sval = ops->popOperand();

        if (sval->kind() != kind) {
            std::ostringstream msg;
            msg << "operand kind differs from " << kind;
            ASSERT_require2(false, msg.str());
        }

        auto result = func(sval);
        ASSERT_not_null(result);

        result->kind(kind);
        ops->pushOperand(result);
    }

    template<class BinaryFunc>
    void
    doBinaryOp(Ops ops, ValueKind kind, BinaryFunc func) {
        auto rhs = ops->popOperand();
        auto lhs = ops->popOperand();

        if (lhs->kind() != kind || rhs->kind() != kind) {
            std::ostringstream msg;
            msg << "lhs or rhs kinds differ from " << kind;
            ASSERT_require2(false, msg.str());
        }

        auto result = func(lhs, rhs);
        ASSERT_not_null(result);

        result->kind(kind);
        ops->pushOperand(result);
    }

    bool
    isReference(SValue::Ptr v) {
        return v->kind() == ValueKind::ObjectReference ||
               v->kind() == ValueKind::ArrayReference;
    }

    void methodReturn(Ops /*ops*/, I /*insn*/) { jvmUnsupported("methodReturn(void)"); }
    void methodReturn(Ops /*ops*/, I /*insn*/, SValue::Ptr /*value*/) { jvmUnsupported("methodReturn(value)"); }

    void branch_goto_w(Ops, I, Args) { jvmUnsupported("branch_goto_w"); }
    void branch_if_acmpeq(Ops, I, Args) { jvmUnsupported("branch_if_acmpeq"); }
    void branch_if_acmpne(Ops, I, Args) { jvmUnsupported("branch_if_acmpne"); }
    void branch_if_icmpeq(Ops, I, Args) { jvmUnsupported("branch_if_icmpeq"); }
    void branch_if_icmpne(Ops, I, Args) { jvmUnsupported("branch_if_icmpne"); }
    void branch_if_icmplt(Ops, I, Args) { jvmUnsupported("branch_if_icmplt"); }
    void branch_if_icmpge(Ops, I, Args) { jvmUnsupported("branch_if_icmpge"); }
    void branch_if_icmpgt(Ops, I, Args) { jvmUnsupported("branch_if_icmpgt"); }
    void branch_if_icmple(Ops, I, Args) { jvmUnsupported("branch_if_icmple"); }
    void branch_jsr(Ops, I, Args) { jvmUnsupported("branch_jsr"); }
    void branch_jsr_w(Ops, I, Args) { jvmUnsupported("branch_jsr_w"); }
    void branch_lookupswitch(Ops, I, Args) { jvmUnsupported("branch_lookupswitch"); }
    void branch_tableswitch(Ops, I, Args) { jvmUnsupported("branch_tableswitch"); }
    void execute_ret(Ops, I, Args) { jvmUnsupported("execute_ret"); }

    void execute_anewarray(Ops ops, I /*insn*/, Args args) {
        uint8_t b1 = asU1(args[0]);
        uint8_t b2 = asU1(args[0]);
        // index into constant pool for array component type
        int64_t constantPoolIndex = (static_cast<uint16_t>(b1) << 8) | static_cast<uint16_t>(b2);
        (void) constantPoolIndex; // defeat clang warning

        auto count = ops->popOperand();
        ASSERT_require(count->kind() == ValueKind::Integer32);

        // Create an ArrayReference and set properties
        auto arrayRef = ops->protoval();
        arrayRef->kind(ValueKind::ArrayReference);
        arrayRef->arrayLength(count->copy());
        arrayRef->typeDescriptor("<unknown-type>"); // obtain from constant pool

        ops->pushOperand(arrayRef);
    }

    void execute_iinc(Ops ops, size_t index, int32_t increment) {
        auto oldValue = ops->readLocal(index);
        ASSERT_require(oldValue->nBits() == 32);
        ASSERT_require(oldValue->kind() == ValueKind::Integer32);

        // A negative increment is represented as its 32-bit two's-complement bit
        // pattern so the semantic addition has normal JVM int wraparound behavior.
        auto delta = ops->number_(32, static_cast<uint32_t>(increment));
        delta->kind(ValueKind::Integer32);

        auto newValue = ops->add(oldValue, delta);
        ASSERT_require(newValue->nBits() == 32);
        newValue->kind(ValueKind::Integer32);

        ops->writeLocal(index, newValue);
    }

    void execute_ldc(Ops ops, size_t index) {
        ASSERT_not_null(ops);
        SValuePtr value;

        auto pool = DispatcherJvm::constantPool(ops);
        ASSERT_not_null(pool);

        auto entry = pool->get_entry(index);
        ASSERT_not_null(entry);

        switch (entry->get_tag()) {
            case SgAsmJvmConstantPoolEntry::CONSTANT_Integer:
                value = ops->number_(32, entry->get_bytes());
                value->kind(ValueKind::Integer32);
                break;

            case SgAsmJvmConstantPoolEntry::CONSTANT_Float:
                value = ops->number_(32, entry->get_bytes());
                value->kind(ValueKind::Float32);
                break;

            case SgAsmJvmConstantPoolEntry::CONSTANT_Long:
                value = ops->number_(64, entry->get_bytes());
                value->kind(ValueKind::Integer64);
                break;

            case SgAsmJvmConstantPoolEntry::CONSTANT_Double:
                value = ops->number_(64, entry->get_bytes());
                value->kind(ValueKind::Float64);
                break;

            case SgAsmJvmConstantPoolEntry::CONSTANT_String:
                value = DispatcherJvm::syntheticObjectReference(ops->protoval(), "Ljava/lang/String;");
                break;

            case SgAsmJvmConstantPoolEntry::CONSTANT_Class:
                value = DispatcherJvm::syntheticObjectReference(ops->protoval(), "Ljava/lang/Class;");
                break;

            default: ASSERT_not_implemented("unsupported constant-pool entry for execute_ldc");
        }

        ASSERT_not_null(value);
        ops->pushOperand(value);
    }

    void execute_getstatic(Ops ops, size_t index) {
        const std::string symName = className(ops, index) + "." + fieldName(ops, index);
        const std::string fieldDesc = descriptor(ops, index);

        // Determine the ValueKind of the field descriptor
        auto descType = DescriptorParser::parseFieldDescriptor(fieldDesc);
        auto kind = descType.kind;

        auto sval = ops->undefined_(32);
        sval->kind(kind);
        sval->symbolName(symName);
        sval->typeDescriptor(fieldDesc);

        ops->pushOperand(sval);
    }

    enum class InvocationKind {
        Special,
        Virtual,
        Static
    };

    void execute_invoke(Ops ops, size_t index, SgAsmInstruction *insn, InvocationKind invocationKind) {
        ASSERT_not_null(ops);
        ASSERT_not_null(insn);

        auto state = ops->currentState();
        ASSERT_not_null(state);

        auto callerFrame = state->currentFrame();
        ASSERT_not_null(callerFrame);

        auto method = callerFrame->method();
        ASSERT_not_null(method);

        auto jvmMethod = ByteCode::JvmMethod::promote(method);
        ASSERT_not_null(jvmMethod);
        auto pool = jvmMethod->constant_pool();
        ASSERT_not_null(pool);

        const std::string descriptor = DispatcherJvm::methodDescriptor(pool, index);
        const MethodDescriptor methodDesc = DescriptorParser::parseMethodDescriptor(descriptor);
        const bool hasReceiver = invocationKind != InvocationKind::Static;

        // Need way to find ByteCode::Method for callee
        auto calleeFrame = FrameState::instance(state->protoval(), Sawyer::Nothing(), ByteCode::Method::Ptr());
        ASSERT_not_null(calleeFrame);

        // Pops arguments and, for instance methods, the receiver from the
        // caller's stack and installs them in the callee's locals.
        DispatcherJvm::initializeInvocationLocals(ops, calleeFrame, descriptor, hasReceiver);

        const Address callerResumeAddress = insn->get_address() + insn->get_size();
        calleeFrame->returnAddress(callerResumeAddress);

        // For now, represent the invocation frame lifecycle without
        // interpreting the callee.
        state->pushFrame(calleeFrame);

        const auto poppedFrame = state->popFrame();
        ASSERT_require(poppedFrame == calleeFrame);

        // Since the callee was summarized rather than interpreted, create
        // an unknown result of the declared return type.
        if (!methodDesc.returnType.isVoid()) {
            auto result = DispatcherJvm::syntheticValue(state->protoval(), methodDesc.returnType);
            ASSERT_not_null(result);
            ops->pushOperand(result);
        }
    }

    enum class LocalKind {
        Integer32,
        Integer64,
        Float32,
        Float64,
        Reference
    };

    void execute_load(Ops ops, size_t index, LocalKind expectedKind) {
        ASSERT_not_null(ops);

        auto state = ops->currentState();
        ASSERT_not_null(state);

        auto frame = state->currentFrame();
        ASSERT_not_null(frame);

        auto value = frame->readLocal(index);
        ASSERT_not_null(value);

        switch (expectedKind) {
            case LocalKind::Integer32:
                ASSERT_require(value->kind() == ValueKind::Integer32);
                break;
            case LocalKind::Integer64:
                ASSERT_require(value->kind() == ValueKind::Integer64);
                break;
            case LocalKind::Float32:
                ASSERT_require(value->kind() == ValueKind::Float32);
                break;
            case LocalKind::Float64:
                ASSERT_require(value->kind() == ValueKind::Float64);
                break;
            case LocalKind::Reference:
                ASSERT_require(value->kind() == ValueKind::ObjectReference ||
                               value->kind() == ValueKind::ArrayReference);
                break;
        }

        ops->pushOperand(value);
    }

    static void execute_store(Ops ops, size_t index, LocalKind expectedKind) {
        ASSERT_not_null(ops);

        auto value = ops->popOperand();
        ASSERT_not_null(value);

        switch (expectedKind) {
            case LocalKind::Integer32:
                ASSERT_require(value->kind() == ValueKind::Integer32);
                break;
            case LocalKind::Integer64:
                ASSERT_require(value->kind() == ValueKind::Integer64);
                break;
            case LocalKind::Float32:
                ASSERT_require(value->kind() == ValueKind::Float32);
                break;
            case LocalKind::Float64:
                ASSERT_require(value->kind() == ValueKind::Float64);
                break;
            case LocalKind::Reference:
                ASSERT_require(value->kind() == ValueKind::ObjectReference ||
                               value->kind() == ValueKind::ArrayReference);
                break;
        }

        auto state = ops->currentState();
        ASSERT_not_null(state);

        auto frame = state->currentFrame();
        ASSERT_not_null(frame);

        frame->writeLocal(index, value);
    }

    enum class ArrayElementKind {
        Integer32,
        Integer64,
        Float32,
        Float64,
        Reference,
        ByteOrBoolean,
        Character,
        Short
    };

    // Synthesize the result of reading an array element
    SValuePtr readArrayElement(Ops ops, const SValuePtr &arrayRef, const SValuePtr &index, size_t nBits) {
        ASSERT_not_null(ops);
        ASSERT_not_null(arrayRef);
        ASSERT_not_null(index);

        ASSERT_require(arrayRef->kind() == ValueKind::ArrayReference);
        ASSERT_require(index->kind() == ValueKind::Integer32);

        // Array contents are not modeled yet.
        return ops->undefined_(nBits);
    }

    void execute_array_load(Ops ops, ArrayElementKind loadKind) {
        ASSERT_not_null(ops);

        auto index = ops->popOperand();
        ASSERT_not_null(index);
        ASSERT_require(index->kind() == ValueKind::Integer32);

        auto arrayRef = ops->popOperand();
        ASSERT_not_null(arrayRef);
        ASSERT_require(arrayRef->kind() == ValueKind::ArrayReference);

        const std::string descriptor = arrayRef->typeDescriptor();
        ASSERT_require(!descriptor.empty());
        ASSERT_require(descriptor[0] == '[');

        SValuePtr result;

        switch (loadKind) {
          case ArrayElementKind::Integer32:
              result = readArrayElement(ops, arrayRef, index, 32);
              result->kind(ValueKind::Integer32);
              break;
          case ArrayElementKind::Integer64:
              result = readArrayElement(ops, arrayRef, index, 64);
              result->kind(ValueKind::Integer64);
              break;
          case ArrayElementKind::Float32:
              result = readArrayElement(ops, arrayRef, index, 32);
              result->kind(ValueKind::Float32);
              break;
          case ArrayElementKind::Float64:
              result = readArrayElement(ops, arrayRef, index, 64);
              result->kind(ValueKind::Float64);
              break;
          case ArrayElementKind::Reference:
              result = readArrayElement(ops, arrayRef, index, 32);
              result->kind(ValueKind::ObjectReference);
              break;
          case ArrayElementKind::ByteOrBoolean: {
              ASSERT_require(descriptor == "[B" || descriptor == "[Z");

              auto value = readArrayElement(ops, arrayRef, index, 8);

              // byte[] requires sign extension; boolean[] requires
              // zero extension. The array descriptor must distinguish them.
              result = descriptor == "[Z"
                  ? ops->unsignedExtend(value, 32)
                  : ops->signExtend(value, 32);

              result->kind(ValueKind::Integer32);
              break;
          }
          case ArrayElementKind::Character: {
              auto value = readArrayElement(ops, arrayRef, index, 16);
              result = ops->unsignedExtend(value, 32);
              result->kind(ValueKind::Integer32);
              break;
          }
          case ArrayElementKind::Short: {
              auto value = readArrayElement(ops, arrayRef, index, 16);
              result = ops->signExtend(value, 32);
              result->kind(ValueKind::Integer32);
              break;
          }
        }

        ASSERT_not_null(result);
        ops->pushOperand(result);
    }

    void execute_array_store(Ops ops, ArrayElementKind storeKind) {
        ASSERT_not_null(ops);

        auto value = ops->popOperand();
        ASSERT_not_null(value);

        auto index = ops->popOperand();
        ASSERT_not_null(index);
        ASSERT_require(index->kind() == ValueKind::Integer32);

        auto arrayRef = ops->popOperand();
        ASSERT_not_null(arrayRef);
        ASSERT_require(arrayRef->kind() == ValueKind::ArrayReference);

        const std::string descriptor = arrayRef->typeDescriptor();
        ASSERT_require(!descriptor.empty());
        ASSERT_require(descriptor[0] == '[');

        switch (storeKind) {
            case ArrayElementKind::Integer32:
                ASSERT_require(descriptor == "[I");
                ASSERT_require(value->kind() == ValueKind::Integer32);
                break;
            case ArrayElementKind::Integer64:
                ASSERT_require(descriptor == "[J");
                ASSERT_require(value->kind() == ValueKind::Integer64);
                break;
            case ArrayElementKind::Float32:
                ASSERT_require(descriptor == "[F");
                ASSERT_require(value->kind() == ValueKind::Float32);
                break;
            case ArrayElementKind::Float64:
                ASSERT_require(descriptor == "[D");
                ASSERT_require(value->kind() == ValueKind::Float64);
                break;
            case ArrayElementKind::ByteOrBoolean:
                ASSERT_require(descriptor == "[B" || descriptor == "[Z");
                ASSERT_require(value->kind() == ValueKind::Integer32);
                break;
            case ArrayElementKind::Character:
                ASSERT_require(descriptor == "[C");
                ASSERT_require(value->kind() == ValueKind::Integer32);
                break;
            case ArrayElementKind::Short:
                ASSERT_require(descriptor == "[S");
                ASSERT_require(value->kind() == ValueKind::Integer32);
                break;
            case ArrayElementKind::Reference: {
                ASSERT_require(value->kind() == ValueKind::ObjectReference ||
                               value->kind() == ValueKind::ArrayReference);

                const std::string elementDescriptor = descriptor.substr(1);

                ASSERT_require(!elementDescriptor.empty());
                ASSERT_require(elementDescriptor[0] == 'L' ||
                               elementDescriptor[0] == '[');
                break;
            }
        }
        // Without an array-content model, the assignment is not retained.
    }

    Address branchTargetAddress(I insn, int32_t displacement) {
        ASSERT_not_null(insn);

        const int64_t target = static_cast<int64_t>(insn->get_address()) +
                               static_cast<int64_t>(displacement);
        ASSERT_require(target >= 0);

        return static_cast<Address>(target);
    }

    void execute_condition(D d, Ops ops, I insn, int16_t displacement, const SValuePtr &cond) {
        ASSERT_not_null(cond);

        const RegisterDescriptor pcReg = d->instructionPointerRegister();

        // PC has already advanced to the fall-through instruction.
        auto fallThrough = ops->readRegister(pcReg);
        ASSERT_not_null(fallThrough);

        auto targetAddr = JvmSemantics::branchTargetAddress(insn, displacement);
        auto target = ops->number_(pcReg.nBits(), targetAddr);
        ASSERT_not_null(target);
        ASSERT_require(target->nBits() == fallThrough->nBits());

        auto nextPc = ops->ite(cond, target, fallThrough);
        ops->writeRegister(pcReg, nextPc);
    }

    enum class NullBranchKind {
        IfNull,
        IfNonNull
    };

    void execute_null_branch(D d, Ops ops, I insn, Args args, NullBranchKind branchKind) {
        ASSERT_not_null(d);
        ASSERT_not_null(ops);
        ASSERT_not_null(insn);

        auto reference = ops->popOperand();
        ASSERT_not_null(reference);

        ASSERT_require(reference->kind() == ValueKind::ObjectReference ||
                       reference->kind() == ValueKind::ArrayReference);

        // This semantic model represents the null reference as zero.
        auto nullRef = JvmSemantics::nullReference(ops);
        ASSERT_not_null(nullRef);

        SValuePtr condition;

        switch (branchKind) {
            case NullBranchKind::IfNull:
                condition = ops->isEqual(reference, nullRef);
                break;
            case NullBranchKind::IfNonNull:
                condition = ops->isNotEqual(reference, nullRef);
                break;
        }
        ASSERT_not_null(condition);

        const RegisterDescriptor pcReg = d->instructionPointerRegister();

        // The dispatcher has already advanced PC to the fall-through address.
        auto fallThrough = ops->readRegister(pcReg);
        ASSERT_not_null(fallThrough);
        ASSERT_require(fallThrough->nBits() == pcReg.nBits());

        auto targetAddr = JvmSemantics::branchTargetAddress(insn, d->asS2(args[0]));
        auto target = ops->number_(pcReg.nBits(), targetAddr);
        ASSERT_not_null(target);
        ASSERT_require(target->nBits() == fallThrough->nBits());

        auto nextPc = ops->ite(condition, target, fallThrough);
        ops->writeRegister(pcReg, nextPc);
    }

    void execute_athrow(Ops, I, Args) { jvmUnsupported("execute_athrow"); }
    void execute_checkcast(Ops, I, Args) { jvmUnsupported("execute_checkcast"); }
    void execute_instanceof(Ops, I, Args) { jvmUnsupported("execute_instanceof"); }
    void execute_invokedynamic(Ops, I, Args) { jvmUnsupported("execute_invokedynamic"); }
    void execute_invokeinterface(Ops, I, Args) { jvmUnsupported("execute_invokeinterface"); }
    void execute_monitorenter(Ops, I, Args) { jvmUnsupported("execute_monitorenter"); }
    void execute_monitorexit(Ops, I, Args) { jvmUnsupported("execute_monitorexit"); }
    void execute_multianewarray(Ops, I, Args) { jvmUnsupported("execute_multianewarray"); }
    void execute_newarray(Ops, I, Args) { jvmUnsupported("execute_newarray"); }
    void execute_putstatic(Ops, I, Args) { jvmUnsupported("execute_putstatic"); }

    void throwIfNull(Ops /*ops*/, const char *exceptionName, SValue::Ptr ref) {
        if (ref && ref->get_number() == 0)
            throw std::runtime_error(exceptionName);
    }

    void throwIfArrayIndexOutOfBounds(Ops ops, SValue::Ptr arrayref, SValue::Ptr index) {
     // 1. Calculate the actual address being accessed
        SValue::Ptr accessAddr = ops->add(arrayref, index);

     // 2. Retrieve the concrete address value (if available)
        auto va = accessAddr->toUnsigned();
        if (!va) return; // Cannot check bounds for purely symolic addresses without a solver
#if 0
     // DQ (5/19/2026): Not clear how to implement this part (discuss with Craig and Robb).
     // 3. Get the intended region from the array reference
     // In PartitionerModel, SValues can carry region information
     // const AddressInterval referencedRegion = arrayref->region();
     // if (!referencedRegion) return;

     // Alternative to using Model Checking...
        namespace IntervalSemantics = Rose::BinaryAnalysis::InstructionSemantics::IntervalSemantics;

        if (IntervalSemantics::SValue::Ptr iv = IntervalSemantics::SValue::promote(abstractVal)) {
             const Intervals &possibleValues = iv->get_intervals();
          // Check if intervals are within your known array bounds
        }

     // 4. Check if the accessed address (and size, e.g., 1 byte) is within the region
        AddressInterval accessedRegion = AddressInterval::baseSize(*va, 1);
        if (!referencedRegion.contains(accessedRegion)) {
            throw std::runtime_error("ArrayIndexOutOfBoundsException");
        }
#endif
    }

    void throwIfArrayStoreNotCompatible(Ops /*ops*/, SValue::Ptr arrayref, SValue::Ptr value) {
        auto myValue = MyDomain::SValue::promote(value);
        auto myArray = MyDomain::SValue::promote(arrayref);

        if (myValue && myArray) {
            SgType* valType = myValue->get_type();
            if (valType && !SageInterface::isAssignable(valType)) {
                throw std::runtime_error("ArrayStoreException");
            }
        }
    }

    void throwIfIntegerDivisorZero(Ops /*ops*/, SValue::Ptr divisor) {
        if (divisor && divisor->get_number() == 0)
            throw std::runtime_error("ArithmeticException");
    }
    void throwIfLongDivisorZero(Ops ops, SValue::Ptr divisor) { throwIfIntegerDivisorZero(ops, divisor); }

    void throwIfNegativeArraySize(Ops /*ops*/, SValue::Ptr count) {
        if (count) {
            auto signedCount = count->toSigned();
            if (signedCount && *signedCount < 0) {
                throw std::runtime_error("NegativeArraySizeException");
            }
        }
    }

SgType* getTargetTypeFromInsn(I insn, Args /*args*/) {
    // Cast to JVM instruction
    auto jvmInsn = isSgAsmJvmInstruction(insn);
    if (!jvmInsn) return nullptr;

    // Only checkcast and instanceof have type targets
    switch (jvmInsn->get_kind()) {
        case JvmInstructionKind::checkcast:
        case JvmInstructionKind::instanceof: {
            // Get the first operand (constant pool index)
            if (auto expr = isSgAsmIntegerValueExpression(jvmInsn->get_operandList()->get_operands()[0])) {
                uint16_t classIndex = static_cast<uint16_t>(expr->get_value());
                (void) classIndex; // defeat clang warning

                // You need access to the constant pool here
                // This typically comes from the method or class context
                // SgAsmJvmConstantPool* pool = getConstantPoolFromContext();
                // std::string className = JvmClass::name(classIndex, pool);

                // Convert class name to SgType
                // return buildSgTypeFromClassName(className);
            }
            break;
        }
        default:
            return nullptr;
    }
    return nullptr;
}

bool isSubclassOf(SgClassType* derived, SgClassType* base);

bool isCastValid(SgType* objectType, SgType* targetType) {
    // 1. Null reference can always be cast to any reference type
    if (!objectType) return true;

    // 2. Exact type match is always valid
    if (objectType->isEquivalentType(targetType)) {
        return true;
    }

    // 3. For class types, check if objectType is a subclass of targetType
    if (auto objClass = isSgClassType(objectType)) {
        if (auto targetClass = isSgClassType(targetType)) {
            return isSubclassOf(objClass, targetClass);
        }
    }

    // 4. Handle array types (arrays can be cast to Object)
    if (isSgArrayType(objectType) && isSgClassType(targetType)) {
        // Arrays can be cast to Object or Cloneable
        // This is a simplified check - you may need more specific logic
        return true;
    }

    return false;
}

bool isSubclassOf(SgClassType* derived, SgClassType* base) {
    // Get class definitions
    SgClassDeclaration* derivedDecl = isSgClassDeclaration(derived->get_declaration());
    SgClassDeclaration* baseDecl = isSgClassDeclaration(base->get_declaration());

    if (!derivedDecl || !baseDecl) return false;

    SgClassDefinition* derivedDef = derivedDecl->get_definition();
    SgClassDefinition* baseDef = baseDecl->get_definition();

    if (!derivedDef || !baseDef) return false;

    // Check if they're the same class
    if (derivedDef == baseDef) return true;

    // Check base classes
    const SgBaseClassPtrList& bases = derivedDef->get_inheritances();
    for (SgBaseClass* baseClass : bases) {
        if (!baseClass->get_isDirectBaseClass()) continue;

        SgClassDeclaration* baseClassDecl = baseClass->get_base_class();
        if (!baseClassDecl) continue;

        SgClassDeclaration* baseClassDefDecl = isSgClassDeclaration(
            baseClassDecl->get_definingDeclaration());
        if (!baseClassDefDecl) continue;

        SgClassDefinition* baseClassDef = baseClassDefDecl->get_definition();
        if (!baseClassDef) continue;

        // Recursively check if this base class matches or is a subclass
        SgClassType* baseClassType = isSgClassType(baseClassDefDecl->get_type());
        if (baseClassType && isSubclassOf(baseClassType, base)) {
            return true;
        }
    }

    return false;
}

    void throwIfClassCastFails(Ops /*ops*/, I insn, Args args, SValue::Ptr objectref) {
     // if (objectref && objectref->classCastFails())
     //    throw std::runtime_error("ClassCastException");
     // 1. Promote to your custom domain's SValue to access type information
        auto myObject = MyDomain::SValue::promote(objectref);

        if (myObject) {
            // 2. Get the object's actual type (stored in your custom SValue)
            SgType* objectType = myObject->get_type();

            // 3. Get the target type from the instruction or args
            // This depends on how you're encoding the cast target in your JVM semantics
            SgType* targetType = getTargetTypeFromInsn(insn, args);

            // 4. Check if the cast is valid using ROSE's type system
            if (objectType && targetType && !isCastValid(objectType, targetType)) {
                throw std::runtime_error("ClassCastException");
            }
        }
    }

    void throwIfMonitorOwnershipInvalid(Ops ops, SValue::Ptr objectref) {
     // 1. Check if objectref is null (null reference never owns a monitor)
        if (!objectref) {
            throw std::runtime_error("IllegalMonitorStateException");
        }

     // 2. Get the current thread's monitor ownership state
        auto myState = MyDomain::State::promote(ops->currentState());

     // 3. Check if current thread owns this object's monitor
     // Promote objectref to MyDomain::SValue before passing to ownsMonitor
        auto myObjectRef = MyDomain::SValue::promote(objectref);
        if (!myState->ownsMonitor(myObjectRef)) {
            throw std::runtime_error("IllegalMonitorStateException");
        }
    }

    SValue::Ptr
    arrayLength(Ops /*ops*/, SValue::Ptr arrayRef) {
        ASSERT_require(arrayRef->kind() == ValueKind::ArrayReference);
        ASSERT_require(arrayRef->hasArrayLength());

        // Get the array length from the array and convert it to Integer kind
        SValuePtr length = arrayRef->arrayLength();
        length->kind(ValueKind::Integer32);
        return length;
    }
}

using JIK = JvmInstructionKind;
namespace JS = JvmSemantics;

using JvmSemantics::asU1;
using JvmSemantics::doBinaryOp;
using JvmSemantics::doUnaryOp;
using JvmSemantics::LocalKind;
using JvmSemantics::ArrayElementKind;
using JvmSemantics::InvocationKind;

// aaload (50 (0x32))
        // Description:
        //   Pop arrayref and index; load the reference component at index and push the value.
        // Run-time Exceptions:
        //   NullPointerException if arrayref is null.
        //   ArrayIndexOutOfBoundsException if index is outside the array bounds.
struct IP_aaload: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        execute_array_load(ops, ArrayElementKind::Reference);
    }
};

// aastore (83 (0x53))
        // Description:
        //   Pop value, index, and arrayref; store the reference value into the array component.
        // Run-time Exceptions:
        //   NullPointerException if arrayref is null.
        //   ArrayIndexOutOfBoundsException if index is outside the array bounds.
        //   ArrayStoreException if a non-null value is not assignment-compatible with the array component type.
struct IP_aastore: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        execute_array_store(ops, ArrayElementKind::Reference);
    }
};

// aconst_null (1 (0x01))
        // Description:
        //   Push the null object reference.
        // Notes:
        //   The JVM does not prescribe a concrete bit pattern for null.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_aconst_null: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        ops->pushOperand(JvmSemantics::nullReference(ops));
    }
};

// aload (25 (0x19))
        // Description:
        //   Load a reference value from a local variable and push it.
        // Notes:
        //   The _<n> form uses an implicit local-variable index.
        //   aload cannot load returnAddress values; astore can store them.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_aload: P {
    void p(D d, Ops ops, I insn, Args args) {
        assert_args(insn, args, 1);
        execute_load(ops, d->asU1(args[0]), LocalKind::Reference);
    }
};

// aload_0 (42 (0x2a))
        // Description:
        //   Load a reference value from a local variable and push it.
        // Notes:
        //   The _<n> form uses an implicit local-variable index.
        //   aload cannot load returnAddress values; astore can store them.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_aload_0: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        execute_load(ops, 0, LocalKind::Reference);
    }
};

// aload_1 (43 (0x2b))
        // Description:
        //   Load a reference value from a local variable and push it.
        // Notes:
        //   The _<n> form uses an implicit local-variable index.
        //   aload cannot load returnAddress values; astore can store them.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_aload_1: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        execute_load(ops, 1, LocalKind::Reference);
    }
};

// aload_2 (44 (0x2c))
        // Description:
        //   Load a reference value from a local variable and push it.
        // Notes:
        //   The _<n> form uses an implicit local-variable index.
        //   aload cannot load returnAddress values; astore can store them.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_aload_2: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        execute_load(ops, 2, LocalKind::Reference);
    }
};

// aload_3 (45 (0x2d))
        // Description:
        //   Load a reference value from a local variable and push it.
        // Notes:
        //   The _<n> form uses an implicit local-variable index.
        //   aload cannot load returnAddress values; astore can store them.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_aload_3: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        execute_load(ops, 3, LocalKind::Reference);
    }
};

// anewarray (189 (0xbd))
        // Description:
        //   Create a new array, using count values popped from the operand stack, and push the resulting array reference.
        // Run-time Exceptions:
        //   NegativeArraySizeException if any requested dimension is negative.
struct IP_anewarray: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 2);
        JvmSemantics::execute_anewarray(ops, insn, args);
    }
};

// areturn (176 (0xb0))
        // Description:
        //   Return from the current method; typed returns move the return value to the invoker frame, while return returns void.
        // Run-time Exceptions:
        //   IllegalMonitorStateException can be thrown for synchronized methods if structured locking ownership rules are violated.
struct IP_areturn: P {
    void p(D d, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);

        auto result = ops->popOperand();
        ASSERT_not_null(result);
        ASSERT_require(result->kind() == ValueKind::ObjectReference ||
                       result->kind() == ValueKind::ArrayReference);

        d->completeReturn(ops, result);
    }
};

// return_ (177 (0xb1))
        // Description:
        //   Return from the current method; typed returns move the return value to the invoker frame, while return returns void.
        // Run-time Exceptions:
        //   IllegalMonitorStateException can be thrown for synchronized methods if structured locking ownership rules are violated.
struct IP_return_: P {
    void p(D d, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);

        d->completeReturn(ops);
    }
};

// arraylength (190 (0xbe))
        // Description:
        //   Pop arrayref and push its length as an int.
        // Run-time Exceptions:
        //   NullPointerException if arrayref is null.
struct IP_arraylength: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        auto arrayRef = ops->popOperand();
        ASSERT_require(arrayRef->kind() == ValueKind::ArrayReference);
        ASSERT_require(arrayRef->hasArrayLength());

        auto length = arrayRef->arrayLength();
        ASSERT_require(length->kind() == ValueKind::Integer32);

        ops->pushOperand(length);
    }
};

// astore (58 (0x3a))
        // Description:
        //   Pop a reference or returnAddress value and store it into a local variable.
        // Notes:
        //   The _<n> form uses an implicit local-variable index.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_astore: P {
    void p(D d, Ops ops, I insn, Args args) {
        assert_args(insn, args, 1);
        execute_store(ops, d->asU1(args[0]), LocalKind::Reference);
    }
};

// astore_0 (75 (0x4b))
        // Description:
        //   Pop a reference or returnAddress value and store it into a local variable.
        // Notes:
        //   The _<n> form uses an implicit local-variable index.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_astore_0: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        execute_store(ops, 0, LocalKind::Reference);
    }
};

// astore_1 (76 (0x4c))
        // Description:
        //   Pop a reference or returnAddress value and store it into a local variable.
        // Notes:
        //   The _<n> form uses an implicit local-variable index.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_astore_1: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        execute_store(ops, 1, LocalKind::Reference);
    }
};

// astore_2 (77 (0x4d))
        // Description:
        //   Pop a reference or returnAddress value and store it into a local variable.
        // Notes:
        //   The _<n> form uses an implicit local-variable index.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_astore_2: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        execute_store(ops, 2, LocalKind::Reference);
    }
};

// astore_3 (78 (0x4e))
        // Description:
        //   Pop a reference or returnAddress value and store it into a local variable.
        // Notes:
        //   The _<n> form uses an implicit local-variable index.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_astore_3: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        execute_store(ops, 3, LocalKind::Reference);
    }
};

// athrow (191 (0xbf))
        // Description:
        //   Throw the Throwable object reference on top of the operand stack.
        // Notes:
        //   The current stack is cleared and objectref remains as the sole operand while exception handling searches.
        // Run-time Exceptions:
        //   NullPointerException if objectref is null; otherwise the referenced Throwable is thrown.
struct IP_athrow: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        JvmSemantics::execute_athrow(ops, insn, args);
    }
};

// baload (51 (0x33))
        // Description:
        //   Pop arrayref and index; load the byte/boolean component at index and push the value.
        // Run-time Exceptions:
        //   NullPointerException if arrayref is null.
        //   ArrayIndexOutOfBoundsException if index is outside the array bounds.
struct IP_baload: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        execute_array_load(ops, ArrayElementKind::ByteOrBoolean);
    }
};

// bastore (84 (0x54))
        // Description:
        //   Pop value, index, and arrayref; store the byte/boolean value into the array component.
        // Run-time Exceptions:
        //   NullPointerException if arrayref is null.
        //   ArrayIndexOutOfBoundsException if index is outside the array bounds.
struct IP_bastore: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        execute_array_store(ops, ArrayElementKind::ByteOrBoolean);
    }
};

// bipush (16 (0x10))
        // Description:
        //   Sign-extend the immediate operand to int and push it.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_bipush: P {
    void p(D d, Ops ops, I insn, Args args) {
        assert_args(insn, args, 1);

        // bipush has one signed 8-bit immediate operand.
        auto imm = d->asS1(args[0]);

        // Create the SValue and set its type/kind
        auto result = ops->number_(32, imm);
        result->kind(ValueKind::Integer32);

        ops->pushOperand(result);
    }
};

// caload (52 (0x34))
        // Description:
        //   Pop arrayref and index; load the char component at index and push the value.
        // Run-time Exceptions:
        //   NullPointerException if arrayref is null.
        //   ArrayIndexOutOfBoundsException if index is outside the array bounds.
struct IP_caload: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        execute_array_load(ops, ArrayElementKind::Character);
    }
};

// castore (85 (0x55))
        // Description:
        //   Pop value, index, and arrayref; store the char value into the array component.
        // Run-time Exceptions:
        //   NullPointerException if arrayref is null.
        //   ArrayIndexOutOfBoundsException if index is outside the array bounds.
struct IP_castore: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        execute_array_store(ops, ArrayElementKind::Character);
    }
};

// checkcast (192 (0xc0))
        // Description:
        //   Check that objectref is null or assignment-compatible with the resolved type; push the unchanged reference if the check succeeds.
        // Notes:
        //   A null objectref always succeeds.
        // Run-time Exceptions:
        //   ClassCastException if objectref is non-null and not assignment-compatible with the resolved type.
struct IP_checkcast: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 2);
        JvmSemantics::execute_checkcast(ops, insn, args);
    }
};

// d2f (144 (0x90))
        // Description:
        //   Pop a numeric value, convert it to the target primitive type, and push the converted value.
        // Notes:
        //   Conversions follow JVM numeric conversion rules including rounding, NaN handling, and narrowing behavior.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_d2f: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);

        auto value = ops->popOperand();
        ASSERT_require2(value->kind() == ValueKind::Float64, "d2f operand is not Float64");

        ops->pushOperand(ops->fpConvert(value, ValueKind::Float32));
    }
};

// d2i (142 (0x8e))
        // Description:
        //   Pop a numeric value, convert it to the target primitive type, and push the converted value.
        // Notes:
        //   Conversions follow JVM numeric conversion rules including rounding, NaN handling, and narrowing behavior.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_d2i: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);

        auto value = ops->popOperand();
        ASSERT_require2(value->kind() == ValueKind::Float64, "d2i operand is not Float64");

        ops->pushOperand(ops->fpToInteger(value, ValueKind::Integer32));
    }
};

// d2l (143 (0x8f))
        // Description:
        //   Pop a numeric value, convert it to the target primitive type, and push the converted value.
        // Notes:
        //   Conversions follow JVM numeric conversion rules including rounding, NaN handling, and narrowing behavior.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_d2l: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);

        auto value = ops->popOperand();
        ASSERT_require2(value->kind() == ValueKind::Float64, "d2l operand is not Float64");

        ops->pushOperand(ops->fpToInteger(value, ValueKind::Integer64));
    }
};

// dadd (99 (0x63))
        // Description:
        //   Pop operands, apply the arithmetic/bitwise operation, and push the result.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_dadd: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        doBinaryOp(ops, ValueKind::Float64,
             [ops](auto lhs, auto rhs) { return ops->fpAdd(lhs, rhs); });
    }
};

// daload (49 (0x31))
        // Description:
        //   Pop arrayref and index; load the double component at index and push the value.
        // Run-time Exceptions:
        //   NullPointerException if arrayref is null.
        //   ArrayIndexOutOfBoundsException if index is outside the array bounds.
struct IP_daload: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        execute_array_load(ops, ArrayElementKind::Float64);
    }
};

// dastore (82 (0x52))
        // Description:
        //   Pop value, index, and arrayref; store the double value into the array component.
        // Run-time Exceptions:
        //   NullPointerException if arrayref is null.
        //   ArrayIndexOutOfBoundsException if index is outside the array bounds.
struct IP_dastore: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        execute_array_store(ops, ArrayElementKind::Float64);
    }
};

// dcmpl (151 (0x97))
        // Description:
        //   Compare two numeric values and push int -1, 0, or 1; cmpg/cmpl choose the NaN result direction.
        // Notes:
        //   For cmpg NaN yields 1; for cmpl NaN yields -1.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_dcmpl: P {
    void p(D /*d*/, Ops /*ops*/, I insn, Args args) {
        assert_args(insn, args, 0);
        ASSERT_require2(false, "dcmpl unimplemented");
    }
};

// dcmpg (152 (0x98))
        // Description:
        //   Compare two numeric values and push int -1, 0, or 1; cmpg/cmpl choose the NaN result direction.
        // Notes:
        //   For cmpg NaN yields 1; for cmpl NaN yields -1.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_dcmpg: P {
    void p(D /*d*/, Ops /*ops*/, I insn, Args args) {
        assert_args(insn, args, 0);
        ASSERT_require2(false, "dcmpg unimplemented");
    }
};

// dconst_0 (14 (0x0e))
        // Description:
        //   Push the named numeric constant.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_dconst_0: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        ops->pushOperand(ops->fpNumber(0.0));
    }
};

// dconst_1 (15 (0x0f))
        // Description:
        //   Push the named numeric constant.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_dconst_1: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        ops->pushOperand(ops->fpNumber(1.0));
    }
};

// ddiv (111 (0x6f))
        // Description:
        //   Pop operands, apply the arithmetic/bitwise operation, and push the result.
        // Notes:
        //   Floating-point division/remainder follows IEEE 754 behavior and does not throw on zero divisors.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_ddiv: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        doBinaryOp(ops, ValueKind::Float64,
             [ops](auto lhs, auto rhs) { return ops->fpDivide(lhs, rhs); });
    }
};

// dload (24 (0x18))
        // Description:
        //   Load a double value from a local variable and push it.
        // Notes:
        //   The _<n> form uses an implicit local-variable index.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_dload: P {
    void p(D d, Ops ops, I insn, Args args) {
        assert_args(insn, args, 1);
        execute_load(ops, d->asU1(args[0]), LocalKind::Float64);
    }
};

// dload_0 (38 (0x26))
        // Description:
        //   Load a double value from a local variable and push it.
        // Notes:
        //   The _<n> form uses an implicit local-variable index.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_dload_0: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        execute_load(ops, 0, LocalKind::Float64);
    }
};

// dload_1 (39 (0x27))
        // Description:
        //   Load a double value from a local variable and push it.
        // Notes:
        //   The _<n> form uses an implicit local-variable index.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_dload_1: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        execute_load(ops, 1, LocalKind::Float64);
    }
};

// dload_2 (40 (0x28))
        // Description:
        //   Load a double value from a local variable and push it.
        // Notes:
        //   The _<n> form uses an implicit local-variable index.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_dload_2: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        execute_load(ops, 2, LocalKind::Float64);
    }
};

// dload_3 (41 (0x29))
        // Description:
        //   Load a double value from a local variable and push it.
        // Notes:
        //   The _<n> form uses an implicit local-variable index.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_dload_3: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        execute_load(ops, 3, LocalKind::Float64);
    }
};

// dmul (107 (0x6b))
        // Description:
        //   Pop operands, apply the arithmetic/bitwise operation, and push the result.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_dmul: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        doBinaryOp(ops, ValueKind::Float64,
             [ops](auto lhs, auto rhs) { return ops->fpMultiply(lhs, rhs); });
    }
};

// dneg (119 (0x77))
        // Description:
        //   Pop a numeric value, negate it, and push the result.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_dneg: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);

        auto sval = ops->popOperand();
        ASSERT_require(sval->kind() == ValueKind::Float64);
        ASSERT_require(sval->nBits() == 64);

        auto signMask = ops->number_(64, 0x8000000000000000ull);
        auto result = ops->xor_(sval, signMask);
        result->kind(ValueKind::Float64);

        ops->pushOperand(result);
    }
};

// drem (115 (0x73))
        // Description:
        //   Pop operands, apply the arithmetic/bitwise operation, and push the result.
        // Notes:
        //   Floating-point division/remainder follows IEEE 754 behavior and does not throw on zero divisors.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_drem: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        doBinaryOp(ops, ValueKind::Float64,
             [ops](auto lhs, auto rhs) { return ops->fpModulo(lhs, rhs); });
    }
};

// dreturn (175 (0xaf))
        // Description:
        //   Return from the current method; typed returns move the return value to the invoker frame, while return returns void.
        // Run-time Exceptions:
        //   IllegalMonitorStateException can be thrown for synchronized methods if structured locking ownership rules are violated.
struct IP_dreturn: P {
    void p(D d, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);

        auto result = ops->popOperand();
        ASSERT_not_null(result);
        ASSERT_require(result->kind() == ValueKind::Float64);

        d->completeReturn(ops, result);
    }
};

// dstore (57 (0x39))
        // Description:
        //   Pop a double value and store it into a local variable.
        // Notes:
        //   The _<n> form uses an implicit local-variable index.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_dstore: P {
    void p(D d, Ops ops, I insn, Args args) {
        assert_args(insn, args, 1);
        execute_store(ops, d->asU1(args[0]), LocalKind::Float64);
    }
};

// dstore_0 (71 (0x47))
        // Description:
        //   Pop a double value and store it into a local variable.
        // Notes:
        //   The _<n> form uses an implicit local-variable index.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_dstore_0: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        execute_store(ops, 0, LocalKind::Float64);
    }
};

// dstore_1 (72 (0x48))
        // Description:
        //   Pop a double value and store it into a local variable.
        // Notes:
        //   The _<n> form uses an implicit local-variable index.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_dstore_1: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        execute_store(ops, 1, LocalKind::Float64);
    }
};

// dstore_2 (73 (0x49))
        // Description:
        //   Pop a double value and store it into a local variable.
        // Notes:
        //   The _<n> form uses an implicit local-variable index.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_dstore_2: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        execute_store(ops, 2, LocalKind::Float64);
    }
};

// dstore_3 (74 (0x4a))
        // Description:
        //   Pop a double value and store it into a local variable.
        // Notes:
        //   The _<n> form uses an implicit local-variable index.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_dstore_3: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        execute_store(ops, 3, LocalKind::Float64);
    }
};

// dsub (103 (0x67))
        // Description:
        //   Pop operands, apply the arithmetic/bitwise operation, and push the result.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_dsub: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        doBinaryOp(ops, ValueKind::Float64,
             [ops](auto lhs, auto rhs) { return ops->fpSubtract(lhs, rhs); });
    }
};

// dup (89 (0x59))
        // Description:
        //   Rearrange or discard operand stack values according to the JVM category-1/category-2 stack-form rules.
        // Notes:
        //   These instructions must not be used to split a category-2 value.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_dup: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);

        auto v1 = ops->peekOperand();
        ASSERT_require2(v1->isJvmCategory1(), "dup requires top value to be category-1");

        ops->pushOperand(v1->copy());
    }
};

// dup_x1 (90 (0x5a))
        // Description:
        //   Rearrange or discard operand stack values according to the JVM category-1/category-2 stack-form rules.
        // Notes:
        //   These instructions must not be used to split a category-2 value.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_dup_x1: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);

        SValuePtr v1 = ops->popOperand();
        SValuePtr v2 = ops->popOperand();

        ASSERT_require(v1->isJvmCategory1());
        ASSERT_require(v2->isJvmCategory1());

        ops->pushOperand(v1->copy());
        ops->pushOperand(v2);
        ops->pushOperand(v1);
    }
};

// dup_x2 (91 (0x5b))
        // Description:
        //   Rearrange or discard operand stack values according to the JVM category-1/category-2 stack-form rules.
        // Notes:
        //   These instructions must not be used to split a category-2 value.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_dup_x2: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);

        auto v1 = ops->popOperand();   // top
        ASSERT_require2(v1->isJvmCategory1(), "dup_x2 requires top value to be category-1");

        auto v2 = ops->popOperand();
        if (v2->isJvmCategory2()) {
            // Form 2: before: ..., v2, v1; after: ..., v1, v2, v1
            ops->pushOperand(v1->copy());
            ops->pushOperand(v2);
            ops->pushOperand(v1);
        } else {
            // Form 1: before: ..., v3, v2, v1; after: ..., v1, v3, v2, v1
            ASSERT_require2(v2->isJvmCategory1(), "dup_x2 malformed stack");

            SValuePtr v3 = ops->popOperand();
            ASSERT_require2(v3->isJvmCategory1(), "dup_x2 form-1 requires category-1 value3");

            ops->pushOperand(v1->copy());
            ops->pushOperand(v3);
            ops->pushOperand(v2);
            ops->pushOperand(v1);
        }
    }
};

// dup2 (92 (0x5c))
        // Description:
        //   Rearrange or discard operand stack values according to the JVM category-1/category-2 stack-form rules.
        // Notes:
        //   These instructions must not be used to split a category-2 value.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_dup2: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        SValue::Ptr v1 = ops->peekOperand();
        if (v1->isJvmCategory2()) {
            ops->pushOperand(v1->copy());
        }
        else {
            SValue::Ptr v2 = ops->peekOperand(1);
            ASSERT_require2(v2->isJvmCategory1(), "dup2 requires value2 category-1");
            ops->pushOperand(v2->copy());
            ops->pushOperand(v1->copy());
        }
    }
};

// dup2_x1 (93 (0x5d))
        // Description:
        //   Rearrange or discard operand stack values according to the JVM category-1/category-2 stack-form rules.
        // Notes:
        //   These instructions must not be used to split a category-2 value.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_dup2_x1: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        SValuePtr v1 = ops->popOperand();
        if (v1->isJvmCategory2()) {
            // Form 2: before: ..., v2, v1; after: ..., v1, v2, v1
            SValuePtr v2 = ops->popOperand();
            ASSERT_require2(v2->isJvmCategory1(), "dup2_x1 form-2 requires value2 category-1");
            ops->pushOperand(v1->copy());
            ops->pushOperand(v2);
            ops->pushOperand(v1);
        } else {
            // Form 1: before: ..., v3, v2, v1; after: ..., v2, v1, v3, v2, v1
            ASSERT_require2(v1->isJvmCategory1(), "dup2_x1 form-1 requires value1 category-1");

            SValuePtr v2 = ops->popOperand();
            ASSERT_require2(v2->isJvmCategory1(), "dup2_x1 form-1 requires value2 category-1");

            SValuePtr v3 = ops->popOperand();
            ASSERT_require2(v3->isJvmCategory1(), "dup2_x1 form-1 requires value3 category-1");

            ops->pushOperand(v2->copy());
            ops->pushOperand(v1->copy());
            ops->pushOperand(v3);
            ops->pushOperand(v2);
            ops->pushOperand(v1);
        }
    }
};

// dup2_x2 (94 (0x5e))
        // Description:
        //   Rearrange or discard operand stack values according to the JVM category-1/category-2 stack-form rules.
        // Notes:
        //   These instructions must not be used to split a category-2 value.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_dup2_x2: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        SValuePtr v1 = ops->popOperand();
        SValuePtr v2 = ops->popOperand();
        if (v1->isJvmCategory2()) {
            if (v2->isJvmCategory2()) {
                // Form 4: value1, value2 are category-2
                ops->pushOperand(v1->copy());
                ops->pushOperand(v2);
                ops->pushOperand(v1);
            } else {
                // Form 3 value1 is category-2, value2, value3 are category-1
                SValuePtr v3 = ops->popOperand();
                ASSERT_require2(v3->isJvmCategory1(), "dup2_x2 form-3 requires value3 category-1");
                ops->pushOperand(v1->copy());
                ops->pushOperand(v3);
                ops->pushOperand(v2);
                ops->pushOperand(v1);
            }
        } else {
            SValuePtr v3 = ops->popOperand();
            if (v3->isJvmCategory2()) {
                // Form 2: value1, value2 are category-1, value3 is category-2
                ASSERT_require2(v2->isJvmCategory1(), "dup2_x2 form-2 requires value2 category-1");
                ops->pushOperand(v2->copy());
                ops->pushOperand(v1->copy());
                ops->pushOperand(v3);
                ops->pushOperand(v2);
                ops->pushOperand(v1);
            } else {
                // Form 1: value1, value2, value3, value4 are category-1
                SValuePtr v4 = ops->popOperand();
                ASSERT_require2(v3->isJvmCategory1(), "dup2_x2 form-1 requires value3 category-1");
                ASSERT_require2(v4->isJvmCategory1(), "dup2_x2 form-1 requires value4 category-1");
                ops->pushOperand(v2->copy());
                ops->pushOperand(v1->copy());
                ops->pushOperand(v4);
                ops->pushOperand(v3);
                ops->pushOperand(v2);
                ops->pushOperand(v1);
            }
        }
    }
};

// f2d (141 (0x8d))
        // Description:
        //   Pop a numeric value, convert it to the target primitive type, and push the converted value.
        // Notes:
        //   Conversions follow JVM numeric conversion rules including rounding, NaN handling, and narrowing behavior.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_f2d: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);

        auto value = ops->popOperand();
        ASSERT_require2(value->kind() == ValueKind::Float32, "f2d operand is not Float32");

        ops->pushOperand(ops->fpConvert(value, ValueKind::Float64));
    }
};

// f2i (139 (0x8b))
        // Description:
        //   Pop a numeric value, convert it to the target primitive type, and push the converted value.
        // Notes:
        //   Conversions follow JVM numeric conversion rules including rounding, NaN handling, and narrowing behavior.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_f2i: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);

        auto value = ops->popOperand();
        ASSERT_require2(value->kind() == ValueKind::Float32, "f2i operand is not Float32");

        ops->pushOperand(ops->fpToInteger(value, ValueKind::Integer32));
    }
};

// f2l (140 (0x8c))
        // Description:
        //   Pop a numeric value, convert it to the target primitive type, and push the converted value.
        // Notes:
        //   Conversions follow JVM numeric conversion rules including rounding, NaN handling, and narrowing behavior.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_f2l: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);

        auto value = ops->popOperand();
        ASSERT_require2(value->kind() == ValueKind::Float32, "f2l operand is not Float32");

        ops->pushOperand(ops->fpToInteger(value, ValueKind::Integer64));
    }
};

// fadd (98 (0x62))
        // Description:
        //   Pop operands, apply the arithmetic/bitwise operation, and push the result.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_fadd: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        doBinaryOp(ops, ValueKind::Float32,
             [ops](auto lhs, auto rhs) { return ops->fpAdd(lhs, rhs); });
    }
};

// faload (48 (0x30))
        // Description:
        //   Pop arrayref and index; load the float component at index and push the value.
        // Run-time Exceptions:
        //   NullPointerException if arrayref is null.
        //   ArrayIndexOutOfBoundsException if index is outside the array bounds.
struct IP_faload: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        execute_array_load(ops, ArrayElementKind::Float32);
    }
};

// fastore (81 (0x51))
        // Description:
        //   Pop value, index, and arrayref; store the float value into the array component.
        // Run-time Exceptions:
        //   NullPointerException if arrayref is null.
        //   ArrayIndexOutOfBoundsException if index is outside the array bounds.
struct IP_fastore: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        execute_array_store(ops, ArrayElementKind::Float32);
    }
};

// fcmpl (149 (0x95))
        // Description:
        //   Compare two numeric values and push int -1, 0, or 1; cmpg/cmpl choose the NaN result direction.
        // Notes:
        //   For cmpg NaN yields 1; for cmpl NaN yields -1.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_fcmpl: P {
    void p(D /*d*/, Ops /*ops*/, I insn, Args args) {
        assert_args(insn, args, 0);
        ASSERT_require2(false, "fcmpl unimplemented");
    }
};

// fcmpg (150 (0x96))
        // Description:
        //   Compare two numeric values and push int -1, 0, or 1; cmpg/cmpl choose the NaN result direction.
        // Notes:
        //   For cmpg NaN yields 1; for cmpl NaN yields -1.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_fcmpg: P {
    void p(D /*d*/, Ops /*ops*/, I insn, Args args) {
        assert_args(insn, args, 0);
        ASSERT_require2(false, "fcmpg unimplemented");
    }
};

// fconst_0 (11 (0x0b))
        // Description:
        //   Push the named numeric constant.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_fconst_0: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        ops->pushOperand(ops->fpNumber(0.0f));
    }
};

// fconst_1 (12 (0x0c))
        // Description:
        //   Push the named numeric constant.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_fconst_1: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        ops->pushOperand(ops->fpNumber(1.0f));
    }
};

// fconst_2 (13 (0x0d))
        // Description:
        //   Push the named numeric constant.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_fconst_2: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        ops->pushOperand(ops->fpNumber(2.0f));
    }
};

// fdiv (110 (0x6e))
        // Description:
        //   Pop operands, apply the arithmetic/bitwise operation, and push the result.
        // Notes:
        //   Floating-point division/remainder follows IEEE 754 behavior and does not throw on zero divisors.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_fdiv: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        doBinaryOp(ops, ValueKind::Float32,
             [ops](auto lhs, auto rhs) { return ops->fpDivide(lhs, rhs); });
    }
};

// fload (23 (0x17))
        // Description:
        //   Load a float value from a local variable and push it.
        // Notes:
        //   The _<n> form uses an implicit local-variable index.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_fload: P {
    void p(D d, Ops ops, I insn, Args args) {
        assert_args(insn, args, 1);
        execute_load(ops, d->asU1(args[0]), LocalKind::Float32);
    }
};

// fload_0 (34 (0x22))
        // Description:
        //   Load a float value from a local variable and push it.
        // Notes:
        //   The _<n> form uses an implicit local-variable index.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_fload_0: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        ops->pushOperand(ops->readLocal(0));
    }
};

// fload_1 (35 (0x23))
        // Description:
        //   Load a float value from a local variable and push it.
        // Notes:
        //   The _<n> form uses an implicit local-variable index.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_fload_1: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        ops->pushOperand(ops->readLocal(1));
    }
};

// fload_2 (36 (0x24))
        // Description:
        //   Load a float value from a local variable and push it.
        // Notes:
        //   The _<n> form uses an implicit local-variable index.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_fload_2: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        ops->pushOperand(ops->readLocal(2));
    }
};

// fload_3 (37 (0x25))
        // Description:
        //   Load a float value from a local variable and push it.
        // Notes:
        //   The _<n> form uses an implicit local-variable index.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_fload_3: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        ops->pushOperand(ops->readLocal(3));
    }
};

// fmul (106 (0x6a))
        // Description:
        //   Pop operands, apply the arithmetic/bitwise operation, and push the result.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_fmul: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        doBinaryOp(ops, ValueKind::Float32,
             [ops](auto lhs, auto rhs) { return ops->fpMultiply(lhs, rhs); });
    }
};

// fneg (118 (0x76))
        // Description:
        //   Pop a numeric value, negate it, and push the result.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_fneg: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);

        auto sval = ops->popOperand();
        ASSERT_require(sval->kind() == ValueKind::Float32);
        ASSERT_require(sval->nBits() == 32);

        auto signMask = ops->number_(32, 0x80000000u);
        auto result = ops->xor_(sval, signMask);
        result->kind(ValueKind::Float32);

        ops->pushOperand(result);
    }
};

// frem (114 (0x72))
        // Description:
        //   Pop operands, apply the arithmetic/bitwise operation, and push the result.
        // Notes:
        //   Floating-point division/remainder follows IEEE 754 behavior and does not throw on zero divisors.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_frem: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        doBinaryOp(ops, ValueKind::Float32,
             [ops](auto lhs, auto rhs) { return ops->fpModulo(lhs, rhs); });
    }
};

// freturn (174 (0xae))
        // Description:
        //   Return from the current method; typed returns move the return value to the invoker frame, while return returns void.
        // Run-time Exceptions:
        //   IllegalMonitorStateException can be thrown for synchronized methods if structured locking ownership rules are violated.
struct IP_freturn: P {
    void p(D d, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);

        auto result = ops->popOperand();
        ASSERT_not_null(result);
        ASSERT_require(result->kind() == ValueKind::Float32);

        d->completeReturn(ops, result);
    }
};

// fstore (56 (0x38))
        // Description:
        //   Pop a float value and store it into a local variable.
        // Notes:
        //   The _<n> form uses an implicit local-variable index.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_fstore: P {
    void p(D d, Ops ops, I insn, Args args) {
        assert_args(insn, args, 1);
        execute_store(ops, d->asU1(args[0]), LocalKind::Float32);
    }
};

// fstore_0 (67 (0x43))
        // Description:
        //   Pop a float value and store it into a local variable.
        // Notes:
        //   The _<n> form uses an implicit local-variable index.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_fstore_0: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        execute_store(ops, 0, LocalKind::Float32);
    }
};

// fstore_1 (68 (0x44))
        // Description:
        //   Pop a float value and store it into a local variable.
        // Notes:
        //   The _<n> form uses an implicit local-variable index.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_fstore_1: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        execute_store(ops, 1, LocalKind::Float32);
    }
};

// fstore_2 (69 (0x45))
        // Description:
        //   Pop a float value and store it into a local variable.
        // Notes:
        //   The _<n> form uses an implicit local-variable index.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_fstore_2: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        execute_store(ops, 2, LocalKind::Float32);
    }
};

// fstore_3 (70 (0x46))
        // Description:
        //   Pop a float value and store it into a local variable.
        // Notes:
        //   The _<n> form uses an implicit local-variable index.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_fstore_3: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        execute_store(ops, 3, LocalKind::Float32);
    }
};

// fsub (102 (0x66))
        // Description:
        //   Pop operands, apply the arithmetic/bitwise operation, and push the result.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_fsub: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        doBinaryOp(ops, ValueKind::Float32,
             [ops](auto lhs, auto rhs) { return ops->fpSubtract(lhs, rhs); });
    }
};

// getfield (180 (0xb4))
        // Description:
        //   Resolve the referenced field and read or write the instance/static field value.
        // Run-time Exceptions:
        //   NullPointerException if objectref is null.
struct IP_getfield: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 1);

        auto sval = ops->popOperand();
        ASSERT_not_null(sval);
        ASSERT_require(sval->kind() == ValueKind::ObjectReference);

        auto pool = DispatcherJvm::constantPool(ops);
        ASSERT_not_null(pool);

        auto ivExpr = isSgAsmIntegerValueExpression(args[0]);
        ASSERT_not_null(ivExpr);
        const size_t index = static_cast<uint16_t>(ivExpr->get_absoluteValue());

        // Determine the ValueKind of the field descriptor
        std::string fieldDesc = DispatcherJvm::fieldDescriptor(pool, index);
        auto descType = DescriptorParser::parseFieldDescriptor(fieldDesc);

        auto value = ops->undefined_(DispatcherJvm::nBitsForKind(descType.kind));
        ASSERT_not_null(value);
        value->kind(descType.kind);

        if (descType.isReference()) {
            value->typeDescriptor(descType.descriptor);
        }

        // Without a heap model, this is an unknown value of the correct type.
        ops->pushOperand(value);
    }
};

// getstatic (178 (0xb2))
        // Description:
        //   Resolve the referenced field and read or write the instance/static field value.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_getstatic: P {
    void p(D d, Ops ops, I insn, Args args) {
        assert_args(insn, args, 1);
        JvmSemantics::execute_getstatic(ops, d->asU2(args[0]));
    }
};

// goto (167 (0xa7))
        // Description:
        //   Branch unconditionally by the signed offset.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_goto_: P {
    void p(D d, Ops ops, I insn, Args args) {
        assert_args(insn, args, 1);

        auto targetAddr = JvmSemantics::branchTargetAddress(insn, d->asS2(args[0]));
        const RegisterDescriptor pcReg = d->instructionPointerRegister();

        ops->writeRegister(pcReg, ops->number_(pcReg.nBits(), targetAddr));
    }
};

// goto_w (200 (0xc8))
        // Description:
        //   Branch unconditionally by the signed offset.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_goto_w: P {
    void p(D /*d*/, Ops /*ops*/, I insn, Args args) {
        assert_args(insn, args, 4);
        ASSERT_require2(false, "goto_w unimplemented");
    }
};

// i2b (145 (0x91))
        // Description:
        //   Pop a numeric value, convert it to the target primitive type, and push the converted value.
        // Notes:
        //   Conversions follow JVM numeric conversion rules including rounding, NaN handling, and narrowing behavior.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_i2b: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        // keep low 8 bits, then sign-extend to 32 bits
        doUnaryOp(ops, ValueKind::Integer32,
                  [ops](auto value) {
                      auto low8 = ops->extract(value, 0, 8);
                      return ops->signExtend(low8, 32);
                  });
    }
};

// i2c (146 (0x92))
        // Description:
        //   Pop a numeric value, convert it to the target primitive type, and push the converted value.
        // Notes:
        //   Conversions follow JVM numeric conversion rules including rounding, NaN handling, and narrowing behavior.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_i2c: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        // keep low 16 bits, then zero-extend to 32 bits
        doUnaryOp(ops, ValueKind::Integer32,
                  [ops](auto value) {
                      auto low16 = ops->extract(value, 0, 16);
                      return ops->unsignedExtend(low16, 32);
                  });
    }
};

// i2d (135 (0x87))
        // Description:
        //   Pop a numeric value, convert it to the target primitive type, and push the converted value.
        // Notes:
        //   Conversions follow JVM numeric conversion rules including rounding, NaN handling, and narrowing behavior.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_i2d: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);

        auto sval = ops->popOperand();
        ASSERT_require2(sval->kind() == ValueKind::Integer32, "i2d operand is not Integer32");

        ops->pushOperand(ops->fpFromInteger(sval, ValueKind::Float64));
    }
};

// i2f (134 (0x86))
        // Description:
        //   Pop a numeric value, convert it to the target primitive type, and push the converted value.
        // Notes:
        //   Conversions follow JVM numeric conversion rules including rounding, NaN handling, and narrowing behavior.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_i2f: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);

        auto sval = ops->popOperand();
        ASSERT_require2(sval->kind() == ValueKind::Integer32, "i2f operand is not Integer32");

        ops->pushOperand(ops->fpFromInteger(sval, ValueKind::Float32));
}
};

// i2l (133 (0x85))
        // Description:
        //   Pop a numeric value, convert it to the target primitive type, and push the converted value.
        // Notes:
        //   Conversions follow JVM numeric conversion rules including rounding, NaN handling, and narrowing behavior.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_i2l: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);

        auto sval = ops->popOperand();
        ASSERT_require(sval->kind() == ValueKind::Integer32);

        auto result = ops->signExtend(sval, 64);
        result->kind(ValueKind::Integer64);

        ops->pushOperand(result);
    }
};

// i2s (147 (0x93))
        // Description:
        //   Pop a numeric value, convert it to the target primitive type, and push the converted value.
        // Notes:
        //   Conversions follow JVM numeric conversion rules including rounding, NaN handling, and narrowing behavior.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_i2s: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        // keep low 16 bits, then sign-extend to 32 bits
        doUnaryOp(ops, ValueKind::Integer32,
                  [ops](auto value) {
                      auto low16 = ops->extract(value, 0, 16);
                      return ops->signExtend(low16, 32);
                  });
    }
};

// iadd (96 (0x60))
        // Description:
        //   Pop operands, apply the arithmetic/bitwise operation, and push the result.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_iadd: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        doBinaryOp(ops, ValueKind::Integer32,
             [ops](auto lhs, auto rhs) { return ops->add(lhs, rhs); });
    }
};

// iaload (46 (0x2e))
        // Description:
        //   Pop arrayref and index; load the int component at index and push the value.
        // Run-time Exceptions:
        //   NullPointerException if arrayref is null.
        //   ArrayIndexOutOfBoundsException if index is outside the array bounds.
struct IP_iaload: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        execute_array_load(ops, ArrayElementKind::Integer32);
    }
};

// iand (126 (0x7e))
        // Description:
        //   Pop operands, apply the arithmetic/bitwise operation, and push the result.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_iand: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        doBinaryOp(ops, ValueKind::Integer32,
             [ops](auto lhs, auto rhs) { return ops->and_(lhs, rhs); });
    }
};

// iastore (79 (0x4f))
        // Description:
        //   Pop value, index, and arrayref; store the int value into the array component.
        // Run-time Exceptions:
        //   NullPointerException if arrayref is null.
        //   ArrayIndexOutOfBoundsException if index is outside the array bounds.
struct IP_iastore: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        execute_array_store(ops, ArrayElementKind::Integer32);
    }
};

// iconst_m1 (2 (0x02))
        // Description:
        //   Load the int value -1 onto the stack.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_iconst_m1: P {
    void p(D d, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        ops->pushOperand(d->makeConstant(ValueKind::Integer32, -1));
    }
};

// iconst_0 (3 (0x03))
        // Description:
        //   Load the int value 0 onto the stack.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_iconst_0: P {
    void p(D d, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        ops->pushOperand(d->makeConstant(ValueKind::Integer32, 0));
    }
};

// iconst_1 (4 (0x04))
        // Description:
        //   Load the int value 1 onto the stack.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_iconst_1: P {
    void p(D d, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        ops->pushOperand(d->makeConstant(ValueKind::Integer32, 1));
    }
};

// iconst_2 (5 (0x05))
        // Description:
        //   Load the int value 2 onto the stack.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_iconst_2: P {
    void p(D d, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        ops->pushOperand(d->makeConstant(ValueKind::Integer32, 2));
    }
};

// iconst_3 (6 (0x06))
        // Description:
        //   Load the int value 3 onto the stack.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_iconst_3: P {
    void p(D d, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        ops->pushOperand(d->makeConstant(ValueKind::Integer32, 3));
    }
};

// iconst_4 (7 (0x07))
        // Description:
        //   Load the int value 4 onto the stack.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_iconst_4: P {
    void p(D d, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        ops->pushOperand(d->makeConstant(ValueKind::Integer32, 4));
    }
};

// iconst_5 (8 (0x08))
        // Description:
        //   Load the int value 5 onto the stack.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_iconst_5: P {
    void p(D d, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        ops->pushOperand(d->makeConstant(ValueKind::Integer32, 5));
    }
};

// idiv (108 (0x6c))
        // Description:
        //   Pop operands, apply the arithmetic/bitwise operation, and push the result.
        // Run-time Exceptions:
        //   ArithmeticException if the divisor is zero.
struct IP_idiv: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);

        auto rhs = ops->popOperand(); // divisor
        auto lhs = ops->popOperand(); // dividend

        ASSERT_require(lhs->kind() == ValueKind::Integer32);
        ASSERT_require(rhs->kind() == ValueKind::Integer32);
        ASSERT_require(lhs->nBits() == 32);
        ASSERT_require(rhs->nBits() == 32);

        if (rhs->isConcrete() && rhs->toUnsigned().get() == 0) {
            throw BaseSemantics::Exception("integer division by zero", ops->currentInstruction());
        }

        // JVM specifies INT_MIN / -1 == INT_MIN rather than overflow.
        if (lhs->isConcrete() && rhs->isConcrete() &&
            lhs->toUnsigned().get() == 0x80000000u &&
            rhs->toUnsigned().get() == 0xffffffffu)
        {
            auto result = ops->number_(32, 0x80000000u);
            result->kind(ValueKind::Integer32);
            ops->pushOperand(result);
            return;
        }

        auto result = ops->signedDivide(lhs, rhs);
        ASSERT_require(result->nBits() == 32);
        result->kind(ValueKind::Integer32);
        ops->pushOperand(result);
    }
};

// if_acmpeq (165 (0xa5))
        // Description:
        //   Pop comparison operand(s); branch to the signed offset if the condition is true, otherwise continue with the next instruction.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_if_acmpeq: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 2);
        JvmSemantics::branch_if_acmpeq(ops, insn, args);
    }
};

// if_acmpne (166 (0xa6))
        // Description:
        //   Pop comparison operand(s); branch to the signed offset if the condition is true, otherwise continue with the next instruction.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_if_acmpne: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 2);
        JvmSemantics::branch_if_acmpne(ops, insn, args);
    }
};

// if_icmpeq (159 (0x9f))
        // Description:
        //   Pop comparison operand(s); branch to the signed offset if the condition is true, otherwise continue with the next instruction.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_if_icmpeq: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 2);
        JvmSemantics::branch_if_icmpeq(ops, insn, args);
    }
};

// if_icmpne (160 (0xa0))
        // Description:
        //   Pop comparison operand(s); branch to the signed offset if the condition is true, otherwise continue with the next instruction.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_if_icmpne: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 2);
        JvmSemantics::branch_if_icmpne(ops, insn, args);
    }
};

// if_icmplt (161 (0xa1))
        // Description:
        //   Pop comparison operand(s); branch to the signed offset if the condition is true, otherwise continue with the next instruction.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_if_icmplt: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 2);
        JvmSemantics::branch_if_icmplt(ops, insn, args);
    }
};

// if_icmpge (162 (0xa2))
        // Description:
        //   Pop comparison operand(s); branch to the signed offset if the condition is true, otherwise continue with the next instruction.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_if_icmpge: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 2);
        JvmSemantics::branch_if_icmpge(ops, insn, args);
    }
};

// if_icmpgt (163 (0xa3))
        // Description:
        //   Pop comparison operand(s); branch to the signed offset if the condition is true, otherwise continue with the next instruction.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_if_icmpgt: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 2);
        JvmSemantics::branch_if_icmpgt(ops, insn, args);
    }
};

// if_icmple (164 (0xa4))
        // Description:
        //   Pop comparison operand(s); branch to the signed offset if the condition is true, otherwise continue with the next instruction.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_if_icmple: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 2);
        JvmSemantics::branch_if_icmple(ops, insn, args);
    }
};

// ifeq (153 (0x99))
        // Description:
        //   Pop comparison operand(s); branch to the signed offset if the condition is true, otherwise continue with the next instruction.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_ifeq: P {
    void p(D d, Ops ops, I insn, Args args) {
        assert_args(insn, args, 1);

        auto value = ops->popOperand();
        ASSERT_not_null(value);
        ASSERT_require(value->kind() == ValueKind::Integer32);

        auto zero = ops->number_(value->nBits(), 0);
        ASSERT_not_null(zero);

        const auto condition = ops->isEqual(value, zero);

        JvmSemantics::execute_condition(d, ops, insn, d->asS2(args[0]), ops->isEqual(value, zero));
    }
};

// ifne (154 (0x9a))
        // Description:
        //   Pop comparison operand(s); branch to the signed offset if the condition is true, otherwise continue with the next instruction.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_ifne: P {
    void p(D d, Ops ops, I insn, Args args) {
        assert_args(insn, args, 1);

        auto value = ops->popOperand();
        ASSERT_not_null(value);
        ASSERT_require(value->kind() == ValueKind::Integer32);

        auto zero = ops->number_(value->nBits(), 0);
        ASSERT_not_null(zero);

        const auto condition = ops->isNotEqual(value, zero);

        JvmSemantics::execute_condition(d, ops, insn, d->asS2(args[0]), condition);
    }
};

// iflt (155 (0x9b))
        // Description:
        //   Pop comparison operand(s); branch to the signed offset if the condition is true, otherwise continue with the next instruction.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_iflt: P {
    void p(D d, Ops ops, I insn, Args args) {
        assert_args(insn, args, 1);

        auto value = ops->popOperand();
        ASSERT_not_null(value);
        ASSERT_require(value->kind() == ValueKind::Integer32);

        auto zero = ops->number_(value->nBits(), 0);
        ASSERT_not_null(zero);

        const auto condition = ops->isSignedLessThan(value, zero);

        JvmSemantics::execute_condition(d, ops, insn, d->asS2(args[0]), condition);
    }
};

// ifge (156 (0x9c))
        // Description:
        //   Pop comparison operand(s); branch to the signed offset if the condition is true, otherwise continue with the next instruction.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_ifge: P {
    void p(D d, Ops ops, I insn, Args args) {
        assert_args(insn, args, 1);

        auto value = ops->popOperand();
        ASSERT_not_null(value);
        ASSERT_require(value->kind() == ValueKind::Integer32);

        auto zero = ops->number_(value->nBits(), 0);
        ASSERT_not_null(zero);

        const auto condition = ops->isSignedGreaterThanOrEqual(value, zero);

        JvmSemantics::execute_condition(d, ops, insn, d->asS2(args[0]), condition);
    }
};

// ifgt (157 (0x9d))
        // Description:
        //   Pop comparison operand(s); branch to the signed offset if the condition is true, otherwise continue with the next instruction.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_ifgt: P {
    void p(D d, Ops ops, I insn, Args args) {
        assert_args(insn, args, 1);

        auto value = ops->popOperand();
        ASSERT_not_null(value);
        ASSERT_require(value->kind() == ValueKind::Integer32);

        auto zero = ops->number_(value->nBits(), 0);
        ASSERT_not_null(zero);

        const auto condition = ops->isSignedGreaterThan(value, zero);

        JvmSemantics::execute_condition(d, ops, insn, d->asS2(args[0]), condition);
    }
};

// ifle (158 (0x9e))
        // Description:
        //   Pop comparison operand(s); branch to the signed offset if the condition is true, otherwise continue with the next instruction.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_ifle: P {
    void p(D d, Ops ops, I insn, Args args) {
        assert_args(insn, args, 1);

        auto value = ops->popOperand();
        ASSERT_not_null(value);
        ASSERT_require(value->kind() == ValueKind::Integer32);

        auto zero = ops->number_(value->nBits(), 0);
        ASSERT_not_null(zero);

        const auto condition = ops->isSignedLessThanOrEqual(value, zero);

        JvmSemantics::execute_condition(d, ops, insn, d->asS2(args[0]), condition);
    }
};

// ifnonnull (199 (0xc7))
        // Description:
        //   Pop comparison operand(s); branch to the signed offset if the condition is true, otherwise continue with the next instruction.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_ifnonnull: P {
    void p(D d, Ops ops, I insn, Args args) {
        assert_args(insn, args, 1);
        execute_null_branch(d, ops, insn, args, JvmSemantics::NullBranchKind::IfNonNull);
    }
};

// ifnull (198 (0xc6))
        // Description:
        //   Pop comparison operand(s); branch to the signed offset if the condition is true, otherwise continue with the next instruction.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_ifnull: P {
    void p(D d, Ops ops, I insn, Args args) {
        assert_args(insn, args, 1);
        execute_null_branch(d, ops, insn, args, JvmSemantics::NullBranchKind::IfNull);
    }
};

// iinc (132 (0x84))
        // Description:
        //   Increment the selected local int variable by the signed constant.
        // Notes:
        //   The wide prefix expands both the local-variable index and increment constant.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_iinc: P {
    void p(D d, Ops ops, I insn, Args args) {
        assert_args(insn, args, 2);
        int32_t increment = d->asS1(args[1]);
        JS::execute_iinc(ops, d->asU1(args[0]), increment);
    }
};

// iload (21 (0x15))
        // Description:
        //   Load an int value from a local variable #index
        // Notes:
        //   The _<n> form uses an implicit local-variable index.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_iload: P {
    void p(D d, Ops ops, I insn, Args args) {
        assert_args(insn, args, 1);
        execute_load(ops, d->asU1(args[0]), LocalKind::Integer32);
    }
};

// iload_0 (26 (0x1a))
        // Description:
        //   Load an int value from a local variable #index
        // Notes:
        //   The _<n> form uses an implicit local-variable index.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_iload_0: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        ops->pushOperand(ops->readLocal(0));
    }
};

// iload_1 (27 (0x1b))
        // Description:
        //   Load a int value from a local variable and push it.
        // Notes:
        //   The _<n> form uses an implicit local-variable index.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_iload_1: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        ops->pushOperand(ops->readLocal(1));
    }
};

// iload_2 (28 (0x1c))
        // Description:
        //   Load a int value from a local variable and push it.
        // Notes:
        //   The _<n> form uses an implicit local-variable index.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_iload_2: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        ops->pushOperand(ops->readLocal(2));
    }
};

// iload_3 (29 (0x1d))
        // Description:
        //   Load a int value from a local variable and push it.
        // Notes:
        //   The _<n> form uses an implicit local-variable index.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_iload_3: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        ops->pushOperand(ops->readLocal(3));
    }
};


// imul (104 (0x68))
        // Description:
        //   Pop operands, apply the arithmetic/bitwise operation, and push the result.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_imul: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        doBinaryOp(ops, ValueKind::Integer32,
                   [ops](auto lhs, auto rhs) {
                       auto product = ops->unsignedMultiply(lhs, rhs);
                       return ops->extract(product, 0, 32);
                   });
    }
};

// ineg (116 (0x74))
        // Description:
        //   Pop a numeric value, negate it, and push the result.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_ineg: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        doUnaryOp(ops, ValueKind::Integer32,
             [ops](auto sval) { return ops->negate(sval); });
    }
};

// instanceof (193 (0xc1))
        // Description:
        //   Pop objectref and push int 1 if it is an instance of the resolved type, otherwise push int 0.
        // Notes:
        //   A null objectref results in 0.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_instanceof: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 2);
        JvmSemantics::execute_instanceof(ops, insn, args);
    }
};

// invokedynamic (186 (0xba))
        // Description:
        //   Resolve the call target, pop receiver/arguments as required, invoke the method, and push any return value.
        // Run-time Exceptions:
        //   NullPointerException if an instance invocation receiver is null.
        //   Errors from method resolution or class/interface initialization may be observed as specified by the JVM.
struct IP_invokedynamic: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 4);
        JvmSemantics::execute_invokedynamic(ops, insn, args);
    }
};

// invokeinterface (185 (0xb9))
        // Description:
        //   Resolve the call target, pop receiver/arguments as required, invoke the method, and push any return value.
        // Run-time Exceptions:
        //   NullPointerException if an instance invocation receiver is null.
        //   Errors from method resolution or class/interface initialization may be observed as specified by the JVM.
struct IP_invokeinterface: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 4);
        JvmSemantics::execute_invokeinterface(ops, insn, args);
    }
};

// invokespecial (183 (0xb7))
        // Description:
        //   Resolve the call target, pop receiver/arguments as required, invoke the method, and push any return value.
        // Run-time Exceptions:
        //   NullPointerException if an instance invocation receiver is null.
        //   Errors from method resolution or class/interface initialization may be observed as specified by the JVM.
struct IP_invokespecial: P {
    void p(D d, Ops ops, I insn, Args args) {
        assert_args(insn, args, 1);
        JvmSemantics::execute_invoke(ops, d->asU2(args[0]), insn, InvocationKind::Special);
    }
};

// invokestatic (184 (0xb8))
        // Description:
        //   Resolve the call target, pop receiver/arguments as required, invoke the method, and push any return value.
        // Run-time Exceptions:
        //   NullPointerException if an instance invocation receiver is null.
        //   Errors from method resolution or class/interface initialization may be observed as specified by the JVM.
struct IP_invokestatic: P {
    void p(D d, Ops ops, I insn, Args args) {
        assert_args(insn, args, 1);
        JvmSemantics::execute_invoke(ops, d->asU2(args[0]), insn, InvocationKind::Static);
    }
};

// invokevirtual (182 (0xb6))
        // Description:
        //   Resolve the call target, pop receiver/arguments as required, invoke the method, and push any return value.
        // Run-time Exceptions:
        //   NullPointerException if an instance invocation receiver is null.
        //   Errors from method resolution or class/interface initialization may be observed as specified by the JVM.
struct IP_invokevirtual: P {
    void p(D d, Ops ops, I insn, Args args) {
        assert_args(insn, args, 1);
        JvmSemantics::execute_invoke(ops, d->asU2(args[0]), insn, InvocationKind::Virtual);
    }
};

// ior (128 (0x80))
        // Description:
        //   Pop operands, apply the arithmetic/bitwise operation, and push the result.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_ior: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        doBinaryOp(ops, ValueKind::Integer32,
             [ops](auto lhs, auto rhs) { return ops->or_(lhs, rhs); });
    }
};

// irem (112 (0x70))
        // Description:
        //   Pop operands, apply the arithmetic/bitwise operation, and push the result.
        // Run-time Exceptions:
        //   ArithmeticException if the divisor is zero.
struct IP_irem: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);

        auto divisor = ops->peekOperand();
        if (divisor->isConcrete() && divisor->toUnsigned().get() == 0) {
            // Eventually: model java/lang/ArithmeticException.
            ASSERT_require2(false, "irem division by zero");
        }

        // Otherwise calculate the signedModulo
        doBinaryOp(ops, ValueKind::Integer32,
             [ops](auto lhs, auto rhs) { return ops->signedModulo(lhs, rhs); });
    }
};

// ireturn (172 (0xac))
        // Description:
        //   Return from the current method; typed returns move the return value to the invoker frame, while return returns void.
        // Run-time Exceptions:
        //   IllegalMonitorStateException can be thrown for synchronized methods if structured locking ownership rules are violated.
struct IP_ireturn: P {
    void p(D d, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);

        auto result = ops->popOperand();
        ASSERT_not_null(result);
        ASSERT_require(result->kind() == ValueKind::Integer32);

        d->completeReturn(ops, result);
    }
};

// ishl (120 (0x78))
        // Description:
        //   Pop operands, apply the arithmetic/bitwise operation, and push the result.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_ishl: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);

        auto count = ops->popOperand();
        auto value = ops->popOperand();

        ASSERT_require2(count->kind() == ValueKind::Integer32, "ishl shift count must be Integer32");
        ASSERT_require2(value->kind() == ValueKind::Integer32, "ishl value must be Integer32");
        ASSERT_require(count->nBits() == 32);
        ASSERT_require(value->nBits() == 32);

        // JVM uses only low 5 bits of the shift count for int shifts.
        auto maskedCount = ops->and_(count, ops->number_(32, 0x1f));

        auto result = ops->shiftLeft(value, maskedCount);
        result->kind(ValueKind::Integer32);

        ops->pushOperand(result);
    }
};

// ishr (122 (0x7a))
        // Description:
        //   Pop operands, apply the arithmetic/bitwise operation, and push the result.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_ishr: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);

        auto count = ops->popOperand();
        auto value = ops->popOperand();

        ASSERT_require2(count->kind() == ValueKind::Integer32, "ishr shift count must be Integer32");
        ASSERT_require2(value->kind() == ValueKind::Integer32, "ishr value must be Integer32");
        ASSERT_require(count->nBits() == 32);
        ASSERT_require(value->nBits() == 32);

        // JVM int shifts use only the low five bits of the shift distance.
        auto distance = ops->extract(count, 0, 5);

        auto result = ops->shiftRightArithmetic(value, distance);
        ASSERT_require(result->nBits() == 32);
        result->kind(ValueKind::Integer32);

        ops->pushOperand(result);
    }
};

// istore (54 (0x36))
        // Description:
        //   Pop a int value and store it into a local variable.
        // Notes:
        //   The _<n> form uses an implicit local-variable index.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_istore: P {
    void p(D d, Ops ops, I insn, Args args) {
        assert_args(insn, args, 1);
        execute_store(ops, d->asU1(args[0]), LocalKind::Integer32);
    }
};

// istore_0 (59 (0x3b))
        // Description:
        //   Pop a int value and store it into a local variable.
        // Notes:
        //   The _<n> form uses an implicit local-variable index.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_istore_0: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        execute_store(ops, 0, LocalKind::Integer32);
    }
};

// istore_1 (60 (0x3c))
        // Description:
        //   Pop a int value and store it into a local variable.
        // Notes:
        //   The _<n> form uses an implicit local-variable index.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_istore_1: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        execute_store(ops, 1, LocalKind::Integer32);
    }
};

// istore_2 (61 (0x3d))
        // Description:
        //   Pop a int value and store it into a local variable.
        // Notes:
        //   The _<n> form uses an implicit local-variable index.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_istore_2: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        execute_store(ops, 2, LocalKind::Integer32);
    }
};

// istore_3 (62 (0x3e))
        // Description:
        //   Pop a int value and store it into a local variable.
        // Notes:
        //   The _<n> form uses an implicit local-variable index.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_istore_3: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        execute_store(ops, 3, LocalKind::Integer32);
    }
};

// isub (100 (0x64))
        // Description:
        //   Pop operands, apply the arithmetic/bitwise operation, and push the result.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_isub: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        doBinaryOp(ops, ValueKind::Integer32,
             [ops](auto lhs, auto rhs) { return ops->subtract(lhs, rhs); });
    }
};

// iushr (124 (0x7c))
        // Description:
        //   Pop operands, apply the arithmetic/bitwise operation, and push the result.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_iushr: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);

        auto count = ops->popOperand();
        auto value = ops->popOperand();

        ASSERT_require2(count->kind() == ValueKind::Integer32, "iushr shift distance must be Integer32");
        ASSERT_require2(value->kind() == ValueKind::Integer32, "iushr value must be Integer32");

        // JVM int shifts use only the low five bits.
        auto maskedCount = ops->and_(count, ops->number_(32, 0x1f));

        auto result = ops->shiftRight(value, maskedCount);
        result->kind(ValueKind::Integer32);

        ops->pushOperand(result);
    }
};

// ixor (130 (0x82))
        // Description:
        //   Pop operands, apply the arithmetic/bitwise operation, and push the result.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_ixor: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        doBinaryOp(ops, ValueKind::Integer32,
             [ops](auto lhs, auto rhs) { return ops->xor_(lhs, rhs); });
    }
};

// jsr (168 (0xa8))
        // Description:
        //   Push the returnAddress of the following instruction and branch to the subroutine offset.
        // Notes:
        //   jsr/jsr_w are obsolete and used with ret in pre-verification-era subroutines.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_jsr: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 2);
        JvmSemantics::branch_jsr(ops, insn, args);
    }
};

// jsr_w (201 (0xc9))
        // Description:
        //   Push the returnAddress of the following instruction and branch to the subroutine offset.
        // Notes:
        //   jsr/jsr_w are obsolete and used with ret in pre-verification-era subroutines.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_jsr_w: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 4);
        JvmSemantics::branch_jsr_w(ops, insn, args);
    }
};

// l2d (138 (0x8a))
        // Description:
        //   Pop a numeric value, convert it to the target primitive type, and push the converted value.
        // Notes:
        //   Conversions follow JVM numeric conversion rules including rounding, NaN handling, and narrowing behavior.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_l2d: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);

        auto sval = ops->popOperand();
        ASSERT_require2(sval->kind() == ValueKind::Integer64, "l2d operand is not Integer64");

        auto result = ops->fpConvert(sval, ValueKind::Float64);
        ASSERT_require2(result->kind() == ValueKind::Float64, "l2d result is not Float64");
        ops->pushOperand(result);
    }
};

// l2f (137 (0x89))
        // Description:
        //   Pop a numeric value, convert it to the target primitive type, and push the converted value.
        // Notes:
        //   Conversions follow JVM numeric conversion rules including rounding, NaN handling, and narrowing behavior.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_l2f: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);

        auto sval = ops->popOperand();
        ASSERT_require2(sval->kind() == ValueKind::Integer64, "l2f operand is not Integer64");

        auto result = ops->fpConvert(sval, ValueKind::Float32);
        ASSERT_require2(result->kind() == ValueKind::Float32, "l2f result is not Float32");
        ops->pushOperand(result);
    }
};

// l2i (136 (0x88))
        // Description:
        //   Pop a numeric value, convert it to the target primitive type, and push the converted value.
        // Notes:
        //   Conversions follow JVM numeric conversion rules including rounding, NaN handling, and narrowing behavior.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_l2i: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);

        auto sval = ops->popOperand();
        ASSERT_require2(sval->kind() == ValueKind::Integer64,
                        "l2i operand is not Integer64");

        auto result = ops->extract(sval, 0, 32);
        result->kind(ValueKind::Integer32);
        ops->pushOperand(result);
    }
};

// ladd (97 (0x61))
        // Description:
        //   Pop operands, apply the arithmetic/bitwise operation, and push the result.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_ladd: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        doBinaryOp(ops, ValueKind::Integer64,
             [ops](auto lhs, auto rhs) { return ops->add(lhs, rhs); });
    }
};

// laload (47 (0x2f))
        // Description:
        //   Pop arrayref and index; load the long component at index and push the value.
        // Run-time Exceptions:
        //   NullPointerException if arrayref is null.
        //   ArrayIndexOutOfBoundsException if index is outside the array bounds.
struct IP_laload: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        execute_array_load(ops, ArrayElementKind::Integer64);
    }
};

// land (127 (0x7f))
        // Description:
        //   Pop operands, apply the arithmetic/bitwise operation, and push the result.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_land: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        doBinaryOp(ops, ValueKind::Integer64,
             [ops](auto lhs, auto rhs) { return ops->and_(lhs, rhs); });
    }
};

// lastore (80 (0x50))
        // Description:
        //   Pop value, index, and arrayref; store the long value into the array component.
        // Run-time Exceptions:
        //   NullPointerException if arrayref is null.
        //   ArrayIndexOutOfBoundsException if index is outside the array bounds.
struct IP_lastore: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        execute_array_store(ops, ArrayElementKind::Integer64);
    }
};

// lcmp (148 (0x94))
        // Description:
        //   Compare two numeric values and push int -1, 0, or 1; cmpg/cmpl choose the NaN result direction.
        // Notes:
        //   For cmpg NaN yields 1; for cmpl NaN yields -1.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_lcmp: P {
    void p(D /*d*/, Ops /*ops*/, I insn, Args args) {
        assert_args(insn, args, 0);
        ASSERT_require2(false, "lcmp unimplemented");
    }
};

// lconst_0 (9 (0x09))
        // Description:
        //   Push the named numeric constant.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_lconst_0: P {
    void p(D d, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        ops->pushOperand(d->makeConstant(ValueKind::Integer64, 0));
    }
};

// lconst_1 (10 (0x0a))
        // Description:
        //   Push the named numeric constant.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_lconst_1: P {
    void p(D d, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        ops->pushOperand(d->makeConstant(ValueKind::Integer64, 1));
    }
};

// ldc (18 (0x12))
        // Description:
        //   Push a constant-pool constant or resolved symbolic reference value.
        // Notes:
        //   ldc2_w is used for long and double constants.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_ldc: P {
    void p(D d, Ops ops, I insn, Args args) {
        assert_args(insn, args, 1);
        // ldc: unsigned 8-bit constant-pool index, the compact form, others are 16-bit
        JvmSemantics::execute_ldc(ops, d->asU1(args[0]));
    }
};

// ldc_w (19 (0x13))
        // Description:
        //   Push a constant-pool constant or resolved symbolic reference value.
        // Notes:
        //   ldc2_w is used for long and double constants.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_ldc_w: P {
    void p(D d, Ops ops, I insn, Args args) {
        assert_args(insn, args, 2);
        // ldc_w: unsigned 16-bit constant-pool index
        JvmSemantics::execute_ldc(ops, d->asU2(args[0]));
    }
};

// ldc2_w (20 (0x14))
        // Description:
        //   Push a constant-pool constant or resolved symbolic reference value.
        // Notes:
        //   ldc2_w is used for long and double constants.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_ldc2_w: P {
    void p(D d, Ops ops, I insn, Args args) {
        assert_args(insn, args, 1);
        // the instruction pushes an Integer64 or Float64 value, respectively.
        JvmSemantics::execute_ldc(ops, d->asU2(args[0]));
    }
};

// ldiv (109 (0x6d))
        // Description:
        //   Pop operands, apply the arithmetic/bitwise operation, and push the result.
        // Run-time Exceptions:
        //   ArithmeticException if the divisor is zero.
struct IP_ldiv: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);

        auto rhs = ops->popOperand(); // divisor
        auto lhs = ops->popOperand(); // dividend

        ASSERT_require(lhs->kind() == ValueKind::Integer64);
        ASSERT_require(rhs->kind() == ValueKind::Integer64);
        ASSERT_require(lhs->nBits() == 64);
        ASSERT_require(rhs->nBits() == 64);

        if (rhs->isConcrete() && rhs->toUnsigned().get() == 0) {
            throw BaseSemantics::Exception("long division by zero", ops->currentInstruction());
        }

        // JVM specifies LONG_MIN / -1 == LONG_MIN rather than overflow.
        if (lhs->isConcrete() && rhs->isConcrete() &&
            lhs->toUnsigned().get() == 0x8000000000000000ull &&
            rhs->toUnsigned().get() == 0xffffffffffffffffull)
        {
            auto result = ops->number_(64, 0x8000000000000000ull);
            result->kind(ValueKind::Integer64);
            ops->pushOperand(result);
            return;
        }

        auto result = ops->signedDivide(lhs, rhs);
        ASSERT_require(result->nBits() == 64);
        result->kind(ValueKind::Integer64);
        ops->pushOperand(result);
    }
};

// lload (22 (0x16))
        // Description:
        //   Load a long value from a local variable and push it.
        // Notes:
        //   The _<n> form uses an implicit local-variable index.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_lload: P {
    void p(D d, Ops ops, I insn, Args args) {
        assert_args(insn, args, 1);
        execute_load(ops, d->asU1(args[0]), LocalKind::Integer64);
    }
};

// lload_0 (30 (0x1e))
        // Description:
        //   Load a long value from a local variable and push it.
        // Notes:
        //   The _<n> form uses an implicit local-variable index.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_lload_0: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        execute_load(ops, 0, LocalKind::Integer64);
    }
};

// lload_1 (31 (0x1f))
        // Description:
        //   Load a long value from a local variable and push it.
        // Notes:
        //   The _<n> form uses an implicit local-variable index.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_lload_1: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        execute_load(ops, 1, LocalKind::Integer64);
    }
};

// lload_2 (32 (0x20))
        // Description:
        //   Load a long value from a local variable and push it.
        // Notes:
        //   The _<n> form uses an implicit local-variable index.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_lload_2: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        execute_load(ops, 2, LocalKind::Integer64);
    }
};

// lload_3 (33 (0x21))
        // Description:
        //   Load a long value from a local variable and push it.
        // Notes:
        //   The _<n> form uses an implicit local-variable index.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_lload_3: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        execute_load(ops, 3, LocalKind::Integer64);
    }
};

// lmul (105 (0x69))
        // Description:
        //   Pop operands, apply the arithmetic/bitwise operation, and push the result.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_lmul: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        doBinaryOp(ops, ValueKind::Integer64,
                   [ops](auto lhs, auto rhs) {
                       auto product = ops->unsignedMultiply(lhs, rhs);
                       return ops->extract(product, 0, 64);
                   });
    }
};

// lneg (117 (0x75))
        // Description:
        //   Pop a numeric value, negate it, and push the result.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_lneg: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        doUnaryOp(ops, ValueKind::Integer64,
             [ops](auto sval) { return ops->negate(sval); });
    }
};

// lookupswitch (171 (0xab))
        // Description:
        //   Pop the int key and branch to the matching case offset or to the default offset.
        // Notes:
        //   Switch operands are padded so defaultbyte begins at a 4-byte boundary from the method start.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_lookupswitch: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        /* Variable-length instruction: decoded operands are supplied by the front end. */
        JvmSemantics::branch_lookupswitch(ops, insn, args);
    }
};

// lor (129 (0x81))
        // Description:
        //   Pop operands, apply the arithmetic/bitwise operation, and push the result.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_lor: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        doBinaryOp(ops, ValueKind::Integer64,
             [ops](auto lhs, auto rhs) { return ops->or_(lhs, rhs); });
    }
};

// lrem (113 (0x71))
        // Description:
        //   Pop operands, apply the arithmetic/bitwise operation, and push the result.
        // Run-time Exceptions:
        //   ArithmeticException if the divisor is zero.
struct IP_lrem: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);

        auto divisor = ops->peekOperand();
        if (divisor->isConcrete() && divisor->toUnsigned().get() == 0) {
            // Eventually: model java/lang/ArithmeticException.
            ASSERT_require2(false, "lrem division by zero");
        }

        // Otherwise calculate the signed modulo.
        doBinaryOp(ops, ValueKind::Integer64,
             [ops](auto lhs, auto rhs) { return ops->signedModulo(lhs, rhs); });
    }
};

// lreturn (173 (0xad))
        // Description:
        //   Return from the current method; typed returns move the return value to the invoker frame, while return returns void.
        // Run-time Exceptions:
        //   IllegalMonitorStateException can be thrown for synchronized methods if structured locking ownership rules are violated.
struct IP_lreturn: P {
    void p(D d, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);

        auto result = ops->popOperand();
        ASSERT_not_null(result);
        ASSERT_require(result->kind() == ValueKind::Integer64);

        d->completeReturn(ops, result);
    }
};

// lshl (121 (0x79))
        // Description:
        //   Pop operands, apply the arithmetic/bitwise operation, and push the result.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_lshl: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);

        auto count = ops->popOperand();
        auto value = ops->popOperand();

        ASSERT_require2(count->kind() == ValueKind::Integer32, "lshl shift count must be Integer32");
        ASSERT_require2(value->kind() == ValueKind::Integer64, "lshl value must be Long");
        ASSERT_require(count->nBits() == 32);
        ASSERT_require(value->nBits() == 64);

        // JVM long shifts use only the low six bits of the shift distance.
        auto maskedCount = ops->and_(count, ops->number_(32, 0x3f));

        auto result = ops->shiftLeft(value, maskedCount);
        result->kind(ValueKind::Integer64);

        ops->pushOperand(result);
    }
};

// lshr (123 (0x7b))
        // Description:
        //   Pop operands, apply the arithmetic/bitwise operation, and push the result.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_lshr: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);

        auto count = ops->popOperand();
        auto value = ops->popOperand();

        ASSERT_require2(count->kind() == ValueKind::Integer32, "lshr shift count must be Integer32");
        ASSERT_require2(value->kind() == ValueKind::Integer64, "lshr value must be Long");
        ASSERT_require(count->nBits() == 32);
        ASSERT_require(value->nBits() == 64);


        // JVM long shifts use only the low six bits of the shift distance.
        auto distance = ops->extract(count, 0, 6);

        auto result = ops->shiftRightArithmetic(value, distance);
        ASSERT_require(result->nBits() == 64);
        result->kind(ValueKind::Integer64);

        ops->pushOperand(result);
    }
};

// lstore (55 (0x37))
        // Description:
        //   Pop a long value and store it into a local variable.
        // Notes:
        //   The _<n> form uses an implicit local-variable index.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_lstore: P {
    void p(D d, Ops ops, I insn, Args args) {
        assert_args(insn, args, 1);
        execute_store(ops, d->asU1(args[0]), LocalKind::Integer64);
    }
};

// lstore_0 (63 (0x3f))
        // Description:
        //   Pop a long value and store it into a local variable.
        // Notes:
        //   The _<n> form uses an implicit local-variable index.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_lstore_0: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        execute_store(ops, 0, LocalKind::Integer64);
    }
};

// lstore_1 (64 (0x40))
        // Description:
        //   Pop a long value and store it into a local variable.
        // Notes:
        //   The _<n> form uses an implicit local-variable index.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_lstore_1: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        execute_store(ops, 1, LocalKind::Integer64);
    }
};

// lstore_2 (65 (0x41))
        // Description:
        //   Pop a long value and store it into a local variable.
        // Notes:
        //   The _<n> form uses an implicit local-variable index.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_lstore_2: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        execute_store(ops, 2, LocalKind::Integer64);
    }
};

// lstore_3 (66 (0x42))
        // Description:
        //   Pop a long value and store it into a local variable.
        // Notes:
        //   The _<n> form uses an implicit local-variable index.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_lstore_3: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        execute_store(ops, 3, LocalKind::Integer64);
    }
};

// lsub (101 (0x65))
        // Description:
        //   Pop operands, apply the arithmetic/bitwise operation, and push the result.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_lsub: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        doBinaryOp(ops, ValueKind::Integer64,
             [ops](auto lhs, auto rhs) { return ops->subtract(lhs, rhs); });
    }
};

// lushr (125 (0x7d))
        // Description:
        //   Pop operands, apply the arithmetic/bitwise operation, and push the result.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_lushr: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);

        auto count = ops->popOperand(); // int
        auto value = ops->popOperand(); // long

        ASSERT_require2(count->kind() == ValueKind::Integer32, "lushr shift distance must be Integer32");
        ASSERT_require2(value->kind() == ValueKind::Integer64, "lushr value must be Integer64");

        // JVM long shifts mask count with 0x3f.
        auto maskedCount = ops->and_(count, ops->number_(32, 0x3f));

        // Use the logical/unsigned right-shift operator, not arithmetic shift.
        auto result = ops->shiftRight(value, maskedCount);
        result->kind(ValueKind::Integer64);

        ops->pushOperand(result);
    }
};

// lxor (131 (0x83))
        // Description:
        //   Pop operands, apply the arithmetic/bitwise operation, and push the result.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_lxor: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        doBinaryOp(ops, ValueKind::Integer64,
             [ops](auto lhs, auto rhs) { return ops->xor_(lhs, rhs); });
    }
};

// monitorenter (194 (0xc2))
        // Description:
        //   Enter or exit the monitor associated with objectref.
        // Run-time Exceptions:
        //   NullPointerException if objectref is null.
struct IP_monitorenter: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        JvmSemantics::execute_monitorenter(ops, insn, args);
    }
};

// monitorexit (195 (0xc3))
        // Description:
        //   Enter or exit the monitor associated with objectref.
        // Run-time Exceptions:
        //   NullPointerException if objectref is null.
        //   IllegalMonitorStateException if the current thread does not own the monitor.
struct IP_monitorexit: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        JvmSemantics::execute_monitorexit(ops, insn, args);
    }
};

// multianewarray (197 (0xc5))
        // Description:
        //   Create a new array, using count values popped from the operand stack, and push the resulting array reference.
        // Notes:
        //   Only the first dimensions operand counts are created; lower dimensions may remain unallocated.
        // Run-time Exceptions:
        //   NegativeArraySizeException if any requested dimension is negative.
struct IP_multianewarray: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 3);
        JvmSemantics::execute_multianewarray(ops, insn, args);
    }
};

// new_ (187 (0xbb))
        // Description:
        //   Resolve the class, allocate an uninitialized instance, and push its reference.
        // Run-time Exceptions:
        //   InstantiationError/IllegalAccessError/other resolution errors can occur as specified by class resolution.
struct IP_new_: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 1);

        auto pool = DispatcherJvm::constantPool(ops);
        ASSERT_not_null(pool);

        auto ivExpr = isSgAsmIntegerValueExpression(args[0]);
        ASSERT_not_null(ivExpr);
        const size_t index = static_cast<uint16_t>(ivExpr->get_absoluteValue());

        // Resolve CONSTANT_Class[index] to its internal class name
        auto entry = pool->get_entry(index);
        ASSERT_not_null(entry);

        std::string className = pool->get_utf8_string(entry->get_name_index());
        const std::string descriptor = "L" + className + ";";
        auto sval = DispatcherJvm::syntheticObjectReference(ops->protoval(), descriptor, className + "::new");

        ops->pushOperand(sval);
    }
};

// newarray (188 (0xbc))
        // Description:
        //   Create a new array, using count values popped from the operand stack, and push the resulting array reference.
        // Run-time Exceptions:
        //   NegativeArraySizeException if any requested dimension is negative.
struct IP_newarray: P {
    void p(D d, Ops ops, I insn, Args args) {
        assert_args(insn, args, 1);
        auto length = ops->popOperand();
        ASSERT_require2(length->kind() == ValueKind::Integer32, "newarray requires Integer32 stack value");

        // Make a copy for the ArrayReference
        SValue::Ptr arrayRef = length->copy();
        arrayRef->kind(ValueKind::ArrayReference);
        arrayRef->arrayLength(length);
        arrayRef->typeDescriptor("<unknown-type>"); // obtained and reset below

        switch (d->asU1(args[0])) {
          case 0x04:
            arrayRef->typeDescriptor("Boolean");
            break;
          case 0x05:
            arrayRef->typeDescriptor("Char");
            break;
          case 0x06:
            arrayRef->typeDescriptor("Float");
            break;
          case 0x07:
            arrayRef->typeDescriptor("Double");
            break;
          case 0x08:
            arrayRef->typeDescriptor("Byte");
            break;
          case 0x09:
            arrayRef->typeDescriptor("Short");
            break;
          case 0x0a:
            arrayRef->typeDescriptor("Integer");
            break;
          case 0x0b:
            arrayRef->typeDescriptor("Long");
            break;
          default:
            ASSERT_require2(false, "unknown type for newarray");
        }

        ops->pushOperand(arrayRef);
    }
};

// nop (0 (0x00))
        // Description:
        //   Do nothing.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_nop: P {
    void p(D /*d*/, Ops /*ops*/, I insn, Args args) {
        assert_args(insn, args, 0);
        /* no operation */
    }
};

// pop (87 (0x57))
        // Description:
        //   Rearrange or discard operand stack values according to the JVM category-1/category-2 stack-form rules.
        // Notes:
        //   These instructions must not be used to split a category-2 value.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_pop: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        auto v1 = ops->popOperand();
        ASSERT_require2(!v1->isJvmCategory2(), "pop may not remove a cat2 object");
    }
};

// pop2 (88 (0x58))
        // Description:
        //   Rearrange or discard operand stack values according to the JVM category-1/category-2 stack-form rules.
        // Notes:
        //   These instructions must not be used to split a category-2 value.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_pop2: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        auto v1 = ops->popOperand();
        if (v1->isJvmCategory2()) {
            // Form 2
        } else {
            // Form 1
            auto v2 = ops->popOperand();
            ASSERT_require2(!v2->isJvmCategory2(), "pop2 may not remove cat1 followed by cat2");
        }
    }
};

// putfield (181 (0xb5))
        // Description:
        //   Resolve the referenced field and read or write the instance/static field value.
        // Run-time Exceptions:
        //   NullPointerException if objectref is null.
struct IP_putfield: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 1);

        auto pool = DispatcherJvm::constantPool(ops);
        ASSERT_not_null(pool);

        auto ivExpr = isSgAsmIntegerValueExpression(args[0]);
        ASSERT_not_null(ivExpr);

        const size_t index = static_cast<uint16_t>(ivExpr->get_absoluteValue());

        // Determine the declared field type.
        const std::string fieldDesc = DispatcherJvm::fieldDescriptor(pool, index);
        const auto descType = DescriptorParser::parseFieldDescriptor(fieldDesc);

        // Stack shape:
        //
        //     ..., objectref, value  ->  ...
        //
        const auto value = ops->popOperand();
        ASSERT_not_null(value);
        ASSERT_require(value->kind() == descType.kind);

        const auto objectRef = ops->popOperand();
        ASSERT_not_null(objectRef);
        ASSERT_require(objectRef->kind() == ValueKind::ObjectReference);

        if (descType.isReference()) {
            ASSERT_require(value->kind() == ValueKind::ObjectReference ||
                           value->kind() == ValueKind::ArrayReference);
        }

        // Without a heap model, the field assignment is not retained.
    }
};

// putstatic (179 (0xb3))
        // Description:
        //   Resolve the referenced field and read or write the instance/static field value.
        // Run-time Exceptions:
        //   ExceptionInInitializerError or other class initialization errors may be observed while initializing the declaring class.
struct IP_putstatic: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 2);
        JvmSemantics::execute_putstatic(ops, insn, args);
    }
};

// ret (169 (0xa9))
        // Description:
        //   Continue execution at the returnAddress stored in the selected local variable.
        // Notes:
        //   Used with jsr/jsr_w subroutines; wide can extend the local-variable index.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_ret: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 1);
        JS::execute_ret(ops, insn, args);
    }
};

// saload (53 (0x35))
        // Description:
        //   Pop arrayref and index; load the short component at index and push the value.
        // Run-time Exceptions:
        //   NullPointerException if arrayref is null.
        //   ArrayIndexOutOfBoundsException if index is outside the array bounds.
struct IP_saload: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        execute_array_load(ops, ArrayElementKind::Short);
    }
};

// sastore (86 (0x56))
        // Description:
        //   Pop value, index, and arrayref; store the short value into the array component.
        // Run-time Exceptions:
        //   NullPointerException if arrayref is null.
        //   ArrayIndexOutOfBoundsException if index is outside the array bounds.
struct IP_sastore: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);
        execute_array_store(ops, ArrayElementKind::Short);
    }
};

// sipush (17 (0x11))
        // Description:
        //   Sign-extend the immediate operand to int and push it.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_sipush: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 1);

        auto ivExpr = isSgAsmIntegerValueExpression(args[0]);
        ASSERT_not_null(ivExpr);

        // sipush has one signed 16-bit immediate operand.
        int16_t imm16 = static_cast<int16_t>(ivExpr->get_signedValue());
        int32_t imm32 = imm16;

        // Create the SValue and set its type/kind
        auto result = ops->number_(32, imm32);
        result->kind(ValueKind::Integer32);

        ops->pushOperand(result);
    }
};

// swap (95 (0x5f))
        // Description:
        //   Rearrange or discard operand stack values according to the JVM category-1/category-2 stack-form rules.
        // Notes:
        //   These instructions must not be used to split a category-2 value.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_swap: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        assert_args(insn, args, 0);

        auto sval1 = ops->popOperand(); // original top
        auto sval2 = ops->popOperand();

        ASSERT_require(FrameState::isCategory1(sval1));
        ASSERT_require(FrameState::isCategory1(sval2));

        ops->pushOperand(sval1);
        ops->pushOperand(sval2);
    }
};

// tableswitch (170 (0xaa))
        // Description:
        //   Pop the int key and branch to the matching case offset or to the default offset.
        // Notes:
        //   Switch operands are padded so defaultbyte begins at a 4-byte boundary from the method start.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_tableswitch: P {
    void p(D /*d*/, Ops ops, I insn, Args args) {
        /* Variable-length instruction: decoded operands are supplied by the front end. */
        JvmSemantics::branch_tableswitch(ops, insn, args);
    }
};

// wide (196 (0xc4))
        // Description:
        //   Modify a following local-variable instruction so it uses a 16-bit local-variable index; for iinc, also use a 16-bit signed constant.
        // Notes:
        //   wide is not a normal standalone execution step; it prefixes the following instruction.
        // Run-time Exceptions:
        //   None specified other than VirtualMachineError subclasses.
struct IP_wide: P {
    void p(D d, Ops ops, I insn, Args args) {
        SValue::Ptr sval;

        size_t index = d->asU2(args[1]);
        const auto opcode = static_cast<JIK>(d->asU1(args[0]));

        switch (opcode) {
            case JIK::iload:
                ASSERT_require(args.size() == 2);
                execute_load(ops, index, LocalKind::Integer32);
                break;
            case JIK::lload:
                ASSERT_require(args.size() == 2);
                execute_load(ops, index, LocalKind::Integer64);
                break;
            case JIK::fload:
                ASSERT_require(args.size() == 2);
                execute_load(ops, index, LocalKind::Float32);
                break;
            case JIK::dload:
                ASSERT_require(args.size() == 2);
                execute_load(ops, index, LocalKind::Float64);
                break;
            case JIK::aload:
                ASSERT_require(args.size() == 2);
                execute_load(ops, index, LocalKind::Reference);
                break;
            case JIK::istore:
                assert_args(insn, args, 2);
                execute_store(ops, index, LocalKind::Integer32);
                break;
            case JIK::lstore:
                ASSERT_require(args.size() == 2);
                execute_store(ops, index, LocalKind::Integer64);
                break;
            case JIK::fstore:
                ASSERT_require(args.size() == 2);
                execute_store(ops, index, LocalKind::Float32);
                break;
            case JIK::dstore:
                ASSERT_require(args.size() == 2);
                execute_store(ops, index, LocalKind::Float64);
                break;
            case JIK::astore:
                ASSERT_require(args.size() == 2);
                execute_store(ops, index, LocalKind::Reference);
                break;
            case JIK::iinc:
                ASSERT_require(args.size() == 3);
                JS::execute_iinc(ops, index, d->asS2(args[2]));
                break;
            case JIK::ret:
                ASSERT_require(args.size() == 2);
                ASSERT_require2(false, "wide ret is not implemented");

            default: ASSERT_not_reachable("instruction cannot be modified by wide");
        }
    }
};

} // namespace

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      DispatcherJvm
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void
DispatcherJvm::initializeDispatchTable() {
    iprocSet(0x00,  new Jvm::IP_nop);
//  iprocSet(0x01,  new Jvm::IP_aconst_null);

    iprocSet(0xbb,  new Jvm::IP_new_);
    iprocSet(0xbc,  new Jvm::IP_newarray);
    iprocSet(0xbd,  new Jvm::IP_anewarray);
    iprocSet(0xbe,  new Jvm::IP_arraylength);
//  iprocSet(0xbf,  new Jvm::IP_athrow);

    iprocSet(0x10,  new Jvm::IP_bipush);
    iprocSet(0x11,  new Jvm::IP_sipush);

    iprocSet(0x19,  new Jvm::IP_aload);
    iprocSet(0x2a,  new Jvm::IP_aload_0);
    iprocSet(0x2b,  new Jvm::IP_aload_1);
    iprocSet(0x2c,  new Jvm::IP_aload_2);
    iprocSet(0x2d,  new Jvm::IP_aload_3);
    iprocSet(0x2e,  new Jvm::IP_iaload);
    iprocSet(0x2f,  new Jvm::IP_laload);
    iprocSet(0x30,  new Jvm::IP_faload);
    iprocSet(0x31,  new Jvm::IP_daload);
    iprocSet(0x32,  new Jvm::IP_aaload);
    iprocSet(0x33,  new Jvm::IP_baload);
    iprocSet(0x34,  new Jvm::IP_caload);
    iprocSet(0x35,  new Jvm::IP_saload);

    iprocSet(0x3a,  new Jvm::IP_astore);
    iprocSet(0x4b,  new Jvm::IP_astore_0);
    iprocSet(0x4c,  new Jvm::IP_astore_1);
    iprocSet(0x4d,  new Jvm::IP_astore_2);
    iprocSet(0x4e,  new Jvm::IP_astore_3);

    iprocSet(0x0b,  new Jvm::IP_fconst_0);
    iprocSet(0x0c,  new Jvm::IP_fconst_1);
    iprocSet(0x0d,  new Jvm::IP_fconst_2);
    iprocSet(0x0e,  new Jvm::IP_dconst_0);
    iprocSet(0x0f,  new Jvm::IP_dconst_1);

    iprocSet(0x17,  new Jvm::IP_fload);
    iprocSet(0x18,  new Jvm::IP_dload);

    iprocSet(0x22,  new Jvm::IP_fload_0);
    iprocSet(0x23,  new Jvm::IP_fload_1);
    iprocSet(0x24,  new Jvm::IP_fload_2);
    iprocSet(0x25,  new Jvm::IP_fload_3);

    iprocSet(0x26,  new Jvm::IP_dload_0);
    iprocSet(0x27,  new Jvm::IP_dload_1);
    iprocSet(0x28,  new Jvm::IP_dload_2);
    iprocSet(0x29,  new Jvm::IP_dload_3);

    iprocSet(0x38,  new Jvm::IP_fstore);
    iprocSet(0x39,  new Jvm::IP_dstore);

    iprocSet(0x43,  new Jvm::IP_fstore_0);
    iprocSet(0x44,  new Jvm::IP_fstore_1);
    iprocSet(0x45,  new Jvm::IP_fstore_2);
    iprocSet(0x46,  new Jvm::IP_fstore_3);

    iprocSet(0x47,  new Jvm::IP_dstore_0);
    iprocSet(0x48,  new Jvm::IP_dstore_1);
    iprocSet(0x49,  new Jvm::IP_dstore_2);
    iprocSet(0x4a,  new Jvm::IP_dstore_3);

    iprocSet(0x4f,  new Jvm::IP_iastore);
    iprocSet(0x50,  new Jvm::IP_lastore);
    iprocSet(0x51,  new Jvm::IP_fastore);
    iprocSet(0x52,  new Jvm::IP_dastore);
    iprocSet(0x53,  new Jvm::IP_aastore);
    iprocSet(0x54,  new Jvm::IP_bastore);
    iprocSet(0x55,  new Jvm::IP_castore);
    iprocSet(0x56,  new Jvm::IP_sastore);

    iprocSet(0x02,  new Jvm::IP_iconst_m1);
    iprocSet(0x03,  new Jvm::IP_iconst_0);
    iprocSet(0x04,  new Jvm::IP_iconst_1);
    iprocSet(0x05,  new Jvm::IP_iconst_2);
    iprocSet(0x06,  new Jvm::IP_iconst_3);
    iprocSet(0x07,  new Jvm::IP_iconst_4);
    iprocSet(0x08,  new Jvm::IP_iconst_5);

    iprocSet(0x12,  new Jvm::IP_ldc);
    iprocSet(0x13,  new Jvm::IP_ldc_w);
    iprocSet(0x14,  new Jvm::IP_ldc2_w);
    iprocSet(0x15,  new Jvm::IP_iload);
    iprocSet(0x1a,  new Jvm::IP_iload_0);
    iprocSet(0x1b,  new Jvm::IP_iload_1);
    iprocSet(0x1c,  new Jvm::IP_iload_2);
    iprocSet(0x1d,  new Jvm::IP_iload_3);

    iprocSet(0x36,  new Jvm::IP_istore);
    iprocSet(0x3b,  new Jvm::IP_istore_0);
    iprocSet(0x3c,  new Jvm::IP_istore_1);
    iprocSet(0x3d,  new Jvm::IP_istore_2);
    iprocSet(0x3e,  new Jvm::IP_istore_3);

    iprocSet(0x09,  new Jvm::IP_lconst_0);
    iprocSet(0x0a,  new Jvm::IP_lconst_1);

    iprocSet(0x16,  new Jvm::IP_lload);
    iprocSet(0x1e,  new Jvm::IP_lload_0);
    iprocSet(0x1f,  new Jvm::IP_lload_1);
    iprocSet(0x20,  new Jvm::IP_lload_2);
    iprocSet(0x21,  new Jvm::IP_lload_3);

    iprocSet(0x37,  new Jvm::IP_lstore);
    iprocSet(0x3f,  new Jvm::IP_lstore_0);
    iprocSet(0x40,  new Jvm::IP_lstore_1);
    iprocSet(0x41,  new Jvm::IP_lstore_2);
    iprocSet(0x42,  new Jvm::IP_lstore_3);

    iprocSet(0x57,  new Jvm::IP_pop);
    iprocSet(0x58,  new Jvm::IP_pop2);
    iprocSet(0x59,  new Jvm::IP_dup);
    iprocSet(0x5a,  new Jvm::IP_dup_x1);
    iprocSet(0x5b,  new Jvm::IP_dup_x2);
    iprocSet(0x5c,  new Jvm::IP_dup2);
    iprocSet(0x5d,  new Jvm::IP_dup2_x1);
    iprocSet(0x5e,  new Jvm::IP_dup2_x2);
    iprocSet(0x5f,  new Jvm::IP_swap);

    // Binary operators
    iprocSet(0x60,  new Jvm::IP_iadd);
    iprocSet(0x61,  new Jvm::IP_ladd);
    iprocSet(0x62,  new Jvm::IP_fadd);
    iprocSet(0x63,  new Jvm::IP_dadd);
    iprocSet(0x64,  new Jvm::IP_isub);
    iprocSet(0x65,  new Jvm::IP_lsub);
    iprocSet(0x66,  new Jvm::IP_fsub);
    iprocSet(0x67,  new Jvm::IP_dsub);
    iprocSet(0x68,  new Jvm::IP_imul);
    iprocSet(0x69,  new Jvm::IP_lmul);
    iprocSet(0x6a,  new Jvm::IP_fmul);
    iprocSet(0x6b,  new Jvm::IP_dmul);
    iprocSet(0x6c,  new Jvm::IP_idiv);
    iprocSet(0x6d,  new Jvm::IP_ldiv);
    iprocSet(0x6e,  new Jvm::IP_fdiv);
    iprocSet(0x6f,  new Jvm::IP_ddiv);
    iprocSet(0x70,  new Jvm::IP_irem);
    iprocSet(0x71,  new Jvm::IP_lrem);
    iprocSet(0x72,  new Jvm::IP_frem);
    iprocSet(0x73,  new Jvm::IP_drem);

    // Unary(ish) operators
    iprocSet(0x74,  new Jvm::IP_ineg);
    iprocSet(0x75,  new Jvm::IP_lneg);
    iprocSet(0x76,  new Jvm::IP_fneg);
    iprocSet(0x77,  new Jvm::IP_dneg);
    iprocSet(0x78,  new Jvm::IP_ishl);
    iprocSet(0x79,  new Jvm::IP_lshl);
    iprocSet(0x7a,  new Jvm::IP_ishr);
    iprocSet(0x7b,  new Jvm::IP_lshr);
    iprocSet(0x7c,  new Jvm::IP_iushr);
    iprocSet(0x7d,  new Jvm::IP_lushr);
    iprocSet(0x7e,  new Jvm::IP_iand);
    iprocSet(0x7f,  new Jvm::IP_land);
    iprocSet(0x80,  new Jvm::IP_ior);
    iprocSet(0x81,  new Jvm::IP_lor);
    iprocSet(0x82,  new Jvm::IP_ixor);
    iprocSet(0x83,  new Jvm::IP_lxor);
    iprocSet(0x84,  new Jvm::IP_iinc);
    iprocSet(0x85,  new Jvm::IP_i2l);
    iprocSet(0x86,  new Jvm::IP_i2f);
    iprocSet(0x87,  new Jvm::IP_i2d);
    iprocSet(0x88,  new Jvm::IP_l2i);
    iprocSet(0x89,  new Jvm::IP_l2f);
    iprocSet(0x8a,  new Jvm::IP_l2d);
    iprocSet(0x8b,  new Jvm::IP_f2i);
    iprocSet(0x8c,  new Jvm::IP_f2l);
    iprocSet(0x8d,  new Jvm::IP_f2d);
    iprocSet(0x8e,  new Jvm::IP_d2i);
    iprocSet(0x8f,  new Jvm::IP_d2l);
    iprocSet(0x90,  new Jvm::IP_d2f);
    iprocSet(0x91,  new Jvm::IP_i2b);
    iprocSet(0x92,  new Jvm::IP_i2c);
    iprocSet(0x93,  new Jvm::IP_i2s);

//  iprocSet(0x94,  new Jvm::IP_lcmp);
//  iprocSet(0x95,  new Jvm::IP_fcml);
//  iprocSet(0x96,  new Jvm::IP_fcmpg);
//  iprocSet(0x97,  new Jvm::IP_dcmpl);
//  iprocSet(0x98,  new Jvm::IP_dcmpg);
    iprocSet(0x99,  new Jvm::IP_ifeq);
    iprocSet(0x9a,  new Jvm::IP_ifne);
    iprocSet(0x9b,  new Jvm::IP_iflt);
    iprocSet(0x9c,  new Jvm::IP_ifge);
    iprocSet(0x9d,  new Jvm::IP_ifgt);
    iprocSet(0x9e,  new Jvm::IP_ifle);

//  iprocSet(0x9f,  new Jvm::IP_if_icmpeq); // if_icmpeq (159 (0x9f))
//  iprocSet(0xa0,  new Jvm::IP_if_icmpne); // if_icmpne (160 (0xa0))
//  iprocSet(0xa1,  new Jvm::IP_if_icmplt); // if_icmplt (161 (0xa1))
//  iprocSet(0xa2,  new Jvm::IP_if_icmpge); // if_icmpge (162 (0xa2))
//  iprocSet(0xa3,  new Jvm::IP_if_icmpgt); // if_icmpgt (163 (0xa3))
//  iprocSet(0xa4,  new Jvm::IP_if_icmple); // if_icmple (164 (0xa4))
//  iprocSet(0xa5,  new Jvm::IP_if_acmpeq); // if_acmpeq (165 (0xa5))
//  iprocSet(0xa6,  new Jvm::IP_if_acmpne); // if_acmpne (166 (0xa6))

    iprocSet(0xa7,  new Jvm::IP_goto_);
//  iprocSet(0xa8,  new Jvm::IP_jsr);
//  iprocSet(0xa9,  new Jvm::IP_ret);
//  iprocSet(0xaa,  new Jvm::IP_tableswitch);
//  iprocSet(0xab,  new Jvm::IP_lookupswitch);
    iprocSet(0xac,  new Jvm::IP_ireturn);
    iprocSet(0xad,  new Jvm::IP_lreturn);
    iprocSet(0xae,  new Jvm::IP_freturn);
    iprocSet(0xaf,  new Jvm::IP_dreturn);
    iprocSet(0xb0,  new Jvm::IP_areturn);

    iprocSet(0xb1,  new Jvm::IP_return_);
    iprocSet(0xb2,  new Jvm::IP_getstatic);
//  iprocSet(0xb3,  new Jvm::IP_putstatic);
    iprocSet(0xb4,  new Jvm::IP_getfield);
    iprocSet(0xb5,  new Jvm::IP_putfield);

    iprocSet(0xb6,  new Jvm::IP_invokevirtual);
    iprocSet(0xb7,  new Jvm::IP_invokespecial);
    iprocSet(0xb8,  new Jvm::IP_invokestatic);
//  iprocSet(0xb9,  new Jvm::IP_invokeinterface);
//  iprocSet(0xba,  new Jvm::IP_invokedynamic);
//  iprocSet(0xc0,  new Jvm::IP_checkcast);
//  iprocSet(0xc1,  new Jvm::IP_instanceof);
//  iprocSet(0xc2,  new Jvm::IP_monitorenter);
//  iprocSet(0xc3,  new Jvm::IP_monitorexit);
    iprocSet(0xc4,  new Jvm::IP_wide);
//  iprocSet(0xc5,  new Jvm::IP_multianewarray);

    iprocSet(0xc6,  new Jvm::IP_ifnull);
    iprocSet(0xc7,  new Jvm::IP_ifnonnull);
//  iprocSet(0xc8,  new Jvm::IP_goto_w);
//  iprocSet(0xc9,  new Jvm::IP_jsr_w);

//  breakpoint = 202, // 0xca
//  impdep1    = 254, // 0xfe
//  impdep2    = 255, // 0xff
//  unknown    = 666  // unknown/illegal opcode
}

DispatcherJvm::~DispatcherJvm() {}

DispatcherJvm::DispatcherJvm(const Architecture::Base::ConstPtr &arch)
    : Super(arch) {}

DispatcherJvm::DispatcherJvm(const Architecture::Base::ConstPtr &arch, const RiscOperators::Ptr &ops)
    : Super(arch, ops),
      REG_PC(arch->registerDictionary()->findOrThrow("pc")),
      REG_SP(arch->registerDictionary()->findOrThrow("sp")),
      REG_FP(arch->registerDictionary()->findOrThrow("fp")) {
    initializeDispatchTable();
    initializeMemoryState();
}

DispatcherJvm::Ptr
DispatcherJvm::instance(const Architecture::Base::ConstPtr &arch) {
    return Ptr(new DispatcherJvm(arch));
}

DispatcherJvm::Ptr
DispatcherJvm::instance(const Architecture::BaseConstPtr &arch,
                        const BaseSemantics::RiscOperatorsPtr &ops) {
    return Ptr(new DispatcherJvm(arch, ops));
}

BaseSemantics::Dispatcher::Ptr
DispatcherJvm::create(const BaseSemantics::RiscOperators::Ptr &ops) const {
    return instance(architecture(), ops);
}

DispatcherJvm::Ptr
DispatcherJvm::promote(const BaseSemantics::Dispatcher::Ptr &d) {
    Ptr retval = boost::dynamic_pointer_cast<DispatcherJvm>(d);
    ASSERT_not_null(retval);
    return retval;
}

int
DispatcherJvm::iprocKey(SgAsmInstruction *insn_) const {
    SgAsmJvmInstruction *insn = isSgAsmJvmInstruction(insn_);
    ASSERT_not_null(insn);
    return static_cast<int>(insn->get_kind());
}

RegisterDescriptor
DispatcherJvm::instructionPointerRegister() const {
    return REG_PC;
}

SgAsmJvmConstantPool*
DispatcherJvm::constantPool(BaseSemantics::RiscOperators *ops) {
    auto state = ops->currentState();
    ASSERT_not_null(state);

    auto frame = state->currentFrame();
    ASSERT_not_null(frame);

    auto jvmMethod = ByteCode::JvmMethod::promote(frame->method());
    ASSERT_not_null(jvmMethod);
    auto pool = jvmMethod->constant_pool();
    ASSERT_not_null(pool);

    return pool;
}

size_t
DispatcherJvm::nBitsForKind(ValueKind kind) {
    switch (kind) {
        case ValueKind::Integer32:
        case ValueKind::Float32:
            return 32;

        case ValueKind::Integer64:
        case ValueKind::Float64:
            return 64;

        case ValueKind::ObjectReference:
        case ValueKind::ArrayReference:
            return 32;

        default:
            ASSERT_not_reachable("cannot determine bit width for JVM value kind");
    }
}

std::string
DispatcherJvm::fieldDescriptor(SgAsmJvmConstantPool *pool, size_t index) {
    auto entry = pool->get_entry(index);
    ASSERT_not_null(entry);

    ASSERT_require(entry->get_tag() == SgAsmJvmConstantPoolEntry::CONSTANT_Fieldref);
    entry = pool->get_entry(entry->get_name_and_type_index());

    ASSERT_require(entry->get_tag() == SgAsmJvmConstantPoolEntry::CONSTANT_NameAndType);

    return pool->get_utf8_string(entry->get_descriptor_index());
}

std::string
DispatcherJvm::methodDescriptor(SgAsmJvmConstantPool *pool, size_t index) {
    auto entry = pool->get_entry(index);
    ASSERT_not_null(entry);

    ASSERT_require(entry->get_tag() == SgAsmJvmConstantPoolEntry::CONSTANT_Methodref);
    entry = pool->get_entry(entry->get_name_and_type_index());

    ASSERT_require(entry->get_tag() == SgAsmJvmConstantPoolEntry::CONSTANT_NameAndType);

    return pool->get_utf8_string(entry->get_descriptor_index());
}

BaseSemantics::SValuePtr
DispatcherJvm::syntheticValue(const BaseSemantics::SValuePtr &protoval,
                              const DescriptorType &type,
                              const std::string &symbolName) {
    ASSERT_not_null(protoval);
    BaseSemantics::SValuePtr value;

    switch (type.kind) {
        case ValueKind::ObjectReference:
            value = syntheticObjectReference(protoval, type.descriptor, symbolName);
            break;

        case ValueKind::ArrayReference:
            value = syntheticArrayReference(protoval, type.descriptor, symbolName);
            break;

        case ValueKind::Integer32:
        case ValueKind::Integer64:
        case ValueKind::Float32:
        case ValueKind::Float64:
            value = protoval->undefined_(nBitsForKind(type.kind));
            ASSERT_not_null(value);

            value->kind(type.kind);

            if (!symbolName.empty())
                value->symbolName(symbolName);
            break;

        default:
            ASSERT_not_reachable("cannot create a synthetic value for this JVM value kind");
    }

    ASSERT_not_null(value);
    return value;
}

BaseSemantics::SValuePtr
DispatcherJvm::syntheticObjectReference(const BaseSemantics::SValuePtr &protoval,
                                        const std::string &descriptor,
                                        const std::string &symbolName) {
    ASSERT_not_null(protoval);
    ASSERT_require(descriptor.size() >= 2);
    ASSERT_require(descriptor.front() == 'L');
    ASSERT_require(descriptor.back() == ';');

    BaseSemantics::SValuePtr reference = protoval->undefined_(protoval->nBits());
    ASSERT_not_null(reference);

    reference->kind(BaseSemantics::ValueKind::ObjectReference);
    reference->typeDescriptor(descriptor);

    if (!symbolName.empty()) {
        reference->symbolName(symbolName);
    }

    return reference;
}

BaseSemantics::SValuePtr
DispatcherJvm::syntheticArrayReference(const BaseSemantics::SValuePtr &protoval,
                                       const std::string &descriptor,
                                       const std::string &symbolName) {
    ASSERT_not_null(protoval);
    ASSERT_require(!descriptor.empty());
    ASSERT_require(descriptor.front() == '[');

    BaseSemantics::SValuePtr reference = protoval->undefined_(protoval->nBits());
    ASSERT_not_null(reference);

    reference->kind(BaseSemantics::ValueKind::ArrayReference);
    reference->typeDescriptor(descriptor);

    if (!symbolName.empty()) {
        reference->symbolName(symbolName);
    }

    // The array type is known, but its length is not.
    BaseSemantics::SValuePtr length = protoval->undefined_(32);
    ASSERT_not_null(length);

    length->kind(BaseSemantics::ValueKind::Integer32);
    reference->arrayLength(length);

    return reference;
}

void
DispatcherJvm::initializeInvocationLocals(BaseSemantics::RiscOperators *ops,
                                          const BaseSemantics::FrameState::Ptr &calleeFrame,
                                          const std::string &descriptor, bool hasReceiver) {
    ASSERT_not_null(ops);
    ASSERT_not_null(calleeFrame);
    size_t localIndex{0};

    const auto parser = DescriptorParser::parseMethodDescriptor(descriptor);

    // The last declared argument is at the top of the caller's operand stack.
    // Save the popped values in declaration order.
    std::vector<BaseSemantics::SValue::Ptr> args(parser.arguments.size());

    for (size_t i = args.size(); i > 0; --i) {
        args[i-1] = ops->popOperand();
    }

    BaseSemantics::SValue::Ptr receiver{nullptr};
    if (hasReceiver) {
        receiver = ops->popOperand(); // "this" pointer
        calleeFrame->writeLocal(localIndex++, receiver);
    }

    // Place explicit arguments into locals in declaration order.
    for (size_t i = 0; i < args.size(); ++i) {
        calleeFrame->writeLocal(localIndex, args[i]);
        localIndex += parser.arguments[i].isCategory2() ? 2 : 1;
    }
}

void
DispatcherJvm::initializeMemoryState() {
    if (State::Ptr state = currentState()) {
        if (MemoryState::Ptr memory = state->memoryState()) {
            switch (memory->get_byteOrder()) {
                case ByteOrder::BE:
                case ByteOrder::EL:
                    break;
                case ByteOrder::ORDER_UNSPECIFIED:
                    memory->set_byteOrder(architecture()->byteOrder());
                    break;
            }
        }
    }
}

void
DispatcherJvm::completeReturn(BaseSemantics::RiscOperators *ops, const BaseSemantics::SValuePtr &result) {
    ASSERT_not_null(ops);

    auto state = ops->currentState();
    ASSERT_not_null(state);

    auto calleeFrame = state->currentFrame();
    ASSERT_not_null(calleeFrame);

    // Save before removing the frame.
    const auto returnAddress = calleeFrame->returnAddress();

    auto poppedFrame = state->popFrame();
    ASSERT_require(poppedFrame == calleeFrame);

    if (returnAddress) {
        // The caller frame is now current.
        if (result) {
            ops->pushOperand(result);
        }
        const RegisterDescriptor pcReg = instructionPointerRegister();
        ops->writeRegister(pcReg, ops->number_(pcReg.nBits(), *returnAddress));
    }
    else {
        // The root analysis frame returned.
        //
        // If desired, store result as the final analysis result here.
        // For a void return, result is null.
    }
}

void
DispatcherJvm::recordSemanticError(const std::string &msg) {
//TODO: This should probably be improved (a switch/command-line option)
#if MOVE_ON
    operators()->comment(msg);
#else
    throw Rose::Exception(msg);
#endif
}

} // namespace
} // namespace
} // namespace

#ifdef ROSE_HAVE_BOOST_SERIALIZATION_LIB
BOOST_CLASS_EXPORT_IMPLEMENT(Rose::BinaryAnalysis::InstructionSemantics::DispatcherJvm);
#endif

#endif
