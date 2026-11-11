#include "sage3basic.h"
#include "clang-frontend-private.hpp"
#include "clang-to-rose-support.hpp" 

using namespace Sawyer::Message;
SgNode * ClangToSageTranslator::Traverse(clang::Stmt * stmt) {
    if (stmt == NULL)
        return NULL;

    std::map<clang::Stmt *, SgNode *>::iterator it = p_stmt_translation_map.find(stmt);
    if (it != p_stmt_translation_map.end())
        return it->second; 

    SgNode * result = NULL;
    bool ret_status = false;

    switch (stmt->getStmtClass()) {
        case clang::Stmt::GCCAsmStmtClass:
            ret_status = VisitGCCAsmStmt((clang::GCCAsmStmt *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::MSAsmStmtClass:
            ret_status = VisitMSAsmStmt((clang::MSAsmStmt *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::BreakStmtClass:
            ret_status = VisitBreakStmt((clang::BreakStmt *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::CapturedStmtClass:
            ret_status = VisitCapturedStmt((clang::CapturedStmt *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::CompoundStmtClass:
            ret_status = VisitCompoundStmt((clang::CompoundStmt *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::ContinueStmtClass:
            ret_status = VisitContinueStmt((clang::ContinueStmt *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::CoreturnStmtClass:
            ret_status = VisitCoreturnStmt((clang::CoreturnStmt *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::CXXCatchStmtClass:
            ret_status = VisitCXXCatchStmt((clang::CXXCatchStmt *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::CXXForRangeStmtClass:
            ret_status = VisitCXXForRangeStmt((clang::CXXForRangeStmt *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::CXXTryStmtClass:
            ret_status = VisitCXXTryStmt((clang::CXXTryStmt *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::DeclStmtClass:
            ret_status = VisitDeclStmt((clang::DeclStmt *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::DoStmtClass:
            ret_status = VisitDoStmt((clang::DoStmt *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::ForStmtClass:
            ret_status = VisitForStmt((clang::ForStmt *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::GotoStmtClass:
            ret_status = VisitGotoStmt((clang::GotoStmt *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::IfStmtClass:
            ret_status = VisitIfStmt((clang::IfStmt *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::IndirectGotoStmtClass:
            ret_status = VisitIndirectGotoStmt((clang::IndirectGotoStmt *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::MSDependentExistsStmtClass:
            ret_status = VisitMSDependentExistsStmt((clang::MSDependentExistsStmt *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::NullStmtClass:
            ret_status = VisitNullStmt((clang::NullStmt *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::OMPAtomicDirectiveClass:
            ret_status = VisitOMPAtomicDirective((clang::OMPAtomicDirective *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::OMPBarrierDirectiveClass:
            ret_status = VisitOMPBarrierDirective((clang::OMPBarrierDirective *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::OMPCancellationPointDirectiveClass:
            ret_status = VisitOMPCancellationPointDirective((clang::OMPCancellationPointDirective *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::OMPCriticalDirectiveClass:
            ret_status = VisitOMPCriticalDirective((clang::OMPCriticalDirective *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::OMPFlushDirectiveClass:
            ret_status = VisitOMPFlushDirective((clang::OMPFlushDirective *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::OMPDistributeDirectiveClass:
            ret_status = VisitOMPDistributeDirective((clang::OMPDistributeDirective *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::OMPDistributeParallelForDirectiveClass:
            ret_status = VisitOMPDistributeParallelForDirective((clang::OMPDistributeParallelForDirective *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::OMPDistributeParallelForSimdDirectiveClass:
            ret_status = VisitOMPDistributeParallelForSimdDirective((clang::OMPDistributeParallelForSimdDirective *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::OMPDistributeSimdDirectiveClass:
            ret_status = VisitOMPDistributeSimdDirective((clang::OMPDistributeSimdDirective *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::OMPForDirectiveClass:
            ret_status = VisitOMPForDirective((clang::OMPForDirective *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::OMPForSimdDirectiveClass:
            ret_status = VisitOMPForSimdDirective((clang::OMPForSimdDirective *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        //case clang::Stmt::OMPMasterTaskLoopDirectiveClass:
        //    ret_status = VisitOMPMasterTaskLoopDirective((clang::OMPMasterTaskLoopDirective *)stmt, &result);
        //    break;
        //case clang::Stmt::OMPMasterTaskLoopSimdDirectiveClass:
        //    ret_status = VisitOMPMasterTaskLoopSimdDirective((clang::OMPMasterTaskLoopSimdDirective *)stmt, &result);
        //    break;
        case clang::Stmt::OMPParallelForDirectiveClass:
            ret_status = VisitOMPParallelForDirective((clang::OMPParallelForDirective *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::OMPParallelForSimdDirectiveClass:
            ret_status = VisitOMPParallelForSimdDirective((clang::OMPParallelForSimdDirective *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        //case clang::Stmt::OMPParallelMasterTaskLoopDirectiveClass:
        //    ret_status = VisitOMPParallelMasterTaskLoopDirective((clang::OMPParallelMasterTaskLoopDirective *)stmt, &result);
        //    break;
        case clang::Stmt::OMPSimdDirectiveClass:
            ret_status = VisitOMPSimdDirective((clang::OMPSimdDirective *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::OMPTargetParallelForDirectiveClass:
            ret_status = VisitOMPTargetParallelForDirective((clang::OMPTargetParallelForDirective *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::OMPTargetParallelForSimdDirectiveClass:
            ret_status = VisitOMPTargetParallelForSimdDirective((clang::OMPTargetParallelForSimdDirective *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::OMPTargetSimdDirectiveClass:
            ret_status = VisitOMPTargetSimdDirective((clang::OMPTargetSimdDirective *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::OMPTargetTeamsDistributeDirectiveClass:
            ret_status = VisitOMPTargetTeamsDistributeDirective((clang::OMPTargetTeamsDistributeDirective *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        //case clang::Stmt::OMPTargetTeamsDistributeParallelForSimdDirectiveClass:
        //    ret_status = VisitOMPTargetTeamsDistributeParallelForSimdDirective((clang::OMPTargetTeamsDistributeParallelForSimdDirective *)stmt, &result);
        //    break;
        case clang::Stmt::OMPTargetTeamsDistributeSimdDirectiveClass:
            ret_status = VisitOMPTargetTeamsDistributeSimdDirective((clang::OMPTargetTeamsDistributeSimdDirective *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::OMPTaskLoopDirectiveClass:
            ret_status = VisitOMPTaskLoopDirective((clang::OMPTaskLoopDirective *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::OMPTaskLoopSimdDirectiveClass:
            ret_status = VisitOMPTaskLoopSimdDirective((clang::OMPTaskLoopSimdDirective *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        //case clang::Stmt::OMPTeamDistributeDirectiveClass:
        //    ret_status = VisitOMPTeamDistributeDirective((clang::OMPTeamDistributeDirective *)stmt, &result);
        //    break;
        //case clang::Stmt::OMPTeamDistributeParallelForSimdDirectiveClass:
        //    ret_status = VisitOMPTeamDistributeParallelForSimdDirective((clang::OMPTeamDistributeParallelForSimdDirective *)stmt, &result);
        //    break;
        //case clang::Stmt::OMPTeamDistributeSimdDirectiveClass:
        //    ret_status = VisitOMPTeamDistributeSimdDirective((clang::OMPTeamDistributeSimdDirective *)stmt, &result);
        //    break;
        case clang::Stmt::OMPMasterDirectiveClass:
            ret_status = VisitOMPMasterDirective((clang::OMPMasterDirective *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::OMPOrderedDirectiveClass:
            ret_status = VisitOMPOrderedDirective((clang::OMPOrderedDirective *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::OMPParallelDirectiveClass:
            ret_status = VisitOMPParallelDirective((clang::OMPParallelDirective *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::OMPParallelSectionsDirectiveClass:
            ret_status = VisitOMPParallelSectionsDirective((clang::OMPParallelSectionsDirective *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::ReturnStmtClass:
            ret_status = VisitReturnStmt((clang::ReturnStmt *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::SEHExceptStmtClass:
            ret_status = VisitSEHExceptStmt((clang::SEHExceptStmt *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::SEHFinallyStmtClass:
            ret_status = VisitSEHFinallyStmt((clang::SEHFinallyStmt *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::SEHLeaveStmtClass:
            ret_status = VisitSEHLeaveStmt((clang::SEHLeaveStmt *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::SEHTryStmtClass:
            ret_status = VisitSEHTryStmt((clang::SEHTryStmt *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::CaseStmtClass:
            ret_status = VisitCaseStmt((clang::CaseStmt *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::DefaultStmtClass:
            ret_status = VisitDefaultStmt((clang::DefaultStmt *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::SwitchStmtClass:
            ret_status = VisitSwitchStmt((clang::SwitchStmt *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::AttributedStmtClass:
            ret_status = VisitAttributedStmt((clang::AttributedStmt *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::BinaryConditionalOperatorClass:
            ret_status = VisitBinaryConditionalOperator((clang::BinaryConditionalOperator *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::ConditionalOperatorClass:
            ret_status = VisitConditionalOperator((clang::ConditionalOperator *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::AddrLabelExprClass:
            ret_status = VisitAddrLabelExpr((clang::AddrLabelExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::ArrayInitIndexExprClass:
            ret_status = VisitArrayInitIndexExpr((clang::ArrayInitIndexExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::ArrayInitLoopExprClass:
            ret_status = VisitArrayInitLoopExpr((clang::ArrayInitLoopExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::ArraySubscriptExprClass:
            ret_status = VisitArraySubscriptExpr((clang::ArraySubscriptExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::ArrayTypeTraitExprClass:
            ret_status = VisitArrayTypeTraitExpr((clang::ArrayTypeTraitExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::AsTypeExprClass:
            ret_status = VisitAsTypeExpr((clang::AsTypeExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::AtomicExprClass:
            ret_status = VisitAtomicExpr((clang::AtomicExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::CompoundAssignOperatorClass:
            ret_status = VisitCompoundAssignOperator((clang::CompoundAssignOperator *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::BlockExprClass:
            ret_status = VisitBlockExpr((clang::BlockExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::CUDAKernelCallExprClass:
            ret_status = VisitCUDAKernelCallExpr((clang::CUDAKernelCallExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::CXXMemberCallExprClass:
            ret_status = VisitCXXMemberCallExpr((clang::CXXMemberCallExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::CXXOperatorCallExprClass:
            ret_status = VisitCXXOperatorCallExpr((clang::CXXOperatorCallExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::UserDefinedLiteralClass:
            ret_status = VisitUserDefinedLiteral((clang::UserDefinedLiteral *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::BuiltinBitCastExprClass:
            ret_status = VisitBuiltinBitCastExpr((clang::BuiltinBitCastExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::CStyleCastExprClass:
            ret_status = VisitCStyleCastExpr((clang::CStyleCastExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::CXXFunctionalCastExprClass:
            ret_status = VisitCXXFunctionalCastExpr((clang::CXXFunctionalCastExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::CXXConstCastExprClass:
            ret_status = VisitCXXConstCastExpr((clang::CXXConstCastExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::CXXDynamicCastExprClass:
            ret_status = VisitCXXDynamicCastExpr((clang::CXXDynamicCastExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::CXXReinterpretCastExprClass:
            ret_status = VisitCXXReinterpretCastExpr((clang::CXXReinterpretCastExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::CXXStaticCastExprClass:
            ret_status = VisitCXXStaticCastExpr((clang::CXXStaticCastExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::ImplicitCastExprClass:
            ret_status = VisitImplicitCastExpr((clang::ImplicitCastExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::CharacterLiteralClass:
            ret_status = VisitCharacterLiteral((clang::CharacterLiteral *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::ChooseExprClass:
            ret_status = VisitChooseExpr((clang::ChooseExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::CompoundLiteralExprClass:
            ret_status = VisitCompoundLiteralExpr((clang::CompoundLiteralExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        //case clang::Stmt::ConceptSpecializationExprClass:
        //    ret_status = VisitConceptSpecializationExpr((clang::ConceptSpecializationExpr *)stmt, &result);
        //    break;
        case clang::Stmt::ConvertVectorExprClass:
            ret_status = VisitConvertVectorExpr((clang::ConvertVectorExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::CoawaitExprClass:
            ret_status = VisitCoawaitExpr((clang::CoawaitExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::CoyieldExprClass:
            ret_status = VisitCoyieldExpr((clang::CoyieldExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::CXXBindTemporaryExprClass:
            ret_status = VisitCXXBindTemporaryExpr((clang::CXXBindTemporaryExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::CXXBoolLiteralExprClass:
            ret_status = VisitCXXBoolLiteralExpr((clang::CXXBoolLiteralExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::CXXConstructExprClass:
            ret_status = VisitCXXConstructExpr((clang::CXXConstructExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::CXXTemporaryObjectExprClass:
            ret_status = VisitCXXTemporaryObjectExpr((clang::CXXTemporaryObjectExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::CXXDefaultArgExprClass:
            ret_status = VisitCXXDefaultArgExpr((clang::CXXDefaultArgExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::CXXDefaultInitExprClass:
            ret_status = VisitCXXDefaultInitExpr((clang::CXXDefaultInitExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::CXXDeleteExprClass:
            ret_status = VisitCXXDeleteExpr((clang::CXXDeleteExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::CXXDependentScopeMemberExprClass:
            ret_status = VisitCXXDependentScopeMemberExpr((clang::CXXDependentScopeMemberExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::CXXFoldExprClass:
            ret_status = VisitCXXFoldExpr((clang::CXXFoldExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::CXXInheritedCtorInitExprClass:
            ret_status = VisitCXXInheritedCtorInitExpr((clang::CXXInheritedCtorInitExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::CXXNewExprClass:
            ret_status = VisitCXXNewExpr((clang::CXXNewExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::CXXNoexceptExprClass:
            ret_status = VisitCXXNoexceptExpr((clang::CXXNoexceptExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::CXXNullPtrLiteralExprClass:
            ret_status = VisitCXXNullPtrLiteralExpr((clang::CXXNullPtrLiteralExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::CXXPseudoDestructorExprClass:
            ret_status = VisitCXXPseudoDestructorExpr((clang::CXXPseudoDestructorExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        //case clang::Stmt::CXXRewrittenBinaryOperatorClass:
        //    ret_status = VisitCXXRewrittenBinaryOperator((clang::CXXRewrittenBinaryOperator *)stmt, &result);
        //    break;
        case clang::Stmt::CXXScalarValueInitExprClass:
            ret_status = VisitCXXScalarValueInitExpr((clang::CXXScalarValueInitExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::CXXStdInitializerListExprClass:
            ret_status = VisitCXXStdInitializerListExpr((clang::CXXStdInitializerListExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::CXXThisExprClass:
            ret_status = VisitCXXThisExpr((clang::CXXThisExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::CXXThrowExprClass:
            ret_status = VisitCXXThrowExpr((clang::CXXThrowExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::CXXTypeidExprClass:
            ret_status = VisitCXXTypeidExpr((clang::CXXTypeidExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::CXXUnresolvedConstructExprClass:
            ret_status = VisitCXXUnresolvedConstructExpr((clang::CXXUnresolvedConstructExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::CXXUuidofExprClass:
            ret_status = VisitCXXUuidofExpr((clang::CXXUuidofExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::DeclRefExprClass:
            ret_status = VisitDeclRefExpr((clang::DeclRefExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::DependentCoawaitExprClass:
            ret_status = VisitDependentCoawaitExpr((clang::DependentCoawaitExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::DependentScopeDeclRefExprClass:
            ret_status = VisitDependentScopeDeclRefExpr((clang::DependentScopeDeclRefExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::DesignatedInitExprClass:
            ret_status = VisitDesignatedInitExpr((clang::DesignatedInitExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::DesignatedInitUpdateExprClass:
            ret_status = VisitDesignatedInitUpdateExpr((clang::DesignatedInitUpdateExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::ExpressionTraitExprClass:
            ret_status = VisitExpressionTraitExpr((clang::ExpressionTraitExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::ExtVectorElementExprClass:
            ret_status = VisitExtVectorElementExpr((clang::ExtVectorElementExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::FixedPointLiteralClass:
            ret_status = VisitFixedPointLiteral((clang::FixedPointLiteral *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::FloatingLiteralClass:
            ret_status = VisitFloatingLiteral((clang::FloatingLiteral *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::ConstantExprClass:
            ret_status = VisitConstantExpr((clang::ConstantExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::ExprWithCleanupsClass:
            ret_status = VisitExprWithCleanups((clang::ExprWithCleanups *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::FunctionParmPackExprClass:
            ret_status = VisitFunctionParmPackExpr((clang::FunctionParmPackExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::GenericSelectionExprClass:
            ret_status = VisitGenericSelectionExpr((clang::GenericSelectionExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::GNUNullExprClass:
            ret_status = VisitGNUNullExpr((clang::GNUNullExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::ImaginaryLiteralClass:
            ret_status = VisitImaginaryLiteral((clang::ImaginaryLiteral *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::ImplicitValueInitExprClass:
            ret_status = VisitImplicitValueInitExpr((clang::ImplicitValueInitExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::InitListExprClass:
            ret_status = VisitInitListExpr((clang::InitListExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::IntegerLiteralClass:
            ret_status = VisitIntegerLiteral((clang::IntegerLiteral *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::LambdaExprClass:
            ret_status = VisitLambdaExpr((clang::LambdaExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::MaterializeTemporaryExprClass:
            ret_status = VisitMaterializeTemporaryExpr((clang::MaterializeTemporaryExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::MemberExprClass:
            ret_status = VisitMemberExpr((clang::MemberExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::MSPropertyRefExprClass:
            ret_status = VisitMSPropertyRefExpr((clang::MSPropertyRefExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::MSPropertySubscriptExprClass:
            ret_status = VisitMSPropertySubscriptExpr((clang::MSPropertySubscriptExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::NoInitExprClass:
            ret_status = VisitNoInitExpr((clang::NoInitExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::OffsetOfExprClass:
            ret_status = VisitOffsetOfExpr((clang::OffsetOfExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case RoseClangCompat::ArraySectionExprClass:
            ret_status = VisitOMPArraySectionExpr((RoseClangCompat::ArraySectionExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::OpaqueValueExprClass:
            ret_status = VisitOpaqueValueExpr((clang::OpaqueValueExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::UnresolvedLookupExprClass:
            ret_status = VisitUnresolvedLookupExpr((clang::UnresolvedLookupExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::UnresolvedMemberExprClass:
            ret_status = VisitUnresolvedMemberExpr((clang::UnresolvedMemberExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::PackExpansionExprClass:
            ret_status = VisitPackExpansionExpr((clang::PackExpansionExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::ParenExprClass:
            ret_status = VisitParenExpr((clang::ParenExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::ParenListExprClass:
            ret_status = VisitParenListExpr((clang::ParenListExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::PredefinedExprClass:
            ret_status = VisitPredefinedExpr((clang::PredefinedExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::PseudoObjectExprClass:
            ret_status = VisitPseudoObjectExpr((clang::PseudoObjectExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::ShuffleVectorExprClass:
            ret_status = VisitShuffleVectorExpr((clang::ShuffleVectorExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::SizeOfPackExprClass:
            ret_status = VisitSizeOfPackExpr((clang::SizeOfPackExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::SourceLocExprClass:
            ret_status = VisitSourceLocExpr((clang::SourceLocExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::StmtExprClass:
            ret_status = VisitStmtExpr((clang::StmtExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::StringLiteralClass:
            ret_status = VisitStringLiteral((clang::StringLiteral *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::SubstNonTypeTemplateParmPackExprClass:
            ret_status = VisitSubstNonTypeTemplateParmPackExpr((clang::SubstNonTypeTemplateParmPackExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::TypeTraitExprClass:
            ret_status = VisitTypeTraitExpr((clang::TypeTraitExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        // (10/30/2026) lin32: Clang 21 removed TypoExpr and its statement kind; exclude the old dispatch case.
#if CLANG_VERSION_MAJOR < 21
        case clang::Stmt::TypoExprClass:
            ret_status = VisitTypoExpr((clang::TypoExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
#endif
        case clang::Stmt::UnaryExprOrTypeTraitExprClass:
            ret_status = VisitUnaryExprOrTypeTraitExpr((clang::UnaryExprOrTypeTraitExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::VAArgExprClass:
            ret_status = VisitVAArgExpr((clang::VAArgExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::LabelStmtClass:
            ret_status = VisitLabelStmt((clang::LabelStmt *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::WhileStmtClass:
            ret_status = VisitWhileStmt((clang::WhileStmt *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::UnaryOperatorClass:
            ret_status = VisitUnaryOperator((clang::UnaryOperator *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::CallExprClass:
            ret_status = VisitCallExpr((clang::CallExpr *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::BinaryOperatorClass:
            ret_status = VisitBinaryOperator((clang::BinaryOperator *)stmt, &result);
            ROSE_ASSERT(result != NULL);
            break;
        case clang::Stmt::RecoveryExprClass:
            result = SageBuilder::buildIntVal(42);
            ROSE_ASSERT(FAIL_FIXME == 0); // There is no concept of recovery expression in ROSE
            break;

        default:
            logger[ERROR] << "Unknown statement kind: " << stmt->getStmtClassName() << " !" << "\n";
            ROSE_ABORT();
    }

    ROSE_ASSERT(result != NULL);

    p_stmt_translation_map.insert(std::pair<clang::Stmt *, SgNode *>(stmt, result));

    return result;
}

/********************/
/* Visit Statements */
/********************/

bool ClangToSageTranslator::VisitStmt(clang::Stmt * stmt, SgNode ** node)
   {
#if DEBUG_VISIT_STMT
     logger[DEBUG] << "ClangToSageTranslator::VisitStmt" << "\n";
#endif

     if (*node == NULL) 
        {
          logger[WARN] << "Runtime error: No Sage node associated with the Statement..." << "\n";
          return false;
        }

  // TODO Is there anything else todo?

     if (isSgLocatedNode(*node) != NULL && (isSgLocatedNode(*node)->get_file_info() == NULL || !(isSgLocatedNode(*node)->get_file_info()->isCompilerGenerated()) ))
        {
          applySourceRange(*node, stmt->getSourceRange());
        }

     return true;
   }

bool ClangToSageTranslator::VisitAsmStmt(clang::AsmStmt * asm_stmt, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitAsmStmt" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO
    return VisitStmt(asm_stmt, node) && res;
}


bool ClangToSageTranslator::VisitGCCAsmStmt(clang::GCCAsmStmt * gcc_asm_stmt, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitGCCAsmStmt" << "\n";
#endif
    bool res = true;

    unsigned asmNumInput = gcc_asm_stmt->getNumInputs(); 
    unsigned asmNumOutput = gcc_asm_stmt->getNumOutputs(); 
    unsigned asmClobber = gcc_asm_stmt->getNumClobbers(); 

    // (10/30/2026) lin32: Clang 21 GCCAsmStmt::getAsmString() returns a std::string, matching file-scope assembly.
#if CLANG_VERSION_MAJOR >= 21
    std::string AsmStringRef = gcc_asm_stmt->getAsmString();
#else
    clang::StringLiteral* AsmStringLiteral = gcc_asm_stmt->getAsmString();
    llvm::StringRef AsmStringRef = AsmStringLiteral->getString();
#endif

    std::cout << "input op:" << asmNumInput << " output op: " << asmNumOutput<< "\n";
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "AsmStringRef:" << static_cast<std::string>(AsmStringRef) << "\n";
#endif

    SgAsmStmt* asmStmt = SageBuilder::buildAsmStatement(static_cast<std::string>(AsmStringRef)); 
    asmStmt->set_firstNondefiningDeclaration(asmStmt);
    asmStmt->set_definingDeclaration(asmStmt);
    asmStmt->set_parent(SageBuilder::topScopeStack());
    asmStmt->set_useGnuExtendedFormat(true);


    // Pei-Hung (03/22/2022)  The clobber string is available.
    // The implementation adding clobber into ROSE AST is not in place.
    for(unsigned i=0; i < asmClobber; ++i)
    {
      std::string clobberStr = static_cast<std::string>(gcc_asm_stmt->getClobber(i));
#if DEBUG_VISIT_STMT
      logger[DEBUG] << "AsmOp clobber["<< i<<  "]: " << clobberStr << "\n";
#endif
      // Pei-Hung "cc" clobber is skipped by EDG
      if(clobberStr.compare(0, sizeof(clobberStr), "cc") == 0)
        continue;
  
      SgInitializedName::asm_register_name_enum sageRegisterName = get_sgAsmRegister(clobberStr);
      asmStmt->get_clobberRegisterList().push_back(sageRegisterName);
    }

    // Pei-Hung (03/22/2022) use regular expression to check the first modifier, + and =, for ouput Ops.  
    // Then the second modifier for both input and output Ops.  The rest is for constraints.
    // regex_match should report 4 matched results:
    // 1. the whole matched string
    // 2. first modifier: =, +, or empty
    // 3. second modifier: empty or &, %, *, #, ?, !
    // 4. The constraint
    std::regex e ("([\\=\\+]*)([\\&\\%\\*\\#\\?\\!]*)(.+)", std::regex_constants::ECMAScript | std::regex_constants::icase);

    // process output
    for(unsigned i=0; i < asmNumOutput; ++i)
    {
      SgNode* tmp_node = Traverse(gcc_asm_stmt->getOutputExpr(i));
      SgExpression * outputExpr = isSgExpression(tmp_node);
      ROSE_ASSERT(outputExpr != NULL);

      std::string outputConstraintStr = static_cast<std::string>(gcc_asm_stmt->getOutputConstraint(i));
// Clang's constraint is equivalent to ROSE's modifier + operand constraints 
#if DEBUG_VISIT_STMT
      logger[DEBUG] << "AsmOp output constraint["<< i<<  "]: " << outputConstraintStr << "\n";
#endif

      std::smatch sm; 
      std::regex_match (outputConstraintStr,sm,e);
#if DEBUG_VISIT_STMT
        std::cout << "string literal: "<< outputConstraintStr  <<"  with " << sm.size() << " matches\n";
        if(sm.size())
          std::cout << "the matches were: ";
        for (unsigned i=0; i<sm.size(); ++i) {
          std::cout << "[" << sm[i] << "] \n";
        }
        if(sm.size())
          std::cout << "\n";
#endif

      SgAsmOp::asm_operand_constraint_enum constraint = (SgAsmOp::asm_operand_constraint_enum) SgAsmOp::e_any;
      SgAsmOp::asm_operand_modifier_enum   modifiers  = (SgAsmOp::asm_operand_modifier_enum)   SgAsmOp::e_unknown;
      SgAsmOp* sageAsmOp = new SgAsmOp(constraint,modifiers,outputExpr);
      outputExpr->set_parent(sageAsmOp);

      sageAsmOp->set_recordRawAsmOperandDescriptions(false);

      // set as an output AsmOp
      sageAsmOp->set_isOutputOperand (true);

      ROSE_ASSERT(sm.size() == 4);

      unsigned modifierVal = static_cast<int>(modifiers);
      if(!sm[1].str().empty())
        modifierVal += static_cast<int>(get_sgAsmOperandModifier(sm[1].str()));

      if(!sm[2].str().empty())
        modifierVal += static_cast<int>(get_sgAsmOperandModifier(sm[2].str()));
     
      sageAsmOp->set_modifiers(static_cast<SgAsmOp::asm_operand_modifier_enum>(modifierVal));

      // set constraint
      sageAsmOp->set_constraint(get_sgAsmOperandConstraint(sm[3].str()));
      sageAsmOp->set_constraintString(sm[3]);


      Sg_File_Info * start_fi = Sg_File_Info::generateDefaultFileInfoForCompilerGeneratedNode();
      start_fi->setCompilerGenerated();
      sageAsmOp->set_startOfConstruct(start_fi);

      Sg_File_Info * end_fi   = Sg_File_Info::generateDefaultFileInfoForCompilerGeneratedNode();
      end_fi->setCompilerGenerated();
      sageAsmOp->set_endOfConstruct(end_fi);
      
      asmStmt->get_operands().push_back(sageAsmOp);
      sageAsmOp->set_parent(asmStmt);
    }

    // process input
    for(unsigned i=0; i < asmNumInput; ++i)
    {
      SgNode* tmp_node = Traverse(gcc_asm_stmt->getInputExpr(i));
      SgExpression * inputExpr = isSgExpression(tmp_node);
      ROSE_ASSERT(inputExpr != NULL);

      std::string inputConstraintStr = static_cast<std::string>(gcc_asm_stmt->getInputConstraint(i));
// Clang's constraint is equivalent to ROSE's modifier + operand constraints 
#if DEBUG_VISIT_STMT
      logger[DEBUG] << "AsmOp input constraint["<< i<<  "]: " << inputConstraintStr << "\n";
#endif

      std::smatch sm; 
      std::regex_match (inputConstraintStr,sm,e);
#if DEBUG_VISIT_STMT
        std::cout << "string literal: "<< inputConstraintStr  <<"  with " << sm.size() << " matches\n";
        if(sm.size())
          std::cout << "the matches were: ";
        for (unsigned i=0; i<sm.size(); ++i) {
          std::cout << "[" << sm[i] << "] \n";
        }
        if(sm.size())
          std::cout << "\n";
#endif

      SgAsmOp::asm_operand_constraint_enum constraint = (SgAsmOp::asm_operand_constraint_enum) SgAsmOp::e_any;
      SgAsmOp::asm_operand_modifier_enum   modifiers  = (SgAsmOp::asm_operand_modifier_enum)   SgAsmOp::e_unknown;
      SgAsmOp* sageAsmOp = new SgAsmOp(constraint,modifiers,inputExpr);
      inputExpr->set_parent(sageAsmOp);

      sageAsmOp->set_recordRawAsmOperandDescriptions(false);

      // set as an input AsmOp
      sageAsmOp->set_isOutputOperand (false);

      ROSE_ASSERT(sm.size() == 4);

      unsigned modifierVal = static_cast<int>(modifiers);

      // "+" and "=" should not be part of the input AsmOp.  Skip checking sm[1] for the inputs.

//      if(!sm[1].str().empty())
//        modifierVal += static_cast<int>(get_sgAsmOperandModifier(sm[1].str()));
//        modifiers &= get_sgAsmOperandModifier(sm[1].str());

      if(!sm[2].str().empty())
        modifierVal += static_cast<int>(get_sgAsmOperandModifier(sm[2].str()));
     
      sageAsmOp->set_modifiers(static_cast<SgAsmOp::asm_operand_modifier_enum>(modifierVal));

      // set constraint
      sageAsmOp->set_constraint(get_sgAsmOperandConstraint(sm[3].str()));
      sageAsmOp->set_constraintString(sm[3]);


      Sg_File_Info * start_fi = Sg_File_Info::generateDefaultFileInfoForCompilerGeneratedNode();
      start_fi->setCompilerGenerated();
      sageAsmOp->set_startOfConstruct(start_fi);

      Sg_File_Info * end_fi   = Sg_File_Info::generateDefaultFileInfoForCompilerGeneratedNode();
      end_fi->setCompilerGenerated();
      sageAsmOp->set_endOfConstruct(end_fi);
      
      asmStmt->get_operands().push_back(sageAsmOp);
      sageAsmOp->set_parent(asmStmt);
    }
    *node = asmStmt;

    return VisitStmt(gcc_asm_stmt, node) && res;
}

bool ClangToSageTranslator::VisitMSAsmStmt(clang::MSAsmStmt * ms_asm_stmt, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitMSAsmStmt" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO
    return VisitStmt(ms_asm_stmt, node) && res;
}

bool ClangToSageTranslator::VisitBreakStmt(clang::BreakStmt * break_stmt, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitBreakStmt" << "\n";
#endif

    *node = SageBuilder::buildBreakStmt();
    return VisitStmt(break_stmt, node);
}

bool ClangToSageTranslator::VisitCapturedStmt(clang::CapturedStmt * captured_stmt, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitCapturedStmt" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO
    return VisitStmt(captured_stmt, node) && res;
}

bool ClangToSageTranslator::VisitCompoundStmt(clang::CompoundStmt * compound_stmt, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitCompoundStmt" << "\n";
#endif

    bool res = true;

    SgBasicBlock * block = SageBuilder::buildBasicBlock();

    block->set_parent(SageBuilder::topScopeStack());

    SageBuilder::pushScopeStack(block);

    clang::CompoundStmt::body_iterator it;
    for (it = compound_stmt->body_begin(); it != compound_stmt->body_end(); it++) {
        SgNode * tmp_node = Traverse(*it);

#if DEBUG_VISIT_STMT
        if (tmp_node != NULL)
          logger[DEBUG] << "In VisitCompoundStmt : child is " << tmp_node->class_name() << "\n";
        else
          logger[DEBUG] << "In VisitCompoundStmt : child is NULL" << "\n";
#endif

        SgClassDeclaration * class_decl = isSgClassDeclaration(tmp_node);
        if (class_decl != NULL && (class_decl->get_name() == "" || class_decl->get_isUnNamed())) continue;
        SgEnumDeclaration * enum_decl = isSgEnumDeclaration(tmp_node);
        if (enum_decl != NULL && (enum_decl->get_name() == "" || enum_decl->get_isUnNamed())) continue;
#if DEBUG_VISIT_STMT
        else if (enum_decl != NULL)
          logger[DEBUG] << "enum_decl = " << enum_decl << " >> name: " << enum_decl->get_name() << "\n";
#endif

        SgStatement * stmt  = isSgStatement(tmp_node);
        SgExpression * expr = isSgExpression(tmp_node);
        if (tmp_node != NULL && stmt == NULL && expr == NULL) {
            logger[WARN] << "Runtime error: tmp_node != NULL && stmt == NULL && expr == NULL" << "\n";
            res = false;
        }
        else if (stmt != NULL) {
            block->append_statement(stmt);
        }
        else if (expr != NULL) {
            SgExprStatement * expr_stmt = SageBuilder::buildExprStatement(expr);
            block->append_statement(expr_stmt);
        }
    }

    SageBuilder::popScopeStack();

    *node = block;

    return VisitStmt(compound_stmt, node) && res;
}

bool ClangToSageTranslator::VisitContinueStmt(clang::ContinueStmt * continue_stmt, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitContinueStmt" << "\n";
#endif

    *node = SageBuilder::buildContinueStmt();
    return VisitStmt(continue_stmt, node);
}

bool ClangToSageTranslator::VisitCoreturnStmt(clang::CoreturnStmt * core_turn_stmt, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitCoreturnStmt" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO
    return VisitStmt(core_turn_stmt, node) && res;
}

bool ClangToSageTranslator::VisitCoroutineBodyStmt(clang::CoroutineBodyStmt * coroutine_body_stmt, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitCoroutineBodyStmt" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO
    return VisitStmt(coroutine_body_stmt, node) && res;
}

bool ClangToSageTranslator::VisitCXXCatchStmt(clang::CXXCatchStmt * cxx_catch_stmt, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitCXXCatchStmt" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO
    return VisitStmt(cxx_catch_stmt, node) && res;
}

bool ClangToSageTranslator::VisitCXXForRangeStmt(clang::CXXForRangeStmt * cxx_for_range_stmt, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitCXXForRangeStmt" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO
    return VisitStmt(cxx_for_range_stmt, node) && res;
}

bool ClangToSageTranslator::VisitCXXTryStmt(clang::CXXTryStmt * cxx_try_stmt, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitCXXTryStmt" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO
    return VisitStmt(cxx_try_stmt, node) && res;
}

bool ClangToSageTranslator::VisitDeclStmt(clang::DeclStmt * decl_stmt, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitDeclStmt" << "\n";
#endif

    bool res = true;

    if (decl_stmt->isSingleDecl()) {
        *node = Traverse(decl_stmt->getSingleDecl());
#if DEBUG_VISIT_STMT
        logger[DEBUG] << "In VisitDeclStmt(): *node = " << *node << " = " << (*node)->class_name().c_str() << "\n";
#endif
    }
    else {
        std::vector<SgNode *> tmp_decls;
        //SgDeclarationStatement * decl;
        clang::DeclStmt::decl_iterator it;

        SgScopeStatement * scope = SageBuilder::topScopeStack();

        for (it = decl_stmt->decl_begin(); it != decl_stmt->decl_end()-1; it++) {
            clang::Decl* decl = (*it);
            if (decl == nullptr) continue;
            SgNode * child = Traverse(decl);

            SgDeclarationStatement * sub_decl_stmt = isSgDeclarationStatement(child);
            if (sub_decl_stmt == NULL && child != NULL) {
                logger[WARN] << "Runtime error: the node produce for a clang::Decl is not a SgDeclarationStatement !" << "\n";
                logger[WARN] << "    class = " << child->class_name() << "\n";
                res = false;
                continue;
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

            }
            scope->append_statement(sub_decl_stmt);
            sub_decl_stmt->set_parent(scope);
        }
        // last declaration in scope
        it = decl_stmt->decl_end();
        --it;
        SgNode * lastDecl = Traverse((clang::Decl*)(*it));
        SgDeclarationStatement * last_decl_Stmt = isSgDeclarationStatement(lastDecl);
        if (lastDecl != NULL && last_decl_Stmt == NULL) {
            logger[WARN] << "Runtime error: lastDecl != NULL && last_decl_Stmt == NULL" << "\n";
            res = false;
        }
        *node = last_decl_Stmt;
    }

#if DEBUG_VISIT_STMT
    logger[DEBUG] << "In VisitDeclStmt(): identify where the parent is not set: *node = " << *node << " = " << (*node)->class_name().c_str() << "\n";
    logger[DEBUG] << " --- *node parent = " << (*node)->get_parent() << "\n";
#endif

    return res;
}

bool ClangToSageTranslator::VisitDoStmt(clang::DoStmt * do_stmt, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitDoStmt" << "\n";
#endif

    SgNode * tmp_cond = Traverse(do_stmt->getCond());
    SgExpression * cond = isSgExpression(tmp_cond);
    ROSE_ASSERT(cond != NULL);

    SgStatement * expr_stmt = SageBuilder::buildExprStatement(cond);

    ROSE_ASSERT(expr_stmt != NULL);

    SgDoWhileStmt * sg_do_stmt = SageBuilder::buildDoWhileStmt_nfi(expr_stmt, NULL);

    sg_do_stmt->set_parent(SageBuilder::topScopeStack());

    sg_do_stmt->set_condition(expr_stmt);

    cond->set_parent(expr_stmt);
    expr_stmt->set_parent(sg_do_stmt);

    SageBuilder::pushScopeStack(sg_do_stmt);

    SgNode * tmp_body = Traverse(do_stmt->getBody());
    SgStatement * body = isSgStatement(tmp_body);
    SgExpression * expr = isSgExpression(tmp_body);
    if (expr != NULL) {
        body =  SageBuilder::buildExprStatement(expr);
        applySourceRange(body, do_stmt->getBody()->getSourceRange());
    }
    ROSE_ASSERT(body != NULL);

    body->set_parent(sg_do_stmt);

    SageBuilder::popScopeStack();

    sg_do_stmt->set_body(body);

    *node = sg_do_stmt;

    return VisitStmt(do_stmt, node); 
}

bool ClangToSageTranslator::VisitForStmt(clang::ForStmt * for_stmt, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitForStmt" << "\n";
#endif

    bool res = true;

 // DQ (11/28/2020): We have to build the scope first, and then build the rest bottom up.
    SgForStatement* sg_for_stmt = new SgForStatement((SgStatement*)NULL,(SgExpression*)NULL,(SgStatement*)NULL);

#if DEBUG_VISIT_STMT
    logger[DEBUG] << "In VisitForStmt(): Setting the parent of the sg_for_stmt" << "\n";
#endif

 // DQ (11/28/2020): this is required for test2012_127.c.
    sg_for_stmt->set_parent(SageBuilder::topScopeStack());

 // DQ (11/28/2020): Adding asertion.
    ROSE_ASSERT(sg_for_stmt->get_parent() != NULL);

    SageBuilder::pushScopeStack(sg_for_stmt);

  // Initialization

    SgForInitStatement * for_init_stmt = NULL;

    {
        SgStatementPtrList for_init_stmt_list;
        SgNode * tmp_init = Traverse(for_stmt->getInit());
        SgStatement * init_stmt = isSgStatement(tmp_init);
        SgExpression * init_expr = isSgExpression(tmp_init);
        if (tmp_init != NULL && init_stmt == NULL && init_expr == NULL) {
            logger[WARN] << "Runtime error: tmp_init != NULL && init_stmt == NULL && init_expr == NULL (" << tmp_init->class_name() << ")" << "\n";
            res = false;
        }
        else if (init_expr != NULL) {
            init_stmt = SageBuilder::buildExprStatement(init_expr);
            applySourceRange(init_stmt, for_stmt->getInit()->getSourceRange());
        }
        if (init_stmt != NULL)
            for_init_stmt_list.push_back(init_stmt);

        if(for_init_stmt_list.size() == 0)
        {
          SgNullStatement* nullStmt = SageBuilder::buildNullStatement_nfi();
          setCompilerGeneratedFileInfo(nullStmt, true);
          for_init_stmt_list.push_back(nullStmt);
        }

        for_init_stmt = SageBuilder::buildForInitStatement_nfi(for_init_stmt_list);

#if DEBUG_VISIT_STMT
        logger[DEBUG] << "In VisitForStmt(): for_init_stmt = " << for_init_stmt << "\"n";
#endif

        if (for_stmt->getInit() != NULL)
            applySourceRange(for_init_stmt, for_stmt->getInit()->getSourceRange());
        else
            setCompilerGeneratedFileInfo(for_init_stmt, true);
    }

  // Condition

    SgStatement * cond_stmt = NULL;

    {
        SgNode * tmp_cond = Traverse(for_stmt->getCond());
        SgExpression * cond = isSgExpression(tmp_cond);
        if (tmp_cond != NULL && cond == NULL) {
            logger[WARN] << "Runtime error: tmp_cond != NULL && cond == NULL" << "\n";
            res = false;
        }
        if (cond != NULL) { 
            cond_stmt = SageBuilder::buildExprStatement(cond);
            applySourceRange(cond_stmt, for_stmt->getCond()->getSourceRange());
        }
        else {
            cond_stmt = SageBuilder::buildNullStatement_nfi();
            setCompilerGeneratedFileInfo(cond_stmt, true);
        }
    }

  // Increment

    SgExpression * inc = NULL;

    {
        SgNode * tmp_inc  = Traverse(for_stmt->getInc());
        inc = isSgExpression(tmp_inc);
        if (tmp_inc != NULL && inc == NULL) {
            logger[WARN] << "Runtime error: tmp_inc != NULL && inc == NULL" << "\n";
            res = false;
        }
        if (inc == NULL) {
            inc = SageBuilder::buildNullExpression_nfi();
            setCompilerGeneratedFileInfo(inc, true);
        }
    }

  // Body

    SgStatement * body = NULL;

    {
        SgNode * tmp_body = Traverse(for_stmt->getBody());
        body = isSgStatement(tmp_body);
        if (body == NULL) {
            SgExpression * body_expr = isSgExpression(tmp_body);
            if (body_expr != NULL) {
                body = SageBuilder::buildExprStatement(body_expr);
                applySourceRange(body, for_stmt->getBody()->getSourceRange());
            }
        }
        if (tmp_body != NULL && body == NULL) {
            logger[WARN] << "Runtime error: tmp_body != NULL && body == NULL" << "\n";
            res = false;
        }
        if (body == NULL) {
            body = SageBuilder::buildNullStatement_nfi();
            setCompilerGeneratedFileInfo(body);
        }
    }

    SageBuilder::popScopeStack();

  // Attach sub trees to the for statement

    for_init_stmt->set_parent(sg_for_stmt);
    if (sg_for_stmt->get_for_init_stmt() != NULL)
        SageInterface::deleteAST(sg_for_stmt->get_for_init_stmt());
    sg_for_stmt->set_for_init_stmt(for_init_stmt);

    if (cond_stmt != NULL) {
        cond_stmt->set_parent(sg_for_stmt);
        sg_for_stmt->set_test(cond_stmt);
    }

    if (inc != NULL) {
        inc->set_parent(sg_for_stmt);
        sg_for_stmt->set_increment(inc);
    }

    if (body != NULL) {
        body->set_parent(sg_for_stmt);
        sg_for_stmt->set_loop_body(body);
    }

 // DQ (11/28/2020): Now we want to use the scope that is already on the stack (instead of adding a new one).
    SageBuilder::buildForStatement_nfi(sg_for_stmt, for_init_stmt, cond_stmt, inc, body);

 // DQ (11/28/2020): Adding asertion.
    ROSE_ASSERT(sg_for_stmt->get_parent() != NULL);

    *node = sg_for_stmt;

    return VisitStmt(for_stmt, node) && res;
}

bool ClangToSageTranslator::VisitGotoStmt(clang::GotoStmt * goto_stmt, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitGotoStmt" << "\n";
#endif

    bool res = true;

    SgSymbol * tmp_sym = GetSymbolFromSymbolTable(goto_stmt->getLabel());
    SgLabelSymbol * sym = isSgLabelSymbol(tmp_sym);
    if (sym == NULL) {
        SgNode * tmp_label = Traverse(goto_stmt->getLabel()->getStmt());
        SgLabelStatement * label_stmt = isSgLabelStatement(tmp_label);
        if (label_stmt == NULL) {
            logger[WARN] << "Runtime error: Cannot find the symbol for the label: \"" << goto_stmt->getLabel()->getStmt()->getName() << "\"." << "\n";
            logger[WARN] << "Runtime Error: Cannot find the label: \"" << goto_stmt->getLabel()->getStmt()->getName() << "\"." << "\n";
            res = false;
        }
        else {
            *node = SageBuilder::buildGotoStatement(label_stmt);
        }
    }
    else {
        *node = SageBuilder::buildGotoStatement(sym->get_declaration());
    }

/*
    SgNode * tmp_label = Traverse(goto_stmt->getLabel()->getStmt());
    SgLabelStatement * label_stmt = isSgLabelStatement(tmp_label);
    if (label_stmt == NULL) {
        logger[WARN] << "Runtime Error: Cannot find the label: \"" << goto_stmt->getLabel()->getStmt()->getName() << "\"." << "\n";
        res = false;
    }
    else {
        *node = SageBuilder::buildGotoStatement(label_stmt);
    }
*/
    return VisitStmt(goto_stmt, node) && res;
}


bool ClangToSageTranslator::VisitIfStmt(clang::IfStmt * if_stmt, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitIfStmt" << "\n";
#endif

    bool res = true;

    // TODO if_stmt->getConditionVariable() appears when a variable is declared in the condition...

    *node = SageBuilder::buildIfStmt_nfi(NULL, NULL, NULL);

    // Pei-Hung (04/22/22) Needs to setup parent node before processing the operands.
    // Needed for test2013_55.c and other similar tests
    (*node)->set_parent(SageBuilder::topScopeStack());
    SageBuilder::pushScopeStack(isSgScopeStatement(*node));

    SgNode * tmp_cond = Traverse(if_stmt->getCond());
    SgExpression * cond_expr = isSgExpression(tmp_cond);
    SgStatement * cond_stmt = SageBuilder::buildExprStatement(cond_expr);
    applySourceRange(cond_stmt, if_stmt->getCond()->getSourceRange());

    SgNode * tmp_then = Traverse(if_stmt->getThen());
    SgStatement * then_stmt = isSgStatement(tmp_then);
    if (then_stmt == NULL) {
        SgExpression * then_expr = isSgExpression(tmp_then);
        ROSE_ASSERT(then_expr != NULL);
        then_stmt = SageBuilder::buildExprStatement(then_expr);
    }
    applySourceRange(then_stmt, if_stmt->getThen()->getSourceRange());

    SgNode * tmp_else = Traverse(if_stmt->getElse());
    SgStatement * else_stmt = isSgStatement(tmp_else);
    if (else_stmt == NULL) {
        SgExpression * else_expr = isSgExpression(tmp_else);
        if (else_expr != NULL)
            else_stmt = SageBuilder::buildExprStatement(else_expr);
    }
    if (else_stmt != NULL) applySourceRange(else_stmt, if_stmt->getElse()->getSourceRange());

    SageBuilder::popScopeStack();

    cond_stmt->set_parent(*node);
    isSgIfStmt(*node)->set_conditional(cond_stmt);

    then_stmt->set_parent(*node);
    isSgIfStmt(*node)->set_true_body(then_stmt);
    if (else_stmt != NULL) {
      else_stmt->set_parent(*node);
      isSgIfStmt(*node)->set_false_body(else_stmt);
    }

    return VisitStmt(if_stmt, node) && res;
}

bool ClangToSageTranslator::VisitIndirectGotoStmt(clang::IndirectGotoStmt * indirect_goto_stmt, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitIndirectGotoStmt" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

     return VisitStmt(indirect_goto_stmt, node) && res;
}

bool ClangToSageTranslator::VisitMSDependentExistsStmt(clang::MSDependentExistsStmt * ms_dependent_exists_stmt, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitMSDependentExistsStmt" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

     return VisitStmt(ms_dependent_exists_stmt, node) && res;
}

bool ClangToSageTranslator::VisitNullStmt(clang::NullStmt * null_stmt, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitNullStmt" << "\n";
#endif
    bool res = true;

    *node = SageBuilder::buildNullStatement();

    return VisitStmt(null_stmt, node) && res;
}

bool ClangToSageTranslator::VisitOMPExecutableDirective(clang::OMPExecutableDirective * omp_executable_directive, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitOMPExecutableDirective" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitStmt(omp_executable_directive, node) && res;
}

bool ClangToSageTranslator::VisitOMPAtomicDirective(clang::OMPAtomicDirective * omp_atomic_directive, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitOMPAtomicDirective" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPExecutableDirective(omp_atomic_directive, node) && res;
}

bool ClangToSageTranslator::VisitOMPBarrierDirective(clang::OMPBarrierDirective * omp_barrier_directive, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitOMPBarrierDirective" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPExecutableDirective(omp_barrier_directive, node) && res;
}

bool ClangToSageTranslator::VisitOMPCancelDirective(clang::OMPCancelDirective * omp_cancel_directive, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitOMPCancelDirective" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPExecutableDirective(omp_cancel_directive, node) && res;
}

bool ClangToSageTranslator::VisitOMPCancellationPointDirective(clang::OMPCancellationPointDirective * omp_cancellation_point_directive, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitOMPCancellationPointDirective" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPExecutableDirective(omp_cancellation_point_directive, node) && res;
}

bool ClangToSageTranslator::VisitOMPCriticalDirective(clang::OMPCriticalDirective * omp_critical_directive, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitOMPCriticalDirective" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPExecutableDirective(omp_critical_directive, node) && res;
}

bool ClangToSageTranslator::VisitOMPFlushDirective(clang::OMPFlushDirective * omp_flush_directive, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitOMPFlushDirective" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPExecutableDirective(omp_flush_directive, node) && res;
}

bool ClangToSageTranslator::VisitOMPLoopDirective(clang::OMPLoopDirective * omp_loop_directive, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitOMPLoopDirective" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPExecutableDirective(omp_loop_directive, node) && res;
}

bool ClangToSageTranslator::VisitOMPDistributeDirective(clang::OMPDistributeDirective * omp_distribute_directive, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitOMPDistributeDirective" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPLoopDirective(omp_distribute_directive, node) && res;
}

bool ClangToSageTranslator::VisitOMPDistributeParallelForDirective(clang::OMPDistributeParallelForDirective * omp_distribute_parallel_for_directive, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitOMPDistributeParallelForDirective" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPExecutableDirective(omp_distribute_parallel_for_directive, node) && res;
}

bool ClangToSageTranslator::VisitOMPDistributeParallelForSimdDirective(clang::OMPDistributeParallelForSimdDirective * omp_distribute_parallel_for_simd_directive, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitOMPDistributeParallelForSimdDirective" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPExecutableDirective(omp_distribute_parallel_for_simd_directive, node) && res;
}

bool ClangToSageTranslator::VisitOMPDistributeSimdDirective(clang::OMPDistributeSimdDirective * omp_distribute__simd_directive, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitOMPDistributeSimdDirective" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPExecutableDirective(omp_distribute__simd_directive, node) && res;
}

bool ClangToSageTranslator::VisitOMPForDirective(clang::OMPForDirective * omp_for_directive, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitOMPForDirective" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPLoopDirective(omp_for_directive, node) && res;
}

bool ClangToSageTranslator::VisitOMPForSimdDirective(clang::OMPForSimdDirective * omp_for_simd_directive, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitOMPForSimdDirective" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPLoopDirective(omp_for_simd_directive, node) && res;
}

bool ClangToSageTranslator::VisitOMPParallelForDirective(clang::OMPParallelForDirective * omp_parallel_for_directive, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitOMPParallelForDirective" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPLoopDirective(omp_parallel_for_directive, node) && res;
}

bool ClangToSageTranslator::VisitOMPParallelForSimdDirective(clang::OMPParallelForSimdDirective * omp_parallel_for_simd_directive, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitOMPParallelForSimdDirective" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPLoopDirective(omp_parallel_for_simd_directive, node) && res;
}

bool ClangToSageTranslator::VisitOMPSimdDirective(clang::OMPSimdDirective * omp_simd_directive, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitOMPSimdDirective" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPLoopDirective(omp_simd_directive, node) && res;
}

bool ClangToSageTranslator::VisitOMPTargetParallelForDirective(clang::OMPTargetParallelForDirective * omp_target_parallel_for_directive, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitOMPTargetParallelForDirective" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPLoopDirective(omp_target_parallel_for_directive, node) && res;
}

bool ClangToSageTranslator::VisitOMPTargetParallelForSimdDirective(clang::OMPTargetParallelForSimdDirective * omp_target_parallel_for_simd_directive, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitOMPTargetParallelForSimdDirective" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPLoopDirective(omp_target_parallel_for_simd_directive, node) && res;
}

bool ClangToSageTranslator::VisitOMPTargetSimdDirective(clang::OMPTargetSimdDirective * omp_target_simd_directive, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitOMPTargetSimdDirective" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPLoopDirective(omp_target_simd_directive, node) && res;
}

bool ClangToSageTranslator::VisitOMPTargetTeamsDistributeDirective(clang::OMPTargetTeamsDistributeDirective * omp_target_teams_distribute_directive, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitOMPTargetTeamsDistributeDirective" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPLoopDirective(omp_target_teams_distribute_directive, node) && res;
}

bool ClangToSageTranslator::VisitOMPTargetTeamsDistributeSimdDirective(clang::OMPTargetTeamsDistributeSimdDirective * omp_target_teams_distribute_simd_directive, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitOMPTargetTeamsDistributeSimdDirective" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPLoopDirective(omp_target_teams_distribute_simd_directive, node) && res;
}

bool ClangToSageTranslator::VisitOMPTaskLoopDirective(clang::OMPTaskLoopDirective * omp_task_loop_directive, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitOMPTaskLoopDirective" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPLoopDirective(omp_task_loop_directive, node) && res;
}

bool ClangToSageTranslator::VisitOMPTaskLoopSimdDirective(clang::OMPTaskLoopSimdDirective * omp_task_loop_simd_directive, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitOMPTaskLoopSimdDirective" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPLoopDirective(omp_task_loop_simd_directive, node) && res;
}

bool ClangToSageTranslator::VisitOMPMasterDirective(clang::OMPMasterDirective * omp_master_directive, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitOMPMasterDirective" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPExecutableDirective(omp_master_directive, node) && res;
}

bool ClangToSageTranslator::VisitOMPOrderedDirective(clang::OMPOrderedDirective * omp_ordered_directive, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitOMPOrderedDirective" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPExecutableDirective(omp_ordered_directive, node) && res;
}

bool ClangToSageTranslator::VisitOMPParallelDirective(clang::OMPParallelDirective * omp_parallel_directive, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitOMPParallelDirective" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPExecutableDirective(omp_parallel_directive, node) && res;
}

bool ClangToSageTranslator::VisitOMPParallelSectionsDirective(clang::OMPParallelSectionsDirective * omp_parallel_sections_directive, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitOMPParallelSectionsDirective" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPExecutableDirective(omp_parallel_sections_directive, node) && res;
}

bool ClangToSageTranslator::VisitReturnStmt(clang::ReturnStmt * return_stmt, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitReturnStmt" << "\n";
#endif

    bool res = true;

    SgNode * tmp_expr = Traverse(return_stmt->getRetValue());
    SgExpression * expr = isSgExpression(tmp_expr);
    if (tmp_expr != NULL && expr == NULL) {
        logger[WARN] << "Runtime error: tmp_expr != NULL && expr == NULL" << "\n";
        res = false;
    }
    *node = SageBuilder::buildReturnStmt(expr);

    return VisitStmt(return_stmt, node) && res;
}

bool ClangToSageTranslator::VisitSEHExceptStmt(clang::SEHExceptStmt * seh_except_stmt, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitSEHExceptStmt" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitStmt(seh_except_stmt, node) && res;
}

bool ClangToSageTranslator::VisitSEHFinallyStmt(clang::SEHFinallyStmt * seh_finally_stmt, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitSEHFinallyStmt" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitStmt(seh_finally_stmt, node) && res;
}

bool ClangToSageTranslator::VisitSEHLeaveStmt(clang::SEHLeaveStmt * seh_leave_stmt, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitSEHLeaveStmt" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitStmt(seh_leave_stmt, node) && res;
}

bool ClangToSageTranslator::VisitSEHTryStmt(clang::SEHTryStmt * seh_try_stmt, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitSEHTryStmt" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitStmt(seh_try_stmt, node) && res;
}

bool ClangToSageTranslator::VisitSwitchCase(clang::SwitchCase * switch_case, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitSwitchCase" << "\n";
#endif
    bool res = true;
    
    // TODO

    return VisitStmt(switch_case, node) && res;
}

bool ClangToSageTranslator::VisitCaseStmt(clang::CaseStmt * case_stmt, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitCaseStmt" << "\n";
#endif

    SgNode * tmp_stmt = Traverse(case_stmt->getSubStmt());
    SgStatement * stmt = isSgStatement(tmp_stmt);
    SgExpression * expr = isSgExpression(tmp_stmt);
    if (expr != NULL) {
        stmt = SageBuilder::buildExprStatement(expr);
        applySourceRange(stmt, case_stmt->getSubStmt()->getSourceRange());
    }
    ROSE_ASSERT(stmt != NULL);

    SgNode * tmp_lhs = Traverse(case_stmt->getLHS());
    SgExpression * lhs = isSgExpression(tmp_lhs);
    ROSE_ASSERT(lhs != NULL);

    SgExpression* rhs = NULL; 
    if(case_stmt->getRHS() != nullptr)
    {
      SgNode * tmp_rhs = Traverse(case_stmt->getRHS());
      rhs = isSgExpression(tmp_rhs);
      ROSE_ASSERT(rhs != NULL);
    }

    SgCaseOptionStmt* caseOptionStmt = SageBuilder::buildCaseOptionStmt_nfi(lhs, stmt);

    if(rhs != NULL)
    {
      caseOptionStmt->set_key_range_end(rhs);
    }

    *node = caseOptionStmt;
    return VisitSwitchCase(case_stmt, node);
}

bool ClangToSageTranslator::VisitDefaultStmt(clang::DefaultStmt * default_stmt, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitDefaultStmt" << "\n";
#endif

    SgNode * tmp_stmt = Traverse(default_stmt->getSubStmt());
    SgStatement * stmt = isSgStatement(tmp_stmt);
    SgExpression * expr = isSgExpression(tmp_stmt);
    if (expr != NULL) {
        stmt = SageBuilder::buildExprStatement(expr);
        applySourceRange(stmt, default_stmt->getSubStmt()->getSourceRange());
    }
    ROSE_ASSERT(stmt != NULL);

    *node = SageBuilder::buildDefaultOptionStmt_nfi(stmt);

    return VisitSwitchCase(default_stmt, node);
}

bool ClangToSageTranslator::VisitSwitchStmt(clang::SwitchStmt * switch_stmt, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitSwitchStmt" << "\n";
#endif

    SgNode * tmp_cond = Traverse(switch_stmt->getCond());
    SgExpression * cond = isSgExpression(tmp_cond);
    ROSE_ASSERT(cond != NULL);
    
    SgStatement * expr_stmt = SageBuilder::buildExprStatement(cond);
        applySourceRange(expr_stmt, switch_stmt->getCond()->getSourceRange());

    SgSwitchStatement * sg_switch_stmt = SageBuilder::buildSwitchStatement_nfi(expr_stmt, NULL);

    sg_switch_stmt->set_parent(SageBuilder::topScopeStack());

    cond->set_parent(expr_stmt);
    expr_stmt->set_parent(sg_switch_stmt);

    SageBuilder::pushScopeStack(sg_switch_stmt);

    SgNode * tmp_body = Traverse(switch_stmt->getBody());
    SgStatement * body = isSgStatement(tmp_body);
    ROSE_ASSERT(body != NULL);

    SageBuilder::popScopeStack();

    sg_switch_stmt->set_body(body);
    //Pei-Hung (07/29/2024) In the case of test2001_14.C, body can be the SgDefaultStmt and the parent
    // needs to be set properly.
    body->set_parent(sg_switch_stmt);

    *node = sg_switch_stmt;

    return VisitStmt(switch_stmt, node);
}

bool ClangToSageTranslator::VisitValueStmt(clang::ValueStmt * value_stmt, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitValueStmt" << "\n";
#endif
    bool res = true;

 // DQ (11/28/2020): In test2020_45.c: I think this is the enum field.
 // clang::Expr* expr = value_stmt->getExprStmt();
 // ROSE_ASSERT(expr != NULL);

 // DQ (11/28/2020): Note that value_stmt->getExprStmt() == value_stmt, but not sure why.

 // DQ (11/28/2020): This was previously commented out, and I think there is nothing to do here.
 // The actual implementation was done in VisitFullExp
 // ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitStmt(value_stmt, node) && res;
}

bool ClangToSageTranslator::VisitAttributedStmt(clang::AttributedStmt * attributed_stmt, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitAttributedStmt" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitValueStmt(attributed_stmt, node) && res;
}

bool ClangToSageTranslator::VisitExpr(clang::Expr * expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitExpr" << "\n";
#endif

     // TODO Is there anything to be done? (maybe in relation with typing?)

     return VisitValueStmt(expr, node);
}

bool ClangToSageTranslator::VisitAbstractConditionalOperator(clang::AbstractConditionalOperator * abstract_conditional_operator, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitAbstractConditionalOperator" << "\n";
#endif
     bool res = true;

     // TODO 

     return VisitStmt(abstract_conditional_operator, node) && res;
}

bool ClangToSageTranslator::VisitBinaryConditionalOperator(clang::BinaryConditionalOperator * binary_conditional_operator, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitBinaryConditionalOperator" << "\n";
#endif
     bool res = true;

     SgNode * tmp_cond  = Traverse(binary_conditional_operator->getCond());
     SgExpression * cond_expr = isSgExpression(tmp_cond);
     ROSE_ASSERT(cond_expr);
     SgNode * tmp_true  = Traverse(binary_conditional_operator->getTrueExpr());
     SgExpression * true_expr = isSgExpression(tmp_true);
     ROSE_ASSERT(true_expr);
     SgNode * tmp_false = Traverse(binary_conditional_operator->getFalseExpr());
     SgExpression * false_expr = isSgExpression(tmp_false);
     ROSE_ASSERT(false_expr);
  
     *node = SageBuilder::buildConditionalExp(cond_expr, true_expr, false_expr);

     return VisitAbstractConditionalOperator(binary_conditional_operator, node) && res;
}

bool ClangToSageTranslator::VisitConditionalOperator(clang::ConditionalOperator * conditional_operator, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitConditionalOperator" << "\n";
#endif

    bool res = true;

    SgNode * tmp_cond  = Traverse(conditional_operator->getCond());
    SgExpression * cond_expr = isSgExpression(tmp_cond);
    ROSE_ASSERT(cond_expr);
    SgNode * tmp_true  = Traverse(conditional_operator->getTrueExpr());
    SgExpression * true_expr = isSgExpression(tmp_true);
    ROSE_ASSERT(true_expr);
    SgNode * tmp_false = Traverse(conditional_operator->getFalseExpr());
    SgExpression * false_expr = isSgExpression(tmp_false);
    ROSE_ASSERT(false_expr);

    *node = SageBuilder::buildConditionalExp(cond_expr, true_expr, false_expr);

    return VisitAbstractConditionalOperator(conditional_operator, node) && res;
}

bool ClangToSageTranslator::VisitAddrLabelExpr(clang::AddrLabelExpr * addr_label_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitAddrLabelExpr" << "\n";
#endif
     bool res = true;

     // TODO 

     return VisitExpr(addr_label_expr, node) && res;
}

bool ClangToSageTranslator::VisitArrayInitIndexExpr(clang::ArrayInitIndexExpr * array_init_index_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitArrayInitIndexExpr" << "\n";
#endif
     bool res = true;

     // TODO 

     return VisitExpr(array_init_index_expr, node) && res;
}

bool ClangToSageTranslator::VisitArrayInitLoopExpr(clang::ArrayInitLoopExpr * array_init_loop_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitArrayInitLoopExpr" << "\n";
#endif
     bool res = true;

     // TODO 

     return VisitExpr(array_init_loop_expr, node) && res;
}

bool ClangToSageTranslator::VisitArraySubscriptExpr(clang::ArraySubscriptExpr * array_subscript_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitArraySubscriptExpr" << "\n";
#endif

    bool res = true;

    SgNode * tmp_base = Traverse(array_subscript_expr->getBase());
    SgExpression * base = isSgExpression(tmp_base);
    if (tmp_base != NULL && base == NULL) {
        logger[WARN] << "Runtime error: tmp_base != NULL && base == NULL" << "\n";
        res = false;
    }

    SgNode * tmp_idx = Traverse(array_subscript_expr->getIdx());
    SgExpression * idx = isSgExpression(tmp_idx);
    if (tmp_idx != NULL && idx == NULL) {
        logger[WARN] << "Runtime error: tmp_idx != NULL && idx == NULL" << "\n";
        res = false;
    }

    *node = SageBuilder::buildPntrArrRefExp(base, idx);

    return VisitExpr(array_subscript_expr, node) && res;
}

bool ClangToSageTranslator::VisitArrayTypeTraitExpr(clang::ArrayTypeTraitExpr * array_type_trait_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitArrayTypeTraitExpr" << "\n";
#endif
     bool res = true;

     // TODO 

     return VisitExpr(array_type_trait_expr, node) && res;
}

bool ClangToSageTranslator::VisitAsTypeExpr(clang::AsTypeExpr * as_type_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitAsTypeExpr" << "\n";
#endif
     bool res = true;

     // TODO 

     return VisitExpr(as_type_expr, node) && res;
}

bool ClangToSageTranslator::VisitAtomicExpr(clang::AtomicExpr * atomic_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitAtomicExpr" << "\n";
#endif
     bool res = true;

     // TODO 

     return VisitExpr(atomic_expr, node) && res;
}

bool ClangToSageTranslator::VisitBinaryOperator(clang::BinaryOperator * binary_operator, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitBinaryOperator" << "\n";
#endif

    bool res = true;

    SgNode * tmp_lhs = Traverse(binary_operator->getLHS());
    SgExpression * lhs = isSgExpression(tmp_lhs);
    if (tmp_lhs != NULL && lhs == NULL) {
        logger[WARN] << "Runtime error: tmp_lhs != NULL && lhs == NULL" << "\n";
        res = false;
    }

    SgNode * tmp_rhs = Traverse(binary_operator->getRHS());
    SgExpression * rhs = isSgExpression(tmp_rhs);
    if (tmp_rhs != NULL && rhs == NULL) {
        logger[WARN] << "Runtime error: tmp_rhs != NULL && rhs == NULL" << "\n";
        res = false;
    }

    switch (binary_operator->getOpcode()) {
        case clang::BO_PtrMemD:   ROSE_ASSERT(!"clang::BO_PtrMemD:");//*node = SageBuilder::build(lhs, rhs); break;
        case clang::BO_PtrMemI:   ROSE_ASSERT(!"clang::BO_PtrMemI:");//*node = SageBuilder::build(lhs, rhs); break;
        case clang::BO_Mul:       *node = SageBuilder::buildMultiplyOp(lhs, rhs); break;
        case clang::BO_Div:       *node = SageBuilder::buildDivideOp(lhs, rhs); break;
        case clang::BO_Rem:       *node = SageBuilder::buildModOp(lhs, rhs); break;
        case clang::BO_Add:       *node = SageBuilder::buildAddOp(lhs, rhs); break;
        case clang::BO_Sub:       *node = SageBuilder::buildSubtractOp(lhs, rhs); break;
        case clang::BO_Shl:       *node = SageBuilder::buildLshiftOp(lhs, rhs); break;
        case clang::BO_Shr:       *node = SageBuilder::buildRshiftOp(lhs, rhs); break;
        case clang::BO_LT:        *node = SageBuilder::buildLessThanOp(lhs, rhs); break;
        case clang::BO_GT:        *node = SageBuilder::buildGreaterThanOp(lhs, rhs); break;
        case clang::BO_LE:        *node = SageBuilder::buildLessOrEqualOp(lhs, rhs); break;
        case clang::BO_GE:        *node = SageBuilder::buildGreaterOrEqualOp(lhs, rhs); break;
        case clang::BO_EQ:        *node = SageBuilder::buildEqualityOp(lhs, rhs); break;
        case clang::BO_NE:        *node = SageBuilder::buildNotEqualOp(lhs, rhs); break;
        case clang::BO_And:       *node = SageBuilder::buildBitAndOp(lhs, rhs); break;
        case clang::BO_Xor:       *node = SageBuilder::buildBitXorOp(lhs, rhs); break;
        case clang::BO_Or:        *node = SageBuilder::buildBitOrOp(lhs, rhs); break;
        case clang::BO_LAnd:      *node = SageBuilder::buildAndOp(lhs, rhs); break;
        case clang::BO_LOr:       *node = SageBuilder::buildOrOp(lhs, rhs); break;
        case clang::BO_Assign:    *node = SageBuilder::buildAssignOp(lhs, rhs); break;
        case clang::BO_MulAssign: *node = SageBuilder::buildMultAssignOp(lhs, rhs); break;
        case clang::BO_DivAssign: *node = SageBuilder::buildDivAssignOp(lhs, rhs); break;
        case clang::BO_RemAssign: *node = SageBuilder::buildModAssignOp(lhs, rhs); break;
        case clang::BO_AddAssign: *node = SageBuilder::buildPlusAssignOp(lhs, rhs); break;
        case clang::BO_SubAssign: *node = SageBuilder::buildMinusAssignOp(lhs, rhs); break;
        case clang::BO_ShlAssign: *node = SageBuilder::buildLshiftAssignOp(lhs, rhs); break;
        case clang::BO_ShrAssign: *node = SageBuilder::buildRshiftAssignOp(lhs, rhs); break;
        case clang::BO_AndAssign: *node = SageBuilder::buildAndAssignOp(lhs, rhs); break;
        case clang::BO_XorAssign: *node = SageBuilder::buildXorAssignOp(lhs, rhs); break;
        case clang::BO_OrAssign:  *node = SageBuilder::buildIorAssignOp(lhs, rhs); break;
        case clang::BO_Comma:     *node = SageBuilder::buildCommaOpExp(lhs, rhs); break;
        default:
            logger[WARN] << "Unknown opcode for binary operator: " << binary_operator->getOpcodeStr().str() << "\n";
            res = false;
    }

    return VisitExpr(binary_operator, node) && res;
}

bool ClangToSageTranslator::VisitCompoundAssignOperator(clang::CompoundAssignOperator * compound_assign_operator, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitCompoundAssignOperator" << "\n";
#endif
     bool res = true;

     // TODO 

     return VisitBinaryOperator(compound_assign_operator, node) && res;
}

bool ClangToSageTranslator::VisitBlockExpr(clang::BlockExpr * block_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitBlockExpr" << "\n";
#endif
     bool res = true;

     // TODO 

     return VisitExpr(block_expr, node) && res;
}

bool ClangToSageTranslator::VisitCallExpr(clang::CallExpr * call_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitCallExpr" << "\n";
#endif

    bool res = true;

    SgNode * tmp_expr = Traverse(call_expr->getCallee());
    SgExpression * expr = isSgExpression(tmp_expr);
    if (tmp_expr != NULL && expr == NULL) {
        logger[WARN] << "Runtime error: tmp_expr != NULL && expr == NULLL" << "\n";
        res = false;
    }

    SgExprListExp * param_list = SageBuilder::buildExprListExp_nfi();
        applySourceRange(param_list, call_expr->getSourceRange());

    clang::CallExpr::arg_iterator it;
    for (it = call_expr->arg_begin(); it != call_expr->arg_end(); ++it) {
        SgNode * tmp_expr = Traverse(*it);
        SgExpression * expr = isSgExpression(tmp_expr);
        if (tmp_expr != NULL && expr == NULL) {
            logger[WARN] << "Runtime error: tmp_expr != NULL && expr == NULL" << "\n";
            res = false;
            continue;
        }
        param_list->append_expression(expr);
    }

    *node = SageBuilder::buildFunctionCallExp_nfi(expr, param_list);

    return VisitExpr(call_expr, node) && res;
}

bool ClangToSageTranslator::VisitCUDAKernelCallExpr(clang::CUDAKernelCallExpr * cuda_kernel_call_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitCUDAKernelCallExpr" << "\n";
#endif
     bool res = true;

     // TODO 

     return VisitExpr(cuda_kernel_call_expr, node) && res;
}

bool ClangToSageTranslator::VisitCXXMemberCallExpr(clang::CXXMemberCallExpr * cxx_member_call_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitCXXMemberCallExpr" << "\n";
#endif
     bool res = true;

     clang::CXXMethodDecl* methodDecl = cxx_member_call_expr->getMethodDecl();
     clang::CXXRecordDecl* recordDecl = cxx_member_call_expr->getRecordDecl();
     // TODO 

     return VisitCallExpr(cxx_member_call_expr, node) && res;
}

bool ClangToSageTranslator::VisitCXXOperatorCallExpr(clang::CXXOperatorCallExpr * cxx_operator_call_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitCXXOperatorCallExpr" << "\n";
#endif
     bool res = true;

     // TODO 

     return VisitExpr(cxx_operator_call_expr, node) && res;
}

bool ClangToSageTranslator::VisitUserDefinedLiteral(clang::UserDefinedLiteral * user_defined_literal, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitUserDefinedLiteral" << "\n";
#endif
     bool res = true;

     // TODO 

     return VisitExpr(user_defined_literal, node) && res;
}

bool ClangToSageTranslator::VisitCastExpr(clang::CastExpr * cast_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitCastExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(cast_expr, node) && res;
}

bool ClangToSageTranslator::VisitExplicitCastExpr(clang::ExplicitCastExpr * explicit_cast_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitExplicitCastExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitCastExpr(explicit_cast_expr, node) && res;
}
    
bool ClangToSageTranslator::VisitBuiltinBitCastExpr(clang::BuiltinBitCastExpr * builtin_bit_cast_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitBuiltinBitCastExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExplicitCastExpr(builtin_bit_cast_expr, node) && res;
}
    
bool ClangToSageTranslator::VisitCStyleCastExpr(clang::CStyleCastExpr * c_style_cast, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitCStyleCastExpr" << "\n";
#endif

    bool res = true;

    SgNode * tmp_expr = Traverse(c_style_cast->getSubExpr());
    SgExpression * expr = isSgExpression(tmp_expr);

    ROSE_ASSERT(expr);

    SgType * type = buildTypeFromQualifiedType(c_style_cast->getTypeAsWritten());

    *node = SageBuilder::buildCastExp(expr, type, SgCastExp::e_C_style_cast);

    return VisitExplicitCastExpr(c_style_cast, node) && res;
}
    
bool ClangToSageTranslator::VisitCXXFunctionalCastExpr(clang::CXXFunctionalCastExpr * cxx_functional_cast_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitCXXFunctionalCastExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExplicitCastExpr(cxx_functional_cast_expr, node) && res;
}
    
bool ClangToSageTranslator::VisitCXXNamedCastExpr(clang::CXXNamedCastExpr * cxx_named_cast_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitCXXNamedCastExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExplicitCastExpr(cxx_named_cast_expr, node) && res;
}

bool ClangToSageTranslator::VisitCXXConstCastExpr(clang::CXXConstCastExpr * cxx_const_cast_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitCXXConstCastExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitCXXNamedCastExpr(cxx_const_cast_expr, node) && res;
}

bool ClangToSageTranslator::VisitCXXDynamicCastExpr(clang::CXXDynamicCastExpr * cxx_dynamic_cast_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitCXXDynamicCastExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitCXXNamedCastExpr(cxx_dynamic_cast_expr, node) && res;
}

bool ClangToSageTranslator::VisitCXXReinterpretCastExpr(clang::CXXReinterpretCastExpr * cxx_reinterpret_cast_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitCXXReinterpretCastExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitCXXNamedCastExpr(cxx_reinterpret_cast_expr, node) && res;
}

bool ClangToSageTranslator::VisitCXXStaticCastExpr(clang::CXXStaticCastExpr * cxx_static_cast_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitCXXStaticCastExpr" << "\n";
#endif
    SgNode * tmp_expr = Traverse(cxx_static_cast_expr->getSubExpr());
    SgExpression * expr = isSgExpression(tmp_expr);
    SgType * type = buildTypeFromQualifiedType(cxx_static_cast_expr->getType());
    SgCastExp * res = SageBuilder::buildCastExp(expr, type, SgCastExp::e_dynamic_cast);

    *node = res;

    return VisitCXXNamedCastExpr(cxx_static_cast_expr, node);
}


bool ClangToSageTranslator::VisitImplicitCastExpr(clang::ImplicitCastExpr * implicit_cast_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitImplicitCastExpr" << "\n";
#endif

    SgNode * tmp_expr = Traverse(implicit_cast_expr->getSubExpr());
    SgExpression * expr = isSgExpression(tmp_expr);
    
    ROSE_ASSERT(expr != NULL);
/*
    FIXME why not? I dont remember why i commented it... :)

    SgType * type = buildTypeFromQualifiedType(implicit_cast_expr->getType());
    SgCastExp * res = SageBuilder::buildCastExp(expr, type);
    setCompilerGeneratedFileInfo(res);

    *node = res;
*/

    *node = expr;

    return VisitCastExpr(implicit_cast_expr, node);
}

bool ClangToSageTranslator::VisitCharacterLiteral(clang::CharacterLiteral * character_literal, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitCharacterLiteral" << "\n";
#endif

    *node = SageBuilder::buildCharVal(character_literal->getValue());

    return VisitExpr(character_literal, node);
}

bool ClangToSageTranslator::VisitChooseExpr(clang::ChooseExpr * choose_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitChooseExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(choose_expr, node) && res;
}

bool ClangToSageTranslator::VisitCompoundLiteralExpr(clang::CompoundLiteralExpr * compound_literal, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitCompoundLiteralExpr" << "\n";
#endif

    SgNode * tmp_node = Traverse(compound_literal->getInitializer());
    SgExprListExp * expr = isSgExprListExp(tmp_node);
    ROSE_ASSERT(expr != NULL);

    SgType * type = buildTypeFromQualifiedType(compound_literal->getType());
    ROSE_ASSERT(type != NULL);

    SgAggregateInitializer * initializer = SageBuilder::buildAggregateInitializer_nfi(expr,type);

    initializer->set_uses_compound_literal(true);

    SgName name = std::string("compound_literal_") + Rose::StringUtility::numberToString(compound_literal);
    SgInitializedName* iname = SageBuilder::buildInitializedName_nfi(name, type, initializer);

    SgScopeStatement* scope = SageBuilder::topScopeStack();
    iname->set_scope(scope);
    iname->set_parent(scope);

    SgVariableSymbol * vsym = new SgVariableSymbol(iname);
    ROSE_ASSERT(vsym != nullptr);

    scope->insert_symbol(name,vsym);

    *node = SageBuilder::buildCompoundLiteralExp_nfi(vsym);

    return VisitExpr(compound_literal, node);
}

//bool ClangToSageTranslator::VisitConceptSpecializationExpr(clang::ConceptSpecializationExpr * concept_specialization_expr, SgNode ** node) {
//#if DEBUG_VISIT_STMT
//    logger[DEBUG] << "ClangToSageTranslator::VisitConceptSpecializationExpr" << "\n";
//#endif
//    bool res = true;
//
//    // TODO
//
//    return VisitExpr(concept_specialization_expr, node) && res;
//}

bool ClangToSageTranslator::VisitConvertVectorExpr(clang::ConvertVectorExpr * convert_vector_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitConvertVectorExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(convert_vector_expr, node) && res;
}

bool ClangToSageTranslator::VisitCoroutineSuspendExpr(clang::CoroutineSuspendExpr * coroutine_suspend_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitCoroutineSuspendExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(coroutine_suspend_expr, node) && res;
}

bool ClangToSageTranslator::VisitCoawaitExpr(clang::CoawaitExpr * coawait_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitCoawaitExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitCoroutineSuspendExpr(coawait_expr, node) && res;
}

bool ClangToSageTranslator::VisitCoyieldExpr(clang::CoyieldExpr * coyield_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitCoyieldExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitCoroutineSuspendExpr(coyield_expr, node) && res;
}

bool ClangToSageTranslator::VisitCXXBindTemporaryExpr(clang::CXXBindTemporaryExpr * cxx_bind_temporary_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitCXXBindTemporaryExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(cxx_bind_temporary_expr, node) && res;
}

bool ClangToSageTranslator::VisitCXXBoolLiteralExpr(clang::CXXBoolLiteralExpr * cxx_bool_literal_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitCXXBoolLiteralExpr" << "\n";
#endif
    bool res = true;

    *node = SageBuilder::buildBoolValExp(cxx_bool_literal_expr->getValue());
    applySourceRange(*node, cxx_bool_literal_expr->getSourceRange());

    return VisitExpr(cxx_bool_literal_expr, node) && res;
}

bool ClangToSageTranslator::VisitCXXConstructExpr(clang::CXXConstructExpr * cxx_construct_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitCXXConstructExpr" << "\n";
    // isElidable seems to be related to copy elision: https://en.cppreference.com/w/cpp/language/copy_elision
    logger[DEBUG] << "isElidable " << cxx_construct_expr->isElidable() << "\n";
    logger[DEBUG] << "hadMultipleCandidates " << cxx_construct_expr->hadMultipleCandidates() << "\n";
    logger[DEBUG] << "isListInitialization " << cxx_construct_expr->isListInitialization() << "\n";
    logger[DEBUG] << "isStdInitListInitialization " << cxx_construct_expr->isStdInitListInitialization() << "\n";
    logger[DEBUG] << "requiresZeroInitialization " << cxx_construct_expr->requiresZeroInitialization() << "\n";
#endif
    bool res = true;

    // 
    clang::CXXConstructorDecl* cxx_constructor_decl = cxx_construct_expr->getConstructor();
    SgMemberFunctionDeclaration* cxxConstructorDecl = isSgMemberFunctionDeclaration(Traverse(cxx_constructor_decl));

    SgExprListExp * param_list = SageBuilder::buildExprListExp_nfi();
    applySourceRange(param_list, cxx_construct_expr->getParenOrBraceRange());

    clang::CXXConstructExpr::arg_iterator it;
    for (it = cxx_construct_expr->arg_begin(); it != cxx_construct_expr->arg_end(); ++it) {
        SgNode * tmp_expr = Traverse(*it);
        SgExpression * expr = isSgExpression(tmp_expr);
        if (tmp_expr != NULL && expr == NULL) {
            logger[WARN] << "Runtime error: tmp_expr != NULL && expr == NULL" << "\n";
            res = false;
            continue;
        }
        param_list->append_expression(expr);
    }
    SgConstructorInitializer* constructorInitilizer = SageBuilder::buildConstructorInitializer(cxxConstructorDecl, param_list, cxxConstructorDecl->get_orig_return_type(), true, false, false, false );
    
    bool isCompilerGenerated = false;
    // isCompilerGenerated = !cxx_construct_expr->isElidable();
    // Pei-Hung (07/14/23) isElidable is not a proper approach to check and doesn't work for the following example
    // struct C2 { C2() : x(-1){};C2(int v) : x(v){};int x;};
    // C2 c2_3(2);
    //

    // Pei-Hung (07/22/24) enclosingClassDecl is only accessible from the first nondefiningDeclaration.
    SgMemberFunctionDeclaration* firstNonDefMemberFuncDecl = isSgMemberFunctionDeclaration(cxxConstructorDecl->get_firstNondefiningDeclaration());
    SgClassDefinition* enclosingClassDef = isSgClassDefinition(firstNonDefMemberFuncDecl->get_parent());
    ROSE_ASSERT(enclosingClassDef);
    SgClassDeclaration* enclosingClassDecl = isSgClassDeclaration(enclosingClassDef->get_declaration());
    ROSE_ASSERT(enclosingClassDecl);
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "clang::CXXConstructExpr: is default constructor: " << cxx_constructor_decl->isDefaultConstructor() << "\n";
    logger[DEBUG] << "clang::CXXConstructExpr: is from UnNamed class: " << enclosingClassDecl->get_isUnNamed() << "\n";
#endif
    if(cxx_constructor_decl->isDefaultConstructor() && enclosingClassDecl->get_isUnNamed())
      isCompilerGenerated = true; 

    if(isCompilerGenerated)
    {
      setCompilerGeneratedFileInfo(constructorInitilizer, false);
    }
    else
    {
      applySourceRange(constructorInitilizer, cxx_construct_expr->getSourceRange());
    }

    *node = constructorInitilizer;

    return VisitExpr(cxx_construct_expr, node); 
}

bool ClangToSageTranslator::VisitCXXTemporaryObjectExpr(clang::CXXTemporaryObjectExpr * cxx_temporary_object_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitCXXTemporaryObjectExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitCXXConstructExpr(cxx_temporary_object_expr, node) && res;
}

bool ClangToSageTranslator::VisitCXXDefaultArgExpr(clang::CXXDefaultArgExpr * cxx_default_arg_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitCXXDefaultArgExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(cxx_default_arg_expr, node) && res;
}

bool ClangToSageTranslator::VisitCXXDefaultInitExpr(clang::CXXDefaultInitExpr * cxx_default_init_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitCXXDefaultInitExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(cxx_default_init_expr, node) && res;
}

bool ClangToSageTranslator::VisitCXXDeleteExpr(clang::CXXDeleteExpr * cxx_delete_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitCXXDeleteExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(cxx_delete_expr, node) && res;
}

bool ClangToSageTranslator::VisitCXXDependentScopeMemberExpr(clang::CXXDependentScopeMemberExpr * cxx_dependent_scope_member_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitCXXDependentScopeMemberExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(cxx_dependent_scope_member_expr, node) && res;
}

bool ClangToSageTranslator::VisitCXXFoldExpr(clang::CXXFoldExpr * cxx_fold_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitCXXFoldExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(cxx_fold_expr, node) && res;
}

bool ClangToSageTranslator::VisitCXXInheritedCtorInitExpr(clang::CXXInheritedCtorInitExpr * cxx_inherited_ctor_init_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitCXXInheritedCtorInitExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(cxx_inherited_ctor_init_expr, node) && res;
}

bool ClangToSageTranslator::VisitCXXNewExpr(clang::CXXNewExpr * cxx_new_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitCXXNewExpr" << "\n";
#endif
    bool res = true;
    clang::QualType allocatedType = cxx_new_expr->getAllocatedType();
    SgType * sg_type = buildTypeFromQualifiedType(allocatedType);

    SgExprListExp* placementArgs = NULL;
    if(cxx_new_expr->getNumPlacementArgs() > 0)
    {
      placementArgs = SageBuilder::buildExprListExp();
      for (clang::Expr* placementArg : cxx_new_expr->placement_arguments()) {
        SgNode * tmpArg = Traverse(placementArg);
        SgExpression * expr = isSgExpression(tmpArg);
        placementArgs->append_expression(expr);
      }
    }

    
    SgNode* constructorInitilizer = Traverse(const_cast<clang::CXXConstructExpr*>(cxx_new_expr->getConstructExpr()));
    SgConstructorInitializer *constructor_args = isSgConstructorInitializer(constructorInitilizer);

    SgNode* ClangFuncDecl = Traverse(cxx_new_expr->getOperatorNew());
    if(constructor_args)
    {
      // (4/28/23 Pei-Hung) The type name is given through sg_type, SgConstructorInitializer doesn't seem to provide name for unparsing.
      constructor_args->set_need_name(false);
      ClangFuncDecl = Traverse(cxx_new_expr->getConstructExpr()->getConstructor());
      
    }
    SgFunctionDeclaration* sgFuncDecl = isSgFunctionDeclaration(ClangFuncDecl);

    if(cxx_new_expr->hasInitializer())
    {
      //TODO
    }

    SgExpression* builtin_args = NULL;
    short int need_global_specifier = (short int)cxx_new_expr->isGlobalNew();

    SgNewExp* newExp = SageBuilder::buildNewExp(sg_type, placementArgs, constructor_args, builtin_args, need_global_specifier , sgFuncDecl);
    *node = newExp;

    return VisitExpr(cxx_new_expr, node) && res;
}

bool ClangToSageTranslator::VisitCXXNoexceptExpr(clang::CXXNoexceptExpr * cxx_noexcept_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitCXXNoexceptExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(cxx_noexcept_expr, node) && res;
}

bool ClangToSageTranslator::VisitCXXNullPtrLiteralExpr(clang::CXXNullPtrLiteralExpr * cxx_null_ptr_literal_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitCXXNullPtrLiteralExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(cxx_null_ptr_literal_expr, node) && res;
}

bool ClangToSageTranslator::VisitCXXPseudoDestructorExpr(clang::CXXPseudoDestructorExpr * cxx_pseudo_destructor_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitCXXPseudoDestructorExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(cxx_pseudo_destructor_expr, node) && res;
}

//bool ClangToSageTranslator::VisitCXXRewrittenBinaryOperator(clang::CXXRewrittenBinaryOperator * cxx_rewrite_binary_operator, SgNode ** node) {
//#if DEBUG_VISIT_STMT
//    logger[DEBUG] << "ClangToSageTranslator::VisitCXXRewrittenBinaryOperator" << "\n";
//#endif
//    bool res = true;
//
//    // TODO
//
//    return VisitExpr(cxx_rewrite_binary_operator, node) && res;
//}

bool ClangToSageTranslator::VisitCXXScalarValueInitExpr(clang::CXXScalarValueInitExpr * cxx_scalar_value_init_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitCXXScalarValueInitExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(cxx_scalar_value_init_expr, node) && res;
}

bool ClangToSageTranslator::VisitCXXStdInitializerListExpr(clang::CXXStdInitializerListExpr * cxx_std_initializer_list_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitCXXStdInitializerListExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(cxx_std_initializer_list_expr, node) && res;
}

bool ClangToSageTranslator::VisitCXXThisExpr(clang::CXXThisExpr * cxx_this_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitCXXThisExpr" << "\n";
    logger[DEBUG] << "isImplicit: " << cxx_this_expr->isImplicit() << "\n";
#endif
    bool res = true;
    const clang::CXXRecordDecl* cxxRecordDecl = cxx_this_expr->getBestDynamicClassType();
    SgSymbol * symbol = GetSymbolFromSymbolTable(const_cast<clang::CXXRecordDecl*>(cxxRecordDecl));
    SgThisExp* thisExpr = SageBuilder::buildThisExp(symbol);
    // TODO
    *node = thisExpr;

    return VisitExpr(cxx_this_expr, node) && res;
}

bool ClangToSageTranslator::VisitCXXThrowExpr(clang::CXXThrowExpr * cxx_throw_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitCXXThrowExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(cxx_throw_expr, node) && res;
}

bool ClangToSageTranslator::VisitCXXTypeidExpr(clang::CXXTypeidExpr * cxx_typeid_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitCXXTypeidExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(cxx_typeid_expr, node) && res;
}

bool ClangToSageTranslator::VisitCXXUnresolvedConstructExpr(clang::CXXUnresolvedConstructExpr * cxx_unresolved_construct_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitCXXUnresolvedConstructExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(cxx_unresolved_construct_expr, node) && res;
}

bool ClangToSageTranslator::VisitCXXUuidofExpr(clang::CXXUuidofExpr * cxx_uuidof_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitCXXUuidofExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(cxx_uuidof_expr, node) && res;
}

bool ClangToSageTranslator::VisitDeclRefExpr(clang::DeclRefExpr * decl_ref_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitDeclRefExpr" << "\n";
    logger[DEBUG] << "hasQualifier " << decl_ref_expr->hasQualifier() << "\n";
#endif

    bool res = true;
    // SgSymbol for symbol representing the varDecl
    SgSymbol* sym = NULL;


    if(decl_ref_expr->hasQualifier())
    {
      // (10/30/2026) lin32: Adapt Clang 22's value qualifier while preserving namespace, alias, record and global lookup.
      RoseClangCompat::NestedNameSpecifier qualifier = decl_ref_expr->getQualifier();
#if CLANG_VERSION_MAJOR >= 22
      qualifier.dump();
#else
      qualifier->dump();
#endif
      clang::NamespaceDecl * namespaceDecl = RoseClangCompat::getAsNamespace(qualifier);
      clang::NamespaceAliasDecl * namespaceAliasDecl = RoseClangCompat::getAsNamespaceAlias(qualifier);
      clang::CXXRecordDecl  * cxxRecordDecl = RoseClangCompat::getAsRecordDecl(qualifier);
      if(namespaceDecl != nullptr)
      {
        SgNamespaceDeclarationStatement* namespaceeDeclStmt = isSgNamespaceDeclarationStatement(Traverse(namespaceDecl));
        ROSE_ASSERT(namespaceeDeclStmt);
        SgNamespaceDefinitionStatement* namespaceDefiniton = namespaceeDeclStmt->get_definition(); 
        // push the namesapce as top scope to look for the symbol
        SageBuilder::pushScopeStack(namespaceDefiniton);
        sym = GetSymbolFromSymbolTable(decl_ref_expr->getDecl());
        SageBuilder::popScopeStack();
      }
      else if(cxxRecordDecl != nullptr)
      {
        SgClassDeclaration* classDecl = isSgClassDeclaration(Traverse(cxxRecordDecl));
        ROSE_ASSERT(classDecl);
        SgClassDefinition* classDef = classDecl->get_definition(); 
        // push the namesapce as top scope to look for the symbol
        SageBuilder::pushScopeStack(classDef);
        sym = GetSymbolFromSymbolTable(decl_ref_expr->getDecl());
        SageBuilder::popScopeStack();
      }
      else if(namespaceAliasDecl != nullptr)
      {
        SgNamespaceAliasDeclarationStatement* namespaceAliasDeclStmt = isSgNamespaceAliasDeclarationStatement(Traverse(namespaceAliasDecl)); 
        SgNamespaceDeclarationStatement* namespaceeDeclStmt = namespaceAliasDeclStmt->get_namespaceDeclaration(); 
        ROSE_ASSERT(namespaceeDeclStmt);
        SgNamespaceDefinitionStatement* namespaceDefiniton = namespaceeDeclStmt->get_definition(); 
        // push the namesapce as top scope to look for the symbol
        SageBuilder::pushScopeStack(namespaceDefiniton);
        sym = GetSymbolFromSymbolTable(decl_ref_expr->getDecl());
        SageBuilder::popScopeStack();
      }
      else
      {
        // This should be the case when namespace is empty (e.g. ::X).
        if(RoseClangCompat::isGlobal(qualifier))
        {
          SgGlobal* globalScope = SageInterface::getGlobalScope(SageBuilder::topScopeStack());
          std::string declName = decl_ref_expr->getDecl()->getNameAsString();
          sym = globalScope->lookup_symbol (declName);
          ROSE_ASSERT(sym);
        }
        else
        {
          logger[ERROR] << "No valid qualifier found!\n";
          ROSE_ASSERT(0);
        }
      }
    }
    else

    // SgNode * tmp_node = Traverse(decl_ref_expr->getDecl());
    // DONE: Do not use Traverse(...) as the declaration can not be complete (recursive functions)
    //       Instead use SymbolTable from ROSE as the symbol should be ready (cannot have a reference before the declaration)
    // FIXME: This fix will not work for C++ (methods/fields can be use before they are declared...)
    // FIXME: I feel like it could work now, we will see ....

     sym = GetSymbolFromSymbolTable(decl_ref_expr->getDecl());

     if (sym == NULL) 
        {
          SgNode * tmp_decl = Traverse(decl_ref_expr->getDecl());

       // DQ (11/29/2020): Added assertion.
          ROSE_ASSERT(tmp_decl != NULL);

#if DEBUG_VISIT_STMT
          logger[DEBUG] << "temp_decl = " << tmp_decl << " = " << tmp_decl->class_name().c_str() << "\n";
#endif
          SgInitializedName* initializedName = isSgInitializedName(tmp_decl);
#if DEBUG_VISIT_STMT
          if (initializedName != NULL)
             {
               logger[DEBUG] << "Found SgInitializedName: initializedName->get_name() = " << initializedName->get_name().str() << "\n";
             }
#endif

          if (tmp_decl != NULL)
             {
               sym = GetSymbolFromSymbolTable(decl_ref_expr->getDecl());
             }

       // FIXME hack Traverse have added the symbol but we cannot find it (probably: problem with type and function lookup)

          if (sym == NULL && isSgFunctionDeclaration(tmp_decl) != NULL)
             {
               sym = new SgFunctionSymbol(isSgFunctionDeclaration(tmp_decl));
               sym->set_parent(tmp_decl);
             }
          // Pei-Hung (04/07/2022) sym can be NULL in the case for C99 VLA
          if (sym == NULL && isSgInitializedName(tmp_decl) != NULL)
             {
               sym = new SgVariableSymbol(isSgInitializedName(tmp_decl));
               sym->set_parent(tmp_decl);
               SageBuilder::topScopeStack()->insert_symbol(isSgInitializedName(tmp_decl)->get_name(), sym);        
             }
        }

     if (sym != NULL) 
        {
       // Not else if it was NULL we have try to traverse it....
          SgVariableSymbol  * var_sym  = isSgVariableSymbol(sym);
          SgFunctionSymbol  * func_sym = isSgFunctionSymbol(sym);
          SgEnumFieldSymbol * enum_sym = isSgEnumFieldSymbol(sym);

          if (var_sym != NULL) 
             {
               *node = SageBuilder::buildVarRefExp(var_sym);
             }
            else 
             {
               if (func_sym != NULL)
                  {
                    *node = SageBuilder::buildFunctionRefExp(func_sym);
                  }
                 else
                  {
                    if (enum_sym != NULL)
                       {
                         SgEnumDeclaration * enum_decl = isSgEnumDeclaration(enum_sym->get_declaration()->get_parent());
                         ROSE_ASSERT(enum_decl != NULL);
                         SgName name = enum_sym->get_name();
                         *node = SageBuilder::buildEnumVal_nfi(0, enum_decl, name);
                       }
                      else
                       {
                         if (sym != NULL)
                            {
                              logger[ERROR] << "Runtime error: Unknown type of symbol for a declaration reference." << "\n";
                              logger[ERROR] << "    sym->class_name() = " << sym->class_name()  << "\n";
                              ROSE_ABORT();
                            }
                       }
                  }
             }
             applySourceRange(*node, decl_ref_expr->getSourceRange());
        }
       else
        {
          logger[ERROR] << "Runtime error: Cannot find the symbol for a declaration reference (even after trying to buil th declaration)" << "\n";
          ROSE_ABORT();
        }

    return VisitExpr(decl_ref_expr, node) && res;
}

bool ClangToSageTranslator::VisitDependentCoawaitExpr(clang::DependentCoawaitExpr * dependent_coawait_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitDependentCoawaitExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(dependent_coawait_expr, node) && res;
}

bool ClangToSageTranslator::VisitDependentScopeDeclRefExpr(clang::DependentScopeDeclRefExpr * dependent_scope_decl_ref_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitDependentScopeDeclRefExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(dependent_scope_decl_ref_expr, node) && res;
}

// bool ClangToSageTranslator::VisitDependentScopeDeclRefExpr(clang::DependentScopeDeclRefExpr * dependent_scope_decl_ref_expr);

bool ClangToSageTranslator::VisitDesignatedInitExpr(clang::DesignatedInitExpr * designated_init_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitDesignatedInitExpr" << "\n";
#endif

    SgInitializer * base_init = NULL;    
    SgDesignatedInitializer * designated_init = NULL;    
    SgExprListExp * expr_list_exp = NULL; 
    {
        SgNode * tmp_expr = Traverse(designated_init_expr->getInit());
        SgExpression * expr = isSgExpression(tmp_expr);
        ROSE_ASSERT(expr != NULL);
        SgExprListExp * expr_list_exp = isSgExprListExp(expr);
        if (expr_list_exp != NULL) {
            // FIXME get the type right...
            base_init = SageBuilder::buildAggregateInitializer_nfi(expr_list_exp, NULL);
        }
        else {
            base_init = SageBuilder::buildAssignInitializer_nfi(expr, expr->get_type());
        }
        ROSE_ASSERT(base_init != NULL);
        applySourceRange(base_init, designated_init_expr->getInit()->getSourceRange());
    }


/* Pei-Hung (06/10/2022) revision to handle Initializer in test2013_37.c
 *  After calling getSyntacticForm from InitListExpr, the type and multidimensional array hierarchy is missing.
 *  This version can construct the array structure but need additional support to grab the type structure from 
 *  parent AST node, such as VarDecl.
 */

    auto designatorSize = designated_init_expr->size();

    for (auto it=designatorSize; it > 0; it--) {
        expr_list_exp = SageBuilder::buildExprListExp_nfi();
        SgExpression * expr = NULL;
        clang::DesignatedInitExpr::Designator * D = designated_init_expr->getDesignator(it-1);
        if (D->isFieldDesignator()) {
#if CLANG_VERSION_MAJOR >= 18
            SgSymbol * symbol = GetSymbolFromSymbolTable(D->getFieldDecl());
#else
            SgSymbol * symbol = GetSymbolFromSymbolTable(D->getField());
#endif
            SgVariableSymbol * var_sym = isSgVariableSymbol(symbol);
            ROSE_ASSERT(var_sym != NULL);
            expr = SageBuilder::buildVarRefExp_nfi(var_sym);
        }
        else if (D->isArrayDesignator()) {
            SgNode * tmp_expr = NULL;
            if(clang::ConstantExpr::classof(designated_init_expr->getArrayIndex(*D)))
            {
               clang::FullExpr* fullExpr = (clang::FullExpr*) designated_init_expr->getArrayIndex(*D);
               clang::IntegerLiteral* integerLiteral = (clang::IntegerLiteral*) fullExpr->getSubExpr();
               tmp_expr = SageBuilder::buildUnsignedLongVal((unsigned long) integerLiteral->getValue().getSExtValue());
            }
            else
            {
               tmp_expr = Traverse(designated_init_expr->getArrayIndex(*D));
            }
            expr = isSgExpression(tmp_expr);
            ROSE_ASSERT(expr != NULL);

        }
        else if (D->isArrayRangeDesignator()) {
            ROSE_ASSERT(!"I don't believe range designator initializer are supported by ROSE...");    
        }
        else ROSE_ABORT();

        ROSE_ASSERT(expr != NULL);

        applySourceRange(expr, D->getSourceRange());
        expr->set_parent(expr_list_exp);
        expr_list_exp->append_expression(expr);
        if(it > 1)
        {
            SgDesignatedInitializer * design_init = new SgDesignatedInitializer(expr_list_exp, base_init);
            applySourceRange(design_init, designated_init_expr->getDesignatorsSourceRange());
            expr_list_exp->set_parent(design_init);
            base_init->set_parent(design_init);
            SgExprListExp* aggListExp = SageBuilder::buildExprListExp_nfi();
            design_init->set_parent(aggListExp);
            aggListExp->append_expression(design_init);
            SgAggregateInitializer* newAggInit = SageBuilder::buildAggregateInitializer_nfi(aggListExp, NULL);
            expr_list_exp = SageBuilder::buildExprListExp_nfi(); 
            base_init = newAggInit; 
        }

    }

    applySourceRange(expr_list_exp, designated_init_expr->getDesignatorsSourceRange());
    designated_init = new SgDesignatedInitializer(expr_list_exp, base_init);
    expr_list_exp->set_parent(base_init);
    base_init->set_parent(designated_init);

    *node = designated_init;

    return VisitExpr(designated_init_expr, node);

// Pei-Hung (06/10/2022) keep the original implementation which has the array information stored in the list
/*
    for (auto it=0; it < designatorSize; it++) {
        SgExpression * expr = NULL;
        clang::DesignatedInitExpr::Designator * D = designated_init_expr->getDesignator(it);
        if (D->isFieldDesignator()) {
            SgSymbol * symbol = GetSymbolFromSymbolTable(D->getField());
            SgVariableSymbol * var_sym = isSgVariableSymbol(symbol);
            ROSE_ASSERT(var_sym != NULL);
            expr = SageBuilder::buildVarRefExp_nfi(var_sym);
            applySourceRange(expr, D->getSourceRange());
        }
        else if (D->isArrayDesignator()) {
            SgNode * tmp_expr = NULL;
            if(clang::ConstantExpr::classof(designated_init_expr->getArrayIndex(*D)))
            {
               clang::FullExpr* fullExpr = (clang::FullExpr*) designated_init_expr->getArrayIndex(*D);
               clang::IntegerLiteral* integerLiteral = (clang::IntegerLiteral*) fullExpr->getSubExpr();
               tmp_expr = SageBuilder::buildUnsignedLongVal((unsigned long) integerLiteral->getValue().getSExtValue());
logger[DEBUG] << "idx:" << integerLiteral->getValue().getSExtValue() << "\n";
            }
            else
            {
               tmp_expr = Traverse(designated_init_expr->getArrayIndex(*D));
            }
            expr = isSgExpression(tmp_expr);
            ROSE_ASSERT(expr != NULL);
        }
        else if (D->isArrayRangeDesignator()) {
            ROSE_ASSERT(!"I don't believe range designator initializer are supported by ROSE...");    
        }
        else ROSE_ABORT();

        ROSE_ASSERT(expr != NULL);

        expr->set_parent(expr_list_exp);
        expr_list_exp->append_expression(expr);
    }

    applySourceRange(expr_list_exp, designated_init_expr->getDesignatorsSourceRange());

    SgDesignatedInitializer * design_init = new SgDesignatedInitializer(expr_list_exp, init);
    expr_list_exp->set_parent(design_init);
    init->set_parent(design_init);

    *node = design_init;

    return VisitExpr(designated_init_expr, node);
*/
}

bool ClangToSageTranslator::VisitDesignatedInitUpdateExpr(clang::DesignatedInitUpdateExpr * designated_init_update, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitDesignatedInitUpdateExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(designated_init_update, node) && res;
}

bool ClangToSageTranslator::VisitExpressionTraitExpr(clang::ExpressionTraitExpr * expression_trait_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitExpressionTraitExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(expression_trait_expr, node) && res;
}

bool ClangToSageTranslator::VisitExtVectorElementExpr(clang::ExtVectorElementExpr * ext_vector_element_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitExtVectorElementExpr" << "\n";
#endif

    SgNode * tmp_base = Traverse(ext_vector_element_expr->getBase());
    SgExpression * base = isSgExpression(tmp_base);

    ROSE_ASSERT(base != NULL);

    SgType * type = buildTypeFromQualifiedType(ext_vector_element_expr->getType());

    clang::IdentifierInfo & ident_info = ext_vector_element_expr->getAccessor();
    std::string ident = ident_info.getName().str();

    SgScopeStatement * scope = SageBuilder::ScopeStack.front();
    SgGlobal * global = isSgGlobal(scope);
    ROSE_ASSERT(global != NULL);

  // Build Manually a SgVarRefExp to have the same Accessor (text version) TODO ExtVectorAccessor and ExtVectorType
    SgInitializedName * init_name = SageBuilder::buildInitializedName(ident, SageBuilder::buildVoidType(), NULL);
    setCompilerGeneratedFileInfo(init_name);
    init_name->set_scope(global);
    SgVariableSymbol * var_symbol = new SgVariableSymbol(init_name);
    SgVarRefExp * pseudo_field = new SgVarRefExp(var_symbol);
    setCompilerGeneratedFileInfo(pseudo_field, true);
    init_name->set_parent(pseudo_field);

    SgExpression * res = NULL;
    if (ext_vector_element_expr->isArrow())
        res = SageBuilder::buildArrowExp(base, pseudo_field);
    else
        res = SageBuilder::buildDotExp(base, pseudo_field);

    ROSE_ASSERT(res != NULL);

    *node = res;

   return VisitExpr(ext_vector_element_expr, node);
}

bool ClangToSageTranslator::VisitFixedPointLiteral(clang::FixedPointLiteral * fixed_point_literal, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitFixedPointLiteral" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(fixed_point_literal, node) && res;
}

bool ClangToSageTranslator::VisitFloatingLiteral(clang::FloatingLiteral * floating_literal, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitFloatingLiteral" << "\n";
#endif

    unsigned int precision =  llvm::APFloat::semanticsPrecision(floating_literal->getValue().getSemantics());
    llvm::APFloat llvmFloat = floating_literal->getValue();
    std::string str;
    llvm::raw_string_ostream os(str);
    llvmFloat.print(os);
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitFloatingLiteral precision = " << precision  << "\n";
    logger[DEBUG] << "ClangToSageTranslator::VisitFloatingLiteral value in string = " << os.str()  << "\n";
#endif

    if (precision == 24)
        *node = SageBuilder::buildFloatVal(floating_literal->getValue().convertToFloat());
    else if (precision == 53)
        *node = SageBuilder::buildDoubleVal(floating_literal->getValue().convertToDouble());
    else if (precision == 64) 
        *node = SageBuilder::buildLongDoubleVal(std::stold(os.str()));
    else
        ROSE_ASSERT(!"In VisitFloatingLiteral: Unsupported float size");
    applySourceRange(*node, floating_literal->getSourceRange());

    return VisitExpr(floating_literal, node);
}

bool ClangToSageTranslator::VisitFullExpr(clang::FullExpr * full_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitFullExpr" << "\n";
#endif
    bool res = true;

    SgNode * tmp_expr = Traverse(full_expr->getSubExpr());
    SgExpression * expr = isSgExpression(tmp_expr);

#if DEBUG_VISIT_STMT
    logger[DEBUG] <<  "In VisitFullExpr(): built: expr = " << expr << " = " << expr->class_name().c_str() << "\n";
#endif

    *node = expr;

    // TODO

    return VisitExpr(full_expr, node) && res;
}

bool ClangToSageTranslator::VisitConstantExpr(clang::ConstantExpr * constant_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitConstantExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitFullExpr(constant_expr, node) && res;
}

bool ClangToSageTranslator::VisitExprWithCleanups(clang::ExprWithCleanups * expr_with_cleanups, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitExprWithCleanups" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitFullExpr(expr_with_cleanups, node) && res;
}

bool ClangToSageTranslator::VisitFunctionParmPackExpr(clang::FunctionParmPackExpr * function_parm_pack_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitFunctionParmPackExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(function_parm_pack_expr, node) && res;
}

bool ClangToSageTranslator::VisitGenericSelectionExpr(clang::GenericSelectionExpr * generic_Selection_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitGenericSelectionExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(generic_Selection_expr, node) && res;
}

bool ClangToSageTranslator::VisitGNUNullExpr(clang::GNUNullExpr * gnu_null_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitGNUNullExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(gnu_null_expr, node) && res;
}

bool ClangToSageTranslator::VisitImaginaryLiteral(clang::ImaginaryLiteral * imaginary_literal, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitImaginaryLiteral" << "\n";
#endif

    SgNode * tmp_imag_val = Traverse(imaginary_literal->getSubExpr());
    SgValueExp * imag_val = isSgValueExp(tmp_imag_val);
    ROSE_ASSERT(imag_val != NULL);

    SgComplexVal * comp_val = new SgComplexVal(NULL, imag_val, imag_val->get_type(), "");

    *node = comp_val;

    return VisitExpr(imaginary_literal, node);
}

bool ClangToSageTranslator::VisitImplicitValueInitExpr(clang::ImplicitValueInitExpr * implicit_value_init_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitImplicitValueInitExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(implicit_value_init_expr, node) && res;
}

bool ClangToSageTranslator::VisitInitListExpr(clang::InitListExpr * init_list_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitInitListExpr" << "\n";
#endif

    // We use the syntactic version of the initializer if it exists
    if (init_list_expr->getSyntacticForm() != NULL) return VisitInitListExpr(init_list_expr->getSyntacticForm(), node);

    SgExprListExp * expr_list_expr = SageBuilder::buildExprListExp_nfi();

    clang::InitListExpr::iterator it;
    for (it = init_list_expr->begin(); it != init_list_expr->end(); it++) {
        SgNode * tmp_expr = Traverse(*it);
        SgExpression * expr = isSgExpression(tmp_expr);
        ROSE_ASSERT(expr != NULL);

        // Pei-Hung (05/13/2022) the expr can another InitListExpr
        SgExprListExp * child_expr_list_expr = isSgExprListExp(expr);
        SgInitializer * init = NULL;
        if (child_expr_list_expr != NULL)
        {
            SgType * type = expr->get_type();
            init = SageBuilder::buildAggregateInitializer(child_expr_list_expr, type);
        }

        if (init != NULL)
        {
            applySourceRange(init, (*it)->getSourceRange());
            expr_list_expr->append_expression(init);
        }
        else
            expr_list_expr->append_expression(expr);
    }

    *node = expr_list_expr;

    return VisitExpr(init_list_expr, node);
}

bool ClangToSageTranslator::VisitIntegerLiteral(clang::IntegerLiteral * integer_literal, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitIntegerLiteral" << "\n";
#endif

    *node = SageBuilder::buildIntVal(integer_literal->getValue().getSExtValue());
    applySourceRange(*node, integer_literal->getSourceRange());

    return VisitExpr(integer_literal, node);
}

bool ClangToSageTranslator::VisitLambdaExpr(clang::LambdaExpr * lambda_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitLambdaExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(lambda_expr, node) && res;
}

bool ClangToSageTranslator::VisitMaterializeTemporaryExpr(clang::MaterializeTemporaryExpr * materialize_temporary_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitMaterializeTemporaryExpr" << "\n";
#endif
    bool res = true;

    clang::Expr* subExpr = materialize_temporary_expr->getSubExpr();
    *node = Traverse(subExpr);
    // TODO

    return VisitExpr(materialize_temporary_expr, node) && res;
}

bool ClangToSageTranslator::VisitMemberExpr(clang::MemberExpr * member_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitMemberExpr" << "\n";
    logger[DEBUG] << "MemberExpr::hasQualifier() " << member_expr->hasQualifier() << "\n";
#endif

    bool res = true;

    SgNode * tmp_base = Traverse(member_expr->getBase());
    SgExpression * base = isSgExpression(tmp_base);
    ROSE_ASSERT(base != NULL);

    SgSymbol * sym = GetSymbolFromSymbolTable(member_expr->getMemberDecl());

    SgVariableSymbol * var_sym  = isSgVariableSymbol(sym);
    SgMemberFunctionSymbol * func_sym = isSgMemberFunctionSymbol(sym);
    SgClassSymbol * class_sym  = isSgClassSymbol(sym);

    SgExpression * sg_member_expr = NULL;

    bool successful_cast = var_sym || func_sym || class_sym;
    if (sym != NULL && !successful_cast) {
        logger[WARN] << "Runtime error: Unknown type of symbol for a member reference." << "\n";
        logger[WARN] << "    sym->class_name() = " << sym->class_name()  << "\n";
        res = false;
    }
    else if (var_sym != NULL) {
        sg_member_expr = SageBuilder::buildVarRefExp(var_sym);
    }
    else if (func_sym != NULL) { // C++
        sg_member_expr = SageBuilder::buildMemberFunctionRefExp_nfi(func_sym, false, false); // FIXME 2nd and 3rd params ?
    }
    else if (class_sym != NULL) { 
        SgClassDeclaration* classDecl = class_sym->get_declaration();
        SgClassDeclaration* classDefDecl = isSgClassDeclaration(classDecl->get_definition());
        SgType* classType = classDecl->get_type();
//        if(classDecl->get_isUnNamed())
        {
          SgName varName(generate_name_for_variable(member_expr));
          logger[DEBUG] << "build varName:" << varName << "\n";
          SgVariableDeclaration * var_decl = SageBuilder::buildVariableDeclaration(varName, classType, NULL,SageBuilder::topScopeStack());
          var_decl->set_baseTypeDefiningDeclaration(classDefDecl);
          var_decl->set_variableDeclarationContainsBaseTypeDefiningDeclaration(true);
          var_decl->set_parent(SageBuilder::topScopeStack());
          
          sg_member_expr = SageBuilder::buildVarRefExp(var_decl);
        }
    }

    ROSE_ASSERT(sg_member_expr != NULL);

    // TODO (C++) member_expr->getQualifier() : for 'a->Base::foo'

    if (member_expr->isArrow())
        *node = SageBuilder::buildArrowExp(base, sg_member_expr);
    else
        *node = SageBuilder::buildDotExp(base, sg_member_expr);

    return VisitExpr(member_expr, node) && res;
}

bool ClangToSageTranslator::VisitMSPropertyRefExpr(clang::MSPropertyRefExpr * ms_property_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitMSPropertyRefExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(ms_property_expr, node) && res;
}

bool ClangToSageTranslator::VisitMSPropertySubscriptExpr(clang::MSPropertySubscriptExpr * ms_property_subscript_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitMSPropertySubscriptExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(ms_property_subscript_expr, node) && res;
}

bool ClangToSageTranslator::VisitNoInitExpr(clang::NoInitExpr * no_init_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitNoInitExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(no_init_expr, node) && res;
}

bool ClangToSageTranslator::VisitOffsetOfExpr(clang::OffsetOfExpr * offset_of_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitOffsetOfExpr" << "\n";
#endif
    bool res = true;

    SgNodePtrList nodePtrList;

    SgType* type = buildTypeFromQualifiedType(offset_of_expr->getTypeSourceInfo()->getType());

    nodePtrList.push_back(type);

    SgExpression* topExp = nullptr;
 
    for (unsigned i = 0, n = offset_of_expr->getNumComponents(); i < n; ++i) {
        clang::OffsetOfNode ON = offset_of_expr->getComponent(i);
          
        switch(ON.getKind()) {
           case clang::OffsetOfNode::Array: {
               // Array node
               SgExpression* arrayIdx = isSgExpression(Traverse(offset_of_expr->getIndexExpr(ON.getArrayExprIndex())));
               SgPntrArrRefExp* pntrArrRefExp = SageBuilder::buildPntrArrRefExp(topExp,arrayIdx);
               topExp = isSgExpression(pntrArrRefExp);
               break;
           }
           case clang::OffsetOfNode::Field:{
               SgNode* fieldNode = Traverse(ON.getField());
               SgName fieldName(ON.getFieldName()->getName().str());
               SgVarRefExp* varExp = SageBuilder::buildVarRefExp(fieldName);
               if(topExp == nullptr)
               {
                 topExp = isSgExpression(varExp);
               }
               else
               {
                 SgDotExp* dotExp = SageBuilder::buildDotExp(topExp, varExp);
                 topExp = isSgExpression(dotExp);
               }
               break;
           }
           // TODO
           case clang::OffsetOfNode::Identifier:{
               SgName fieldName(ON.getFieldName()->getName().str());
               SgVarRefExp* varExp = SageBuilder::buildVarRefExp(fieldName);
               break;
           }
           // TODO
           case clang::OffsetOfNode::Base:
               break;
        }
    }
    nodePtrList.push_back(topExp);

    SgTypeTraitBuiltinOperator* typeTraitBuiltinOperator = SageBuilder::buildTypeTraitBuiltinOperator("__builtin_offsetof", nodePtrList);

    *node = typeTraitBuiltinOperator;

    return VisitExpr(offset_of_expr, node) && res;
}

bool ClangToSageTranslator::VisitOMPArraySectionExpr(RoseClangCompat::ArraySectionExpr * omp_array_section_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitOMPArraySectionExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(omp_array_section_expr, node) && res;
}

bool ClangToSageTranslator::VisitOpaqueValueExpr(clang::OpaqueValueExpr * opaque_value_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitOpaqueValueExpr" << "\n";
#endif
    bool res = true;

    SgNode * sourceExpr  = Traverse(opaque_value_expr->getSourceExpr());

    *node = sourceExpr;

    return VisitExpr(opaque_value_expr, node) && res;
}

bool ClangToSageTranslator::VisitOverloadExpr(clang::OverloadExpr * overload_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitOverloadExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(overload_expr, node) && res;
}

bool ClangToSageTranslator::VisitUnresolvedLookupExpr(clang::UnresolvedLookupExpr * unresolved_lookup_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitUnresolvedLookupExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitOverloadExpr(unresolved_lookup_expr, node) && res;
}

bool ClangToSageTranslator::VisitUnresolvedMemberExpr(clang::UnresolvedMemberExpr * unresolved_member_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitUnresolvedMemberExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitOverloadExpr(unresolved_member_expr, node) && res;
}

bool ClangToSageTranslator::VisitPackExpansionExpr(clang::PackExpansionExpr * pack_expansion_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitPackExpansionExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(pack_expansion_expr, node) && res;
}

bool ClangToSageTranslator::VisitParenExpr(clang::ParenExpr * paren_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitParenExpr" << "\n";
#endif

    bool res = true;

    SgNode * tmp_subexpr = Traverse(paren_expr->getSubExpr());
    SgExpression * subexpr = isSgExpression(tmp_subexpr);
    if (tmp_subexpr != NULL && subexpr == NULL) {
        logger[WARN] << "Runtime error: tmp_subexpr != NULL && subexpr == NULL" << "\n";
        res = false;
    }

    // bypass ParenExpr, their is nothing equivalent in SageIII
    *node = subexpr;

    return VisitExpr(paren_expr, node) && res;
}

bool ClangToSageTranslator::VisitParenListExpr(clang::ParenListExpr * paran_list_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitParenListExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(paran_list_expr, node) && res;
}

bool ClangToSageTranslator::VisitPredefinedExpr(clang::PredefinedExpr * predefined_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitPredefinedExpr" << "\n";
#endif

    // FIXME It's get tricky here: PredefinedExpr represent compiler generateed variables
    //    I choose to attach those variables on demand in the function definition scope 

  // Traverse the scope's stack to find the last function definition:

    SgFunctionDefinition * func_def = NULL;
    std::list<SgScopeStatement *>::reverse_iterator it = SageBuilder::ScopeStack.rbegin();
    while (it != SageBuilder::ScopeStack.rend() && func_def == NULL) {
        func_def = isSgFunctionDefinition(*it);
        it++;
    }
    ROSE_ASSERT(func_def != NULL);

  // Determine the name of the variable

    SgName name;

 // (01/29/2020) Pei-Hung: change to getIndentKind.  And this list is incomplete for Clang 9   
    // (10/30/2026) lin32: Explicitly reject unsupported predefined identifiers instead of continuing with an empty name.
#if CLANG_VERSION_MAJOR >= 18
    switch (predefined_expr->getIdentKind()) {
        case clang::PredefinedIdentKind::Func:
            name = "__func__";
            break;
        case clang::PredefinedIdentKind::Function:
            name = "__FUNCTION__";
            break;
        case clang::PredefinedIdentKind::PrettyFunction:
            name = "__PRETTY_FUNCTION__";
            break;
        case clang::PredefinedIdentKind::PrettyFunctionNoVirtual:
        default:
            ROSE_ABORT();
    }
#else
    switch (predefined_expr->getIdentKind()) {
        case clang::PredefinedExpr::Func:
            name = "__func__";
            break;
        case clang::PredefinedExpr::Function:
            name = "__FUNCTION__";
            break;
        case clang::PredefinedExpr::PrettyFunction:
            name = "__PRETTY_FUNCTION__";
            break;
        case clang::PredefinedExpr::PrettyFunctionNoVirtual:
        default:
            ROSE_ABORT();
    }
#endif

  // Retrieve the associate symbol if it exists

    SgVariableSymbol * symbol = func_def->lookup_variable_symbol(name);

  // Else, build a compiler generated initialized name for this variable in the function defintion scope.

    if (symbol == NULL) {
        SgInitializedName * init_name = SageBuilder::buildInitializedName_nfi(name, SageBuilder::buildPointerType(SageBuilder::buildCharType()), NULL);

        init_name->set_parent(func_def);
        init_name->set_scope(func_def);

        Sg_File_Info * start_fi = Sg_File_Info::generateDefaultFileInfoForCompilerGeneratedNode();
        start_fi->setCompilerGenerated();
        init_name->set_startOfConstruct(start_fi);

        Sg_File_Info * end_fi   = Sg_File_Info::generateDefaultFileInfoForCompilerGeneratedNode();
        end_fi->setCompilerGenerated();
        init_name->set_endOfConstruct(end_fi);

        symbol = new SgVariableSymbol(init_name);

        func_def->insert_symbol(name, symbol);
    }
    ROSE_ASSERT(symbol != NULL);

  // Finally build the variable reference

    *node = SageBuilder::buildVarRefExp_nfi(symbol);

    return VisitExpr(predefined_expr, node);
}

bool ClangToSageTranslator::VisitPseudoObjectExpr(clang::PseudoObjectExpr * pseudo_object_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitPseudoObjectExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(pseudo_object_expr, node) && res;
}

bool ClangToSageTranslator::VisitShuffleVectorExpr(clang::ShuffleVectorExpr * shuffle_vector_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitShuffleVectorExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(shuffle_vector_expr, node) && res;
}

bool ClangToSageTranslator::VisitSizeOfPackExpr(clang::SizeOfPackExpr * size_of_pack_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitSizeOfPackExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(size_of_pack_expr, node) && res;
}

bool ClangToSageTranslator::VisitSourceLocExpr(clang::SourceLocExpr * source_loc_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitSourceLocExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(source_loc_expr, node) && res;
}

bool ClangToSageTranslator::VisitStmtExpr(clang::StmtExpr * stmt_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitStmtExpr" << "\n";
#endif

    bool res = true;

    SgNode * tmp_substmt = Traverse(stmt_expr->getSubStmt());
    SgStatement * substmt = isSgStatement(tmp_substmt);
    if (tmp_substmt != NULL && substmt == NULL) {
        logger[WARN] << "Runtime error: tmp_substmt != NULL && substmt == NULL" << "\n";
        res = false;
    }

    *node = new SgStatementExpression(substmt);

    return VisitExpr(stmt_expr, node) && res;
}

bool ClangToSageTranslator::VisitStringLiteral(clang::StringLiteral * string_literal, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitStringLiteral" << "\n";
#endif
    bool res = true;

    std::string rawstr = string_literal->getBytes().str();
    const char * rawdata = string_literal->getBytes().data();
    std::string newstr;
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "In ClangToSageTranslator string_literal length: " << string_literal->getLength() << " byteLength:" << string_literal->getByteLength() << "\n";
#endif

// Pei-Hung (09/30/2022) handled wchar_t support.  
// Supports for UTF-8, UTF-16, UTF-32 might just follow this, but need more examples.
    if(string_literal->isWide())
    {
      void *memadrs = (void*)rawdata;
      wchar_t const *newText = (wchar_t const *)memadrs;
      for(int ii = 0; ii <  string_literal->getLength(); ++ii)
      {
        unsigned contentVal = static_cast<unsigned>(newText[ii]);
        std::stringstream ss;
/*
C99 6.4.3p2: A universal character name shall not specify a character whose short identifier 
is less than 00A0 other than 0024 ($), 0040 (@), or 0060 (`), nor one in the range D800 through DFFF inclusive.)
*/
        bool passUCharRule = false;
        passUCharRule = !((contentVal < 0xA0 && (contentVal != 0x24 || contentVal != 0x40 || contentVal != 0x60)) || (contentVal >= 0xD800 && contentVal <= 0xDFFF)); 
        if(passUCharRule)
        {
          ss << std::setfill('0') << std::setw(4)  << std::uppercase  << std::hex <<  (contentVal & 0xFF);
          newstr.append("\\u"+ss.str());
        }
        else
        {
          switch (char(contentVal)) {
              case '\\':
                  newstr.append("\\\\");
                  break;
              case '\n':
                  newstr.append("\\n");
                  break;
              case '\r':
                  newstr.append("\\r");
                  break;
              case '"':
                  newstr.append("\\r");
                  break;
              case '\0':
                  newstr.append("\0");
                  break;
              default:
                  newstr.push_back(char(contentVal));
          }
        }
      }
    }
    else if(string_literal->isUTF8() || string_literal->isUTF16() || string_literal->isUTF32())
    {
       ROSE_ASSERT(FAIL_TODO == 0); // TODO
       res = false;
    }
    else // ordinary
    {
      for(auto aa:rawstr)
      {
        switch (aa) {
            case '\\':
                newstr.append("\\\\");
                break;
            case '\n':
                newstr.append("\\n");
                break;
            case '\r':
                newstr.append("\\r");
                break;
            case '"':
                newstr.append("\\r");
                break;
            case '\0':
                newstr.append("\0");
                break;
            default:
                newstr.push_back(aa);
        }
      }
    }
    SgStringVal* sgStrVal = SageBuilder::buildStringVal(newstr);

    if(string_literal->isWide())
      sgStrVal->set_wcharString(true);
    *node = sgStrVal;

    return VisitExpr(string_literal, node) && res;
}

bool ClangToSageTranslator::VisitSubstNonTypeTemplateParmExpr(clang::SubstNonTypeTemplateParmExpr * subst_non_type_template_parm_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitSubstNonTypeTemplateParmExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(subst_non_type_template_parm_expr, node) && res;
}

bool ClangToSageTranslator::VisitSubstNonTypeTemplateParmPackExpr(clang::SubstNonTypeTemplateParmPackExpr * subst_non_type_template_parm_pack_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitSubstNonTypeTemplateParmPackExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(subst_non_type_template_parm_pack_expr, node) && res;
}

bool ClangToSageTranslator::VisitTypeTraitExpr(clang::TypeTraitExpr * type_trait, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitTypeTraitExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(type_trait, node) && res;
}


// (10/30/2026) lin32: Match the declaration/dispatch guards for TypoExpr, which was removed in Clang 21.
#if CLANG_VERSION_MAJOR < 21
bool ClangToSageTranslator::VisitTypoExpr(clang::TypoExpr * typo_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitTypoExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(typo_expr, node) && res;
}

#endif

bool ClangToSageTranslator::VisitUnaryExprOrTypeTraitExpr(clang::UnaryExprOrTypeTraitExpr * unary_expr_or_type_trait_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitUnaryExprOrTypeTraitExpr" << "\n";
#endif

    bool res = true;

    SgExpression * expr = NULL;
    SgType * type = NULL;

    if (unary_expr_or_type_trait_expr->isArgumentType()) {
        type = buildTypeFromQualifiedType(unary_expr_or_type_trait_expr->getArgumentType());
    }
    else {
        SgNode * tmp_expr = Traverse(unary_expr_or_type_trait_expr->getArgumentExpr());
        expr = isSgExpression(tmp_expr);

        if (tmp_expr != NULL && expr == NULL) {
            logger[WARN] << "Runtime error: tmp_expr != NULL && expr == NULL" << "\n";
            res = false;
        }
    }

    switch (unary_expr_or_type_trait_expr->getKind()) {
        case clang::UETT_SizeOf:
            if (type != NULL) 
            {
               std::map<SgClassType *, bool>::iterator bool_it = p_class_type_decl_first_see_in_type.find(isSgClassType(type));
               SgSizeOfOp* sizeofOp = SageBuilder::buildSizeOfOp_nfi(type);

               //Pei-Hung (08/16/22): try to follow VisitTypedefDecl to check if the classType is first seen 

               clang::QualType argumentQualType = unary_expr_or_type_trait_expr->getArgumentType();
               const clang::Type* argumentType = argumentQualType.getTypePtr();
               bool isembedded = false;
               bool iscompleteDefined = false;

               while((RoseClangCompat::isElaboratedType(argumentType)) || (llvm::isa<clang::PointerType>(argumentType)) || (llvm::isa<clang::ArrayType>(argumentType)))
               {
                  if(RoseClangCompat::isElaboratedType(argumentType))
                  {
                    argumentQualType = RoseClangCompat::getNamedType(argumentQualType);
                  }
                  else if(llvm::isa<clang::PointerType>(argumentType))
                  {
                    argumentQualType = ((clang::PointerType *)argumentType)->getPointeeType();
                  }
                  else if(llvm::isa<clang::ArrayType>(argumentType))
                  {
                    argumentQualType = ((clang::ArrayType *)argumentType)->getElementType();
                  }
                  argumentType = argumentQualType.getTypePtr();
               }

               if(llvm::isa<clang::RecordType>(argumentType))
               {
                  clang::RecordType* argumentRecordType = (clang::RecordType*)argumentType;
                  clang::RecordDecl* recordDeclaration = argumentRecordType->getDecl();
                  isembedded = recordDeclaration->isEmbeddedInDeclarator();
                  iscompleteDefined = recordDeclaration->isCompleteDefinition();
               }

               if (isSgClassType(type) && iscompleteDefined) {
                   std::map<SgClassType *, bool>::iterator bool_it = p_class_type_decl_first_see_in_type.find(isSgClassType(type));
                   ROSE_ASSERT(bool_it != p_class_type_decl_first_see_in_type.end());
                   if (bool_it->second) {
                       // Pei-Hung (08/16/22) If it is first seen, the definition should be unparsed in sizeofOp
                       sizeofOp->set_sizeOfContainsBaseTypeDefiningDeclaration(true);
                       bool_it->second = false;
                   }
                 
               }

               *node = sizeofOp;
            }
            else if (expr != NULL) *node = SageBuilder::buildSizeOfOp_nfi(expr);
            else res = false;
            break;
        case clang::UETT_AlignOf:
        case clang::UETT_PreferredAlignOf:
            if (type != NULL) {
              *node = SageBuilder::buildSizeOfOp_nfi(type);
              ROSE_ASSERT(FAIL_FIXME == 0); // difference between AlignOf and PreferredAlignOf is not represented in ROSE
            }
            else if (expr != NULL) *node = SageBuilder::buildSizeOfOp_nfi(expr);
            else res = false;
            break;
        case clang::UETT_VecStep:
            ROSE_ASSERT(!"OpenCL - VecStep is not supported!");
        default:
            ROSE_ASSERT(!"Unknown clang::UETT_xx");
    }

    return VisitStmt(unary_expr_or_type_trait_expr, node) && res;
}

bool ClangToSageTranslator::VisitUnaryOperator(clang::UnaryOperator * unary_operator, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitUnaryOperator" << "\n";
#endif

    bool res = true;

    SgNode * tmp_subexpr = Traverse(unary_operator->getSubExpr());
    SgExpression * subexpr = isSgExpression(tmp_subexpr);
    if (tmp_subexpr != NULL && subexpr == NULL) {
        logger[WARN] << "Runtime error: tmp_subexpr != NULL && subexpr == NULL" << "\n";
        res = false;
    }

    switch (unary_operator->getOpcode()) {
        case clang::UO_PostInc:
            *node = SageBuilder::buildPlusPlusOp(subexpr, SgUnaryOp::postfix);
            break;
        case clang::UO_PostDec:
            *node = SageBuilder::buildMinusMinusOp(subexpr, SgUnaryOp::postfix);
            break;
        case clang::UO_PreInc:
            *node = SageBuilder::buildPlusPlusOp(subexpr, SgUnaryOp::prefix);
            break;
        case clang::UO_PreDec:
            *node = SageBuilder::buildMinusMinusOp(subexpr, SgUnaryOp::prefix);
            break;
        case clang::UO_AddrOf:
            *node = SageBuilder::buildAddressOfOp(subexpr);
            break;
        case clang::UO_Deref:
            *node = SageBuilder::buildPointerDerefExp(subexpr);
            break;
        case clang::UO_Plus:
            *node = SageBuilder::buildUnaryAddOp(subexpr);
            break;
        case clang::UO_Minus:
            *node = SageBuilder::buildMinusOp(subexpr);
            break;
        // Def. in Clang: UNARY_OPERATION(Not, "~")
        case clang::UO_Not:
            *node = SageBuilder::buildBitComplementOp(subexpr);
            break;
        // Def. in UNARY_OPERATION(LNot, "!")
        case clang::UO_LNot:
            *node = SageBuilder::buildNotOp(subexpr);
            break;
        case clang::UO_Real:
            *node = SageBuilder::buildImagPartOp(subexpr);
            break;
        case clang::UO_Imag:
            *node = SageBuilder::buildRealPartOp(subexpr);
            break;
        case clang::UO_Extension:
            *node = subexpr;
            break;
        default:
            logger[WARN] << "Runtime error: Unknown unary operator." << "\n";
            res = false;
    }

    return VisitExpr(unary_operator, node) && res;
}

bool ClangToSageTranslator::VisitVAArgExpr(clang::VAArgExpr * va_arg_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitVAArgExpr" << "\n";
#endif

    SgNode * tmp_expr = Traverse(va_arg_expr->getSubExpr());
    SgExpression * expr = isSgExpression(tmp_expr);
    ROSE_ASSERT(expr != NULL);

    SgType* type = buildTypeFromQualifiedType(va_arg_expr->getWrittenTypeInfo()->getType());
    ROSE_ASSERT(type != NULL);

    *node = SageBuilder::buildVarArgOp_nfi(expr, type);

    return VisitExpr(va_arg_expr, node);
}
bool ClangToSageTranslator::VisitLabelStmt(clang::LabelStmt * label_stmt, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitLabelStmt" << "\n";
#endif

    bool res = true;

    SgName name(label_stmt->getName());

    *node = SageBuilder::buildLabelStatement_nfi(name, NULL, SageBuilder::topScopeStack());
    SgLabelStatement * sg_label_stmt = isSgLabelStatement(*node);

    SgFunctionDefinition * label_scope = NULL;
    std::list<SgScopeStatement *>::reverse_iterator it = SageBuilder::ScopeStack.rbegin();
    while (it != SageBuilder::ScopeStack.rend() && label_scope == NULL) {
        label_scope = isSgFunctionDefinition(*it);
        it++;
    }
    if (label_scope == NULL) {
         logger[DEBUG] << "Runtime error: Cannot find a surrounding function definition for the label statement: \"" << name << "\"." << "\n";
         res = false;
    }
    else {
        sg_label_stmt->set_scope(label_scope);
        SgLabelSymbol* label_sym = new SgLabelSymbol(sg_label_stmt);
        label_scope->insert_symbol(label_sym->get_name(), label_sym);
    }

    SgNode * tmp_sub_stmt = Traverse(label_stmt->getSubStmt());
    SgStatement * sg_sub_stmt = isSgStatement(tmp_sub_stmt);
    if (sg_sub_stmt == NULL) {
        SgExpression * sg_sub_expr = isSgExpression(tmp_sub_stmt);
        ROSE_ASSERT(sg_sub_expr != NULL);
        sg_sub_stmt = SageBuilder::buildExprStatement(sg_sub_expr);
    }

    ROSE_ASSERT(sg_sub_stmt != NULL);

    sg_sub_stmt->set_parent(sg_label_stmt);
    sg_label_stmt->set_statement(sg_sub_stmt);

    return VisitStmt(label_stmt, node) && res;
}

bool ClangToSageTranslator::VisitWhileStmt(clang::WhileStmt * while_stmt, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToSageTranslator::VisitWhileStmt" << "\n";
#endif

    SgNode * tmp_cond = Traverse(while_stmt->getCond());
    SgExpression * cond = isSgExpression(tmp_cond);
    ROSE_ASSERT(cond != NULL);

    SgStatement * expr_stmt = SageBuilder::buildExprStatement(cond);

    SgWhileStmt * sg_while_stmt = SageBuilder::buildWhileStmt_nfi(expr_stmt, NULL);

    sg_while_stmt->set_parent(SageBuilder::topScopeStack());

    cond->set_parent(expr_stmt);
    expr_stmt->set_parent(sg_while_stmt);

    SageBuilder::pushScopeStack(sg_while_stmt);

    SgNode * tmp_body = Traverse(while_stmt->getBody());
    SgStatement * body = isSgStatement(tmp_body);
    SgExpression * expr = isSgExpression(tmp_body);
    if (expr != NULL) {
        body =  SageBuilder::buildExprStatement(expr);
        applySourceRange(body, while_stmt->getBody()->getSourceRange());
    }
    ROSE_ASSERT(body != NULL);

    body->set_parent(sg_while_stmt);

    SageBuilder::popScopeStack();

    sg_while_stmt->set_body(body);

    *node = sg_while_stmt;

    return VisitStmt(while_stmt, node);
}
