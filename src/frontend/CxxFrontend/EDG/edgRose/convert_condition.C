
#include "edgRose.h"

SgStatement* EDG_ROSE_Translation::convert_condition(an_expr_node_ptr e, SourceSequenceList& sse, a_statement_ptr target_statement_after_conditional)
   {
#ifdef DEBUG_CONVERSION
     indent_line(); debug_print("condition\n");
     ScopedIndent si(indent);
#endif

     an_expr_node_kind kind = (an_expr_node_kind)(e->kind);

#define DEBUG_CONVERT_CONDITION 0

  // DQ (9/23/2020): I think we can assert this (this fails for a switch statement (see Cxx11_tests/test2020_103.C).
  // ROSE_ASSERT(target_statement_after_conditional != NULL);

#if DEBUG_CONVERT_CONDITION
     printf ("At TOP of convert_condition() kind = %s \n",output_expr_enum(kind).c_str());
     printf (" --- target_statement_after_conditional = %p = %s \n",target_statement_after_conditional,summarize_statement_in_string(target_statement_after_conditional));
#endif

#if DEBUG_CONVERT_CONDITION
     printf ("In convert_condition(): before processEntriesRelatedToConditional(): iek_src_seq_end_of_construct entry_kind = %p = %s \n",
          sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif

#if DEBUG_CONVERT_CONDITION
     printf ("Calling processEntriesRelatedToConditional() \n");
#endif
  // DQ (7/5/2019): This can return a list of statements (see test2019_05.c for an example).
  // DQ (7/1/2019): Adding support (refactored and moved from the end of this function) to process types 
  // and constructs (e.g. GNU statement expressions) that are a part of the conditional.
  // SgStatement* extraStatement = processEntriesRelatedToConditional( sse, target_statement_after_conditional );
     std::vector<SgStatement*> extraStatementList = processEntriesRelatedToConditional( sse, target_statement_after_conditional );

#if DEBUG_CONVERT_CONDITION
     printf ("DONE: Calling processEntriesRelatedToConditional() \n");
#endif

#if 0
     if (extraStatement != NULL)
        {
          printf ("extraStatement = %p = %s \n",extraStatement,extraStatement->class_name().c_str());
        }
#endif

#if DEBUG_CONVERT_CONDITION
  // DQ (7/5/2019): Debugging information.
     if (extraStatementList.empty() == false)
        {
          printf ("extraStatementList.size() = %zu \n",extraStatementList.size());
          for (size_t i = 0; i < extraStatementList.size(); i++)
             {
               ROSE_ASSERT(extraStatementList[i] != NULL);
               printf (" --- extraStatementList[%zu] = %p = %s \n",i,extraStatementList[i],extraStatementList[i]->class_name().c_str());
             }
        }
       else
        {
          printf ("extraStatementList is empty \n");
        }
#endif

#if DEBUG_CONVERT_CONDITION
     printf ("In convert_condition(): after processEntriesRelatedToConditional(): iek_src_seq_end_of_construct entry_kind = %p = %s \n",
          sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif

     SgStatement* statement = NULL;
     if (kind == enk_condition)
        {
       // statement = set_basic_file_info_for_expr(buildVariableDeclaration_nfi(convert_expression(e).value()), e);

          a_condition_supplement_ptr condition_supplement = e->variant.condition;
          ROSE_ASSERT(condition_supplement != NULL);

          ROSE_ASSERT(condition_supplement->expr != NULL);

       // Build the required type.
          ROSE_ASSERT(condition_supplement->expr->type != NULL);

#if 0
       // DQ (7/14/2014): Look for a better representation of the type to use (for test2014_107.C).
       // However, I think this is always a bool type, so it is not very representative of the original source code.
          ROSE_ASSERT(condition_supplement->expr->type != NULL);
          SgType* type_from_condition_expr = convert_type(condition_supplement->expr->type);
          printf ("What is a better type: type_from_condition_expr = %p = %s = %s \n",type_from_condition_expr,type_from_condition_expr->class_name().c_str(),type_from_condition_expr->unparseToString().c_str());
#endif
       // Build the initializer (if it is available).
          ROSE_ASSERT(condition_supplement->dynamic_init != NULL);
          SgInitializer* initializer = NULL;
          if (condition_supplement->dynamic_init != NULL)
             {
#if DEBUG_CONVERT_CONDITION
               printf ("Note: Processing the initializer in convert_condition() \n");
#endif
               SgExpression* temp_expr = convert_dynamic_initializer(condition_supplement->dynamic_init,condition_supplement->expr->type);
#if DEBUG_CONVERT_CONDITION
               printf ("What is this: temp_expr = %p = %s \n",temp_expr,temp_expr->class_name().c_str());
#endif
            // If this is a SgCastExp then initializer will be false, so wrap this to be an initializer (for now).
               initializer = wrap_expr_as_initializer(temp_expr);
#if DEBUG_CONVERT_CONDITION
               printf ("What is this: initializer = %p = %s \n",initializer,initializer->class_name().c_str());
#endif
               ROSE_ASSERT(initializer != NULL);

#if DEBUG_CONVERT_CONDITION
            // DQ (1/17/2019): Mark this as an initializer used in a conditional.
               printf ("Mark this as an initializer used in a conditional \n");
#endif
            // DQ (1/17/2019): Mark this as an initializer used in a conditional.
               SgConstructorInitializer* constructorInitializer = isSgConstructorInitializer(initializer);
               if (constructorInitializer != NULL)
                  {
                    constructorInitializer->set_is_used_in_conditional(true);
                  }
             }

       // Build the scope.
          SgScopeStatement* scope = topScopeStack();
          ROSE_ASSERT(scope != NULL);

       // DQ (2/13/2012): Not clear why we can't use condition_supplement->expr->type, but it evaluates to SgCharType (which is wrong).
          ROSE_ASSERT(initializer != NULL);

       // DQ (7/14/2014): This should not be the initializer's type and should be the type more directly identified 
       // independent of the initializer (which could reflect a cast).  See test2014_107.C for an example.
          SgType* type = initializer->get_type();
#if DEBUG_CONVERT_CONDITION
          printf ("What is the type: type = %p = %s = %s \n",type,type->class_name().c_str(),type->unparseToString().c_str());
#endif
#if 0
          printf ("Calling summarize_expression(e) \n");
          summarize_expr(e);
#endif
#if 0
          printf ("Calling summarize_expression(condition_supplement->expr) \n");
          summarize_expr(condition_supplement->expr);
#endif

#if 0
       // DQ (7/14/2014): Look for a better representation of the type to use (for test2014_107.C).
       // However, I think this is always a bool type, so it is not very representative of the original source code.
          ROSE_ASSERT(e->type != NULL);
          SgType* type_from_expr = convert_type(e->type);
          printf ("What is a better type: type_from_expr = %p = %s = %s \n",type_from_expr,type_from_expr->class_name().c_str(),type_from_expr->unparseToString().c_str());
#endif

#ifdef DEBUG_CONVERSION
          printf ("Error: Skipping generation of correct conditional variable name in convert_condition() \n");
#endif
          SgName name = "unknown_condition";

       // DQ (7/14/2014): This is supporting the boost foreach macro, however it is building a redundant expression tree in ROSE which is likely a problem longer-term.
          SgExpression* expr_from_condition = convert_expression(condition_supplement->expr);

#ifdef DEBUG_CONVERSION
       // DQ (7/15/2014): Note that this causes an error for some test codes (e.g. test2014_124.C).
       // printf ("In convert_condition(): redundantly constructed expression: expr_from_condition = %p = %s = %s \n",expr_from_condition,expr_from_condition->class_name().c_str(),expr_from_condition->unparseToString().c_str());
          printf ("In convert_condition(): redundantly constructed expression: expr_from_condition = %p = %s \n",expr_from_condition,expr_from_condition->class_name().c_str());
#endif
       // DQ (7/14/2014): Adding support for boost foreach.
          SgFunctionCallExp* functionCallExp = isSgFunctionCallExp(expr_from_condition);
          if (functionCallExp != NULL)
             {
            // In the case of a boost foreach, we need to handle the generation of the correct type.
               SgExpression* dot_or_arrow_expr = isSgExpression(functionCallExp->get_function());
               ROSE_ASSERT(dot_or_arrow_expr != NULL);
               SgDotExp* dotExp = isSgDotExp(dot_or_arrow_expr);
               ROSE_ASSERT(dotExp != NULL);

               SgVarRefExp* varRefExp = isSgVarRefExp(dotExp->get_lhs_operand());
               ROSE_ASSERT(varRefExp != NULL);

               SgMemberFunctionRefExp* memberFunctionRefExp = isSgMemberFunctionRefExp(dotExp->get_rhs_operand());
               if (memberFunctionRefExp != NULL)
                  {
                    name = varRefExp->get_symbol()->get_name();

                    SgType* type_from_condition_expr = varRefExp->get_type();
#if DEBUG_CONVERT_CONDITION
                    printf ("In convert_condition() What is a better type: type_from_condition_expr = %p = %s = %s \n",
                         type_from_condition_expr,type_from_condition_expr->class_name().c_str(),type_from_condition_expr->unparseToString().c_str());
                    if (initializer != NULL)
                       {
                         printf ("In convert_condition() initializer = %p = %s = %s \n",initializer,initializer->class_name().c_str(),initializer->unparseToString().c_str());
                       }
#endif
                    type = type_from_condition_expr;
                  }
             }

#ifdef DEBUG_CONVERSION
          printf ("In convert_condition() kind == enk_condition: condition_supplement->expr->kind = %s \n",output_expr_enum(condition_supplement->expr->kind).c_str());
#endif
       // DQ (2/15/2012): For non-switch statement the variable name is hidden behind a boolean expression.
       // ROSE_ASSERT(condition_supplement->expr->kind == enk_operation);
          an_expr_node_ptr lhs = NULL;
          if (condition_supplement->expr->kind == enk_operation)
             {
            // an_expr_operator_kind_tag inner_kind = (an_expr_operator_kind_tag)(e->variant.operation.kind);
#ifdef DEBUG_CONVERSION
               printf ("In convert_condition() condition_supplement->expr->kind == enk_operation: condition_supplement->expr->variant.operation.kind = %s \n",summarize_expr_operator(condition_supplement->expr->variant.operation.kind).c_str());
#endif
            // DQ (6/10/2012): This fails for test2012_103.C and test2012_104.C.
            // ROSE_ASSERT(condition_supplement->expr->variant.operation.kind == eok_bool_cast);
               if (condition_supplement->expr->variant.operation.kind == eok_bool_cast)
                  {
#ifdef DEBUG_CONVERSION
                    printf ("In convert_condition() condition_supplement->expr->kind == enk_operation: this IS a boolean cast \n");
#endif
                 // The lhs expression is a variable that refers to the variable.
                    lhs = condition_supplement->expr->variant.operation.operands;
                  }
                 else
                  {
#ifdef DEBUG_CONVERSION
                    printf ("In convert_condition() condition_supplement->expr->kind == enk_operation: this is NOT a boolean cast \n");
#endif
                 // DQ (6/10/2012): This fails for test2012_103.C, and test2012_104.C (simple declaration as conditional test in for loop).
                    ROSE_ASSERT(condition_supplement->expr != NULL);
                 // lhs = convert_expression(condition_supplement->expr);
                 // lhs = condition_supplement->expr;
                    lhs = condition_supplement->expr->variant.operation.operands;
                    ROSE_ASSERT(lhs != NULL);

                 // DQ (6/19/2012): Get the 2nd operand (which is the variable).
                    lhs = lhs->next;
                    ROSE_ASSERT(lhs != NULL);
                  }
             }
            else
             {
            // DQ (2/15/2012): For a switch statement with variable declaration in the condition, the lhs is just the condition_supplement->expr.
               lhs = condition_supplement->expr;
               ROSE_ASSERT(lhs->kind == enk_variable || lhs->kind == enk_constant);
             }

#if DEBUG_CONVERT_CONDITION
          printf ("In convert_condition(): lhs = %p \n",lhs);
          printf ("In convert_condition(): initializer = %p = %s \n",initializer,initializer != NULL ? initializer->class_name().c_str() : "NULL");
#endif

          ROSE_ASSERT(lhs != NULL);

       // DQ (6/19/2012): The statement has not be set yet.
          ROSE_ASSERT(statement == NULL);

#ifdef DEBUG_CONVERSION
       // printf ("In convert_condition() lhs->kind = %s \n",summarize_expr_operator(lhs->kind).c_str());
          printf ("In convert_condition() lhs->kind = %s \n",summarize_expr_kind(lhs->kind));
#endif
       // ROSE_ASSERT(lhs->kind == enk_variable);

       // In the code below we only want to get the name for the variable that is the simple declaration.
       // Nothing more complex is allowed by the C/C++ grammer inside of a conditional.
          a_variable_ptr edg_variable = NULL;
          if (lhs->kind == enk_variable)
             {
               edg_variable = lhs->variant.variable.ptr;
               ROSE_ASSERT(edg_variable != NULL);

            // printf ("Calling summarize_variable(edg_variable) \n");
            // summarize_variable(edg_variable);

               ROSE_ASSERT(edg_variable->source_corresp.name != NULL);
               name = edg_variable->source_corresp.name;
             }
            else
             {
               if (lhs->kind == enk_constant)
                  {
                    a_constant_ptr edg_constant = lhs->variant.constant.ptr;
                    ROSE_ASSERT(edg_constant != NULL);
                    an_expr_node_ptr edg_constant_expression = edg_constant->expr;
                    ROSE_ASSERT(edg_constant_expression != NULL);
#if 0
                    printf ("What is this: calling summarize_expression(): edg_constant_expression\n");
                    summarize_expr(edg_constant_expression);
#endif
                 // Mark this as a "const" type.
                    type = SageBuilder::buildConstType(type);
                    if (edg_constant_expression->kind == enk_operation)
                       {
#if 0
                         printf ("What is this: calling summarize_expression(): edg_constant_expression->variant.operation.operands \n");
                         summarize_expr(edg_constant_expression->variant.operation.operands);
#endif
                         ROSE_ASSERT(edg_constant_expression->variant.operation.operands != NULL);
#if DEBUG_CONVERT_CONDITION
                         printf ("In convert_condition(): edg_constant_expression->variant.operation.operands->kind = %s \n",summarize_expr_kind(edg_constant_expression->variant.operation.operands->kind));
#endif
                      // ROSE_ASSERT(edg_constant_expression->variant.operation.operands->kind == enk_variable);
                         if (edg_constant_expression->variant.operation.operands->kind == enk_variable)
                            {
                           // Not clear now to set this.
                              edg_variable = edg_constant_expression->variant.operation.operands->variant.variable.ptr;

                              ROSE_ASSERT(edg_variable != NULL);

                              ROSE_ASSERT(edg_variable->source_corresp.name != NULL);
                              name = edg_variable->source_corresp.name;
                            }
                           else
                            {
                           // DQ (10/20/2016): This case happens only with EDG 4.11 and later on test2016_88.C.
                              ROSE_ASSERT(edg_constant_expression->variant.operation.operands->kind == enk_constant);
                              ROSE_ASSERT(edg_constant_expression->variant.operation.operands->variant.constant.ptr != NULL);
#if DEBUG_CONVERT_CONDITION
                              printf ("edg_constant_expression->variant.operation.operands->variant.constant.ptr->kind = %s \n",
                                   summarize_expr_kind(edg_constant_expression->variant.operation.operands->variant.constant.ptr->kind));
                           // ROSE_ASSERT(edg_variable != NULL);
#endif
                              SgExpression* expression = convert_expression(edg_constant_expression);

                              SgStatement* tmp_statement = buildExprStatement_nfi(expression);
                              ROSE_ASSERT(tmp_statement != NULL);

                           // Setup the statement as a expression statement here and it will not be setup as a variable declaration below.
                              statement = set_basic_file_info_for_expr(tmp_statement, e);
                            }
                       }
                      else
                       {
                      // DQ (10/18/2016): This is a case that is not handled yet, or might not exist.
#if DEBUG_CONVERT_CONDITION
                         printf ("In convert_condition(): edg_constant_expression->kind = %s \n",summarize_expr_kind(edg_constant_expression->kind));
#endif
                      // DQ (1/29/2019): Adding variable declaration for constexpr variables within conditionals (see Cxx11_tests/test2019_50.C);
                         if (edg_constant_expression->kind == enk_variable)
                            {
                           // SgVariableDeclaration* convert_variable_declaration(a_variable_ptr variable);
                              a_variable_ptr edg_variable = edg_constant_expression->variant.variable.ptr;
                              ROSE_ASSERT(edg_variable != NULL);
#if DEBUG_CONVERT_CONDITION
                              printf ("Found variable (building as a variable declaration): edg_variable = %p \n",edg_variable);
#endif
                              SgVariableDeclaration* variableDeclaration = convert_variable_declaration(edg_variable);
                              ROSE_ASSERT(variableDeclaration != NULL);
                              statement = variableDeclaration;
                            }
                       }
                  }
                 else
                  {
                 // DQ (2/18/2013): This branch is taken as part of ROSE compiling rose.h and not any other code that I know of.
#if 0
                    printf ("In convert_condition() lhs->kind = %s (branch taken in boost processing) \n",summarize_expr_kind(lhs->kind));
#endif
#if 0
                    printf ("What is this: calling summarize_expression(): lhs \n");
                    summarize_expr(lhs);
#endif
                 // DQ (2/11/2013): This is the case for "if (optional<Vertex> vertex = find_vertex(name, g))" 
                 // in /home/dquinlan/local/boost_1_42_0_installTree-gxx-4.2.4/include/boost/graph/named_graph.hpp:401:29
                 // I'm not clear how to process this at the moment.

                    ROSE_ASSERT(lhs != NULL);
                    mprintf ("Not clear how to process this expression, proceeding with incomplete information edg lhs = %p \n",lhs);
                  }
             }

#if DEBUG_CONVERT_CONDITION
          printf ("In convert_condition(): statement = %p \n",statement);
#endif

       // DQ (10/20/2016): If the statement has not been set then make it a variable declaration.
          if (statement == NULL)
             {
#if DEBUG_CONVERT_CONDITION
               printf ("Conditional variable name = %s \n",name.str());
               printf ("   --- type        = %p = %s = %s \n",type,type->class_name().c_str(),type->unparseToString().c_str());
               printf ("   --- initializer = %p = %s = %s \n",initializer,initializer->class_name().c_str(),initializer->unparseToString().c_str());
#endif

            // SgVariableDeclaration* SageBuilder::buildVariableDeclaration_nfi (const SgName & name, SgType* type, SgInitializer * varInit, SgScopeStatement* scope)
               statement = set_basic_file_info_for_expr(buildVariableDeclaration_nfi(name,type,initializer,scope), e);
#if 0
               printf ("In convert_condition(): statement = %p = %s = %s \n",statement,statement->class_name().c_str(),statement->unparseToString().c_str());
               SgVariableDeclaration* variableDeclaration = isSgVariableDeclaration(statement);
               if (variableDeclaration != NULL)
                  {
                    SgInitializedName* initializedName = SageInterface::getFirstInitializedName(variableDeclaration);
                    ROSE_ASSERT(initializedName != NULL);
                    printf ("In convert_condition(): condition statement's initializedName = %p = %s \n",initializedName,initializedName->unparseToString().c_str());
                    ROSE_ASSERT(initializedName->get_initptr() != NULL);
                  }
#endif
            // Lookup the symbol.
               SgVariableSymbol* variableSymbol = scope->lookup_variable_symbol(name);
               ROSE_ASSERT(variableSymbol != NULL);

#if 0
               printf ("In convert_condition(): Putting variableSymbol name = %s into variable_cache using key edg_variable = %p EDG name = %s \n",
                    variableSymbol->get_name().str(),edg_variable,edg_variable->source_corresp.name);
#endif
            // DQ (8/29/2020): I think we can enforce this now.
            // ROSE_ASSERT(variable_cache.find(edg_variable) == variable_cache.end());

            // DQ (8/29/2020): Make sure we don't put NULL pointes into the variable_cache.
               ROSE_ASSERT(variableSymbol != NULL);

            // Add the symbol to the variable_cache.
            // variable_cache[edg_variable] = variableSymbol;

            // DQ (8/29/2020): Changed this to avoid overwriting entries already in the variable_cache.
            // variable_cache[p] = sym;
               if (edg_variable != NULL)
                  {
               if (variable_cache.find(edg_variable) == variable_cache.end())
                  {
                 // DQ (8/29/2020): Make sure we don't put NULL pointes into the variable_cache.
                    ROSE_ASSERT(variableSymbol != NULL);

                 // If it is not in the variable_cache then add it.
                 // variable_cache[p] = sym;
                    variable_cache[edg_variable] = variableSymbol;
                  }
                 else
                  {
                 // But avoid overwritting it if it is already present.
#if DEBUG_VARIABLE_CACHE
                    ROSE_ASSERT(edg_variable != NULL);
                    ROSE_ASSERT(variableSymbol != NULL);

                    printf ("NOTE: In convert_condition(): test 1: variable edg_variable = %p name = %s is already present in the variable_cache: variableSymbol name = %s \n",
                         edg_variable,edg_variable->source_corresp.name,variableSymbol->get_name().str());
#endif
                  }
                  }
                 else
                  {
#if DEBUG_VARIABLE_CACHE
                    printf ("NOTE: In convert_condition(): test 1: edg_variable == NULL: variableSymbol->get_name() = %s \n",variableSymbol->get_name().str());
#endif
                  }

               SgVariableDeclaration* decl = isSgVariableDeclaration(statement);
               ROSE_ASSERT(decl != NULL);
            // fixup_variable_or_field_declaration(decl, e->type);
            // fixup_variable_or_field_declaration(decl, condition_supplement->expr->type);

            // Not clear how to get proper source position information.
               set_generated_file_info(getFirstInitializedName(decl));
               set_generated_file_info(decl->get_definition());

               ROSE_ASSERT(getFirstInitializedName(decl)->get_scope() != NULL);
#if 0
            // Testing to see where the index variable is referenced (available).
               printf ("Testing to see where the index variable is referenced (available). \n");
               SgExpression* bool_expr = convert_expression(condition_supplement->expr).value();
               printf ("bool_expr = %p = %s \n",bool_expr,bool_expr->class_name().c_str());
               printf ("bool_expr->unparseToString() = %s \n",bool_expr->unparseToString().c_str());
#endif
#if 0
               printf ("Exiting at end of convert_condition(): case of kind == enk_condition \n");
               ROSE_ASSERT(false);
#endif
             }

#if 0
          printf ("We have previously incremented the sse, but maybe we don't need to do this now! \n");
          printf ("In convert_condition(): (before increment sse): entry = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif

       // printf ("Skipping previous increment of sse \n");

       // DQ (7/2/2019): Skipping previous increment of sse.
       // I think we have to increment this to avoid reparsing the same line in the body (of the conditional statement).
       // ++sse;

#if 0
          printf ("In convert_condition(): (after increment sse): entry = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif

       // ROSE_ASSERT(kind != enk_condition);
        }
       else
        {
#if DEBUG_CONVERT_CONDITION
          printf ("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ \n");
          printf ("In convert_condition(): predicate not processed as conditional (calling convert_expression(e = %p)) \n",e);
          printf ("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ \n");
#endif
#if 0
          printf ("In convert_condition(): predicate not processed as conditionalCalling summarize_expression(e) \n");
          summarize_expr(e);
#endif

       // DQ (7/5/2019): If the extra code in the conditional is just a declaration then process the expression 
       // (and add the extra declaration), else it is likely a block and will be a SgStatementExpression.
       // if (extraStatement != NULL)
       // SgDeclarationStatement* extraDeclaration = isSgDeclarationStatement(extraStatement);
       // SgDeclarationStatement* extraDeclaration = NULL;

          SgStatement* extraStatement = NULL;

          if (extraStatementList.empty() == false)
             {
#if DEBUG_CONVERT_CONDITION
               printf ("extraStatementList.size() = %zu \n",extraStatementList.size());
               for (size_t i = 0; i < extraStatementList.size(); i++)
                  {
                    ROSE_ASSERT(extraStatementList[i] != NULL);
                    printf (" --- extraStatementList[%zu] = %p = %s \n",i,extraStatementList[i],extraStatementList[i]->class_name().c_str());
                  }
#endif
               if (extraStatementList.size() == 1)
                  {
                    extraStatement = extraStatementList[0];

                    ROSE_ASSERT(extraStatement != NULL);
                 // extraDeclaration = isSgDeclarationStatement(extraStatement);
                  }
                 else
                  {
                 // DQ (7/5/2019): Not clear how to handle this case, but the code below works well and any additional source 
                 // sequence points have had code generated for them at this point, so maybe we don't have a problem.

                    ROSE_ASSERT(extraStatementList.size() > 1);
#if DEBUG_CONVERT_CONDITION
                    printf ("NOTE: There are multiple statements from the conditional \n");
#endif
                  }
             }
            else
             {
#if DEBUG_CONVERT_CONDITION
               printf ("extraStatementList is empty \n");
#endif
             }

#if DEBUG_CONVERT_CONDITION
       // printf ("extraDeclaration = %p \n",extraDeclaration);
#endif

#if 0
       // DQ (7/6/2019): It appears that we just need the origial code, and to have translated what were previously 
       // missing source sequence points.

       // DQ (7/6/2019): Force the false branch in the case of test2019_07.c.
          if (extraDeclaration == NULL && extraStatement != NULL)
             {
            // DQ (7/3/2019): This is likely a better approach to supporting statements in conditionals.
               statement = extraStatement;
             }
            else
#endif
             {
            // Note: set_basic_file_info_for_expr() works for a SgStatement because it is a template function.
            // statement = set_basic_file_info_for_expr(buildExprStatement_nfi(convert_expression(e).value()), e);
               SgExpression* expression = convert_expression(e);
               ROSE_ASSERT(expression != NULL);
               SgStatement* tmp_statement = buildExprStatement_nfi(expression);
               ROSE_ASSERT(tmp_statement != NULL);
#if 0
               printf ("Built a SgExprStatement = %p \n",tmp_statement);
               printf (" --- expression = %p = %s \n",expression,expression->class_name().c_str());
#endif
               statement = set_basic_file_info_for_expr(tmp_statement, e);
#if 0
               statement->get_file_info()->display("In convert_condition(): debug");
#endif
               SgCastExp* castExp = isSgCastExp(expression);
            // if (castExp != NULL && castExp->isCompilerGenerated() == true)
               if (statement->isCompilerGenerated() == true && castExp != NULL && castExp->isCompilerGenerated() == true)
                  {
#if DEBUG_CONVERT_CONDITION
                    printf ("Fixup the source position of the statement containing the SgCastExp \n");
#endif
                 // Set the source position of the statement to the source position of the cast's operand.
                    SgExpression* operand = castExp->get_operand();
                    ROSE_ASSERT(operand != NULL);
#if DEBUG_CONVERT_CONDITION
                    printf ("Fixup the source position of the statement containing the SgCastExp operand = %p = %s \n",operand,operand->class_name().c_str());
#endif
                    if (operand->isCompilerGenerated() == false)
                       {
                         *(tmp_statement->get_startOfConstruct()) = *(operand->get_startOfConstruct());
                         *(tmp_statement->get_endOfConstruct())   = *(operand->get_endOfConstruct());

                         ROSE_ASSERT(statement->isCompilerGenerated() == false);
                       }
                      else
                       {
                         SgArrowExp* arrowExp = isSgArrowExp(operand);
                         if (arrowExp != NULL && arrowExp->isCompilerGenerated() == true)
                            {
                              SgExpression* arrowExp_operand = arrowExp->get_rhs_operand();
                              ROSE_ASSERT(arrowExp_operand != NULL);

                              *(tmp_statement->get_startOfConstruct()) = *(arrowExp_operand->get_startOfConstruct());
                              *(tmp_statement->get_endOfConstruct())   = *(arrowExp_operand->get_endOfConstruct());
#if DEBUG_CONVERT_CONDITION
                              printf ("Example of compiler generated operand in cast expression: (not a compiler generated SgArrowOp): arrowExp_operand = %p = %s \n",arrowExp_operand,arrowExp_operand->class_name().c_str());
#endif
                           // DQ (5/3/2016): This fails for an specific file.
                              if (tmp_statement->isCompilerGenerated() == true)
	                         {
#if 0
                                   tmp_statement->get_file_info()->display("Example of compiler generated operand in cast expression: (not a compiler generated SgArrowOp)");
#endif
                                 }
                           // ROSE_ASSERT(tmp_statement->isCompilerGenerated() == false);
                              ROSE_ASSERT(tmp_statement == statement);
                           // ROSE_ASSERT(statement->isCompilerGenerated() == false);
                            }
                           else
                            {
#if 0
                              printf ("In convert_condition(): predicate not processed as conditionalCalling summarize_expression(e) \n");
                              summarize_expr(e);
#endif
#if DEBUG_CONVERT_CONDITION
                              printf ("Example of compiler generated operand in cast expression: (not a SgArrowOp): operand = %p = %s \n",operand,operand->class_name().c_str());
#endif
                           // ROSE_ASSERT(false);
                            }
                       }
#if 0
                    printf ("Exiting as a test! \n");
                    ROSE_ASSERT(false);
#endif
                  }
             }
        }

  // DQ (7/3/2019): This is a failing assertion now that we are using the new extraStatement.
     if (statement == NULL)
        {
          printf ("This should be a valid pointer before using the extraStatement \n");
        }
     ROSE_ASSERT(statement != NULL);

#if DEBUG_CONVERT_CONDITION
     printf ("In convert_condition(): target_statement_after_conditional = %p \n",target_statement_after_conditional);
#endif

#if 0
  // DQ (7/1/2019): Refactor to a seperate function, so that it can be called from the top of the function.

  // DQ (9/6/2013): Only use this approach when we have a conditional and not a "simple expression".
  // DQ (9/6/2013): In the case of a type declared in a statement expression the type and some other things can be in the 
  // source sequence list and need to be skipped over.  This code below will loop over the entries in the source sequence
  // list to skip over nodes that are between the end of the conditional expression and the next relevant statement (when
  // that statement is provide as input to the function.  This only addressed the GNU extension of statement expressions
  // and is only useful as part of the handling to type declarations appearing in such statement expressions.
     if (target_statement_after_conditional != NULL)
  // if (target_statement_after_conditional != NULL && kind == enk_condition)
        {
       // DQ (9/6/2013): Now we know that we have a valid loop body for the while statement, so if this is a type we need 
       // to skip over it (similar to the processing for the stmk_if case (above).
#if DEBUG_CONVERT_CONDITION
          printf ("Now we know we have a valid target_statement_after_conditional (%p) for the conditional statement \n",target_statement_after_conditional);
          printf ("In convert_condition(): (target_statement_after_conditional != NULL): entry = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif

       // DQ (9/9/2013): We need to evaluate if this is a good node to use.  If it is a compiler generated node then it 
       // would not appear in the source sequence list and the following loop will run off the end of the source sequence 
       // list to the end of the problem.

       // If this is not a valid source sequence entry then we will need to find a better statement to use instead.
#if DEBUG_CONVERT_CONDITION
          printf ("target_statement_after_conditional->source_sequence_entry = %p \n",target_statement_after_conditional->source_sequence_entry);
#endif
          if (target_statement_after_conditional->source_sequence_entry == NULL)
             {
            // Find a better target since this target will not be found in the source sequence list.
               switch(target_statement_after_conditional->kind)
                  {
                    case stmk_block:
                       {
                      // If this is a stmk_block, then find the first statement with a valid source_sequence_entry pointer.
                         a_statement_ptr s = target_statement_after_conditional->variant.block.statements;
                         while(s != NULL && s->source_sequence_entry == NULL) 
                            {
#if DEBUG_CONVERT_CONDITION
                              printf ("$$$$$$$$$$$$$$$$$$ In convert_condition(): iterate over the statements appearing after the conditional : s = %p kind = %s \n",s,summarize_statement_in_string(s));
#endif
                              s = s->next;
                            }
                         target_statement_after_conditional = s;
                         ROSE_ASSERT(target_statement_after_conditional != NULL);
                         break;
                       }

                    default:
                       {
                          printf ("In convert_condition(): (target_statement_after_conditional->source_sequence_entry == NULL): default reached in selecting better canidate target_statement_after_conditional \n");
                          printf ("   --- target_statement_after_conditional->kind = %s \n",summarize_statement_in_string(target_statement_after_conditional));
                          ROSE_ASSERT(false);
                       }
                  }
             }
          ROSE_ASSERT(target_statement_after_conditional->source_sequence_entry != NULL);

       // For more complex cases we need to just increment the sse until we reach the next statement (which we know is the loop_statement).
#if DEBUG_CONVERT_CONDITION
          printf ("In convert_condition(): We might want to increment the sse until we reach the target_statement_after_conditional = %p \n",target_statement_after_conditional);
#endif
       // DQ (9/6/2013): If we are not already at the expected next statement, then iterate past the end of the current construct.
          if ((a_statement_ptr)(sse->entity.ptr) != target_statement_after_conditional)
             {
               while ((a_statement_ptr)(sse->entity.ptr) != target_statement_after_conditional)
                  {
#if DEBUG_CONVERT_CONDITION
                    printf ("In convert_condition(): Increment sse until we reach target_statement_after_conditional = %p \n",target_statement_after_conditional);
                    printf ("In convert_condition(): BEFORE: (increment until we reach the target_statement_after_conditional): entry = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif

                 // DQ (7/1/2019): Added support to process the clas declaration that can be hidden in a conditional (of a if stmt for example).
                    SgDeclarationStatement* declaration = NULL;

                    a_tagged_pointer & entry = sse->entity;
                    an_il_entry_kind entry_kind = (an_il_entry_kind)entry.kind;
                    switch (entry_kind)
                       {
                      // Process the case of a type: which is all that I have see hidden in the conditional, not clear if there could be anything else.
                      // But it might be that it could be a template declaration, but I hope not and I have never seen that case.
                         case iek_type:
                            {
#if 0
                              printf ("Found type: entry_kind == iek_type: \n");
#endif
                              a_type_ptr edg_type = (a_type_ptr)(sse->entity.ptr);
                              ROSE_ASSERT(edg_type != NULL);
#if 0
                              printf ("################## In convert_condition(): kind = %s \n",summarize_type_kind_in_string(edg_type));
                              printf ("Calling convert_type() \n");
#endif
                           // DQ (7/1/2019): build the ROSE type, but we might want to do this at the top of this function.
                              bool forceTemplateClassDeclaration = false;
                              a_template_ptr edg_template        = NULL;
                              declaration = parse_structlike(sse, edg_type, SgClassDeclaration::e_union, forceTemplateClassDeclaration, edg_template);
#if 0
                              printf ("In convert_condition(): in while ((a_statement_ptr)(sse->entity.ptr) != target_statement_after_conditional): declaration = %p \n",declaration);
#endif
                           // DQ (7/1/2019): It might be that this can be NULL.
                           // ROSE_ASSERT(declaration != NULL);
#if 0
                              printf ("DONE: Calling convert_type() \n");
                              printf ("In convert_condition(): DONE: Calling convert_type(): entry = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
#if 0
                              printf ("Exiting as a test! \n");
                              ROSE_ASSERT(false);
#endif
                              break;
                            }

                         default:
                            {
                              printf ("default reached: This was not a type: entry = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
                              break;
                            }
                       }

#if 1
                 // DQ (7/1/2019): If we found a valid type then don't increment the sse, I think.
                    if (declaration == NULL)
                       {
                         ++sse;
                       }
#else
                    ++sse;
#endif

#if DEBUG_CONVERT_CONDITION
                    printf ("In convert_condition(): AFTER: (increment until we reach the target_statement_after_conditional): entry = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
                  }
#if DEBUG_CONVERT_CONDITION
               printf ("In convert_condition(): AFTER LAST INCREMENT: entry = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
             }
#if 0
          printf ("Exiting as a test! \n");
          ROSE_ASSERT(false);
#endif
        }
#endif

#if 0
     printf ("Leaving convert_condition(): iek_src_seq_end_of_construct entry_kind = %p = %s \n",
          sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif

#if DEBUG_CONVERT_CONDITION || 0
     printf ("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ \n");
     printf ("Leaving convert_condition() kind = %s statement = %p = %s \n",output_expr_enum(kind).c_str(),statement,statement->class_name().c_str());
     printf ("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ \n");
#endif

#if 0
     printf ("Leaving convert_condition(): statement = %p = %s file_id = %d physical_file_id = %d \n",
          statement,statement->class_name().c_str(),statement->get_file_info()->get_file_id(),
          statement->get_file_info()->get_physical_file_id());
#endif

#if 0
     printf ("Exiting as a test! \n");
     ROSE_ASSERT(false);
#endif

  // return set_basic_file_info_for_expr(buildExprStatement_nfi(convert_expression(e).value()), e);
     return statement;
   }
