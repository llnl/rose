
#include "edgRose.h"

namespace EDG_ROSE_Translation {

#define OPCOUNT(cond) ROSE_ASSERT (operands.size() cond)

#define DEBUG__convert_expression__enk_operation 0

SgExpression * convert_expression__enk_operation(an_expr_node_ptr e) {
#if DEBUG__convert_expression__
  printf ("  Enter convert_expression__enk_operation():\n");
#endif
   SgExpression * result = nullptr;

               an_expr_operator_kind inner_kind = (an_expr_operator_kind)(e->variant.operation.kind);
               std::vector<SgExpression*> operands;
               for (an_expr_node_ptr operand_ptr_raw = e->variant.operation.operands; operand_ptr_raw; operand_ptr_raw = operand_ptr_raw->next)
                  {
                    operands.push_back(convert_expression(operand_ptr_raw));
                  }

               switch (inner_kind)
                  {
                    case eok_indirect: 
                       {
                         OPCOUNT(== 1);
#if 0
                         printf ("In convert_expression(): case eok_indirect: wrap in SgPointerDerefExp: operands[0] = %p = %s \n",operands[0],operands[0]->class_name().c_str());
#endif
                      // DQ (10/23/2016): Eliminate the normalization associated with the typeid() operator.
                         SgExpression* expression = operands[0];
                         if (isSgTypeIdOp(expression) != NULL)
                           {
                             result = expression;
                           }
                         else
                           {
                             result = buildPointerDerefExp_nfi(expression);
                           }
                         break;
                       }

                    case eok_unary_plus: OPCOUNT(== 1); result = buildUnaryAddOp_nfi(operands[0]); break;
                    case eok_not: OPCOUNT(== 1); result = buildNotOp_nfi(operands[0]); break;

                    case eok_cast:
                        {
                         OPCOUNT(== 1);

                      // DQ (4/9/2013): Added assertion (not clear if we have to handle this case).
                         ROSE_ASSERT(e->variant.operation.is_reference_cast == false);
                         SgCastExp::cast_type_enum cast_type = typeOfCast(e);
                         SgCastExp* cast_op = buildCastExp_nfi(operands[0], convert_type(e->type), cast_type);
#if 0
                         printf ("In convert_expression(): case enk_operation: case eok_cast: DONE: Calling typeOfCast(): cast_op = %p \n",cast_op);
#endif
                      // DQ (8/6/2013): The default is always to build these as non-compiler generated.
                         ROSE_ASSERT(cast_op->get_startOfConstruct()->isCompilerGenerated() == false);
                         ROSE_ASSERT(cast_op->get_file_info()->isCompilerGenerated() == false);
                         ROSE_ASSERT(cast_op->get_endOfConstruct()->isCompilerGenerated() == false);

                         result = cast_op;

                         break;
                       }

                    case eok_base_class_cast:
                       {
                      // Note that in the case of a dynamic cast from a ddrived class to a base class, EDG will turn the
                      // dynamic cast into a this kind of cast (and we have no choise but to interpret this as a C style cast).
                      // This is an example of information lost from the source code to the AST.

                         OPCOUNT(== 1);
#if 0
                         printf ("In convert_expression(): case enk_operation: case eok_base_class_cast: calling typeOfCast() \n");
#endif
                         SgCastExp::cast_type_enum cast_type = typeOfCast(e);
                         SgType * type = convert_type(e->type);
                         if (e->variant.operation.is_reference_cast) {
                           type = SageBuilder::buildReferenceType(type);
                         }
                         result = SageBuilder::buildCastExp_nfi(operands[0], type, cast_type);
                         break;
                       }

                 // DQ (4/9/2013): Revisiting this case to support reference cast.
                    case eok_derived_class_cast:
                       {
                         OPCOUNT(== 1);
#if 0
                         printf ("In convert_expression(): case enk_operation: case eok_derived_class_cast: calling typeOfCast() \n");
#endif
                      // DQ (8/22/2016): Implementing better support for richer C++ style casts.
                         SgCastExp::cast_type_enum cast_type = typeOfCast(e);

                      // DQ (4/9/2013): Adding support for reference cast.
                         bool is_reference_cast = e->variant.operation.is_reference_cast;
                         if (is_reference_cast == true)
                            {
                              result = buildCastExp_nfi(operands[0], buildReferenceType(convert_type(e->type)), cast_type);
                            }
                           else
                            {
                              result = buildCastExp_nfi(operands[0], convert_type(e->type), cast_type);
                            }
                         break;
                       }

                    case eok_pm_base_class_cast:
                       {
                         OPCOUNT(== 1);

                      // DQ (4/9/2013): Added assertion (not clear if we have to handle this case).
                         ROSE_ASSERT(e->variant.operation.is_reference_cast == false);
                      // DQ (8/22/2016): Implementing better support for richer C++ style casts.
                         SgCastExp::cast_type_enum cast_type = typeOfCast(e);
                         result = buildCastExp_nfi(operands[0], convert_type(e->type), cast_type);
                         break;
                       }
                    
                    case eok_pm_derived_class_cast:
                       {
                         OPCOUNT(== 1);

                         ROSE_ASSERT(e->variant.operation.is_reference_cast == false);
                         SgCastExp::cast_type_enum cast_type = typeOfCast(e);

                         result = buildCastExp_nfi(operands[0], convert_type(e->type), cast_type);
                         break;
                       }
                    
                    case eok_lvalue_cast:
                       {
                         OPCOUNT(== 1);

                         ROSE_ASSERT(e->variant.operation.is_reference_cast == false);
#if 0
                         printf ("In convert_expression(): case enk_operation: case eok_lvalue_cast: calling typeOfCast() \n");
#endif
                         SgCastExp::cast_type_enum cast_type = typeOfCast(e);

                         result = SageBuilder::buildCastExp_nfi(operands[0], convert_type(e->type), cast_type);
                         break;
                       }

                    case eok_dynamic_cast:
                       {
                         OPCOUNT(== 1);
                         ROSE_ASSERT(e->variant.operation.is_reference_cast == false);
                         result = SageBuilder::buildCastExp_nfi(operands[0], convert_type(e->type), SgCastExp::e_dynamic_cast);
                         break;
                       }

                    case eok_bool_cast:
                       {
                         OPCOUNT(== 1);
                         ROSE_ASSERT(e->variant.operation.is_reference_cast == false);
                         result = SageBuilder::buildCastExp_nfi(operands[0], convert_type(e->type), SgCastExp::e_C_style_cast);
                         break;
                       }

                    case eok_complement: OPCOUNT(== 1); result = buildBitComplementOp_nfi(operands[0]); break;

#if MICROSOFT_EXTENSIONS_ALLOWED
                    case eok_assume: OPCOUNT(== 1); ROSE_ASSERT (!"__assume not supported"); break;
#endif
                    case eok_padd: OPCOUNT(== 2); result = buildAddOp_nfi(operands[0], operands[1]); break;
                    case eok_psubtract: OPCOUNT(== 2); result = buildSubtractOp_nfi(operands[0], operands[1]); break;
                    case eok_jmultiply: OPCOUNT(== 2); result = buildMultiplyOp_nfi(operands[0], operands[1]); break;
                    case eok_jdivide: OPCOUNT(== 2); result = buildDivideOp_nfi(operands[0], operands[1]); break;
                    case eok_fjadd: OPCOUNT(== 2); result = buildAddOp_nfi(operands[0], operands[1]); break;
                    case eok_jfadd: OPCOUNT(== 2); result = buildAddOp_nfi(operands[0], operands[1]); break;
                    case eok_fjsubtract: OPCOUNT(== 2); result = buildSubtractOp_nfi(operands[0], operands[1]); break;
                    case eok_jfsubtract: OPCOUNT(== 2); result = buildSubtractOp_nfi(operands[0], operands[1]); break;
#if GNU_EXTENSIONS_ALLOWED
                    case eok_xconj: OPCOUNT(== 1); result = buildConjugateOp_nfi(operands[0]); break;
                    case eok_real_part: OPCOUNT(== 1); result = buildRealPartOp_nfi(operands[0]); break;
                    case eok_imag_part: OPCOUNT(== 1); result = buildImagPartOp_nfi(operands[0]); break;
#endif
                    case eok_remainder: OPCOUNT(== 2); result = buildModOp_nfi(operands[0], operands[1]); break;
                    case eok_pdiff: OPCOUNT(== 2); result = buildSubtractOp_nfi(operands[0], operands[1]); break;
                    case eok_bassign: OPCOUNT(== 2); ROSE_ASSERT (!"bassign should not occur"); break;
                    case eok_remainder_assign: OPCOUNT(== 2); result = SageBuilder::buildModAssignOp_nfi(operands[0], operands[1]); break;
                    case eok_padd_assign: OPCOUNT(== 2); result = SageBuilder::buildPlusAssignOp_nfi(operands[0], operands[1]); break;
                    case eok_psubtract_assign: OPCOUNT(== 2); result = SageBuilder::buildMinusAssignOp_nfi(operands[0], operands[1]); break;
                    case eok_shiftl_assign: OPCOUNT(== 2); result = SageBuilder::buildLshiftAssignOp_nfi(operands[0], operands[1]); break;
                    case eok_shiftr_assign: OPCOUNT(== 2); result = SageBuilder::buildRshiftAssignOp_nfi(operands[0], operands[1]); break;
                    case eok_and_assign: OPCOUNT(== 2); result = SageBuilder::buildAndAssignOp_nfi(operands[0], operands[1]); break;
                    case eok_or_assign: OPCOUNT(== 2); result = SageBuilder::buildIorAssignOp_nfi(operands[0], operands[1]); break;
                    case eok_xor_assign: OPCOUNT(== 2); result = SageBuilder::buildXorAssignOp_nfi(operands[0], operands[1]); break;

                    case eok_subscript:
#ifdef DEBUG_CONVERSION
                         indent_line(); debug_print("eok_subscript\n");
#endif
#if 0
                         printf ("case eok_subscript: operands[0] = %s \n",operands[0]->class_name().c_str());
                         printf ("case eok_subscript: operands[0] = %s \n",operands[0]->class_name().c_str());
                         printf ("case eok_subscript: operands[0].has_address_of  = %s \n",operands[0].has_address_of ? "true" : "false");
#endif
                         OPCOUNT(== 2);

                         result = buildPntrArrRefExp_nfi(operands[0], operands[1]); 
                         break;

                 // DQ (2/25/2012): Added support for pointer to member.
                    case eok_pm_field:
                       {
                      // Selection of a nonstatic data member of a class using a pointer to member, source form x.*pm.
                      // The first operand is an lvalue or rvalue of class type.  The second operand is an rvalue of
                      // pointer-to-data-member type.

#ifdef DEBUG_CONVERSION
                         indent_line(); debug_print("eok_pm_field \n");
#endif
                         OPCOUNT(== 2);
#if 0
                         printf ("Calling buildDotStarOp_nfi() to support case eok_pm_field \n");
#endif
                         result = SageBuilder::buildDotStarOp_nfi(operands[0], operands[1]);
                         break;
                       }

                    case eok_points_to_static:
                       {
                         OPCOUNT(== 2);

                         ROSE_ASSERT(operands[1] != NULL);
#if 0
                         printf ("operands[1] = %p = %s \n",operands[1],operands[1]->class_name().c_str());
#endif
                      // FIXME: we would like to generate the constant's name instead of the constant's value (which requires the following fixup).
                      // DQ (5/18/2012): When the rhs is a constant value we can't generate code such as "X->3.14" for a constant value data member.
                      // At a later point we want to have the un-normalized form that would be "X->PI".
                         if (isSgValueExp(operands[1]) != NULL)
                            {
                              result = operands[1];
                            }
                           else
                            {
                              result = buildArrowExp_nfi(operands[0], operands[1]);
                            }
                         break;
                       }

                    case eok_shiftl:   OPCOUNT(== 2); result = buildLshiftOp_nfi  (operands[0], operands[1]); break;
                    case eok_shiftr:   OPCOUNT(== 2); result = buildRshiftOp_nfi  (operands[0], operands[1]); break;
                    case eok_and:      OPCOUNT(== 2); result = buildBitAndOp_nfi  (operands[0], operands[1]); break;
                    case eok_or:       OPCOUNT(== 2); result = buildBitOrOp_nfi   (operands[0], operands[1]); break;
                    case eok_xor:      OPCOUNT(== 2); result = buildBitXorOp_nfi  (operands[0], operands[1]); break;
                    case eok_comma:    OPCOUNT(== 2); result = buildCommaOpExp_nfi(operands[0], operands[1]); break;

                    case eok_virtual_function_ptr: ROSE_ASSERT (!"eok_virtual_function_ptr not supported"); break;

                    case eok_land:     OPCOUNT(== 2); result = buildAndOp_nfi         (operands[0], operands[1]); break;
                    case eok_lor:      OPCOUNT(== 2); result = buildOrOp_nfi          (operands[0], operands[1]); break;
                    case eok_question: OPCOUNT(== 3); result = buildConditionalExp_nfi(operands[0], operands[1], operands[2], convert_type(e->type)); break;

                 // DQ (2/25/2013): I notice now that there is no "fall through" here.
                 // DQ (12/31/2008): Note that there is "fall through" here, so we have to be careful how we use conditional compilation.
                    case eok_call:
                       {
                      // A call of a non-member function or static member function.  Also any call in C.  The first operand
                      // identifies the routine (as a routine address) and the rest are its arguments.  Note that the operand
                      // specifying the routine can be an expression (e.g., for a call through a pointer), or
                      // eok_dot_static/eok_points_to_static for a static member function call.

                      // FIXME: handle member function calls
                         OPCOUNT(>= 1);

                      // DQ (11/2/2012): Calling set_generated_file_info_to_unparse() overwrites the existing source position information.
                         SgExpression* func = operands[0];
                         ROSE_ASSERT(func != NULL);
#if 0
                         printf ("In convert_expression(): e = %p case eok_call: func = %p = %s \n",e,func,func->class_name().c_str());
#endif
                      // DQ (11/10/2014): Test this closer to where func is defined (also tested below).
                         if (func->get_type() == NULL)
                            {
                              printf ("WARNING: In convert_expression(): case eok_call: func->get_type() == NULL \n");
                            }
#if 0
                         printf ("In case eok_call: func = %p = %s operands.size() = %" PRIuPTR " \n",func,func->class_name().c_str(),operands.size());
#endif
                         SgDotExp*   dotExp   = isSgDotExp(func);
                         SgArrowExp* arrowExp = isSgArrowExp(func);
                         if (dotExp != NULL || arrowExp != NULL)
                            {
                              SgBinaryOp* binaryOp = isSgBinaryOp(func);
                              ROSE_ASSERT(binaryOp != NULL);

                              ROSE_ASSERT(binaryOp->get_lhs_operand() != NULL);
                              ROSE_ASSERT(binaryOp->get_rhs_operand() != NULL);
#if 0
                              printf ("binaryOp->get_lhs_operand() = %p = %s \n",binaryOp->get_lhs_operand(),binaryOp->get_lhs_operand()->class_name().c_str());
                              printf ("binaryOp->get_rhs_operand() = %p = %s \n",binaryOp->get_rhs_operand(),binaryOp->get_rhs_operand()->class_name().c_str());
#endif
                           // Detect known error in EDG representation of function call in template to template member function.
                              SgVarRefExp* varRefExp = isSgVarRefExp(binaryOp->get_rhs_operand());
                              if (varRefExp != NULL)
                                 {
                                // DQ (8/20/2013): I now think this is just a function call off of a variable which is a pointer to a function.

                                // This is an error in EDG, that for a member function call in a template of a template parameter, 
                                // the function reference expression is expressed as a kind = ck_template_param sub-kind = tpck_member 
                                // which is only for building data members not function references.  I think we have to detect this 
                                // case as an error and fix it here where we know it is incorrect.
#ifdef DEBUG_CONVERSION
                                // DQ (6/28/2013): Mark this as an unusual (rare) case.
                                   printf ("In convert_expression(): case eok_call: I think this is a rare case where a template member function call in a template is expressed as a kind = ck_template_param sub-kind = tpck_member \n");
#endif
                                   SgVariableSymbol* variableSymbol = varRefExp->get_symbol();
                                   ROSE_ASSERT(variableSymbol != NULL);
#if 0
                                   printf ("variableSymbol = %p = %s = %s \n",variableSymbol,variableSymbol->class_name().c_str(),variableSymbol->get_name().str());
#endif
                                   SgInitializedName* initializedName = variableSymbol->get_declaration();
                                   ROSE_ASSERT(initializedName != NULL);
#ifdef DEBUG_CONVERSION
                                   printf ("In case eok_call: Fixup reference to data member and replace with template member function reference: initializedName = %p = %s \n",initializedName,initializedName->get_name().str());
#endif
                                   SgName name = initializedName->get_name();

                                   SgScopeStatement* rose_scope = initializedName->get_scope();
                                   ROSE_ASSERT(rose_scope != NULL);
#if 0
                                   printf ("rose_scope = %p = %s \n",rose_scope,rose_scope->class_name().c_str());
#endif
                                 }
                                else
                                 {
#if 0
                                   printf ("WARNING: In convert_expression(): case eok_call: (dotExp != NULL || arrowExp != NULL) == false \n");
#endif
                                 }
                            }
                           else
                            {
#if 0
                              printf ("WARNING: In convert_expression(): case eok_call: not clear what this case is, since it is not a function call using a variable which is a pointer to a function \n");
#endif
                            }

                      // DQ (11/10/2014): Adding test to support C++11 testing.
                         ROSE_ASSERT(func != NULL);

                      // DQ (11/10/2014): Comment out this assertion and add comment for when it fails.
                      // DQ (12/31/2008): All function should have a valid type! (AST consistancy)
                         if (func->get_type() == NULL)
                            {
                              printf ("WARNING: In convert_expression(): case eok_call: func->get_type() == NULL \n");
                            }

                         std::vector<SgExpression*> args;
                         for (size_t i = 1; i < operands.size(); ++i)
                            {
#if 0
                              printf ("In case eok_call: arg #%" PRIuPTR " = %p = %s \n",i,operands[i],operands[i]->class_name().c_str());
#endif
#if 0
                              operands[i]->get_file_info()->display("argument in function call");
#endif
                              args.push_back(operands[i]); // FIXME handle references
                            }

                         SgFunctionCallExp* functionCallExp = NULL;
#if !ENABLE_CUDA_SUPPORT
                         functionCallExp = buildFunctionCallExp_nfi(func, set_generated_file_info_to_unparse(buildExprListExp_nfi(args)));
#else /* ENABLE_CUDA_SUPPORT */
                         if (!e->variant.operation.is_kernel_call)
                            {
                              functionCallExp = buildFunctionCallExp_nfi(func, set_generated_file_info_to_unparse(buildExprListExp_nfi(args)));
                            }
                           else
                            {
                              an_expr_node_ptr config_operand_ptr_raw = e->variant.operation.config_operands;
                              vector<SgExpression*> config_operands;
                              int config_operand_cnt = 0;
                              for (; config_operand_ptr_raw; config_operand_ptr_raw = config_operand_ptr_raw->next) 
                                 {
                                   config_operands.push_back(convert_expression(config_operand_ptr_raw));
                                   config_operand_cnt++;
                                 }
                              ROSE_ASSERT(config_operand_cnt <= 4);
                              functionCallExp = buildCudaKernelCallExp_nfi(func,
                                            set_generated_file_info_to_unparse(buildExprListExp_nfi(args)),
                                            set_generated_file_info_to_unparse(buildCudaKernelExecConfig_nfi(
                                                 config_operand_cnt > 0 ? config_operands[0] : NULL,
                                                 config_operand_cnt > 1 ? config_operands[1] : NULL,
                                                 config_operand_cnt > 2 ? config_operands[2] : NULL,
                                                 config_operand_cnt > 3 ? config_operands[3] : NULL
                                             ))
                                         );
                            }
#endif /* ENABLE_CUDA_SUPPORT */

#if 0
                         printf ("functionCallExp->get_args() = %p = %s \n",functionCallExp->get_args(),functionCallExp->get_args()->class_name().c_str());
#endif
                      // DQ (4/8/2013): Mark the function call as using operator or non-operator syntax.
                         bool call_uses_operator_syntax = e->variant.operation.call_uses_operator_syntax;

                      // DQ (4/14/2013): Added support for whole translation unit specification of there using the overloaded 
                      // operator names or the operator syntax.  This is mostly used for testing of the unparser to make sure
                      // that both are supported equally.  The default is to use the behavior specified in the original source code.
                         call_uses_operator_syntax = (global_call_uses_operator_syntax == true) ? true  : call_uses_operator_syntax;
                         call_uses_operator_syntax = (global_call_uses_operator_names  == true) ? false : call_uses_operator_syntax;

                         functionCallExp->set_uses_operator_syntax(call_uses_operator_syntax);
#if 0
                      // DQ (4/26/2013): Debugging code.
                         printf ("In case eok_call: Calling testFunctionCallArguments() \n");
                         testFunctionCallArguments(functionCallExp);
#endif
                         result = functionCallExp;
                         break;
                       }

                 // DQ (12/30/2008): Fix maybe specific to EDG 4.0.
                    case eok_va_start: 
                       {
#ifdef DEBUG_CONVERSION
                         printf ("Warning: this does not appear to always be a binary operator: operands.size() = %" PRIuPTR " \n",operands.size());
#endif
                         OPCOUNT(>= 2);
                         result = buildVarArgStartOp_nfi(operands[0], operands[1]);

                         ROSE_ASSERT(result != NULL);

                      // DQ (12/30/2008): Added debugging code (test2003_30.C fails to have parent set)
                      // This might be a limitation of the mechanism that sets parents (so maybe a 
                      // better fix would be to check that).
                         for (size_t i = 0; i < operands.size(); ++i)
                            {
                              ROSE_ASSERT(operands[i] != NULL);
                              if (operands[i]->get_parent() == NULL)
                                 {
#ifdef DEBUG_CONVERSION
                                   printf ("Warning: Resetting to valid parent --- operands[%" PRIuPTR "]->get_parent() == NULL result = %p \n",i,result);
#endif
                                   operands[i]->set_parent(result);
                                   ROSE_ASSERT(operands[i]->get_parent() != NULL);
                                 }
                              ROSE_ASSERT(operands[i]->get_parent() != NULL);
                            }
                         break;
                       }

                    case eok_va_arg:                  OPCOUNT(== 1); result = SageBuilder::buildVarArgOp_nfi(operands[0], convert_type(e->type));           break;
                    case eok_va_end:                  OPCOUNT(== 1); result = SageBuilder::buildVarArgEndOp_nfi(operands[0]);                               break;
                    case eok_va_copy:                 OPCOUNT(== 2); result = SageBuilder::buildVarArgCopyOp_nfi(operands[0], operands[1]); break;
                    case eok_va_start_single_operand: OPCOUNT(== 2); result = SageBuilder::buildVarArgStartOneOperandOp_nfi(operands[0]);                   break;
                    case eok_negate:                  OPCOUNT(== 1); result = SageBuilder::buildMinusOp_nfi(operands[0]);                                           break;
                    case eok_post_incr:               OPCOUNT(== 1); result = SageBuilder::buildPlusPlusOp_nfi(operands[0], SgUnaryOp::postfix);            break;
                    case eok_post_decr:               OPCOUNT(== 1); result = SageBuilder::buildMinusMinusOp_nfi(operands[0], SgUnaryOp::postfix);          break;
                    case eok_pre_incr:                OPCOUNT(== 1); result = SageBuilder::buildPlusPlusOp_nfi(operands[0], SgUnaryOp::prefix);             break;
                    case eok_pre_decr:                OPCOUNT(== 1); result = SageBuilder::buildMinusMinusOp_nfi(operands[0], SgUnaryOp::prefix);           break;

                    case eok_add:                     OPCOUNT(== 2); result = buildAddOp_nfi(operands[0], operands[1]);            break;
                    case eok_subtract:                OPCOUNT(== 2); result = buildSubtractOp_nfi(operands[0], operands[1]);       break;
                    case eok_multiply:                OPCOUNT(== 2); result = buildMultiplyOp_nfi(operands[0], operands[1]);       break;
                    case eok_divide:                  OPCOUNT(== 2); result = buildDivideOp_nfi(operands[0], operands[1]);         break;
                    case eok_eq:                      OPCOUNT(== 2); result = buildEqualityOp_nfi(operands[0], operands[1]);       break;
                    case eok_ne:                      OPCOUNT(== 2); result = buildNotEqualOp_nfi(operands[0], operands[1]);       break;
                    case eok_gt:                      OPCOUNT(== 2); result = buildGreaterThanOp_nfi(operands[0], operands[1]);    break;
                    case eok_lt:                      OPCOUNT(== 2); result = buildLessThanOp_nfi(operands[0], operands[1]);       break;
                    case eok_ge:                      OPCOUNT(== 2); result = buildGreaterOrEqualOp_nfi(operands[0], operands[1]); break;
                    case eok_le:                      OPCOUNT(== 2); result = buildLessOrEqualOp_nfi(operands[0], operands[1]);    break;

                    case eok_gnu_min: OPCOUNT(== 2); ROSE_ASSERT (!"<? not supported"); break;
                    case eok_gnu_max: OPCOUNT(== 2); ROSE_ASSERT (!">? not supported"); break;

                    case eok_assign:          OPCOUNT(== 2); result = SageBuilder::buildAssignOp_nfi(operands[0], operands[1]);      break;
                    case eok_add_assign:      OPCOUNT(== 2); result = SageBuilder::buildPlusAssignOp_nfi(operands[0], operands[1]);  break;
                    case eok_subtract_assign: OPCOUNT(== 2); result = SageBuilder::buildMinusAssignOp_nfi(operands[0], operands[1]); break;
                    case eok_multiply_assign: OPCOUNT(== 2); result = SageBuilder::buildMultAssignOp_nfi(operands[0], operands[1]);  break;
                    case eok_divide_assign:   OPCOUNT(== 2); result = SageBuilder::buildDivAssignOp_nfi(operands[0], operands[1]);   break;

                    case eok_lvalue:
                       {
                         OPCOUNT(== 1);
                         result = operands[0];
                         break;
                       }

                    case eok_spaceship:
                       {
                         OPCOUNT(== 2);
                         result = SageBuilder::buildSpaceshipOp_nfi(operands[0], operands[1]);
                         break;
                       }

                    case eok_array_to_pointer:
                       {
#ifdef DEBUG_CONVERSION
                         indent_line(); debug_print("array to pointer\n");
#endif
                         OPCOUNT(== 1);
#if 0
                         printf ("case eok_array_to_pointer: e->is_lvalue = %s \n",e->is_lvalue ? "true" : "false");
                         printf ("case eok_array_to_pointer: operands[0] = %s \n",operands[0]->class_name().c_str());
                         printf ("case eok_array_to_pointer: operands[0] = %s \n",operands[0]->class_name().c_str());
                         printf ("case eok_array_to_pointer: operands[0].has_address_of  = %s \n",operands[0].has_address_of ? "true" : "false");
#endif
                         result = operands[0];
                         break;
                       }
          
                 // DQ (12/30/2008): Added to support test2001_25.C.
                    case eok_dot_field:
                       {
#ifdef DEBUG_CONVERSION
                         indent_line(); debug_print("dot field\n");
#endif
                         OPCOUNT(== 2);
#if 0
                         printf ("case eok_dot_field: e->is_lvalue = %s \n",e->is_lvalue ? "true" : "false");
                         printf ("case eok_dot_field: operands[0] = %s \n",operands[0]->class_name().c_str());
                         printf ("case eok_dot_field: operands[1] = %s \n",operands[1]->class_name().c_str());
                         printf ("case eok_dot_field: operands[0].has_address_of = %s \n",operands[0].has_address_of ? "true" : "false");
#endif
                         result = SageBuilder::buildDotExp_nfi(operands[0], operands[1]);
                         break;
                       }

                 // DQ (12/30/2008): Added to support test2003_19.C.
                    case eok_address_of:
                       {
#ifdef DEBUG_CONVERSION
                         indent_line(); debug_print("address of\n");
#endif
                         OPCOUNT(== 1);
                         result = SageBuilder::buildAddressOfOp_nfi(operands[0]);
                         break;
                       }

                    case eok_points_to_field:
                       {
#ifdef DEBUG_CONVERSION
                         indent_line(); debug_print("points to field\n");
#endif
#if 0
                         printf ("Calling buildArrorExp() \n");
#endif
                         OPCOUNT(== 2);
                         result = SageBuilder::buildArrowExp_nfi(operands[0], operands[1]);
                         break;
                       }

                    case eok_lvalue_adjust:
                       {
                      // DQ (1/26/2009): See test2001_06.C for an example of this.
#ifdef DEBUG_CONVERSION
                         indent_line(); debug_print("lvalue adjust\n");
#endif
                         OPCOUNT(== 1);

                      // This is not the same as a cast, as in:
                      //    case eok_lvalue_cast: OPCOUNT(== 1); result = buildCastExp_nfi(operands[0], convert_type(e->type), SgCastExp::e_C_style_cast); break;
                      // I think it is more as though we just passed it through unmodified...so we don't represent this in the ROSE AST.
#ifdef DEBUG_CONVERSION
                         indent_line(); debug_print("Note: case eok_lvalue_adjust not represented explicitly in the IR ... \n");
#endif
                         result = operands[0];

                      // printf ("In convert_expression(): case eok_lvalue_adjust: sharing LHS operand directly = %p = %s \n",result,result->class_name().c_str());

                      // printf ("Sorry, 'lvalue adjust' not implemented! \n");
                      // ROSE_ASSERT(false);
                         break;
                       }

                    case eok_reference_to:
                       {
                      // DQ (1/26/2009): See test2001_06.C for an example of this.
#ifdef DEBUG_CONVERSION
                         indent_line(); debug_print("reference to\n");
#endif
                         OPCOUNT(== 1);

#ifdef DEBUG_CONVERSION
                         indent_line(); debug_print("Note: case eok_reference_to not represented explicitly in the IR ... \n");
#endif
                         result = operands[0];

                      // printf ("In convert_expression(): case eok_reference_to: sharing LHS operand directly = %p = %s \n",result,result->class_name().c_str());

                      // printf ("Sorry, 'reference to' not implemented! \n");
                      // ROSE_ASSERT(false);
                         break;
                       }

                    case eok_ref_indirect:
                       {
                      // DQ (1/26/2009): See test2001_06.C for an example of this.
#ifdef DEBUG_CONVERSION
                         indent_line(); debug_print("ref indirect\n");
#endif
                         OPCOUNT(== 1);

#ifdef DEBUG_CONVERSION
                         indent_line(); debug_print("Note: case eok_ref_indirect not represented explicitly in the IR ... \n");
#endif
                         result = operands[0];

                      // printf ("In convert_expression(): case eok_ref_indirect: sharing LHS operand directly = %p = %s \n",result,result->class_name().c_str());

                      // printf ("Sorry, 'ref indirect' not implemented! \n");
                      // ROSE_ASSERT(false);
                         break;
                       }

                    case eok_ref_cast:
                       {
                      // DQ (1/26/2009): See test2001_06.C for an example of this.
#ifdef DEBUG_CONVERSION
                         indent_line(); debug_print("ref cast\n");
#endif
                         OPCOUNT(== 1);

#ifdef DEBUG_CONVERSION
                         indent_line(); debug_print("Note: case eok_ref_cast not represented explicitly in the IR ... \n");
#endif

#if 0
                         printf ("In convert_expression(): case enk_operation: case eok_ref_cast: calling typeOfCast() (if it is a reference then it is implemented as C style cast) \n");
#endif
                      // DQ (8/22/2016): Implementing better support for richer C++ style casts.
                      // But I think this maps to a C style cast.
#if 0
                         printf ("This should map to a C style cast \n");
#endif
                      // DQ (1/22/2013): This needs to be checked for being a reference cast and that case handled better.  See test2013_12.C
                      // Note that the value of is_rvalue_reference_cast might also be important.
                      // result = operands[0];
                         if (e->variant.operation.is_reference_cast == false)
                            {
                              result = operands[0];
                            }
                           else
                            {
                           // DQ (1/22/2013): Currently the this case is the same as the false case, but I think this needs to be changed shortly (see test2013_12.C for an example of this problem).
#ifdef DEBUG_CONVERSION
                              printf ("In convert_expression(): case eok_ref_cast: is_reference_cast == true (need a cast expression here) is_rvalue_reference_cast = %s \n",e->variant.operation.is_rvalue_reference_cast ? "true" : "false");
#endif
#if 0
                              printf ("In convert_expression(): case eok_ref_cast: is_reference_cast == true (need a cast expression here) is_rvalue_reference_cast = %s \n",e->variant.operation.is_rvalue_reference_cast ? "true" : "false");
#endif
#if 0
                              printf ("In convert_expression() Calling summarize_expression(): e = %p \n",e);
                              summarize_expr(e);
                              printf ("In convert_expression() operands[0] = %p = %s \n",operands[0],operands[0]->class_name().c_str());
#endif
                           // DQ (1/30/2019): Need to add support for the C++11 r-value reference type.
                              if (e->variant.operation.is_rvalue_reference_cast == true)
                                 {
                                   result = buildCastExp_nfi(operands[0], buildRvalueReferenceType(convert_type(e->type)), SgCastExp::e_C_style_cast);
                                 }
                                else
                                 {
                                   result = buildCastExp_nfi(operands[0], buildReferenceType(convert_type(e->type)), SgCastExp::e_C_style_cast);
                                 }
                            }
                         break;
                       }

                 // DQ (11/7/2011): Added case (required for test2001_28.C)
                    case eok_points_to_member_call:
                       {
#if 0
                         printf ("In case eok_points_to_member_call: \n");
#endif
                         OPCOUNT(>= 2);
                         SgExpression* func = set_generated_file_info_to_unparse(operands[0]);
                         SgExpression* obj  = set_generated_file_info_to_unparse(operands[1]);
                         vector<SgExpression*> args;
                         for (size_t i = 2; i < operands.size(); ++i)
                            {
                              args.push_back(operands[i]); // FIXME handle references
                            }
#if 0
                         printf ("Calling buildArrorExp() \n");
#endif
                         SgFunctionCallExp* functionCallExp = buildFunctionCallExp_nfi(set_generated_file_info_to_unparse(buildArrowExp_nfi(obj, func)),set_generated_file_info_to_unparse(buildExprListExp_nfi(args)));

                      // DQ (4/8/2013): Mark the function call as using operator or non-operator syntax.
                         bool call_uses_operator_syntax = e->variant.operation.call_uses_operator_syntax;

                      // DQ (4/14/2013): Added support for whole translation unit specification of there using the overloaded 
                      // operator names or the operator syntax.  This is mostly used for testing of the unparser to make sure
                      // that both are supported equally.  The default is to use the behavior specified in the original source code.
                         call_uses_operator_syntax = (global_call_uses_operator_syntax == true) ? true  : call_uses_operator_syntax;
                         call_uses_operator_syntax = (global_call_uses_operator_names  == true) ? false : call_uses_operator_syntax;

                         functionCallExp->set_uses_operator_syntax(call_uses_operator_syntax);

                         result = functionCallExp;

                         break;
                       }
                     
                    case eok_dot_member_call:
                       {
#ifdef DEBUG_CONVERSION
                         indent_line(); debug_print("dot member call\n");
#endif
#if 0
                         printf ("In case eok_dot_member_call: \n");
#endif
                         OPCOUNT(>= 2);
                         SgExpression* func = set_generated_file_info_to_unparse(operands[0]);
                         SgExpression* obj  = set_generated_file_info_to_unparse(operands[1]);
                         std::vector<SgExpression*> args;
                         for (size_t i = 2; i < operands.size(); ++i)
                            {
                           // DQ (8/4/2012): for the arguments we want to specify that parenthesis might be required (see test2012_158.C).
                           // DQ (4/3/2012): This is a bug. The index "i" needs to be used instead.
                              SgExpression* argExpression = operands[i];
                              ROSE_ASSERT(argExpression != NULL);

                           // DQ (8/4/2012): This is so far the only place where we specify via the front-end where parenthesis 
                           // may be required.  This result is factored into the logic in the backend (unparser) and "()" are 
                           // output if the argExpression is a binary operator.  It would be better for all of the logic to be 
                           // only in the backend code generation (unparser).
#ifdef DEBUG_CONVERSION
                              printf ("In case eok_dot_member_call: Calling set_need_paren(true) (later we want to move all such logic to the unparser) \n");
#endif
                              argExpression->set_need_paren(true);

                              args.push_back(argExpression);
                            }

                         SgFunctionCallExp* functionCallExp = buildFunctionCallExp_nfi(set_generated_file_info_to_unparse(buildDotExp_nfi(obj, func)),set_generated_file_info_to_unparse(buildExprListExp_nfi(args)));

                      // DQ (4/8/2013): Mark the function call as using operator or non-operator syntax.
                         bool call_uses_operator_syntax = e->variant.operation.call_uses_operator_syntax;

                      // DQ (4/14/2013): Added support for whole translation unit specification of there using the overloaded 
                      // operator names or the operator syntax.  This is mostly used for testing of the unparser to make sure
                      // that both are supported equally.  The default is to use the behavior specified in the original source code.
                         call_uses_operator_syntax = (global_call_uses_operator_syntax == true) ? true  : call_uses_operator_syntax;
                         call_uses_operator_syntax = (global_call_uses_operator_names  == true) ? false : call_uses_operator_syntax;

                         functionCallExp->set_uses_operator_syntax(call_uses_operator_syntax);

                         result = functionCallExp;

                         break;
                       }

                    case eok_dot_static:
                       {
                      // DQ (12/11/2011): This is the same code as in the case eok_dot_field.  so there is an opportunity to refactor this after debugging...
#ifdef DEBUG_CONVERSION
                         indent_line(); debug_print("dot static\n");
#endif
                         OPCOUNT(== 2);
#if 0
                         printf ("case eok_dot_static: e->is_lvalue = %s \n",e->is_lvalue ? "true" : "false");
                         printf ("case eok_dot_static: operands[0] = %s \n",operands[0]->class_name().c_str());
                         printf ("case eok_dot_static: operands[1] = %s \n",operands[1]->class_name().c_str());
                         printf ("case eok_dot_static: operands[0].has_address_of = %s \n",operands[0].has_address_of ? "true" : "false");
#endif
                         if (e->is_lvalue == true)
                            {
                              result = SageBuilder::buildDotExp_nfi(operands[0], operands[1]);
                            }
                           else
                            {
                           // DQ (5/18/2012): Fixup the case where the rhs is a constant value.
                           // result = buildDotExp_nfi(operands[0], operands[1]);
                              if (isSgValueExp(operands[1]) != NULL)
                                 {
                                // If the lhs is a value then we just want the value to be returned (see test2012_90.C and test2005_32.C).
                                   result = operands[1];
                                 }
                                else
                                 {
                                   result = buildDotExp_nfi(operands[0], operands[1]);
                                 }
                            }
                         break;
                       }

                 // DQ (2/11/2012): Added support for eok_class_rvalue_adjust.
                    case eok_class_rvalue_adjust:
                       {
                      // Used to adjust the cv-qualifiers on a class rvalue. The operand is an rvalue with class type.  The
                      // result is the same class rvalue with its type changed to the type of the rvalue-adjust expression.
                      // Used only in C++.  Always compiler-generated.

                      // DQ (2/11/2012): See test2004_35.C for an example of this.
#ifdef DEBUG_CONVERSION
                         indent_line(); debug_print("class rvalue adjust\n");
#endif
                         OPCOUNT(== 1);

#ifdef DEBUG_CONVERSION
                         printf ("case eok_class_rvalue_adjust not represented explicitly in the IR ... \n");
#endif
                         result = operands[0];

#ifdef DEBUG_CONVERSION
                         printf ("In convert_expression(): case eok_class_rvalue_adjust: sharing LHS operand directly = %p = %s \n",result,result->class_name().c_str());
#endif
                         break;
                       }

                 // DQ (2/11/2012): Added support for eok_points_to_vacuous_destructor_call.
                    case eok_points_to_vacuous_destructor_call:
                       {
                      // This case uses the same form as the regular vacuous destructor call by saving only the type (stripped of the pointer type wrapper).

                      // DQ (2/11/2012): See test2004_35.C for an example of this.
#ifdef DEBUG_CONVERSION
                         indent_line(); debug_print("points_to_vacuous_destructor_call\n");
#endif
                         if (operands.size() == 2)
                            {
#if 0
                              printf ("In case eok_points_to_vacuous_destructor_call: CASE OF TWO OPERANDS \n");
                              printf ("Number of operands is different for EDG 4.11: operands.size() = %zu \n",operands.size());
#endif
                              ROSE_ASSERT(operands[0] != NULL);
#if 0
                              printf ("operands[0] = %p = %s \n",operands[0],operands[0]->class_name().c_str());
#endif
                              ROSE_ASSERT(operands[1] != NULL);
#if 0
                              printf ("operands[1] = %p = %s \n",operands[1],operands[1]->class_name().c_str());
#endif
                            }
                           else
                            {
                              OPCOUNT(== 1);
#if 0
                              printf ("In case eok_points_to_vacuous_destructor_call: CASE OF ONE OPERAND \n");
                              printf ("Number of operands is different for EDG 4.11: operands.size() = %zu \n",operands.size());
#endif
                              ROSE_ASSERT(operands[0] != NULL);
#if 0
                              printf ("operands[0] = %p = %s \n",operands[0],operands[0]->class_name().c_str());
#endif
                            }

                      // printf ("case eok_points_to_vacuous_destructor_call not represented explicitly in the IR ... \n");

                         if (operands.size() == 2)
                            {
                              SgPseudoDestructorRefExp* pseudoDestructorRefExp = isSgPseudoDestructorRefExp(operands[1]);
                              ROSE_ASSERT(pseudoDestructorRefExp != NULL);

                              result = buildArrowExp_nfi(operands[0], pseudoDestructorRefExp);
                            }
                           else
                            {
                              result = operands[0];
                            }
                         break;
                       }

                 // DQ (2/25/2012): Added support for eok_pm_points_to_field.
                    case eok_pm_points_to_field:
                       {
                      // Selection of a nonstatic data member of a class using a pointer to member, 
                      // source form x->*pm.  The first operand is an rvalue pointer to class. The 
                      // second operand is an rvalue of pointer-to-data-member type.
#ifdef DEBUG_CONVERSION
                         indent_line(); debug_print("nonstatic data member of a class using a pointer to member \n");
#endif
                         OPCOUNT(== 2);

                      // DQ (7/27/2012): Need ->* by using SgArrowStarOp instead of SgArrowExp.
                         result = buildArrowStarOp_nfi(operands[0], operands[1]);
                         break;
                       }

                 // DQ (3/29/2012): Added support for eok_dot_vacuous_destructor_call.
                    case eok_dot_vacuous_destructor_call:
                       {
                      // This case uses the same form as the regular vacuous destructor call by saving only the type (stripped of the pointer type wrapper).

                      // DQ (2/11/2012): See test2004_35.C for an example of this.
#ifdef DEBUG_CONVERSION
                         indent_line(); debug_print("dots_vacuous_destructor_call\n");
#endif
                      // DQ (10/16/2016): In EDG 4.11 the number of operands for this case is different.
#if 0
                      // DQ (3/5/2017): Converted to use message logging.
                         mprintf ("Number of operands is different for EDG 4.11: operands.size() = %zu \n",operands.size());
#endif
                         if (operands.size() == 2)
                            {
#if 0
                              printf ("In case eok_dot_vacuous_destructor_call: CASE OF TWO OPERANDS \n");
                              printf ("Number of operands is different for EDG 4.11: operands.size() = %zu \n",operands.size());
#endif
                              ROSE_ASSERT(operands[0] != NULL);
#if 0
                              printf ("operands[0] = %p = %s \n",operands[0],operands[0]->class_name().c_str());
#endif
                              ROSE_ASSERT(operands[1] != NULL);
#if 0
                              printf ("operands[1] = %p = %s \n",operands[1],operands[1]->class_name().c_str());
#endif
                            }
                           else
                            {
                              OPCOUNT(== 1);
#if 0
                              printf ("In case eok_dot_vacuous_destructor_call: CASE OF ONE OPERANDS \n");
                              printf ("Number of operands is different for EDG 4.11: operands.size() = %zu \n",operands.size());
#endif
                              ROSE_ASSERT(operands[0] != NULL);
#if 0
                              printf ("operands[0] = %p = %s \n",operands[0],operands[0]->class_name().c_str());
#endif
                            }
                         if (operands.size() == 2)
                            {
                              SgPseudoDestructorRefExp* pseudoDestructorRefExp = isSgPseudoDestructorRefExp(operands[1]);
                              ROSE_ASSERT(pseudoDestructorRefExp != NULL);

                              result = buildDotExp_nfi(operands[0], pseudoDestructorRefExp);
                            }
                           else
                            {
                              result = operands[0];
                            }
                         break;
                       }

                 // DQ (5/17/2012): Added support for this case (see test2005_10.C).
                    case eok_points_to_pm_call:
                       {
                      // A call of a function identified by a pointer to member, with the source form (p->*pmf)(args).
                      // The first operand is the pointer to member function; the second is an rvalue pointer to
                      // class that identifies the selector object; the remaining operands are the arguments.
                      // This is a call to a member function using member function pointer (for example):
                      //    "(object->*(mFieldAccessorMethod))();"

#ifdef DEBUG_CONVERSION
                         indent_line(); debug_print("eok_points_to_pm_call \n");
#endif
                         OPCOUNT(>= 2);
                         SgExpression* func = set_generated_file_info_to_unparse(operands[0]);
                         SgExpression* obj  = set_generated_file_info_to_unparse(operands[1]);
                         vector<SgExpression*> args;
                         for (size_t i = 2; i < operands.size(); ++i)
                            {
                              args.push_back(operands[i]); // FIXME handle references
                            }

                         SgFunctionCallExp* functionCallExp = buildFunctionCallExp_nfi(set_generated_file_info_to_unparse(buildArrowStarOp_nfi(obj, func)),set_generated_file_info_to_unparse(buildExprListExp_nfi(args)));

                      // DQ (4/8/2013): Mark the function call as using operator or non-operator syntax.
                         bool call_uses_operator_syntax = e->variant.operation.call_uses_operator_syntax;

                      // DQ (4/14/2013): Added support for whole translation unit specification of there using the overloaded 
                      // operator names or the operator syntax.  This is mostly used for testing of the unparser to make sure
                      // that both are supported equally.  The default is to use the behavior specified in the original source code.
                         call_uses_operator_syntax = (global_call_uses_operator_syntax == true) ? true  : call_uses_operator_syntax;
                         call_uses_operator_syntax = (global_call_uses_operator_names  == true) ? false : call_uses_operator_syntax;

                         functionCallExp->set_uses_operator_syntax(call_uses_operator_syntax);

                         result = functionCallExp;
                         break;
                       }

                 // DQ (8/6/2012): Added support for this case (see test2012_166.C).
                    case eok_dot_pm_call:
                       {
                      // A call of a function identified by a pointer to member, with the source form (x.*pmf)(args).
                      // The first operand is the pointer to member function; the second is the selector object
                      // (class lvalue or class rvalue); the remaining operands are the arguments.
#ifdef DEBUG_CONVERSION
                         indent_line(); debug_print("eok_dot_pm_call \n");
#endif
                         OPCOUNT(>= 2);
                         SgExpression* func = set_generated_file_info_to_unparse(operands[0]);
                         SgExpression* obj  = set_generated_file_info_to_unparse(operands[1]);
                         vector<SgExpression*> args;
                         for (size_t i = 2; i < operands.size(); ++i)
                            {
                              args.push_back(operands[i]); // FIXME handle references
                            }

                         SgFunctionCallExp* functionCallExp = buildFunctionCallExp_nfi(set_generated_file_info_to_unparse(buildDotStarOp_nfi(obj, func)),set_generated_file_info_to_unparse(buildExprListExp_nfi(args)));

                      // DQ (4/8/2013): Mark the function call as using operator or non-operator syntax.
                         bool call_uses_operator_syntax = e->variant.operation.call_uses_operator_syntax;

                      // DQ (4/14/2013): Added support for whole translation unit specification of there using the overloaded 
                      // operator names or the operator syntax.  This is mostly used for testing of the unparser to make sure
                      // that both are supported equally.  The default is to use the behavior specified in the original source code.
                         call_uses_operator_syntax = (global_call_uses_operator_syntax == true) ? true  : call_uses_operator_syntax;
                         call_uses_operator_syntax = (global_call_uses_operator_names  == true) ? false : call_uses_operator_syntax;

                         functionCallExp->set_uses_operator_syntax(call_uses_operator_syntax);

                         result = functionCallExp;

                      // ROSE_ASSERT(!"case eok_dot_pm_call not implemented!");
                         break;
                       }

                    case eok_ref_dynamic_cast:
                       {
                      // Liao, 10/29/2012, support dynamic cast to a reference type. Explicit creation of reference type is needed since only the base type is stored now. 
                         OPCOUNT(== 1);
#if 0
                         printf ("In convert_expression(): case enk_operation: case eok_ref_dynamic_cast: calling typeOfCast() (implemented as dynamic cast) \n");
#endif
                      // DQ (8/22/2016): Implementing better support for richer C++ style casts.
                      // But I think this maps to an explicit dynamic cast.
#if 0
                         printf ("This should map to an explicit dynamic cast \n");
#endif
                      // SgCastExp::cast_type_enum cast_type = typeOfCast(e);

                      // DQ (1/22/2013): This needs to be checked for being a reference cast and that case handled better.  See test2013_12.C
                      // Note that the value of is_rvalue_reference_cast might also be important.
                      // result = operands[0];
                         if (e->variant.operation.is_reference_cast == false)
                            {
                              result = buildCastExp_nfi(operands[0], buildReferenceType(convert_type(e->type)), SgCastExp::e_dynamic_cast);
                            }
                           else
                            {
                           // DQ (1/22/2013): Currently the this case is the same as the false case, but I think this needs to be changed shortly (but I don't think I have an example of this problem).
#ifdef DEBUG_CONVERSION
                              printf ("In convert_expression(): case eok_ref_dynamic_cast: is_reference_cast == true (need a cast expression here) is_rvalue_reference_cast = %s \n",e->variant.operation.is_rvalue_reference_cast ? "true" : "false");
#endif
                              result = buildCastExp_nfi(operands[0], buildReferenceType(convert_type(e->type)), SgCastExp::e_dynamic_cast);
#if 0
                              printf (" Need an example of this case! \n");
                              ROSE_ASSERT(false);
#endif
                            }

                         break; // FIXME handle lvalue and rvalue cases
                       }

                 // DQ (2/5/2014): Added support for C++11 operator noexcept.
                    case eok_noexcept:
                       {
                      // C++11 noexcept operator.  The operand is not evaluated; the result is a constant that indicates
                      // whether the expression can throw an exception (false) or not (true).  Only used as a backing
                      // expression or in a template-dependent constant.  The operand can be an lvalue or an rvalue.
#if 0
                         printf ("In convert_expression() case eok_noexcept: Calling summarize_expression() \n");
                         summarize_expr(e);
#endif
                         SgExpression* expr = operands[0];
                         ROSE_ASSERT(expr != NULL);
                         result = buildNoexceptOp_nfi(expr);
#if 0
                         printf ("Case eok_noexcept not handled! inner_kind = %s \n",summarize_expr_operator(inner_kind).c_str());
                         ROSE_ASSERT(false);
#endif
                         break;
                       }

                    case eok_await: ROSE_ASSERT (!"eok_await"); break;
                    case eok_yield: ROSE_ASSERT (!"eok_yield"); break;
                    case eok_error: ROSE_ASSERT (!"eok_error"); break;
                    case eok_parens:
                    case eok_last:
                       {
                         printf ("Case not handled! inner_kind = %s \n",summarize_expr_operator(inner_kind).c_str());
                         ROSE_ASSERT(false);
                         break;
                       }

                    default:
                       {
                      // ERROR_MESSAGE(stderr, "Bad operation kind %d\n", inner_kind);
                         printf ("Case not handled! inner_kind = %s \n",summarize_expr_operator(inner_kind).c_str());
                         ROSE_ASSERT(false);
                       }
                  }

            // DQ (12/30/2008): Added debugging code (test2003_30.C fails to have parent set)
               for (size_t i = 1; i < operands.size(); ++i)
                  {
                    ROSE_ASSERT(operands[i] != NULL);
                    if (operands[i]->get_parent() == NULL)
                       {
#ifdef DEBUG_CONVERSION
                         printf ("########## Warning: operands[i]->get_parent() == NULL \n");
#endif
                       }
                 // ROSE_ASSERT(operands[i]->get_parent() != NULL);
                  }

            // DQ (11/3/2012): Added assertions.
               if (result->get_startOfConstruct() == NULL)
                  {
                 // printf ("In convert_expression(): (startOfConstruct() == NULL): an_expr_node_ptr e = %p inner_kind = %s result = %p = %s \n",e,summarize_expr_kind(kind),result,result->class_name().c_str());
                    printf ("In convert_expression(): (startOfConstruct() == NULL): operator: inner_kind = %" PRIuPTR " %s operator: %s is an %s result = %p = %s \n",(size_t)inner_kind,summarize_expr_operator(inner_kind).c_str(),db_operator_names[inner_kind], (e->variant.operation.returns_lvalue_instead_of_usual_rvalue ? "lvalue" : "rvalue"),result,result->class_name().c_str());
                  }
               ROSE_ASSERT(result->get_startOfConstruct() != NULL);
               ROSE_ASSERT(result->get_endOfConstruct()   != NULL);
               ROSE_ASSERT(result->get_operatorPosition() != NULL);

  return result;
}

}
