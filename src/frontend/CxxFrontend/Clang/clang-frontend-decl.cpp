#include "sage3basic.h"
#include "clang-frontend-private.hpp"

using namespace Sawyer::Message;
SgSymbol * ClangToSageTranslator::GetSymbolFromSymbolTable(clang::NamedDecl * decl) {
    if (decl == NULL) return NULL;

    SgScopeStatement * scope = SageBuilder::topScopeStack();


/* Pei-Hung (08/29/2022) fieldDecl can be anonymous.
 * Following EDG's implementation to apply anonymous name to allow symbol lookup.
*/
    std::string declName = decl->getNameAsString();

    if(llvm::isa<clang::FieldDecl>(decl) && ((clang::FieldDecl*)decl)->isAnonymousStructOrUnion())
    {
      declName = "__anonymous_" +  generate_source_position_string(decl->getBeginLoc());  
#if DEBUG_SYMBOL_TABLE_LOOKUP
    logger[DEBUG] << "Find anonymous fieldDecl: " << declName << "\n";
#endif
    }
    else if(llvm::isa<clang::EnumDecl>(decl) && declName == "")
    {
      declName = "__anonymous_" +  generate_source_position_string(decl->getBeginLoc());
#if DEBUG_SYMBOL_TABLE_LOOKUP
    logger[DEBUG] << "Find anonymous EnumDecl: " << declName << "\n";
#endif
    }

    SgName name(declName);

#if DEBUG_SYMBOL_TABLE_LOOKUP
    logger[DEBUG] << "Lookup symbol for: " << name << "\n";
#endif

    if (name == "") {
        return NULL;
    }

    std::list<SgScopeStatement *>::reverse_iterator it;
    SgSymbol * sym = NULL;
    switch (decl->getKind()) {
        case clang::Decl::Typedef:
        {
            it = SageBuilder::ScopeStack.rbegin();
            while (it != SageBuilder::ScopeStack.rend() && sym == NULL) {
                 sym = (*it)->lookup_typedef_symbol(name);
                 it++;
            }
            break;
        }
        case clang::Decl::Var:
        case clang::Decl::ParmVar:
        {
            it = SageBuilder::ScopeStack.rbegin();
            while (it != SageBuilder::ScopeStack.rend() && sym == NULL) {
                sym = (*it)->lookup_variable_symbol(name);
                it++;
            }
            break;
        }
        case clang::Decl::Function:
        {
            SgType * tmp_type = buildTypeFromQualifiedType(((clang::FunctionDecl *)decl)->getType());
            SgFunctionType * type = isSgFunctionType(tmp_type);
            ROSE_ASSERT(type);
            it = SageBuilder::ScopeStack.rbegin();
            while (it != SageBuilder::ScopeStack.rend() && sym == NULL) {
                sym = (*it)->lookup_function_symbol(name, type);
                it++;
            }
            break;
        }
        case clang::Decl::CXXMethod:
        {
            /** Pei-Hung (08/14/23) SgMemberFunctionSymbol can only be found from the SgClassDefinition
             clang::FunctionProtoType doesn't seem to distinguish between funciton and method 
             The return type from buildTypeFromQualifiedType can only be SgfuncitonType
             Tried to lookup by name only and check if the symbol is a SgMemberFunctionSymbol
            **/
            /*
            SgType * tmp_type = buildTypeFromQualifiedType(((clang::CXXMethodDecl *)decl)->getType());
            SgFunctionType * type = isSgFunctionType(tmp_type);
            ROSE_ASSERT(type);
            */
            clang::CXXRecordDecl* enclosingRecordDecl = ((clang::CXXMethodDecl*)decl)->getParent();
            SgClassDeclaration* cxxRecordDeclaration  = isSgClassDeclaration(Traverse(enclosingRecordDecl));
            ROSE_ASSERT(cxxRecordDeclaration);
            SgClassDefinition* cxxRecordDefinition  = cxxRecordDeclaration->get_definition(); 
            sym = cxxRecordDefinition->lookup_function_symbol(name);
            ROSE_ASSERT(isSgMemberFunctionSymbol(sym));
            break;
        }
        case clang::Decl::Field:
        {
            // field can be variable or ClassDefinition
            
            clang::QualType fieldQualType = ((clang::FieldDecl*)decl)->getType();

            const clang::Type* fieldType = fieldQualType.getTypePtr();

            while((llvm::isa<clang::ElaboratedType>(fieldType)) || (llvm::isa<clang::ArrayType>(fieldType)))
            {
               if(llvm::isa<clang::ElaboratedType>(fieldType))
               {
                 fieldQualType = ((clang::ElaboratedType *)fieldType)->getNamedType();
               }
               else if(llvm::isa<clang::ArrayType>(fieldType))
               {
                 fieldQualType = ((clang::ArrayType *)fieldType)->getElementType();
               }
               fieldType = fieldQualType.getTypePtr();
            }
            bool isAnonymousStructOrUnion = false;
            if(llvm::isa<clang::RecordType>(fieldType))
            {
                isAnonymousStructOrUnion = ((clang::FieldDecl *)decl)->isAnonymousStructOrUnion();
            }

            SgClassDeclaration * sg_class_decl = isSgClassDeclaration(Traverse(((clang::FieldDecl *)decl)->getParent()));
            ROSE_ASSERT(sg_class_decl != NULL);
            if (sg_class_decl->get_definingDeclaration() == NULL)
                logger[WARN] << "Runtime Error: cannot find the definition of the class/struct associate to the field: " << name << "\n";
            else {
            /* Pei-Hung (09/20/22) by accessing FieldDecl through here seems to imply the RecordDcl has an associated declarator.
             * Therefore, the isAutonomousDeclaration hould be set false following the definition from EDG.
            */
                SgClassDeclaration* definingClassDecl = isSgClassDeclaration(sg_class_decl->get_definingDeclaration());
                sg_class_decl->set_isAutonomousDeclaration(false);
                definingClassDecl->set_isAutonomousDeclaration(false);
                scope = definingClassDecl->get_definition();
                // TODO: for C++, if 'scope' is in 'SageBuilder::ScopeStack': problem!!!
                //       It means that we are currently building the class
                while (scope != NULL && sym == NULL) {
                    if(isAnonymousStructOrUnion)
                        sym = scope->lookup_class_symbol(name);
                    else 
                        sym = scope->lookup_variable_symbol(name);
                    scope = scope->get_scope();
                }
            }
            break;
        }
        case clang::Decl::CXXRecord:
        case clang::Decl::Record:
        {
            it = SageBuilder::ScopeStack.rbegin();
            while (it != SageBuilder::ScopeStack.rend() && sym == NULL) {
                sym = (*it)->lookup_class_symbol(name);
                it++;
            }
            break;
        }
        case clang::Decl::Label:
        {
            // Should not be reach as we use Traverse to retrieve Label (they are "terminal" statements) (it avoids the problem of forward use of label: goto before declaration)
            name = SgName(((clang::LabelDecl *)decl)->getStmt()->getName());
            it = SageBuilder::ScopeStack.rbegin();
            while (it != SageBuilder::ScopeStack.rend() && sym == NULL) {
                sym = (*it)->lookup_label_symbol(name);
                it++;
            }
            break;
        }
        case clang::Decl::EnumConstant:
        {
            name = SgName(((clang::EnumConstantDecl *)decl)->getName().str());
            it = SageBuilder::ScopeStack.rbegin();
            while (it != SageBuilder::ScopeStack.rend() && sym == NULL) {
                sym = (*it)->lookup_enum_field_symbol(name);
                it++;
            }
            break;
        }
        case clang::Decl::Enum:
        {
            // An anonymous enum should have its name set with prefix "__anonymous_".  There is no need to to retreive the name from clang::EnumDecl, as it will be empty.
            it = SageBuilder::ScopeStack.rbegin();
            while (it != SageBuilder::ScopeStack.rend() && sym == NULL) {
                sym = (*it)->lookup_enum_symbol(name);
                it++;
            }
            break;
        }
        default:
            logger[WARN] << "Runtime Error: Unknown type of Decl. (" << decl->getDeclKindName() << ")" << "\n";
    }

    return sym;
}

SgNode * ClangToSageTranslator::Traverse(clang::Decl * decl) {
    if (decl == NULL)
        return NULL;

    std::map<clang::Decl *, SgNode *>::iterator it = p_decl_translation_map.find(decl);
    if (it != p_decl_translation_map.end()) {
#if DEBUG_TRAVERSE_DECL
        logger[DEBUG] << "Traverse Decl : " << decl << " ";
        if (clang::NamedDecl::classof(decl)) {
            logger[DEBUG] << ": " << ((clang::NamedDecl *)decl)->getNameAsString() << ") ";
        }
        logger[DEBUG] << " already visited : node = " << it->second << "\n";
#endif
        return it->second;
    }

    SgNode * result = NULL;
    bool ret_status = false;

    switch (decl->getKind()) {
        case clang::Decl::AccessSpec:
            ret_status = VisitAccessSpecDecl((clang::AccessSpecDecl *)decl, &result);
            ROSE_ASSERT(ret_status == false || result != NULL);
            break;
        case clang::Decl::Block:
            ret_status = VisitBlockDecl((clang::BlockDecl *)decl, &result);
            ROSE_ASSERT(ret_status == false || result != NULL);
            break;
        case clang::Decl::Captured:
            ret_status = VisitCapturedDecl((clang::CapturedDecl *)decl, &result);
            ROSE_ASSERT(ret_status == false || result != NULL);
            break;
        case clang::Decl::Empty:
            ret_status = VisitEmptyDecl((clang::EmptyDecl *)decl, &result);
            ROSE_ASSERT(ret_status == false || result != NULL);
            break;
        case clang::Decl::Export:
            ret_status = VisitExportDecl((clang::ExportDecl *)decl, &result);
            ROSE_ASSERT(ret_status == false || result != NULL);
            break;
        case clang::Decl::ExternCContext:
            ret_status = VisitExternCContextDecl((clang::ExternCContextDecl *)decl, &result);
            ROSE_ASSERT(ret_status == false || result != NULL);
            break;
        case clang::Decl::FileScopeAsm:
            ret_status = VisitFileScopeAsmDecl((clang::FileScopeAsmDecl *)decl, &result);
            ROSE_ASSERT(ret_status == false || result != NULL);
            break;
        case clang::Decl::Friend:
            ret_status = VisitFriendDecl((clang::FriendDecl *)decl, &result);
            ROSE_ASSERT(ret_status == false || result != NULL);
            break;
        case clang::Decl::FriendTemplate:
            ret_status = VisitFriendTemplateDecl((clang::FriendTemplateDecl *)decl, &result);
            ROSE_ASSERT(ret_status == false || result != NULL);
            break;
        case clang::Decl::Import:
            ret_status = VisitImportDecl((clang::ImportDecl *)decl, &result);
            ROSE_ASSERT(ret_status == false || result != NULL);
            break;
        case clang::Decl::Label:
            ret_status = VisitLabelDecl((clang::LabelDecl *)decl, &result);
            ROSE_ASSERT(ret_status == false || result != NULL);
            break;
        case clang::Decl::NamespaceAlias:
            ret_status = VisitNamespaceAliasDecl((clang::NamespaceAliasDecl *)decl, &result);
            ROSE_ASSERT(ret_status == false || result != NULL);
            break;
        case clang::Decl::Namespace:
            ret_status = VisitNamespaceDecl((clang::NamespaceDecl *)decl, &result);
            ROSE_ASSERT(ret_status == false || result != NULL);
            break;
        case clang::Decl::BuiltinTemplate:
            ret_status = VisitBuiltinTemplateDecl((clang::BuiltinTemplateDecl *)decl, &result);
            ROSE_ASSERT(ret_status == false || result != NULL);
            break;
        case clang::Decl::Concept:
            ret_status = VisitConceptDecl((clang::ConceptDecl *)decl, &result);
            ROSE_ASSERT(ret_status == false || result != NULL);
            break;
        case clang::Decl::ClassTemplate:
            ret_status = VisitClassTemplateDecl((clang::ClassTemplateDecl *)decl, &result);
            ROSE_ASSERT(ret_status == false || result != NULL);
            break;
        case clang::Decl::FunctionTemplate:
            ret_status = VisitFunctionTemplateDecl((clang::FunctionTemplateDecl *)decl, &result);
            ROSE_ASSERT(ret_status == false || result != NULL);
            break;
        case clang::Decl::TypeAliasTemplate:
            ret_status = VisitTypeAliasTemplateDecl((clang::TypeAliasTemplateDecl *)decl, &result);
            ROSE_ASSERT(ret_status == false || result != NULL);
            break;
        case clang::Decl::VarTemplate:
            ret_status = VisitVarTemplateDecl((clang::VarTemplateDecl *)decl, &result);
            ROSE_ASSERT(ret_status == false || result != NULL);
            break;
        case clang::Decl::TemplateTemplateParm:
            ret_status = VisitTemplateTemplateParmDecl((clang::TemplateTemplateParmDecl *)decl, &result);
            ROSE_ASSERT(ret_status == false || result != NULL);
            break;
        case clang::Decl::Record:
            ret_status = VisitRecordDecl((clang::RecordDecl *)decl, &result);
            ROSE_ASSERT(ret_status == false || result != NULL);
            break;
        case clang::Decl::CXXRecord:
            ret_status = VisitCXXRecordDecl((clang::CXXRecordDecl *)decl, &result);
            ROSE_ASSERT(ret_status == false || result != NULL);
            break;
        case clang::Decl::ClassTemplateSpecialization:
            ret_status = VisitClassTemplateSpecializationDecl((clang::ClassTemplateSpecializationDecl *)decl, &result);
            ROSE_ASSERT(ret_status == false || result != NULL);
            break;
        case clang::Decl::ClassTemplatePartialSpecialization:
            ret_status = VisitClassTemplatePartialSpecializationDecl((clang::ClassTemplatePartialSpecializationDecl *)decl, &result);
            ROSE_ASSERT(ret_status == false || result != NULL);
            break;
        case clang::Decl::Enum:
            ret_status = VisitEnumDecl((clang::EnumDecl *)decl, &result);
            ROSE_ASSERT(ret_status == false || result != NULL);
            break;
        case clang::Decl::TemplateTypeParm:
            ret_status = VisitTemplateTypeParmDecl((clang::TemplateTypeParmDecl *)decl, &result);
            ROSE_ASSERT(ret_status == false || result != NULL);
            break;
        case clang::Decl::Typedef:
            ret_status = VisitTypedefDecl((clang::TypedefDecl *)decl, &result);
            ROSE_ASSERT(ret_status == false || result != NULL);
            break;
        case clang::Decl::TypeAlias:
            ret_status = VisitTypeAliasDecl((clang::TypeAliasDecl *)decl, &result);
            ROSE_ASSERT(ret_status == false || result != NULL);
            break;
        case clang::Decl::UnresolvedUsingTypename:
            ret_status = VisitUnresolvedUsingTypenameDecl((clang::UnresolvedUsingTypenameDecl *)decl, &result);
            ROSE_ASSERT(ret_status == false || result != NULL);
            break;
        case clang::Decl::Using:
            ret_status = VisitUsingDecl((clang::UsingDecl *)decl, &result);
            ROSE_ASSERT(ret_status == false || result != NULL);
            break;
        case clang::Decl::UsingDirective:
            ret_status = VisitUsingDirectiveDecl((clang::UsingDirectiveDecl *)decl, &result);
            ROSE_ASSERT(ret_status == false || result != NULL);
            break;
        case clang::Decl::UsingPack:
            ret_status = VisitUsingPackDecl((clang::UsingPackDecl *)decl, &result);
            ROSE_ASSERT(ret_status == false || result != NULL);
            break;
        case clang::Decl::ConstructorUsingShadow:
            ret_status = VisitConstructorUsingShadowDecl((clang::ConstructorUsingShadowDecl *)decl, &result);
            ROSE_ASSERT(ret_status == false || result != NULL);
            break;
        case clang::Decl::Binding:
            ret_status = VisitBindingDecl((clang::BindingDecl *)decl, &result);
            ROSE_ASSERT(ret_status == false || result != NULL);
            break;
        case clang::Decl::Field:
            ret_status = VisitFieldDecl((clang::FieldDecl *)decl, &result);
            ROSE_ASSERT(ret_status == false || result != NULL);
            break;
        case clang::Decl::Function:
            ret_status = VisitFunctionDecl((clang::FunctionDecl *)decl, &result);
            ROSE_ASSERT(ret_status == false || result != NULL);
            break;
        case clang::Decl::CXXDeductionGuide:
            ret_status = VisitCXXDeductionGuideDecl((clang::CXXDeductionGuideDecl *)decl, &result);
            ROSE_ASSERT(ret_status == false || result != NULL);
            break;
        case clang::Decl::CXXMethod:
            ret_status = VisitCXXMethodDecl((clang::CXXMethodDecl *)decl, &result);
            ROSE_ASSERT(ret_status == false || result != NULL);
            break;
        case clang::Decl::CXXConstructor:
            ret_status = VisitCXXConstructorDecl((clang::CXXConstructorDecl *)decl, &result);
            ROSE_ASSERT(ret_status == false || result != NULL);
            break;
        case clang::Decl::CXXConversion:
            ret_status = VisitCXXConversionDecl((clang::CXXConversionDecl *)decl, &result);
            ROSE_ASSERT(ret_status == false || result != NULL);
            break;
        case clang::Decl::CXXDestructor:
            ret_status = VisitCXXDestructorDecl((clang::CXXDestructorDecl *)decl, &result);
            ROSE_ASSERT(ret_status == false || result != NULL);
            break;
        case clang::Decl::MSProperty:
            ret_status = VisitMSPropertyDecl((clang::MSPropertyDecl *)decl, &result);
            ROSE_ASSERT(ret_status == false || result != NULL);
            break;
        case clang::Decl::NonTypeTemplateParm:
            ret_status = VisitNonTypeTemplateParmDecl((clang::NonTypeTemplateParmDecl *)decl, &result);
            ROSE_ASSERT(ret_status == false || result != NULL);
            break;
        case clang::Decl::Decomposition:
            ret_status = VisitDecompositionDecl((clang::DecompositionDecl *)decl, &result);
            ROSE_ASSERT(ret_status == false || result != NULL);
            break;
        case clang::Decl::ImplicitParam:
            ret_status = VisitImplicitParamDecl((clang::ImplicitParamDecl *)decl, &result);
            ROSE_ASSERT(ret_status == false || result != NULL);
            break;
        case clang::Decl::OMPCapturedExpr:
            ret_status = VisitOMPCaptureExprDecl((clang::OMPCapturedExprDecl *)decl, &result);
            ROSE_ASSERT(ret_status == false || result != NULL);
            break;
        case clang::Decl::ParmVar:
            ret_status = VisitParmVarDecl((clang::ParmVarDecl *)decl, &result);
            ROSE_ASSERT(ret_status == false || result != NULL);
            break;
        case clang::Decl::VarTemplatePartialSpecialization:
            ret_status = VisitVarTemplatePartialSpecializationDecl((clang::VarTemplatePartialSpecializationDecl *)decl, &result);
            ROSE_ASSERT(ret_status == false || result != NULL);
            break;
        case clang::Decl::EnumConstant:
            ret_status = VisitEnumConstantDecl((clang::EnumConstantDecl *)decl, &result);
            ROSE_ASSERT(ret_status == false || result != NULL);
            break;
        case clang::Decl::IndirectField:
            ret_status = VisitIndirectFieldDecl((clang::IndirectFieldDecl *)decl, &result);
            ROSE_ASSERT(ret_status == false || result != NULL);
            break;
        case clang::Decl::OMPDeclareMapper:
            ret_status = VisitOMPDeclareMapperDecl((clang::OMPDeclareMapperDecl *)decl, &result);
            ROSE_ASSERT(ret_status == false || result != NULL);
            break;
        case clang::Decl::OMPDeclareReduction:
            ret_status = VisitOMPDeclareReductionDecl((clang::OMPDeclareReductionDecl *)decl, &result);
            ROSE_ASSERT(ret_status == false || result != NULL);
            break;
        case clang::Decl::UnresolvedUsingValue:
            ret_status = VisitUnresolvedUsingValueDecl((clang::UnresolvedUsingValueDecl *)decl, &result);
            ROSE_ASSERT(ret_status == false || result != NULL);
            break;
        case clang::Decl::OMPAllocate:
            ret_status = VisitOMPAllocateDecl((clang::OMPAllocateDecl *)decl, &result);
            ROSE_ASSERT(ret_status == false || result != NULL);
            break;
        case clang::Decl::OMPRequires:
            ret_status = VisitOMPRequiresDecl((clang::OMPRequiresDecl *)decl, &result);
            ROSE_ASSERT(ret_status == false || result != NULL);
            break;
        case clang::Decl::OMPThreadPrivate:
            ret_status = VisitOMPThreadPrivateDecl((clang::OMPThreadPrivateDecl *)decl, &result);
            ROSE_ASSERT(ret_status == false || result != NULL);
            break;
        case clang::Decl::PragmaComment:
            ret_status = VisitPragmaCommentDecl((clang::PragmaCommentDecl *)decl, &result);
            ROSE_ASSERT(ret_status == false || result != NULL);
            break;
        case clang::Decl::PragmaDetectMismatch:
            ret_status = VisitPragmaDetectMismatchDecl((clang::PragmaDetectMismatchDecl *)decl, &result);
            ROSE_ASSERT(ret_status == false || result != NULL);
            break;
        case clang::Decl::StaticAssert:
            ret_status = VisitStaticAssertDecl((clang::StaticAssertDecl *)decl, &result);
            ROSE_ASSERT(ret_status == false || result != NULL);
            break;
        case clang::Decl::TranslationUnit:
            ret_status = VisitTranslationUnitDecl((clang::TranslationUnitDecl *)decl, &result);
            ROSE_ASSERT(ret_status == false || result != NULL);
            break;
        case clang::Decl::Var:
            ret_status = VisitVarDecl((clang::VarDecl *)decl, &result);
            ROSE_ASSERT(ret_status == false || result != NULL);
            break;
        case clang::Decl::LinkageSpec:
            ret_status = VisitLinkageSpecDecl((clang::LinkageSpecDecl *)decl, &result);
            ROSE_ASSERT(ret_status == false || result != NULL);
            break;

        default:
            logger[ERROR] << "Unknown declacaration kind: " << decl->getDeclKindName() << " !" << "\n";
            ROSE_ABORT();
    }

    ROSE_ASSERT(ret_status == false || result != NULL);

    p_decl_translation_map.insert(std::pair<clang::Decl *, SgNode *>(decl, result));

#if DEBUG_TRAVERSE_DECL
    logger[DEBUG] << "Traverse(clang::Decl : " << decl << " ";
    if (clang::NamedDecl::classof(decl)) {
        logger[DEBUG] << ": " << ((clang::NamedDecl *)decl)->getNameAsString() << ") ";
    }
    logger[DEBUG] << " visit done : node = " << result << "\n";
#endif

    return ret_status ? result : NULL;
}


// Pei-Hung (09/01/2023) Revised this to iterate Decls in the DeclContext.
// DeclContext is derived into others but this is called only by VisitTranslationUnit and VisitNamesapce for now. 
// The top scope retrieved from SageBuilder::topScopeStack() should be properly defined before calling this.

bool ClangToSageTranslator::TraverseForDeclContext(clang::DeclContext * decl_context) {
    SgScopeStatement * scope = SageBuilder::topScopeStack();
    SgGlobal* global_scope = isSgGlobal(scope);
    SgNamespaceDefinitionStatement* namespace_scope = isSgNamespaceDefinitionStatement(scope);

    p_decl_context_map.insert(std::pair<clang::DeclContext *, SgScopeStatement *>(decl_context, scope));
    bool res = true;
    clang::DeclContext::decl_iterator it;
    for (it = decl_context->decls_begin(); it != decl_context->decls_end(); it++) {
        clang::Decl* decl = (*it);
        if (decl == nullptr) continue;
        SgNode * child = Traverse(decl);

        SgDeclarationStatement * decl_stmt = isSgDeclarationStatement(child);
        if (decl_stmt == NULL && child != NULL) {
            logger[WARN] << "Runtime error: the node produce for a clang::Decl is not a SgDeclarationStatement !" << "\n";
            logger[WARN] << "    class = " << child->class_name() << "\n";
            res = false;
        }
        else if (child != NULL) {
            // FIXME This is a hack to avoid autonomous decl of unnamed type to being added to the global scope....
            SgClassDeclaration * class_decl = isSgClassDeclaration(child);
            if (class_decl != NULL && (class_decl->get_name() == "" || class_decl->get_isUnNamed())) continue;

            SgEnumDeclaration * enum_decl = isSgEnumDeclaration(child);
            if (enum_decl != NULL && (enum_decl->get_name() == "" || enum_decl->get_isUnNamed())) continue;

            if(clang::TagDecl::classof(decl))
            {
              clang::TagDecl* tagDecl = (clang::TagDecl*)decl;
              if(tagDecl->isEmbeddedInDeclarator())  continue;
            }

            if(global_scope)
            {
              global_scope->append_declaration(decl_stmt);
            }
            else if(namespace_scope)
            {
              namespace_scope->append_declaration(decl_stmt);
            }
            else
            {
              logger[WARN] << "Not global or namespace scope applied in ClangToSageTranslator::TraverseForDeclContext\n";
              return false;
            }
        }
    }
    return res;
}

/**********************/
/* Visit Declarations */
/**********************/

bool ClangToSageTranslator::VisitDecl(clang::Decl * decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitDecl" << "\n";
#endif    
    if (*node == NULL) {
        logger[DEBUG] << "Runtime error: No Sage node associated with the declaration..." << "\n";
        return false;
    }

    if (!isSgGlobal(*node) && !isSgTemplateParameter(*node))
        applySourceRange(*node, decl->getSourceRange());

    if(isSgDeclarationStatement(*node))
    {
      SgDeclarationStatement* declStmt = isSgDeclarationStatement(*node);
      ROSE_ASSERT(declStmt);
      clang::AccessSpecifier accessSpec = decl->getAccess();
      switch(accessSpec){
        case clang::AS_public:
          declStmt->get_declarationModifier().get_accessModifier().setPublic();
          break;
        case clang::AS_protected:
          declStmt->get_declarationModifier().get_accessModifier().setProtected();
          break;
        case clang::AS_private:
          declStmt->get_declarationModifier().get_accessModifier().setPrivate();
          break;
        case clang::AS_none:
          declStmt->get_declarationModifier().get_accessModifier().setDefault();
          break;
        default:
          logger[WARN] << "no accessSpecifier is valid" << "\n";
      }
    }


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

bool ClangToSageTranslator::VisitAccessSpecDecl(clang::AccessSpecDecl * access_spec_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitAccessSpecDecl" << "\n";
#endif
    bool res = true;

    clang::AccessSpecifier accessSpec = access_spec_decl->getAccess();
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitAccessSpecDecl " << accessSpec << "\n";
#endif
    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitDecl(access_spec_decl, node) && res;
}

bool ClangToSageTranslator::VisitBlockDecl(clang::BlockDecl * block_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitBlockDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitDecl(block_decl, node) && res;
}

bool ClangToSageTranslator::VisitCapturedDecl(clang::CapturedDecl * captured_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitCapturedDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitDecl(captured_decl, node) && res;
}

bool ClangToSageTranslator::VisitEmptyDecl(clang::EmptyDecl * empty_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitEmptyDecl" << "\n";
#endif
    bool res = true;

    // (3/29/2022) Pei-Hung it seems to be okay just skip processing EmptyDecl
    // as SgBasicBlock allows no decl/stmt stored in it.

    return VisitDecl(empty_decl, node) && res;
}

bool ClangToSageTranslator::VisitExportDecl(clang::ExportDecl * export_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitExportDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitDecl(export_decl, node) && res;
}

bool ClangToSageTranslator::VisitExternCContextDecl(clang::ExternCContextDecl * ccontent_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitCContextDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitDecl(ccontent_decl, node) && res;
}

bool ClangToSageTranslator::VisitFileScopeAsmDecl(clang::FileScopeAsmDecl * file_scope_asm_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitFileScopeAsmDecl" << "\n";
#endif
    bool res = true;

    clang::StringLiteral* AsmStringLiteral = file_scope_asm_decl->getAsmString();
    llvm::StringRef AsmStringRef = AsmStringLiteral->getString();

#if DEBUG_VISIT_DECL
    logger[DEBUG] << "AsmStringRef:" << static_cast<std::string>(AsmStringRef) << "\n";
#endif
    SgAsmStmt* asmStmt = SageBuilder::buildAsmStatement(static_cast<std::string>(AsmStringRef)); 
    asmStmt->set_firstNondefiningDeclaration(asmStmt);
    asmStmt->set_definingDeclaration(asmStmt);
    asmStmt->set_parent(SageBuilder::topScopeStack());
    *node = asmStmt;

    return VisitDecl(file_scope_asm_decl, node) && res;
}

bool ClangToSageTranslator::VisitFriendDecl(clang::FriendDecl * friend_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitFriendDecl" << "\n";
    logger[DEBUG] << "FriendDecl::isUnsupportedFriend () " << friend_decl->isUnsupportedFriend() << "\n";
#endif
    bool res = true;

    clang::TypeSourceInfo * type_source_info = friend_decl->getFriendType();
    
    clang::NamedDecl* friend_inner_decl = friend_decl->getFriendDecl();
    if(friend_inner_decl)
    {
       *node = Traverse(friend_inner_decl);

       SgDeclarationStatement* declStmt = isSgDeclarationStatement(*node);
       if(declStmt != NULL)
       {
         declStmt->get_declarationModifier().setFriend(); 
       } 
    }
    // Pei-Hung: When FriendDecl names type, we need to construct the SgDeclarationStatement.
    // In current support, SgClassDeclaration is created to represent a class type. Other supports are TBD.
    if(type_source_info)
    {
       clang::QualType friendQualType = type_source_info->getType(); 
       const clang::Type* friendType = friendQualType.getTypePtr();
       // Pei-Hung: currently looking for ElaboratedType and RecordType
       if(llvm::isa<clang::ElaboratedType>(friendType))
       {
          friendQualType = ((clang::ElaboratedType *)friendType)->getNamedType();
          friendType =  friendQualType.getTypePtr();
          if(llvm::isa<clang::RecordType>(friendType))
          {
             clang::RecordType* friendBasedType = (clang::RecordType *)friendType; 
             clang::RecordDecl* recordDecl = friendBasedType->getDecl();
             clang::DeclContext* declContext = recordDecl->getDeclContext();
             SgName recordName(recordDecl->getNameAsString());

             SgScopeStatement * scope = SageBuilder::topScopeStack();
             SgScopeStatement * declContextScope = scope; 
             std::map<clang::DeclContext *, SgScopeStatement *>::iterator it = p_decl_context_map.find(declContext);
             if (it != p_decl_context_map.end()) {
               declContextScope = it->second;
             }

             // get type of class
#if (__clang__)  && (__clang_major__ >= 18)
             SgClassDeclaration::class_types type_of_class;
             switch (recordDecl->getTagKind()) {
                 case clang::TagTypeKind::Struct:
                     type_of_class = SgClassDeclaration::e_struct;
                     break;
                 case clang::TagTypeKind::Class:
                     type_of_class = SgClassDeclaration::e_class;
                     break;
                 case clang::TagTypeKind::Union:
                     type_of_class = SgClassDeclaration::e_union;
                     break;
                 default:
                     logger[WARN] << "Runtime error: RecordDecl can only be a struct/class/union." << "\n";
                     res = false;
             }
#else
             SgClassDeclaration::class_types type_of_class;
             switch (recordDecl->getTagKind()) {
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
#endif

             SgSymbol * sym = GetSymbolFromSymbolTable(recordDecl);
             SgClassSymbol * class_sym = isSgClassSymbol(sym);
             if(class_sym != NULL)
             {
               SgClassDeclaration* sg_def_class_decl = class_sym->get_declaration(); 
               SgClassDeclaration* sg_friend_class_decl = new SgClassDeclaration(recordName, type_of_class, sg_def_class_decl->get_type(), NULL);
               sg_friend_class_decl->set_definingDeclaration(sg_def_class_decl);
               sg_friend_class_decl->set_firstNondefiningDeclaration(sg_def_class_decl->get_firstNondefiningDeclaration());
               sg_friend_class_decl->set_scope(declContextScope);
               sg_friend_class_decl->set_parent(scope);
               sg_friend_class_decl->get_declarationModifier().setFriend(); 
               *node = sg_friend_class_decl;
             }
          }
       }
       else
       {
          // This part returns a SgType* but has no impact to the result because there is no SgDeclarationStatement returned for the FriendDecl
          *node = Traverse(friendType);
       }
    }

    return VisitDecl(friend_decl, node) && res;
}

bool ClangToSageTranslator::VisitFriendTemplateDecl(clang::FriendTemplateDecl * friend_template_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitFriendTemplateDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitDecl(friend_template_decl, node) && res;
}

bool ClangToSageTranslator::VisitImportDecl(clang::ImportDecl * import_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitImportDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME

    return VisitDecl(import_decl, node) && res;
}

bool ClangToSageTranslator::VisitNamedDecl(clang::NamedDecl * named_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitNamedDecl" << "\n";
    logger[DEBUG] << "hasLinkage() " << named_decl->hasLinkage() << "\n";
    logger[DEBUG] << "isCXXClassMember() " << named_decl->isCXXClassMember() << "\n";
    logger[DEBUG] << "isCXXInstanceMember() " << named_decl->isCXXInstanceMember() << "\n";
    logger[DEBUG] << "hasExternalFormalLinkage() " << named_decl->hasExternalFormalLinkage() << "\n";
    logger[DEBUG] << "isExternallyVisible () " << named_decl->isExternallyVisible () << "\n";
    logger[DEBUG] << "isExternallyDeclarable () " << named_decl->isExternallyDeclarable () << "\n";
    logger[DEBUG] << "isLinkageValid () " << named_decl->isLinkageValid () << "\n";
    logger[DEBUG] << "hasLinkageBeenComputed() " << named_decl->hasLinkageBeenComputed() << "\n";
    logger[DEBUG] << "isModulePrivate() " << named_decl->isModulePrivate() << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME

    return VisitDecl(named_decl, node) && res;
}

bool ClangToSageTranslator::VisitLabelDecl(clang::LabelDecl * label_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitLabelDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitNamedDecl(label_decl, node) && res;
}

bool ClangToSageTranslator::VisitNamespaceAliasDecl(clang::NamespaceAliasDecl * namespace_alias_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitNamespaceAliasDecl" << namespace_alias_decl->getAliasedNamespace() <<  "\n";
#endif
    bool res = true;

    clang::NamespaceDecl* namespaceDecl = namespace_alias_decl->getNamespace();
    clang::NamedDecl* aliasedNamespaeDecl = namespace_alias_decl->getAliasedNamespace();

    SgNamespaceDeclarationStatement* sgNamespaceDeclStmt = isSgNamespaceDeclarationStatement(Traverse(namespaceDecl));
    ROSE_ASSERT(sgNamespaceDeclStmt);
    SgName name(namespace_alias_decl->getNameAsString());

    SgNamespaceAliasDeclarationStatement* sgNamespaceAliasDeclStmt = SageBuilder::buildNamespaceAliasDeclarationStatement(name ,sgNamespaceDeclStmt);
    *node = sgNamespaceAliasDeclStmt;
    return VisitNamedDecl(namespace_alias_decl, node) && res;
}

bool ClangToSageTranslator::VisitNamespaceDecl(clang::NamespaceDecl * namespace_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitNamespaceDecl " << namespace_decl->getNameAsString() << "\n";
    logger[DEBUG] << "isAnonymousNamespace " << namespace_decl->isAnonymousNamespace() << "\n";
    logger[DEBUG] << "isInline " << namespace_decl->isInline() << "\n";
//    logger[DEBUG] << "isNested " << namespace_decl->isNested() << "\n";
    logger[DEBUG] << "isOriginalNamespace " << namespace_decl->isOriginalNamespace() << "\n";
    logger[DEBUG] << "isAnonymousNamespace " << namespace_decl->isAnonymousNamespace() << "\n";
#endif
    bool res = true;

    SgName name(namespace_decl->getNameAsString());
    SgScopeStatement * scope = SageBuilder::topScopeStack(); 
    SgNamespaceDeclarationStatement* namespaceDecl = SageBuilder::buildNamespaceDeclaration(name, scope);
    SgNamespaceDefinitionStatement* namespaceDefiniton = namespaceDecl->get_definition(); 
    namespaceDecl->set_isInlinedNamespace(namespace_decl->isInline());

    // The following is using same rpocess from VisitTranslationUnit.  Consider using a function
    SageBuilder::pushScopeStack(namespaceDefiniton);
    clang::DeclContext * decl_context = (clang::DeclContext *)namespace_decl; 
    p_decl_context_map.insert(std::pair<clang::DeclContext *, SgScopeStatement *>(decl_context, namespaceDefiniton));
    // DeclContext handling is repalced by calling TraverseForDeclContext
    res = TraverseForDeclContext(decl_context); 
    SageBuilder::popScopeStack();

    applySourceRange(namespaceDecl, namespace_decl->getSourceRange());
    *node = namespaceDecl;
    return VisitNamedDecl(namespace_decl, node) && res;
}

bool ClangToSageTranslator::VisitTemplateDecl(clang::TemplateDecl * template_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitTemplateDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME

    return VisitNamedDecl(template_decl, node) && res;
}

bool ClangToSageTranslator::VisitBuiltinTemplateDecl(clang::BuiltinTemplateDecl * builtin_template_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitBuiltinTemplateDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME

    return VisitTemplateDecl(builtin_template_decl, node) && res;
}

bool ClangToSageTranslator::VisitConceptDecl(clang::ConceptDecl * concept_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitConceptDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME

    return VisitTemplateDecl(concept_decl, node) && res;
}

bool ClangToSageTranslator::VisitRedeclarableTemplateDecl(clang::RedeclarableTemplateDecl * redeclarable_template_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitRedeclarableTemplateDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME

    return VisitTemplateDecl(redeclarable_template_decl, node) && res;
}

bool ClangToSageTranslator::VisitClassTemplateDecl(clang::ClassTemplateDecl * class_template_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitClassTemplateDecl" << "\n";
    logger[DEBUG] << "VisitClassTemplateDecl name:" <<class_template_decl->getNameAsString() <<  "\n";
    logger[DEBUG] << "VisitClassTemplateDecl isThisDeclarationADefinition:" <<class_template_decl->isThisDeclarationADefinition() <<  "\n";
#endif
    bool res = true;

    std::string templateClassName = class_template_decl->getNameAsString();
    // create top SgDeclarationScope
    SgDeclarationScope* declScope = SageBuilder::buildDeclarationScope();
    SageBuilder::pushScopeStack(isSgScopeStatement(declScope));

    // build template parameters
    clang::TemplateParameterList* templateParamLst = class_template_decl->getTemplateParameters();
    SgTemplateParameterPtrList sgTemplateParamLst;
    for (clang::NamedDecl* param : *templateParamLst){
         // three types: clang::TemplateTypeParmDecl, clang::NonTypeTemplateTypeParmDecl, and clang::templateTemplateTypeParmDecl
//       if(llvm::isa<clang::TemplateTypeParmDecl>(param)){
         SgTemplateParameter* sgParam = isSgTemplateParameter(Traverse((clang::Decl*)param));
         sgTemplateParamLst.push_back(sgParam);
//       }
//       else if(llvm::isa<clang::NonTypeTemplateTypeParmDecl>(param)){
//         SgTemplateParameter* param = isSgTemplateParameter(Traverse((clang::Decl*)param));
//       }
    }
    SageBuilder::popScopeStack();

    SgTemplateArgumentPtrList templateSpecializationArgumentList;
    SgName name(templateClassName);
    clang::CXXRecordDecl* templatedDecl = class_template_decl->getTemplatedDecl();
    SgClassDeclaration::class_types t =  SgClassDeclaration::e_class;
    if(templatedDecl->isStruct())
    {
      t = SgClassDeclaration::e_struct;
    }
    else if(templatedDecl->isUnion())
    {
      t = SgClassDeclaration::e_union;
    }

    SgScopeStatement* scope = SageBuilder::topScopeStack();
    SgTemplateClassDeclaration* nonDefTemplateClassDecl = SageBuilder::buildNondefiningTemplateClassDeclaration(name, t, scope, &sgTemplateParamLst, &templateSpecializationArgumentList );
    SgTemplateClassDeclaration* defTemplateClassDecl = SageBuilder::buildTemplateClassDeclaration(name, t, scope, nonDefTemplateClassDecl, &sgTemplateParamLst, &templateSpecializationArgumentList);
    nonDefTemplateClassDecl->set_parent(scope);
    nonDefTemplateClassDecl->set_definingDeclaration(defTemplateClassDecl);
    defTemplateClassDecl->set_parent(scope);

    // ROSE currently uses string, from EDG, to handle the template unparsing.  
    // Need to get the equivalent string from Clang, through the print() function.

    std::string templateString;
    llvm::raw_string_ostream Steram(templateString);
    class_template_decl->print(Steram);
    Steram.flush();
    // Clang's template string doesn't include the ending ; symbol.  
    if(class_template_decl->isThisDeclarationADefinition())
    {
       templateString += ';';
    }
    defTemplateClassDecl->set_string(templateString);

    *node = defTemplateClassDecl;
    return VisitRedeclarableTemplateDecl(class_template_decl, node) && res;
}

bool ClangToSageTranslator::VisitFunctionTemplateDecl(clang::FunctionTemplateDecl * function_template_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitFunctionTemplateDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitRedeclarableTemplateDecl(function_template_decl, node) && res;
}

bool ClangToSageTranslator::VisitTypeAliasTemplateDecl(clang::TypeAliasTemplateDecl * type_alias_template_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitTypeAliasTemplateDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitRedeclarableTemplateDecl(type_alias_template_decl, node) && res;
}

bool ClangToSageTranslator::VisitVarTemplateDecl(clang::VarTemplateDecl * var_template_decl, SgNode ** node) {

#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitVarTemplateDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitRedeclarableTemplateDecl(var_template_decl, node) && res;
}


bool ClangToSageTranslator::VisitTemplateTemplateParmDecl(clang::TemplateTemplateParmDecl * template_template_parm_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitTemplateTemplateParmDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO
//    SgTemplateParameter* param = SageBuilder::buildTemplateParameter(SgTemplateParameter::template_parameter);
//    node = *param;
    return VisitTemplateDecl(template_template_parm_decl, node) && res;
}

bool ClangToSageTranslator::VisitTypeDecl(clang::TypeDecl * type_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitTypeDecl" << "\n";
#endif

    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME

    return VisitNamedDecl(type_decl, node) && res;
}

bool ClangToSageTranslator::VisitTagDecl(clang::TagDecl * tag_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitTagDecl" << "\n";
    logger[DEBUG] << "isThisDeclarationADefinition() " << tag_decl->isThisDeclarationADefinition() << "\n";
    logger[DEBUG] << "isCompleteDefinition() " << tag_decl->isCompleteDefinition() << "\n";
    logger[DEBUG] << "isCompleteDefinitionRequired() " << tag_decl->isCompleteDefinitionRequired() << "\n";
    logger[DEBUG] << "isBeingDefined() " << tag_decl->isBeingDefined() << "\n";
    logger[DEBUG] << "isEmbeddedInDeclarator() " << tag_decl->isEmbeddedInDeclarator() << "\n";
    logger[DEBUG] << "isFreeStanding() " << tag_decl->isFreeStanding() << "\n";
    logger[DEBUG] << "mayHaveOutOfDateDef() " << tag_decl->mayHaveOutOfDateDef() << "\n";
    logger[DEBUG] << "isDependentType() " << tag_decl->isDependentType() << "\n";
    logger[DEBUG] << "isThisDeclarationADemotedDefinition() " << tag_decl->isThisDeclarationADemotedDefinition() << "\n";
    logger[DEBUG] << "isStruct () " << tag_decl->isStruct () << "\n";
    logger[DEBUG] << "isInterface () " << tag_decl->isInterface () << "\n";
    logger[DEBUG] << "isUnion () " << tag_decl->isUnion () << "\n";
    logger[DEBUG] << "isEnum () " << tag_decl->isEnum () << "\n";
    logger[DEBUG] << "hasNameForLinkage () " << tag_decl->hasNameForLinkage () << "\n";
#endif

    bool res = true;

    //ROSE_ASSERT(FAIL_FIXME == 0); // FIXME
    clang::DeclContext* decl_context = static_cast<clang::DeclContext*>(tag_decl);

    SgClassDeclaration* sg_class_decl = NULL; 
    SgClassDefinition* sg_class_def = NULL;
    if(sg_class_decl = isSgClassDeclaration(*node))
    {
       sg_class_def = isSgClassDefinition(sg_class_decl->get_definition());
       if(sg_class_def)
       { 
         p_decl_context_map.insert(std::pair<clang::DeclContext *, SgScopeStatement *>(decl_context, sg_class_def));
         SageBuilder::pushScopeStack(sg_class_def);
       }
    }

#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitTagDecl: casting to DeclContext" << "\n";
#endif
    for (clang::Decl* tmpDecl : decl_context->decls()) {
       // CXXMethodDecl is processed under VisitCXXRecordDecl. Need to check how to merge to here.
       if(llvm::isa<clang::CXXMethodDecl>(tmpDecl))
       {
#if DEBUG_VISIT_DECL
         logger[DEBUG] << "ClangToSageTranslator::VisitTagDecl: skipping CXXMethodDecl in DeclContext\n";
#endif
         continue;
       }
       // skip for now.
       if(tmpDecl->isImplicit())
       {
#if DEBUG_VISIT_DECL
         logger[DEBUG] << "ClangToSageTranslator::VisitTagDecl: skipping implicit in DeclContext\n";
#endif
         continue;
       }
       // ROSE doesn't seem to need to support AccessSpecDecl
       if(llvm::isa<clang::AccessSpecDecl>(tmpDecl))
       {
#if DEBUG_VISIT_DECL
         logger[DEBUG] << "ClangToSageTranslator::VisitTagDecl: skipping AccessSpecDecl in DeclContext\n";
#endif
         continue;
       }
#if DEBUG_VISIT_DECL
       logger[DEBUG] << "ClangToSageTranslator::VisitTagDecl: checking decls in DeclContext\n";
#endif
       SgNode * tmp_context = Traverse(tmpDecl);
       SgDeclarationStatement * decl_context = isSgDeclarationStatement(tmp_context);
       ROSE_ASSERT(decl_context != NULL);

       if(sg_class_def)
       {
         if(llvm::isa<clang::FieldDecl>(tmpDecl))
         {
#if DEBUG_VISIT_DECL
            logger[DEBUG] << "ClangToSageTranslator::VisitTagDecl: processing decl as a field\n";
#endif   
            sg_class_def->append_member(decl_context);
            decl_context->set_parent(sg_class_def);
         }
         if(llvm::isa<clang::RecordDecl>(tmpDecl))
         {
#if DEBUG_VISIT_DECL
            logger[DEBUG] << "ClangToSageTranslator::VisitTagDecl: processing decl as a record\n";
#endif   
         }
         if(llvm::isa<clang::FriendDecl>(tmpDecl))
         {
#if DEBUG_VISIT_DECL
            logger[DEBUG] << "ClangToSageTranslator::VisitTagDecl: processing decl as a friend decl\n";
#endif   
            sg_class_def->append_member(decl_context);
            decl_context->set_parent(sg_class_def);
         }
         if(llvm::isa<clang::VarDecl>(tmpDecl))
         {
#if DEBUG_VISIT_DECL
            logger[DEBUG] << "ClangToSageTranslator::VisitTagDecl: processing decl as a var decl\n";
#endif   
            clang::VarDecl* varDecl = (clang::VarDecl*)tmpDecl;
            if(varDecl->isStaticDataMember())
            {
              sg_class_def->append_member(decl_context);
              decl_context->set_parent(sg_class_def);
            }
            else
            {
              logger[WARN] << "Other clang::VarDecl in the decl_context is not handled";
            }
         }
       }

    }
    if(sg_class_def)
      SageBuilder::popScopeStack();

    return VisitTypeDecl(tag_decl, node) && res;
}

bool ClangToSageTranslator::VisitRecordDecl(clang::RecordDecl * record_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitRecordDecl" << "\n";
#endif

    // FIXME May have to check the symbol table first, because of out-of-order traversal of C++ classes (Could be done in CxxRecord class...)

    bool res = true;
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitRecordDecl name:" <<record_decl->getNameAsString() <<  "\n";
    logger[DEBUG] << "isAnonymousStructOrUnion() " << record_decl->isAnonymousStructOrUnion() << "\n";
    logger[DEBUG] << "hasObjectMember() " << record_decl->hasObjectMember() << "\n";
    logger[DEBUG] << "hasVolatileMember() " << record_decl->hasVolatileMember() << "\n";
    logger[DEBUG] << "hasLoadedFieldsFromExternalStorage() " << record_decl->hasLoadedFieldsFromExternalStorage() << "\n";
    logger[DEBUG] << "isNonTrivialToPrimitiveDefaultInitialize() " << record_decl->isNonTrivialToPrimitiveDefaultInitialize() << "\n";
    logger[DEBUG] << "isNonTrivialToPrimitiveCopy() " << record_decl->isNonTrivialToPrimitiveCopy() << "\n";
    logger[DEBUG] << "isNonTrivialToPrimitiveDestroy() " << record_decl->isNonTrivialToPrimitiveDestroy() << "\n";
    logger[DEBUG] << "hasNonTrivialToPrimitiveDefaultInitializeCUnion() " << record_decl->hasNonTrivialToPrimitiveDefaultInitializeCUnion() << "\n";
    logger[DEBUG] << "hasNonTrivialToPrimitiveDestructCUnion() " << record_decl->hasNonTrivialToPrimitiveDestructCUnion() << "\n";
    logger[DEBUG] << "hasNonTrivialToPrimitiveCopyCUnion() " << record_decl->hasNonTrivialToPrimitiveCopyCUnion() << "\n";
    logger[DEBUG] << "canPassInRegisters() " << record_decl->canPassInRegisters() << "\n";
    logger[DEBUG] << "isParamDestroyedInCallee() " << record_decl->isParamDestroyedInCallee() << "\n";
    logger[DEBUG] << "isRandomized() " << record_decl->isRandomized() << "\n";
    logger[DEBUG] << "isInjectedClassName() " << record_decl->isInjectedClassName() << "\n";
    logger[DEBUG] << "isLambda() " << record_decl->isLambda() << "\n";
    logger[DEBUG] << "isCapturedRecord() " << record_decl->isCapturedRecord() << "\n";
    logger[DEBUG] << "isOrContainsUnion() " << record_decl->isOrContainsUnion() << "\n";
    logger[DEBUG] << "field_empty() " << record_decl->field_empty() << "\n";
#endif

    SgClassDeclaration * sg_class_decl = NULL;

  // Find previous declaration

    clang::RecordDecl * prev_record_decl = record_decl->getPreviousDecl();
    clang::RecordDecl * record_Definition = record_decl->getDefinition();
    bool isDefined = record_decl->isThisDeclarationADefinition();
    bool isAnonymousStructOrUnion = record_decl->isAnonymousStructOrUnion();
    bool hasNameForLinkage = record_decl->hasNameForLinkage();

    //bool definedInSameDeclContext = (record_decl->getDeclContext() == record_Definition->getDeclContext());
    //logger[DEBUG] << "defining recordDecl and nondefining are in same declContext " << definedInSameDeclContext << "\n"; 
    //logger[DEBUG] << record_decl << ":" << record_decl->getDeclContext() << " " << record_Definition << ":" << record_Definition->getDeclContext() <<  "\n";

    // Finding the scope of enclosing DeclContext
    clang::DeclContext* declContext = record_decl->getDeclContext();

    SgScopeStatement * scope = SageBuilder::topScopeStack();
    SgScopeStatement * declContextScope = scope; 
    std::map<clang::DeclContext *, SgScopeStatement *>::iterator it = p_decl_context_map.find(declContext);
    if (it != p_decl_context_map.end()) {
      declContextScope = it->second;
    }

    SgClassSymbol * sg_prev_class_sym = isSgClassSymbol(GetSymbolFromSymbolTable(prev_record_decl));
    SgClassDeclaration * sg_prev_class_decl = sg_prev_class_sym == NULL ? NULL : isSgClassDeclaration(sg_prev_class_sym->get_declaration());

    SgClassDeclaration * sg_first_class_decl = sg_prev_class_decl == NULL ? NULL : isSgClassDeclaration(sg_prev_class_decl->get_firstNondefiningDeclaration());

    //SgClassDeclaration * sg_def_class_decl = sg_prev_class_decl == NULL ? NULL : isSgClassDeclaration(sg_prev_class_decl->get_definingDeclaration());
    SgClassSymbol * sg_defining_sym = NULL;
    // Pei-Hung (07/25/24) The case that a CXXRecordDecl has its definition inside a namespace.
    if(isa<clang::CXXRecordDecl>(record_decl) && ((clang::CXXRecordDecl*)record_decl)->hasDefinition())
    {
      sg_defining_sym = NULL;
    }
    else
    {
      sg_defining_sym = isSgClassSymbol(GetSymbolFromSymbolTable(record_Definition));
    }
    SgClassDeclaration * sg_def_class_decl = sg_defining_sym == NULL ? NULL : isSgClassDeclaration(sg_defining_sym->get_declaration()->get_definingDeclaration());


    ROSE_ASSERT(sg_first_class_decl != NULL || sg_def_class_decl == NULL);

    bool had_prev_decl = sg_first_class_decl != NULL;

  // Name


/* Pei-Hung (08/29/2022) RecordDecl can be anonymous.
 * Following EDG's implementation to apply anonymous name to allow symbol lookup.
 *
 * The following in Clang is anonymous:
 * struct { int i; float f; };
 * Whereas the following are not:
 *   struct X { int i; float f; };
 *   struct { int i; float f; } obj; 
 *
 * For EDG, the following is also considered as anonymous:
 *   struct { int i; float f; } obj;
 *
 * recordDecl with hasNameForLinkage set to false is more close to the definition of anonymous defined by EDG
*/
    std::string recordDeclName = record_decl->getNameAsString();
    bool isUnNamed = false;
    // Pei-Hung (06/30/2023) recordDeclName caould be empty if linkaged name being defined in a typedef of this type
    if(!hasNameForLinkage || recordDeclName == "")
    {
      recordDeclName = "__anonymous_" +  generate_source_position_string(record_decl->getBeginLoc());
      isUnNamed = true;
    }

    SgName name(recordDeclName);


  // Type of class

#if (__clang__)  && (__clang_major__ >= 18)
    SgClassDeclaration::class_types type_of_class;
    switch (record_decl->getTagKind()) {
        case clang::TagTypeKind::Struct:
            type_of_class = SgClassDeclaration::e_struct;
            break;
        case clang::TagTypeKind::Class:
            type_of_class = SgClassDeclaration::e_class;
            break;
        case clang::TagTypeKind::Union:
            type_of_class = SgClassDeclaration::e_union;
            break;
        default:
            logger[WARN] << "Runtime error: RecordDecl can only be a struct/class/union." << "\n";
            res = false;
    }
#else
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
#endif

  // Build declaration(s)

    sg_class_decl = new SgClassDeclaration(name, type_of_class, NULL, NULL);

// Pei-Hung: the scope seems to be represented as declContext in Clang.  Need to confirm
    // This needs more checking.  What if there are two nondefiningDeclaration, can they
    // be placed in the same scope?
    sg_class_decl->set_scope(declContextScope);
    if(record_decl->isCanonicalDecl())
      sg_class_decl->set_parent(declContextScope);
    else
      sg_class_decl->set_parent(scope);
    //sg_class_decl->set_scope(SageBuilder::topScopeStack());
    //sg_class_decl->set_parent(SageBuilder::topScopeStack());

 // DQ (11/28/2020): Adding asertion.
    ROSE_ASSERT(sg_class_decl->get_parent() != NULL);

    SgClassType * type = NULL;
    if (sg_first_class_decl != NULL) {
        type = sg_first_class_decl->get_type();
    }
    else {
        type = SgClassType::createType(sg_class_decl);
    }
    ROSE_ASSERT(type != NULL);
    sg_class_decl->set_type(type);

    if (isUnNamed) sg_class_decl->set_isUnNamed(true);

    if (!had_prev_decl) {
        sg_first_class_decl = sg_class_decl;
        sg_first_class_decl->set_firstNondefiningDeclaration(sg_first_class_decl);
        sg_first_class_decl->set_definingDeclaration(NULL);
        sg_first_class_decl->set_definition(NULL);
        sg_first_class_decl->setForward();
        SgClassSymbol * class_symbol = new SgClassSymbol(sg_first_class_decl);
        declContextScope->insert_symbol(name, class_symbol);
    }
    else if (!isDefined)
    { 
    //    return false; // FIXME ROSE need only one non-defining declaration (SageBuilder don't let me build another one....)
        sg_class_decl->set_firstNondefiningDeclaration(sg_prev_class_decl->get_firstNondefiningDeclaration());
        sg_class_decl->set_definingDeclaration(NULL);
        sg_class_decl->setForward();
    }

    SgClassDefinition * sg_class_def = NULL; 
    if (isDefined) {
        sg_def_class_decl = new SgClassDeclaration(name, type_of_class, type, NULL);
        sg_def_class_decl->set_scope(SageBuilder::topScopeStack());
        if (isUnNamed) sg_def_class_decl->set_isUnNamed(true);
        sg_def_class_decl->set_parent(SageBuilder::topScopeStack());

        sg_class_decl = sg_def_class_decl; // we return thew defining decl

        sg_def_class_decl->set_firstNondefiningDeclaration(sg_first_class_decl);
        sg_def_class_decl->set_definingDeclaration(sg_def_class_decl);

        sg_first_class_decl->set_definingDeclaration(sg_def_class_decl);
        setCompilerGeneratedFileInfo(sg_first_class_decl);

        if(had_prev_decl) {
          sg_first_class_decl->set_definingDeclaration(sg_def_class_decl);
        }
        clang::RecordDecl* currentRecordDecl = record_decl;
        while(currentRecordDecl = currentRecordDecl->getPreviousDecl())
        {
          logger[DEBUG] << "PrevDecl: " << currentRecordDecl << "\n";
          //currentRecordDecl = record_decl->getPreviousDecl();
          SgClassDeclaration* sg_prev_decl = isSgClassDeclaration(Traverse(currentRecordDecl));
          sg_prev_decl->set_definingDeclaration(sg_def_class_decl);
        }
     

  // Build ClassDefinition
        sg_class_def = isSgClassDefinition(sg_def_class_decl->get_definition());
        if (sg_class_def == NULL) {
            sg_class_def = SageBuilder::buildClassDefinition_nfi(sg_def_class_decl);
        }
        sg_def_class_decl->set_definition(sg_class_def);

        ROSE_ASSERT(sg_class_def->get_symbol_table() != NULL);

        applySourceRange(sg_class_def, record_decl->getSourceRange());
/*
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
*/
    }

    // By default set class decl to be autonomous   
    sg_class_decl->set_isAutonomousDeclaration(true);
    sg_first_class_decl->set_isAutonomousDeclaration(true);


    ROSE_ASSERT(sg_class_decl->get_definingDeclaration() == NULL || isSgClassDeclaration(sg_class_decl->get_definingDeclaration())->get_definition() != NULL);
    ROSE_ASSERT(sg_first_class_decl->get_definition() == NULL);
    ROSE_ASSERT(sg_def_class_decl == NULL || sg_def_class_decl->get_definition() != NULL);

    *node = sg_class_decl;
    res = VisitTagDecl(record_decl, node);
    // Clang can expose a nested record through the enclosing DeclContext
    // without adding it to the Sage class member list. Attach only named
    // records used by an enclosing static record-array member.
    SgClassDefinition * parentClassDefinition = isSgClassDefinition(declContextScope);
    clang::CXXRecordDecl * nestedCxxRecord = llvm::dyn_cast<clang::CXXRecordDecl>(record_decl);
    clang::CXXRecordDecl * enclosingCxxRecord = llvm::dyn_cast<clang::CXXRecordDecl>(record_decl->getDeclContext());
    bool referencedByStaticDataMember = false;
    if (nestedCxxRecord != NULL && enclosingCxxRecord != NULL) {
        for (clang::Decl * enclosingDecl : enclosingCxxRecord->decls()) {
            clang::VarDecl * staticMember = llvm::dyn_cast<clang::VarDecl>(enclosingDecl);
            if (staticMember == NULL || !staticMember->isStaticDataMember()) continue;
            const clang::Type * memberType = staticMember->getType().getTypePtr();
            while (true) {
                if (const clang::ElaboratedType * elaboratedType = llvm::dyn_cast<clang::ElaboratedType>(memberType)) {
                    memberType = elaboratedType->getNamedType().getTypePtr();
                } else if (const clang::ArrayType * arrayType = llvm::dyn_cast<clang::ArrayType>(memberType)) {
                    memberType = arrayType->getElementType().getTypePtr();
                } else {
                    break;
                }
            }
            if (const clang::RecordType * recordType = llvm::dyn_cast<clang::RecordType>(memberType))
                referencedByStaticDataMember = recordType->getDecl()->getCanonicalDecl() == nestedCxxRecord->getCanonicalDecl();
            if (referencedByStaticDataMember) break;
        }
    }
    if (parentClassDefinition != NULL && nestedCxxRecord != NULL && referencedByStaticDataMember
        && nestedCxxRecord->hasNameForLinkage()
        && !nestedCxxRecord->isAnonymousStructOrUnion()
        && !nestedCxxRecord->isInjectedClassName()) {
        SgDeclarationStatementPtrList& memberList = parentClassDefinition->get_members();
        if (std::find(memberList.begin(), memberList.end(), sg_class_decl) == memberList.end()) {
            parentClassDefinition->append_member(sg_class_decl);
            sg_class_decl->set_parent(parentClassDefinition);
        }
    }
    if (isDefined) {
        SageBuilder::pushScopeStack(sg_class_def);

        // check if all fields are properly inserted into the definition
        clang::RecordDecl::field_iterator it;
        for (it = record_decl->field_begin(); it != record_decl->field_end(); it++) {
#if DEBUG_VISIT_DECL
            logger[DEBUG] << "ClangToSageTranslator::VisitRecordDecl visit field " << (*it)->getNameAsString() << "\n";
#endif
            SgNode * tmp_field = Traverse(*it);
            SgDeclarationStatement * field_decl = isSgDeclarationStatement(tmp_field);
            ROSE_ASSERT(field_decl != NULL);
            SgDeclarationStatementPtrList& memberList = sg_class_def->get_members();
            if(std::find(memberList.begin(), memberList.end(), field_decl) != memberList.end() && field_decl->get_parent() == sg_class_def)
            {
              //logger[DEBUG] << "Field is inserted\n";
              continue;
            }
            //logger[DEBUG] << "Field to be inserted\n";
            sg_class_def->append_member(field_decl);
            field_decl->set_parent(sg_class_def);
        }

        SageBuilder::popScopeStack();
    } 


    return res;
}

bool ClangToSageTranslator::VisitCXXRecordDecl(clang::CXXRecordDecl * cxx_record_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitCXXRecordDecl" << "\n";
    logger[DEBUG] << "hasDefinition() " << cxx_record_decl->hasDefinition() << "\n";
    if(cxx_record_decl->hasDefinition())
    {
      logger[DEBUG] << "isAggregate() " << cxx_record_decl->isAggregate() << "\n";
      logger[DEBUG] << "hasInClassInitializer() " << cxx_record_decl->hasInClassInitializer() << "\n";
      logger[DEBUG] << "hasUninitializedReferenceMember() " << cxx_record_decl->hasUninitializedReferenceMember() << "\n";
      logger[DEBUG] << "isPOD() " << cxx_record_decl->isPOD() << "\n";
      logger[DEBUG] << "isCLike() " << cxx_record_decl->isCLike() << "\n";
      logger[DEBUG] << "isEmpty() " << cxx_record_decl->isEmpty() << "\n";
      logger[DEBUG] << "hasInitMethod() " << cxx_record_decl->hasInitMethod() << "\n";
      logger[DEBUG] << "hasProtectedFields() " << cxx_record_decl->hasProtectedFields() << "\n";
      logger[DEBUG] << "hasPrivateFields() " << cxx_record_decl->hasPrivateFields() << "\n";
      logger[DEBUG] << "hasDirectFields() " << cxx_record_decl->hasDirectFields() << "\n";
      logger[DEBUG] << "isPolymorphic() " << cxx_record_decl->isPolymorphic() << "\n";
      logger[DEBUG] << "isAbstract () " << cxx_record_decl->isAbstract () << "\n";
      logger[DEBUG] << "isStandardLayout() " << cxx_record_decl->isStandardLayout() << "\n";
      logger[DEBUG] << "isCXX11StandardLayout () " << cxx_record_decl->isCXX11StandardLayout () << "\n";
      logger[DEBUG] << "hasMutableFields() " << cxx_record_decl->hasMutableFields() << "\n";
      logger[DEBUG] << "hasVariantMembers() " << cxx_record_decl->hasVariantMembers() << "\n";
      logger[DEBUG] << "hasTrivialDefaultConstructor() " << cxx_record_decl->hasTrivialDefaultConstructor() << "\n";
      logger[DEBUG] << "hasNonTrivialDefaultConstructor() " << cxx_record_decl->hasNonTrivialDefaultConstructor() << "\n";
      logger[DEBUG] << "isAnonymousStructOrUnion() " << cxx_record_decl->isAnonymousStructOrUnion() << "\n";
    }
    // to be filled up for full list
#endif
    bool res = VisitRecordDecl(cxx_record_decl, node);
    SgClassDeclaration* CxxRecordDeclaration = isSgClassDeclaration(*node);
    ROSE_ASSERT(CxxRecordDeclaration); 

    if(!cxx_record_decl->isThisDeclarationADefinition())
      return res;

    p_decl_translation_map.insert(std::pair<clang::Decl *, SgNode *>(cxx_record_decl, *node));

    SageBuilder::pushScopeStack(CxxRecordDeclaration->get_definition());
    clang::CXXRecordDecl::base_class_iterator it_base;
    for (it_base = cxx_record_decl->bases_begin(); it_base !=  cxx_record_decl->bases_end(); it_base++) {
        // TODO add base classes
    }

    clang::CXXRecordDecl::method_iterator it_method;
    for (it_method = cxx_record_decl->method_begin(); it_method !=  cxx_record_decl->method_end(); it_method++) {
        // TODO
#if DEBUG_VISIT_DECL
        logger[DEBUG] << "ClangToSageTranslator::VisitCXXRecordDecl visit method " << (*it_method)->getNameAsString() << "\n";
#endif
        SgNode* methodNode = Traverse(*it_method);
        SgDeclarationStatement* methodDeclStmt = isSgDeclarationStatement(methodNode);
        ROSE_ASSERT(methodDeclStmt);
        methodDeclStmt->set_scope(CxxRecordDeclaration->get_definition());
        CxxRecordDeclaration->get_definition()->append_member(methodDeclStmt);
        if((*it_method)->isImplicit())
        {
#if DEBUG_IMPLICIT_NODE
           logger[DEBUG] << "VisitCXXRecordDecl: implicit method marked as compiler generated: " << (*it_method)->getNameAsString() << "\n";
#endif
           methodDeclStmt->setCompilerGenerated();
        }
    }

    clang::CXXRecordDecl::ctor_iterator it_ctor;
    for (it_ctor = cxx_record_decl->ctor_begin(); it_ctor != cxx_record_decl->ctor_end(); it_ctor++) {
        // TODO if not tranversed as methods
#if DEBUG_VISIT_DECL
        logger[DEBUG] << "ClangToSageTranslator::VisitCXXRecordDecl visit ctor " << (*it_ctor)->getNameAsString() << "\n";
#endif
    }

    clang::CXXRecordDecl::friend_iterator it_friend;
    for (it_friend = cxx_record_decl->friend_begin(); it_friend != cxx_record_decl->friend_end(); it_friend++) {
        // TODO
    }

    clang::CXXDestructorDecl * destructor = cxx_record_decl->getDestructor();
    // TODO

    SageBuilder::popScopeStack();
    return res;
}

bool ClangToSageTranslator::VisitClassTemplateSpecializationDecl(clang::ClassTemplateSpecializationDecl * class_tpl_spec_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitClassTemplateSpecializationDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitCXXRecordDecl(class_tpl_spec_decl, node) && res;
}

bool ClangToSageTranslator::VisitClassTemplatePartialSpecializationDecl(clang::ClassTemplatePartialSpecializationDecl * class_tpl_part_spec_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitClassTemplatePartialSpecializationDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitClassTemplateSpecializationDecl(class_tpl_part_spec_decl, node) && res;
}

bool ClangToSageTranslator::VisitEnumDecl(clang::EnumDecl * enum_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitEnumDecl" << "\n";
#endif
    bool res = true;
    std::string enumDeclName = enum_decl->getNameAsString();

    if(enumDeclName.empty())
    {
      enumDeclName = "__anonymous_" +  generate_source_position_string(enum_decl->getBeginLoc());  
    }

    SgName name(enumDeclName);

#if DEBUG_VISIT_DECL
    logger[DEBUG] << "isfreestanding:" << enum_decl->isFreeStanding() << " isembedded:" << enum_decl->isEmbeddedInDeclarator() << "\n";
    logger[DEBUG] << "hasNameForLinkage:" << enum_decl->hasNameForLinkage() << "\n";
    logger[DEBUG] << "enum name:" << enumDeclName << "\n";
#endif

    clang::EnumDecl * prev_enum_decl = enum_decl->getPreviousDecl();
    SgEnumSymbol * sg_prev_enum_sym = isSgEnumSymbol(GetSymbolFromSymbolTable(prev_enum_decl));
    SgEnumDeclaration * sg_prev_enum_decl = sg_prev_enum_sym == NULL ? NULL : isSgEnumDeclaration(sg_prev_enum_sym->get_declaration());
    sg_prev_enum_decl = sg_prev_enum_decl == NULL ? NULL : isSgEnumDeclaration(sg_prev_enum_decl->get_definingDeclaration());

    SgEnumDeclaration * sg_enum_decl = SageBuilder::buildEnumDeclaration(name, SageBuilder::topScopeStack());
    *node = sg_enum_decl;

    if(enumDeclName.empty())
    {
      sg_enum_decl->set_isUnNamed(true);
    }

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

bool ClangToSageTranslator::VisitTemplateTypeParmDecl(clang::TemplateTypeParmDecl * template_type_parm_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitTemplateTypeParmDecl" << "\n";
    logger[DEBUG] << "VisitTemplateTypeParmDecl wasDeclaredWithTypename: " << template_type_parm_decl->wasDeclaredWithTypename() << "\n";
    logger[DEBUG] << "VisitTemplateTypeParmDecl hasDefaultArgument: " << template_type_parm_decl->hasDefaultArgument() << "\n";
#endif
    bool res = true;
    SgName typeName(template_type_parm_decl->getNameAsString());
    SgDeclarationScope* declScope = isSgDeclarationScope(SageBuilder::topScopeStack());
    SgNonrealDecl* nonrealDecl = SageBuilder::buildNonrealDecl(typeName, declScope, NULL);
    nonrealDecl->set_parent(declScope);
    SgNonrealType* type = nonrealDecl->get_type();
    SgTemplateParameter* param = SageBuilder::buildTemplateParameter(SgTemplateParameter::type_parameter, type);
    *node = param;
    return VisitTypeDecl(template_type_parm_decl, node) && res;
}

bool ClangToSageTranslator::VisitTypedefNameDecl(clang::TypedefNameDecl * typedef_name_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitTypedefNameDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME

    return VisitTypeDecl(typedef_name_decl, node) && res;
}

bool ClangToSageTranslator::VisitTypedefDecl(clang::TypedefDecl * typedef_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitTypedefDecl" << "\n";
#endif
    bool res = true;
    SgTypedefDeclaration * sg_typedef_decl;

    SgSymbol * sym = GetSymbolFromSymbolTable(typedef_decl);
    SgTypedefSymbol * tdef_sym = isSgTypedefSymbol(sym);

    // Pei-Hung (09/23/2022) if typedefType is referenced before first appearance
    // of TypedefDecl, tthe TypedefDecl is declared to fulfill the need for symbol
    // lookup in VisitTypedefType.  Skip declaration here if that's the case.
    if(tdef_sym != NULL)
    {
      sg_typedef_decl = tdef_sym->get_declaration(); 
      *node = sg_typedef_decl;

      return VisitTypedefNameDecl(typedef_decl, node) && res;
    }

    SgName name(typedef_decl->getNameAsString());
//    SgType * type = buildTypeFromQualifiedType(typedef_decl->getUnderlyingType());


    clang::QualType underlyingQualType = typedef_decl->getUnderlyingType();

    const clang::Type* underlyingType = underlyingQualType.getTypePtr();


    // Pei-Hung (06/01/2022) check if the declaration is considered embedded in Clang AST.
    // If it is embedded, no explicit SgDeclaration should be placed for ROSE AST.
    bool isembedded = false;
    // determine if the declaration associated with type (through getDecl()) has complete definition
    bool iscompleteDefined = false;
    // check if ElaboratedType is part of the type 
    bool hasElaboratedType = false;
    // if hasElaboratedType, check if declaration of this type owned by this occurrence of this type is also 
    // a definition
    bool isOwnedTagDeclADefinition = false;
    // checking is definition should be setup in ROSE AST
    bool isDefinitionaRequired = false; 

    // Adding check for EaboratedType and PointerType to retrieve base EnumType
    while((llvm::isa<clang::ElaboratedType>(underlyingType)) || (llvm::isa<clang::PointerType>(underlyingType)) || (llvm::isa<clang::ArrayType>(underlyingType)))
    {
       if(llvm::isa<clang::ElaboratedType>(underlyingType))
       {
         hasElaboratedType = true;
         underlyingQualType = ((clang::ElaboratedType *)underlyingType)->getNamedType();
         clang::TagDecl* ownedTagDecl = ((clang::ElaboratedType *)underlyingType)->getOwnedTagDecl();
         if(ownedTagDecl != nullptr)
         {
#if DEBUG_VISIT_DECL
            logger[DEBUG] << "ClangToSageTranslator::VisitTypedefDecl has ownedTagDecl " << ownedTagDecl->isThisDeclarationADefinition() << "\n";
#endif
            isOwnedTagDeclADefinition = ownedTagDecl->isThisDeclarationADefinition();
         }
       }
       else if(llvm::isa<clang::PointerType>(underlyingType))
       {
         underlyingQualType = ((clang::PointerType *)underlyingType)->getPointeeType();
       }
       else if(llvm::isa<clang::ArrayType>(underlyingType))
       {
         underlyingQualType = ((clang::ArrayType *)underlyingType)->getElementType();
       }
       underlyingType = underlyingQualType.getTypePtr();
    }

    if(llvm::isa<clang::EnumType>(underlyingType))
    {
       clang::EnumType* underlyingEnumType = (clang::EnumType*)underlyingType;
       clang::EnumDecl* enumDeclaration = underlyingEnumType->getDecl();
       isembedded = enumDeclaration->isEmbeddedInDeclarator();
       iscompleteDefined = enumDeclaration->isCompleteDefinition();
    }

    if(llvm::isa<clang::RecordType>(underlyingType))
    {
       clang::RecordType* underlyingRecordType = (clang::RecordType*)underlyingType;
       clang::RecordDecl* recordDeclaration = underlyingRecordType->getDecl();
       isembedded = recordDeclaration->isEmbeddedInDeclarator();
       iscompleteDefined = recordDeclaration->isCompleteDefinition();
    }

    if(hasElaboratedType)
    {
      isDefinitionaRequired = isOwnedTagDeclADefinition;
    }
    else
    {
      isDefinitionaRequired = iscompleteDefined;
    }

    SgType * sg_underlyingType = buildTypeFromQualifiedType(underlyingQualType);
    SgType * type = buildTypeFromQualifiedType(typedef_decl->getUnderlyingType());

    sg_typedef_decl = SageBuilder::buildTypedefDeclaration_nfi(name, type, SageBuilder::topScopeStack());

    // finding the bottom base type and check
    while(type->findBaseType() != type)
    {
      type = type->findBaseType();
      if(type == sg_underlyingType)
        break;
    }

#if DEBUG_VISIT_DECL
    logger[DEBUG] << "isDefinitionaRequired = " << isDefinitionaRequired << "\n";
    logger[DEBUG] << "hasElaboratedType = " << hasElaboratedType << "\n";
    logger[DEBUG] << "iscompleteDefined = " << iscompleteDefined << "\n";
    logger[DEBUG] << "isembedded = " << isembedded << "\n";
    logger[DEBUG] << "isOwnedTagDeclADefinition = " << isOwnedTagDeclADefinition << "\n";
#endif
// Pei-Hung (05/31/2022) set "bool_it->second = false" to avoid duplicated definition
    if (isSgClassType(type) && isDefinitionaRequired) {
        SgClassDeclaration* classDecl = isSgClassDeclaration(isSgClassType(type)->get_declaration());
        SgClassDeclaration* classDefDecl = isSgClassDeclaration(isSgClassType(type)->get_declaration()->get_definingDeclaration());
        if(isembedded && classDefDecl != nullptr && !isSgDeclarationStatement(classDefDecl->get_parent()))
        {
          classDefDecl->set_parent(sg_typedef_decl);
//          classDefDecl->set_isAutonomousDeclaration(false);
          sg_typedef_decl->set_declaration(classDefDecl);
          sg_typedef_decl->set_typedefBaseTypeContainsDefiningDeclaration(true);
        }

        std::map<SgClassType *, bool>::iterator bool_it = p_class_type_decl_first_see_in_type.find(isSgClassType(type));
        ROSE_ASSERT(bool_it != p_class_type_decl_first_see_in_type.end());
        if (bool_it->second) {
            sg_typedef_decl->set_declaration(isSgNamedType(type)->get_declaration()->get_definingDeclaration());
            sg_typedef_decl->set_typedefBaseTypeContainsDefiningDeclaration(true);
            bool_it->second = false;
        }
    }
    else if (isSgEnumType(type) && isDefinitionaRequired) {

// Pei-Hung (06/01/2022) Clang places a EnumDecl before TypedefDecl.  
// A SgEnumDeclaration for an  embedded EnumDecl is not attached to the scope but its parent node needs to be setup as the SgTypedefDeclaration

        SgEnumDeclaration* enumDecl = isSgEnumDeclaration(isSgEnumType(type)->get_declaration());
        SgEnumDeclaration* enumDefDecl = isSgEnumDeclaration(isSgEnumType(type)->get_declaration()->get_definingDeclaration());
        if(isembedded && enumDefDecl != nullptr && !isSgDeclarationStatement(enumDefDecl->get_parent()))
        {
          enumDefDecl->set_parent(sg_typedef_decl);
//          enumDefDecl->set_isAutonomousDeclaration(false);
          sg_typedef_decl->set_declaration(enumDefDecl);
          sg_typedef_decl->set_typedefBaseTypeContainsDefiningDeclaration(true);
        }

        std::map<SgEnumType *, bool>::iterator bool_it = p_enum_type_decl_first_see_in_type.find(isSgEnumType(type));
        ROSE_ASSERT(bool_it != p_enum_type_decl_first_see_in_type.end());
        if (bool_it->second) {
            sg_typedef_decl->set_declaration(isSgEnumType(type)->get_declaration()->get_definingDeclaration());
            sg_typedef_decl->set_typedefBaseTypeContainsDefiningDeclaration(true);
            bool_it->second = false;
        }
    }

    *node = sg_typedef_decl;

    return VisitTypedefNameDecl(typedef_decl, node) && res;
}

bool ClangToSageTranslator::VisitTypeAliasDecl(clang::TypeAliasDecl * type_alias_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitTypeAliasDecl" << "\n";
#endif  
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitTypedefNameDecl(type_alias_decl, node) && res;
}

bool ClangToSageTranslator::VisitUnresolvedUsingTypenameDecl(clang::UnresolvedUsingTypenameDecl * unresolved_using_type_name_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitUnresolvedUsingTypenameDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME

    return VisitTypeDecl(unresolved_using_type_name_decl, node) && res;
}

bool ClangToSageTranslator::VisitUsingDecl(clang::UsingDecl * using_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitUsingDecl" << "\n";
#endif  
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME

    return VisitNamedDecl(using_decl, node) && res;
}

bool ClangToSageTranslator::VisitUsingDirectiveDecl(clang::UsingDirectiveDecl * using_directive_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitUsingDirectiveDecl" << "\n";
#endif  
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME

    return VisitNamedDecl(using_directive_decl, node) && res;
}

bool ClangToSageTranslator::VisitUsingPackDecl(clang::UsingPackDecl * using_pack_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitUsingPackDecl" << "\n";
#endif  
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME

    return VisitNamedDecl(using_pack_decl, node) && res;
}

bool ClangToSageTranslator::VisitUsingShadowDecl(clang::UsingShadowDecl * using_shadow_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitUsingShadowDecl" << "\n";
#endif  
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME

    return VisitNamedDecl(using_shadow_decl, node) && res;
}

bool ClangToSageTranslator::VisitConstructorUsingShadowDecl(clang::ConstructorUsingShadowDecl * constructor_using_shadow_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitConstructorUsingShadowDecl" << "\n";
#endif  
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME

    return VisitNamedDecl(constructor_using_shadow_decl, node) && res;
}

bool ClangToSageTranslator::VisitValueDecl(clang::ValueDecl * value_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitValueDecl" << "\n";
#endif  
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME

    return VisitNamedDecl(value_decl, node) && res;
}

bool ClangToSageTranslator::VisitBindingDecl(clang::BindingDecl * binding_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitBindingDecl" << "\n";
#endif  
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME

    return VisitValueDecl(binding_decl, node) && res;
}
    
bool ClangToSageTranslator::VisitDeclaratorDecl(clang::DeclaratorDecl * declarator_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitDeclaratorDecl" << "\n";
#endif  
    bool res = true;

    ROSE_ASSERT(FAIL_FIXME == 0); // FIXME

    return VisitValueDecl(declarator_decl, node) && res;
}
    

bool ClangToSageTranslator::VisitFieldDecl(clang::FieldDecl * field_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitFieldDecl" << "\n";
    logger[DEBUG] << "parent: " << field_decl->getParent() << "\n";
#endif  
    bool res = true;
    
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitFieldDecl name:" <<field_decl->getNameAsString() <<  "\n";
    logger[DEBUG] << "isAnonymousStructOrUnion() " << field_decl->isAnonymousStructOrUnion() << "\n";
#endif

    SgName name(field_decl->getNameAsString());

    clang::RecordDecl* enclosingRecordDecl = field_decl->getParent();
    const clang::Type* encolsingRecordDeclType = enclosingRecordDecl->getTypeForDecl();

    clang::QualType fieldQualType = field_decl->getType();

    const clang::Type* fieldType = fieldQualType.getTypePtr();

    // Pei-Hung (06/01/2022) check if the declaration is considered embedded in Clang AST.
    // If it is embedded, no explicit SgDeclaration should be placed for ROSE AST.
    bool isembedded = false;
    bool iscompleteDefined = false;
    // check if ElaboratedType is part of the type 
    bool hasElaboratedType = false;
    // if hasElaboratedType, check if declaration of this type owned by this occurrence of this type is also 
    // a definition
    bool isOwnedTagDeclADefinition = false;
    // checking is definition should be setup in ROSE AST
    bool isDefinitionaRequired = false; 
    bool isAnonymousStructOrUnion = false;

    // Adding check for EaboratedType and PointerType to retrieve base EnumType
    // Removing PointerType here before finding a better implementation to handle pointer
    while((llvm::isa<clang::ElaboratedType>(fieldType)) || (llvm::isa<clang::PointerType>(fieldType)) || (llvm::isa<clang::ArrayType>(fieldType)))
    {
       if(llvm::isa<clang::ElaboratedType>(fieldType))
       {
         hasElaboratedType = true;
         fieldQualType = ((clang::ElaboratedType *)fieldType)->getNamedType();
         clang::TagDecl* ownedTagDecl = ((clang::ElaboratedType *)fieldType)->getOwnedTagDecl();
         if(ownedTagDecl != nullptr)
         {
            logger[DEBUG] << "ClangToSageTranslator::VisitFieldDecl ownedTagDecl " << ownedTagDecl->isThisDeclarationADefinition() << "\n";
            isOwnedTagDeclADefinition = ownedTagDecl->isThisDeclarationADefinition();
         }
       }
       else if(llvm::isa<clang::PointerType>(fieldType))
       {
         fieldQualType = ((clang::PointerType *)fieldType)->getPointeeType();
       }
       else if(llvm::isa<clang::ArrayType>(fieldType))
       {
         fieldQualType = ((clang::ArrayType *)fieldType)->getElementType();
       }
       fieldType = fieldQualType.getTypePtr();
    }

#if DEBUG_VISIT_DECL
    logger[DEBUG] << "fieldType: " << fieldType << " enclosingRecordDeclType: " << encolsingRecordDeclType <<  "\n";
#endif

    if(llvm::isa<clang::EnumType>(fieldType))
    {
       clang::EnumType* underlyingEnumType = (clang::EnumType*)fieldType;
       clang::EnumDecl* enumDeclaration = underlyingEnumType->getDecl();
       isembedded = enumDeclaration->isEmbeddedInDeclarator();
       iscompleteDefined = enumDeclaration->isCompleteDefinition();
    }

    if(llvm::isa<clang::RecordType>(fieldType))
    {
       clang::RecordType* underlyingRecordType = (clang::RecordType*)fieldType;
       clang::RecordDecl* recordDeclaration = underlyingRecordType->getDecl();
       isembedded = recordDeclaration->isEmbeddedInDeclarator();
       iscompleteDefined = recordDeclaration->isCompleteDefinition();
    }

    if(hasElaboratedType)
    {
      isDefinitionaRequired = isOwnedTagDeclADefinition;
    }
    else
    {
// This might not be the  precise info for set_isAutonomousDeclaration
      isDefinitionaRequired = iscompleteDefined;
    }

    isAnonymousStructOrUnion = field_decl->isAnonymousStructOrUnion();

    SgType * sg_fieldType = buildTypeFromQualifiedType(fieldQualType);
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

    if(isAnonymousStructOrUnion)
    {
        if (isSgClassType(type) && isDefinitionaRequired) {
//            SgClassDeclaration* classDecl = isSgClassDeclaration(isSgClassType(type)->get_declaration());
            SgClassDeclaration* classDefDecl = isSgClassDeclaration(isSgClassType(type)->get_declaration()->get_definingDeclaration());
            *node = classDefDecl;
        }
        else if (isSgEnumType(type) && isDefinitionaRequired) {
//            SgEnumDeclaration* enumDecl = isSgEnumDeclaration(isSgEnumType(type)->get_declaration());
            SgEnumDeclaration* enumDefDecl = isSgEnumDeclaration(isSgEnumType(type)->get_declaration()->get_definingDeclaration());
            *node = enumDefDecl;
        }
    }
    else
    {
      // Cannot use 'SageBuilder::buildVariableDeclaration' because of anonymous field
        // *node = SageBuilder::buildVariableDeclaration(name, type, init, SageBuilder::topScopeStack());
      // Build it by hand...
        SgVariableDeclaration * var_decl = new SgVariableDeclaration(name, type, init);
       
     // Pei-Hung (09/27/2022) isAssociatedWithDeclarationList should be set to handle multiple variables in
     // a single declaration list.
     // As Clang does not have concept such as declaration list, we might need revision to handle this in future.
        var_decl->set_isAssociatedWithDeclarationList(true);    
 
        // finding the bottom base type and check
        while(type->findBaseType() != type)
        {
          type = type->findBaseType();
          if(type == sg_fieldType)
            break;
        }
     
        if (isSgClassType(type) && isDefinitionaRequired) {
            SgClassDeclaration* classDecl = isSgClassDeclaration(isSgClassType(type)->get_declaration());
            SgClassDeclaration* classDefDecl = isSgClassDeclaration(classDecl->get_definingDeclaration());
            classDecl->set_isAutonomousDeclaration(false);
            classDefDecl->set_isAutonomousDeclaration(false);
            // Pei-Hung (08/29/23) When the field has a pointer type to the enclosing class (e.g. test2003_08.C),
            // the following setup should be skipped.  Implementation details should be handled under visitVarDecl (at least for tux2003_08.C).
            if(fieldType != encolsingRecordDeclType && isembedded && classDefDecl != nullptr && !isSgDeclarationStatement(classDefDecl->get_parent()))
            {
              classDefDecl->set_parent(var_decl);
//              classDefDecl->set_isAutonomousDeclaration(false);
              var_decl->set_baseTypeDefiningDeclaration(classDefDecl);
              var_decl->set_variableDeclarationContainsBaseTypeDefiningDeclaration(true);
            }
     
            std::map<SgClassType *, bool>::iterator bool_it = p_class_type_decl_first_see_in_type.find(isSgClassType(type));
            ROSE_ASSERT(bool_it != p_class_type_decl_first_see_in_type.end());
            if (bool_it->second) {
//                var_decl->set_baseTypeDefiningDeclaration(isSgNamedType(type)->get_declaration()->get_definingDeclaration());
                var_decl->set_variableDeclarationContainsBaseTypeDefiningDeclaration(true);
                bool_it->second = false;
            }
        }
        else if (isSgEnumType(type) && isDefinitionaRequired) {
//            SgEnumDeclaration* enumDecl = isSgEnumDeclaration(isSgEnumType(type)->get_declaration());
            SgEnumDeclaration* enumDefDecl = isSgEnumDeclaration(isSgEnumType(type)->get_declaration()->get_definingDeclaration());
            if(isembedded && enumDefDecl != nullptr && !isSgDeclarationStatement(enumDefDecl->get_parent()))
            {
              enumDefDecl->set_parent(var_decl);
//              enumDefDecl->set_isAutonomousDeclaration(false);
              var_decl->set_baseTypeDefiningDeclaration(enumDefDecl);
              var_decl->set_variableDeclarationContainsBaseTypeDefiningDeclaration(true);
            }
     
            std::map<SgEnumType *, bool>::iterator bool_it = p_enum_type_decl_first_see_in_type.find(isSgEnumType(type));
            ROSE_ASSERT(bool_it != p_enum_type_decl_first_see_in_type.end());
            if (bool_it->second) {
                var_decl->set_baseTypeDefiningDeclaration(isSgEnumType(type)->get_declaration()->get_definingDeclaration());
                var_decl->set_variableDeclarationContainsBaseTypeDefiningDeclaration(true);
                bool_it->second = false;
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
        }
    return VisitDeclaratorDecl(field_decl, node) && res; 
}

bool ClangToSageTranslator::VisitFunctionDecl(clang::FunctionDecl * function_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitFunctionDecl" << "\n";
    logger[DEBUG] << "ClangToSageTranslator::VisitFunctionDecl " << function_decl->getNameInfo().getName().getAsString() << "\n";
    logger[DEBUG] << "ClangToSageTranslator::VisitFunctionDecl isThisDeclarationADefinition " << function_decl->isThisDeclarationADefinition() << "\n";
#endif
    bool res = true;

    // FIXME: There is something weird here when try to Traverse a function reference in a recursive function (when first Traverse is not complete)
    //        It seems that it tries to instantiate the decl inside the function...
    //        It may be faster to recode from scratch...
    //   If I am not wrong this have been fixed....

    SgName name(function_decl->getNameAsString());

    clang::QualType funcQualType = function_decl->getType();

    const clang::Type* funcType = funcQualType.getTypePtr();

    const clang::FunctionProtoType* funcProtoType = (llvm::isa<clang::FunctionProtoType>(funcType)) ? (clang::FunctionProtoType*)funcType : nullptr;

    bool diffInProtoType = false;

    // Finding the scope of enclosing DeclContext
    clang::DeclContext* declContext = function_decl->getDeclContext();
    SgScopeStatement * scope = SageBuilder::topScopeStack();
    SgScopeStatement * declContextScope = scope; 
    std::map<clang::DeclContext *, SgScopeStatement *>::iterator it = p_decl_context_map.find(declContext);
    if (it != p_decl_context_map.end()) {
      declContextScope = it->second;
    }




#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitFunctionDecl name:" << name.getString() << "\n";
#endif

    SgType * ret_type = buildTypeFromQualifiedType(function_decl->getReturnType());

    SgFunctionParameterList * param_list = SageBuilder::buildFunctionParameterList_nfi();
    applySourceRange(param_list, function_decl->getParametersSourceRange()); // FIXME find the good SourceRange (should be stored by Clang...)

    if(funcProtoType != nullptr && funcProtoType->getNumParams() != function_decl->getNumParams())
        diffInProtoType = true;

    SgDeclarationScope* declScope = SageBuilder::buildDeclarationScope();
    declScope->set_parent(declContextScope);
    //declScope->set_parent(SageBuilder::topScopeStack());
    SageBuilder::pushScopeStack(declScope);
    
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitFunctionDecl NumParams:" << function_decl->getNumParams() << "\n";
#endif

    for (unsigned i = 0; i < function_decl->getNumParams(); i++) {
        if(funcProtoType != nullptr && function_decl->getParamDecl(i)->getType() != funcProtoType->getParamType(i))
        {
#if DEBUG_VISIT_DECL
            std::cout << "Func arg type :" << function_decl->getParamDecl(i)->getType().getAsString() << " funcProtoType arg type:" << funcProtoType->getParamType(i).getAsString() << "\n";
#endif
            diffInProtoType = true;
        } 
        SgNode * tmp_init_name = Traverse(function_decl->getParamDecl(i));
        SgInitializedName * init_name = isSgInitializedName(tmp_init_name);

        // Pei-Hung (05/09/2022) Need to setup set_needs_definitions to SgInitializedName when the
        // Enum or Class type is actually declared in a function prototype scope
        if(isSgEnumType(init_name->get_type()) || isSgClassType(init_name->get_type()))
        {
          SgNamedType* namedType = isSgNamedType(init_name->get_type());
          SgDeclarationStatement* namedTypeDecl = isSgDeclarationStatement(namedType->get_declaration());
          SgDeclarationStatement* definingDecl = namedTypeDecl->get_definingDeclaration(); 
          if(definingDecl != NULL)
          {
            SgDeclarationStatement* definingNamedTypeDecl = isSgDeclarationStatement(definingDecl);
            SgScopeStatement* definitionEnclosingScope = definingNamedTypeDecl->get_scope();
            // case 1: definition is under SgDeclarationScope
            if(isSgScopeStatement(definitionEnclosingScope) == SageBuilder::topScopeStack()) 
            {
              init_name->set_needs_definitions(true);
            }
            // case 2: definition is at other scope but not in the SgDeclarationStatementPtrList from that scope
            // e.g. test2018_15.c, test2018_13.c 
            else
            {
              SgDeclarationStatementPtrList& declList = definitionEnclosingScope->getDeclarationList();
              if(std::find(declList.begin(), declList.end(), definingNamedTypeDecl) == declList.end())
              {
                init_name->set_needs_definitions(true);
              }
            }
          } 
        }

        if (tmp_init_name != NULL && init_name == NULL) {
            logger[WARN] << "Runtime error: tmp_init_name != NULL && init_name == NULL" << "\n";
            res = false;
            continue;
        }

        param_list->append_arg(init_name);
    }

    SageBuilder::popScopeStack();

    if (function_decl->isVariadic()) {
        SgName empty = "";
        SgType * ellipses_type = SgTypeEllipse::createType();
        param_list->append_arg(SageBuilder::buildInitializedName_nfi(empty, ellipses_type, NULL));
    }

    SgFunctionDeclaration * sg_function_decl;

    //if (function_decl->isThisDeclarationADefinition() && function_decl->hasBody()) {
    if (function_decl->isThisDeclarationADefinition()) {
        if(llvm::isa<clang::CXXMethodDecl>(function_decl))
        {
          SgClassDeclaration* cxxRecordDecl = NULL;
          SgScopeStatement* methodScope = NULL;
          if(llvm::isa<clang::CXXConstructorDecl>(function_decl))
          {
            clang::CXXRecordDecl* parentClassDecl = static_cast<clang::CXXConstructorDecl*>(function_decl)->getParent();
            SgClassDeclaration* CxxRecordDeclaration  = isSgClassDeclaration(Traverse(parentClassDecl));
            name = CxxRecordDeclaration->get_name(); 
          }
          if(p_decl_translation_map.find(((clang::CXXMethodDecl *)function_decl)->getParent()) != p_decl_translation_map.end())
          {
             cxxRecordDecl = isSgClassDeclaration(Traverse(((clang::CXXMethodDecl *)function_decl)->getParent()));
             logger[DEBUG] << "defining method is in CxxRecordDecl: " << cxxRecordDecl << "\n";
             methodScope = isSgScopeStatement(cxxRecordDecl->get_definition());
          }
          sg_function_decl = SageBuilder::buildDefiningMemberFunctionDeclaration(name, ret_type, param_list, methodScope);
        }
        else
          sg_function_decl = SageBuilder::buildDefiningFunctionDeclaration(name, ret_type, param_list, NULL);
        sg_function_decl->set_definingDeclaration(sg_function_decl);

        if (function_decl->isVariadic()) {
            sg_function_decl->hasEllipses();
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

        SgNode * tmp_body;
        if (!function_decl->hasBody()) {
            logger[DEBUG] << "Defining function declaration without body..." << "\n";
            //res = false;
            tmp_body = SageBuilder::buildBasicBlock();
        }
        else
        {
            tmp_body = Traverse(function_decl->getBody());
        }
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
/*
        SgFunctionDeclaration * first_decl;
        if (function_decl->isFirstDecl()) {
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
//            first_decl = SageBuilder::buildNondefiningFunctionDeclaration(sg_function_decl, NULL,  NULL);
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
*/ 
        // Pei-Hung (06/27/22) This seems to be the way to get test2004_21.c unprarsed properly
        // by checking if the functionProtoType has different argument types.

        if(diffInProtoType)
        {
            sg_function_decl->set_parameterList_syntax(param_list);
            sg_function_decl->set_type_syntax_is_available(true);
            sg_function_decl->set_oldStyleDefinition(true);
        }
    }
    else {
        if(llvm::isa<clang::CXXMethodDecl>(function_decl))
        {
          SgClassDeclaration* cxxRecordDecl = NULL;
          SgScopeStatement* methodScope = NULL;
          if(p_decl_translation_map.find(((clang::CXXMethodDecl *)function_decl)->getParent()) != p_decl_translation_map.end())
          {
             cxxRecordDecl = isSgClassDeclaration(Traverse(((clang::CXXMethodDecl *)function_decl)->getParent()));
             logger[DEBUG] << "Nondefining method is in CxxRecordDecl: " << cxxRecordDecl << "\n";
             methodScope = isSgScopeStatement(cxxRecordDecl->get_definition());
          }
          sg_function_decl = SageBuilder::buildNondefiningMemberFunctionDeclaration(name, ret_type, param_list, methodScope);
        }
        else
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

    sg_function_decl->set_declarationScope(declScope);
    declScope->set_parent(sg_function_decl);

    //Pei-Hung (09/27/2022) setup linkage
    if(function_decl->isExternC())
    {
      sg_function_decl->get_declarationModifier().get_storageModifier().setExtern();
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

    // Pei-Hung (06/16/22) added "extern" modifier
    bool hasExternalStorage = function_decl->isLocalExternDecl();
    if(hasExternalStorage)
    {
      sg_function_decl->get_declarationModifier().get_storageModifier().setExtern();
    }

    applySourceRange(sg_function_decl, function_decl->getSourceRange());
    *node = sg_function_decl;

    return VisitDeclaratorDecl(function_decl, node) && res;
}

bool ClangToSageTranslator::VisitCXXDeductionGuideDecl(clang::CXXDeductionGuideDecl * cxx_deduction_guide_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitCXXDeductionGuideDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitFunctionDecl(cxx_deduction_guide_decl, node) && res;
}

bool ClangToSageTranslator::VisitCXXMethodDecl(clang::CXXMethodDecl * cxx_method_decl, SgNode ** node) {
    SgName name(cxx_method_decl->getNameAsString());
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitCXXMethodDecl " << name.getString() << "\n";
    logger[DEBUG] << "ClangToSageTranslator::VisitCXXMethodDecl isStatic = " << cxx_method_decl->isStatic() << "\n";
    logger[DEBUG] << "ClangToSageTranslator::VisitCXXMethodDecl isInstance = " << cxx_method_decl->isInstance() << "\n";
    logger[DEBUG] << "ClangToSageTranslator::VisitCXXMethodDecl isVolatile = " << cxx_method_decl->isVolatile() << "\n";
    logger[DEBUG] << "ClangToSageTranslator::VisitCXXMethodDecl isVirtual = " << cxx_method_decl->isVirtual() << "\n";
    logger[DEBUG] << "ClangToSageTranslator::VisitCXXMethodDecl isCopyAssignmentOperator = " << cxx_method_decl->isCopyAssignmentOperator() << "\n";
    logger[DEBUG] << "ClangToSageTranslator::VisitCXXMethodDecl isMoveAssignmentOperator = " << cxx_method_decl->isMoveAssignmentOperator() << "\n";
    logger[DEBUG] << "ClangToSageTranslator::VisitCXXMethodDecl hasInlineBody = " << cxx_method_decl->hasInlineBody() << "\n";
    logger[DEBUG] << "ClangToSageTranslator::VisitCXXMethodDecl isLambdaStaticInvoker = " << cxx_method_decl->isLambdaStaticInvoker() << "\n";
#endif
    bool res = VisitFunctionDecl(cxx_method_decl, node);



//    ROSE_ASSERT(FAIL_TODO == 0); // TODO
//    SgClassDeclaration* CxxRecordDeclaration = isSgClassDeclaration(Traverse(cxx_method_decl->getParent()));
//    ROSE_ASSERT(CxxRecordDeclaration); 
//    CxxRecordDeclaration->get_definition()->append_member(functionDecl);

    return res;
}

bool ClangToSageTranslator::VisitCXXConstructorDecl(clang::CXXConstructorDecl * cxx_constructor_decl, SgNode ** node) {
    bool res = true;
    SgName name(cxx_constructor_decl->getNameAsString());
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitCXXConstructorDecl name:" << name.getString() << "\n";
    logger[DEBUG] << "isDefaultConstructor = " << cxx_constructor_decl->isDefaultConstructor() << "\n";
    logger[DEBUG] << "isCopyConstructor = " << cxx_constructor_decl->isCopyConstructor() << "\n";
    logger[DEBUG] << "isMoveConstructor = " << cxx_constructor_decl->isMoveConstructor() << "\n";
    logger[DEBUG] << "isInheritingConstructor = " << cxx_constructor_decl->isInheritingConstructor() << "\n";
    logger[DEBUG] << "isExplicit = " << cxx_constructor_decl->isExplicit() << "\n";
    logger[DEBUG] << "isImplicit = " << cxx_constructor_decl->isImplicit() << "\n";
#endif

    // getting ctorInitializer

//
//    SgFunctionParameterList * param_list = SageBuilder::buildFunctionParameterList_nfi();

//    SgMemberFunctionDeclaration* memberFunctionDecl = SageBuilder::buildDefiningMemberFunctionDeclaration(name,ret_type, param_list, SageBuilder::topScopeStack());

//    ROSE_ASSERT(FAIL_TODO == 0); // TODO

//    *node = memberFunctionDecl;
    res = VisitCXXMethodDecl(cxx_constructor_decl, node);
    SgMemberFunctionDeclaration* cxxConstructorDecl = isSgMemberFunctionDeclaration(*node);
    SgMemberFunctionDeclaration* cxxDefiningConstructorDecl = isSgMemberFunctionDeclaration(cxxConstructorDecl->get_definingDeclaration());
    cxxConstructorDecl->get_specialFunctionModifier().setConstructor();

    // apply ctorInitializer
    if(cxx_constructor_decl->getNumCtorInitializers() != 0 && cxxDefiningConstructorDecl != NULL)
    {
      SgCtorInitializerList* ctorInitializerList = SageBuilder::buildCtorInitializerList_nfi();
      clang::CXXConstructorDecl::init_iterator initializer;
      unsigned cnt = 0;
      for (initializer = cxx_constructor_decl->init_begin(); initializer != cxx_constructor_decl->init_end(); initializer++) 
      {
        cnt++;
  #if DEBUG_VISIT_DECL
        logger[DEBUG] << "isBaseInitializer = " << (*initializer)->isBaseInitializer() << "\n";
        logger[DEBUG] << "isMemberInitializer = " << (*initializer)->isMemberInitializer() << "\n";
        logger[DEBUG] << "isAnyMemberInitializer  = " << (*initializer)->isAnyMemberInitializer() << "\n";
        logger[DEBUG] << "isIndirectMemberInitializer = " << (*initializer)->isIndirectMemberInitializer() << "\n";
  #endif
        if((*initializer)->isMemberInitializer())
        {
           clang::FieldDecl * field_decl = (*initializer)->getMember();
           SgName fieldName(field_decl->getNameAsString());
           SgVariableDeclaration* fieldMemberDecl = isSgVariableDeclaration(Traverse(field_decl));
           SgInitializedName* fieldInitializedName = fieldMemberDecl->get_decl_item(fieldName);
           SgType* fieldType = fieldInitializedName->get_type();

           SgExpression* initExpr = isSgExpression(Traverse((*initializer)->getInit())); 
           SgInitializer* sgCtorInitializer = SageBuilder::buildAssignInitializer_nfi(initExpr, fieldType);
           SgInitializedName* sgCtorInitializedName = SageBuilder::buildInitializedName(fieldName,fieldType, sgCtorInitializer);
           applySourceRange(sgCtorInitializedName, (*initializer)->getSourceRange());
           sgCtorInitializer->set_parent(sgCtorInitializedName);
           applySourceRange(sgCtorInitializer, ((*initializer)->getInit())->getSourceRange());
           ctorInitializerList->append_ctor_initializer(sgCtorInitializedName);
           sgCtorInitializedName->set_parent(ctorInitializerList); 
           sgCtorInitializedName->set_scope(SageBuilder::topScopeStack()); 
        }
      }
      cxxDefiningConstructorDecl->set_CtorInitializerList(ctorInitializerList);
      ctorInitializerList->set_parent(cxxDefiningConstructorDecl);
      ctorInitializerList->set_definingDeclaration(ctorInitializerList);
      ctorInitializerList->set_firstNondefiningDeclaration(ctorInitializerList);
    }
    if(cxx_constructor_decl->isDefaultConstructor())
    {
#if DEBUG_VISIT_DECL
      logger[DEBUG] << "set as Default constructor\n";
#endif
      cxxConstructorDecl->get_functionModifier().setDefault(); 
    }

    return res;
}

bool ClangToSageTranslator::VisitCXXConversionDecl(clang::CXXConversionDecl * cxx_conversion_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitCXXConversionDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitCXXMethodDecl(cxx_conversion_decl, node) && res;
}

bool ClangToSageTranslator::VisitCXXDestructorDecl(clang::CXXDestructorDecl * cxx_destructor_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitCXXDestructorDecl" << "\n";
#endif
    bool res = true;

    //ROSE_ASSERT(FAIL_TODO == 0); // TODO

    res = VisitCXXMethodDecl(cxx_destructor_decl, node);
    SgMemberFunctionDeclaration* cxxDestructorDecl = isSgMemberFunctionDeclaration(*node);
    cxxDestructorDecl->get_specialFunctionModifier().setDestructor();
    return res;
}

bool ClangToSageTranslator::VisitMSPropertyDecl(clang::MSPropertyDecl * ms_property_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitMSPropertyDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitDeclaratorDecl(ms_property_decl, node) && res;
}

bool ClangToSageTranslator::VisitNonTypeTemplateParmDecl(clang::NonTypeTemplateParmDecl * non_type_template_param_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitNonTypeTemplateParmDecl" << "\n";
#endif
    bool res = true;

    SgType * type = buildTypeFromQualifiedType(non_type_template_param_decl->getType());
    SgTemplateParameter* param = SageBuilder::buildTemplateParameter(SgTemplateParameter::nontype_parameter, type);
    *node = param;
    return VisitDeclaratorDecl(non_type_template_param_decl, node) && res;
}

bool ClangToSageTranslator::VisitVarDecl(clang::VarDecl * var_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitVarDecl " << var_decl->getNameAsString() << "\n";
    logger[DEBUG] << "isStaticLocal " << var_decl->isStaticLocal() << "\n";
    logger[DEBUG] << "isStaticDataMember " << var_decl->isStaticDataMember() << "\n";
#endif
    bool res = true;

  // Create the SAGE node: SgVariableDeclaration

    SgName name(var_decl->getNameAsString());

    clang::QualType varQualType = var_decl->getType();

    const clang::Type* varType = varQualType.getTypePtr();

    // Pei-Hung (06/01/2022) check if the declaration is considered embedded in Clang AST.
    // If it is embedded, no explicit SgDeclaration should be placed for ROSE AST.
    bool isembedded = false;
    bool iscompleteDefined = false;
    // check if ElaboratedType is part of the type 
    bool hasElaboratedType = false;
    // if hasElaboratedType, check if declaration of this type owned by this occurrence of this type is also 
    // a definition
    bool isOwnedTagDeclADefinition = false;
    // checking is definition should be setup in ROSE AST
    bool isDefinitionaRequired = false; 

/*  (09/16/2022) Pei-Hung copied Definition from EDG: 
    TRUE if this type entry represents a class, struct,
    union, or enum and its primary source sequence
    entry refers to a declaration that is not part
    of the declaration of another entity.  For instance,
      class A { int i; };    // An "autonomous" decl
      class B { int i; } b;  // Not "autonomous"
    The flag would be set TRUE for A but not for B
    since the latter's definition is part of the
    declaration of variable b.  Also TRUE for
    anonymous unions.  Also TRUE for all classes
    produced by a template instantiation, no matter
    what triggers the instantiation. 
*/
    bool isAutonomousDeclaration = true; 

    // Adding check for EaboratedType and PointerType to retrieve base EnumType
    //while((varType->getTypeClass() == clang::Type::Elaborated) || (varType->getTypeClass() == clang::Type::Pointer) || (varType->getTypeClass() == clang::Type::Array))
    while((llvm::isa<clang::ElaboratedType>(varType)) || (llvm::isa<clang::PointerType>(varType)) || (llvm::isa<clang::ArrayType>(varType)))
    {
       if(llvm::isa<clang::ElaboratedType>(varType))
       {
         hasElaboratedType = true;
         varQualType = ((clang::ElaboratedType *)varType)->getNamedType();
         clang::TagDecl* ownedTagDecl = ((clang::ElaboratedType *)varType)->getOwnedTagDecl();
         if(ownedTagDecl != nullptr)
         {
#if DEBUG_VISIT_DECL
            logger[DEBUG] << "ownedTagDecl " << ownedTagDecl->isThisDeclarationADefinition() << "\n";
#endif
            isOwnedTagDeclADefinition = ownedTagDecl->isThisDeclarationADefinition();
         }
       }
       else if(llvm::isa<clang::PointerType>(varType))
       {
         varQualType = ((clang::PointerType *)varType)->getPointeeType();
       }
       else if(llvm::isa<clang::ArrayType>(varType))
       {
         varQualType = ((clang::ArrayType *)varType)->getElementType();
       }
       varType = varQualType.getTypePtr();
    }

    if(llvm::isa<clang::EnumType>(varType))
    {
       clang::EnumType* underlyingEnumType = (clang::EnumType*)varType;
       clang::EnumDecl* enumDeclaration = underlyingEnumType->getDecl();
       isembedded = enumDeclaration->isEmbeddedInDeclarator();
       iscompleteDefined = enumDeclaration->isCompleteDefinition();
       isAutonomousDeclaration = false;
    }

    if(llvm::isa<clang::RecordType>(varType))
    {
       clang::RecordType* underlyingRecordType = (clang::RecordType*)varType;
       clang::RecordDecl* recordDeclaration = underlyingRecordType->getDecl();
       isembedded = recordDeclaration->isEmbeddedInDeclarator();
       iscompleteDefined = recordDeclaration->isCompleteDefinition();
       isAutonomousDeclaration = false;
    }

    if(hasElaboratedType)
    {
      isDefinitionaRequired = isOwnedTagDeclADefinition;
    }
    else
    {
// This might not be the  precise info for set_isAutonomousDeclaration
      isDefinitionaRequired = iscompleteDefined;
    }

#if DEBUG_VISIT_DECL
            logger[DEBUG] << "isDefinitionaRequired by Clang " << isDefinitionaRequired << "\n";
            logger[DEBUG] << "isembedded " << isembedded << "\n";
#endif

    SgType * sg_varType = buildTypeFromQualifiedType(varQualType);
    SgType * type = buildTypeFromQualifiedType(var_decl->getType());

    bool isStaticDataMember = var_decl->isStaticDataMember();

//    SgVariableDeclaration * sg_var_decl = new SgVariableDeclaration(name, type, init); // scope: obtain from the scope stack.
   // Pei-Hung (09/01/2022) In test2022_3.c, the variable symbol needs to be avaiable before processing the RHS.
   // calling buildVariableDeclaration_nfi to get the symbol in place.
   SgVariableDeclaration * sg_var_decl = NULL;
   // Pei-Hung (09/29/23) The definition of a staic data member needs to call
   //  set_prev_decl_item to point to its first static data member declaration inside the class. 
   // buildVariableDeclaration_nfi will take care of the details by looking up the SgSymbol in
   // the symbol table of the class. 
   if(isStaticDataMember && var_decl->getPreviousDecl() != NULL)
   {
     clang::VarDecl* prevDecl = var_decl->getPreviousDecl();
     SgVariableDeclaration* sgPrevDecl = isSgVariableDeclaration(Traverse(prevDecl));
     ROSE_ASSERT(sgPrevDecl);
     sg_var_decl = SageBuilder::buildVariableDeclaration_nfi(name,type, NULL , SageInterface::getScope(sgPrevDecl));
   }
   else
   {
     sg_var_decl = SageBuilder::buildVariableDeclaration_nfi(name,type, NULL ,SageBuilder::topScopeStack());
   }

   // Pei-Hung (09/27/2022) isAssociatedWithDeclarationList should be set to handle multiple variables in
   // a single declaration list.
   // As Clang does not have concept such as declaration list, we might need revision to handle this in future.
   sg_var_decl->set_isAssociatedWithDeclarationList(true);    
 
    clang::Expr * init_expr = var_decl->getInit();
    // Clang can synthesize a CXXConstructExpr for an out-of-class static
    // record-array definition without a source initializer. Preserve it as an
    // uninitialized declaration instead of unparsing `T array[N] = T()`.
    const clang::Type * arrayElementType = NULL;
    if (const clang::ArrayType * arrayType = llvm::dyn_cast<clang::ArrayType>(var_decl->getType().getTypePtr())) {
      arrayElementType = arrayType->getElementType().getTypePtr();
      if (const clang::ElaboratedType * elaboratedType = llvm::dyn_cast<clang::ElaboratedType>(arrayElementType))
        arrayElementType = elaboratedType->getNamedType().getTypePtr();
    }
    if(isStaticDataMember && var_decl->getPreviousDecl() != NULL
       && arrayElementType != NULL
       && llvm::isa<clang::RecordType>(arrayElementType)
       && llvm::isa<clang::CXXConstructExpr>(init_expr))
    {
      init_expr = NULL;
    }
     SgNode * tmp_init = init_expr != NULL ? Traverse(init_expr) : NULL;
    SgExpression * expr = isSgExpression(tmp_init);
    if (tmp_init != NULL && expr == NULL) {
        logger[WARN] << "Runtime error: not a SgInitializer..." << "\n"; // TODO
        res = false;
    }
    SgExprListExp * expr_list_expr = isSgExprListExp(expr);

    SgInitializer * init = NULL;
    if (isSgInitializer(tmp_init))
    {
        init = isSgInitializer(tmp_init);
    }
    else if (expr_list_expr != NULL)
    {
        init = SageBuilder::buildAggregateInitializer(expr_list_expr, type);
    }
    else if (expr != NULL)
    {
        init = SageBuilder::buildAssignInitializer_nfi(expr, expr->get_type());
    }
    if (init != NULL)
    {
        //Pei-Hung (07/12/2023): 
        //applySourceRange should be set whenever the SgInitializer was just created.
        //Otherwise, it could cause overwrite the setting done in setCompilerGeneratedFileInfo.
        if(!llvm::isa<clang::CXXConstructExpr>(init_expr))
        {
            applySourceRange(init, init_expr->getSourceRange());
        }
    }
    // Pei-Hung (09/01/2022) setup initializer once the RHS is processed.
    sg_var_decl->reset_initializer(init);

    // finding the bottom base type and check
    while(type->findBaseType() != type)
    {
      type = type->findBaseType();
      if(type == sg_varType)
        break;
    }

    if (isSgClassType(type) && isDefinitionaRequired) {
        SgClassDeclaration* classDecl = isSgClassDeclaration(isSgClassType(type)->get_declaration());
        SgClassDeclaration* classDefDecl = isSgClassDeclaration(isSgClassType(type)->get_declaration()->get_definingDeclaration());
        if(isembedded && classDefDecl != nullptr && !isSgDeclarationStatement(classDefDecl->get_parent()))
        {
          classDefDecl->set_parent(sg_var_decl);
          classDefDecl->set_isAutonomousDeclaration(isAutonomousDeclaration);
          sg_var_decl->set_baseTypeDefiningDeclaration(classDefDecl);
          sg_var_decl->set_variableDeclarationContainsBaseTypeDefiningDeclaration(true);
#if DEBUG_VISIT_DECL
            logger[DEBUG] << "set_baseTypeDefiningDeclaration " << classDefDecl << "\n";
            logger[DEBUG] << "set_variableDeclarationContainsBaseTypeDefiningDeclaration \n";
#endif
        }

        std::map<SgClassType *, bool>::iterator bool_it = p_class_type_decl_first_see_in_type.find(isSgClassType(type));
        ROSE_ASSERT(bool_it != p_class_type_decl_first_see_in_type.end());
        if (bool_it->second) {
            sg_var_decl->set_baseTypeDefiningDeclaration(isSgNamedType(type)->get_declaration()->get_definingDeclaration());
            sg_var_decl->set_variableDeclarationContainsBaseTypeDefiningDeclaration(true);
#if DEBUG_VISIT_DECL
            logger[DEBUG] << "set_variableDeclarationContainsBaseTypeDefiningDeclaration \n";
#endif
            bool_it->second = false;
        }
    }
    else if (isSgEnumType(type) && isDefinitionaRequired) {
        SgEnumDeclaration* enumDecl = isSgEnumDeclaration(isSgEnumType(type)->get_declaration());
        SgEnumDeclaration* enumDefDecl = isSgEnumDeclaration(isSgEnumType(type)->get_declaration()->get_definingDeclaration());
        if(isembedded && enumDefDecl != nullptr && !isSgDeclarationStatement(enumDefDecl->get_parent()))
        {
          enumDefDecl->set_parent(sg_var_decl);
          enumDefDecl->set_isAutonomousDeclaration(isAutonomousDeclaration);
          sg_var_decl->set_baseTypeDefiningDeclaration(enumDefDecl);
          sg_var_decl->set_variableDeclarationContainsBaseTypeDefiningDeclaration(true);
#if DEBUG_VISIT_DECL
            logger[DEBUG] << "set_baseTypeDefiningDeclaration " << enumDefDecl << "\n";
            logger[DEBUG] << "set_variableDeclarationContainsBaseTypeDefiningDeclaration \n";
#endif
        }

        std::map<SgEnumType *, bool>::iterator bool_it = p_enum_type_decl_first_see_in_type.find(isSgEnumType(type));
        ROSE_ASSERT(bool_it != p_enum_type_decl_first_see_in_type.end());
        if (bool_it->second) {
            sg_var_decl->set_baseTypeDefiningDeclaration(isSgEnumType(type)->get_declaration()->get_definingDeclaration());
            sg_var_decl->set_variableDeclarationContainsBaseTypeDefiningDeclaration(true);
#if DEBUG_VISIT_DECL
            logger[DEBUG] << "set_variableDeclarationContainsBaseTypeDefiningDeclaration \n";
#endif
            bool_it->second = false;
        }
    }

    sg_var_decl->set_firstNondefiningDeclaration(sg_var_decl);
    sg_var_decl->set_parent(SageBuilder::topScopeStack());

    ROSE_ASSERT(sg_var_decl->get_variables().size() == 1);

    SgInitializedName * init_name = sg_var_decl->get_variables()[0];
    ROSE_ASSERT(init_name != NULL);
    if(init != NULL)
    {
       init->set_parent(init_name);
       // (09/29/2023) the scope of SgInitializedName of a static data member
       // can be set earlier by buildVariableDeclaration_nfi
       if(init_name->get_scope() == NULL)
       {
         init_name->set_scope(SageBuilder::topScopeStack());
       }
    }
    applySourceRange(init_name, var_decl->getSourceRange());

    SgVariableDefinition * var_def = isSgVariableDefinition(init_name->get_declptr());
    ROSE_ASSERT(var_def != NULL);
    applySourceRange(var_def, var_decl->getSourceRange());

    // Pei-Hung (08/15/23): The following causes duplicated symbols in some cases.  Comment it out and need further investigation
//    SgVariableSymbol * var_symbol = new SgVariableSymbol(init_name);
//    SageBuilder::topScopeStack()->insert_symbol(name, var_symbol);

    // Pei-Hung (06/16/22) added "extern" modifier
    bool hasExternalStorage = var_decl->hasExternalStorage();
    if(hasExternalStorage)
    {
      sg_var_decl->get_declarationModifier().get_storageModifier().setExtern();
    }

    // Pei-Hung (06/16/22) added "static" modifier
    bool isStaticDecl = var_decl->isStaticLocal();
    if(isStaticDecl)
    {
      sg_var_decl->get_declarationModifier().get_storageModifier().setStatic();
    }
    // Pei-Hung (03/14/23) added "static" modifier for data member
    if(isStaticDataMember)
    {
      // Pei-Hung (09/29/23) Only set for the first static data member declaration
      // This implies the static data member declaration inside the class
      // Example in test2005_154.C
      if(var_decl->getPreviousDecl() == NULL)
      {
        sg_var_decl->get_declarationModifier().get_storageModifier().setStatic();
      }
    }
    //Pei-Hung (09/27/2022) setup linkage
    if(var_decl->hasExternalStorage())
    {
      sg_var_decl->get_declarationModifier().get_storageModifier().setExtern();
    }


    *node = sg_var_decl;

    return VisitDeclaratorDecl(var_decl, node) && res;
}

bool ClangToSageTranslator::VisitDecompositionDecl(clang::DecompositionDecl * decomposition_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitDecompositionDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitVarDecl(decomposition_decl, node) && res;
}

bool ClangToSageTranslator::VisitImplicitParamDecl(clang::ImplicitParamDecl * implicit_param_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitImplicitParamDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitVarDecl(implicit_param_decl, node) && res;
}

bool ClangToSageTranslator::VisitOMPCaptureExprDecl(clang::OMPCapturedExprDecl * omp_capture_expr_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitOMPCaptureExprDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitVarDecl(omp_capture_expr_decl, node) && res;
}

bool ClangToSageTranslator::VisitParmVarDecl(clang::ParmVarDecl * param_var_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitParmVarDecl" << "\n";
#endif
    bool res = true;

    SgName name(param_var_decl->getNameAsString());

    // use getOriginalType instead of getType.  This type has to match the DecayedType when VLA is used in parameter
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
    applySourceRange(*node, param_var_decl->getSourceRange());

    return VisitDeclaratorDecl(param_var_decl, node) && res;
}

bool ClangToSageTranslator::VisitVarTemplateSpecializationDecl(clang::VarTemplateSpecializationDecl * var_template_specialization_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitVarTemplateSpecializationDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitDeclaratorDecl(var_template_specialization_decl, node) && res;
}

bool ClangToSageTranslator::VisitVarTemplatePartialSpecializationDecl(clang::VarTemplatePartialSpecializationDecl * var_template_partial_specialization_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitVarTemplatePartialSpecializationDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitVarTemplateSpecializationDecl(var_template_partial_specialization_decl, node) && res;
}

bool  ClangToSageTranslator::VisitEnumConstantDecl(clang::EnumConstantDecl * enum_constant_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitEnumConstantDecl" << "\n";
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

bool ClangToSageTranslator::VisitIndirectFieldDecl(clang::IndirectFieldDecl * indirect_field_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitIndirectFieldDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitValueDecl(indirect_field_decl, node) && res;
}

bool ClangToSageTranslator::VisitOMPDeclareMapperDecl(clang::OMPDeclareMapperDecl * omp_declare_mapper_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitOMPDeclareMapperDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitValueDecl(omp_declare_mapper_decl, node) && res;
}

bool ClangToSageTranslator::VisitOMPDeclareReductionDecl(clang::OMPDeclareReductionDecl * omp_declare_reduction_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitOMPDeclareReductionDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitValueDecl(omp_declare_reduction_decl, node) && res;
}

bool ClangToSageTranslator::VisitUnresolvedUsingValueDecl(clang::UnresolvedUsingValueDecl * unresolved_using_value_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitUnresolvedUsingValueDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitValueDecl(unresolved_using_value_decl, node) && res;
}

bool ClangToSageTranslator::VisitOMPAllocateDecl(clang::OMPAllocateDecl * omp_allocate_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitOMPAllocateDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitDecl(omp_allocate_decl, node) && res;
}

bool ClangToSageTranslator::VisitOMPRequiresDecl(clang::OMPRequiresDecl * omp_requires_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitOMPRequiresDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitDecl(omp_requires_decl, node) && res;
}

bool ClangToSageTranslator::VisitOMPThreadPrivateDecl(clang::OMPThreadPrivateDecl * omp_thread_private_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitOMPThreadPrivateDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitDecl(omp_thread_private_decl, node) && res;
}

bool ClangToSageTranslator::VisitPragmaCommentDecl(clang::PragmaCommentDecl * pragma_comment_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitPragmaCommentDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitDecl(pragma_comment_decl, node) && res;
}

bool ClangToSageTranslator::VisitPragmaDetectMismatchDecl(clang::PragmaDetectMismatchDecl * pragma_detect_mismatch_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitPragmaDetectMismatchDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitDecl(pragma_detect_mismatch_decl, node) && res;
}

bool ClangToSageTranslator::VisitStaticAssertDecl(clang::StaticAssertDecl * pragma_static_assert_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitStaticAssertDecl" << "\n";
#endif
    bool res = true;

    SgNode * tmp_condition = Traverse(pragma_static_assert_decl->getAssertExpr());
    SgExpression * condition = isSgExpression(tmp_condition);
    if (tmp_condition != NULL && condition == NULL) {
        logger[WARN] << "Runtime error: tmp_condition != NULL && condition == NULL" << "\n";
        res = false;
    } else {
#if (__clang__)  && (__clang_major__ >= 18)
      clang::Expr* Message = pragma_static_assert_decl->getMessage();
      if (const auto *SL = dyn_cast<clang::StringLiteral>(Message)) {
         assert(SL->isUnevaluated() && "expected an unevaluated string");
         *node = SageBuilder::buildStaticAssertionDeclaration(condition, SL->getString().str());
      }
#else
      *node = SageBuilder::buildStaticAssertionDeclaration(condition, pragma_static_assert_decl->getMessage()->getString().str());
#endif
    }

    return VisitDecl(pragma_static_assert_decl, node) && res;
}

bool ClangToSageTranslator::VisitLinkageSpecDecl(clang::LinkageSpecDecl * linkage_spec_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitLinkageSpecDecl" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitDecl(linkage_spec_decl, node) && res;
}


bool ClangToSageTranslator::VisitTranslationUnitDecl(clang::TranslationUnitDecl * translation_unit_decl, SgNode ** node) {
#if DEBUG_VISIT_DECL
    logger[DEBUG] << "ClangToSageTranslator::VisitTranslationUnitDecl" << "\n";
#endif
    if (*node != NULL) {
        logger[WARN] << "Runtime error: The TranslationUnitDecl is already associated to a SAGE node." << "\n";
        return false;
    }

  // Create the SAGE node: SgGlobal

    if (p_global_scope != NULL) {
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
    p_decl_context_map.insert(std::pair<clang::DeclContext *, SgScopeStatement *>(decl_context, global_scope));
    // DeclContext handling is repalced by calling TraverseForDeclContext
    bool res = TraverseForDeclContext(decl_context);
    SageBuilder::popScopeStack();

  // Traverse the class hierarchy

    return VisitDecl(translation_unit_decl, node) && res;
}
