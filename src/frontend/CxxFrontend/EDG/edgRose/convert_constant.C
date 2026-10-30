
#include "edgRose.h"

namespace EDG_ROSE_Translation {

// Debug entry point
#define DEBUG__convert_constant 0

// Debug switch cases
#define DEBUG__convert_constant__ck_integer        0
#define DEBUG__convert_constant__ck_string         0
#define DEBUG__convert_constant__ck_float          0
#define DEBUG__convert_constant__ck_complex        0
#define DEBUG__convert_constant__ck_imaginary      0
#define DEBUG__convert_constant__ck_address        0
#define DEBUG__convert_constant__ck_ptr_to_member  0
#define DEBUG__convert_constant__ck_dynamic_init   0
#define DEBUG__convert_constant__ck_aggregate      0
#define DEBUG__convert_constant__ck_init_repeat    0
#define DEBUG__convert_constant__ck_template_param 0
#define DEBUG__convert_constant__ck_void           0
#define DEBUG__convert_constant__ck_upc_threads    0
#define DEBUG__convert_constant__ck_upc_mythread   0

// Debug original expression tree (constant unfolding)
#define DEBUG__convert_constant__attach_original_expression_tree 0

// Exceptions that might not be relevant for newer EDG versions
#define USING__convert_constant__attach_original_expression_tree__unnamed_struct_decl 1
#define USING__convert_constant__attach_original_expression_tree__typeid 1

// Work around potential issues with deleting subtrees
#define FIXME__convert_constant__attach_original_expression_tree__delete_subtree 1

// Causes being unable to attach an original expression to be a fatal error
#define FATAL__convert_constant__attach_original_expression_tree__unknown_target 0

static SgExpression * convert_constant__ck_integer(a_constant_ptr expr, a_type_ptr actual_type) {
#if DEBUG__convert_constant__ck_integer
  printf("  Enter convert_constant__ck_integer:");
  printf("    integer constant %lld '%s'\n", (long long)(expr->variant.integer_value), expr->int_value_lexeme);
#endif
  ROSE_ASSERT(actual_type != nullptr);

  SgExpression * result = nullptr;

  bool actual_type_is_reference_type = is_reference_type(actual_type);
  bool cst_is_implicitly_cast_integral_constant = is_implicitly_cast_integral_constant(expr);
#if DEBUG__convert_constant__ck_integer
  printf("    actual_type_is_reference_type = %s\n", actual_type_is_reference_type ? "true" : "false");
  printf("    is_implicitly_cast_integral_constant(expr) = %s \n", cst_is_implicitly_cast_integral_constant ? "true" : "false");
  printf("    expr->explicit_cast_applied = %s\n", expr->explicit_cast_applied ? "true" : "false");
#endif
    
  if (actual_type_is_reference_type) {
    result = SageBuilder::buildIntVal_nfi(expr->variant.integer_value, expr->int_value_lexeme);
    result = set_file_info_for_corresp(result, expr->source_corresp);
    ROSE_ASSERT(actual_type != NULL);
    SgType* constantType = convert_type(actual_type);

    SgCastExp::cast_type_enum cast_type = typeOfCast(expr);
    ROSE_ASSERT(cast_type == SgCastExp::e_C_style_cast);

    result = SageBuilder::buildCastExp(result, constantType, SgCastExp::e_C_style_cast);

  } else if (cst_is_implicitly_cast_integral_constant) {
    result = SageBuilder::buildIntVal_nfi(expr->variant.integer_value, expr->int_value_lexeme);
    result = set_file_info_for_corresp(result, expr->source_corresp);

    ROSE_ASSERT(actual_type != NULL);
    SgType* constantType = convert_type(actual_type);
    result = SageBuilder::buildCastExp(result, constantType, typeOfCast(expr));

  } else if (actual_type->kind == tk_integer) {
    result = convert_integer_constant(expr, actual_type, expr->variant.integer_value, expr->int_value_lexeme, false);
    result = set_file_info_for_corresp(result, expr->source_corresp);

  } else if (expr->nullptr_keyword) {
    result = SageBuilder::buildNullptrValExp_nfi();
    result = set_file_info_for_corresp(result, expr->source_corresp);

  } else {
    result = SageBuilder::buildIntVal_nfi(expr->variant.integer_value, expr->int_value_lexeme);
    result = set_file_info_for_corresp(result, expr->source_corresp);
    ROSE_ASSERT(actual_type != NULL);
    SgType * constantType = convert_type(actual_type);
    result = SageBuilder::buildCastExp(result, constantType, SgCastExp::e_C_style_cast);
  }

  ROSE_ASSERT(result != NULL);
  ROSE_ASSERT(result->get_file_info() != NULL);
  if (result->get_file_info()->get_filenameString() == "") {
    printf("    Error: result->get_file_info()->get_filenameString() = %s \n",result->get_file_info()->get_filenameString().c_str());
  }
  ROSE_ASSERT(result->get_file_info()->get_filenameString() != "");
#if DEBUG__convert_constant__ck_integer
  printf("    result = %p = %s \n", result, result->class_name().c_str());
#endif
  return result;
}

static SgExpression * convert_constant__ck_string(a_constant_ptr expr, a_type_ptr actual_type) {
#if DEBUG__convert_constant__ck_string
  printf("  Enter convert_constant__ck_string:");
#endif
  SgStringVal* stringValue = SageBuilder::buildStringVal_nfi(convert_string_constant(expr));
  ROSE_ASSERT(stringValue != NULL);

  ROSE_ASSERT(stringValue->get_startOfConstruct() != NULL);
               ROSE_ASSERT(stringValue->get_endOfConstruct()   != NULL);
  ROSE_ASSERT(stringValue->get_operatorPosition() != NULL);
#if DEBUG__convert_constant__ck_string
  printf("    Before evaluation: stringValue->get_wcharString() = %s \n",stringValue->get_wcharString() ? "true" : "false");
  printf("       expr->type->kind = %s \n",summarize_type_kind_in_string(expr->type));
#endif
  if (expr->type->kind == tk_array) {
    bool is_variable_or_dependent_size_array = expr->type->variant.array.is_variable_size_array || expr->type->variant.array.is_template_dependent_size_array;
    if (is_variable_or_dependent_size_array) {
      printf("    Fatal: case ck_string: unexpected case \n");
      if (expr->type->variant.array.is_variable_size_array == true) {
        printf("      array: element_count_expr = %p \n", expr->type->variant.array.variant.element_count_expr);
      } else if (expr->type->variant.array.is_template_dependent_size_array == true) {
        printf("      array: element_count_constant = %p \n", expr->type->variant.array.variant.element_count_constant);
      }
      ROSE_ABORT();
    }
    a_type_ptr referenced_element_type = expr->type->variant.array.element_type;

    if (referenced_element_type->kind == tk_typeref) {
      referenced_element_type = referenced_element_type->variant.typeref.type;
      ROSE_ASSERT(referenced_element_type != NULL);
      ROSE_ASSERT(referenced_element_type->kind != tk_typeref);
    }
    ROSE_ASSERT(referenced_element_type != NULL);
    bool is_wide_character = isWideChar(referenced_element_type);
#if DEBUG__convert_constant__ck_string
    printf("    is_wide_character = %s \n",is_wide_character ? "true" : "false");
#endif
    if (is_wide_character) {
      stringValue->set_wcharString(true);
    }

    bool is_16bit_wide_character = is16BitWideChar(referenced_element_type);
#if DEBUG__convert_constant__ck_string
    printf("    is_16bit_wide_character = %s \n",is_16bit_wide_character ? "true" : "false");
#endif
    if (is_16bit_wide_character) {
      stringValue->set_is16bitString(true);
    }

    bool is_32bit_wide_character = is32BitWideChar(referenced_element_type);
#if DEBUG__convert_constant__ck_string
    printf("    is_32bit_wide_character = %s \n",is_32bit_wide_character ? "true" : "false");
#endif
    if (is_32bit_wide_character) {
      stringValue->set_is32bitString(true);
    }

    bool is_raw_string = (expr->variant.string.literal_kind & SCLK_RAW_STRING_LITERAL) != 0;
#if DEBUG__convert_constant__ck_string
    printf("    is_raw_string = %s \n",is_raw_string ? "true" : "false");
#endif
    if (is_raw_string) {
      stringValue->set_isRawString(true);
      std::string raw_string(expr->variant.string.value, expr->variant.string.length);
#if DEBUG__convert_constant__ck_string
      printf("    raw_string = %s \n",raw_string.c_str());
#endif
      if (!raw_string.empty() && raw_string[raw_string.size() - 1] == '\0') {
#if DEBUG__convert_constant__ck_string
        printf("    Removing the  trailing null character in the string copied from EDG \n");
#endif
        raw_string.resize(raw_string.size() - 1); // Remove null byte
      }

      stringValue->set_raw_string_value(raw_string);
    }
  }
#if DEBUG__convert_constant__ck_string
  printf("    stringValue->get_wcharString() = %s \n",stringValue->get_wcharString() ? "true" : "false");
#endif
  return set_file_info_for_corresp(stringValue, expr->source_corresp);
}

static SgExpression * convert_constant__ck_float(a_constant_ptr expr, a_type_ptr actual_type) {
#if DEBUG__convert_constant__ck_float
   printf("  Enter convert_constant__ck_float:\n");
#endif
  ROSE_ASSERT(actual_type != NULL);
  ROSE_ASSERT (actual_type->kind == tk_float);
  a_host_fp_value val = fetch_host_fp_value(actual_type->variant.float_kind, &expr->variant.float_value);
#if DEBUG__convert_constant__ck_float
  printf("   expr = %p float constant %Lf '%s'\n", expr, (long double)val, expr->float_value_lexeme);
#endif
  SgExpression * result = convert_float_constant(actual_type, val, expr->float_value_lexeme);
  return set_file_info_for_corresp(result, expr->source_corresp);
}

static SgExpression * convert_constant__ck_complex(a_constant_ptr expr, a_type_ptr actual_type) {
#if DEBUG__convert_constant__ck_complex
  printf("  Enter convert_constant__ck_complex:\n");
#endif
  ROSE_ASSERT(actual_type != NULL);
  ROSE_ASSERT(actual_type->kind == tk_complex);

  a_host_fp_value edg_val_real = fetch_host_fp_value(actual_type->variant.float_kind, &expr->variant.complex_value->real);
  SgValueExp * rose_val_real = convert_float_constant(actual_type, edg_val_real, "");
  rose_val_real = set_generated_file_info_to_unparse(rose_val_real);

  a_host_fp_value edg_val_imag = fetch_host_fp_value(actual_type->variant.float_kind, &expr->variant.complex_value->imag);
  SgValueExp * rose_val_imag = convert_float_constant(actual_type, edg_val_imag, "");
  rose_val_imag = set_generated_file_info_to_unparse(rose_val_imag);

  SgExpression * result = SageBuilder::buildComplexVal_nfi(rose_val_real, rose_val_imag, expr->float_value_lexeme);
  return set_file_info_for_corresp(result, expr->source_corresp);
}

static SgExpression * convert_constant__ck_imaginary(a_constant_ptr expr, a_type_ptr actual_type) {
#if DEBUG__convert_constant__ck_imaginary
  printf("  WEnter ck_imaginary:\n");
#endif
  ROSE_ASSERT(actual_type != NULL);
  ROSE_ASSERT (actual_type->kind == tk_imaginary);
  a_host_fp_value val_imag = fetch_host_fp_value(actual_type->variant.float_kind, &expr->variant.float_value);
  SgValueExp * rose_val_imag = convert_float_constant(actual_type, val_imag, "");
  rose_val_imag = set_generated_file_info_to_unparse(rose_val_imag);
  SgExpression * result = SageBuilder::buildImaginaryVal_nfi(rose_val_imag, expr->float_value_lexeme);
  return set_file_info_for_corresp(result, expr->source_corresp);
}

static SgExpression * convert_constant__ck_address(a_constant_ptr expr, a_type_ptr actual_type) {
#if DEBUG__convert_constant__ck_address
  printf("  Enter ck_address:\n");
#endif
  an_address_base_kind kind = (an_address_base_kind)(expr->variant.address.kind);
#if DEBUG__convert_constant__ck_address
  printf("    kind = %s \n", summarize_address_base_kind_in_string(kind));
  printf("    offset = %ld\n", (long)expr->variant.address.offset);
  printf("    implicit_cast = %s\n", expr->implicit_cast ? "true" : "false");
#endif
  SgExpression * result = nullptr;
  switch (kind) {
    case abk_routine: {
#if DEBUG__convert_constant__ck_address
      printf("    case of abk_routine:\n");
#endif
      a_routine_ptr rout = expr->variant.address.variant.routine;
      SgSymbol* symbol =  convert_function_use(rout);
      ROSE_ASSERT(symbol != NULL);
      SgFunctionSymbol* functionSymbol = isSgFunctionSymbol(symbol);
      if (functionSymbol != NULL) {
        result = SageBuilder::buildFunctionRefExp_nfi(functionSymbol);
        result = set_generated_file_info_to_unparse(result);
      } else {
        printf("      Fatal: Case of %s not implemented! (symbol = %p)\n", symbol->class_name().c_str(), symbol);
        ROSE_ABORT();
      }
      break;
    }

    case abk_variable: {
#if DEBUG__convert_constant__ck_address
      printf("    case abk_variable:\n");
#endif
      a_variable_ptr var = expr->variant.address.variant.variable;
#if DEBUG__convert_constant__ck_address
      printf("      var = %p \n",var);
#endif
      bool skip_taking_address = (var->type->kind == tk_array);
#if DEBUG__convert_constant__ck_address
      printf("      skip_taking_address = %s \n",skip_taking_address ? "true" : "false");
#endif
      a_targ_ptrdiff_t offset_in_elements = 0;
      if (var->type->kind == tk_array) {
#if DEBUG__convert_constant__ck_address
        printf("      case of var->type->kind == tk_array \n");
#endif
        a_targ_size_t element_size = f_skip_typerefs(var->type->variant.array.element_type)->size;
        ROSE_ASSERT (element_size != 0);
        offset_in_elements = expr->variant.address.offset / (signed long)element_size;
      } else {
#if DEBUG__convert_constant__ck_address
        printf("      expr->variant.address.offset = %" PRIuPTR " \n",(size_t)(expr->variant.address.offset));
#endif
        offset_in_elements = expr->variant.address.offset;
      }
      SgVariableSymbol* variableSymbol = convert_variable_use(var);
      ROSE_ASSERT(variableSymbol != NULL);
#if DEBUG__convert_constant__ck_address
      printf("      variableSymbol = %p \n", variableSymbol);
#endif
      SgExpression* raw_expr = NULL;
      if (var->is_compound_literal) {
#if DEBUG__convert_constant__ck_address
        printf("      Found case of compound literal and need to handle this case! \n");
#endif
        SgCompoundLiteralExp* compoundLiteral = buildCompoundLiteralExp_nfi(variableSymbol);

        SgInitializedName* iname = variableSymbol->get_declaration();
        ROSE_ASSERT(iname != NULL);
#if DEBUG__convert_constant__ck_address
        SgNode* iname_p = iname->get_parent();
        printf("      iname_p = %p = %s \n", iname_p, iname_p ? iname_p->class_name().c_str() : "");
#endif
        iname->set_parent(compoundLiteral);

        raw_expr = set_generated_file_info_to_unparse(compoundLiteral);
        ROSE_ASSERT(raw_expr != NULL);
      } else {
        SgVarRefExp * varRefExp = buildVarRefExp_nfi(variableSymbol);
        ROSE_ASSERT(varRefExp != NULL);
#if DEBUG__convert_constant__ck_address
        printf("      varRefExp = %p \n", varRefExp);
#endif
        raw_expr = set_generated_file_info_to_unparse(varRefExp);
        ROSE_ASSERT(raw_expr != NULL);
      }

#if DEBUG__convert_constant__ck_address
      printf("      raw_expr = %p = %s\n", raw_expr, raw_expr->class_name().c_str());
      printf("      offset_in_elements = %d\n", (int)(offset_in_elements));
#endif
      if (offset_in_elements > 0) {
        SgIntVal * offset = SageBuilder::buildIntVal_nfi(offset_in_elements, "");
        offset = set_generated_file_info_to_unparse(offset);
        raw_expr = SageBuilder::buildAddOp_nfi(raw_expr, offset);
        raw_expr = set_generated_file_info_to_unparse(raw_expr);
      } else if (offset_in_elements < 0) {
        SgIntVal * offset = SageBuilder::buildIntVal_nfi(-offset_in_elements, "");
        offset = set_generated_file_info_to_unparse(offset);
        raw_expr = SageBuilder::buildSubtractOp_nfi(raw_expr, offset);
        raw_expr = set_generated_file_info_to_unparse(raw_expr);
      }
#if DEBUG__convert_constant__ck_address
      printf("      expr->type->kind = %s\n", summarize_type_kind_in_string(expr->type));
      printf("      expr->type->kind == tk_pointer = %s \n", expr->type->kind == tk_pointer ? "true" : "false");
      printf("      expr->implicit_cast = %s \n", expr->implicit_cast ? "true" : "false");
      if (expr->type->kind == tk_pointer) {
        printf("      expr->type->variant.pointer.is_reference = %s \n", expr->type->variant.pointer.is_reference ? "true" : "false");
      }
#endif
      if (expr->implicit_cast == false) {
        if (expr->type->kind == tk_pointer && expr->type->variant.pointer.is_reference == true) {
          result = raw_expr;
        } else {
          SgExpression * referenceOperator = SageBuilder::buildAddressOfOp_nfi(raw_expr);
          referenceOperator = set_file_info_for_corresp(referenceOperator, expr->source_corresp);
          result = referenceOperator;
        }
      } else {
        result = raw_expr;
      }
#if DEBUG__convert_constant__ck_address
      printf("      raw_expr = %p = %s\n", raw_expr, raw_expr->class_name().c_str());
      printf("      result = %p = %s\n", result, result->class_name().c_str());
#endif
      break;
    }

    case abk_temporary:
    case abk_constant: {
      a_constant_ptr inner_p = expr->variant.address.variant.constant;
      a_constant_repr_kind inner_kind = (a_constant_repr_kind)(inner_p->kind);
#if DEBUG__convert_constant__ck_address
      printf("      inner_kind = %s \n",summarize_constant_kind_in_string(inner_p));
#endif
      switch (inner_kind) {
        case ck_string: {
          result = set_generated_file_info_to_unparse(buildStringVal_nfi(convert_string_constant(inner_p)));
          break;
        }

        case ck_template_param: {
          result = convert_expression(inner_p->variant.template_param.variant.expr);
          break;
        }

        case ck_aggregate: {
#if DEBUG__convert_constant__ck_address
          printf("      case ck_aggregate \n");
          printf("        expr->explicit_braces_on_aggregate = %s \n", expr->explicit_braces_on_aggregate ? "true" : "false");
          printf("        ck_aggregate: first_constant = %p \n", inner_p->variant.aggregate.first_constant);
          printf("        ck_aggregate: last_constant  = %p \n", inner_p->variant.aggregate.last_constant);
#endif
          SgExprListExp* exprListExp = SageBuilder::buildExprListExp_nfi();
          ROSE_ASSERT(exprListExp != NULL);

          a_constant_ptr temp_constant = inner_p->variant.aggregate.first_constant;
          while (temp_constant != NULL) {
#if DEBUG__convert_constant__ck_address
            printf("      temp_constant = %p \n",temp_constant);
#endif
            SgExpression* constantExpression = convert_constant(temp_constant);
            ROSE_ASSERT(constantExpression != NULL);
            constantExpression->set_parent(exprListExp);

            exprListExp->append_expression(constantExpression);

            temp_constant = temp_constant->next;
          }
          result = exprListExp;
          break;
        }

        case ck_integer: {
          if (inner_p->is_simple_zero) {
            result = SageBuilder::buildUnsignedLongVal_nfi(0,"0");
          } else {
            result = SageBuilder::buildUnsignedLongVal_nfi(inner_p->variant.integer_value,"");
          }
          break;
        }

        case ck_ptr_to_member:
        case ck_dynamic_init:
        case ck_error:
        case ck_float:
        case ck_complex:
        case ck_imaginary:
        case ck_address:
        case ck_init_repeat:
        case ck_designator:
#if FIXED_POINT_ALLOWED
        case ck_fixed_point:
#endif
#if GNU_EXTENSIONS_ALLOWED
        case ck_label_difference:
#endif
#if UPC_EXTENSIONS_ALLOWED
        case ck_upc_threads:
        case ck_upc_mythread:
#endif
        case ck_last:
        default: {
          printf("      Fatal: inner_kind = %d = %s \n", inner_kind, summarize_constant_kind_in_string(inner_p));
          ROSE_ABORT();
        }
      }
      break;
    }

    case abk_typeid: {
#if DEBUG__convert_constant__ck_address
      printf("    case abk_typeid:\n");
#endif
      SgExpression * operand_expr = NULL;
      SgType * operand_type       = NULL;

      ROSE_ASSERT(expr->expr != NULL);
      operand_expr = convert_expression(expr->expr);
      ROSE_ASSERT(operand_expr != NULL);
#if DEBUG__convert_constant__ck_address
      printf("      operand_expr = %p = %s\n", operand_expr, operand_expr->class_name().c_str());
#endif
      ROSE_ASSERT(expr->variant.address.variant.type != NULL);
      operand_type = convert_type(expr->variant.address.variant.type);
      ROSE_ASSERT(operand_type != NULL);
#if DEBUG__convert_constant__ck_address
      printf("      operand_type = %p = %s\n", operand_type, operand_type->class_name().c_str());
#endif

      // FIXME: ignoring the type
      result = operand_expr;
      break;
    }

    case abk_label: {
      a_label_ptr label = expr->variant.address.variant.label;
      ROSE_ASSERT(label != NULL);
#if DEBUG__convert_constant__ck_address
      printf("      label = %p\n", label);
#endif
      SgLabelStatement* labelStatement = convert_label_use(label);
      ROSE_ASSERT(labelStatement != NULL);

      SgScopeStatement* labelScope = labelStatement->get_scope();
      ROSE_ASSERT(labelScope != NULL);

      ROSE_ASSERT(labelStatement->get_name().is_null() == false);

      SgSymbol* symbol = labelScope->lookup_symbol(labelStatement->get_name());
      ROSE_ASSERT(symbol != NULL);

      SgLabelSymbol* labelSymbol = isSgLabelSymbol(symbol);
      ROSE_ASSERT(labelSymbol != NULL);

      SgLabelRefExp* labelRefExp = SageBuilder::buildLabelRefExp(labelSymbol);
      ROSE_ASSERT(labelRefExp != NULL);

      result = labelRefExp;
      break;
    }

    case abk_uuidof:
#if MICROSOFT_EXTENSIONS_ALLOWED
    case abk_cli_typeid:
    case abk_cli_array:
#endif /* MICROSOFT_EXTENSIONS_ALLOWED */
    case abk_last:
    default: {
      printf("      case not handled! kind %d\n", kind);
      ROSE_ABORT();
    }
  }
  return result;
}

static SgExpression * convert_constant__ck_ptr_to_member(a_constant_ptr expr, a_type_ptr actual_type) {
#if DEBUG__convert_constant__ck_ptr_to_member
  printf("  Enter ck_ptr_to_member:\n");
#endif
  SgExpression * result = nullptr;
  bool is_cxx11_and_up = il_header.source_language == sl_Cplusplus && il_header.std_version >= 201103;
  if (expr->variant.ptr_to_member.is_function_ptr == true) {
    auto ptr_to_member__routine = expr->variant.ptr_to_member.variant.routine;
#if DEBUG__convert_constant__ck_ptr_to_member
    printf("    expr->variant.ptr_to_member.variant.routine    = %p \n", ptr_to_member__routine);
#endif
    if (ptr_to_member__routine == NULL) {
      SgExpression * res_expr = is_cxx11_and_up ?
          (SgExpression *)SageBuilder::buildNullptrValExp_nfi() :
          (SgExpression *)SageBuilder::buildIntVal_nfi(0,"0L");
      result = set_file_info_for_corresp(res_expr, expr->source_corresp);
    } else {
      ROSE_ASSERT(ptr_to_member__routine != NULL);
      SgSymbol* symbol = function_cache[ptr_to_member__routine];
#if DEBUG__convert_constant__ck_ptr_to_member
      printf("    symbol = %p \n",symbol);
#endif
      if (symbol == NULL) {
        SgFunctionDeclaration* previouslyUnseenFunction = convert_routine_prototype(ptr_to_member__routine, false, nullptr);
        ROSE_ASSERT (previouslyUnseenFunction != NULL);
        symbol = function_cache[ptr_to_member__routine];
      }
      ROSE_ASSERT (symbol != NULL);
#if DEBUG__convert_constant__ck_ptr_to_member
      printf("    symbol = %p = %s\n", symbol, symbol->class_name().c_str());
#endif
      SgMemberFunctionSymbol* memberFunctionSymbol = isSgMemberFunctionSymbol(symbol);
      ROSE_ASSERT(memberFunctionSymbol != NULL);

      SgExpression * res_expr = SageBuilder::buildMemberFunctionRefExp_nfi(memberFunctionSymbol, false, true);
      res_expr = SageBuilder::buildAddressOfOp_nfi(res_expr);
      result = set_file_info_for_corresp(res_expr, expr->source_corresp);
    }
  } else {
#if DEBUG__convert_constant__ck_ptr_to_member
    printf("    expr->variant.ptr_to_member.variant.field = %p \n", expr->variant.ptr_to_member.variant.field);
#endif
    if (expr->variant.ptr_to_member.variant.field == NULL) {
#if DEBUG__convert_constant__ck_ptr_to_member
      printf("    Case of NULL value for pointer \n");
#endif
      SgExpression * res_expr = is_cxx11_and_up ?
          (SgExpression *)SageBuilder::buildNullptrValExp_nfi() :
          (SgExpression *)SageBuilder::buildIntVal_nfi(0,"0L");
      result = set_file_info_for_corresp(res_expr, expr->source_corresp);
    } else {
#if DEBUG__convert_constant__ck_ptr_to_member
      printf("    Case of non-null pointer to member data \n");
#endif
      SgExpression * res_expr = SageBuilder::buildVarRefExp_nfi(convert_field_use(expr->variant.ptr_to_member.variant.field));
      res_expr = set_file_info_for_corresp(res_expr, expr->source_corresp);
      res_expr = SageBuilder::buildAddressOfOp_nfi(res_expr);
      result = set_file_info_for_corresp(res_expr, expr->source_corresp);
    }
  }

#if DEBUG__convert_constant__ck_ptr_to_member
  printf("    expr->variant.ptr_to_member.cast_to_base       = %s \n", expr->variant.ptr_to_member.cast_to_base ? "true" : "false");
#endif
  if (expr->variant.ptr_to_member.cast_to_base == true) {
    ROSE_ASSERT(expr->variant.ptr_to_member.casting_base_class != NULL);
    SgType* class_type = convert_type(expr->variant.ptr_to_member.casting_base_class->type);
    SgCastExp::cast_type_enum cast_type = typeOfCast(expr);
    ROSE_ASSERT(cast_type == SgCastExp::e_C_style_cast);

    ROSE_ASSERT(class_type != NULL);
    SgCastExp* castExp = buildCastExp_nfi(result, class_type, SgCastExp::e_C_style_cast);

    ROSE_ASSERT(castExp != NULL);
    result = castExp;
  }

#if DEBUG__convert_constant__ck_ptr_to_member
  printf("    result = %p = %s \n", result, result->class_name().c_str());
#endif
  return result;
}

static SgExpression * convert_constant__ck_dynamic_init(a_constant_ptr expr, a_type_ptr actual_type) {
#if DEBUG__convert_constant__ck_dynamic_init
  printf("  Enter ck_dynamic_init:\n");
#endif
  a_type_ptr type = expr->type;
  ROSE_ASSERT(type != NULL);
  return convert_dynamic_initializer(expr->variant.dynamic_init.ptr, type);
}

static SgExpression * convert_constant__ck_aggregate(a_constant_ptr expr, a_type_ptr actual_type) {
#if DEBUG__convert_constant__ck_aggregate
  printf("  Enter convert_constant__ck_aggregate:\n");
#endif
  SgExpression * result = nullptr;
  SgExprListExp* contents = SageBuilder::buildExprListExp_nfi();
  contents = set_generated_file_info_to_unparse(contents);
#if DEBUG__convert_constant__ck_aggregate
  printf("    contents = %p = %s \n", contents, contents->class_name().c_str());
#endif

  bool compilerGenerated = (expr->source_corresp.decl_position.seq == 0);
#if DEBUG__convert_constant__ck_aggregate
  printf("    compilerGenerated = %s \n", compilerGenerated ? "true" : "false");
#endif

  std::vector<SgExpression*> designators;
#if DEBUG__convert_constant__ck_aggregate
  printf("    expr->variant.aggregate.first_constant = %p\n", expr->variant.aggregate.first_constant);
  printf("    expr->variant.aggregate.last_constant  = %p\n", expr->variant.aggregate.last_constant);
#endif

  bool constant_resolved_to_empty_list = false;
  if (expr->variant.aggregate.first_constant == NULL) {
#if DEBUG__convert_constant__ck_aggregate
    printf("    Found case of empty list:\n");
    printf("      expr->expr = %p \n", expr->expr);
#endif
    if (expr->expr != NULL) {
      SgExpression* unfoldedExpression = convert_expression(expr->expr);
      ROSE_ASSERT(unfoldedExpression != NULL);

      appendExpression(contents, unfoldedExpression);
      constant_resolved_to_empty_list = true;
    }
  }

  for (a_constant_ptr c = expr->variant.aggregate.first_constant; c != NULL; c = c->next) {
    SgExpression * element = NULL;
    bool compilerGenerated_inloop = (c->source_corresp.decl_position.seq == 0);
#if DEBUG__convert_constant__ck_aggregate
    printf("    LOOP:\n");
    printf("      c = %p\n", c);
    printf("      c->kind = %s\n", summarize_constant_kind_in_string(c));
    printf("      c->next = %p\n", c->next);

 // DQ (7/13/2025): Added debugging code...
    if (c->next != NULL)
       {
         printf("      c->next->kind = %s\n", summarize_constant_kind_in_string(c->next));
       }

    printf("      designators.size() = %" PRIuPTR " \n", designators.size());
    printf("      compilerGenerated_inloop = %s = %d\n",compilerGenerated_inloop ? "true" : "false", c->source_corresp.decl_position.seq );
#endif
    switch (c->kind) {
      case ck_designator: {
        // When kind == ck_designator:
        //    A ck_designator is only used in initialization, and as such is always an unshared constant.  
        //    The designated field or element is initialized by the constant pointed to by "next".
#if DEBUG__convert_constant__ck_aggregate
        printf("      case ck_designator:\n");
        printf("        c->explicit_braces_on_aggregate = %s \n",c->explicit_braces_on_aggregate ? "true" : "false");
        printf("        c->variant.designator.is_generic          = %s \n",c->variant.designator.is_generic ? "true" : "false");
        printf("        c->variant.designator.is_field_designator = %s \n",c->variant.designator.is_field_designator ? "true" : "false");
#endif
        if (c->variant.designator.is_generic == false) {
          if (c->variant.designator.is_field_designator == true) {
            ROSE_ASSERT(c->variant.designator.variant.field != NULL);
            designators.push_back(buildVarRefExp_nfi(convert_field_use(c->variant.designator.variant.field)));
          } else {
            designators.push_back(buildUnsignedLongVal_nfi(c->variant.designator.variant.array_element,""));
            designators.back()->set_parent(contents);
          }
        } else {
          ROSE_ASSERT (c->variant.designator.is_generic == true);
          if (c->variant.designator.is_field_designator == true) {
            // When is_field_designator == TRUE and is_generic == TRUE:
            //    a_const_char *field_name;
            // The name of the designated field.
            ROSE_ASSERT(c->variant.designator.variant.field_name != NULL);
            printf("        c->variant.designator.variant.field_name = %s \n",c->variant.designator.variant.field_name);

#if DEBUG__convert_constant__ck_aggregate
            summarize_constant(c);
#endif
            ROSE_ABORT();
          } else {
            ROSE_ASSERT(c->variant.designator.variant.subscript != NULL);
            printf("        c->variant.designator.variant.subscript = %p \n",c->variant.designator.variant.subscript);
            ROSE_ABORT();
          }
        }

        set_generated_file_info_to_unparse(designators.back());

        ROSE_ASSERT(c->next != NULL);

        SgExpression * constantExpression = convert_constant(c->next);
        constantExpression->set_parent(contents);

        SgInitializer * element_init = wrap_expr_as_initializer(constantExpression);

        SgExprListExp * el = set_generated_file_info_to_unparse(buildExprListExp_nfi());
#if DEBUG__convert_constant__ck_aggregate
        printf("        el = %p = %s \n",el,el->class_name().c_str());
#endif

        appendExpressionList(el, designators);
        SgDesignatedInitializer * designatedInitializer = new SgDesignatedInitializer(el, element_init); /* FIXME: need build function */
        ROSE_ASSERT(designatedInitializer != NULL);

        setSourcePosition(designatedInitializer);

        element = designatedInitializer;
        c = c->next;
        break;
      }
      case ck_dynamic_init: {
#if DEBUG__convert_constant__ck_aggregate
        printf("      case ck_dynamic_init\n");
#endif
        element = convert_constant(c);
        break;
      }
      case ck_aggregate: {
#if DEBUG__convert_constant__ck_aggregate
        printf("      case ck_aggregate\n");
#endif
        element = convert_constant(c);
        ROSE_ASSERT(element != NULL);
        break;
      }
      case ck_init_repeat: {
#if DEBUG__convert_constant__ck_aggregate
        printf("      case ck_init_repeat\n");
#endif
        element = convert_constant(c);
        break;
      }
      default: {
#if DEBUG__convert_constant__ck_aggregate
        printf("      case default\n");
#endif
        SgExpression * constantExpression = convert_constant(c);
        ROSE_ASSERT(constantExpression->get_file_info()->get_filenameString() != "");

        SgInitializer * element_init = wrap_expr_as_initializer(constantExpression);
#if DEBUG__convert_constant__ck_aggregate
        printf("        element_init->get_file_info()->get_filenameString() = %s\n", element_init->get_file_info()->get_filenameString().c_str());
#endif
        if (designators.empty() == false) {
          if (designators.size() > 1) {
            printf("        Fatal: Rare case of designators.size() = %" PRIuPTR " (expected only one) \n", designators.size());
            ROSE_ABORT();
          }

          SgExprListExp* el = set_generated_file_info_to_unparse(SageBuilder::buildExprListExp_nfi());
          appendExpressionList(el, designators);
#if DEBUG__convert_constant__ck_aggregate
          printf("        element_init = %p \n",element_init);
#endif
          SgDesignatedInitializer* designatedInitializer = new SgDesignatedInitializer(el, element_init); /* FIXME: build function */
          setSourcePosition(designatedInitializer);

          if (compilerGenerated_inloop) {
            element = set_generated_file_info(designatedInitializer);
          } else {
            element = set_generated_file_info_to_unparse(designatedInitializer);
          }
          designatedInitializer->set_is_explicit_cast(false);
#if DEBUG__convert_constant__ck_aggregate
          printf("        designatedInitializer->get_is_explicit_cast() = %s \n", designatedInitializer->get_is_explicit_cast() ? "true" : "false");
#endif
          ROSE_ASSERT(element_init->get_parent() != NULL);
          designators.clear();
        } else {
          element = element_init;
        }
      }
    }
    if (element != NULL) {
#if DEBUG__convert_constant__ck_aggregate
      printf("      element = %p \n", element, element->class_name().c_str());
#endif
      appendExpression(contents, element);
    }
#if DEBUG__convert_constant__ck_aggregate
    printf("      designators.size() = %" PRIuPTR " \n", designators.size());
#endif
    designators.clear();
  }
#if DEBUG__convert_constant__ck_aggregate
  printf("    contents = %p = %s \n", contents, contents->class_name().c_str());
#endif
  SgType * constantType = convert_type(actual_type);
  ROSE_ASSERT(constantType != NULL);
#if DEBUG__convert_constant__ck_aggregate
  printf("    constantType = %p = %s\n", constantType, constantType->class_name().c_str());
  printf("    is_compound_literal = %s\n", expr->is_compound_literal ? "true" : "false");
#endif

  SgType * aggregateInitializerType = convert_type(expr->type);
  ROSE_ASSERT(aggregateInitializerType != NULL);
#if DEBUG__convert_constant__ck_aggregate
  printf("    aggregateInitializerType = %p = %s\n", aggregateInitializerType, aggregateInitializerType->class_name().c_str());
#endif

  SgAggregateInitializer * aggregateInitializer = buildAggregateInitializer_nfi(contents, aggregateInitializerType);
  ROSE_ASSERT(aggregateInitializer != NULL);
#if DEBUG__convert_constant__ck_aggregate
  printf("    aggregateInitializer = %p = %s\n", aggregateInitializer, aggregateInitializer->class_name().c_str());
  printf("    aggregateInitializer->get_type() = %p = %s\n", aggregateInitializer->get_type(), aggregateInitializer->get_type()->class_name().c_str());
#endif
  aggregateInitializer->set_need_explicit_braces(expr->explicit_braces_on_aggregate);
  aggregateInitializer->set_uses_compound_literal(expr->is_compound_literal);
#if DEBUG__convert_constant__ck_aggregate
  printf("    explicit_braces_on_aggregate = %s\n", expr->explicit_braces_on_aggregate ? "true" : "false");
  printf("    is_compound_literal = %s\n", expr->is_compound_literal ? "true" : "false");
#endif
  if (expr->explicit_braces_on_aggregate || expr->is_compound_literal) {
    result = set_file_info_for_corresp(aggregateInitializer, expr->source_corresp);
  } else {
    result = set_generated_file_info(aggregateInitializer);
  }

  if (compilerGenerated == true && constant_resolved_to_empty_list == false) {
    result->get_file_info()->unsetOutputInCodeGeneration();
  }
  ROSE_ASSERT(expr->type != NULL);

#if DEBUG__convert_constant__ck_aggregate
  printf("    result = %p = %s \n", result, result->class_name().c_str());
#endif
  return result;
}

static SgExpression * convert_constant__ck_init_repeat(a_constant_ptr expr, a_type_ptr actual_type) {
#if DEBUG__convert_constant__ck_init_repeat
  printf("  Enter convert_constant__ck_init_repeat:\n");
#endif
  a_constant_ptr repeat_constant = expr->variant.init_repeat.constant;
  ROSE_ASSERT(repeat_constant != NULL);
  return set_file_info_for_corresp(convert_constant(repeat_constant), expr->source_corresp);
}

static SgExpression * convert_constant__ck_template_param(a_constant_ptr expr, a_type_ptr actual_type) {
#if DEBUG__convert_constant__ck_template_param
  printf("  Enter convert_constant__ck_template_param:\n");
#endif
  SgExpression * result = convert_constant_template_parameter(expr);
#if DEBUG__convert_constant__ck_template_param
  printf("    result = %p = %s \n", result, result->class_name().c_str());
#endif
  ROSE_ASSERT(result->get_parent() == NULL);
  return result;
}

static SgExpression * convert_constant__ck_void(a_constant_ptr expr, a_type_ptr actual_type) {
#if DEBUG__convert_constant__ck_void
  printf("  Enter convert_constant__ck_void:\n");
#endif
  return set_file_info_for_corresp(SageBuilder::buildVoidVal_nfi(), expr->source_corresp);
}

#if UPC_EXTENSIONS_ALLOWED
static SgExpression * convert_constant__ck_upc_threads(a_constant_ptr expr, a_type_ptr actual_type) {
#if DEBUG__convert_constant__ck_upc_threads
  printf("  Enter ck_upc_threads:\n");
#endif
  SgExpression * result = SageBuilder::buildUpcThreads_nfi();
  result = set_generated_file_info_to_unparse(result);
  if (expr->variant.integer_value != 1) {
    SgExpression * value = SageBuilder::buildIntVal_nfi(expr->variant.integer_value, "");
    value = set_generated_file_info_to_unparse(value);
    result = SageBuilder::buildMultiplyOp_nfi(result, value);
    result = set_generated_file_info_to_unparse(result);
  }
  return result;
}

static SgExpression * convert_constant__ck_upc_mythread(a_constant_ptr expr, a_type_ptr actual_type) {
#if DEBUG__convert_constant__ck_upc_mythread
  printf("  Enter ck_upc_mythread:\n");
#endif
  return set_generated_file_info_to_unparse(SageBuilder::buildUpcMythread_nfi());
}
#endif

static void convert_constant__attach_original_expression_tree(SgExpression * target, an_expr_node_ptr expr) {
#if DEBUG__convert_constant__attach_original_expression_tree
  printf("  Enter convert_constant__attach_original_expression_tree():\n");
  printf("    target = %p = %s \n", target, target->class_name().c_str());
  printf("    expr   = %p = %s \n", expr, expr ? summarize_expression_in_string(expr).c_str() : "");
#endif
  if (expr == nullptr) return;

  SgExpression * orig_expr = convert_expression(expr);
  ROSE_ASSERT(orig_expr != NULL);
#if DEBUG__convert_constant__attach_original_expression_tree
  printf("    orig_expr = %p = %s \n", orig_expr, orig_expr->class_name().c_str());
#endif

#if USING__convert_constant__attach_original_expression_tree__unnamed_struct_decl
  SgSizeOfOp * sizeOfOp = isSgSizeOfOp(orig_expr);
  SgClassType * classType = sizeOfOp != nullptr ? isSgClassType(sizeOfOp->get_operand_type()) : nullptr;
  SgClassDeclaration * xdecl = classType != nullptr ? isSgClassDeclaration(classType->get_declaration()) : nullptr;
  if (xdecl != NULL) {
    SgClassDeclaration * defn_xdecl = isSgClassDeclaration(xdecl->get_definingDeclaration());
    bool has_defn_decl = defn_xdecl != nullptr;
    if (xdecl->get_isUnNamed() && !has_defn_decl || (has_defn_decl && xdecl == defn_xdecl) ) {
#if DEBUG__convert_constant__attach_original_expression_tree
      printf("    Warning: original expression tree is sizeof an unnamed class!\n");
#endif
#if FIXME__convert_constant__attach_original_expression_tree__delete_subtree
      orig_expr = NULL;
#else
      deleteAST(orig_expr);
#endif
    }
  }
#endif

#if USING__convert_constant__attach_original_expression_tree__typeid
  SgTypeIdOp* typeIdOp = isSgTypeIdOp(orig_expr);
  if (typeIdOp != NULL) {
#if DEBUG__convert_constant__attach_original_expression_tree
    printf("   Warning: original expression tree is SgTypeIdOp!\n");
#endif
#if FIXME__convert_constant__attach_original_expression_tree__delete_subtree
    orig_expr = NULL;
#else
    deleteAST(orig_expr);
#endif
  }
#endif

  if (orig_expr != NULL) {
    SgValueExp* value_target = isSgValueExp(target);
    SgBinaryOp* binop_target = isSgBinaryOp(target);
    SgAggregateInitializer * aggr_target = isSgAggregateInitializer(target);
    SgCastExp * cast_target = isSgCastExp(target);
    SgVarRefExp * vref_target = isSgVarRefExp(target);
    SgFunctionRefExp * fref_target = isSgFunctionRefExp(target);
    SgAddressOfOp * addr_target = isSgAddressOfOp(target);
    if (value_target != NULL) {
      value_target->set_originalExpressionTree(orig_expr);
    } else if (binop_target != NULL) {
      binop_target->set_originalExpressionTree(orig_expr);
    } else if (aggr_target != NULL) {
      aggr_target->set_originalExpressionTree(orig_expr);
    } else if (cast_target != NULL) {
      cast_target->set_originalExpressionTree(orig_expr);
    } else if (vref_target != NULL) {
      vref_target->set_originalExpressionTree(orig_expr);
    } else if (fref_target != NULL) {
      fref_target->set_originalExpressionTree(orig_expr);
    } else if (addr_target != NULL) {
      addr_target->set_originalExpressionTree(orig_expr);
#if FATAL__convert_constant__attach_original_expression_tree__unknown_target
    } else {
      printf("  Fatal: Unknown target for original expression tree.\n");
      ROSE_ABORT();
#endif
    }
    orig_expr->set_parent(target);
  }
}

////////////////////
// Entry function //
////////////////////

SgExpression * convert_constant(a_constant_ptr cstp) {
#if DEBUG__convert_constant
  printf("Enter convert_constant():\n");
  printf("  cstp = %p = %s \n", cstp, summarize_constant_kind_in_string(cstp));
#endif

  if (cstp == nullptr) {
#if DEBUG__convert_constant
    printf("Leave convert_constant(nullptr)\n");
#endif
    return set_generated_file_info_to_unparse(buildNullExpression_nfi());
  }

  a_type_ptr actual_type = nullptr;
  if (cstp->type != nullptr) {
    actual_type = skip_typerefs(cstp->type);
#if DEBUG__convert_constant
    if (actual_type != cstp->type) {
      printf("  Warning: cstp->type = %p BUT actual_type = %p \n", cstp->type, actual_type);
    }
#endif
  }

#if DEBUG__convert_constant
  printf("  cstp->type  = %p = %s \n", cstp->type, summarize_type_kind_in_string(cstp->type));
  printf("  actual_type = %p = %s \n", actual_type, actual_type != nullptr ? summarize_type_kind_in_string(actual_type) : "");
#endif

  a_constant_repr_kind kind = (a_constant_repr_kind)(cstp->kind);
  // EDG represents an empty lambda closure as a compiler-generated aggregate
  // whose backing expression is the source lambda.  The generic aggregate
  // path would convert that expression as the sole effective element and then
  // convert it again as the constant's original expression tree.  It is one
  // lambda, not a folded value paired with a source expression, so preserve
  // the normal lambda graph and its canonical cached closure type directly.
  const bool isEmptyLambdaClosureConstant =
      kind == ck_aggregate &&
      cstp->source_corresp.decl_position.seq == 0 &&
      cstp->variant.aggregate.first_constant == nullptr &&
      cstp->expr != nullptr && cstp->expr->kind == enk_lambda;
  if (isEmptyLambdaClosureConstant) {
    SgExpression * result = convert_expression(cstp->expr);
    ROSE_ASSERT(isSgLambdaExp(result) != nullptr);
    ROSE_ASSERT(result->get_parent() == nullptr);
    ROSE_ASSERT(result->get_file_info()->get_filenameString() != "");
    return result;
  }

  SgExpression * result = nullptr;
  switch (kind) {
    case ck_integer:          result = convert_constant__ck_integer(cstp, actual_type);        break;
    case ck_string:           result = convert_constant__ck_string(cstp, actual_type);         break;
    case ck_float:            result = convert_constant__ck_float(cstp, actual_type);          break;
    case ck_complex:          result = convert_constant__ck_complex(cstp, actual_type);        break;
    case ck_imaginary:        result = convert_constant__ck_imaginary(cstp, actual_type);      break;
    case ck_address:          result = convert_constant__ck_address(cstp, actual_type);        break;
    case ck_ptr_to_member:    result = convert_constant__ck_ptr_to_member(cstp, actual_type);  break;
    case ck_dynamic_init:     result = convert_constant__ck_dynamic_init(cstp, actual_type);   break;
    case ck_aggregate:        result = convert_constant__ck_aggregate(cstp, actual_type);      break;
    case ck_init_repeat:      result = convert_constant__ck_init_repeat(cstp, actual_type);    break;
    case ck_template_param:   result = convert_constant__ck_template_param(cstp, actual_type); break;
    case ck_void:             result = convert_constant__ck_void(cstp, actual_type);           break;
#if UPC_EXTENSIONS_ALLOWED
    case ck_upc_threads:      result = convert_constant__ck_upc_threads(cstp, actual_type);    break;
    case ck_upc_mythread:     result = convert_constant__ck_upc_mythread(cstp, actual_type);   break;
#endif
    case ck_designator:       printf("  Fatal: Found a designator outside an aggregate initializer\n"); ROSE_ABORT();
#if FIXED_POINT_ALLOWED
    case ck_fixed_point:      printf("  Fatal: Not supportedd fixed point constant\n"); ROSE_ABORT();
#endif
#if GNU_EXTENSIONS_ALLOWED
    case ck_label_difference: printf("  Fatal: ck_label_difference\n"); ROSE_ABORT();
#endif
    default:                  printf("  Fatal: Bad constant kind %d\n", kind); ROSE_ABORT();
  }

#if DEBUG__convert_constant
  printf("  result = %p = %s\n", result, result->class_name().c_str());
  printf("    ->isCompilerGenerated() = %s\n", result->get_file_info()->isCompilerGenerated() ? "true" : "false" );
  SgCastExp * castExp = isSgCastExp(result);
  if (castExp != NULL) {
    printf("  Found a SgCastExp:\n");
    printf("    castExp = %p = %s\n", castExp, castExp->class_name().c_str());
    printf("    castExp->get_operand() = %p = %s\n", castExp->get_operand(), castExp->get_operand()->class_name().c_str());
  }
#endif
  ROSE_ASSERT(result != NULL);
  ROSE_ASSERT(result->get_parent() == NULL);
  ROSE_ASSERT(result->get_file_info()->get_filenameString() != "");

  convert_constant__attach_original_expression_tree(result, cstp->expr);

#if DEBUG__convert_constant
  printf("Leave convert_constant(%p)\n", cstp);
#endif
  return result;
}

}
