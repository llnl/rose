
#ifndef _CLANG_TO_DOT_FRONTEND_PRIVATE_HPP_
# define _CLANG_TO_DOT_FRONTEND_PRIVATE_HPP_

#include "clang-frontend.hpp"
#include "clang-compat.hpp"

#include "clang/AST/AST.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclarationName.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclFriend.h"
#include "clang/AST/DeclGroup.h"
#include "clang/AST/DeclObjC.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/DeclVisitor.h"
#include "clang/AST/Expr.h"
#include "clang/AST/ExprCXX.h"
#include "clang/AST/ExprObjC.h"
#include "clang/AST/NestedNameSpecifier.h"
#include "clang/AST/ParentMap.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/Stmt.h"
#include "clang/AST/StmtCXX.h"
#include "clang/AST/StmtObjC.h"
#include "clang/AST/StmtVisitor.h"
#include "clang/AST/TemplateBase.h"
#include "clang/AST/TemplateName.h"
#include "clang/AST/Type.h"
#include "clang/AST/TypeLoc.h"
#include "clang/AST/TypeLocVisitor.h"

#include "clang/Basic/Builtins.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/IdentifierTable.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/TargetOptions.h"

#include "clang/FrontendTool/Utils.h"

#include "clang/Frontend/CompilerInvocation.h"
#include "clang/Frontend/CompilerInstance.h"

#include "clang/Basic/DiagnosticOptions.h"

#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Frontend/TextDiagnosticBuffer.h"

#include "clang/Lex/HeaderSearch.h"
#include "clang/Lex/PPCallbacks.h"
#include "clang/Lex/Preprocessor.h"

#include "clang/Parse/ParseAST.h"

#include "clang/Sema/Sema.h"

#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"

#include "llvm/Config/llvm-config.h"

// (10/30/2026) lin32: LLVM header selection must use LLVM_VERSION_MAJOR, independently of the host compiler.
#if LLVM_VERSION_MAJOR >= 18
#include "llvm/TargetParser/Host.h"
#else
#include "llvm/Support/Host.h"
#endif
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/raw_os_ostream.h"

#include "llvm/Frontend/OpenMP/OMPIRBuilder.h"

// DQ (10/23/2020): Adding clang-to-dot support (graph functions).
#include "clang_graph.h"

// DQ (11/27/2020): Turn on the debugging information as we visit clang IR nodes.
#define DEBUG_VISITOR             0
#define DEBUG_TRAVERSAL           0
#define DEBUG_SOURCE_LOCATION     0
#define DEBUG_SYMBOL_TABLE_LOOKUP 0
#define DEBUG_ARGS                0
#define DEBUG_TRAVERSE_DECL       0

// Print visitor name when visiting a node inheritance hierarchy
#ifdef DEBUG_VISITOR
#  ifndef DEBUG_VISIT_STMT
#    define DEBUG_VISIT_STMT DEBUG_VISITOR
#  endif
#  ifndef DEBUG_VISIT_DECL
#    define DEBUG_VISIT_DECL DEBUG_VISITOR
#  endif
#  ifndef DEBUG_VISIT_TYPE
#    define DEBUG_VISIT_TYPE DEBUG_VISITOR
#  endif
#else
#  define DEBUG_VISITOR    1
#  define DEBUG_VISIT_STMT 1
#  define DEBUG_VISIT_DECL 1
#  define DEBUG_VISIT_TYPE 1
#endif

// Print results of traversal of the nodes: "already done?" and "generated Sage ptr"
#ifdef DEBUG_TRAVERSAL
#  ifndef DEBUG_TRAVERSE_STMT
#    define DEBUG_TRAVERSE_STMT DEBUG_TRAVERSAL
#  endif
#  ifndef DEBUG_TRAVERSE_DECL
#    define DEBUG_TRAVERSE_DECL DEBUG_TRAVERSAL
#  endif
#  ifndef DEBUG_TRAVERSE_TYPE
#    define DEBUG_TRAVERSE_TYPE DEBUG_TRAVERSAL
#  endif
#else
#  define DEBUG_TRAVERSAL     1
#  define DEBUG_TRAVERSE_STMT 1
#  define DEBUG_TRAVERSE_DECL 1
#  define DEBUG_TRAVERSE_TYPE 1
#endif

// Print debug info when attaching source location
#ifndef DEBUG_SOURCE_LOCATION
#  define DEBUG_SOURCE_LOCATION 1
#endif

// Display symbol lookup process
#ifndef DEBUG_SYMBOL_TABLE_LOOKUP
#  define DEBUG_SYMBOL_TABLE_LOOKUP 1
#endif

// Print args receive by clang_main
#ifndef DEBUG_ARGS
#  define DEBUG_ARGS 1
#endif

// Fail when a FIXME is reach
#ifndef FAIL_FIXME
#  define FAIL_FIXME 0
#endif

// Fail when a TODO is reach
#ifndef FAIL_TODO
#  define FAIL_TODO 1
#endif

class ClangToDotPreprocessorRecord;

#if 1
// class ClangToDot : public clang::ASTConsumer 
class ClangToDotTranslator : public clang::ASTConsumer 
   {
    public:
        enum Language {
            C,
            CPLUSPLUS,
            OBJC,
            CUDA,
            OPENCL,
            unknown
        };

        struct NodeDescriptor {
            NodeDescriptor(std::string ident_);

            void toDot(std::ostream & out) const;

            std::string ident;

            std::vector<std::string> kind_hierarchy;
            std::vector<std::pair<std::string, std::string> > successors;
            std::vector<std::pair<std::string, std::string> > attributes;
        };

    protected:
        static Sawyer::Message::Facility logger;
        clang::CompilerInstance  * p_compiler_instance;

        std::map<clang::Decl *, std::string> p_decl_translation_map;
        std::map<clang::Stmt *, std::string> p_stmt_translation_map;
        std::map<const clang::Type *, std::string> p_type_translation_map;

        std::map<std::string, NodeDescriptor> p_node_desc;

        Language language;

        unsigned long ident_cnt;

    protected:
        void VisitTemplateArgument(const clang::TemplateArgument & template_argument, NodeDescriptor & node_desc, std::string prefix);
        void VisitTemplateName(const clang::TemplateName & template_name, NodeDescriptor & node_desc, std::string prefix);
        void VisitNestedNameSpecifier(RoseClangCompat::NestedNameSpecifier nested_name_specifier, NodeDescriptor & node_desc, std::string prefix);

          ClangToDotPreprocessorRecord   * p_sage_preprocessor_recorder;

    public:
     // ClangToDot(clang::CompilerInstance * compiler_instance, Language language_);
        ClangToDotTranslator(clang::CompilerInstance * compiler_instance, Language language_);

     // virtual ~ClangToDot();
        virtual ~ClangToDotTranslator();

        std::string genNextIdent();

        void toDot(std::ostream & out) const;

  /* ASTConsumer's methods overload */

        virtual void HandleTranslationUnit(clang::ASTContext & ast_context);

  /* Traverse methods */

        virtual std::string Traverse(clang::Decl * decl);
        virtual std::string Traverse(clang::Stmt * stmt);
        virtual std::string Traverse(const clang::Type * type);

#else

/*! \brief Translator from Clang AST to SAGE III (ROSE Compiler AST)
 */
class ClangToDotTranslator : public clang::ASTConsumer 
   {
     public:
        /*! \brief the 5 C-family languages supported by Clang
         */
          enum Language 
             {
               C,
               CPLUSPLUS,
               OBJC,
               CUDA,
               OPENCL,
               unknown
             };

        /*! \brief Update a Sage node source position using Clang information
         *  \param node Sage node to update
         *  \param source_range Clang's source position information
         */
          void applySourceRange(SgNode * node, clang::SourceRange source_range);
        /*! \brief Set a Sage node source position to compiler generated
         *  \param node Sage node to update
         *  \param to_be_unparse should this compiler generated node be unparse?
         */
          void setCompilerGeneratedFileInfo(SgNode * node, bool to_be_unparse = false);

     protected:
          std::map<clang::Decl *, SgNode *> p_decl_translation_map;
          std::map<clang::Stmt *, SgNode *> p_stmt_translation_map;
          std::map<const clang::Type *, SgNode *> p_type_translation_map;
          SgGlobal * p_global_scope;

          std::map<SgClassType *, bool> p_class_type_decl_first_see_in_type;
          std::map<SgEnumType *, bool>  p_enum_type_decl_first_see_in_type;

          clang::CompilerInstance  * p_compiler_instance;
          ClangToDotPreprocessorRecord   * p_sage_preprocessor_recorder;

          Language language;

          SgSymbol * GetSymbolFromSymbolTable(clang::NamedDecl * decl);

          SgType * buildTypeFromQualifiedType(const clang::QualType & qual_type);

     public:
          ClangToDotTranslator(clang::CompilerInstance * compiler_instance, Language language_);

          virtual ~ClangToDotTranslator();

          SgGlobal * getGlobalScope();

  /* ASTConsumer's methods overload */

          virtual void HandleTranslationUnit(clang::ASTContext & ast_context);

  /* Traverse methods */

          virtual SgNode * Traverse(clang::Decl * decl);
          virtual SgNode * Traverse(clang::Stmt * stmt);
          virtual SgNode * Traverse(const clang::Type * type);
#endif
       // DQ (11/3/2020): I don't think we need this.
       // virtual SgNode * TraverseForDeclContext(clang::DeclContext * decl_context);

  /* Visit methods */
  /* 
     Reference: https://clang.llvm.org/doxygen/classclang_1_1Decl.html 
     Overall 84 decl AST nodes according as 04/24/2019
  */
          virtual bool VisitDecl(clang::Decl * decl, NodeDescriptor & node_desc);
          virtual bool VisitAccessSpecDecl(clang::AccessSpecDecl * access_spec_decl, NodeDescriptor & node_desc);
          virtual bool VisitBlockDecl(clang::BlockDecl * block_decl, NodeDescriptor & node_desc);
          virtual bool VisitCapturedDecl(clang::CapturedDecl * capture_decl, NodeDescriptor & node_desc);
          virtual bool VisitEmptyDecl(clang::EmptyDecl * empty_decl, NodeDescriptor & node_desc);
          virtual bool VisitExportDecl(clang::ExportDecl * export_decl, NodeDescriptor & node_desc);
          virtual bool VisitExternCContextDecl(clang::ExternCContextDecl * ccontext_decl, NodeDescriptor & node_desc);
          virtual bool VisitFileScopeAsmDecl(clang::FileScopeAsmDecl * file_scope_asm_decl, NodeDescriptor & node_desc);
          virtual bool VisitFriendDecl(clang::FriendDecl * friend_decl, NodeDescriptor & node_desc);
          virtual bool VisitFriendTemplateDecl(clang::FriendTemplateDecl * friend_template_decl, NodeDescriptor & node_desc);
          virtual bool VisitImportDecl(clang::ImportDecl * import_decl, NodeDescriptor & node_desc);
          virtual bool VisitNamedDecl(clang::NamedDecl * named_decl, NodeDescriptor & node_desc);
          virtual bool VisitLabelDecl(clang::LabelDecl * label_decl, NodeDescriptor & node_desc);
          virtual bool VisitNamespaceAliasDecl(clang::NamespaceAliasDecl * namespace_alias_decl, NodeDescriptor & node_desc);
          virtual bool VisitNamespaceDecl(clang::NamespaceDecl * namespace_decl, NodeDescriptor & node_desc);
       // virtual bool VisitObjCCompatibleAliasDecl
       // virtual bool VisitObjCContainerDecl
       // virtual bool VisitObjCCategoryDecl
       // virtual bool VisitObjCInterfaceDecl
       // virtual bool VisitBuiCProtocolDecl
       // virtual bool VisitBuiltinTemplateDecl
       // virtual bool VisitObjCMethodDecl
       // virtual bool VisitObjCPropertyDecl
          virtual bool VisitTemplateDecl(clang::TemplateDecl * template_decl, NodeDescriptor & node_desc);
          virtual bool VisitBuiltinTemplateDecl(clang::BuiltinTemplateDecl * builtin_template_decl, NodeDescriptor & node_desc);
          virtual bool VisitConceptDecl(clang::ConceptDecl * concept_decl, NodeDescriptor & node_desc);
          virtual bool VisitRedeclarableTemplateDecl(clang::RedeclarableTemplateDecl * redeclarable_template_decl, NodeDescriptor & node_desc);
          virtual bool VisitClassTemplateDecl(clang::ClassTemplateDecl * class_template_decl, NodeDescriptor & node_desc);
          virtual bool VisitFunctionTemplateDecl(clang::FunctionTemplateDecl * function_template_decl, NodeDescriptor & node_desc);
          virtual bool VisitTypeAliasTemplateDecl(clang::TypeAliasTemplateDecl * type_alias_template_decl, NodeDescriptor & node_desc);
          virtual bool VisitVarTemplateDecl(clang::VarTemplateDecl * var_template_decl, NodeDescriptor & node_desc);
          virtual bool VisitTemplateTemplateParmDecl(clang::TemplateTemplateParmDecl * template_template_parm_decl, NodeDescriptor & node_desc);
          virtual bool VisitTypeDecl(clang::TypeDecl * type_decl, NodeDescriptor & node_desc);
          virtual bool VisitTagDecl(clang::TagDecl * tag_decl, NodeDescriptor & node_desc);
          virtual bool VisitRecordDecl(clang::RecordDecl * record_decl, NodeDescriptor & node_desc);
          virtual bool VisitCXXRecordDecl(clang::CXXRecordDecl * cxx_record_decl, NodeDescriptor & node_desc);
          virtual bool VisitClassTemplateSpecializationDecl(clang::ClassTemplateSpecializationDecl * class_tpl_spec_decl, NodeDescriptor & node_desc);
          virtual bool VisitClassTemplatePartialSpecializationDecl(clang::ClassTemplatePartialSpecializationDecl * class_tpl_part_spec_decl, NodeDescriptor & node_desc);
          virtual bool VisitEnumDecl(clang::EnumDecl * enum_decl, NodeDescriptor & node_desc); 
          virtual bool VisitTemplateTypeParmDecl(clang::TemplateTypeParmDecl * template_type_parm_decl, NodeDescriptor & node_desc);
          virtual bool VisitTypedefNameDecl(clang::TypedefNameDecl * typedef_name_decl, NodeDescriptor & node_desc);
          virtual bool VisitTypedefDecl(clang::TypedefDecl * typedef_decl, NodeDescriptor & node_desc);
          virtual bool VisitTypeAliasDecl(clang::TypeAliasDecl * type_alias_decl, NodeDescriptor & node_desc);
       // virtual bool VisitObjCTypeParamDecl(clang::ObjCTypeParamDecl * obj_type_param_decl, NodeDescriptor & node_desc);
          virtual bool VisitUnresolvedUsingTypenameDecl(clang::UnresolvedUsingTypenameDecl * unresolved_using_type_name_decl, NodeDescriptor & node_desc);
          virtual bool VisitUsingDecl(clang::UsingDecl * using_decl, NodeDescriptor & node_desc);
          virtual bool VisitUsingDirectiveDecl(clang::UsingDirectiveDecl * using_directive_decl, NodeDescriptor & node_desc);
          virtual bool VisitUsingPackDecl(clang::UsingPackDecl * using_pack_decl, NodeDescriptor & node_desc);
          virtual bool VisitUsingShadowDecl(clang::UsingShadowDecl * using_shadow_decl, NodeDescriptor & node_desc);
          virtual bool VisitConstructorUsingShadowDecl(clang::ConstructorUsingShadowDecl * constructor_using_shadow_decl, NodeDescriptor & node_desc);
          virtual bool VisitValueDecl(clang::ValueDecl * value_decl, NodeDescriptor & node_desc);
          virtual bool VisitBindingDecl(clang::BindingDecl * binding_decl, NodeDescriptor & node_desc);
          virtual bool VisitDeclaratorDecl(clang::DeclaratorDecl * declarator_decl, NodeDescriptor & node_desc);
          virtual bool VisitFieldDecl(clang::FieldDecl * field_decl, NodeDescriptor & node_desc);
       // virtual bool VisitObjCAtDefsFieldDecl
       // virtual bool VisitObjCvarDecl
          virtual bool VisitFunctionDecl(clang::FunctionDecl * function_decl, NodeDescriptor & node_desc);
          virtual bool VisitCXXDeductionGuideDecl(clang::CXXDeductionGuideDecl * cxx_deduction_guide_guide, NodeDescriptor & node_desc);
          virtual bool VisitCXXMethodDecl(clang::CXXMethodDecl * cxx_method_decl, NodeDescriptor & node_desc);
          virtual bool VisitCXXConstructorDecl(clang::CXXConstructorDecl * cxx_constructor_decl, NodeDescriptor & node_desc);
          virtual bool VisitCXXConversionDecl(clang::CXXConversionDecl * cxx_conversion_decl, NodeDescriptor & node_desc);
          virtual bool VisitCXXDestructorDecl(clang::CXXDestructorDecl * cxx_destructor_decl, NodeDescriptor & node_desc);
          virtual bool VisitMSPropertyDecl(clang::MSPropertyDecl * ms_property_decl, NodeDescriptor & node_desc);
          virtual bool VisitNonTypeTemplateParmDecl(clang::NonTypeTemplateParmDecl * non_type_template_param_decl, NodeDescriptor & node_desc);
          virtual bool VisitVarDecl(clang::VarDecl * var_decl, NodeDescriptor & node_desc);
          virtual bool VisitDecompositionDecl(clang::DecompositionDecl* decomposition_decl, NodeDescriptor & node_desc);
          virtual bool VisitImplicitParamDecl(clang::ImplicitParamDecl * implicit_param_decl, NodeDescriptor & node_desc);
          virtual bool VisitOMPCaptureExprDecl(clang::OMPCapturedExprDecl* omp_capture_expr_decl, NodeDescriptor & node_desc);
          virtual bool VisitParmVarDecl(clang::ParmVarDecl * param_var_decl, NodeDescriptor & node_desc);
          virtual bool VisitVarTemplateSpecializationDecl(clang::VarTemplateSpecializationDecl * var_template_specialization_decl, NodeDescriptor & node_desc);
          virtual bool VisitVarTemplatePartialSpecializationDecl(clang::VarTemplatePartialSpecializationDecl * var_template_partial_specialization, NodeDescriptor & node_desc);
          virtual bool VisitEnumConstantDecl(clang::EnumConstantDecl * enum_constant_decl, NodeDescriptor & node_desc);
          virtual bool VisitIndirectFieldDecl(clang::IndirectFieldDecl * indirect_field_decl, NodeDescriptor & node_desc);
          virtual bool VisitOMPDeclareMapperDecl(clang::OMPDeclareMapperDecl * omp_declare_mapper_decl, NodeDescriptor & node_desc);
          virtual bool VisitOMPDeclareReductionDecl(clang::OMPDeclareReductionDecl * omp_declare_reduction_decl, NodeDescriptor & node_desc);
          virtual bool VisitUnresolvedUsingValueDecl(clang::UnresolvedUsingValueDecl * unresolved_using_value_decl, NodeDescriptor & node_desc);
       // virtual bool VisitObjCPropertyImplDecl
          virtual bool VisitOMPAllocateDecl(clang::OMPAllocateDecl * omp_allocate_decl, NodeDescriptor & node_desc);
          virtual bool VisitOMPRequiresDecl(clang::OMPRequiresDecl * omp_requires_decl, NodeDescriptor & node_desc);
          virtual bool VisitOMPThreadPrivateDecl(clang::OMPThreadPrivateDecl * omp_thread_private_decl, NodeDescriptor & node_desc);
          virtual bool VisitPragmaCommentDecl(clang::PragmaCommentDecl * pragma_comment_decl, NodeDescriptor & node_desc);
          virtual bool VisitPragmaDetectMismatchDecl(clang::PragmaDetectMismatchDecl * pragma_detect_mismatch, NodeDescriptor & node_desc);
          virtual bool VisitStaticAssertDecl(clang::StaticAssertDecl * static_assert_decl, NodeDescriptor & node_desc);
          virtual bool VisitTranslationUnitDecl(clang::TranslationUnitDecl * translation_unit_decl, NodeDescriptor & node_desc);

  /* 
     Reference: https://clang.llvm.org/doxygen/classclang_1_1Stmt.html 
     Overall 198 stmt AST nodes according as 02/19/2019
  */
          virtual bool VisitStmt(clang::Stmt * stmt, NodeDescriptor & node_desc);
          virtual bool VisitAsmStmt(clang::AsmStmt * asm_stmt, NodeDescriptor & node_desc);
          virtual bool VisitGCCAsmStmt(clang::GCCAsmStmt * gcc_asm_stmt, NodeDescriptor & node_desc);
          virtual bool VisitMSAsmStmt(clang::MSAsmStmt * ms_asm_stmt, NodeDescriptor & node_desc);
          virtual bool VisitBreakStmt(clang::BreakStmt * break_stmt, NodeDescriptor & node_desc);
          virtual bool VisitCapturedStmt(clang::CapturedStmt * captured_stmt, NodeDescriptor & node_desc);
          virtual bool VisitCompoundStmt(clang::CompoundStmt * compound_stmt, NodeDescriptor & node_desc);
          virtual bool VisitContinueStmt(clang::ContinueStmt * continue_stmt, NodeDescriptor & node_desc);
          virtual bool VisitCoreturnStmt(clang::CoreturnStmt * coreturn_stmt, NodeDescriptor & node_desc);
          virtual bool VisitCoroutineBodyStmt(clang::CoroutineBodyStmt * coroutine_body_stmt, NodeDescriptor & node_desc);
          virtual bool VisitCXXCatchStmt(clang::CXXCatchStmt * cxx_catch_stmt, NodeDescriptor & node_desc);
          virtual bool VisitCXXForRangeStmt(clang::CXXForRangeStmt * cxx_for_range_stmt, NodeDescriptor & node_desc);
          virtual bool VisitCXXTryStmt(clang::CXXTryStmt * cxx_try_stmt, NodeDescriptor & node_desc);
          virtual bool VisitDeclStmt(clang::DeclStmt * decl_stmt, NodeDescriptor & node_desc);
          virtual bool VisitDoStmt(clang::DoStmt * do_stmt, NodeDescriptor & node_desc);
          virtual bool VisitForStmt(clang::ForStmt * for_stmt, NodeDescriptor & node_desc);
          virtual bool VisitGotoStmt(clang::GotoStmt * goto_stmt, NodeDescriptor & node_desc);
          virtual bool VisitIfStmt(clang::IfStmt * if_stmt, NodeDescriptor & node_desc);
          virtual bool VisitIndirectGotoStmt(clang::IndirectGotoStmt * indirect_goto_stmt, NodeDescriptor & node_desc);
          virtual bool VisitMSDependentExistsStmt(clang::MSDependentExistsStmt * ms_dependent_exists_stmt, NodeDescriptor & node_desc);
          virtual bool VisitNullStmt(clang::NullStmt * null_stmt, NodeDescriptor & node_desc);
       // virtual bool VisitObjCAtCatchStmt
       // virtual bool VisitObjCAtFinallyStmt
       // virtual bool VisitObjCAtSynchronizedStmt
       // virtual bool VisitObjCAtThrowStmt
       // virtual bool VisitObjCAtTryStmt
       // virtual bool VisitObjCAutoreleasePoolStmt
       // virtual bool VisitObjCForCollectionStmt
          virtual bool VisitOMPExecutableDirective(clang::OMPExecutableDirective * omp_executable_directive, NodeDescriptor & node_desc);
          virtual bool VisitOMPAtomicDirective(clang::OMPAtomicDirective * omp_atomic_directive, NodeDescriptor & node_desc);
          virtual bool VisitOMPBarrierDirective(clang::OMPBarrierDirective * omp_barrier_directive, NodeDescriptor & node_desc);
          virtual bool VisitOMPCancelDirective(clang::OMPCancelDirective * omp_cancel_directive, NodeDescriptor & node_desc);
          virtual bool VisitOMPCancellationPointDirective(clang::OMPCancellationPointDirective * omp_cancellation_point_directive, NodeDescriptor & node_desc);
          virtual bool VisitOMPCriticalDirective(clang::OMPCriticalDirective * omp_critical_directive, NodeDescriptor & node_desc);
          virtual bool VisitOMPFlushDirective(clang::OMPFlushDirective * omp_flush_directive, NodeDescriptor & node_desc);
          virtual bool VisitOMPLoopDirective(clang::OMPLoopDirective * omp_loop_directive, NodeDescriptor & node_desc);
          virtual bool VisitOMPDistributeDirective(clang::OMPDistributeDirective * omp_distribute_directive, NodeDescriptor & node_desc);
          virtual bool VisitOMPDistributeParallelForDirective(clang::OMPDistributeParallelForDirective * omp_distribute_parallel_for_directive, NodeDescriptor & node_desc);
          virtual bool VisitOMPDistributeParallelForSimdDirective(clang::OMPDistributeParallelForSimdDirective * omp_distribute_parallel_for_simd_directive, NodeDescriptor & node_desc);
          virtual bool VisitOMPDistributeSimdDirective(clang::OMPDistributeSimdDirective * omp_distribute__simd_directive, NodeDescriptor & node_desc);
          virtual bool VisitOMPForDirective(clang::OMPForDirective * omp_for_directive, NodeDescriptor & node_desc);
          virtual bool VisitOMPForSimdDirective(clang::OMPForSimdDirective * omp_for_simd_directive, NodeDescriptor & node_desc);
       // virtual bool VisitOMPMasterTaskLoopDirective(clang::OMPMasterTaskLoopDirective * omp_master_task_loop_directive, NodeDescriptor & node_desc);
       // virtual bool VisitOMPMasterTaskLoopSimdDirective
          virtual bool VisitOMPParallelForDirective(clang::OMPParallelForDirective * omp_parallel_for_directive, NodeDescriptor & node_desc);
          virtual bool VisitOMPParallelForSimdDirective(clang::OMPParallelForSimdDirective * omp_parallel_for_simd_directive, NodeDescriptor & node_desc);
       // virtual bool VisitOMPParallelMasterTaskLoopDirective
          virtual bool VisitOMPSimdDirective(clang::OMPSimdDirective * omp_simd_directive, NodeDescriptor & node_desc);
          virtual bool VisitOMPTargetParallelForDirective(clang::OMPTargetParallelForDirective * omp_target_parallel_for_directive, NodeDescriptor & node_desc);
          virtual bool VisitOMPTargetParallelForSimdDirective(clang::OMPTargetParallelForSimdDirective * omp_target_parallel_for_simd_directive, NodeDescriptor & node_desc);
          virtual bool VisitOMPTargetSimdDirective(clang::OMPTargetSimdDirective * omp_target_simd_directive, NodeDescriptor & node_desc);
          virtual bool VisitOMPTargetTeamsDistributeDirective(clang::OMPTargetTeamsDistributeDirective * omp_target_teams_distribute_directive, NodeDescriptor & node_desc);
       // virtual bool VisitOMPTargetTeamsDistributeParallelForSimdDirective
          virtual bool VisitOMPTargetTeamsDistributeSimdDirective(clang::OMPTargetTeamsDistributeSimdDirective * omp_target_teams_distribute_simd_directive, NodeDescriptor & node_desc);
          virtual bool VisitOMPTaskLoopDirective(clang::OMPTaskLoopDirective * omp_task_loop_directive, NodeDescriptor & node_desc);
          virtual bool VisitOMPTaskLoopSimdDirective(clang::OMPTaskLoopSimdDirective * omp_task_loop_simd_directive, NodeDescriptor & node_desc);
       // virtual bool VisitOMPTeamDistributeDirective
       // virtual bool VisitOMPTeamDistributeParallelForSimdDirective
       // virtual bool VisitOMPTeamDistributeSimdDirective
          virtual bool VisitOMPMasterDirective(clang::OMPMasterDirective * omp_master_directive, NodeDescriptor & node_desc);
          virtual bool VisitOMPOrderedDirective(clang::OMPOrderedDirective * omp_ordered_directive, NodeDescriptor & node_desc);
          virtual bool VisitOMPParallelDirective(clang::OMPParallelDirective * omp_parallel_directive, NodeDescriptor & node_desc);
          virtual bool VisitOMPParallelSectionsDirective(clang::OMPParallelSectionsDirective * omp_parallel_sections_directive, NodeDescriptor & node_desc);
          virtual bool VisitReturnStmt(clang::ReturnStmt * return_stmt, NodeDescriptor & node_desc);
          virtual bool VisitSEHExceptStmt(clang::SEHExceptStmt * seh_except_stmt, NodeDescriptor & node_desc);
          virtual bool VisitSEHFinallyStmt(clang::SEHFinallyStmt * seh_finally_stmt, NodeDescriptor & node_desc);
          virtual bool VisitSEHLeaveStmt(clang::SEHLeaveStmt * seh_leave_stmt, NodeDescriptor & node_desc);
          virtual bool VisitSEHTryStmt(clang::SEHTryStmt * seh_try_stmt, NodeDescriptor & node_desc);
          virtual bool VisitSwitchCase(clang::SwitchCase * switch_case, NodeDescriptor & node_desc);
          virtual bool VisitCaseStmt(clang::CaseStmt * case_stmt, NodeDescriptor & node_desc);
          virtual bool VisitDefaultStmt(clang::DefaultStmt * default_stmt, NodeDescriptor & node_desc);
          virtual bool VisitSwitchStmt(clang::SwitchStmt * switch_stmt, NodeDescriptor & node_desc);
          virtual bool VisitValueStmt(clang::ValueStmt * value_stmt, NodeDescriptor & node_desc);
          virtual bool VisitAttributedStmt(clang::AttributedStmt * attributed_stmt, NodeDescriptor & node_desc);
          virtual bool VisitExpr(clang::Expr * expr, NodeDescriptor & node_desc);
          virtual bool VisitAbstractConditionalOperator(clang::AbstractConditionalOperator * abstract_conditional_operator, NodeDescriptor & node_desc);
          virtual bool VisitBinaryConditionalOperator(clang::BinaryConditionalOperator * binary_conditional_operator, NodeDescriptor & node_desc);
          virtual bool VisitConditionalOperator(clang::ConditionalOperator * conditional_operator, NodeDescriptor & node_desc);
          virtual bool VisitAddrLabelExpr(clang::AddrLabelExpr * addr_label_expr, NodeDescriptor & node_desc);
          virtual bool VisitArrayInitIndexExpr(clang::ArrayInitIndexExpr * array_init_index_expr, NodeDescriptor & node_desc);
          virtual bool VisitArrayInitLoopExpr(clang::ArrayInitLoopExpr * array_init_loop_expr, NodeDescriptor & node_desc);
          virtual bool VisitArraySubscriptExpr(clang::ArraySubscriptExpr * array_subscript_expr, NodeDescriptor & node_desc);
          virtual bool VisitArrayTypeTraitExpr(clang::ArrayTypeTraitExpr * array_type_trait_expr, NodeDescriptor & node_desc);
          virtual bool VisitAsTypeExpr(clang::AsTypeExpr * as_type_expr, NodeDescriptor & node_desc);
          virtual bool VisitAtomicExpr(clang::AtomicExpr * atomic_expr, NodeDescriptor & node_desc);
          virtual bool VisitBinaryOperator(clang::BinaryOperator * binary_operator, NodeDescriptor & node_desc);
          virtual bool VisitCompoundAssignOperator(clang::CompoundAssignOperator * compound_assign_operator, NodeDescriptor & node_desc);
          virtual bool VisitBlockExpr(clang::BlockExpr * block_expr, NodeDescriptor & node_desc);
          virtual bool VisitCallExpr(clang::CallExpr * call_expr, NodeDescriptor & node_desc);
          virtual bool VisitCUDAKernelCallExpr(clang::CUDAKernelCallExpr * cuda_kernel_call_expr, NodeDescriptor & node_desc);
          virtual bool VisitCXXMemberCallExpr(clang::CXXMemberCallExpr * cxx_member_call_expr, NodeDescriptor & node_desc);
          virtual bool VisitCXXOperatorCallExpr(clang::CXXOperatorCallExpr * cxx_operator_call_expr, NodeDescriptor & node_desc);
          virtual bool VisitUserDefinedLiteral(clang::UserDefinedLiteral * user_defined_literal, NodeDescriptor & node_desc);
          virtual bool VisitCastExpr(clang::CastExpr * cast_expr, NodeDescriptor & node_desc);
          virtual bool VisitExplicitCastExpr(clang::ExplicitCastExpr * explicit_cast_expr, NodeDescriptor & node_desc);
          virtual bool VisitBuiltinBitCastExpr(clang::BuiltinBitCastExpr * builtin_bit_cast_expr, NodeDescriptor & node_desc);
          virtual bool VisitCStyleCastExpr(clang::CStyleCastExpr * c_style_cast, NodeDescriptor & node_desc);
          virtual bool VisitCXXFunctionalCastExpr(clang::CXXFunctionalCastExpr * cxx_functional_cast_expr, NodeDescriptor & node_desc);
          virtual bool VisitCXXNamedCastExpr(clang::CXXNamedCastExpr * cxx_named_cast_expr, NodeDescriptor & node_desc);
          virtual bool VisitCXXConstCastExpr(clang::CXXConstCastExpr * cxx_const_cast_expr, NodeDescriptor & node_desc);
          virtual bool VisitCXXDynamicCastExpr(clang::CXXDynamicCastExpr * cxx_dynamic_cast_expr, NodeDescriptor & node_desc);
          virtual bool VisitCXXReinterpretCastExpr(clang::CXXReinterpretCastExpr * cxx_reinterpret_cast_expr, NodeDescriptor & node_desc);
          virtual bool VisitCXXStaticCastExpr(clang::CXXStaticCastExpr * cxx_static_cast_expr, NodeDescriptor & node_desc);
       // virtual bool VisitObjCBridgedCastExpr
          virtual bool VisitImplicitCastExpr(clang::ImplicitCastExpr * implicit_cast_expr, NodeDescriptor & node_desc);
          virtual bool VisitCharacterLiteral(clang::CharacterLiteral * character_literal, NodeDescriptor & node_desc);
          virtual bool VisitChooseExpr(clang::ChooseExpr * choose_expr, NodeDescriptor & node_desc);
          virtual bool VisitCompoundLiteralExpr(clang::CompoundLiteralExpr * compound_literal, NodeDescriptor & node_desc);
       // virtual bool VisitConceptSpecializationExpr(clang::ConceptSpecializationExpr * concept_specialization_expr, NodeDescriptor & node_desc);
          virtual bool VisitConvertVectorExpr(clang::ConvertVectorExpr * convert_vector_expr, NodeDescriptor & node_desc);
          virtual bool VisitCoroutineSuspendExpr(clang::CoroutineSuspendExpr * coroutine_suspend_expr, NodeDescriptor & node_desc);
          virtual bool VisitCoawaitExpr(clang::CoawaitExpr * coawait_expr, NodeDescriptor & node_desc);
          virtual bool VisitCoyieldExpr(clang::CoyieldExpr * coyield_expr, NodeDescriptor & node_desc);
          virtual bool VisitCXXBindTemporaryExpr(clang::CXXBindTemporaryExpr * cxx_bind_temporary_expr, NodeDescriptor & node_desc);
          virtual bool VisitCXXBoolLiteralExpr(clang::CXXBoolLiteralExpr * cxx_bool_literal_expr, NodeDescriptor & node_desc);
          virtual bool VisitCXXConstructExpr(clang::CXXConstructExpr * cxx_construct_expr, NodeDescriptor & node_desc);
          virtual bool VisitCXXTemporaryObjectExpr(clang::CXXTemporaryObjectExpr * cxx_temporary_object_expr, NodeDescriptor & node_desc);
          virtual bool VisitCXXDefaultArgExpr(clang::CXXDefaultArgExpr * cxx_default_arg_expr, NodeDescriptor & node_desc);
          virtual bool VisitCXXDefaultInitExpr(clang::CXXDefaultInitExpr * cxx_default_init_expr, NodeDescriptor & node_desc);
          virtual bool VisitCXXDeleteExpr(clang::CXXDeleteExpr * cxx_delete_expr, NodeDescriptor & node_desc);
          virtual bool VisitCXXDependentScopeMemberExpr(clang::CXXDependentScopeMemberExpr * cxx_dependent_scope_member_expr, NodeDescriptor & node_desc);
          virtual bool VisitCXXFoldExpr(clang::CXXFoldExpr * cxx_fold_expr, NodeDescriptor & node_desc);
          virtual bool VisitCXXInheritedCtorInitExpr(clang::CXXInheritedCtorInitExpr * cxx_inherited_ctor_init_expr, NodeDescriptor & node_desc);
          virtual bool VisitCXXNewExpr(clang::CXXNewExpr * cxx_new_expr, NodeDescriptor & node_desc);
          virtual bool VisitCXXNoexceptExpr(clang::CXXNoexceptExpr * cxx_noexcept_expr, NodeDescriptor & node_desc);
          virtual bool VisitCXXNullPtrLiteralExpr(clang::CXXNullPtrLiteralExpr * cxx_null_ptr_literal_expr, NodeDescriptor & node_desc);
          virtual bool VisitCXXPseudoDestructorExpr(clang::CXXPseudoDestructorExpr * cxx_pseudo_destructor_expr, NodeDescriptor & node_desc);
       // virtual bool VisitCXXRewrittenBinaryOperator(clang::CXXRewrittenBinaryOperator * cxx_rewrite_binary_operator, NodeDescriptor & node_desc);
          virtual bool VisitCXXScalarValueInitExpr(clang::CXXScalarValueInitExpr * cxx_scalar_value_init_expr, NodeDescriptor & node_desc);
          virtual bool VisitCXXStdInitializerListExpr(clang::CXXStdInitializerListExpr * cxx_std_initializer_list_expr, NodeDescriptor & node_desc);
          virtual bool VisitCXXThisExpr(clang::CXXThisExpr * cxx_this_expr, NodeDescriptor & node_desc);
          virtual bool VisitCXXThrowExpr(clang::CXXThrowExpr * cxx_throw_expr, NodeDescriptor & node_desc);
          virtual bool VisitCXXTypeidExpr(clang::CXXTypeidExpr * cxx_typeid_expr, NodeDescriptor & node_desc);
          virtual bool VisitCXXUnresolvedConstructExpr(clang::CXXUnresolvedConstructExpr * cxx_unresolved_construct_expr, NodeDescriptor & node_desc);
          virtual bool VisitCXXUuidofExpr(clang::CXXUuidofExpr * cxx_uuidof_expr, NodeDescriptor & node_desc);
          virtual bool VisitDeclRefExpr(clang::DeclRefExpr * decl_ref_expr, NodeDescriptor & node_desc);
          virtual bool VisitDependentCoawaitExpr(clang::DependentCoawaitExpr * dependent_coawait_expr, NodeDescriptor & node_desc);
          virtual bool VisitDependentScopeDeclRefExpr(clang::DependentScopeDeclRefExpr * dependent_scope_decl_ref_expr, NodeDescriptor & node_desc);
          virtual bool VisitDesignatedInitExpr(clang::DesignatedInitExpr * designated_init_expr, NodeDescriptor & node_desc);
          virtual bool VisitDesignatedInitUpdateExpr(clang::DesignatedInitUpdateExpr * designated_init_update_expr, NodeDescriptor & node_desc);

          virtual bool VisitExpressionTraitExpr(clang::ExpressionTraitExpr * expression_trait_expr, NodeDescriptor & node_desc);
          virtual bool VisitExtVectorElementExpr(clang::ExtVectorElementExpr * ext_vector_element_expr, NodeDescriptor & node_desc);
          virtual bool VisitFixedPointLiteral(clang::FixedPointLiteral * fixed_point_literal, NodeDescriptor & node_desc);
          virtual bool VisitFloatingLiteral(clang::FloatingLiteral * floating_literal, NodeDescriptor & node_desc);
          virtual bool VisitFullExpr(clang::FullExpr * full_expr, NodeDescriptor & node_desc);
          virtual bool VisitConstantExpr(clang::ConstantExpr * constant_expr, NodeDescriptor & node_desc);
          virtual bool VisitExprWithCleanups(clang::ExprWithCleanups * expr_with_cleanups, NodeDescriptor & node_desc);
          virtual bool VisitFunctionParmPackExpr(clang::FunctionParmPackExpr * function_parm_pack_expr, NodeDescriptor & node_desc);
          virtual bool VisitGenericSelectionExpr(clang::GenericSelectionExpr * generic_selection_expr, NodeDescriptor & node_desc);
          virtual bool VisitGNUNullExpr(clang::GNUNullExpr * gnu_null_expr, NodeDescriptor & node_desc);
          virtual bool VisitImaginaryLiteral(clang::ImaginaryLiteral * imaginary_literal, NodeDescriptor & node_desc);
          virtual bool VisitImplicitValueInitExpr(clang::ImplicitValueInitExpr * implicit_value_init_expr, NodeDescriptor & node_desc);
          virtual bool VisitInitListExpr(clang::InitListExpr * init_list_expr, NodeDescriptor & node_desc);
          virtual bool VisitIntegerLiteral(clang::IntegerLiteral * integer_literal, NodeDescriptor & node_desc);
          virtual bool VisitLambdaExpr(clang::LambdaExpr * lambda_expr, NodeDescriptor & node_desc);
          virtual bool VisitMaterializeTemporaryExpr(clang::MaterializeTemporaryExpr * materialize_temporary_expr, NodeDescriptor & node_desc);
          virtual bool VisitMemberExpr(clang::MemberExpr * member_expr, NodeDescriptor & node_desc);
          virtual bool VisitMSPropertyRefExpr(clang::MSPropertyRefExpr * ms_property_expr, NodeDescriptor & node_desc);
          virtual bool VisitMSPropertySubscriptExpr(clang::MSPropertySubscriptExpr * ms_property_subscript_expr, NodeDescriptor & node_desc);
          virtual bool VisitNoInitExpr(clang::NoInitExpr * no_init_expr, NodeDescriptor & node_desc);
       // virtual bool VisitObjCArrayLiteral
       // virtual bool VisitObjCAvailabilityCheckExpr
       // virtual bool VisitObjCBoolLiteralExpr
       // virtual bool VisitObjCBoxedExpr
       // virtual bool VisitObjCDictionaryLiteral
       // virtual bool VisitObjCEncodeExpr
       // virtual bool VisitObjCIndirectCopyRestoreExpr
       // virtual bool VisitObjCIsaExpr
       // virtual bool VisitObjClvarRefExpr
       // virtual bool VisitObjCMessageExpr
       // virtual bool VisitObjCPropertyRefExpr
       // virtual bool VisitObjCProtocolExpr
       // virtual bool VisitObjCSelectorExpr
       // virtual bool VisitObjCCStringLiteral
       // virtual bool VisitObjCSubscriptRefexpr
          virtual bool VisitOffsetOfExpr(clang::OffsetOfExpr * offset_of_expr, NodeDescriptor & node_desc);
          virtual bool VisitOMPArraySectionExpr(RoseClangCompat::ArraySectionExpr * omp_array_section_expr, NodeDescriptor & node_desc);
          virtual bool VisitOpaqueValueExpr(clang::OpaqueValueExpr * opaque_value_expr, NodeDescriptor & node_desc);
          virtual bool VisitOverloadExpr(clang::OverloadExpr * overload_expr, NodeDescriptor & node_desc);
          virtual bool VisitUnresolvedLookupExpr(clang::UnresolvedLookupExpr * unresolved_lookup_expr, NodeDescriptor & node_desc);
          virtual bool VisitUnresolvedMemberExpr(clang::UnresolvedMemberExpr * unresolved_member_expr, NodeDescriptor & node_desc);
          virtual bool VisitPackExpansionExpr(clang::PackExpansionExpr * pack_expansion_expr, NodeDescriptor & node_desc);
          virtual bool VisitParenExpr(clang::ParenExpr * paren_expr, NodeDescriptor & node_desc);
          virtual bool VisitParenListExpr(clang::ParenListExpr * paran_list_expr, NodeDescriptor & node_desc);
          virtual bool VisitPredefinedExpr(clang::PredefinedExpr * predefined_expr, NodeDescriptor & node_desc);
          virtual bool VisitPseudoObjectExpr(clang::PseudoObjectExpr * pseudo_object_expr, NodeDescriptor & node_desc);
          virtual bool VisitShuffleVectorExpr(clang::ShuffleVectorExpr * shuffle_vector_expr, NodeDescriptor & node_desc);
          virtual bool VisitSizeOfPackExpr(clang::SizeOfPackExpr * size_of_pack_expr, NodeDescriptor & node_desc);
          virtual bool VisitSourceLocExpr(clang::SourceLocExpr * source_loc_expr, NodeDescriptor & node_desc);
          virtual bool VisitStmtExpr(clang::StmtExpr * stmt_expr, NodeDescriptor & node_desc);
          virtual bool VisitStringLiteral(clang::StringLiteral * string_literal, NodeDescriptor & node_desc);
          virtual bool VisitSubstNonTypeTemplateParmExpr(clang::SubstNonTypeTemplateParmExpr * subst_non_type_template_parm_expr, NodeDescriptor & node_desc);
          virtual bool VisitSubstNonTypeTemplateParmPackExpr(clang::SubstNonTypeTemplateParmPackExpr * subst_non_type_template_parm_pack_expr, NodeDescriptor & node_desc);
          virtual bool VisitTypeTraitExpr(clang::TypeTraitExpr * type_trait, NodeDescriptor & node_desc);
// (10/30/2026) lin32: TypoExpr was removed in Clang 21; do not declare visitors for a missing AST class.
#if CLANG_VERSION_MAJOR < 21
          virtual bool VisitTypoExpr(clang::TypoExpr * typo_expr, NodeDescriptor & node_desc);
#endif
          virtual bool VisitUnaryExprOrTypeTraitExpr(clang::UnaryExprOrTypeTraitExpr * unary_expr_or_type_trait_expr, NodeDescriptor & node_desc);
          virtual bool VisitUnaryOperator(clang::UnaryOperator * unary_operator, NodeDescriptor & node_desc);
          virtual bool VisitVAArgExpr(clang::VAArgExpr * va_arg_expr, NodeDescriptor & node_desc);
          virtual bool VisitLabelStmt(clang::LabelStmt * label_stmt, NodeDescriptor & node_desc);
          virtual bool VisitWhileStmt(clang::WhileStmt * while_stmt, NodeDescriptor & node_desc);

  /* 
     Reference: https://clang.llvm.org/doxygen/classclang_1_1Type.html 
     Overall 58 type AST nodes according as 02/19/2019
  */

          virtual bool VisitType(clang::Type * type, NodeDescriptor & node_desc);
          virtual bool VisitAdjustedType(clang::AdjustedType * adjusted_type, NodeDescriptor & node_desc);
          virtual bool VisitDecayedType(clang::DecayedType * decayed_type, NodeDescriptor & node_desc);
          virtual bool VisitArrayType(clang::ArrayType * array_type, NodeDescriptor & node_desc);
          virtual bool VisitConstantArrayType(clang::ConstantArrayType * constant_array_type, NodeDescriptor & node_desc);
          virtual bool VisitDependentSizedArrayType(clang::DependentSizedArrayType * dependent_sized_array_type, NodeDescriptor & node_desc);
          virtual bool VisitIncompleteArrayType(clang::IncompleteArrayType * incomplete_array_type, NodeDescriptor & node_desc);
          virtual bool VisitVariableArrayType(clang::VariableArrayType * variable_array_type, NodeDescriptor & node_desc);
          virtual bool VisitAtomicType(clang::AtomicType * atomic_type, NodeDescriptor & node_desc);
          virtual bool VisitAttributedType(clang::AttributedType * attributed_type, NodeDescriptor & node_desc);
          virtual bool VisitBlockPointerType(clang::BlockPointerType * block_pointer_type, NodeDescriptor & node_desc);
          virtual bool VisitBuiltinType(clang::BuiltinType * builtin_type, NodeDescriptor & node_desc);
          virtual bool VisitComplexType(clang::ComplexType * complex_type, NodeDescriptor & node_desc);
          virtual bool VisitDecltypeType(clang::DecltypeType * decltype_type, NodeDescriptor & node_desc);
          virtual bool VisitDependentDecltypeType(clang::DependentDecltypeType * dependent_decltype_type, NodeDescriptor & node_desc);
          virtual bool VisitDeducedType(clang::DeducedType * deduced_type, NodeDescriptor & node_desc);
          virtual bool VisitAutoType(clang::AutoType * auto_type, NodeDescriptor & node_desc);
          virtual bool VisitDeducedTemplateSpecializationType(clang::DeducedTemplateSpecializationType * deduced_template_specialization_type, NodeDescriptor & node_desc);
          virtual bool VisitDependentAddressSpaceType(clang::DependentAddressSpaceType * dependent_address_space_type, NodeDescriptor & node_desc);
          virtual bool VisitDependentSizedExtVectorType(clang::DependentSizedExtVectorType * dependent_sized_ext_vector_type, NodeDescriptor & node_desc);
          virtual bool VisitDependentVectorType(clang::DependentVectorType * dependent_vector_type, NodeDescriptor & node_desc);
          virtual bool VisitFunctionType(clang::FunctionType * function_type, NodeDescriptor & node_desc);
          virtual bool VisitFunctionNoProtoType(clang::FunctionNoProtoType * function_no_proto_type, NodeDescriptor & node_desc);
          virtual bool VisitFunctionProtoType(clang::FunctionProtoType * function_proass_symo_type, NodeDescriptor & node_desc);
          virtual bool VisitInjectedClassNameType(clang::InjectedClassNameType * injected_class_name_type, NodeDescriptor & node_desc);
          virtual bool VisitLocInfoType(clang::LocInfoType * loc_info_type, NodeDescriptor & node_desc);
          virtual bool VisitMacroQualifiedType(clang::MacroQualifiedType * macro_qualified_type, NodeDescriptor & node_desc);
          virtual bool VisitMemberPointerType(clang::MemberPointerType * member_pointer_type, NodeDescriptor & node_desc);
       // virtual bool VisitObjCObjectPointerType
       // virtual bool VisitObjCObjectType
       // virtual bool VisitObjCTypeParamType
          virtual bool VisitPackExpansionType(clang::PackExpansionType * pack_expansion_type, NodeDescriptor & node_desc);
          virtual bool VisitParenType(clang::ParenType * paren_type, NodeDescriptor & node_desc);
          virtual bool VisitPipeType(clang::PipeType * pipe_type, NodeDescriptor & node_desc);
          virtual bool VisitPointerType(clang::PointerType * pointer_type, NodeDescriptor & node_desc);
          virtual bool VisitReferenceType(clang::ReferenceType * reference_type, NodeDescriptor & node_desc);
          virtual bool VisitLValueReferenceType(clang::LValueReferenceType * lvalue_reference_type, NodeDescriptor & node_desc);
          virtual bool VisitRValueReferenceType(clang::RValueReferenceType * rvalue_reference_type, NodeDescriptor & node_desc);
          virtual bool VisitSubstTemplateTypeParmPackType(clang::SubstTemplateTypeParmPackType * subst_template_type_parm_pack_type, NodeDescriptor & node_desc);
          virtual bool VisitSubstTemplateTypeParmType(clang::SubstTemplateTypeParmType * subst_template_type_parm_type, NodeDescriptor & node_desc);
          virtual bool VisitTagType(clang::TagType * tag_type, NodeDescriptor & node_desc);
          virtual bool VisitEnumType(clang::EnumType * enum_type, NodeDescriptor & node_desc);
          virtual bool VisitRecordType(clang::RecordType * record_type, NodeDescriptor & node_desc);
          virtual bool VisitTemplateSpecializationType(clang::TemplateSpecializationType * template_specialization_type, NodeDescriptor & node_desc);
          virtual bool VisitTemplateTypeParmType(clang::TemplateTypeParmType * template_type_parm_type, NodeDescriptor & node_desc);
          virtual bool VisitTypedefType(clang::TypedefType * typedef_type, NodeDescriptor & node_desc);
          virtual bool VisitTypeOfExprType(clang::TypeOfExprType * type_of_expr_type, NodeDescriptor & node_desc);
          virtual bool VisitDependentTypeOfExprType(clang::DependentTypeOfExprType * dependent_type_of_expr_type, NodeDescriptor & node_desc);
          virtual bool VisitTypeOfType(clang::TypeOfType * type_of_type, NodeDescriptor & node_desc);
          virtual bool VisitTypeWithKeyword(clang::TypeWithKeyword * type_with_keyword, NodeDescriptor & node_desc);
          virtual bool VisitDependentNameType(clang::DependentNameType * dependent_name_type, NodeDescriptor & node_desc);
// (10/30/2026) lin32: Clang 22 removed ElaboratedType and merged dependent specializations into TemplateSpecializationType.
#if CLANG_VERSION_MAJOR < 22
          virtual bool VisitDependentTemplateSpecializationType(clang::DependentTemplateSpecializationType * dependent_template_specialization_type, NodeDescriptor & node_desc);
          virtual bool VisitElaboratedType(clang::ElaboratedType * elaborated_type, NodeDescriptor & node_desc);
#endif
          virtual bool VisitUnaryTransformType(clang::UnaryTransformType * unary_transform_type, NodeDescriptor & node_desc);
// (10/30/2026) lin32: Clang 21 folded DependentUnaryTransformType into UnaryTransformType.
#if CLANG_VERSION_MAJOR < 21
          virtual bool VisitDependentUnaryTransformType(clang::DependentUnaryTransformType * dependent_unary_transform_type, NodeDescriptor & node_desc);
#endif
          virtual bool VisitUnresolvedUsingType(clang::UnresolvedUsingType * unresolved_using_type, NodeDescriptor & node_desc);
          virtual bool VisitVectorType(clang::VectorType * vector_type, NodeDescriptor & node_desc);
          virtual bool VisitExtVectorType(clang::ExtVectorType * ext_vector_type, NodeDescriptor & node_desc);

       // Preprocessing access
          std::pair<Sg_File_Info *, PreprocessingInfo *> preprocessor_top();
          bool preprocessor_pop();
   };


void finishSageAST(ClangToDotTranslator & translator);

// class SagePreprocessorRecord : public clang::PPCallbacks {
class ClangToDotPreprocessorRecord : public clang::PPCallbacks 
   {
     public:

     protected:
          static Sawyer::Message::Facility logger;
          clang::SourceManager * p_source_manager;

          std::vector<std::pair<Sg_File_Info *, PreprocessingInfo *> > p_preprocessor_record_list;

     public:
       // SagePreprocessorRecord(clang::SourceManager * source_manager);
          ClangToDotPreprocessorRecord(clang::SourceManager * source_manager);

          void InclusionDirective(clang::SourceLocation HashLoc, const clang::Token & IncludeTok, llvm::StringRef FileName, bool IsAngled,
                                  const clang::FileEntry * File, clang::SourceLocation EndLoc, llvm::StringRef SearchPath, llvm::StringRef RelativePath);
          void EndOfMainFile();
          void Ident(clang::SourceLocation Loc, const std::string & str);
          void PragmaComment(clang::SourceLocation Loc, const clang::IdentifierInfo * Kind, const std::string & Str);
          void PragmaMessage(clang::SourceLocation Loc, llvm::StringRef Str);
          void PragmaDiagnosticPush(clang::SourceLocation Loc, llvm::StringRef Namespace);
          void PragmaDiagnosticPop(clang::SourceLocation Loc, llvm::StringRef Namespace);
          void PragmaDiagnostic(clang::SourceLocation Loc, llvm::StringRef Namespace, clang::diag::Severity Severity, llvm::StringRef Str);
          void MacroExpands(const clang::Token & MacroNameTok, const clang::MacroInfo * MI, clang::SourceRange Range);
          void MacroDefined(const clang::Token & MacroNameTok, const clang::MacroInfo * MI);
          void MacroUndefined(const clang::Token & MacroNameTok, const clang::MacroInfo * MI);
          void Defined(const clang::Token & MacroNameTok);
          void SourceRangeSkipped(clang::SourceRange Range);
          void If(clang::SourceRange Range);
          void Elif(clang::SourceRange Range);
          void Ifdef(const clang::Token & MacroNameTok);
          void Ifndef(const clang::Token & MacroNameTok);
          void Else();
          void Endif();

          std::pair<Sg_File_Info *, PreprocessingInfo *> top();
          bool pop(); 
   };


// struct NextPreprocessorToInsert 
struct ClangToDotNextPreprocessorToInsert 
   {
     Sg_File_Info * cursor;
     SgLocatedNode * candidat;
     PreprocessingInfo * next_to_insert;
     ClangToDotTranslator & translator;

     ClangToDotNextPreprocessorToInsert(ClangToDotTranslator &);

     ClangToDotNextPreprocessorToInsert * next();
   };

// class PreprocessorInserter : public AstTopDownProcessing<NextPreprocessorToInsert *> 
class ClangToDotPreprocessorInserter : public AstTopDownProcessing<ClangToDotNextPreprocessorToInsert *> 
   {
     public:
          ClangToDotNextPreprocessorToInsert * evaluateInheritedAttribute(SgNode * astNode, ClangToDotNextPreprocessorToInsert * inheritedValue);
   };

#endif /* _CLANG_TO_DOT_FRONTEND_PRIVATE_HPP_ */
