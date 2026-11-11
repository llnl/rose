#include "sage3basic.h"

// #include "clang-frontend-private.hpp"
#include "clang-to-dot-private.hpp"

using namespace Sawyer::Message;

// SgNode * ClangToDotTranslator::Traverse(clang::Stmt * stmt) 
std::string ClangToDotTranslator::Traverse(clang::Stmt * stmt) 
   {
     if (stmt == NULL)
       // return NULL;
          return "";

 // Look for previous translation
     std::map<clang::Stmt *, std::string>::iterator it = p_stmt_translation_map.find(stmt);
     if (it != p_stmt_translation_map.end())
          return it->second; 

 // If first time, create a new entry
     std::string node_ident = genNextIdent();
     p_stmt_translation_map.insert(std::pair<clang::Stmt *, std::string>(stmt, node_ident));
     NodeDescriptor & node_desc = p_node_desc.insert(std::pair<std::string, NodeDescriptor>(node_ident, NodeDescriptor(node_ident))).first->second;

  // SgNode * result = NULL;
     bool ret_status = false;

  // CLANG_ROSE_Graph::graph (stmt);

     switch (stmt->getStmtClass()) 
        {
        case clang::Stmt::GCCAsmStmtClass:
          ret_status = VisitGCCAsmStmt((clang::GCCAsmStmt *)stmt, node_desc);
            break;
        case clang::Stmt::MSAsmStmtClass:
          ret_status = VisitMSAsmStmt((clang::MSAsmStmt *)stmt, node_desc);
            break;
        case clang::Stmt::BreakStmtClass:
            ret_status = VisitBreakStmt((clang::BreakStmt *)stmt, node_desc);
            break;
        case clang::Stmt::CapturedStmtClass:
            ret_status = VisitCapturedStmt((clang::CapturedStmt *)stmt, node_desc);
            break;
        case clang::Stmt::CompoundStmtClass:
            ret_status = VisitCompoundStmt((clang::CompoundStmt *)stmt, node_desc);
            break;
        case clang::Stmt::ContinueStmtClass:
            ret_status = VisitContinueStmt((clang::ContinueStmt *)stmt, node_desc);
            break;
        case clang::Stmt::CoreturnStmtClass:
            ret_status = VisitCoreturnStmt((clang::CoreturnStmt *)stmt, node_desc);
            break;
        case clang::Stmt::CXXCatchStmtClass:
            ret_status = VisitCXXCatchStmt((clang::CXXCatchStmt *)stmt, node_desc);
            break;
        case clang::Stmt::CXXForRangeStmtClass:
            ret_status = VisitCXXForRangeStmt((clang::CXXForRangeStmt *)stmt, node_desc);
            break;
        case clang::Stmt::CXXTryStmtClass:
            ret_status = VisitCXXTryStmt((clang::CXXTryStmt *)stmt, node_desc);
            break;
        case clang::Stmt::DeclStmtClass:
            ret_status = VisitDeclStmt((clang::DeclStmt *)stmt, node_desc);
            break;
        case clang::Stmt::DoStmtClass:
            ret_status = VisitDoStmt((clang::DoStmt *)stmt, node_desc);
            break;
        case clang::Stmt::ForStmtClass:
            ret_status = VisitForStmt((clang::ForStmt *)stmt, node_desc);
            break;
        case clang::Stmt::GotoStmtClass:
            ret_status = VisitGotoStmt((clang::GotoStmt *)stmt, node_desc);
            break;
        case clang::Stmt::IfStmtClass:
            ret_status = VisitIfStmt((clang::IfStmt *)stmt, node_desc);
            break;
        case clang::Stmt::IndirectGotoStmtClass:
            ret_status = VisitIndirectGotoStmt((clang::IndirectGotoStmt *)stmt, node_desc);
            break;
        case clang::Stmt::MSDependentExistsStmtClass:
            ret_status = VisitMSDependentExistsStmt((clang::MSDependentExistsStmt *)stmt, node_desc);
            break;
        case clang::Stmt::NullStmtClass:
            ret_status = VisitNullStmt((clang::NullStmt *)stmt, node_desc);
            break;
        case clang::Stmt::OMPAtomicDirectiveClass:
            ret_status = VisitOMPAtomicDirective((clang::OMPAtomicDirective *)stmt, node_desc);
            break;
        case clang::Stmt::OMPBarrierDirectiveClass:
            ret_status = VisitOMPBarrierDirective((clang::OMPBarrierDirective *)stmt, node_desc);
            break;
        case clang::Stmt::OMPCancellationPointDirectiveClass:
            ret_status = VisitOMPCancellationPointDirective((clang::OMPCancellationPointDirective *)stmt, node_desc);
            break;
        case clang::Stmt::OMPCriticalDirectiveClass:
            ret_status = VisitOMPCriticalDirective((clang::OMPCriticalDirective *)stmt, node_desc);
            break;
        case clang::Stmt::OMPFlushDirectiveClass:
            ret_status = VisitOMPFlushDirective((clang::OMPFlushDirective *)stmt, node_desc);
            break;
        case clang::Stmt::OMPDistributeDirectiveClass:
            ret_status = VisitOMPDistributeDirective((clang::OMPDistributeDirective *)stmt, node_desc);
            break;
        case clang::Stmt::OMPDistributeParallelForDirectiveClass:
            ret_status = VisitOMPDistributeParallelForDirective((clang::OMPDistributeParallelForDirective *)stmt, node_desc);
            break;
        case clang::Stmt::OMPDistributeParallelForSimdDirectiveClass:
            ret_status = VisitOMPDistributeParallelForSimdDirective((clang::OMPDistributeParallelForSimdDirective *)stmt, node_desc);
            break;
        case clang::Stmt::OMPDistributeSimdDirectiveClass:
            ret_status = VisitOMPDistributeSimdDirective((clang::OMPDistributeSimdDirective *)stmt, node_desc);
            break;
        case clang::Stmt::OMPForDirectiveClass:
            ret_status = VisitOMPForDirective((clang::OMPForDirective *)stmt, node_desc);
            break;
        case clang::Stmt::OMPForSimdDirectiveClass:
            ret_status = VisitOMPForSimdDirective((clang::OMPForSimdDirective *)stmt, node_desc);
            break;
        //case clang::Stmt::OMPMasterTaskLoopDirectiveClass:
        //    ret_status = VisitOMPMasterTaskLoopDirective((clang::OMPMasterTaskLoopDirective *)stmt, node_desc);
        //    break;
        //case clang::Stmt::OMPMasterTaskLoopSimdDirectiveClass:
        //    ret_status = VisitOMPMasterTaskLoopSimdDirective((clang::OMPMasterTaskLoopSimdDirective *)stmt, node_desc);
        //    break;
        case clang::Stmt::OMPParallelForDirectiveClass:
            ret_status = VisitOMPParallelForDirective((clang::OMPParallelForDirective *)stmt, node_desc);
            break;
        case clang::Stmt::OMPParallelForSimdDirectiveClass:
            ret_status = VisitOMPParallelForSimdDirective((clang::OMPParallelForSimdDirective *)stmt, node_desc);
            break;
        //case clang::Stmt::OMPParallelMasterTaskLoopDirectiveClass:
        //    ret_status = VisitOMPParallelMasterTaskLoopDirective((clang::OMPParallelMasterTaskLoopDirective *)stmt, node_desc);
        //    break;
        case clang::Stmt::OMPSimdDirectiveClass:
            ret_status = VisitOMPSimdDirective((clang::OMPSimdDirective *)stmt, node_desc);
            break;
        case clang::Stmt::OMPTargetParallelForDirectiveClass:
            ret_status = VisitOMPTargetParallelForDirective((clang::OMPTargetParallelForDirective *)stmt, node_desc);
            break;
        case clang::Stmt::OMPTargetParallelForSimdDirectiveClass:
            ret_status = VisitOMPTargetParallelForSimdDirective((clang::OMPTargetParallelForSimdDirective *)stmt, node_desc);
            break;
        case clang::Stmt::OMPTargetSimdDirectiveClass:
            ret_status = VisitOMPTargetSimdDirective((clang::OMPTargetSimdDirective *)stmt, node_desc);
            break;
        case clang::Stmt::OMPTargetTeamsDistributeDirectiveClass:
            ret_status = VisitOMPTargetTeamsDistributeDirective((clang::OMPTargetTeamsDistributeDirective *)stmt, node_desc);
            break;
        //case clang::Stmt::OMPTargetTeamsDistributeParallelForSimdDirectiveClass:
        //    ret_status = VisitOMPTargetTeamsDistributeParallelForSimdDirective((clang::OMPTargetTeamsDistributeParallelForSimdDirective *)stmt, node_desc);
        //    break;
        case clang::Stmt::OMPTargetTeamsDistributeSimdDirectiveClass:
            ret_status = VisitOMPTargetTeamsDistributeSimdDirective((clang::OMPTargetTeamsDistributeSimdDirective *)stmt, node_desc);
            break;
        case clang::Stmt::OMPTaskLoopDirectiveClass:
            ret_status = VisitOMPTaskLoopDirective((clang::OMPTaskLoopDirective *)stmt, node_desc);
            break;
        case clang::Stmt::OMPTaskLoopSimdDirectiveClass:
            ret_status = VisitOMPTaskLoopSimdDirective((clang::OMPTaskLoopSimdDirective *)stmt, node_desc);
            break;
        //case clang::Stmt::OMPTeamDistributeDirectiveClass:
        //    ret_status = VisitOMPTeamDistributeDirective((clang::OMPTeamDistributeDirective *)stmt, node_desc);
        //    break;
        //case clang::Stmt::OMPTeamDistributeParallelForSimdDirectiveClass:
        //    ret_status = VisitOMPTeamDistributeParallelForSimdDirective((clang::OMPTeamDistributeParallelForSimdDirective *)stmt, node_desc);
        //    break;
        //case clang::Stmt::OMPTeamDistributeSimdDirectiveClass:
        //    ret_status = VisitOMPTeamDistributeSimdDirective((clang::OMPTeamDistributeSimdDirective *)stmt, node_desc);
        //    break;
        case clang::Stmt::OMPMasterDirectiveClass:
            ret_status = VisitOMPMasterDirective((clang::OMPMasterDirective *)stmt, node_desc);
            break;
        case clang::Stmt::OMPOrderedDirectiveClass:
            ret_status = VisitOMPOrderedDirective((clang::OMPOrderedDirective *)stmt, node_desc);
            break;
        case clang::Stmt::OMPParallelDirectiveClass:
            ret_status = VisitOMPParallelDirective((clang::OMPParallelDirective *)stmt, node_desc);
            break;
        case clang::Stmt::OMPParallelSectionsDirectiveClass:
            ret_status = VisitOMPParallelSectionsDirective((clang::OMPParallelSectionsDirective *)stmt, node_desc);
            break;
        case clang::Stmt::ReturnStmtClass:
            ret_status = VisitReturnStmt((clang::ReturnStmt *)stmt, node_desc);
            break;
        case clang::Stmt::SEHExceptStmtClass:
            ret_status = VisitSEHExceptStmt((clang::SEHExceptStmt *)stmt, node_desc);
            break;
        case clang::Stmt::SEHFinallyStmtClass:
            ret_status = VisitSEHFinallyStmt((clang::SEHFinallyStmt *)stmt, node_desc);
            break;
        case clang::Stmt::SEHLeaveStmtClass:
            ret_status = VisitSEHLeaveStmt((clang::SEHLeaveStmt *)stmt, node_desc);
            break;
        case clang::Stmt::SEHTryStmtClass:
            ret_status = VisitSEHTryStmt((clang::SEHTryStmt *)stmt, node_desc);
            break;
        case clang::Stmt::CaseStmtClass:
            ret_status = VisitCaseStmt((clang::CaseStmt *)stmt, node_desc);
            break;
        case clang::Stmt::DefaultStmtClass:
            ret_status = VisitDefaultStmt((clang::DefaultStmt *)stmt, node_desc);
            break;
        case clang::Stmt::SwitchStmtClass:
            ret_status = VisitSwitchStmt((clang::SwitchStmt *)stmt, node_desc);
            break;
        case clang::Stmt::AttributedStmtClass:
            ret_status = VisitAttributedStmt((clang::AttributedStmt *)stmt, node_desc);
            break;
        case clang::Stmt::BinaryConditionalOperatorClass:
            ret_status = VisitBinaryConditionalOperator((clang::BinaryConditionalOperator *)stmt, node_desc);
            break;
        case clang::Stmt::ConditionalOperatorClass:
            ret_status = VisitConditionalOperator((clang::ConditionalOperator *)stmt, node_desc);
            break;
        case clang::Stmt::AddrLabelExprClass:
          ret_status = VisitAddrLabelExpr((clang::AddrLabelExpr *)stmt, node_desc);
            break;
        case clang::Stmt::ArrayInitIndexExprClass:
            ret_status = VisitArrayInitIndexExpr((clang::ArrayInitIndexExpr *)stmt, node_desc);
            break;
        case clang::Stmt::ArrayInitLoopExprClass:
            ret_status = VisitArrayInitLoopExpr((clang::ArrayInitLoopExpr *)stmt, node_desc);
            break;
        case clang::Stmt::ArraySubscriptExprClass:
            ret_status = VisitArraySubscriptExpr((clang::ArraySubscriptExpr *)stmt, node_desc);
            break;
        case clang::Stmt::ArrayTypeTraitExprClass:
            ret_status = VisitArrayTypeTraitExpr((clang::ArrayTypeTraitExpr *)stmt, node_desc);
            break;
        case clang::Stmt::AsTypeExprClass:
            ret_status = VisitAsTypeExpr((clang::AsTypeExpr *)stmt, node_desc);
            break;
        case clang::Stmt::AtomicExprClass:
            ret_status = VisitAtomicExpr((clang::AtomicExpr *)stmt, node_desc);
            break;
        case clang::Stmt::CompoundAssignOperatorClass:
            ret_status = VisitCompoundAssignOperator((clang::CompoundAssignOperator *)stmt, node_desc);
            break;
        case clang::Stmt::BlockExprClass:
            ret_status = VisitBlockExpr((clang::BlockExpr *)stmt, node_desc);
            break;
        case clang::Stmt::CUDAKernelCallExprClass:
            ret_status = VisitCUDAKernelCallExpr((clang::CUDAKernelCallExpr *)stmt, node_desc);
            break;
        case clang::Stmt::CXXMemberCallExprClass:
            ret_status = VisitCXXMemberCallExpr((clang::CXXMemberCallExpr *)stmt, node_desc);
            break;
        case clang::Stmt::CXXOperatorCallExprClass:
            ret_status = VisitCXXOperatorCallExpr((clang::CXXOperatorCallExpr *)stmt, node_desc);
            break;
        case clang::Stmt::UserDefinedLiteralClass:
            ret_status = VisitUserDefinedLiteral((clang::UserDefinedLiteral *)stmt, node_desc);
            break;
        case clang::Stmt::BuiltinBitCastExprClass:
            ret_status = VisitBuiltinBitCastExpr((clang::BuiltinBitCastExpr *)stmt, node_desc);
            break;
        case clang::Stmt::CStyleCastExprClass:
            ret_status = VisitCStyleCastExpr((clang::CStyleCastExpr *)stmt, node_desc);
            break;
        case clang::Stmt::CXXFunctionalCastExprClass:
            ret_status = VisitCXXFunctionalCastExpr((clang::CXXFunctionalCastExpr *)stmt, node_desc);
            break;
        case clang::Stmt::CXXConstCastExprClass:
            ret_status = VisitCXXConstCastExpr((clang::CXXConstCastExpr *)stmt, node_desc);
            break;
        case clang::Stmt::CXXDynamicCastExprClass:
            ret_status = VisitCXXDynamicCastExpr((clang::CXXDynamicCastExpr *)stmt, node_desc);
            break;
        case clang::Stmt::CXXReinterpretCastExprClass:
            ret_status = VisitCXXReinterpretCastExpr((clang::CXXReinterpretCastExpr *)stmt, node_desc);
            break;
        case clang::Stmt::CXXStaticCastExprClass:
            ret_status = VisitCXXStaticCastExpr((clang::CXXStaticCastExpr *)stmt, node_desc);
            break;
        case clang::Stmt::ImplicitCastExprClass:
            ret_status = VisitImplicitCastExpr((clang::ImplicitCastExpr *)stmt, node_desc);
            break;
        case clang::Stmt::CharacterLiteralClass:
            ret_status = VisitCharacterLiteral((clang::CharacterLiteral *)stmt, node_desc);
            break;
        case clang::Stmt::ChooseExprClass:
            ret_status = VisitChooseExpr((clang::ChooseExpr *)stmt, node_desc);
            break;
        case clang::Stmt::CompoundLiteralExprClass:
            ret_status = VisitCompoundLiteralExpr((clang::CompoundLiteralExpr *)stmt, node_desc);
            break;
        //case clang::Stmt::ConceptSpecializationExprClass:
        //    ret_status = VisitConceptSpecializationExpr((clang::ConceptSpecializationExpr *)stmt, node_desc);
        //    break;
        case clang::Stmt::ConvertVectorExprClass:
            ret_status = VisitConvertVectorExpr((clang::ConvertVectorExpr *)stmt, node_desc);
            break;
        case clang::Stmt::CoawaitExprClass:
            ret_status = VisitCoawaitExpr((clang::CoawaitExpr *)stmt, node_desc);
            break;
        case clang::Stmt::CoyieldExprClass:
            ret_status = VisitCoyieldExpr((clang::CoyieldExpr *)stmt, node_desc);
            break;
        case clang::Stmt::CXXBindTemporaryExprClass:
            ret_status = VisitCXXBindTemporaryExpr((clang::CXXBindTemporaryExpr *)stmt, node_desc);
            break;
        case clang::Stmt::CXXBoolLiteralExprClass:
            ret_status = VisitCXXBoolLiteralExpr((clang::CXXBoolLiteralExpr *)stmt, node_desc);
            break;
        case clang::Stmt::CXXConstructExprClass:
            ret_status = VisitCXXConstructExpr((clang::CXXConstructExpr *)stmt, node_desc);
            break;
        case clang::Stmt::CXXTemporaryObjectExprClass:
            ret_status = VisitCXXTemporaryObjectExpr((clang::CXXTemporaryObjectExpr *)stmt, node_desc);
            break;
        case clang::Stmt::CXXDefaultArgExprClass:
            ret_status = VisitCXXDefaultArgExpr((clang::CXXDefaultArgExpr *)stmt, node_desc);
            break;
        case clang::Stmt::CXXDefaultInitExprClass:
            ret_status = VisitCXXDefaultInitExpr((clang::CXXDefaultInitExpr *)stmt, node_desc);
            break;
        case clang::Stmt::CXXDeleteExprClass:
            ret_status = VisitCXXDeleteExpr((clang::CXXDeleteExpr *)stmt, node_desc);
            break;
        case clang::Stmt::CXXDependentScopeMemberExprClass:
            ret_status = VisitCXXDependentScopeMemberExpr((clang::CXXDependentScopeMemberExpr *)stmt, node_desc);
            break;
        case clang::Stmt::CXXFoldExprClass:
            ret_status = VisitCXXFoldExpr((clang::CXXFoldExpr *)stmt, node_desc);
            break;
        case clang::Stmt::CXXInheritedCtorInitExprClass:
            ret_status = VisitCXXInheritedCtorInitExpr((clang::CXXInheritedCtorInitExpr *)stmt, node_desc);
            break;
        case clang::Stmt::CXXNewExprClass:
            ret_status = VisitCXXNewExpr((clang::CXXNewExpr *)stmt, node_desc);
            break;
        case clang::Stmt::CXXNoexceptExprClass:
            ret_status = VisitCXXNoexceptExpr((clang::CXXNoexceptExpr *)stmt, node_desc);
            break;
        case clang::Stmt::CXXNullPtrLiteralExprClass:
            ret_status = VisitCXXNullPtrLiteralExpr((clang::CXXNullPtrLiteralExpr *)stmt, node_desc);
            break;
        case clang::Stmt::CXXPseudoDestructorExprClass:
            ret_status = VisitCXXPseudoDestructorExpr((clang::CXXPseudoDestructorExpr *)stmt, node_desc);
            break;
        //case clang::Stmt::CXXRewrittenBinaryOperatorClass:
        //    ret_status = VisitCXXRewrittenBinaryOperator((clang::CXXRewrittenBinaryOperator *)stmt, node_desc);
        //    break;
        case clang::Stmt::CXXScalarValueInitExprClass:
            ret_status = VisitCXXScalarValueInitExpr((clang::CXXScalarValueInitExpr *)stmt, node_desc);
            break;
        case clang::Stmt::CXXStdInitializerListExprClass:
            ret_status = VisitCXXStdInitializerListExpr((clang::CXXStdInitializerListExpr *)stmt, node_desc);
            break;
        case clang::Stmt::CXXThisExprClass:
            ret_status = VisitCXXThisExpr((clang::CXXThisExpr *)stmt, node_desc);
            break;
        case clang::Stmt::CXXThrowExprClass:
            ret_status = VisitCXXThrowExpr((clang::CXXThrowExpr *)stmt, node_desc);
            break;
        case clang::Stmt::CXXTypeidExprClass:
            ret_status = VisitCXXTypeidExpr((clang::CXXTypeidExpr *)stmt, node_desc);
            break;
        case clang::Stmt::CXXUnresolvedConstructExprClass:
            ret_status = VisitCXXUnresolvedConstructExpr((clang::CXXUnresolvedConstructExpr *)stmt, node_desc);
            break;
        case clang::Stmt::CXXUuidofExprClass:
            ret_status = VisitCXXUuidofExpr((clang::CXXUuidofExpr *)stmt, node_desc);
            break;
        case clang::Stmt::DeclRefExprClass:
            ret_status = VisitDeclRefExpr((clang::DeclRefExpr *)stmt, node_desc);
            break;
        case clang::Stmt::DependentCoawaitExprClass:
            ret_status = VisitDependentCoawaitExpr((clang::DependentCoawaitExpr *)stmt, node_desc);
            break;
        case clang::Stmt::DependentScopeDeclRefExprClass:
            ret_status = VisitDependentScopeDeclRefExpr((clang::DependentScopeDeclRefExpr *)stmt, node_desc);
            break;
        case clang::Stmt::DesignatedInitExprClass:
            ret_status = VisitDesignatedInitExpr((clang::DesignatedInitExpr *)stmt, node_desc);
            break;
        case clang::Stmt::DesignatedInitUpdateExprClass:
            ret_status = VisitDesignatedInitUpdateExpr((clang::DesignatedInitUpdateExpr *)stmt, node_desc);
            break;
        case clang::Stmt::ExpressionTraitExprClass:
            ret_status = VisitExpressionTraitExpr((clang::ExpressionTraitExpr *)stmt, node_desc);
            break;
        case clang::Stmt::ExtVectorElementExprClass:
            ret_status = VisitExtVectorElementExpr((clang::ExtVectorElementExpr *)stmt, node_desc);
            break;
        case clang::Stmt::FixedPointLiteralClass:
            ret_status = VisitFixedPointLiteral((clang::FixedPointLiteral *)stmt, node_desc);
            break;
        case clang::Stmt::FloatingLiteralClass:
            ret_status = VisitFloatingLiteral((clang::FloatingLiteral *)stmt, node_desc);
            break;
        case clang::Stmt::ConstantExprClass:
            ret_status = VisitConstantExpr((clang::ConstantExpr *)stmt, node_desc);
            break;
        case clang::Stmt::ExprWithCleanupsClass:
            ret_status = VisitExprWithCleanups((clang::ExprWithCleanups *)stmt, node_desc);
            break;
        case clang::Stmt::FunctionParmPackExprClass:
            ret_status = VisitFunctionParmPackExpr((clang::FunctionParmPackExpr *)stmt, node_desc);
            break;
        case clang::Stmt::GenericSelectionExprClass:
            ret_status = VisitGenericSelectionExpr((clang::GenericSelectionExpr *)stmt, node_desc);
            break;
        case clang::Stmt::GNUNullExprClass:
            ret_status = VisitGNUNullExpr((clang::GNUNullExpr *)stmt, node_desc);
            break;
        case clang::Stmt::ImaginaryLiteralClass:
            ret_status = VisitImaginaryLiteral((clang::ImaginaryLiteral *)stmt, node_desc);
            break;
        case clang::Stmt::ImplicitValueInitExprClass:
            ret_status = VisitImplicitValueInitExpr((clang::ImplicitValueInitExpr *)stmt, node_desc);
            break;
        case clang::Stmt::InitListExprClass:
            ret_status = VisitInitListExpr((clang::InitListExpr *)stmt, node_desc);
            break;
        case clang::Stmt::IntegerLiteralClass:
            ret_status = VisitIntegerLiteral((clang::IntegerLiteral *)stmt, node_desc);
            break;
        case clang::Stmt::LambdaExprClass:
            ret_status = VisitLambdaExpr((clang::LambdaExpr *)stmt, node_desc);
            break;
        case clang::Stmt::MaterializeTemporaryExprClass:
            ret_status = VisitMaterializeTemporaryExpr((clang::MaterializeTemporaryExpr *)stmt, node_desc);
            break;
        case clang::Stmt::MemberExprClass:
            ret_status = VisitMemberExpr((clang::MemberExpr *)stmt, node_desc);
            break;
        case clang::Stmt::MSPropertyRefExprClass:
            ret_status = VisitMSPropertyRefExpr((clang::MSPropertyRefExpr *)stmt, node_desc);
            break;
        case clang::Stmt::MSPropertySubscriptExprClass:
            ret_status = VisitMSPropertySubscriptExpr((clang::MSPropertySubscriptExpr *)stmt, node_desc);
            break;
        case clang::Stmt::NoInitExprClass:
            ret_status = VisitNoInitExpr((clang::NoInitExpr *)stmt, node_desc);
            break;
        case clang::Stmt::OffsetOfExprClass:
            ret_status = VisitOffsetOfExpr((clang::OffsetOfExpr *)stmt, node_desc);
            break;
        case RoseClangCompat::ArraySectionExprClass:
            ret_status = VisitOMPArraySectionExpr((RoseClangCompat::ArraySectionExpr *)stmt, node_desc);
            break;
        case clang::Stmt::OpaqueValueExprClass:
            ret_status = VisitOpaqueValueExpr((clang::OpaqueValueExpr *)stmt, node_desc);
            break;
        case clang::Stmt::UnresolvedLookupExprClass:
            ret_status = VisitUnresolvedLookupExpr((clang::UnresolvedLookupExpr *)stmt, node_desc);
            break;
        case clang::Stmt::UnresolvedMemberExprClass:
            ret_status = VisitUnresolvedMemberExpr((clang::UnresolvedMemberExpr *)stmt, node_desc);
            break;
        case clang::Stmt::PackExpansionExprClass:
            ret_status = VisitPackExpansionExpr((clang::PackExpansionExpr *)stmt, node_desc);
            break;
        case clang::Stmt::ParenExprClass:
            ret_status = VisitParenExpr((clang::ParenExpr *)stmt, node_desc);
            break;
        case clang::Stmt::ParenListExprClass:
            ret_status = VisitParenListExpr((clang::ParenListExpr *)stmt, node_desc);
            break;
        case clang::Stmt::PredefinedExprClass:
            ret_status = VisitPredefinedExpr((clang::PredefinedExpr *)stmt, node_desc);
            break;
        case clang::Stmt::PseudoObjectExprClass:
            ret_status = VisitPseudoObjectExpr((clang::PseudoObjectExpr *)stmt, node_desc);
            break;
        case clang::Stmt::ShuffleVectorExprClass:
            ret_status = VisitShuffleVectorExpr((clang::ShuffleVectorExpr *)stmt, node_desc);
            break;
        case clang::Stmt::SizeOfPackExprClass:
            ret_status = VisitSizeOfPackExpr((clang::SizeOfPackExpr *)stmt, node_desc);
            break;
        case clang::Stmt::SourceLocExprClass:
            ret_status = VisitSourceLocExpr((clang::SourceLocExpr *)stmt, node_desc);
            break;
        case clang::Stmt::StmtExprClass:
            ret_status = VisitStmtExpr((clang::StmtExpr *)stmt, node_desc);
            break;
        case clang::Stmt::StringLiteralClass:
            ret_status = VisitStringLiteral((clang::StringLiteral *)stmt, node_desc);
            break;
        case clang::Stmt::SubstNonTypeTemplateParmPackExprClass:
            ret_status = VisitSubstNonTypeTemplateParmPackExpr((clang::SubstNonTypeTemplateParmPackExpr *)stmt, node_desc);
            break;
        case clang::Stmt::TypeTraitExprClass:
            ret_status = VisitTypeTraitExpr((clang::TypeTraitExpr *)stmt, node_desc);
            break;
        // (10/30/2026) lin32: Clang 21 removed TypoExpr and its statement kind; exclude the old DOT dispatch case.
#if CLANG_VERSION_MAJOR < 21
        case clang::Stmt::TypoExprClass:
            ret_status = VisitTypoExpr((clang::TypoExpr *)stmt, node_desc);
            break;
#endif
        case clang::Stmt::UnaryExprOrTypeTraitExprClass:
            ret_status = VisitUnaryExprOrTypeTraitExpr((clang::UnaryExprOrTypeTraitExpr *)stmt, node_desc);
            break;
        case clang::Stmt::VAArgExprClass:
            ret_status = VisitVAArgExpr((clang::VAArgExpr *)stmt, node_desc);
            break;
        case clang::Stmt::LabelStmtClass:
            ret_status = VisitLabelStmt((clang::LabelStmt *)stmt, node_desc);
            break;
        case clang::Stmt::WhileStmtClass:
            ret_status = VisitWhileStmt((clang::WhileStmt *)stmt, node_desc);
            break;
        case clang::Stmt::UnaryOperatorClass:
            ret_status = VisitUnaryOperator((clang::UnaryOperator *)stmt, node_desc);
            break;
        case clang::Stmt::CallExprClass:
            ret_status = VisitCallExpr((clang::CallExpr *)stmt, node_desc);
            break;
        case clang::Stmt::BinaryOperatorClass:
            ret_status = VisitBinaryOperator((clang::BinaryOperator *)stmt, node_desc);
            break;
        default:
            logger[ERROR] << "Unknown statement kind: " << stmt->getStmtClassName() << " !" << "\n";
            ROSE_ABORT();
    }

 // ROSE_ASSERT(result != NULL);
 // p_stmt_translation_map.insert(std::pair<clang::Stmt *, SgNode *>(stmt, result));
 // return result;

    assert(ret_status != false);

    return node_ident;
}

/********************/
/* Visit Statements */
/********************/

#if 0
bool ClangToDotTranslator::VisitStmt(clang::Stmt * stmt, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitStmt" << "\n";
#endif

    if (*node == NULL) {
        logger[WARN] << "Runtime error: No Sage node associated with the Statement..." << "\n";
        return false;
    }

    // TODO Is there anything else todo?

    if (
        isSgLocatedNode(*node) != NULL &&
        (
            isSgLocatedNode(*node)->get_file_info() == NULL ||
            !(isSgLocatedNode(*node)->get_file_info()->isCompilerGenerated())
        )
    ) {
        applySourceRange(*node, stmt->getSourceRange());
    }

    return true;
}
#else
bool ClangToDotTranslator::VisitStmt(clang::Stmt * stmt, NodeDescriptor & node_desc)
   {
#if DEBUG_VISIT_STMT
     logger[DEBUG] << "ClangToDotTranslator::VisitStmt" << "\n";
#endif

#if 0
    if (*node == NULL) {
        logger[WARN] << "Runtime error: No Sage node associated with the Statement..." << "\n";
        return false;
    }

    // TODO Is there anything else todo?

    if (
        isSgLocatedNode(*node) != NULL &&
        (
            isSgLocatedNode(*node)->get_file_info() == NULL ||
            !(isSgLocatedNode(*node)->get_file_info()->isCompilerGenerated())
        )
    ) {
        applySourceRange(*node, stmt->getSourceRange());
    }
#endif

     node_desc.kind_hierarchy.push_back("Stmt");

     return true;
   }
#endif

#if 0
bool ClangToDotTranslator::VisitAsmStmt(clang::AsmStmt * asm_stmt, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitAsmStmt" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO
    return VisitStmt(asm_stmt, node) && res;
}
#else
bool ClangToDotTranslator::VisitAsmStmt(clang::AsmStmt * asm_stmt, NodeDescriptor & node_desc)
   {
#if DEBUG_VISIT_STMT
     logger[DEBUG] << "ClangToDotTranslator::VisitAsmStmt" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("AsmStmt");

     ROSE_ASSERT(FAIL_TODO == 0); // TODO
     return VisitStmt(asm_stmt, node_desc) && res;
   }
#endif

#if 0
bool ClangToDotTranslator::VisitGCCAsmStmt(clang::GCCAsmStmt * gcc_asm_stmt, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitGCCAsmStmt" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO
    return VisitStmt(gcc_asm_stmt, node) && res;
}
#else
bool ClangToDotTranslator::VisitGCCAsmStmt(clang::GCCAsmStmt * gcc_asm_stmt, NodeDescriptor & node_desc)
   {
#if DEBUG_VISIT_STMT
     logger[DEBUG] << "ClangToDotTranslator::VisitGCCAsmStmt" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("GCCAsmStmt");

  // ROSE_ASSERT(FAIL_TODO == 0); // TODO
     printf ("ClangToDotTranslator::VisitGCCAsmStmt called but not implemented! \n");

     return VisitStmt(gcc_asm_stmt, node_desc) && res;
   }
#endif

#if 0
bool ClangToDotTranslator::VisitMSAsmStmt(clang::MSAsmStmt * ms_asm_stmt, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitMSAsmStmt" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO
    return VisitStmt(ms_asm_stmt, node) && res;
}
#else
bool ClangToDotTranslator::VisitMSAsmStmt(clang::MSAsmStmt * ms_asm_stmt, NodeDescriptor & node_desc) 
   {
#if DEBUG_VISIT_STMT
     logger[DEBUG] << "ClangToDotTranslator::VisitMSAsmStmt" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("MSAsmStmt");

     ROSE_ASSERT(FAIL_TODO == 0); // TODO
     return VisitStmt(ms_asm_stmt, node_desc) && res;
   }
#endif

#if 0
bool ClangToDotTranslator::VisitBreakStmt(clang::BreakStmt * break_stmt, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitBreakStmt" << "\n";
#endif

    *node = SageBuilder::buildBreakStmt();
    return VisitStmt(break_stmt, node);
}
#else
bool ClangToDotTranslator::VisitBreakStmt(clang::BreakStmt * break_stmt, NodeDescriptor & node_desc) 
   {
#if DEBUG_VISIT_STMT
     logger[DEBUG] << "ClangToDotTranslator::VisitBreakStmt" << "\n";
#endif

#if 0
     *node = SageBuilder::buildBreakStmt();
#endif

     node_desc.kind_hierarchy.push_back("BreakStmt");

     return VisitStmt(break_stmt, node_desc);
   }
#endif

#if 0
bool ClangToDotTranslator::VisitCapturedStmt(clang::CapturedStmt * captured_stmt, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCapturedStmt" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO
    return VisitStmt(captured_stmt, node) && res;
}
#else
bool ClangToDotTranslator::VisitCapturedStmt(clang::CapturedStmt * captured_stmt, NodeDescriptor & node_desc)
   {
#if DEBUG_VISIT_STMT
     logger[DEBUG] << "ClangToDotTranslator::VisitCapturedStmt" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("CapturedStmt");

     ROSE_ASSERT(FAIL_TODO == 0); // TODO
     return VisitStmt(captured_stmt, node_desc) && res;
   }
#endif

#if 0
bool ClangToDotTranslator::VisitCompoundStmt(clang::CompoundStmt * compound_stmt, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCompoundStmt" << "\n";
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
#else
bool ClangToDotTranslator::VisitCompoundStmt(clang::CompoundStmt * compound_stmt, NodeDescriptor & node_desc) 
   {
#if DEBUG_VISIT_STMT
     logger[DEBUG] << "ClangToDotTranslator::VisitCompoundStmt" << "\n";
#endif

     bool res = true;

#if 0
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
#endif

     node_desc.kind_hierarchy.push_back("CompoundStmt");

     clang::CompoundStmt::body_iterator it;
     unsigned cnt = 0;
     for (it = compound_stmt->body_begin(); it != compound_stmt->body_end(); it++) 
        {
          std::ostringstream oss;
          oss << "child[" << cnt++ << "]";
          node_desc.successors.push_back(std::pair<std::string, std::string>(oss.str(), Traverse(*it)));
        }

     return VisitStmt(compound_stmt, node_desc) && res;
   }
#endif

#if 0
bool ClangToDotTranslator::VisitContinueStmt(clang::ContinueStmt * continue_stmt, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitContinueStmt" << "\n";
#endif

    *node = SageBuilder::buildContinueStmt();
    return VisitStmt(continue_stmt, node);
}
#else
bool ClangToDotTranslator::VisitContinueStmt(clang::ContinueStmt * continue_stmt, NodeDescriptor & node_desc)
   {
#if DEBUG_VISIT_STMT
     logger[DEBUG] << "ClangToDotTranslator::VisitContinueStmt" << "\n";
#endif

#if 0
     *node = SageBuilder::buildContinueStmt();
#endif

     node_desc.kind_hierarchy.push_back("ContinueStmt");

     return VisitStmt(continue_stmt, node_desc);
   }
#endif

#if 0
bool ClangToDotTranslator::VisitCoreturnStmt(clang::CoreturnStmt * core_turn_stmt, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCoreturnStmt" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO
    return VisitStmt(core_turn_stmt, node) && res;
}
#else
bool ClangToDotTranslator::VisitCoreturnStmt(clang::CoreturnStmt * core_turn_stmt, NodeDescriptor & node_desc) 
   {
#if DEBUG_VISIT_STMT
     logger[DEBUG] << "ClangToDotTranslator::VisitCoreturnStmt" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("CoreturnStmt");

     ROSE_ASSERT(FAIL_TODO == 0); // TODO
     return VisitStmt(core_turn_stmt, node_desc) && res;
   }
#endif

#if 0
bool ClangToDotTranslator::VisitCoroutineBodyStmt(clang::CoroutineBodyStmt * coroutine_body_stmt, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCoroutineBodyStmt" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO
    return VisitStmt(coroutine_body_stmt, node) && res;
}
#else
bool ClangToDotTranslator::VisitCoroutineBodyStmt(clang::CoroutineBodyStmt * coroutine_body_stmt, NodeDescriptor & node_desc) 
   {
#if DEBUG_VISIT_STMT
     logger[DEBUG] << "ClangToDotTranslator::VisitCoroutineBodyStmt" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("CoroutineBodyStmt");

     ROSE_ASSERT(FAIL_TODO == 0); // TODO
     return VisitStmt(coroutine_body_stmt, node_desc) && res;
   }
#endif

#if 0
bool ClangToDotTranslator::VisitCXXCatchStmt(clang::CXXCatchStmt * cxx_catch_stmt, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXCatchStmt" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO
    return VisitStmt(cxx_catch_stmt, node) && res;
}
#else
bool ClangToDotTranslator::VisitCXXCatchStmt(clang::CXXCatchStmt * cxx_catch_stmt, NodeDescriptor & node_desc) 
   {
#if DEBUG_VISIT_STMT
     logger[DEBUG] << "ClangToDotTranslator::VisitCXXCatchStmt" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("CXXCatchStmt");

     ROSE_ASSERT(FAIL_TODO == 0); // TODO
     return VisitStmt(cxx_catch_stmt, node_desc) && res;
   }
#endif


#if 0
bool ClangToDotTranslator::VisitCXXForRangeStmt(clang::CXXForRangeStmt * cxx_for_range_stmt, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXForRangeStmt" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO
    return VisitStmt(cxx_for_range_stmt, node) && res;
}
#else
bool ClangToDotTranslator::VisitCXXForRangeStmt(clang::CXXForRangeStmt * cxx_for_range_stmt, NodeDescriptor & node_desc)
   {
#if DEBUG_VISIT_STMT
     logger[DEBUG] << "ClangToDotTranslator::VisitCXXForRangeStmt" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("CXXForRangeStmt");

     ROSE_ASSERT(FAIL_TODO == 0); // TODO
     return VisitStmt(cxx_for_range_stmt, node_desc) && res;
   }
#endif

#if 0
bool ClangToDotTranslator::VisitCXXTryStmt(clang::CXXTryStmt * cxx_try_stmt, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXTryStmt" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO
    return VisitStmt(cxx_try_stmt, node) && res;
}
#else
bool ClangToDotTranslator::VisitCXXTryStmt(clang::CXXTryStmt * cxx_try_stmt, NodeDescriptor & node_desc)
   {
#if DEBUG_VISIT_STMT
     logger[DEBUG] << "ClangToDotTranslator::VisitCXXTryStmt" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("CXXTryStmt");

     ROSE_ASSERT(FAIL_TODO == 0); // TODO
     return VisitStmt(cxx_try_stmt, node_desc) && res;
   }
#endif

#if 0
bool ClangToDotTranslator::VisitDeclStmt(clang::DeclStmt * decl_stmt, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitDeclStmt" << "\n";
#endif

    bool res = true;

    if (decl_stmt->isSingleDecl()) {
        *node = Traverse(decl_stmt->getSingleDecl());
    }
    else {
        std::vector<SgNode *> tmp_decls;
        SgDeclarationStatement * decl;
        clang::DeclStmt::decl_iterator it;

        SgScopeStatement * scope = SageBuilder::topScopeStack();

        for (it = decl_stmt->decl_begin(); it != decl_stmt->decl_end(); it++)
            tmp_decls.push_back(Traverse(*it));
        for (unsigned i = 0; i < tmp_decls.size() - 1; i++) {
            decl = isSgDeclarationStatement(tmp_decls[i]);
            if (tmp_decls[i] != NULL && decl == NULL) {
                logger[WARN] << "Runtime error: tmp_decls[i] != NULL && decl == NULL" << "\n";
                res = false;
                continue;
            }
            else {
              SgClassDeclaration * class_decl = isSgClassDeclaration(decl);
              if (class_decl != NULL && (class_decl->get_name() == "" || class_decl->get_isUnNamed())) continue;
              SgEnumDeclaration * enum_decl = isSgEnumDeclaration(decl);
              if (enum_decl != NULL && (enum_decl->get_name() == "" || enum_decl->get_isUnNamed())) continue;
            }
            scope->append_statement(decl);
            decl->set_parent(scope);
        }
        decl = isSgDeclarationStatement(tmp_decls[tmp_decls.size() - 1]);
        if (tmp_decls[tmp_decls.size() - 1] != NULL && decl == NULL) {
            logger[WARN] << "Runtime error: tmp_decls[tmp_decls.size() - 1] != NULL && decl == NULL" << "\n";
            res = false;
        }
        *node = decl;
    }

    return res;
}
#else
bool ClangToDotTranslator::VisitDeclStmt(clang::DeclStmt * decl_stmt, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitDeclStmt" << "\n";
#endif

    bool res = true;

#if 0
    if (decl_stmt->isSingleDecl()) {
        *node = Traverse(decl_stmt->getSingleDecl());
    }
    else {
        std::vector<SgNode *> tmp_decls;
        SgDeclarationStatement * decl;
        clang::DeclStmt::decl_iterator it;

        SgScopeStatement * scope = SageBuilder::topScopeStack();

        for (it = decl_stmt->decl_begin(); it != decl_stmt->decl_end(); it++)
            tmp_decls.push_back(Traverse(*it));
        for (unsigned i = 0; i < tmp_decls.size() - 1; i++) {
            decl = isSgDeclarationStatement(tmp_decls[i]);
            if (tmp_decls[i] != NULL && decl == NULL) {
                logger[WARN] << "Runtime error: tmp_decls[i] != NULL && decl == NULL" << "\n";
                res = false;
                continue;
            }
            else {
              SgClassDeclaration * class_decl = isSgClassDeclaration(decl);
              if (class_decl != NULL && (class_decl->get_name() == "" || class_decl->get_isUnNamed())) continue;
              SgEnumDeclaration * enum_decl = isSgEnumDeclaration(decl);
              if (enum_decl != NULL && (enum_decl->get_name() == "" || enum_decl->get_isUnNamed())) continue;
            }
            scope->append_statement(decl);
            decl->set_parent(scope);
        }
        decl = isSgDeclarationStatement(tmp_decls[tmp_decls.size() - 1]);
        if (tmp_decls[tmp_decls.size() - 1] != NULL && decl == NULL) {
            logger[WARN] << "Runtime error: tmp_decls[tmp_decls.size() - 1] != NULL && decl == NULL" << "\n";
            res = false;
        }
        *node = decl;
    }
#endif

     node_desc.kind_hierarchy.push_back("DeclStmt");

     if (decl_stmt->isSingleDecl())
        {
          node_desc.successors.push_back(std::pair<std::string, std::string>("declaration[0]", Traverse(decl_stmt->getSingleDecl())));
        }
       else
        {
          clang::DeclStmt::decl_iterator it;
          unsigned cnt = 0;
          for (it = decl_stmt->decl_begin(); it != decl_stmt->decl_end(); it++)
             {
               std::ostringstream oss;
               oss << "declaration[" << cnt++ << "]";
               node_desc.successors.push_back(std::pair<std::string, std::string>(oss.str(), Traverse(*it)));
             }
        }

 // DQ (11/27/2020): I think we need to call this instead (in Tristan's code).
 // return res;
    return VisitStmt(decl_stmt, node_desc) && res;
}
#endif


#if 0
bool ClangToDotTranslator::VisitDoStmt(clang::DoStmt * do_stmt, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitDoStmt" << "\n";
#endif

    SgNode * tmp_cond = Traverse(do_stmt->getCond());
    SgExpression * cond = isSgExpression(tmp_cond);
    ROSE_ASSERT(cond != NULL);

    SgStatement * expr_stmt = SageBuilder::buildExprStatement(cond);

    ROSE_ASSERT(expr_stmt != NULL);

    SgDoWhileStmt * sg_do_stmt = SageBuilder::buildDoWhileStmt_nfi(expr_stmt, NULL);

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
#else
bool ClangToDotTranslator::VisitDoStmt(clang::DoStmt * do_stmt, NodeDescriptor & node_desc)
   {
#if DEBUG_VISIT_STMT
     logger[DEBUG] << "ClangToDotTranslator::VisitDoStmt" << "\n";
#endif

#if 0
    SgNode * tmp_cond = Traverse(do_stmt->getCond());
    SgExpression * cond = isSgExpression(tmp_cond);
    ROSE_ASSERT(cond != NULL);

    SgStatement * expr_stmt = SageBuilder::buildExprStatement(cond);

    ROSE_ASSERT(expr_stmt != NULL);

    SgDoWhileStmt * sg_do_stmt = SageBuilder::buildDoWhileStmt_nfi(expr_stmt, NULL);

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
#endif

     node_desc.kind_hierarchy.push_back("DoStmt");

     node_desc.successors.push_back(std::pair<std::string, std::string>("condition", Traverse(do_stmt->getCond())));

     node_desc.successors.push_back(std::pair<std::string, std::string>("body", Traverse(do_stmt->getBody())));

     return VisitStmt(do_stmt, node_desc); 
   }
#endif

#if 0
bool ClangToDotTranslator::VisitForStmt(clang::ForStmt * for_stmt, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitForStmt" << "\n";
#endif

    bool res = true;

    SgForStatement * sg_for_stmt = SageBuilder::buildForStatement_nfi((SgForInitStatement *)NULL, NULL, NULL, NULL);

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
        for_init_stmt = SageBuilder::buildForInitStatement_nfi(for_init_stmt_list);
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
            setCompilerGeneratedFileInfo(cond_stmt);
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
            setCompilerGeneratedFileInfo(inc);
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

    *node = sg_for_stmt;

    return VisitStmt(for_stmt, node) && res;
}
#else
bool ClangToDotTranslator::VisitForStmt(clang::ForStmt * for_stmt, NodeDescriptor & node_desc)
   {
#if DEBUG_VISIT_STMT
     logger[DEBUG] << "ClangToDotTranslator::VisitForStmt" << "\n";
#endif

     bool res = true;

#if 0
    SgForStatement * sg_for_stmt = SageBuilder::buildForStatement_nfi((SgForInitStatement *)NULL, NULL, NULL, NULL);

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
        for_init_stmt = SageBuilder::buildForInitStatement_nfi(for_init_stmt_list);
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
            setCompilerGeneratedFileInfo(cond_stmt);
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
            setCompilerGeneratedFileInfo(inc);
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

    *node = sg_for_stmt;
#endif

     node_desc.kind_hierarchy.push_back("ForStmt");

     node_desc.successors.push_back(std::pair<std::string, std::string>("init", Traverse(for_stmt->getInit())));

     node_desc.successors.push_back(std::pair<std::string, std::string>("cond", Traverse(for_stmt->getCond())));

     node_desc.successors.push_back(std::pair<std::string, std::string>("inc", Traverse(for_stmt->getInc())));

     node_desc.successors.push_back(std::pair<std::string, std::string>("body", Traverse(for_stmt->getBody())));

     return VisitStmt(for_stmt, node_desc) && res;
   }
#endif

#if 0
bool ClangToDotTranslator::VisitGotoStmt(clang::GotoStmt * goto_stmt, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitGotoStmt" << "\n";
#endif

    bool res = true;
/*
    SgSymbol * tmp_sym = GetSymbolFromSymbolTable(goto_stmt->getLabel());
    SgLabelSymbol * sym = isSgLabelSymbol(tmp_sym);
    if (sym == NULL) {
        logger[WARN] << "Runtime error: Cannot find the symbol for the label: \"" << goto_stmt->getLabel()->getStmt()->getName() << "\"." << "\n";
        res = false;
    }
    else {
        *node = SageBuilder::buildGotoStatement(sym->get_declaration());
    }
*/

    SgNode * tmp_label = Traverse(goto_stmt->getLabel()->getStmt());
    SgLabelStatement * label_stmt = isSgLabelStatement(tmp_label);
    if (label_stmt == NULL) {
        logger[WARN] << "Runtime Error: Cannot find the label: \"" << goto_stmt->getLabel()->getStmt()->getName() << "\"." << "\n";
        res = false;
    }
    else {
        *node = SageBuilder::buildGotoStatement(label_stmt);
    }

    return VisitStmt(goto_stmt, node) && res;
}
#else
bool ClangToDotTranslator::VisitGotoStmt(clang::GotoStmt * goto_stmt, NodeDescriptor & node_desc)
   {
#if DEBUG_VISIT_STMT
     logger[DEBUG] << "ClangToDotTranslator::VisitGotoStmt" << "\n";
#endif

     bool res = true;

#if 0
/*
    SgSymbol * tmp_sym = GetSymbolFromSymbolTable(goto_stmt->getLabel());
    SgLabelSymbol * sym = isSgLabelSymbol(tmp_sym);
    if (sym == NULL) {
        logger[WARN] << "Runtime error: Cannot find the symbol for the label: \"" << goto_stmt->getLabel()->getStmt()->getName() << "\"." << "\n";
        res = false;
    }
    else {
        *node = SageBuilder::buildGotoStatement(sym->get_declaration());
    }
*/

    SgNode * tmp_label = Traverse(goto_stmt->getLabel()->getStmt());
    SgLabelStatement * label_stmt = isSgLabelStatement(tmp_label);
    if (label_stmt == NULL) {
        logger[WARN] << "Runtime Error: Cannot find the label: \"" << goto_stmt->getLabel()->getStmt()->getName() << "\"." << "\n";
        res = false;
    }
    else {
        *node = SageBuilder::buildGotoStatement(label_stmt);
    }
#endif

     node_desc.kind_hierarchy.push_back("GotoStmt");

     node_desc.successors.push_back(std::pair<std::string, std::string>("label", Traverse(goto_stmt->getLabel()->getStmt())));

     return VisitStmt(goto_stmt, node_desc) && res;
   }
#endif

#if 0
bool ClangToDotTranslator::VisitIfStmt(clang::IfStmt * if_stmt, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitIfStmt" << "\n";
#endif

    bool res = true;

    // TODO if_stmt->getConditionVariable() appears when a variable is declared in the condition...

    *node = SageBuilder::buildIfStmt_nfi(NULL, NULL, NULL);

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
#else
bool ClangToDotTranslator::VisitIfStmt(clang::IfStmt * if_stmt, NodeDescriptor & node_desc)
   {
#if DEBUG_VISIT_STMT
     logger[DEBUG] << "ClangToDotTranslator::VisitIfStmt" << "\n";
#endif

     bool res = true;

    // TODO if_stmt->getConditionVariable() appears when a variable is declared in the condition...

#if 0
    *node = SageBuilder::buildIfStmt_nfi(NULL, NULL, NULL);

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
#endif

     node_desc.kind_hierarchy.push_back("IfStmt");

     node_desc.successors.push_back(std::pair<std::string, std::string>("cond", Traverse(if_stmt->getCond())));

     node_desc.successors.push_back(std::pair<std::string, std::string>("then", Traverse(if_stmt->getThen())));

     node_desc.successors.push_back(std::pair<std::string, std::string>("else", Traverse(if_stmt->getElse())));

     return VisitStmt(if_stmt, node_desc) && res;
   }
#endif

#if 0
bool ClangToDotTranslator::VisitIndirectGotoStmt(clang::IndirectGotoStmt * indirect_goto_stmt, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitIndirectGotoStmt" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

     return VisitStmt(indirect_goto_stmt, node) && res;
}
#else
bool ClangToDotTranslator::VisitIndirectGotoStmt(clang::IndirectGotoStmt * indirect_goto_stmt, NodeDescriptor & node_desc)
   {
#if DEBUG_VISIT_STMT
     logger[DEBUG] << "ClangToDotTranslator::VisitIndirectGotoStmt" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("IndirectGotoStmt");

     ROSE_ASSERT(FAIL_TODO == 0); // TODO

     return VisitStmt(indirect_goto_stmt, node_desc) && res;
   }
#endif

#if 0
bool ClangToDotTranslator::VisitMSDependentExistsStmt(clang::MSDependentExistsStmt * ms_dependent_exists_stmt, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitMSDependentExistsStmt" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

     return VisitStmt(ms_dependent_exists_stmt, node) && res;
}
#else
bool ClangToDotTranslator::VisitMSDependentExistsStmt(clang::MSDependentExistsStmt * ms_dependent_exists_stmt, NodeDescriptor & node_desc) 
   {
#if DEBUG_VISIT_STMT
     logger[DEBUG] << "ClangToDotTranslator::VisitMSDependentExistsStmt" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("MSDependentExistsStmt");

     ROSE_ASSERT(FAIL_TODO == 0); // TODO

     return VisitStmt(ms_dependent_exists_stmt, node_desc) && res;
   }
#endif

#if 0
bool ClangToDotTranslator::VisitNullStmt(clang::NullStmt * null_stmt, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitNullStmt" << "\n";
#endif
    bool res = true;

    *node = SageBuilder::buildNullStatement();

    return VisitStmt(null_stmt, node) && res;
}
#else
bool ClangToDotTranslator::VisitNullStmt(clang::NullStmt * null_stmt, NodeDescriptor & node_desc) 
   {
#if DEBUG_VISIT_STMT
     logger[DEBUG] << "ClangToDotTranslator::VisitNullStmt" << "\n";
#endif
     bool res = true;

#if 0
     *node = SageBuilder::buildNullStatement();
#endif

     node_desc.kind_hierarchy.push_back("NullStmt");

     return VisitStmt(null_stmt, node_desc) && res;
   }
#endif

#if 0
bool ClangToDotTranslator::VisitOMPExecutableDirective(clang::OMPExecutableDirective * omp_executable_directive, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitOMPExecutableDirective" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitStmt(omp_executable_directive, node) && res;
}
#else
bool ClangToDotTranslator::VisitOMPExecutableDirective(clang::OMPExecutableDirective * omp_executable_directive, NodeDescriptor & node_desc)
   {
#if DEBUG_VISIT_STMT
     logger[DEBUG] << "ClangToDotTranslator::VisitOMPExecutableDirective" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("OMPExecutableDirective");

     ROSE_ASSERT(FAIL_TODO == 0); // TODO

     return VisitStmt(omp_executable_directive, node_desc) && res;
   }
#endif

#if 0
bool ClangToDotTranslator::VisitOMPAtomicDirective(clang::OMPAtomicDirective * omp_atomic_directive, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitOMPAtomicDirective" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPExecutableDirective(omp_atomic_directive, node) && res;
}
#else
bool ClangToDotTranslator::VisitOMPAtomicDirective(clang::OMPAtomicDirective * omp_atomic_directive, NodeDescriptor & node_desc)
   {
#if DEBUG_VISIT_STMT
     logger[DEBUG] << "ClangToDotTranslator::VisitOMPAtomicDirective" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("OMPAtomicDirective");

     ROSE_ASSERT(FAIL_TODO == 0); // TODO

     return VisitOMPExecutableDirective(omp_atomic_directive, node_desc) && res;
   }
#endif

#if 0
bool ClangToDotTranslator::VisitOMPBarrierDirective(clang::OMPBarrierDirective * omp_barrier_directive, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitOMPBarrierDirective" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPExecutableDirective(omp_barrier_directive, node) && res;
}
#else
bool ClangToDotTranslator::VisitOMPBarrierDirective(clang::OMPBarrierDirective * omp_barrier_directive, NodeDescriptor & node_desc)
   {
#if DEBUG_VISIT_STMT
     logger[DEBUG] << "ClangToDotTranslator::VisitOMPBarrierDirective" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("OMPAtomicDirective");

     ROSE_ASSERT(FAIL_TODO == 0); // TODO

     return VisitOMPExecutableDirective(omp_barrier_directive, node_desc) && res;
   }
#endif

#if 0
bool ClangToDotTranslator::VisitOMPCancelDirective(clang::OMPCancelDirective * omp_cancel_directive, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitOMPCancelDirective" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPExecutableDirective(omp_cancel_directive, node) && res;
}
#else
bool ClangToDotTranslator::VisitOMPCancelDirective(clang::OMPCancelDirective * omp_cancel_directive, NodeDescriptor & node_desc)
   {
#if DEBUG_VISIT_STMT
     logger[DEBUG] << "ClangToDotTranslator::VisitOMPCancelDirective" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("OMPCancelDirective");

     ROSE_ASSERT(FAIL_TODO == 0); // TODO

     return VisitOMPExecutableDirective(omp_cancel_directive, node_desc) && res;
   }
#endif

#if 0
bool ClangToDotTranslator::VisitOMPCancellationPointDirective(clang::OMPCancellationPointDirective * omp_cancellation_point_directive, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitOMPCancellationPointDirective" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPExecutableDirective(omp_cancellation_point_directive, node) && res;
}
#else
bool ClangToDotTranslator::VisitOMPCancellationPointDirective(clang::OMPCancellationPointDirective * omp_cancellation_point_directive, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitOMPCancellationPointDirective" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("OMPCancellationPointDirective");

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPExecutableDirective(omp_cancellation_point_directive, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitOMPCriticalDirective(clang::OMPCriticalDirective * omp_critical_directive, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitOMPCriticalDirective" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPExecutableDirective(omp_critical_directive, node) && res;
}
#else
bool ClangToDotTranslator::VisitOMPCriticalDirective(clang::OMPCriticalDirective * omp_critical_directive, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitOMPCriticalDirective" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("OMPCriticalDirective");

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPExecutableDirective(omp_critical_directive, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitOMPFlushDirective(clang::OMPFlushDirective * omp_flush_directive, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitOMPFlushDirective" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPExecutableDirective(omp_flush_directive, node) && res;
}
#else
bool ClangToDotTranslator::VisitOMPFlushDirective(clang::OMPFlushDirective * omp_flush_directive, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitOMPFlushDirective" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("OMPFlushDirective");

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPExecutableDirective(omp_flush_directive, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitOMPLoopDirective(clang::OMPLoopDirective * omp_loop_directive, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitOMPLoopDirective" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPExecutableDirective(omp_loop_directive, node) && res;
}
#else
bool ClangToDotTranslator::VisitOMPLoopDirective(clang::OMPLoopDirective * omp_loop_directive, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitOMPLoopDirective" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("OMPLoopDirective");

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPExecutableDirective(omp_loop_directive, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitOMPDistributeDirective(clang::OMPDistributeDirective * omp_distribute_directive, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitOMPDistributeDirective" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPLoopDirective(omp_distribute_directive, node) && res;
}
#else
bool ClangToDotTranslator::VisitOMPDistributeDirective(clang::OMPDistributeDirective * omp_distribute_directive, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitOMPDistributeDirective" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("OMPDistributeDirective");

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPLoopDirective(omp_distribute_directive, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitOMPDistributeParallelForDirective(clang::OMPDistributeParallelForDirective * omp_distribute_parallel_for_directive, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitOMPDistributeParallelForDirective" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPExecutableDirective(omp_distribute_parallel_for_directive, node) && res;
}
#else
bool ClangToDotTranslator::VisitOMPDistributeParallelForDirective(clang::OMPDistributeParallelForDirective * omp_distribute_parallel_for_directive, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitOMPDistributeParallelForDirective" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("OMPDistributeParallelForDirective");

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPExecutableDirective(omp_distribute_parallel_for_directive, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitOMPDistributeParallelForSimdDirective(clang::OMPDistributeParallelForSimdDirective * omp_distribute_parallel_for_simd_directive, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitOMPDistributeParallelForSimdDirective" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPExecutableDirective(omp_distribute_parallel_for_simd_directive, node) && res;
}
#else
bool ClangToDotTranslator::VisitOMPDistributeParallelForSimdDirective(clang::OMPDistributeParallelForSimdDirective * omp_distribute_parallel_for_simd_directive, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitOMPDistributeParallelForSimdDirective" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("OMPDistributeParallelForSimdDirective");

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPExecutableDirective(omp_distribute_parallel_for_simd_directive, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitOMPDistributeSimdDirective(clang::OMPDistributeSimdDirective * omp_distribute__simd_directive, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitOMPDistributeSimdDirective" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPExecutableDirective(omp_distribute__simd_directive, node) && res;
}
#else
bool ClangToDotTranslator::VisitOMPDistributeSimdDirective(clang::OMPDistributeSimdDirective * omp_distribute__simd_directive, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitOMPDistributeSimdDirective" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("OMPDistributeSimdDirective");

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPExecutableDirective(omp_distribute__simd_directive, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitOMPForDirective(clang::OMPForDirective * omp_for_directive, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitOMPForDirective" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPLoopDirective(omp_for_directive, node) && res;
}
#else
bool ClangToDotTranslator::VisitOMPForDirective(clang::OMPForDirective * omp_for_directive, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitOMPForDirective" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("OMPForDirective");

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPLoopDirective(omp_for_directive, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitOMPForSimdDirective(clang::OMPForSimdDirective * omp_for_simd_directive, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitOMPForSimdDirective" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPLoopDirective(omp_for_simd_directive, node) && res;
}
#else
bool ClangToDotTranslator::VisitOMPForSimdDirective(clang::OMPForSimdDirective * omp_for_simd_directive, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitOMPForSimdDirective" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("OMPForSimdDirective");

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPLoopDirective(omp_for_simd_directive, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitOMPParallelForDirective(clang::OMPParallelForDirective * omp_parallel_for_directive, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitOMPParallelForDirective" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPLoopDirective(omp_parallel_for_directive, node) && res;
}
#else
bool ClangToDotTranslator::VisitOMPParallelForDirective(clang::OMPParallelForDirective * omp_parallel_for_directive, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitOMPParallelForDirective" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("OMPParallelForDirective");

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPLoopDirective(omp_parallel_for_directive, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitOMPParallelForSimdDirective(clang::OMPParallelForSimdDirective * omp_parallel_for_simd_directive, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitOMPParallelForSimdDirective" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPLoopDirective(omp_parallel_for_simd_directive, node) && res;
}
#else
bool ClangToDotTranslator::VisitOMPParallelForSimdDirective(clang::OMPParallelForSimdDirective * omp_parallel_for_simd_directive, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitOMPParallelForSimdDirective" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("OMPParallelForSimdDirective");

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPLoopDirective(omp_parallel_for_simd_directive, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitOMPSimdDirective(clang::OMPSimdDirective * omp_simd_directive, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitOMPSimdDirective" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPLoopDirective(omp_simd_directive, node) && res;
}
#else
bool ClangToDotTranslator::VisitOMPSimdDirective(clang::OMPSimdDirective * omp_simd_directive, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitOMPSimdDirective" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("OMPSimdDirective");

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPLoopDirective(omp_simd_directive, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitOMPTargetParallelForDirective(clang::OMPTargetParallelForDirective * omp_target_parallel_for_directive, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitOMPTargetParallelForDirective" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPLoopDirective(omp_target_parallel_for_directive, node) && res;
}
#else
bool ClangToDotTranslator::VisitOMPTargetParallelForDirective(clang::OMPTargetParallelForDirective * omp_target_parallel_for_directive, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitOMPTargetParallelForDirective" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("OMPTargetParallelForDirective");

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPLoopDirective(omp_target_parallel_for_directive, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitOMPTargetParallelForSimdDirective(clang::OMPTargetParallelForSimdDirective * omp_target_parallel_for_simd_directive, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitOMPTargetParallelForSimdDirective" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPLoopDirective(omp_target_parallel_for_simd_directive, node) && res;
}
#else
bool ClangToDotTranslator::VisitOMPTargetParallelForSimdDirective(clang::OMPTargetParallelForSimdDirective * omp_target_parallel_for_simd_directive, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitOMPTargetParallelForSimdDirective" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("OMPTargetParallelForSimdDirective");

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPLoopDirective(omp_target_parallel_for_simd_directive, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitOMPTargetSimdDirective(clang::OMPTargetSimdDirective * omp_target_simd_directive, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitOMPTargetSimdDirective" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPLoopDirective(omp_target_simd_directive, node) && res;
}
#else
bool ClangToDotTranslator::VisitOMPTargetSimdDirective(clang::OMPTargetSimdDirective * omp_target_simd_directive, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitOMPTargetSimdDirective" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("OMPTargetSimdDirective");

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPLoopDirective(omp_target_simd_directive, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitOMPTargetTeamsDistributeDirective(clang::OMPTargetTeamsDistributeDirective * omp_target_teams_distribute_directive, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitOMPTargetTeamsDistributeDirective" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPLoopDirective(omp_target_teams_distribute_directive, node) && res;
}
#else
bool ClangToDotTranslator::VisitOMPTargetTeamsDistributeDirective(clang::OMPTargetTeamsDistributeDirective * omp_target_teams_distribute_directive, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitOMPTargetTeamsDistributeDirective" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("OMPTargetTeamsDistributeDirective");

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPLoopDirective(omp_target_teams_distribute_directive, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitOMPTargetTeamsDistributeSimdDirective(clang::OMPTargetTeamsDistributeSimdDirective * omp_target_teams_distribute_simd_directive, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitOMPTargetTeamsDistributeSimdDirective" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPLoopDirective(omp_target_teams_distribute_simd_directive, node) && res;
}
#else
bool ClangToDotTranslator::VisitOMPTargetTeamsDistributeSimdDirective(clang::OMPTargetTeamsDistributeSimdDirective * omp_target_teams_distribute_simd_directive, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitOMPTargetTeamsDistributeSimdDirective" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("OMPTargetTeamsDistributeSimdDirective");

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPLoopDirective(omp_target_teams_distribute_simd_directive, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitOMPTaskLoopDirective(clang::OMPTaskLoopDirective * omp_task_loop_directive, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitOMPTaskLoopDirective" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPLoopDirective(omp_task_loop_directive, node) && res;
}
#else
bool ClangToDotTranslator::VisitOMPTaskLoopDirective(clang::OMPTaskLoopDirective * omp_task_loop_directive, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitOMPTaskLoopDirective" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("OMPTaskLoopDirective");

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPLoopDirective(omp_task_loop_directive, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitOMPTaskLoopSimdDirective(clang::OMPTaskLoopSimdDirective * omp_task_loop_simd_directive, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitOMPTaskLoopSimdDirective" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPLoopDirective(omp_task_loop_simd_directive, node) && res;
}
#else
bool ClangToDotTranslator::VisitOMPTaskLoopSimdDirective(clang::OMPTaskLoopSimdDirective * omp_task_loop_simd_directive, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitOMPTaskLoopSimdDirective" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("OMPTaskLoopSimdDirective");

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPLoopDirective(omp_task_loop_simd_directive, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitOMPMasterDirective(clang::OMPMasterDirective * omp_master_directive, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitOMPMasterDirective" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPExecutableDirective(omp_master_directive, node) && res;
}
#else
bool ClangToDotTranslator::VisitOMPMasterDirective(clang::OMPMasterDirective * omp_master_directive, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitOMPMasterDirective" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("OMPMasterDirective");

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPExecutableDirective(omp_master_directive, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitOMPOrderedDirective(clang::OMPOrderedDirective * omp_ordered_directive, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitOMPOrderedDirective" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPExecutableDirective(omp_ordered_directive, node) && res;
}
#else
bool ClangToDotTranslator::VisitOMPOrderedDirective(clang::OMPOrderedDirective * omp_ordered_directive, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitOMPOrderedDirective" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("OMPOrderedDirective");

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPExecutableDirective(omp_ordered_directive, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitOMPParallelDirective(clang::OMPParallelDirective * omp_parallel_directive, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitOMPParallelDirective" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPExecutableDirective(omp_parallel_directive, node) && res;
}
#else
bool ClangToDotTranslator::VisitOMPParallelDirective(clang::OMPParallelDirective * omp_parallel_directive, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitOMPParallelDirective" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("OMPParallelDirective");

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPExecutableDirective(omp_parallel_directive, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitOMPParallelSectionsDirective(clang::OMPParallelSectionsDirective * omp_parallel_sections_directive, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitOMPParallelSectionsDirective" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPExecutableDirective(omp_parallel_sections_directive, node) && res;
}
#else
bool ClangToDotTranslator::VisitOMPParallelSectionsDirective(clang::OMPParallelSectionsDirective * omp_parallel_sections_directive, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitOMPParallelSectionsDirective" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("OMPParallelSectionsDirective");

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitOMPExecutableDirective(omp_parallel_sections_directive, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitReturnStmt(clang::ReturnStmt * return_stmt, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitReturnStmt" << "\n";
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
#else
bool ClangToDotTranslator::VisitReturnStmt(clang::ReturnStmt * return_stmt, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitReturnStmt" << "\n";
#endif

    bool res = true;

#if 0
    SgNode * tmp_expr = Traverse(return_stmt->getRetValue());
    SgExpression * expr = isSgExpression(tmp_expr);
    if (tmp_expr != NULL && expr == NULL) {
        logger[WARN] << "Runtime error: tmp_expr != NULL && expr == NULL" << "\n";
        res = false;
    }
    *node = SageBuilder::buildReturnStmt(expr);
#endif

     node_desc.kind_hierarchy.push_back("ReturnStmt");

     node_desc.successors.push_back(std::pair<std::string, std::string>("return_value", Traverse(return_stmt->getRetValue())));

    return VisitStmt(return_stmt, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitSEHExceptStmt(clang::SEHExceptStmt * seh_except_stmt, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitSEHExceptStmt" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitStmt(seh_except_stmt, node) && res;
}
#else
bool ClangToDotTranslator::VisitSEHExceptStmt(clang::SEHExceptStmt * seh_except_stmt, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitSEHExceptStmt" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("SEHExceptStmt");

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitStmt(seh_except_stmt, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitSEHFinallyStmt(clang::SEHFinallyStmt * seh_finally_stmt, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitSEHFinallyStmt" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitStmt(seh_finally_stmt, node) && res;
}
#else
bool ClangToDotTranslator::VisitSEHFinallyStmt(clang::SEHFinallyStmt * seh_finally_stmt, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitSEHFinallyStmt" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("SEHFinallyStmt");

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitStmt(seh_finally_stmt, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitSEHLeaveStmt(clang::SEHLeaveStmt * seh_leave_stmt, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitSEHLeaveStmt" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitStmt(seh_leave_stmt, node) && res;
}
#else
bool ClangToDotTranslator::VisitSEHLeaveStmt(clang::SEHLeaveStmt * seh_leave_stmt, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitSEHLeaveStmt" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("SEHLeaveStmt");

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitStmt(seh_leave_stmt, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitSEHTryStmt(clang::SEHTryStmt * seh_try_stmt, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitSEHTryStmt" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitStmt(seh_try_stmt, node) && res;
}
#else
bool ClangToDotTranslator::VisitSEHTryStmt(clang::SEHTryStmt * seh_try_stmt, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitSEHTryStmt" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("SEHTryStmt");

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitStmt(seh_try_stmt, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitSwitchCase(clang::SwitchCase * switch_case, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitSwitchCase" << "\n";
#endif
    bool res = true;
    
    // TODO

    return VisitStmt(switch_case, node) && res;
}
#else
bool ClangToDotTranslator::VisitSwitchCase(clang::SwitchCase * switch_case, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitSwitchCase" << "\n";
#endif
    bool res = true;
    
     node_desc.kind_hierarchy.push_back("SwitchStmt");

    // TODO

     node_desc.successors.push_back(std::pair<std::string, std::string>("sub_stmt", Traverse(switch_case->getSubStmt())));

    return VisitStmt(switch_case, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitCaseStmt(clang::CaseStmt * case_stmt, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCaseStmt" << "\n";
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

/*  FIXME GNU extension not-handled by ROSE
    SgNode * tmp_rhs = Traverse(case_stmt->getRHS());
    SgExpression * rhs = isSgExpression(tmp_rhs);
    ROSE_ASSERT(rhs != NULL);
*/
    ROSE_ASSERT(case_stmt->getRHS() == NULL);

    *node = SageBuilder::buildCaseOptionStmt_nfi(lhs, stmt);

    return VisitSwitchCase(case_stmt, node);
}
#else
bool ClangToDotTranslator::VisitCaseStmt(clang::CaseStmt * case_stmt, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCaseStmt" << "\n";
#endif

#if 0
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

/*  FIXME GNU extension not-handled by ROSE
    SgNode * tmp_rhs = Traverse(case_stmt->getRHS());
    SgExpression * rhs = isSgExpression(tmp_rhs);
    ROSE_ASSERT(rhs != NULL);
*/
    ROSE_ASSERT(case_stmt->getRHS() == NULL);

    *node = SageBuilder::buildCaseOptionStmt_nfi(lhs, stmt);
#endif

     node_desc.kind_hierarchy.push_back("CaseStmt");

     node_desc.successors.push_back(std::pair<std::string, std::string>("lhs", Traverse(case_stmt->getLHS())));

     node_desc.successors.push_back(std::pair<std::string, std::string>("rhs", Traverse(case_stmt->getRHS())));

    return VisitSwitchCase(case_stmt, node_desc);
}
#endif

#if 0
bool ClangToDotTranslator::VisitDefaultStmt(clang::DefaultStmt * default_stmt, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitDefaultStmt" << "\n";
#endif

    SgNode * tmp_stmt = Traverse(default_stmt->getSubStmt());
    SgStatement * stmt = isSgStatement(tmp_stmt);

    *node = SageBuilder::buildDefaultOptionStmt_nfi(stmt);

    return VisitSwitchCase(default_stmt, node);
}
#else
bool ClangToDotTranslator::VisitDefaultStmt(clang::DefaultStmt * default_stmt, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitDefaultStmt" << "\n";
#endif

#if 0
    SgNode * tmp_stmt = Traverse(default_stmt->getSubStmt());
    SgStatement * stmt = isSgStatement(tmp_stmt);

    *node = SageBuilder::buildDefaultOptionStmt_nfi(stmt);
#endif

     node_desc.kind_hierarchy.push_back("DefaultStmt");

    return VisitSwitchCase(default_stmt, node_desc);
}
#endif

#if 0
bool ClangToDotTranslator::VisitSwitchStmt(clang::SwitchStmt * switch_stmt, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitSwitchStmt" << "\n";
#endif

    SgNode * tmp_cond = Traverse(switch_stmt->getCond());
    SgExpression * cond = isSgExpression(tmp_cond);
    ROSE_ASSERT(cond != NULL);
    
    SgStatement * expr_stmt = SageBuilder::buildExprStatement(cond);
        applySourceRange(expr_stmt, switch_stmt->getCond()->getSourceRange());

    SgSwitchStatement * sg_switch_stmt = SageBuilder::buildSwitchStatement_nfi(expr_stmt, NULL);

    cond->set_parent(expr_stmt);
    expr_stmt->set_parent(sg_switch_stmt);

    SageBuilder::pushScopeStack(sg_switch_stmt);

    SgNode * tmp_body = Traverse(switch_stmt->getBody());
    SgStatement * body = isSgStatement(tmp_body);
    ROSE_ASSERT(body != NULL);

    SageBuilder::popScopeStack();

    sg_switch_stmt->set_body(body);

    *node = sg_switch_stmt;

    return VisitStmt(switch_stmt, node);
}
#else
bool ClangToDotTranslator::VisitSwitchStmt(clang::SwitchStmt * switch_stmt, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitSwitchStmt" << "\n";
#endif

#if 0
    SgNode * tmp_cond = Traverse(switch_stmt->getCond());
    SgExpression * cond = isSgExpression(tmp_cond);
    ROSE_ASSERT(cond != NULL);
    
    SgStatement * expr_stmt = SageBuilder::buildExprStatement(cond);
        applySourceRange(expr_stmt, switch_stmt->getCond()->getSourceRange());

    SgSwitchStatement * sg_switch_stmt = SageBuilder::buildSwitchStatement_nfi(expr_stmt, NULL);

    cond->set_parent(expr_stmt);
    expr_stmt->set_parent(sg_switch_stmt);

    SageBuilder::pushScopeStack(sg_switch_stmt);

    SgNode * tmp_body = Traverse(switch_stmt->getBody());
    SgStatement * body = isSgStatement(tmp_body);
    ROSE_ASSERT(body != NULL);

    SageBuilder::popScopeStack();

    sg_switch_stmt->set_body(body);

    *node = sg_switch_stmt;
#endif

     node_desc.kind_hierarchy.push_back("SwitchStmt");

     node_desc.successors.push_back(std::pair<std::string, std::string>("cond", Traverse(switch_stmt->getCond())));
    
     node_desc.successors.push_back(std::pair<std::string, std::string>("body", Traverse(switch_stmt->getBody())));

    return VisitStmt(switch_stmt, node_desc);
}
#endif

#if 0
bool ClangToDotTranslator::VisitValueStmt(clang::ValueStmt * value_stmt, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitValueStmt" << "\n";
#endif
    bool res = true;

    //ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitStmt(value_stmt, node) && res;
}
#else
bool ClangToDotTranslator::VisitValueStmt(clang::ValueStmt * value_stmt, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitValueStmt" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("ValueStmt");

    //ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitStmt(value_stmt, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitAttributedStmt(clang::AttributedStmt * attributed_stmt, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitAttributedStmt" << "\n";
#endif
    bool res = true;

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitValueStmt(attributed_stmt, node) && res;
}
#else
bool ClangToDotTranslator::VisitAttributedStmt(clang::AttributedStmt * attributed_stmt, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitAttributedStmt" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("AttributedStmt");

    ROSE_ASSERT(FAIL_TODO == 0); // TODO

    return VisitValueStmt(attributed_stmt, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitExpr(clang::Expr * expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitExpr" << "\n";
#endif

     // TODO Is there anything to be done? (maybe in relation with typing?)

     return VisitValueStmt(expr, node);
}
#else
bool ClangToDotTranslator::VisitExpr(clang::Expr * expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitExpr" << "\n";
#endif

     node_desc.kind_hierarchy.push_back("Expr");

     // TODO Is there anything to be done? (maybe in relation with typing?)

     node_desc.successors.push_back(std::pair<std::string, std::string>("type", Traverse(expr->getType().getTypePtr())));

     return VisitValueStmt(expr, node_desc);
}
#endif

#if 0
bool ClangToDotTranslator::VisitAbstractConditionalOperator(clang::AbstractConditionalOperator * abstract_conditional_operator, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitAbstractConditionalOperator" << "\n";
#endif
     bool res = true;

     // TODO 

     return VisitStmt(abstract_conditional_operator, node) && res;
}
#else
bool ClangToDotTranslator::VisitAbstractConditionalOperator(clang::AbstractConditionalOperator * abstract_conditional_operator, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitAbstractConditionalOperator" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("AbstractConditionalOperator");

     node_desc.successors.push_back(std::pair<std::string, std::string>("condition", Traverse(abstract_conditional_operator->getCond())));

     node_desc.successors.push_back(std::pair<std::string, std::string>("true_stmt", Traverse(abstract_conditional_operator->getTrueExpr())));

     node_desc.successors.push_back(std::pair<std::string, std::string>("false_stmt", Traverse(abstract_conditional_operator->getFalseExpr())));

     // TODO 

     return VisitStmt(abstract_conditional_operator, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitBinaryConditionalOperator(clang::BinaryConditionalOperator * binary_conditional_operator, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitBinaryConditionalOperator" << "\n";
#endif
     bool res = true;

     // TODO 

     return VisitStmt(binary_conditional_operator, node) && res;
}
#else
bool ClangToDotTranslator::VisitBinaryConditionalOperator(clang::BinaryConditionalOperator * binary_conditional_operator, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitBinaryConditionalOperator" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("BinaryConditionalOperator");

     // TODO 

     return VisitStmt(binary_conditional_operator, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitConditionalOperator(clang::ConditionalOperator * conditional_operator, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitConditionalOperator" << "\n";
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
#else
bool ClangToDotTranslator::VisitConditionalOperator(clang::ConditionalOperator * conditional_operator, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitConditionalOperator" << "\n";
#endif

    bool res = true;

#if 0
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
#endif

     node_desc.kind_hierarchy.push_back("ConditionalOperator");

    return VisitAbstractConditionalOperator(conditional_operator, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitAddrLabelExpr(clang::AddrLabelExpr * addr_label_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitAddrLabelExpr" << "\n";
#endif
     bool res = true;

     // TODO 

     return VisitExpr(addr_label_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitAddrLabelExpr(clang::AddrLabelExpr * addr_label_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitAddrLabelExpr" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("AddrLabelExpr");

     // TODO 

     return VisitExpr(addr_label_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitArrayInitIndexExpr(clang::ArrayInitIndexExpr * array_init_index_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitArrayInitIndexExpr" << "\n";
#endif
     bool res = true;

     // TODO 

     return VisitExpr(array_init_index_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitArrayInitIndexExpr(clang::ArrayInitIndexExpr * array_init_index_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitArrayInitIndexExpr" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("ArrayInitIndexExpr");

     // TODO 

     return VisitExpr(array_init_index_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitArrayInitLoopExpr(clang::ArrayInitLoopExpr * array_init_loop_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitArrayInitLoopExpr" << "\n";
#endif
     bool res = true;

     // TODO 

     return VisitExpr(array_init_loop_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitArrayInitLoopExpr(clang::ArrayInitLoopExpr * array_init_loop_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitArrayInitLoopExpr" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("ArrayInitLoopExpr");

     // TODO 

     return VisitExpr(array_init_loop_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitArraySubscriptExpr(clang::ArraySubscriptExpr * array_subscript_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitArraySubscriptExpr" << "\n";
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
#else
bool ClangToDotTranslator::VisitArraySubscriptExpr(clang::ArraySubscriptExpr * array_subscript_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitArraySubscriptExpr" << "\n";
#endif

    bool res = true;

#if 0
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
#endif

     node_desc.kind_hierarchy.push_back("ArraySubscriptExpr");

     node_desc.successors.push_back(std::pair<std::string, std::string>("base", Traverse(array_subscript_expr->getBase())));

     node_desc.successors.push_back(std::pair<std::string, std::string>("index", Traverse(array_subscript_expr->getIdx())));

    return VisitExpr(array_subscript_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitArrayTypeTraitExpr(clang::ArrayTypeTraitExpr * array_type_trait_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitArrayTypeTraitExpr" << "\n";
#endif
     bool res = true;

     // TODO 

     return VisitExpr(array_type_trait_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitArrayTypeTraitExpr(clang::ArrayTypeTraitExpr * array_type_trait_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitArrayTypeTraitExpr" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("ArrayTypeTraitExpr");

     // TODO 

     return VisitExpr(array_type_trait_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitAsTypeExpr(clang::AsTypeExpr * as_type_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitAsTypeExpr" << "\n";
#endif
     bool res = true;

     // TODO 

     return VisitExpr(as_type_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitAsTypeExpr(clang::AsTypeExpr * as_type_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitAsTypeExpr" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("AsTypeExpr");

     // TODO 

     return VisitExpr(as_type_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitAtomicExpr(clang::AtomicExpr * atomic_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitAtomicExpr" << "\n";
#endif
     bool res = true;

     // TODO 

     return VisitExpr(atomic_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitAtomicExpr(clang::AtomicExpr * atomic_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitAtomicExpr" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("AtomicExpr");

     // TODO 

     return VisitExpr(atomic_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitBinaryOperator(clang::BinaryOperator * binary_operator, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitBinaryOperator" << "\n";
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
#else
bool ClangToDotTranslator::VisitBinaryOperator(clang::BinaryOperator * binary_operator, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitBinaryOperator" << "\n";
#endif

    bool res = true;

#if 0
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
#endif

     node_desc.kind_hierarchy.push_back("BinaryOperator");

     node_desc.successors.push_back(std::pair<std::string, std::string>("lhs", Traverse(binary_operator->getLHS())));

     node_desc.successors.push_back(std::pair<std::string, std::string>("rhs", Traverse(binary_operator->getRHS())));

     node_desc.attributes.push_back(std::pair<std::string, std::string>("opcode", binary_operator->getOpcodeStr()));

     return VisitExpr(binary_operator, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitCompoundAssignOperator(clang::CompoundAssignOperator * compound_assign_operator, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCompoundAssignOperator" << "\n";
#endif
     bool res = true;

     // TODO 

     return VisitBinaryOperator(compound_assign_operator, node) && res;
}
#else
bool ClangToDotTranslator::VisitCompoundAssignOperator(clang::CompoundAssignOperator * compound_assign_operator, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCompoundAssignOperator" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("CompoundAssignOperator");

     node_desc.successors.push_back(std::pair<std::string, std::string>(
        "computation_lhs_type", Traverse(compound_assign_operator->getComputationLHSType().getTypePtr())
     ));

     node_desc.successors.push_back(std::pair<std::string, std::string>(
        "computation_result_type", Traverse(compound_assign_operator->getComputationResultType().getTypePtr())
     ));

     // TODO 

     return VisitBinaryOperator(compound_assign_operator, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitBlockExpr(clang::BlockExpr * block_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitBlockExpr" << "\n";
#endif
     bool res = true;

     // TODO 

     return VisitExpr(block_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitBlockExpr(clang::BlockExpr * block_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitBlockExpr" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("BlockExpr");

     // TODO 

     return VisitExpr(block_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitCallExpr(clang::CallExpr * call_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCallExpr" << "\n";
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
#else
bool ClangToDotTranslator::VisitCallExpr(clang::CallExpr * call_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCallExpr" << "\n";
#endif

    bool res = true;

#if 0
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
#endif

     node_desc.kind_hierarchy.push_back("CallExpr");

     node_desc.successors.push_back(std::pair<std::string, std::string>("callee", Traverse(call_expr->getCallee())));

     node_desc.successors.push_back(std::pair<std::string, std::string>("callee_decl", Traverse(call_expr->getCalleeDecl())));

     node_desc.successors.push_back(std::pair<std::string, std::string>("direct_callee", Traverse(call_expr->getDirectCallee())));

     clang::CallExpr::arg_iterator it;
     unsigned cnt = 0;
     for (it = call_expr->arg_begin(); it != call_expr->arg_end(); ++it)
        {
          std::ostringstream oss;
          oss << "argument[" << cnt++ << "]";
          node_desc.successors.push_back(std::pair<std::string, std::string>(oss.str(), Traverse(*it)));
        }

     return VisitExpr(call_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitCUDAKernelCallExpr(clang::CUDAKernelCallExpr * cuda_kernel_call_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCUDAKernelCallExpr" << "\n";
#endif
     bool res = true;

     // TODO 

     return VisitExpr(cuda_kernel_call_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitCUDAKernelCallExpr(clang::CUDAKernelCallExpr * cuda_kernel_call_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCUDAKernelCallExpr" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("CUDAKernelCallExpr");

     // TODO 

     return VisitExpr(cuda_kernel_call_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitCXXMemberCallExpr(clang::CXXMemberCallExpr * cxx_member_call_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXMemberCallExpr" << "\n";
#endif
     bool res = true;

     // TODO 

     return VisitExpr(cxx_member_call_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitCXXMemberCallExpr(clang::CXXMemberCallExpr * cxx_member_call_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXMemberCallExpr" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("CXXMemberCallExpr");

     // TODO 

     return VisitExpr(cxx_member_call_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitCXXOperatorCallExpr(clang::CXXOperatorCallExpr * cxx_operator_call_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXOperatorCallExpr" << "\n";
#endif
     bool res = true;

     // TODO 

     return VisitExpr(cxx_operator_call_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitCXXOperatorCallExpr(clang::CXXOperatorCallExpr * cxx_operator_call_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXOperatorCallExpr" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("CXXOperatorCallExpr");

     // TODO 

     return VisitExpr(cxx_operator_call_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitUserDefinedLiteral(clang::UserDefinedLiteral * user_defined_literal, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitUserDefinedLiteral" << "\n";
#endif
     bool res = true;

     // TODO 

     return VisitExpr(user_defined_literal, node) && res;
}
#else
bool ClangToDotTranslator::VisitUserDefinedLiteral(clang::UserDefinedLiteral * user_defined_literal, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitUserDefinedLiteral" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("UserDefinedLiteral");

     // TODO 

     return VisitExpr(user_defined_literal, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitCastExpr(clang::CastExpr * cast_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCastExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(cast_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitCastExpr(clang::CastExpr * cast_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCastExpr" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("CastExpr");

     node_desc.successors.push_back(std::pair<std::string, std::string>("sub_expr", Traverse(cast_expr->getSubExpr())));

    // TODO

    return VisitExpr(cast_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitExplicitCastExpr(clang::ExplicitCastExpr * explicit_cast_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitExplicitCastExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitCastExpr(explicit_cast_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitExplicitCastExpr(clang::ExplicitCastExpr * explicit_cast_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
     logger[DEBUG] << "ClangToDotTranslator::VisitExplicitCastExpr" << "\n";
#endif
     bool res = true;

     node_desc.kind_hierarchy.push_back("ExplicitCastExpr");

     node_desc.successors.push_back(std::pair<std::string, std::string>("type_as_written", Traverse(explicit_cast_expr->getTypeAsWritten().getTypePtr())));

 // TODO

     return VisitCastExpr(explicit_cast_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitBuiltinBitCastExpr(clang::BuiltinBitCastExpr * builtin_bit_cast_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
     logger[DEBUG] << "ClangToDotTranslator::VisitBuiltinBitCastExpr" << "\n";
#endif
     bool res = true;

  // TODO

     return VisitExplicitCastExpr(builtin_bit_cast_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitBuiltinBitCastExpr(clang::BuiltinBitCastExpr * builtin_bit_cast_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitBuiltinBitCastExpr" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("BuiltinBitCastExpr");

    // TODO

    return VisitExplicitCastExpr(builtin_bit_cast_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitCStyleCastExpr(clang::CStyleCastExpr * c_style_cast, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCStyleCastExpr" << "\n";
#endif

    bool res = true;

    SgNode * tmp_expr = Traverse(c_style_cast->getSubExpr());
    SgExpression * expr = isSgExpression(tmp_expr);

    ROSE_ASSERT(expr);

    SgType * type = buildTypeFromQualifiedType(c_style_cast->getTypeAsWritten());

    *node = SageBuilder::buildCastExp(expr, type, SgCastExp::e_C_style_cast);

    return VisitExplicitCastExpr(c_style_cast, node) && res;
}
#else
bool ClangToDotTranslator::VisitCStyleCastExpr(clang::CStyleCastExpr * c_style_cast, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCStyleCastExpr" << "\n";
#endif

    bool res = true;

#if 0
    SgNode * tmp_expr = Traverse(c_style_cast->getSubExpr());
    SgExpression * expr = isSgExpression(tmp_expr);

    ROSE_ASSERT(expr);

    SgType * type = buildTypeFromQualifiedType(c_style_cast->getTypeAsWritten());

    *node = SageBuilder::buildCastExp(expr, type, SgCastExp::e_C_style_cast);
#endif

     node_desc.kind_hierarchy.push_back("CStyleCastExpr");

    return VisitExplicitCastExpr(c_style_cast, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitCXXFunctionalCastExpr(clang::CXXFunctionalCastExpr * cxx_functional_cast_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXFunctionalCastExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExplicitCastExpr(cxx_functional_cast_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitCXXFunctionalCastExpr(clang::CXXFunctionalCastExpr * cxx_functional_cast_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXFunctionalCastExpr" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("CXXFunctionalCastExpr");

    // TODO

    return VisitExplicitCastExpr(cxx_functional_cast_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitCXXNamedCastExpr(clang::CXXNamedCastExpr * cxx_named_cast_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXNamedCastExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExplicitCastExpr(cxx_named_cast_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitCXXNamedCastExpr(clang::CXXNamedCastExpr * cxx_named_cast_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXNamedCastExpr" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("CXXNamedCastExpr");

    // TODO

    return VisitExplicitCastExpr(cxx_named_cast_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitCXXConstCastExpr(clang::CXXConstCastExpr * cxx_const_cast_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXConstCastExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitCXXNamedCastExpr(cxx_const_cast_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitCXXConstCastExpr(clang::CXXConstCastExpr * cxx_const_cast_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXConstCastExpr" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("CXXConstCastExpr");

    // TODO

    return VisitCXXNamedCastExpr(cxx_const_cast_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitCXXDynamicCastExpr(clang::CXXDynamicCastExpr * cxx_dynamic_cast_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXDynamicCastExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitCXXNamedCastExpr(cxx_dynamic_cast_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitCXXDynamicCastExpr(clang::CXXDynamicCastExpr * cxx_dynamic_cast_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXDynamicCastExpr" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("CXXDynamicCastExpr");

    // TODO

    return VisitCXXNamedCastExpr(cxx_dynamic_cast_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitCXXReinterpretCastExpr(clang::CXXReinterpretCastExpr * cxx_reinterpret_cast_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXReinterpretCastExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitCXXNamedCastExpr(cxx_reinterpret_cast_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitCXXReinterpretCastExpr(clang::CXXReinterpretCastExpr * cxx_reinterpret_cast_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXReinterpretCastExpr" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("CXXReinterpretCastExpr");

    // TODO

    return VisitCXXNamedCastExpr(cxx_reinterpret_cast_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitCXXStaticCastExpr(clang::CXXStaticCastExpr * cxx_static_cast_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXStaticCastExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitCXXNamedCastExpr(cxx_static_cast_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitCXXStaticCastExpr(clang::CXXStaticCastExpr * cxx_static_cast_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXStaticCastExpr" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("CXXStaticCastExpr");

    // TODO

    return VisitCXXNamedCastExpr(cxx_static_cast_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitImplicitCastExpr(clang::ImplicitCastExpr * implicit_cast_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitImplicitCastExpr" << "\n";
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
#else
bool ClangToDotTranslator::VisitImplicitCastExpr(clang::ImplicitCastExpr * implicit_cast_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitImplicitCastExpr" << "\n";
#endif

#if 0
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
#endif

     node_desc.kind_hierarchy.push_back("ImplicitCastExpr");

    return VisitCastExpr(implicit_cast_expr, node_desc);
}
#endif

#if 0
bool ClangToDotTranslator::VisitCharacterLiteral(clang::CharacterLiteral * character_literal, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCharacterLiteral" << "\n";
#endif

    *node = SageBuilder::buildCharVal(character_literal->getValue());

    return VisitExpr(character_literal, node);
}
#else
bool ClangToDotTranslator::VisitCharacterLiteral(clang::CharacterLiteral * character_literal, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCharacterLiteral" << "\n";
#endif

#if 0
    *node = SageBuilder::buildCharVal(character_literal->getValue());
#endif

     node_desc.kind_hierarchy.push_back("CharacterLiteral");

#if CLANG_VERSION_MAJOR >= 18
    switch (character_literal->getKind()) {
        case clang::CharacterLiteralKind::Ascii:
            node_desc.attributes.push_back(std::pair<std::string, std::string>("kind", "Ascii"));
            break;
        case clang::CharacterLiteralKind::Wide:
            node_desc.attributes.push_back(std::pair<std::string, std::string>("kind", "Wide"));
            break;
        case clang::CharacterLiteralKind::UTF16:
            node_desc.attributes.push_back(std::pair<std::string, std::string>("kind", "UTF16"));
            break;
        case clang::CharacterLiteralKind::UTF32:
            node_desc.attributes.push_back(std::pair<std::string, std::string>("kind", "UTF32"));
            break;
    }
#else
    switch (character_literal->getKind()) {
        case clang::CharacterLiteral::Ascii:
            node_desc.attributes.push_back(std::pair<std::string, std::string>("kind", "Ascii"));
            break;
        case clang::CharacterLiteral::Wide:
            node_desc.attributes.push_back(std::pair<std::string, std::string>("kind", "Wide"));
            break;
        case clang::CharacterLiteral::UTF16:
            node_desc.attributes.push_back(std::pair<std::string, std::string>("kind", "UTF16"));
            break;
        case clang::CharacterLiteral::UTF32:
            node_desc.attributes.push_back(std::pair<std::string, std::string>("kind", "UTF32"));
            break;
    }
#endif
    std::ostringstream oss;
    oss << std::hex << character_literal->getValue();
    node_desc.attributes.push_back(std::pair<std::string, std::string>("hex_value", oss.str()));

    return VisitExpr(character_literal, node_desc);
}
#endif

#if 0
bool ClangToDotTranslator::VisitChooseExpr(clang::ChooseExpr * choose_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitChooseExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(choose_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitChooseExpr(clang::ChooseExpr * choose_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitChooseExpr" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("ChooseExpr");

    // TODO

    return VisitExpr(choose_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitCompoundLiteralExpr(clang::CompoundLiteralExpr * compound_literal, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCompoundLiteralExpr" << "\n";
#endif

    SgNode * tmp_node = Traverse(compound_literal->getInitializer());
    SgExprListExp * expr = isSgExprListExp(tmp_node);

    ROSE_ASSERT(expr != NULL);

    SgType * type = buildTypeFromQualifiedType(compound_literal->getType());

    ROSE_ASSERT(type != NULL);

    *node = SageBuilder::buildCompoundInitializer_nfi(expr, type);

    return VisitExpr(compound_literal, node);
}
#else
bool ClangToDotTranslator::VisitCompoundLiteralExpr(clang::CompoundLiteralExpr * compound_literal, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCompoundLiteralExpr" << "\n";
#endif

#if 0
    SgNode * tmp_node = Traverse(compound_literal->getInitializer());
    SgExprListExp * expr = isSgExprListExp(tmp_node);

    ROSE_ASSERT(expr != NULL);

    SgType * type = buildTypeFromQualifiedType(compound_literal->getType());

    ROSE_ASSERT(type != NULL);

    *node = SageBuilder::buildCompoundInitializer_nfi(expr, type);
#endif

     node_desc.kind_hierarchy.push_back("CompoundLiteralExpr");

     node_desc.successors.push_back(std::pair<std::string, std::string>("initializer", Traverse(compound_literal->getInitializer())));

    return VisitExpr(compound_literal, node_desc);
}
#endif

//bool ClangToDotTranslator::VisitConceptSpecializationExpr(clang::ConceptSpecializationExpr * concept_specialization_expr, SgNode ** node) {
//#if DEBUG_VISIT_STMT
//    logger[DEBUG] << "ClangToDotTranslator::VisitConceptSpecializationExpr" << "\n";
//#endif
//    bool res = true;
//
//    // TODO
//
//    return VisitExpr(concept_specialization_expr, node) && res;
//}

#if 0
bool ClangToDotTranslator::VisitConvertVectorExpr(clang::ConvertVectorExpr * convert_vector_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitConvertVectorExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(convert_vector_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitConvertVectorExpr(clang::ConvertVectorExpr * convert_vector_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitConvertVectorExpr" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("ConvertVectorExpr");

    // TODO

    return VisitExpr(convert_vector_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitCoroutineSuspendExpr(clang::CoroutineSuspendExpr * coroutine_suspend_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCoroutineSuspendExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(coroutine_suspend_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitCoroutineSuspendExpr(clang::CoroutineSuspendExpr * coroutine_suspend_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCoroutineSuspendExpr" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("CoroutineSuspendExpr");

    // TODO

    return VisitExpr(coroutine_suspend_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitCoawaitExpr(clang::CoawaitExpr * coawait_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCoawaitExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitCoroutineSuspendExpr(coawait_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitCoawaitExpr(clang::CoawaitExpr * coawait_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCoawaitExpr" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("CoawaitExpr");

    // TODO

    return VisitCoroutineSuspendExpr(coawait_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitCoyieldExpr(clang::CoyieldExpr * coyield_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCoyieldExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitCoroutineSuspendExpr(coyield_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitCoyieldExpr(clang::CoyieldExpr * coyield_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCoyieldExpr" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("CoyieldExpr");

    // TODO

    return VisitCoroutineSuspendExpr(coyield_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitCXXBindTemporaryExpr(clang::CXXBindTemporaryExpr * cxx_bind_temporary_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXBindTemporaryExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(cxx_bind_temporary_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitCXXBindTemporaryExpr(clang::CXXBindTemporaryExpr * cxx_bind_temporary_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXBindTemporaryExpr" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("CXXBindTemporaryExpr");

    // TODO

    return VisitExpr(cxx_bind_temporary_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitCXXBoolLiteralExpr(clang::CXXBoolLiteralExpr * cxx_bool_literal_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXBoolLiteralExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(cxx_bool_literal_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitCXXBoolLiteralExpr(clang::CXXBoolLiteralExpr * cxx_bool_literal_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXBoolLiteralExpr" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("CXXBoolLiteralExpr");

    // TODO

    return VisitExpr(cxx_bool_literal_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitCXXConstructExpr(clang::CXXConstructExpr * cxx_construct_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXConstructExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(cxx_construct_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitCXXConstructExpr(clang::CXXConstructExpr * cxx_construct_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXConstructExpr" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("CXXConstructExpr");

    // TODO

     node_desc.successors.push_back(std::pair<std::string, std::string>("constructor", Traverse(cxx_construct_expr->getConstructor())));

     clang::CXXConstructExpr::arg_iterator it;
     unsigned cnt = 0;
     for (it = cxx_construct_expr->arg_begin(); it != cxx_construct_expr->arg_end(); ++it)
        {
          std::ostringstream oss;
          oss << "argument[" << cnt++ << "]";
          node_desc.successors.push_back(std::pair<std::string, std::string>(oss.str(), Traverse(*it)));
        }

     return VisitExpr(cxx_construct_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitCXXTemporaryObjectExpr(clang::CXXTemporaryObjectExpr * cxx_temporary_object_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXTemporaryObjectExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitCXXConstructExpr(cxx_temporary_object_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitCXXTemporaryObjectExpr(clang::CXXTemporaryObjectExpr * cxx_temporary_object_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXTemporaryObjectExpr" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("CXXTemporaryObjectExpr");

    // TODO

    return VisitCXXConstructExpr(cxx_temporary_object_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitCXXDefaultArgExpr(clang::CXXDefaultArgExpr * cxx_default_arg_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXDefaultArgExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(cxx_default_arg_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitCXXDefaultArgExpr(clang::CXXDefaultArgExpr * cxx_default_arg_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXDefaultArgExpr" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("CXXDefaultArgExpr");

    // TODO

    return VisitExpr(cxx_default_arg_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitCXXDefaultInitExpr(clang::CXXDefaultInitExpr * cxx_default_init_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXDefaultInitExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(cxx_default_init_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitCXXDefaultInitExpr(clang::CXXDefaultInitExpr * cxx_default_init_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXDefaultInitExpr" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("CXXDefaultInitExpr");

    // TODO

    return VisitExpr(cxx_default_init_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitCXXDeleteExpr(clang::CXXDeleteExpr * cxx_delete_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXDeleteExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(cxx_delete_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitCXXDeleteExpr(clang::CXXDeleteExpr * cxx_delete_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXDeleteExpr" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("CXXDeleteExpr");

    // TODO

    return VisitExpr(cxx_delete_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitCXXDependentScopeMemberExpr(clang::CXXDependentScopeMemberExpr * cxx_dependent_scope_member_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXDependentScopeMemberExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(cxx_dependent_scope_member_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitCXXDependentScopeMemberExpr(clang::CXXDependentScopeMemberExpr * cxx_dependent_scope_member_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXDependentScopeMemberExpr" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("CXXDependentScopeMemberExpr");

    // TODO

    return VisitExpr(cxx_dependent_scope_member_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitCXXFoldExpr(clang::CXXFoldExpr * cxx_fold_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXFoldExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(cxx_fold_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitCXXFoldExpr(clang::CXXFoldExpr * cxx_fold_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXFoldExpr" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("CXXFoldExpr");

    // TODO

    return VisitExpr(cxx_fold_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitCXXInheritedCtorInitExpr(clang::CXXInheritedCtorInitExpr * cxx_inherited_ctor_init_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXInheritedCtorInitExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(cxx_inherited_ctor_init_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitCXXInheritedCtorInitExpr(clang::CXXInheritedCtorInitExpr * cxx_inherited_ctor_init_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXInheritedCtorInitExpr" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("CXXInheritedCtorInitExpr");

    // TODO

    return VisitExpr(cxx_inherited_ctor_init_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitCXXNewExpr(clang::CXXNewExpr * cxx_new_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXNewExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(cxx_new_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitCXXNewExpr(clang::CXXNewExpr * cxx_new_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXNewExpr" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("CXXNewExpr");

    // TODO

    return VisitExpr(cxx_new_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitCXXNoexceptExpr(clang::CXXNoexceptExpr * cxx_noexcept_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXNoexceptExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(cxx_noexcept_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitCXXNoexceptExpr(clang::CXXNoexceptExpr * cxx_noexcept_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXNoexceptExpr" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("CXXNoexceptExpr");

    // TODO

    return VisitExpr(cxx_noexcept_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitCXXNullPtrLiteralExpr(clang::CXXNullPtrLiteralExpr * cxx_null_ptr_literal_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXNullPtrLiteralExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(cxx_null_ptr_literal_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitCXXNullPtrLiteralExpr(clang::CXXNullPtrLiteralExpr * cxx_null_ptr_literal_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXNullPtrLiteralExpr" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("CXXNullPtrLiteralExpr");

    // TODO

    return VisitExpr(cxx_null_ptr_literal_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitCXXPseudoDestructorExpr(clang::CXXPseudoDestructorExpr * cxx_pseudo_destructor_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXPseudoDestructorExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(cxx_pseudo_destructor_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitCXXPseudoDestructorExpr(clang::CXXPseudoDestructorExpr * cxx_pseudo_destructor_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXPseudoDestructorExpr" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("CXXPseudoDestructorExpr");

    // TODO

    return VisitExpr(cxx_pseudo_destructor_expr, node_desc) && res;
}
#endif

//bool ClangToDotTranslator::VisitCXXRewrittenBinaryOperator(clang::CXXRewrittenBinaryOperator * cxx_rewrite_binary_operator, SgNode ** node) {
//#if DEBUG_VISIT_STMT
//    logger[DEBUG] << "ClangToDotTranslator::VisitCXXRewrittenBinaryOperator" << "\n";
//#endif
//    bool res = true;
//
//    // TODO
//
//    return VisitExpr(cxx_rewrite_binary_operator, node) && res;
//}

#if 0
bool ClangToDotTranslator::VisitCXXScalarValueInitExpr(clang::CXXScalarValueInitExpr * cxx_scalar_value_init_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXScalarValueInitExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(cxx_scalar_value_init_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitCXXScalarValueInitExpr(clang::CXXScalarValueInitExpr * cxx_scalar_value_init_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXScalarValueInitExpr" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("CXXScalarValueInitExpr");

    // TODO

    return VisitExpr(cxx_scalar_value_init_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitCXXStdInitializerListExpr(clang::CXXStdInitializerListExpr * cxx_std_initializer_list_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXStdInitializerListExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(cxx_std_initializer_list_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitCXXStdInitializerListExpr(clang::CXXStdInitializerListExpr * cxx_std_initializer_list_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXStdInitializerListExpr" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("CXXStdInitializerListExpr");

    // TODO

    return VisitExpr(cxx_std_initializer_list_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitCXXThisExpr(clang::CXXThisExpr * cxx_this_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXThisExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(cxx_this_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitCXXThisExpr(clang::CXXThisExpr * cxx_this_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXThisExpr" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("CXXThisExpr");

    // TODO

    return VisitExpr(cxx_this_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitCXXThrowExpr(clang::CXXThrowExpr * cxx_throw_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXThrowExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(cxx_throw_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitCXXThrowExpr(clang::CXXThrowExpr * cxx_throw_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXThrowExpr" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("CXXThrowExpr");

    // TODO

    return VisitExpr(cxx_throw_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitCXXTypeidExpr(clang::CXXTypeidExpr * cxx_typeid_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXTypeidExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(cxx_typeid_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitCXXTypeidExpr(clang::CXXTypeidExpr * cxx_typeid_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXTypeidExpr" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("CXXTypeidExpr");

    // TODO

    return VisitExpr(cxx_typeid_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitCXXUnresolvedConstructExpr(clang::CXXUnresolvedConstructExpr * cxx_unresolved_construct_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXUnresolvedConstructExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(cxx_unresolved_construct_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitCXXUnresolvedConstructExpr(clang::CXXUnresolvedConstructExpr * cxx_unresolved_construct_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXUnresolvedConstructExpr" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("CXXUnresolvedConstructExpr");

    // TODO

    return VisitExpr(cxx_unresolved_construct_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitCXXUuidofExpr(clang::CXXUuidofExpr * cxx_uuidof_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXUuidofExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(cxx_uuidof_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitCXXUuidofExpr(clang::CXXUuidofExpr * cxx_uuidof_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitCXXUuidofExpr" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("CXXUuidofExpr");

    // TODO

    return VisitExpr(cxx_uuidof_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitDeclRefExpr(clang::DeclRefExpr * decl_ref_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitDeclRefExpr" << "\n";
#endif

    bool res = true;

    //SgNode * tmp_node = Traverse(decl_ref_expr->getDecl());
    // DONE: Do not use Traverse(...) as the declaration can not be complete (recursive functions)
    //       Instead use SymbolTable from ROSE as the symbol should be ready (cannot have a reference before the declaration)
    // FIXME: This fix will not work for C++ (methods/fields can be use before they are declared...)
    // FIXME: I feel like it could work now, we will see ....

    SgSymbol * sym = GetSymbolFromSymbolTable(decl_ref_expr->getDecl());

    if (sym == NULL) {
        SgNode * tmp_decl = Traverse(decl_ref_expr->getDecl());

        if (tmp_decl != NULL) {
            sym = GetSymbolFromSymbolTable(decl_ref_expr->getDecl());
        }
        // FIXME hack Traverse have added the symbol but we cannot find it (probably: problem with type and function lookup)
        if (sym == NULL && isSgFunctionDeclaration(tmp_decl) != NULL) {
            sym = new SgFunctionSymbol(isSgFunctionDeclaration(tmp_decl));
            sym->set_parent(tmp_decl);
        }
        
    }

    if (sym != NULL) { // Not else if it was NULL we have try to traverse it....
        SgVariableSymbol  * var_sym  = isSgVariableSymbol(sym);
        SgFunctionSymbol  * func_sym = isSgFunctionSymbol(sym);
        SgEnumFieldSymbol * enum_sym = isSgEnumFieldSymbol(sym);

        if (var_sym != NULL) {
            *node = SageBuilder::buildVarRefExp(var_sym);
        }
        else if (func_sym != NULL) {
            *node = SageBuilder::buildFunctionRefExp(func_sym);
        }
        else if (enum_sym != NULL) {
            SgEnumDeclaration * enum_decl = isSgEnumDeclaration(enum_sym->get_declaration()->get_parent());
            ROSE_ASSERT(enum_decl != NULL);
            SgName name = enum_sym->get_name();
            *node = SageBuilder::buildEnumVal_nfi(0, enum_decl, name);
        }
        else if (sym != NULL) {
            logger[ERROR] << "Runtime error: Unknown type of symbol for a declaration reference." << "\n";
            logger[ERROR] << "    sym->class_name() = " << sym->class_name()  << "\n";
            ROSE_ABORT();
        }
    }
    else {
         logger[ERROR] << "Runtime error: Cannot find the symbol for a declaration reference (even after trying to buil th declaration)" << "\n";
         ROSE_ABORT();
    }

    return VisitExpr(decl_ref_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitDeclRefExpr(clang::DeclRefExpr * decl_ref_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitDeclRefExpr" << "\n";
#endif

    bool res = true;

#if 0
    //SgNode * tmp_node = Traverse(decl_ref_expr->getDecl());
    // DONE: Do not use Traverse(...) as the declaration can not be complete (recursive functions)
    //       Instead use SymbolTable from ROSE as the symbol should be ready (cannot have a reference before the declaration)
    // FIXME: This fix will not work for C++ (methods/fields can be use before they are declared...)
    // FIXME: I feel like it could work now, we will see ....

    SgSymbol * sym = GetSymbolFromSymbolTable(decl_ref_expr->getDecl());

    if (sym == NULL) {
        SgNode * tmp_decl = Traverse(decl_ref_expr->getDecl());

        if (tmp_decl != NULL) {
            sym = GetSymbolFromSymbolTable(decl_ref_expr->getDecl());
        }
        // FIXME hack Traverse have added the symbol but we cannot find it (probably: problem with type and function lookup)
        if (sym == NULL && isSgFunctionDeclaration(tmp_decl) != NULL) {
            sym = new SgFunctionSymbol(isSgFunctionDeclaration(tmp_decl));
            sym->set_parent(tmp_decl);
        }
        
    }

    if (sym != NULL) { // Not else if it was NULL we have try to traverse it....
        SgVariableSymbol  * var_sym  = isSgVariableSymbol(sym);
        SgFunctionSymbol  * func_sym = isSgFunctionSymbol(sym);
        SgEnumFieldSymbol * enum_sym = isSgEnumFieldSymbol(sym);

        if (var_sym != NULL) {
            *node = SageBuilder::buildVarRefExp(var_sym);
        }
        else if (func_sym != NULL) {
            *node = SageBuilder::buildFunctionRefExp(func_sym);
        }
        else if (enum_sym != NULL) {
            SgEnumDeclaration * enum_decl = isSgEnumDeclaration(enum_sym->get_declaration()->get_parent());
            ROSE_ASSERT(enum_decl != NULL);
            SgName name = enum_sym->get_name();
            *node = SageBuilder::buildEnumVal_nfi(0, enum_decl, name);
        }
        else if (sym != NULL) {
            logger[ERROR] << "Runtime error: Unknown type of symbol for a declaration reference." << "\n";
            logger[ERROR] << "    sym->class_name() = " << sym->class_name()  << "\n";
            ROSE_ABORT();
        }
    }
    else {
         logger[ERROR] << "Runtime error: Cannot find the symbol for a declaration reference (even after trying to buil th declaration)" << "\n";
         ROSE_ABORT();
    }
#endif

     node_desc.kind_hierarchy.push_back("DeclRefExpr");

     node_desc.successors.push_back(std::pair<std::string, std::string>("decl", Traverse(decl_ref_expr->getDecl())));

     return VisitExpr(decl_ref_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitDependentCoawaitExpr(clang::DependentCoawaitExpr * dependent_coawait_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitDependentCoawaitExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(dependent_coawait_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitDependentCoawaitExpr(clang::DependentCoawaitExpr * dependent_coawait_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitDependentCoawaitExpr" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("DependentCoawaitExpr");

    // TODO

    return VisitExpr(dependent_coawait_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitDependentScopeDeclRefExpr(clang::DependentScopeDeclRefExpr * dependent_scope_decl_ref_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitDependentScopeDeclRefExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(dependent_scope_decl_ref_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitDependentScopeDeclRefExpr(clang::DependentScopeDeclRefExpr * dependent_scope_decl_ref_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitDependentScopeDeclRefExpr" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("DependentScopeDeclRefExpr");

    // TODO

    return VisitExpr(dependent_scope_decl_ref_expr, node_desc) && res;
}
#endif

// bool ClangToDotTranslator::VisitDependentScopeDeclRefExpr(clang::DependentScopeDeclRefExpr * dependent_scope_decl_ref_expr);

#if 0
bool ClangToDotTranslator::VisitDesignatedInitExpr(clang::DesignatedInitExpr * designated_init_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitDesignatedInitExpr" << "\n";
#endif

    SgInitializer * init = NULL;    
    {
        SgNode * tmp_expr = Traverse(designated_init_expr->getInit());
        SgExpression * expr = isSgExpression(tmp_expr);
        ROSE_ASSERT(expr != NULL);
        SgExprListExp * expr_list_exp = isSgExprListExp(expr);
        if (expr_list_exp != NULL) {
            // FIXME get the type right...
            init = SageBuilder::buildAggregateInitializer_nfi(expr_list_exp, NULL);
        }
        else {
            init = SageBuilder::buildAssignInitializer_nfi(expr, expr->get_type());
        }
        ROSE_ASSERT(init != NULL);
        applySourceRange(init, designated_init_expr->getInit()->getSourceRange());
    }

    SgExprListExp * expr_list_exp = SageBuilder::buildExprListExp_nfi();
    auto designatorSize = designated_init_expr->size();
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
            SgNode * tmp_expr = Traverse(designated_init_expr->getArrayIndex(*D));
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
}
#else
bool ClangToDotTranslator::VisitDesignatedInitExpr(clang::DesignatedInitExpr * designated_init_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitDesignatedInitExpr" << "\n";
#endif

#if 0
    SgInitializer * init = NULL;    
    {
        SgNode * tmp_expr = Traverse(designated_init_expr->getInit());
        SgExpression * expr = isSgExpression(tmp_expr);
        ROSE_ASSERT(expr != NULL);
        SgExprListExp * expr_list_exp = isSgExprListExp(expr);
        if (expr_list_exp != NULL) {
            // FIXME get the type right...
            init = SageBuilder::buildAggregateInitializer_nfi(expr_list_exp, NULL);
        }
        else {
            init = SageBuilder::buildAssignInitializer_nfi(expr, expr->get_type());
        }
        ROSE_ASSERT(init != NULL);
        applySourceRange(init, designated_init_expr->getInit()->getSourceRange());
    }

    SgExprListExp * expr_list_exp = SageBuilder::buildExprListExp_nfi();
    auto designatorSize = designated_init_expr->size();
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
            SgNode * tmp_expr = Traverse(designated_init_expr->getArrayIndex(*D));
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
#endif

     node_desc.kind_hierarchy.push_back("DesignatedInitExpr");

     node_desc.successors.push_back(std::pair<std::string, std::string>("init", Traverse(designated_init_expr->getInit())));

#if 0
     clang::DesignatedInitExpr::designators_iterator it;
     unsigned cnt = 0;
     for (it = designated_init_expr->designators_begin(); it != designated_init_expr->designators_end(); it++) 
        {
          std::ostringstream oss;
          oss << "designator[" << cnt++ << "]";
          if (it->isFieldDesignator()) {
              oss << " field";
              node_desc.successors.push_back(std::pair<std::string, std::string>(oss.str(), Traverse(it->getField())));
          }
         else if (it->isArrayDesignator()) {
            oss << " array";
            node_desc.successors.push_back(std::pair<std::string, std::string>(oss.str(), Traverse(designated_init_expr->getArrayIndex(*it))));
        }
        else if (it->isArrayRangeDesignator()) {
            oss << " range";
            std::ostringstream oss_;
            oss_ << oss.str() << "_end";
            oss << "_start";
            node_desc.successors.push_back(std::pair<std::string, std::string>(oss.str(), Traverse(designated_init_expr->getArrayRangeStart(*it))));
            node_desc.successors.push_back(std::pair<std::string, std::string>(oss_.str(), Traverse(designated_init_expr->getArrayRangeEnd(*it))));
        }
        else ROSE_ABORT();
        }
#else
  // DQ (11/28/2020): There is no clang::DesignatedInitExpr::designators_iterator in Clang 10, 
  // so this has to be iterated over using a for loop and calling the getDesignator() function.
     unsigned cnt = 0;
     for (unsigned i = 0; i < designated_init_expr->size(); i++)
        {
          clang::DesignatedInitExpr::Designator* it = designated_init_expr->getDesignator(i);

          std::ostringstream oss;
          oss << "designator[" << cnt++ << "]";
          if (it->isFieldDesignator()) 
             {
               oss << " field";
#if CLANG_VERSION_MAJOR >= 18
               node_desc.successors.push_back(std::pair<std::string, std::string>(oss.str(), Traverse(it->getFieldDecl())));
#else
               node_desc.successors.push_back(std::pair<std::string, std::string>(oss.str(), Traverse(it->getField())));
#endif
             }
            else 
             {
               if (it->isArrayDesignator()) 
                  {
                    oss << " array";
                    node_desc.successors.push_back(std::pair<std::string, std::string>(oss.str(), Traverse(designated_init_expr->getArrayIndex(*it))));
                  }
                 else 
                  {
                    if (it->isArrayRangeDesignator()) 
                       {
                         oss << " range";
                         std::ostringstream oss_;
                         oss_ << oss.str() << "_end";
                         oss << "_start";
                         node_desc.successors.push_back(std::pair<std::string, std::string>(oss.str(), Traverse(designated_init_expr->getArrayRangeStart(*it))));
                         node_desc.successors.push_back(std::pair<std::string, std::string>(oss_.str(), Traverse(designated_init_expr->getArrayRangeEnd(*it))));
                       }
                      else 
                       {
                         ROSE_ABORT();
                       }
                  }
             }
        }
#endif

    return VisitExpr(designated_init_expr, node_desc);
}
#endif

#if 0
bool ClangToDotTranslator::VisitDesignatedInitUpdateExpr(clang::DesignatedInitUpdateExpr * designated_init_update, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitDesignatedInitUpdateExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(designated_init_update, node) && res;
}
#else
bool ClangToDotTranslator::VisitDesignatedInitUpdateExpr(clang::DesignatedInitUpdateExpr * designated_init_update, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitDesignatedInitUpdateExpr" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("DesignatedInitUpdateExpr");

    // TODO

    return VisitExpr(designated_init_update, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitExpressionTraitExpr(clang::ExpressionTraitExpr * expression_trait_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitExpressionTraitExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(expression_trait_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitExpressionTraitExpr(clang::ExpressionTraitExpr * expression_trait_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitExpressionTraitExpr" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("ExpressionTraitExpr");

    // TODO

    return VisitExpr(expression_trait_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitExtVectorElementExpr(clang::ExtVectorElementExpr * ext_vector_element_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitExtVectorElementExpr" << "\n";
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
#else
bool ClangToDotTranslator::VisitExtVectorElementExpr(clang::ExtVectorElementExpr * ext_vector_element_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitExtVectorElementExpr" << "\n";
#endif

#if 0
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
#endif

     node_desc.kind_hierarchy.push_back("ExtVectorElementExpr");

     node_desc.successors.push_back(std::pair<std::string, std::string>("base", Traverse(ext_vector_element_expr->getBase())));

     if (ext_vector_element_expr->isArrow()) 
          node_desc.attributes.push_back(std::pair<std::string, std::string>("access_operator", "arrow"));
       else 
          node_desc.attributes.push_back(std::pair<std::string, std::string>("access_operator", "dot"));

     clang::IdentifierInfo & ident_info = ext_vector_element_expr->getAccessor();
     std::string ident = ident_info.getName().str();

     node_desc.attributes.push_back(std::pair<std::string, std::string>("accessed_field", ident));

     return VisitExpr(ext_vector_element_expr, node_desc);
}
#endif

#if 0
bool ClangToDotTranslator::VisitFixedPointLiteral(clang::FixedPointLiteral * fixed_point_literal, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitFixedPointLiteral" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(fixed_point_literal, node) && res;
}
#else
bool ClangToDotTranslator::VisitFixedPointLiteral(clang::FixedPointLiteral * fixed_point_literal, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitFixedPointLiteral" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("FixedPointLiteral");

    // TODO

    return VisitExpr(fixed_point_literal, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitFloatingLiteral(clang::FloatingLiteral * floating_literal, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitFloatingLiteral" << "\n";
#endif

    unsigned int precision =  llvm::APFloat::semanticsPrecision(floating_literal->getValue().getSemantics());
    if (precision == 24)
        *node = SageBuilder::buildFloatVal(floating_literal->getValue().convertToFloat());
    else if (precision == 53)
        *node = SageBuilder::buildDoubleVal(floating_literal->getValue().convertToDouble());
    else
        ROSE_ASSERT(!"In VisitFloatingLiteral: Unsupported float size");

    return VisitExpr(floating_literal, node);
}
#else
bool ClangToDotTranslator::VisitFloatingLiteral(clang::FloatingLiteral * floating_literal, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitFloatingLiteral" << "\n";
#endif

#if 0
    unsigned int precision =  llvm::APFloat::semanticsPrecision(floating_literal->getValue().getSemantics());
    if (precision == 24)
        *node = SageBuilder::buildFloatVal(floating_literal->getValue().convertToFloat());
    else if (precision == 53)
        *node = SageBuilder::buildDoubleVal(floating_literal->getValue().convertToDouble());
    else
        ROSE_ASSERT(!"In VisitFloatingLiteral: Unsupported float size");
#endif

     node_desc.kind_hierarchy.push_back("FloatingLiteral");

  // FIXME

     unsigned int precision =  llvm::APFloat::semanticsPrecision(floating_literal->getValue().getSemantics());
     std::ostringstream oss;
     if (precision == 24) {
         node_desc.attributes.push_back(std::pair<std::string, std::string>("precision", "single"));
         oss << floating_literal->getValue().convertToFloat();
     }
     else if (precision == 53) {
         node_desc.attributes.push_back(std::pair<std::string, std::string>("precision", "double"));
         oss << floating_literal->getValue().convertToDouble();
     }
     else
         assert(!"In VisitFloatingLiteral: Unsupported float size");

     node_desc.attributes.push_back(std::pair<std::string, std::string>("value", oss.str()));

     return VisitExpr(floating_literal, node_desc);
}
#endif

#if 0
bool ClangToDotTranslator::VisitFullExpr(clang::FullExpr * full_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitFullExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(full_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitFullExpr(clang::FullExpr * full_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitFullExpr" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("FullExpr");

    // TODO

    return VisitExpr(full_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitConstantExpr(clang::ConstantExpr * constant_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitConstantExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitFullExpr(constant_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitConstantExpr(clang::ConstantExpr * constant_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitConstantExpr" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("ConstantExpr");

    // TODO

    return VisitFullExpr(constant_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitExprWithCleanups(clang::ExprWithCleanups * expr_with_cleanups, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitExprWithCleanups" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitFullExpr(expr_with_cleanups, node) && res;
}
#else
bool ClangToDotTranslator::VisitExprWithCleanups(clang::ExprWithCleanups * expr_with_cleanups, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitExprWithCleanups" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("ExprWithCleanups");

    // TODO

    return VisitFullExpr(expr_with_cleanups, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitFunctionParmPackExpr(clang::FunctionParmPackExpr * function_parm_pack_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitFunctionParmPackExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(function_parm_pack_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitFunctionParmPackExpr(clang::FunctionParmPackExpr * function_parm_pack_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitFunctionParmPackExpr" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("FunctionParmPackExpr");

    // TODO

    return VisitExpr(function_parm_pack_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitGenericSelectionExpr(clang::GenericSelectionExpr * generic_Selection_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitGenericSelectionExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(generic_Selection_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitGenericSelectionExpr(clang::GenericSelectionExpr * generic_Selection_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitGenericSelectionExpr" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("GenericSelectionExpr");

    // TODO

    return VisitExpr(generic_Selection_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitGNUNullExpr(clang::GNUNullExpr * gnu_null_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitGNUNullExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(gnu_null_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitGNUNullExpr(clang::GNUNullExpr * gnu_null_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitGNUNullExpr" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("GNUNullExpr");

    // TODO

    return VisitExpr(gnu_null_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitImaginaryLiteral(clang::ImaginaryLiteral * imaginary_literal, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitImaginaryLiteral" << "\n";
#endif

    SgNode * tmp_imag_val = Traverse(imaginary_literal->getSubExpr());
    SgValueExp * imag_val = isSgValueExp(tmp_imag_val);
    ROSE_ASSERT(imag_val != NULL);

    SgComplexVal * comp_val = new SgComplexVal(NULL, imag_val, imag_val->get_type(), "");

    *node = comp_val;

    return VisitExpr(imaginary_literal, node);
}
#else
bool ClangToDotTranslator::VisitImaginaryLiteral(clang::ImaginaryLiteral * imaginary_literal, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitImaginaryLiteral" << "\n";
#endif

#if 0
    SgNode * tmp_imag_val = Traverse(imaginary_literal->getSubExpr());
    SgValueExp * imag_val = isSgValueExp(tmp_imag_val);
    ROSE_ASSERT(imag_val != NULL);

    SgComplexVal * comp_val = new SgComplexVal(NULL, imag_val, imag_val->get_type(), "");

    *node = comp_val;
#endif

     node_desc.kind_hierarchy.push_back("ImaginaryLiteral");

     node_desc.successors.push_back(std::pair<std::string, std::string>("sub_expr", Traverse(imaginary_literal->getSubExpr())));

    return VisitExpr(imaginary_literal, node_desc);
}
#endif

#if 0
bool ClangToDotTranslator::VisitImplicitValueInitExpr(clang::ImplicitValueInitExpr * implicit_value_init_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitImplicitValueInitExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(implicit_value_init_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitImplicitValueInitExpr(clang::ImplicitValueInitExpr * implicit_value_init_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitImplicitValueInitExpr" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("ImplicitValueInitExpr");

    // TODO

    return VisitExpr(implicit_value_init_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitInitListExpr(clang::InitListExpr * init_list_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitInitListExpr" << "\n";
#endif

    // We use the syntactic version of the initializer if it exists
    if (init_list_expr->getSyntacticForm() != NULL) return VisitInitListExpr(init_list_expr->getSyntacticForm(), node);

    SgExprListExp * expr_list_expr = SageBuilder::buildExprListExp_nfi();

    clang::InitListExpr::iterator it;
    for (it = init_list_expr->begin(); it != init_list_expr->end(); it++) {
        SgNode * tmp_expr = Traverse(*it);
        SgExpression * expr = isSgExpression(tmp_expr);
        ROSE_ASSERT(expr != NULL);
        expr_list_expr->append_expression(expr);
    }

    *node = expr_list_expr;

    return VisitExpr(init_list_expr, node);
}
#else
bool ClangToDotTranslator::VisitInitListExpr(clang::InitListExpr * init_list_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitInitListExpr" << "\n";
#endif

#if 0
    // We use the syntactic version of the initializer if it exists
    if (init_list_expr->getSyntacticForm() != NULL) return VisitInitListExpr(init_list_expr->getSyntacticForm(), node);

    SgExprListExp * expr_list_expr = SageBuilder::buildExprListExp_nfi();

    clang::InitListExpr::iterator it;
    for (it = init_list_expr->begin(); it != init_list_expr->end(); it++) {
        SgNode * tmp_expr = Traverse(*it);
        SgExpression * expr = isSgExpression(tmp_expr);
        ROSE_ASSERT(expr != NULL);
        expr_list_expr->append_expression(expr);
    }

    *node = expr_list_expr;
#endif

     node_desc.kind_hierarchy.push_back("InitListExpr");

    clang::InitListExpr::iterator it;
    unsigned cnt = 0;
    for (it = init_list_expr->begin(); it != init_list_expr->end(); it++) {
        std::ostringstream oss;
        oss << "init[" << cnt++ << "]";
        node_desc.successors.push_back(std::pair<std::string, std::string>(oss.str(), Traverse(*it)));
    }

    return VisitExpr(init_list_expr, node_desc);
}
#endif

#if 0
bool ClangToDotTranslator::VisitIntegerLiteral(clang::IntegerLiteral * integer_literal, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitIntegerLiteral" << "\n";
#endif

    *node = SageBuilder::buildIntVal(integer_literal->getValue().getSExtValue());

    return VisitExpr(integer_literal, node);
}
#else
bool ClangToDotTranslator::VisitIntegerLiteral(clang::IntegerLiteral * integer_literal, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitIntegerLiteral" << "\n";
#endif

#if 0
    *node = SageBuilder::buildIntVal(integer_literal->getValue().getSExtValue());
#endif

     node_desc.kind_hierarchy.push_back("IntegerLiteral");

    // FIXME

  // DQ (11/28/2020): getHashValue is no longer available.
  // std::ostringstream oss;
  // oss << std::hex << integer_literal->getValue().getHashValue();
  // node_desc.attributes.push_back(std::pair<std::string, std::string>("hex_hash_value", oss.str()));

    return VisitExpr(integer_literal, node_desc);
}
#endif

#if 0
bool ClangToDotTranslator::VisitLambdaExpr(clang::LambdaExpr * lambda_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitLambdaExpr" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("LambdaExpr");

    // TODO

    return VisitExpr(lambda_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitLambdaExpr(clang::LambdaExpr * lambda_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitLambdaExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(lambda_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitMaterializeTemporaryExpr(clang::MaterializeTemporaryExpr * materialize_temporary_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitMaterializeTemporaryExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(materialize_temporary_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitMaterializeTemporaryExpr(clang::MaterializeTemporaryExpr * materialize_temporary_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitMaterializeTemporaryExpr" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("MaterializeTemporaryExpr");

    // TODO

    return VisitExpr(materialize_temporary_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitMemberExpr(clang::MemberExpr * member_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitMemberExpr" << "\n";
#endif

    bool res = true;

    SgNode * tmp_base = Traverse(member_expr->getBase());
    SgExpression * base = isSgExpression(tmp_base);
    ROSE_ASSERT(base != NULL);

    SgSymbol * sym = GetSymbolFromSymbolTable(member_expr->getMemberDecl());

    SgVariableSymbol * var_sym  = isSgVariableSymbol(sym);
    SgMemberFunctionSymbol * func_sym = isSgMemberFunctionSymbol(sym);

    SgExpression * sg_member_expr = NULL;

    bool successful_cast = var_sym || func_sym;
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

    ROSE_ASSERT(sg_member_expr != NULL);

    // TODO (C++) member_expr->getQualifier() : for 'a->Base::foo'

    if (member_expr->isArrow())
        *node = SageBuilder::buildArrowExp(base, sg_member_expr);
    else
        *node = SageBuilder::buildDotExp(base, sg_member_expr);

    return VisitExpr(member_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitMemberExpr(clang::MemberExpr * member_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitMemberExpr" << "\n";
#endif

    bool res = true;

#if 0
    SgNode * tmp_base = Traverse(member_expr->getBase());
    SgExpression * base = isSgExpression(tmp_base);
    ROSE_ASSERT(base != NULL);

    SgSymbol * sym = GetSymbolFromSymbolTable(member_expr->getMemberDecl());

    SgVariableSymbol * var_sym  = isSgVariableSymbol(sym);
    SgMemberFunctionSymbol * func_sym = isSgMemberFunctionSymbol(sym);

    SgExpression * sg_member_expr = NULL;

    bool successful_cast = var_sym || func_sym;
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

    ROSE_ASSERT(sg_member_expr != NULL);

    // TODO (C++) member_expr->getQualifier() : for 'a->Base::foo'

    if (member_expr->isArrow())
        *node = SageBuilder::buildArrowExp(base, sg_member_expr);
    else
        *node = SageBuilder::buildDotExp(base, sg_member_expr);
#endif

     node_desc.kind_hierarchy.push_back("MemberExpr");

     node_desc.successors.push_back(std::pair<std::string, std::string>("base", Traverse(member_expr->getBase())));

     node_desc.successors.push_back(std::pair<std::string, std::string>("member_decl", Traverse(member_expr->getMemberDecl())));

     if (member_expr->isArrow())
         node_desc.attributes.push_back(std::pair<std::string, std::string>("access_operator", "arrow"));
     else
         node_desc.attributes.push_back(std::pair<std::string, std::string>("access_operator", "dot"));

    return VisitExpr(member_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitMSPropertyRefExpr(clang::MSPropertyRefExpr * ms_property_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitMSPropertyRefExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(ms_property_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitMSPropertyRefExpr(clang::MSPropertyRefExpr * ms_property_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitMSPropertyRefExpr" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("MSPropertyRefExpr");

    // TODO

    return VisitExpr(ms_property_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitMSPropertySubscriptExpr(clang::MSPropertySubscriptExpr * ms_property_subscript_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitMSPropertySubscriptExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(ms_property_subscript_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitMSPropertySubscriptExpr(clang::MSPropertySubscriptExpr * ms_property_subscript_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitMSPropertySubscriptExpr" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("MSPropertySubscriptExpr");

    // TODO

    return VisitExpr(ms_property_subscript_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitNoInitExpr(clang::NoInitExpr * no_init_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitNoInitExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(no_init_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitNoInitExpr(clang::NoInitExpr * no_init_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitNoInitExpr" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("NoInitExpr");

    // TODO

    return VisitExpr(no_init_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitOffsetOfExpr(clang::OffsetOfExpr * offset_of_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitOffsetOfExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(offset_of_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitOffsetOfExpr(clang::OffsetOfExpr * offset_of_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitOffsetOfExpr" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("OffsetOfExpr");

    // TODO

    return VisitExpr(offset_of_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitOMPArraySectionExpr(RoseClangCompat::ArraySectionExpr * omp_array_section_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitOMPArraySectionExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(omp_array_section_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitOMPArraySectionExpr(RoseClangCompat::ArraySectionExpr * omp_array_section_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitOMPArraySectionExpr" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("OMPArraySectionExpr");

    // TODO

    return VisitExpr(omp_array_section_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitOpaqueValueExpr(clang::OpaqueValueExpr * opaque_value_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitOpaqueValueExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(opaque_value_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitOpaqueValueExpr(clang::OpaqueValueExpr * opaque_value_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitOpaqueValueExpr" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("OpaqueValueExpr");

    // TODO

    return VisitExpr(opaque_value_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitOverloadExpr(clang::OverloadExpr * overload_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitOverloadExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(overload_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitOverloadExpr(clang::OverloadExpr * overload_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitOverloadExpr" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("OverloadExpr");

    // TODO

    return VisitExpr(overload_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitUnresolvedLookupExpr(clang::UnresolvedLookupExpr * unresolved_lookup_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitUnresolvedLookupExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitOverloadExpr(unresolved_lookup_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitUnresolvedLookupExpr(clang::UnresolvedLookupExpr * unresolved_lookup_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitUnresolvedLookupExpr" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("UnresolvedLookupExpr");

    // TODO

    return VisitOverloadExpr(unresolved_lookup_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitUnresolvedMemberExpr(clang::UnresolvedMemberExpr * unresolved_member_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitUnresolvedMemberExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitOverloadExpr(unresolved_member_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitUnresolvedMemberExpr(clang::UnresolvedMemberExpr * unresolved_member_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitUnresolvedMemberExpr" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("UnresolvedMemberExpr");

    // TODO

    return VisitOverloadExpr(unresolved_member_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitPackExpansionExpr(clang::PackExpansionExpr * pack_expansion_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitPackExpansionExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(pack_expansion_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitPackExpansionExpr(clang::PackExpansionExpr * pack_expansion_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitPackExpansionExpr" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("PackExpansionExpr");

    // TODO

    return VisitExpr(pack_expansion_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitParenExpr(clang::ParenExpr * paren_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitParenExpr" << "\n";
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
#else
bool ClangToDotTranslator::VisitParenExpr(clang::ParenExpr * paren_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitParenExpr" << "\n";
#endif

    bool res = true;

#if 0
    SgNode * tmp_subexpr = Traverse(paren_expr->getSubExpr());
    SgExpression * subexpr = isSgExpression(tmp_subexpr);
    if (tmp_subexpr != NULL && subexpr == NULL) {
        logger[WARN] << "Runtime error: tmp_subexpr != NULL && subexpr == NULL" << "\n";
        res = false;
    }

    // bypass ParenExpr, their is nothing equivalent in SageIII
    *node = subexpr;
#endif

     node_desc.kind_hierarchy.push_back("ParenExpr");

     node_desc.successors.push_back(std::pair<std::string, std::string>("sub_expr", Traverse(paren_expr->getSubExpr())));

    return VisitExpr(paren_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitParenListExpr(clang::ParenListExpr * paran_list_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitParenListExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(paran_list_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitParenListExpr(clang::ParenListExpr * paran_list_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitParenListExpr" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("ParenListExpr");

    // TODO

    return VisitExpr(paran_list_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitPredefinedExpr(clang::PredefinedExpr * predefined_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitPredefinedExpr" << "\n";
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
            ROSE_ABORT();
            break;
    }

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
#else
bool ClangToDotTranslator::VisitPredefinedExpr(clang::PredefinedExpr * predefined_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitPredefinedExpr" << "\n";
#endif

    // FIXME It's get tricky here: PredefinedExpr represent compiler generateed variables
    //    I choose to attach those variables on demand in the function definition scope 

  // Traverse the scope's stack to find the last function definition:

#if 0
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
            ROSE_ABORT();
            break;
    }

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
#endif

     node_desc.kind_hierarchy.push_back("PredefinedExpr");

  // DQ (11/28/2020): Change of function name in Clang 10.
  // switch (predefined_expr->getIdentType()) 
#if CLANG_VERSION_MAJOR >= 18
     switch (predefined_expr->getIdentKind()) 
        {
          case clang::PredefinedIdentKind::Func:
               node_desc.attributes.push_back(std::pair<std::string, std::string>("ident_type", "func"));
               break;
          case clang::PredefinedIdentKind::Function:
               node_desc.attributes.push_back(std::pair<std::string, std::string>("ident_type", "function"));
               break;
          case clang::PredefinedIdentKind::PrettyFunction:
               node_desc.attributes.push_back(std::pair<std::string, std::string>("ident_type", "pretty_function"));
               break;
          case clang::PredefinedIdentKind::PrettyFunctionNoVirtual:
               node_desc.attributes.push_back(std::pair<std::string, std::string>("ident_type", "pretty_function_no_virtual"));
               break;
        }
#else
     switch (predefined_expr->getIdentKind()) 
        {
          case clang::PredefinedExpr::Func:
               node_desc.attributes.push_back(std::pair<std::string, std::string>("ident_type", "func"));
               break;
          case clang::PredefinedExpr::Function:
               node_desc.attributes.push_back(std::pair<std::string, std::string>("ident_type", "function"));
               break;
          case clang::PredefinedExpr::PrettyFunction:
               node_desc.attributes.push_back(std::pair<std::string, std::string>("ident_type", "pretty_function"));
               break;
          case clang::PredefinedExpr::PrettyFunctionNoVirtual:
               node_desc.attributes.push_back(std::pair<std::string, std::string>("ident_type", "pretty_function_no_virtual"));
               break;
        }
#endif
    return VisitExpr(predefined_expr, node_desc);
}
#endif

#if 0
bool ClangToDotTranslator::VisitPseudoObjectExpr(clang::PseudoObjectExpr * pseudo_object_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitPseudoObjectExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(pseudo_object_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitPseudoObjectExpr(clang::PseudoObjectExpr * pseudo_object_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitPseudoObjectExpr" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("PseudoObjectExpr");

    // TODO

    return VisitExpr(pseudo_object_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitShuffleVectorExpr(clang::ShuffleVectorExpr * shuffle_vector_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitShuffleVectorExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(shuffle_vector_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitShuffleVectorExpr(clang::ShuffleVectorExpr * shuffle_vector_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitShuffleVectorExpr" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("ShuffleVectorExpr");

    // TODO

    return VisitExpr(shuffle_vector_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitSizeOfPackExpr(clang::SizeOfPackExpr * size_of_pack_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitSizeOfPackExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(size_of_pack_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitSizeOfPackExpr(clang::SizeOfPackExpr * size_of_pack_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitSizeOfPackExpr" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("SizeOfPackExpr");

    // TODO

    return VisitExpr(size_of_pack_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitSourceLocExpr(clang::SourceLocExpr * source_loc_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitSourceLocExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(source_loc_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitSourceLocExpr(clang::SourceLocExpr * source_loc_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitSourceLocExpr" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("SourceLocExpr");

    // TODO

    return VisitExpr(source_loc_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitStmtExpr(clang::StmtExpr * stmt_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitStmtExpr" << "\n";
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
#else
bool ClangToDotTranslator::VisitStmtExpr(clang::StmtExpr * stmt_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitStmtExpr" << "\n";
#endif

    bool res = true;

#if 0
    SgNode * tmp_substmt = Traverse(stmt_expr->getSubStmt());
    SgStatement * substmt = isSgStatement(tmp_substmt);
    if (tmp_substmt != NULL && substmt == NULL) {
        logger[WARN] << "Runtime error: tmp_substmt != NULL && substmt == NULL" << "\n";
        res = false;
    }

    *node = new SgStatementExpression(substmt);
#endif

     node_desc.kind_hierarchy.push_back("StmtExpr");

     node_desc.successors.push_back(std::pair<std::string, std::string>("sub_stmt", Traverse(stmt_expr->getSubStmt())));

    return VisitExpr(stmt_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitStringLiteral(clang::StringLiteral * string_literal, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitStringLiteral" << "\n";
#endif

    std::string tmp = string_literal->getString().str();
    const char * raw_str = tmp.c_str();

    unsigned i = 0;
    unsigned l = 0;
    while (raw_str[i] != '\0') {
        if (
            raw_str[i] == '\\' ||
            raw_str[i] == '\n' ||
            raw_str[i] == '\r' ||
            raw_str[i] == '"')
        {
            l++;
        }
        l++;
        i++;
    }
    l++;

    char * str = (char *)malloc(l * sizeof(char));
    i = 0;
    unsigned cnt = 0;

    while (raw_str[i] != '\0') {
        switch (raw_str[i]) {
            case '\\':
                str[cnt++] = '\\';
                str[cnt++] = '\\';
                break;
            case '\n':
                str[cnt++] = '\\';
                str[cnt++] = 'n';
                break;
            case '\r':
                str[cnt++] = '\\';
                str[cnt++] = 'r';
                break;
            case '"':
                str[cnt++] = '\\';
                str[cnt++] = '"';
                break;
            default:
                str[cnt++] = raw_str[i];
        }
        i++;
    }
    str[cnt] = '\0';

    ROSE_ASSERT(l==cnt+1);

    *node = SageBuilder::buildStringVal(str);

    return VisitExpr(string_literal, node);
}
#else
bool ClangToDotTranslator::VisitStringLiteral(clang::StringLiteral * string_literal, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitStringLiteral" << "\n";
#endif

    std::string tmp = string_literal->getString().str();
    const char * raw_str = tmp.c_str();

    unsigned i = 0;
    unsigned l = 0;
    while (raw_str[i] != '\0') {
        if (
            raw_str[i] == '\\' ||
            raw_str[i] == '\n' ||
            raw_str[i] == '\r' ||
            raw_str[i] == '"')
        {
            l++;
        }
        l++;
        i++;
    }
    l++;

    char * str = (char *)malloc(l * sizeof(char));
    i = 0;
    unsigned cnt = 0;

    while (raw_str[i] != '\0') {
        switch (raw_str[i]) {
            case '\\':
                str[cnt++] = '\\';
                str[cnt++] = '\\';
                break;
            case '\n':
                str[cnt++] = '\\';
                str[cnt++] = 'n';
                break;
            case '\r':
                str[cnt++] = '\\';
                str[cnt++] = 'r';
                break;
            case '"':
                str[cnt++] = '\\';
                str[cnt++] = '"';
                break;
            default:
                str[cnt++] = raw_str[i];
        }
        i++;
    }
    str[cnt] = '\0';

    ROSE_ASSERT(l==cnt+1);

#if 0
    *node = SageBuilder::buildStringVal(str);
#endif

     node_desc.kind_hierarchy.push_back("StringLiteral");

     node_desc.attributes.push_back(std::pair<std::string, std::string>("string", string_literal->getString().str()));

    return VisitExpr(string_literal, node_desc);
}
#endif

#if 0
bool ClangToDotTranslator::VisitSubstNonTypeTemplateParmExpr(clang::SubstNonTypeTemplateParmExpr * subst_non_type_template_parm_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitSubstNonTypeTemplateParmExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(subst_non_type_template_parm_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitSubstNonTypeTemplateParmExpr(clang::SubstNonTypeTemplateParmExpr * subst_non_type_template_parm_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitSubstNonTypeTemplateParmExpr" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("SubstNonTypeTemplateParmExpr");

    // TODO

    return VisitExpr(subst_non_type_template_parm_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitSubstNonTypeTemplateParmPackExpr(clang::SubstNonTypeTemplateParmPackExpr * subst_non_type_template_parm_pack_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitSubstNonTypeTemplateParmPackExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(subst_non_type_template_parm_pack_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitSubstNonTypeTemplateParmPackExpr(clang::SubstNonTypeTemplateParmPackExpr * subst_non_type_template_parm_pack_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitSubstNonTypeTemplateParmPackExpr" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("SubstNonTypeTemplateParmPackExpr");

    // TODO

    return VisitExpr(subst_non_type_template_parm_pack_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitTypeTraitExpr(clang::TypeTraitExpr * type_trait, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitTypeTraitExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(type_trait, node) && res;
}
#else
bool ClangToDotTranslator::VisitTypeTraitExpr(clang::TypeTraitExpr * type_trait, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitTypeTraitExpr" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("TypeTraitExpr");

    // TODO

    return VisitExpr(type_trait, node_desc) && res;
}
#endif

// (10/30/2026) lin32: Match the declaration/dispatch guards for TypoExpr, which was removed in Clang 21.
#if CLANG_VERSION_MAJOR < 21
#if 0
bool ClangToDotTranslator::VisitTypoExpr(clang::TypoExpr * typo_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitTypoExpr" << "\n";
#endif
    bool res = true;

    // TODO

    return VisitExpr(typo_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitTypoExpr(clang::TypoExpr * typo_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitTypoExpr" << "\n";
#endif
    bool res = true;

     node_desc.kind_hierarchy.push_back("TypoExpr");

    // TODO

    return VisitExpr(typo_expr, node_desc) && res;
}
#endif

#endif // CLANG_VERSION_MAJOR < 21

#if 0
bool ClangToDotTranslator::VisitUnaryExprOrTypeTraitExpr(clang::UnaryExprOrTypeTraitExpr * unary_expr_or_type_trait_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitUnaryExprOrTypeTraitExpr" << "\n";
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
            if (type != NULL) *node = SageBuilder::buildSizeOfOp_nfi(type);
            else if (expr != NULL) *node = SageBuilder::buildSizeOfOp_nfi(expr);
            else res = false;
            break;
        case clang::UETT_AlignOf:
            ROSE_ASSERT(!"C/C++  - AlignOf is not supported!");
        case clang::UETT_VecStep:
            ROSE_ASSERT(!"OpenCL - VecStep is not supported!");
    }

    return VisitStmt(unary_expr_or_type_trait_expr, node) && res;
}
#else
bool ClangToDotTranslator::VisitUnaryExprOrTypeTraitExpr(clang::UnaryExprOrTypeTraitExpr * unary_expr_or_type_trait_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitUnaryExprOrTypeTraitExpr" << "\n";
#endif

    bool res = true;

#if 0
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
            if (type != NULL) *node = SageBuilder::buildSizeOfOp_nfi(type);
            else if (expr != NULL) *node = SageBuilder::buildSizeOfOp_nfi(expr);
            else res = false;
            break;
        case clang::UETT_AlignOf:
            ROSE_ASSERT(!"C/C++  - AlignOf is not supported!");
        case clang::UETT_VecStep:
            ROSE_ASSERT(!"OpenCL - VecStep is not supported!");
    }
#endif

     node_desc.kind_hierarchy.push_back("UnaryExprOrTypeTraitExpr");

    if (unary_expr_or_type_trait_expr->isArgumentType()) {
        node_desc.successors.push_back(std::pair<std::string, std::string>(
            "argument_type", Traverse(unary_expr_or_type_trait_expr->getArgumentType().getTypePtr())
        ));
    }
    else {
        node_desc.successors.push_back(std::pair<std::string, std::string>("argument_expr", Traverse(unary_expr_or_type_trait_expr->getArgumentExpr())));
    }

    switch (unary_expr_or_type_trait_expr->getKind()) {
        case clang::UETT_SizeOf:
            node_desc.attributes.push_back(std::pair<std::string, std::string>("kind", "sizeof"));
            break;
        case clang::UETT_AlignOf:
            node_desc.attributes.push_back(std::pair<std::string, std::string>("kind", "alignof"));
            break;
        case clang::UETT_VecStep:
            node_desc.attributes.push_back(std::pair<std::string, std::string>("kind", "vecstep"));
            break;
    }

    return VisitStmt(unary_expr_or_type_trait_expr, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitUnaryOperator(clang::UnaryOperator * unary_operator, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitUnaryOperator" << "\n";
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
        case clang::UO_Not:
            *node = SageBuilder::buildNotOp(subexpr);
            break;
        case clang::UO_LNot:
            *node = SageBuilder::buildBitComplementOp(subexpr);
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
#else
bool ClangToDotTranslator::VisitUnaryOperator(clang::UnaryOperator * unary_operator, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitUnaryOperator" << "\n";
#endif

    bool res = true;

#if 0
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
        case clang::UO_Not:
            *node = SageBuilder::buildNotOp(subexpr);
            break;
        case clang::UO_LNot:
            *node = SageBuilder::buildBitComplementOp(subexpr);
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
#endif

     node_desc.kind_hierarchy.push_back("UnaryOperator");

    node_desc.successors.push_back(std::pair<std::string, std::string>("sub_expr", Traverse(unary_operator->getSubExpr())));

    switch (unary_operator->getOpcode()) {
        case clang::UO_PostInc:
            node_desc.attributes.push_back(std::pair<std::string, std::string>("opcode", "PostInc"));
            break;
        case clang::UO_PostDec:
            node_desc.attributes.push_back(std::pair<std::string, std::string>("opcode", "PostDec"));
            break;
        case clang::UO_PreInc:
            node_desc.attributes.push_back(std::pair<std::string, std::string>("opcode", "PreInc"));
            break;
        case clang::UO_PreDec:
            node_desc.attributes.push_back(std::pair<std::string, std::string>("opcode", "PreDec"));
            break;
        case clang::UO_AddrOf:
            node_desc.attributes.push_back(std::pair<std::string, std::string>("opcode", "AddrOf"));
            break;
        case clang::UO_Deref:
            node_desc.attributes.push_back(std::pair<std::string, std::string>("opcode", "Deref"));
            break;
        case clang::UO_Plus:
            node_desc.attributes.push_back(std::pair<std::string, std::string>("opcode", "Plus"));
            break;
        case clang::UO_Minus:
            node_desc.attributes.push_back(std::pair<std::string, std::string>("opcode", "Minus"));
            break;
        case clang::UO_Not:
            node_desc.attributes.push_back(std::pair<std::string, std::string>("opcode", "Not"));
            break;
        case clang::UO_LNot:
            node_desc.attributes.push_back(std::pair<std::string, std::string>("opcode", "LNot"));
            break;
        case clang::UO_Real:
            node_desc.attributes.push_back(std::pair<std::string, std::string>("opcode", "Real"));
            break;
        case clang::UO_Imag:
            node_desc.attributes.push_back(std::pair<std::string, std::string>("opcode", "Imag"));
            break;
        case clang::UO_Extension:
            node_desc.attributes.push_back(std::pair<std::string, std::string>("opcode", "Extension"));
            break;
    }

    return VisitExpr(unary_operator, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitVAArgExpr(clang::VAArgExpr * va_arg_expr, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitVAArgExpr" << "\n";
#endif

    SgNode * tmp_expr = Traverse(va_arg_expr->getSubExpr());
    SgExpression * expr = isSgExpression(tmp_expr);
    ROSE_ASSERT(expr != NULL);

    *node = SageBuilder::buildVarArgOp_nfi(expr, expr->get_type());

    return VisitExpr(va_arg_expr, node);
}
#else
bool ClangToDotTranslator::VisitVAArgExpr(clang::VAArgExpr * va_arg_expr, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitVAArgExpr" << "\n";
#endif

#if 0
    SgNode * tmp_expr = Traverse(va_arg_expr->getSubExpr());
    SgExpression * expr = isSgExpression(tmp_expr);
    ROSE_ASSERT(expr != NULL);

    *node = SageBuilder::buildVarArgOp_nfi(expr, expr->get_type());
#endif

     node_desc.kind_hierarchy.push_back("VAArgExpr");

     node_desc.successors.push_back(std::pair<std::string, std::string>("sub_expr", Traverse(va_arg_expr->getSubExpr())));

    return VisitExpr(va_arg_expr, node_desc);
}
#endif

#if 0
bool ClangToDotTranslator::VisitLabelStmt(clang::LabelStmt * label_stmt, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitLabelStmt" << "\n";
#endif

    bool res = true;

    SgName name(label_stmt->getName());

    SgNode * tmp_sub_stmt = Traverse(label_stmt->getSubStmt());
    SgStatement * sg_sub_stmt = isSgStatement(tmp_sub_stmt);
    if (sg_sub_stmt == NULL) {
        SgExpression * sg_sub_expr = isSgExpression(tmp_sub_stmt);
        ROSE_ASSERT(sg_sub_expr != NULL);
        sg_sub_stmt = SageBuilder::buildExprStatement(sg_sub_expr);
    }

    ROSE_ASSERT(sg_sub_stmt != NULL);

    *node = SageBuilder::buildLabelStatement_nfi(name, sg_sub_stmt, SageBuilder::topScopeStack());

    SgLabelStatement * sg_label_stmt = isSgLabelStatement(*node);
    SgFunctionDefinition * label_scope = NULL;
    std::list<SgScopeStatement *>::reverse_iterator it = SageBuilder::ScopeStack.rbegin();
    while (it != SageBuilder::ScopeStack.rend() && label_scope == NULL) {
        label_scope = isSgFunctionDefinition(*it);
        it++;
    }
    if (label_scope == NULL) {
         logger[WARN] << "Runtime error: Cannot find a surrounding function definition for the label statement: \"" << name << "\"." << "\n";
         res = false;
    }
    else {
        sg_label_stmt->set_scope(label_scope);
        SgLabelSymbol* label_sym = new SgLabelSymbol(sg_label_stmt);
        label_scope->insert_symbol(label_sym->get_name(), label_sym);
    }

    return VisitStmt(label_stmt, node) && res;
}
#else
bool ClangToDotTranslator::VisitLabelStmt(clang::LabelStmt * label_stmt, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitLabelStmt" << "\n";
#endif

    bool res = true;

#if 0
    SgName name(label_stmt->getName());

    SgNode * tmp_sub_stmt = Traverse(label_stmt->getSubStmt());
    SgStatement * sg_sub_stmt = isSgStatement(tmp_sub_stmt);
    if (sg_sub_stmt == NULL) {
        SgExpression * sg_sub_expr = isSgExpression(tmp_sub_stmt);
        ROSE_ASSERT(sg_sub_expr != NULL);
        sg_sub_stmt = SageBuilder::buildExprStatement(sg_sub_expr);
    }

    ROSE_ASSERT(sg_sub_stmt != NULL);

    *node = SageBuilder::buildLabelStatement_nfi(name, sg_sub_stmt, SageBuilder::topScopeStack());

    SgLabelStatement * sg_label_stmt = isSgLabelStatement(*node);
    SgFunctionDefinition * label_scope = NULL;
    std::list<SgScopeStatement *>::reverse_iterator it = SageBuilder::ScopeStack.rbegin();
    while (it != SageBuilder::ScopeStack.rend() && label_scope == NULL) {
        label_scope = isSgFunctionDefinition(*it);
        it++;
    }
    if (label_scope == NULL) {
         logger[WARN] << "Runtime error: Cannot find a surrounding function definition for the label statement: \"" << name << "\"." << "\n";
         res = false;
    }
    else {
        sg_label_stmt->set_scope(label_scope);
        SgLabelSymbol* label_sym = new SgLabelSymbol(sg_label_stmt);
        label_scope->insert_symbol(label_sym->get_name(), label_sym);
    }
#endif

     node_desc.kind_hierarchy.push_back("LabelStmt");

     node_desc.attributes.push_back(std::pair<std::string, std::string>("name", label_stmt->getName()));

     node_desc.successors.push_back(std::pair<std::string, std::string>("sub_stmt", Traverse(label_stmt->getSubStmt())));

    return VisitStmt(label_stmt, node_desc) && res;
}
#endif

#if 0
bool ClangToDotTranslator::VisitWhileStmt(clang::WhileStmt * while_stmt, SgNode ** node) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitWhileStmt" << "\n";
#endif

    SgNode * tmp_cond = Traverse(while_stmt->getCond());
    SgExpression * cond = isSgExpression(tmp_cond);
    ROSE_ASSERT(cond != NULL);

    SgStatement * expr_stmt = SageBuilder::buildExprStatement(cond);

    SgWhileStmt * sg_while_stmt = SageBuilder::buildWhileStmt_nfi(expr_stmt, NULL);

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
#else
bool ClangToDotTranslator::VisitWhileStmt(clang::WhileStmt * while_stmt, NodeDescriptor & node_desc) {
#if DEBUG_VISIT_STMT
    logger[DEBUG] << "ClangToDotTranslator::VisitWhileStmt" << "\n";
#endif

#if 0
    SgNode * tmp_cond = Traverse(while_stmt->getCond());
    SgExpression * cond = isSgExpression(tmp_cond);
    ROSE_ASSERT(cond != NULL);

    SgStatement * expr_stmt = SageBuilder::buildExprStatement(cond);

    SgWhileStmt * sg_while_stmt = SageBuilder::buildWhileStmt_nfi(expr_stmt, NULL);

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
#endif

     node_desc.kind_hierarchy.push_back("WhileStmt");

     node_desc.successors.push_back(std::pair<std::string, std::string>("cond", Traverse(while_stmt->getCond())));

     node_desc.successors.push_back(std::pair<std::string, std::string>("body", Traverse(while_stmt->getBody())));

     return VisitStmt(while_stmt, node_desc);
}
#endif
