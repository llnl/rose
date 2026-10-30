
#include "edgRose.h"

namespace EDG_ROSE_Translation {

#define DEBUG_construct_compound_literal 0

SgCompoundLiteralExp * construct_compound_literal(an_expr_node_ptr expr, SgExpression * expression) {
#if DEBUG_construct_compound_literal
  printf ("Enter construct_compound_literal():\n");
  printf ("  expr = %p\n");
  printf ("  expression = %p = %s \n", expression, expression->class_name().c_str());
#endif
  SgType * type = convert_type(expr->type);
  ROSE_ASSERT(type != NULL);

#if DEBUG_construct_compound_literal
  printf ("  type = %p = %s \n", type, type->class_name().c_str());
#endif

  SgInitializer * initializer = isSgInitializer(expression);
  if (initializer == NULL) {
    SgExprListExp* expr_list =  SageBuilder::buildExprListExp(expression);
    expr_list = set_generated_file_info_to_unparse(expr_list);
    SgAggregateInitializer * aggr_init = SageBuilder::buildAggregateInitializer_nfi(expr_list, type);
    aggr_init = set_generated_file_info_to_unparse(aggr_init);
    ROSE_ASSERT(aggr_init != NULL);
    aggr_init->set_uses_compound_literal(true);
    initializer = aggr_init;
  }
#if DEBUG_construct_compound_literal
  printf("  initializer = %p = %s \n", initializer, initializer->class_name().c_str());
#endif

  a_dynamic_init_ptr dip = expr->variant.init.dynamic_init;
  ROSE_ASSERT(dip != nullptr);

  SgName name = string("compound_literal_") + StringUtility::numberToString(dip);
#if DEBUG_construct_compound_literal
  printf ("  name = %s\n", name.str());
#endif

  SgInitializedName * iname = SageBuilder::buildInitializedName_nfi(name, type, initializer);
  ROSE_ASSERT(iname != NULL);

  ROSE_ASSERT(iname->get_declptr() == NULL);
  ROSE_ASSERT(iname->get_prev_decl_item() == NULL);

  SgScopeStatement* scope = topScopeStack();
#if DEBUG_construct_compound_literal
  printf ("  scope = %p = %s\n", scope, scope->class_name().c_str());
#endif

  iname->set_scope(scope);
  iname->set_parent(scope);

  set_basic_file_info_for_expr(iname, expr);

  SgVariableSymbol* symbol = new SgVariableSymbol(iname);
  ROSE_ASSERT(symbol != nullptr);

#if DEBUG_construct_compound_literal
  printf("  vsym = %p\n", symbol);
#endif

  scope->insert_symbol(name,symbol);

  SgCompoundLiteralExp* compoundLiteralExp = SageBuilder::buildCompoundLiteralExp_nfi(symbol);
#if DEBUG_construct_compound_literal
  printf("Leave construct_compound_literal(%p)\n", expr);
#endif
  return compoundLiteralExp;
}

}
