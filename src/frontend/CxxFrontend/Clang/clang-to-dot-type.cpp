#include "sage3basic.h"
#include "clang-to-dot-private.hpp"

using namespace Sawyer::Message;

std::string ClangToDotTranslator::Traverse(const clang::Type * type) 
   {
     if (type == NULL)
          return "";

 // Look for previous translation
    std::map<const clang::Type *, std::string>::iterator it = p_type_translation_map.find(type);
    if (it != p_type_translation_map.end()) 
         return it->second;

  // SgNode * result = NULL;
  // bool ret_status = false;

    // If first time, create a new entry
    std::string node_ident = genNextIdent();
    p_type_translation_map.insert(std::pair<const clang::Type *, std::string>(type, node_ident));
    NodeDescriptor & node_desc = p_node_desc.insert(std::pair<std::string, NodeDescriptor>(node_ident, NodeDescriptor(node_ident))).first->second;

    bool ret_status = false;

  // CLANG_ROSE_Graph::graph (type);

     switch (type->getTypeClass())
        {
        case clang::Type::Decayed:
            ret_status = VisitDecayedType((clang::DecayedType *)type, node_desc);
            break;
        case clang::Type::ConstantArray:
            ret_status = VisitConstantArrayType((clang::ConstantArrayType *)type, node_desc);
            break;
        case clang::Type::DependentSizedArray:
            ret_status = VisitDependentSizedArrayType((clang::DependentSizedArrayType *)type, node_desc);
            break;
       case clang::Type::IncompleteArray:
            ret_status = VisitIncompleteArrayType((clang::IncompleteArrayType *)type, node_desc);
            break;
        case clang::Type::VariableArray:
            ret_status = VisitVariableArrayType((clang::VariableArrayType *)type, node_desc);
            break;
        case clang::Type::Atomic:
            ret_status = VisitAtomicType((clang::AtomicType *)type, node_desc);
            break;
        case clang::Type::Attributed:
            ret_status = VisitAttributedType((clang::AttributedType *)type, node_desc);
            break;
        case clang::Type::BlockPointer:
            ret_status = VisitBlockPointerType((clang::BlockPointerType *)type, node_desc);
            break;
        case clang::Type::Builtin:
            ret_status = VisitBuiltinType((clang::BuiltinType *)type, node_desc);
            break;
        case clang::Type::Complex:
            ret_status = VisitComplexType((clang::ComplexType *)type, node_desc);
            break;
        case clang::Type::Decltype:
            ret_status = VisitDecltypeType((clang::DecltypeType *)type, node_desc);
            break;
     // case clang::Type::DependentDecltype:
     //     ret_status = VisitDependentDecltypeType((clang::DependentDecltypeType *)type, node_desc);
     //     break;
        case clang::Type::Auto:
            ret_status = VisitAutoType((clang::AutoType *)type, node_desc);
            break;
        case clang::Type::DeducedTemplateSpecialization:
            ret_status = VisitDeducedTemplateSpecializationType((clang::DeducedTemplateSpecializationType *)type, node_desc);
            break;
        case clang::Type::DependentSizedExtVector:
            ret_status = VisitDependentSizedExtVectorType((clang::DependentSizedExtVectorType *)type, node_desc);
            break;
        case clang::Type::DependentVector:
            ret_status = VisitDependentVectorType((clang::DependentVectorType *)type, node_desc);
            break;
        case clang::Type::FunctionNoProto:
            ret_status = VisitFunctionNoProtoType((clang::FunctionNoProtoType *)type, node_desc);
            break;
        case clang::Type::FunctionProto:
            ret_status = VisitFunctionProtoType((clang::FunctionProtoType *)type, node_desc);
            break;
        case clang::Type::InjectedClassName:
            ret_status = VisitInjectedClassNameType((clang::InjectedClassNameType *)type, node_desc);
            break;
     // case clang::Type::LocInfo:
     //     ret_status = VisitLocInfoType((clang::LocInfoType *)type, node_desc);
     //     break;
        case clang::Type::MacroQualified:
            ret_status = VisitMacroQualifiedType((clang::MacroQualifiedType *)type, node_desc);
            break;
        case clang::Type::MemberPointer:
            ret_status = VisitMemberPointerType((clang::MemberPointerType *)type, node_desc);
            break;
        case clang::Type::PackExpansion:
            ret_status = VisitPackExpansionType((clang::PackExpansionType *)type, node_desc);
            break;
        case clang::Type::Paren:
            ret_status = VisitParenType((clang::ParenType *)type, node_desc);
            break;
        case clang::Type::Pipe:
            ret_status = VisitPipeType((clang::PipeType *)type, node_desc);
            break;
        case clang::Type::Pointer:
            ret_status = VisitPointerType((clang::PointerType *)type, node_desc);
            break;
        case clang::Type::LValueReference:
            ret_status = VisitLValueReferenceType((clang::LValueReferenceType *)type, node_desc);
            break;
        case clang::Type::RValueReference:
            ret_status = VisitRValueReferenceType((clang::RValueReferenceType *)type, node_desc);
            break;
        case clang::Type::SubstTemplateTypeParmPack:
            ret_status = VisitSubstTemplateTypeParmPackType((clang::SubstTemplateTypeParmPackType *)type, node_desc);
            break;
        case clang::Type::SubstTemplateTypeParm:
            ret_status = VisitSubstTemplateTypeParmType((clang::SubstTemplateTypeParmType *)type, node_desc);
            break;
        case clang::Type::Enum:
            ret_status = VisitEnumType((clang::EnumType *)type, node_desc);
            break;
        case clang::Type::Record:
            ret_status = VisitRecordType((clang::RecordType *)type, node_desc);
            break;
        case clang::Type::TemplateSpecialization:
            ret_status = VisitTemplateSpecializationType((clang::TemplateSpecializationType *)type, node_desc);
            break;
        case clang::Type::TemplateTypeParm:
            ret_status = VisitTemplateTypeParmType((clang::TemplateTypeParmType *)type, node_desc);
            break;
        case clang::Type::Typedef:
            ret_status = VisitTypedefType((clang::TypedefType *)type, node_desc);
            break;
      case clang::Type::TypeOfExpr:
          ret_status = VisitTypeOfExprType((clang::TypeOfExprType *)type, node_desc);
          break;
    //  case clang::Type::DependentTypeOfExpr:
    //      ret_status = VisitDependentTypeOfExprType((clang::DependentTypeOfExprType *)type, node_desc);
    //      break;
        case clang::Type::TypeOf:
            ret_status = VisitTypeOfType((clang::TypeOfType *)type, node_desc);
            break;
        case clang::Type::DependentName:
            ret_status = VisitDependentNameType((clang::DependentNameType *)type, node_desc);
            break;
        // (10/30/2026) lin32: Clang 22 removed ElaboratedType and merged dependent specializations into TemplateSpecializationType.
#if CLANG_VERSION_MAJOR < 22
        case clang::Type::DependentTemplateSpecialization:
            ret_status = VisitDependentTemplateSpecializationType((clang::DependentTemplateSpecializationType *)type, node_desc);
            break;
        case clang::Type::Elaborated:
            ret_status = VisitElaboratedType((clang::ElaboratedType *)type, node_desc);
            break;
#endif
        case clang::Type::UnaryTransform:
            ret_status = VisitUnaryTransformType((clang::UnaryTransformType *)type, node_desc);
            break;
        case clang::Type::UnresolvedUsing:
            ret_status = VisitUnresolvedUsingType((clang::UnresolvedUsingType *)type, node_desc);
            break;
        case clang::Type::Vector:
            ret_status = VisitVectorType((clang::VectorType *)type, node_desc);
            break;
        case clang::Type::ExtVector:
            ret_status = VisitExtVectorType((clang::ExtVectorType *)type, node_desc);
            break;
        default:
            logger[ERROR] << "Unhandled type" << "\n";
            ROSE_ABORT();
    }

 // ROSE_ASSERT(result != NULL);
 // p_type_translation_map.insert(std::pair<const clang::Type *, SgNode *>(type, result));
 // return result;

    assert(ret_status != false);

    return node_ident;
}

/***************/
/* Visit Types */
/***************/

#if 0
bool ClangToDotTranslator::VisitType(clang::Type * type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::VisitType" << "\n";
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
#else
bool ClangToDotTranslator::VisitType(clang::Type * type, NodeDescriptor & node_desc) 
   {
#if DEBUG_VISIT_TYPE
     logger[DEBUG] << "ClangToDotTranslator::VisitType" << "\n";
#endif

#if 0
    if (*node == NULL) {
        logger[WARN] << "Runtime error: No Sage node associated with the type..." << "\n";
        return false;
    }
/*
    logger[DEBUG] << "Dump type " << type->getTypeClassName() << "(" << type << "): ";
    type->dump();
    logger[DEBUG] << "\n";
*/
#endif

     node_desc.kind_hierarchy.push_back("Type");
#if CLANG_VERSION_MAJOR >= 18
     switch (type->getLinkage())
        {
          case clang::Linkage::Invalid:
          case clang::Linkage::None:
               break;
          case clang::Linkage::Internal:
               node_desc.attributes.push_back(std::pair<std::string, std::string>("linkage", "internal"));
               break;
          case clang::Linkage::UniqueExternal:
               node_desc.attributes.push_back(std::pair<std::string, std::string>("linkage", "unique external"));
               break;
          case clang::Linkage::VisibleNone:
               node_desc.attributes.push_back(std::pair<std::string, std::string>("linkage", "VisibleNone"));
               break;
          case clang::Linkage::External:
               node_desc.attributes.push_back(std::pair<std::string, std::string>("linkage", "external"));
               break;
          case clang::Linkage::Module:
               node_desc.attributes.push_back(std::pair<std::string, std::string>("linkage", "module"));
               break;
        }
#else
     switch (type->getLinkage())
        {
          case clang::NoLinkage:
               break;
          case clang::InternalLinkage:
               node_desc.attributes.push_back(std::pair<std::string, std::string>("linkage", "internal"));
               break;
          case clang::UniqueExternalLinkage:
               node_desc.attributes.push_back(std::pair<std::string, std::string>("linkage", "unique external"));
               break;
          case clang::ExternalLinkage:
               node_desc.attributes.push_back(std::pair<std::string, std::string>("linkage", "external"));
               break;
        }
#endif

     switch (type->getVisibility()) 
        {
          case clang::HiddenVisibility:
               node_desc.attributes.push_back(std::pair<std::string, std::string>("visibility", "hidden"));
               break;
          case clang::ProtectedVisibility:
               node_desc.attributes.push_back(std::pair<std::string, std::string>("visibility", "protected"));
               break;
          case clang::DefaultVisibility:
               node_desc.attributes.push_back(std::pair<std::string, std::string>("visibility", "default"));
               break;
        }

  // DQ (11/27/2020): Comment from original code that generated the ROSE AST.
  // TODO

     return true;
   }
#endif

#if 0
bool ClangToDotTranslator::VisitAdjustedType(clang::AdjustedType * adjusted_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::VisitAdjustedType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitType(adjusted_type, node) && res;
}
#else
bool ClangToDotTranslator::VisitAdjustedType(clang::AdjustedType * adjusted_type, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::VisitAdjustedType" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("AdjustedType");

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitType(adjusted_type, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitDecayedType(clang::DecayedType * decayed_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::VisitDecayedType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitAdjustedType(decayed_type, node) && res;
}
#else
bool ClangToDotTranslator::VisitDecayedType(clang::DecayedType * decayed_type, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::VisitDecayedType" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("DecayedType");

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitAdjustedType(decayed_type, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitArrayType(clang::ArrayType * array_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::VisitArrayType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME

    return VisitType(array_type, node) && res;
}
#else
bool ClangToDotTranslator::VisitArrayType(clang::ArrayType * array_type, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::VisitArrayType" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("ArrayType");

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME

    return VisitType(array_type, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitConstantArrayType(clang::ConstantArrayType * constant_array_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::VisitConstantArrayType" << "\n";
#endif

    SgType * type = buildTypeFromQualifiedType(constant_array_type->getElementType());

    // TODO clang::ArrayType::ArraySizeModifier

    SgExpression * expr = SageBuilder::buildIntVal(constant_array_type->getSize().getSExtValue());

    *node = SageBuilder::buildArrayType(type, expr);

    return VisitArrayType(constant_array_type, node);
}
#else
bool ClangToDotTranslator::VisitConstantArrayType(clang::ConstantArrayType * constant_array_type, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::VisitConstantArrayType" << "\n";
#endif

#if 0
    SgType * type = buildTypeFromQualifiedType(constant_array_type->getElementType());

    // TODO clang::ArrayType::ArraySizeModifier

    SgExpression * expr = SageBuilder::buildIntVal(constant_array_type->getSize().getSExtValue());

    *node = SageBuilder::buildArrayType(type, expr);
#endif

     node_desc.kind_hierarchy.push_back("ConstantArrayType");

    return VisitArrayType(constant_array_type, node_desc);
}
#endif

#if 0
bool ClangToDotTranslator::VisitDependentSizedArrayType(clang::DependentSizedArrayType * dependent_sized_array_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::VisitDependentSizedArrayType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitArrayType(dependent_sized_array_type, node) && res;
}
#else
bool ClangToDotTranslator::VisitDependentSizedArrayType(clang::DependentSizedArrayType * dependent_sized_array_type, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::VisitDependentSizedArrayType" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("DependentSizedArrayType");

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitArrayType(dependent_sized_array_type, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitIncompleteArrayType(clang::IncompleteArrayType * incomplete_array_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::VisitIncompleteArrayType" << "\n";
#endif

    SgType * type = buildTypeFromQualifiedType(incomplete_array_type->getElementType());

    // TODO clang::ArrayType::ArraySizeModifier

    *node = SageBuilder::buildArrayType(type);

    return VisitArrayType(incomplete_array_type, node);
}
#else
bool ClangToDotTranslator::VisitIncompleteArrayType(clang::IncompleteArrayType * incomplete_array_type, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::VisitIncompleteArrayType" << "\n";
#endif

#if 0
    SgType * type = buildTypeFromQualifiedType(incomplete_array_type->getElementType());

    // TODO clang::ArrayType::ArraySizeModifier

    *node = SageBuilder::buildArrayType(type);
#endif

     node_desc.kind_hierarchy.push_back("IncompleteArrayType");

    return VisitArrayType(incomplete_array_type, node_desc);
}
#endif

#if 0
bool ClangToDotTranslator::VisitVariableArrayType(clang::VariableArrayType * variable_array_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::VisitVariableArrayType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitArrayType(variable_array_type, node) && res;
}
#else
bool ClangToDotTranslator::VisitVariableArrayType(clang::VariableArrayType * variable_array_type, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::VisitVariableArrayType" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("VariableArrayType");

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitArrayType(variable_array_type, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitAtomicType(clang::AtomicType * atomic_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::VisitAtomicType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitType(atomic_type, node) && res;
}
#else
bool ClangToDotTranslator::VisitAtomicType(clang::AtomicType * atomic_type, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::VisitAtomicType" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("AtomicArrayType");

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitType(atomic_type, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitAttributedType(clang::AttributedType * attributed_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::VisitAttributedType" << "\n";
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
            logger[WARN] << "Unsupported attribute attr_address_space" << "\n"; ROSE_ASSERT(false);
        case clang::AttributedType::attr_regparm:
            logger[WARN] << "Unsupported attribute attr_regparm" << "\n"; ROSE_ASSERT(false);
        case clang::AttributedType::attr_vector_size:
            logger[WARN] << "Unsupported attribute attr_vector_size" << "\n"; ROSE_ASSERT(false);
        case clang::AttributedType::attr_neon_vector_type:
            logger[WARN] << "Unsupported attribute attr_neon_vector_type" << "\n"; ROSE_ASSERT(false);
        case clang::AttributedType::attr_neon_polyvector_type:
            logger[WARN] << "Unsupported attribute attr_neon_polyvector_type" << "\n"; ROSE_ASSERT(false);
        case clang::AttributedType::attr_objc_gc:
            logger[WARN] << "Unsupported attribute attr_objc_gc" << "\n"; ROSE_ASSERT(false);
        case clang::AttributedType::attr_objc_ownership:
            logger[WARN] << "Unsupported attribute attr_objc_ownership" << "\n"; ROSE_ASSERT(false);
        case clang::AttributedType::attr_pcs:
            logger[WARN] << "Unsupported attribute attr_pcs" << "\n"; ROSE_ASSERT(false);
        case clang::AttributedType::attr_fastcall:
            logger[WARN] << "Unsupported attribute attr_fastcall" << "\n"; ROSE_ASSERT(false);
        case clang::AttributedType::attr_thiscall:
            logger[WARN] << "Unsupported attribute attr_thiscall" << "\n"; ROSE_ASSERT(false);
        case clang::AttributedType::attr_pascal:
            logger[WARN] << "Unsupported attribute attr_pascal" << "\n"; ROSE_ASSERT(false);
        default:
            logger[WARN] << "Unknown attribute" << "\n"; ROSE_ASSERT(false);
    } 
*/
    *node = SgModifierType::insertModifierTypeIntoTypeTable(modified_type);;

    return VisitType(attributed_type, node);
}
#else
bool ClangToDotTranslator::VisitAttributedType(clang::AttributedType * attributed_type, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::VisitAttributedType" << "\n";
#endif

#if 0
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
            logger[WARN] << "Unsupported attribute attr_address_space" << "\n"; ROSE_ASSERT(false);
        case clang::AttributedType::attr_regparm:
            logger[WARN] << "Unsupported attribute attr_regparm" << "\n"; ROSE_ASSERT(false);
        case clang::AttributedType::attr_vector_size:
            logger[WARN] << "Unsupported attribute attr_vector_size" << "\n"; ROSE_ASSERT(false);
        case clang::AttributedType::attr_neon_vector_type:
            logger[WARN] << "Unsupported attribute attr_neon_vector_type" << "\n"; ROSE_ASSERT(false);
        case clang::AttributedType::attr_neon_polyvector_type:
            logger[WARN] << "Unsupported attribute attr_neon_polyvector_type" << "\n"; ROSE_ASSERT(false);
        case clang::AttributedType::attr_objc_gc:
            logger[WARN] << "Unsupported attribute attr_objc_gc" << "\n"; ROSE_ASSERT(false);
        case clang::AttributedType::attr_objc_ownership:
            logger[WARN] << "Unsupported attribute attr_objc_ownership" << "\n"; ROSE_ASSERT(false);
        case clang::AttributedType::attr_pcs:
            logger[WARN] << "Unsupported attribute attr_pcs" << "\n"; ROSE_ASSERT(false);
        case clang::AttributedType::attr_fastcall:
            logger[WARN] << "Unsupported attribute attr_fastcall" << "\n"; ROSE_ASSERT(false);
        case clang::AttributedType::attr_thiscall:
            logger[WARN] << "Unsupported attribute attr_thiscall" << "\n"; ROSE_ASSERT(false);
        case clang::AttributedType::attr_pascal:
            logger[WARN] << "Unsupported attribute attr_pascal" << "\n"; ROSE_ASSERT(false);
        default:
            logger[WARN] << "Unknown attribute" << "\n"; ROSE_ASSERT(false);
    } 
*/
    *node = SgModifierType::insertModifierTypeIntoTypeTable(modified_type);;
#endif

     node_desc.kind_hierarchy.push_back("AttributedType");

    return VisitType(attributed_type, node_desc);
}
#endif

#if 0
bool ClangToDotTranslator::VisitBlockPointerType(clang::BlockPointerType * block_pointer_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::VisitBlockPointerType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitType(block_pointer_type, node) && res;
}
#else
bool ClangToDotTranslator::VisitBlockPointerType(clang::BlockPointerType * block_pointer_type, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::VisitBlockPointerType" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("BlockPointerType");

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitType(block_pointer_type, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitBuiltinType(clang::BuiltinType * builtin_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::VisitBuiltinType" << "\n";
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
 
        case clang::BuiltinType::Char_U:    logger[INFO] << "Char_U    -> "; break;
        case clang::BuiltinType::WChar_U:   logger[INFO] << "WChar_U   -> "; break;
        case clang::BuiltinType::Char16:    logger[INFO] << "Char16    -> "; break;
        case clang::BuiltinType::Char32:    logger[INFO] << "Char32    -> "; break;
        case clang::BuiltinType::WChar_S:   logger[INFO] << "WChar_S   -> "; break;


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
#else
bool ClangToDotTranslator::VisitBuiltinType(clang::BuiltinType * builtin_type, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::VisitBuiltinType" << "\n";
#endif

#if 0
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
 
        case clang::BuiltinType::Char_U:    logger[INFO] << "Char_U    -> "; break;
        case clang::BuiltinType::WChar_U:   logger[INFO] << "WChar_U   -> "; break;
        case clang::BuiltinType::Char16:    logger[INFO] << "Char16    -> "; break;
        case clang::BuiltinType::Char32:    logger[INFO] << "Char32    -> "; break;
        case clang::BuiltinType::WChar_S:   logger[INFO] << "WChar_S   -> "; break;


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
#endif

     node_desc.kind_hierarchy.push_back("BuiltinType");

     switch (builtin_type->getKind()) 
        {
          case clang::BuiltinType::Void:
               node_desc.attributes.push_back(std::pair<std::string, std::string>("type", "void"));
               break; 
          case clang::BuiltinType::Bool:
               node_desc.attributes.push_back(std::pair<std::string, std::string>("type", "bool"));
               break; 
        case clang::BuiltinType::Short:
            node_desc.attributes.push_back(std::pair<std::string, std::string>("type", "short"));
            break; 
        case clang::BuiltinType::Int:
            node_desc.attributes.push_back(std::pair<std::string, std::string>("type", "int"));
            break; 
        case clang::BuiltinType::Long:
            node_desc.attributes.push_back(std::pair<std::string, std::string>("type", "long"));
            break; 
        case clang::BuiltinType::LongLong:
            node_desc.attributes.push_back(std::pair<std::string, std::string>("type", "long long"));
            break; 
        case clang::BuiltinType::Float:
            node_desc.attributes.push_back(std::pair<std::string, std::string>("type", "float"));
            break; 
        case clang::BuiltinType::Double:
            node_desc.attributes.push_back(std::pair<std::string, std::string>("type", "double"));
            break; 
        case clang::BuiltinType::LongDouble:
            node_desc.attributes.push_back(std::pair<std::string, std::string>("type", "long double"));
            break; 
        case clang::BuiltinType::Char_S:
            node_desc.attributes.push_back(std::pair<std::string, std::string>("type", "char_s"));
            break; 
        case clang::BuiltinType::UInt:
            node_desc.attributes.push_back(std::pair<std::string, std::string>("type", "unsigned int"));
            break; 
        case clang::BuiltinType::UChar:
            node_desc.attributes.push_back(std::pair<std::string, std::string>("type", "unsigned char"));
            break; 
        case clang::BuiltinType::SChar:
            node_desc.attributes.push_back(std::pair<std::string, std::string>("type", "signed char"));
            break; 
        case clang::BuiltinType::UShort:
            node_desc.attributes.push_back(std::pair<std::string, std::string>("type", "unsigned short"));
            break; 
        case clang::BuiltinType::ULong:
            node_desc.attributes.push_back(std::pair<std::string, std::string>("type", "unsigned long"));
            break; 
        case clang::BuiltinType::ULongLong:
            node_desc.attributes.push_back(std::pair<std::string, std::string>("type", "unsigned long long"));
            break; 
        case clang::BuiltinType::NullPtr:
            node_desc.attributes.push_back(std::pair<std::string, std::string>("type", "null pointer"));
            break; 
        case clang::BuiltinType::UInt128:
            node_desc.attributes.push_back(std::pair<std::string, std::string>("type", "uint_128"));
            break; 
        case clang::BuiltinType::Int128:
            node_desc.attributes.push_back(std::pair<std::string, std::string>("type", "int_128"));
            break; 
        case clang::BuiltinType::Char_U:
            node_desc.attributes.push_back(std::pair<std::string, std::string>("type", "char_u"));
            break; 
        case clang::BuiltinType::WChar_U:
            node_desc.attributes.push_back(std::pair<std::string, std::string>("type", "wchar_u"));
            break; 
        case clang::BuiltinType::Char16:
            node_desc.attributes.push_back(std::pair<std::string, std::string>("type", "char_16"));
            break; 
        case clang::BuiltinType::Char32:
            node_desc.attributes.push_back(std::pair<std::string, std::string>("type", "char_32"));
            break; 
        case clang::BuiltinType::WChar_S:
            node_desc.attributes.push_back(std::pair<std::string, std::string>("type", "wchar_s"));
            break; 
        case clang::BuiltinType::ObjCId:
            node_desc.attributes.push_back(std::pair<std::string, std::string>("type", "ObjCId"));
            break; 
        case clang::BuiltinType::ObjCClass:
            node_desc.attributes.push_back(std::pair<std::string, std::string>("type", "ObjCClass"));
            break; 
        case clang::BuiltinType::ObjCSel:
            node_desc.attributes.push_back(std::pair<std::string, std::string>("type", "ObjCSel"));
            break; 
        case clang::BuiltinType::Dependent:
            node_desc.attributes.push_back(std::pair<std::string, std::string>("type", "Dependent"));
            break; 
        case clang::BuiltinType::Overload:
            node_desc.attributes.push_back(std::pair<std::string, std::string>("type", "Overload"));
            break; 
        case clang::BuiltinType::BoundMember:
            node_desc.attributes.push_back(std::pair<std::string, std::string>("type", "BoundMember"));
            break; 
        case clang::BuiltinType::UnknownAny:
            node_desc.attributes.push_back(std::pair<std::string, std::string>("type", "UnknownAny"));
            break; 
        // (10/30/2026) lin32: New Clang releases add many builtin kinds; preserve their names without an exhaustive switch.
        default:
            node_desc.attributes.push_back(std::pair<std::string, std::string>(
                "type", builtin_type->getName(p_compiler_instance->getASTContext().getPrintingPolicy()).str()));
            break;
    }


    return VisitType(builtin_type, node_desc);
}
#endif

#if 0
bool ClangToDotTranslator::VisitComplexType(clang::ComplexType * complex_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::VisitComplexType" << "\n";
#endif

    bool res = true;

    SgType * type = buildTypeFromQualifiedType(complex_type->getElementType());

    *node = SageBuilder::buildComplexType(type);

    return VisitType(complex_type, node) && res;
}
#else
bool ClangToDotTranslator::VisitComplexType(clang::ComplexType * complex_type, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::VisitComplexType" << "\n";
#endif

    bool res = true;

#if 0
    SgType * type = buildTypeFromQualifiedType(complex_type->getElementType());

    *node = SageBuilder::buildComplexType(type);
#endif

     node_desc.kind_hierarchy.push_back("ComplexType");

     node_desc.successors.push_back(std::pair<std::string, std::string>("element_type", Traverse(complex_type->getElementType().getTypePtr())));

    return VisitType(complex_type, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitDecltypeType(clang::DecltypeType * decltype_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::VisitDecltypeType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitType(decltype_type, node) && res;
}
#else
bool ClangToDotTranslator::VisitDecltypeType(clang::DecltypeType * decltype_type, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::VisitDecltypeType" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("DecltypeType");

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitType(decltype_type, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitDependentDecltypeType(clang::DependentDecltypeType * dependent_decltype_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::VisitDependentDecltypeType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitDecltypeType(dependent_decltype_type, node) && res;
}
#else
bool ClangToDotTranslator::VisitDependentDecltypeType(clang::DependentDecltypeType * dependent_decltype_type, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::VisitDependentDecltypeType" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("DependentDecltypeType");

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitDecltypeType(dependent_decltype_type, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitDeducedType(clang::DeducedType * deduced_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::VisitDeducedType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitType(deduced_type, node) && res;
}
#else
bool ClangToDotTranslator::VisitDeducedType(clang::DeducedType * deduced_type, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::VisitDeducedType" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("DeducedType");

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitType(deduced_type, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitAutoType(clang::AutoType * auto_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::VisitAutoType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitDeducedType(auto_type, node) && res;
}
#else
bool ClangToDotTranslator::VisitAutoType(clang::AutoType * auto_type, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::VisitAutoType" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("AutoType");

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitDeducedType(auto_type, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitDeducedTemplateSpecializationType(clang::DeducedTemplateSpecializationType * deduced_template_specialization_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::VisitDeducedTemplateSpecializationType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitDeducedType(deduced_template_specialization_type, node) && res;
}
#else
bool ClangToDotTranslator::VisitDeducedTemplateSpecializationType(clang::DeducedTemplateSpecializationType * deduced_template_specialization_type, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::VisitDeducedTemplateSpecializationType" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("DeducedTemplateSpecializationType");

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitDeducedType(deduced_template_specialization_type, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitDependentAddressSpaceType(clang::DependentAddressSpaceType * dependent_address_space_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::VisitDependentAddressSpaceType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitType(dependent_address_space_type, node) && res;
}
#else
bool ClangToDotTranslator::VisitDependentAddressSpaceType(clang::DependentAddressSpaceType * dependent_address_space_type, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::VisitDependentAddressSpaceType" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("DependentAddressSpaceType");

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitType(dependent_address_space_type, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitDependentSizedExtVectorType(clang::DependentSizedExtVectorType * dependent_sized_ext_vector_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::DependentSizedExtVectorType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitType(dependent_sized_ext_vector_type, node) && res;
}
#else
bool ClangToDotTranslator::VisitDependentSizedExtVectorType(clang::DependentSizedExtVectorType * dependent_sized_ext_vector_type, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::DependentSizedExtVectorType" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("DependentSizedExtVectorType");

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitType(dependent_sized_ext_vector_type, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitDependentVectorType(clang::DependentVectorType * dependent_vector_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::DependentVectorType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitType(dependent_vector_type, node) && res;
}
#else
bool ClangToDotTranslator::VisitDependentVectorType(clang::DependentVectorType * dependent_vector_type, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::DependentVectorType" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("DependentVectorType");

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitType(dependent_vector_type, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitFunctionType(clang::FunctionType * function_type, SgNode ** node)  {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::VisitFunctionType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME

    return VisitType(function_type, node) && res;
}
#else
bool ClangToDotTranslator::VisitFunctionType(clang::FunctionType * function_type, NodeDescriptor & node_desc)
   {
#if DEBUG_VISIT_TYPE
     logger[DEBUG] << "ClangToDotTranslator::VisitFunctionType" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("FunctionType");

  // node_desc.successors.push_back(std::pair<std::string, std::string>("result_type", Traverse(function_type->getResultType().getTypePtr())));
     node_desc.successors.push_back(std::pair<std::string, std::string>("result_type", Traverse(function_type->getReturnType().getTypePtr())));

  // TODO some attr

     ROSE_ASSERT(FAIL_FIXME == 0); // FIXME

     return VisitType(function_type, node_desc) && res;
   }
#endif

#if 0
bool ClangToDotTranslator::VisitFunctionNoProtoType(clang::FunctionNoProtoType * function_no_proto_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::VisitFunctionNoProtoType" << "\n";
#endif

    bool res = true;

    SgFunctionParameterTypeList * param_type_list = new SgFunctionParameterTypeList();

    SgType * ret_type = buildTypeFromQualifiedType(function_no_proto_type->getReturnType()); 

    *node = SageBuilder::buildFunctionType(ret_type, param_type_list);

    return VisitType(function_no_proto_type, node) && res;
}
#else
bool ClangToDotTranslator::VisitFunctionNoProtoType(clang::FunctionNoProtoType * function_no_proto_type, NodeDescriptor & node_desc) 
   {
#if DEBUG_VISIT_TYPE
     logger[DEBUG] << "ClangToDotTranslator::VisitFunctionNoProtoType" << "\n";
#endif

     bool res = true;

#if 0
     SgFunctionParameterTypeList * param_type_list = new SgFunctionParameterTypeList();

     SgType * ret_type = buildTypeFromQualifiedType(function_no_proto_type->getReturnType()); 

     *node = SageBuilder::buildFunctionType(ret_type, param_type_list);
#endif

     node_desc.kind_hierarchy.push_back("FunctionNoProtoType");

     return VisitType(function_no_proto_type, node_desc) && res;
   }
#endif

#if 0
bool ClangToDotTranslator::VisitFunctionProtoType(clang::FunctionProtoType * function_proto_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::VisitFunctionProtoType" << "\n";
#endif

    bool res = true;
    SgFunctionParameterTypeList * param_type_list = new SgFunctionParameterTypeList();
    for (unsigned i = 0; i < function_proto_type->getNumParams(); i++) {
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
#else
bool ClangToDotTranslator::VisitFunctionProtoType(clang::FunctionProtoType * function_proto_type, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::VisitFunctionProtoType" << "\n";
#endif

    bool res = true;

#if 0
    SgFunctionParameterTypeList * param_type_list = new SgFunctionParameterTypeList();
    for (unsigned i = 0; i < function_proto_type->getNumParams(); i++) {
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
#endif

     node_desc.kind_hierarchy.push_back("FunctionProtoType");

  // DQ (11/27/2020): Updated to Clang 10.
  // for (unsigned i = 0; i < function_proto_type->getNumArgs(); i++)
     for (unsigned i = 0; i < function_proto_type->getNumParams(); i++)
        {
          std::ostringstream oss;
          oss << "arg_type[" << i << "]";

       // DQ (11/27/2020): Updated to Clang 10.
       // node_desc.successors.push_back(std::pair<std::string, std::string>(oss.str(), Traverse(function_proto_type->getArgType(i).getTypePtr())));
          node_desc.successors.push_back(std::pair<std::string, std::string>(oss.str(), Traverse(function_proto_type->getParamType(i).getTypePtr())));
        }

     if (function_proto_type->isVariadic())
          node_desc.attributes.push_back(std::pair<std::string, std::string>("have", "variadic"));

  // Using the return from Tristan's dot generator code.
  // return VisitType(function_proto_type, node_desc) && res;
     return VisitFunctionType(function_proto_type, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitInjectedClassNameType(clang::InjectedClassNameType * injected_class_name_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::InjectedClassNameType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitType(injected_class_name_type, node) && res;
}
#else
bool ClangToDotTranslator::VisitInjectedClassNameType(clang::InjectedClassNameType * injected_class_name_type, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::InjectedClassNameType" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("InjectedClassNameType");

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

#if CLANG_VERSION_MAJOR >= 22
    // (10/30/2026) lin32: Clang 22 moved injected specialization types to primary/partial template declarations.
    // (10/30/2026) lin32: Retrieve their canonical specialization to preserve both existing DOT successor labels.
    clang::CXXRecordDecl *decl = injected_class_name_type->getDecl();
    const clang::ASTContext &context = decl->getASTContext();
    clang::QualType injectedType;
    if (auto *partial = llvm::dyn_cast<clang::ClassTemplatePartialSpecializationDecl>(decl))
        injectedType = partial->getCanonicalInjectedSpecializationType(context);
    else if (clang::ClassTemplateDecl *classTemplate = decl->getDescribedClassTemplate())
        injectedType = classTemplate->getCanonicalInjectedSpecializationType(context);
    ROSE_ASSERT(!injectedType.isNull());

    node_desc.successors.push_back(
        std::pair<std::string, std::string>("injected_specialization_type", Traverse(injectedType.getTypePtr()))
    );
    node_desc.successors.push_back(
        std::pair<std::string, std::string>("injected_template_specialization_type", Traverse(injectedType.getTypePtr()))
    );
#else
    node_desc.successors.push_back(
        std::pair<std::string, std::string>("injected_specialization_type", Traverse(injected_class_name_type->getInjectedSpecializationType().getTypePtr()))
    );

    node_desc.successors.push_back(
        std::pair<std::string, std::string>("injected_template_specialization_type", Traverse(injected_class_name_type->getInjectedTST()))
    );
#endif

    node_desc.successors.push_back(
        std::pair<std::string, std::string>("declaration", Traverse(injected_class_name_type->getDecl()))
    );

    return VisitType(injected_class_name_type, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitLocInfoType(clang::LocInfoType * loc_info_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::LocInfoType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitType(loc_info_type, node) && res;
}
#else
bool ClangToDotTranslator::VisitLocInfoType(clang::LocInfoType * loc_info_type, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::LocInfoType" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("LocInfoType");

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitType(loc_info_type, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitMacroQualifiedType(clang::MacroQualifiedType * macro_qualified_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::MacroQualifiedType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitType(macro_qualified_type, node) && res;
}
#else
bool ClangToDotTranslator::VisitMacroQualifiedType(clang::MacroQualifiedType * macro_qualified_type, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::MacroQualifiedType" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("MacroQualifiedType");

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitType(macro_qualified_type, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitMemberPointerType(clang::MemberPointerType * member_pointer_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::MemberPointerType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitType(member_pointer_type, node) && res;
}
#else
bool ClangToDotTranslator::VisitMemberPointerType(clang::MemberPointerType * member_pointer_type, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::MemberPointerType" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("MemberPointerType");

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitType(member_pointer_type, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitPackExpansionType(clang::PackExpansionType * pack_expansion_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::PackExpansionType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitType(pack_expansion_type, node) && res;
}
#else
bool ClangToDotTranslator::VisitPackExpansionType(clang::PackExpansionType * pack_expansion_type, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::PackExpansionType" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("PackExpansionType");

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitType(pack_expansion_type, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitParenType(clang::ParenType * paren_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::VisitParenType" << "\n";
#endif

    *node = buildTypeFromQualifiedType(paren_type->getInnerType());

    return VisitType(paren_type, node);
}
#else
bool ClangToDotTranslator::VisitParenType(clang::ParenType * paren_type, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::VisitParenType" << "\n";
#endif

#if 0
    *node = buildTypeFromQualifiedType(paren_type->getInnerType());
#endif

     node_desc.kind_hierarchy.push_back("ParenType");

     node_desc.successors.push_back(std::pair<std::string, std::string>("inner_type", Traverse(paren_type->getInnerType().getTypePtr())));

    return VisitType(paren_type, node_desc);
}
#endif

#if 0
bool ClangToDotTranslator::VisitPipeType(clang::PipeType * pipe_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::PipeType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitType(pipe_type, node) && res;
}
#else
bool ClangToDotTranslator::VisitPipeType(clang::PipeType * pipe_type, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::PipeType" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("PipeType");

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitType(pipe_type, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitPointerType(clang::PointerType * pointer_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::VisitPointerType" << "\n";
#endif

    SgType * type = buildTypeFromQualifiedType(pointer_type->getPointeeType());

    *node = SageBuilder::buildPointerType(type);

    return VisitType(pointer_type, node);
}
#else
bool ClangToDotTranslator::VisitPointerType(clang::PointerType * pointer_type, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::VisitPointerType" << "\n";
#endif

#if 0
    SgType * type = buildTypeFromQualifiedType(pointer_type->getPointeeType());

    *node = SageBuilder::buildPointerType(type);
#endif

     node_desc.kind_hierarchy.push_back("PointerType");

     node_desc.successors.push_back(std::pair<std::string, std::string>("pointee_type", Traverse(pointer_type->getPointeeType().getTypePtr())));

    return VisitType(pointer_type, node_desc);
}
#endif

#if 0
bool ClangToDotTranslator::VisitReferenceType(clang::ReferenceType * reference_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::ReferenceType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitType(reference_type, node) && res;
}
#else
bool ClangToDotTranslator::VisitReferenceType(clang::ReferenceType * reference_type, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::ReferenceType" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("ReferenceType");

     ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

     node_desc.successors.push_back(std::pair<std::string, std::string>("pointee_type", Traverse(reference_type->getPointeeType().getTypePtr())));

    return VisitType(reference_type, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitLValueReferenceType(clang::LValueReferenceType * lvalue_reference_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::LValueReferenceType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitReferenceType(lvalue_reference_type, node) && res;
}
#else
bool ClangToDotTranslator::VisitLValueReferenceType(clang::LValueReferenceType * lvalue_reference_type, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::LValueReferenceType" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("LValueReferenceType");

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitReferenceType(lvalue_reference_type, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitRValueReferenceType(clang::RValueReferenceType * rvalue_reference_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::RValueReferenceType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitReferenceType(rvalue_reference_type, node) && res;
}
#else
bool ClangToDotTranslator::VisitRValueReferenceType(clang::RValueReferenceType * rvalue_reference_type, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::RValueReferenceType" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("RValueReferenceType");

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitReferenceType(rvalue_reference_type, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitSubstTemplateTypeParmPackType(clang::SubstTemplateTypeParmPackType * subst_template_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::SubstTemplateTypeParmPackType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitType(subst_template_type, node) && res;
}
#else
bool ClangToDotTranslator::VisitSubstTemplateTypeParmPackType(clang::SubstTemplateTypeParmPackType * subst_template_type, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::SubstTemplateTypeParmPackType" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("SubstTemplateTypeParmPackType");

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitType(subst_template_type, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitSubstTemplateTypeParmType(clang::SubstTemplateTypeParmType * subst_template_type_parm_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::SubstTemplateTypeParmType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitType(subst_template_type_parm_type, node) && res;
}
#else
bool ClangToDotTranslator::VisitSubstTemplateTypeParmType(clang::SubstTemplateTypeParmType * subst_template_type_parm_type, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::SubstTemplateTypeParmType" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("SubstTemplateTypeParmType");

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

#if CLANG_VERSION_MAJOR > 15
     node_desc.successors.push_back(
        std::pair<std::string, std::string>("replacement_decl", Traverse(subst_template_type_parm_type->getAssociatedDecl())) );

     clang::TemplateTypeParmDecl* parmDecl = const_cast<clang::TemplateTypeParmDecl*>(subst_template_type_parm_type->getReplacedParameter());
     node_desc.successors.push_back(std::pair<std::string, std::string>("replaced_parameter", Traverse(parmDecl)));
#else
     node_desc.successors.push_back(std::pair<std::string, std::string>("replaced_parameter", Traverse(subst_template_type_parm_type->getReplacedParameter())));
#endif

     node_desc.successors.push_back(
        std::pair<std::string, std::string>("replacement_type", Traverse(subst_template_type_parm_type->getReplacementType().getTypePtr())) );

    return VisitType(subst_template_type_parm_type, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitTagType(clang::TagType * tag_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::VisitTagType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME

    return VisitType(tag_type, node) && res;
}
#else
bool ClangToDotTranslator::VisitTagType(clang::TagType * tag_type, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_TYPE
     logger[DEBUG] << "ClangToDotTranslator::VisitTagType" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("TagType");

     ROSE_ASSERT(FAIL_FIXME == 0); // FIXME

     node_desc.successors.push_back(std::pair<std::string, std::string>("declaration", Traverse(tag_type->getDecl())));

     return VisitType(tag_type, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitEnumType(clang::EnumType * enum_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::VisitEnumType" << "\n";
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
#else
bool ClangToDotTranslator::VisitEnumType(clang::EnumType * enum_type, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::VisitEnumType" << "\n";
#endif

#if 0
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
#endif

     node_desc.kind_hierarchy.push_back("EnumType");

    return VisitType(enum_type, node_desc);
}
#endif

#if 0
bool ClangToDotTranslator::VisitRecordType(clang::RecordType * record_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::VisitRecordType" << "\n";
#endif

    SgSymbol * sym = GetSymbolFromSymbolTable(record_type->getDecl());

    SgClassSymbol * class_sym = isSgClassSymbol(sym);

    if (class_sym == NULL) {
        SgNode * tmp_decl = Traverse(record_type->getDecl());
        SgClassDeclaration * sg_decl = isSgClassDeclaration(tmp_decl);

        ROSE_ASSERT(sg_decl != NULL);

        *node = sg_decl->get_type();
    }
    else {
        *node = class_sym->get_type();
    }

    if (isSgClassType(*node) != NULL) {
        if (class_sym == NULL) {
            p_class_type_decl_first_see_in_type.insert(std::pair<SgClassType *, bool>(isSgClassType(*node), true));
            isSgNamedType(*node)->set_autonomous_declaration(true);
        }
        else
            p_class_type_decl_first_see_in_type.insert(std::pair<SgClassType *, bool>(isSgClassType(*node), false));
    }

    return VisitType(record_type, node);
}
#else
bool ClangToDotTranslator::VisitRecordType(clang::RecordType * record_type, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::VisitRecordType" << "\n";
#endif

#if 0
    SgSymbol * sym = GetSymbolFromSymbolTable(record_type->getDecl());

    SgClassSymbol * class_sym = isSgClassSymbol(sym);

    if (class_sym == NULL) {
        SgNode * tmp_decl = Traverse(record_type->getDecl());
        SgClassDeclaration * sg_decl = isSgClassDeclaration(tmp_decl);

        ROSE_ASSERT(sg_decl != NULL);

        *node = sg_decl->get_type();
    }
    else {
        *node = class_sym->get_type();
    }

    if (isSgClassType(*node) != NULL) {
        if (class_sym == NULL) {
            p_class_type_decl_first_see_in_type.insert(std::pair<SgClassType *, bool>(isSgClassType(*node), true));
            isSgNamedType(*node)->set_autonomous_declaration(true);
        }
        else
            p_class_type_decl_first_see_in_type.insert(std::pair<SgClassType *, bool>(isSgClassType(*node), false));
    }
#endif

     node_desc.successors.push_back(std::pair<std::string, std::string>("decl", Traverse(record_type->getDecl())));
     node_desc.kind_hierarchy.push_back("RecordType");

    return VisitType(record_type, node_desc);
}
#endif

#if 0
bool ClangToDotTranslator::VisitTemplateSpecializationType(clang::TemplateSpecializationType * template_specialization_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::TemplateSpecializationType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitType(template_specialization_type, node) && res;
}
#else
bool ClangToDotTranslator::VisitTemplateSpecializationType(clang::TemplateSpecializationType * template_specialization_type, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::TemplateSpecializationType" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("TemplateSpecializationType");

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    if (template_specialization_type->isTypeAlias())
        node_desc.successors.push_back(
            std::pair<std::string, std::string>("aliased_type", Traverse(template_specialization_type->getAliasedType().getTypePtr()))
        );

    const clang::TemplateName & template_name = template_specialization_type->getTemplateName();
    VisitTemplateName(template_name, node_desc, "template_name");

    clang::ArrayRef<clang::TemplateArgument> args;
    unsigned cnt = 0;
    for (const auto& arg : args) {
        std::ostringstream oss;
        oss << "template_argument[" << cnt++ << "]";
        VisitTemplateArgument(arg, node_desc, oss.str());
    }
    return VisitType(template_specialization_type, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitTemplateTypeParmType(clang::TemplateTypeParmType * template_type_parm_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
     logger[DEBUG] << "ClangToDotTranslator::TemplateTypeParmType" << "\n";
#endif
     bool res = true;

     ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

     return VisitType(template_type_parm_type, node) && res;
}
#else
bool ClangToDotTranslator::VisitTemplateTypeParmType(clang::TemplateTypeParmType * template_type_parm_type, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_TYPE
     logger[DEBUG] << "ClangToDotTranslator::TemplateTypeParmType" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("TemplateTypeParmType");

     ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

     node_desc.successors.push_back(std::pair<std::string, std::string>("declaration", Traverse(template_type_parm_type->getDecl())));

     clang::IdentifierInfo * indent_info = template_type_parm_type->getIdentifier();

     assert(indent_info != NULL); // I am not sure of it let try

     node_desc.attributes.push_back(std::pair<std::string, std::string>("identifier_name", indent_info->getName().data()));

     return VisitType(template_type_parm_type, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitTypedefType(clang::TypedefType * typedef_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::VisitTypedefType" << "\n";
#endif

    bool res = true;

    SgSymbol * sym = GetSymbolFromSymbolTable(typedef_type->getDecl());
    SgTypedefSymbol * tdef_sym = isSgTypedefSymbol(sym);

    if (tdef_sym == NULL) {
        logger[WARN] << "Runtime Error: Cannot find a typedef symbol for the TypedefType." << "\n";
        res = false;
    }

    *node = tdef_sym->get_type();

   return VisitType(typedef_type, node) && res;
}
#else
bool ClangToDotTranslator::VisitTypedefType(clang::TypedefType * typedef_type, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::VisitTypedefType" << "\n";
#endif

    bool res = true;

#if 0
    SgSymbol * sym = GetSymbolFromSymbolTable(typedef_type->getDecl());
    SgTypedefSymbol * tdef_sym = isSgTypedefSymbol(sym);

    if (tdef_sym == NULL) {
        logger[WARN] << "Runtime Error: Cannot find a typedef symbol for the TypedefType." << "\n";
        res = false;
    }

    *node = tdef_sym->get_type();
#endif

     node_desc.kind_hierarchy.push_back("TypedefType");

     node_desc.successors.push_back(std::pair<std::string, std::string>("declaration", Traverse(typedef_type->getDecl())));

    return VisitType(typedef_type, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitTypeOfExprType(clang::TypeOfExprType * type_of_expr_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::TypeOfExprType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitType(type_of_expr_type, node) && res;
}
#else
bool ClangToDotTranslator::VisitTypeOfExprType(clang::TypeOfExprType * type_of_expr_type, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::TypeOfExprType" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("TypeOfExprType");

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitType(type_of_expr_type, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitDependentTypeOfExprType(clang::DependentTypeOfExprType * dependent_type_of_expr_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::DependentTypeOfExprType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitTypeOfExprType(dependent_type_of_expr_type, node) && res;
}
#else
bool ClangToDotTranslator::VisitDependentTypeOfExprType(clang::DependentTypeOfExprType * dependent_type_of_expr_type, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::DependentTypeOfExprType" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("DependentTypeOfExprType");

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitTypeOfExprType(dependent_type_of_expr_type, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitTypeOfType(clang::TypeOfType * type_of_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::TypeOfType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitType(type_of_type, node) && res;
}
#else
bool ClangToDotTranslator::VisitTypeOfType(clang::TypeOfType * type_of_type, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::TypeOfType" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("TypeOfType");

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitType(type_of_type, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitTypeWithKeyword(clang::TypeWithKeyword * type_with_keyword, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::VisitTypeWithKeyword" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME

    return VisitType(type_with_keyword, node) && res;
}
#else
bool ClangToDotTranslator::VisitTypeWithKeyword(clang::TypeWithKeyword * type_with_keyword, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::VisitTypeWithKeyword" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("TypeWithKeyword");

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME

#if CLANG_VERSION_MAJOR >= 18
    switch (type_with_keyword->getKeyword()) {
        case clang::ElaboratedTypeKeyword::Struct:
            node_desc.attributes.push_back(std::pair<std::string, std::string>("elaborated_type_keyword", "struct"));
            break;
        case clang::ElaboratedTypeKeyword::Union:
            node_desc.attributes.push_back(std::pair<std::string, std::string>("elaborated_type_keyword", "union"));
            break;
        case clang::ElaboratedTypeKeyword::Class:
            node_desc.attributes.push_back(std::pair<std::string, std::string>("elaborated_type_keyword", "class"));
            break;
        case clang::ElaboratedTypeKeyword::Enum:
            node_desc.attributes.push_back(std::pair<std::string, std::string>("elaborated_type_keyword", "enum"));
            break;
        case clang::ElaboratedTypeKeyword::Typename:
            node_desc.attributes.push_back(std::pair<std::string, std::string>("elaborated_type_keyword", "typename"));
            break;
        case clang::ElaboratedTypeKeyword::Interface:
            node_desc.attributes.push_back(std::pair<std::string, std::string>("elaborated_type_keyword", "interface"));
            break;
        case clang::ElaboratedTypeKeyword::None:
            break;
    }
#else
    switch (type_with_keyword->getKeyword()) {
        case clang::ETK_Struct:
            node_desc.attributes.push_back(std::pair<std::string, std::string>("elaborated_type_keyword", "struct"));
            break;
        case clang::ETK_Union:
            node_desc.attributes.push_back(std::pair<std::string, std::string>("elaborated_type_keyword", "union"));
            break;
        case clang::ETK_Class:
            node_desc.attributes.push_back(std::pair<std::string, std::string>("elaborated_type_keyword", "class"));
            break;
        case clang::ETK_Enum:
            node_desc.attributes.push_back(std::pair<std::string, std::string>("elaborated_type_keyword", "enum"));
            break;
        case clang::ETK_Typename:
            node_desc.attributes.push_back(std::pair<std::string, std::string>("elaborated_type_keyword", "typename"));
            break;
        case clang::ETK_None:
            break;
    }
#endif

    return VisitType(type_with_keyword, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitDependentNameType(clang::DependentNameType * dependent_name_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::DependentNameType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitTypeWithKeyword(dependent_name_type, node) && res;
}
#else
bool ClangToDotTranslator::VisitDependentNameType(clang::DependentNameType * dependent_name_type, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::DependentNameType" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("DependentNameType");

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    VisitNestedNameSpecifier(dependent_name_type->getQualifier(), node_desc, "nested_name_qualifier");

    const clang::IdentifierInfo * identifier = dependent_name_type->getIdentifier();
    assert(identifier != NULL);
    node_desc.attributes.push_back(std::pair<std::string, std::string>("identifier" , identifier->getName().data()));

    return VisitTypeWithKeyword(dependent_name_type, node_desc) && res;
}
#endif

// (10/30/2026) lin32: Retain the old DOT visitors only where the pre-Clang-22 AST classes still exist.
#if CLANG_VERSION_MAJOR < 22
#if 0
bool ClangToDotTranslator::VisitDependentTemplateSpecializationType(clang::DependentTemplateSpecializationType * ependent_template_specialization_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::DependentTemplateSpecializationType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitTypeWithKeyword(ependent_template_specialization_type, node) && res;
}
#else
bool ClangToDotTranslator::VisitDependentTemplateSpecializationType(clang::DependentTemplateSpecializationType * ependent_template_specialization_type, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::DependentTemplateSpecializationType" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("DependentTemplateSpecializationType");

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitTypeWithKeyword(ependent_template_specialization_type, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitElaboratedType(clang::ElaboratedType * elaborated_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::VisitElaboratedType" << "\n";
#endif

    SgType * type = buildTypeFromQualifiedType(elaborated_type->getNamedType());

    // FIXME clang::ElaboratedType contains the "sugar" of a type reference (eg, "struct A" or "M::N::A"), it should be pass down to ROSE

    *node = type;

    return VisitTypeWithKeyword(elaborated_type, node);
}
#else
bool ClangToDotTranslator::VisitElaboratedType(clang::ElaboratedType * elaborated_type, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::VisitElaboratedType" << "\n";
#endif

#if 0
    SgType * type = buildTypeFromQualifiedType(elaborated_type->getNamedType());

    // FIXME clang::ElaboratedType contains the "sugar" of a type reference (eg, "struct A" or "M::N::A"), it should be pass down to ROSE

    *node = type;
#endif

     node_desc.kind_hierarchy.push_back("ElaboratedType");

     node_desc.successors.push_back(std::pair<std::string, std::string>("named_type", Traverse(elaborated_type->getNamedType().getTypePtr())));

     return VisitTypeWithKeyword(elaborated_type, node_desc);
}
#endif

#endif // CLANG_VERSION_MAJOR < 22

#if 0
bool ClangToDotTranslator::VisitUnaryTransformType(clang::UnaryTransformType * unary_transform_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::UnaryTransformType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitType(unary_transform_type, node) && res;
}
#else
bool ClangToDotTranslator::VisitUnaryTransformType(clang::UnaryTransformType * unary_transform_type, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::UnaryTransformType" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("UnaryTransformType");

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitType(unary_transform_type, node_desc) && res;
}
#endif

// (10/30/2026) lin32: Clang 21 represents dependent unary transforms with UnaryTransformType instead.
#if CLANG_VERSION_MAJOR < 21
#if 0
bool ClangToDotTranslator::VisitDependentUnaryTransformType(clang::DependentUnaryTransformType * dependent_unary_transform_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::DependentUnaryTransformType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitUnaryTransformType(dependent_unary_transform_type, node) && res;
}
#else
bool ClangToDotTranslator::VisitDependentUnaryTransformType(clang::DependentUnaryTransformType * dependent_unary_transform_type, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::DependentUnaryTransformType" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("DependentUnaryTransformType");

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitUnaryTransformType(dependent_unary_transform_type, node_desc) && res;
}
#endif

#endif // CLANG_VERSION_MAJOR < 21

#if 0
bool ClangToDotTranslator::VisitUnresolvedUsingType(clang::UnresolvedUsingType * unresolved_using_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::UnresolvedUsingType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitType(unresolved_using_type, node) && res;
}
#else
bool ClangToDotTranslator::VisitUnresolvedUsingType(clang::UnresolvedUsingType * unresolved_using_type, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::UnresolvedUsingType" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("UnresolvedUsingType");

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME 

    return VisitType(unresolved_using_type, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitVectorType(clang::VectorType * vector_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::VisitVectorType" << "\n";
#endif

    SgType * type = buildTypeFromQualifiedType(vector_type->getElementType());

    SgModifierType * modified_type = new SgModifierType(type);
    SgTypeModifier & sg_modifer = modified_type->get_typeModifier();

    sg_modifer.setVectorType();
    sg_modifer.set_vector_size(vector_type->getNumElements());

    *node = SgModifierType::insertModifierTypeIntoTypeTable(modified_type);

    return VisitType(vector_type, node);
}
#else
bool ClangToDotTranslator::VisitVectorType(clang::VectorType * vector_type, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::VisitVectorType" << "\n";
#endif

#if 0
    SgType * type = buildTypeFromQualifiedType(vector_type->getElementType());

    SgModifierType * modified_type = new SgModifierType(type);
    SgTypeModifier & sg_modifer = modified_type->get_typeModifier();

    sg_modifer.setVectorType();
    sg_modifer.set_vector_size(vector_type->getNumElements());

    *node = SgModifierType::insertModifierTypeIntoTypeTable(modified_type);
#endif

     node_desc.kind_hierarchy.push_back("VectorType");

     node_desc.successors.push_back(std::pair<std::string, std::string>("element_type", Traverse(vector_type->getElementType().getTypePtr())));

     std::ostringstream oss;
     oss << vector_type->getNumElements();
     node_desc.attributes.push_back(std::pair<std::string, std::string>("number_element", oss.str()));

     return VisitType(vector_type, node_desc);
}
#endif

#if 0
bool ClangToDotTranslator::VisitExtVectorType(clang::ExtVectorType * ext_vector_type, SgNode ** node) {
#if DEBUG_VISIT_TYPE
    logger[DEBUG] << "ClangToDotTranslator::VisitExtVectorType" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME Is it anything to be done here?

    return VisitVectorType(ext_vector_type, node) && res;
}
#else
bool ClangToDotTranslator::VisitExtVectorType(clang::ExtVectorType * ext_vector_type, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_TYPE
     logger[DEBUG] << "ClangToDotTranslator::VisitExtVectorType" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("ExtVectorType");

     ROSE_ASSERT(FAIL_FIXME == 0); // FIXME Is it anything to be done here?

     return VisitVectorType(ext_vector_type, node_desc) && res;
}
#endif
