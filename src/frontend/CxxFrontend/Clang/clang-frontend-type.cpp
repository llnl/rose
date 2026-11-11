#include "sage3basic.h"
#include "clang-frontend-private.hpp"

using namespace Sawyer::Message;
SgType * ClangToSageTranslator::buildTypeFromQualifiedType(const clang::QualType & qual_type) {
    SgNode * tmp_type = Traverse(qual_type.getTypePtr());
    SgType * type = isSgType(tmp_type);

    ROSE_ASSERT(type != NULL); 

    if (qual_type.hasLocalQualifiers()) {
        SgModifierType * modified_type = new SgModifierType(type);
        SgTypeModifier & sg_modifer = modified_type->get_typeModifier();
        clang::Qualifiers qualifier = qual_type.getLocalQualifiers();

        if (qualifier.hasConst()) sg_modifer.get_constVolatileModifier().setConst();
        if (qualifier.hasVolatile()) sg_modifer.get_constVolatileModifier().setVolatile();
        if (qualifier.hasRestrict()) sg_modifer.setRestrict();
        
        if (qualifier.hasAddressSpace()) {
            clang::LangAS addrspace = qualifier.getAddressSpace();
            switch (addrspace) {
                case clang::LangAS::opencl_global:
                    sg_modifer.setOpenclGlobal();
                    break;
                case clang::LangAS::opencl_local:
                    sg_modifer.setOpenclLocal();
                    break;
                case clang::LangAS::opencl_constant:
                    sg_modifer.setOpenclConstant();
                    break;
                default:
                    sg_modifer.setAddressSpace();
                    sg_modifer.set_address_space_value(static_cast<unsigned int>(addrspace));
            }
        }
        modified_type = SgModifierType::insertModifierTypeIntoTypeTable(modified_type);

        return modified_type;
    }
    else {
        return type;
    }
}

SgNode * ClangToSageTranslator::Traverse(const clang::Type * type) {
    if (type == NULL)
        return NULL;

    std::map<const clang::Type *, SgNode *>::iterator it = p_type_translation_map.find(type);
#if DEBUG_TRAVERSE_TYPE
    logger[DEBUG] << "Traverse Type : " << type << " " << type->getTypeClassName ()<< "\n";
#endif
    if (it != p_type_translation_map.end()) {
#if DEBUG_TRAVERSE_TYPE
      logger[DEBUG] << " already visited : node = " << it->second << "\n";
#endif
      return it->second;
    }

    SgNode * result = NULL;
    bool ret_status = false;

    switch (type->getTypeClass()) {
        case clang::Type::Decayed:
            ret_status = VisitDecayedType((clang::DecayedType *)type, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Type::ConstantArray:
            ret_status = VisitConstantArrayType((clang::ConstantArrayType *)type, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Type::DependentSizedArray:
            ret_status = VisitDependentSizedArrayType((clang::DependentSizedArrayType *)type, &result);
            ROSE_ASSERT(result != NULL);
            break;
       case clang::Type::IncompleteArray:
            ret_status = VisitIncompleteArrayType((clang::IncompleteArrayType *)type, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Type::VariableArray:
            ret_status = VisitVariableArrayType((clang::VariableArrayType *)type, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Type::Atomic:
            ret_status = VisitAtomicType((clang::AtomicType *)type, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Type::Attributed:
            ret_status = VisitAttributedType((clang::AttributedType *)type, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Type::BlockPointer:
            ret_status = VisitBlockPointerType((clang::BlockPointerType *)type, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Type::Builtin:
            ret_status = VisitBuiltinType((clang::BuiltinType *)type, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Type::Complex:
            ret_status = VisitComplexType((clang::ComplexType *)type, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Type::Decltype:
            ret_status = VisitDecltypeType((clang::DecltypeType *)type, &result);
            ROSE_ASSERT(result != NULL);
            break;
     // case clang::Type::DependentDecltype:
     //     ret_status = VisitDependentDecltypeType((clang::DependentDecltypeType *)type, &result);
     //     break;
        case clang::Type::Auto:
            ret_status = VisitAutoType((clang::AutoType *)type, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Type::DeducedTemplateSpecialization:
            ret_status = VisitDeducedTemplateSpecializationType((clang::DeducedTemplateSpecializationType *)type, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Type::DependentSizedExtVector:
            ret_status = VisitDependentSizedExtVectorType((clang::DependentSizedExtVectorType *)type, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Type::DependentVector:
            ret_status = VisitDependentVectorType((clang::DependentVectorType *)type, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Type::FunctionNoProto:
            ret_status = VisitFunctionNoProtoType((clang::FunctionNoProtoType *)type, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Type::FunctionProto:
            ret_status = VisitFunctionProtoType((clang::FunctionProtoType *)type, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Type::InjectedClassName:
            ret_status = VisitInjectedClassNameType((clang::InjectedClassNameType *)type, &result);
            ROSE_ASSERT(result != NULL);
            break;
     // case clang::Type::LocInfo:
     //     ret_status = VisitLocInfoType((clang::LocInfoType *)type, &result);
     //     break;
        case clang::Type::MacroQualified:
            ret_status = VisitMacroQualifiedType((clang::MacroQualifiedType *)type, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Type::MemberPointer:
            ret_status = VisitMemberPointerType((clang::MemberPointerType *)type, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Type::PackExpansion:
            ret_status = VisitPackExpansionType((clang::PackExpansionType *)type, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Type::Paren:
            ret_status = VisitParenType((clang::ParenType *)type, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Type::Pipe:
            ret_status = VisitPipeType((clang::PipeType *)type, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Type::Pointer:
            ret_status = VisitPointerType((clang::PointerType *)type, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Type::LValueReference:
            ret_status = VisitLValueReferenceType((clang::LValueReferenceType *)type, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Type::RValueReference:
            ret_status = VisitRValueReferenceType((clang::RValueReferenceType *)type, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Type::SubstTemplateTypeParmPack:
            ret_status = VisitSubstTemplateTypeParmPackType((clang::SubstTemplateTypeParmPackType *)type, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Type::SubstTemplateTypeParm:
            ret_status = VisitSubstTemplateTypeParmType((clang::SubstTemplateTypeParmType *)type, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Type::Enum:
            ret_status = VisitEnumType((clang::EnumType *)type, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Type::Record:
            ret_status = VisitRecordType((clang::RecordType *)type, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Type::TemplateSpecialization:
            ret_status = VisitTemplateSpecializationType((clang::TemplateSpecializationType *)type, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Type::TemplateTypeParm:
            ret_status = VisitTemplateTypeParmType((clang::TemplateTypeParmType *)type, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Type::Typedef:
            ret_status = VisitTypedefType((clang::TypedefType *)type, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Type::TypeOfExpr:
            ret_status = VisitTypeOfExprType((clang::TypeOfExprType *)type, &result);
            ROSE_ASSERT(result != NULL);
            break;
    //  case clang::Type::DependentTypeOfExpr:
    //      ret_status = VisitDependentTypeOfExprType((clang::DependentTypeOfExprType *)type, &result);
    //      break;
        case clang::Type::TypeOf:
            ret_status = VisitTypeOfType((clang::TypeOfType *)type, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Type::DependentName:
            ret_status = VisitDependentNameType((clang::DependentNameType *)type, &result);
            ROSE_ASSERT(result != NULL);
            break;
        // (10/30/2026) lin32: Clang 22 removed ElaboratedType and merged dependent specializations into TemplateSpecializationType.
#if CLANG_VERSION_MAJOR < 22
        case clang::Type::DependentTemplateSpecialization:
            ret_status = VisitDependentTemplateSpecializationType((clang::DependentTemplateSpecializationType *)type, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Type::Elaborated:
            ret_status = VisitElaboratedType((clang::ElaboratedType *)type, &result);
            ROSE_ASSERT(result != NULL);
            break;
#endif
        case clang::Type::UnaryTransform:
            ret_status = VisitUnaryTransformType((clang::UnaryTransformType *)type, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Type::UnresolvedUsing:
            ret_status = VisitUnresolvedUsingType((clang::UnresolvedUsingType *)type, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Type::Vector:
            ret_status = VisitVectorType((clang::VectorType *)type, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Type::ExtVector:
            ret_status = VisitExtVectorType((clang::ExtVectorType *)type, &result);
            ROSE_ASSERT(result != NULL);
            break;

        default:
            logger[ERROR] << "Unhandled type" << "\n";
            ROSE_ABORT();
    }

    ROSE_ASSERT(result != NULL);

    p_type_translation_map.insert(std::pair<const clang::Type *, SgNode *>(type, result));

#if DEBUG_TRAVERSE_TYPE
    logger[DEBUG] << "Traverse(clang::Type : " << type << " ";
    logger[DEBUG] << " visit done : node = " << result << "\n";
#endif
    return result;
}

/***************/
/* Visit Types */
/***************/

bool ClangToSageTranslator::VisitType(clang::Type * type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToSageTranslator::VisitType" << "\n";
#endif

    if (*node == NULL) {
        logger[WARN] << "Runtime error: No Sage node associated with the type..." << "\n";
        return false;
    }
/*
    logger[DEBUG] << "Dump type " << type->getTypeClassName() << "(" << type << "): ";
    type->dump();
    logger[DEBUG] << "\n";
*/
    // TODO

    return true;
}

bool ClangToSageTranslator::VisitAdjustedType(clang::AdjustedType * adjusted_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToSageTranslator::VisitAdjustedType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitType(adjusted_type, node) && res;
}

bool ClangToSageTranslator::VisitDecayedType(clang::DecayedType * decayed_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToSageTranslator::VisitDecayedType" << "\n";
#endif
    bool res = true;

//    SgType * decayType = buildTypeFromQualifiedType(decayed_type->getDecayedType ());
    SgType * pointeeType = buildTypeFromQualifiedType(decayed_type->getPointeeType ());

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

//    *node = pointeeType;
// Pei-Hung (04/08/2022) Building SgArrayyType to represent the DecayedType in Clang,
// in order to match the type of ParmVarDecl  in FunctionProtoType
// Might need to check the case when the pointeeType is a functionType
    if(decayed_type->getPointeeType()->getTypeClass() == clang::Type::VariableArray ||
       decayed_type->getPointeeType()->getTypeClass() == clang::Type::ConstantArray ||
       decayed_type->getPointeeType()->getTypeClass() == clang::Type::DependentSizedArray ||
       decayed_type->getPointeeType()->getTypeClass() == clang::Type::IncompleteArray 
       )
      *node = SageBuilder::buildArrayType(pointeeType);
    else 
      *node = pointeeType;

    return VisitAdjustedType(decayed_type, node) && res;
}

bool ClangToSageTranslator::VisitArrayType(clang::ArrayType * array_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToSageTranslator::VisitArrayType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME

 // DQ (11/28/2020): Added assertion.
    ROSE_ASSERT(*node != NULL);

    return VisitType(array_type, node) && res;
}

bool ClangToSageTranslator::VisitConstantArrayType(clang::ConstantArrayType * constant_array_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToSageTranslator::VisitConstantArrayType" << "\n";
#endif

    SgType * type = buildTypeFromQualifiedType(constant_array_type->getElementType());

    // TODO clang::ArrayType::ArraySizeModifier

    SgExpression * expr = SageBuilder::buildIntVal(constant_array_type->getSize().getSExtValue());

    *node = SageBuilder::buildArrayType(type, expr);

    return VisitArrayType(constant_array_type, node);
}

bool ClangToSageTranslator::VisitDependentSizedArrayType(clang::DependentSizedArrayType * dependent_sized_array_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToSageTranslator::VisitDependentSizedArrayType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitArrayType(dependent_sized_array_type, node) && res;
}

bool ClangToSageTranslator::VisitIncompleteArrayType(clang::IncompleteArrayType * incomplete_array_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToSageTranslator::VisitIncompleteArrayType" << "\n";
#endif

    SgType * type = buildTypeFromQualifiedType(incomplete_array_type->getElementType());

    // TODO clang::ArrayType::ArraySizeModifier

#if CLANG_VERSION_MAJOR >= 18
    clang::ArraySizeModifier sizeModifier = incomplete_array_type->getSizeModifier();

    if(sizeModifier == clang::ArraySizeModifier::Star)
    {
      SgExprListExp* exprListExp = SageBuilder::buildExprListExp(SageBuilder::buildNullExpression());
      *node = SageBuilder::buildArrayType(type, exprListExp);
    }
    else if(sizeModifier == clang::ArraySizeModifier::Static)
    {
      // TODO check how to handle Static 
      *node = SageBuilder::buildArrayType(type);
    }
    else  // clang::ArrayType::ArraySizeModifier::Normal
    {
      *node = SageBuilder::buildArrayType(type);
    }
#else
    clang::ArrayType::ArraySizeModifier sizeModifier = incomplete_array_type->getSizeModifier();

    if(sizeModifier == clang::ArrayType::ArraySizeModifier::Star)
    {
      SgExprListExp* exprListExp = SageBuilder::buildExprListExp(SageBuilder::buildNullExpression());
      *node = SageBuilder::buildArrayType(type, exprListExp);
    }
    else if(sizeModifier == clang::ArrayType::ArraySizeModifier::Static)
    {
      // TODO check how to handle Static 
      *node = SageBuilder::buildArrayType(type);
    }
    else  // clang::ArrayType::ArraySizeModifier::Normal
    {
      *node = SageBuilder::buildArrayType(type);
    }
#endif


 // DQ (11/28/2020): Added assertion.
 // ROSE_ASSERT(*node != NULL);

    return VisitArrayType(incomplete_array_type, node);
}

bool ClangToSageTranslator::VisitVariableArrayType(clang::VariableArrayType * variable_array_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToSageTranslator::VisitVariableArrayType" << "\n";
#endif
    bool res = true;

    SgType * type = buildTypeFromQualifiedType(variable_array_type->getElementType());

    SgNode* tmp_expr = Traverse(variable_array_type->getSizeExpr());
    SgExpression* array_size = isSgExpression(tmp_expr);

    SgArrayType* arrayType = SageBuilder::buildArrayType(type,array_size);
    arrayType->set_is_variable_length_array(true);
    *node = arrayType;

 // DQ (11/28/2020): Added assertion.
    ROSE_ASSERT(*node != NULL);

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitArrayType(variable_array_type, node) && res;
}

bool ClangToSageTranslator::VisitAtomicType(clang::AtomicType * atomic_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToSageTranslator::VisitAtomicType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitType(atomic_type, node) && res;
}

bool ClangToSageTranslator::VisitAttributedType(clang::AttributedType * attributed_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToSageTranslator::VisitAttributedType" << "\n";
#endif

    SgType * type = buildTypeFromQualifiedType(attributed_type->getModifiedType());

    SgModifierType * modified_type = SgModifierType::createType(type);
    SgTypeModifier & sg_modifer = modified_type->get_typeModifier();

//(01/29/2020) Pei-Hung needs to revisit this part.
/*
    switch (attributed_type->getAttrKind()) {
        case clang::AttributedType::attr_noreturn:             sg_modifer.setGnuAttributeNoReturn();      break;
        case clang::AttributedType::attr_cdecl:                sg_modifer.setGnuAttributeCdecl();         break;
        case clang::AttributedType::attr_stdcall:              sg_modifer.setGnuAttributeStdcall();       break;

        case clang::AttributedType::attr_address_space:
            logger[ERROR] << "Unsupported attribute attr_address_space" << "\n"; ROSE_ASSERT(false);
        case clang::AttributedType::attr_regparm:
            logger[ERROR] << "Unsupported attribute attr_regparm" << "\n"; ROSE_ASSERT(false);
        case clang::AttributedType::attr_vector_size:
            logger[ERROR] << "Unsupported attribute attr_vector_size" << "\n"; ROSE_ASSERT(false);
        case clang::AttributedType::attr_neon_vector_type:
            logger[ERROR] << "Unsupported attribute attr_neon_vector_type" << "\n"; ROSE_ASSERT(false);
        case clang::AttributedType::attr_neon_polyvector_type:
            logger[ERROR] << "Unsupported attribute attr_neon_polyvector_type" << "\n"; ROSE_ASSERT(false);
        case clang::AttributedType::attr_objc_gc:
            logger[ERROR] << "Unsupported attribute attr_objc_gc" << "\n"; ROSE_ASSERT(false);
        case clang::AttributedType::attr_objc_ownership:
            logger[ERROR] << "Unsupported attribute attr_objc_ownership" << "\n"; ROSE_ASSERT(false);
        case clang::AttributedType::attr_pcs:
            logger[ERROR] << "Unsupported attribute attr_pcs" << "\n"; ROSE_ASSERT(false);
        case clang::AttributedType::attr_fastcall:
            logger[ERROR] << "Unsupported attribute attr_fastcall" << "\n"; ROSE_ASSERT(false);
        case clang::AttributedType::attr_thiscall:
            logger[ERROR] << "Unsupported attribute attr_thiscall" << "\n"; ROSE_ASSERT(false);
        case clang::AttributedType::attr_pascal:
            logger[ERROR] << "Unsupported attribute attr_pascal" << "\n"; ROSE_ASSERT(false);
        default:
            logger[ERROR] << "Unknown attribute" << "\n"; ROSE_ASSERT(false);
    } 
*/
    *node = SgModifierType::insertModifierTypeIntoTypeTable(modified_type);;

    return VisitType(attributed_type, node);
}

bool ClangToSageTranslator::VisitBlockPointerType(clang::BlockPointerType * block_pointer_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToSageTranslator::VisitBlockPointerType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitType(block_pointer_type, node) && res;
}

bool ClangToSageTranslator::VisitBuiltinType(clang::BuiltinType * builtin_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToSageTranslator::VisitBuiltinType" << "\n";
#endif

    switch (builtin_type->getKind()) {
        case clang::BuiltinType::Void:       *node = SageBuilder::buildVoidType();             break;
        case clang::BuiltinType::Bool:       *node = SageBuilder::buildBoolType();             break;
        case clang::BuiltinType::Short:      *node = SageBuilder::buildShortType();            break;
        case clang::BuiltinType::Int:        *node = SageBuilder::buildIntType();              break;
        case clang::BuiltinType::Long:       *node = SageBuilder::buildLongType();             break;
        case clang::BuiltinType::LongLong:   *node = SageBuilder::buildLongLongType();         break;
        case clang::BuiltinType::Float:      *node = SageBuilder::buildFloatType();            break;
        case clang::BuiltinType::Double:     *node = SageBuilder::buildDoubleType();           break;
        case clang::BuiltinType::LongDouble: *node = SageBuilder::buildLongDoubleType();       break;

        case clang::BuiltinType::Char_S:     *node = SageBuilder::buildCharType();             break;

        case clang::BuiltinType::UInt:       *node = SageBuilder::buildUnsignedIntType();      break;
        case clang::BuiltinType::UChar:      *node = SageBuilder::buildUnsignedCharType();     break;
        case clang::BuiltinType::SChar:      *node = SageBuilder::buildSignedCharType();       break;
        case clang::BuiltinType::UShort:     *node = SageBuilder::buildUnsignedShortType();    break;
        case clang::BuiltinType::ULong:      *node = SageBuilder::buildUnsignedLongType();     break;
        case clang::BuiltinType::ULongLong:  *node = SageBuilder::buildUnsignedLongLongType(); break;
/*
        case clang::BuiltinType::NullPtr:    *node = SageBuilder::build(); break;
*/
        // TODO ROSE type ?
        case clang::BuiltinType::UInt128:    *node = SageBuilder::buildUnsignedLongLongType(); break;
        case clang::BuiltinType::Int128:     *node = SageBuilder::buildLongLongType();         break;
 
        case clang::BuiltinType::Char_U:    logger[WARN] << "Char_U    -> "; break;
        case clang::BuiltinType::WChar_U:   logger[WARN] << "WChar_U   -> "; break;
        case clang::BuiltinType::Char16:    logger[WARN] << "Char16    -> "; break;
        case clang::BuiltinType::Char32:    logger[WARN] << "Char32    -> "; break;
        case clang::BuiltinType::WChar_S:   logger[WARN] << "WChar_S   -> "; break;


        case clang::BuiltinType::ObjCId:
        case clang::BuiltinType::ObjCClass:
        case clang::BuiltinType::ObjCSel:
        case clang::BuiltinType::Dependent:
        case clang::BuiltinType::Overload:
        case clang::BuiltinType::BoundMember:
        case clang::BuiltinType::UnknownAny:
        default:
            logger[ERROR] << "Unknown builtin type: " << builtin_type->getName(p_compiler_instance->getLangOpts()).str() << " !" << "\n";
            exit(-1);
    }

    return VisitType(builtin_type, node);
}

bool ClangToSageTranslator::VisitComplexType(clang::ComplexType * complex_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToSageTranslator::VisitComplexType" << "\n";
#endif

    bool res = true;

    SgType * type = buildTypeFromQualifiedType(complex_type->getElementType());

    *node = SageBuilder::buildComplexType(type);

    return VisitType(complex_type, node) && res;
}

bool ClangToSageTranslator::VisitDecltypeType(clang::DecltypeType * decltype_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToSageTranslator::VisitDecltypeType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitType(decltype_type, node) && res;
}

bool ClangToSageTranslator::VisitDependentDecltypeType(clang::DependentDecltypeType * dependent_decltype_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToSageTranslator::VisitDependentDecltypeType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitDecltypeType(dependent_decltype_type, node) && res;
}

bool ClangToSageTranslator::VisitDeducedType(clang::DeducedType * deduced_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToSageTranslator::VisitDeducedType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitType(deduced_type, node) && res;
}

bool ClangToSageTranslator::VisitAutoType(clang::AutoType * auto_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToSageTranslator::VisitAutoType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitDeducedType(auto_type, node) && res;
}

bool ClangToSageTranslator::VisitDeducedTemplateSpecializationType(clang::DeducedTemplateSpecializationType * deduced_template_specialization_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToSageTranslator::VisitDeducedTemplateSpecializationType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitDeducedType(deduced_template_specialization_type, node) && res;
}

bool ClangToSageTranslator::VisitDependentAddressSpaceType(clang::DependentAddressSpaceType * dependent_address_space_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToSageTranslator::VisitDependentAddressSpaceType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitType(dependent_address_space_type, node) && res;
}

bool ClangToSageTranslator::VisitDependentSizedExtVectorType(clang::DependentSizedExtVectorType * dependent_sized_ext_vector_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToSageTranslator::DependentSizedExtVectorType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitType(dependent_sized_ext_vector_type, node) && res;
}

bool ClangToSageTranslator::VisitDependentVectorType(clang::DependentVectorType * dependent_vector_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToSageTranslator::DependentVectorType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitType(dependent_vector_type, node) && res;
}

bool ClangToSageTranslator::VisitFunctionType(clang::FunctionType * function_type, SgNode ** node)  {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToSageTranslator::VisitFunctionType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME

    return VisitType(function_type, node) && res;
}

bool ClangToSageTranslator::VisitFunctionNoProtoType(clang::FunctionNoProtoType * function_no_proto_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToSageTranslator::VisitFunctionNoProtoType" << "\n";
#endif

    bool res = true;

    SgFunctionParameterTypeList * param_type_list = new SgFunctionParameterTypeList();

    SgType * ret_type = buildTypeFromQualifiedType(function_no_proto_type->getReturnType()); 

    *node = SageBuilder::buildFunctionType(ret_type, param_type_list);

    return VisitType(function_no_proto_type, node) && res;
}

bool ClangToSageTranslator::VisitFunctionProtoType(clang::FunctionProtoType * function_proto_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToSageTranslator::VisitFunctionProtoType" << "\n";
#endif

    bool res = true;
    SgFunctionParameterTypeList * param_type_list = new SgFunctionParameterTypeList();
    for (unsigned i = 0; i < function_proto_type->getNumParams(); i++) {
#if DEBUG_VISIT_TYPE
        logger[DEBUG] << "funcProtoType: " << i << " th param" << "\n";
#endif
        SgType * param_type = buildTypeFromQualifiedType(function_proto_type->getParamType(i));

        param_type_list->append_argument(param_type);
    }

    if (function_proto_type->isVariadic()) {
        param_type_list->append_argument(SgTypeEllipse::createType());
    }

    SgType * ret_type = buildTypeFromQualifiedType(function_proto_type->getReturnType());

    SgFunctionType * func_type = SageBuilder::buildFunctionType(ret_type, param_type_list);
    if (function_proto_type->isVariadic()) func_type->set_has_ellipses(1);

    *node = func_type;

    return VisitType(function_proto_type, node) && res;
}

bool ClangToSageTranslator::VisitInjectedClassNameType(clang::InjectedClassNameType * injected_class_name_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToSageTranslator::InjectedClassNameType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitType(injected_class_name_type, node) && res;
}

bool ClangToSageTranslator::VisitLocInfoType(clang::LocInfoType * loc_info_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToSageTranslator::LocInfoType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitType(loc_info_type, node) && res;
}

bool ClangToSageTranslator::VisitMacroQualifiedType(clang::MacroQualifiedType * macro_qualified_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToSageTranslator::MacroQualifiedType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitType(macro_qualified_type, node) && res;
}

bool ClangToSageTranslator::VisitMemberPointerType(clang::MemberPointerType * member_pointer_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToSageTranslator::MemberPointerType" << "\n";
    logger[DEBUG] << "isMemberFunctionPointer  " << member_pointer_type->isMemberFunctionPointer() << "\n";
    logger[DEBUG] << "isMemberDataPointer  " << member_pointer_type->isMemberDataPointer() << "\n";
    logger[DEBUG] << "isSugared  " << member_pointer_type->isSugared() << "\n";
#endif
    bool res = true;

    // (10/30/2026) lin32: Clang 21 replaced getClass() with a qualifier; Clang 22 changed that qualifier to a value type.
#if CLANG_VERSION_MAJOR >= 22
    const clang::Type* memberClassType = member_pointer_type->getQualifier().getAsType();
#elif CLANG_VERSION_MAJOR >= 21
    const clang::Type* memberClassType = member_pointer_type->getQualifier()->getAsType();
#else
    const clang::Type* memberClassType = member_pointer_type->getClass();
#endif
    SgType* classType = isSgClassType(Traverse(memberClassType));
    ROSE_ASSERT(classType);   

    SgType* baseType = buildTypeFromQualifiedType(member_pointer_type->getPointeeType());
    ROSE_ASSERT(baseType);   
    if(member_pointer_type->isMemberFunctionPointer())
    {
      SgFunctionType* functionType = isSgFunctionType(baseType);
      ROSE_ASSERT(functionType);
      SgMemberFunctionType* memFuncType = SageBuilder::buildMemberFunctionType(functionType->get_return_type(), functionType->get_argument_list(), classType, 0);
      baseType = memFuncType;
      ROSE_ASSERT(baseType);   
    }

    SgPointerMemberType* pointerToMemberType = SageBuilder::buildPointerMemberType(baseType, classType);

    *node = pointerToMemberType;
    //ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitType(member_pointer_type, node) && res;
}

bool ClangToSageTranslator::VisitPackExpansionType(clang::PackExpansionType * pack_expansion_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToSageTranslator::PackExpansionType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitType(pack_expansion_type, node) && res;
}

bool ClangToSageTranslator::VisitParenType(clang::ParenType * paren_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToSageTranslator::VisitParenType" << "\n";
    logger[DEBUG] << "isSugared " << paren_type->isSugared() << "\n";
#endif

    if(paren_type->isSugared())
      *node = buildTypeFromQualifiedType(paren_type->desugar());
    else
      *node = buildTypeFromQualifiedType(paren_type->getInnerType());

    return VisitType(paren_type, node);
}

bool ClangToSageTranslator::VisitPipeType(clang::PipeType * pipe_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToSageTranslator::PipeType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitType(pipe_type, node) && res;
}

bool ClangToSageTranslator::VisitPointerType(clang::PointerType * pointer_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToSageTranslator::VisitPointerType" << "\n";
#endif

    SgType * type = buildTypeFromQualifiedType(pointer_type->getPointeeType());

    *node = SageBuilder::buildPointerType(type);

    return VisitType(pointer_type, node);
}

bool ClangToSageTranslator::VisitReferenceType(clang::ReferenceType * reference_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToSageTranslator::ReferenceType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitType(reference_type, node) && res;
}

bool ClangToSageTranslator::VisitLValueReferenceType(clang::LValueReferenceType * lvalue_reference_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToSageTranslator::LValueReferenceType" << "\n";
    logger[DEBUG] << "isSugared " << lvalue_reference_type->isSugared() << "\n";
#endif
    bool res = true;

    SgType * type = buildTypeFromQualifiedType(lvalue_reference_type->getPointeeType());
    *node = SageBuilder::buildReferenceType(type);
    //ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitReferenceType(lvalue_reference_type, node) && res;
}

bool ClangToSageTranslator::VisitRValueReferenceType(clang::RValueReferenceType * rvalue_reference_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToSageTranslator::RValueReferenceType" << "\n";
    logger[DEBUG] << "isSugared " << rvalue_reference_type->isSugared() << "\n";
#endif
    bool res = true;
    SgType * type = buildTypeFromQualifiedType(rvalue_reference_type->getPointeeType());
    *node = SageBuilder::buildRvalueReferenceType(type);

    //ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitReferenceType(rvalue_reference_type, node) && res;
}

bool ClangToSageTranslator::VisitSubstTemplateTypeParmPackType(clang::SubstTemplateTypeParmPackType * subst_template_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToSageTranslator::SubstTemplateTypeParmPackType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitType(subst_template_type, node) && res;
}

bool ClangToSageTranslator::VisitSubstTemplateTypeParmType(clang::SubstTemplateTypeParmType * subst_template_type_parm_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToSageTranslator::SubstTemplateTypeParmType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitType(subst_template_type_parm_type, node) && res;
}

bool ClangToSageTranslator::VisitTagType(clang::TagType * tag_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToSageTranslator::VisitTagType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME

    return VisitType(tag_type, node) && res;
}

bool ClangToSageTranslator::VisitEnumType(clang::EnumType * enum_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToSageTranslator::VisitEnumType" << "\n";
    logger[DEBUG] << "VisitEnumType isSugared: " << enum_type->isSugared() << "\n";
#endif

    SgSymbol * sym = GetSymbolFromSymbolTable(enum_type->getDecl());

    SgEnumSymbol * enum_sym = isSgEnumSymbol(sym);

    if (enum_sym == NULL) {
        SgNode * tmp_decl = Traverse(enum_type->getDecl());
        SgEnumDeclaration * sg_decl = isSgEnumDeclaration(tmp_decl);

        ROSE_ASSERT(sg_decl != NULL);
        *node = sg_decl->get_type();
    }
    else {
        *node = enum_sym->get_type();
    }

    if (isSgEnumType(*node) != NULL) {
        if (enum_sym == NULL) {
            p_enum_type_decl_first_see_in_type.insert(std::pair<SgEnumType *, bool>(isSgEnumType(*node), true));
        }
        else
            p_enum_type_decl_first_see_in_type.insert(std::pair<SgEnumType *, bool>(isSgEnumType(*node), false));
    }

    return VisitType(enum_type, node);
}

bool ClangToSageTranslator::VisitRecordType(clang::RecordType * record_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToSageTranslator::VisitRecordType" << "\n";
#endif

    clang::RecordDecl *record_decl = record_type->getDecl();
    auto translatedClassDecl = [&](clang::RecordDecl *decl) -> SgClassDeclaration* {
        if (decl == nullptr)
            return nullptr;
        auto it = p_decl_translation_map.find(decl);
        return it == p_decl_translation_map.end() ? nullptr : isSgClassDeclaration(it->second);
    };

    // A name-only symbol lookup can select a nearer class with the same name
    // (e.g., a local `A` when this RecordType refers to `::A`). Prefer the
    // exact Clang declaration identity whenever it has already been visited.
    SgClassDeclaration *sg_decl = translatedClassDecl(record_decl);
    if (sg_decl == nullptr)
        sg_decl = translatedClassDecl(llvm::dyn_cast<clang::RecordDecl>(record_decl->getCanonicalDecl()));
    if (sg_decl == nullptr)
        sg_decl = translatedClassDecl(record_decl->getDefinition());

    SgClassSymbol *class_sym = isSgClassSymbol(GetSymbolFromSymbolTable(record_decl));
    // Capture this before Traverse populates sg_decl. Definitions first
    // encountered through a type must still be emitted in sizeof operands.
    const bool firstSeenInType = sg_decl == nullptr && class_sym == nullptr;

    if (sg_decl != nullptr) {
        *node = sg_decl->get_type();
    }
    else if (class_sym == NULL) {
        SgNode * tmp_decl = Traverse(record_type->getDecl());
        sg_decl = isSgClassDeclaration(tmp_decl);

        ROSE_ASSERT(sg_decl != NULL);

        *node = sg_decl->get_type();
    }
    else {
        *node = class_sym->get_type();
    }

    if (isSgClassType(*node) != NULL) {
        if (firstSeenInType) {
            p_class_type_decl_first_see_in_type.insert(std::pair<SgClassType *, bool>(isSgClassType(*node), true));
            isSgNamedType(*node)->set_autonomous_declaration(true);
        }
        else
            p_class_type_decl_first_see_in_type.insert(std::pair<SgClassType *, bool>(isSgClassType(*node), false));
    }

    return VisitType(record_type, node);
}

bool ClangToSageTranslator::VisitTemplateSpecializationType(clang::TemplateSpecializationType * template_specialization_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToSageTranslator::TemplateSpecializationType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitType(template_specialization_type, node) && res;
}

bool ClangToSageTranslator::VisitTemplateTypeParmType(clang::TemplateTypeParmType * template_type_parm_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToSageTranslator::TemplateTypeParmType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitType(template_type_parm_type, node) && res;
}

bool ClangToSageTranslator::VisitTypedefType(clang::TypedefType * typedef_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToSageTranslator::VisitTypedefType" << "\n";
#endif

    bool res = true;

    SgSymbol * sym = GetSymbolFromSymbolTable(typedef_type->getDecl());
    SgTypedefSymbol * tdef_sym = isSgTypedefSymbol(sym);

    if (tdef_sym == NULL) {
        // Pei-Hung (09/23/2022) typedef declaration can appear after it is used. Example: test2012_58.c 
        // build tthe typedef declaration here if it's still missing
        SgNode * child = Traverse(typedef_type->getDecl());
    }

    sym = GetSymbolFromSymbolTable(typedef_type->getDecl());
    tdef_sym = isSgTypedefSymbol(sym);
    if (tdef_sym == NULL) {
        logger[WARN] << "Runtime Error: Cannot find a typedef symbol for the TypedefType." << "\n";
        res = false;
    }

    *node = tdef_sym->get_type();

   return VisitType(typedef_type, node) && res;
}

bool ClangToSageTranslator::VisitTypeOfExprType(clang::TypeOfExprType * type_of_expr_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToSageTranslator::TypeOfExprType" << "\n";
#endif
    bool res = true;

    SgNode* tmp_expr = Traverse(type_of_expr_type->getUnderlyingExpr());

 // logger[DEBUG] << "In VisitTypeOfExprType(): tmp_expr = " << tmp_expr << " = " << tmp_expr->class_name().c_str() << "\n";

    SgExpression* expr = isSgExpression(tmp_expr);
    SgType* type = SageBuilder::buildTypeOfType(expr,NULL);

    *node = type;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitType(type_of_expr_type, node) && res;
}

bool ClangToSageTranslator::VisitDependentTypeOfExprType(clang::DependentTypeOfExprType * dependent_type_of_expr_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToSageTranslator::DependentTypeOfExprType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitTypeOfExprType(dependent_type_of_expr_type, node) && res;
}

bool ClangToSageTranslator::VisitTypeOfType(clang::TypeOfType * type_of_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToSageTranslator::TypeOfType" << "\n";
#endif
    bool res = true;

#if CLANG_VERSION_MAJOR > 15
    SgType* underlyinigType = buildTypeFromQualifiedType(type_of_type->getUnmodifiedType());
#else
    SgType* underlyinigType = buildTypeFromQualifiedType(type_of_type->getUnderlyingType());
#endif

    SgType* type = SageBuilder::buildTypeOfType(NULL,underlyinigType);

    *node = type;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitType(type_of_type, node) && res;
}

bool ClangToSageTranslator::VisitTypeWithKeyword(clang::TypeWithKeyword * type_with_keyword, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToSageTranslator::VisitTypeWithKeyword" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME

    return VisitType(type_with_keyword, node) && res;
}

bool ClangToSageTranslator::VisitDependentNameType(clang::DependentNameType * dependent_name_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToSageTranslator::DependentNameType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitTypeWithKeyword(dependent_name_type, node) && res;
}

// (10/30/2026) lin32: Retain the old visitors only where the pre-Clang-22 AST classes still exist.
#if CLANG_VERSION_MAJOR < 22
bool ClangToSageTranslator::VisitDependentTemplateSpecializationType(clang::DependentTemplateSpecializationType * ependent_template_specialization_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToSageTranslator::DependentTemplateSpecializationType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitTypeWithKeyword(ependent_template_specialization_type, node) && res;
}

bool ClangToSageTranslator::VisitElaboratedType(clang::ElaboratedType * elaborated_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToSageTranslator::VisitElaboratedType" << "\n";
#endif

    SgType * type = buildTypeFromQualifiedType(elaborated_type->getNamedType());

    clang::TagDecl* ownedTagDecl = elaborated_type->getOwnedTagDecl();
#if DEBUG_VISIT_TYPE
    if(ownedTagDecl != nullptr)
    {
       logger[DEBUG] << "ClangToSageTranslator::VisitElaboratedType has ownedTagDecl and isThisDeclarationADefinition =" << ownedTagDecl->isThisDeclarationADefinition() << "\n";
    }
#endif
    // FIXME clang::ElaboratedType contains the "sugar" of a type reference (eg, "struct A" or "M::N::A"), it should be pass down to ROSE

    *node = type;

    return VisitTypeWithKeyword(elaborated_type, node);
}

#endif

bool ClangToSageTranslator::VisitUnaryTransformType(clang::UnaryTransformType * unary_transform_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToSageTranslator::UnaryTransformType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitType(unary_transform_type, node) && res;
}

// (10/30/2026) lin32: Clang 21 represents dependent unary transforms with UnaryTransformType instead.
#if CLANG_VERSION_MAJOR < 21
bool ClangToSageTranslator::VisitDependentUnaryTransformType(clang::DependentUnaryTransformType * dependent_unary_transform_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToSageTranslator::DependentUnaryTransformType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitUnaryTransformType(dependent_unary_transform_type, node) && res;
}

#endif

bool ClangToSageTranslator::VisitUnresolvedUsingType(clang::UnresolvedUsingType * unresolved_using_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToSageTranslator::UnresolvedUsingType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitType(unresolved_using_type, node) && res;
}

bool ClangToSageTranslator::VisitVectorType(clang::VectorType * vector_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToSageTranslator::VisitVectorType" << "\n";
#endif

    SgType * type = buildTypeFromQualifiedType(vector_type->getElementType());

    SgModifierType * modified_type = new SgModifierType(type);
    SgTypeModifier & sg_modifer = modified_type->get_typeModifier();

    sg_modifer.setVectorType();
    sg_modifer.set_vector_size(vector_type->getNumElements());

    *node = SgModifierType::insertModifierTypeIntoTypeTable(modified_type);

    return VisitType(vector_type, node);
}

bool ClangToSageTranslator::VisitExtVectorType(clang::ExtVectorType * ext_vector_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToSageTranslator::VisitExtVectorType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME Is it anything to be done here?

    return VisitVectorType(ext_vector_type, node) && res;
}
