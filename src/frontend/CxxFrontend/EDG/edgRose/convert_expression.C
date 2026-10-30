
#include "edgRose.h"

namespace EDG_ROSE_Translation {

// Debug entry point
#define DEBUG__convert_expression 0

// Debug switch cases
#define DEBUG__convert_expression__enk_constant            0
#define DEBUG__convert_expression__enk_variable            0
#define DEBUG__convert_expression__enk_field               0
#define DEBUG__convert_expression__enk_temp_init           0
#define DEBUG__convert_expression__enk_throw               0
#define DEBUG__convert_expression__enk_object_lifetime     0
#define DEBUG__convert_expression__enk_typeid              0
#define DEBUG__convert_expression__enk_sizeof              0
#define DEBUG__convert_expression__enk_alignof             0
#define DEBUG__convert_expression__enk_statement           0
#define DEBUG__convert_expression__enk_reuse_value         0
#define DEBUG__convert_expression__enk_type_operand        0
#define DEBUG__convert_expression__enk_builtin_operation   0
#define DEBUG__convert_expression__enk_lambda              0
#define DEBUG__convert_expression__enk_routine             0
#define DEBUG__convert_expression__enk_param_ref           0
#define DEBUG__convert_expression__enk_braced_init_list    0
#define DEBUG__convert_expression__enk_c11_generic         0
#define DEBUG__convert_expression__enk_sizeof_pack         0
#define DEBUG__convert_expression__enk_fold                0
#define DEBUG__convert_expression__enk_initializer         0
#define DEBUG__convert_expression__enk_builtin_choose_expr 0
#define DEBUG__convert_expression__enk_yield               0
#define DEBUG__convert_expression__enk_await               0


// GNU statement-expressions have both `statement_expression_bodies` and `statement_expression_cache`.
// It seems redundant but `statement_expression_cache` is not updated here...
#define FATAL__convert_expression__enk_statement__caching 0

// There is no builder for SgStatementExpression so the statement's parent is set manually
#define FIXME__convert_expression__enk_statement__parent 1

// Should probably have an original expression tree and a representation of C11 _Generic
#define FIXME__expression__enk_c11_generic__ignored_operand 1

// Forward declarations of cases that have been moved to their own files (named using the functions name)
SgExpression * convert_expression__enk_new_delete(an_expr_node_ptr e);
SgExpression * convert_expression__enk_operation(an_expr_node_ptr e);

static SgExpression * convert_expression__enk_constant(an_expr_node_ptr e) {
#if DEBUG__convert_expression__enk_constant
  printf ("  Enter convert_expression__enk_constant():\n");
#endif
  return convert_constant(e->variant.constant.ptr);
}

static SgExpression * convert_expression__enk_variable(an_expr_node_ptr expr) {
#if DEBUG__convert_expression__enk_variable
  printf ("  Enter convert_expression__enk_variable():\n");
  printf ("    is_this_parameter   = %s\n", expr->variant.variable.ptr->is_this_parameter ? "true" : "false");
  printf ("    is_compound_literal = %s\n", expr->variant.variable.ptr->is_compound_literal ? "true" : "false");
#endif
  SgExpression * result = nullptr;
  if (expr->variant.variable.ptr->is_this_parameter) {
    SgSymbol * xsym = convert_class_use(expr->variant.variable.ptr);
    ROSE_ASSERT(xsym != nullptr);
    result = SageBuilder::buildThisExp_nfi(xsym);
  } else {
    SgVariableSymbol * vsym = convert_variable_use(expr->variant.variable.ptr);
    ROSE_ASSERT(vsym != nullptr);
    result = SageBuilder::buildVarRefExp_nfi(vsym);
  }
  return result;
}

static SgExpression * convert_expression__enk_field(an_expr_node_ptr expr) {
#if DEBUG__convert_expression__enk_field
  printf ("  Enter convert_expression__enk_field():\n");
#endif
  SgVariableSymbol * vsym = convert_field_use(expr->variant.field.ptr);
  ROSE_ASSERT(vsym != nullptr);
  return SageBuilder::buildVarRefExp_nfi(vsym);
}

static SgExpression * convert_expression__enk_temp_init(an_expr_node_ptr expr) {
#if DEBUG__convert_expression__enk_temp_init
  printf ("  Enter convert_expression__enk_temp_init():\n");
  printf ("    is_functional_notation_cast = %s\n", expr->is_functional_notation_cast ? "true" : "false");
#if GNU_EXTENSIONS_ALLOWED
  printf ("    marked_as_gnu_extension = %s\n", expr->marked_as_gnu_extension ? "true" : "false");
#endif
  printf ("    is_compound_literal = %s\n", expr->variant.init.dynamic_init->is_compound_literal ? "true" : "false");
#endif
  ROSE_ASSERT(expr->variant.init.dynamic_init != NULL);
  ROSE_ASSERT(expr->type != nullptr);

  SgExpression * result = nullptr;
  if (expr->variant.init.dynamic_init->is_compound_literal) {
    result = convert_dynamic_initializer(expr->variant.init.dynamic_init, expr->type);
    ROSE_ASSERT(result != nullptr);
    SgInitializer * initializer = isSgInitializer(result);
    if (initializer != nullptr && expr->variant.init.dynamic_init->is_braced_initializer) {
      initializer->set_is_braced_initialized(true);
    }
    result = construct_compound_literal(expr, result);
  } else {
    result = convert_dynamic_initializer(expr->variant.init.dynamic_init, expr->type);
  }
  SgInitializer * initializer = isSgInitializer(result);
  if (initializer != nullptr && expr->variant.init.dynamic_init->is_braced_initializer) {
    initializer->set_is_braced_initialized(true);
  }
  return result;
}

static SgExpression * convert_expression__enk_throw(an_expr_node_ptr e) {
#if DEBUG__convert_expression__enk_throw
  printf ("  Enter convert_expression__enk_throw():\n");
#endif
  SgExpression * throw_expr = nullptr;
  SgThrowOp::e_throw_kind throw_kind = SgThrowOp::unknown_throw;
  if (e->variant.throw_info != nullptr) {
    throw_kind = SgThrowOp::throw_expression;
    a_throw_supplement_ptr tsp = e->variant.throw_info;
    throw_expr = convert_dynamic_initializer(tsp->dynamic_init, tsp->type);
    ROSE_ASSERT(throw_expr != nullptr);
  } else {
    throw_kind = SgThrowOp::rethrow;
  }
  SgExpression * result = SageBuilder::buildThrowOp(throw_expr, throw_kind);
  ROSE_ASSERT(throw_expr == nullptr || throw_expr->get_parent() == result);
  return result;
}

static SgExpression * convert_expression__enk_object_lifetime(an_expr_node_ptr e) {
#if DEBUG__convert_expression__enk_object_lifetime
  printf ("  Enter convert_expression__enk_object_lifetime():\n");
#endif
  ROSE_ASSERT(e->variant.object_lifetime.expr != NULL);
  return convert_expression(e->variant.object_lifetime.expr);
}


static SgExpression * convert_expression__enk_typeid(an_expr_node_ptr e) {
#if DEBUG__convert_expression__enk_typeid
  printf ("  Enter convert_expression__enk_typeid():\n");
#endif
  // TODO EDG 6.5 see change EDGcpfe/25453
  SgExpression * result = convert_expression(e->variant.typeid_info.type_with_opt_expr);
  return SageBuilder::buildTypeIdOp(result, nullptr);
}

static SgExpression * convert_expression__enk_sizeof(an_expr_node_ptr expr) {
#if DEBUG__convert_expression__enk_sizeof
  printf ("  Enter convert_expression__enk_sizeof():\n");
#endif
  SgSizeOfOp * result = nullptr;
  if (expr->variant.sizeof_info.is_type) {
    a_type_ptr type = expr->variant.sizeof_info.variant.type;
    ROSE_ASSERT(type != NULL);
    SgType* ty = convert_type(type);
    result = SageBuilder::buildSizeOfOp_nfi(ty);

    while (type->kind == tk_typeref) {
      type = type->variant.typeref.type;
      ROSE_ASSERT(type != NULL);
    }

    bool is_class_struct_union = (type->kind == tk_class || type->kind == tk_struct || type->kind == tk_union);               
    bool originally_unnamed = is_class_struct_union && type->variant.class_struct_union.originally_unnamed;
    if (originally_unnamed && !type->autonomous_primary_tag_decl) {
      result->set_sizeOfContainsBaseTypeDefiningDeclaration(true);
    }
  } else {
    an_expr_node_ptr sizeof_expr = expr->variant.sizeof_info.variant.expr;
    ROSE_ASSERT(sizeof_expr != NULL);
    result = SageBuilder::buildSizeOfOp_nfi(convert_expression(sizeof_expr));
    if (sizeof_expr->is_objectless_nonstatic_data_mem_ref == true) {
      result->set_is_objectless_nonstatic_data_member_reference(true);
    }
  }
  return result;
}

static SgExpression * convert_expression__enk_alignof(an_expr_node_ptr expr) {
#if DEBUG__convert_expression__enk_alignof
  printf ("  Enter convert_expression__enk_alignof():\n");
#endif
  SgAlignOfOp * result = nullptr;
  if (expr->variant.sizeof_info.is_type) {
    a_type_ptr type = expr->variant.sizeof_info.variant.type;
    ROSE_ASSERT(type != NULL);

    result = SageBuilder::buildAlignOfOp_nfi(convert_type(type));

    while (type->kind == tk_typeref) {
      type = type->variant.typeref.type;
      ROSE_ASSERT(type != NULL);
    }

    bool is_class_struct_union = (type->kind == tk_class || type->kind == tk_struct || type->kind == tk_union);               
    bool originally_unnamed = is_class_struct_union && type->variant.class_struct_union.originally_unnamed;
    if (originally_unnamed && !type->autonomous_primary_tag_decl) {
      result->set_alignOfContainsBaseTypeDefiningDeclaration(true);
    }
  } else {
    an_expr_node_ptr alignof_expr = expr->variant.sizeof_info.variant.expr;
    ROSE_ASSERT(alignof_expr != NULL);
    result = SageBuilder::buildAlignOfOp_nfi(convert_expression(alignof_expr));
  }
  return result;
}

#if GNU_EXTENSIONS_ALLOWED
static SgExpression * convert_expression__enk_statement(an_expr_node_ptr expr) {
#if DEBUG__convert_expression__enk_statement
  printf ("  Enter convert_expression__enk_statement():\n");
#endif
  a_statement_ptr stmt = expr->variant.statement;

#if DEBUG__convert_expression__enk_statement
  printf ("    stmt = %p\n", stmt);
#endif
#if FATAL__convert_expression__enk_statement__caching
  ROSE_ABORT();
#endif

  SgStatementExpression * result = NULL;
  if (statement_expression_cache.find(stmt) != statement_expression_cache.end()) {
#if DEBUG__convert_expression__enk_statement
    printf ("    Found in cache\n");
#endif
    result = statement_expression_cache[stmt];
  } else {
    SourceSequenceList sse(stmt->source_sequence_entry);
    std::vector<SgStatement*> inner_statement_list = parse_statement(sse);
#if DEBUG__convert_expression__enk_statement
    printf ("    inner_statement_list.size() = %zu \n", inner_statement_list.size());
#endif
    SgStatement* outer_statement = build_basic_block_if_multiple_statements(inner_statement_list);
    result = new SgStatementExpression(outer_statement);
#if !FIXME__convert_expression__enk_statement__parent
    outer_statement->set_parent(result);
#endif
  }

  setSourcePosition(result);
#if FIXME__convert_expression__enk_statement__parent
  result->get_statement()->set_parent(result);
#else
  ROSE_ASSERT(result->get_statement()->get_parent() == result);
#endif
  statement_expression_bodies.insert(stmt);
  return result;
}
#endif

static SgExpression * convert_expression__enk_reuse_value(an_expr_node_ptr expr) {
#if DEBUG__convert_expression__enk_reuse_value
  printf ("  Enter convert_expression__enk_reuse_value():\n");
#endif
  a_dynamic_init_ptr dip = expr->variant.reused_value_init;
  ROSE_ASSERT(dip != NULL);

  a_type_ptr type = expr->type;
  ROSE_ASSERT(type != nullptr);

  return convert_dynamic_initializer(dip, type);
}

static SgExpression * convert_expression__enk_type_operand(an_expr_node_ptr expr) {
#if DEBUG__convert_expression__enk_type_operand
  printf ("  Enter convert_expression__enk_type_operand():\n");
#endif
  SgExpression * result = nullptr;
  a_type_ptr type_operand_type = expr->variant.type_operand.type;
  ROSE_ASSERT(type_operand_type != NULL);

  SgType * type = convert_type(type_operand_type);
  ROSE_ASSERT(type != NULL);
  return SageBuilder::buildTypeExpression(type);
}

static SgExpression * convert_expression__enk_builtin_operation(an_expr_node_ptr expr) {
#if DEBUG__convert_expression__enk_builtin_operation
  printf ("  Enter convert_expression__enk_builtin_operation():\n");
#endif
  SgName fname = builtin_operation_function_name(expr->variant.builtin_operation.kind);
#if DEBUG__convert_expression__enk_builtin_operation
  printf("    fname = %s \n", fname.str());
#endif
  std::vector<SgNode *> operands;
  an_expr_node_ptr operand = expr->variant.builtin_operation.operands;
  while (operand != nullptr) {
#if DEBUG__convert_expression__enk_builtin_operation
    printf("    operand = %p\n", operand, summarize_expression_in_string(operand).c_str());
#endif
    if (operand->kind == enk_type_operand) {
      SgType* type_operand = convert_type(operand->variant.type_operand.type);
      ROSE_ASSERT(type_operand != NULL);
      operands.push_back(type_operand);
    } else {
      SgExpression * expression_operand = convert_expression(operand);
      ROSE_ASSERT(expression_operand != NULL);
      operands.push_back(expression_operand);
    }
    operand = operand->next;
  }
  SgExpression * result = SageBuilder::buildTypeTraitBuiltinOperator(fname, operands);
  setSourcePosition(result);
  return result;
}

static SgExpression * convert_expression__enk_lambda(an_expr_node_ptr expr) {
#if DEBUG__convert_expression__enk_lambda
  printf ("  Enter convert_expression__enk_lambda():\n");
#endif
  SgExpression * result = convert_lambda(expr->variant.init.source.lambda);
  ROSE_ASSERT(result != nullptr);
  set_file_info_for_expr(result, expr);
  return result;
}

static SgExpression * convert_expression__enk_routine(an_expr_node_ptr expr) {
#if DEBUG__convert_expression__enk_routine
  printf ("  Enter convert_expression__enk_routine():\n");
#endif
  a_routine_ptr routine = expr->variant.routine.ptr;
  a_name_reference_ptr name_reference = expr->variant.routine.name_reference;
#if DEBUG__convert_expression__enk_routine
  printf ("    routine        = %p\n", routine);
  printf ("    name_reference = %p\n", name_reference);
#endif
  SgExpression * result = nullptr;
  if (routine != nullptr) {
    SgSymbol * symbol = convert_function_use(routine);
    ROSE_ASSERT(symbol != NULL);
    result = buildReferenceExpressionFromSymbol(symbol);

  } else if (name_reference != nullptr && name_reference->special_kind == sfk_none) {
    a_type_ptr destructor_type = name_reference->variant.destructor_type;
    ROSE_ASSERT(destructor_type != NULL);

    SgType * roseType = convert_type(destructor_type);
    ROSE_ASSERT(roseType != NULL);

    result = new SgPseudoDestructorRefExp(roseType);
    setSourcePosition(result);
    result = set_generated_file_info_to_unparse(result);
  } else {
    printf ("Fatal: routine expression should either have a valid pointer or a name_reference to a vacuous destructor!\n");
    ROSE_ABORT();
  }

#if DEBUG__convert_expression__enk_routine
  printf ("    qualifier = %p\n", name_reference ? name_reference->qualifier : nullptr);
#endif
  if (name_reference != nullptr && name_reference->qualifier != nullptr) {
    SgMemberFunctionRefExp * memberFunctionRefExp = isSgMemberFunctionRefExp(result);
    if (memberFunctionRefExp != NULL) {
      memberFunctionRefExp->set_need_qualifier(true);
    }
  }
  return result;
}

static SgExpression * convert_expression__enk_param_ref(an_expr_node_ptr expr) {
#if DEBUG__convert_expression__enk_param_ref
  printf ("  Enter convert_expression__enk_param_ref():\n");
#endif
  SgType * type = convert_type(expr->type);
  ROSE_ASSERT(type != NULL);
#if DEBUG__convert_expression__enk_param_ref
  printf ("    type = %p = %s \n", type, type->class_name().c_str());
#endif

  SgFunctionParameterRefExp * result = SageBuilder::buildFunctionParameterRefExp(expr->variant.param_ref.param_num, expr->variant.param_ref.levels_up);
  result->set_parameter_type(type);

  return result;
}

static SgExpression * convert_expression__enk_braced_init_list(an_expr_node_ptr expr) {
#if DEBUG__convert_expression__enk_braced_init_list
  printf ("  Enter convert_expression__enk_braced_init_list():\n");
#endif
  SgExprListExp * expr_list = SageBuilder::buildExprListExp_nfi();
  an_expr_node_ptr braced_init_list = expr->variant.braced_init_list;
  while (braced_init_list != NULL) {
#if DEBUG__convert_expression__enk_braced_init_list
    printf ("  braced_init_list = %p\n", braced_init_list);
#endif
    SgExpression * elem = convert_expression(braced_init_list);
    ROSE_ASSERT(elem != NULL);

    expr_list->append_expression(elem);
    elem->set_parent(expr_list);

    braced_init_list = braced_init_list->next;
  }
  return SageBuilder::buildBracedInitializer(expr_list, nullptr);
}

static SgExpression * convert_expression__enk_c11_generic(an_expr_node_ptr e) {
#if DEBUG__convert_expression__enk_c11_generic
  printf ("  Enter convert_expression__enk_c11_generic():\n");
#endif

  an_expr_node_ptr result_exp = e->variant.c11_generic.result;
  ROSE_ASSERT(result_exp  != NULL);

  SgExpression * result = convert_expression(result_exp);
  ROSE_ASSERT(result != nullptr);

#if !FIXME__expression__enk_c11_generic__ignored_operand
  an_expr_node_ptr operand_exp = e->variant.c11_generic.operands;
  ROSE_ASSERT(operand_exp != NULL);
  ROSE_ABORT();
#endif

  return result;
}

static SgExpression * convert_expression__enk_sizeof_pack(an_expr_node_ptr expr) {
#if DEBUG__convert_expression__enk_sizeof_pack
  printf ("  Enter convert_expression__enk_sizeof_pack():\n");
#endif
  bool is_type              = expr->variant.sizeof_pack.is_type;
  bool is_template_template = expr->variant.sizeof_pack.is_template_template;
#if DEBUG__convert_expression__enk_sizeof_pack
  printf("  is_type              = %s \n", is_type ? "true" : "false");
  printf("  is_template_template = %s \n", is_template_template ? "true" : "false");
#endif
  SgExpression * result = nullptr;
  if (is_template_template) {
    a_template_ptr templ = expr->variant.sizeof_pack.variant.templ;
    ROSE_ASSERT(templ != NULL);

    printf ("Fatal: NIY: sizeof pack of template template parameter!\n");
    ROSE_ABORT();
  } else if (is_type) {
    a_type_ptr type = expr->variant.sizeof_pack.variant.type;
    ROSE_ASSERT(type != NULL);

    SgType* rose_type = convert_type(type);
    ROSE_ASSERT(rose_type != NULL);

    result = SageBuilder::buildSizeOfOp_nfi(rose_type);
  } else {
    an_expr_node_ptr sz_expr = expr->variant.sizeof_pack.variant.expr;
    ROSE_ASSERT(sz_expr != NULL);

    SgExpression* rose_expr = convert_expression(sz_expr);
    ROSE_ASSERT(rose_expr != NULL);

    result = SageBuilder::buildSizeOfOp_nfi(rose_expr);
  }
  return result;
}

static SgExpression * convert_expression__enk_fold(an_expr_node_ptr expr) {
#if DEBUG__convert_expression__enk_fold
  printf ("  Enter convert_expression__enk_fold():\n");
#endif
  SgExpression * operands = convert_expression(expr->variant.fold.operands);
  ROSE_ASSERT(operands != nullptr);
#if DEBUG__convert_expression__enk_fold
  printf("    operands = %p = %s\n", operands, operands->class_name().c_str());
#endif
  std::string operator_token_string = summarize_token_kind(expr->variant.fold.operator_token);
  bool is_left_associative = expr->variant.fold.left_associative;
#if DEBUG__convert_expression__enk_fold
  printf("    operator_token_string = %s\n", operator_token_string.c_str());
  printf("    is_left_associative = %s\n", is_left_associative ? "true" : "false");
#endif

  SgExpression * result = SageBuilder::buildFoldExpression(operands, operator_token_string, is_left_associative);
  ROSE_ASSERT(result != NULL);
  operands->set_parent(result);
  return result;
}

static SgExpression * convert_expression__enk_initializer(an_expr_node_ptr e) {
#if DEBUG__convert_expression__enk_initializer
  printf ("  Enter convert_expression__enk_initializer():\n");
#endif
  return convert_dynamic_initializer(e->variant.initializer.dyn_init, e->type);
}


static SgExpression * convert_expression__enk_builtin_choose_expr(an_expr_node_ptr e) {
#if DEBUG__convert_expression__enk_builtin_choose_expr
  printf ("  Enter convert_expression__enk_builtin_choose_expr():\n");
#endif
  return SageBuilder::buildChooseExpression();
}


static SgExpression * convert_expression__enk_yield(an_expr_node_ptr e) {
#if DEBUG__convert_expression__enk_yield
  printf ("  Enter convert_expression__enk_yield():\n");
#endif
  return SageBuilder::buildYieldExpression(nullptr);
}


static SgExpression * convert_expression__enk_await(an_expr_node_ptr e) {
#if DEBUG__convert_expression__enk_await
  printf ("  Enter convert_expression__enk_await():\n");
#endif
  return SageBuilder::buildAwaitExpression();
}

/////////////////
// Entry point //
/////////////////

SgExpression * convert_expression(an_expr_node_ptr expr) {
#if DEBUG__convert_expression
  printf("Enter convert_expression():\n");
  printf("  expr = %p = %s\n", expr, expr ? summarize_expression_in_string(expr).c_str() : "");
#endif

  if (expr == nullptr) {
#if DEBUG__convert_expression
    printf("Leave convert_expression(nullptr)\n");
#endif
    return set_generated_file_info_to_unparse(SageBuilder::buildNullExpression_nfi());
  }

  SgExpression * result = nullptr;
  switch ((an_expr_node_kind)(expr->kind)) {
    case enk_operation:           result = convert_expression__enk_operation(expr);           break;
    case enk_constant:            result = convert_expression__enk_constant(expr);            break;
    case enk_variable:            result = convert_expression__enk_variable(expr);            break;
    case enk_field:               result = convert_expression__enk_field(expr);               break;
    case enk_temp_init:           result = convert_expression__enk_temp_init(expr);           break;
    case enk_new_delete:          result = convert_expression__enk_new_delete(expr);          break;
    case enk_throw:               result = convert_expression__enk_throw(expr);               break;
    case enk_object_lifetime:     result = convert_expression__enk_object_lifetime(expr);     break;
    case enk_typeid:              result = convert_expression__enk_typeid(expr);              break;
    case enk_sizeof:              result = convert_expression__enk_sizeof(expr);              break;
    case enk_alignof:             result = convert_expression__enk_alignof(expr);             break;
    case enk_reuse_value:         result = convert_expression__enk_reuse_value(expr);         break;
    case enk_type_operand:        result = convert_expression__enk_type_operand(expr);        break;
    case enk_builtin_operation:   result = convert_expression__enk_builtin_operation(expr);   break;
    case enk_lambda:              result = convert_expression__enk_lambda(expr);              break;
    case enk_routine:             result = convert_expression__enk_routine(expr);             break;
    case enk_param_ref:           result = convert_expression__enk_param_ref(expr);           break;
    case enk_braced_init_list:    result = convert_expression__enk_braced_init_list(expr);    break;
    case enk_c11_generic:         result = convert_expression__enk_c11_generic(expr);         break;
    case enk_sizeof_pack:         result = convert_expression__enk_sizeof_pack(expr);         break;
    case enk_fold:                result = convert_expression__enk_fold(expr);                break;
    case enk_initializer:         result = convert_expression__enk_initializer(expr);         break;
    case enk_yield:               result = convert_expression__enk_yield(expr);               break;
    case enk_await:               result = convert_expression__enk_await(expr);               break;
#if GNU_EXTENSIONS_ALLOWED
    case enk_statement:           result = convert_expression__enk_statement(expr);           break;
#endif
#if BUILTIN_FUNCTIONS_ENABLED
    case enk_builtin_choose_expr: result = convert_expression__enk_builtin_choose_expr(expr); break;
#endif
    default:
      printf("Fatal: Unsupported expression kind %s\n", summarize_expression_in_string(expr).c_str());
      ROSE_ABORT();
  }

  ROSE_ASSERT(result != NULL);
  ROSE_ASSERT(result->get_startOfConstruct() != NULL);
  ROSE_ASSERT(result->get_endOfConstruct()   != NULL);
  ROSE_ASSERT(result->get_operatorPosition() != NULL);

  set_file_info_for_expr(result, expr);

  if (result->get_startOfConstruct()->isCompilerGenerated() == true) {
    ROSE_ASSERT ( (expr->kind != enk_field) || ((expr->kind == enk_field) && (expr->variant.field.ptr != NULL)) );
    if ((expr->kind == enk_field) && (expr->variant.field.ptr->source_corresp.decl_position.seq > 0)) {
#if DEBUG__convert_expression
      printf ("  Found case of field with source positon not set properly \n");
#endif
      set_file_info_for_corresp(result, expr->variant.field.ptr->source_corresp);
    }
  }

  if (expr->generated_default_arg) {
    ROSE_ASSERT(result->get_startOfConstruct()->isDefaultArgument() == true);
    ROSE_ASSERT(result->get_endOfConstruct  ()->isDefaultArgument() == true);
    ROSE_ASSERT(result->get_operatorPosition()->isDefaultArgument() == true);
  }

#if DEBUG__convert_expression
  printf ("  isDefaultArgument = %s \n", result->get_file_info()->isDefaultArgument() ? "true" : "false");
#endif

  if (result->get_file_info()->isDefaultArgument()) {
    SgExpression * original_expression_tree = result->get_originalExpressionTree();
    if (original_expression_tree != nullptr) {
      original_expression_tree->get_file_info()->setDefaultArgument();
    }
  }

#if DEBUG__convert_expression
  printf("  result = %p = %s \n", result, result->class_name().c_str());
  printf("  type   = %p = %s \n", result->get_type(), result->get_type()->class_name().c_str());
  printf("Leave convert_expression(%p)\n", expr);
#endif
  return result;
}
}