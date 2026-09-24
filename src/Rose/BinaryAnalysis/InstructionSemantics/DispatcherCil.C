#include <featureTests.h>
#ifdef ROSE_ENABLE_BINARY_ANALYSIS
#include <Rose/BinaryAnalysis/InstructionSemantics/DispatcherCil.h>

#include <Rose/Affirm.h>
#include <Rose/As.h>
#include <Rose/BinaryAnalysis/Architecture/Base.h>
#include <Rose/BinaryAnalysis/InstructionEnumsCil.h>
#include <Rose/BinaryAnalysis/InstructionSemantics/BaseSemantics/Dispatcher.h>
#include <Rose/BinaryAnalysis/InstructionSemantics/BaseSemantics/Exception.h>
#include <Rose/BinaryAnalysis/InstructionSemantics/BaseSemantics/FrameState.h>
#include <Rose/BinaryAnalysis/InstructionSemantics/BaseSemantics/MemoryState.h>
#include <Rose/BinaryAnalysis/InstructionSemantics/BaseSemantics/RiscOperators.h>
#include <Rose/BinaryAnalysis/InstructionSemantics/BaseSemantics/State.h>
#include <Rose/StringUtility/Diagnostics.h>

#include <SgAsmCilInstruction.h>
#include <SgAsmFloatType.h>
#include <SgAsmExpression.h>
#include <SgAsmIntegerValueExpression.h>
#include <SgAsmOperandList.h>
#include <SgAsmValueExpression.h>

// DQ (6/4/2026): Adding support for getEnclosingNode<SgAsmPEFileHeader>(cilInsn).
#include <sageInterface.h>

#include <Cxx_GrammarDowncast.h>

#include <algorithm>
#include <climits>
#include <cmath>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <limits>
#include <string>

using namespace Rose::BinaryAnalysis::InstructionSemantics::BaseSemantics;

namespace Rose {
namespace BinaryAnalysis {
namespace InstructionSemantics {

////////////////////////////////////////////////////
// Added type analysis support for CIL semantics.
////////////////////////////////////////////////////

void
DispatcherCil::CilTypeAnalysis::setPossibleTypes(const SValuePtr &v, const TypeSet &types)
   {
     ASSERT_not_null(v);
  // possibleTypes_[v.get()] = types;
     possibleTypes_[v.operator->()] = types;
   }

void
DispatcherCil::CilTypeAnalysis::addPossibleType(const SValuePtr &v, TypeToken type)
   {
     ASSERT_not_null(v);
  // possibleTypes_[v.get()].insert(type);
     possibleTypes_[v.operator->()].insert(type);
   }

DispatcherCil::CilTypeAnalysis::TypeSet
DispatcherCil::CilTypeAnalysis::possibleDynamicTypes(const SValuePtr &v) const
   {
     ASSERT_not_null(v);

  // auto found = possibleTypes_.find(v.get());
     auto found = possibleTypes_.find(v.operator->());
     if (found != possibleTypes_.end())
        return found->second;

  // Unknown object type.
     return TypeSet{};
   }

bool
DispatcherCil::CilTypeAnalysis::hasTypeInfo(const SValuePtr &v) const
   {
     ASSERT_not_null(v);
  // return possibleTypes_.find(v.get()) != possibleTypes_.end();
     return possibleTypes_.find(v.operator->()) != possibleTypes_.end();
   }

void
DispatcherCil::CilTypeAnalysis::rememberTypeDescriptor(const TypeDescriptor &type)
   {
     if (!type.isValid())
          return;

     std::map<TypeToken, TypeDescriptor>::iterator found = knownTypes_.find(type.metadataToken);
     if (found == knownTypes_.end() || type.isResolved() || !found->second.isResolved())
          knownTypes_[type.metadataToken] = type;
   }

const DispatcherCil::CilTypeAnalysis::TypeDescriptor*
DispatcherCil::CilTypeAnalysis::typeDescriptorFor(TypeToken type) const
   {
     std::map<TypeToken, TypeDescriptor>::const_iterator found = knownTypes_.find(type);
     return found == knownTypes_.end() ? nullptr : &found->second;
   }

#if 0
// DQ (6/17/2026): I can't get this to compile outside of the class definition, so leave it there.
static const void*
DispatcherCil::CilTypeAnalysis::key(const SValuePtr &v)
   {
     ASSERT_not_null(v);

  // SValuePtr is Sawyer::SharedPointer, not std::shared_ptr,
  // so it does not have v.get().
     return static_cast<const void*>(v.operator->());
   }
#endif

void
DispatcherCil::CilTypeAnalysis::setNullness(const SValuePtr &v, Nullness n)
   {
     ASSERT_not_null(v);
     nullness_[key(v)] = n;
   }

DispatcherCil::CilTypeAnalysis::Nullness
DispatcherCil::CilTypeAnalysis::nullness(const SValuePtr &v) const
   {
     ASSERT_not_null(v);

     auto found = nullness_.find(key(v));
     if (found != nullness_.end())
          return found->second;

     return NullnessUnknown;
   }

bool
DispatcherCil::CilTypeAnalysis::isDefinitelyNull(const SValuePtr &v) const
   {
     return nullness(v) == DefinitelyNull;
   }

bool
DispatcherCil::CilTypeAnalysis::isDefinitelyNonNull(const SValuePtr &v) const
   {
     return nullness(v) == DefinitelyNonNull;
   }


#if 0
bool
DispatcherCil::CilTypeAnalysis::isAssignableTo(uint32_t sourceType, const DispatcherCil::CilTypeAnalysis::TypeDescriptor *targetType)
   {
     ASSERT_not_null(targetType);

  // First approximation:
  // a type is assignable to itself.
  // if (sourceType == targetType->metadataToken)
  //      return true;

  // TODO:
  // - System.Object
  // - base classes
  // - interfaces
  // - arrays
  // - generic TypeSpec rules

     if (possibleTypes.empty())
          return unknown;

     if target is System.Object:
          return true for all reference types;

     if source == target:
          return true;

     otherwise:
          return unknown, not definitely false.
      
     return false;
   }
#else

     enum class Assignability
        {
          No,
          Yes,
          Unknown
        };

  // Old function signatures:
  // bool isAssignableTo(uint32_t sourceType, const TypeDescriptor *targetType);
  // bool allAssignableTo(const DispatcherCil::CilTypeAnalysis::TypeSet &possibleTypes, const TypeDescriptor *targetType);
  // bool noneAssignableTo(const DispatcherCil::CilTypeAnalysis::TypeSet &possibleTypes, const TypeDescriptor *targetType);

  // New function signatures:
  // Assignability isAssignableTo(uint32_t sourceType, const TypeDescriptor *targetType);
  // bool allAssignableTo(const DispatcherCil::CilTypeAnalysis::TypeSet &possibleTypes, const TypeDescriptor *targetType);
  // bool noneAssignableTo(const DispatcherCil::CilTypeAnalysis::TypeSet &possibleTypes, const TypeDescriptor *targetType);
#if 0
  // static Assignability isAssignableTo(const DispatcherCil::CilTypeAnalysis::TypeDescriptor &source, const DispatcherCil::CilTypeAnalysis::TypeDescriptor &target)
     Assignability isAssignableTo(uint32_t sourceType, const TypeDescriptor *targetType);
        {
       // No source information means we cannot prove either yes or no.
          if (!source.isKnown())
               return Assignability::Unknown;

       // No target information also means unknown.
          if (!target.isKnown())
               return Assignability::Unknown;

       // Exact type match is definitely assignable.
          if (source.token == target.token)
               return Assignability::Yes;

       // Everything that is a managed reference type is assignable to System.Object.
          if (target.isSystemObject())
             {
               if (source.isReferenceType())
                    return Assignability::Yes;

               return Assignability::Unknown;
             }

       // Optional: null is assignable to any reference type.
          if (source.isNullType())
             {
               if (target.isReferenceType())
                    return Assignability::Yes;

               return Assignability::No;
             }

       // If you do not yet have class hierarchy/interface information, do NOT return No here.
          return Assignability::Unknown;
        }
#else
DispatcherCil::CilTypeAnalysis::Assignability
DispatcherCil::CilTypeAnalysis::isAssignableTo(
        uint32_t sourceType,
        const DispatcherCil::CilTypeAnalysis::TypeDescriptor *targetType)
   {
     ASSERT_not_null(targetType);

  // If the target metadata token is unknown, we cannot prove yes or no.
     if (!targetType->isValid())
          return AssignabilityUnknown;

  // Exact metadata-token match is definitely assignable.
     if (sourceType == targetType->metadataToken)
          return AssignabilityYes;

     const TypeDescriptor *source = typeDescriptorFor(sourceType);

  // If metadata resolution has identified System.Object, every resolved
  // managed runtime type we track here is assignment-compatible with it.
     if (targetType->name == "System.Object")
        {
          if (source && source->isResolved())
               return AssignabilityYes;
          return AssignabilityUnknown;
        }

  // A resolved non-identical value type is not assignment-compatible with a
  // different value type. Leave value-type-to-reference cases mostly unknown:
  // boxed value types can be compatible with System.ValueType, System.Object,
  // and implemented interfaces, and interface metadata is optional.
     if (source && source->isKnownValueType() && targetType->isKnownValueType())
          return AssignabilityNo;

  // Use optional interface metadata when a provider supplies it. The set is
  // interpreted as all directly or transitively implemented/inherited
  // interfaces for the source descriptor.
     if (source && targetType->isInterface && source->hasKnownInterfaceTypes() &&
         source->interfaceTypeTokens.find(targetType->metadataToken) != source->interfaceTypeTokens.end())
        {
          return AssignabilityYes;
        }

  // Walk optional base-class metadata. Stop conservatively if the chain is
  // incomplete or a referenced descriptor has not been resolved/cached.
     std::set<uint32_t> visited;
     const TypeDescriptor *current = source;
     while (current && current->hasKnownBaseType() && current->baseTypeToken != 0)
        {
          if (!visited.insert(current->metadataToken).second)
               return AssignabilityUnknown;

          if (current->baseTypeToken == targetType->metadataToken)
               return AssignabilityYes;

          current = typeDescriptorFor(current->baseTypeToken);

          if (current && targetType->isInterface && current->hasKnownInterfaceTypes() &&
              current->interfaceTypeTokens.find(targetType->metadataToken) != current->interfaceTypeTokens.end())
             {
               return AssignabilityYes;
             }
        }

  // Without complete base-class/interface/array/generic metadata, non-equality
  // is not proof of non-assignability. It might be a subclass, interface
  // implementation, array covariance case, generic TypeSpec rule, etc.
     return AssignabilityUnknown;
   }
#endif
#endif

#if 0
bool
DispatcherCil::CilTypeAnalysis::allAssignableTo(const DispatcherCil::CilTypeAnalysis::TypeSet &possibleTypes,
                const DispatcherCil::CilTypeAnalysis::TypeDescriptor *targetType)
   {
     ASSERT_not_null(targetType);

     if (possibleTypes.empty())
     // return false; // unknown, not definitely assignable
     // return Assignability::Unknown;
     // return AssignabilityUnknown;
        return false; // unknown, not definitely assignable

     for (uint32_t sourceType: possibleTypes)
        {
#if 0
          if (!isAssignableTo(sourceType, targetType))
               return false;
#else
          switch (isAssignableTo(sourceType, targetType))
             {
            // case Assignability::Yes:
               case AssignabilityYes:
            // definitely assignable
               break;

            // case Assignability::No:
               case AssignabilityNo:
            // definitely not assignable
               break;

            // case Assignability::Unknown:
               case AssignabilityUnknown:
            // cannot prove; keep conservative normal semantics
               break;
             }
#endif
        }

     return true;
   }
#else
bool
DispatcherCil::CilTypeAnalysis::allAssignableTo(
    const TypeSet &possibleTypes,
    const TypeDescriptor *targetType)
{
    ASSERT_not_null(targetType);

    if (possibleTypes.empty())
        return false; // unknown, not definitely all assignable

    for (uint32_t sourceType: possibleTypes) {
        if (isAssignableTo(sourceType, targetType) != AssignabilityYes)
            return false; // No or Unknown both mean "not proven"
    }

    return true;
}
#endif

#if 0
bool
DispatcherCil::CilTypeAnalysis::noneAssignableTo(const DispatcherCil::CilTypeAnalysis::TypeSet &possibleTypes,
                 const DispatcherCil::CilTypeAnalysis::TypeDescriptor *targetType)
{
    ASSERT_not_null(targetType);

    if (possibleTypes.empty())
     // return false; // unknown, not definitely impossible
     // return Assignability::Unknown;
     // return AssignabilityUnknown;
        return false; // unknown, not definitely assignable

    for (uint32_t sourceType: possibleTypes) {
#if 0
        if (isAssignableTo(sourceType, targetType))
            return false;
#else
          switch (isAssignableTo(sourceType, targetType))
             {
            // case Assignability::Yes:
               case AssignabilityYes:
            // definitely assignable
               break;

            // case Assignability::No:
               case AssignabilityNo:
            // definitely not assignable
               break;

            // case Assignability::Unknown:
               case AssignabilityUnknown:
            // cannot prove; keep conservative normal semantics
               break;
             }
#endif
    }

    return true;
}
#else
bool
DispatcherCil::CilTypeAnalysis::noneAssignableTo(
    const TypeSet &possibleTypes,
    const TypeDescriptor *targetType)
{
    ASSERT_not_null(targetType);

    if (possibleTypes.empty())
        return false; // unknown, not definitely none assignable

    for (uint32_t sourceType: possibleTypes) {
        if (isAssignableTo(sourceType, targetType) != AssignabilityNo)
            return false; // Yes or Unknown both mean "not proven impossible"
    }

    return true;
}
#endif
  
void
DispatcherCil::CilTypeAnalysis::copyFacts(const SValuePtr &dst, const SValuePtr &src)
   {
     ASSERT_not_null(dst);
     ASSERT_not_null(src);

     auto typeFound = possibleTypes_.find(key(src));
     if (typeFound != possibleTypes_.end())
          possibleTypes_[key(dst)] = typeFound->second;

     auto nullFound = nullness_.find(key(src));
     if (nullFound != nullness_.end())
          nullness_[key(dst)] = nullFound->second;
   }

DispatcherCil::CilTypeAnalysis::FieldDescriptor
DispatcherCil::CilTypeAnalysis::resolveField(DispatcherCil* dispatcher, uint32_t token)
   {
     if (dispatcher && dispatcher->cilMetadata())
          return dispatcher->cilMetadata()->resolveField(token);
     return DispatcherCil::CilTypeAnalysis::FieldDescriptor(token);
   }
               

void
DispatcherCil::CilPendingPrefixState::clear()
   {
     hasUnaligned         = false;
     unalignedBytes       = 0;
     isVolatile           = false;
     isTail               = false;
     hasConstrained       = false;
     constrainedTypeToken = 0;
     isReadonly           = false;
   }


#if 0
DispatcherCil::CilTypeAnalysis::TypeDescriptor
MyCilMetadataProvider::resolveType(uint32_t token) const
{
    DispatcherCil::CilTypeAnalysis::TypeDescriptor t(token);

    const MyTypeRecord *record = lookupTypeRecord(token);
    if (record == nullptr) {
        // Token known, metadata unresolved.
        return t;
    }

    t.metadataKnown = true;
    t.name = record->name;

    t.isValueType = record->isValueType;
    t.isReferenceType = !record->isValueType;
    t.isInterface = record->isInterface;
    t.isArray = record->isArray;
    t.valueBitWidth = record->valueBitWidth; // 0 if layout is still unknown

    return t;
}
#endif

  
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Functors that handle individual CIL instruction kinds
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Cil {

// This implementation intentionally mirrors DispatcherJvm.C: one small processor adapter performs all common dispatcher,
// operator, instruction, operand-list, PC-advance, and checking work, then delegates to instruction-specific semantics.
class P: public BaseSemantics::InsnProcessor
   {
     public:
          using D = DispatcherCil*;
          using Ops = RiscOperators*;
          using I = SgAsmCilInstruction*;
          using Args = const SgAsmExpressionPtrList&;

          virtual void p(D, Ops, I, Args) = 0;

       // DQ (6/26/2026): Added to support compilation.
       // static unsigned CilSemantics::cilRawOpcodeKey(SgAsmInstruction *insn);
       
          virtual void process(const Dispatcher::Ptr &dispatcher_, SgAsmInstruction *insn_) override;
#if 0
          virtual void process(const Dispatcher::Ptr &dispatcher_, SgAsmInstruction *insn_) override
             {
               DispatcherCil::Ptr dispatcher = DispatcherCil::promote(dispatcher_);
               RiscOperators::Ptr operators = dispatcher->operators();
               SgAsmCilInstruction *insn = isSgAsmCilInstruction(insn_);
               ASSERT_not_null(insn);
               ASSERT_require(insn == operators->currentInstruction());

               dispatcher->advanceInstructionPointer(insn);
               SgAsmExpressionPtrList &operands = insn->get_operandList()->get_operands();
#if 0
            // dispatcher->advanceInstructionPointer(insn);
            // SgAsmExpressionPtrList &operands = insn->get_operandList()->get_operands();
#endif
               const unsigned opcode = CilSemantics::cilRawOpcodeKey(insn);

               const bool isPrefix =
                               opcode == 0xfe12 ||   // unaligned.
                               opcode == 0xfe13 ||   // volatile.
                               opcode == 0xfe14 ||   // tail.
                               opcode == 0xfe16 ||   // constrained.
                               opcode == 0xfe1e;     // readonly.

               operators->comment("executing CIL instruction core");
               p(dispatcher.get(), operators.get(), insn, operands);

            // Prefixes apply to the next real instruction, not to themselves.
            // Therefore, do not clear after executing a prefix. Clear after the
            // immediately following non-prefix instruction has had a chance to consume it.
               if (!isPrefix)
                  {
                    dispatcher->clearPendingPrefix();
                  }
#if 0               
               operators->comment("executing CIL instruction core");
               p(dispatcher.get(), operators.get(), insn, operands);

            // operators->comment("executing CIL instruction core");
            // p(dispatcher.get(), operators.get(), insn, operands);

               if (!isPrefix)
                  {
                    dispatcher->clearPendingPrefix();
                  }
#endif
             }
#endif

          void assert_args(I insn, Args args, size_t nargs) {
              if (args.size() != nargs) {
                  std::string mesg = "CIL instruction must have " + StringUtility::plural(nargs, "operand");
                  throw BaseSemantics::Exception(mesg, insn);
              }
          }
   };

namespace CilSemantics
   {
     using D = DispatcherCil*;
     using Ops = RiscOperators*;
     using I = SgAsmCilInstruction*;
     using Args = const SgAsmExpressionPtrList&;

  // Synthetic starting address for CIL localloc allocations.
  //
  // CIL localloc allocates memory from a method-local dynamic memory area,
  // similar in spirit to alloca. The CLI instruction produces a native address,
  // but the initial CIL dispatcher does not yet have a real stack-frame or
  // method-local allocation model.
  //
  // This constant is therefore only a modeling base address used by the optional
  // precise localloc approximation:
  //
  //   oldPtr = localAllocPointer
  //   newPtr = oldPtr + align(size)
  //   localAllocPointer = newPtr
  //   push oldPtr
  //
  // It is not a real CLI-mandated address. Once DispatcherCil has a proper
  // method-frame/local-memory model, this synthetic base should be replaced by
  // that model.
     static const uint64_t CIL_LOCAL_ALLOC_BASE = 0x70000000ull;

  // Alignment used for synthetic localloc allocations.
  //
  // Real runtimes align localloc results according to platform/runtime rules.
  // Since this dispatcher does not yet model those rules, use a conservative
  // power-of-two alignment so that successive symbolic allocations are separated
  // in a plausible native-address space.
  //
  // The value must remain a power of two because alignUpPowerOfTwo() implements:
  //
  //   aligned = (size + (alignment - 1)) & ~(alignment - 1)
  //
  // If the runtime/architecture alignment becomes available later, replace this
  // constant with that value.
     static const uint64_t CIL_LOCAL_ALLOC_ALIGNMENT = 8;

  // static constexpr size_t CIL_ARGUMENT_SLOT_BASE = 0;
  // static constexpr size_t CIL_LOCAL_SLOT_BASE    = 1000000;
     static constexpr size_t CIL_ARGUMENT_SLOT_BASE = 0x00000000;
     static constexpr size_t CIL_LOCAL_SLOT_BASE    = 0x00100000;
     static constexpr size_t CIL_RETURN_VALUE_SLOT  = 0x00200000;
     static constexpr size_t CIL_CURRENT_EXCEPTION_SLOT = 0x00200001;
     static constexpr size_t CIL_LOCAL_ALLOC_POINTER_SLOT = 0x00200002;

  // DQ (6/24/2026): This constant will need to be set by the CIL loader (later).
  // static size_t cilPointerBits_ = 32;

     static size_t
     cilArgumentSlot(size_t argIndex)
        {
          return CIL_ARGUMENT_SLOT_BASE + argIndex;
        }

     static size_t
     cilLocalSlot(size_t localIndex)
        {
          return CIL_LOCAL_SLOT_BASE + localIndex;
        }

     static size_t
     cilReturnValueSlot()
        {
          return CIL_RETURN_VALUE_SLOT;
        }

     static size_t
     cilCurrentExceptionSlot()
        {
          return CIL_CURRENT_EXCEPTION_SLOT;
        }

     static size_t
     cilLocalAllocPointerSlot()
        {
          return CIL_LOCAL_ALLOC_POINTER_SLOT;
        }

     enum Branch_Compare_Op_Kind_Type
        {
          branch_eq,
          branch_ne_un,

          branch_ge,
          branch_gt,
          branch_le,
          branch_lt,

          branch_ge_un,
          branch_gt_un,
          branch_le_un,
          branch_lt_un
        };

     enum Indirect_Load_Kind_Type
        {
          ldind_i1_kind,
          ldind_u1_kind,
          ldind_i2_kind,
          ldind_u2_kind,
          ldind_i4_kind,
          ldind_u4_kind,
          ldind_i8_kind,
          ldind_i_kind,
          ldind_r4_kind,
          ldind_r8_kind,
          ldind_ref_kind
        };

     enum Indirect_Store_Kind_Type
        {
          stind_ref_kind,
          stind_i1_kind,
          stind_i2_kind,
          stind_i4_kind,
          stind_i8_kind,
          stind_r4_kind,
          stind_r8_kind,
          stind_i_kind,
        };

     enum Convert_Kind_Type
        {
          conv_i1_kind,
          conv_i2_kind,
          conv_i4_kind,
          conv_i8_kind,
          conv_r4_kind,
          conv_r8_kind,
          conv_u1_kind,
          conv_u2_kind,
          conv_u4_kind,
          conv_u8_kind,
          conv_i_kind,
          conv_u_kind,
          conv_r_un_kind,
    
          conv_ovf_i1_un_kind,
          conv_ovf_i2_un_kind,
          conv_ovf_i4_un_kind,
          conv_ovf_i8_un_kind,
          conv_ovf_u1_un_kind,
          conv_ovf_u2_un_kind,
          conv_ovf_u4_un_kind,
          conv_ovf_u8_un_kind,
          conv_ovf_i_un_kind,
          conv_ovf_u_un_kind,

          conv_ovf_i1_kind,
          conv_ovf_u1_kind,
          conv_ovf_i2_kind,
          conv_ovf_u2_kind,
          conv_ovf_i4_kind,
          conv_ovf_u4_kind,
          conv_ovf_i8_kind,
          conv_ovf_u8_kind,
          conv_ovf_i_kind,
          conv_ovf_u_kind
        };

     enum Element_Load_Kind_Type
        {
          ldelem_i1_kind,
          ldelem_u1_kind,
          ldelem_i2_kind,
          ldelem_u2_kind,
          ldelem_i4_kind,
          ldelem_u4_kind,
          ldelem_i8_kind,
          ldelem_i_kind,
          ldelem_r4_kind,
          ldelem_r8_kind,
          ldelem_ref_kind
        };

     enum Element_Store_Kind_Type
        {
          stelem_i_kind,
          stelem_i1_kind,
          stelem_i2_kind,
          stelem_i4_kind,
          stelem_i8_kind,
          stelem_r4_kind,
          stelem_r8_kind,
          stelem_ref_kind
        };

     enum Compare_Op_Kind_Type
        {
          compare_eq,
          compare_gt,
          compare_gt_un,
          compare_lt,
          compare_lt_un
        };

     enum Float_Compare_Op_Kind_Type
        {
          float_compare_eq,
          float_compare_ne_un,
          float_compare_ge,
          float_compare_gt,
          float_compare_le,
          float_compare_lt,
          float_compare_ge_un,
          float_compare_gt_un,
          float_compare_le_un,
          float_compare_lt_un
        };

     static bool isFloatLike(ValueKind kind);

     static Sawyer::Optional<bool> concreteFloatComparison(const SValue::Ptr &lhs,
                                                           const SValue::Ptr &rhs,
                                                           Float_Compare_Op_Kind_Type kind);

     static SValue::Ptr concreteBoolean(Ops ops, bool value);
     
     struct ElementLoadDescriptor
        {
          size_t elementBits   = 32;      // bits read from array storage
          size_t stackBits     = 32;      // bits pushed onto evaluation stack
          bool signExtend      = false;
          bool zeroExtend      = false;
          ValueKind resultKind = ValueKind::Integer32;
        };

     struct CilConversionTarget
        {
          size_t targetBits    = 32;
          size_t stackBits     = 32;
          bool targetIsSigned  = true;
          ValueKind resultKind = ValueKind::Integer32;
        };

     struct ElementStoreDescriptor
        {
          size_t elementBits     = 32;
          ValueKind expectedKind = ValueKind::Integer32;
        };
     
     uint64_t asUnsigned(const SgAsmExpression *expr, uint64_t dflt = 0)
        {
          if (const SgAsmIntegerValueExpression *ival = isSgAsmIntegerValueExpression(expr))
               return ival->get_value();
          return dflt;
        }

     uint32_t asIndex(const SgAsmExpression *expr)
        {
          return static_cast<uint32_t>(asUnsigned(expr));
        }

     void doNothing(Ops ops)
        {
          ASSERT_not_null(ops);
        }

     void pushUnknown(Ops ops, size_t nbits)
        {
          ASSERT_not_null(ops);
          ops->pushOperand(ops->undefined_(nbits));
        }

     static size_t
     cilPointerBits(D dispatcher)
        {
          ASSERT_not_null(dispatcher);
          return dispatcher->cilPointerBits();
        }

     static void
     writeUnknownInstructionPointer(D dispatcher, Ops ops)
        {
          ASSERT_not_null(dispatcher);
          ASSERT_not_null(ops);
          const RegisterDescriptor ipReg = dispatcher->instructionPointerRegister();
          ops->writeRegister(ipReg, ops->undefined_(ipReg.nBits()));
        }

     void pushConstant(Ops ops, Args args, size_t nbits)
        {
          ASSERT_not_null(ops);
          ASSERT_require(!args.empty());
          ops->pushOperand(ops->number_(nbits, asUnsigned(args[0])));
        }

     void discard(Ops ops, size_t nvalues)
        {
       // pop
       // Description:
       //   Removes the top value from the evaluation stack.
       //
       // Stack:
       //   ..., value -> ...
       //
       // Semantics:
       //   Pops and discards one evaluation-stack value. No memory, local, argument,
       //   or register state is modified.
       //
       // Run-time Exceptions:
       //   None for a valid method body.

          ASSERT_not_null(ops);
          while (nvalues-- > 0)
             {
               ops->popOperand();
             }
        }
#if 0
   // I think this is a duplicate of the previous function...
     static ValueKind
     cilNumericResultKind(ValueKind lhsKind, ValueKind rhsKind)
        {
       // This function needs to be called for all of these operators.
       // case add:
       // case add_with_overflow_check:
       // case add_unsigned_with_overflow_check:
       // case subtract:
       // case subtract_with_overflow_check:
       // case subtract_unsigned_with_overflow_check:
       // case multiply:
       // case multiply_with_overflow_check:
       // case multiply_unsigned_with_overflow_check:
       // case divide:
       // case divide_unsigned:
       // case remainder:
       // case remainder_unsigned:

          if (lhsKind == ValueKind::NativeInt || rhsKind == ValueKind::NativeInt)
               return ValueKind::NativeInt;

          if (lhsKind == ValueKind::Integer64 || rhsKind == ValueKind::Integer64)
               return ValueKind::Integer64;

          return ValueKind::Integer32;
        }
#endif


static std::string
convertKindToString(Convert_Kind_Type kind)
{
    switch (kind) {
        case conv_i1_kind:          return "conv.i1";
        case conv_i2_kind:          return "conv.i2";
        case conv_i4_kind:          return "conv.i4";
        case conv_i8_kind:          return "conv.i8";
        case conv_r4_kind:          return "conv.r4";
        case conv_r8_kind:          return "conv.r8";
        case conv_u1_kind:          return "conv.u1";
        case conv_u2_kind:          return "conv.u2";
        case conv_u4_kind:          return "conv.u4";
        case conv_u8_kind:          return "conv.u8";
        case conv_i_kind:           return "conv.i";
        case conv_u_kind:           return "conv.u";
        case conv_r_un_kind:        return "conv.r.un";

        case conv_ovf_i1_un_kind:   return "conv.ovf.i1.un";
        case conv_ovf_i2_un_kind:   return "conv.ovf.i2.un";
        case conv_ovf_i4_un_kind:   return "conv.ovf.i4.un";
        case conv_ovf_i8_un_kind:   return "conv.ovf.i8.un";
        case conv_ovf_u1_un_kind:   return "conv.ovf.u1.un";
        case conv_ovf_u2_un_kind:   return "conv.ovf.u2.un";
        case conv_ovf_u4_un_kind:   return "conv.ovf.u4.un";
        case conv_ovf_u8_un_kind:   return "conv.ovf.u8.un";
        case conv_ovf_i_un_kind:    return "conv.ovf.i.un";
        case conv_ovf_u_un_kind:    return "conv.ovf.u.un";

        case conv_ovf_i1_kind:      return "conv.ovf.i1";
        case conv_ovf_u1_kind:      return "conv.ovf.u1";
        case conv_ovf_i2_kind:      return "conv.ovf.i2";
        case conv_ovf_u2_kind:      return "conv.ovf.u2";
        case conv_ovf_i4_kind:      return "conv.ovf.i4";
        case conv_ovf_u4_kind:      return "conv.ovf.u4";
        case conv_ovf_i8_kind:      return "conv.ovf.i8";
        case conv_ovf_u8_kind:      return "conv.ovf.u8";
        case conv_ovf_i_kind:       return "conv.ovf.i";
        case conv_ovf_u_kind:       return "conv.ovf.u";

        default:
            return "conv.<unknown>";
    }
}

static void
overflowExceptionPath(D dispatcher, Ops ops, const std::string &comment)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);

    ops->comment(comment);

    SValue::Ptr exceptionObject = ops->undefined_(CilSemantics::cilPointerBits(dispatcher));
    exceptionObject->kind(ValueKind::ObjectReference);
    exceptionObject->typeDescriptor("cil:exception:System.OverflowException");
    dispatcher->typeAnalysis().setNullness(
        exceptionObject, DispatcherCil::CilTypeAnalysis::DefinitelyNonNull);
    ops->writeLocal(CilSemantics::cilCurrentExceptionSlot(), exceptionObject);

    CilSemantics::writeUnknownInstructionPointer(dispatcher, ops);
}

static void
conversionOverflowExceptionEffect(D dispatcher,
                                  Ops ops,
                                  I /*insn*/,
                                  Convert_Kind_Type kind)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);

    CilSemantics::overflowExceptionPath(
        dispatcher, ops,
        "CIL " + convertKindToString(kind) +
        ": concrete conversion overflow; OverflowException path not fully modeled");
}


   // DQ (6/7/2026): Added to support different kinds of unary operators.
     enum Unary_Op_Kind_Type
        {
          negate,
          not_op,
          castclass,
          isinst,
          unbox,
          box,
          unbox_any,
          newarr,
          ldlen,
          refanyval,
          mkrefany,
          localloc,
          refanytype,
        };

     std::string
     unary_op_kind_toString(Unary_Op_Kind_Type op_kind)
        {
          switch (op_kind)
             {
               case negate:     return "negate";
               case not_op:     return "not_op";
               case castclass:  return "castclass";
               case isinst:     return "isinst";
               case unbox:      return "unbox";
               case box:        return "box";
               case unbox_any:  return "unbox_any";
               case newarr:     return "newarr";
               case ldlen:      return "ldlen";
               case refanyval:  return "refanyval";
               case mkrefany:   return "mkrefany";
               case localloc:   return "localloc";
               case refanytype: return "refanytype";
               default:
                  {
                    fprintf(stderr,"In unary_op_kind_toString(): default reached: op_kind = %d \n",op_kind);
                    fflush(stderr);
                    ROSE_ASSERT(false);
                    break;
                  }
             }
        }



     static DispatcherCil::CilTypeAnalysis::TypeDescriptor
     resolveType(D dispatcher, uint32_t token)
        {
          DispatcherCil::CilTypeAnalysis::TypeDescriptor retval(token);

          if (dispatcher && dispatcher->cilMetadata())
               retval = dispatcher->cilMetadata()->resolveType(token);

          if (dispatcher)
               dispatcher->typeAnalysis().rememberTypeDescriptor(retval);

          return retval;
        }

  // static SValue::Ptr nullReference(D dispatcher, Ops ops, size_t ptrBits = 32)
     static SValue::Ptr
     nullReference(D dispatcher, Ops ops, size_t ptrBits)
        {
          ASSERT_not_null(ops);

       // const size_t ptrBits = CilSemantics::cilPointerBits(dispatcher);
          
          SValue::Ptr retval = ops->number_(ptrBits, 0);
          retval->kind(ValueKind::ObjectReference);

          if (dispatcher)
               dispatcher->typeAnalysis().setNullness(retval, DispatcherCil::CilTypeAnalysis::DefinitelyNull);

          return retval;
        }

     static bool
     isMaybeNull(D dispatcher, const SValue::Ptr &value)
        {
          ASSERT_not_null(dispatcher);
          ASSERT_not_null(value);

          return dispatcher->typeAnalysis().nullness(value) == DispatcherCil::CilTypeAnalysis::MaybeNull;
        }

     static void
     noteMaybeNullReferenceException(D dispatcher, Ops ops, const SValue::Ptr &value, const std::string &context)
        {
          ASSERT_not_null(dispatcher);
          ASSERT_not_null(ops);
          ASSERT_not_null(value);

          if (CilSemantics::isMaybeNull(dispatcher, value))
              {
                  // The normal path is still modeled by the caller, but a maybe-null
                  // reference also has a possible NullReferenceException edge. Do
                  // not write an active exception into the merged state that will
                  // also contain the caller's normal stack/memory effect.
                  ops->comment("CIL " + context + ": maybe-null reference; NullReferenceException edge not split");
              }
        }

  // DQ (6/11/2026): Now I want to add support to use more of the functionality in the SValue class (described in SValue.h).
  // New goals are:
  // Use SValue directly for:
  //    ValueKind
  //    exact known type descriptor/token
  //    array length
  //    concrete zero/nonzero queries
  //    comments/debug labels

  // Keep CilTypeAnalysis only for (or improve the SValue class to include these):
  //    possible runtime type sets
  //    maybe-null / definitely-non-null for symbolic references
  //    assignability rules
  //    method/call-site metadata descriptors
  
     static std::string
     cilTypeTokenDescriptor(uint32_t token)
        {
          return "cil:type:0x" + StringUtility::addrToString(token);
        }


static std::string
cilTypedReferenceDescriptor(uint32_t typeToken)
{
    return "cil:typedref:" + CilSemantics::cilTypeTokenDescriptor(typeToken);
}

static bool
cilTypeTokenFromDescriptor(const std::string &descriptor, uint32_t &typeToken)
{
    static const char *marker = "0x";

    std::string::size_type pos = descriptor.rfind(marker);
    if (pos == std::string::npos)
        return false;

    const char *begin = descriptor.c_str() + pos;
    char *end = nullptr;
    unsigned long parsed = std::strtoul(begin, &end, 16);

    if (end == begin || *end != '\0' || parsed > 0xfffffffful)
        return false;

    typeToken = static_cast<uint32_t>(parsed);
    return typeToken != 0;
}


static SValue::Ptr
normalizeToPointerBits(D dispatcher, Ops ops, const SValue::Ptr &value)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);
    ASSERT_not_null(value);

    const size_t ptrBits = CilSemantics::cilPointerBits(dispatcher);

    SValue::Ptr normalized = value;

    if (value->nBits() < ptrBits)
        normalized = ops->unsignedExtend(value, ptrBits);
    else if (value->nBits() > ptrBits)
        normalized = ops->extract(value, 0, ptrBits);

    if (normalized != value) {
        normalized->kind(value->kind());
        if (value->hasTypeDescriptor())
            normalized->typeDescriptor(value->typeDescriptor());
        dispatcher->typeAnalysis().copyFacts(normalized, value);
    }

    return normalized;
}


static std::string
cilRuntimeHandleDescriptor(uint32_t token)
{
    const uint32_t table = (token >> 24) & 0xff;
    const uint32_t rid   = token & 0x00ffffff;

    std::ostringstream ss;

    switch (table) {
        case 0x01:
            ss << "cil:RuntimeTypeHandle:TypeRef:";
            break;

        case 0x02:
            ss << "cil:RuntimeTypeHandle:TypeDef:";
            break;

        case 0x04:
            ss << "cil:RuntimeFieldHandle:Field:";
            break;

        case 0x06:
            ss << "cil:RuntimeMethodHandle:MethodDef:";
            break;

        case 0x0a:
            ss << "cil:RuntimeMethodHandle:MemberRef:";
            break;

        case 0x1b:
            ss << "cil:RuntimeTypeHandle:TypeSpec:";
            break;

        default:
            ss << "cil:RuntimeHandle:table0x" << std::hex << table << ":";
            break;
    }

    ss << "rid0x" << std::hex << rid;
    return ss.str();
}

static bool
isLdtokenMetadataTable(uint32_t token)
{
    const uint32_t table = (token >> 24) & 0xff;

    switch (table) {
        case 0x01: // TypeRef
        case 0x02: // TypeDef
        case 0x04: // Field
        case 0x06: // MethodDef
        case 0x0a: // MemberRef
        case 0x1b: // TypeSpec
            return true;

        default:
            return false;
    }
}


// Internal packed typedref layout:
//
//   bits [0, ptrBits)            = managed address
//   bits [ptrBits, 2 * ptrBits)  = type token / RuntimeTypeHandle-like value
//
// ROSE concatLoHi(lowBits, highBits) puts lowBits in the low-order bits.
static SValue::Ptr
makePackedTypedReferenceWithHandle(D dispatcher,
                                   Ops ops,
                                   const SValue::Ptr &rawAddress,
                                   const SValue::Ptr &rawTypeHandle,
                                   const std::string &descriptor)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);
    ASSERT_not_null(rawAddress);
    ASSERT_not_null(rawTypeHandle);

    SValue::Ptr addr = CilSemantics::normalizeToPointerBits(dispatcher, ops, rawAddress);
    addr->kind(ValueKind::ManagedPointer);

    SValue::Ptr typeHandle = CilSemantics::normalizeToPointerBits(dispatcher, ops, rawTypeHandle);
    typeHandle->kind(ValueKind::NativeInt);

    SValue::Ptr typedRef = ops->concatLoHi(addr, typeHandle);

    // SValue.h currently has no ValueKind::TypedReference.  Use Unknown plus
    // a type descriptor that identifies this as the packed CIL typedref model.
    typedRef->kind(ValueKind::Unknown);
    typedRef->typeDescriptor(descriptor);

    return typedRef;
}

static SValue::Ptr
makePackedTypedReference(D dispatcher,
                         Ops ops,
                         const SValue::Ptr &rawAddress,
                         uint32_t typeToken)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);
    ASSERT_not_null(rawAddress);

    const size_t ptrBits = CilSemantics::cilPointerBits(dispatcher);
    SValue::Ptr typeHandle = ops->number_(ptrBits, typeToken);
    typeHandle->kind(ValueKind::NativeInt);
    typeHandle->typeDescriptor(CilSemantics::cilRuntimeHandleDescriptor(typeToken));

    return CilSemantics::makePackedTypedReferenceWithHandle(
        dispatcher,
        ops,
        rawAddress,
        typeHandle,
        CilSemantics::cilTypedReferenceDescriptor(typeToken));
}

#if 0
static bool
isPackedTypedReference(D dispatcher, const SValue::Ptr &value)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(value);

    const size_t ptrBits = CilSemantics::cilPointerBits(dispatcher);

    return value->nBits() == 2 * ptrBits &&
           value->hasTypeDescriptor() &&
           value->typeDescriptor().find("cil:typedref:") == 0;
}
#endif

static SValue::Ptr
packedTypedReferenceAddress(D dispatcher,
                            Ops ops,
                            const SValue::Ptr &typedRef)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);
    ASSERT_not_null(typedRef);

    const size_t ptrBits = CilSemantics::cilPointerBits(dispatcher);

    SValue::Ptr result;

    if (typedRef->nBits() >= 2 * ptrBits) {
        result = ops->extract(typedRef, 0, ptrBits);
    } else {
        // Bad or unknown typedref representation. Keep the stack well-formed.
        result = ops->undefined_(ptrBits);
    }

    result->kind(ValueKind::ManagedPointer);
    result->typeDescriptor("cil:typedref-address");

    return result;
}


static SValue::Ptr
packedTypedReferenceTypeHandle(D dispatcher,
                               Ops ops,
                               const SValue::Ptr &typedRef)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);
    ASSERT_not_null(typedRef);

    const size_t ptrBits = CilSemantics::cilPointerBits(dispatcher);

    SValue::Ptr result;

    if (typedRef->nBits() >= 2 * ptrBits) {
        result = ops->extract(typedRef, ptrBits, 2 * ptrBits);
    } else {
        result = ops->undefined_(ptrBits);
    }

    result->kind(ValueKind::NativeInt);
    result->typeDescriptor("cil:RuntimeTypeHandle/from-typedref");

    return result;
}


static Sawyer::Optional<uint64_t>
concretePackedTypedReferenceTypeToken(D dispatcher,
                                      Ops ops,
                                      const SValue::Ptr &typedRef)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);
    ASSERT_not_null(typedRef);

    SValue::Ptr typeHandle =
        CilSemantics::packedTypedReferenceTypeHandle(dispatcher, ops, typedRef);

    return typeHandle->toUnsigned();
}

static size_t
typeStackBits(D dispatcher, const DispatcherCil::CilTypeAnalysis::TypeDescriptor &type)
{
    ASSERT_not_null(dispatcher);

    const size_t ptrBits = CilSemantics::cilPointerBits(dispatcher);

    if (type.isKnownReferenceType())
        return ptrBits;

    if (type.hasKnownValueWidth())
        return type.valueBitWidth < 32 ? 32 : type.valueBitWidth;

    return ptrBits;
}

static ValueKind
typeStackValueKind(const DispatcherCil::CilTypeAnalysis::TypeDescriptor &type, size_t bits)
{
    if (type.isKnownReferenceType())
        return type.isArray ? ValueKind::ArrayReference : ValueKind::ObjectReference;

    if (!type.metadataKnown)
        return ValueKind::Unknown;

    if (bits == 64)
        return ValueKind::Integer64;

    if (bits <= 32)
        return ValueKind::Integer32;

    return ValueKind::Unknown;
}

static std::string
boxedPayloadDescriptor(const DispatcherCil::CilTypeAnalysis::TypeDescriptor &type)
{
    if (type.metadataToken != 0)
        return "cil:boxed-payload-address:" + CilSemantics::cilTypeTokenDescriptor(type.metadataToken);

    return "cil:boxed-payload-address:unknown";
}

static SValue::Ptr
resizeBoxedPayloadValue(Ops ops, const SValue::Ptr &value, size_t payloadBits)
{
    ASSERT_not_null(ops);
    ASSERT_not_null(value);

    if (value->nBits() < payloadBits)
        return ops->unsignedExtend(value, payloadBits);

    if (value->nBits() > payloadBits)
        return ops->extract(value, 0, payloadBits);

    return value;
}

static SValue::Ptr
boxedPayloadAddress(D dispatcher,
                    Ops ops,
                    const SValue::Ptr &boxedObject,
                    const DispatcherCil::CilTypeAnalysis::TypeDescriptor &type)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);
    ASSERT_not_null(boxedObject);

    const size_t ptrBits = CilSemantics::cilPointerBits(dispatcher);
    SValue::Ptr base = CilSemantics::normalizeToPointerBits(dispatcher, ops, boxedObject);

    // Synthetic boxed-object header.  The exact CLI object layout is metadata
    // and runtime dependent; use a stable nonzero offset so the boxed payload
    // does not alias the object reference/base address.
    static const uint64_t CIL_BOXED_PAYLOAD_OFFSET = 0x10;

    SValue::Ptr addr = ops->add(base, ops->number_(ptrBits, CIL_BOXED_PAYLOAD_OFFSET));
    addr->kind(ValueKind::ManagedPointer);
    addr->typeDescriptor(CilSemantics::boxedPayloadDescriptor(type));

    return addr;
}

static void
markTargetTypeFacts(D dispatcher,
                    const SValue::Ptr &value,
                    const DispatcherCil::CilTypeAnalysis::TypeDescriptor &type,
                    ValueKind kind)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(value);

    value->kind(kind);

    if (type.metadataToken != 0)
        value->typeDescriptor(CilSemantics::cilTypeTokenDescriptor(type.metadataToken));

    if (type.metadataToken != 0 &&
        (kind == ValueKind::ObjectReference || kind == ValueKind::ArrayReference)) {
        DispatcherCil::CilTypeAnalysis::TypeSet targetTypes;
        targetTypes.insert(type.metadataToken);
        dispatcher->typeAnalysis().rememberTypeDescriptor(type);
        dispatcher->typeAnalysis().setPossibleTypes(value, targetTypes);
    }
}

static void
invalidCastExceptionPath(D dispatcher, Ops ops)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);

    SValue::Ptr exceptionObject = ops->undefined_(CilSemantics::cilPointerBits(dispatcher));
    exceptionObject->kind(ValueKind::ObjectReference);
    exceptionObject->typeDescriptor("cil:exception:System.InvalidCastException");
    dispatcher->typeAnalysis().setNullness(
        exceptionObject, DispatcherCil::CilTypeAnalysis::DefinitelyNonNull);
    ops->writeLocal(CilSemantics::cilCurrentExceptionSlot(), exceptionObject);
    CilSemantics::writeUnknownInstructionPointer(dispatcher, ops);
}

static bool
definitelyInvalidBoxedValueTarget(D dispatcher,
                                  const SValue::Ptr &boxedObject,
                                  const DispatcherCil::CilTypeAnalysis::TypeDescriptor &targetType)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(boxedObject);

    DispatcherCil::CilTypeAnalysis &ta = dispatcher->typeAnalysis();
    if (!ta.isDefinitelyNonNull(boxedObject))
        return false;

    DispatcherCil::CilTypeAnalysis::TypeSet possibleTypes =
        ta.possibleDynamicTypes(boxedObject);
    return ta.noneAssignableTo(possibleTypes, &targetType);
}

#if 0
static void
outOfMemoryExceptionPath(D dispatcher, Ops ops, const std::string &comment)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);

    ops->comment(comment);

    SValue::Ptr exceptionObject = ops->undefined_(CilSemantics::cilPointerBits(dispatcher));
    exceptionObject->kind(ValueKind::ObjectReference);
    exceptionObject->typeDescriptor("cil:exception:System.OutOfMemoryException");
    dispatcher->typeAnalysis().setNullness(
        exceptionObject, DispatcherCil::CilTypeAnalysis::DefinitelyNonNull);
    ops->writeLocal(CilSemantics::cilCurrentExceptionSlot(), exceptionObject);
    CilSemantics::writeUnknownInstructionPointer(dispatcher, ops);
}
#endif

#if 0
static void
stackOverflowExceptionPath(D dispatcher, Ops ops, const std::string &comment)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);

    ops->comment(comment);

    SValue::Ptr exceptionObject = ops->undefined_(CilSemantics::cilPointerBits(dispatcher));
    exceptionObject->kind(ValueKind::ObjectReference);
    exceptionObject->typeDescriptor("cil:exception:System.StackOverflowException");
    dispatcher->typeAnalysis().setNullness(
        exceptionObject, DispatcherCil::CilTypeAnalysis::DefinitelyNonNull);
    ops->writeLocal(CilSemantics::cilCurrentExceptionSlot(), exceptionObject);
    CilSemantics::writeUnknownInstructionPointer(dispatcher, ops);
}
#endif

static void
arithmeticExceptionPath(D dispatcher, Ops ops, const std::string &comment)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);

    ops->comment(comment);

    SValue::Ptr exceptionObject = ops->undefined_(CilSemantics::cilPointerBits(dispatcher));
    exceptionObject->kind(ValueKind::ObjectReference);
    exceptionObject->typeDescriptor("cil:exception:System.ArithmeticException");
    dispatcher->typeAnalysis().setNullness(
        exceptionObject, DispatcherCil::CilTypeAnalysis::DefinitelyNonNull);
    ops->writeLocal(CilSemantics::cilCurrentExceptionSlot(), exceptionObject);
    CilSemantics::writeUnknownInstructionPointer(dispatcher, ops);
}

static void
invalidProgramExceptionPath(D dispatcher, Ops ops, const std::string &comment)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);

    ops->comment(comment);

    SValue::Ptr exceptionObject = ops->undefined_(CilSemantics::cilPointerBits(dispatcher));
    exceptionObject->kind(ValueKind::ObjectReference);
    exceptionObject->typeDescriptor("cil:exception:System.InvalidProgramException");
    dispatcher->typeAnalysis().setNullness(
        exceptionObject, DispatcherCil::CilTypeAnalysis::DefinitelyNonNull);
    ops->writeLocal(CilSemantics::cilCurrentExceptionSlot(), exceptionObject);
    CilSemantics::writeUnknownInstructionPointer(dispatcher, ops);
}




  // void unaryResult(D dispatcher, Ops ops, Unary_Op_Kind_Type op_kind, const DispatcherCil::CilTypeAnalysis::TypeDescriptor *targetType = nullptr, size_t nbits = 32)
     void
     unaryResult(D dispatcher, Ops ops, Unary_Op_Kind_Type op_kind, const DispatcherCil::CilTypeAnalysis::TypeDescriptor *targetType = nullptr)
        {
          ASSERT_not_null(ops);

#if CIL_DEBUG
          fprintf(stderr, "TOP of unaryResult \n");
          fflush(stderr);
#endif

       // CIL unary opcodes represented here all consume one stack value.
          SValue::Ptr v = ops->popOperand();
          ASSERT_not_null(v);

       // const size_t nbits = CilSemantics::cilPointerBits(dispatcher);

       // const size_t valueBits = v->nBits();
       // const size_t ptrBits = nbits;       // TODO: replace with architecture/CIL native pointer width.
          const size_t ptrBits = CilSemantics::cilPointerBits(dispatcher);
 
          SValue::Ptr result;
          bool preserveInputKind = false;

#if CIL_DEBUG
          fprintf(stderr, "TOP of unaryResult: op_kind = %s input = %s \n",unary_op_kind_toString(op_kind).c_str(),v->toString().c_str());
          fflush(stderr);
#endif

          switch (op_kind)
             {
               case negate:
                  {
                 // CIL instruction: neg
                 //
                 // Stack transition:
                 //   ..., value -> ..., -value
                 //
                 // Purpose:
                 //   Computes the arithmetic negation of the value on top of the CIL
                 //   evaluation stack. This is numeric negation, not bitwise inversion.
                 //
                 // ROSE semantics:
                 //   Use ops->negate(v), which models arithmetic two's-complement negation
                 //   in the selected semantic domain.
                 //
                 // Notes:
                 //   The result has the same bit width and the same CIL stack category as
                 //   the input value. This instruction may have language-level overflow
                 //   implications for some source languages, but the plain CIL neg opcode
                 //   itself is modeled here as the corresponding low-level arithmetic
                 //   operation.
                 // CIL neg: arithmetic negation.
                    result = ops->negate(v);
                    preserveInputKind = true;
                    break;
                  }

               case not_op:
                  {
                 // CIL instruction: not
                 //
                 // Stack transition:
                 //   ..., value -> ..., ~value
                 //
                 // Purpose:
                 //   Computes the bitwise complement of the value on top of the CIL
                 //   evaluation stack. Each bit of the input is inverted.
                 //
                 // ROSE semantics:
                 //   Use ops->invert(v). Do not use ops->negate(v): negate is arithmetic
                 //   negation, while CIL not is bitwise inversion.
                 //
                 // Notes:
                 //   The result has the same bit width and CIL stack category as the input.
                 // CIL not: bitwise complement, not arithmetic negation.
                    result = ops->invert(v);
                    preserveInputKind = true;
                    break;
                  }
#if 0
               case castclass:
                  {
                 // CIL instruction: castclass <typeTok>
                 //
                 // Stack transition:
                 //   ..., objref -> ..., objref
                 //
                 // Purpose:
                 //   Checks whether the object reference on top of the stack is assignment-
                 //   compatible with the metadata target type. If the input is null, the
                 //   result is null. If the input is non-null and compatible, the result is
                 //   the original object reference. If the input is non-null and not
                 //   compatible, the real CLI semantics throw System.InvalidCastException.
                 //
                 // Required information:
                 //   Faithful semantics require the metadata token from the instruction,
                 //   possible runtime type information for the object, nullness information,
                 //   and an assignment-compatibility query over the CLI type hierarchy.
                 //
                 // Current conservative model:
                 //   If the type-analysis side table can prove nullness or compatibility,
                 //   preserve the original reference. Otherwise, use an unknown objref of
                 //   the same width to keep the evaluation stack well-formed. Exception
                 //   control-flow is not yet modeled here.
                 //
                 // Notes:
                 //   This is not a numeric unary operator. It is a metadata/type-system
                 //   operation that happens to consume one stack value.
                    ASSERT_not_null(dispatcher);
                    ASSERT_not_null(targetType);

                    DispatcherCil::CilTypeAnalysis &ta = dispatcher->typeAnalysis();

                    DispatcherCil::CilTypeAnalysis::TypeSet possibleTypes = ta.possibleDynamicTypes(v);

                 // const uint32_t targetToken = targetType->metadataToken;

                    if (ta.isDefinitelyNull(v))
                       {
                         result = v;
                         ta.copyFacts(result, v);
                       }
                      else
                       {
                         if (ta.allAssignableTo(possibleTypes, targetType))
                            {
                              result = v;
                              ta.copyFacts(result, v);
                            }
                           else
                            {
                           // Unknown, mixed, or definitely invalid cast.
                           // Precise CIL semantics:
                           //   invalid non-null cast -> InvalidCastException.
                           // Current approximation:
                           //   keep stack well-formed with unknown target-compatible objref.

                           // DQ (6/11/2026): Better implementation using SValue semantics.
                              result = ops->undefined_(v->nBits());
                              result->kind(ValueKind::ObjectReference);
                              result->typeDescriptor(cilTypeTokenDescriptor(targetType->metadataToken));
                              dispatcher->typeAnalysis().setNullness(result, DispatcherCil::CilTypeAnalysis::MaybeNull);
                            }
                       }

                    preserveInputKind = true;
                    break;
                  }
#else
               case castclass:
                  {
                    ASSERT_not_null(dispatcher);
                    ASSERT_not_null(targetType);

                    DispatcherCil::CilTypeAnalysis &ta = dispatcher->typeAnalysis();
                    DispatcherCil::CilTypeAnalysis::TypeSet possibleTypes =
                        ta.possibleDynamicTypes(v);

                    if (ta.isDefinitelyNull(v))
                       {
                      // castclass null returns null. This result is exactly
                      // the original input value, so preserving the input
                      // ValueKind is safe.
                         result = v;
                         ta.copyFacts(result, v);
                         preserveInputKind = true;
                       }
                      else if (ta.allAssignableTo(possibleTypes, targetType))
                       {
                      // Successful castclass returns the original object
                      // reference. Again, preserving the input kind is safe.
                         result = v;
                         ta.copyFacts(result, v);
                         preserveInputKind = true;
                       }
                      else if (ta.isDefinitelyNonNull(v) &&
                               ta.noneAssignableTo(possibleTypes, targetType))
                       {
                      // Definitely invalid non-null castclass throws
                      // InvalidCastException and has no normal result.
                         CilSemantics::invalidCastExceptionPath(dispatcher, ops);
                         return;
                       }
                      else
                       {
                      // Unknown, mixed, or invalid cast.
                      //
                      // Precise CIL semantics:
                      //   invalid non-null cast -> InvalidCastException.
                      //
                      // Current approximation:
                      //   keep stack well-formed with unknown target-compatible objref.
                      //
                      // Important: do NOT preserve the input kind here. This is a
                      // newly-created target-compatible reference value.
                         result = ops->undefined_(v->nBits());
                         CilSemantics::markTargetTypeFacts(dispatcher, result, *targetType, ValueKind::ObjectReference);
                         ta.setNullness(result,
                             ta.isDefinitelyNonNull(v) ?
                                 DispatcherCil::CilTypeAnalysis::DefinitelyNonNull :
                                 DispatcherCil::CilTypeAnalysis::MaybeNull);

                      // Optional but recommended: record that the normal result,
                      // when non-null, is compatible with the target type.
                         // markTargetTypeFacts records the target type for
                         // reference-like normal results.

                         preserveInputKind = false;
                       }

                    break;
                  }
#endif
#if 0
               case isinst:
                  {
                 // CIL instruction: isinst <typeTok>
                 //
                 // Stack transition:
                 //   ..., objref -> ..., objref-or-null
                 //
                 // Purpose:
                 //   Tests whether the object reference on top of the stack is assignment-
                 //   compatible with the metadata target type. Unlike castclass, failure
                 //   does not throw. Instead, failure produces null.
                 //
                 // Real CLI behavior:
                 //   If objref is null:
                 //       result is null.
                 //   Else if runtime type of objref is assignment-compatible with typeTok:
                 //       result is the original objref.
                 //   Else:
                 //       result is null.
                 //
                 // Current conservative model:
                 //   Without precise runtime type and nullness facts, produce an unknown
                 //   reference-width value. If type analysis is available, record that any
                 //   non-null result is compatible with the target type.
                 //
                 // Notes:
                 //   isinst is roughly a safe dynamic cast. It is not equivalent to
                 //   castclass because an incompatible non-null input returns null instead
                 //   of throwing InvalidCastException.
                    ASSERT_not_null(dispatcher);
                    ASSERT_not_null(targetType);

                    DispatcherCil::CilTypeAnalysis &ta = dispatcher->typeAnalysis();
                    DispatcherCil::CilTypeAnalysis::TypeSet possibleTypes = ta.possibleDynamicTypes(v);

                 // const uint32_t targetToken = targetType->metadataToken;

                    if (ta.isDefinitelyNull(v))
                       {
                         result = v;
                       }
                      else
                       {
                         if (ta.allAssignableTo(possibleTypes, targetType))
                            {
                              result = v;
                            }
                           else
                            {
                              if (ta.noneAssignableTo(possibleTypes, targetType))
                                 {
                                   result = nullReference(dispatcher, ops, v->nBits());
                                 }
                                else
                                 {
                                // DQ (6/11/2026): Better implementation using SValue semantics.
                                   result = ops->undefined_(v->nBits());
                                   result->kind(ValueKind::ObjectReference);
                                   result->typeDescriptor(cilTypeTokenDescriptor(targetType->metadataToken));
                                   dispatcher->typeAnalysis().setNullness(result, DispatcherCil::CilTypeAnalysis::MaybeNull);
                                 }
                            }
                       }

                    preserveInputKind = true;
                    break;
                  }
#else
               case isinst:
                  {
                    ASSERT_not_null(dispatcher);
                    ASSERT_not_null(targetType);

                    DispatcherCil::CilTypeAnalysis &ta = dispatcher->typeAnalysis();
                    DispatcherCil::CilTypeAnalysis::TypeSet possibleTypes =
                        ta.possibleDynamicTypes(v);

                    if (ta.isDefinitelyNull(v))
                       {
                         // isinst null returns null. This is exactly the input.
                         result = v;
                         ta.copyFacts(result, v);
                         preserveInputKind = true;
                       }
                      else if (ta.allAssignableTo(possibleTypes, targetType))
                       {
                         // Successful isinst returns the original object reference.
                         result = v;
                         ta.copyFacts(result, v);
                         preserveInputKind = true;
                       }
                      else if (ta.noneAssignableTo(possibleTypes, targetType))
                       {
                         // Definitely incompatible: isinst returns null.
                         //
                         // Do not preserve input kind. nullReference already creates
                         // an ObjectReference and records DefinitelyNull.
                         result = nullReference(dispatcher, ops, v->nBits());
                         preserveInputKind = false;
                       }
                      else
                       {
                         // Unknown compatibility: result may be null or a compatible
                         // object reference.
                         //
                         // Important: do NOT preserve the input kind here. For example,
                         // if v->kind() is Unknown or ArrayReference, overwriting this
                         // would lose the target-compatible ObjectReference result kind.
                         result = ops->undefined_(v->nBits());
                         CilSemantics::markTargetTypeFacts(dispatcher, result, *targetType, ValueKind::ObjectReference);
                         ta.setNullness(result,
                             DispatcherCil::CilTypeAnalysis::MaybeNull);

                      // Optional but recommended: if non-null, the result is
                      // compatible with targetType.
                         // markTargetTypeFacts records the target type for
                         // reference-like normal results.

                         preserveInputKind = false;
                       }

                    break;
                  }
#endif
          case unbox:
             {
            // CIL instruction: unbox <typeTok>
            //
            // Stack transition:
            //   ..., objref -> ..., managed-pointer
            //
            // Purpose:
            //   Converts a boxed value-type object reference into a managed pointer to the
            //   value stored inside the boxed object. This instruction does not copy the
            //   value onto the stack; it returns the address of the boxed payload.
            //
            // Real CLI behavior:
            //   If objref is null, the runtime throws NullReferenceException.
            //   If objref is not a boxed value compatible with the metadata target type,
            //   the runtime throws InvalidCastException.
            //
            // ROSE/RISC modeling:
            //   ROSE RISC semantics do not know the CLI boxed-object layout by default.
            //   Therefore, unless this dispatcher defines a heap/object layout, model the
            //   result as an unknown pointer-sized value. If a boxed-object layout is later
            //   added, this can be refined to objref + boxedPayloadOffset.
            //
            // Type-analysis note:
            //   The result is a managed pointer, not an object reference. Therefore do not
            //   record it with possibleDynamicTypes() unless that table is generalized to
            //   include pointer-target types. A separate pointer-target-type side fact would
            //   be more accurate.

            // Exact target type comes from metadata token, not available here.
            // result = ops->undefined_(ptrBits);
            // Do NOT blindly preserve input kind if objref and managed ptr differ.
            // break;
               ASSERT_not_null(dispatcher);
               ASSERT_not_null(targetType);

               if (dispatcher->typeAnalysis().isDefinitelyNull(v)) {
                  SValue::Ptr exceptionObject = ops->undefined_(CilSemantics::cilPointerBits(dispatcher));
                  exceptionObject->kind(ValueKind::ObjectReference);
                  exceptionObject->typeDescriptor("cil:exception:System.NullReferenceException");
                  dispatcher->typeAnalysis().setNullness(
                                                         exceptionObject, DispatcherCil::CilTypeAnalysis::DefinitelyNonNull);
                  ops->writeLocal(CilSemantics::cilCurrentExceptionSlot(), exceptionObject);
                  CilSemantics::writeUnknownInstructionPointer(dispatcher, ops);
                  ops->comment("CIL unbox of definitely-null object reference: NullReferenceException path");
                  return;
               }
               CilSemantics::noteMaybeNullReferenceException(dispatcher, ops, v, "unbox");

               if (CilSemantics::definitelyInvalidBoxedValueTarget(dispatcher, v, *targetType)) {
                   CilSemantics::invalidCastExceptionPath(dispatcher, ops);
                   return;
               }

               result = CilSemantics::boxedPayloadAddress(dispatcher, ops, v, *targetType);
               
            // dispatcher->typeAnalysis().setNullness(result,DispatcherCil::CilTypeAnalysis::DefinitelyNonNull);
               result->kind(ValueKind::ManagedPointer);
               dispatcher->typeAnalysis().setNullness(result, DispatcherCil::CilTypeAnalysis::DefinitelyNonNull);
 
               preserveInputKind = false;               
               break;
             }

          case box:
             {
            // CIL instruction: box <typeTok>
            //
            // Stack transition:
            //   ..., value -> ..., objref
            //
            // Purpose:
            //   Converts a value into an object reference. For ordinary value types,
            //   this allocates a boxed object on the managed heap and copies the value
            //   into the payload of that object.
            //
            // Real CLI behavior:
            //   For value types, the result is a newly allocated object reference.
            //   For reference types, the input reference can be returned unchanged.
            //   For Nullable<T>, null is produced when HasValue is false; otherwise
            //   the contained T value is boxed. Allocation failure can throw.
            //
            // ROSE/RISC modeling:
            //   ROSE RISC semantics do not know the CLI managed heap object layout by
            //   default. Unless this dispatcher models allocation and boxed-object
            //   memory layout, represent the result as an unknown pointer-sized object
            //   reference.
            //
            // Type-analysis note:
            //   Unlike unbox, the result is an object reference. Therefore it is
            //   reasonable to record the result's possible dynamic type as the metadata
            //   target type. For ordinary value-type boxing, the successful allocation
            //   result is definitely non-null.

               ASSERT_not_null(dispatcher);
               ASSERT_not_null(targetType);

               DispatcherCil::CilTypeAnalysis &ta = dispatcher->typeAnalysis();

               if (targetType->isKnownReferenceType()) {
                  // box on a reference-type value is an identity conversion on
                  // the reference; no new boxed allocation is modeled.
                  result = v;
                  ta.copyFacts(result, v);
                  preserveInputKind = true;
                  break;
               }

            // DQ (6/11/2026): Better implementation using SValue class support.
            // Boxing may throw OutOfMemoryException, but this dispatcher does
            // not split normal and exceptional allocation edges.  Do not write
            // an active exception into the merged state that will also contain
            // the normally boxed object.
               ops->comment("CIL box: value-type boxing allocation failure edge not split");

               result = ops->undefined_(ptrBits);
               CilSemantics::markTargetTypeFacts(dispatcher, result, *targetType, ValueKind::ObjectReference);

               ta.setNullness(result, DispatcherCil::CilTypeAnalysis::DefinitelyNonNull);

               {
                 const size_t payloadBits = CilSemantics::typeStackBits(dispatcher, *targetType);
                 SValue::Ptr payloadValue = CilSemantics::resizeBoxedPayloadValue(ops, v, payloadBits);
                 if (payloadValue != v)
                      ta.copyFacts(payloadValue, v);

                 SValue::Ptr payloadAddress = CilSemantics::boxedPayloadAddress(dispatcher, ops, result, *targetType);
                 ops->writeMemory(RegisterDescriptor(), payloadAddress, payloadValue, ops->number_(1, 1));
               }
 
               preserveInputKind = false;
               break;
            }

          case unbox_any:
             {
            // CIL instruction: unbox.any <typeTok>
            //
            // Stack transition:
            //   ..., objref -> ..., value-or-objref
            //
            // Purpose:
            //   Extracts a value from a boxed object, or performs a type-compatible
            //   reference conversion depending on the target type.
            //
            // Real CLI behavior:
            //   If the target type is a value type, the boxed value is copied out and
            //   pushed as a value. If the target type is a reference type, the operation
            //   behaves more like a checked reference conversion. Null and incompatible
            //   inputs have type-dependent behavior.
            //
            // Current conservative model:
            //   The exact result width depends on the metadata target type, which is
            //   not fully resolved here. Use an unknown value with a conservative width.
            //
            // Notes:
            //   This instruction is metadata-sensitive. A more precise implementation
            //   should use the target TypeDescriptor to choose the result width and
            //   stack category.
            // Stack: objref -> value or objref, depending on metadata type.
            // Without token/type width, use conservative unknown.
            // If later you pass the type width in nbits, this becomes useful.

            // unbox.any <typeTok> converts an object reference to a value of the target
            // type. For value-type targets, it checks that the object is a compatible boxed
            // value and pushes a copy of the boxed payload. For reference-type targets, it
            // behaves like a checked reference conversion: null remains null, compatible
            // objects are returned, and incompatible objects throw. Since this dispatcher
            // does not yet model boxed-object layout or exception edges, value-type results
            // are modeled as unknown values of the target width, while reference-type
            // results are modeled like conservative castclass results.

               ASSERT_not_null(dispatcher);
               ASSERT_not_null(targetType);

               DispatcherCil::CilTypeAnalysis &ta = dispatcher->typeAnalysis();

               if (targetType->isKnownReferenceType())
                  {
                    DispatcherCil::CilTypeAnalysis::TypeSet possibleTypes = ta.possibleDynamicTypes(v);

                    if (ta.isDefinitelyNull(v))
                       {
                         result = v;
                         ta.copyFacts(result, v);

                    // } else if (ta.allAssignableTo(possibleTypes, targetToken))
                       } else if (ta.allAssignableTo(possibleTypes, targetType))
                       {
                         result = v;
                         ta.copyFacts(result, v);
                       }
                    else if (ta.isDefinitelyNonNull(v) &&
                             ta.noneAssignableTo(possibleTypes, targetType))
                        {
                            CilSemantics::invalidCastExceptionPath(dispatcher, ops);
                            return;
                        }
                      else
                       {
                         result = ops->undefined_(v->nBits());
                         ValueKind kind = targetType->isArray ? ValueKind::ArrayReference : ValueKind::ObjectReference;
                         CilSemantics::markTargetTypeFacts(dispatcher, result, *targetType, kind);

                         ta.setNullness(result, DispatcherCil::CilTypeAnalysis::MaybeNull);
                         preserveInputKind = false;
                         break;
                       }

                    preserveInputKind = true;

                  }
                 else
                  {
                    if (ta.isDefinitelyNull(v)) {
                       SValue::Ptr exceptionObject = ops->undefined_(CilSemantics::cilPointerBits(dispatcher));
                       exceptionObject->kind(ValueKind::ObjectReference);
                       exceptionObject->typeDescriptor("cil:exception:System.NullReferenceException");
                       dispatcher->typeAnalysis().setNullness(
                           exceptionObject, DispatcherCil::CilTypeAnalysis::DefinitelyNonNull);
                       ops->writeLocal(CilSemantics::cilCurrentExceptionSlot(), exceptionObject);
                       CilSemantics::writeUnknownInstructionPointer(dispatcher, ops);
                       ops->comment("CIL unbox.any of definitely-null object reference: NullReferenceException path");
                       return;
                    }
                    CilSemantics::noteMaybeNullReferenceException(dispatcher, ops, v, "unbox.any");

                    const size_t resultBits = CilSemantics::typeStackBits(dispatcher, *targetType);
                    const ValueKind kind = CilSemantics::typeStackValueKind(*targetType, resultBits);

                    if (ta.isDefinitelyNull(v)) {
                       result = ops->undefined_(resultBits);
                       if (targetType->metadataKnown) {
                          CilSemantics::markTargetTypeFacts(dispatcher, result, *targetType, kind);
                       } else {
                          result->kind(kind);
                       }
                       preserveInputKind = false;
                       break;
                    }

                    if (CilSemantics::definitelyInvalidBoxedValueTarget(dispatcher, v, *targetType)) {
                        CilSemantics::invalidCastExceptionPath(dispatcher, ops);
                        return;
                    }

                    SValue::Ptr payloadAddress = CilSemantics::boxedPayloadAddress(dispatcher, ops, v, *targetType);
                    result = ops->readMemory(
                        RegisterDescriptor(),
                        payloadAddress,
                        ops->undefined_(resultBits),
                        ops->number_(1, 1));
                    if (targetType->metadataKnown) {
                       CilSemantics::markTargetTypeFacts(dispatcher, result, *targetType, kind);
                    } else {
                       result->kind(kind);
                    }
                    preserveInputKind = false;
                  }

               break;
             }


          case newarr:
             {
            // CIL instruction: newarr <etype>
            //
            // Stack transition:
            //   ..., length -> ..., array-objref
            //
            // Purpose:
            //   Allocates a one-dimensional, zero-based array whose element type is
            //   specified by the metadata operand. The stack input is the requested
            //   array length.
            //
            // Real CLI behavior:
            //   Negative lengths and allocation failures can throw. The result is a
            //   new array object reference.
            //
            // Current conservative model:
            //   Heap allocation and array layout are not modeled here, so produce an
            //   unknown pointer-sized object reference. If type analysis is available,
            //   this result can be marked definitely non-null and associated with an
            //   array type derived from the element-type token.
            //
            // Notes:
            //   Do not preserve the input ValueKind. The input is an integer length;
            //   the output is an array object reference.
            // Stack: length/native int -> array object reference.

            // newarr <etype> allocates a one-dimensional, zero-based managed array.
            // The input stack value is the requested length; the result is an object
            // reference to the new array. Since this dispatcher does not yet model the
            // managed heap, allocation, array headers, or element storage, represent the
            // result as an unknown pointer-sized object reference. Record type-analysis
            // facts saying that the result is a non-null array object whose element type
            // is the metadata target type. Do not preserve the input kind because the
            // input is an integer length and the output is an array reference.

            // DQ (6/11/2026): Better implementation using SValue class.
               ASSERT_not_null(dispatcher);
               ASSERT_not_null(targetType);

               SValue::Ptr length = v;

               SValue::Ptr arrayRef = ops->undefined_(ptrBits);
               arrayRef->kind(ValueKind::ArrayReference);
               arrayRef->arrayLength(length);
               arrayRef->typeDescriptor(cilTypeTokenDescriptor(targetType->metadataToken));

               if (Sawyer::Optional<int64_t> concreteLength = length->toSigned()) {
                  if (*concreteLength < 0) {
                     CilSemantics::overflowExceptionPath(
                         dispatcher, ops,
                         "CIL newarr: negative array length; OverflowException path not fully modeled");
                     return;
                  } else {
                     // Allocation may throw OutOfMemoryException, but this
                     // dispatcher does not split normal and exceptional
                     // allocation edges.  Do not write an active exception
                     // into the merged state that will also contain the
                     // normally allocated array.
                     ops->comment("CIL newarr: array allocation failure edge not split");
                     dispatcher->typeAnalysis().setNullness(arrayRef, DispatcherCil::CilTypeAnalysis::DefinitelyNonNull);
                  }
               } else {
                  // A symbolic length may be negative and allocation may fail,
                  // but those are not definite on the merged normal path.
                  // Record the imprecision without writing an active exception
                  // into the state that also contains the normal array result.
                  ops->comment("CIL newarr: symbolic array length; negative-length OverflowException edge not split");
                  ops->comment("CIL newarr: symbolic array allocation failure edge not split");
                  dispatcher->typeAnalysis().setNullness(arrayRef, DispatcherCil::CilTypeAnalysis::DefinitelyNonNull);
               }

               result = arrayRef;
               preserveInputKind = false;
               break;
           }


          case ldlen:
             {
            // CIL instruction: ldlen
            //
            // Stack transition:
            //   ..., array-objref -> ..., native-unsigned-int
            //
            // Purpose:
            //   Loads the length of a one-dimensional array and pushes it as a native
            //   unsigned integer.
            //
            // Real CLI behavior:
            //   Null array references throw NullReferenceException.
            //
            // Current conservative model:
            //   Array object layout and length fields are not modeled here, so produce
            //   an unknown native-sized integer. Use ptrBits as the current native-int
            //   approximation.
            //
            // Notes:
            //   Do not preserve the input ValueKind. The input is an array reference;
            //   the result is an integer length.
            // Stack: array ref -> native unsigned int length.
            // CIL returns native unsigned int; use ptrBits until native width is known.

            // DQ (6/11/2026): Better implementation using SValue class.
               if (dispatcher != nullptr && dispatcher->typeAnalysis().isDefinitelyNull(v))
                  {
                 // Real CIL throws NullReferenceException for ldlen on a null
                 // array reference.  This dispatcher does not yet split an
                 // exceptional edge, so conservatively make control unknown.
                    SValue::Ptr exceptionObject = ops->undefined_(CilSemantics::cilPointerBits(dispatcher));
                    exceptionObject->kind(ValueKind::ObjectReference);
                    exceptionObject->typeDescriptor("cil:exception:System.NullReferenceException");
                    dispatcher->typeAnalysis().setNullness(
                        exceptionObject, DispatcherCil::CilTypeAnalysis::DefinitelyNonNull);
                    ops->writeLocal(CilSemantics::cilCurrentExceptionSlot(), exceptionObject);
                    CilSemantics::writeUnknownInstructionPointer(dispatcher, ops);
                    return;
                  }
               else
                  {
                    if (dispatcher != nullptr)
                         CilSemantics::noteMaybeNullReferenceException(dispatcher, ops, v, "ldlen");

                    if (v->hasArrayLength())
                       {
                      // Do not push the stored arrayLength SValue itself: the code
                      // below sets the result kind to NativeInt, and mutating the
                      // stored length object would alter array metadata.  Derive a
                      // fresh value with equivalent bits instead.
                         SValue::Ptr length = v->arrayLength();
                         ASSERT_not_null(length);

                         if (length->nBits() < ptrBits)
                            {
                              result = ops->unsignedExtend(length, ptrBits);
                            }
                           else
                            {
                              if (length->nBits() > ptrBits)
                                 {
                                   result = ops->extract(length, 0, ptrBits);
                                 }
                                else
                                 {
                                   result = ops->add(length, ops->number_(ptrBits, 0));
                                 }

                            }
                       }
                      else
                       {
                         result = ops->undefined_(ptrBits);
                       }
                  }

               result->kind(ValueKind::NativeInt);
               preserveInputKind = false;
               break;
             }

          case localloc:
             {
            // CIL instruction: localloc
            //
            // Stack transition:
            //   ..., size -> ..., native-address
            //
            // Purpose:
            //   Allocates a block of memory from the local dynamic memory pool for the
            //   current method activation and pushes the native address of that block.
            //
            // Complete ROSE/RISC-level model:
            //   1. Pop the requested allocation size.
            //   2. Read the current local allocation pointer.
            //   3. Align the requested size.
            //   4. Add the aligned size to the local allocation pointer.
            //   5. Write the updated pointer back to semantic state.
            //   6. Push the old pointer as the result address.
            //
            // Notes:
            //   This models allocation, but it does not initialize the allocated memory.
            //   It also does not model stack overflow or security restrictions.

               ASSERT_not_null(dispatcher);

               DispatcherCil::CilTypeAnalysis &ta = dispatcher->typeAnalysis();

               SValue::Ptr size = v;
               ASSERT_not_null(size);

            // Normalize allocation size to native pointer width.
               if (size->nBits() < ptrBits)
                  {
                    size = ops->unsignedExtend(size, ptrBits);
                  } else if (size->nBits() > ptrBits)
                  {
                    size = ops->extract(size, 0, ptrBits);
                  }
#if CIL_DEBUG
            // DQ (6/10/2026): Until we have a concept of heap that we can support, we can skip some parts of the semantics.
               fprintf(stderr, "unaryResult: case localloc: allocation of heap memory bypassed \n");
               fflush(stderr);
#endif
            // Model a method-local dynamic allocation area with a hidden
            // monotonic pointer.  If the slot has not been initialized yet,
            // start from the synthetic CIL local-allocation base.
               SValue::Ptr oldPointer = ops->readLocal(CilSemantics::cilLocalAllocPointerSlot());
               if (oldPointer == NULL)
                    oldPointer = ops->number_(ptrBits, CIL_LOCAL_ALLOC_BASE);
               else
                    oldPointer = CilSemantics::normalizeToPointerBits(dispatcher, ops, oldPointer);

               SValue::Ptr alignedSize = size;
               if (Sawyer::Optional<uint64_t> concreteSize = size->toUnsigned())
                  {
                    const uint64_t alignment = CIL_LOCAL_ALLOC_ALIGNMENT;
                    const uint64_t aligned = (*concreteSize + (alignment - 1)) & ~(alignment - 1);
                    alignedSize = ops->number_(ptrBits, aligned);
                 }
                else
                  {
                    const uint64_t alignment = CIL_LOCAL_ALLOC_ALIGNMENT;
                    const uint64_t allOnes =
                        ptrBits >= 64 ? UINT64_MAX : ((uint64_t{1} << ptrBits) - 1);
                    const uint64_t alignmentMask = allOnes & ~(alignment - 1);
                    alignedSize = ops->and_(
                        ops->add(size, ops->number_(ptrBits, alignment - 1)),
                        ops->number_(ptrBits, alignmentMask));
                  }

               SValue::Ptr newPointer = ops->add(oldPointer, alignedSize);
               ops->writeLocal(CilSemantics::cilLocalAllocPointerSlot(), newPointer);

               result = oldPointer;
               result->kind(ValueKind::UnmanagedPointer);
               result->typeDescriptor("cil:localloc-address");

           // The normal localloc result is a non-null native address in this model.
           // localloc may throw StackOverflowException, but this dispatcher does
           // not split normal and exceptional allocation edges.  Do not write
           // an active exception into the merged state that will also contain
           // the normally allocated address.
               ops->comment("CIL localloc: stack allocation failure edge not split");
               ta.setNullness(result, DispatcherCil::CilTypeAnalysis::DefinitelyNonNull);
               preserveInputKind = false;
               break;
           }
#if 0
          case mkrefany:
             {
            // CIL instruction: mkrefany <typeTok>
            //
            // Stack transition:
            //   ..., managed-pointer -> ..., typedref
            //
            // Purpose:
            //   Builds a typed reference from a managed pointer and a metadata type
            //   token. A typed reference conceptually contains both the address and the
            //   runtime type information for the referenced value.
            //
            // Current conservative model:
            //   The semantic domain does not currently have a concrete typedref layout.
            //   Model the result as an unknown value. Using 2 * ptrBits is a useful
            //   approximation because a typedref conceptually contains both a type
            //   handle and an address.
            //
            // Notes:
            //   Do not preserve the input ValueKind. The input is a managed pointer;
            //   the output is a typed reference.

            // Stack: address -> typedref.
            // A typedref is conceptually {type, address}. Without a typedref
            // layout in the semantic domain, use an unknown value. 2*ptrBits is
            // a better conservative model than ptrBits if you want to remember
            // that typedref carries both type and address.

            // This implementation can handle when the targetType is not available (because args to unaryResult were size zero).
               ASSERT_not_null(dispatcher);

               SValue::Ptr addr = CilSemantics::normalizeToPointerBits(dispatcher, ops, v);

               if (targetType != nullptr)
                  {
                    SValue::Ptr typeHandle = ops->number_(ptrBits, targetType->metadataToken);

                 // result = ops->concat(typeHandle, addr);

                 // DQ (6/28/2026): This code is consistant with refanytype.
                 // refanyval: extract address
                    result = ops->extract(v, 0, ptrBits);
                    result->kind(ValueKind::ManagedPointer);
                  }
                 else
                  {
                 // mkrefany requires an InlineType token, but the current decoder/test
                 // path consumed the operand bytes without exposing the token in the
                 // SgAsmOperandList. Keep semantics alive with an unknown typedref.
                    result = ops->undefined_(2 * ptrBits);
                  }

               preserveInputKind = false;
               break;
             }
#else
case mkrefany:
   {
      // CIL instruction: mkrefany <typeTok>
      //
      // Stack:
      //   ..., managed-pointer -> ..., typedref
      //
      // Internal model:
      //   typedref = { high: type token, low: managed address }
      //
      // In ROSE terms:
      //   concatLoHi(address, typeHandle)

            // Purpose:
            //   Builds a typed reference from a managed pointer and a metadata type
            //   token. A typed reference conceptually contains both the address and the
            //   runtime type information for the referenced value.
            //
            // Current conservative model:
            //   The semantic domain does not currently have a concrete typedref layout.
            //   Model the result as an unknown value. Using 2 * ptrBits is a useful
            //   approximation because a typedref conceptually contains both a type
            //   handle and an address.
            //
            // Notes:
            //   Do not preserve the input ValueKind. The input is a managed pointer;
            //   the output is a typed reference.

            // Stack: address -> typedref.
            // A typedref is conceptually {type, address}. Without a typedref
            // layout in the semantic domain, use an unknown value. 2*ptrBits is
            // a better conservative model than ptrBits if you want to remember
            // that typedref carries both type and address.

      ASSERT_not_null(dispatcher);

      if (targetType == nullptr || !targetType->isValid()) {
          // mkrefany really requires an InlineType token. If the decoder did
          // not expose one, preserve the packed layout and the address in the
          // low half while making only the type-handle half unknown.
          SValue::Ptr unknownTypeHandle = ops->undefined_(ptrBits);
          unknownTypeHandle->kind(ValueKind::NativeInt);
          unknownTypeHandle->typeDescriptor("cil:RuntimeTypeHandle:unknown");

          result = CilSemantics::makePackedTypedReferenceWithHandle(
              dispatcher,
              ops,
              v,
              unknownTypeHandle,
              "cil:typedref:unknown");
      } else {
          result = CilSemantics::makePackedTypedReference(
              dispatcher,
              ops,
              v,
              targetType->metadataToken);
      }

      preserveInputKind = false;
      break;
   }
#endif
#if 0
        // DQ (6/9/2026): We might want this one later, but we currently need one that will match mkrefany.
          case refanyval:
             {
            // CIL instruction: refanyval <typeTok>
            //
            // Stack transition:
            //   ..., typedref -> ..., managed-pointer
            //
            // Purpose:
            //   Extracts the address stored in a typed reference if the typed reference
            //   has the requested target type.
            //
            // Real CLI behavior:
            //   The typed reference carries both a type descriptor and an address. If
            //   the embedded type is not compatible with the metadata target type, the
            //   runtime can throw.
            //
            // Current conservative model:
            //   Typed-reference layout is not modeled, so produce an unknown pointer-
            //   sized managed pointer.
            //
            // Notes:
            //   This is metadata-sensitive and should eventually consult the typedref's
            //   stored type and the target type token.
            // Stack: typedref -> managed pointer to value if type matches.

               ASSERT_not_null(dispatcher);
               ASSERT_not_null(targetType);

               static const uint64_t typedRefAddressOffset = 0; // TODO: define layout.

               SValue::Ptr offset = ops->number_(v->nBits(), typedRefAddressOffset);
               SValue::Ptr addressFieldAddr = ops->add(v, offset);

               SValue::Ptr dflt = ops->undefined_(ptrBits);
               SValue::Ptr cond = ops->boolean_(true);

               result = ops->readMemory(RegisterDescriptor(), addressFieldAddr, dflt, cond);

               preserveInputKind = false;
               break;
             }
#else
case refanyval:
   {
      // CIL instruction: refanyval <typeTok>
      //
      // Stack:
      //   ..., typedref -> ..., managed-pointer
      //
      // The address is embedded in the typedref itself. Do NOT read memory
      // at the typedref value.

      ASSERT_not_null(dispatcher);
      ASSERT_not_null(targetType);

      // Optional precision: if the embedded token is concrete and the target
      // token is concrete, detect a definite mismatch.
      if (targetType->isValid()) {
          Sawyer::Optional<uint64_t> embeddedToken =
              CilSemantics::concretePackedTypedReferenceTypeToken(
                  dispatcher, ops, v);

          if (embeddedToken && *embeddedToken != targetType->metadataToken) {
              ops->comment("CIL refanyval: typedref type mismatch; InvalidCastException path not fully modeled");
              CilSemantics::invalidCastExceptionPath(dispatcher, ops);
              return;
          }
      }

      result = CilSemantics::packedTypedReferenceAddress(dispatcher, ops, v);

      if (targetType->isValid()) {
          result->typeDescriptor(
              CilSemantics::cilTypeTokenDescriptor(targetType->metadataToken));
      }

      preserveInputKind = false;
      break;
   }
#endif
#if 0
       // case refanyval:
          case refanytype:
             {
            // CIL instruction: refanytype
            //
            // Stack transition:
            //   ..., typedref -> ..., RuntimeTypeHandle
            //
            // Purpose:
            //   Extracts the runtime type handle stored in a typed reference.
            //
            // Current conservative model:
            //   Typed-reference layout and runtime type handles are not modeled here,
            //   so produce an unknown pointer-sized type-handle value.
            //
            // Notes:
            //   Do not preserve the input ValueKind. The input is a typed reference;
            //   the output is a type-handle-like value.
            // Stack: typedref -> RuntimeTypeHandle / type token-like value.
               if (v->nBits() >= 2 * ptrBits)
                  {
                 // result = ops->extract(v, 0, ptrBits);

                 // DQ (6/28/2026): This code is consistant with mkrefany.
                 // refanytype: extract type handle
                    result = ops->extract(v, ptrBits, 2 * ptrBits);
                    result->kind(ValueKind::NativeInt);
                  }
                 else
                  {
                    result = ops->undefined_(ptrBits);
                  }

               preserveInputKind = false;
               break;
             }
#else
case refanytype:
   {
      // CIL instruction: refanytype
      //
      // Stack:
      //   ..., typedref -> ..., RuntimeTypeHandle-like native value

      ASSERT_not_null(dispatcher);

      result = CilSemantics::packedTypedReferenceTypeHandle(
          dispatcher,
          ops,
          v);

      preserveInputKind = false;
      break;
   }
#endif
          default:
             {
               fprintf(stderr, "unaryResult: default reached: op_kind = %d\n", op_kind);
               fflush(stderr);
               ROSE_ASSERT(false);
             }
        }

          ASSERT_not_null(result);

       // Preserve the CIL stack category only for instructions whose result is the
       // same kind of value as the input. This is correct for operations like neg,
       // not, castclass on the normal path, and ckfinite. It is not correct for
       // operations that change categories, such as box, unbox, ldlen, localloc,
       // mkrefany, refanytype, or ldvirtftn.
       // Only preserve ValueKind for operations whose result is the same category
       // as the input. Do not do this unconditionally: ldlen, localloc, box, etc.
       // change the stack type category.
          if (preserveInputKind)
               result->kind(v->kind());

#if CIL_DEBUG
          fprintf(stderr, "unaryResult: op_kind = %s input = %s result = %s\n",
                 unary_op_kind_toString(op_kind).c_str(),
                 v->toString().c_str(),
                 result->toString().c_str());
          fflush(stderr);
#endif

          ops->pushOperand(result);
        }

   // DQ (6/4/2026): Added to support different kinds of binary operators.
     enum Binary_Op_Kind_Type
        {
          add,
          add_with_overflow_check,
          add_unsigned_with_overflow_check,
          subtract,
          subtract_with_overflow_check,
          subtract_unsigned_with_overflow_check,
          multiply,
          multiply_with_overflow_check,
          multiply_unsigned_with_overflow_check,
          divide,
          divide_unsigned,
          and_op,
          or_op,
          xor_op,
          remainder,
          remainder_unsigned,
          shiftleft_op,
          shiftright_op,
          shiftleft_unsigned,
          shiftright_unsigned,
        };

     std::string op_kind_toString (Binary_Op_Kind_Type op_kind)
        {
          std::string s;
          switch (op_kind)
             {
               case add:                                   s = "add"; break;
               case add_with_overflow_check:               s = "add_with_overflow_check"; break;
               case add_unsigned_with_overflow_check:      s = "add_unsigned_with_overflow_check"; break;
               case subtract:                              s = "subtract"; break;
               case subtract_with_overflow_check:          s = "subtract_with_overflow_check"; break;
               case subtract_unsigned_with_overflow_check: s = "subtract_unsigned_with_overflow_check"; break;
               case multiply:                              s = "multiply"; break;
               case multiply_with_overflow_check:          s = "multiply_with_overflow_check"; break;
               case multiply_unsigned_with_overflow_check: s = "multiply_unsigned_with_overflow_check"; break;
               case divide:                                s = "divide"; break;
               case divide_unsigned:                       s = "divide_unsigned"; break;
               case and_op:                                s = "and_op"; break;
               case or_op:                                 s = "or_op"; break;
               case xor_op:                                s = "xor_op"; break;
               case remainder:                             s = "remainder"; break;
               case remainder_unsigned:                    s = "remainder_unsigned"; break;
               case shiftleft_op:                          s = "shiftleft_op"; break;
               case shiftright_op:                         s = "shiftright_op"; break;
               case shiftleft_unsigned:                    s = "shiftleft_unsigned"; break;
               case shiftright_unsigned:                   s = "shiftright_unsigned"; break;
               default:
                  {
                    fprintf (stderr,"op_kind_toString: default reached: rhs = %d \n",op_kind);
                    fflush(stderr);
                    ROSE_ASSERT(false);
                  }
             }

          return s;
        }

     static bool
     isCilIntegerLike(ValueKind k)
        {
          return k == ValueKind::Integer32 ||
                 k == ValueKind::Integer64 ||
                 k == ValueKind::NativeInt;
        }

     static ValueKind
     integerBinaryResultKind(ValueKind lhsKind, ValueKind rhsKind)
        {
          if (lhsKind == ValueKind::Integer64 || rhsKind == ValueKind::Integer64)
               return ValueKind::Integer64;

          if (lhsKind == ValueKind::NativeInt || rhsKind == ValueKind::NativeInt)
               return ValueKind::NativeInt;

          if (lhsKind == ValueKind::Integer32 && rhsKind == ValueKind::Integer32)
               return ValueKind::Integer32;

          return ValueKind::Unknown;
        }

static size_t
cilStackBitsForKind(D dispatcher, ValueKind kind)
{
    ASSERT_not_null(dispatcher);

    switch (kind) {
        case ValueKind::Integer32:
            return 32;

        case ValueKind::Integer64:
            return 64;

        case ValueKind::NativeInt:
            return CilSemantics::cilPointerBits(dispatcher);

        default:
            return 0;
    }
}

static SValue::Ptr
resizeUnsignedToBits(Ops ops, const SValue::Ptr &value, size_t bits)
{
    ASSERT_not_null(ops);
    ASSERT_not_null(value);
    ASSERT_require(bits > 0);

    if (value->nBits() < bits)
        return ops->unsignedExtend(value, bits);

    if (value->nBits() > bits)
        return ops->extract(value, 0, bits);

    return value;
}

static SValue::Ptr
resizeSignedToBits(Ops ops, const SValue::Ptr &value, size_t bits)
{
    ASSERT_not_null(ops);
    ASSERT_not_null(value);
    ASSERT_require(bits > 0);

    if (value->nBits() < bits)
        return ops->signExtend(value, bits);

    if (value->nBits() > bits)
        return ops->extract(value, 0, bits);

    return value;
}

#if 0
static bool
isUnsignedBinaryOp(Binary_Op_Kind_Type op_kind)
{
    switch (op_kind) {
        case add_unsigned_with_overflow_check:
        case subtract_unsigned_with_overflow_check:
        case multiply_unsigned_with_overflow_check:
        case divide_unsigned:
        case remainder_unsigned:
        case and_op:
        case or_op:
        case xor_op:
            return true;

        default:
            return false;
    }
}
#endif

static bool
isSignedBinaryOp(Binary_Op_Kind_Type op_kind)
{
    switch (op_kind) {
        case add:
        case add_with_overflow_check:
        case subtract:
        case subtract_with_overflow_check:
        case multiply:
        case multiply_with_overflow_check:
        case divide:
        case remainder:
            return true;

        default:
            return false;
    }
}

static bool
isOverflowBinaryOp(Binary_Op_Kind_Type op_kind)
{
    switch (op_kind) {
        case add_with_overflow_check:
        case add_unsigned_with_overflow_check:
        case subtract_with_overflow_check:
        case subtract_unsigned_with_overflow_check:
        case multiply_with_overflow_check:
        case multiply_unsigned_with_overflow_check:
            return true;

        default:
            return false;
    }
}

static bool
signedBounds(size_t bits, int64_t &lo, int64_t &hi)
{
    if (bits == 0 || bits > 64)
        return false;

    if (bits == 64) {
        lo = INT64_MIN;
        hi = INT64_MAX;
        return true;
    }

    const int64_t one = 1;
    lo = -(one << (bits - 1));
    hi =  (one << (bits - 1)) - 1;
    return true;
}

static bool
unsignedMax(size_t bits, uint64_t &hi)
{
    if (bits == 0 || bits > 64)
        return false;

    if (bits == 64)
        hi = UINT64_MAX;
    else
        hi = (uint64_t(1) << bits) - 1;

    return true;
}


static bool
concreteSignedSourceConversionOverflows(const SValue::Ptr &value,
                                        const CilConversionTarget &target)
{
    ASSERT_not_null(value);

    Sawyer::Optional<int64_t> source = value->toSigned();

    // Symbolic or non-concrete value: cannot prove overflow here.
    if (!source)
        return false;

    const int64_t x = *source;

    if (target.targetIsSigned) {
        int64_t lo = 0;
        int64_t hi = 0;

        if (!signedBounds(target.targetBits, lo, hi))
            return false;

        return x < lo || x > hi;
    } else {
        // Signed source to unsigned target.
        // Negative values do not fit in unsigned targets.
        if (x < 0)
            return true;

        uint64_t hi = 0;

        if (!unsignedMax(target.targetBits, hi))
            return false;

        return static_cast<uint64_t>(x) > hi;
    }
}

static bool
concreteUnsignedSourceConversionOverflows(const SValue::Ptr &value,
                                          const CilConversionTarget &target)
{
    ASSERT_not_null(value);

    Sawyer::Optional<uint64_t> source = value->toUnsigned();

    // Symbolic or non-concrete value: cannot prove overflow here.
    if (!source)
        return false;

    const uint64_t x = *source;

    if (target.targetIsSigned) {
        int64_t lo = 0;
        int64_t hi = 0;

        if (!signedBounds(target.targetBits, lo, hi))
            return false;

        // Unsigned source is never below signed lower bound unless the target
        // lower bound is positive, which it is not for normal signed integer
        // targets. The real check is against signed max.
        return x > static_cast<uint64_t>(hi);
    } else {
        uint64_t hi = 0;

        if (!unsignedMax(target.targetBits, hi))
            return false;

        return x > hi;
    }
}

static SValue::Ptr
finishOverflowConversion(Ops ops,
                         const SValue::Ptr &value,
                         const CilConversionTarget &target,
                         bool sourceIsUnsigned)
{
    ASSERT_not_null(ops);
    ASSERT_not_null(value);

    SValue::Ptr result = value;

    // First convert to the target storage width.
    //
    // This is safe only after overflow has been checked against the original
    // value.
    if (result->nBits() > target.targetBits) {
        result = ops->extract(result, 0, target.targetBits);
    } else if (result->nBits() < target.targetBits) {
        if (sourceIsUnsigned)
            result = ops->unsignedExtend(result, target.targetBits);
        else
            result = ops->signExtend(result, target.targetBits);
    }

    // Then convert from target width to CIL stack width.
    if (result->nBits() < target.stackBits) {
        if (target.targetIsSigned)
            result = ops->signExtend(result, target.stackBits);
        else
            result = ops->unsignedExtend(result, target.stackBits);
    } else if (result->nBits() > target.stackBits) {
        result = ops->extract(result, 0, target.stackBits);
    }

    result->kind(target.resultKind);
    return result;
}



static bool
concreteSignedOverflow(Binary_Op_Kind_Type op_kind,
                       const SValue::Ptr &lhs,
                       const SValue::Ptr &rhs,
                       size_t bits)
{
    ASSERT_not_null(lhs);
    ASSERT_not_null(rhs);

    Sawyer::Optional<int64_t> lhsOpt = lhs->toSigned();
    Sawyer::Optional<int64_t> rhsOpt = rhs->toSigned();

    if (!lhsOpt || !rhsOpt)
        return false;

    int64_t lo = 0, hi = 0;
    if (!signedBounds(bits, lo, hi))
        return false;

    const int64_t a = *lhsOpt;
    const int64_t b = *rhsOpt;

    switch (op_kind) {
        case add_with_overflow_check:
            if ((b > 0 && a > hi - b) || (b < 0 && a < lo - b))
                return true;
            return false;

        case subtract_with_overflow_check:
            if ((b < 0 && a > hi + b) || (b > 0 && a < lo + b))
                return true;
            return false;

        case multiply_with_overflow_check: {
#if defined(__SIZEOF_INT128__)
            __int128 product = static_cast<__int128>(a) * static_cast<__int128>(b);
            return product < lo || product > hi;
#else
            // Conservative fallback: do not prove overflow without wider arithmetic.
            return false;
#endif
        }

        default:
            return false;
    }
}

static bool
concreteUnsignedOverflow(Binary_Op_Kind_Type op_kind,
                         const SValue::Ptr &lhs,
                         const SValue::Ptr &rhs,
                         size_t bits)
{
    ASSERT_not_null(lhs);
    ASSERT_not_null(rhs);

    Sawyer::Optional<uint64_t> lhsOpt = lhs->toUnsigned();
    Sawyer::Optional<uint64_t> rhsOpt = rhs->toUnsigned();

    if (!lhsOpt || !rhsOpt)
        return false;

    uint64_t hi = 0;
    if (!unsignedMax(bits, hi))
        return false;

    const uint64_t a = *lhsOpt;
    const uint64_t b = *rhsOpt;

    switch (op_kind) {
        case add_unsigned_with_overflow_check:
            return a > hi - b;

        case subtract_unsigned_with_overflow_check:
            return a < b;

        case multiply_unsigned_with_overflow_check:
            if (b == 0)
                return false;
            return a > hi / b;

        default:
            return false;
    }
}

#if 0
  // DQ (6/28/2026): This version may provide more accurate semantics. but may not be easier to use to stand up initial testing.
     static void
     overflowExceptionEffect(D dispatcher, Ops ops, I insn, Binary_Op_Kind_Type op_kind)
        {
          ASSERT_not_null(dispatcher);
          ASSERT_not_null(ops);

          if (insn != NULL)
             {
               ops->comment("CIL " + op_kind_toString(op_kind) + ": concrete arithmetic overflow; OverflowException path not fully modeled");
             }

          ops->writeRegister(dispatcher->instructionPointerRegister(), ops->undefined_(dispatcher->instructionPointerRegister().nBits()));
        }
#else
#if 0
  // DQ (6/28/2026): This version may be easier to use to stand up initial testing.
     static void
     overflowExceptionEffect(D /*dispatcher*/, Ops /*ops*/, I insn, Binary_Op_Kind_Type op_kind)
        {
          throw BaseSemantics::Exception("CIL " + op_kind_toString(op_kind) + " concrete overflow: OverflowException not modeled", insn);
        }
#else
#if 0
     static void
     overflowExceptionEffect(D dispatcher, Ops ops, I insn, Binary_Op_Kind_Type op_kind)
        {
          ASSERT_not_null(dispatcher);
          ASSERT_not_null(ops);

          if (!dispatcher->allowByteOnlyExceptionFallback())
             {
               throw BaseSemantics::Exception("CIL " + op_kind_toString(op_kind) + " concrete overflow: OverflowException not modeled", insn);
             }

          ops->comment("CIL " + op_kind_toString(op_kind) + ": concrete overflow; byte-only fallback suppresses OverflowException");

          ops->writeRegister(dispatcher->instructionPointerRegister(), ops->undefined_(dispatcher->instructionPointerRegister().nBits()));
        }
#endif
     static void
     overflowExceptionEffect(D dispatcher, Ops ops, I insn, Binary_Op_Kind_Type op_kind)
        {
          ASSERT_not_null(dispatcher);
          ASSERT_not_null(ops);

          (void)insn;

          CilSemantics::overflowExceptionPath(
              dispatcher, ops,
              "CIL " + op_kind_toString(op_kind) +
              ": concrete arithmetic overflow; OverflowException path not fully modeled");
        }
#endif
#endif

     static bool
     isShiftOp(Binary_Op_Kind_Type op_kind)
        {
          return op_kind == shiftleft_op ||
                 op_kind == shiftleft_unsigned ||
                 op_kind == shiftright_op ||
                 op_kind == shiftright_unsigned;
        }

  // DQ (6/24/2026): This version improves the semantics.
     static SValue::Ptr
     normalizeShiftCount(Ops ops, const SValue::Ptr &count, size_t targetBits)
        {
          ASSERT_not_null(ops);
          ASSERT_not_null(count);
          ASSERT_require(targetBits > 0);

       // ECMA-335 CIL shift semantics:
       //
       //   int32 / native int on 32-bit target: use low 5 bits of the shift count
       //   int64 / native int on 64-bit target: use low 6 bits of the shift count
       //
       // In other words:
       //
       //   shl/shr on 32-bit value: count & 0x1f
       //   shl/shr on 64-bit value: count & 0x3f
       //
       // ROSE's bit-vector shift operators expect the shift-count operand to have
       // the same width as the value being shifted, so we first extract the
       // architecturally meaningful low bits, then zero-extend them to targetBits.

          size_t maskBits = 0;

          if (targetBits <= 32) {
               maskBits = 5;
          } else if (targetBits <= 64) {
              maskBits = 6;
          } else {
         // CIL should normally not reach this case for integer shifts, but this
         // keeps the helper well-defined for wider semantic values.
         //
         // Compute ceil(log2(targetBits)).
              size_t n = targetBits - 1;
              maskBits = 0;
              while (n != 0) {
                  ++maskBits;
                  n >>= 1;
              }
          }

       // We cannot extract more bits than the count value actually contains.
          const size_t extractedBits = std::min(maskBits, count->nBits());

          SValue::Ptr normalized;

          if (extractedBits == count->nBits()) {
              normalized = count;
          } else {
              normalized = ops->extract(count, 0, extractedBits);
          }

          if (normalized->nBits() < targetBits) {
              normalized = ops->unsignedExtend(normalized, targetBits);
          } else if (normalized->nBits() > targetBits) {
              normalized = ops->extract(normalized, 0, targetBits);
          }

          return normalized;
        }

     static SValue::Ptr
     shiftResult(Ops ops, Binary_Op_Kind_Type op_kind, const SValue::Ptr &lhs, const SValue::Ptr &rhs)
        {
          ASSERT_not_null(ops);
          ASSERT_not_null(lhs);
          ASSERT_not_null(rhs);

          ValueKind resultKind = lhs->kind();

       // if (!isCilIntegerLike(resultKind) && resultKind != ValueKind::NativeInt)
          if (!isCilIntegerLike(resultKind))
             {
               SValue::Ptr result = ops->undefined_(lhs->nBits());
               result->kind(ValueKind::Unknown);
               return result;
             }

          SValue::Ptr shiftCount = normalizeShiftCount(ops, rhs, lhs->nBits());

          SValue::Ptr result;

          switch (op_kind)
             {
               case shiftleft_op:
               case shiftleft_unsigned:
                    result = ops->shiftLeft(lhs, shiftCount);
                    break;

               case shiftright_op:
                    result = ops->shiftRightArithmetic(lhs, shiftCount);
                    break;

               case shiftright_unsigned:
                    result = ops->shiftRight(lhs, shiftCount);
                    break;

               default:
                    ASSERT_not_reachable("not a CIL shift operation");
            }

          result->kind(resultKind);
          return result;
        }



  // DQ (6/11/2026): provide a prototype with the default parameter, but define the function so
  // that we don't get a warning about nbits not being used. I expect that we will need it late.
  // void binaryResult(Ops ops, Binary_Op_Kind_Type op_kind, size_t nbits = 32 );
  // void binaryResult(Ops ops, Binary_Op_Kind_Type op_kind );
     void binaryResult(D dispatcher, Ops ops, I insn, Binary_Op_Kind_Type op_kind);

#if 0
  // void binaryResult(Ops ops, Binary_Op_Kind_Type op_kind, size_t /* nbits = 32 */ )
     void binaryResult(Ops ops, Binary_Op_Kind_Type op_kind )
        {
          ASSERT_not_null(ops);
          SValue::Ptr rhs = ops->popOperand();
          SValue::Ptr lhs = ops->popOperand();

          SValue::Ptr result;

          ValueKind resultKind = ValueKind::Unknown;

       // Do not use the generic integer rule for shifts.
       // For shifts, the result kind is the lhs kind and the rhs is just the shift count.
          if (op_kind == shiftleft_op  || op_kind == shiftleft_unsigned ||
              op_kind == shiftright_op || op_kind == shiftright_unsigned)
             {
               resultKind = lhs->kind();
             }
            else
             {
               resultKind = integerBinaryResultKind(lhs->kind(), rhs->kind());
             }

          if (resultKind == ValueKind::Unknown)
             {
               result = ops->undefined_(std::max(lhs->nBits(), rhs->nBits()));
               result->kind(ValueKind::Unknown);
               ops->pushOperand(result);
               return;
             }

          if (isShiftOp(op_kind))
             {
               result = shiftResult(ops, op_kind, lhs, rhs);
               ops->pushOperand(result);
               return;
             }

       // DQ (6/22/2026): This is too strict for Cil.
       // Verify types are the same
       // ASSERT_require2(rhs->kind() == lhs->kind(), "type mismatch");

          std::string rhs_str = rhs->toString();
          std::string lhs_str = lhs->toString();

#if CIL_DEBUG
          fprintf (stderr,"binaryResult: op_kind_toString = %s lhs = %s rhs = %s \n",op_kind_toString(op_kind).c_str(),lhs_str.c_str(),rhs_str.c_str());
          fflush(stderr);
#endif
       // const size_t lhsBits = lhs ? lhs->nBits() : nbits;
       // const size_t rhsBits = rhs ? rhs->nBits() : nbits;

       // ops->pushOperand(ops->undefined_(std::max(nbits, std::max(lhsBits, rhsBits))));
       // ops->pushOperand(ops->add(rhs,lhs));
       // ops->pushOperand(lhs);
       // SValue::Ptr result = ops->add(rhs,lhs);
       // SValue::Ptr result;
          switch (op_kind)
             {
               case add:
                  {
                    result = ops->add(lhs,rhs);
                    result->kind(cilNumericResultKind(lhs->kind(), rhs->kind()));
                    break;
                  }
               case add_with_overflow_check:
                  {
                    result = ops->add(lhs,rhs);
                    result->kind(cilNumericResultKind(lhs->kind(), rhs->kind()));
                    break;
                  }
               case add_unsigned_with_overflow_check:
                  {
                    result = ops->add(lhs,rhs);
                    result->kind(cilNumericResultKind(lhs->kind(), rhs->kind()));
                    break;
                  }
               case subtract:
                  {
                 // DQ (6/17/2026): Fix non-communative operations.
                 // result = ops->subtract(rhs,lhs);
                    result = ops->subtract(lhs,rhs);
                    result->kind(cilNumericResultKind(lhs->kind(), rhs->kind()));
                    break;
                  }
               case subtract_with_overflow_check:
                  {
                 // DQ (6/17/2026): Fix non-communative operations.
                 // result = ops->subtract(rhs,lhs);
                    result = ops->subtract(lhs,rhs);
                    result->kind(cilNumericResultKind(lhs->kind(), rhs->kind()));
                    break;
                  }
               case subtract_unsigned_with_overflow_check:
                  {
                 // DQ (6/17/2026): Fix non-communative operations.
                 // result = ops->subtract(rhs,lhs);
                    result = ops->subtract(lhs,rhs);
                    result->kind(cilNumericResultKind(lhs->kind(), rhs->kind()));
                    break;
                  }
               case multiply:
                  {
                    result = ops->signedMultiply(lhs,rhs);
                    result->kind(cilNumericResultKind(lhs->kind(), rhs->kind()));
                    break;
                  }
               case multiply_with_overflow_check:
                  {
                    result = ops->signedMultiply(lhs,rhs);
                    result->kind(cilNumericResultKind(lhs->kind(), rhs->kind()));
                    break;
                  }
               case multiply_unsigned_with_overflow_check:
                  {
                    result = ops->unsignedMultiply(lhs,rhs);
                    result->kind(cilNumericResultKind(lhs->kind(), rhs->kind()));
                    break;
                  }
               case divide:
                  {
                 // DQ (6/17/2026): Fix non-communative operations.
                 // result = ops->signedDivide(rhs,lhs);
                    result = ops->signedDivide(lhs,rhs);
                    result->kind(cilNumericResultKind(lhs->kind(), rhs->kind()));
                    break;
                  }
               case divide_unsigned:
                  {
                 // result = ops->unsignedDivide(rhs,lhs);
                    result = ops->unsignedDivide(lhs,rhs);
                    result->kind(cilNumericResultKind(lhs->kind(), rhs->kind()));
                    break;
                  }
               case and_op:
                  {
                    result = ops->and_(lhs,rhs);
                    result->kind(cilNumericResultKind(lhs->kind(), rhs->kind()));
                    break;
                  }
               case or_op:
                  {
                    result = ops->or_(lhs,rhs);
                    result->kind(cilNumericResultKind(lhs->kind(), rhs->kind()));
                    break;
                  }
               case xor_op:
                  {
                    result = ops->xor_(lhs,rhs);
                    result->kind(cilNumericResultKind(lhs->kind(), rhs->kind()));
                    break;
                  }
               case remainder:
                  {
                 // The lhs is the dividend, and the rhs is the divisor (I think).
                 // result = ops->signedModulo(rhs, lhs);
                    result = ops->signedModulo(lhs,rhs);
                    result->kind(cilNumericResultKind(lhs->kind(), rhs->kind()));
                    break;
                  }
               case remainder_unsigned:
                  {
                 // The lhs is the dividend, and the rhs is the divisor (I think).
                 // result = ops->unsignedModulo(rhs, lhs);
                    result = ops->unsignedModulo(lhs,rhs);
                    result->kind(cilNumericResultKind(lhs->kind(), rhs->kind()));
                    break;
                  }
               case shiftleft_op:
                  {
                 // result = ops->shiftLeft(rhs,lhs);
                    result = ops->shiftLeft(lhs,rhs);
                    result->kind(lhs->kind());
                    break;
                  }
               case shiftleft_unsigned:
                  {
                 // result = ops->shiftLeft(rhs,lhs);
                    result = ops->shiftLeft(lhs,rhs);
                    result->kind(lhs->kind());
                    break;
                  }
               case shiftright_op:
                  {
                 // result = ops->shiftRightArithmetic(rhs,lhs);
                    result = ops->shiftRightArithmetic(lhs,rhs);
                    result->kind(lhs->kind());
                    break;
                  }
               case shiftright_unsigned:
                  {
                 // result = ops->shiftRight(rhs,lhs);
                    result = ops->shiftRight(lhs,rhs);
                    result->kind(lhs->kind());
                    break;
                  }

               default:
                  {
                    fprintf (stderr,"binaryResult: default reached: op_kind = %d \n",op_kind);
                    fflush(stderr);
                    ROSE_ASSERT(false);
                    break;
                  }
             }

       // Set ValueKind on result since RiscOperators don't grok ValueKind (actually this is redundant now).
       // result->kind(rhs->kind());

#if CIL_DEBUG
          std::string result_str = result->toString();
          fprintf (stderr,"binaryResult: result = %s \n",result_str.c_str());
          fflush(stderr);
#endif
          ops->pushOperand(result);
      }
#endif


void
binaryResult(D dispatcher, Ops ops, I insn, Binary_Op_Kind_Type op_kind)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);

#if CIL_DEBUG
    fprintf(stderr, "Top of binaryResult: op_kind = %s \n",op_kind_toString(op_kind).c_str());
    fflush(stderr);
#endif

    SValue::Ptr rhs = ops->popOperand(); // value2, top of stack
    SValue::Ptr lhs = ops->popOperand(); // value1

    ASSERT_not_null(lhs);
    ASSERT_not_null(rhs);

    // Shifts are special: rhs is only the shift count, and the result kind/width
    // is determined by lhs. Your existing shiftResult() already normalizes the
    // count to lhs width.
    if (isShiftOp(op_kind)) {
        SValue::Ptr result = shiftResult(ops, op_kind, lhs, rhs);
        ops->pushOperand(result);
        return;
    }

    const ValueKind resultKind = integerBinaryResultKind(lhs->kind(), rhs->kind());
    const size_t resultBits = cilStackBitsForKind(dispatcher, resultKind);

    if (resultKind == ValueKind::Unknown || resultBits == 0) {
        SValue::Ptr result = ops->undefined_(std::max(lhs->nBits(), rhs->nBits()));
        result->kind(ValueKind::Unknown);
        ops->pushOperand(result);
        return;
    }

    // Normalize both operands to the CIL stack-result width before arithmetic.
    //
    // For signed arithmetic and signed overflow checks, sign-extend.
    // For unsigned arithmetic, bitwise ops, and unsigned overflow checks, zero-extend.
    if (isSignedBinaryOp(op_kind)) {
        lhs = resizeSignedToBits(ops, lhs, resultBits);
        rhs = resizeSignedToBits(ops, rhs, resultBits);
    } else {
        lhs = resizeUnsignedToBits(ops, lhs, resultBits);
        rhs = resizeUnsignedToBits(ops, rhs, resultBits);
    }

#if CIL_DEBUG
    fprintf(stderr, "binaryResult: op_kind = %s lhs = %s rhs = %s resultBits = %zu\n",
            op_kind_toString(op_kind).c_str(),
            lhs->toString().c_str(),
            rhs->toString().c_str(),
            resultBits);
    fflush(stderr);
#endif

    // Cheap precise handling for concrete overflow cases.
    //
    // For symbolic operands, these helpers return false and we proceed with
    // ordinary symbolic arithmetic, with a comment below indicating that the
    // exceptional edge is not yet modeled.
    if (isOverflowBinaryOp(op_kind)) {
        bool overflow = false;
        bool operandsConcrete = false;

        switch (op_kind) {
            case add_with_overflow_check:
            case subtract_with_overflow_check:
            case multiply_with_overflow_check:
                overflow = concreteSignedOverflow(op_kind, lhs, rhs, resultBits);
                operandsConcrete = lhs->toSigned() && rhs->toSigned();
                break;

            case add_unsigned_with_overflow_check:
            case subtract_unsigned_with_overflow_check:
            case multiply_unsigned_with_overflow_check:
                overflow = concreteUnsignedOverflow(op_kind, lhs, rhs, resultBits);
                operandsConcrete = lhs->toUnsigned() && rhs->toUnsigned();
                break;

            default:
                break;
        }

        if (overflow) {
            overflowExceptionEffect(dispatcher, ops, insn, op_kind);
            return;
        }

        if (!operandsConcrete) {
            // Symbolic operands may overflow on some paths, but that is not a
            // definite exception on the merged normal path.  Do not write an
            // active OverflowException into the state that will also contain
            // the normal arithmetic result.
            ops->comment("CIL " + op_kind_toString(op_kind) +
                         ": symbolic checked arithmetic; overflow status unknown");
        }
    }

    SValue::Ptr result;

    switch (op_kind) {
        case add:
        case add_with_overflow_check:
        case add_unsigned_with_overflow_check:
            result = ops->add(lhs, rhs);
            break;

        case subtract:
        case subtract_with_overflow_check:
        case subtract_unsigned_with_overflow_check:
            // CIL: value1 - value2
            result = ops->subtract(lhs, rhs);
            break;

        case multiply:
        case multiply_with_overflow_check:
            result = ops->signedMultiply(lhs, rhs);
            break;

        case multiply_unsigned_with_overflow_check:
            result = ops->unsignedMultiply(lhs, rhs);
            break;

        case divide:
            // CIL: value1 / value2
            result = ops->signedDivide(lhs, rhs);
            break;

        case divide_unsigned:
            result = ops->unsignedDivide(lhs, rhs);
            break;

        case remainder:
            // CIL: value1 % value2
            result = ops->signedModulo(lhs, rhs);
            break;

        case remainder_unsigned:
            result = ops->unsignedModulo(lhs, rhs);
            break;

        case and_op:
            result = ops->and_(lhs, rhs);
            break;

        case or_op:
            result = ops->or_(lhs, rhs);
            break;

        case xor_op:
            result = ops->xor_(lhs, rhs);
            break;

        default:
            ASSERT_not_reachable("invalid CIL binary operation");
    }

    ASSERT_not_null(result);

    // Some ROSE operators, especially multiply, may return a wider result than
    // the CIL stack category. Force the stack value back to the CIL result width.
    //
    // For signed multiply, sign truncation is still low-bit extraction when
    // narrowing. For widening, signed-vs-unsigned matters; here we normally
    // expect narrowing after multiply, but handle both cases.
    if (result->nBits() < resultBits) {
        if (isSignedBinaryOp(op_kind))
            result = ops->signExtend(result, resultBits);
        else
            result = ops->unsignedExtend(result, resultBits);
    } else if (result->nBits() > resultBits) {
        result = ops->extract(result, 0, resultBits);
    }

    result->kind(resultKind);

#if CIL_DEBUG
    fprintf(stderr, "binaryResult: result = %s\n", result->toString().c_str());
    fflush(stderr);
#endif

    ops->pushOperand(result);
}



#if 0
     void compareResult(Ops ops)
        {
          ASSERT_not_null(ops);
          discard(ops, 2);
          ops->pushOperand(ops->undefined_(32));
        }
#endif
#if 0
     void convertResult(Ops ops, size_t nbits = 32)
        {
          ASSERT_not_null(ops);
          discard(ops, 1);
          ops->pushOperand(ops->undefined_(nbits));
        }

     void loadResult(Ops ops, size_t nbits)
        {
          ASSERT_not_null(ops);
          discard(ops, 1);
          ops->pushOperand(ops->undefined_(nbits));
        }
#endif

     void storeEffect(Ops ops)
        {
          ASSERT_not_null(ops);
          discard(ops, 2);
        }

#if 0
     void callEffect(Ops ops)
        {
          ASSERT_not_null(ops);
       // TODO: resolve the metadata signature, pop actual arguments, and push the return value when non-void.
          ops->pushOperand(ops->undefined_(32));
        }
#endif

     static unsigned
     cilRawOpcodeKey(SgAsmInstruction *insn)
        {
          ASSERT_not_null(insn);

          const SgUnsignedCharList &bytes = insn->get_rawBytes();
          ASSERT_require(!bytes.empty());

          const unsigned first = static_cast<unsigned>(bytes[0]);

          if (first == 0xfe)
             {
               ASSERT_require2(bytes.size() >= 2, "truncated CIL extended opcode");
               return 0xfe00u | static_cast<unsigned>(bytes[1]);
             }

          return first;
        }

     static SValue::Ptr
     cilBranchTarget(D dispatcher, Ops ops, I insn, Args args)
        {
          ASSERT_not_null(dispatcher);
          ASSERT_not_null(ops);
          ASSERT_not_null(insn);
          ASSERT_require(!args.empty());

          SgAsmIntegerValueExpression *expr = isSgAsmIntegerValueExpression(args[0]);

          ASSERT_not_null(expr);

          const uint64_t raw = expr->get_value();
       // const uint64_t fallThrough = insn->get_address() + insn->get_size();

          int64_t rel = 0;

          enum instruction_targets_type
             {
               br_s      = 0x2b, // br.s
               brfalse_s = 0x2c, // brfalse.s
               brtrue_s  = 0x2d, // brtrue.s
               beq_s     = 0x2e, // beq.s
               bge_s     = 0x2f, // bge.s
               bgt_s     = 0x30, // bgt.s
               ble_s     = 0x31, // ble.s
               blt_s     = 0x32, // blt.s
               bne_un_s  = 0x33, // bne.un.s
               bge_un_s  = 0x34, // bge.un.s
               bgt_un_s  = 0x35, // bgt.un.s
               ble_un_s  = 0x36, // ble.un.s
               blt_un_s  = 0x37, // blt.un.s
               leave_s   = 0xde // leave.s
             };

       // DQ (6/22/2026): This use of an enum avoids a compiler warning (when using hex values in the case statements).
       // instruction_targets_type instruction = (instruction_targets_type) insn->get_kind();
       // switch (instruction)
          const unsigned opcode = cilRawOpcodeKey(insn);
          instruction_targets_type instruction = (instruction_targets_type) opcode;
          switch (instruction)
             {
               case br_s: // br.s
               case brfalse_s: // brfalse.s
               case brtrue_s: // brtrue.s
               case beq_s: // beq.s
               case bge_s: // bge.s
               case bgt_s: // bgt.s
               case ble_s: // ble.s
               case blt_s: // blt.s
               case bne_un_s: // bne.un.s
               case bge_un_s: // bge.un.s
               case bgt_un_s: // bgt.un.s
               case ble_un_s: // ble.un.s
               case blt_un_s: // blt.un.s
               case leave_s: // leave.s
                  {
                    rel = static_cast<int8_t>(raw & 0xff);
                    break;
                  }

               default:
                  {
                    rel = static_cast<int32_t>(raw & 0xffffffffu);
                    break;
                  }
             }

          const int64_t targetSigned = static_cast<int64_t>(insn->get_address() + insn->get_size()) + rel;
          const RegisterDescriptor ipReg = dispatcher->instructionPointerRegister();
          return ops->number_(ipReg.nBits(), static_cast<uint64_t>(targetSigned));
        }

     void branchEffect(D dispatcher, Ops ops, I insn, Args args)
        {
          ASSERT_not_null(dispatcher);
          ASSERT_not_null(ops);
          ASSERT_not_null(insn);
          ASSERT_require(!args.empty());

          const RegisterDescriptor ipReg = dispatcher->instructionPointerRegister();
          const size_t ipBits = ipReg.nBits();

          SValue::Ptr target = CilSemantics::cilBranchTarget(dispatcher, ops, insn, args);

          if (target->nBits() < ipBits)
            {
              target = ops->unsignedExtend(target, ipBits);
            }
           else
            {
              if (target->nBits() > ipBits)
                 {
                   target = ops->extract(target, 0, ipBits);
                 }
            }

          ops->writeRegister(ipReg, target);
        }


     void returnEffect(Ops /*ops*/)
        {
       // TODO: model transfer of the optional method return value to the caller.
        }

     void throwEffect(Ops ops)
        {
          ASSERT_not_null(ops);
          discard(ops, 1);
        }

     void copyBlock(Ops ops)
        {
          ASSERT_not_null(ops);
          discard(ops, 3);     // destination, source, size
        }

     void initBlock(Ops ops)
        {
          ASSERT_not_null(ops);
          discard(ops, 3);     // address, value, size
        }

     void prefixEffect(Ops ops)
        {
          ASSERT_not_null(ops);
       // TODO: carry prefix state into interpretation of the following instruction.
        }

#if 0
  // DQ (6/2/2026): Helper function for ldelem
     void arrayLoadResult(Ops ops, size_t nbits)
        {
          ASSERT_not_null(ops);
          discard(ops, 2);       // array reference, index
          ops->pushOperand(ops->undefined_(nbits));
        }

  // DQ (6/2/2026): Helper function for stelem
     void arrayStoreEffect(Ops ops)
        {
          ASSERT_not_null(ops);
          discard(ops, 3);       // array reference, index, value
        }
#endif

     static std::string
     cilMethodFunctionPointerDescriptor(uint32_t token)
        {
          std::ostringstream ss;
          ss << "cil:method-function-pointer:0x" << std::hex << token;
          return ss.str();
        }

     static bool
     isCilMethodFunctionPointer(const SValue::Ptr &value)
        {
          ASSERT_not_null(value);

          if (!value->hasTypeDescriptor())
               return false;

          const std::string descriptor = value->typeDescriptor();
          return descriptor.find("cil:method-function-pointer:") == 0 ||
                 descriptor.find("cil:virtual-method-function-pointer:") == 0;
        }

     static void
     ldvirtftnResult(D dispatcher, Ops ops, const DispatcherCil::CilTypeAnalysis::MethodDescriptor &targetMethod)
        {
          ASSERT_not_null(dispatcher);
          ASSERT_not_null(ops);

       // Stack:
       //   ..., objref -> ..., native-function-pointer
          SValue::Ptr objref = ops->popOperand();
          ASSERT_not_null(objref);

          const size_t ptrBits = CilSemantics::cilPointerBits(dispatcher);

       // ldvirtftn performs a null check on objref.
          if (dispatcher->typeAnalysis().isDefinitelyNull(objref))
             {
            // TODO: Model NullReferenceException precisely.
            // Conservative behavior: normal control flow is no longer known.
               SValue::Ptr exceptionObject = ops->undefined_(CilSemantics::cilPointerBits(dispatcher));
               exceptionObject->kind(ValueKind::ObjectReference);
               exceptionObject->typeDescriptor("cil:exception:System.NullReferenceException");
               dispatcher->typeAnalysis().setNullness(
                   exceptionObject, DispatcherCil::CilTypeAnalysis::DefinitelyNonNull);
               ops->writeLocal(CilSemantics::cilCurrentExceptionSlot(), exceptionObject);
               CilSemantics::writeUnknownInstructionPointer(dispatcher, ops);
               return;
             }
          CilSemantics::noteMaybeNullReferenceException(dispatcher, ops, objref, "ldvirtftn");

       // Precise modeling would:
       //   1. use objref's runtime type,
       //   2. resolve the virtual target implementation,
       //   3. map the resolved method to a code address.
       //
       // For now, produce a stable method-token-derived pointer if possible.
       // This preserves the fact that the pointer corresponds to this method token.
          SValue::Ptr functionPointer;

          if (targetMethod.isValid())
             {
               functionPointer = ops->number_(ptrBits, targetMethod.metadataToken);
             }
            else
             {
               functionPointer = ops->undefined_(ptrBits);
             }

          functionPointer->kind(ValueKind::UnmanagedPointer);

          if (targetMethod.isValid())
             {
               functionPointer->typeDescriptor(CilSemantics::cilMethodFunctionPointerDescriptor(targetMethod.metadataToken));
             }
            else
             {
               functionPointer->typeDescriptor("cil:virtual-method-function-pointer:unknown");
             }

          ops->pushOperand(functionPointer);
        }

     static SValue::Ptr
     readCilLocal(D dispatcher, Ops ops, size_t localIndex)
        {
       // This version of the function checks for NULL values.
          ASSERT_not_null(ops);

       // SValue::Ptr value = ops->readLocal(localIndex);
          SValue::Ptr value = ops->readLocal(cilLocalSlot(localIndex));

          if (value == NULL)
             {
            // value = ops->undefined_(32);
               value = ops->undefined_(CilSemantics::cilPointerBits(dispatcher));
               value->kind(ValueKind::Unknown);
               value->typeDescriptor("cil:unknown-local");
             }

          return value;
        }

#if 0
     static void
     writeCilLocal(D /*dispatcher*/, Ops ops, size_t localIndex, const SValue::Ptr &value)
        {
          ASSERT_not_null(ops);
          ASSERT_not_null(value);

       // Current CIL dispatcher model:
       //   CIL local variables are represented using the ROSE semantic
       //   local-slot mechanism.
       //
       // Future metadata-aware refinements can be added here:
       //   - validate that localIndex exists in the method local signature
       //   - normalize value width to the local's declared CLI type
       //   - set/preserve ValueKind based on the local's declared type
       //   - record object-reference/nullness/type facts in CilTypeAnalysis
       //   - handle managed-pointer locals distinctly from object-reference locals
       //
       // For now, preserve the existing first-pass behavior.
       // ops->writeLocal(localIndex, value);
          ops->writeLocal(cilLocalSlot(localIndex), value);
 
       // Optional future hook:
       // if (dispatcher)
       //     dispatcher->typeAnalysis().recordLocalFacts(localIndex, value);
        }
#endif

static SValue::Ptr
localAddress(D dispatcher, Ops ops, uint32_t localIndex)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);

    const size_t ptrBits = CilSemantics::cilPointerBits(dispatcher);

    // Synthetic address range for local slots.
    const uint64_t base = 0x61000000;
    const uint64_t stride = 0x10;

    SValue::Ptr addr = ops->number_(ptrBits, base + stride * localIndex);
    addr->kind(ValueKind::ManagedPointer);
    addr->typeDescriptor("cil:local-address");

    return addr;
}

     static void
     writeCilLocal(D dispatcher, Ops ops, size_t localIndex, const SValue::Ptr &value)
        {
          ASSERT_not_null(dispatcher);
          ASSERT_not_null(ops);
          ASSERT_not_null(value);

       // 1. Authoritative CIL local slot.
          ops->writeLocal(cilLocalSlot(localIndex), value);

       // 2. Mirror into the synthetic address used by ldloca/ldind/stind.
          SValue::Ptr addr = CilSemantics::localAddress(dispatcher, ops, localIndex);
          ASSERT_not_null(addr);

          ops->writeMemory(RegisterDescriptor(), addr, value, ops->number_(1, 1));
        }

     static void
     storeCilLocal(D dispatcher, Ops ops, size_t localIndex)
        {
          ASSERT_not_null(ops);

          SValue::Ptr value = ops->popOperand();
          ASSERT_not_null(value);

          writeCilLocal(dispatcher, ops, localIndex, value);
        }

     static SValue::Ptr
     readCilArgument(D dispatcher, Ops ops, size_t argIndex)
        {
       // This version of the function checks for NULL values.
          ASSERT_not_null(ops);

       // SValue::Ptr value = ops->readLocal(argIndex);
          SValue::Ptr value = ops->readLocal(cilArgumentSlot(argIndex));

          if (value == NULL)
             {
            // value = ops->undefined_(32);
               value = ops->undefined_(CilSemantics::cilPointerBits(dispatcher));
               value->kind(ValueKind::Unknown);
               value->typeDescriptor("cil:unknown-argument");
             }

          return value;
        }

static SValue::Ptr
argumentAddress(D dispatcher, Ops ops, uint32_t argIndex)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);

    const size_t ptrBits = CilSemantics::cilPointerBits(dispatcher);

    // Synthetic address range for argument slots.
    // The exact constants are arbitrary; keep them stable and disjoint.
    const uint64_t base = 0x60000000;
    const uint64_t stride = 0x10;

    SValue::Ptr addr = ops->number_(ptrBits, base + stride * argIndex);
    addr->kind(ValueKind::ManagedPointer);
    addr->typeDescriptor("cil:argument-address");

    return addr;
}


#if 0
     static void
     writeCilArgument(D /*dispatcher*/, Ops ops, size_t argIndex, const SValue::Ptr &value)
        {
          ASSERT_not_null(ops);
          ASSERT_not_null(value);

       // Current model:
       //   CIL arguments are represented using ROSE local slots.
       //
       // Future refinements:
       //   - validate starg legality
       //   - normalize width to the declared argument type
       //   - update type-analysis facts for this argument slot
       // ops->writeLocal(argIndex, value);
          ops->writeLocal(cilArgumentSlot(argIndex), value);
        }
#else
     static void
     writeCilArgument(D dispatcher, Ops ops, size_t argIndex, const SValue::Ptr &value)
        {
          ASSERT_not_null(dispatcher);
          ASSERT_not_null(ops);
          ASSERT_not_null(value);

       // 1. Authoritative CIL argument slot.
          ops->writeLocal(cilArgumentSlot(argIndex), value);

       // 2. Mirror into the synthetic address used by ldarga/ldind/stind.
          SValue::Ptr addr = CilSemantics::argumentAddress(dispatcher, ops, argIndex);
          ASSERT_not_null(addr);

          ops->writeMemory(RegisterDescriptor(), addr, value, ops->number_(1, 1));
        }
#endif

     static void
     loadCilArgument(D dispatcher, Ops ops, size_t argIndex)
        {
          ASSERT_not_null(ops);
          ops->pushOperand(readCilArgument(dispatcher, ops, argIndex));
        }

     static void
     storeCilArgument(D dispatcher, Ops ops, size_t argIndex)
        {
          ASSERT_not_null(ops);

          SValue::Ptr value = ops->popOperand();
          ASSERT_not_null(value);

          writeCilArgument(dispatcher, ops, argIndex, value);
        }

  // DQ (6/22/2026): Added this function.
     static void
     loadCilLocal(D dispatcher, Ops ops, size_t localIndex)
        {
          ASSERT_not_null(ops);
          ops->pushOperand(readCilLocal(dispatcher, ops, localIndex));
        }
#if 0
     static SValue::Ptr
     addressOfCilLocal(D /*dispatcher*/, Ops ops, size_t /*localIndex*/, size_t ptrBits)
        {
          ASSERT_not_null(ops);

          SValue::Ptr retval = ops->undefined_(ptrBits);
          retval->kind(ValueKind::ManagedPointer);

          return retval;
        }
#endif
#if 0
     static void
     loadAddressOfCilLocal(D dispatcher, Ops ops, size_t localIndex, size_t ptrBits)
        {
          ops->pushOperand(addressOfCilLocal(dispatcher, ops, localIndex, ptrBits));
        }
#endif
#if 0
     static SValue::Ptr
     addressOfCilArgument(D /*dispatcher*/, Ops ops, size_t /*argIndex*/, size_t ptrBits)
        {
          ASSERT_not_null(ops);

          SValue::Ptr retval = ops->undefined_(ptrBits);
          retval->kind(ValueKind::ManagedPointer);

          return retval;
        }
#endif
#if 0
     static void
     loadAddressOfCilArgument(D dispatcher, Ops ops, size_t argIndex, size_t ptrBits)
        {
          ops->pushOperand(addressOfCilArgument(dispatcher, ops, argIndex, ptrBits));
        }
#endif
     static SValue::Ptr
     makeCilIntegerConstant(D /*dispatcher*/, Ops ops, int64_t value, size_t nbits)
        {
          ASSERT_not_null(ops);

          SValue::Ptr retval = ops->number_(nbits, uint64_t(value));

          if (nbits <= 32)
             {
               retval->kind(ValueKind::Integer32);
             }
            else
             {
               if (nbits == 64)
                  {
                    retval->kind(ValueKind::Integer64);
                  }
                 else
                  {
                    retval->kind(ValueKind::Unknown);
                  }
             }

          return retval;
        }

     static void
     pushCilIntegerConstant(D dispatcher, Ops ops, int64_t value, size_t nbits)
        {
          ASSERT_not_null(ops);
          ops->pushOperand(makeCilIntegerConstant(dispatcher, ops, value, nbits));
        }

     static void
     jmpMethod(D dispatcher, Ops ops, I /*insn*/, uint32_t methodToken)
        {
          ASSERT_not_null(dispatcher);
          ASSERT_not_null(ops);

       // CIL jmp has an InlineMethod operand. It transfers control to another
       // method with the same arguments as the current method. It is not an IL
       // branch to an instruction offset in the current method body.
       //
       // First-pass conservative semantics:
       //   We do not yet resolve metadata method tokens to callee entry addresses,
       //   so model this as an unknown non-returning control transfer.

       // This is deliberately not branchEffect(), because methodToken is not a
       // relative IL offset, and the token itself is not an executable address.
       // If/when method-token resolution exists, replace this with a write to PC
       // using the resolved callee entry address.
          (void)methodToken;
          CilSemantics::writeUnknownInstructionPointer(dispatcher, ops);
        }
#if 0
     static void
     popCilCallArguments(Ops ops, size_t nArgs)
        {
          ASSERT_not_null(ops);

          for (size_t i = 0; i < nArgs; ++i)
             {
#if 0
               ops->popOperand();
#else
               SValue::Ptr functionPointer = ops->popOperand();
               ASSERT_not_null(functionPointer);

               if (!sig.signatureKnown)
                  {
                    throw BaseSemantics::Exception("CIL calli signature is required", insn);
                  }
#endif
             }
        }
#else
static void
popCilCallArguments(Ops ops, size_t nArgs)
{
    ASSERT_not_null(ops);

    for (size_t i = 0; i < nArgs; ++i) {
        SValue::Ptr arg = ops->popOperand();
        ASSERT_not_null(arg);
    }
}
#endif

static size_t
cilCallReturnBits(D dispatcher, size_t metadataBits)
{
    ASSERT_not_null(dispatcher);

    return metadataBits != 0 ? metadataBits : CilSemantics::cilPointerBits(dispatcher);
}

static void
markCallReturnFacts(D dispatcher, const SValue::Ptr &retval, ValueKind kind)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(retval);

    if (kind == ValueKind::ObjectReference || kind == ValueKind::ArrayReference)
        dispatcher->typeAnalysis().setNullness(retval, DispatcherCil::CilTypeAnalysis::MaybeNull);
}

static SValue::Ptr
methodReturnValue(D dispatcher, Ops ops, const DispatcherCil::CilTypeAnalysis::MethodDescriptor &method)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);

    const size_t nbits = CilSemantics::cilCallReturnBits(dispatcher, method.returnBits);
    SValue::Ptr retval = ops->undefined_(nbits);

    if (method.returnKind != BaseSemantics::ValueKind::Unknown)
        retval->kind(method.returnKind);

    if (!method.returnTypeDescriptor.empty())
        retval->typeDescriptor(method.returnTypeDescriptor);

    CilSemantics::markCallReturnFacts(dispatcher, retval, method.returnKind);
    return retval;
}

static bool
methodHasStackReceiver(const DispatcherCil::CilTypeAnalysis::MethodDescriptor &method)
{
    // CLI instance-call signatures can represent the receiver with hasThis.
    // explicitThis is also a stack receiver; it means the this parameter is
    // explicitly represented by the call-site signature rather than implied by
    // metadata.  Both forms consume one value below the explicit arguments.
    return method.hasThis || method.explicitThis;
}

static SValue::Ptr
normalizeCurrentMethodReturnValue(D dispatcher,
                                  Ops ops,
                                  const DispatcherCil::CilTypeAnalysis::CurrentMethodDescriptor &method,
                                  const SValue::Ptr &value)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);
    ASSERT_not_null(value);

    const size_t returnBits = CilSemantics::cilCallReturnBits(dispatcher, method.returnBits);
    SValue::Ptr result = value;

    if (result->nBits() < returnBits)
        result = ops->unsignedExtend(result, returnBits);
    else if (result->nBits() > returnBits)
        result = ops->extract(result, 0, returnBits);

    if (result != value) {
        result->kind(value->kind());

        if (value->hasTypeDescriptor())
            result->typeDescriptor(value->typeDescriptor());

        dispatcher->typeAnalysis().copyFacts(result, value);
    }

    if (method.returnKind != ValueKind::Unknown)
        result->kind(method.returnKind);

    if (!method.returnTypeDescriptor.empty())
        result->typeDescriptor(method.returnTypeDescriptor);

    CilSemantics::markCallReturnFacts(dispatcher, result, result->kind());
    return result;
}

static SValue::Ptr
preserveCurrentMethodReturnValue(D dispatcher,
                                 Ops ops,
                                 const DispatcherCil::CilTypeAnalysis::CurrentMethodDescriptor &method,
                                 const SValue::Ptr &value)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);
    ASSERT_not_null(value);

    SValue::Ptr normalized =
        CilSemantics::normalizeCurrentMethodReturnValue(dispatcher, ops, method, value);

    ops->writeLocal(CilSemantics::cilReturnValueSlot(), normalized);

    return normalized;
}

     static void
     callMethod(D dispatcher,
                Ops ops,
                I /*insn*/,
                const DispatcherCil::CilTypeAnalysis::MethodDescriptor &method,
                bool isCallVirtInstruction,
                bool isTailCall = false,
                const DispatcherCil::CilTypeAnalysis::TypeDescriptor *constrainedReceiverType = nullptr)
        {
          ASSERT_not_null(dispatcher);
          ASSERT_not_null(ops);

          if (!method.signatureKnown)
             {
               ops->comment("CIL call/callvirt: method signature unknown; stack-preserving byte-only fallback semantics used");
               if (isTailCall)
                  {
                    ops->comment("CIL tail. prefix on unknown-signature call: stack effect unknown, return-transfer modeled with unknown continuation");
                    CilSemantics::writeUnknownInstructionPointer(dispatcher, ops);
                  }

            // In byte-only mode we cannot know how many arguments to pop, whether
            // there is an instance receiver, or whether a return value is pushed.
            //
            // Conservative choice for raw instruction tests: preserve the stack
            // exactly rather than guessing. Popping an unknown number of
            // arguments or pushing a guessed return value silently corrupts the
            // evaluation stack when the real signature differs.
               return;
             }

          DispatcherCil::CilTypeAnalysis &types = dispatcher->typeAnalysis();

       // Stack before an instance call:
       //
       //   ..., this, arg0, arg1, ..., argN
       //
       // The last explicit argument is on top, so pop explicit arguments first.
          for (size_t i = 0; i < method.parameterCount; ++i)
               ops->popOperand();

          if (CilSemantics::methodHasStackReceiver(method))
             {
               SValue::Ptr thisObj = ops->popOperand();
               ASSERT_not_null(thisObj);

               if (isCallVirtInstruction)
                  {
                    if (constrainedReceiverType != nullptr &&
                        constrainedReceiverType->isKnownValueType())
                       {
                      // constrained. callvirt on a known value type consumes a
                      // managed pointer to the unboxed value.  Do not treat that
                      // address as an object reference and do not apply the
                      // normal object-reference nullness refinement to it.
                         thisObj->kind(ValueKind::ManagedPointer);
                         if (constrainedReceiverType->metadataToken != 0)
                            {
                              thisObj->typeDescriptor("cil:constrained-this-address:" +
                                  CilSemantics::cilTypeTokenDescriptor(constrainedReceiverType->metadataToken));
                            }

                         ops->comment("CIL constrained. callvirt on value type: receiver is managed pointer; exact dispatch/boxing not fully modeled");
                       }
                    else
                       {
                 // callvirt performs a null check on the receiver before normal
                 // virtual dispatch.  A definitely-null receiver transfers to
                 // the NullReferenceException path, so do not first overwrite
                 // that fact with DefinitelyNonNull.
                         if (types.isDefinitelyNull(thisObj))
                            {
                              SValue::Ptr exceptionObject = ops->undefined_(CilSemantics::cilPointerBits(dispatcher));
                              exceptionObject->kind(ValueKind::ObjectReference);
                              exceptionObject->typeDescriptor("cil:exception:System.NullReferenceException");
                              dispatcher->typeAnalysis().setNullness(
                                  exceptionObject, DispatcherCil::CilTypeAnalysis::DefinitelyNonNull);
                              ops->writeLocal(CilSemantics::cilCurrentExceptionSlot(), exceptionObject);
                              CilSemantics::writeUnknownInstructionPointer(dispatcher, ops);
                              return;
                            }
                         CilSemantics::noteMaybeNullReferenceException(dispatcher, ops, thisObj, "callvirt");

                 // On the normal fall-through path, the receiver passed the null
                 // check and is known to be non-null.
                         types.setNullness(thisObj,DispatcherCil::CilTypeAnalysis::DefinitelyNonNull);
                       }
                  }
             }

          if (!method.returnsVoid)
             {
               SValue::Ptr retval = CilSemantics::methodReturnValue(dispatcher, ops, method);

               if (isTailCall)
                  {
                    const auto &cm = dispatcher->currentMethod();
                    if (!cm.returnsVoid)
                       {
                         CilSemantics::preserveCurrentMethodReturnValue(dispatcher, ops, cm, retval);
                       }
                  }
                 else
                  {
                    ops->pushOperand(retval);
                  }
             }

          if (isTailCall)
             {
               ops->comment("CIL tail. call: modeled as return transfer with unknown caller continuation");
               CilSemantics::writeUnknownInstructionPointer(dispatcher, ops);
             }
        }

     static void
     callIndirect(D dispatcher, Ops ops, I /*insn*/, const DispatcherCil::CilTypeAnalysis::CallSiteDescriptor &sig, bool isTailCall = false)
        {
          ASSERT_not_null(dispatcher);
          ASSERT_not_null(ops);

          if (!sig.signatureKnown)
             {
            // In byte-only mode there is no standalone-signature metadata, so
            // the dispatcher cannot know how many arguments precede the function
            // pointer or whether the call returns a value. Keep this fallback
            // explicitly stack-preserving instead of guessing.
               ops->comment("CIL calli: call-site signature unknown; stack-preserving byte-only fallback semantics used");
               if (isTailCall)
                  {
                    ops->comment("CIL tail. prefix on unknown-signature calli: stack effect unknown, return-transfer modeled with unknown continuation");
                    CilSemantics::writeUnknownInstructionPointer(dispatcher, ops);
                  }
               return;
             }

       // calli consumes call arguments and a function pointer from the stack.
       // The operand describes the call-site signature, not the callee method.

          SValue::Ptr functionPointer = ops->popOperand();
          ASSERT_not_null(functionPointer);

          popCilCallArguments(ops, sig.stackArgumentCount());

          Sawyer::Optional<uint64_t> concreteFunctionPointer = functionPointer->toUnsigned();
          if (concreteFunctionPointer && *concreteFunctionPointer == 0) {
              ops->comment("CIL calli: null function pointer; NullReferenceException path not fully modeled");

              SValue::Ptr exceptionObject = ops->undefined_(CilSemantics::cilPointerBits(dispatcher));
              exceptionObject->kind(ValueKind::ObjectReference);
              exceptionObject->typeDescriptor("cil:exception:System.NullReferenceException");
              dispatcher->typeAnalysis().setNullness(
                  exceptionObject, DispatcherCil::CilTypeAnalysis::DefinitelyNonNull);
              ops->writeLocal(CilSemantics::cilCurrentExceptionSlot(), exceptionObject);

              CilSemantics::writeUnknownInstructionPointer(dispatcher, ops);
              return;
          }

          if (!concreteFunctionPointer) {
              // A symbolic function pointer is not definitely invalid: in a
              // concrete execution it may denote a valid unmanaged target.
              // Therefore do not write an active InvalidProgramException into
              // the merged normal state.  Model the normal stack effect below
              // and leave exact target validation to a domain with enough
              // pointer provenance/metadata information.
              ops->comment("CIL calli: symbolic function pointer; target validity unknown");
          }
          else if (!CilSemantics::isCilMethodFunctionPointer(functionPointer)) {
              // Concrete nonzero does not necessarily mean "known managed
              // method target". It may be an unmanaged/native address or an
              // arbitrary integer from a concrete domain. Keep the call stack
              // effect conservative, but do not treat the integer itself as a
              // resolved CIL method address.
              ops->comment("CIL calli: concrete nonzero target is not a known CIL method function pointer; modeled as unresolved external call");
          }

          if (!sig.returnsVoid)
             {
               SValue::Ptr retval = ops->undefined_(CilSemantics::cilCallReturnBits(dispatcher, sig.returnBits));
               retval->kind(ValueKind::Unknown);
               if (isTailCall)
                  {
                    const auto &cm = dispatcher->currentMethod();
                    if (!cm.returnsVoid)
                       {
                         CilSemantics::preserveCurrentMethodReturnValue(dispatcher, ops, cm, retval);
                       }
                  }
                 else
                  {
                    ops->pushOperand(retval);
                  }
             }

          if (isTailCall)
             {
               ops->comment("CIL tail. calli: modeled as return transfer with unknown caller continuation");
               CilSemantics::writeUnknownInstructionPointer(dispatcher, ops);
             }
        }

  // Skipping the indentation convention

static void
conditionalBranchEffect(D dispatcher, Ops ops, I insn, Args args, const SValue::Ptr &cond)
   {
     ASSERT_not_null(dispatcher);
     ASSERT_not_null(ops);
     ASSERT_not_null(cond);
  // assert_args(insn, args, 1);

  // const size_t pcBits = 32;

  // SValue::Ptr branchTarget = CilSemantics::branchTarget(dispatcher, ops, insn, args);
     SValue::Ptr branchTarget = CilSemantics::cilBranchTarget(dispatcher, ops, insn, args);

  // SValue::Ptr fallThrough = ops->number_(pcBits, insn->get_address() + insn->get_size());
     const RegisterDescriptor ipReg = dispatcher->instructionPointerRegister();
     const size_t pcBits = ipReg.nBits();
     SValue::Ptr fallThrough = ops->number_(pcBits, insn->get_address() + insn->get_size());

     SValue::Ptr nextPc = ops->ite(cond, branchTarget, fallThrough);

     ops->writeRegister(dispatcher->instructionPointerRegister(), nextPc);
   }

static bool
isReferenceLike(ValueKind k)
   {
     return k == ValueKind::ObjectReference ||
            k == ValueKind::ArrayReference ||
            k == ValueKind::ManagedPointer ||
            k == ValueKind::UnmanagedPointer;
   }

static bool
isIntegerLike(ValueKind k)
   {
     return k == ValueKind::Integer32 ||
            k == ValueKind::Integer64 ||
            k == ValueKind::NativeInt;
   }


// DQ (6/11/2026): Better implementation using SValue support.
static void
branchFalseEffect(D dispatcher, Ops ops, I insn, Args args)
   {
     ASSERT_not_null(dispatcher);
     ASSERT_not_null(ops);
  // assert_args(insn, args, 1);

     SValue::Ptr value = ops->popOperand();
     ASSERT_not_null(value);

     ValueKind kind = value->kind();

     if (isIntegerLike(kind) || isReferenceLike(kind))
        {
          SValue::Ptr cond = ops->equalToZero(value); // value == 0/null
          CilSemantics::conditionalBranchEffect(dispatcher, ops, insn, args, cond);
        }
       else
        {
          SValue::Ptr cond = ops->undefined_(1);
          CilSemantics::conditionalBranchEffect(dispatcher, ops, insn, args, cond);
        }
   }


// DQ (6/11/2026): Better implementation using SValue support.
static void
branchTrueEffect(D dispatcher, Ops ops, I insn, Args args)
   {
     ASSERT_not_null(dispatcher);
     ASSERT_not_null(ops);
  // assert_args(insn, args, 1);

     SValue::Ptr value = ops->popOperand();
     ASSERT_not_null(value);

     ValueKind kind = value->kind();

     if (isIntegerLike(kind) || isReferenceLike(kind))
        {
          SValue::Ptr isZero = ops->equalToZero(value);
          SValue::Ptr cond = ops->invert(isZero); // value != 0
          CilSemantics::conditionalBranchEffect(dispatcher, ops, insn, args, cond);
        }
       else
        {
       // Float/Unknown/typedref/etc. Conservative fallback.
          SValue::Ptr cond = ops->undefined_(1);
          CilSemantics::conditionalBranchEffect(dispatcher, ops, insn, args, cond);
        }
   }


static void
compareBranchEffect(D dispatcher,
                    Ops ops,
                    I insn,
                    Args args,
                    Branch_Compare_Op_Kind_Type kind)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);
    ASSERT_not_null(insn);

    // Stack:
    //
    //   ..., value1, value2 -> ...
    //
    // value2 is on top of the stack.
    //
    // Branch condition:
    //
    //   value1 <op> value2

    SValue::Ptr rhs = ops->popOperand(); // value2
    SValue::Ptr lhs = ops->popOperand(); // value1

    ASSERT_not_null(lhs);
    ASSERT_not_null(rhs);

    const ValueKind lhsKind = lhs->kind();
    const ValueKind rhsKind = rhs->kind();

    SValue::Ptr cond = ops->undefined_(1);

    // Helper: normalize two bit-vector operands to the same width for equality
    // and unsigned comparisons. Zero-extension is appropriate for equality
    // because equality is bitwise after CIL stack normalization, and it is also
    // appropriate for unsigned comparisons.
    auto normalizeUnsignedPair =
        [&ops](SValue::Ptr &a, SValue::Ptr &b) {
            ASSERT_not_null(a);
            ASSERT_not_null(b);

            const size_t width = std::max(a->nBits(), b->nBits());

            if (a->nBits() < width) {
                a = ops->unsignedExtend(a, width);
            } else if (a->nBits() > width) {
                a = ops->extract(a, 0, width);
            }

            if (b->nBits() < width) {
                b = ops->unsignedExtend(b, width);
            } else if (b->nBits() > width) {
                b = ops->extract(b, 0, width);
            }
        };

    // Helper: normalize two integer operands to the same width for signed
    // comparisons. Sign-extension is used when widening.
    auto normalizeSignedPair =
        [&ops](SValue::Ptr &a, SValue::Ptr &b) {
            ASSERT_not_null(a);
            ASSERT_not_null(b);

            const size_t width = std::max(a->nBits(), b->nBits());

            if (a->nBits() < width) {
                a = ops->signExtend(a, width);
            } else if (a->nBits() > width) {
                a = ops->extract(a, 0, width);
            }

            if (b->nBits() < width) {
                b = ops->signExtend(b, width);
            } else if (b->nBits() > width) {
                b = ops->extract(b, 0, width);
            }
        };

    switch (kind) {
        case branch_eq: {
            if ((isIntegerLike(lhsKind) && isIntegerLike(rhsKind)) ||
                (isReferenceLike(lhsKind) && isReferenceLike(rhsKind))) {
                SValue::Ptr a = lhs;
                SValue::Ptr b = rhs;
                normalizeUnsignedPair(a, b);

                cond = ops->isEqual(a, b);
            } else if (isFloatLike(lhsKind) && isFloatLike(rhsKind)) {
                Sawyer::Optional<bool> concrete =
                    CilSemantics::concreteFloatComparison(lhs, rhs, float_compare_eq);
                cond = concrete ? CilSemantics::concreteBoolean(ops, *concrete) : ops->undefined_(1);
            }
            break;
        }

        case branch_ne_un: {
            if ((isIntegerLike(lhsKind) && isIntegerLike(rhsKind)) ||
                (isReferenceLike(lhsKind) && isReferenceLike(rhsKind))) {
                SValue::Ptr a = lhs;
                SValue::Ptr b = rhs;
                normalizeUnsignedPair(a, b);

                cond = ops->isNotEqual(a, b);
            } else {
                // For floating-point bne.un, CLI semantics are unordered-aware:
                // the branch is taken if the values are unordered or not equal.
                Sawyer::Optional<bool> concrete =
                    CilSemantics::concreteFloatComparison(lhs, rhs, float_compare_ne_un);
                cond = concrete ? CilSemantics::concreteBoolean(ops, *concrete) : ops->undefined_(1);
            }
            break;
        }

        case branch_ge:
        case branch_gt:
        case branch_le:
        case branch_lt: {
            if (isIntegerLike(lhsKind) && isIntegerLike(rhsKind)) {
                SValue::Ptr a = lhs;
                SValue::Ptr b = rhs;
                normalizeSignedPair(a, b);

                switch (kind) {
                    case branch_ge:
                        cond = ops->isSignedGreaterThanOrEqual(a, b);
                        break;

                    case branch_gt:
                        cond = ops->isSignedGreaterThan(a, b);
                        break;

                    case branch_le:
                        cond = ops->isSignedLessThanOrEqual(a, b);
                        break;

                    case branch_lt:
                        cond = ops->isSignedLessThan(a, b);
                        break;

                    default:
                        ASSERT_not_reachable("invalid signed CIL branch comparison kind");
                }
            } else if (isFloatLike(lhsKind) && isFloatLike(rhsKind)) {
                Float_Compare_Op_Kind_Type floatKind = float_compare_gt;
                switch (kind) {
                    case branch_ge:
                        floatKind = float_compare_ge;
                        break;

                    case branch_gt:
                        floatKind = float_compare_gt;
                        break;

                    case branch_le:
                        floatKind = float_compare_le;
                        break;

                    case branch_lt:
                        floatKind = float_compare_lt;
                        break;

                    default:
                        ASSERT_not_reachable("invalid signed CIL branch comparison kind");
                }

                Sawyer::Optional<bool> concrete =
                    CilSemantics::concreteFloatComparison(lhs, rhs, floatKind);
                cond = concrete ? CilSemantics::concreteBoolean(ops, *concrete) : ops->undefined_(1);
            } else {
                // Floating-point ordered comparisons require float-aware
                // semantics.  Symbolic float values remain conservative.
                cond = ops->undefined_(1);
            }
            break;
        }

        case branch_ge_un:
        case branch_gt_un:
        case branch_le_un:
        case branch_lt_un: {
            if (isIntegerLike(lhsKind) && isIntegerLike(rhsKind)) {
                SValue::Ptr a = lhs;
                SValue::Ptr b = rhs;
                normalizeUnsignedPair(a, b);

                switch (kind) {
                    case branch_ge_un:
                        cond = ops->isUnsignedGreaterThanOrEqual(a, b);
                        break;

                    case branch_gt_un:
                        cond = ops->isUnsignedGreaterThan(a, b);
                        break;

                    case branch_le_un:
                        cond = ops->isUnsignedLessThanOrEqual(a, b);
                        break;

                    case branch_lt_un:
                        cond = ops->isUnsignedLessThan(a, b);
                        break;

                    default:
                        ASSERT_not_reachable("invalid unsigned CIL branch comparison kind");
                }
            } else if (isFloatLike(lhsKind) && isFloatLike(rhsKind)) {
                Float_Compare_Op_Kind_Type floatKind = float_compare_gt_un;
                switch (kind) {
                    case branch_ge_un:
                        floatKind = float_compare_ge_un;
                        break;

                    case branch_gt_un:
                        floatKind = float_compare_gt_un;
                        break;

                    case branch_le_un:
                        floatKind = float_compare_le_un;
                        break;

                    case branch_lt_un:
                        floatKind = float_compare_lt_un;
                        break;

                    default:
                        ASSERT_not_reachable("invalid unsigned CIL branch comparison kind");
                }

                Sawyer::Optional<bool> concrete =
                    CilSemantics::concreteFloatComparison(lhs, rhs, floatKind);
                cond = concrete ? CilSemantics::concreteBoolean(ops, *concrete) : ops->undefined_(1);
            } else {
                // Floating-point .un branches have unordered NaN-aware
                // semantics.  Symbolic float values remain conservative.
                cond = ops->undefined_(1);
            }
            break;
        }

        default:
            ASSERT_not_reachable("invalid CIL branch comparison kind");
    }

    CilSemantics::conditionalBranchEffect(dispatcher, ops, insn, args, cond);
}

     static void
     noteMemoryPrefixSemantics(D dispatcher, Ops ops, const char *operationName)
        {
       // DQ (6/27/2026): Added support to handle attributes.
          const auto &prefix = dispatcher->pendingPrefix();

          if (prefix.isVolatile)
             {
               ops->comment(std::string("CIL volatile. prefix on ") + operationName + ": access has volatile memory-ordering semantics");
             }

          if (prefix.hasUnaligned)
             {
               switch (prefix.unalignedBytes)
                  {
                    case 1:
                    case 2:
                    case 4:
                         break;
                    default:
                         ops->comment("CIL unaligned. prefix has unexpected alignment operand");
                         break;
                  }

               ops->comment(std::string("CIL unaligned. prefix on ") + operationName + ": access may be unaligned");
             }
        }

     static void
     noteCallPrefixSemantics(D dispatcher, Ops ops, const char *operationName)
        {
          ASSERT_not_null(dispatcher);
          ASSERT_not_null(ops);

          const auto &prefix = dispatcher->pendingPrefix();

          if (prefix.isTail)
             {
               ops->comment(std::string("CIL tail. prefix on ") + operationName + ": tail-call return transfer modeled conservatively");
             }

          if (prefix.hasConstrained)
             {
               ops->comment(std::string("CIL constrained. prefix on ") + operationName + ": constrained virtual dispatch not fully modeled");
             }
        }

static SValue::Ptr
readAddressOrCilSlot(D dispatcher,
                     Ops ops,
                     const SValue::Ptr &addr,
                     size_t loadBits);

static void
loadIndirectResult(D dispatcher, Ops ops, Indirect_Load_Kind_Type kind)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);

 // DQ (6/27/2026): Added support to handle attributes.
    noteMemoryPrefixSemantics(dispatcher, ops, "ldind");

    // Stack:
    //   ..., address -> ..., value
    SValue::Ptr addr = ops->popOperand();
    ASSERT_not_null(addr);

    // Normalize address width to native pointer width.
    addr = CilSemantics::normalizeToPointerBits(dispatcher, ops, addr);

    const size_t ptrBits = CilSemantics::cilPointerBits(dispatcher);

    size_t loadBits = 0;
    size_t resultBits = 0;
    bool signExtendResult = false;
    bool zeroExtendResult = false;
    ValueKind resultKind = ValueKind::Unknown;

    switch (kind) {
        case ldind_i1_kind:
            loadBits = 8;
            resultBits = 32;
            signExtendResult = true;
            resultKind = ValueKind::Integer32;
            break;

        case ldind_u1_kind:
            loadBits = 8;
            resultBits = 32;
            zeroExtendResult = true;
            resultKind = ValueKind::Integer32;
            break;

        case ldind_i2_kind:
            loadBits = 16;
            resultBits = 32;
            signExtendResult = true;
            resultKind = ValueKind::Integer32;
            break;

        case ldind_u2_kind:
            loadBits = 16;
            resultBits = 32;
            zeroExtendResult = true;
            resultKind = ValueKind::Integer32;
            break;

        case ldind_i4_kind:
            loadBits = 32;
            resultBits = 32;
            resultKind = ValueKind::Integer32;
            break;

        case ldind_u4_kind:
            // CLI loads a 4-byte unsigned integer, but the stack category is
            // still int32. Keep the ValueKind as Integer32.
            loadBits = 32;
            resultBits = 32;
            resultKind = ValueKind::Integer32;
            break;

        case ldind_i8_kind:
            loadBits = 64;
            resultBits = 64;
            resultKind = ValueKind::Integer64;
            break;

        case ldind_i_kind:
            loadBits = ptrBits;
            resultBits = ptrBits;
            resultKind = ValueKind::NativeInt;
            break;

        case ldind_r4_kind:
            loadBits = 32;
            resultBits = 32;
            resultKind = ValueKind::Float32;
            break;

        case ldind_r8_kind:
            loadBits = 64;
            resultBits = 64;
            resultKind = ValueKind::Float64;
            break;

        case ldind_ref_kind:
            loadBits = ptrBits;
            resultBits = ptrBits;
            resultKind = ValueKind::ObjectReference;
            break;

        default:
            ASSERT_not_reachable("invalid CIL indirect load kind");
    }

    // Reads through ldloca/ldarga synthetic addresses must observe the
    // authoritative CIL local/argument slots. Normal addresses still perform a
    // single flat-memory read.
    SValue::Ptr loaded = CilSemantics::readAddressOrCilSlot(dispatcher, ops, addr, loadBits);
    ASSERT_not_null(loaded);

    if (loaded->nBits() < resultBits) {
        if (signExtendResult) {
            loaded = ops->signExtend(loaded, resultBits);
        } else if (zeroExtendResult) {
            loaded = ops->unsignedExtend(loaded, resultBits);
        } else {
            loaded = ops->unsignedExtend(loaded, resultBits);
        }
    } else if (loaded->nBits() > resultBits) {
        loaded = ops->extract(loaded, 0, resultBits);
    }

    loaded->kind(resultKind);

    // For reference loads, the value may be null or non-null. Unless you have
    // stronger memory facts, treat it as maybe-null.
    if (kind == ldind_ref_kind) {
        dispatcher->typeAnalysis().setNullness(
            loaded, DispatcherCil::CilTypeAnalysis::MaybeNull);
    }

    ops->pushOperand(loaded);
}

static SValue::Ptr
resizeForStore(Ops ops, const SValue::Ptr &value, size_t storeBits)
{
    ASSERT_not_null(ops);
    ASSERT_not_null(value);

    if (value->nBits() < storeBits)
        return ops->unsignedExtend(value, storeBits);

    if (value->nBits() > storeBits)
        return ops->extract(value, 0, storeBits);

    return value;
}

static bool
isReadonlyManagedPointer(const SValue::Ptr &value)
{
    ASSERT_not_null(value);

    return value->kind() == ValueKind::ManagedPointer &&
           value->hasTypeDescriptor() &&
           value->typeDescriptor().find(":readonly") != std::string::npos;
}

static void
markReadonlyManagedPointer(const SValue::Ptr &value)
{
    ASSERT_not_null(value);

    std::string descriptor = value->hasTypeDescriptor() ?
                             value->typeDescriptor() :
                             std::string("cil:managed-pointer");

    if (descriptor.find(":readonly") == std::string::npos)
        descriptor += ":readonly";

    value->typeDescriptor(descriptor);
}

#if 0
static bool
tryDecodeLocalAddress(const SValue::Ptr &addr, uint32_t &localIndex)
{
    ASSERT_not_null(addr);

    Sawyer::Optional<uint64_t> concrete = addr->toUnsigned();
    if (!concrete)
        return false;

    static const uint64_t base = 0x61000000;
    static const uint64_t stride = 0x10;

    if (*concrete < base)
        return false;

    const uint64_t delta = *concrete - base;

    if (delta % stride != 0)
        return false;

    localIndex = static_cast<uint32_t>(delta / stride);
    return true;
}
#else
static bool
tryDecodeLocalAddress(const SValue::Ptr &addr, uint32_t &localIndex)
{
    ASSERT_not_null(addr);

    if (addr->kind() != ValueKind::ManagedPointer ||
        !addr->hasTypeDescriptor() ||
        addr->typeDescriptor() != "cil:local-address") {
        return false;
    }

    Sawyer::Optional<uint64_t> concrete = addr->toUnsigned();
    if (!concrete)
        return false;

    static const uint64_t base = 0x61000000;
    static const uint64_t stride = 0x10;

    if (*concrete < base)
        return false;

    const uint64_t delta = *concrete - base;
    if (delta % stride != 0)
        return false;

    localIndex = static_cast<uint32_t>(delta / stride);
    return true;
}
#endif

#if 0
static bool
tryDecodeArgumentAddress(const SValue::Ptr &addr, uint32_t &argIndex)
{
    ASSERT_not_null(addr);

    Sawyer::Optional<uint64_t> concrete = addr->toUnsigned();
    if (!concrete)
        return false;

    static const uint64_t base = 0x60000000;
    static const uint64_t stride = 0x10;

    if (*concrete < base)
        return false;

    const uint64_t delta = *concrete - base;

    if (delta % stride != 0)
        return false;

    argIndex = static_cast<uint32_t>(delta / stride);
    return true;
}
#else
static bool
tryDecodeArgumentAddress(const SValue::Ptr &addr, uint32_t &argIndex)
{
    ASSERT_not_null(addr);

    if (addr->kind() != ValueKind::ManagedPointer ||
        !addr->hasTypeDescriptor() ||
        addr->typeDescriptor() != "cil:argument-address") {
        return false;
    }

    Sawyer::Optional<uint64_t> concrete = addr->toUnsigned();
    if (!concrete)
        return false;

    static const uint64_t base = 0x60000000;
    static const uint64_t stride = 0x10;

    if (*concrete < base)
        return false;

    const uint64_t delta = *concrete - base;
    if (delta % stride != 0)
        return false;

    argIndex = static_cast<uint32_t>(delta / stride);
    return true;
}
#endif

static SValue::Ptr
mergeLowStoreIntoSlot(Ops ops,
                      const SValue::Ptr &oldSlot,
                      const SValue::Ptr &storedValue,
                      size_t storeBits)
{
    ASSERT_not_null(ops);
    ASSERT_not_null(storedValue);
    ASSERT_require(storeBits > 0);

    SValue::Ptr low = resizeForStore(ops, storedValue, storeBits);

    // No previous slot value, or the store is at least as wide as the slot:
    // replace the slot.
    if (oldSlot == NULL || oldSlot->nBits() <= storeBits)
        return low;

    // Partial-width store through ldloca/ldarga.
    //
    // Example:
    //   ldloca.s 0
    //   ldc.i4 ...
    //   stind.i1
    //
    // If local 0 is currently 32 bits, stind.i1 should update only the low
    // 8 bits and preserve the high 24 bits.
    SValue::Ptr high = ops->extract(oldSlot, storeBits, oldSlot->nBits());

    // Assumes ROSE concat(high, low) places the first operand in the high bits.
    return ops->concat(high, low);
}


static void
writeAddressOrCilSlot(D dispatcher,
                      Ops ops,
                      const SValue::Ptr &addr,
                      const SValue::Ptr &storedValue,
                      size_t storeBits)
{
 // DQ (6/29/2026): This is refactored function used within both storeIndirectEffect and storeObjectEffect.

    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);
    ASSERT_not_null(addr);
    ASSERT_not_null(storedValue);
    ASSERT_require(storeBits > 0);

    // readonly. on ldelema produces a readonly managed pointer. All stores
    // through the common address-write helper observe that marker, not just
    // stind.*, so stobj/initobj/cpobj-style writes do not bypass it.
    if (CilSemantics::isReadonlyManagedPointer(addr)) {
        ops->comment("CIL write through readonly. managed pointer: store suppressed in conservative model");
        return;
    }

    uint32_t localIndex = 0;
    uint32_t argIndex = 0;

    if (tryDecodeLocalAddress(addr, localIndex)) {
        // For addresses produced by ldloca/ldloca.s:
        // Update the authoritative CIL local slot exactly once, relying on
        // writeCilLocal to mirror into synthetic memory.
        SValue::Ptr oldSlot = ops->readLocal(cilLocalSlot(localIndex));
        SValue::Ptr newSlot =
            mergeLowStoreIntoSlot(ops, oldSlot, storedValue, storeBits);

        writeCilLocal(dispatcher, ops, localIndex, newSlot);
        return;
    }

    if (tryDecodeArgumentAddress(addr, argIndex)) {
        // For addresses produced by ldarga/ldarga.s:
        // Update the authoritative CIL argument slot exactly once, relying on
        // writeCilArgument to mirror into synthetic memory.
        SValue::Ptr oldSlot = ops->readLocal(cilArgumentSlot(argIndex));
        SValue::Ptr newSlot =
            mergeLowStoreIntoSlot(ops, oldSlot, storedValue, storeBits);

        writeCilArgument(dispatcher, ops, argIndex, newSlot);
        return;
    }

    // Normal address: perform exactly one direct memory write.
    ops->writeMemory(RegisterDescriptor(),
                     addr,
                     storedValue,
                     ops->number_(1, 1));
}

static SValue::Ptr
readAddressOrCilSlot(D dispatcher,
                     Ops ops,
                     const SValue::Ptr &addr,
                     size_t loadBits)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);
    ASSERT_not_null(addr);
    ASSERT_require(loadBits > 0);

    SValue::Ptr result;
    uint32_t localIndex = 0;
    uint32_t argIndex = 0;

    if (tryDecodeLocalAddress(addr, localIndex)) {
        result = ops->readLocal(cilLocalSlot(localIndex));
    } else if (tryDecodeArgumentAddress(addr, argIndex)) {
        result = ops->readLocal(cilArgumentSlot(argIndex));
    } else {
        result = ops->readMemory(RegisterDescriptor(),
                                 addr,
                                 ops->undefined_(loadBits),
                                 ops->number_(1, 1));
    }

    if (result == NULL)
        result = ops->undefined_(loadBits);

    if (result->nBits() < loadBits)
        result = ops->unsignedExtend(result, loadBits);
    else if (result->nBits() > loadBits)
        result = ops->extract(result, 0, loadBits);

    return result;
}

static size_t
objectStorageBits(D dispatcher, const DispatcherCil::CilTypeAnalysis::TypeDescriptor &type)
{
    ASSERT_not_null(dispatcher);

    if (type.isKnownReferenceType())
        return CilSemantics::cilPointerBits(dispatcher);

    if (type.hasKnownValueWidth())
        return type.valueBitWidth;

    // Metadata/layout unavailable.  Use the same conservative width for
    // ldobj, stobj, and cpobj so unknown typed-object operations round-trip
    // consistently instead of depending on the incidental stack value width.
    return 32;
}

static size_t
objectStackBits(D dispatcher, const DispatcherCil::CilTypeAnalysis::TypeDescriptor &type)
{
    const size_t storageBits = CilSemantics::objectStorageBits(dispatcher, type);

    if (type.isKnownReferenceType())
        return storageBits;

    // CLI evaluation stack normalizes small value-type scalars to int32.
    if (type.hasKnownValueWidth() && storageBits < 32)
        return 32;

    return storageBits;
}

static ValueKind
objectValueKind(D dispatcher, const DispatcherCil::CilTypeAnalysis::TypeDescriptor &type, size_t resultBits)
{
    ASSERT_not_null(dispatcher);

    if (type.isKnownReferenceType())
        return ValueKind::ObjectReference;

    if (!type.metadataKnown)
        return ValueKind::Unknown;

    if (resultBits == 64)
        return ValueKind::Integer64;

    if (resultBits <= 32)
        return ValueKind::Integer32;

    return ValueKind::Unknown;
}

static void
markObjectValueFacts(D dispatcher,
                     const SValue::Ptr &value,
                     const DispatcherCil::CilTypeAnalysis::TypeDescriptor &type,
                     ValueKind kind)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(value);

    value->kind(kind);

    if (type.metadataToken != 0)
        value->typeDescriptor(CilSemantics::cilTypeTokenDescriptor(type.metadataToken));

    if (kind == ValueKind::ObjectReference || kind == ValueKind::ArrayReference)
        dispatcher->typeAnalysis().setNullness(value, DispatcherCil::CilTypeAnalysis::MaybeNull);
}


static void
storeIndirectEffect(D dispatcher, Ops ops, Indirect_Store_Kind_Type kind)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);

 // DQ (6/27/2026): Added support to handle attributes.
    noteMemoryPrefixSemantics(dispatcher, ops, "stind");

    // Stack:
    //   ..., address, value -> ...
    //
    // The value is on top of the evaluation stack.
    SValue::Ptr value = ops->popOperand();
    SValue::Ptr addr  = ops->popOperand();

    ASSERT_not_null(value);
    ASSERT_not_null(addr);

    // Normalize address to native pointer width.
    addr = CilSemantics::normalizeToPointerBits(dispatcher, ops, addr);

    const size_t ptrBits = CilSemantics::cilPointerBits(dispatcher);

    size_t storeBits = 0;

    switch (kind) {
        case stind_ref_kind:
            storeBits = ptrBits;
            break;

        case stind_i1_kind:
            storeBits = 8;
            break;

        case stind_i2_kind:
            storeBits = 16;
            break;

        case stind_i4_kind:
        case stind_r4_kind:
            storeBits = 32;
            break;

        case stind_i8_kind:
        case stind_r8_kind:
            storeBits = 64;
            break;

        case stind_i_kind:
             storeBits = ptrBits;
             break;

        default:
            ASSERT_not_reachable("invalid CIL indirect store kind");
    }

    // Compute the value that will be stored (truncated/extended to the store width).
    SValue::Ptr storedValue = resizeForStore(ops, value, storeBits);

    // Critical invariant:
    //  - Normal addresses: exactly one ops->writeMemory.
    //  - ldloca/ldloca.s addresses: exactly one writeCilLocal (which mirrors once).
    //  - ldarga/ldarga.s addresses: exactly one writeCilArgument (which mirrors once).
    CilSemantics::writeAddressOrCilSlot(dispatcher, ops, addr, storedValue, storeBits);
}


     static SValue::Ptr
     resizeForSignedIntegerConversion(Ops ops, const SValue::Ptr &value, size_t sourceBits, size_t resultBits)
        {
          ASSERT_not_null(ops);
          ASSERT_not_null(value);

          SValue::Ptr narrowed = value;

          if (narrowed->nBits() > sourceBits)
             {
               narrowed = ops->extract(narrowed, 0, sourceBits);
             }
            else
             {
               if (narrowed->nBits() < sourceBits)
                  {
                 // narrowed = ops->unsignedExtend(narrowed, sourceBits);
                    narrowed = ops->signExtend(narrowed, sourceBits);
                  }
             }
    
          if (narrowed->nBits() < resultBits)
               return ops->signExtend(narrowed, resultBits);

          if (narrowed->nBits() > resultBits)
               return ops->extract(narrowed, 0, resultBits);

          return narrowed;
        }

     static SValue::Ptr
     resizeForUnsignedIntegerConversion(Ops ops, const SValue::Ptr &value, size_t sourceBits, size_t resultBits)
        {
          ASSERT_not_null(ops);
          ASSERT_not_null(value);

          SValue::Ptr narrowed = value;

          if (narrowed->nBits() > sourceBits)
               narrowed = ops->extract(narrowed, 0, sourceBits);
          else
             if (narrowed->nBits() < sourceBits)
                  narrowed = ops->unsignedExtend(narrowed, sourceBits);

          if (narrowed->nBits() < resultBits)
               return ops->unsignedExtend(narrowed, resultBits);

          if (narrowed->nBits() > resultBits)
               return ops->extract(narrowed, 0, resultBits);

          return narrowed;
        }

static bool
isFloatLike(ValueKind kind)
   {
     return kind == ValueKind::Float32 || kind == ValueKind::Float64;
   }

static Sawyer::Optional<double>
concreteFloatAsDouble(const SValue::Ptr &value)
   {
     ASSERT_not_null(value);

     Sawyer::Optional<uint64_t> bits = value->toUnsigned();
     if (!bits)
          return Sawyer::Nothing();

     if (value->kind() == ValueKind::Float32)
        {
          uint32_t raw = static_cast<uint32_t>(*bits & 0xffffffffu);
          float f = 0.0f;
          static_assert(sizeof raw == sizeof f, "unexpected float32 size");
          std::memcpy(&f, &raw, sizeof f);
          return static_cast<double>(f);
        }

     if (value->kind() == ValueKind::Float64)
        {
          uint64_t raw = *bits;
          double d = 0.0;
          static_assert(sizeof raw == sizeof d, "unexpected float64 size");
          std::memcpy(&d, &raw, sizeof d);
          return d;
        }

     return Sawyer::Nothing();
   }

static Sawyer::Optional<uint64_t>
rawFloatExpressionBits(const SgAsmFloatValueExpression *expr, size_t nbits)
   {
     ASSERT_not_null(expr);
     ASSERT_require(nbits <= 64);

     const Sawyer::Container::BitVector &bits = expr->get_bitVector();
     if (bits.size() == nbits)
          return bits.toInteger();

     return Sawyer::Nothing();
   }

static Sawyer::Optional<bool>
concreteFloatComparison(const SValue::Ptr &lhs,
                        const SValue::Ptr &rhs,
                        Float_Compare_Op_Kind_Type kind)
   {
     ASSERT_not_null(lhs);
     ASSERT_not_null(rhs);

     if (!isFloatLike(lhs->kind()) || !isFloatLike(rhs->kind()))
          return Sawyer::Nothing();

     Sawyer::Optional<double> a = CilSemantics::concreteFloatAsDouble(lhs);
     Sawyer::Optional<double> b = CilSemantics::concreteFloatAsDouble(rhs);
     if (!a || !b)
          return Sawyer::Nothing();

     const bool unordered = std::isnan(*a) || std::isnan(*b);

     switch (kind)
        {
          case float_compare_eq:
             return !unordered && *a == *b;

          case float_compare_ne_un:
             return unordered || *a != *b;

          case float_compare_ge:
             return !unordered && *a >= *b;

          case float_compare_gt:
             return !unordered && *a > *b;

          case float_compare_le:
             return !unordered && *a <= *b;

          case float_compare_lt:
             return !unordered && *a < *b;

          case float_compare_ge_un:
             return unordered || *a >= *b;

          case float_compare_gt_un:
             return unordered || *a > *b;

          case float_compare_le_un:
             return unordered || *a <= *b;

          case float_compare_lt_un:
             return unordered || *a < *b;
        }

     ASSERT_not_reachable("invalid CIL floating-point comparison kind");
   }

static SValue::Ptr
concreteBoolean(Ops ops, bool value)
   {
     ASSERT_not_null(ops);
     return ops->number_(1, value ? 1 : 0);
   }

static SValue::Ptr
concreteFloatFromDouble(Ops ops, double value, ValueKind kind)
   {
     ASSERT_not_null(ops);

     if (kind == ValueKind::Float32)
        {
          const float f = static_cast<float>(value);
          uint32_t raw = 0;
          static_assert(sizeof raw == sizeof f, "unexpected float32 size");
          std::memcpy(&raw, &f, sizeof raw);

          SValue::Ptr result = ops->number_(32, raw);
          result->kind(ValueKind::Float32);
          return result;
        }

     if (kind == ValueKind::Float64)
        {
          uint64_t raw = 0;
          static_assert(sizeof raw == sizeof value, "unexpected float64 size");
          std::memcpy(&raw, &value, sizeof raw);

          SValue::Ptr result = ops->number_(64, raw);
          result->kind(ValueKind::Float64);
          return result;
        }

     ASSERT_not_reachable("invalid CIL floating-point result kind");
   }

static Sawyer::Optional<double>
concreteSignedFloatConversionSource(const SValue::Ptr &value)
   {
     ASSERT_not_null(value);

     if (isFloatLike(value->kind()))
          return CilSemantics::concreteFloatAsDouble(value);

     if (!isIntegerLike(value->kind()))
          return Sawyer::Nothing();

     Sawyer::Optional<int64_t> signedValue = value->toSigned();
     if (!signedValue)
          return Sawyer::Nothing();

     return static_cast<double>(*signedValue);
   }

static Sawyer::Optional<double>
concreteUnsignedFloatConversionSource(const SValue::Ptr &value)
   {
     ASSERT_not_null(value);

     if (!isIntegerLike(value->kind()))
          return Sawyer::Nothing();

     Sawyer::Optional<uint64_t> unsignedValue = value->toUnsigned();
     if (!unsignedValue)
          return Sawyer::Nothing();

     return static_cast<double>(*unsignedValue);
   }

static uint64_t
lowBitsMask(size_t nbits)
   {
     ASSERT_require(nbits > 0 && nbits <= 64);
     if (nbits == 64)
          return UINT64_MAX;
     return (uint64_t(1) << nbits) - 1;
   }

static SValue::Ptr
finishConcreteIntegerConversion(Ops ops,
                                uint64_t targetBitsValue,
                                const CilConversionTarget &target)
   {
     ASSERT_not_null(ops);

     SValue::Ptr result = ops->number_(target.targetBits,
                                       targetBitsValue & CilSemantics::lowBitsMask(target.targetBits));

     if (result->nBits() < target.stackBits) {
          if (target.targetIsSigned)
               result = ops->signExtend(result, target.stackBits);
          else
               result = ops->unsignedExtend(result, target.stackBits);
     } else if (result->nBits() > target.stackBits) {
          result = ops->extract(result, 0, target.stackBits);
     }

     result->kind(target.resultKind);
     return result;
   }

static SValue::Ptr
concreteFloatToIntegerConversion(Ops ops,
                                 const SValue::Ptr &value,
                                 const CilConversionTarget &target,
                                 bool &overflows)
   {
     ASSERT_not_null(ops);
     ASSERT_not_null(value);

     overflows = false;

     Sawyer::Optional<double> concrete = CilSemantics::concreteFloatAsDouble(value);
     if (!concrete)
          return SValue::Ptr();

     if (!std::isfinite(*concrete)) {
          overflows = true;
          return SValue::Ptr();
     }

     const long double truncated = std::trunc(*concrete);

     if (target.targetIsSigned) {
          long double lo = 0.0;
          long double hi = 0.0;

          if (target.targetBits == 64) {
               lo = static_cast<long double>(std::numeric_limits<int64_t>::min());
               hi = static_cast<long double>(std::numeric_limits<int64_t>::max());
          } else {
               const int64_t one = 1;
               lo = -static_cast<long double>(one << (target.targetBits - 1));
               hi =  static_cast<long double>((one << (target.targetBits - 1)) - 1);
          }

          if (truncated < lo || truncated > hi) {
               overflows = true;
               return SValue::Ptr();
          }

          const int64_t signedValue = static_cast<int64_t>(truncated);
          return CilSemantics::finishConcreteIntegerConversion(
              ops, static_cast<uint64_t>(signedValue), target);
     }

     const long double lo = 0.0;
     long double hi = 0.0;
     if (target.targetBits == 64)
          hi = static_cast<long double>(std::numeric_limits<uint64_t>::max());
     else
          hi = static_cast<long double>((uint64_t(1) << target.targetBits) - 1);

     if (truncated < lo || truncated > hi) {
          overflows = true;
          return SValue::Ptr();
     }

     const uint64_t unsignedValue = static_cast<uint64_t>(truncated);
     return CilSemantics::finishConcreteIntegerConversion(ops, unsignedValue, target);
   }

static SValue::Ptr
uncheckedFloatToIntegerConversion(Ops ops,
                                  const SValue::Ptr &value,
                                  const CilConversionTarget &target)
   {
     ASSERT_not_null(ops);
     ASSERT_not_null(value);

     bool overflows = false;
     SValue::Ptr result =
         CilSemantics::concreteFloatToIntegerConversion(ops, value, target, overflows);
     if (result != NULL)
          return result;

     result = ops->undefined_(target.stackBits);
     result->kind(target.resultKind);
     return result;
   }


// Forward declarations.
// static void convertOverflowUnsignedResult(D dispatcher, Ops ops, Convert_Kind_Type kind);
// static void convertOverflowSignedResult(D dispatcher, Ops ops, Convert_Kind_Type kind);
static SValue::Ptr convertOverflowUnsignedValue(D dispatcher, Ops ops, I insn, Convert_Kind_Type kind, const SValue::Ptr &value);
static SValue::Ptr convertOverflowSignedValue  (D dispatcher, Ops ops, I insn, Convert_Kind_Type kind, const SValue::Ptr &value);

static void
convertResult(D dispatcher, Ops ops, Convert_Kind_Type kind, I insn)
{
    ASSERT_not_null(ops);

    // Stack:
    //   ..., value -> ..., convertedValue
    SValue::Ptr value = ops->popOperand();
    ASSERT_not_null(value);

    SValue::Ptr result;

    switch (kind) {
        case conv_i1_kind:
            // Convert to signed int8, then sign-extend to int32.
            if (isFloatLike(value->kind()))
                result = CilSemantics::uncheckedFloatToIntegerConversion(
                    ops, value, CilConversionTarget{8, 32, true, ValueKind::Integer32});
            else
                result = resizeForSignedIntegerConversion(ops, value, 8, 32);
            result->kind(ValueKind::Integer32);
            break;

        case conv_i2_kind:
            // Convert to signed int16, then sign-extend to int32.
            if (isFloatLike(value->kind()))
                result = CilSemantics::uncheckedFloatToIntegerConversion(
                    ops, value, CilConversionTarget{16, 32, true, ValueKind::Integer32});
            else
                result = resizeForSignedIntegerConversion(ops, value, 16, 32);
            result->kind(ValueKind::Integer32);
            break;

        case conv_i4_kind:
            // Convert to int32.
            if (isFloatLike(value->kind()))
                result = CilSemantics::uncheckedFloatToIntegerConversion(
                    ops, value, CilConversionTarget{32, 32, true, ValueKind::Integer32});
            else
                result = resizeForSignedIntegerConversion(ops, value, 32, 32);
            result->kind(ValueKind::Integer32);
            break;

        case conv_i8_kind:
            // Convert to int64. For integer inputs, sign-extend/truncate.
            if (isFloatLike(value->kind()))
                result = CilSemantics::uncheckedFloatToIntegerConversion(
                    ops, value, CilConversionTarget{64, 64, true, ValueKind::Integer64});
            else
                result = resizeForSignedIntegerConversion(ops, value, 64, 64);
            result->kind(ValueKind::Integer64);
            break;

        case conv_u4_kind:
            // Convert to unsigned int32. The CIL stack category is still int32.
            if (isFloatLike(value->kind()))
                result = CilSemantics::uncheckedFloatToIntegerConversion(
                    ops, value, CilConversionTarget{32, 32, false, ValueKind::Integer32});
            else
                result = resizeForUnsignedIntegerConversion(ops, value, 32, 32);
            result->kind(ValueKind::Integer32);
            break;

        case conv_u8_kind:
            // Convert to unsigned int64. The stack category is int64.
            if (isFloatLike(value->kind()))
                result = CilSemantics::uncheckedFloatToIntegerConversion(
                    ops, value, CilConversionTarget{64, 64, false, ValueKind::Integer64});
            else
                result = resizeForUnsignedIntegerConversion(ops, value, 64, 64);
            result->kind(ValueKind::Integer64);
            break;

        case conv_r4_kind:
           {
            Sawyer::Optional<double> concrete = CilSemantics::concreteSignedFloatConversionSource(value);
            if (concrete)
                result = CilSemantics::concreteFloatFromDouble(ops, *concrete, ValueKind::Float32);
            else
                result = ops->undefined_(32);
            result->kind(ValueKind::Float32);
            break;
           }

        case conv_r8_kind:
           {
            Sawyer::Optional<double> concrete = CilSemantics::concreteSignedFloatConversionSource(value);
            if (concrete)
                result = CilSemantics::concreteFloatFromDouble(ops, *concrete, ValueKind::Float64);
            else
                result = ops->undefined_(64);
            result->kind(ValueKind::Float64);
            break;
           }

        case conv_ovf_i1_un_kind:
        case conv_ovf_i2_un_kind:
        case conv_ovf_i4_un_kind:
        case conv_ovf_i8_un_kind:
        case conv_ovf_u1_un_kind:
        case conv_ovf_u2_un_kind:
        case conv_ovf_u4_un_kind:
        case conv_ovf_u8_un_kind:
        case conv_ovf_i_un_kind:
        case conv_ovf_u_un_kind:
          result = CilSemantics::convertOverflowUnsignedValue(dispatcher, ops, insn, kind, value);
             break;

        case conv_u1_kind:
           {
          // Convert to unsigned int8, then zero-extend to int32 stack type.
             if (isFloatLike(value->kind()))
                {
                  result = CilSemantics::uncheckedFloatToIntegerConversion(
                      ops, value, CilConversionTarget{8, 32, false, ValueKind::Integer32});
                }
               else
                {
                  result = resizeForUnsignedIntegerConversion(ops, value, 8, 32);
                }
             result->kind(ValueKind::Integer32);
             break;
           }

        case conv_u2_kind:
           {
          // Convert to unsigned int16, then zero-extend to int32 stack type.
             if (isFloatLike(value->kind()))
                {
                  result = CilSemantics::uncheckedFloatToIntegerConversion(
                      ops, value, CilConversionTarget{16, 32, false, ValueKind::Integer32});
                }
               else
                {
                  result = resizeForUnsignedIntegerConversion(ops, value, 16, 32);
                }
             result->kind(ValueKind::Integer32);
             break;
           }

        case conv_i_kind:
           {
          // Convert to native signed int.
             const size_t ptrBits = CilSemantics::cilPointerBits(dispatcher);

             if (isFloatLike(value->kind()))
                {
                  result = CilSemantics::uncheckedFloatToIntegerConversion(
                      ops, value, CilConversionTarget{ptrBits, ptrBits, true, ValueKind::NativeInt});
                }
               else
                {
                  result = resizeForSignedIntegerConversion(ops, value, ptrBits, ptrBits);
                }

             result->kind(ValueKind::NativeInt);
             break;
           }

        case conv_u_kind:
           {
          // Convert to native unsigned int.
             const size_t ptrBits = CilSemantics::cilPointerBits(dispatcher);

             if (isFloatLike(value->kind()))
                {
                  result = CilSemantics::uncheckedFloatToIntegerConversion(
                      ops, value, CilConversionTarget{ptrBits, ptrBits, false, ValueKind::NativeInt});
                }
               else
                {
                  result = resizeForUnsignedIntegerConversion(ops, value, ptrBits, ptrBits);
                }

             result->kind(ValueKind::NativeInt);
             break;
           }

        case conv_r_un_kind:
           {
          // Convert unsigned integer source to floating point.
          //
          // CLI result is F/native floating type. In this dispatcher, model it as
          // a 64-bit floating value. Do not reinterpret the integer bits as IEEE.
             Sawyer::Optional<double> concrete = CilSemantics::concreteUnsignedFloatConversionSource(value);
             if (concrete)
                  result = CilSemantics::concreteFloatFromDouble(ops, *concrete, ValueKind::Float64);
             else
                  result = ops->undefined_(64);
             result->kind(ValueKind::Float64);
             break;
           }

        case conv_ovf_i1_kind:
        case conv_ovf_u1_kind:
        case conv_ovf_i2_kind:
        case conv_ovf_u2_kind:
        case conv_ovf_i4_kind:
        case conv_ovf_u4_kind:
        case conv_ovf_i8_kind:
        case conv_ovf_u8_kind:
        case conv_ovf_i_kind:
        case conv_ovf_u_kind:
          result = CilSemantics::convertOverflowSignedValue(dispatcher, ops, insn, kind, value);
             break;

         default:
            ASSERT_not_reachable("invalid CIL conversion kind");
     }

     if (result == NULL)
          return;

     ops->pushOperand(result);
   }

static void
copyObjectEffect(D dispatcher, Ops ops, const DispatcherCil::CilTypeAnalysis::TypeDescriptor &type)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);

    // Stack:
    //   ..., destinationAddress, sourceAddress -> ...
    SValue::Ptr srcAddr = ops->popOperand();
    SValue::Ptr dstAddr = ops->popOperand();

    ASSERT_not_null(srcAddr);
    ASSERT_not_null(dstAddr);

    srcAddr = CilSemantics::normalizeToPointerBits(dispatcher, ops, srcAddr);
    dstAddr = CilSemantics::normalizeToPointerBits(dispatcher, ops, dstAddr);

    const size_t copyBits = CilSemantics::objectStorageBits(dispatcher, type);

    SValue::Ptr value = CilSemantics::readAddressOrCilSlot(dispatcher, ops, srcAddr, copyBits);
    ASSERT_not_null(value);

    // Route writes through the same synthetic local/argument address handling
    // used by stind.  This keeps cpobj through an ldloca/ldarga destination
    // visible in the authoritative CIL local/argument slot, not only in the
    // mirrored synthetic memory cell.
    CilSemantics::writeAddressOrCilSlot(dispatcher, ops, dstAddr, value, copyBits);
}

static void
loadObjectEffect(D dispatcher, Ops ops,
                 const DispatcherCil::CilTypeAnalysis::TypeDescriptor &type)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);

 // DQ (6/27/2026): Added support to handle attributes.
    noteMemoryPrefixSemantics(dispatcher, ops, "ldobj");

    SValue::Ptr addr = ops->popOperand();
    ASSERT_not_null(addr);

    addr = CilSemantics::normalizeToPointerBits(dispatcher, ops, addr);

    const size_t storageBits = CilSemantics::objectStorageBits(dispatcher, type);
    const size_t resultBits = CilSemantics::objectStackBits(dispatcher, type);

    SValue::Ptr result = CilSemantics::readAddressOrCilSlot(dispatcher, ops, addr, storageBits);
    ASSERT_not_null(result);

    if (result->nBits() < resultBits)
        result = ops->unsignedExtend(result, resultBits);
    else if (result->nBits() > resultBits)
        result = ops->extract(result, 0, resultBits);

    const ValueKind kind = CilSemantics::objectValueKind(dispatcher, type, resultBits);
    CilSemantics::markObjectValueFacts(dispatcher, result, type, kind);

    ops->pushOperand(result);
}

static SValue::Ptr
stringReference(D dispatcher, Ops ops, uint32_t userStringToken)
{
    ASSERT_not_null(ops);

    const size_t ptrBits = CilSemantics::cilPointerBits(dispatcher);

    SValue::Ptr result = ops->undefined_(ptrBits);
    result->kind(ValueKind::ObjectReference);

    // Use a simple descriptor until real metadata/string handling is wired in.
    result->typeDescriptor("cil:System.String/userstring:" +
                           StringUtility::addrToString(userStringToken));

    if (dispatcher) {
        dispatcher->typeAnalysis().setNullness(
            result, DispatcherCil::CilTypeAnalysis::DefinitelyNonNull);
    }

    return result;
}


     static void
     newObjectEffect(D dispatcher, Ops ops, I /*insn*/, const DispatcherCil::CilTypeAnalysis::MethodDescriptor &ctor)
        {
          ASSERT_not_null(dispatcher);
          ASSERT_not_null(ops);

       // DQ (7/1/2026): Keep byte-only smoke tests alive when no
       // metadata provider is available for the constructor token.
          if (!ctor.signatureKnown)
             {
               ops->comment("CIL newobj: constructor signature unknown; stack-preserving byte-only fallback semantics used");

            // In byte-only testing, there is no metadata provider, so we cannot know
            // how many constructor arguments to pop. Even though real CIL
            // newobj pushes a newly allocated object on the normal path, doing
            // so without popping the constructor arguments corrupts the
            // evaluation stack. Preserve the stack exactly until metadata is
            // available.
               return;
             }

          for (size_t i = 0; i < ctor.parameterCount; ++i)
             {
               SValue::Ptr arg = ops->popOperand();
               ASSERT_not_null(arg);
             }

          const size_t ptrBits = CilSemantics::cilPointerBits(dispatcher);

          // Allocation may throw OutOfMemoryException, but this dispatcher does
          // not split normal and exceptional allocation edges.  Do not write an
          // active exception into the merged state that will also contain the
          // normally allocated object.
          ops->comment("CIL newobj: object allocation failure edge not split");

          SValue::Ptr obj = ops->undefined_(ptrBits);
          obj->kind(ValueKind::ObjectReference);

          if (ctor.metadataToken != 0)
             {
               obj->typeDescriptor("cil:constructed-by:" + StringUtility::addrToString(ctor.metadataToken));
             }

          dispatcher->typeAnalysis().setNullness(obj, DispatcherCil::CilTypeAnalysis::DefinitelyNonNull);

          ops->pushOperand(obj);
        }

#if 0
static DispatcherCil::CilTypeAnalysis::TypeDescriptor
typeDescriptorFromToken(uint32_t token)
   {
  // TODO: Replace this with real CIL metadata resolution.
  // For now this only preserves the metadata token.
     return DispatcherCil::CilTypeAnalysis::TypeDescriptor(token);
   }
#endif

static bool
isCilExceptionReference(const SValue::Ptr &value)
{
    ASSERT_not_null(value);

    const ValueKind kind = value->kind();
    return kind == ValueKind::ObjectReference ||
           kind == ValueKind::ArrayReference ||
           kind == ValueKind::Unknown;
}

static SValue::Ptr
normalizedExceptionReference(D dispatcher, Ops ops, const SValue::Ptr &value)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);
    ASSERT_not_null(value);

    const size_t ptrBits = CilSemantics::cilPointerBits(dispatcher);
    SValue::Ptr result = value;

    if (result->nBits() < ptrBits)
        result = ops->unsignedExtend(result, ptrBits);
    else if (result->nBits() > ptrBits)
        result = ops->extract(result, 0, ptrBits);

    if (result != value) {
        result->kind(value->kind());
        if (value->hasTypeDescriptor())
            result->typeDescriptor(value->typeDescriptor());
        dispatcher->typeAnalysis().copyFacts(result, value);
    }

    return result;
}

static void
recordCurrentException(D dispatcher, Ops ops, const SValue::Ptr &exceptionObject)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);
    ASSERT_not_null(exceptionObject);

    if (!CilSemantics::isCilExceptionReference(exceptionObject))
        return;

    SValue::Ptr currentException;

    if (dispatcher->typeAnalysis().isDefinitelyNull(exceptionObject)) {
        currentException = ops->undefined_(CilSemantics::cilPointerBits(dispatcher));
        currentException->kind(ValueKind::ObjectReference);
        currentException->typeDescriptor("cil:exception:System.NullReferenceException");
        dispatcher->typeAnalysis().setNullness(
            currentException, DispatcherCil::CilTypeAnalysis::DefinitelyNonNull);
    } else if (CilSemantics::isMaybeNull(dispatcher, exceptionObject)) {
        // throw maybe-null has two exceptional possibilities:
        //   - non-null input: throw that object,
        //   - null input: throw System.NullReferenceException.
        //
        // Either way, the active exception object is non-null.  Preserve that
        // fact instead of recording a maybe-null value in the current-exception
        // slot.
        currentException = ops->undefined_(CilSemantics::cilPointerBits(dispatcher));
        currentException->kind(ValueKind::ObjectReference);
        currentException->typeDescriptor("cil:exception:maybe-input-or-System.NullReferenceException");
        dispatcher->typeAnalysis().setNullness(
            currentException, DispatcherCil::CilTypeAnalysis::DefinitelyNonNull);
    } else {
        currentException =
            CilSemantics::normalizedExceptionReference(dispatcher, ops, exceptionObject);
    }

    ops->writeLocal(CilSemantics::cilCurrentExceptionSlot(), currentException);
}

static void
throwExceptionEffect(D dispatcher, Ops ops)
   {
     ASSERT_not_null(dispatcher);
     ASSERT_not_null(ops);

     SValue::Ptr exceptionObject = ops->popOperand();
     ASSERT_not_null(exceptionObject);

     if (!CilSemantics::isCilExceptionReference(exceptionObject))
        {
          CilSemantics::invalidProgramExceptionPath(
              dispatcher, ops,
              "CIL throw: non-object exception operand; InvalidProgramException path not fully modeled");
          return;
        }

     if (dispatcher->typeAnalysis().isDefinitelyNull(exceptionObject))
        {
       // TODO: Model the CLI behavior precisely:
       //   throw null
       // throws System.NullReferenceException.
       //
       // For now, both ordinary exception throwing and null-reference
       // throwing leave normal control flow.
        }
       else
       {
       // TODO: Use exceptionObject's typeDescriptor/possible runtime types
       // to resolve a matching exception handler.
        }

     CilSemantics::recordCurrentException(dispatcher, ops, exceptionObject);
     CilSemantics::writeUnknownInstructionPointer(dispatcher, ops);
   }

#if 0
static DispatcherCil::CilTypeAnalysis::FieldDescriptor
fieldDescriptorFromToken(uint32_t token)
   {
  // TODO: Replace with real CIL metadata resolution.
     return DispatcherCil::CilTypeAnalysis::FieldDescriptor(token);
   }
#endif

static SValue::Ptr
normalizePointer(D dispatcher, Ops ops, const SValue::Ptr &addr)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);
    ASSERT_not_null(addr);

    const size_t ptrBits = CilSemantics::cilPointerBits(dispatcher);

    SValue::Ptr normalized = addr;

    if (addr->nBits() < ptrBits)
        normalized = ops->unsignedExtend(addr, ptrBits);
    else if (addr->nBits() > ptrBits)
        normalized = ops->extract(addr, 0, ptrBits);

    if (normalized != addr) {
        normalized->kind(addr->kind());
        if (addr->hasTypeDescriptor())
            normalized->typeDescriptor(addr->typeDescriptor());
        dispatcher->typeAnalysis().copyFacts(normalized, addr);
    }

    return normalized;
}

static SValue::Ptr
resizeFieldValueForStore(Ops ops, const SValue::Ptr &value, size_t fieldBits)
{
    ASSERT_not_null(ops);
    ASSERT_not_null(value);

    if (value->nBits() < fieldBits)
        return ops->unsignedExtend(value, fieldBits);

    if (value->nBits() > fieldBits)
        return ops->extract(value, 0, fieldBits);

    return value;
}


static ValueKind
defaultFieldValueKind(size_t fieldBits)
{
    if (fieldBits == 64)
        return ValueKind::Integer64;

    if (fieldBits <= 32)
        return ValueKind::Integer32;

    return ValueKind::Unknown;
}

static bool
fieldIsReferenceLike(const DispatcherCil::CilTypeAnalysis::FieldDescriptor &field)
{
    return field.valueKind == ValueKind::ObjectReference ||
           field.valueKind == ValueKind::ArrayReference ||
           field.isReferenceType;
}

static size_t
fieldStorageBits(D dispatcher, const DispatcherCil::CilTypeAnalysis::FieldDescriptor &field, size_t fallbackBits)
{
    ASSERT_not_null(dispatcher);

    if (field.typeKnown && field.fieldBits != 0)
        return field.fieldBits;

    if (fieldIsReferenceLike(field))
        return CilSemantics::cilPointerBits(dispatcher);

    return fallbackBits != 0 ? fallbackBits : 32;
}

static ValueKind
fieldValueKind(const DispatcherCil::CilTypeAnalysis::FieldDescriptor &field, size_t fieldBits)
{
    if (field.typeKnown && field.valueKind != ValueKind::Unknown)
        return field.valueKind;

    if (fieldIsReferenceLike(field))
        return field.isValueType ? ValueKind::Unknown : ValueKind::ObjectReference;

    return defaultFieldValueKind(fieldBits);
}

static void
markFieldValueFacts(D dispatcher,
                    const SValue::Ptr &value,
                    const DispatcherCil::CilTypeAnalysis::FieldDescriptor &field,
                    ValueKind kind)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(value);

    value->kind(kind);

    if (!field.typeDescriptor.empty())
        value->typeDescriptor(field.typeDescriptor);
    else if (field.fieldTypeToken != 0)
        value->typeDescriptor(CilSemantics::cilTypeTokenDescriptor(field.fieldTypeToken));

    if (kind == ValueKind::ObjectReference || kind == ValueKind::ArrayReference)
        dispatcher->typeAnalysis().setNullness(value, DispatcherCil::CilTypeAnalysis::MaybeNull);
}

static SValue::Ptr
instanceFieldAddress(D dispatcher,
                     Ops ops,
                     const SValue::Ptr &objectOrAddress,
                     const DispatcherCil::CilTypeAnalysis::FieldDescriptor &field)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);
    ASSERT_not_null(objectOrAddress);

    const size_t ptrBits = CilSemantics::cilPointerBits(dispatcher);

    SValue::Ptr base = normalizePointer(dispatcher, ops, objectOrAddress);

    if (field.offsetKnown) {
        SValue::Ptr offset = ops->number_(ptrBits, field.offsetBytes);
        return ops->add(base, offset);
    }

    if (field.metadataKnown && field.metadataToken != 0) {
        // Layout unavailable, but the field token is stable.  Use a synthetic
        // token-derived displacement so ldfld/stfld/ldflda for the same field
        // alias consistently instead of each use getting a fresh unknown
        // address.
        const uint64_t syntheticOffset =
            0x01000000ULL + (uint64_t(field.metadataToken & 0x00ffffffU) * 16ULL);
        SValue::Ptr offset = ops->number_(ptrBits, syntheticOffset);
        return ops->add(base, offset);
    }

    return ops->undefined_(ptrBits);
}


static SValue::Ptr
staticFieldAddress(D dispatcher, Ops ops, const DispatcherCil::CilTypeAnalysis::FieldDescriptor &field)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);

    const size_t ptrBits = CilSemantics::cilPointerBits(dispatcher);

    // Synthetic static-field memory base. Pick a range that is only used by
    // this CIL model.
    static const uint64_t CIL_STATIC_FIELD_BASE = 0x70000000ULL;

    uint64_t tokenPart = uint64_t(field.metadataToken) * 16ULL;
    return ops->number_(ptrBits, CIL_STATIC_FIELD_BASE + tokenPart);
}

static void
loadInstanceField(D dispatcher, Ops ops, const DispatcherCil::CilTypeAnalysis::FieldDescriptor &field)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);

 // DQ (6/27/2026): Added support to handle attributes.
    noteMemoryPrefixSemantics(dispatcher, ops, "ldfld");

    SValue::Ptr objectOrAddress = ops->popOperand();
    ASSERT_not_null(objectOrAddress);

    // TODO: If definitely null object reference, model NullReferenceException.
    if (dispatcher->typeAnalysis().isDefinitelyNull(objectOrAddress)) {
        SValue::Ptr exceptionObject = ops->undefined_(CilSemantics::cilPointerBits(dispatcher));
        exceptionObject->kind(ValueKind::ObjectReference);
        exceptionObject->typeDescriptor("cil:exception:System.NullReferenceException");
        dispatcher->typeAnalysis().setNullness(
            exceptionObject, DispatcherCil::CilTypeAnalysis::DefinitelyNonNull);
        CilSemantics::recordCurrentException(dispatcher, ops, exceptionObject);
        CilSemantics::writeUnknownInstructionPointer(dispatcher, ops);
        return;
    }
    CilSemantics::noteMaybeNullReferenceException(dispatcher, ops, objectOrAddress, "ldfld");

    const size_t fieldBits = CilSemantics::fieldStorageBits(dispatcher, field, 32);
    const ValueKind kind = CilSemantics::fieldValueKind(field, fieldBits);

    SValue::Ptr addr = instanceFieldAddress(dispatcher, ops, objectOrAddress, field);

    SValue::Ptr result = ops->readMemory(
        RegisterDescriptor(),
        addr,
        ops->undefined_(fieldBits),
        ops->number_(1, 1));

    CilSemantics::markFieldValueFacts(dispatcher, result, field, kind);

    ops->pushOperand(result);
}


static void
loadInstanceFieldAddress(D dispatcher, Ops ops, const DispatcherCil::CilTypeAnalysis::FieldDescriptor &field)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);

    SValue::Ptr objectOrAddress = ops->popOperand();
    ASSERT_not_null(objectOrAddress);

    if (dispatcher->typeAnalysis().isDefinitelyNull(objectOrAddress)) {
        SValue::Ptr exceptionObject = ops->undefined_(CilSemantics::cilPointerBits(dispatcher));
        exceptionObject->kind(ValueKind::ObjectReference);
        exceptionObject->typeDescriptor("cil:exception:System.NullReferenceException");
        dispatcher->typeAnalysis().setNullness(
            exceptionObject, DispatcherCil::CilTypeAnalysis::DefinitelyNonNull);
        CilSemantics::recordCurrentException(dispatcher, ops, exceptionObject);
        CilSemantics::writeUnknownInstructionPointer(dispatcher, ops);
        return;
    }
    CilSemantics::noteMaybeNullReferenceException(dispatcher, ops, objectOrAddress, "ldflda");

    SValue::Ptr addr = instanceFieldAddress(dispatcher, ops, objectOrAddress, field);
    addr->kind(ValueKind::ManagedPointer);
    if (field.metadataToken != 0)
        addr->typeDescriptor("cil:instance-field-address:" + StringUtility::addrToString(field.metadataToken));

    ops->pushOperand(addr);
}


static void
storeInstanceField(D dispatcher, Ops ops, const DispatcherCil::CilTypeAnalysis::FieldDescriptor &field)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);

 // DQ (6/27/2026): Added support to handle attributes.
    noteMemoryPrefixSemantics(dispatcher, ops, "stfld");

    SValue::Ptr value = ops->popOperand();
    SValue::Ptr objectOrAddress = ops->popOperand();

    ASSERT_not_null(value);
    ASSERT_not_null(objectOrAddress);

    if (dispatcher->typeAnalysis().isDefinitelyNull(objectOrAddress)) {
        SValue::Ptr exceptionObject = ops->undefined_(CilSemantics::cilPointerBits(dispatcher));
        exceptionObject->kind(ValueKind::ObjectReference);
        exceptionObject->typeDescriptor("cil:exception:System.NullReferenceException");
        dispatcher->typeAnalysis().setNullness(
            exceptionObject, DispatcherCil::CilTypeAnalysis::DefinitelyNonNull);
        CilSemantics::recordCurrentException(dispatcher, ops, exceptionObject);
        CilSemantics::writeUnknownInstructionPointer(dispatcher, ops);
        return;
    }
    CilSemantics::noteMaybeNullReferenceException(dispatcher, ops, objectOrAddress, "stfld");

    const size_t fieldBits = CilSemantics::fieldStorageBits(dispatcher, field, value->nBits());
    const ValueKind kind = CilSemantics::fieldValueKind(field, fieldBits);

    SValue::Ptr addr = instanceFieldAddress(dispatcher, ops, objectOrAddress, field);
    SValue::Ptr storedValue = resizeFieldValueForStore(ops, value, fieldBits);
    if (storedValue != value)
        dispatcher->typeAnalysis().copyFacts(storedValue, value);
    CilSemantics::markFieldValueFacts(dispatcher, storedValue, field, kind);

    ops->writeMemory(RegisterDescriptor(), addr, storedValue, ops->number_(1, 1));
}


static void
loadStaticField(D dispatcher, Ops ops, const DispatcherCil::CilTypeAnalysis::FieldDescriptor &field)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);

 // DQ (6/27/2026): Added support to handle attributes.
    noteMemoryPrefixSemantics(dispatcher, ops, "ldsfld");

    const size_t fieldBits = CilSemantics::fieldStorageBits(dispatcher, field, 32);
    const ValueKind kind = CilSemantics::fieldValueKind(field, fieldBits);

    SValue::Ptr addr = staticFieldAddress(dispatcher, ops, field);

    SValue::Ptr result = ops->readMemory(
        RegisterDescriptor(),
        addr,
        ops->undefined_(fieldBits),
        ops->number_(1, 1));

    CilSemantics::markFieldValueFacts(dispatcher, result, field, kind);

    ops->pushOperand(result);
}

static void
loadStaticFieldAddress(D dispatcher, Ops ops, const DispatcherCil::CilTypeAnalysis::FieldDescriptor &field)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);

    SValue::Ptr addr = staticFieldAddress(dispatcher, ops, field);
    addr->kind(ValueKind::ManagedPointer);
    if (field.metadataToken != 0)
        addr->typeDescriptor("cil:static-field-address:" + StringUtility::addrToString(field.metadataToken));

    ops->pushOperand(addr);
}

static void
storeStaticField(D dispatcher, Ops ops, const DispatcherCil::CilTypeAnalysis::FieldDescriptor &field)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);

 // DQ (6/27/2026): Added support to handle attributes.
    noteMemoryPrefixSemantics(dispatcher, ops, "stsfld");

    SValue::Ptr value = ops->popOperand();
    ASSERT_not_null(value);

    const size_t fieldBits = CilSemantics::fieldStorageBits(dispatcher, field, value->nBits());
    const ValueKind kind = CilSemantics::fieldValueKind(field, fieldBits);

    SValue::Ptr addr = staticFieldAddress(dispatcher, ops, field);
    SValue::Ptr storedValue = resizeFieldValueForStore(ops, value, fieldBits);
    if (storedValue != value)
        dispatcher->typeAnalysis().copyFacts(storedValue, value);
    CilSemantics::markFieldValueFacts(dispatcher, storedValue, field, kind);

    ops->writeMemory(RegisterDescriptor(), addr, storedValue, ops->number_(1, 1));
}


static void
storeObjectEffect(D dispatcher,
                  Ops ops,
                  const DispatcherCil::CilTypeAnalysis::TypeDescriptor &type)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);

 // DQ (6/27/2026): Added support to handle attributes.
    noteMemoryPrefixSemantics(dispatcher, ops, "stobj");

    // Stack:
    //   ..., address, value -> ...
    SValue::Ptr value = ops->popOperand();
    SValue::Ptr addr  = ops->popOperand();

    ASSERT_not_null(value);
    ASSERT_not_null(addr);

    addr = CilSemantics::normalizeToPointerBits(dispatcher, ops, addr);

    const size_t storeBits = CilSemantics::objectStorageBits(dispatcher, type);
    
#if 0
#if 0
    SValue::Ptr storedValue = value;

    if (storedValue->nBits() < storeBits) {
        storedValue = ops->unsignedExtend(storedValue, storeBits);
    } else if (storedValue->nBits() > storeBits) {
        storedValue = ops->extract(storedValue, 0, storeBits);
    }

    ops->writeMemory(RegisterDescriptor(), addr, storedValue, ops->number_(1, 1));
#else
 // DQ (6/29/2026): This is a better version.
    SValue::Ptr storedValue = resizeForStore(ops, value, storeBits);
    SValue::Ptr cond = ops->number_(1, 1);

    uint32_t localIndex = 0, argIndex = 0;
    if (tryDecodeLocalAddress(addr, localIndex))
       {
         writeCilLocal(dispatcher, ops, localIndex, storedValue);
       }
      else
       {
         if (tryDecodeArgumentAddress(addr, argIndex))
            {
              writeCilArgument(dispatcher, ops, argIndex, storedValue);
            }
           else
            {
              ops->writeMemory(RegisterDescriptor(), addr, storedValue, cond);
            }
       }
#endif
#else

 // DQ (6/29/2026): This used a refactored function used within storeIndirectEffect.
    SValue::Ptr storedValue = resizeForStore(ops, value, storeBits);
    const ValueKind kind = CilSemantics::objectValueKind(dispatcher, type, storeBits);
    CilSemantics::markObjectValueFacts(dispatcher, storedValue, type, kind);

    CilSemantics::writeAddressOrCilSlot( dispatcher, ops, addr, storedValue, storeBits);
#endif
}



static CilConversionTarget
overflowUnsignedConversionTarget(D dispatcher, Convert_Kind_Type kind)
{
    const size_t ptrBits = CilSemantics::cilPointerBits(dispatcher);

    switch (kind) {
        case conv_ovf_i1_un_kind:
            // target int8, stack result int32
            return CilConversionTarget{8, 32, true, ValueKind::Integer32};

        case conv_ovf_i2_un_kind:
            // target int16, stack result int32
            return CilConversionTarget{16, 32, true, ValueKind::Integer32};

        case conv_ovf_i4_un_kind:
            // target int32, stack result int32
            return CilConversionTarget{32, 32, true, ValueKind::Integer32};

        case conv_ovf_i8_un_kind:
            // target int64, stack result int64
            return CilConversionTarget{64, 64, true, ValueKind::Integer64};

        case conv_ovf_u1_un_kind:
            // target uint8, stack result int32
            return CilConversionTarget{8, 32, false, ValueKind::Integer32};

        case conv_ovf_u2_un_kind:
            // target uint16, stack result int32
            return CilConversionTarget{16, 32, false, ValueKind::Integer32};

        case conv_ovf_u4_un_kind:
            // target uint32, stack result int32
            return CilConversionTarget{32, 32, false, ValueKind::Integer32};

        case conv_ovf_u8_un_kind:
            // target uint64, stack result int64
            return CilConversionTarget{64, 64, false, ValueKind::Integer64};

        case conv_ovf_i_un_kind:
            // target native int, stack result native int
            return CilConversionTarget{ptrBits, ptrBits, true, ValueKind::NativeInt};

        case conv_ovf_u_un_kind:
            // target native unsigned int, stack result native int category
            return CilConversionTarget{ptrBits, ptrBits, false, ValueKind::NativeInt};

        default:
            ASSERT_not_reachable("invalid unsigned overflow conversion kind");
    }
}

#if 0
static SValue::Ptr
convertOverflowUnsignedValue(D dispatcher, Ops ops, I insn,
                             Convert_Kind_Type kind,
                             const SValue::Ptr &value)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);
    ASSERT_not_null(value);

    const CilConversionTarget target =
        CilSemantics::overflowUnsignedConversionTarget(dispatcher, kind);

    SValue::Ptr result = value;

    // Floating-source conversions require numeric float-to-integer conversion.
    // Do not reinterpret IEEE bits as an integer.
    if (isFloatLike(value->kind())) {
        result = ops->undefined_(target.stackBits);
        result->kind(target.resultKind);
        return result;
    }

    // For .un overflow conversions, the source is interpreted as unsigned.
    // This models result width/kind but does not yet model OverflowException.
    if (result->nBits() > target.targetBits) {
        result = ops->extract(result, 0, target.targetBits);
    } else if (result->nBits() < target.targetBits) {
        result = ops->unsignedExtend(result, target.targetBits);
    }

    // Convert from target width to CIL stack width.
    if (result->nBits() < target.stackBits) {
        if (target.targetIsSigned)
            result = ops->signExtend(result, target.stackBits);
        else
            result = ops->unsignedExtend(result, target.stackBits);
    } else if (result->nBits() > target.stackBits) {
        result = ops->extract(result, 0, target.stackBits);
    }

    result->kind(target.resultKind);
    return result;
}
#else
static SValue::Ptr
convertOverflowUnsignedValue(D dispatcher,
                             Ops ops,
                             I insn,
                             Convert_Kind_Type kind,
                             const SValue::Ptr &value)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);
    ASSERT_not_null(value);

    const CilConversionTarget target =
        CilSemantics::overflowUnsignedConversionTarget(dispatcher, kind);

    // Floating-source conversions require real numeric float-to-integer
    // conversion. Do not reinterpret IEEE bits.
    if (isFloatLike(value->kind())) {
        bool overflows = false;
        SValue::Ptr result =
            CilSemantics::concreteFloatToIntegerConversion(ops, value, target, overflows);
        if (result != NULL)
            return result;

        if (overflows) {
            CilSemantics::conversionOverflowExceptionEffect(dispatcher, ops, insn, kind);
            return SValue::Ptr();                    // no normal result
        }

        ops->comment(
            "CIL " + convertKindToString(kind) +
            ": symbolic float-to-integer checked conversion; overflow status unknown");
        result = ops->undefined_(target.stackBits);
        result->kind(target.resultKind);
        return result;
    }

    // .un form: interpret the original source as unsigned.
    if (concreteUnsignedSourceConversionOverflows(value, target)) {
        CilSemantics::conversionOverflowExceptionEffect(dispatcher, ops, insn, kind);
        return SValue::Ptr();                    // no normal result
    }

    // Symbolic values may overflow on some paths. Do not write an active
    // OverflowException into the merged normal state; compute the normal
    // abstract result and record the imprecision as a comment.
    if (!value->toUnsigned())
       {
         ops->comment("CIL " + convertKindToString(kind) +
                      ": symbolic checked conversion; overflow status unknown");
       }

    return finishOverflowConversion(ops, value, target, true);
}
#endif


static SValue::Ptr
arrayElementAddress(D dispatcher,
                    Ops ops,
                    const SValue::Ptr &arrayref,
                    const SValue::Ptr &index,
                    size_t elementBytes);

static bool
arrayAccessDefinitelyInvalid(D dispatcher,
                             Ops ops,
                             const SValue::Ptr &arrayref,
                             const SValue::Ptr &index)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);
    ASSERT_not_null(arrayref);
    ASSERT_not_null(index);

    if (dispatcher->typeAnalysis().isDefinitelyNull(arrayref)) {
        SValue::Ptr exceptionObject = ops->undefined_(CilSemantics::cilPointerBits(dispatcher));
        exceptionObject->kind(ValueKind::ObjectReference);
        exceptionObject->typeDescriptor("cil:exception:System.NullReferenceException");
        dispatcher->typeAnalysis().setNullness(
            exceptionObject, DispatcherCil::CilTypeAnalysis::DefinitelyNonNull);
        CilSemantics::recordCurrentException(dispatcher, ops, exceptionObject);
        CilSemantics::writeUnknownInstructionPointer(dispatcher, ops);
        return true;
    }
    CilSemantics::noteMaybeNullReferenceException(dispatcher, ops, arrayref, "array access");

    if (!arrayref->hasArrayLength())
        return false;

    SValue::Ptr length = arrayref->arrayLength();
    ASSERT_not_null(length);

    Sawyer::Optional<int64_t> concreteIndex = index->toSigned();
    if (!concreteIndex)
        return false;

    if (*concreteIndex < 0) {
        SValue::Ptr exceptionObject = ops->undefined_(CilSemantics::cilPointerBits(dispatcher));
        exceptionObject->kind(ValueKind::ObjectReference);
        exceptionObject->typeDescriptor("cil:exception:System.IndexOutOfRangeException");
        dispatcher->typeAnalysis().setNullness(
            exceptionObject, DispatcherCil::CilTypeAnalysis::DefinitelyNonNull);
        CilSemantics::recordCurrentException(dispatcher, ops, exceptionObject);
        CilSemantics::writeUnknownInstructionPointer(dispatcher, ops);
        return true;
    }

    Sawyer::Optional<int64_t> concreteLength = length->toSigned();
    if (!concreteLength || *concreteLength < 0)
        return false;

    if (static_cast<uint64_t>(*concreteIndex) >= static_cast<uint64_t>(*concreteLength)) {
        SValue::Ptr exceptionObject = ops->undefined_(CilSemantics::cilPointerBits(dispatcher));
        exceptionObject->kind(ValueKind::ObjectReference);
        exceptionObject->typeDescriptor("cil:exception:System.IndexOutOfRangeException");
        dispatcher->typeAnalysis().setNullness(
            exceptionObject, DispatcherCil::CilTypeAnalysis::DefinitelyNonNull);
        CilSemantics::recordCurrentException(dispatcher, ops, exceptionObject);
        CilSemantics::writeUnknownInstructionPointer(dispatcher, ops);
        return true;
    }

    return false;
}

static void
loadElementAddress(D dispatcher,
                   Ops ops,
                   I /*insn*/,
                   Args /*args*/,
                   const DispatcherCil::CilTypeAnalysis::TypeDescriptor &elementType,
                   bool readonlyElementAddress = false )
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);
 // assert_args(insn, args, 1);

#if CIL_DEBUG
    fprintf (stderr,"loadElementAddress: at TOP of instruction semantics \n");
    fflush(stderr);
#endif

    // Stack:
    //   ..., array, index -> ..., managedPointer
    SValue::Ptr index    = ops->popOperand();
    SValue::Ptr arrayref = ops->popOperand();

    ASSERT_not_null(index);
    ASSERT_not_null(arrayref);

    const size_t ptrBits = CilSemantics::cilPointerBits(dispatcher);

    // Optional sanity: ldelema expects an array reference.
    //
    // Do not assert hard yet unless all earlier instructions reliably set
    // SValue::kind(). In early bring-up, use a conservative warning/comment.
    if (arrayref->kind() != ValueKind::ArrayReference) {
        // TODO: This may be a managed pointer or an unknown value in early
        // semantics. Eventually model NullReferenceException / type errors.
    }

    if (CilSemantics::arrayAccessDefinitelyInvalid(dispatcher, ops, arrayref, index))
        return;

    // Prefer metadata-derived element size when available.
    //
    // If elementType.valueBitWidth is unknown, do not guess a precise offset
    // from a bad size. Use a conservative unknown managed pointer instead.
 // const bool elementSizeKnown = elementType.valueBitWidth != 0;
    const bool elementSizeKnown = elementType.hasKnownValueWidth();
    const size_t elementBytes =
        elementSizeKnown ? ((elementType.valueBitWidth + 7) / 8) : 0;

    SValue::Ptr elementAddress;

    if (elementSizeKnown) {
        elementAddress = CilSemantics::arrayElementAddress(dispatcher, ops, arrayref, index, elementBytes);
    } else {
        // Metadata/layout unavailable. Still return a managed pointer.
        elementAddress = ops->undefined_(ptrBits);
    }

    elementAddress->kind(ValueKind::ManagedPointer);

    if (elementType.metadataToken != 0) {
        elementAddress->typeDescriptor(
            CilSemantics::cilTypeTokenDescriptor(elementType.metadataToken));
    }

    if (readonlyElementAddress == true)
      {
     // SValue does not currently have a dedicated readonly-managed-pointer
     // field, so encode this prefix in the descriptor.  storeIndirectEffect
     // recognizes the marker through isReadonlyManagedPointer().
        CilSemantics::markReadonlyManagedPointer(elementAddress);
      }
    
    ops->pushOperand(elementAddress);

#if CIL_DEBUG
    fprintf (stderr,"loadElementAddress: at TOP of instruction semantics \n");
    fflush(stderr);
#endif
}

static ElementLoadDescriptor
elementLoadDescriptor(D dispatcher, Element_Load_Kind_Type kind)
{
    const size_t ptrBits = CilSemantics::cilPointerBits(dispatcher);

    switch (kind) {
        case ldelem_i1_kind:
            return ElementLoadDescriptor{8, 32, true, false, ValueKind::Integer32};

        case ldelem_u1_kind:
            return ElementLoadDescriptor{8, 32, false, true, ValueKind::Integer32};

        case ldelem_i2_kind:
            return ElementLoadDescriptor{16, 32, true, false, ValueKind::Integer32};

        case ldelem_u2_kind:
            return ElementLoadDescriptor{16, 32, false, true, ValueKind::Integer32};

        case ldelem_i4_kind:
            return ElementLoadDescriptor{32, 32, false, false, ValueKind::Integer32};

        case ldelem_u4_kind:
            // CIL stack category is still int32.
            return ElementLoadDescriptor{32, 32, false, false, ValueKind::Integer32};

        case ldelem_i8_kind:
            return ElementLoadDescriptor{64, 64, false, false, ValueKind::Integer64};

        case ldelem_i_kind:
            return ElementLoadDescriptor{ptrBits, ptrBits, false, false, ValueKind::NativeInt};

        case ldelem_r4_kind:
            return ElementLoadDescriptor{32, 32, false, false, ValueKind::Float32};

        case ldelem_r8_kind:
            return ElementLoadDescriptor{64, 64, false, false, ValueKind::Float64};

        case ldelem_ref_kind:
            return ElementLoadDescriptor{ptrBits, ptrBits, false, false, ValueKind::ObjectReference};

        default:
            ASSERT_not_reachable("invalid CIL ldelem kind");
    }
}

static SValue::Ptr
resizeToBits(Ops ops, const SValue::Ptr &value, size_t bits)
{
    ASSERT_not_null(ops);
    ASSERT_not_null(value);

    if (value->nBits() < bits)
        return ops->unsignedExtend(value, bits);

    if (value->nBits() > bits)
        return ops->extract(value, 0, bits);

    return value;
}

static SValue::Ptr
arrayElementAddress(D dispatcher,
                    Ops ops,
                    const SValue::Ptr &arrayref,
                    const SValue::Ptr &index,
                    size_t elementBytes)
   {
     ASSERT_not_null(dispatcher);
     ASSERT_not_null(ops);
     ASSERT_not_null(arrayref);
     ASSERT_not_null(index);

#if CIL_DEBUG
     std::string arrayref_string = arrayref->toString();
     std::string index_string    = index->toString();

     fprintf (stderr,"In arrayElementAddress: arrayref_string = %s \n",arrayref_string.c_str());
     fprintf (stderr,"In arrayElementAddress: index_string    = %s \n",index_string.c_str());
     fflush(stderr);
#endif

     const size_t ptrBits = CilSemantics::cilPointerBits(dispatcher);

     SValue::Ptr normalizedArray = arrayref;
     if (normalizedArray->nBits() < ptrBits)
        {
          normalizedArray = ops->unsignedExtend(normalizedArray, ptrBits);
        }
       else
          if (normalizedArray->nBits() > ptrBits)
             {
               normalizedArray = ops->extract(normalizedArray, 0, ptrBits);
             }

     SValue::Ptr normalizedIndex = index;
     if (normalizedIndex->nBits() < ptrBits)
        {
          normalizedIndex = ops->signExtend(normalizedIndex, ptrBits);
        }
       else
          if (normalizedIndex->nBits() > ptrBits)
             {
               normalizedIndex = ops->extract(normalizedIndex, 0, ptrBits);
             }

  // TODO: Replace this with the real array-data offset once the CIL object
  // layout is modeled.  Use a stable nonzero synthetic header size so element
  // zero does not alias the array object reference/base address itself.
     static const uint64_t CIL_ARRAY_DATA_OFFSET = 0x20;

     SValue::Ptr base   = ops->add(normalizedArray, ops->number_(ptrBits, CIL_ARRAY_DATA_OFFSET));
     SValue::Ptr offset = ops->unsignedMultiply(normalizedIndex, ops->number_(ptrBits, elementBytes));

#if CIL_DEBUG
     std::string offset_string_1          = offset->toString();
     fprintf (stderr,"In arrayElementAddress: offset_string          = %s \n",offset_string_1.c_str());
     fflush(stderr);
#endif

  // Important: unsignedMultiply may widen the result.
  // Address arithmetic needs pointer-width operands.
     offset = CilSemantics::resizeToBits(ops, offset, ptrBits);

#if CIL_DEBUG
     SValue::Ptr number_1               = ops->number_(ptrBits, elementBytes);
     SValue::Ptr number_2               = ops->number_(ptrBits, CIL_ARRAY_DATA_OFFSET);
     std::string number_1_string        = number_1->toString();
     std::string number_2_string        = number_2->toString();
     std::string normalizedArray_string = normalizedArray->toString();
     std::string normalizedIndex_string = normalizedIndex->toString();
     std::string base_string            = base->toString();
     std::string offset_string_2        = offset->toString();

     fprintf (stderr,"In arrayElementAddress: number_1_string        = %s \n",number_1_string.c_str());
     fprintf (stderr,"In arrayElementAddress: number_2_string        = %s \n",number_2_string.c_str());
     fprintf (stderr,"In arrayElementAddress: ptrBits                = %zu \n",ptrBits);
     fprintf (stderr,"In arrayElementAddress: elementBytes           = %zu \n",elementBytes);
     fprintf (stderr,"In arrayElementAddress: normalizedArray_string = %s \n",normalizedArray_string.c_str());
     fprintf (stderr,"In arrayElementAddress: normalizedIndex_string = %s \n",normalizedIndex_string.c_str());
     fprintf (stderr,"In arrayElementAddress: base_string            = %s \n",base_string.c_str());
     fprintf (stderr,"In arrayElementAddress: offset_string_2        = %s \n",offset_string_2.c_str());
     fflush(stderr);
#endif

     return ops->add(base, offset);
   }

static void
loadArrayElement(D dispatcher, Ops ops, Element_Load_Kind_Type kind)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);

    // Stack:
    //   ..., array, index -> ..., value
    SValue::Ptr index = ops->popOperand();
    SValue::Ptr arrayref = ops->popOperand();

    ASSERT_not_null(index);
    ASSERT_not_null(arrayref);

    const ElementLoadDescriptor desc =
        CilSemantics::elementLoadDescriptor(dispatcher, kind);

    const size_t elementBytes = (desc.elementBits + 7) / 8;

    // Optional sanity check. Do not hard-assert until all array-producing
    // instructions reliably set ArrayReference.
    if (arrayref->kind() != ValueKind::ArrayReference) {
        // TODO: expected ArrayReference. Continue conservatively.
    }

    if (CilSemantics::arrayAccessDefinitelyInvalid(dispatcher, ops, arrayref, index))
        return;

    SValue::Ptr addr = CilSemantics::arrayElementAddress(dispatcher, ops, arrayref, index, elementBytes);

    SValue::Ptr loaded = ops->readMemory(
        RegisterDescriptor(),
        addr,
        ops->undefined_(desc.elementBits),
        ops->number_(1, 1));

    ASSERT_not_null(loaded);

    if (loaded->nBits() < desc.stackBits) {
        if (desc.signExtend) {
            loaded = ops->signExtend(loaded, desc.stackBits);
        } else {
            loaded = ops->unsignedExtend(loaded, desc.stackBits);
        }
    } else if (loaded->nBits() > desc.stackBits) {
        loaded = ops->extract(loaded, 0, desc.stackBits);
    }

    loaded->kind(desc.resultKind);

    if (kind == ldelem_ref_kind) {
        dispatcher->typeAnalysis().setNullness(
            loaded, DispatcherCil::CilTypeAnalysis::MaybeNull);
    }

    ops->pushOperand(loaded);
}


static ElementStoreDescriptor
elementStoreDescriptor(D dispatcher, Element_Store_Kind_Type kind)
{
    const size_t ptrBits = CilSemantics::cilPointerBits(dispatcher);

    switch (kind) {
        case stelem_i_kind:
            return ElementStoreDescriptor{ptrBits, ValueKind::NativeInt};

        case stelem_i1_kind:
            return ElementStoreDescriptor{8, ValueKind::Integer32};

        case stelem_i2_kind:
            return ElementStoreDescriptor{16, ValueKind::Integer32};

        case stelem_i4_kind:
            return ElementStoreDescriptor{32, ValueKind::Integer32};

        case stelem_i8_kind:
            return ElementStoreDescriptor{64, ValueKind::Integer64};

        case stelem_r4_kind:
            return ElementStoreDescriptor{32, ValueKind::Float32};

        case stelem_r8_kind:
            return ElementStoreDescriptor{64, ValueKind::Float64};

        case stelem_ref_kind:
            return ElementStoreDescriptor{ptrBits, ValueKind::ObjectReference};

        default:
            ASSERT_not_reachable("invalid CIL stelem kind");
    }
}


static SValue::Ptr
resizeArrayElementValueForStore(Ops ops,
                                const SValue::Ptr &value,
                                size_t elementBits)
{
    ASSERT_not_null(ops);
    ASSERT_not_null(value);

    if (value->nBits() < elementBits)
        return ops->unsignedExtend(value, elementBits);

    if (value->nBits() > elementBits)
        return ops->extract(value, 0, elementBits);

    return value;
}

static bool
isReferenceLikeKind(ValueKind kind)
{
    return kind == ValueKind::ObjectReference || kind == ValueKind::ArrayReference;
}

static bool
arrayElementTypeFromReference(D dispatcher,
                              const SValue::Ptr &arrayref,
                              DispatcherCil::CilTypeAnalysis::TypeDescriptor &elementType)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(arrayref);

    if (!arrayref->hasTypeDescriptor())
        return false;

    uint32_t typeToken = 0;
    if (!CilSemantics::cilTypeTokenFromDescriptor(arrayref->typeDescriptor(), typeToken))
        return false;

    elementType = CilSemantics::resolveType(dispatcher, typeToken);
    return elementType.isValid();
}

static void
arrayTypeMismatchExceptionPath(D dispatcher, Ops ops)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);

    SValue::Ptr exceptionObject = ops->undefined_(CilSemantics::cilPointerBits(dispatcher));
    exceptionObject->kind(ValueKind::ObjectReference);
    exceptionObject->typeDescriptor("cil:exception:System.ArrayTypeMismatchException");
    dispatcher->typeAnalysis().setNullness(
        exceptionObject, DispatcherCil::CilTypeAnalysis::DefinitelyNonNull);
    CilSemantics::recordCurrentException(dispatcher, ops, exceptionObject);
    CilSemantics::writeUnknownInstructionPointer(dispatcher, ops);
}

static void
markArrayElementStoreValueFacts(D dispatcher,
                                const SValue::Ptr &storedValue,
                                const SValue::Ptr &originalValue,
                                ValueKind kind,
                                const DispatcherCil::CilTypeAnalysis::TypeDescriptor *type = nullptr)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(storedValue);
    ASSERT_not_null(originalValue);

    if (storedValue != originalValue)
        dispatcher->typeAnalysis().copyFacts(storedValue, originalValue);

    storedValue->kind(kind);

    if (type != nullptr && type->metadataToken != 0)
        storedValue->typeDescriptor(CilSemantics::cilTypeTokenDescriptor(type->metadataToken));

    if (CilSemantics::isReferenceLikeKind(kind) &&
        dispatcher->typeAnalysis().nullness(storedValue) == DispatcherCil::CilTypeAnalysis::NullnessUnknown) {
        dispatcher->typeAnalysis().setNullness(storedValue, DispatcherCil::CilTypeAnalysis::MaybeNull);
    }
}

static void
storeArrayElement(D dispatcher, Ops ops, Element_Store_Kind_Type kind)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);

    // Stack:
    //   ..., array, index, value -> ...
    SValue::Ptr value = ops->popOperand();
    SValue::Ptr index = ops->popOperand();
    SValue::Ptr arrayref = ops->popOperand();

    ASSERT_not_null(value);
    ASSERT_not_null(index);
    ASSERT_not_null(arrayref);

    const ElementStoreDescriptor desc =
        CilSemantics::elementStoreDescriptor(dispatcher, kind);

    const size_t elementBytes = (desc.elementBits + 7) / 8;

    if (arrayref->kind() != ValueKind::ArrayReference) {
        // TODO: expected ArrayReference. Keep going conservatively during
        // bring-up because earlier semantics may still produce Unknown.
    }

    if (CilSemantics::arrayAccessDefinitelyInvalid(dispatcher, ops, arrayref, index))
        return;

    if (kind == stelem_ref_kind &&
        !dispatcher->typeAnalysis().isDefinitelyNull(value)) {
        DispatcherCil::CilTypeAnalysis::TypeDescriptor elementType;
        if (CilSemantics::arrayElementTypeFromReference(dispatcher, arrayref, elementType)) {
            DispatcherCil::CilTypeAnalysis::TypeSet possibleTypes =
                dispatcher->typeAnalysis().possibleDynamicTypes(value);
            if (dispatcher->typeAnalysis().noneAssignableTo(possibleTypes, &elementType)) {
                if (dispatcher->typeAnalysis().isDefinitelyNonNull(value)) {
                    CilSemantics::arrayTypeMismatchExceptionPath(dispatcher, ops);
                    return;
                }

                // A null reference value is always assignable to a
                // reference-typed array element.  If this value may be null,
                // the mismatch is only a possible edge, so do not write an
                // active ArrayTypeMismatchException into the merged normal
                // store state.
                ops->comment("CIL stelem.ref: possible ArrayTypeMismatchException edge not split");
            }
        }
    }

    SValue::Ptr addr = CilSemantics::arrayElementAddress(dispatcher, ops, arrayref, index, elementBytes);

    SValue::Ptr storedValue =
        CilSemantics::resizeArrayElementValueForStore(ops, value, desc.elementBits);
    CilSemantics::markArrayElementStoreValueFacts(dispatcher, storedValue, value, desc.expectedKind);

    ops->writeMemory(RegisterDescriptor(), addr, storedValue, ops->number_(1, 1));
}


static ElementLoadDescriptor
typedElementLoadDescriptor(D dispatcher,
                           const DispatcherCil::CilTypeAnalysis::TypeDescriptor &type)
{
    const size_t ptrBits = CilSemantics::cilPointerBits(dispatcher);

 // if (type.isReferenceType && !type.isValueType)
    if (type.isKnownReferenceType())
      {
        return ElementLoadDescriptor{ptrBits, ptrBits, false, false,
                                     CilSemantics::typeStackValueKind(type, ptrBits)};
    }

 // if (type.valueBitWidth != 0)
    if (type.hasKnownValueWidth())
      {
        ValueKind kind = ValueKind::Unknown;

        if (type.valueBitWidth <= 32)
            kind = ValueKind::Integer32;
        else if (type.valueBitWidth == 64)
            kind = ValueKind::Integer64;

        return ElementLoadDescriptor{type.valueBitWidth,
                                     type.valueBitWidth <= 32 ? 32 : type.valueBitWidth,
                                     false,
                                     false,
                                     kind};
    }

    // Metadata unavailable. Conservative default.
    return ElementLoadDescriptor{32, 32, false, false, ValueKind::Unknown};
}

static ElementStoreDescriptor
typedElementStoreDescriptor(D dispatcher,
                            const DispatcherCil::CilTypeAnalysis::TypeDescriptor &type)
{
    const size_t ptrBits = CilSemantics::cilPointerBits(dispatcher);

 // if (type.isReferenceType && !type.isValueType)
    if (type.isKnownReferenceType())
        return ElementStoreDescriptor{ptrBits, CilSemantics::typeStackValueKind(type, ptrBits)};

 // if (type.valueBitWidth != 0)
    if (type.hasKnownValueWidth())
      {
        ValueKind kind = ValueKind::Unknown;

        if (type.valueBitWidth <= 32)
            kind = ValueKind::Integer32;
        else if (type.valueBitWidth == 64)
            kind = ValueKind::Integer64;

        return ElementStoreDescriptor{type.valueBitWidth, kind};
    }

    return ElementStoreDescriptor{32, ValueKind::Unknown};
}

static void
loadTypedArrayElement(D dispatcher,
                      Ops ops,
                      const DispatcherCil::CilTypeAnalysis::TypeDescriptor &type)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);

    SValue::Ptr index = ops->popOperand();
    SValue::Ptr arrayref = ops->popOperand();

    ASSERT_not_null(index);
    ASSERT_not_null(arrayref);

    const ElementLoadDescriptor desc =
        CilSemantics::typedElementLoadDescriptor(dispatcher, type);

    const size_t elementBytes = (desc.elementBits + 7) / 8;

    if (CilSemantics::arrayAccessDefinitelyInvalid(dispatcher, ops, arrayref, index))
        return;

    SValue::Ptr addr = CilSemantics::arrayElementAddress(dispatcher, ops, arrayref, index, elementBytes);

    SValue::Ptr loaded = ops->readMemory(
        RegisterDescriptor(),
        addr,
        ops->undefined_(desc.elementBits),
        ops->number_(1, 1));

    ASSERT_not_null(loaded);

    if (loaded->nBits() < desc.stackBits)
        loaded = ops->unsignedExtend(loaded, desc.stackBits);
    else if (loaded->nBits() > desc.stackBits)
        loaded = ops->extract(loaded, 0, desc.stackBits);

    loaded->kind(desc.resultKind);

    if (type.metadataToken != 0)
        loaded->typeDescriptor(CilSemantics::cilTypeTokenDescriptor(type.metadataToken));

    if (desc.resultKind == ValueKind::ObjectReference ||
        desc.resultKind == ValueKind::ArrayReference) {
        dispatcher->typeAnalysis().setNullness(
            loaded, DispatcherCil::CilTypeAnalysis::MaybeNull);
    }

    ops->pushOperand(loaded);
}


static void
storeTypedArrayElement(D dispatcher,
                       Ops ops,
                       const DispatcherCil::CilTypeAnalysis::TypeDescriptor &type)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);

    SValue::Ptr value = ops->popOperand();
    SValue::Ptr index = ops->popOperand();
    SValue::Ptr arrayref = ops->popOperand();

    ASSERT_not_null(value);
    ASSERT_not_null(index);
    ASSERT_not_null(arrayref);

    const ElementStoreDescriptor desc =
        CilSemantics::typedElementStoreDescriptor(dispatcher, type);

    const size_t elementBytes = (desc.elementBits + 7) / 8;

    if (CilSemantics::arrayAccessDefinitelyInvalid(dispatcher, ops, arrayref, index))
        return;

    SValue::Ptr addr = CilSemantics::arrayElementAddress(dispatcher, ops, arrayref, index, elementBytes);

    SValue::Ptr storedValue = CilSemantics::resizeArrayElementValueForStore(ops, value, desc.elementBits);
    CilSemantics::markArrayElementStoreValueFacts(dispatcher, storedValue, value, desc.expectedKind, &type);

    ops->writeMemory(RegisterDescriptor(), addr, storedValue, ops->number_(1, 1));
}


static CilConversionTarget
overflowSignedConversionTarget(D dispatcher, Convert_Kind_Type kind)
   {
     const size_t ptrBits = CilSemantics::cilPointerBits(dispatcher);

     switch (kind)
        {
        case conv_ovf_i1_kind:
            return CilConversionTarget{8, 32, true, ValueKind::Integer32};

        case conv_ovf_u1_kind:
            return CilConversionTarget{8, 32, false, ValueKind::Integer32};

        case conv_ovf_i2_kind:
            return CilConversionTarget{16, 32, true, ValueKind::Integer32};

        case conv_ovf_u2_kind:
            return CilConversionTarget{16, 32, false, ValueKind::Integer32};

        case conv_ovf_i4_kind:
            return CilConversionTarget{32, 32, true, ValueKind::Integer32};

        case conv_ovf_u4_kind:
            return CilConversionTarget{32, 32, false, ValueKind::Integer32};

        case conv_ovf_i8_kind:
            return CilConversionTarget{64, 64, true, ValueKind::Integer64};

        case conv_ovf_u8_kind:
            return CilConversionTarget{64, 64, false, ValueKind::Integer64};

        case conv_ovf_i_kind:
            return CilConversionTarget{ptrBits, ptrBits, true, ValueKind::NativeInt};

        case conv_ovf_u_kind:
            return CilConversionTarget{ptrBits, ptrBits, false, ValueKind::NativeInt};

        default:
            ASSERT_not_reachable("invalid signed overflow conversion kind");
        }
   }

#if 0
static SValue::Ptr
convertOverflowSignedValue(D dispatcher, Ops ops, I insn,
                           Convert_Kind_Type kind,
                           const SValue::Ptr &value)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);
    ASSERT_not_null(value);

    const CilConversionTarget target =
        CilSemantics::overflowSignedConversionTarget(dispatcher, kind);

    SValue::Ptr result = value;

    // Floating-source conversions require numeric float-to-integer conversion.
    // Do not reinterpret IEEE bits as an integer.
    if (isFloatLike(value->kind())) {
        result = ops->undefined_(target.stackBits);
        result->kind(target.resultKind);
        return result;
    }

    // For non-.un overflow conversions, the source is interpreted as signed.
    // This models result width/kind but does not yet model OverflowException.
    if (result->nBits() > target.targetBits) {
        result = ops->extract(result, 0, target.targetBits);
    } else if (result->nBits() < target.targetBits) {
        result = ops->signExtend(result, target.targetBits);
    }

    // Convert from target width to CIL stack width.
    if (result->nBits() < target.stackBits) {
        if (target.targetIsSigned)
            result = ops->signExtend(result, target.stackBits);
        else
            result = ops->unsignedExtend(result, target.stackBits);
    } else if (result->nBits() > target.stackBits) {
        result = ops->extract(result, 0, target.stackBits);
    }

    result->kind(target.resultKind);
    return result;
}
#else
static SValue::Ptr
convertOverflowSignedValue(D dispatcher,
                           Ops ops,
                           I insn,
                           Convert_Kind_Type kind,
                           const SValue::Ptr &value)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);
    ASSERT_not_null(value);

    const CilConversionTarget target =
        CilSemantics::overflowSignedConversionTarget(dispatcher, kind);

    // Floating-source conversions require real numeric float-to-integer
    // conversion. Do not reinterpret IEEE bits.
    if (isFloatLike(value->kind())) {
        bool overflows = false;
        SValue::Ptr result =
            CilSemantics::concreteFloatToIntegerConversion(ops, value, target, overflows);
        if (result != NULL)
            return result;

        if (overflows) {
            CilSemantics::conversionOverflowExceptionEffect(dispatcher, ops, insn, kind);
            return SValue::Ptr();                    // no normal result
        }

        ops->comment(
            "CIL " + convertKindToString(kind) +
            ": symbolic float-to-integer checked conversion; overflow status unknown");
        result = ops->undefined_(target.stackBits);
        result->kind(target.resultKind);
        return result;
    }

    // Non-.un form: interpret the original source as signed.
    if (concreteSignedSourceConversionOverflows(value, target)) {
        CilSemantics::conversionOverflowExceptionEffect(dispatcher, ops, insn, kind);
        return SValue::Ptr();                    // no normal result
    }

    // Symbolic values may overflow on some paths. Do not write an active
    // OverflowException into the merged normal state; compute the normal
    // abstract result and record the imprecision as a comment.
    if (!value->toSigned())
       {
         ops->comment("CIL " + convertKindToString(kind) +
                      ": symbolic checked conversion; overflow status unknown");
       }

    return finishOverflowConversion(ops, value, target, false);
}
#endif

static void
loadToken(D dispatcher, Ops ops, I /*insn*/, Args args)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);
 // assert_args(insn, args, 1);

    const uint32_t token = CilSemantics::asIndex(args[0]);
    const size_t ptrBits = CilSemantics::cilPointerBits(dispatcher);

    if (!CilSemantics::isLdtokenMetadataTable(token)) {
        CilSemantics::invalidProgramExceptionPath(
            dispatcher, ops,
            "CIL ldtoken: invalid metadata token table; InvalidProgramException path not fully modeled");
        return;
    }

    SValue::Ptr handle = ops->number_(ptrBits, token);

    // RuntimeTypeHandle / RuntimeFieldHandle / RuntimeMethodHandle are
    // modeled as native-sized handles.
    handle->kind(ValueKind::NativeInt);
    handle->typeDescriptor(CilSemantics::cilRuntimeHandleDescriptor(token));

    ops->pushOperand(handle);
}


static void
endFinallyEffect(D dispatcher, Ops ops)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);

    // Stack:
    //   ..., -> ...
    //
    // endfinally/endfault transfers control to the pending continuation:
    //   - the instruction after the protected region for normal leave,
    //   - the next enclosing handler/finally,
    //   - or the original exception propagation target.
    //
    // We do not yet model the hidden CLR exception/finally continuation state,
    // so conservatively mark the next instruction pointer as unknown.
    CilSemantics::writeUnknownInstructionPointer(dispatcher, ops);
}

static void
leaveEffect(D dispatcher, Ops ops, I insn, Args args)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);
 // assert_args(insn, args, 1);

    // Stack:
    //   ..., -> ...
    //
    // CIL leave transfers control outside a protected region. If there are
    // intervening finally/fault handlers, control first transfers to those
    // handlers and only later reaches the leave target.
    //
    // Until exception-region/finally-continuation state is modeled, use the
    // decoded branch target like a normal branch. This is precise for leave
    // instructions not crossing a finally/fault boundary and conservative
    // enough for initial control-flow semantics.
    //
    // Unlike br, leave empties the evaluation stack before transferring
    // control.  BaseSemantics::FrameState::clear() clears only the operand
    // stack, not the CIL local slots.
    State::Ptr state = ops->currentState();
    ASSERT_not_null(state);
    FrameState::Ptr frame = state->frameState();
    if (frame) {
        frame->clear();
    } else {
        ops->comment("CIL leave: no frame state available; evaluation stack clear not modeled");
    }

    CilSemantics::branchEffect(dispatcher, ops, insn, args);
}


static SValue::Ptr
runtimeArgumentHandle(D dispatcher, Ops ops)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);

    const size_t ptrBits = CilSemantics::cilPointerBits(dispatcher);

    SValue::Ptr result = ops->undefined_(ptrBits);
    result->kind(ValueKind::NativeInt);
    result->typeDescriptor("cil:RuntimeArgumentHandle");

    return result;
}


static SValue::Ptr
booleanResultToInt32(Ops ops, const SValue::Ptr &cond)
{
    ASSERT_not_null(ops);
    ASSERT_not_null(cond);

    SValue::Ptr result = cond;

    if (result->nBits() < 32)
        result = ops->unsignedExtend(result, 32);
    else if (result->nBits() > 32)
        result = ops->extract(result, 0, 32);

    result->kind(ValueKind::Integer32);
    return result;
}

static void
compareResult(Ops ops, Compare_Op_Kind_Type kind)
{
    ASSERT_not_null(ops);

    // Stack:
    //
    //   ..., value1, value2 -> ..., int32_result
    //
    // value2 is on top of the stack.
    //
    // Result:
    //
    //   1 if value1 <op> value2
    //   0 otherwise
    //
    // The result is always int32.

    SValue::Ptr rhs = ops->popOperand(); // value2
    SValue::Ptr lhs = ops->popOperand(); // value1

    ASSERT_not_null(lhs);
    ASSERT_not_null(rhs);

    const ValueKind lhsKind = lhs->kind();
    const ValueKind rhsKind = rhs->kind();

    SValue::Ptr cond = ops->undefined_(1);

    auto normalizeUnsignedPair =
        [&ops](SValue::Ptr &a, SValue::Ptr &b) {
            ASSERT_not_null(a);
            ASSERT_not_null(b);

            const size_t width = std::max(a->nBits(), b->nBits());

            if (a->nBits() < width) {
                a = ops->unsignedExtend(a, width);
            } else if (a->nBits() > width) {
                a = ops->extract(a, 0, width);
            }

            if (b->nBits() < width) {
                b = ops->unsignedExtend(b, width);
            } else if (b->nBits() > width) {
                b = ops->extract(b, 0, width);
            }
        };

    auto normalizeSignedPair =
        [&ops](SValue::Ptr &a, SValue::Ptr &b) {
            ASSERT_not_null(a);
            ASSERT_not_null(b);

            const size_t width = std::max(a->nBits(), b->nBits());

            if (a->nBits() < width) {
                a = ops->signExtend(a, width);
            } else if (a->nBits() > width) {
                a = ops->extract(a, 0, width);
            }

            if (b->nBits() < width) {
                b = ops->signExtend(b, width);
            } else if (b->nBits() > width) {
                b = ops->extract(b, 0, width);
            }
        };

    switch (kind) {
        case compare_eq: {
            if ((isIntegerLike(lhsKind) && isIntegerLike(rhsKind)) ||
                (isReferenceLike(lhsKind) && isReferenceLike(rhsKind))) {
                SValue::Ptr a = lhs;
                SValue::Ptr b = rhs;
                normalizeUnsignedPair(a, b);

                cond = ops->isEqual(a, b);
            } else if (isFloatLike(lhsKind) && isFloatLike(rhsKind)) {
                Sawyer::Optional<bool> concrete =
                    CilSemantics::concreteFloatComparison(lhs, rhs, float_compare_eq);
                cond = concrete ? CilSemantics::concreteBoolean(ops, *concrete) : ops->undefined_(1);
            } else {
                // Floating-point ceq has ordered floating-point semantics.
                // Symbolic or non-floating unknown values remain conservative.
                cond = ops->undefined_(1);
            }
            break;
        }

        case compare_gt: {
            if (isIntegerLike(lhsKind) && isIntegerLike(rhsKind)) {
                SValue::Ptr a = lhs;
                SValue::Ptr b = rhs;
                normalizeSignedPair(a, b);

                cond = ops->isSignedGreaterThan(a, b);
            } else if (isFloatLike(lhsKind) && isFloatLike(rhsKind)) {
                Sawyer::Optional<bool> concrete =
                    CilSemantics::concreteFloatComparison(lhs, rhs, float_compare_gt);
                cond = concrete ? CilSemantics::concreteBoolean(ops, *concrete) : ops->undefined_(1);
            } else {
                // Floating-point cgt requires float-aware ordered comparison.
                // Symbolic or non-floating unknown values remain conservative.
                cond = ops->undefined_(1);
            }
            break;
        }

        case compare_lt: {
            if (isIntegerLike(lhsKind) && isIntegerLike(rhsKind)) {
                SValue::Ptr a = lhs;
                SValue::Ptr b = rhs;
                normalizeSignedPair(a, b);

                cond = ops->isSignedLessThan(a, b);
            } else if (isFloatLike(lhsKind) && isFloatLike(rhsKind)) {
                Sawyer::Optional<bool> concrete =
                    CilSemantics::concreteFloatComparison(lhs, rhs, float_compare_lt);
                cond = concrete ? CilSemantics::concreteBoolean(ops, *concrete) : ops->undefined_(1);
            } else {
                // Floating-point clt requires float-aware ordered comparison.
                // Symbolic or non-floating unknown values remain conservative.
                cond = ops->undefined_(1);
            }
            break;
        }

        case compare_gt_un: {
            if (isIntegerLike(lhsKind) && isIntegerLike(rhsKind)) {
                SValue::Ptr a = lhs;
                SValue::Ptr b = rhs;
                normalizeUnsignedPair(a, b);

                cond = ops->isUnsignedGreaterThan(a, b);
            } else if (isFloatLike(lhsKind) && isFloatLike(rhsKind)) {
                Sawyer::Optional<bool> concrete =
                    CilSemantics::concreteFloatComparison(lhs, rhs, float_compare_gt_un);
                cond = concrete ? CilSemantics::concreteBoolean(ops, *concrete) : ops->undefined_(1);
            } else {
                // Floating-point cgt.un is unordered-aware:
                // result is true if unordered or greater-than.
                // Symbolic or non-floating unknown values remain conservative.
                cond = ops->undefined_(1);
            }
            break;
        }

        case compare_lt_un: {
            if (isIntegerLike(lhsKind) && isIntegerLike(rhsKind)) {
                SValue::Ptr a = lhs;
                SValue::Ptr b = rhs;
                normalizeUnsignedPair(a, b);

                cond = ops->isUnsignedLessThan(a, b);
            } else if (isFloatLike(lhsKind) && isFloatLike(rhsKind)) {
                Sawyer::Optional<bool> concrete =
                    CilSemantics::concreteFloatComparison(lhs, rhs, float_compare_lt_un);
                cond = concrete ? CilSemantics::concreteBoolean(ops, *concrete) : ops->undefined_(1);
            } else {
                // Floating-point clt.un is unordered-aware:
                // result is true if unordered or less-than.
                // Symbolic or non-floating unknown values remain conservative.
                cond = ops->undefined_(1);
            }
            break;
        }

        default:
            ASSERT_not_reachable("invalid CIL compare kind");
    }

    ops->pushOperand(CilSemantics::booleanResultToInt32(ops, cond));
}


static SValue::Ptr
methodFunctionPointer(D dispatcher, Ops ops, uint32_t methodToken)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);

    const size_t ptrBits = CilSemantics::cilPointerBits(dispatcher);

    // Use a concrete token-derived native-sized value so repeated ldftn of
    // the same method token is stable.
    SValue::Ptr result = ops->number_(ptrBits, methodToken);

    result->kind(ValueKind::UnmanagedPointer);
    result->typeDescriptor(
        "cil:method-function-pointer:" + StringUtility::addrToString(methodToken));

    return result;
}


static uint32_t
cilInlineIndex(Args args)
{
    return CilSemantics::asIndex(args[0]);
}

#if 0
     static void
     loadArgument(Ops ops, uint32_t argIndex)
        {
          ASSERT_not_null(ops);

          SValue::Ptr value = ops->readLocal(cilArgumentSlot(argIndex));
          if (value == NULL)
             {
            // value = ops->undefined_(32);
               value = ops->undefined_(CilSemantics::cilPointerBits(dispatcher));
               value->kind(ValueKind::Unknown);
               value->typeDescriptor("cil:unknown-argument");
             }

          ops->pushOperand(value);
        }
#else
     static void
     loadArgument(D dispatcher, Ops ops, uint32_t argIndex)
        {
          ASSERT_not_null(dispatcher);
          ASSERT_not_null(ops);

          SValue::Ptr value = ops->readLocal(cilArgumentSlot(argIndex));

          if (value == NULL)
             {
               const size_t ptrBits = CilSemantics::cilPointerBits(dispatcher);

               value = ops->undefined_(ptrBits);
               value->kind(ValueKind::Unknown);
               value->typeDescriptor("cil:unknown-argument");
            }

          ops->pushOperand(value);
        }
#endif

#if 0
static void
storeArgument(D dispatcher, Ops ops, uint32_t argIndex)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);

    SValue::Ptr value = ops->popOperand();
    ASSERT_not_null(value);

    writeCilArgument(dispatcher, ops, argIndex, value);
}
#else
static void
storeArgument(D dispatcher, Ops ops, uint32_t argIndex)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);

    SValue::Ptr value = ops->popOperand();
    ASSERT_not_null(value);

    writeCilArgument(dispatcher, ops, argIndex, value);
}
#endif

static void
materializeArgumentAddress(D dispatcher, Ops ops, uint32_t argIndex)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);

    SValue::Ptr addr = CilSemantics::argumentAddress(dispatcher, ops, argIndex);
    SValue::Ptr value = ops->readLocal(cilArgumentSlot(argIndex));

    if (value != NULL) {
        ops->writeMemory(RegisterDescriptor(), addr, value, ops->number_(1, 1));
    }

    ops->pushOperand(addr);
}

static void
materializeLocalAddress(D dispatcher, Ops ops, uint32_t localIndex)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);

    SValue::Ptr addr = CilSemantics::localAddress(dispatcher, ops, localIndex);
    SValue::Ptr value = ops->readLocal(cilLocalSlot(localIndex));

    if (value != NULL) {
        ops->writeMemory(RegisterDescriptor(), addr, value, ops->number_(1, 1));
    }

    ops->pushOperand(addr);
}

#if 0
static void
storeLocal(D dispatcher, Ops ops, uint32_t localIndex)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);

    SValue::Ptr value = ops->popOperand();
    ASSERT_not_null(value);

    writeCilLocal(dispatcher, ops, localIndex, value);

    SValue::Ptr addr = CilSemantics::localAddress(dispatcher, ops, localIndex);
    ASSERT_not_null(addr);

    ops->writeMemory(RegisterDescriptor(), addr, value, ops->number_(1, 1));
}
#else
static void
storeLocal(D dispatcher, Ops ops, uint32_t localIndex)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);

    SValue::Ptr value = ops->popOperand();
    ASSERT_not_null(value);

    writeCilLocal(dispatcher, ops, localIndex, value);
}
#endif

static void
unalignedPrefixEffect(D dispatcher, Ops ops, I /*insn*/, Args args)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);
 // assert_args(insn, args, 1);

    const uint32_t alignment = CilSemantics::asIndex(args[0]);

    dispatcher->pendingPrefix().hasUnaligned = true;
    dispatcher->pendingPrefix().unalignedBytes = static_cast<uint8_t>(alignment);

    // No stack effect.
}


static void
volatilePrefixEffect(D dispatcher, Ops ops)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);

    dispatcher->pendingPrefix().isVolatile = true;

    // No stack effect.
}

static void
tailPrefixEffect(D dispatcher, Ops ops)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);

    dispatcher->pendingPrefix().isTail = true;

    // No stack effect.
}

static void
constrainedPrefixEffect(D dispatcher, Ops ops, I /*insn*/, Args args)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);
 // assert_args(insn, args, 1);

    const uint32_t typeToken = CilSemantics::asIndex(args[0]);

    dispatcher->pendingPrefix().hasConstrained = true;
    dispatcher->pendingPrefix().constrainedTypeToken = typeToken;

    // No stack effect.
}

static void
endFilterEffect(D dispatcher, Ops ops)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);

    // Stack:
    //   ..., filterResult -> ...
    SValue::Ptr filterResult = ops->popOperand();
    ASSERT_not_null(filterResult);

    const ValueKind filterKind = filterResult->kind();
    if (!CilSemantics::isIntegerLike(filterKind) && filterKind != ValueKind::Unknown) {
        CilSemantics::invalidProgramExceptionPath(
            dispatcher, ops,
            "CIL endfilter: non-integer filter result; InvalidProgramException path not fully modeled");
        return;
    }

    // A precise model would use filterResult:
    //   0 -> continue searching for a handler
    //   nonzero -> enter this exception handler
    //
    // Until exception-filter dispatch is modeled, normal control flow becomes
    // unknown. This is better than doNothing(), which incorrectly falls through.
    CilSemantics::writeUnknownInstructionPointer(dispatcher, ops);
}

static size_t
typeStorageBits(D dispatcher, const DispatcherCil::CilTypeAnalysis::TypeDescriptor &type)
{
    const size_t ptrBits = CilSemantics::cilPointerBits(dispatcher);

 // if (type.isReferenceType && !type.isValueType)
    if (type.isKnownReferenceType())
        return ptrBits;

 // if (type.valueBitWidth != 0)
    if (type.hasKnownValueWidth())
        return type.valueBitWidth;

    // Metadata/layout unavailable.
    return 32;
}

static SValue::Ptr
defaultValueForType(D dispatcher,
                    Ops ops,
                    const DispatcherCil::CilTypeAnalysis::TypeDescriptor &type)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);

    const size_t bits = CilSemantics::typeStorageBits(dispatcher, type);

    SValue::Ptr value = ops->number_(bits, 0);

 // if (type.isReferenceType && !type.isValueType)
    if (type.isKnownReferenceType())
      {
        value->kind(ValueKind::ObjectReference);
        dispatcher->typeAnalysis().setNullness(
            value, DispatcherCil::CilTypeAnalysis::DefinitelyNull);
    } else if (bits <= 32) {
        value->kind(ValueKind::Integer32);
    } else if (bits == 64) {
        value->kind(ValueKind::Integer64);
    } else {
        value->kind(ValueKind::Unknown);
    }

    if (type.metadataToken != 0)
        value->typeDescriptor(CilSemantics::cilTypeTokenDescriptor(type.metadataToken));

    return value;
}


static void
initObjectEffect(D dispatcher,
                 Ops ops,
                 const DispatcherCil::CilTypeAnalysis::TypeDescriptor &type)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);

    // Stack:
    //   ..., address -> ...
    SValue::Ptr addr = ops->popOperand();
    ASSERT_not_null(addr);

    addr = CilSemantics::normalizeToPointerBits(dispatcher, ops, addr);

    SValue::Ptr defaultValue =
        CilSemantics::defaultValueForType(dispatcher, ops, type);

    // Route writes through the same synthetic local/argument address handling
    // used by stind.  This keeps initobj through an ldloca/ldarga destination
    // visible in the authoritative CIL local/argument slot, not only in the
    // mirrored synthetic memory cell.
    CilSemantics::writeAddressOrCilSlot(dispatcher, ops, addr, defaultValue, defaultValue->nBits());
}

static SValue::Ptr
normalizeNativeAddress(D dispatcher, Ops ops, SValue::Ptr value)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);
    ASSERT_not_null(value);

    const size_t ptrBits = CilSemantics::cilPointerBits(dispatcher);

    SValue::Ptr normalized = value;

    if (value->nBits() < ptrBits)
        normalized = ops->unsignedExtend(value, ptrBits);
    else if (value->nBits() > ptrBits)
        normalized = ops->extract(value, 0, ptrBits);

    if (normalized != value) {
        normalized->kind(value->kind());
        if (value->hasTypeDescriptor())
            normalized->typeDescriptor(value->typeDescriptor());
        dispatcher->typeAnalysis().copyFacts(normalized, value);
    }

    return normalized;
}

static bool
isConcreteZero(const SValue::Ptr &value)
{
    ASSERT_not_null(value);

    if (Sawyer::Optional<uint64_t> n = value->toUnsigned())
        return *n == 0;

    return false;
}

static SValue::Ptr
byteOffsetAddress(D dispatcher, Ops ops, const SValue::Ptr &base, uint64_t offset)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);
    ASSERT_not_null(base);

    if (offset == 0)
        return base;

    const size_t ptrBits = CilSemantics::cilPointerBits(dispatcher);
    return ops->add(base, ops->number_(ptrBits, offset));
}

static void
copyBlockEffect(D dispatcher, Ops ops)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);

 // DQ (6/27/2026): Added support to handle attributes.
    noteMemoryPrefixSemantics(dispatcher, ops, "cpblk");

    // Stack:
    //   ..., dest, src, size -> ...
    SValue::Ptr size = ops->popOperand();
    SValue::Ptr src  = ops->popOperand();
    SValue::Ptr dst  = ops->popOperand();

    ASSERT_not_null(size);
    ASSERT_not_null(src);
    ASSERT_not_null(dst);

    if (CilSemantics::isConcreteZero(size))
        return;

    src = CilSemantics::normalizeNativeAddress(dispatcher, ops, src);
    dst = CilSemantics::normalizeNativeAddress(dispatcher, ops, dst);

    if (Sawyer::Optional<uint64_t> concreteSize = size->toUnsigned()) {
        static const uint64_t CIL_MAX_EXPLICIT_BLOCK_BYTES = 64;
        if (*concreteSize <= CIL_MAX_EXPLICIT_BLOCK_BYTES) {
            for (uint64_t i = 0; i < *concreteSize; ++i) {
                SValue::Ptr srcByteAddr = CilSemantics::byteOffsetAddress(dispatcher, ops, src, i);
                SValue::Ptr dstByteAddr = CilSemantics::byteOffsetAddress(dispatcher, ops, dst, i);
                SValue::Ptr copied = CilSemantics::readAddressOrCilSlot(dispatcher, ops, srcByteAddr, 8);
                CilSemantics::writeAddressOrCilSlot(dispatcher, ops, dstByteAddr, copied, 8);
            }
            return;
        }

        ops->comment("CIL cpblk: large concrete block summarized by first-byte copy");
    }

    // Conservative fallback: if size is symbolic or too large for explicit
    // byte-by-byte expansion, copy a one-byte summary.
    SValue::Ptr copied = CilSemantics::readAddressOrCilSlot(dispatcher, ops, src, 8);

    CilSemantics::writeAddressOrCilSlot(dispatcher, ops, dst, copied, 8);
}


static void
initBlockEffect(D dispatcher, Ops ops)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);

 // DQ (6/27/2026): Added support to handle attributes.
    noteMemoryPrefixSemantics(dispatcher, ops, "initblk");

    // Stack:
    //   ..., dest, value, size -> ...
    SValue::Ptr size  = ops->popOperand();
    SValue::Ptr value = ops->popOperand();
    SValue::Ptr dst   = ops->popOperand();

    ASSERT_not_null(size);
    ASSERT_not_null(value);
    ASSERT_not_null(dst);

    if (CilSemantics::isConcreteZero(size))
        return;

    dst = CilSemantics::normalizeNativeAddress(dispatcher, ops, dst);

    // initblk stores the low byte of value into each destination byte.
    SValue::Ptr byteValue = value;
    if (byteValue->nBits() > 8)
        byteValue = ops->extract(byteValue, 0, 8);
    else if (byteValue->nBits() < 8)
        byteValue = ops->unsignedExtend(byteValue, 8);

    if (Sawyer::Optional<uint64_t> concreteSize = size->toUnsigned()) {
        static const uint64_t CIL_MAX_EXPLICIT_BLOCK_BYTES = 64;
        if (*concreteSize <= CIL_MAX_EXPLICIT_BLOCK_BYTES) {
            for (uint64_t i = 0; i < *concreteSize; ++i) {
                SValue::Ptr dstByteAddr = CilSemantics::byteOffsetAddress(dispatcher, ops, dst, i);
                CilSemantics::writeAddressOrCilSlot(dispatcher, ops, dstByteAddr, byteValue, 8);
            }
            return;
        }

        ops->comment("CIL initblk: large concrete block summarized by first-byte initialization");
    }

    // Conservative fallback: initialize at least the first byte.
    CilSemantics::writeAddressOrCilSlot(dispatcher, ops, dst, byteValue, 8);
}

static void
rethrowEffect(D dispatcher, Ops ops)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);

    // Stack:
    //   ..., -> ...
    //
    // Precise semantics would resume propagation of the currently handled
    // exception object. Keep the current-exception slot when a preceding throw
    // or handler setup recorded one; otherwise fall back to unknown exceptional
    // control flow.
    //
    // Conservative behavior: normal control flow becomes unknown.
    SValue::Ptr currentException = ops->readLocal(CilSemantics::cilCurrentExceptionSlot());
    if (currentException != NULL) {
        ops->writeLocal(CilSemantics::cilCurrentExceptionSlot(), currentException);
    } else {
        CilSemantics::invalidProgramExceptionPath(
            dispatcher, ops,
            "CIL rethrow: no active exception object; InvalidProgramException path not fully modeled");
        return;
    }

    CilSemantics::writeUnknownInstructionPointer(dispatcher, ops);
}

static void
sizeofResult(D dispatcher,
             Ops ops,
             const DispatcherCil::CilTypeAnalysis::TypeDescriptor &type)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);

    SValue::Ptr result;

 // if (type.valueBitWidth != 0)
    if (type.hasKnownValueWidth())
      {
        const uint64_t bytes = (type.valueBitWidth + 7) / 8;
        result = ops->number_(32, bytes);
    } else
     // if (type.isReferenceType && !type.isValueType)
        if (type.isKnownReferenceType())
          {
        const uint64_t bytes = CilSemantics::cilPointerBits(dispatcher) / 8;
        result = ops->number_(32, bytes);
    } else {
        // Metadata/layout unavailable.
        result = ops->undefined_(32);
    }

    result->kind(ValueKind::Integer32);
    result->typeDescriptor("cil:sizeof-result");

    ops->pushOperand(result);
}

static void
readonlyPrefixEffect(D dispatcher, Ops ops)
{
    ASSERT_not_null(dispatcher);
    ASSERT_not_null(ops);

    // Stack:
    //   ..., -> ...
    //
    // readonly. is a prefix. It modifies interpretation of the following
    // instruction, typically ldelema. It does not directly push or pop.
    dispatcher->pendingPrefix().isReadonly = true;
}

static bool
tryConcreteUnsigned(const SValue::Ptr &value, uint64_t &result)
{
    ASSERT_not_null(value);

    // TODO: Replace this with the proper ROSE SValue concrete extraction API
    // if available in your SValue subclass.
    //
    // Some ROSE SValue implementations have methods like:
    //   value->toUnsigned()
    //   value->get_number()
    //   value->isNumber()
    //
    // Until confirmed, return false so symbolic/unknown selectors use the
    // conservative path.
#if 0
    (void)value;
    result = 0;
    return false;
#else
    if (Sawyer::Optional<uint64_t> n = value->toUnsigned())
       {
         result = *n;
         return true;
       }

    return false;
#endif
    
}

static std::vector<uint64_t>
cilSwitchTargets(I insn, Args args)
{

 // CIL switch operands are a uint32 case count followed by that many int32
 // relative target offsets.  ROSE keeps the decoded count as the first
 // operand, so skip it when constructing the target list.

    ASSERT_not_null(insn);

    std::vector<uint64_t> targets;

    const uint64_t fallThroughVa = insn->get_address() + insn->get_size();

    size_t firstTargetArg = 0;
    if (!args.empty()) {
        if (SgAsmIntegerValueExpression *countExpr = isSgAsmIntegerValueExpression(args[0])) {
            const uint64_t nTargets = countExpr->get_value();
            if (nTargets + 1 == args.size())
                firstTargetArg = 1;
        }
    }

    for (size_t i = firstTargetArg; i < args.size(); ++i) {
        SgAsmExpression *arg = args[i];
        SgAsmIntegerValueExpression *expr = isSgAsmIntegerValueExpression(arg);
        if (expr == NULL)
            continue;

        const int32_t rel =
            static_cast<int32_t>(static_cast<uint32_t>(expr->get_value()));

        targets.push_back(fallThroughVa + static_cast<int64_t>(rel));
    }

    return targets;
}

  // DQ (6/24/2026): This version supports better semantics.
     static void
     switchEffect(D dispatcher, Ops ops, I insn, Args args)
        {
          ASSERT_not_null(dispatcher);
          ASSERT_not_null(ops);
          ASSERT_not_null(insn);

       // CIL switch stack effect:
       //
       //   ..., selector -> ...
       //
       // The selector is an int32 value. If selector is in the range
       //
       //   0 <= selector < number_of_targets
       //
       // then control transfers to targets[selector]. Otherwise control falls
       // through to the next instruction.

          SValue::Ptr selector = ops->popOperand();
          ASSERT_not_null(selector);

          const RegisterDescriptor ipReg = dispatcher->instructionPointerRegister();
          const size_t ipBits = ipReg.nBits();

          const uint64_t fallThroughVa = insn->get_address() + insn->get_size();
          const std::vector<uint64_t> targets = CilSemantics::cilSwitchTargets(insn, args);

#if CIL_DEBUG
          fprintf(stderr,
                  "switchEffect: selector = %s targets = %zu fallThrough = 0x%jx\n",
                  selector->toString().c_str(),
                  targets.size(),
                  static_cast<uintmax_t>(fallThroughVa));
          fflush(stderr);
#endif

       // Normalize the selector to the CLI switch comparison width.
       //
       // The CLI switch instruction consumes an int32 selector. Comparisons against
       // case indices should therefore be performed at 32 bits, not at the PC width.
          SValue::Ptr selector32 = selector;

          if (selector32->nBits() < 32)
             {
               selector32 = ops->unsignedExtend(selector32, 32);
             }
            else 
             {
               if (selector32->nBits() > 32)
                  {
                    selector32 = ops->extract(selector32, 0, 32);
                  }
             }

       // Fast path for concrete selectors after normalization to the CLI
       // int32 selector width.  This is important for wider stack values:
       // switch compares the low 32 bits, not the original value width.
          uint64_t concreteSelector = 0;
          if (CilSemantics::tryConcreteUnsigned(selector32, concreteSelector))
             {
               uint64_t nextVa = fallThroughVa;

               if (concreteSelector < targets.size())
                    nextVa = targets[concreteSelector];

               ops->writeRegister(ipReg, ops->number_(ipBits, nextVa));
               return;
             }

       // Default target is fall-through.
          SValue::Ptr nextPc = ops->number_(ipBits, fallThroughVa);

       // Build:
       //
       //   pc = selector == 0 ? target[0] :
       //        selector == 1 ? target[1] :
       //        ...
       //        fallThrough
       //
       // Construct the chain in reverse order so the resulting expression has the
       // natural source-level order.
          for (size_t i = targets.size(); i > 0; --i)
             {
               const size_t caseIndex = i - 1;

               SValue::Ptr caseValue = ops->number_(32, caseIndex);
               SValue::Ptr cond = ops->isEqual(selector32, caseValue);

               SValue::Ptr targetPc = ops->number_(ipBits, targets[caseIndex]);

               nextPc = ops->ite(cond, targetPc, nextPc);
             }

          ops->writeRegister(ipReg, nextPc);
        }



#if 0
     static size_t stackBitsForKind(D dispatcher, ValueKind k)
        {
          switch (k)
             {
               case ValueKind::Integer32: return 32;
               case ValueKind::Integer64: return 64;
               case ValueKind::NativeInt: return CilSemantics::cilPointerBits(dispatcher);
               default: return 0;
             }
        }
#endif
#if 0
     static SValue::Ptr resizeUnsigned(Ops ops, SValue::Ptr v, size_t bits)
        {
          if (v->nBits() < bits)
               return ops->unsignedExtend(v, bits);

          if (v->nBits() > bits)
               return ops->extract(v, 0, bits);

          return v;
        }
#endif














  // end of namespace declaration for CilSemantics
   }


#if 1
       // virtual void process(const Dispatcher::Ptr &dispatcher_, SgAsmInstruction *insn_) override
          void P::process(const Dispatcher::Ptr &dispatcher_, SgAsmInstruction *insn_)
             {
               DispatcherCil::Ptr dispatcher = DispatcherCil::promote(dispatcher_);
               RiscOperators::Ptr operators = dispatcher->operators();
               SgAsmCilInstruction *insn = isSgAsmCilInstruction(insn_);
               ASSERT_not_null(insn);
               ASSERT_require(insn == operators->currentInstruction());

               dispatcher->advanceInstructionPointer(insn);
               SgAsmExpressionPtrList &operands = insn->get_operandList()->get_operands();
#if 0
            // dispatcher->advanceInstructionPointer(insn);
            // SgAsmExpressionPtrList &operands = insn->get_operandList()->get_operands();
#endif
               const unsigned opcode = CilSemantics::cilRawOpcodeKey(insn);

               const bool isPrefix =
                               opcode == 0xfe12 ||   // unaligned.
                               opcode == 0xfe13 ||   // volatile.
                               opcode == 0xfe14 ||   // tail.
                               opcode == 0xfe16 ||   // constrained.
                               opcode == 0xfe1e;     // readonly.

               operators->comment("executing CIL instruction core");
               p(dispatcher.get(), operators.get(), insn, operands);

            // Prefixes apply to the next real instruction, not to themselves.
            // Therefore, do not clear after executing a prefix. Clear after the
            // immediately following non-prefix instruction has had a chance to consume it.
               if (!isPrefix)
                  {
                    dispatcher->clearPendingPrefix();
                  }
#if 0               
               operators->comment("executing CIL instruction core");
               p(dispatcher.get(), operators.get(), insn, operands);

            // operators->comment("executing CIL instruction core");
            // p(dispatcher.get(), operators.get(), insn, operands);

               if (!isPrefix)
                  {
                    dispatcher->clearPendingPrefix();
                  }
#endif
             }
#endif

  // ***********************************************************************************************
  // ***********************************************************************************************
  //          Implementation of the instruction semantics using the helper functions above
  // ***********************************************************************************************
  // ***********************************************************************************************

  
// nop (0 (0x00))
        // Description:
        //   No operation; may be used to fill space when opcodes are patched.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_nop: P {
    void p(D /*dispatcher*/, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
        CilSemantics::doNothing(ops);
    }
};

// break (1 (0x01))
        // Description:
        //   Signals a breakpoint to an attached debugger.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_break: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
        ops->comment("CIL break: debugger breakpoint/trap; normal continuation not modeled");
        CilSemantics::writeUnknownInstructionPointer(dispatcher, ops);
    }
};

// ldarg_0 (2 (0x02))
        // Description:
        //   Load argument 0 onto the evaluation stack.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ldarg_0: P {
     void p(D dispatcher, Ops ops, I /*insn*/, Args /*args*/) override
      {
     // This would be the more compact implementation using our new helper function.
        ops->pushOperand(CilSemantics::readCilArgument(dispatcher, ops, 0));
      }
};

// ldarg_1 (3 (0x03))
        // Description:
        //   Load argument 1 onto the evaluation stack.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ldarg_1: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // ops->pushOperand(ops->readLocal(1));
        ops->pushOperand(CilSemantics::readCilArgument(dispatcher, ops, 1));
    }
};

// ldarg_2 (4 (0x04))
        // Description:
        //   Load argument 2 onto the evaluation stack.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ldarg_2: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // ops->pushOperand(ops->readLocal(2));
        ops->pushOperand(CilSemantics::readCilArgument(dispatcher, ops, 2));
    }
};

// ldarg_3 (5 (0x05))
        // Description:
        //   Load argument 3 onto the evaluation stack.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ldarg_3: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // ops->pushOperand(ops->readLocal(3));
        ops->pushOperand(CilSemantics::readCilArgument(dispatcher, ops, 3));
    }
};

// ldloc_0 (6 (0x06))
        // Description:
        //   Load local variable 0 onto the evaluation stack.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ldloc_0: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // ops->pushOperand(ops->readLocal(0));
        ops->pushOperand(CilSemantics::readCilLocal(dispatcher, ops, 0));
    }
};

// ldloc_1 (7 (0x07))
        // Description:
        //   Load local variable 1 onto the evaluation stack.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ldloc_1: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // ops->pushOperand(ops->readLocal(1));
        ops->pushOperand(CilSemantics::readCilLocal(dispatcher, ops, 1));
    }
};

// ldloc_2 (8 (0x08))
        // Description:
        //   Load local variable 2 onto the evaluation stack.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ldloc_2: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // ops->pushOperand(ops->readLocal(2));
        ops->pushOperand(CilSemantics::readCilLocal(dispatcher, ops, 2));
    }
};

// ldloc_3 (9 (0x09))
        // Description:
        //   Load local variable 3 onto the evaluation stack.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ldloc_3: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // ops->pushOperand(ops->readLocal(3));
        ops->pushOperand(CilSemantics::readCilLocal(dispatcher, ops, 3));
    }
};

// stloc_0 (10 (0x0a))
        // Description:
        //   Store the top evaluation-stack value into local variable 0.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_stloc_0: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // ops->writeLocal(0, ops->popOperand());
        CilSemantics::storeCilLocal(dispatcher, ops, 0);
    }
};

// stloc_1 (11 (0x0b))
        // Description:
        //   Store the top evaluation-stack value into local variable 1.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_stloc_1: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // ops->writeLocal(1, ops->popOperand());
        CilSemantics::storeCilLocal(dispatcher, ops, 1);
    }
};

// stloc_2 (12 (0x0c))
        // Description:
        //   Store the top evaluation-stack value into local variable 2.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_stloc_2: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // ops->writeLocal(2, ops->popOperand());
        CilSemantics::storeCilLocal(dispatcher, ops, 2);
    }
};

// stloc_3 (13 (0x0d))
        // Description:
        //   Store the top evaluation-stack value into local variable 3.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_stloc_3: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // ops->writeLocal(3, ops->popOperand());
        CilSemantics::storeCilLocal(dispatcher, ops, 3);
    }
};

// ldarg_s (14 (0x0e))
        // Description:
        //   Load argument s onto the evaluation stack.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ldarg_s: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);
     // ops->pushOperand(ops->readLocal(CilSemantics::asIndex(args[0])));
     // CilSemantics::loadCilArgument(dispatcher, ops, CilSemantics::asIndex(args[0]));
        const size_t argIndex = CilSemantics::asIndex(args[0]);
        CilSemantics::loadCilArgument(dispatcher, ops, argIndex);
    }
};

// ldarga_s (15 (0x0f))
        // Description:
        //   Load the address of an argument, selected by an inline index operand, onto the evaluation stack.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ldarga_s: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);
     // CilSemantics::pushAddress(ops);
     // CilSemantics::loadAddressOfCilArgument(dispatcher, ops, CilSemantics::asIndex(args[0]));
     // const size_t argIndex = CilSemantics::asIndex(args[0]);
     // CilSemantics::loadAddressOfCilArgument(dispatcher, ops, argIndex);

     // SValue::Ptr addr = ops->undefined_(ptrBits);
     // addr->kind(ValueKind::ManagedPointer);

        const size_t argIndex = CilSemantics::asIndex(args[0]);
#if 0
        const size_t ptrBits = CilSemantics::cilPointerBits(dispatcher);
        CilSemantics::loadAddressOfCilArgument(dispatcher, ops, argIndex, ptrBits);
#else        
        CilSemantics::materializeArgumentAddress(dispatcher, ops, argIndex);
#endif
    }
};

// starg_s (16 (0x10))
        // Description:
        //   Store the top evaluation-stack value into an argument selected by an inline index operand.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_starg_s: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);
     // CilSemantics::discard(ops, 1);
     // CilSemantics::storeCilArgument(dispatcher, ops, CilSemantics::asIndex(args[0]));
        const size_t argIndex = CilSemantics::asIndex(args[0]);
        CilSemantics::storeCilArgument(dispatcher, ops, argIndex);
    }
};


// ldloc_s (17 (0x11))
        // Description:
        //   Load local variable s onto the evaluation stack.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ldloc_s: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);
     // ops->pushOperand(ops->readLocal(CilSemantics::asIndex(args[0])));
     // ops->writeLocal(cilArgumentSlot(argIndex), value);
     // const size_t localIndex = CilSemantics::asIndex(args[0]);
     // CilSemantics::loadCilLocal(dispatcher, ops, localIndex);

        const size_t localIndex = CilSemantics::asIndex(args[0]);
        ops->pushOperand(CilSemantics::readCilLocal(dispatcher, ops, localIndex));
    }
};

// ldloca_s (18 (0x12))
        // Description:
        //   Load the address of a local variable, selected by an inline index operand, onto the evaluation stack.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ldloca_s: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);
     // CilSemantics::pushAddress(ops);
     // const size_t localIndex = CilSemantics::asIndex(args[0]);
     // CilSemantics::loadAddressOfCilLocal(dispatcher, ops, localIndex);
        
     // SValue::Ptr addr = ops->undefined_(ptrBits);
     // addr->kind(ValueKind::ManagedPointer);
        const size_t localIndex = CilSemantics::asIndex(args[0]);
#if 0
        const size_t ptrBits = CilSemantics::cilPointerBits(dispatcher);
        CilSemantics::loadAddressOfCilLocal(dispatcher, ops, localIndex, ptrBits);
#else
        CilSemantics::materializeLocalAddress(dispatcher, ops, localIndex);
#endif
    }
};

// stloc_s (19 (0x13))
        // Description:
        //   Store the top evaluation-stack value into local variable s.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_stloc_s: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);
     // ops->writeLocal(CilSemantics::asIndex(args[0]), ops->popOperand());
        const size_t localIndex = CilSemantics::asIndex(args[0]);
        CilSemantics::storeCilLocal(dispatcher, ops, localIndex);
    }
};

// ldnull (20 (0x14))
        // Description:
        //   Push a null object reference onto the evaluation stack.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ldnull: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // ops->pushOperand(ops->number_(32, 0));
     // const size_t ptrBits = 32; // TODO: derive from Cil/architecture word size.
     // ops->pushOperand(CilSemantics::nullReference(dispatcher, ops, ptrBits));
        ops->pushOperand(CilSemantics::nullReference(dispatcher, ops, CilSemantics::cilPointerBits(dispatcher)));
    }
};
  
// ldc_i4_m1 (21 (0x15))
        // Description:
        //   Push a numeric constant onto the evaluation stack.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ldc_i4_m1: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // ops->pushOperand(ops->number_(32, uint64_t(-1)));
        CilSemantics::pushCilIntegerConstant(dispatcher, ops, -1, 32);
    }
};

// ldc_i4_0 (22 (0x16))
        // Description:
        //   Push a numeric constant onto the evaluation stack.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ldc_i4_0: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // ops->pushOperand(dispatcher->makeConstant("i", 0, 32));
        CilSemantics::pushCilIntegerConstant(dispatcher, ops, 0, 32);
    }
};

// ldc_i4_1 (23 (0x17))
        // Description:
        //   Push a numeric constant onto the evaluation stack.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ldc_i4_1: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // ops->pushOperand(dispatcher->makeConstant("i", 1, 32));
        CilSemantics::pushCilIntegerConstant(dispatcher, ops, 1, 32);
    }
};

// ldc_i4_2 (24 (0x18))
        // Description:
        //   Push a numeric constant onto the evaluation stack.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ldc_i4_2: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // ops->pushOperand(dispatcher->makeConstant("i", 2, 32));
        CilSemantics::pushCilIntegerConstant(dispatcher, ops, 2, 32);
    }
};

// ldc_i4_3 (25 (0x19))
        // Description:
        //   Push a numeric constant onto the evaluation stack.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ldc_i4_3: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // ops->pushOperand(dispatcher->makeConstant("i", 3, 32));
        CilSemantics::pushCilIntegerConstant(dispatcher, ops, 3, 32);
    }
};

// ldc_i4_4 (26 (0x1a))
        // Description:
        //   Push a numeric constant onto the evaluation stack.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ldc_i4_4: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // ops->pushOperand(dispatcher->makeConstant("i", 4, 32));
        CilSemantics::pushCilIntegerConstant(dispatcher, ops, 4, 32);
    }
};

// ldc_i4_5 (27 (0x1b))
        // Description:
        //   Push a numeric constant onto the evaluation stack.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ldc_i4_5: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // ops->pushOperand(dispatcher->makeConstant("i", 5, 32));
        CilSemantics::pushCilIntegerConstant(dispatcher, ops, 5, 32);
    }
};

// ldc_i4_6 (28 (0x1c))
        // Description:
        //   Push a numeric constant onto the evaluation stack.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ldc_i4_6: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // ops->pushOperand(dispatcher->makeConstant("i", 6, 32));
        CilSemantics::pushCilIntegerConstant(dispatcher, ops, 6, 32);
    }
};

// ldc_i4_7 (29 (0x1d))
        // Description:
        //   Push a numeric constant onto the evaluation stack.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ldc_i4_7: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // ops->pushOperand(dispatcher->makeConstant("i", 7, 32));
        CilSemantics::pushCilIntegerConstant(dispatcher, ops, 7, 32);
    }
};

// ldc_i4_8 (30 (0x1e))
        // Description:
        //   Push a numeric constant onto the evaluation stack.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ldc_i4_8: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // ops->pushOperand(dispatcher->makeConstant("i", 8, 32));
        CilSemantics::pushCilIntegerConstant(dispatcher, ops, 8, 32);
    }
};

// ldc_i4_s (31 (0x1f))
        // Description:
        //   Push a numeric constant onto the evaluation stack.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ldc_i4_s: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);
        //signed byte
     // auto imm = dispatcher->asS1(args[0]);
     // ops->pushOperand(dispatcher->makeConstant("i", imm, 32));
     // const int8_t imm = dispatcher->asS1(args[0]);
     // CilSemantics::pushCilIntegerConstant(dispatcher, ops, imm, 32);
        const int8_t imm = dispatcher->asS1(args[0]);
        CilSemantics::pushCilIntegerConstant(dispatcher, ops, imm, 32);

    }
};

// ldc_i4 (32 (0x20))
        // Description:
        //   Push a numeric constant onto the evaluation stack.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ldc_i4: P {
     void p(D /*dispatcher*/, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);
     // CilSemantics::pushConstant(ops, args, 32);
     // const int32_t imm = dispatcher->asS4(args[0]);
     // CilSemantics::pushCilIntegerConstant(dispatcher, ops, imm, 32);
     // CilSemantics::pushConstant(ops, args, 32);

     // DQ (6/11/2026): Now using the SValue supporting functionality.
        CilSemantics::pushConstant(ops, args, 32);
        SValue::Ptr v = ops->popOperand();
        v->kind(ValueKind::Integer32);
        ops->pushOperand(v);
    }
};

// ldc_i8 (33 (0x21))
        // Description:
        //   Push a numeric constant onto the evaluation stack.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ldc_i8: P {
     void p(D /*dispatcher*/, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);
     // CilSemantics::pushConstant(ops, args, 64);
     // const int64_t imm = dispatcher->asS8(args[0]);
     // CilSemantics::pushCilIntegerConstant(dispatcher, ops, imm, 64);
     // CilSemantics::pushConstant(ops, args, 64);

     // DQ (6/11/2026): Now using the SValue supporting functionality.
        CilSemantics::pushConstant(ops, args, 64);
        SValue::Ptr v = ops->popOperand();
        v->kind(ValueKind::Integer64);
        ops->pushOperand(v);
    }
};

// ldc_r4 (34 (0x22))
        // Description:
        //   Push a numeric constant onto the evaluation stack.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
#if 0
struct IP_ldc_r4: P {
     void p(D /*dispatcher*/, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);
     // CilSemantics::pushUnknown(ops, 32);
     // const uint32_t bits = dispatcher->asU4(args[0]);
     // CilSemantics::pushCilFloatBits(dispatcher, ops, bits, 32);
     // TODO: Replace with exact IEEE-754 immediate bits when the CIL
     // decoder exposes a reliable raw 32-bit immediate accessor.
     // CilSemantics::pushUnknown(ops, 32);

     // DQ (6/11/2026): Now using the SValue supporting functionality.
        SValue::Ptr v = ops->undefined_(32);
        v->kind(ValueKind::Float32);
        ops->pushOperand(v);
     }
};
#endif

struct IP_ldc_r4: P {
    void p(D /*dispatcher*/, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);
 
#if 0
        SgAsmFloatValueExpression* expr = isSgAsmFloatValueExpression(args[0]);
        ASSERT_not_null(expr);

     // This current code reads a double from get_nativeValue() and copies the first 4 bytes of
     // the double into a uint32_t. That is not the IEEE-754 float encoding of the CIL r4 value.
        double val = expr->get_nativeValue();

        uint32_t bits = 0;
        std::memcpy(&bits, &val, sizeof bits);
#else
     // This was suggested to be better than the above code, but I think that Craig disagrees.
        SgAsmFloatValueExpression* expr = isSgAsmFloatValueExpression(args[0]);
        ASSERT_not_null(expr);

        uint32_t bits = 0;
        Sawyer::Optional<uint64_t> rawBits = CilSemantics::rawFloatExpressionBits(expr, 32);
        if (rawBits) {
            bits = static_cast<uint32_t>(*rawBits);
        } else {
            double val = expr->get_nativeValue();
            float f = static_cast<float>(val);
            static_assert(sizeof bits == sizeof f, "unexpected float size");
            std::memcpy(&bits, &f, sizeof bits);
        }
#endif
        /** Returns a number of the specified bit width.  Uses the prototypical value to virtually construct a new value. */
        SValue::Ptr v = ops->number_(32, bits);
        v->kind(ValueKind::Float32);
        ops->pushOperand(v);
    }
};


// ldc_r8 (35 (0x23))
        // Description:
        //   Push a numeric constant onto the evaluation stack.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ldc_r8: P {
     void p(D /*dispatcher*/, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);
     // CilSemantics::pushUnknown(ops, 64);
     // const uint64_t bits = dispatcher->asU8(args[0]);
     // CilSemantics::pushCilFloatBits(dispatcher, ops, bits, 64);
     // TODO: Replace with exact IEEE-754 immediate bits when the CIL
     // decoder exposes a reliable raw 64-bit immediate accessor.
     // CilSemantics::pushUnknown(ops, 64);
#if 0
     // DQ (6/11/2026): Now using the SValue supporting functionality.
        SValue::Ptr v = ops->undefined_(64);
        v->kind(ValueKind::Float64);
        ops->pushOperand(v);
#else
        SgAsmFloatValueExpression* expr = isSgAsmFloatValueExpression(args[0]);
        ASSERT_not_null(expr);

        uint64_t bits = 0;
        Sawyer::Optional<uint64_t> rawBits = CilSemantics::rawFloatExpressionBits(expr, 64);
        if (rawBits) {
            bits = *rawBits;
        } else {
            double val = expr->get_nativeValue();
            std::memcpy(&bits, &val, sizeof bits);
        }

        /** Returns a number of the specified bit width.  Uses the prototypical value to virtually construct a new value. */
        SValue::Ptr v = ops->number_(64, bits);
        v->kind(ValueKind::Float64);
        ops->pushOperand(v);
#endif
    }
};
  
// dup (37 (0x25))
        // Description:
        //   Duplicate the value currently on top of the evaluation stack.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_dup: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);

     // DQ (6/24/2026): Replaced with better semantics.
     // CilSemantics::dup(ops);
        
        SValue::Ptr value = ops->popOperand();
        SValue::Ptr copy = value->copy();

        dispatcher->typeAnalysis().copyFacts(copy, value);

        ops->pushOperand(value);
        ops->pushOperand(copy);
    }
};

// pop (38 (0x26))
        // Description:
        //   Remove the value currently on top of the evaluation stack.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_pop: P {
    void p(D /*dispatcher*/, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
        CilSemantics::discard(ops, 1);
    }
};

// jmp (39 (0x27))
        // Description:
        //   Unconditionally transfer control to the inline target for jmp.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_jmp: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);
     // CilSemantics::branchEffect(dispatcher, ops, insn, args);
        const uint32_t methodToken = CilSemantics::asIndex(args[0]);
        CilSemantics::jmpMethod(dispatcher, ops, insn, methodToken);
    }
};

// call (40 (0x28))
        // Description:
        //   Call, create, or otherwise transfer to the metadata target specified by call.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_call: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);

        CilSemantics::noteCallPrefixSemantics(dispatcher, ops, "call");
 
        const uint32_t methodToken = CilSemantics::asIndex(args[0]);

        DispatcherCil::CilTypeAnalysis::MethodDescriptor target(methodToken);

        if (dispatcher->cilMetadata())
             target = dispatcher->cilMetadata()->resolveMethod(methodToken);

        bool isCallVirtInstruction = false;
        CilSemantics::callMethod(dispatcher, ops, insn, target, isCallVirtInstruction, dispatcher->pendingPrefix().isTail);
    }
};

// calli (41 (0x29))
        // Description:
        //   Call, create, or otherwise transfer to the metadata target specified by calli.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_calli: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);

        CilSemantics::noteCallPrefixSemantics(dispatcher, ops, "calli");
 
        const uint32_t sigToken = CilSemantics::asIndex(args[0]);

        DispatcherCil::CilTypeAnalysis::CallSiteDescriptor sig(sigToken);

        if (dispatcher->cilMetadata())
           {
             sig = dispatcher->cilMetadata()->resolveCallSite(sigToken);
           }

        CilSemantics::callIndirect(dispatcher, ops, insn, sig, dispatcher->pendingPrefix().isTail);
    }
};

// ret (42 (0x2a))
        // Description:
        //   Return from the current method.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ret: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);

        auto state = ops->currentState();
        ASSERT_not_null(state);

        auto frame = state->currentFrame();
        ASSERT_not_null(frame);

        auto method = frame->method();
        ASSERT_not_null(method);

        SValue::Ptr retval;

        if (!method->returnsVoid()) {
            retval = ops->popOperand();
            ASSERT_not_null(retval);
        }

        dispatcher->completeReturn(ops, retval);
    }
};
 
// br_s (43 (0x2b))
// Description:
//   Unconditionally branches to a short-form relative IL target.
//
// Stack:
//   ..., -> ...
//
// Semantics:
//   Writes the instruction pointer to the decoded branch target. The short
//   operand is a signed 1-byte offset from the next instruction unless the
//   decoder has already converted it to an absolute target.
//
// Run-time Exceptions:
//   None for a valid method body.
struct IP_br_s: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);
        
     // Unconditional branch. No stack operands are consumed.
        CilSemantics::branchEffect(dispatcher, ops, insn, args);
    }
};

// brfalse_s (44 (0x2c))
// Description:
//   Branches to the short-form target if the popped value is zero, false, or
//   null.
//
// Stack:
//   ..., value -> ...
//
// Semantics:
//   Pops one stack value and branches if that value compares equal to zero.
//   For object references, null is represented as zero in the current
//   bitvector model.
//
// Run-time Exceptions:
//   None for a valid method body.
struct IP_brfalse_s: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);
     // CilSemantics::branchEffect(dispatcher, ops, insn, args);
        CilSemantics::branchFalseEffect(dispatcher, ops, insn, args);
    }
};

// brtrue_s (45 (0x2d))
// Description:
//   Branches to the short-form target if the popped value is nonzero, true, or
//   non-null.
//
// Stack:
//   ..., value -> ...
//
// Semantics:
//   Pops one stack value and branches if that value compares not equal to zero.
//   For object references, non-null is represented as nonzero in the current
//   bitvector model.
//
// Run-time Exceptions:
//   None for a valid method body.
struct IP_brtrue_s: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);
     // CilSemantics::branchEffect(dispatcher, ops, insn, args);
        CilSemantics::branchTrueEffect(dispatcher, ops, insn, args);
    }
};

// beq_s (46 (0x2e))
        // Description:
        //   Conditionally transfer control to the inline target for beq.s.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_beq_s: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);

     // DQ (6/11/2026): Use the version that uses the SValue class support.
     // CilSemantics::branchEffect(dispatcher, ops, insn, args);
     // CilSemantics::compareBranchEffect(dispatcher, ops, insn, args);

        CilSemantics::compareBranchEffect(dispatcher, ops, insn, args, CilSemantics::branch_eq);

    }
};

// bge_s (47 (0x2f))
        // Description:
        //   Conditionally transfer control to the inline target for bge.s.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_bge_s: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);

     // DQ (6/11/2026): Use the version that uses the SValue class support.
     // CilSemantics::branchEffect(dispatcher, ops, insn, args);
     // CilSemantics::compareBranchEffect(dispatcher, ops, insn, args);
        CilSemantics::compareBranchEffect(dispatcher, ops, insn, args, CilSemantics::branch_ge);        
    }
};

// bgt_s (48 (0x30))
        // Description:
        //   Conditionally transfer control to the inline target for bgt.s.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_bgt_s: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);

     // DQ (6/11/2026): Use the version that uses the SValue class support.
     // CilSemantics::branchEffect(dispatcher, ops, insn, args);
     // CilSemantics::compareBranchEffect(dispatcher, ops, insn, args);
        CilSemantics::compareBranchEffect(dispatcher, ops, insn, args, CilSemantics::branch_gt);        
    }
};

// ble_s (49 (0x31))
        // Description:
        //   Conditionally transfer control to the inline target for ble.s.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ble_s: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);

     // DQ (6/11/2026): Use the version that uses the SValue class support.
     // CilSemantics::branchEffect(dispatcher, ops, insn, args);
     // CilSemantics::compareBranchEffect(dispatcher, ops, insn, args);
        CilSemantics::compareBranchEffect(dispatcher, ops, insn, args, CilSemantics::branch_le);        
    }
};

// blt_s (50 (0x32))
        // Description:
        //   Conditionally transfer control to the inline target for blt.s.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_blt_s: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);

     // DQ (6/11/2026): Use the version that uses the SValue class support.
     // CilSemantics::branchEffect(dispatcher, ops, insn, args);
     // CilSemantics::compareBranchEffect(dispatcher, ops, insn, args);
        CilSemantics::compareBranchEffect(dispatcher, ops, insn, args, CilSemantics::branch_lt);        
    }
};


// bne_un_s (51 (0x33))
        // Description:
        //   Conditionally transfer control to the inline target for bne.un.s.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_bne_un_s: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);
     // CilSemantics::branchEffect(dispatcher, ops, insn, args);
        CilSemantics::compareBranchEffect( dispatcher, ops, insn, args, CilSemantics::branch_ne_un);
    }
};

// bge_un_s (52 (0x34))
        // Description:
        //   Conditionally transfer control to the inline target for bge.un.s.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_bge_un_s: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);
     // CilSemantics::branchEffect(dispatcher, ops, insn, args);
        CilSemantics::compareBranchEffect( dispatcher, ops, insn, args, CilSemantics::branch_ge_un);
    }
};

// bgt_un_s (53 (0x35))
        // Description:
        //   Conditionally transfer control to the inline target for bgt.un.s.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_bgt_un_s: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);
     // CilSemantics::branchEffect(dispatcher, ops, insn, args);
        CilSemantics::compareBranchEffect( dispatcher, ops, insn, args, CilSemantics::branch_gt_un);
    }
};

// ble_un_s (54 (0x36))
        // Description:
        //   Conditionally transfer control to the inline target for ble.un.s.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ble_un_s: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);
     // CilSemantics::branchEffect(dispatcher, ops, insn, args);
        CilSemantics::compareBranchEffect( dispatcher, ops, insn, args, CilSemantics::branch_le_un);
    }
};

// blt_un_s (55 (0x37))
        // Description:
        //   Conditionally transfer control to the inline target for blt.un.s.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_blt_un_s: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);
     // CilSemantics::branchEffect(dispatcher, ops, insn, args);
        CilSemantics::compareBranchEffect( dispatcher, ops, insn, args, CilSemantics::branch_lt_un);
    }
};

// br (56 (0x38))
        // Description:
        //   Unconditionally transfer control to the inline target for br.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_br: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);
        CilSemantics::branchEffect(dispatcher, ops, insn, args);
    }
};

// brfalse (57 (0x39))
        // Description:
        //   Conditionally transfer control to the inline target for brfalse.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_brfalse: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);
     // CilSemantics::branchEffect(dispatcher, ops, insn, args);
        CilSemantics::branchFalseEffect(dispatcher, ops, insn, args);
       
    }
};

// brtrue (58 (0x3a))
        // Description:
        //   Conditionally transfer control to the inline target for brtrue.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_brtrue: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);
     // CilSemantics::branchEffect(dispatcher, ops, insn, args);
        CilSemantics::branchTrueEffect(dispatcher, ops, insn, args);
    }
};

// beq (59 (0x3b))
        // Description:
        //   Conditionally transfer control to the inline target for beq.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_beq: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);
     // CilSemantics::branchEffect(dispatcher, ops, insn, args);
        CilSemantics::compareBranchEffect(dispatcher, ops, insn, args, CilSemantics::branch_eq);
    }
};

// bge (60 (0x3c))
        // Description:
        //   Conditionally transfer control to the inline target for bge.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_bge: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);
     // CilSemantics::branchEffect(dispatcher, ops, insn, args);
        CilSemantics::compareBranchEffect(dispatcher, ops, insn, args, CilSemantics::branch_ge);
    }
};

// bgt (61 (0x3d))
        // Description:
        //   Conditionally transfer control to the inline target for bgt.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_bgt: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);
     // CilSemantics::branchEffect(dispatcher, ops, insn, args);
        CilSemantics::compareBranchEffect(dispatcher, ops, insn, args, CilSemantics::branch_gt);
    }
};

// ble (62 (0x3e))
        // Description:
        //   Conditionally transfer control to the inline target for ble.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ble: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);
     // CilSemantics::branchEffect(dispatcher, ops, insn, args);
        CilSemantics::compareBranchEffect(dispatcher, ops, insn, args, CilSemantics::branch_le);
    }
};

// blt (63 (0x3f))
        // Description:
        //   Conditionally transfer control to the inline target for blt.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_blt: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);
     // CilSemantics::branchEffect(dispatcher, ops, insn, args);
        CilSemantics::compareBranchEffect(dispatcher, ops, insn, args, CilSemantics::branch_lt);
    }
};

// bne_un (64 (0x40))
        // Description:
        //   Conditionally transfer control to the inline target for bne.un.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_bne_un: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);
     // CilSemantics::branchEffect(dispatcher, ops, insn, args);
        CilSemantics::compareBranchEffect(dispatcher, ops, insn, args, CilSemantics::branch_ne_un);
    }
};

// bge_un (65 (0x41))
        // Description:
        //   Conditionally transfer control to the inline target for bge.un.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_bge_un: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);
     // CilSemantics::branchEffect(dispatcher, ops, insn, args);
        CilSemantics::compareBranchEffect(dispatcher, ops, insn, args, CilSemantics::branch_ge_un);
    }
};

// bgt_un (66 (0x42))
        // Description:
        //   Conditionally transfer control to the inline target for bgt.un.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_bgt_un: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);
     // CilSemantics::branchEffect(dispatcher, ops, insn, args);
        CilSemantics::compareBranchEffect(dispatcher, ops, insn, args, CilSemantics::branch_gt_un);
    }
};

// ble_un (67 (0x43))
        // Description:
        //   Conditionally transfer control to the inline target for ble.un.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ble_un: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);
     // CilSemantics::branchEffect(dispatcher, ops, insn, args);
        CilSemantics::compareBranchEffect(dispatcher, ops, insn, args, CilSemantics::branch_le_un);
    }
};

// blt_un (68 (0x44))
        // Description:
        //   Conditionally transfer control to the inline target for blt.un.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_blt_un: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);
     // CilSemantics::branchEffect(dispatcher, ops, insn, args);
        CilSemantics::compareBranchEffect(dispatcher, ops, insn, args, CilSemantics::branch_lt_un);
    }
};

// switch (69 (0x45))
        // Description:
        //   Transfer control to one of the inline switch targets selected by an int32 stack value.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_switch: P
   {
     void p(D dispatcher, Ops ops, I insn, Args args) override
        {
          CilSemantics::switchEffect(dispatcher, ops, insn, args);
        }
   };


// ldind_i1 (70 (0x46))
        // Description:
        //   Load a value indirectly from the address on top of the evaluation stack using ldind.i1.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ldind_i1: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::loadResult(ops, 32);
        CilSemantics::loadIndirectResult(dispatcher, ops, CilSemantics::ldind_i1_kind);
    }
};

// ldind_u1 (71 (0x47))
        // Description:
        //   Load a value indirectly from the address on top of the evaluation stack using ldind.u1.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ldind_u1: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::loadResult(ops, 32);
        CilSemantics::loadIndirectResult(dispatcher, ops, CilSemantics::ldind_u1_kind);
    }
};

// ldind_i2 (72 (0x48))
        // Description:
        //   Load a value indirectly from the address on top of the evaluation stack using ldind.i2.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ldind_i2: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::loadResult(ops, 32);
        CilSemantics::loadIndirectResult(dispatcher, ops, CilSemantics::ldind_i2_kind);
    }
};

// ldind_u2 (73 (0x49))
        // Description:
        //   Load a value indirectly from the address on top of the evaluation stack using ldind.u2.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ldind_u2: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::loadResult(ops, 32);
        CilSemantics::loadIndirectResult(dispatcher, ops, CilSemantics::ldind_u2_kind);
    }
};

// ldind_i4 (74 (0x4a))
        // Description:
        //   Load a value indirectly from the address on top of the evaluation stack using ldind.i4.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ldind_i4: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::loadResult(ops, 32);
        CilSemantics::loadIndirectResult(dispatcher, ops, CilSemantics::ldind_i4_kind);
    }
};

// ldind_u4 (75 (0x4b))
        // Description:
        //   Load a value indirectly from the address on top of the evaluation stack using ldind.u4.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ldind_u4: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::loadResult(ops, 32);
        CilSemantics::loadIndirectResult(dispatcher, ops, CilSemantics::ldind_u4_kind);
    }
};

// ldind_i8 (76 (0x4c))
        // Description:
        //   Load a value indirectly from the address on top of the evaluation stack using ldind.i8.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ldind_i8: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::loadResult(ops, 64);
        CilSemantics::loadIndirectResult(dispatcher, ops, CilSemantics::ldind_i8_kind);
    }
};

// ldind_i (77 (0x4d))
        // Description:
        //   Load a value indirectly from the address on top of the evaluation stack using ldind.i.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ldind_i: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::loadResult(ops, 32);
        CilSemantics::loadIndirectResult(dispatcher, ops, CilSemantics::ldind_i_kind);
    }
};

// ldind_r4 (78 (0x4e))
        // Description:
        //   Load a value indirectly from the address on top of the evaluation stack using ldind.r4.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ldind_r4: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::loadResult(ops, 32);
        CilSemantics::loadIndirectResult(dispatcher, ops, CilSemantics::ldind_r4_kind);
    }
};

// ldind_r8 (79 (0x4f))
        // Description:
        //   Load a value indirectly from the address on top of the evaluation stack using ldind.r8.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ldind_r8: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::loadResult(ops, 64);
        CilSemantics::loadIndirectResult(dispatcher, ops, CilSemantics::ldind_r8_kind);
    }
};

// ldind_ref (80 (0x50))
        // Description:
        //   Load a value indirectly from the address on top of the evaluation stack using ldind.ref.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ldind_ref: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::loadResult(ops, 32);
        CilSemantics::loadIndirectResult(dispatcher, ops, CilSemantics::ldind_ref_kind);
    }
};


// stind_ref (81 (0x51))
        // Description:
        //   Store a value indirectly through the address on the evaluation stack using stind.ref.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_stind_ref: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::storeEffect(ops);
        CilSemantics::storeIndirectEffect(dispatcher, ops, CilSemantics::stind_ref_kind);
    }
};

// stind_i1 (82 (0x52))
        // Description:
        //   Store a value indirectly through the address on the evaluation stack using stind.i1.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_stind_i1: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::storeEffect(ops);
        CilSemantics::storeIndirectEffect(dispatcher, ops, CilSemantics::stind_i1_kind);
    }
};

// stind_i2 (83 (0x53))
        // Description:
        //   Store a value indirectly through the address on the evaluation stack using stind.i2.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_stind_i2: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::storeEffect(ops);
        CilSemantics::storeIndirectEffect(dispatcher, ops, CilSemantics::stind_i2_kind);
    }
};

// stind_i4 (84 (0x54))
        // Description:
        //   Store a value indirectly through the address on the evaluation stack using stind.i4.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_stind_i4: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::storeEffect(ops);
        CilSemantics::storeIndirectEffect(dispatcher, ops, CilSemantics::stind_i4_kind);
    }
};

// stind_i8 (85 (0x55))
        // Description:
        //   Store a value indirectly through the address on the evaluation stack using stind.i8.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_stind_i8: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::storeEffect(ops);
        CilSemantics::storeIndirectEffect(dispatcher, ops, CilSemantics::stind_i8_kind);
    }
};

// stind_r4 (86 (0x56))
        // Description:
        //   Store a value indirectly through the address on the evaluation stack using stind.r4.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_stind_r4: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::storeEffect(ops);
        CilSemantics::storeIndirectEffect(dispatcher, ops, CilSemantics::stind_r4_kind);
    }
};

// stind_r8 (87 (0x57))
        // Description:
        //   Store a value indirectly through the address on the evaluation stack using stind.r8.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_stind_r8: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::storeEffect(ops);
        CilSemantics::storeIndirectEffect(dispatcher, ops, CilSemantics::stind_r8_kind);
    }
};

// add (88 (0x58))
        // Description:
        //   Pop two values, compute add, and push the result onto the evaluation stack.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_add: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
        CilSemantics::binaryResult(dispatcher,ops,insn,CilSemantics::add);
    }
};

// sub (89 (0x59))
        // Description:
        //   Pop two values, compute sub, and push the result onto the evaluation stack.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_sub: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
        CilSemantics::binaryResult(dispatcher,ops,insn,CilSemantics::subtract);
    }
};

// mul (90 (0x5a))
        // Description:
        //   Pop two values, compute mul, and push the result onto the evaluation stack.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_mul: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
        CilSemantics::binaryResult(dispatcher,ops,insn,CilSemantics::multiply);
    }
};

// div (91 (0x5b))
        // Description:
        //   Pop two values, compute div, and push the result onto the evaluation stack.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_div: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
        CilSemantics::binaryResult(dispatcher,ops,insn,CilSemantics::divide);
    }
};

// div_un (92 (0x5c))
        // Description:
        //   Pop two values, compute div.un, and push the result onto the evaluation stack.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_div_un: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
        CilSemantics::binaryResult(dispatcher,ops,insn,CilSemantics::divide_unsigned);
    }
};

// rem (93 (0x5d))
        // Description:
        //   Pop two values, compute rem, and push the result onto the evaluation stack.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_rem: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
        CilSemantics::binaryResult(dispatcher,ops,insn,CilSemantics::remainder);
    }
};

// rem_un (94 (0x5e))
        // Description:
        //   Pop two values, compute rem.un, and push the result onto the evaluation stack.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_rem_un: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
        CilSemantics::binaryResult(dispatcher,ops,insn,CilSemantics::remainder_unsigned);
    }
};

// and (95 (0x5f))
        // Description:
        //   Pop two values, compute and, and push the result onto the evaluation stack.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_and: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
        CilSemantics::binaryResult(dispatcher,ops,insn,CilSemantics::and_op);
    }
};

// or (96 (0x60))
        // Description:
        //   Pop two values, compute or, and push the result onto the evaluation stack.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_or: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
        CilSemantics::binaryResult(dispatcher,ops,insn,CilSemantics::or_op);
    }
};

// xor (97 (0x61))
        // Description:
        //   Pop two values, compute xor, and push the result onto the evaluation stack.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_xor: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
        CilSemantics::binaryResult(dispatcher,ops,insn,CilSemantics::xor_op);
    }
};

// shl (98 (0x62))
        // Description:
        //   Pop two values, compute shl, and push the result onto the evaluation stack.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_shl: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
        CilSemantics::binaryResult(dispatcher,ops,insn,CilSemantics::shiftleft_op);
    }
};

// shr (99 (0x63))
        // Description:
        //   Pop two values, compute shr, and push the result onto the evaluation stack.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_shr: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
        CilSemantics::binaryResult(dispatcher,ops,insn,CilSemantics::shiftright_op);
    }
};

// shr_un (100 (0x64))
        // Description:
        //   Pop two values, compute shr.un, and push the result onto the evaluation stack.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_shr_un: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
        CilSemantics::binaryResult(dispatcher,ops,insn,CilSemantics::shiftright_unsigned);
    }
};

// neg (101 (0x65))
        // Description:
        //   Execute the CIL instruction neg.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_neg: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);

     // DQ (6/9/2026): Need to pass 4th parameter to unaryResult(), except where the args are not present.
     // CilSemantics::unaryResult(dispatcher,ops,CilSemantics::negate);
     // DispatcherCil::CilTypeAnalysis::TypeDescriptor targetType(CilSemantics::asIndex(args[0]));
     // CilSemantics::unaryResult(dispatcher,ops,CilSemantics::negate,&targetType);
        CilSemantics::unaryResult(dispatcher,ops,CilSemantics::negate);
    }
};

// not (102 (0x66))
        // Description:
        //   Execute the CIL instruction not.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_not: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);

     // DQ (6/9/2026): Need to pass 4th parameter to unaryResult(), except where the args are not present.
     // CilSemantics::unaryResult(dispatcher,ops,CilSemantics::not_op);
     // DispatcherCil::CilTypeAnalysis::TypeDescriptor targetType(CilSemantics::asIndex(args[0]));
     // CilSemantics::unaryResult(dispatcher,ops,CilSemantics::not_op,&targetType);
        CilSemantics::unaryResult(dispatcher,ops,CilSemantics::not_op);
    }
};


// conv_i1 (103 (0x67))
        // Description:
        //   Convert the top evaluation-stack value according to conv.i1.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_conv_i1: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::convertResult(ops);
        CilSemantics::convertResult(dispatcher, ops, CilSemantics::conv_i1_kind, insn);
    }
};

// conv_i2 (104 (0x68))
        // Description:
        //   Convert the top evaluation-stack value according to conv.i2.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_conv_i2: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::convertResult(ops);
        CilSemantics::convertResult(dispatcher, ops, CilSemantics::conv_i2_kind, insn);
    }
};

// conv_i4 (105 (0x69))
        // Description:
        //   Convert the top evaluation-stack value according to conv.i4.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_conv_i4: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::convertResult(ops);
        CilSemantics::convertResult(dispatcher, ops, CilSemantics::conv_i4_kind, insn);
    }
};

// conv_i8 (106 (0x6a))
        // Description:
        //   Convert the top evaluation-stack value according to conv.i8.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_conv_i8: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::convertResult(ops);
        CilSemantics::convertResult(dispatcher, ops, CilSemantics::conv_i8_kind, insn);
    }
};

// conv_r4 (107 (0x6b))
        // Description:
        //   Convert the top evaluation-stack value according to conv.r4.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_conv_r4: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::convertResult(ops);
        CilSemantics::convertResult(dispatcher, ops, CilSemantics::conv_r4_kind, insn);
    }
};

// conv_r8 (108 (0x6c))
        // Description:
        //   Convert the top evaluation-stack value according to conv.r8.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_conv_r8: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::convertResult(ops);
        CilSemantics::convertResult(dispatcher, ops, CilSemantics::conv_r8_kind, insn);
    }
};

// conv_u4 (109 (0x6d))
        // Description:
        //   Convert the top evaluation-stack value according to conv.u4.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_conv_u4: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::convertResult(ops);
        CilSemantics::convertResult(dispatcher, ops, CilSemantics::conv_u4_kind, insn);
    }
};

// conv_u8 (110 (0x6e))
        // Description:
        //   Convert the top evaluation-stack value according to conv.u8.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_conv_u8: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::convertResult(ops);
        CilSemantics::convertResult(dispatcher, ops, CilSemantics::conv_u8_kind, insn);
    }
};


// callvirt (111 (0x6f))
        // Description:
        //   Call, create, or otherwise transfer to the metadata target specified by callvirt.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
#if 0
struct IP_callvirt: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);
        
        const uint32_t methodToken = CilSemantics::asIndex(args[0]);

        DispatcherCil::CilTypeAnalysis::MethodDescriptor target(methodToken);

        if (dispatcher->cilMetadata())
           {
             target = dispatcher->cilMetadata()->resolveMethod(methodToken);
           }

        bool isCallVirtInstruction = true;
        CilSemantics::callMethod(dispatcher, ops, insn, target, isCallVirtInstruction);
    }
};
#endif

struct IP_callvirt: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);

        CilSemantics::noteCallPrefixSemantics(dispatcher, ops, "callvirt");
 
        const uint32_t methodToken = CilSemantics::asIndex(args[0]);

        DispatcherCil::CilTypeAnalysis::MethodDescriptor target(methodToken);
        if (dispatcher->cilMetadata())
            target = dispatcher->cilMetadata()->resolveMethod(methodToken);

        const auto &prefix = dispatcher->pendingPrefix();
        DispatcherCil::CilTypeAnalysis::TypeDescriptor constrainedType;
        const DispatcherCil::CilTypeAnalysis::TypeDescriptor *constrainedReceiverType = nullptr;

        if (prefix.hasConstrained)
           {
             constrainedType = CilSemantics::resolveType(dispatcher, prefix.constrainedTypeToken);
             constrainedReceiverType = &constrainedType;

             if (constrainedType.metadataToken != 0)
                {
               // Optional diagnostic only.
                  ops->comment("CIL constrained. type token = " + StringUtility::addrToString(constrainedType.metadataToken));
                }
           }

        bool isCallVirtInstruction = true;
        CilSemantics::callMethod(dispatcher, ops, insn, target, isCallVirtInstruction, prefix.isTail, constrainedReceiverType);
    }
};


// cpobj (112 (0x70))
        // Description:
        //   Execute the CIL instruction cpobj.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_cpobj: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);
     // CilSemantics::storeEffect(ops);
#if 0
        const uint32_t typeToken = CilSemantics::asIndex(args[0]);
        DispatcherCil::CilTypeAnalysis::TypeDescriptor type = CilSemantics::typeDescriptorFromToken(typeToken);
#else
        const uint32_t typeToken = CilSemantics::asIndex(args[0]);
        DispatcherCil::CilTypeAnalysis::TypeDescriptor type = CilSemantics::resolveType(dispatcher, typeToken);
#endif
        CilSemantics::copyObjectEffect(dispatcher, ops, type);
    }
};

// ldobj (113 (0x71))
        // Description:
        //   Load a value indirectly from the address on top of the evaluation stack using ldobj.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ldobj: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);
     // CilSemantics::loadResult(ops, 32);
#if 0
        const uint32_t typeToken = CilSemantics::asIndex(args[0]);
        DispatcherCil::CilTypeAnalysis::TypeDescriptor type = CilSemantics::typeDescriptorFromToken(typeToken);
#else
        const uint32_t typeToken = CilSemantics::asIndex(args[0]);
        DispatcherCil::CilTypeAnalysis::TypeDescriptor type = CilSemantics::resolveType(dispatcher, typeToken);
#endif
        CilSemantics::loadObjectEffect(dispatcher, ops, type);
    }
};

// ldstr (114 (0x72))
        // Description:
        //   Push a CIL value modeled here as an unknown 32-bit value for ldstr.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ldstr: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);
     // CilSemantics::pushUnknown(ops, 32);

        const uint32_t userStringToken = CilSemantics::asIndex(args[0]);
        ops->pushOperand(CilSemantics::stringReference(dispatcher, ops, userStringToken));
    }
};

// newobj (115 (0x73))
        // Description:
        //   Call, create, or otherwise transfer to the metadata target specified by newobj.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_newobj: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);

        const uint32_t ctorToken = CilSemantics::asIndex(args[0]);

        DispatcherCil::CilTypeAnalysis::MethodDescriptor ctor;

        if (dispatcher->cilMetadata())
           {
             ctor = dispatcher->cilMetadata()->resolveMethod(ctorToken);
           }
          else
           {
             ctor = DispatcherCil::CilTypeAnalysis::MethodDescriptor(ctorToken);
           }

        CilSemantics::newObjectEffect(dispatcher, ops, insn, ctor);
    }
};



// castclass (116 (0x74))
        // Description:
        //   Execute the CIL instruction castclass.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_castclass: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);

     // DQ (6/9/2026): Need to pass 4th parameter to unaryResult().
     // CilSemantics::unaryResult(dispatcher,ops,CilSemantics::castclass);
#if 0
        DispatcherCil::CilTypeAnalysis::TypeDescriptor targetType(CilSemantics::asIndex(args[0]));
#else
        const uint32_t typeToken = CilSemantics::asIndex(args[0]);
        DispatcherCil::CilTypeAnalysis::TypeDescriptor targetType = CilSemantics::resolveType(dispatcher, typeToken);
#endif

        CilSemantics::unaryResult(dispatcher, ops, CilSemantics::castclass, &targetType);
    }
};

// isinst (117 (0x75))
        // Description:
        //   Execute the CIL instruction isinst.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_isinst: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);

     // DQ (6/9/2026): Need to pass 4th parameter to unaryResult().
     // CilSemantics::unaryResult(dispatcher,ops,CilSemantics::isinst);
#if 0
        DispatcherCil::CilTypeAnalysis::TypeDescriptor targetType(CilSemantics::asIndex(args[0]));
#else
        const uint32_t typeToken = CilSemantics::asIndex(args[0]);
        DispatcherCil::CilTypeAnalysis::TypeDescriptor targetType = CilSemantics::resolveType(dispatcher, typeToken);
#endif
        CilSemantics::unaryResult(dispatcher,ops,CilSemantics::isinst,&targetType);
    }
};

// conv_r_un (118 (0x76))
        // Description:
        //   Convert the top evaluation-stack value according to conv.r.un.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_conv_r_un: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::convertResult(ops);
        CilSemantics::convertResult(dispatcher, ops, CilSemantics::conv_r_un_kind, insn);
    }
};

// unbox (121 (0x79))
        // Description:
        //   Execute the CIL instruction unbox.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_unbox: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);

     // DQ (6/9/2026): Need to pass 4th parameter to unaryResult().
#if 0
     // CilSemantics::unaryResult(dispatcher,ops,CilSemantics::unbox);
        DispatcherCil::CilTypeAnalysis::TypeDescriptor targetType(CilSemantics::asIndex(args[0]));
#else
        const uint32_t typeToken = CilSemantics::asIndex(args[0]);
        DispatcherCil::CilTypeAnalysis::TypeDescriptor targetType = CilSemantics::resolveType(dispatcher, typeToken);
#endif
        CilSemantics::unaryResult(dispatcher,ops,CilSemantics::unbox,&targetType);
    }
};

// throw (122 (0x7a))
        // Description:
        //   Throw or rethrow an exception using throw.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_throw: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::throwEffect(ops);
        CilSemantics::throwExceptionEffect(dispatcher, ops);
    }
};


// ldfld (123 (0x7b))
        // Description:
        //   Load a field value or field address identified by the inline metadata token using ldfld.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ldfld: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);
     // CilSemantics::loadResult(ops, 32);

        const uint32_t fieldToken = CilSemantics::asIndex(args[0]);
     // DispatcherCil::CilTypeAnalysis::FieldDescriptor field = CilSemantics::fieldDescriptorFromToken(fieldToken);
        DispatcherCil::CilTypeAnalysis::FieldDescriptor field = DispatcherCil::CilTypeAnalysis::resolveField(dispatcher, fieldToken);        
        CilSemantics::loadInstanceField(dispatcher, ops, field);
    }
};

// ldflda (124 (0x7c))
        // Description:
        //   Load a field value or field address identified by the inline metadata token using ldflda.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ldflda: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);
     // CilSemantics::pushAddress(ops);
        
        const uint32_t fieldToken = CilSemantics::asIndex(args[0]);
     // DispatcherCil::CilTypeAnalysis::FieldDescriptor field = CilSemantics::fieldDescriptorFromToken(fieldToken);
        DispatcherCil::CilTypeAnalysis::FieldDescriptor field = DispatcherCil::CilTypeAnalysis::resolveField(dispatcher, fieldToken);        
        CilSemantics::loadInstanceFieldAddress(dispatcher, ops, field);
    }
};

// stfld (125 (0x7d))
        // Description:
        //   Store the top value into the field identified by the inline metadata token using stfld.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_stfld: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);
     // CilSemantics::storeEffect(ops);

        const uint32_t fieldToken = CilSemantics::asIndex(args[0]);
     // DispatcherCil::CilTypeAnalysis::FieldDescriptor field = CilSemantics::fieldDescriptorFromToken(fieldToken);
        DispatcherCil::CilTypeAnalysis::FieldDescriptor field = DispatcherCil::CilTypeAnalysis::resolveField(dispatcher, fieldToken);        
        CilSemantics::storeInstanceField(dispatcher, ops, field);
    }
};

// ldsfld (126 (0x7e))
        // Description:
        //   Push a CIL value modeled here as an unknown 32-bit value for ldsfld.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ldsfld: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);
     // CilSemantics::pushUnknown(ops, 32);

        const uint32_t fieldToken = CilSemantics::asIndex(args[0]);
     // DispatcherCil::CilTypeAnalysis::FieldDescriptor field = CilSemantics::fieldDescriptorFromToken(fieldToken);
        DispatcherCil::CilTypeAnalysis::FieldDescriptor field = DispatcherCil::CilTypeAnalysis::resolveField(dispatcher, fieldToken);        
        CilSemantics::loadStaticField(dispatcher, ops, field);
    }
};

// ldsflda (127 (0x7f))
        // Description:
        //   Push a managed/native address modeled here as an unknown pointer-sized value for ldsflda.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ldsflda: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);
     // CilSemantics::pushAddress(ops);

        const uint32_t fieldToken = CilSemantics::asIndex(args[0]);
     // DispatcherCil::CilTypeAnalysis::FieldDescriptor field = CilSemantics::fieldDescriptorFromToken(fieldToken);
        DispatcherCil::CilTypeAnalysis::FieldDescriptor field = DispatcherCil::CilTypeAnalysis::resolveField(dispatcher, fieldToken);        
        CilSemantics::loadStaticFieldAddress(dispatcher, ops, field);
    }
};

// stsfld (128 (0x80))
        // Description:
        //   Execute the CIL instruction stsfld.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_stsfld: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);
     // CilSemantics::doNothing(ops);

        const uint32_t fieldToken = CilSemantics::asIndex(args[0]);
     // DispatcherCil::CilTypeAnalysis::FieldDescriptor field = CilSemantics::fieldDescriptorFromToken(fieldToken);
        DispatcherCil::CilTypeAnalysis::FieldDescriptor field = DispatcherCil::CilTypeAnalysis::resolveField(dispatcher, fieldToken);        
        CilSemantics::storeStaticField(dispatcher, ops, field);
    }
};


// stobj (129 (0x81))
        // Description:
        //   Store a value indirectly through the address on the evaluation stack using stobj.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_stobj: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);
     // CilSemantics::storeEffect(ops);
#if 0
        const uint32_t typeToken = CilSemantics::asIndex(args[0]);
        DispatcherCil::CilTypeAnalysis::TypeDescriptor type(typeToken);
#else
        const uint32_t typeToken = CilSemantics::asIndex(args[0]);
        DispatcherCil::CilTypeAnalysis::TypeDescriptor type = CilSemantics::resolveType(dispatcher, typeToken);
#endif
        CilSemantics::storeObjectEffect(dispatcher, ops, type);
    }
};


// conv_ovf_i1_un (130 (0x82))
        // Description:
        //   Convert the top evaluation-stack value according to conv.ovf.i1.un.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_conv_ovf_i1_un: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::convertResult(ops);
        CilSemantics::convertResult(dispatcher, ops, CilSemantics::conv_ovf_i1_un_kind, insn);
    }
};

// conv_ovf_i2_un (131 (0x83))
        // Description:
        //   Convert the top evaluation-stack value according to conv.ovf.i2.un.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_conv_ovf_i2_un: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::convertResult(ops);
        CilSemantics::convertResult(dispatcher, ops, CilSemantics::conv_ovf_i2_un_kind, insn);
    }
};

// conv_ovf_i4_un (132 (0x84))
        // Description:
        //   Convert the top evaluation-stack value according to conv.ovf.i4.un.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_conv_ovf_i4_un: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::convertResult(ops);
        CilSemantics::convertResult(dispatcher, ops, CilSemantics::conv_ovf_i4_un_kind, insn);
    }
};

// conv_ovf_i8_un (133 (0x85))
        // Description:
        //   Convert the top evaluation-stack value according to conv.ovf.i8.un.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_conv_ovf_i8_un: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::convertResult(ops);
        CilSemantics::convertResult(dispatcher, ops, CilSemantics::conv_ovf_i8_un_kind, insn);
    }
};

// conv_ovf_u1_un (134 (0x86))
        // Description:
        //   Convert the top evaluation-stack value according to conv.ovf.u1.un.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_conv_ovf_u1_un: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::convertResult(ops);
        CilSemantics::convertResult(dispatcher, ops, CilSemantics::conv_ovf_u1_un_kind, insn);
    }
};

// conv_ovf_u2_un (135 (0x87))
        // Description:
        //   Convert the top evaluation-stack value according to conv.ovf.u2.un.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_conv_ovf_u2_un: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::convertResult(ops);
        CilSemantics::convertResult(dispatcher, ops, CilSemantics::conv_ovf_u2_un_kind, insn);
    }
};

// conv_ovf_u4_un (136 (0x88))
        // Description:
        //   Convert the top evaluation-stack value according to conv.ovf.u4.un.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_conv_ovf_u4_un: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::convertResult(ops);
        CilSemantics::convertResult(dispatcher, ops, CilSemantics::conv_ovf_u4_un_kind, insn);
    }
};

// conv_ovf_u8_un (137 (0x89))
        // Description:
        //   Convert the top evaluation-stack value according to conv.ovf.u8.un.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_conv_ovf_u8_un: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::convertResult(ops);
        CilSemantics::convertResult(dispatcher, ops, CilSemantics::conv_ovf_u8_un_kind, insn);
    }
};

// conv_ovf_i_un (138 (0x8a))
        // Description:
        //   Convert the top evaluation-stack value according to conv.ovf.i.un.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_conv_ovf_i_un: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::convertResult(ops);
        CilSemantics::convertResult(dispatcher, ops, CilSemantics::conv_ovf_i_un_kind, insn);
    }
};

// conv_ovf_u_un (139 (0x8b))
        // Description:
        //   Convert the top evaluation-stack value according to conv.ovf.u.un.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_conv_ovf_u_un: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::convertResult(ops);
        CilSemantics::convertResult(dispatcher, ops, CilSemantics::conv_ovf_u_un_kind, insn);
    }
};


// box (140 (0x8c))
        // Description:
        //   Execute the CIL instruction box.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_box: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);

     // DQ (6/9/2026): Need to pass 4th parameter to unaryResult().
     // CilSemantics::unaryResult(dispatcher,ops,CilSemantics::box);
     // DispatcherCil::CilTypeAnalysis::TypeDescriptor targetType(CilSemantics::asIndex(args[0]));
        const uint32_t typeToken = CilSemantics::asIndex(args[0]);
        DispatcherCil::CilTypeAnalysis::TypeDescriptor targetType = CilSemantics::resolveType(dispatcher, typeToken);
        CilSemantics::unaryResult(dispatcher,ops,CilSemantics::box,&targetType);
    }
};

// newarr (141 (0x8d))
        // Description:
        //   Execute the CIL instruction newarr.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_newarr: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);

     // DQ (6/9/2026): Need to pass 4th parameter to unaryResult().
     // CilSemantics::unaryResult(dispatcher,ops,CilSemantics::newarr);
#if 0
        DispatcherCil::CilTypeAnalysis::TypeDescriptor targetType(CilSemantics::asIndex(args[0]));
#else
        const uint32_t typeToken = CilSemantics::asIndex(args[0]);
        DispatcherCil::CilTypeAnalysis::TypeDescriptor targetType = CilSemantics::resolveType(dispatcher, typeToken);
#endif
        CilSemantics::unaryResult(dispatcher,ops,CilSemantics::newarr,&targetType);
    }
};

// ldlen (142 (0x8e))
        // Description:
        //   Execute the CIL instruction ldlen.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ldlen: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);

     // DQ (6/9/2026): Need to pass 4th parameter to unaryResult().
     // CilSemantics::unaryResult(dispatcher,ops,CilSemantics::ldlen);
     // DispatcherCil::CilTypeAnalysis::TypeDescriptor targetType(CilSemantics::asIndex(args[0]));
     // CilSemantics::unaryResult(dispatcher,ops,CilSemantics::ldlen,&targetType);
        CilSemantics::unaryResult(dispatcher,ops,CilSemantics::ldlen);
    }
};

// ldelema (143 (0x8f))
        // Description:
        //   Pop two values, compute ldelema, and push the result onto the evaluation stack.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
#if 0
struct IP_ldelema: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);
#if CIL_DEBUG
        fprintf (stderr,"ldelema: at top of instruction semantics \n");
        fflush(stderr);
#endif
     // DQ (6/16/2026): Better version of code.
     // const uint32_t typeToken = CilSemantics::asIndex(args[0]);
     // DispatcherCil::CilTypeAnalysis::TypeDescriptor elementType(typeToken);
        const uint32_t typeToken = CilSemantics::asIndex(args[0]);
        DispatcherCil::CilTypeAnalysis::TypeDescriptor elementType = CilSemantics::resolveType(dispatcher, typeToken);
        
     // This is the real instruction semantics under test.
        CilSemantics::loadElementAddress(dispatcher, ops, insn, args, elementType);
 
#if CIL_DEBUG
        fprintf (stderr,"ldelema: at bottom of instruction semantics \n");
        fflush(stderr);
#endif
    }
};
#endif

struct IP_ldelema: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);

        const uint32_t typeToken = CilSemantics::asIndex(args[0]);
        DispatcherCil::CilTypeAnalysis::TypeDescriptor elementType = CilSemantics::resolveType(dispatcher, typeToken);

        const bool readonlyElementAddress = dispatcher->pendingPrefix().isReadonly;

        CilSemantics::loadElementAddress(dispatcher, ops, insn, args, elementType, readonlyElementAddress);
    }
};


// ldelem_i1 (144 (0x90))
        // Description:
        //   Load an array element selected by array reference and index using ldelem.i1.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ldelem_i1: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::loadResult(ops, 32);
        CilSemantics::loadArrayElement(dispatcher, ops, CilSemantics::ldelem_i1_kind);
    }
};

// ldelem_u1 (145 (0x91))
        // Description:
        //   Load an array element selected by array reference and index using ldelem.u1.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ldelem_u1: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::loadResult(ops, 32);
        CilSemantics::loadArrayElement(dispatcher, ops, CilSemantics::ldelem_u1_kind);
    }
};

// ldelem_i2 (146 (0x92))
        // Description:
        //   Load an array element selected by array reference and index using ldelem.i2.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ldelem_i2: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::loadResult(ops, 32);
        CilSemantics::loadArrayElement(dispatcher, ops, CilSemantics::ldelem_i2_kind);
    }
};

// ldelem_u2 (147 (0x93))
        // Description:
        //   Load an array element selected by array reference and index using ldelem.u2.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ldelem_u2: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::loadResult(ops, 32);
        CilSemantics::loadArrayElement(dispatcher, ops, CilSemantics::ldelem_u2_kind);
    }
};

// ldelem_i4 (148 (0x94))
        // Description:
        //   Load an array element selected by array reference and index using ldelem.i4.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ldelem_i4: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::loadResult(ops, 32);
        CilSemantics::loadArrayElement(dispatcher, ops, CilSemantics::ldelem_i4_kind);
    }
};

// ldelem_u4 (149 (0x95))
        // Description:
        //   Load an array element selected by array reference and index using ldelem.u4.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ldelem_u4: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
        CilSemantics::loadArrayElement(dispatcher, ops, CilSemantics::ldelem_u4_kind);
    }
};

// ldelem_i8 (150 (0x96))
        // Description:
        //   Load an array element selected by array reference and index using ldelem.i8.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ldelem_i8: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::loadResult(ops, 64);
        CilSemantics::loadArrayElement(dispatcher, ops, CilSemantics::ldelem_i8_kind);
    }
};

// ldelem_i (151 (0x97))
        // Description:
        //   Load an array element selected by array reference and index using ldelem.i.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ldelem_i: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::loadResult(ops, 32);
        CilSemantics::loadArrayElement(dispatcher, ops, CilSemantics::ldelem_i_kind);
    }
};

// ldelem_r4 (152 (0x98))
        // Description:
        //   Load an array element selected by array reference and index using ldelem.r4.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ldelem_r4: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::loadResult(ops, 32);
        CilSemantics::loadArrayElement(dispatcher, ops, CilSemantics::ldelem_r4_kind);
    }
};

// ldelem_r8 (153 (0x99))
        // Description:
        //   Load an array element selected by array reference and index using ldelem.r8.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ldelem_r8: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::loadResult(ops, 64);
        CilSemantics::loadArrayElement(dispatcher, ops, CilSemantics::ldelem_r8_kind);
    }
};

// ldelem_ref (154 (0x9a))
        // Description:
        //   Load an array element selected by array reference and index using ldelem.ref.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ldelem_ref: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::loadResult(ops, 32);
        CilSemantics::loadArrayElement(dispatcher, ops, CilSemantics::ldelem_ref_kind);
    }
};


// stelem_i (155 (0x9b))
        // Description:
        //   Store an array element selected by array reference and index using stelem.i.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_stelem_i: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::storeEffect(ops);
        CilSemantics::storeArrayElement(dispatcher, ops, CilSemantics::stelem_i_kind);
    }
};

// stelem_i1 (156 (0x9c))
        // Description:
        //   Store an array element selected by array reference and index using stelem.i1.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_stelem_i1: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::storeEffect(ops);
        CilSemantics::storeArrayElement(dispatcher, ops, CilSemantics::stelem_i1_kind);
    }
};

// stelem_i2 (157 (0x9d))
        // Description:
        //   Store an array element selected by array reference and index using stelem.i2.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_stelem_i2: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::storeEffect(ops);
        CilSemantics::storeArrayElement(dispatcher, ops, CilSemantics::stelem_i2_kind);
    }
};

// stelem_i4 (158 (0x9e))
        // Description:
        //   Store an array element selected by array reference and index using stelem.i4.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_stelem_i4: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::storeEffect(ops);
        CilSemantics::storeArrayElement(dispatcher, ops, CilSemantics::stelem_i4_kind);
    }
};

// stelem_i8 (159 (0x9f))
        // Description:
        //   Store an array element selected by array reference and index using stelem.i8.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_stelem_i8: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::storeEffect(ops);
        CilSemantics::storeArrayElement(dispatcher, ops, CilSemantics::stelem_i8_kind);
    }
};

// stelem_r4 (160 (0xa0))
        // Description:
        //   Store an array element selected by array reference and index using stelem.r4.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_stelem_r4: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::storeEffect(ops);
        CilSemantics::storeArrayElement(dispatcher, ops, CilSemantics::stelem_r4_kind);
    }
};

// stelem_r8 (161 (0xa1))
        // Description:
        //   Store an array element selected by array reference and index using stelem.r8.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_stelem_r8: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::storeEffect(ops);
        CilSemantics::storeArrayElement(dispatcher, ops, CilSemantics::stelem_r8_kind);
    }
};

// stelem_ref (162 (0xa2))
        // Description:
        //   Store an array element selected by array reference and index using stelem.ref.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_stelem_ref: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::storeEffect(ops);
        CilSemantics::storeArrayElement(dispatcher, ops, CilSemantics::stelem_ref_kind);
    }
};

// ldelem (163 (0xa3))
        // Description:
        //   Load an array element selected by array reference and index using ldelem.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ldelem: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);          // InlineType token
     // CilSemantics::arrayLoadResult(ops, 32);
#if 0
        const uint32_t typeToken = CilSemantics::asIndex(args[0]);
        DispatcherCil::CilTypeAnalysis::TypeDescriptor type(typeToken);
#else
        const uint32_t typeToken = CilSemantics::asIndex(args[0]);
        DispatcherCil::CilTypeAnalysis::TypeDescriptor type = CilSemantics::resolveType(dispatcher, typeToken);
#endif
        CilSemantics::loadTypedArrayElement(dispatcher, ops, type);
    }
};

// stelem (164 (0xa4))
        // Description:
        //   Store an array element selected by array reference and index using stelem.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_stelem: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);          // InlineType token
     // CilSemantics::arrayStoreEffect(ops);
#if 0
        const uint32_t typeToken = CilSemantics::asIndex(args[0]);
        DispatcherCil::CilTypeAnalysis::TypeDescriptor type(typeToken);
#else
        const uint32_t typeToken = CilSemantics::asIndex(args[0]);
        DispatcherCil::CilTypeAnalysis::TypeDescriptor type = CilSemantics::resolveType(dispatcher, typeToken);
#endif
        CilSemantics::storeTypedArrayElement(dispatcher, ops, type);
    }
};


// unbox_any (165 (0xa5))
        // Description:
        //   Execute the CIL instruction unbox.any.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_unbox_any: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);

     // DQ (6/9/2026): Need to pass 4th parameter to unaryResult().
     // CilSemantics::unaryResult(dispatcher,ops,CilSemantics::unbox_any);
     // DispatcherCil::CilTypeAnalysis::TypeDescriptor targetType(CilSemantics::asIndex(args[0]));
        const uint32_t typeToken = CilSemantics::asIndex(args[0]);
        DispatcherCil::CilTypeAnalysis::TypeDescriptor targetType = CilSemantics::resolveType(dispatcher, typeToken);
        CilSemantics::unaryResult(dispatcher,ops,CilSemantics::unbox_any,&targetType);
    }
};

// conv_ovf_i1 (179 (0xb3))
        // Description:
        //   Convert the top evaluation-stack value according to conv.ovf.i1.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_conv_ovf_i1: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::convertResult(ops);
        CilSemantics::convertResult(dispatcher, ops, CilSemantics::conv_ovf_i1_kind, insn);
    }
};

// conv_ovf_u1 (180 (0xb4))
        // Description:
        //   Convert the top evaluation-stack value according to conv.ovf.u1.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_conv_ovf_u1: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::convertResult(ops);
        CilSemantics::convertResult(dispatcher, ops, CilSemantics::conv_ovf_u1_kind, insn);
    }
};

// conv_ovf_i2 (181 (0xb5))
        // Description:
        //   Convert the top evaluation-stack value according to conv.ovf.i2.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_conv_ovf_i2: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::convertResult(ops);
        CilSemantics::convertResult(dispatcher, ops, CilSemantics::conv_ovf_i2_kind, insn);
    }
};

// conv_ovf_u2 (182 (0xb6))
        // Description:
        //   Convert the top evaluation-stack value according to conv.ovf.u2.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_conv_ovf_u2: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::convertResult(ops);
        CilSemantics::convertResult(dispatcher, ops, CilSemantics::conv_ovf_u2_kind, insn);
    }
};

// conv_ovf_i4 (183 (0xb7))
        // Description:
        //   Convert the top evaluation-stack value according to conv.ovf.i4.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_conv_ovf_i4: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::convertResult(ops);
        CilSemantics::convertResult(dispatcher, ops, CilSemantics::conv_ovf_i4_kind, insn);
    }
};

// conv_ovf_u4 (184 (0xb8))
        // Description:
        //   Convert the top evaluation-stack value according to conv.ovf.u4.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_conv_ovf_u4: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::convertResult(ops);
        CilSemantics::convertResult(dispatcher, ops, CilSemantics::conv_ovf_u4_kind, insn);
    }
};

// conv_ovf_i8 (185 (0xb9))
        // Description:
        //   Convert the top evaluation-stack value according to conv.ovf.i8.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_conv_ovf_i8: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::convertResult(ops);
        CilSemantics::convertResult(dispatcher, ops, CilSemantics::conv_ovf_i8_kind, insn);
    }
};

// conv_ovf_u8 (186 (0xba))
        // Description:
        //   Convert the top evaluation-stack value according to conv.ovf.u8.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_conv_ovf_u8: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::convertResult(ops);
        CilSemantics::convertResult(dispatcher, ops, CilSemantics::conv_ovf_u8_kind, insn);
    }
};


// refanyval (194 (0xc2))
        // Description:
        //   Execute the CIL instruction refanyval.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_refanyval: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);

     // DQ (6/9/2026): Need to pass 4th parameter to unaryResult().
     // CilSemantics::unaryResult(dispatcher,ops,CilSemantics::refanyval);
     // DispatcherCil::CilTypeAnalysis::TypeDescriptor targetType(CilSemantics::asIndex(args[0]));
        const uint32_t typeToken = CilSemantics::asIndex(args[0]);
        DispatcherCil::CilTypeAnalysis::TypeDescriptor targetType = CilSemantics::resolveType(dispatcher, typeToken);
        CilSemantics::unaryResult(dispatcher,ops,CilSemantics::refanyval,&targetType);
    }
};

// ckfinite (195 (0xc3))
        // Description:
        //   Execute the CIL instruction ckfinite.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ckfinite: P {
  void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);

     // DQ (6/9/2026): Need to pass 4th parameter to unaryResult().
     // CilSemantics::unaryResult(dispatcher,ops,CilSemantics::ckfinite);
     // DispatcherCil::CilTypeAnalysis::TypeDescriptor targetType(CilSemantics::asIndex(args[0]));
     // CilSemantics::unaryResult(dispatcher,ops,CilSemantics::ckfinite,&targetType);
#if 0
        SValuePtr top = ops->peekOperand();

     // DQ (6/15/2026): This is failing either because I don't know how to push a floating point
     // number onto the stack, or the kind() value is not set in the parser.
        ASSERT_require(top->kind() == ValueKind::Float32 || top->kind() == ValueKind::Float64);

        uint64_t bits = top->get_number();
        uint64_t exponent = (bits >> 52) & 0x7ff;
        ASSERT_require2(exponent != 0x7ff, "ckfinite: value is not finite");
#else
        SValuePtr top = ops->peekOperand();
        ASSERT_not_null(top);

        if (top->kind() != ValueKind::Float32 && top->kind() != ValueKind::Float64)
           {
             ops->comment("CIL ckfinite: non-floating operand; InvalidProgramException path not fully modeled");
             SValue::Ptr exceptionObject = ops->undefined_(CilSemantics::cilPointerBits(dispatcher));
             exceptionObject->kind(ValueKind::ObjectReference);
             exceptionObject->typeDescriptor("cil:exception:System.InvalidProgramException");
             dispatcher->typeAnalysis().setNullness(
                 exceptionObject, DispatcherCil::CilTypeAnalysis::DefinitelyNonNull);
             ops->writeLocal(CilSemantics::cilCurrentExceptionSlot(), exceptionObject);
             CilSemantics::writeUnknownInstructionPointer(dispatcher, ops);
             return;
           }

        auto bitsOpt = top->toUnsigned();
        if (!bitsOpt)
           {
          // Symbolic float: it may be non-finite on some paths. Keep the
          // normal finite result on the stack, but do not write an active
          // ArithmeticException into this merged normal state.
             ops->comment("CIL ckfinite: symbolic floating operand; finite status unknown");
             return;
           }

        uint64_t bits = *bitsOpt;
        if (top->kind() == ValueKind::Float32)
           {
             uint32_t b = static_cast<uint32_t>(bits);
             uint32_t exponent = (b >> 23) & 0xffu;
             if (exponent == 0xffu)
                {
                  CilSemantics::arithmeticExceptionPath(
                      dispatcher, ops,
                      "CIL ckfinite: non-finite float32; ArithmeticException path not fully modeled");
                  return;
                }
           }
          else
           {
             uint64_t exponent = (bits >> 52) & 0x7ffu;
             if (exponent == 0x7ffu)
                {
                  CilSemantics::arithmeticExceptionPath(
                      dispatcher, ops,
                      "CIL ckfinite: non-finite float64; ArithmeticException path not fully modeled");
                  return;
                }
           }
#endif 
    }
};

// mkrefany (198 (0xc6))
        // Description:
        //   Execute the CIL instruction mkrefany.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_mkrefany: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        // mkrefany has an InlineType token, but the current conservative
        // semantics does not inspect the token. Accept either the decoded
        // token form or a decoder that does not expose it in the operand list.
        if (args.size() > 1)
            throw BaseSemantics::Exception("CIL instruction must have zero or one operand", insn);
#if CIL_DEBUG
        fprintf(stderr, "TOP of IP_mkrefany::p(): args.size() = %zu \n",args.size());
        fflush(stderr);
#endif
     // DQ (6/9/2026): Need to pass 4th parameter to unaryResult().
     // CilSemantics::unaryResult(dispatcher,ops,CilSemantics::mkrefany);
        if (args.size() > 0)
           {
          // DispatcherCil::CilTypeAnalysis::TypeDescriptor targetType(CilSemantics::asIndex(args[0]));
             const uint32_t typeToken = CilSemantics::asIndex(args[0]);
             DispatcherCil::CilTypeAnalysis::TypeDescriptor targetType = CilSemantics::resolveType(dispatcher, typeToken);
             CilSemantics::unaryResult(dispatcher,ops,CilSemantics::mkrefany,&targetType);
           }
          else
           {
             CilSemantics::unaryResult(dispatcher,ops,CilSemantics::mkrefany);
           }
    }
};

// ldtoken (208 (0xd0))
        // Description:
        //   Push a CIL value modeled here as an unknown 32-bit value for ldtoken.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ldtoken: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);
     // CilSemantics::pushUnknown(ops, 32);
        CilSemantics::loadToken(dispatcher, ops, insn, args);
    }
};


// conv_u2 (209 (0xd1))
        // Description:
        //   Convert the top evaluation-stack value according to conv.u2.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_conv_u2: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::convertResult(ops);
        CilSemantics::convertResult(dispatcher, ops, CilSemantics::conv_u2_kind, insn);
    }
};

// conv_u1 (210 (0xd2))
        // Description:
        //   Convert the top evaluation-stack value according to conv.u1.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_conv_u1: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::convertResult(ops);
        CilSemantics::convertResult(dispatcher, ops, CilSemantics::conv_u1_kind, insn);
    }
};

// conv_i (211 (0xd3))
        // Description:
        //   Convert the top evaluation-stack value according to conv.i.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_conv_i: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::convertResult(ops);
        CilSemantics::convertResult(dispatcher, ops, CilSemantics::conv_i_kind, insn);
    }
};

// conv_ovf_i (212 (0xd4))
        // Description:
        //   Convert the top evaluation-stack value according to conv.ovf.i.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_conv_ovf_i: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::convertResult(ops);
        CilSemantics::convertResult(dispatcher, ops, CilSemantics::conv_ovf_i_kind, insn);
    }
};

// conv_ovf_u (213 (0xd5))
        // Description:
        //   Convert the top evaluation-stack value according to conv.ovf.u.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_conv_ovf_u: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::convertResult(ops);
        CilSemantics::convertResult(dispatcher, ops, CilSemantics::conv_ovf_u_kind, insn);
    }
};

// add_ovf (214 (0xd6))
        // Description:
        //   Pop two values, compute add.ovf, and push the result onto the evaluation stack.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_add_ovf: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
        CilSemantics::binaryResult(dispatcher,ops,insn,CilSemantics::add_with_overflow_check);
    }
};

// add_ovf_un (215 (0xd7))
        // Description:
        //   Pop two values, compute add.ovf.un, and push the result onto the evaluation stack.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_add_ovf_un: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
        CilSemantics::binaryResult(dispatcher,ops,insn,CilSemantics::add_unsigned_with_overflow_check);
    }
};

// mul_ovf (216 (0xd8))
        // Description:
        //   Pop two values, compute mul.ovf, and push the result onto the evaluation stack.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_mul_ovf: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
        CilSemantics::binaryResult(dispatcher,ops,insn,CilSemantics::multiply_with_overflow_check);
    }
};

// mul_ovf_un (217 (0xd9))
        // Description:
        //   Pop two values, compute mul.ovf.un, and push the result onto the evaluation stack.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_mul_ovf_un: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
        CilSemantics::binaryResult(dispatcher,ops,insn,CilSemantics::multiply_unsigned_with_overflow_check);
    }
};

// sub_ovf (218 (0xda))
        // Description:
        //   Pop two values, compute sub.ovf, and push the result onto the evaluation stack.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_sub_ovf: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
        CilSemantics::binaryResult(dispatcher,ops,insn,CilSemantics::subtract_with_overflow_check);
    }
};

// sub_ovf_un (219 (0xdb))
        // Description:
        //   Pop two values, compute sub.ovf.un, and push the result onto the evaluation stack.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_sub_ovf_un: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
        CilSemantics::binaryResult(dispatcher,ops,insn,CilSemantics::subtract_unsigned_with_overflow_check);
    }
};


// endfinally (220 (0xdc))
        // Description:
        //   Execute the CIL instruction endfinally.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_endfinally: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::doNothing(ops);
        CilSemantics::endFinallyEffect(dispatcher, ops);
    }
};


// leave (221 (0xdd))
        // Description:
        //   Unconditionally transfer control to the inline target for leave.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_leave: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);
     // CilSemantics::branchEffect(dispatcher, ops, insn, args);
        CilSemantics::leaveEffect(dispatcher, ops, insn, args);
    }
};

// leave_s (222 (0xde))
        // Description:
        //   Unconditionally transfer control to the inline target for leave.s.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_leave_s: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);
     // CilSemantics::branchEffect(dispatcher, ops, insn, args);
        CilSemantics::leaveEffect(dispatcher, ops, insn, args);
    }
};

// stind_i (223 (0xdf))
        // Description:
        //   Store a value indirectly through the address on the evaluation stack using stind.i.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_stind_i: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::storeEffect(ops);
        CilSemantics::storeIndirectEffect(dispatcher, ops, CilSemantics::stind_i_kind);
    }
};

// conv_u (224 (0xe0))
        // Description:
        //   Convert the top evaluation-stack value according to conv.u.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_conv_u: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::convertResult(ops);
        CilSemantics::convertResult(dispatcher, ops, CilSemantics::conv_u_kind, insn);
    }
};

// arglist (65024 (0xfe00))
        // Description:
        //   Push a CIL value modeled here as an unknown 32-bit value for arglist.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_arglist: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::pushUnknown(ops, 32);
        ops->pushOperand(CilSemantics::runtimeArgumentHandle(dispatcher, ops));
    }
};


// ceq (65025 (0xfe01))
        // Description:
        //   Pop two values, compare them using ceq, and push an int32 boolean result.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ceq: P {
    void p(D /*dispatcher*/, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::compareResult(ops);
        CilSemantics::compareResult(ops, CilSemantics::compare_eq);
    }
};

// cgt (65026 (0xfe02))
        // Description:
        //   Pop two values, compare them using cgt, and push an int32 boolean result.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_cgt: P {
    void p(D /*dispatcher*/, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::compareResult(ops);
        CilSemantics::compareResult(ops, CilSemantics::compare_gt);
    }
};

// cgt_un (65027 (0xfe03))
        // Description:
        //   Pop two values, compare them using cgt.un, and push an int32 boolean result.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_cgt_un: P {
    void p(D /*dispatcher*/, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::compareResult(ops);
        CilSemantics::compareResult(ops, CilSemantics::compare_gt_un);
    }
};

// clt (65028 (0xfe04))
        // Description:
        //   Pop two values, compare them using clt, and push an int32 boolean result.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_clt: P {
    void p(D /*dispatcher*/, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::compareResult(ops);
        CilSemantics::compareResult(ops, CilSemantics::compare_lt);
    }
};

// clt_un (65029 (0xfe05))
        // Description:
        //   Pop two values, compare them using clt.un, and push an int32 boolean result.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_clt_un: P {
    void p(D /*dispatcher*/, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::compareResult(ops);
        CilSemantics::compareResult(ops, CilSemantics::compare_lt_un);
    }
};

// ldftn (65030 (0xfe06))
        // Description:
        //   Push a CIL value modeled here as an unknown 32-bit value for ldftn.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ldftn: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);
     // CilSemantics::pushUnknown(ops, 32);
        const uint32_t methodToken = CilSemantics::asIndex(args[0]);
        ops->pushOperand(CilSemantics::methodFunctionPointer(dispatcher, ops, methodToken));
    }
};

// DQ (6/9/2026): Newer version.

// ldvirtftn (254 7 (0xfe07))
// Description:
//   Resolves a virtual method on the object reference at the top of the stack
//   and pushes a native function pointer for the selected implementation.
// Stack:
//   ..., objref -> ..., native-function-pointer
// Notes:
//   The metadata operand is a method token, not a type token. Precise modeling
//   requires null checking, runtime type information, CLI virtual method
//   resolution, and a mapping from resolved methods to code addresses. Since
//   this dispatcher does not yet model method tables or virtual dispatch,
//   represent the result as an unknown pointer-sized function pointer. Do not
//   preserve the input kind.
// Run-time Exceptions:
//   NullReferenceException may be thrown if the object reference is null.
//   CIL/CLI exceptions are not modeled precisely by this initial dispatcher
//   skeleton.
struct IP_ldvirtftn: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);

     // DispatcherCil::CilTypeAnalysis::MethodDescriptor targetMethod(CilSemantics::asIndex(args[0]));
     // CilSemantics::ldvirtftnResult(dispatcher, ops, &targetMethod);

        const uint32_t methodToken = CilSemantics::asIndex(args[0]);
        DispatcherCil::CilTypeAnalysis::MethodDescriptor targetMethod(methodToken);
        CilSemantics::ldvirtftnResult(dispatcher, ops, targetMethod);
    }
};


// ldarg (65033 (0xfe09))
        // Description:
        //   Load an argument, selected by an inline index operand, onto the evaluation stack.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ldarg: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);
     // ops->pushOperand(ops->readLocal(CilSemantics::asIndex(args[0])));

        const uint32_t argIndex = CilSemantics::cilInlineIndex(args);
        CilSemantics::loadArgument(dispatcher, ops, argIndex);
    }
};

// ldarga (65034 (0xfe0a))
        // Description:
        //   Load the address of an argument, selected by an inline index operand, onto the evaluation stack.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ldarga: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);
     // CilSemantics::pushAddress(ops);

        const uint32_t argIndex = CilSemantics::cilInlineIndex(args);

     // Simple version:
     // CilSemantics::loadArgumentAddress(dispatcher, ops, argIndex);

     // Or, better if you want ldind.* through the address to read the current value:
        CilSemantics::materializeArgumentAddress(dispatcher, ops, argIndex);
    }
};

// starg (65035 (0xfe0b))
        // Description:
        //   Store the top evaluation-stack value into an argument selected by an inline index operand.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_starg: P {
  void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);
     // CilSemantics::discard(ops, 1);

        const uint32_t argIndex = CilSemantics::cilInlineIndex(args);
     // CilSemantics::storeArgument(ops, argIndex);
        CilSemantics::storeArgument(dispatcher, ops, argIndex);
     }
};

// ldloc (65036 (0xfe0c))
        // Description:
        //   Load a local variable, selected by an inline index operand, onto the evaluation stack.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ldloc: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);
     // ops->pushOperand(ops->readLocal(CilSemantics::asIndex(args[0])));

     // const uint32_t localIndex = CilSemantics::cilInlineIndex(args);
     // CilSemantics::loadLocal(ops, localIndex);

        const size_t localIndex = CilSemantics::asIndex(args[0]);
        CilSemantics::loadCilLocal(dispatcher, ops, localIndex);
    }
};

// ldloca (65037 (0xfe0d))
        // Description:
        //   Load the address of a local variable, selected by an inline index operand, onto the evaluation stack.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_ldloca: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);
     // CilSemantics::pushAddress(ops);
        const uint32_t localIndex = CilSemantics::cilInlineIndex(args);

     // Simple version:
     // CilSemantics::loadLocalAddress(dispatcher, ops, localIndex);

     // Or, better for ldind.*:
        CilSemantics::materializeLocalAddress(dispatcher, ops, localIndex);
    }
};

// stloc (65038 (0xfe0e))
        // Description:
        //   Store the top evaluation-stack value into a local variable selected by an inline index operand.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_stloc: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);
     // ops->writeLocal(CilSemantics::asIndex(args[0]), ops->popOperand());

        const uint32_t localIndex = CilSemantics::asIndex(args[0]);
        CilSemantics::storeLocal(dispatcher, ops, localIndex);
     }
};

// localloc (65039 (0xfe0f))
        // Description:
        //   Execute the CIL instruction localloc.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_localloc: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);

     // DQ (6/9/2026): Need to pass 4th parameter to unaryResult().
     // CilSemantics::unaryResult(dispatcher,ops,CilSemantics::localloc);
     // DispatcherCil::CilTypeAnalysis::TypeDescriptor targetType(CilSemantics::asIndex(args[0]));
     // CilSemantics::unaryResult(dispatcher,ops,CilSemantics::localloc,&targetType);
        CilSemantics::unaryResult(dispatcher,ops,CilSemantics::localloc);
    }
};
































// *****************************************************************************************
// *****************************************************************************************
// Remaining functions to review for better semantics and use of newer SValue class support.
// *****************************************************************************************
// *****************************************************************************************

































// endfilter (65041 (0xfe11))
        // Description:
        //   Execute the CIL instruction endfilter.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_endfilter: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::doNothing(ops);
        CilSemantics::endFilterEffect(dispatcher, ops);
    }
};

// unaligned (65042 (0xfe12))
        // Description:
        //   CIL prefix instruction that modifies interpretation of the following instruction: unaligned.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_unaligned: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);
     // CilSemantics::prefixEffect(ops);
        CilSemantics::unalignedPrefixEffect(dispatcher, ops, insn, args);
    }
};

// volatile (65043 (0xfe13))
        // Description:
        //   CIL prefix instruction that modifies interpretation of the following instruction: volatile.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_volatile: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::prefixEffect(ops);
        CilSemantics::volatilePrefixEffect(dispatcher, ops);
    }
};

// tail (65044 (0xfe14))
        // Description:
        //   CIL prefix instruction that modifies interpretation of the following instruction: tail.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_tail: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::prefixEffect(ops);
        CilSemantics::tailPrefixEffect(dispatcher, ops);
    }
};

// initobj (65045 (0xfe15))
        // Description:
        //   Execute the CIL instruction initobj.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_initobj: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);
     // CilSemantics::doNothing(ops);
#if 0
        const uint32_t typeToken = CilSemantics::asIndex(args[0]);
        DispatcherCil::CilTypeAnalysis::TypeDescriptor type(typeToken);
#else
        const uint32_t typeToken = CilSemantics::asIndex(args[0]);
        DispatcherCil::CilTypeAnalysis::TypeDescriptor type = CilSemantics::resolveType(dispatcher, typeToken);
#endif
        CilSemantics::initObjectEffect(dispatcher, ops, type);
    }
};

// constrained (65046 (0xfe16))
        // Description:
        //   CIL prefix instruction that modifies interpretation of the following instruction: constrained.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_constrained: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);
     // CilSemantics::prefixEffect(ops);
        CilSemantics::constrainedPrefixEffect(dispatcher, ops, insn, args);
    }
};

// cpblk (65047 (0xfe17))
        // Description:
        //   Copy a block of memory.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_cpblk: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::copyBlock(ops);
        CilSemantics::copyBlockEffect(dispatcher, ops);
    }
};

// initblk (65048 (0xfe18))
        // Description:
        //   Initialize a block of memory.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_initblk: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::initBlock(ops);
        CilSemantics::initBlockEffect(dispatcher, ops);
    }
};

// rethrow (65050 (0xfe1a))
        // Description:
        //   Throw or rethrow an exception using rethrow.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_rethrow: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::throwEffect(ops);
        CilSemantics::rethrowEffect(dispatcher, ops);
    }
};

// sizeof (65052 (0xfe1c))
        // Description:
        //   Push a CIL value modeled here as an unknown 32-bit value for sizeof.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_sizeof: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 1);
     // CilSemantics::pushUnknown(ops, 32);
#if 0
        const uint32_t typeToken = CilSemantics::asIndex(args[0]);
        DispatcherCil::CilTypeAnalysis::TypeDescriptor type(typeToken);
#else
        const uint32_t typeToken = CilSemantics::asIndex(args[0]);
        DispatcherCil::CilTypeAnalysis::TypeDescriptor type = CilSemantics::resolveType(dispatcher, typeToken);
#endif
        CilSemantics::sizeofResult(dispatcher, ops, type);
    }
};

// refanytype (65053 (0xfe1d))
        // Description:
        //   Execute the CIL instruction refanytype.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_refanytype: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);

     // DQ (6/9/2026): Need to pass 4th parameter to unaryResult().
     // CilSemantics::unaryResult(dispatcher,ops,CilSemantics::refanytype);
     // DispatcherCil::CilTypeAnalysis::TypeDescriptor targetType(CilSemantics::asIndex(args[0]));
     // CilSemantics::unaryResult(dispatcher,ops,CilSemantics::refanytype,&targetType);
        CilSemantics::unaryResult(dispatcher,ops,CilSemantics::refanytype);
    }
};

// readonly (65054 (0xfe1e))
        // Description:
        //   CIL prefix instruction that modifies interpretation of the following instruction: readonly.
        // Notes:
        //   Generated in the same per-instruction-processor style as DispatcherJvm.C. Some effects are conservative until metadata/type-aware CIL helpers are connected.
        // Run-time Exceptions:
        //   CIL/CLI exceptions are not modeled precisely by this initial dispatcher skeleton.
struct IP_readonly: P {
    void p(D dispatcher, Ops ops, I insn, Args args) override {
        assert_args(insn, args, 0);
     // CilSemantics::prefixEffect(ops);
        CilSemantics::readonlyPrefixEffect(dispatcher, ops);
    }
};

} // namespace Cil

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      DispatcherCil
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void
DispatcherCil::initializeDispatchTable() {
    iprocSet(0x00, new Cil::IP_nop);
    iprocSet(0x01, new Cil::IP_break);
    iprocSet(0x02, new Cil::IP_ldarg_0);
    iprocSet(0x03, new Cil::IP_ldarg_1);
    iprocSet(0x04, new Cil::IP_ldarg_2);
    iprocSet(0x05, new Cil::IP_ldarg_3);
    iprocSet(0x06, new Cil::IP_ldloc_0);
    iprocSet(0x07, new Cil::IP_ldloc_1);
    iprocSet(0x08, new Cil::IP_ldloc_2);
    iprocSet(0x09, new Cil::IP_ldloc_3);
    iprocSet(0x0a, new Cil::IP_stloc_0);
    iprocSet(0x0b, new Cil::IP_stloc_1);
    iprocSet(0x0c, new Cil::IP_stloc_2);
    iprocSet(0x0d, new Cil::IP_stloc_3);
    iprocSet(0x0e, new Cil::IP_ldarg_s);
    iprocSet(0x0f, new Cil::IP_ldarga_s);
    iprocSet(0x10, new Cil::IP_starg_s);
    iprocSet(0x11, new Cil::IP_ldloc_s);
    iprocSet(0x12, new Cil::IP_ldloca_s);
    iprocSet(0x13, new Cil::IP_stloc_s);
    iprocSet(0x14, new Cil::IP_ldnull);
    iprocSet(0x15, new Cil::IP_ldc_i4_m1);
    iprocSet(0x16, new Cil::IP_ldc_i4_0);
    iprocSet(0x17, new Cil::IP_ldc_i4_1);
    iprocSet(0x18, new Cil::IP_ldc_i4_2);
    iprocSet(0x19, new Cil::IP_ldc_i4_3);
    iprocSet(0x1a, new Cil::IP_ldc_i4_4);
    iprocSet(0x1b, new Cil::IP_ldc_i4_5);
    iprocSet(0x1c, new Cil::IP_ldc_i4_6);
    iprocSet(0x1d, new Cil::IP_ldc_i4_7);
    iprocSet(0x1e, new Cil::IP_ldc_i4_8);
    iprocSet(0x1f, new Cil::IP_ldc_i4_s);
    iprocSet(0x20, new Cil::IP_ldc_i4);
    iprocSet(0x21, new Cil::IP_ldc_i8);
    iprocSet(0x22, new Cil::IP_ldc_r4);
    iprocSet(0x23, new Cil::IP_ldc_r8);
    iprocSet(0x25, new Cil::IP_dup);
    iprocSet(0x26, new Cil::IP_pop);
    iprocSet(0x27, new Cil::IP_jmp);
    iprocSet(0x28, new Cil::IP_call);
    iprocSet(0x29, new Cil::IP_calli);
    iprocSet(0x2a, new Cil::IP_ret);
    iprocSet(0x2b, new Cil::IP_br_s);
    iprocSet(0x2c, new Cil::IP_brfalse_s);
    iprocSet(0x2d, new Cil::IP_brtrue_s);
    iprocSet(0x2e, new Cil::IP_beq_s);
    iprocSet(0x2f, new Cil::IP_bge_s);
    iprocSet(0x30, new Cil::IP_bgt_s);
    iprocSet(0x31, new Cil::IP_ble_s);
    iprocSet(0x32, new Cil::IP_blt_s);
    iprocSet(0x33, new Cil::IP_bne_un_s);
    iprocSet(0x34, new Cil::IP_bge_un_s);
    iprocSet(0x35, new Cil::IP_bgt_un_s);
    iprocSet(0x36, new Cil::IP_ble_un_s);
    iprocSet(0x37, new Cil::IP_blt_un_s);
    iprocSet(0x38, new Cil::IP_br);
    iprocSet(0x39, new Cil::IP_brfalse);
    iprocSet(0x3a, new Cil::IP_brtrue);
    iprocSet(0x3b, new Cil::IP_beq);
    iprocSet(0x3c, new Cil::IP_bge);
    iprocSet(0x3d, new Cil::IP_bgt);
    iprocSet(0x3e, new Cil::IP_ble);
    iprocSet(0x3f, new Cil::IP_blt);
    iprocSet(0x40, new Cil::IP_bne_un);
    iprocSet(0x41, new Cil::IP_bge_un);
    iprocSet(0x42, new Cil::IP_bgt_un);
    iprocSet(0x43, new Cil::IP_ble_un);
    iprocSet(0x44, new Cil::IP_blt_un);
    iprocSet(0x45, new Cil::IP_switch);
    iprocSet(0x46, new Cil::IP_ldind_i1);
    iprocSet(0x47, new Cil::IP_ldind_u1);
    iprocSet(0x48, new Cil::IP_ldind_i2);
    iprocSet(0x49, new Cil::IP_ldind_u2);
    iprocSet(0x4a, new Cil::IP_ldind_i4);
    iprocSet(0x4b, new Cil::IP_ldind_u4);
    iprocSet(0x4c, new Cil::IP_ldind_i8);
    iprocSet(0x4d, new Cil::IP_ldind_i);
    iprocSet(0x4e, new Cil::IP_ldind_r4);
    iprocSet(0x4f, new Cil::IP_ldind_r8);
    iprocSet(0x50, new Cil::IP_ldind_ref);
    iprocSet(0x51, new Cil::IP_stind_ref);
    iprocSet(0x52, new Cil::IP_stind_i1);
    iprocSet(0x53, new Cil::IP_stind_i2);
    iprocSet(0x54, new Cil::IP_stind_i4);
    iprocSet(0x55, new Cil::IP_stind_i8);
    iprocSet(0x56, new Cil::IP_stind_r4);
    iprocSet(0x57, new Cil::IP_stind_r8);
    iprocSet(0x58, new Cil::IP_add);
    iprocSet(0x59, new Cil::IP_sub);
    iprocSet(0x5a, new Cil::IP_mul);
    iprocSet(0x5b, new Cil::IP_div);
    iprocSet(0x5c, new Cil::IP_div_un);
    iprocSet(0x5d, new Cil::IP_rem);
    iprocSet(0x5e, new Cil::IP_rem_un);
    iprocSet(0x5f, new Cil::IP_and);
    iprocSet(0x60, new Cil::IP_or);
    iprocSet(0x61, new Cil::IP_xor);
    iprocSet(0x62, new Cil::IP_shl);
    iprocSet(0x63, new Cil::IP_shr);
    iprocSet(0x64, new Cil::IP_shr_un);
    iprocSet(0x65, new Cil::IP_neg);
    iprocSet(0x66, new Cil::IP_not);
    iprocSet(0x67, new Cil::IP_conv_i1);
    iprocSet(0x68, new Cil::IP_conv_i2);
    iprocSet(0x69, new Cil::IP_conv_i4);
    iprocSet(0x6a, new Cil::IP_conv_i8);
    iprocSet(0x6b, new Cil::IP_conv_r4);
    iprocSet(0x6c, new Cil::IP_conv_r8);
    iprocSet(0x6d, new Cil::IP_conv_u4);
    iprocSet(0x6e, new Cil::IP_conv_u8);
    iprocSet(0x6f, new Cil::IP_callvirt);
    iprocSet(0x70, new Cil::IP_cpobj);
    iprocSet(0x71, new Cil::IP_ldobj);
    iprocSet(0x72, new Cil::IP_ldstr);
    iprocSet(0x73, new Cil::IP_newobj);
    iprocSet(0x74, new Cil::IP_castclass);
    iprocSet(0x75, new Cil::IP_isinst);
    iprocSet(0x76, new Cil::IP_conv_r_un);
    iprocSet(0x79, new Cil::IP_unbox);
    iprocSet(0x7a, new Cil::IP_throw);
    iprocSet(0x7b, new Cil::IP_ldfld);
    iprocSet(0x7c, new Cil::IP_ldflda);
    iprocSet(0x7d, new Cil::IP_stfld);
    iprocSet(0x7e, new Cil::IP_ldsfld);
    iprocSet(0x7f, new Cil::IP_ldsflda);
    iprocSet(0x80, new Cil::IP_stsfld);
    iprocSet(0x81, new Cil::IP_stobj);
    iprocSet(0x82, new Cil::IP_conv_ovf_i1_un);
    iprocSet(0x83, new Cil::IP_conv_ovf_i2_un);
    iprocSet(0x84, new Cil::IP_conv_ovf_i4_un);
    iprocSet(0x85, new Cil::IP_conv_ovf_i8_un);
    iprocSet(0x86, new Cil::IP_conv_ovf_u1_un);
    iprocSet(0x87, new Cil::IP_conv_ovf_u2_un);
    iprocSet(0x88, new Cil::IP_conv_ovf_u4_un);
    iprocSet(0x89, new Cil::IP_conv_ovf_u8_un);
    iprocSet(0x8a, new Cil::IP_conv_ovf_i_un);
    iprocSet(0x8b, new Cil::IP_conv_ovf_u_un);
    iprocSet(0x8c, new Cil::IP_box);
    iprocSet(0x8d, new Cil::IP_newarr);
    iprocSet(0x8e, new Cil::IP_ldlen);
    iprocSet(0x8f, new Cil::IP_ldelema);
    iprocSet(0x90, new Cil::IP_ldelem_i1);
    iprocSet(0x91, new Cil::IP_ldelem_u1);
    iprocSet(0x92, new Cil::IP_ldelem_i2);
    iprocSet(0x93, new Cil::IP_ldelem_u2);
    iprocSet(0x94, new Cil::IP_ldelem_i4);
    iprocSet(0x95, new Cil::IP_ldelem_u4);
    iprocSet(0x96, new Cil::IP_ldelem_i8);
    iprocSet(0x97, new Cil::IP_ldelem_i);
    iprocSet(0x98, new Cil::IP_ldelem_r4);
    iprocSet(0x99, new Cil::IP_ldelem_r8);
    iprocSet(0x9a, new Cil::IP_ldelem_ref);
    iprocSet(0x9b, new Cil::IP_stelem_i);
    iprocSet(0x9c, new Cil::IP_stelem_i1);
    iprocSet(0x9d, new Cil::IP_stelem_i2);
    iprocSet(0x9e, new Cil::IP_stelem_i4);
    iprocSet(0x9f, new Cil::IP_stelem_i8);
    iprocSet(0xa0, new Cil::IP_stelem_r4);
    iprocSet(0xa1, new Cil::IP_stelem_r8);
    iprocSet(0xa2, new Cil::IP_stelem_ref);
    iprocSet(0xa3, new Cil::IP_ldelem);
    iprocSet(0xa4, new Cil::IP_stelem);
    iprocSet(0xa5, new Cil::IP_unbox_any);
    iprocSet(0xb3, new Cil::IP_conv_ovf_i1);
    iprocSet(0xb4, new Cil::IP_conv_ovf_u1);
    iprocSet(0xb5, new Cil::IP_conv_ovf_i2);
    iprocSet(0xb6, new Cil::IP_conv_ovf_u2);
    iprocSet(0xb7, new Cil::IP_conv_ovf_i4);
    iprocSet(0xb8, new Cil::IP_conv_ovf_u4);
    iprocSet(0xb9, new Cil::IP_conv_ovf_i8);
    iprocSet(0xba, new Cil::IP_conv_ovf_u8);
    iprocSet(0xc2, new Cil::IP_refanyval);
    iprocSet(0xc3, new Cil::IP_ckfinite);
    iprocSet(0xc6, new Cil::IP_mkrefany);
    iprocSet(0xd0, new Cil::IP_ldtoken);
    iprocSet(0xd1, new Cil::IP_conv_u2);
    iprocSet(0xd2, new Cil::IP_conv_u1);
    iprocSet(0xd3, new Cil::IP_conv_i);
    iprocSet(0xd4, new Cil::IP_conv_ovf_i);
    iprocSet(0xd5, new Cil::IP_conv_ovf_u);
    iprocSet(0xd6, new Cil::IP_add_ovf);
    iprocSet(0xd7, new Cil::IP_add_ovf_un);
    iprocSet(0xd8, new Cil::IP_mul_ovf);
    iprocSet(0xd9, new Cil::IP_mul_ovf_un);
    iprocSet(0xda, new Cil::IP_sub_ovf);
    iprocSet(0xdb, new Cil::IP_sub_ovf_un);
    iprocSet(0xdc, new Cil::IP_endfinally);
    iprocSet(0xdd, new Cil::IP_leave);
    iprocSet(0xde, new Cil::IP_leave_s);
    iprocSet(0xdf, new Cil::IP_stind_i);
    iprocSet(0xe0, new Cil::IP_conv_u);
    iprocSet(0xfe00, new Cil::IP_arglist);
    iprocSet(0xfe01, new Cil::IP_ceq);
    iprocSet(0xfe02, new Cil::IP_cgt);
    iprocSet(0xfe03, new Cil::IP_cgt_un);
    iprocSet(0xfe04, new Cil::IP_clt);
    iprocSet(0xfe05, new Cil::IP_clt_un);
    iprocSet(0xfe06, new Cil::IP_ldftn);
    iprocSet(0xfe07, new Cil::IP_ldvirtftn);
    iprocSet(0xfe09, new Cil::IP_ldarg);
    iprocSet(0xfe0a, new Cil::IP_ldarga);
    iprocSet(0xfe0b, new Cil::IP_starg);
    iprocSet(0xfe0c, new Cil::IP_ldloc);
    iprocSet(0xfe0d, new Cil::IP_ldloca);
    iprocSet(0xfe0e, new Cil::IP_stloc);
    iprocSet(0xfe0f, new Cil::IP_localloc);
    iprocSet(0xfe11, new Cil::IP_endfilter);
    iprocSet(0xfe12, new Cil::IP_unaligned);
    iprocSet(0xfe13, new Cil::IP_volatile);
    iprocSet(0xfe14, new Cil::IP_tail);
    iprocSet(0xfe15, new Cil::IP_initobj);
    iprocSet(0xfe16, new Cil::IP_constrained);
    iprocSet(0xfe17, new Cil::IP_cpblk);
    iprocSet(0xfe18, new Cil::IP_initblk);
    iprocSet(0xfe1a, new Cil::IP_rethrow);
    iprocSet(0xfe1c, new Cil::IP_sizeof);
    iprocSet(0xfe1d, new Cil::IP_refanytype);
    iprocSet(0xfe1e, new Cil::IP_readonly);
}

DispatcherCil::DispatcherCil() {}

DispatcherCil::~DispatcherCil() {}

DispatcherCil::DispatcherCil(const Architecture::Base::ConstPtr &arch)
    : BaseSemantics::Dispatcher(arch) {}

DispatcherCil::DispatcherCil(const Architecture::Base::ConstPtr &arch, const BaseSemantics::RiscOperators::Ptr &ops)
    : Super(arch, ops),
      REG_PC(arch->registerDictionary()->findOrThrow("pc")) {
    initializeDispatchTable();
    initializeMemoryState();
    initializeState(ops->currentState());
}

DispatcherCil::Ptr
DispatcherCil::instance(const Architecture::Base::ConstPtr &arch) {
    return Ptr(new DispatcherCil(arch));
}

DispatcherCil::Ptr
DispatcherCil::instance(const Architecture::Base::ConstPtr &arch, const BaseSemantics::RiscOperators::Ptr &ops) {
    return Ptr(new DispatcherCil(arch, ops));
}

BaseSemantics::Dispatcher::Ptr
DispatcherCil::create(const BaseSemantics::RiscOperators::Ptr &ops) const {
    return instance(architecture(), ops);
}

DispatcherCil::Ptr
DispatcherCil::promote(const BaseSemantics::Dispatcher::Ptr &d) {
    Ptr retval = as<DispatcherCil>(d);
    ASSERT_not_null(retval);
    return retval;
}


// Compute the instruction-processor table key from the actual CIL opcode bytes.
//
// CIL has two opcode encodings:
//   - one-byte opcodes:          xx
//   - extended two-byte opcodes: fe xx
//
// initializeDispatchTable registers extended opcodes as:
//
//   0xfe00 | secondOpcodeByte
//
// so all dispatcher lookup paths must use this same encoding.
     static int
     cilOpcodeDispatchKey(SgAsmInstruction *insn)
        {
          ASSERT_not_null(insn);

          const SgUnsignedCharList &bytes = insn->get_rawBytes();
          ASSERT_require(!bytes.empty());

          const unsigned first = static_cast<unsigned>(bytes[0]);

          if (first == 0xfe)
             {
               ASSERT_require2(bytes.size() >= 2, "truncated CIL extended opcode");
               return static_cast<int>(0xfe00u | static_cast<unsigned>(bytes[1]));
             }

          return static_cast<int>(first);
        }

     int
     DispatcherCil::iprocKey(SgAsmInstruction *insn_) const
        {
          SgAsmCilInstruction *insn = isSgAsmCilInstruction(insn_);
          ASSERT_not_null(insn);

          return cilOpcodeDispatchKey(insn);
        }

     InsnProcessor*
     DispatcherCil::iprocLookup(SgAsmInstruction *insn)
        {
          SgAsmCilInstruction *cilInsn = isSgAsmCilInstruction(insn);
          ASSERT_not_null(cilInsn);

          const int key = cilOpcodeDispatchKey(cilInsn);
          ASSERT_require(key >= 0);

          return iprocGet(key);
        }

void
DispatcherCil::initializeMemoryState() {
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

RegisterDescriptor
DispatcherCil::instructionPointerRegister() const {
    return REG_PC;
}

RegisterDescriptor
DispatcherCil::stackPointerRegister() const {
    return RegisterDescriptor();
}


SValuePtr
DispatcherCil::condition(CilInstructionKind, RiscOperators *ops) {
    ASSERT_not_null(ops);
    return ops->undefined_(1);
}

BaseSemantics::SValuePtr
DispatcherCil::read(SgAsmExpression *e, size_t value_nbits, size_t addr_nbits/*=0*/) {
    ASSERT_not_null(e);
    return Dispatcher::read(e, value_nbits, addr_nbits);
}

void
DispatcherCil::updateFpsrExcInan(const SValuePtr&, SgAsmType*, const SValuePtr&, SgAsmType*) {}

void
DispatcherCil::updateFpsrExcIde(const SValuePtr&, SgAsmType*, const SValuePtr&, SgAsmType*) {}

void
DispatcherCil::updateFpsrExcOvfl(const SValuePtr&, SgAsmType*, SgAsmType*, SgAsmType*) {}

void
DispatcherCil::updateFpsrExcUnfl(const SValuePtr&, SgAsmType*, SgAsmType*, SgAsmType*) {}

void
DispatcherCil::updateFpsrExcInex() {}

void
DispatcherCil::accumulateFpExceptions() {}

void
DispatcherCil::adjustFpConditionCodes(const SValuePtr&, SgAsmFloatType*) {}

} // namespace InstructionSemantics
} // namespace BinaryAnalysis
} // namespace Rose

#ifdef ROSE_HAVE_BOOST_SERIALIZATION_LIB
// BOOST_CLASS_EXPORT_IMPLEMENT(Rose::BinaryAnalysis::InstructionSemantics::DispatcherCil);
#endif

#endif
