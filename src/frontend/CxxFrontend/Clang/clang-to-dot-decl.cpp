#include "sage3basic.h"
#include "clang-to-dot-private.hpp"

using namespace Sawyer::Message;

std::string
ClangToDotTranslator::Traverse(clang::Decl * decl) 
   {
     if (decl == NULL)
        {
          return "";
        }

#if 0
     std::map<clang::Decl *, SgNode *>::iterator it = p_decl_translation_map.find(decl);
     if (it != p_decl_translation_map.end()) 
        {
#if DEBUG_TRAVERSE_DECL
          logger[DEBUG] << "Traverse Decl : " << decl << " ";
          if (clang::NamedDecl::classof(decl)) 
             {
               logger[DEBUG] << ": " << ((clang::NamedDecl *)decl)->getNameAsString() << ") ";
             }
          logger[DEBUG] << " already visited : node = " << it->second << "\n";
#endif
          return it->second;
        }

     SgNode * result = NULL;
     bool ret_status = false;
#else
    // Look for previous translation
    std::map<clang::Decl *, std::string>::iterator it = p_decl_translation_map.find(decl);
    if (it != p_decl_translation_map.end())
        return it->second;

    // If first time, create a new entry
    std::string node_ident = genNextIdent();
    p_decl_translation_map.insert(std::pair<clang::Decl *, std::string>(decl, node_ident));
    NodeDescriptor & node_desc = p_node_desc.insert(std::pair<std::string, NodeDescriptor>(node_ident, NodeDescriptor(node_ident))).first->second;

    bool ret_status = false;
#endif

  // CLANG_ROSE_Graph::graph (decl);

     switch (decl->getKind()) 
        {
        case clang::Decl::AccessSpec:
            ret_status = VisitAccessSpecDecl((clang::AccessSpecDecl *)decl, node_desc);
            break;
        case clang::Decl::Block:
            ret_status = VisitBlockDecl((clang::BlockDecl *)decl, node_desc);
            break;
        case clang::Decl::Captured:
            ret_status = VisitCapturedDecl((clang::CapturedDecl *)decl, node_desc);
            break;
        case clang::Decl::Empty:
            ret_status = VisitEmptyDecl((clang::EmptyDecl *)decl, node_desc);
            break;
        case clang::Decl::Export:
            ret_status = VisitExportDecl((clang::ExportDecl *)decl, node_desc);
            break;
        case clang::Decl::ExternCContext:
            ret_status = VisitExternCContextDecl((clang::ExternCContextDecl *)decl, node_desc);
            break;
        case clang::Decl::FileScopeAsm:
            ret_status = VisitFileScopeAsmDecl((clang::FileScopeAsmDecl *)decl, node_desc);
            break;
        case clang::Decl::Friend:
            ret_status = VisitFriendDecl((clang::FriendDecl *)decl, node_desc);
            break;
        case clang::Decl::FriendTemplate:
            ret_status = VisitFriendTemplateDecl((clang::FriendTemplateDecl *)decl, node_desc);
            break;
        case clang::Decl::Import:
            ret_status = VisitImportDecl((clang::ImportDecl *)decl, node_desc);
            break;
        case clang::Decl::Label:
            ret_status = VisitLabelDecl((clang::LabelDecl *)decl, node_desc);
            break;
        case clang::Decl::NamespaceAlias:
            ret_status = VisitNamespaceAliasDecl((clang::NamespaceAliasDecl *)decl, node_desc);
            break;
        case clang::Decl::Namespace:
            ret_status = VisitNamespaceDecl((clang::NamespaceDecl *)decl, node_desc);
            break;
        case clang::Decl::BuiltinTemplate:
            ret_status = VisitBuiltinTemplateDecl((clang::BuiltinTemplateDecl *)decl, node_desc);
            break;
        case clang::Decl::Concept:
            ret_status = VisitConceptDecl((clang::ConceptDecl *)decl, node_desc);
            break;
        case clang::Decl::ClassTemplate:
            ret_status = VisitClassTemplateDecl((clang::ClassTemplateDecl *)decl, node_desc);
            break;
        case clang::Decl::FunctionTemplate:
            ret_status = VisitFunctionTemplateDecl((clang::FunctionTemplateDecl *)decl, node_desc);
            break;
        case clang::Decl::TypeAliasTemplate:
            ret_status = VisitTypeAliasTemplateDecl((clang::TypeAliasTemplateDecl *)decl, node_desc);
            break;
        case clang::Decl::VarTemplate:
            ret_status = VisitVarTemplateDecl((clang::VarTemplateDecl *)decl, node_desc);
            break;
        case clang::Decl::TemplateTemplateParm:
            ret_status = VisitTemplateTemplateParmDecl((clang::TemplateTemplateParmDecl *)decl, node_desc);
            break;
        case clang::Decl::Record:
            ret_status = VisitRecordDecl((clang::RecordDecl *)decl, node_desc);
            break;
        case clang::Decl::CXXRecord:
            ret_status = VisitCXXRecordDecl((clang::CXXRecordDecl *)decl, node_desc);
            break;
        case clang::Decl::ClassTemplateSpecialization:
            ret_status = VisitClassTemplateSpecializationDecl((clang::ClassTemplateSpecializationDecl *)decl, node_desc);
            break;
        case clang::Decl::ClassTemplatePartialSpecialization:
            ret_status = VisitClassTemplatePartialSpecializationDecl((clang::ClassTemplatePartialSpecializationDecl *)decl, node_desc);
            break;
        case clang::Decl::Enum:
            ret_status = VisitEnumDecl((clang::EnumDecl *)decl, node_desc);
            break;
        case clang::Decl::TemplateTypeParm:
            ret_status = VisitTemplateTypeParmDecl((clang::TemplateTypeParmDecl *)decl, node_desc);
            break;
        case clang::Decl::Typedef:
            ret_status = VisitTypedefDecl((clang::TypedefDecl *)decl, node_desc);
            break;
        case clang::Decl::TypeAlias:
            ret_status = VisitTypeAliasDecl((clang::TypeAliasDecl *)decl, node_desc);
            break;
        case clang::Decl::UnresolvedUsingTypename:
            ret_status = VisitUnresolvedUsingTypenameDecl((clang::UnresolvedUsingTypenameDecl *)decl, node_desc);
            break;
        case clang::Decl::Using:
            ret_status = VisitUsingDecl((clang::UsingDecl *)decl, node_desc);
            break;
        case clang::Decl::UsingDirective:
            ret_status = VisitUsingDirectiveDecl((clang::UsingDirectiveDecl *)decl, node_desc);
            break;
        case clang::Decl::UsingPack:
            ret_status = VisitUsingPackDecl((clang::UsingPackDecl *)decl, node_desc);
            break;
        case clang::Decl::ConstructorUsingShadow:
            ret_status = VisitConstructorUsingShadowDecl((clang::ConstructorUsingShadowDecl *)decl, node_desc);
            break;
        case clang::Decl::Binding:
            ret_status = VisitBindingDecl((clang::BindingDecl *)decl, node_desc);
            break;
        case clang::Decl::Field:
            ret_status = VisitFieldDecl((clang::FieldDecl *)decl, node_desc);
            break;
        case clang::Decl::Function:
            ret_status = VisitFunctionDecl((clang::FunctionDecl *)decl, node_desc);
            break;
        case clang::Decl::CXXDeductionGuide:
            ret_status = VisitCXXDeductionGuideDecl((clang::CXXDeductionGuideDecl *)decl, node_desc);
            break;
        case clang::Decl::CXXMethod:
            ret_status = VisitCXXMethodDecl((clang::CXXMethodDecl *)decl, node_desc);
            break;
        case clang::Decl::CXXConstructor:
            ret_status = VisitCXXConstructorDecl((clang::CXXConstructorDecl *)decl, node_desc);
            break;
        case clang::Decl::CXXConversion:
            ret_status = VisitCXXConversionDecl((clang::CXXConversionDecl *)decl, node_desc);
            break;
        case clang::Decl::CXXDestructor:
            ret_status = VisitCXXDestructorDecl((clang::CXXDestructorDecl *)decl, node_desc);
            break;
        case clang::Decl::MSProperty:
            ret_status = VisitMSPropertyDecl((clang::MSPropertyDecl *)decl, node_desc);
            break;
        case clang::Decl::NonTypeTemplateParm:
            ret_status = VisitNonTypeTemplateParmDecl((clang::NonTypeTemplateParmDecl *)decl, node_desc);
            break;
        case clang::Decl::Decomposition:
            ret_status = VisitDecompositionDecl((clang::DecompositionDecl *)decl, node_desc);
            break;
        case clang::Decl::ImplicitParam:
            ret_status = VisitImplicitParamDecl((clang::ImplicitParamDecl *)decl, node_desc);
            break;
        case clang::Decl::OMPCapturedExpr:
            ret_status = VisitOMPCaptureExprDecl((clang::OMPCapturedExprDecl *)decl, node_desc);
            break;
        case clang::Decl::ParmVar:
            ret_status = VisitParmVarDecl((clang::ParmVarDecl *)decl, node_desc);
            break;
        case clang::Decl::VarTemplatePartialSpecialization:
            ret_status = VisitVarTemplatePartialSpecializationDecl((clang::VarTemplatePartialSpecializationDecl *)decl, node_desc);
            break;
        case clang::Decl::EnumConstant:
            ret_status = VisitEnumConstantDecl((clang::EnumConstantDecl *)decl, node_desc);
            break;
        case clang::Decl::IndirectField:
            ret_status = VisitIndirectFieldDecl((clang::IndirectFieldDecl *)decl, node_desc);
            break;
        case clang::Decl::OMPDeclareMapper:
            ret_status = VisitOMPDeclareMapperDecl((clang::OMPDeclareMapperDecl *)decl, node_desc);
            break;
        case clang::Decl::OMPDeclareReduction:
            ret_status = VisitOMPDeclareReductionDecl((clang::OMPDeclareReductionDecl *)decl, node_desc);
            break;
        case clang::Decl::UnresolvedUsingValue:
            ret_status = VisitUnresolvedUsingValueDecl((clang::UnresolvedUsingValueDecl *)decl, node_desc);
            break;
        case clang::Decl::OMPAllocate:
            ret_status = VisitOMPAllocateDecl((clang::OMPAllocateDecl *)decl, node_desc);
            break;
        case clang::Decl::OMPRequires:
            ret_status = VisitOMPRequiresDecl((clang::OMPRequiresDecl *)decl, node_desc);
            break;
        case clang::Decl::OMPThreadPrivate:
            ret_status = VisitOMPThreadPrivateDecl((clang::OMPThreadPrivateDecl *)decl, node_desc);
            break;
        case clang::Decl::PragmaComment:
            ret_status = VisitPragmaCommentDecl((clang::PragmaCommentDecl *)decl, node_desc);
            break;
        case clang::Decl::PragmaDetectMismatch:
            ret_status = VisitPragmaDetectMismatchDecl((clang::PragmaDetectMismatchDecl *)decl, node_desc);
            break;
        case clang::Decl::StaticAssert:
            ret_status = VisitStaticAssertDecl((clang::StaticAssertDecl *)decl, node_desc);
            break;
        case clang::Decl::TranslationUnit:
            ret_status = VisitTranslationUnitDecl((clang::TranslationUnitDecl *)decl, node_desc);
            break;
        case clang::Decl::Var:
            ret_status = VisitVarDecl((clang::VarDecl *)decl, node_desc);
            break;
        default:
            logger[ERROR] << "Unknown declacaration kind: " << decl->getDeclKindName() << " !" << "\n";
            ROSE_ABORT();
    }

  // DQ (11/27/2020): Added debugging support.
  // logger[DEBUG] << "ret_status = " << (ret_status ? "true" : "false") << "\n";

 // ROSE_ASSERT(ret_status == false || result != NULL);
 // ROSE_ASSERT(ret_status == false);

 // p_decl_translation_map.insert(std::pair<clang::Decl *, SgNode *>(decl, result));

#if DEBUG_TRAVERSE_DECL
    logger[DEBUG] << "Traverse(clang::Decl : " << decl << " ";
    if (clang::NamedDecl::classof(decl)) {
        logger[DEBUG] << ": " << ((clang::NamedDecl *)decl)->getNameAsString() << ") ";
    }
 // logger[DEBUG] << " visit done : node = " << result << "\n";
#endif

 // return ret_status ? result : NULL;
 // return ret_status;
    return node_ident;
}

#if 0
SgNode * ClangToDotTranslator::TraverseForDeclContext(clang::DeclContext * decl_context) 
   {
     CLANG_ROSE_Graph::graph (decl_context);

     return Traverse((clang::Decl*)decl_context);
   }
#endif

/**********************/
/* Visit Declarations */
/**********************/

#if 0
bool ClangToDotTranslator::VisitDecl(clang::Decl * decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitDecl" << "\n";
#endif    
    if (*node == NULL) {
        logger[WARN] << "Runtime error: No Sage node associated with the declaration..." << "\n";
        return false;
    }

    if (!isSgGlobal(*node))
        applySourceRange(*node, decl->getSourceRange());

    // TODO attributes
/*
    logger[DEBUG] << "Attribute list for " << decl->getDeclKindName() << " (" << decl << "): ";
    clang::Decl::attr_iterator it;
    for (it = decl->attr_begin(); it != decl->attr_end(); it++) {
        logger[DEBUG] << (*it)->getKind() << ", ";
    }
    logger[DEBUG] << "\n";

    if (clang::VarDecl::classof(decl)) {
        clang::VarDecl * var_decl = (clang::VarDecl *)decl;
        logger[DEBUG] << "Stoprage class for " << decl->getDeclKindName() << " (" << decl << "): " << var_decl->getStorageClass() << "\n";
    }
*/
    return true;
}
#else
bool ClangToDotTranslator::VisitDecl(clang::Decl * decl, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitDecl" << "\n";
#endif

#if 0
    if (*node == NULL) {
        logger[WARN] << "Runtime error: No Sage node associated with the declaration..." << "\n";
        return false;
    }

    if (!isSgGlobal(*node))
        applySourceRange(*node, decl->getSourceRange());

    // TODO attributes
/*
    logger[DEBUG] << "Attribute list for " << decl->getDeclKindName() << " (" << decl << "): ";
    clang::Decl::attr_iterator it;
    for (it = decl->attr_begin(); it != decl->attr_end(); it++) {
        logger[DEBUG] << (*it)->getKind() << ", ";
    }
    logger[DEBUG] << "\n";

    if (clang::VarDecl::classof(decl)) {
        clang::VarDecl * var_decl = (clang::VarDecl *)decl;
        logger[DEBUG] << "Stoprage class for " << decl->getDeclKindName() << " (" << decl << "): " << var_decl->getStorageClass() << "\n";
    }
*/
#endif

     node_desc.kind_hierarchy.push_back("Decl");

     switch (decl->getAccess()) 
        {
          case clang::AS_public:
               node_desc.attributes.push_back(std::pair<std::string, std::string>("access_specifier", "public"));
               break;
          case clang::AS_protected:
               node_desc.attributes.push_back(std::pair<std::string, std::string>("access_specifier", "protected"));
               break;
          case clang::AS_private:
               node_desc.attributes.push_back(std::pair<std::string, std::string>("access_specifier", "private`"));
               break;
          case clang::AS_none:
               break;
        }

     clang::Decl::attr_iterator it;
     unsigned cnt = 0;
     for (it = decl->attr_begin(); it != decl->attr_end(); it++) 
        {
          std::ostringstream oss;
          oss << "attribute[" << cnt++ << "]";
#if 1
       // DQ (11/27/2020): I'm not clear what code this is exapanding into.
          switch ((*it)->getKind()) 
             {
#define ATTR(X) case clang::attr::X: \
                     node_desc.attributes.push_back(std::pair<std::string, std::string>(oss.str(), "X")); \
                     break;
#include "clang/Basic/AttrList.inc"
             }
#endif
        }

     node_desc.successors.push_back(std::pair<std::string, std::string>("most_recent_decl", Traverse(decl->getMostRecentDecl())));
     node_desc.successors.push_back(std::pair<std::string, std::string>("previous_decl", Traverse(decl->getPreviousDecl())));
     clang::DeclContext* declContext = decl->getDeclContext();
     if(declContext)
       node_desc.successors.push_back(std::pair<std::string, std::string>("DeclContext", Traverse(clang::Decl::castFromDeclContext(declContext))));

     if(decl->isCanonicalDecl())
        {
           node_desc.successors.push_back(std::pair<std::string, std::string>("canonical_decl", Traverse(decl->getCanonicalDecl())));
        }

     if(decl->hasBody())
        {
           node_desc.successors.push_back(std::pair<std::string, std::string>("body", Traverse(decl->getBody())));
        }
     if(decl->isImplicit())
        {
           node_desc.attributes.push_back(std::pair<std::string, std::string>("isImplicit", "Yes"));
        }
     if(decl->isFirstDecl())
        {
           node_desc.attributes.push_back(std::pair<std::string, std::string>("isFirstDecl", "Yes"));
        }

     return true;
   }
#endif


#if 0
bool ClangToDotTranslator::VisitAccessSpecDecl(clang::AccessSpecDecl * access_spec_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitAccessSpecDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitDecl(access_spec_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitAccessSpecDecl(clang::AccessSpecDecl * access_spec_decl, NodeDescriptor & node_desc) 
   {
#if DEBUG_VISIT_DECL
     logger[DEBUG] << "ClangToDotTranslator::VisitAccessSpecDecl" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("AccessSpecDecl");

     //ROSE_ASSERT(FAIL_TODO == 0); // TODO

     return VisitDecl(access_spec_decl, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitBlockDecl(clang::BlockDecl * block_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitBlockDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitDecl(block_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitBlockDecl(clang::BlockDecl * block_decl, NodeDescriptor & node_desc) 
   {
#if DEBUG_VISIT_DECL
     logger[DEBUG] << "ClangToDotTranslator::VisitBlockDecl" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("BlockDecl");

     ROSE_ASSERT(FAIL_TODO == 0); // TODO

     return VisitDecl(block_decl, node_desc) && res;
   }
#endif

#if 0
bool ClangToDotTranslator::VisitCapturedDecl(clang::CapturedDecl * captured_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitCapturedDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitDecl(captured_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitCapturedDecl(clang::CapturedDecl * captured_decl, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitCapturedDecl" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("CapturedDecl");

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitDecl(captured_decl, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitEmptyDecl(clang::EmptyDecl * empty_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitEmptyDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitDecl(empty_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitEmptyDecl(clang::EmptyDecl * empty_decl, NodeDescriptor & node_desc) 
   {
#if DEBUG_VISIT_DECL
     logger[DEBUG] << "ClangToDotTranslator::VisitEmptyDecl" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("EmptyDecl");

  // ROSE_ASSERT(FAIL_TODO == 0); // TODO
     logger[DEBUG] << "ClangToDotTranslator::VisitEmptyDecl called but not implemented! \n";

     return VisitDecl(empty_decl, node_desc) && res;
   }
#endif

#if 0
bool ClangToDotTranslator::VisitExportDecl(clang::ExportDecl * export_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitExportDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitDecl(export_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitExportDecl(clang::ExportDecl * export_decl, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitExportDecl" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("ExportDecl");

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitDecl(export_decl, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitExternCContextDecl(clang::ExternCContextDecl * ccontent_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitCContextDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitDecl(ccontent_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitExternCContextDecl(clang::ExternCContextDecl * ccontent_decl, NodeDescriptor & node_desc) 
   {
#if DEBUG_VISIT_DECL
     logger[DEBUG] << "ClangToDotTranslator::VisitCContextDecl" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("ExternCContextDecl");

     ROSE_ASSERT(FAIL_TODO == 0); // TODO

     return VisitDecl(ccontent_decl, node_desc) && res;
   }
#endif

#if 0
bool ClangToDotTranslator::VisitFileScopeAsmDecl(clang::FileScopeAsmDecl * file_scope_asm_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitFileScopeAsmDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitDecl(file_scope_asm_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitFileScopeAsmDecl(clang::FileScopeAsmDecl * file_scope_asm_decl, NodeDescriptor & node_desc) 
   {
#if DEBUG_VISIT_DECL
     logger[DEBUG] << "ClangToDotTranslator::VisitFileScopeAsmDecl" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("FileScopeAsmDecl");

     // ROSE_ASSERT(FAIL_TODO == 0); // TODO
     logger[DEBUG] <<"ClangToDotTranslator::VisitFileScopeAsmDecl called but not implemented! \n";

     return VisitDecl(file_scope_asm_decl, node_desc) && res;
   }
#endif

#if 0
bool ClangToDotTranslator::VisitFriendDecl(clang::FriendDecl * friend_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitFriendDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitDecl(friend_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitFriendDecl(clang::FriendDecl * friend_decl, NodeDescriptor & node_desc) 
   {
#if DEBUG_VISIT_DECL
     logger[DEBUG] << "ClangToDotTranslator::VisitFriendDecl" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("FriendDecl");

    clang::NamedDecl * named_decl = friend_decl->getFriendDecl();
    clang::TypeSourceInfo * type_source_info = friend_decl->getFriendType();

    assert(named_decl == NULL xor type_source_info == NULL); // I think it is and only one: let see!

    if (named_decl != NULL) {
      node_desc.successors.push_back(std::pair<std::string, std::string>("friend_decl", Traverse(named_decl)));
    }

    if (type_source_info != NULL) {
      node_desc.successors.push_back(std::pair<std::string, std::string>("friend_type", Traverse(type_source_info->getType().getTypePtr())));
    }


     return VisitDecl(friend_decl, node_desc) && res;
   }
#endif

#if 0
bool ClangToDotTranslator::VisitFriendTemplateDecl(clang::FriendTemplateDecl * friend_template_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitFriendTemplateDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitDecl(friend_template_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitFriendTemplateDecl(clang::FriendTemplateDecl * friend_template_decl, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitFriendTemplateDecl" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("FriendTemplateDecl");

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitDecl(friend_template_decl, node_desc) && res;
}
#endif


#if 0
bool ClangToDotTranslator::VisitImportDecl(clang::ImportDecl * import_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitImportDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME

    return VisitDecl(import_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitImportDecl(clang::ImportDecl * import_decl, NodeDescriptor & node_desc) 
   {
#if DEBUG_VISIT_DECL
     logger[DEBUG] << "ClangToDotTranslator::VisitImportDecl" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("ImportDecl");

     ROSE_ASSERT(FAIL_FIXME == 0); // FIXME

     return VisitDecl(import_decl, node_desc) && res;
   }
#endif

#if 0
bool ClangToDotTranslator::VisitNamedDecl(clang::NamedDecl * named_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitNamedDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME

    return VisitDecl(named_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitNamedDecl(clang::NamedDecl * named_decl, NodeDescriptor & node_desc) 
   {
#if DEBUG_VISIT_DECL
     logger[DEBUG] << "ClangToDotTranslator::VisitNamedDecl" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("NamedDecl");
     node_desc.attributes.push_back(std::pair<std::string, std::string>("name", named_decl->getNameAsString()));

#if 0
  // Clang 10 does not store the linkage in the same place as Clang 3.x
     switch (named_decl->getLinkage()) 
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
#else
  // DQ (11/27/2020): I want to print out this warning, but not too much.
#if 0
  // DQ (11/28/2020): Cleaned up all the remaining default output from ROSE using Clang as frontend.
     static int counter = 0;
     if (counter % 1000 == 0)
        {
          logger[DEBUG] <<"Need to support linkage from different location than Clang 3.x (assuming NoLinkage) \n";
        }
     counter++;
#endif
#endif

     switch (named_decl->getVisibility()) 
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

     node_desc.successors.push_back(std::pair<std::string, std::string>("(underlying_decl", Traverse(named_decl->getUnderlyingDecl())));

     ROSE_ASSERT(FAIL_FIXME == 0); // FIXME

     return VisitDecl(named_decl, node_desc) && res;
   }
#endif

#if 0
bool ClangToDotTranslator::VisitLabelDecl(clang::LabelDecl * label_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitLabelDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitNamedDecl(label_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitLabelDecl(clang::LabelDecl * label_decl, NodeDescriptor & node_desc) 
   {
#if DEBUG_VISIT_DECL
     logger[DEBUG] << "ClangToDotTranslator::VisitLabelDecl" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("LabelDecl");

     ROSE_ASSERT(FAIL_TODO == 0); // TODO

     return VisitNamedDecl(label_decl, node_desc) && res;
   }
#endif

#if 0
bool ClangToDotTranslator::VisitNamespaceAliasDecl(clang::NamespaceAliasDecl * namespace_alias_decl, SgNode ** node) 
   {
#if DEBUG_VISIT_DECL
     logger[DEBUG] << "ClangToDotTranslator::VisitNamespaceAliasDecl" << "\n";
#endif
     bool res = true;

     ROSE_ASSERT(FAIL_TODO == 0); // TODO

     return VisitNamedDecl(namespace_alias_decl, node) && res;
   }
#else
bool ClangToDotTranslator::VisitNamespaceAliasDecl(clang::NamespaceAliasDecl * namespace_alias_decl, NodeDescriptor & node_desc) 
   {
#if DEBUG_VISIT_DECL
     logger[DEBUG] << "ClangToDotTranslator::VisitNamespaceAliasDecl" << "\n";
#endif
     bool res = true;

    clang::NamedDecl* aliasedNamespaeDecl = namespace_alias_decl->getAliasedNamespace();

     node_desc.kind_hierarchy.push_back("NamespaceAliasDecl");

     node_desc.successors.push_back(std::pair<std::string, std::string>("Namespace", Traverse(namespace_alias_decl->getNamespace())));
     node_desc.successors.push_back(std::pair<std::string, std::string>("AliasedNamespace", Traverse(namespace_alias_decl->getAliasedNamespace())));

     return VisitNamedDecl(namespace_alias_decl, node_desc) && res;
   }
#endif

#if 0
bool ClangToDotTranslator::VisitNamespaceDecl(clang::NamespaceDecl * namespace_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitNamespaceDecl" << "\n";
#endif
    bool res = true;

    //ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitNamedDecl(namespace_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitNamespaceDecl(clang::NamespaceDecl * namespace_decl, NodeDescriptor & node_desc) 
   {
#if DEBUG_VISIT_DECL
     logger[DEBUG] << "ClangToDotTranslator::VisitNamespaceDecl" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("NamespaceDecl");

    // (10/30/2026) lin32: Clang 19 replaced getOriginalNamespace(); the canonical declaration is still the first namespace.
#if CLANG_VERSION_MAJOR >= 19
    node_desc.successors.push_back(std::pair<std::string, std::string>("original_namespace", Traverse(namespace_decl->getCanonicalDecl())));
#else
    node_desc.successors.push_back(std::pair<std::string, std::string>("original_namespace", Traverse(namespace_decl->getOriginalNamespace())));
#endif

 // DQ (11/28/2020): this function no longer exists in Clang 10.
 // node_desc.successors.push_back(std::pair<std::string, std::string>("next_namespace", Traverse(namespace_decl->getNextNamespace())));

    clang::DeclContext::decl_iterator it;
    unsigned cnt = 0;
    for (it = namespace_decl->decls_begin(); it != namespace_decl->decls_end(); it++) {
        std::ostringstream oss;
        oss << "DeclContext::decls[" << cnt++ << "]";
        node_desc.successors.push_back(std::pair<std::string, std::string>(oss.str(), Traverse(*it)));
    }

  // ROSE_ASSERT(FAIL_TODO == 0); // TODO

     return VisitNamedDecl(namespace_decl, node_desc) && res;
   }
#endif

#if 0
bool ClangToDotTranslator::VisitTemplateDecl(clang::TemplateDecl * template_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitTemplateDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME

    return VisitNamedDecl(template_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitTemplateDecl(clang::TemplateDecl * template_decl, NodeDescriptor & node_desc) 
   {
#if DEBUG_VISIT_DECL
     logger[DEBUG] << "ClangToDotTranslator::VisitTemplateDecl" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("TemplateDecl");

    clang::TemplateParameterList * template_parameters = template_decl->getTemplateParameters();
    assert(template_parameters != NULL);

    clang::TemplateParameterList::iterator it;
    unsigned cnt = 0;
    for (it = template_parameters->begin(); it != template_parameters->end(); it++) {
        std::ostringstream oss;
        oss << "template_parameter[" << cnt++ << "]";
        node_desc.successors.push_back(std::pair<std::string, std::string>(oss.str(), Traverse(*it)));
    }

    node_desc.successors.push_back(std::pair<std::string, std::string>("templated_decl", Traverse(template_decl->getTemplatedDecl())));

     ROSE_ASSERT(FAIL_FIXME == 0); // FIXME

     return VisitNamedDecl(template_decl, node_desc) && res;
   }
#endif

#if 0
bool ClangToDotTranslator::VisitBuiltinTemplateDecl(clang::BuiltinTemplateDecl * builtin_template_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitBuiltinTemplateDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME

    return VisitTemplateDecl(builtin_template_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitBuiltinTemplateDecl(clang::BuiltinTemplateDecl * builtin_template_decl, NodeDescriptor & node_desc) 
   {
#if DEBUG_VISIT_DECL
     logger[DEBUG] << "ClangToDotTranslator::VisitBuiltinTemplateDecl" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("BuiltinTemplateDecl");

     ROSE_ASSERT(FAIL_FIXME == 0); // FIXME

     return VisitTemplateDecl(builtin_template_decl, node_desc) && res;
   }
#endif

#if 0
bool ClangToDotTranslator::VisitConceptDecl(clang::ConceptDecl * concept_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitConceptDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME

    return VisitTemplateDecl(concept_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitConceptDecl(clang::ConceptDecl * concept_decl, NodeDescriptor & node_desc)
   {
#if DEBUG_VISIT_DECL
     logger[DEBUG] << "ClangToDotTranslator::VisitConceptDecl" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("ConceptDecl");

     ROSE_ASSERT(FAIL_FIXME == 0); // FIXME

     return VisitTemplateDecl(concept_decl, node_desc) && res;
   }
#endif

#if 0
bool ClangToDotTranslator::VisitRedeclarableTemplateDecl(clang::RedeclarableTemplateDecl * redeclarable_template_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitRedeclarableTemplateDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME

    return VisitTemplateDecl(redeclarable_template_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitRedeclarableTemplateDecl(clang::RedeclarableTemplateDecl * redeclarable_template_decl, NodeDescriptor & node_desc)
   {
#if DEBUG_VISIT_DECL
     logger[DEBUG] << "ClangToDotTranslator::VisitRedeclarableTemplateDecl" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("RedeclarableTemplateDecl");

     ROSE_ASSERT(FAIL_FIXME == 0); // FIXME

     return VisitTemplateDecl(redeclarable_template_decl, node_desc) && res;
   }
#endif

#if 0
bool ClangToDotTranslator::VisitClassTemplateDecl(clang::ClassTemplateDecl * class_template_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitClassTemplateDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitRedeclarableTemplateDecl(class_template_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitClassTemplateDecl(clang::ClassTemplateDecl * class_template_decl, NodeDescriptor & node_desc)
   {
#if DEBUG_VISIT_DECL
     logger[DEBUG] << "ClangToDotTranslator::VisitClassTemplateDecl" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("ClassTemplateDecl");

     ROSE_ASSERT(FAIL_TODO == 0); // TODO

     return VisitRedeclarableTemplateDecl(class_template_decl, node_desc) && res;
   }
#endif

#if 0
bool ClangToDotTranslator::VisitFunctionTemplateDecl(clang::FunctionTemplateDecl * function_template_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitFunctionTemplateDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitRedeclarableTemplateDecl(function_template_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitFunctionTemplateDecl(clang::FunctionTemplateDecl * function_template_decl, NodeDescriptor & node_desc) 
   {
#if DEBUG_VISIT_DECL
     logger[DEBUG] << "ClangToDotTranslator::VisitFunctionTemplateDecl" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("FunctionTemplateDecl");

     ROSE_ASSERT(FAIL_TODO == 0); // TODO

     return VisitRedeclarableTemplateDecl(function_template_decl, node_desc) && res;
   }
#endif

#if 0
bool ClangToDotTranslator::VisitTypeAliasTemplateDecl(clang::TypeAliasTemplateDecl * type_alias_template_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitTypeAliasTemplateDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitRedeclarableTemplateDecl(type_alias_template_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitTypeAliasTemplateDecl(clang::TypeAliasTemplateDecl * type_alias_template_decl, NodeDescriptor & node_desc) 
   {
#if DEBUG_VISIT_DECL
     logger[DEBUG] << "ClangToDotTranslator::VisitTypeAliasTemplateDecl" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("TypeAliasTemplateDecl");

     ROSE_ASSERT(FAIL_TODO == 0); // TODO

     return VisitRedeclarableTemplateDecl(type_alias_template_decl, node_desc) && res;
   }
#endif

#if 0
bool ClangToDotTranslator::VisitVarTemplateDecl(clang::VarTemplateDecl * var_template_decl, SgNode ** node) {

#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitVarTemplateDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitRedeclarableTemplateDecl(var_template_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitVarTemplateDecl(clang::VarTemplateDecl * var_template_decl, NodeDescriptor & node_desc) {

#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitVarTemplateDecl" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("VarTemplateDecl");

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitRedeclarableTemplateDecl(var_template_decl, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitTemplateTemplateParmDecl(clang::TemplateTemplateParmDecl * template_template_parm_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitTemplateTemplateParmDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitTemplateDecl(template_template_parm_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitTemplateTemplateParmDecl(clang::TemplateTemplateParmDecl * template_template_parm_decl, NodeDescriptor & node_desc)
   {
#if DEBUG_VISIT_DECL
     logger[DEBUG] << "ClangToDotTranslator::VisitTemplateTemplateParmDecl" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("TemplateTemplateParmDecl");

     ROSE_ASSERT(FAIL_TODO == 0); // TODO

     return VisitTemplateDecl(template_template_parm_decl, node_desc) && res;
   }
#endif

#if 0
bool ClangToDotTranslator::VisitTypeDecl(clang::TypeDecl * type_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitTypeDecl" << "\n";
#endif

    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME

    return VisitNamedDecl(type_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitTypeDecl(clang::TypeDecl * type_decl, NodeDescriptor & node_desc) 
   {
#if DEBUG_VISIT_DECL
     logger[DEBUG] << "ClangToDotTranslator::VisitTypeDecl" << "\n";
#endif

     bool res = true;

  // Code copied from Tristan's dot file generator.
     node_desc.kind_hierarchy.push_back("TypeDecl");
#if CLANG_VERSION_MAJOR >= 22
     // (10/30/2026) lin32: Clang 22 creates tag/typedef types lazily; ASTContext replaces direct getTypeForDecl() access.
     clang::QualType type = type_decl->getASTContext().getTypeDeclType(
         clang::ElaboratedTypeKeyword::None, std::nullopt, type_decl);
     node_desc.successors.push_back(std::pair<std::string, std::string>("type_for_decl", Traverse(type.getTypePtr())));
#else
     node_desc.successors.push_back(std::pair<std::string, std::string>("type_for_decl", Traverse(type_decl->getTypeForDecl())));
#endif

     ROSE_ASSERT(FAIL_FIXME == 0); // FIXME

     return VisitNamedDecl(type_decl, node_desc) && res;
   }
#endif

#if 0
bool ClangToDotTranslator::VisitTagDecl(clang::TagDecl * tag_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitTagDecl" << "\n";
#endif

    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME

    return VisitTypeDecl(tag_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitTagDecl(clang::TagDecl * tag_decl, NodeDescriptor & node_desc) 
   {
#if DEBUG_VISIT_DECL
     logger[DEBUG] << "ClangToDotTranslator::VisitTagDecl" << "\n";
#endif

     bool res = true;

    clang::DeclContext* decl_context = static_cast<clang::DeclContext*>(tag_decl);
     node_desc.kind_hierarchy.push_back("TagDecl");

     node_desc.successors.push_back(std::pair<std::string, std::string>("canonical_decl", Traverse(tag_decl->getCanonicalDecl())));

     node_desc.successors.push_back(std::pair<std::string, std::string>("(TagDecl) definition", Traverse(tag_decl->getDefinition())));

     node_desc.attributes.push_back(std::pair<std::string, std::string>("kind", tag_decl->getKindName()));

     node_desc.successors.push_back(std::pair<std::string, std::string>("typedef_name_for_anon_decl", Traverse(tag_decl->getTypedefNameForAnonDecl())));

  // TODO NestedNameSpecifier * getQualifier () const 

     ROSE_ASSERT(FAIL_FIXME == 0); // FIXME

#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitTagDecl: casting to DeclContext" << "\n";
#endif
     unsigned cnt = 0;
    for (clang::Decl* tmpDecl : decl_context->decls()) {
#if DEBUG_VISIT_DECL
          logger[DEBUG] << "ClangToDotTranslator::VisitTagDecl: visit decl #" << cnt << " " << tmpDecl<< "\n";
#endif
          std::ostringstream oss;
          oss << "DeclContext::decls["<< cnt++ << "]";
          node_desc.successors.push_back(std::pair<std::string, std::string>(oss.str(), Traverse(tmpDecl)));
     }

#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitTagDecl:" << cnt << " decls visited" << "\n";
#endif
     return VisitTypeDecl(tag_decl, node_desc) && res;
   }
#endif

#if 0
bool ClangToDotTranslator::VisitRecordDecl(clang::RecordDecl * record_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitRecordDecl" << "\n";
#endif

    // FIXME May have to check the symbol table first, because of out-of-order traversal of C++ classes (Could be done in CxxRecord class...)

    bool res = true;

    SgClassDeclaration * sg_class_decl = NULL;

  // Find previous declaration

    clang::RecordDecl * prev_record_decl = record_decl->getPreviousDecl();
    SgClassSymbol * sg_prev_class_sym = isSgClassSymbol(GetSymbolFromSymbolTable(prev_record_decl));
    SgClassDeclaration * sg_prev_class_decl = sg_prev_class_sym == NULL ? NULL : isSgClassDeclaration(sg_prev_class_sym->get_declaration());

    SgClassDeclaration * sg_first_class_decl = sg_prev_class_decl == NULL ? NULL : isSgClassDeclaration(sg_prev_class_decl->get_firstNondefiningDeclaration());
    SgClassDeclaration * sg_def_class_decl = sg_prev_class_decl == NULL ? NULL : isSgClassDeclaration(sg_prev_class_decl->get_definingDeclaration());

    ROSE_ASSERT(sg_first_class_decl != NULL || sg_def_class_decl == NULL);

    bool had_prev_decl = sg_first_class_decl != NULL;

  // Name

    SgName name(record_decl->getNameAsString());

  // Type of class

    SgClassDeclaration::class_types type_of_class;
    switch (record_decl->getTagKind()) {
        case clang::TTK_Struct:
            type_of_class = SgClassDeclaration::e_struct;
            break;
        case clang::TTK_Class:
            type_of_class = SgClassDeclaration::e_class;
            break;
        case clang::TTK_Union:
            type_of_class = SgClassDeclaration::e_union;
            break;
        default:
            logger[WARN] << "Runtime error: RecordDecl can only be a struct/class/union." << "\n";
            res = false;
    }

  // Build declaration(s)

    sg_class_decl = new SgClassDeclaration(name, type_of_class, NULL, NULL);

    sg_class_decl->set_scope(SageBuilder::topScopeStack());
    sg_class_decl->set_parent(SageBuilder::topScopeStack());

    SgClassType * type = NULL;
    if (sg_first_class_decl != NULL) {
        type = sg_first_class_decl->get_type();
    }
    else {
        type = SgClassType::createType(sg_class_decl);
    }
    ROSE_ASSERT(type != NULL);
    sg_class_decl->set_type(type);

    if (record_decl->isAnonymousStructOrUnion()) sg_class_decl->set_isUnNamed(true);

    if (!had_prev_decl) {
        sg_first_class_decl = sg_class_decl;
        sg_first_class_decl->set_firstNondefiningDeclaration(sg_first_class_decl);
        sg_first_class_decl->set_definingDeclaration(NULL);
        sg_first_class_decl->set_definition(NULL);
        sg_first_class_decl->setForward();
        if (!record_decl->field_empty()) {
            sg_def_class_decl = new SgClassDeclaration(name, type_of_class, type, NULL);
            sg_def_class_decl->set_scope(SageBuilder::topScopeStack());
            if (record_decl->isAnonymousStructOrUnion()) sg_def_class_decl->set_isUnNamed(true);
            sg_def_class_decl->set_parent(SageBuilder::topScopeStack());

            sg_class_decl = sg_def_class_decl; // we return thew defining decl

            sg_def_class_decl->set_firstNondefiningDeclaration(sg_first_class_decl);
            sg_def_class_decl->set_definingDeclaration(sg_def_class_decl);

            sg_first_class_decl->set_definingDeclaration(sg_def_class_decl);
            setCompilerGeneratedFileInfo(sg_first_class_decl);
        }
    }
    else if (!record_decl->field_empty()) {
        if (sg_def_class_decl != NULL) {
            delete sg_class_decl;
            *node = sg_def_class_decl;
            return true;
        }
        sg_def_class_decl = sg_class_decl;
        sg_def_class_decl->set_firstNondefiningDeclaration(sg_first_class_decl);
        sg_def_class_decl->set_definingDeclaration(sg_def_class_decl);
        sg_first_class_decl->set_definingDeclaration(sg_def_class_decl);
    }
    else // second (or more) non-defining declaration
        return false; // FIXME ROSE need only one non-defining declaration (SageBuilder don't let me build another one....)

  // Update symbol table

    if (!had_prev_decl) {
        SgScopeStatement * scope = SageBuilder::topScopeStack();
        SgClassSymbol * class_symbol = new SgClassSymbol(sg_first_class_decl);
        scope->insert_symbol(name, class_symbol);
    }

  // Build ClassDefinition

    if (!record_decl->field_empty()) {
        SgClassDefinition * sg_class_def = isSgClassDefinition(sg_def_class_decl->get_definition());
        if (sg_class_def == NULL) {
            sg_class_def = SageBuilder::buildClassDefinition_nfi(sg_def_class_decl);
        }
        sg_def_class_decl->set_definition(sg_class_def);

        ROSE_ASSERT(sg_class_def->get_symbol_table() != NULL);

        applySourceRange(sg_class_def, record_decl->getSourceRange());

        SageBuilder::pushScopeStack(sg_class_def);

        clang::RecordDecl::field_iterator it;
        for (it = record_decl->field_begin(); it != record_decl->field_end(); it++) {
            SgNode * tmp_field = Traverse(*it);
            SgDeclarationStatement * field_decl = isSgDeclarationStatement(tmp_field);
            ROSE_ASSERT(field_decl != NULL);
            sg_class_def->append_member(field_decl);
            field_decl->set_parent(sg_class_def);
        }

        SageBuilder::popScopeStack();
    }

    ROSE_ASSERT(sg_class_decl->get_definingDeclaration() == NULL || isSgClassDeclaration(sg_class_decl->get_definingDeclaration())->get_definition() != NULL);
    ROSE_ASSERT(sg_first_class_decl->get_definition() == NULL);
    ROSE_ASSERT(sg_def_class_decl == NULL || sg_def_class_decl->get_definition() != NULL);

    *node = sg_class_decl;

    return VisitTagDecl(record_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitRecordDecl(clang::RecordDecl * record_decl, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitRecordDecl" << "\n";
    logger[DEBUG] << "ClangToDotTranslator::VisitRecordDecl name:" <<record_decl->getNameAsString() <<  "\n";
#endif

    // FIXME May have to check the symbol table first, because of out-of-order traversal of C++ classes (Could be done in CxxRecord class...)

    bool res = true;

#if 0
    SgClassDeclaration * sg_class_decl = NULL;

  // Find previous declaration

    clang::RecordDecl * prev_record_decl = record_decl->getPreviousDecl();
    SgClassSymbol * sg_prev_class_sym = isSgClassSymbol(GetSymbolFromSymbolTable(prev_record_decl));
    SgClassDeclaration * sg_prev_class_decl = sg_prev_class_sym == NULL ? NULL : isSgClassDeclaration(sg_prev_class_sym->get_declaration());

    SgClassDeclaration * sg_first_class_decl = sg_prev_class_decl == NULL ? NULL : isSgClassDeclaration(sg_prev_class_decl->get_firstNondefiningDeclaration());
    SgClassDeclaration * sg_def_class_decl = sg_prev_class_decl == NULL ? NULL : isSgClassDeclaration(sg_prev_class_decl->get_definingDeclaration());

    ROSE_ASSERT(sg_first_class_decl != NULL || sg_def_class_decl == NULL);

    bool had_prev_decl = sg_first_class_decl != NULL;

  // Name

    SgName name(record_decl->getNameAsString());

  // Type of class

    SgClassDeclaration::class_types type_of_class;
    switch (record_decl->getTagKind()) {
        case clang::TTK_Struct:
            type_of_class = SgClassDeclaration::e_struct;
            break;
        case clang::TTK_Class:
            type_of_class = SgClassDeclaration::e_class;
            break;
        case clang::TTK_Union:
            type_of_class = SgClassDeclaration::e_union;
            break;
        default:
            logger[WARN] << "Runtime error: RecordDecl can only be a struct/class/union." << "\n";
            res = false;
    }

  // Build declaration(s)

    sg_class_decl = new SgClassDeclaration(name, type_of_class, NULL, NULL);

    sg_class_decl->set_scope(SageBuilder::topScopeStack());
    sg_class_decl->set_parent(SageBuilder::topScopeStack());

    SgClassType * type = NULL;
    if (sg_first_class_decl != NULL) {
        type = sg_first_class_decl->get_type();
    }
    else {
        type = SgClassType::createType(sg_class_decl);
    }
    ROSE_ASSERT(type != NULL);
    sg_class_decl->set_type(type);

    if (record_decl->isAnonymousStructOrUnion()) sg_class_decl->set_isUnNamed(true);

    if (!had_prev_decl) {
        sg_first_class_decl = sg_class_decl;
        sg_first_class_decl->set_firstNondefiningDeclaration(sg_first_class_decl);
        sg_first_class_decl->set_definingDeclaration(NULL);
        sg_first_class_decl->set_definition(NULL);
        sg_first_class_decl->setForward();
        if (!record_decl->field_empty()) {
            sg_def_class_decl = new SgClassDeclaration(name, type_of_class, type, NULL);
            sg_def_class_decl->set_scope(SageBuilder::topScopeStack());
            if (record_decl->isAnonymousStructOrUnion()) sg_def_class_decl->set_isUnNamed(true);
            sg_def_class_decl->set_parent(SageBuilder::topScopeStack());

            sg_class_decl = sg_def_class_decl; // we return thew defining decl

            sg_def_class_decl->set_firstNondefiningDeclaration(sg_first_class_decl);
            sg_def_class_decl->set_definingDeclaration(sg_def_class_decl);

            sg_first_class_decl->set_definingDeclaration(sg_def_class_decl);
            setCompilerGeneratedFileInfo(sg_first_class_decl);
        }
    }
    else if (!record_decl->field_empty()) {
        if (sg_def_class_decl != NULL) {
            delete sg_class_decl;
            *node = sg_def_class_decl;
            return true;
        }
        sg_def_class_decl = sg_class_decl;
        sg_def_class_decl->set_firstNondefiningDeclaration(sg_first_class_decl);
        sg_def_class_decl->set_definingDeclaration(sg_def_class_decl);
        sg_first_class_decl->set_definingDeclaration(sg_def_class_decl);
    }
    else // second (or more) non-defining declaration
        return false; // FIXME ROSE need only one non-defining declaration (SageBuilder don't let me build another one....)

  // Update symbol table

    if (!had_prev_decl) {
        SgScopeStatement * scope = SageBuilder::topScopeStack();
        SgClassSymbol * class_symbol = new SgClassSymbol(sg_first_class_decl);
        scope->insert_symbol(name, class_symbol);
    }

  // Build ClassDefinition

    if (!record_decl->field_empty()) {
        SgClassDefinition * sg_class_def = isSgClassDefinition(sg_def_class_decl->get_definition());
        if (sg_class_def == NULL) {
            sg_class_def = SageBuilder::buildClassDefinition_nfi(sg_def_class_decl);
        }
        sg_def_class_decl->set_definition(sg_class_def);

        ROSE_ASSERT(sg_class_def->get_symbol_table() != NULL);

        applySourceRange(sg_class_def, record_decl->getSourceRange());

        SageBuilder::pushScopeStack(sg_class_def);

        clang::RecordDecl::field_iterator it;
        for (it = record_decl->field_begin(); it != record_decl->field_end(); it++) {
            SgNode * tmp_field = Traverse(*it);
            SgDeclarationStatement * field_decl = isSgDeclarationStatement(tmp_field);
            ROSE_ASSERT(field_decl != NULL);
            sg_class_def->append_member(field_decl);
            field_decl->set_parent(sg_class_def);
        }

        SageBuilder::popScopeStack();
    }

    ROSE_ASSERT(sg_class_decl->get_definingDeclaration() == NULL || isSgClassDeclaration(sg_class_decl->get_definingDeclaration())->get_definition() != NULL);
    ROSE_ASSERT(sg_first_class_decl->get_definition() == NULL);
    ROSE_ASSERT(sg_def_class_decl == NULL || sg_def_class_decl->get_definition() != NULL);

    *node = sg_class_decl;
#endif

     node_desc.kind_hierarchy.push_back("RecordDecl");

  // DQ (11/28/2020): Name change in Clang 10.
  // node_desc.successors.push_back(std::pair<std::string, std::string>("previous_declaration", Traverse(record_decl->getPreviousDeclaration())));

     clang::RecordDecl::field_iterator it;
     unsigned cnt = 0;
     for (it = record_decl->field_begin(); it != record_decl->field_end(); it++)
        {
          std::ostringstream oss;
          oss << "field[" << cnt++ << "]";
          node_desc.successors.push_back(std::pair<std::string, std::string>(oss.str(), Traverse(*it)));
        }
     node_desc.attributes.push_back(std::pair<std::string, std::string>("field_empty", record_decl->field_empty()?"True":"False"));

     return VisitTagDecl(record_decl, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitCXXRecordDecl(clang::CXXRecordDecl * cxx_record_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXRecordDecl" << "\n";
#endif
    bool res = VisitRecordDecl(cxx_record_decl, node);

    clang::CXXRecordDecl::base_class_iterator it_base;
    for (it_base = cxx_record_decl->bases_begin(); it_base !=  cxx_record_decl->bases_end(); it_base++) {
        // TODO add base classes
    }

    clang::CXXRecordDecl::method_iterator it_method;
    for (it_method = cxx_record_decl->method_begin(); it_method !=  cxx_record_decl->method_end(); it_method++) {
        // TODO
    }

    clang::CXXRecordDecl::ctor_iterator it_ctor;
    for (it_ctor = cxx_record_decl->ctor_begin(); it_ctor != cxx_record_decl->ctor_end(); it_ctor++) {
        // TODO if not tranversed as methods
    }

    clang::CXXRecordDecl::friend_iterator it_friend;
    for (it_friend = cxx_record_decl->friend_begin(); it_friend != cxx_record_decl->friend_end(); it_friend++) {
        // TODO
    }

    clang::CXXDestructorDecl * destructor = cxx_record_decl->getDestructor();
    // TODO

    return res;
}
#else
bool ClangToDotTranslator::VisitCXXRecordDecl(clang::CXXRecordDecl * cxx_record_decl, NodeDescriptor & node_desc) 
   {
#if DEBUG_VISIT_DECL
     logger[DEBUG] << "ClangToDotTranslator::VisitCXXRecordDecl" << "\n";
#endif
     bool res = VisitRecordDecl(cxx_record_decl, node_desc);

     node_desc.kind_hierarchy.push_back("CXXRecordDecl");

#if 0
  // DQ (11/28/2020): I think this skeleton is implemented as working code below.
     clang::CXXRecordDecl::base_class_iterator it_base;
     for (it_base = cxx_record_decl->bases_begin(); it_base !=  cxx_record_decl->bases_end(); it_base++) 
        {
       // TODO add base classes
        }

     clang::CXXRecordDecl::method_iterator it_method;
     for (it_method = cxx_record_decl->method_begin(); it_method !=  cxx_record_decl->method_end(); it_method++) 
        {
       // TODO
        }

     clang::CXXRecordDecl::ctor_iterator it_ctor;
     for (it_ctor = cxx_record_decl->ctor_begin(); it_ctor != cxx_record_decl->ctor_end(); it_ctor++) 
        {
       // TODO if not tranversed as methods
        }

     clang::CXXRecordDecl::friend_iterator it_friend;
     for (it_friend = cxx_record_decl->friend_begin(); it_friend != cxx_record_decl->friend_end(); it_friend++) 
        {
       // TODO
        }

     clang::CXXDestructorDecl * destructor = cxx_record_decl->getDestructor();
  // TODO
#endif
    
    node_desc.successors.push_back(std::pair<std::string, std::string>("(CXXRecordecl) definition", Traverse(cxx_record_decl->getDefinition())));

    // Pei-Hung: skipping the remaining processes when there is no definition found.  This should avoid issues when retriving bases, vbases, decls, ctors and friends.
    if(!cxx_record_decl->hasDefinition())
        return res;

    clang::CXXRecordDecl::base_class_iterator it_base;
    unsigned cnt = 0;
    for (it_base = cxx_record_decl->bases_begin(); it_base !=  cxx_record_decl->bases_end(); it_base++) {
        std::ostringstream oss;
        oss << "base_type[" << cnt++ << "]";
        switch (it_base->getAccessSpecifier()) {
            case clang::AS_public:
                oss << " (public)";
                break;
            case clang::AS_protected:
                oss << " (protected)";
                break;
            case clang::AS_private:
                oss << " (private)";
                break;
            case clang::AS_none:
                break;
        }
        node_desc.successors.push_back(std::pair<std::string, std::string>(oss.str(), Traverse(it_base->getType().getTypePtr())));
    }

    clang::CXXRecordDecl::base_class_iterator it_vbase;
    cnt = 0;
    for (it_vbase = cxx_record_decl->vbases_begin(); it_vbase !=  cxx_record_decl->vbases_end(); it_vbase++) {
        std::ostringstream oss;
        oss << "virtual_base_type[" << cnt++ << "]";
        switch (it_base->getAccessSpecifier()) {
            case clang::AS_public:
                oss << " (public)";
                break;
            case clang::AS_protected:
                oss << " (protected)";
                break;
            case clang::AS_private:
                oss << " (private)";
                break;
            case clang::AS_none:
                break;
        }
        node_desc.successors.push_back(std::pair<std::string, std::string>(oss.str(), Traverse(it_vbase->getType().getTypePtr())));
    }

    clang::CXXRecordDecl::method_iterator it_method;
    cnt = 0;
    for (it_method = cxx_record_decl->method_begin(); it_method !=  cxx_record_decl->method_end(); it_method++) {
        std::ostringstream oss;
        oss << "method[" << cnt++ << "]";
        node_desc.successors.push_back(std::pair<std::string, std::string>(oss.str(), Traverse(*it_method)));
    }

    clang::CXXRecordDecl::ctor_iterator it_ctor;
    cnt = 0;
    for (it_ctor = cxx_record_decl->ctor_begin(); it_ctor != cxx_record_decl->ctor_end(); it_ctor++) {
        std::ostringstream oss;
        oss << "constructor[" << cnt++ << "]";
        node_desc.successors.push_back(std::pair<std::string, std::string>(oss.str(), Traverse(*it_ctor)));
    }

    clang::CXXRecordDecl::friend_iterator it_friend;
    cnt = 0;
    for (it_friend = cxx_record_decl->friend_begin(); it_friend != cxx_record_decl->friend_end(); it_friend++) {
        std::ostringstream oss;
        oss << "friend[" << cnt++ << "]";
        node_desc.successors.push_back(std::pair<std::string, std::string>(oss.str(), Traverse(*it_friend)));
    }

    node_desc.successors.push_back(std::pair<std::string, std::string>("destructor", Traverse(cxx_record_decl->getDestructor())));

     return res;
   }
#endif


#if 0
bool ClangToDotTranslator::VisitClassTemplateSpecializationDecl(clang::ClassTemplateSpecializationDecl * class_tpl_spec_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitClassTemplateSpecializationDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitCXXRecordDecl(class_tpl_spec_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitClassTemplateSpecializationDecl(clang::ClassTemplateSpecializationDecl * class_tpl_spec_decl, NodeDescriptor & node_desc) 
   {
#if DEBUG_VISIT_DECL
     logger[DEBUG] << "ClangToDotTranslator::VisitClassTemplateSpecializationDecl" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("ClassTemplateSpecializationDecl");

     ROSE_ASSERT(FAIL_TODO == 0); // TODO

     return VisitCXXRecordDecl(class_tpl_spec_decl, node_desc) && res;
   }
#endif

#if 0
bool ClangToDotTranslator::VisitClassTemplatePartialSpecializationDecl(clang::ClassTemplatePartialSpecializationDecl * class_tpl_part_spec_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitClassTemplatePartialSpecializationDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitClassTemplateSpecializationDecl(class_tpl_part_spec_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitClassTemplatePartialSpecializationDecl(clang::ClassTemplatePartialSpecializationDecl * class_tpl_part_spec_decl, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitClassTemplatePartialSpecializationDecl" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("ClassTemplatePartialSpecializationDecl");

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitClassTemplateSpecializationDecl(class_tpl_part_spec_decl, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitEnumDecl(clang::EnumDecl * enum_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitEnumDecl" << "\n";
#endif
    bool res = true;

    SgName name(enum_decl->getNameAsString());


    clang::EnumDecl * prev_enum_decl = enum_decl->getPreviousDecl();
    SgEnumSymbol * sg_prev_enum_sym = isSgEnumSymbol(GetSymbolFromSymbolTable(prev_enum_decl));
    SgEnumDeclaration * sg_prev_enum_decl = sg_prev_enum_sym == NULL ? NULL : isSgEnumDeclaration(sg_prev_enum_sym->get_declaration());
    sg_prev_enum_decl = sg_prev_enum_decl == NULL ? NULL : isSgEnumDeclaration(sg_prev_enum_decl->get_definingDeclaration());

    SgEnumDeclaration * sg_enum_decl = SageBuilder::buildEnumDeclaration(name, SageBuilder::topScopeStack());
    *node = sg_enum_decl;

    if (sg_prev_enum_decl == NULL || sg_prev_enum_decl->get_enumerators().size() == 0) {
      clang::EnumDecl::enumerator_iterator it;
      for (it = enum_decl->enumerator_begin(); it != enum_decl->enumerator_end(); it++) {
          SgNode * tmp_enumerator = Traverse(*it);
          SgInitializedName * enumerator = isSgInitializedName(tmp_enumerator);

          ROSE_ASSERT(enumerator);

          enumerator->set_scope(SageBuilder::topScopeStack());
          sg_enum_decl->append_enumerator(enumerator);
      }
    }
    else {
      sg_enum_decl->set_definingDeclaration(sg_prev_enum_decl);
      sg_enum_decl->set_firstNondefiningDeclaration(sg_prev_enum_decl->get_firstNondefiningDeclaration());
    }

    return VisitDecl(enum_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitEnumDecl(clang::EnumDecl * enum_decl, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitEnumDecl" << "\n";
#endif
    bool res = true;

#if 0
    SgName name(enum_decl->getNameAsString());

    clang::EnumDecl * prev_enum_decl = enum_decl->getPreviousDecl();
    SgEnumSymbol * sg_prev_enum_sym = isSgEnumSymbol(GetSymbolFromSymbolTable(prev_enum_decl));
    SgEnumDeclaration * sg_prev_enum_decl = sg_prev_enum_sym == NULL ? NULL : isSgEnumDeclaration(sg_prev_enum_sym->get_declaration());
    sg_prev_enum_decl = sg_prev_enum_decl == NULL ? NULL : isSgEnumDeclaration(sg_prev_enum_decl->get_definingDeclaration());

    SgEnumDeclaration * sg_enum_decl = SageBuilder::buildEnumDeclaration(name, SageBuilder::topScopeStack());
    *node = sg_enum_decl;

    if (sg_prev_enum_decl == NULL || sg_prev_enum_decl->get_enumerators().size() == 0) {
      clang::EnumDecl::enumerator_iterator it;
      for (it = enum_decl->enumerator_begin(); it != enum_decl->enumerator_end(); it++) {
          SgNode * tmp_enumerator = Traverse(*it);
          SgInitializedName * enumerator = isSgInitializedName(tmp_enumerator);

          ROSE_ASSERT(enumerator);

          enumerator->set_scope(SageBuilder::topScopeStack());
          sg_enum_decl->append_enumerator(enumerator);
      }
    }
    else {
      sg_enum_decl->set_definingDeclaration(sg_prev_enum_decl);
      sg_enum_decl->set_firstNondefiningDeclaration(sg_prev_enum_decl->get_firstNondefiningDeclaration());
    }
#endif

     node_desc.kind_hierarchy.push_back("EnumDecl");

     node_desc.kind_hierarchy.push_back("EnumDecl");

  // node_desc.successors.push_back(std::pair<std::string, std::string>("previous_declaration", Traverse(enum_decl->getPreviousDeclaration())));
     node_desc.successors.push_back(std::pair<std::string, std::string>("previous_declaration", Traverse(enum_decl->getPreviousDecl())));

     clang::EnumDecl::enumerator_iterator it;
     unsigned cnt = 0;
     for (it = enum_decl->enumerator_begin(); it != enum_decl->enumerator_end(); it++)
        {
          std::ostringstream oss;
          oss << "enumerator[" << cnt++ << "]";
          node_desc.successors.push_back(std::pair<std::string, std::string>(oss.str(), Traverse(*it)));
        }

     node_desc.successors.push_back(std::pair<std::string, std::string>("promotion_type", Traverse(enum_decl->getPromotionType().getTypePtr())));

     return VisitTagDecl(enum_decl, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitTemplateTypeParmDecl(clang::TemplateTypeParmDecl * template_type_parm_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitTemplateTypeParmDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitTypeDecl(template_type_parm_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitTemplateTypeParmDecl(clang::TemplateTypeParmDecl * template_type_parm_decl, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitTemplateTypeParmDecl" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("TemplateTypeParmDecl");

    // (10/30/2026) lin32: Clang 19 returns TemplateArgumentLoc; unwrap its type argument before DOT traversal.
    if (template_type_parm_decl->hasDefaultArgument())
        node_desc.successors.push_back(
#if CLANG_VERSION_MAJOR >= 19
            std::pair<std::string, std::string>("default_argument", Traverse(template_type_parm_decl->getDefaultArgument().getArgument().getAsType().getTypePtr()))
#else
            std::pair<std::string, std::string>("default_argument", Traverse(template_type_parm_decl->getDefaultArgument().getTypePtr()))
#endif
        );

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitTypeDecl(template_type_parm_decl, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitTypedefNameDecl(clang::TypedefNameDecl * typedef_name_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitTypedefNameDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME

    return VisitTypeDecl(typedef_name_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitTypedefNameDecl(clang::TypedefNameDecl * typedef_name_decl, NodeDescriptor & node_desc)
   {
#if DEBUG_VISIT_DECL
     logger[DEBUG] << "ClangToDotTranslator::VisitTypedefNameDecl" << "\n";
#endif
     bool res = true;

  // Code copied from Tristan's dot file generator.
     node_desc.kind_hierarchy.push_back("TypedefNameDecl");
     node_desc.successors.push_back(std::pair<std::string, std::string>("underlying_type", Traverse(typedef_name_decl->getUnderlyingType().getTypePtr())));

  // node_desc.kind_hierarchy.push_back("TypedefNameDecl");

     ROSE_ASSERT(FAIL_FIXME == 0); // FIXME

     return VisitTypeDecl(typedef_name_decl, node_desc) && res;
   }
#endif

#if 0
bool ClangToDotTranslator::VisitTypedefDecl(clang::TypedefDecl * typedef_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitTypedefDecl" << "\n";
#endif
    bool res = true;

    SgName name(typedef_decl->getNameAsString());

    SgType * type = buildTypeFromQualifiedType(typedef_decl->getUnderlyingType());

    SgTypedefDeclaration * sg_typedef_decl = SageBuilder::buildTypedefDeclaration_nfi(name, type, SageBuilder::topScopeStack());

    if (isSgClassType(type)) {
        std::map<SgClassType *, bool>::iterator bool_it = p_class_type_decl_first_see_in_type.find(isSgClassType(type));
        ROSE_ASSERT(bool_it != p_class_type_decl_first_see_in_type.end());
        if (bool_it->second) {
            sg_typedef_decl->set_declaration(isSgNamedType(type)->get_declaration()->get_definingDeclaration());
            sg_typedef_decl->set_typedefBaseTypeContainsDefiningDeclaration(true);
        }
    }
    else if (isSgEnumType(type)) {
        std::map<SgEnumType *, bool>::iterator bool_it = p_enum_type_decl_first_see_in_type.find(isSgEnumType(type));
        ROSE_ASSERT(bool_it != p_enum_type_decl_first_see_in_type.end());
        if (bool_it->second) {
            sg_typedef_decl->set_declaration(isSgEnumType(type)->get_declaration()->get_definingDeclaration());
            sg_typedef_decl->set_typedefBaseTypeContainsDefiningDeclaration(true);
        }
    }

    *node = sg_typedef_decl;

    return VisitTypedefNameDecl(typedef_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitTypedefDecl(clang::TypedefDecl * typedef_decl, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitTypedefDecl" << "\n";
#endif
    bool res = true;

#if 0
    SgName name(typedef_decl->getNameAsString());

    SgType * type = buildTypeFromQualifiedType(typedef_decl->getUnderlyingType());

    SgTypedefDeclaration * sg_typedef_decl = SageBuilder::buildTypedefDeclaration_nfi(name, type, SageBuilder::topScopeStack());

    if (isSgClassType(type)) {
        std::map<SgClassType *, bool>::iterator bool_it = p_class_type_decl_first_see_in_type.find(isSgClassType(type));
        ROSE_ASSERT(bool_it != p_class_type_decl_first_see_in_type.end());
        if (bool_it->second) {
            sg_typedef_decl->set_declaration(isSgNamedType(type)->get_declaration()->get_definingDeclaration());
            sg_typedef_decl->set_typedefBaseTypeContainsDefiningDeclaration(true);
        }
    }
    else if (isSgEnumType(type)) {
        std::map<SgEnumType *, bool>::iterator bool_it = p_enum_type_decl_first_see_in_type.find(isSgEnumType(type));
        ROSE_ASSERT(bool_it != p_enum_type_decl_first_see_in_type.end());
        if (bool_it->second) {
            sg_typedef_decl->set_declaration(isSgEnumType(type)->get_declaration()->get_definingDeclaration());
            sg_typedef_decl->set_typedefBaseTypeContainsDefiningDeclaration(true);
        }
    }

    *node = sg_typedef_decl;
#endif

 // Code copied from Tristan's dot file generator.
    node_desc.kind_hierarchy.push_back("TypedefDecl");

    return VisitTypedefNameDecl(typedef_decl, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitTypeAliasDecl(clang::TypeAliasDecl * type_alias_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitTypeAliasDecl" << "\n";
#endif  
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitTypedefNameDecl(type_alias_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitTypeAliasDecl(clang::TypeAliasDecl * type_alias_decl, NodeDescriptor & node_desc) 
   {
#if DEBUG_VISIT_DECL
     logger[DEBUG] << "ClangToDotTranslator::VisitTypeAliasDecl" << "\n";
#endif  
     bool res = true;

     node_desc.kind_hierarchy.push_back("TypeAliasDecl");

     ROSE_ASSERT(FAIL_TODO == 0); // TODO

     return VisitTypedefNameDecl(type_alias_decl, node_desc) && res;
   }
#endif

#if 0
bool ClangToDotTranslator::VisitUnresolvedUsingTypenameDecl(clang::UnresolvedUsingTypenameDecl * unresolved_using_type_name_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitUnresolvedUsingTypenameDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME

    return VisitTypeDecl(unresolved_using_type_name_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitUnresolvedUsingTypenameDecl(clang::UnresolvedUsingTypenameDecl * unresolved_using_type_name_decl, NodeDescriptor & node_desc) 
   {
#if DEBUG_VISIT_DECL
     logger[DEBUG] << "ClangToDotTranslator::VisitUnresolvedUsingTypenameDecl" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("UnresolvedUsingTypenameDecl");

     ROSE_ASSERT(FAIL_FIXME == 0); // FIXME

     return VisitTypeDecl(unresolved_using_type_name_decl, node_desc) && res;
   }
#endif

#if 0
bool ClangToDotTranslator::VisitUsingDecl(clang::UsingDecl * using_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitUsingDecl" << "\n";
#endif  
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME

    return VisitNamedDecl(using_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitUsingDecl(clang::UsingDecl * using_decl, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitUsingDecl" << "\n";
#endif  
    bool res = true;

     node_desc.kind_hierarchy.push_back("UsingDecl");

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME

    return VisitNamedDecl(using_decl, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitUsingDirectiveDecl(clang::UsingDirectiveDecl * using_directive_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitUsingDirectiveDecl" << "\n";
#endif  
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME

    return VisitNamedDecl(using_directive_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitUsingDirectiveDecl(clang::UsingDirectiveDecl * using_directive_decl, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitUsingDirectiveDecl" << "\n";
#endif  
    bool res = true;

     node_desc.kind_hierarchy.push_back("UsingDirectiveDecl");

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME

    return VisitNamedDecl(using_directive_decl, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitUsingPackDecl(clang::UsingPackDecl * using_pack_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitUsingPackDecl" << "\n";
#endif  
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME

    return VisitNamedDecl(using_pack_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitUsingPackDecl(clang::UsingPackDecl * using_pack_decl, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitUsingPackDecl" << "\n";
#endif  
    bool res = true;

     node_desc.kind_hierarchy.push_back("UsingPackDecl");

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME

    return VisitNamedDecl(using_pack_decl, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitUsingShadowDecl(clang::UsingShadowDecl * using_shadow_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitUsingShadowDecl" << "\n";
#endif  
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME

    return VisitNamedDecl(using_shadow_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitUsingShadowDecl(clang::UsingShadowDecl * using_shadow_decl, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitUsingShadowDecl" << "\n";
#endif  
    bool res = true;

     node_desc.kind_hierarchy.push_back("UsingShadowDecl");

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME

    return VisitNamedDecl(using_shadow_decl, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitConstructorUsingShadowDecl(clang::ConstructorUsingShadowDecl * constructor_using_shadow_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitConstructorUsingShadowDecl" << "\n";
#endif  
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME

    return VisitNamedDecl(constructor_using_shadow_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitConstructorUsingShadowDecl(clang::ConstructorUsingShadowDecl * constructor_using_shadow_decl, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitConstructorUsingShadowDecl" << "\n";
#endif  
    bool res = true;

     node_desc.kind_hierarchy.push_back("ConstructorUsingShadowDecl");

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME

    return VisitNamedDecl(constructor_using_shadow_decl, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitValueDecl(clang::ValueDecl * value_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitValueDecl" << "\n";
#endif  
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME

    return VisitNamedDecl(value_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitValueDecl(clang::ValueDecl * value_decl, NodeDescriptor & node_desc) 
   {
#if DEBUG_VISIT_DECL
     logger[DEBUG] << "ClangToDotTranslator::VisitValueDecl" << "\n";
#endif  
     bool res = true;

     node_desc.kind_hierarchy.push_back("ValueDecl");

     node_desc.successors.push_back(std::pair<std::string, std::string>("type", Traverse(value_decl->getType().getTypePtr())));

     ROSE_ASSERT(FAIL_FIXME == 0); // FIXME

     return VisitNamedDecl(value_decl, node_desc) && res;
   }
#endif

#if 0
bool ClangToDotTranslator::VisitBindingDecl(clang::BindingDecl * binding_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitBindingDecl" << "\n";
#endif  
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME

    return VisitValueDecl(binding_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitBindingDecl(clang::BindingDecl * binding_decl, NodeDescriptor & node_desc)
   {
#if DEBUG_VISIT_DECL
     logger[DEBUG] << "ClangToDotTranslator::VisitBindingDecl" << "\n";
#endif  
     bool res = true;

     node_desc.kind_hierarchy.push_back("BindingDecl");

     ROSE_ASSERT(FAIL_FIXME == 0); // FIXME

     return VisitValueDecl(binding_decl, node_desc) && res;
   }
#endif

#if 0    
bool ClangToDotTranslator::VisitDeclaratorDecl(clang::DeclaratorDecl * declarator_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitDeclaratorDecl" << "\n";
#endif  
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME

    return VisitValueDecl(declarator_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitDeclaratorDecl(clang::DeclaratorDecl * declarator_decl, NodeDescriptor & node_desc)
   {
#if DEBUG_VISIT_DECL
     logger[DEBUG] << "ClangToDotTranslator::VisitDeclaratorDecl" << "\n";
#endif  
     bool res = true;

     node_desc.kind_hierarchy.push_back("DeclaratorDecl");

     ROSE_ASSERT(FAIL_FIXME == 0); // FIXME

     return VisitValueDecl(declarator_decl, node_desc) && res;
   }
#endif

#if 0
bool ClangToDotTranslator::VisitFieldDecl(clang::FieldDecl * field_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitFieldDecl" << "\n";
#endif  
    bool res = true;
    
    SgName name(field_decl->getNameAsString());

    SgType * type = buildTypeFromQualifiedType(field_decl->getType());

    clang::Expr * init_expr = field_decl->getInClassInitializer();
    SgNode * tmp_init = Traverse(init_expr);
    SgExpression * expr = isSgExpression(tmp_init);
    // TODO expression list if aggregated initializer !
    if (tmp_init != NULL && expr == NULL) {
        logger[WARN] << "Runtime error: not a SgInitializer..." << "\n";
        res = false;
    }
    SgInitializer * init = expr != NULL ? SageBuilder::buildAssignInitializer_nfi(expr, expr->get_type()) : NULL;
    if (init != NULL)
        applySourceRange(init, init_expr->getSourceRange());

  // Cannot use 'SageBuilder::buildVariableDeclaration' because of anonymous field
    // *node = SageBuilder::buildVariableDeclaration(name, type, init, SageBuilder::topScopeStack());
  // Build it by hand...
    SgVariableDeclaration * var_decl = new SgVariableDeclaration(name, type, init);

    if (isSgClassType(type)) {
        std::map<SgClassType *, bool>::iterator bool_it = p_class_type_decl_first_see_in_type.find(isSgClassType(type));
        ROSE_ASSERT(bool_it != p_class_type_decl_first_see_in_type.end());
        if (bool_it->second) {
            var_decl->set_baseTypeDefiningDeclaration(isSgNamedType(type)->get_declaration()->get_definingDeclaration());
            var_decl->set_variableDeclarationContainsBaseTypeDefiningDeclaration(true);
        }
    }
    else if (isSgEnumType(type)) {
        std::map<SgEnumType *, bool>::iterator bool_it = p_enum_type_decl_first_see_in_type.find(isSgEnumType(type));
        ROSE_ASSERT(bool_it != p_enum_type_decl_first_see_in_type.end());
        if (bool_it->second) {
            var_decl->set_baseTypeDefiningDeclaration(isSgEnumType(type)->get_declaration()->get_definingDeclaration());
            var_decl->set_variableDeclarationContainsBaseTypeDefiningDeclaration(true);
        }
    }

    var_decl->set_firstNondefiningDeclaration(var_decl);
    var_decl->set_parent(SageBuilder::topScopeStack());

    ROSE_ASSERT(var_decl->get_variables().size() == 1);

    SgInitializedName * init_name = var_decl->get_variables()[0];
    ROSE_ASSERT(init_name != NULL);
    init_name->set_scope(SageBuilder::topScopeStack());

    applySourceRange(init_name, field_decl->getSourceRange());

    SgVariableDefinition * var_def = isSgVariableDefinition(init_name->get_declptr());
    ROSE_ASSERT(var_def != NULL);
    applySourceRange(var_def, field_decl->getSourceRange());

    SgVariableSymbol * var_symbol = new SgVariableSymbol(init_name);
    SageBuilder::topScopeStack()->insert_symbol(name, var_symbol);

    *node = var_decl;

    return VisitDeclaratorDecl(field_decl, node) && res; 
}
#else
bool ClangToDotTranslator::VisitFieldDecl(clang::FieldDecl * field_decl, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitFieldDecl" << "\n";
#endif  
    bool res = true;

#if 0
    SgName name(field_decl->getNameAsString());

    SgType * type = buildTypeFromQualifiedType(field_decl->getType());

    clang::Expr * init_expr = field_decl->getInClassInitializer();
    SgNode * tmp_init = Traverse(init_expr);
    SgExpression * expr = isSgExpression(tmp_init);
    // TODO expression list if aggregated initializer !
    if (tmp_init != NULL && expr == NULL) {
        logger[WARN] << "Runtime error: not a SgInitializer..." << "\n";
        res = false;
    }
    SgInitializer * init = expr != NULL ? SageBuilder::buildAssignInitializer_nfi(expr, expr->get_type()) : NULL;
    if (init != NULL)
        applySourceRange(init, init_expr->getSourceRange());

  // Cannot use 'SageBuilder::buildVariableDeclaration' because of anonymous field
    // *node = SageBuilder::buildVariableDeclaration(name, type, init, SageBuilder::topScopeStack());
  // Build it by hand...
    SgVariableDeclaration * var_decl = new SgVariableDeclaration(name, type, init);

    if (isSgClassType(type)) {
        std::map<SgClassType *, bool>::iterator bool_it = p_class_type_decl_first_see_in_type.find(isSgClassType(type));
        ROSE_ASSERT(bool_it != p_class_type_decl_first_see_in_type.end());
        if (bool_it->second) {
            var_decl->set_baseTypeDefiningDeclaration(isSgNamedType(type)->get_declaration()->get_definingDeclaration());
            var_decl->set_variableDeclarationContainsBaseTypeDefiningDeclaration(true);
        }
    }
    else if (isSgEnumType(type)) {
        std::map<SgEnumType *, bool>::iterator bool_it = p_enum_type_decl_first_see_in_type.find(isSgEnumType(type));
        ROSE_ASSERT(bool_it != p_enum_type_decl_first_see_in_type.end());
        if (bool_it->second) {
            var_decl->set_baseTypeDefiningDeclaration(isSgEnumType(type)->get_declaration()->get_definingDeclaration());
            var_decl->set_variableDeclarationContainsBaseTypeDefiningDeclaration(true);
        }
    }

    var_decl->set_firstNondefiningDeclaration(var_decl);
    var_decl->set_parent(SageBuilder::topScopeStack());

    ROSE_ASSERT(var_decl->get_variables().size() == 1);

    SgInitializedName * init_name = var_decl->get_variables()[0];
    ROSE_ASSERT(init_name != NULL);
    init_name->set_scope(SageBuilder::topScopeStack());

    applySourceRange(init_name, field_decl->getSourceRange());

    SgVariableDefinition * var_def = isSgVariableDefinition(init_name->get_declptr());
    ROSE_ASSERT(var_def != NULL);
    applySourceRange(var_def, field_decl->getSourceRange());

    SgVariableSymbol * var_symbol = new SgVariableSymbol(init_name);
    SageBuilder::topScopeStack()->insert_symbol(name, var_symbol);

    *node = var_decl;
#endif

     node_desc.kind_hierarchy.push_back("FieldDecl");

    node_desc.successors.push_back(std::pair<std::string, std::string>("in_class_initializer", Traverse(field_decl->getInClassInitializer())));

    node_desc.successors.push_back(std::pair<std::string, std::string>("bit_width", Traverse(field_decl->getBitWidth())));

    node_desc.successors.push_back(std::pair<std::string, std::string>("parent", Traverse(field_decl->getParent())));

    return VisitDeclaratorDecl(field_decl, node_desc) && res; 
}
#endif

#if 0
bool ClangToDotTranslator::VisitFunctionDecl(clang::FunctionDecl * function_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitFunctionDecl" << "\n";
#endif
    bool res = true;

    // FIXME: There is something weird here when try to Traverse a function reference in a recursive function (when first Traverse is not complete)
    //        It seems that it tries to instantiate the decl inside the function...
    //        It may be faster to recode from scratch...
    //   If I am not wrong this have been fixed....

    SgName name(function_decl->getNameAsString());

    SgType * ret_type = buildTypeFromQualifiedType(function_decl->getReturnType());

    SgFunctionParameterList * param_list = SageBuilder::buildFunctionParameterList_nfi();
      applySourceRange(param_list, function_decl->getSourceRange()); // FIXME find the good SourceRange (should be stored by Clang...)

    for (unsigned i = 0; i < function_decl->getNumParams(); i++) {
        SgNode * tmp_init_name = Traverse(function_decl->getParamDecl(i));
        SgInitializedName * init_name = isSgInitializedName(tmp_init_name);
        if (tmp_init_name != NULL && init_name == NULL) {
            logger[WARN] << "Runtime error: tmp_init_name != NULL && init_name == NULL" << "\n";
            res = false;
            continue;
        }

        param_list->append_arg(init_name);
    }

    if (function_decl->isVariadic()) {
        SgName empty = "";
        SgType * ellipses_type = SgTypeEllipse::createType();
        param_list->append_arg(SageBuilder::buildInitializedName_nfi(empty, ellipses_type, NULL));
    }

    SgFunctionDeclaration * sg_function_decl;

    if (function_decl->isThisDeclarationADefinition()) {
        sg_function_decl = SageBuilder::buildDefiningFunctionDeclaration(name, ret_type, param_list, NULL);
        sg_function_decl->set_definingDeclaration(sg_function_decl);

        if (function_decl->isVariadic()) {
            sg_function_decl->hasEllipses();
        }

        if (!function_decl->hasBody()) {
            logger[DEBUG] << "Defining function declaration without body..." << "\n";
            res = false;
        }
/*
        if (sg_function_decl->get_definition() != NULL) SageInterface::deleteAST(sg_function_decl->get_definition());

        SgFunctionDefinition * function_definition = new SgFunctionDefinition(sg_function_decl, NULL);

        SgInitializedNamePtrList & init_names = param_list->get_args();
        SgInitializedNamePtrList::iterator it;
        for (it = init_names.begin(); it != init_names.end(); it++) {
            (*it)->set_scope(function_definition);
            SgSymbolTable * st = function_definition->get_symbol_table();
            ROSE_ASSERT(st != NULL);
            SgVariableSymbol * tmp_sym  = new SgVariableSymbol(*it);
            st->insert((*it)->get_name(), tmp_sym);
        }
*/
        SgFunctionDefinition * function_definition = sg_function_decl->get_definition();

        if (sg_function_decl->get_definition()->get_body() != NULL)
            SageInterface::deleteAST(sg_function_decl->get_definition()->get_body());

        SageBuilder::pushScopeStack(function_definition);

        SgNode * tmp_body = Traverse(function_decl->getBody());
        SgBasicBlock * body = isSgBasicBlock(tmp_body);

        SageBuilder::popScopeStack();

        if (tmp_body != NULL && body == NULL) {
            logger[WARN] << "Runtime error: tmp_body != NULL && body == NULL" << "\n";
            res = false;
        }
        else {
            function_definition->set_body(body);
            body->set_parent(function_definition);
            applySourceRange(function_definition, function_decl->getSourceRange());
        }

        sg_function_decl->set_definition(function_definition);
        function_definition->set_parent(sg_function_decl);

        SgFunctionDeclaration * first_decl;
        if (function_decl->getFirstDecl() == function_decl) {
            SgFunctionParameterList * param_list_ = SageBuilder::buildFunctionParameterList_nfi();
              setCompilerGeneratedFileInfo(param_list_);
            SgInitializedNamePtrList & init_names = param_list->get_args();
            SgInitializedNamePtrList::iterator it;
            for (it = init_names.begin(); it != init_names.end(); it++) {
                SgInitializedName * init_param = new SgInitializedName(**it);
                setCompilerGeneratedFileInfo(init_param);
                param_list_->append_arg(init_param);
            }

            first_decl = SageBuilder::buildNondefiningFunctionDeclaration(name, ret_type, param_list_, NULL);
            setCompilerGeneratedFileInfo(first_decl);
            first_decl->set_parent(SageBuilder::topScopeStack());
            first_decl->set_firstNondefiningDeclaration(first_decl);
            if (function_decl->isVariadic()) first_decl->hasEllipses();
        }
        else {
            SgSymbol * tmp_symbol = GetSymbolFromSymbolTable(function_decl->getFirstDecl());
            SgFunctionSymbol * symbol = isSgFunctionSymbol(tmp_symbol);
            if (tmp_symbol != NULL && symbol == NULL) {
                logger[WARN] << "Runtime error: tmp_symbol != NULL && symbol == NULL" << "\n";
                res = false;
            }
            if (symbol != NULL)
                first_decl = isSgFunctionDeclaration(symbol->get_declaration());
        }

        sg_function_decl->set_firstNondefiningDeclaration(first_decl);
        first_decl->set_definingDeclaration(sg_function_decl);
    }
    else {
        sg_function_decl = SageBuilder::buildNondefiningFunctionDeclaration(name, ret_type, param_list, NULL);

        if (function_decl->isVariadic()) sg_function_decl->hasEllipses();

        SgInitializedNamePtrList & init_names = param_list->get_args();
        SgInitializedNamePtrList::iterator it;
        for (it = init_names.begin(); it != init_names.end(); it++) {
             (*it)->set_scope(SageBuilder::topScopeStack());
        }

        if (function_decl->getFirstDecl() != function_decl) {
            SgSymbol * tmp_symbol = GetSymbolFromSymbolTable(function_decl->getFirstDecl());
            SgFunctionSymbol * symbol = isSgFunctionSymbol(tmp_symbol);
            if (tmp_symbol != NULL && symbol == NULL) {
                logger[WARN] << "Runtime error: tmp_symbol != NULL && symbol == NULL" << "\n";
                res = false;
            }
            SgFunctionDeclaration * first_decl = NULL;
            if (symbol != NULL) {
                first_decl = isSgFunctionDeclaration(symbol->get_declaration());
            }
            else {
                // FIXME Is it correct?
                SgNode * tmp_first_decl = Traverse(function_decl->getFirstDecl());
                first_decl = isSgFunctionDeclaration(tmp_first_decl);
                ROSE_ASSERT(first_decl != NULL);
                // ROSE_ASSERT(!"We should have see the first declaration already");
            }

            if (first_decl != NULL) {
                if (first_decl->get_firstNondefiningDeclaration() != NULL)
                    sg_function_decl->set_firstNondefiningDeclaration(first_decl->get_firstNondefiningDeclaration());
                else {
                    ROSE_ASSERT(first_decl->get_firstNondefiningDeclaration() != NULL);
                }
            }
            else {
                ROSE_ASSERT(!"First declaration not found!");
            }
        }
        else {
            sg_function_decl->set_firstNondefiningDeclaration(sg_function_decl);
        }
    }

    ROSE_ASSERT(sg_function_decl->get_firstNondefiningDeclaration() != NULL);
/* // TODO Fix problem with function symbols...
    SgSymbol * symbol = GetSymbolFromSymbolTable(function_decl);
    if (symbol == NULL) {
        SgFunctionSymbol * func_sym = new SgFunctionSymbol(isSgFunctionDeclaration(sg_function_decl->get_firstNondefiningDeclaration()));
        SageBuilder::topScopeStack()->insert_symbol(name, func_sym);        
    }
*/
//  ROSE_ASSERT(GetSymbolFromSymbolTable(function_decl) != NULL);

    *node = sg_function_decl;

    return VisitDeclaratorDecl(function_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitFunctionDecl(clang::FunctionDecl * function_decl, NodeDescriptor & node_desc) 
   {
#if DEBUG_VISIT_DECL
     logger[DEBUG] << "ClangToDotTranslator::VisitFunctionDecl" << "\n";
#endif
     bool res = true;

    // FIXME: There is something weird here when try to Traverse a function reference in a recursive function (when first Traverse is not complete)
    //        It seems that it tries to instantiate the decl inside the function...
    //        It may be faster to recode from scratch...
    //   If I am not wrong this have been fixed....

#if 0
    SgName name(function_decl->getNameAsString());

    SgType * ret_type = buildTypeFromQualifiedType(function_decl->getReturnType());

    SgFunctionParameterList * param_list = SageBuilder::buildFunctionParameterList_nfi();
      applySourceRange(param_list, function_decl->getSourceRange()); // FIXME find the good SourceRange (should be stored by Clang...)

    for (unsigned i = 0; i < function_decl->getNumParams(); i++) {
        SgNode * tmp_init_name = Traverse(function_decl->getParamDecl(i));
        SgInitializedName * init_name = isSgInitializedName(tmp_init_name);
        if (tmp_init_name != NULL && init_name == NULL) {
            logger[WARN] << "Runtime error: tmp_init_name != NULL && init_name == NULL" << "\n";
            res = false;
            continue;
        }

        param_list->append_arg(init_name);
    }

    if (function_decl->isVariadic()) {
        SgName empty = "";
        SgType * ellipses_type = SgTypeEllipse::createType();
        param_list->append_arg(SageBuilder::buildInitializedName_nfi(empty, ellipses_type, NULL));
    }

    SgFunctionDeclaration * sg_function_decl;

    if (function_decl->isThisDeclarationADefinition()) {
        sg_function_decl = SageBuilder::buildDefiningFunctionDeclaration(name, ret_type, param_list, NULL);
        sg_function_decl->set_definingDeclaration(sg_function_decl);

        if (function_decl->isVariadic()) {
            sg_function_decl->hasEllipses();
        }

        if (!function_decl->hasBody()) {
            logger[DEBUG] << "Defining function declaration without body " << "\n";
            res = false;
        }

        SgFunctionDefinition * function_definition = sg_function_decl->get_definition();

        if (sg_function_decl->get_definition()->get_body() != NULL)
            SageInterface::deleteAST(sg_function_decl->get_definition()->get_body());

        SageBuilder::pushScopeStack(function_definition);

        SgNode * tmp_body = Traverse(function_decl->getBody());
        SgBasicBlock * body = isSgBasicBlock(tmp_body);

        SageBuilder::popScopeStack();

        if (tmp_body != NULL && body == NULL) {
            logger[WARN] << "Runtime error: tmp_body != NULL && body == NULL" << "\n";
            res = false;
        }
        else {
            function_definition->set_body(body);
            body->set_parent(function_definition);
            applySourceRange(function_definition, function_decl->getSourceRange());
        }

        sg_function_decl->set_definition(function_definition);
        function_definition->set_parent(sg_function_decl);

        SgFunctionDeclaration * first_decl;
        if (function_decl->getFirstDecl() == function_decl) {
            SgFunctionParameterList * param_list_ = SageBuilder::buildFunctionParameterList_nfi();
              setCompilerGeneratedFileInfo(param_list_);
            SgInitializedNamePtrList & init_names = param_list->get_args();
            SgInitializedNamePtrList::iterator it;
            for (it = init_names.begin(); it != init_names.end(); it++) {
                SgInitializedName * init_param = new SgInitializedName(**it);
                setCompilerGeneratedFileInfo(init_param);
                param_list_->append_arg(init_param);
            }

            first_decl = SageBuilder::buildNondefiningFunctionDeclaration(name, ret_type, param_list_, NULL);
            setCompilerGeneratedFileInfo(first_decl);
            first_decl->set_parent(SageBuilder::topScopeStack());
            first_decl->set_firstNondefiningDeclaration(first_decl);
            if (function_decl->isVariadic()) first_decl->hasEllipses();
        }
        else {
            SgSymbol * tmp_symbol = GetSymbolFromSymbolTable(function_decl->getFirstDecl());
            SgFunctionSymbol * symbol = isSgFunctionSymbol(tmp_symbol);
            if (tmp_symbol != NULL && symbol == NULL) {
                logger[WARN] << "Runtime error: tmp_symbol != NULL && symbol == NULL" << "\n";
                res = false;
            }
            if (symbol != NULL)
                first_decl = isSgFunctionDeclaration(symbol->get_declaration());
        }

        sg_function_decl->set_firstNondefiningDeclaration(first_decl);
        first_decl->set_definingDeclaration(sg_function_decl);
    }
    else {
        sg_function_decl = SageBuilder::buildNondefiningFunctionDeclaration(name, ret_type, param_list, NULL);

        if (function_decl->isVariadic()) sg_function_decl->hasEllipses();

        SgInitializedNamePtrList & init_names = param_list->get_args();
        SgInitializedNamePtrList::iterator it;
        for (it = init_names.begin(); it != init_names.end(); it++) {
             (*it)->set_scope(SageBuilder::topScopeStack());
        }

        if (function_decl->getFirstDecl() != function_decl) {
            SgSymbol * tmp_symbol = GetSymbolFromSymbolTable(function_decl->getFirstDecl());
            SgFunctionSymbol * symbol = isSgFunctionSymbol(tmp_symbol);
            if (tmp_symbol != NULL && symbol == NULL) {
                logger[WARN] << "Runtime error: tmp_symbol != NULL && symbol == NULL" << "\n";
                res = false;
            }
            SgFunctionDeclaration * first_decl = NULL;
            if (symbol != NULL) {
                first_decl = isSgFunctionDeclaration(symbol->get_declaration());
            }
            else {
                // FIXME Is it correct?
                SgNode * tmp_first_decl = Traverse(function_decl->getFirstDecl());
                first_decl = isSgFunctionDeclaration(tmp_first_decl);
                ROSE_ASSERT(first_decl != NULL);
                // ROSE_ASSERT(!"We should have see the first declaration already");
            }

            if (first_decl != NULL) {
                if (first_decl->get_firstNondefiningDeclaration() != NULL)
                    sg_function_decl->set_firstNondefiningDeclaration(first_decl->get_firstNondefiningDeclaration());
                else {
                    ROSE_ASSERT(first_decl->get_firstNondefiningDeclaration() != NULL);
                }
            }
            else {
                ROSE_ASSERT(!"First declaration not found!");
            }
        }
        else {
            sg_function_decl->set_firstNondefiningDeclaration(sg_function_decl);
        }
    }

    ROSE_ASSERT(sg_function_decl->get_firstNondefiningDeclaration() != NULL);
/* // TODO Fix problem with function symbols...
    SgSymbol * symbol = GetSymbolFromSymbolTable(function_decl);
    if (symbol == NULL) {
        SgFunctionSymbol * func_sym = new SgFunctionSymbol(isSgFunctionDeclaration(sg_function_decl->get_firstNondefiningDeclaration()));
        SageBuilder::topScopeStack()->insert_symbol(name, func_sym);        
    }
*/
//  ROSE_ASSERT(GetSymbolFromSymbolTable(function_decl) != NULL);

    *node = sg_function_decl;
#endif

     node_desc.kind_hierarchy.push_back("FunctionDecl");

  // DQ (11/27/2020): Trying to update the code from Clang 3.x to Clang 10.
  // node_desc.successors.push_back(std::pair<std::string, std::string>("previous_declaration", Traverse(function_decl->getPreviousDeclaration())));
  // node_desc.successors.push_back(std::pair<std::string, std::string>("previous_declaration", Traverse(function_decl->getPreviousDeclImpl())));
     node_desc.successors.push_back(std::pair<std::string, std::string>("previous_declaration", Traverse(function_decl->getCanonicalDecl())));

  // DQ (11/27/2020): Trying to update the code from Clang 3.x to Clang 10.
  // node_desc.successors.push_back(std::pair<std::string, std::string>("result_type", Traverse(function_decl->getResultType().getTypePtr())));
     node_desc.successors.push_back(std::pair<std::string, std::string>("result_type", Traverse(function_decl->getReturnType().getTypePtr())));

     for (unsigned i = 0; i < function_decl->getNumParams(); i++) 
        {
          std::ostringstream oss;
          oss << "parameter[" << i << "]";
          node_desc.successors.push_back(std::pair<std::string, std::string>(oss.str(), Traverse(function_decl->getParamDecl(i))));
        }

     node_desc.successors.push_back(std::pair<std::string, std::string>("body", Traverse(function_decl->getBody())));

     return VisitDeclaratorDecl(function_decl, node_desc) && res;
   }
#endif



#if 0
bool ClangToDotTranslator::VisitCXXDeductionGuideDecl(clang::CXXDeductionGuideDecl * cxx_deduction_guide_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXDeductionGuideDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitFunctionDecl(cxx_deduction_guide_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitCXXDeductionGuideDecl(clang::CXXDeductionGuideDecl * cxx_deduction_guide_decl, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXDeductionGuideDecl" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("CXXDeductionGuideDecl");

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitFunctionDecl(cxx_deduction_guide_decl, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitCXXMethodDecl(clang::CXXMethodDecl * cxx_method_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXMethodDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitFunctionDecl(cxx_method_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitCXXMethodDecl(clang::CXXMethodDecl * cxx_method_decl, NodeDescriptor & node_desc)
   {
#if DEBUG_VISIT_DECL
     logger[DEBUG] << "ClangToDotTranslator::VisitCXXMethodDecl" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("CXXMethodDecl");

//     ROSE_ASSERT(FAIL_TODO == 0); // TODO

     return VisitFunctionDecl(cxx_method_decl, node_desc) && res;
   }
#endif


#if 0
bool ClangToDotTranslator::VisitCXXConstructorDecl(clang::CXXConstructorDecl * cxx_constructor_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXConstructorDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitCXXMethodDecl(cxx_constructor_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitCXXConstructorDecl(clang::CXXConstructorDecl * cxx_constructor_decl, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXConstructorDecl" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("CXXConstructorDecl");

    // ...

    clang::CXXConstructorDecl::init_iterator it;
    unsigned cnt = 0;
    for (it = cxx_constructor_decl->init_begin(); it != cxx_constructor_decl->init_end(); it++) {
        std::ostringstream oss;
        oss << "init[" << cnt++ << "]";
//      node_desc.successors.push_back(std::pair<std::string, std::string>(oss.str(), Traverse(*it)));
        if((*it)->isMemberInitializer())
        {
          clang::FieldDecl * field_decl = (*it)->getMember();
          node_desc.successors.push_back(std::pair<std::string, std::string>(oss.str(), Traverse((*it)->getInit())));
          node_desc.attributes.push_back(std::pair<std::string, std::string>(oss.str(), field_decl->getNameAsString()));
        }
    }
    if(cxx_constructor_decl->isDefaultConstructor())
        node_desc.attributes.push_back(std::pair<std::string, std::string>("is_default_constructor", "true"));

//    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    res = VisitCXXMethodDecl(cxx_constructor_decl, node_desc);

    return res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitCXXConversionDecl(clang::CXXConversionDecl * cxx_conversion_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXConversionDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitCXXMethodDecl(cxx_conversion_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitCXXConversionDecl(clang::CXXConversionDecl * cxx_conversion_decl, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXConversionDecl" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("CXXConversionDecl");

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitCXXMethodDecl(cxx_conversion_decl, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitCXXDestructorDecl(clang::CXXDestructorDecl * cxx_destructor_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXDestructorDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitCXXMethodDecl(cxx_destructor_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitCXXDestructorDecl(clang::CXXDestructorDecl * cxx_destructor_decl, NodeDescriptor & node_desc) 
   {
#if DEBUG_VISIT_DECL
     logger[DEBUG] << "ClangToDotTranslator::VisitCXXDestructorDecl" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("CXXDestructorDecl");

     ROSE_ASSERT(FAIL_TODO == 0); // TODO

     return VisitCXXMethodDecl(cxx_destructor_decl, node_desc) && res;
   }
#endif

#if 0
bool ClangToDotTranslator::VisitMSPropertyDecl(clang::MSPropertyDecl * ms_property_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitMSPropertyDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitDeclaratorDecl(ms_property_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitMSPropertyDecl(clang::MSPropertyDecl * ms_property_decl, NodeDescriptor & node_desc)
    {
#if DEBUG_VISIT_DECL
     logger[DEBUG] << "ClangToDotTranslator::VisitMSPropertyDecl" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("MSPropertyDecl");

     ROSE_ASSERT(FAIL_TODO == 0); // TODO

     return VisitDeclaratorDecl(ms_property_decl, node_desc) && res;
   }
#endif

#if 0
bool ClangToDotTranslator::VisitNonTypeTemplateParmDecl(clang::NonTypeTemplateParmDecl * non_type_template_param_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitNonTypeTemplateParmDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitDeclaratorDecl(non_type_template_param_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitNonTypeTemplateParmDecl(clang::NonTypeTemplateParmDecl * non_type_template_param_decl, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitNonTypeTemplateParmDecl" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("NonTypeTemplateParmDecl");

    // (10/30/2026) lin32: Clang 19 returns TemplateArgumentLoc; unwrap its expression argument before DOT traversal.
    if (non_type_template_param_decl->hasDefaultArgument())
#if CLANG_VERSION_MAJOR >= 19
        node_desc.successors.push_back(std::pair<std::string, std::string>("default_argument", Traverse(non_type_template_param_decl->getDefaultArgument().getArgument().getAsExpr())));
#else
        node_desc.successors.push_back(std::pair<std::string, std::string>("default_argument", Traverse(non_type_template_param_decl->getDefaultArgument())));
#endif

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitDeclaratorDecl(non_type_template_param_decl, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitVarDecl(clang::VarDecl * var_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitVarDecl" << "\n";
#endif
    bool res = true;

  // Create the SAGE node: SgVariableDeclaration

    SgName name(var_decl->getNameAsString());

    SgType * type = buildTypeFromQualifiedType(var_decl->getType());

    clang::Expr * init_expr = var_decl->getInit();
    SgNode * tmp_init = Traverse(init_expr);
    SgExpression * expr = isSgExpression(tmp_init);
    if (tmp_init != NULL && expr == NULL) {
        logger[WARN] << "Runtime error: not a SgInitializer..." << "\n"; // TODO
        res = false;
    }
    SgExprListExp * expr_list_expr = isSgExprListExp(expr);

    SgInitializer * init = NULL;
    if (expr_list_expr != NULL)
        init = SageBuilder::buildAggregateInitializer(expr_list_expr, type);
    else if (expr != NULL)
        init = SageBuilder::buildAssignInitializer_nfi(expr, expr->get_type());
    if (init != NULL)
        applySourceRange(init, init_expr->getSourceRange());

    SgVariableDeclaration * sg_var_decl = new SgVariableDeclaration(name, type, init); // scope: obtain from the scope stack.

    if (isSgClassType(type)) {
        std::map<SgClassType *, bool>::iterator bool_it = p_class_type_decl_first_see_in_type.find(isSgClassType(type));
        ROSE_ASSERT(bool_it != p_class_type_decl_first_see_in_type.end());
        if (bool_it->second) {
            sg_var_decl->set_baseTypeDefiningDeclaration(isSgNamedType(type)->get_declaration()->get_definingDeclaration());
            sg_var_decl->set_variableDeclarationContainsBaseTypeDefiningDeclaration(true);
        }
    }
    else if (isSgEnumType(type)) {
        std::map<SgEnumType *, bool>::iterator bool_it = p_enum_type_decl_first_see_in_type.find(isSgEnumType(type));
        ROSE_ASSERT(bool_it != p_enum_type_decl_first_see_in_type.end());
        if (bool_it->second) {
            sg_var_decl->set_baseTypeDefiningDeclaration(isSgEnumType(type)->get_declaration()->get_definingDeclaration());
            sg_var_decl->set_variableDeclarationContainsBaseTypeDefiningDeclaration(true);
        }
    }

    sg_var_decl->set_firstNondefiningDeclaration(sg_var_decl);
    sg_var_decl->set_parent(SageBuilder::topScopeStack());

    ROSE_ASSERT(sg_var_decl->get_variables().size() == 1);

    SgInitializedName * init_name = sg_var_decl->get_variables()[0];
    ROSE_ASSERT(init_name != NULL);
    init_name->set_scope(SageBuilder::topScopeStack());

    applySourceRange(init_name, var_decl->getSourceRange());

    SgVariableDefinition * var_def = isSgVariableDefinition(init_name->get_declptr());
    ROSE_ASSERT(var_def != NULL);
    applySourceRange(var_def, var_decl->getSourceRange());

    SgVariableSymbol * var_symbol = new SgVariableSymbol(init_name);
    SageBuilder::topScopeStack()->insert_symbol(name, var_symbol);

    *node = sg_var_decl;

    return VisitDeclaratorDecl(var_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitVarDecl(clang::VarDecl * var_decl, NodeDescriptor & node_desc)
   {
#if DEBUG_VISIT_DECL
     logger[DEBUG] << "ClangToDotTranslator::VisitVarDecl" << "\n";
#endif
     bool res = true;

  // Create the SAGE node: SgVariableDeclaration

#if 0
    SgName name(var_decl->getNameAsString());

    SgType * type = buildTypeFromQualifiedType(var_decl->getType());

    clang::Expr * init_expr = var_decl->getInit();
    SgNode * tmp_init = Traverse(init_expr);
    SgExpression * expr = isSgExpression(tmp_init);
    if (tmp_init != NULL && expr == NULL) {
        logger[WARN] << "Runtime error: not a SgInitializer..." << "\n"; // TODO
        res = false;
    }
    SgExprListExp * expr_list_expr = isSgExprListExp(expr);

    SgInitializer * init = NULL;
    if (expr_list_expr != NULL)
        init = SageBuilder::buildAggregateInitializer(expr_list_expr, type);
    else if (expr != NULL)
        init = SageBuilder::buildAssignInitializer_nfi(expr, expr->get_type());
    if (init != NULL)
        applySourceRange(init, init_expr->getSourceRange());

    SgVariableDeclaration * sg_var_decl = new SgVariableDeclaration(name, type, init); // scope: obtain from the scope stack.

    if (isSgClassType(type)) {
        std::map<SgClassType *, bool>::iterator bool_it = p_class_type_decl_first_see_in_type.find(isSgClassType(type));
        ROSE_ASSERT(bool_it != p_class_type_decl_first_see_in_type.end());
        if (bool_it->second) {
            sg_var_decl->set_baseTypeDefiningDeclaration(isSgNamedType(type)->get_declaration()->get_definingDeclaration());
            sg_var_decl->set_variableDeclarationContainsBaseTypeDefiningDeclaration(true);
        }
    }
    else if (isSgEnumType(type)) {
        std::map<SgEnumType *, bool>::iterator bool_it = p_enum_type_decl_first_see_in_type.find(isSgEnumType(type));
        ROSE_ASSERT(bool_it != p_enum_type_decl_first_see_in_type.end());
        if (bool_it->second) {
            sg_var_decl->set_baseTypeDefiningDeclaration(isSgEnumType(type)->get_declaration()->get_definingDeclaration());
            sg_var_decl->set_variableDeclarationContainsBaseTypeDefiningDeclaration(true);
        }
    }

    sg_var_decl->set_firstNondefiningDeclaration(sg_var_decl);
    sg_var_decl->set_parent(SageBuilder::topScopeStack());

    ROSE_ASSERT(sg_var_decl->get_variables().size() == 1);

    SgInitializedName * init_name = sg_var_decl->get_variables()[0];
    ROSE_ASSERT(init_name != NULL);
    init_name->set_scope(SageBuilder::topScopeStack());

    applySourceRange(init_name, var_decl->getSourceRange());

    SgVariableDefinition * var_def = isSgVariableDefinition(init_name->get_declptr());
    ROSE_ASSERT(var_def != NULL);
    applySourceRange(var_def, var_decl->getSourceRange());

    SgVariableSymbol * var_symbol = new SgVariableSymbol(init_name);
    SageBuilder::topScopeStack()->insert_symbol(name, var_symbol);

    *node = sg_var_decl;
#endif

     node_desc.successors.push_back(std::pair<std::string, std::string>("acting_definition", Traverse(var_decl->getActingDefinition())));

     node_desc.successors.push_back(std::pair<std::string, std::string>("(VarDecl) definition", Traverse(var_decl->getDefinition())));

  // DQ (11/28/2020): I think this is no longer available in Clang 10.
  // node_desc.successors.push_back(std::pair<std::string, std::string>("out_of_line_definition", Traverse(var_decl->getOutOfLineDefinition())));

     node_desc.successors.push_back(std::pair<std::string, std::string>("init", Traverse(var_decl->getInit())));

     node_desc.kind_hierarchy.push_back("VarDecl");

    return VisitDeclaratorDecl(var_decl, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitDecompositionDecl(clang::DecompositionDecl * decomposition_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitDecompositionDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitVarDecl(decomposition_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitDecompositionDecl(clang::DecompositionDecl * decomposition_decl, NodeDescriptor & node_desc)
   {
#if DEBUG_VISIT_DECL
     logger[DEBUG] << "ClangToDotTranslator::VisitDecompositionDecl" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("DecompositionDecl");

     ROSE_ASSERT(FAIL_TODO == 0); // TODO

     return VisitVarDecl(decomposition_decl, node_desc) && res;
   }
#endif

#if 0
bool ClangToDotTranslator::VisitImplicitParamDecl(clang::ImplicitParamDecl * implicit_param_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitImplicitParamDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitVarDecl(implicit_param_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitImplicitParamDecl(clang::ImplicitParamDecl * implicit_param_decl, NodeDescriptor & node_desc) 
   {
#if DEBUG_VISIT_DECL
     logger[DEBUG] << "ClangToDotTranslator::VisitImplicitParamDecl" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("ImplicitParamDecl");

     ROSE_ASSERT(FAIL_TODO == 0); // TODO

     return VisitVarDecl(implicit_param_decl, node_desc) && res;
   }
#endif

#if 0
bool ClangToDotTranslator::VisitOMPCaptureExprDecl(clang::OMPCapturedExprDecl * omp_capture_expr_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitOMPCaptureExprDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitVarDecl(omp_capture_expr_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitOMPCaptureExprDecl(clang::OMPCapturedExprDecl * omp_capture_expr_decl, NodeDescriptor & node_desc)
   {
#if DEBUG_VISIT_DECL
     logger[DEBUG] << "ClangToDotTranslator::VisitOMPCaptureExprDecl" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("OMPCaptureExprDecl");

     ROSE_ASSERT(FAIL_TODO == 0); // TODO

     return VisitVarDecl(omp_capture_expr_decl, node_desc) && res;
   }
#endif

#if 0
bool ClangToDotTranslator::VisitParmVarDecl(clang::ParmVarDecl * param_var_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitParmVarDecl" << "\n";
#endif
    bool res = true;

    SgName name(param_var_decl->getNameAsString());

    SgType * type = buildTypeFromQualifiedType(param_var_decl->getOriginalType());

    SgInitializer * init = NULL;

    if (param_var_decl->hasDefaultArg()) {
        SgNode * tmp_expr = Traverse(param_var_decl->getDefaultArg());
        SgExpression * expr = isSgExpression(tmp_expr);
        if (tmp_expr != NULL && expr == NULL) {
            logger[WARN] << "Runtime error: tmp_expr != NULL && expr == NULL" << "\n";
            res = false;
        }
        else {
            applySourceRange(expr, param_var_decl->getDefaultArgRange());
            init = SageBuilder::buildAssignInitializer_nfi(expr, expr->get_type());
            applySourceRange(init, param_var_decl->getDefaultArgRange());
        }
    }

    *node = SageBuilder::buildInitializedName(name, type, init);

    return VisitDeclaratorDecl(param_var_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitParmVarDecl(clang::ParmVarDecl * param_var_decl, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitParmVarDecl" << "\n";
#endif
    bool res = true;

#if 0
    SgName name(param_var_decl->getNameAsString());

    SgType * type = buildTypeFromQualifiedType(param_var_decl->getOriginalType());

    SgInitializer * init = NULL;

    if (param_var_decl->hasDefaultArg()) {
        SgNode * tmp_expr = Traverse(param_var_decl->getDefaultArg());
        SgExpression * expr = isSgExpression(tmp_expr);
        if (tmp_expr != NULL && expr == NULL) {
            logger[WARN] << "Runtime error: tmp_expr != NULL && expr == NULL" << "\n";
            res = false;
        }
        else {
            applySourceRange(expr, param_var_decl->getDefaultArgRange());
            init = SageBuilder::buildAssignInitializer_nfi(expr, expr->get_type());
            applySourceRange(init, param_var_decl->getDefaultArgRange());
        }
    }

    *node = SageBuilder::buildInitializedName(name, type, init);
#endif

    node_desc.successors.push_back(std::pair<std::string, std::string>("original_type", Traverse(param_var_decl->getOriginalType().getTypePtr())));

    node_desc.successors.push_back(std::pair<std::string, std::string>("default_arg", Traverse(param_var_decl->getDefaultArg())));

     node_desc.kind_hierarchy.push_back("ParmVarDecl");

     return VisitDeclaratorDecl(param_var_decl, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitVarTemplateSpecializationDecl(clang::VarTemplateSpecializationDecl * var_template_specialization_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitVarTemplateSpecializationDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitDeclaratorDecl(var_template_specialization_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitVarTemplateSpecializationDecl(clang::VarTemplateSpecializationDecl * var_template_specialization_decl, NodeDescriptor & node_desc)
   {
#if DEBUG_VISIT_DECL
     logger[DEBUG] << "ClangToDotTranslator::VisitVarTemplateSpecializationDecl" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("VarTemplateSpecializationDecl");

     ROSE_ASSERT(FAIL_TODO == 0); // TODO

     return VisitDeclaratorDecl(var_template_specialization_decl, node_desc) && res;
   }
#endif

#if 0
bool ClangToDotTranslator::VisitVarTemplatePartialSpecializationDecl(clang::VarTemplatePartialSpecializationDecl * var_template_partial_specialization_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitVarTemplatePartialSpecializationDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitVarTemplateSpecializationDecl(var_template_partial_specialization_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitVarTemplatePartialSpecializationDecl(clang::VarTemplatePartialSpecializationDecl * var_template_partial_specialization_decl, NodeDescriptor & node_desc)
   {
#if DEBUG_VISIT_DECL
     logger[DEBUG] << "ClangToDotTranslator::VisitVarTemplatePartialSpecializationDecl" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("VarTemplatePartialSpecializationDecl");

     ROSE_ASSERT(FAIL_TODO == 0); // TODO

     return VisitVarTemplateSpecializationDecl(var_template_partial_specialization_decl, node_desc) && res;
   }
#endif

#if 0
bool  ClangToDotTranslator::VisitEnumConstantDecl(clang::EnumConstantDecl * enum_constant_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitEnumConstantDecl" << "\n";
#endif
    bool res = true;

    SgName name(enum_constant_decl->getNameAsString());

    SgType * type = buildTypeFromQualifiedType(enum_constant_decl->getType());

    SgInitializer * init = NULL;

    if (enum_constant_decl->getInitExpr() != NULL) {
        SgNode * tmp_expr = Traverse(enum_constant_decl->getInitExpr());
        SgExpression * expr = isSgExpression(tmp_expr);
        if (tmp_expr != NULL && expr == NULL) {
            logger[WARN] << "Runtime error: tmp_expr != NULL && expr == NULL" << "\n";
            res = false;
        }
        else {
            init = SageBuilder::buildAssignInitializer_nfi(expr, expr->get_type());
        }
    }

    SgInitializedName * init_name = SageBuilder::buildInitializedName(name, type, init);

    SgEnumFieldSymbol * symbol = new SgEnumFieldSymbol(init_name);

    SageBuilder::topScopeStack()->insert_symbol(name, symbol);

    *node = init_name;

    return VisitValueDecl(enum_constant_decl, node) && res;
}
#else
bool  ClangToDotTranslator::VisitEnumConstantDecl(clang::EnumConstantDecl * enum_constant_decl, NodeDescriptor & node_desc) 
   {
#if DEBUG_VISIT_DECL
     logger[DEBUG] << "ClangToDotTranslator::VisitEnumConstantDecl" << "\n";
#endif
     bool res = true;

#if 0
    SgName name(enum_constant_decl->getNameAsString());

    SgType * type = buildTypeFromQualifiedType(enum_constant_decl->getType());

    SgInitializer * init = NULL;

    if (enum_constant_decl->getInitExpr() != NULL) {
        SgNode * tmp_expr = Traverse(enum_constant_decl->getInitExpr());
        SgExpression * expr = isSgExpression(tmp_expr);
        if (tmp_expr != NULL && expr == NULL) {
            logger[WARN] << "Runtime error: tmp_expr != NULL && expr == NULL" << "\n";
            res = false;
        }
        else {
            init = SageBuilder::buildAssignInitializer_nfi(expr, expr->get_type());
        }
    }

    SgInitializedName * init_name = SageBuilder::buildInitializedName(name, type, init);

    SgEnumFieldSymbol * symbol = new SgEnumFieldSymbol(init_name);

    SageBuilder::topScopeStack()->insert_symbol(name, symbol);

    *node = init_name;
#endif

     node_desc.kind_hierarchy.push_back("EnumConstantDecl");

    node_desc.successors.push_back(std::pair<std::string, std::string>("init_expr", Traverse(enum_constant_decl->getInitExpr())));

    return VisitValueDecl(enum_constant_decl, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitIndirectFieldDecl(clang::IndirectFieldDecl * indirect_field_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitIndirectFieldDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitValueDecl(indirect_field_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitIndirectFieldDecl(clang::IndirectFieldDecl * indirect_field_decl, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitIndirectFieldDecl" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("IndirectFieldDecl");

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitValueDecl(indirect_field_decl, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitOMPDeclareMapperDecl(clang::OMPDeclareMapperDecl * omp_declare_mapper_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitOMPDeclareMapperDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitValueDecl(omp_declare_mapper_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitOMPDeclareMapperDecl(clang::OMPDeclareMapperDecl * omp_declare_mapper_decl, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitOMPDeclareMapperDecl" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("OMPDeclareMapperDecl");

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitValueDecl(omp_declare_mapper_decl, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitOMPDeclareReductionDecl(clang::OMPDeclareReductionDecl * omp_declare_reduction_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitOMPDeclareReductionDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitValueDecl(omp_declare_reduction_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitOMPDeclareReductionDecl(clang::OMPDeclareReductionDecl * omp_declare_reduction_decl, NodeDescriptor & node_desc)
   {
#if DEBUG_VISIT_DECL
     logger[DEBUG] << "ClangToDotTranslator::VisitOMPDeclareReductionDecl" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("OMPDeclareReductionDecl");

     ROSE_ASSERT(FAIL_TODO == 0); // TODO

     return VisitValueDecl(omp_declare_reduction_decl, node_desc) && res;
   }
#endif

#if 0
bool ClangToDotTranslator::VisitUnresolvedUsingValueDecl(clang::UnresolvedUsingValueDecl * unresolved_using_value_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitUnresolvedUsingValueDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitValueDecl(unresolved_using_value_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitUnresolvedUsingValueDecl(clang::UnresolvedUsingValueDecl * unresolved_using_value_decl, NodeDescriptor & node_desc) 
   {
#if DEBUG_VISIT_DECL
     logger[DEBUG] << "ClangToDotTranslator::VisitUnresolvedUsingValueDecl" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("UnresolvedUsingValueDecl");

     ROSE_ASSERT(FAIL_TODO == 0); // TODO

     return VisitValueDecl(unresolved_using_value_decl, node_desc) && res;
   }
#endif

#if 0
bool ClangToDotTranslator::VisitOMPAllocateDecl(clang::OMPAllocateDecl * omp_allocate_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitOMPAllocateDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitDecl(omp_allocate_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitOMPAllocateDecl(clang::OMPAllocateDecl * omp_allocate_decl, NodeDescriptor & node_desc)
   {
#if DEBUG_VISIT_DECL
     logger[DEBUG] << "ClangToDotTranslator::VisitOMPAllocateDecl" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("OMPAllocateDecl");

     ROSE_ASSERT(FAIL_TODO == 0); // TODO

     return VisitDecl(omp_allocate_decl, node_desc) && res;
   }
#endif

#if 0
bool ClangToDotTranslator::VisitOMPRequiresDecl(clang::OMPRequiresDecl * omp_requires_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitOMPRequiresDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitDecl(omp_requires_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitOMPRequiresDecl(clang::OMPRequiresDecl * omp_requires_decl, NodeDescriptor & node_desc)
   {
#if DEBUG_VISIT_DECL
     logger[DEBUG] << "ClangToDotTranslator::VisitOMPRequiresDecl" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("OMPRequiresDecl");

     ROSE_ASSERT(FAIL_TODO == 0); // TODO

     return VisitDecl(omp_requires_decl, node_desc) && res;
   }
#endif

#if 0
bool ClangToDotTranslator::VisitOMPThreadPrivateDecl(clang::OMPThreadPrivateDecl * omp_thread_private_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitOMPThreadPrivateDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitDecl(omp_thread_private_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitOMPThreadPrivateDecl(clang::OMPThreadPrivateDecl * omp_thread_private_decl, NodeDescriptor & node_desc)
   {
#if DEBUG_VISIT_DECL
     logger[DEBUG] << "ClangToDotTranslator::VisitOMPThreadPrivateDecl" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("OMPThreadPrivateDecl");

     ROSE_ASSERT(FAIL_TODO == 0); // TODO

     return VisitDecl(omp_thread_private_decl, node_desc) && res;
   }
#endif

#if 0
bool ClangToDotTranslator::VisitPragmaCommentDecl(clang::PragmaCommentDecl * pragma_comment_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitPragmaCommentDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitDecl(pragma_comment_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitPragmaCommentDecl(clang::PragmaCommentDecl * pragma_comment_decl, NodeDescriptor & node_desc)
   {
#if DEBUG_VISIT_DECL
     logger[DEBUG] << "ClangToDotTranslator::VisitPragmaCommentDecl" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("PragmaCommentDecl");

     ROSE_ASSERT(FAIL_TODO == 0); // TODO

     return VisitDecl(pragma_comment_decl, node_desc) && res;
   }
#endif

#if 0
bool ClangToDotTranslator::VisitPragmaDetectMismatchDecl(clang::PragmaDetectMismatchDecl * pragma_detect_mismatch_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitPragmaDetectMismatchDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitDecl(pragma_detect_mismatch_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitPragmaDetectMismatchDecl(clang::PragmaDetectMismatchDecl * pragma_detect_mismatch_decl, NodeDescriptor & node_desc)
   {
#if DEBUG_VISIT_DECL
     logger[DEBUG] << "ClangToDotTranslator::VisitPragmaDetectMismatchDecl" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("PragmaDetectMismatchDecl");

     ROSE_ASSERT(FAIL_TODO == 0); // TODO

     return VisitDecl(pragma_detect_mismatch_decl, node_desc) && res;
   }
#endif

#if 0
bool ClangToDotTranslator::VisitStaticAssertDecl(clang::StaticAssertDecl * pragma_static_assert_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitStaticAssertDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitDecl(pragma_static_assert_decl, node) && res;
}
#else
bool ClangToDotTranslator::VisitStaticAssertDecl(clang::StaticAssertDecl * pragma_static_assert_decl, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToDotTranslator::VisitStaticAssertDecl" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("StaticAssertDecl");

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitDecl(pragma_static_assert_decl, node_desc) && res;
}
#endif


#if 0
bool ClangToDotTranslator::VisitTranslationUnitDecl(clang::TranslationUnitDecl * translation_unit_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
     logger[DEBUG] << "ClangToDotTranslator::VisitTranslationUnitDecl" << "\n";
#endif

     if (*node != NULL) 
        {
          logger[WARN] << "Runtime error: The TranslationUnitDecl is already associated to a SAGE node." << "\n";
          return false;
        }

  // Create the SAGE node: SgGlobal

     if (p_global_scope != NULL) 
        {
          logger[WARN] << "Runtime error: Global Scope have already been set !" << "\n";
          return false;
        }

     *node = p_global_scope = new SgGlobal();

     p_decl_translation_map.insert(std::pair<clang::Decl *, SgNode *>(translation_unit_decl, p_global_scope));

  // Traverse the children

  // DQ (4/5/2017): Fixed code to use updated SageBuilder API.
  // SageBuilder::pushScopeStack(*node);
     SgScopeStatement* global_scope = isSgGlobal(*node);
     ROSE_ASSERT(global_scope != NULL);
     SageBuilder::pushScopeStack(global_scope);

     clang::DeclContext * decl_context = (clang::DeclContext *)translation_unit_decl; // useless but more clear

     bool res = true;
     clang::DeclContext::decl_iterator it;
     for (it = decl_context->decls_begin(); it != decl_context->decls_end(); it++) 
        {
          if (*it == NULL) continue;
#if 1
          logger[DEBUG] <<"@@@@@ Calling Traverse on clang::DeclContext::decl_iterator @@@@@ \n";
#endif
          SgNode * child = Traverse(*it);

          SgDeclarationStatement * decl_stmt = isSgDeclarationStatement(child);
          if (decl_stmt == NULL && child != NULL) 
             {
               logger[WARN] << "Runtime error: the node produce for a clang::Decl is not a SgDeclarationStatement !" << "\n";
               logger[WARN] << "    class = " << child->class_name() << "\n";
               res = false;
             }
            else 
             {
               if (child != NULL) 
                  {
                 // FIXME This is a hack to avoid autonomous decl of unnamed type to being added to the global scope....
                    SgClassDeclaration * class_decl = isSgClassDeclaration(child);
                    if (class_decl != NULL && (class_decl->get_name() == "" || class_decl->get_isUnNamed())) continue;
                    SgEnumDeclaration * enum_decl = isSgEnumDeclaration(child);
                    if (enum_decl != NULL && (enum_decl->get_name() == "" || enum_decl->get_isUnNamed())) continue;

                    p_global_scope->append_declaration(decl_stmt);
                  }
             }
        }

     SageBuilder::popScopeStack();

  // Traverse the class hierarchy

     return VisitDecl(translation_unit_decl, node) && res;
   }
#else
bool ClangToDotTranslator::VisitTranslationUnitDecl(clang::TranslationUnitDecl * translation_unit_decl, NodeDescriptor & node_desc)
   {
#if DEBUG_VISIT_DECL
      logger[DEBUG] << "ClangToDotTranslator::VisitTranslationUnitDecl" << "\n";
#endif

     bool res = true;

#if 0
     if (*node != NULL) 
        {
          logger[WARN] << "Runtime error: The TranslationUnitDecl is already associated to a SAGE node." << "\n";
          return false;
        }

  // Create the SAGE node: SgGlobal

     if (p_global_scope != NULL) 
        {
          logger[WARN] << "Runtime error: Global Scope have already been set !" << "\n";
          return false;
        }

     *node = p_global_scope = new SgGlobal();

     p_decl_translation_map.insert(std::pair<clang::Decl *, SgNode *>(translation_unit_decl, p_global_scope));

  // Traverse the children

  // DQ (4/5/2017): Fixed code to use updated SageBuilder API.
  // SageBuilder::pushScopeStack(*node);
     SgScopeStatement* global_scope = isSgGlobal(*node);
     ROSE_ASSERT(global_scope != NULL);
     SageBuilder::pushScopeStack(global_scope);

     clang::DeclContext * decl_context = (clang::DeclContext *)translation_unit_decl; // useless but more clear

  // bool res = true;
     clang::DeclContext::decl_iterator it;
     for (it = decl_context->decls_begin(); it != decl_context->decls_end(); it++) 
        {
          if (*it == NULL) continue;
#if 1
          logger[DEBUG] <<"@@@@@ Calling Traverse on clang::DeclContext::decl_iterator @@@@@ \n";
#endif
          SgNode * child = Traverse(*it);

          SgDeclarationStatement * decl_stmt = isSgDeclarationStatement(child);
          if (decl_stmt == NULL && child != NULL) 
             {
               logger[WARN] << "Runtime error: the node produce for a clang::Decl is not a SgDeclarationStatement !" << "\n";
               logger[WARN] << "    class = " << child->class_name() << "\n";
               res = false;
             }
            else 
             {
               if (child != NULL) 
                  {
                 // FIXME This is a hack to avoid autonomous decl of unnamed type to being added to the global scope....
                    SgClassDeclaration * class_decl = isSgClassDeclaration(child);
                    if (class_decl != NULL && (class_decl->get_name() == "" || class_decl->get_isUnNamed())) continue;
                    SgEnumDeclaration * enum_decl = isSgEnumDeclaration(child);
                    if (enum_decl != NULL && (enum_decl->get_name() == "" || enum_decl->get_isUnNamed())) continue;

                    p_global_scope->append_declaration(decl_stmt);
                  }
             }
        }

     SageBuilder::popScopeStack();
#endif

  // Code copied from Tristan's dot file generator.
     node_desc.kind_hierarchy.push_back("TranslationUnitDecl");

     clang::DeclContext::decl_iterator it;
     unsigned cnt = 0;
     for (it = translation_unit_decl->decls_begin(); it != translation_unit_decl->decls_end(); it++)
        {
          std::ostringstream oss;
          oss << "DeclContext::decls[" << cnt++ << "]";
#ifdef SHORT_CUT_BUILTIN
          if (cnt < 6) continue;
#endif
          node_desc.successors.push_back(std::pair<std::string, std::string>(oss.str(), Traverse(*it)));
        }

     node_desc.successors.push_back(std::pair<std::string, std::string>("anonymous_namespace",  Traverse(translation_unit_decl->getAnonymousNamespace())));

  // Traverse the class hierarchy
     return VisitDecl(translation_unit_decl, node_desc) && res;
   }
#endif
