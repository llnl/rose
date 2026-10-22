
#include "edgRose.h"

namespace EDG_ROSE_Translation {

void initDiagnostics() 
   {
     static bool initialized = false;
     if (!initialized) 
        {
          initialized = true;
          mlog = Sawyer::Message::Facility("EDG_ROSE_Translation", Rose::Diagnostics::destination);
          Rose::Diagnostics::mfacilities.insertAndAdjust(mlog);
        }
   }

#ifdef DEBUG_CONVERSION
void indent_line()
   {
     debug_print("%2d %*s", indent, 2 * indent, "");
   }
#endif

SgType* apply_qualifiers(SgType* result, a_bit_field qualifiers, a_upc_block_size block_size)
   {
  // DQ (11/4/2011): Added assertion.
     ROSE_ASSERT(result != NULL);

  // DQ (11/6/2011): We don't use all modifiers with SgFunctionType objects (const/volitile modifiers are held in the function declarations).
  // However, a pointer to a function is wrapped as a type.
  // ROSE_ASSERT(isSgFunctionType(result) == NULL);

#if 0
     printf ("In apply_qualifiers() TOP: qualifiers = %" PRIuPTR " result = %p = %s \n",(size_t)qualifiers,result,result->class_name().c_str());
#endif

  // if (qualifiers & (TQ_CONST | TQ_VOLATILE))
     if ( (qualifiers & TQ_CONST) && (qualifiers & TQ_VOLATILE) )
        {
#if 0
       // DQ (1/19/2019): Debugging the use of "const volatile" which is output as just "volatile". 
          printf ("In apply_qualifiers(): Rebuild the type to set the const volatile flag \n");
#endif
          result = buildVolatileType(result);
        }
       else
        {
          if (qualifiers & TQ_CONST)
             {
#if 0
            // DQ (1/19/2019): Debugging the use of "const volatile" which is output as just "volatile". 
               printf ("In apply_qualifiers(): Rebuild the type to set the const flag \n");
#endif
               result = buildConstType(result);
             }

          if (qualifiers & TQ_VOLATILE)
             {
#if 0
            // DQ (1/19/2019): Debugging the use of "const volatile" which is output as just "volatile". 
               printf ("In apply_qualifiers(): Rebuild the type to set the volatile flag \n");
#endif
               result = buildVolatileType(result);
             }
        }

     if (qualifiers & TQ_RESTRICT)
        {
#if 0
          printf ("In apply_qualifiers(): Rebuild the type to set the restrict flag \n");
#endif
          result = buildRestrictType(result);
        }

#if UPC_EXTENSIONS_ALLOWED
     if (qualifiers & TQ_UPC_SHARED)
        {
          if (block_size == UPC_BLOCK_SIZE_NONE || block_size == 1)
          { // default block size is 1, Liao 11/29/2012
               result = buildUpcSharedType(result,1);
          } else if (block_size == UPC_BLOCK_SIZE_INDEFINITE /* which is 0 */) {
               result = buildUpcBlockIndefiniteType(result);
          } else if (block_size == UPC_BLOCK_SIZE_BLOCK) {
               result = buildUpcBlockStarType(result);
          } else {
               result = buildUpcBlockNumberType(result, block_size);
          }
        }
     if (qualifiers & TQ_UPC_STRICT) result = buildUpcStrictType(result);
     if (qualifiers & TQ_UPC_RELAXED) result = buildUpcRelaxedType(result);
#endif
  // DQ (11/4/2011): Added assertion.
     ROSE_ASSERT(result != NULL);

#if 0
     printf ("In apply_qualifiers() BOTTOM: result = %p = %s \n",result,result->class_name().c_str());
#endif

     return result;
   }

std::vector<SgStatement*> processEntriesRelatedToConditional( SourceSequenceList& sse, a_statement_ptr target_statement_after_conditional )
   {
  // DQ (7/1/2019): Refactor to a seperate function, so that it can be called from the top of the function.

  // #define DEBUG_CONDITIONAL_ENTRIES 0

  // DQ (7/3/2019): This function shoud return a list of statements that is identifies, if any.

     SgStatement* returnStatement = NULL;

  // DQ (7/4/2019): If we process more then one statement, then we need to include the results into a list, so that if we 
  // see the stmk_stmt_expr_result we can then assemple the list into a block for the GNU statement expression. This is handled
  // this way because the stmk_stmt_expr_result is always the last statement in the list (so we need to be prepared to see it).
     vector<SgStatement*> returnStatementList;

#define DEBUG_PROCESS_ENTRIES_FOR_CONDITIONAL 0

#if DEBUG_PROCESS_ENTRIES_FOR_CONDITIONAL || 0
     printf ("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ \n");
     printf ("In processEntriesRelatedToConditional() target_statement_after_conditional = %p \n",target_statement_after_conditional);
     printf ("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ \n");
#endif

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
#if DEBUG_CONVERT_CONDITION || 0
          printf ("Now we know we have a valid target_statement_after_conditional (%p) for the conditional statement \n",target_statement_after_conditional);
          printf ("In processEntriesRelatedToConditional(): (target_statement_after_conditional != NULL): entry = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
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
                              printf ("$$$$$$$$$$$$$$$$$$ In processEntriesRelatedToConditional(): iterate over the statements appearing after the conditional : s = %p kind = %s \n",s,summarize_statement_in_string(s));
#endif
                              s = s->next;
                            }
                         target_statement_after_conditional = s;
                         ROSE_ASSERT(target_statement_after_conditional != NULL);
                         break;
                       }

                    default:
                       {
                          printf ("In processEntriesRelatedToConditional(): (target_statement_after_conditional->source_sequence_entry == NULL): default reached in selecting better canidate target_statement_after_conditional \n");
                          printf ("   --- target_statement_after_conditional->kind = %s \n",summarize_statement_in_string(target_statement_after_conditional));
                          ROSE_ASSERT(false);
                       }
                  }
             }
          ROSE_ASSERT(target_statement_after_conditional->source_sequence_entry != NULL);

       // For more complex cases we need to just increment the sse until we reach the next statement (which we know is the loop_statement).
#if DEBUG_CONVERT_CONDITION
          printf ("In processEntriesRelatedToConditional(): We might want to increment the sse until we reach the target_statement_after_conditional = %p \n",target_statement_after_conditional);
#endif
       // DQ (9/6/2013): If we are not already at the expected next statement, then iterate past the end of the current construct.
          if ((a_statement_ptr)(sse->entity.ptr) != target_statement_after_conditional)
             {
#if DEBUG_CONVERT_CONDITION || 0
               int counter = 0;
#endif
               while ((a_statement_ptr)(sse->entity.ptr) != target_statement_after_conditional)
                  {
#if DEBUG_CONVERT_CONDITION || 0
                    printf ("In processEntriesRelatedToConditional(): while loop: counter = %d \n",counter++);
                    printf ("In processEntriesRelatedToConditional(): Increment sse until we reach target_statement_after_conditional = %p \n",target_statement_after_conditional);
                    printf ("In processEntriesRelatedToConditional(): BEFORE: (increment until we reach the target_statement_after_conditional): entry = %p kind = %s \n",
                       sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());

                 // DQ (7/26/2020): Enforce a bound on the counter (debugging).
                    ROSE_ASSERT(counter < 10);
#endif

                 // DQ (7/1/2019): Added support to process the clas declaration that can be hidden in a conditional (of a if stmt for example).
                 // SgDeclarationStatement* declaration = NULL;
                    SgStatement* singleStatement = NULL;

                    a_tagged_pointer & entry = sse->entity;
                    an_il_entry_kind entry_kind = (an_il_entry_kind)entry.kind;
                    switch (entry_kind)
                       {
                      // Process the case of a type: which is all that I have see hidden in the conditional, not clear if there could be anything else.
                      // But it might be that it could be a template declaration, but I hope not and I have never seen that case.
                         case iek_type:
                            {
#if DEBUG_CONVERT_CONDITION
                              printf ("Found type: entry_kind == iek_type: \n");
#endif
                              a_type_ptr edg_type = (a_type_ptr)(sse->entity.ptr);
                              ROSE_ASSERT(edg_type != NULL);
#if DEBUG_CONVERT_CONDITION
                              printf ("################## In processEntriesRelatedToConditional(): kind = %s \n",summarize_type_kind_in_string(edg_type));
                              printf ("Calling parse_structlike() \n");
#endif
                           // DQ (7/1/2019): build the ROSE type, but we might want to do this at the top of this function.
                              bool forceTemplateClassDeclaration = false;
                              a_template_ptr edg_template        = NULL;
                           // declaration = parse_structlike(sse, edg_type, SgClassDeclaration::e_union, forceTemplateClassDeclaration, edg_template);
                              singleStatement = parse_structlike(sse, edg_type, SgClassDeclaration::e_union, forceTemplateClassDeclaration, edg_template);
#if 0
                              if (returnStatement != NULL)
                                 {
                                   printf ("returnStatement = %p = %s \n",returnStatement,returnStatement->class_name().c_str());
                                 }
#endif
                           // DQ (8/22/2020): This fails for test2020_13.c and test2020_19.c.
                           // ROSE_ASSERT(returnStatement == NULL);

                              if (singleStatement != NULL)
                                 {
                                // returnStatement = declaration;
                                   returnStatement = singleStatement;

                                   returnStatementList.push_back(returnStatement);

                                // DQ (8/22/2020): This fails for test2020_13.c and test2020_19.c.
                                // ROSE_ASSERT(returnStatementList.size() == 1);
                                // ROSE_ASSERT(returnStatementList[0] != NULL);
                                 }
                                else
                                 {
                                   printf ("Note: In processEntriesRelatedToConditional(): in while loop: case iek_type: parse_structlike() returned NULL pointer \n");
                                 }

#if DEBUG_CONVERT_CONDITION
                              printf ("In processEntriesRelatedToConditional(): in while ((a_statement_ptr)(sse->entity.ptr) != target_statement_after_conditional): case iek_type: declaration = %p \n",declaration);
#endif
                           // DQ (7/1/2019): It might be that this can be NULL.
                           // ROSE_ASSERT(declaration != NULL);
#if DEBUG_CONVERT_CONDITION
                              printf ("DONE: Calling parse_structlike() \n");
                              printf ("In processEntriesRelatedToConditional(): DONE: Calling parse_structlike(): entry = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
                           // DQ (7/5/2019): force the call to ++sse to not be made.
                           // singleStatement = NULL;
#if 0
                              printf ("Exiting as a test! \n");
                              ROSE_ASSERT(false);
#endif
                              break;
                            }

                      // stmk_stmt_expr_result
                         case iek_statement:
                            {
#if DEBUG_CONVERT_CONDITION
                              printf ("Found block: entry_kind == iek_statement: \n");
#endif
                              a_statement_ptr edg_statement = (a_statement_ptr)(sse->entity.ptr);
                              ROSE_ASSERT(edg_statement != NULL);
#if DEBUG_CONVERT_CONDITION
                              printf ("################## In processEntriesRelatedToConditional(): kind = %s \n",summarize_statement_in_string(edg_statement));
                              printf ("Calling parse_statement() \n");
#endif
#if GNU_EXTENSIONS_ALLOWED
                              if (edg_statement->kind == stmk_stmt_expr_result)
                                 {
                                // vector<SgStatement*> statement_list = parse_statement(SourceSequenceList& sse, SgBasicBlock* existingBasicBlock);
                                   SgBasicBlock* existingBasicBlock = NULL;
                                   vector<SgStatement*> statement_list = parse_statement( sse, existingBasicBlock);
#if DEBUG_CONVERT_CONDITION
                                   printf ("DONE: Calling parse_statement(): statement_list.size() = %zu \n",statement_list.size());
                                   printf ("In processEntriesRelatedToConditional(): stmk_stmt_expr_result: DONE: Calling parse_statement(): entry = %p kind = %s \n",
                                        sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
#if DEBUG_CONVERT_CONDITION
                                   printf ("Output statement_list: size = %zu \n",statement_list.size());
                                   for (size_t i = 0; i < statement_list.size(); i++)
                                      {
                                        printf ("statement_list[%zu] = %p = %s \n",i,statement_list[i],statement_list[i]->class_name().c_str());
                                      }
#endif
#if 1
                                   ROSE_ASSERT(statement_list.size() == 1);
                                   SgStatement* statement = statement_list[0];

#if DEBUG_CONVERT_CONDITION
                                   printf ("In processEntriesRelatedToConditional(): stmk_stmt_expr_result: statement = %p = %s \n",statement,statement->class_name().c_str());
                                   if (returnStatement != NULL)
                                      {
                                        printf ("returnStatement = %p = %s \n",returnStatement,returnStatement->class_name().c_str());
                                      }
#endif
                                   ROSE_ASSERT(returnStatement == NULL);

                                   returnStatement = statement;
                                   singleStatement = statement;

                                   returnStatementList.push_back(statement);

                                   ROSE_ASSERT(returnStatementList.size() == 1);
                                   ROSE_ASSERT(returnStatementList[0] != NULL);
#endif
                                 }
                                else
#endif /* GNU_EXTENSIONS_ALLOWED */
                                 {
                                // DQ (7/4/2019): We need ot handle blocks seperately since they need to be make into GNU statement expressions when they appear in a conditional.
                                   if (edg_statement->kind == stmk_block)
                                      {
                                        a_statement_ptr edg_statement = (a_statement_ptr)(sse->entity.ptr);
                                        ROSE_ASSERT(edg_statement != NULL);
#if DEBUG_CONVERT_CONDITION
                                        printf ("Calling summarize_statement(): Processing block into a GNU statement expression \n");
                                        summarize_statement(edg_statement);
#endif
                                        SgBasicBlock* existingBasicBlock = NULL;
                                        vector<SgStatement*> statement_list = parse_statement( sse, existingBasicBlock);

#if DEBUG_CONVERT_CONDITION
                                        printf ("Processing block into a GNU statement expression \n");
                                        printf ("DONE: Calling parse_statement(): statement_list.size() = %zu \n",statement_list.size());
                                        printf ("In processEntriesRelatedToConditional(): stmk_block: DONE: Calling parse_statement(): entry = %p kind = %s \n",
                                             sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
#if DEBUG_CONVERT_CONDITION
                                        printf ("Output statement_list: size = %zu \n",statement_list.size());
                                        for (size_t i = 0; i < statement_list.size(); i++)
                                           {
                                             printf ("statement_list[%zu] = %p = %s \n",i,statement_list[i],statement_list[i]->class_name().c_str());
                                           }
#endif
#if 1
                                     // DQ (7/7/2019): This fails for test2015_14.c, but it is more important that the declarations associated 
                                     // with the for loop condition be processed, than that we do anything specific with them (they will be types 
                                     // and variable declarations). and the expresion we evaluate will reference them.

                                     // ROSE_ASSERT(statement_list.size() == 1);
                                     // SgStatement* statement = statement_list[0];
                                        SgStatement* statement = NULL;
                                        if (statement_list.size() == 1)
                                           {
                                             statement =statement_list[0];
                                           }
#if 0
                                        if (edg_statement->variant.block.extra_info != NULL && edg_statement->variant.block.extra_info->is_statement_expression == true)
                                           {
                                          // Then ignore this since it will be visited again later.
                                             printf ("^^^^^^^^^^^^^^^^^^^^ In processEntriesRelatedToConditional(): stmk_block: SKIP Ignoring the statement expression generated within the processEntriesRelatedToConditional() function \n");
#if 0
                                             statement = NULL;
#endif
#if 0
                                             printf ("Exiting as a test! \n");
                                             ROSE_ASSERT(false);
#endif
                                           }
#endif
                                     // DQ (7/6/2019): Ignoring GNU statement expressions to avoid redundant processing.
                                        if (statement != NULL)
                                           {
#if DEBUG_CONVERT_CONDITION
                                             printf ("In processEntriesRelatedToConditional(): stmk_block: statement = %p = %s \n",statement,statement->class_name().c_str());
                                             if (returnStatement != NULL)
                                                {
                                                  printf ("returnStatement = %p = %s \n",returnStatement,returnStatement->class_name().c_str());
                                                }
#endif
#if DEBUG_CONVERT_CONDITION
                                          // DQ (7/5/2019): test2019_05.c demonstrates the case of multiple GNU statement-expressions used in a a predicate.
                                             printf ("In processEntriesRelatedToConditional(): case iek_statement: stmk_block: statement = %p = %s \n",statement,statement->class_name().c_str());
                                          // DQ (7/5/2019): This fails for test2015_52.c and test2019_92.c regression tests.
                                             if (returnStatement != NULL)
                                                {
                                                  printf ("Warning: returnStatement is a reference to a 2nd block! \n");
                                                }
#endif
                                          // ROSE_ASSERT(returnStatement == NULL);

                                          // returnStatement = statement;
                                          // singleStatement = statement;
#endif


#if DEBUG_CONVERT_CONDITION
                                             printf ("In processEntriesRelatedToConditional(): case iek_statement: stmk_block: building GNU Statement Expression \n");
#endif
                                          // This is the GNU statement-expression.
                                             SgStatementExpression* statementExpression = new SgStatementExpression(statement);

                                          // DQ (7/6/2019): Added a map of SgStatementExpression statements so that we can reuse them.  This appears 
                                          // to be the essential problem currently.  They need to be constructed in the traversal over the source 
                                          // sequence points of the conditional statement and then used in the construction of the conditional expression.
                                             ROSE_ASSERT(edg_statement != NULL);
                                             ROSE_ASSERT(statementExpression != NULL);
                                             if (statement_expression_cache.find(edg_statement) == statement_expression_cache.end())
                                                {
#if 0
                                                  printf ("In processEntriesRelatedToConditional(): stmk_block: edg_statement = %p (ROSE) statementExpression = %p \n",edg_statement,statementExpression);
#endif
                                                  statement_expression_cache[edg_statement] = statementExpression;
                                                }
                                               else
                                                {
                                                  printf ("ERROR: There should not already be an entry for edg_statement = %p and statementExpression = %p = %s in the statement_expression_cache \n",
                                                       edg_statement,statementExpression,statementExpression->class_name().c_str());
                                                  ROSE_ASSERT(false);
                                                }

                                          // DQ (11/3/2012): Added call to set file info to default settings (normally called from SageBuilder).
                                             setSourcePosition(statementExpression);

                                             statementExpression->get_statement()->set_parent(statementExpression);

                                          // DQ (7/4/2019): Unclear if we need this statement.
                                          // statement_expression_bodies.insert(statement);

                                          // SgExprStatement* buildExprStatement_nfi(SgExpression*  exp);
                                             SgStatement* statementHoldingStatementExpression = buildExprStatement_nfi(statementExpression);
                                             statementExpression->set_parent(statementHoldingStatementExpression);

                                          // DQ (7/5/2019): For now set the parent to this, but it will have to be fixed up later.
                                          // Where there are multiple blocks the parent should be the parent expression to which this is a child.
                                             statementHoldingStatementExpression->set_parent(topScopeStack());
#if DEBUG_CONVERT_CONDITION
                                             printf ("++++++++++++++++ statementHoldingStatementExpression = %p \n",statementHoldingStatementExpression);
#endif
                                          // Collect the GNU statement-expressions when there are more than one (see test2019_05.c).
                                          // returnStatementList.push_back(statementExpression);
                                             returnStatementList.push_back(statementHoldingStatementExpression);

                                             ROSE_ASSERT(returnStatementList.size() >= 1);
                                             ROSE_ASSERT(returnStatementList[0] != NULL);

                                             setSourcePosition(statementHoldingStatementExpression);

                                          // returnStatement = statementExpression;
                                          // singleStatement = statementExpression;
                                             returnStatement = statementHoldingStatementExpression;
                                             singleStatement = statementHoldingStatementExpression;
                                           }
#if 0
                                        printf ("Exiting as a test! \n");
                                        ROSE_ASSERT(false);
#endif
                                      }
                                     else
                                      {
#if DEBUG_CONVERT_CONDITION
                                        printf ("In processEntriesRelatedToConditional(): case iek_statement: STATEMENT NOT A GNU STATEMENT-EXPRESSION! \n");
#endif
                                        SgBasicBlock* existingBasicBlock = NULL;
                                        vector<SgStatement*> statement_list = parse_statement( sse, existingBasicBlock);

                                     // A declaration statement (stmk_decl) yields no SgStatement: its declared entities are
                                     // separate source sequence entries handled by the following iterations.  parse_statement()
                                     // has advanced sse, so skip the "++sse" at the end of the loop body.
                                        if (statement_list.empty() == true && edg_statement->kind == stmk_decl)
                                           {
                                             continue;
                                           }

#if 0
                                        printf ("In processEntriesRelatedToConditional(): case iek_statement: Output statement_list: size = %zu \n",statement_list.size());
                                        for (size_t i = 0; i < statement_list.size(); i++)
                                           {
                                             printf ("statement_list[%zu] = %p = %s \n",i,statement_list[i],statement_list[i]->class_name().c_str());
                                           }
#endif
#if 1
                                        if (statement_list.size() != 1)
                                           {
                                             printf ("In processEntriesRelatedToConditional(): case iek_statement: Output statement_list: size = %zu \n",statement_list.size());
                                             for (size_t i = 0; i < statement_list.size(); i++)
                                                {
                                                  printf ("statement_list[%zu] = %p = %s \n",i,statement_list[i],statement_list[i]->class_name().c_str());
                                                }
                                           }
                                          else
                                           {
                                             printf ("In processEntriesRelatedToConditional(): case iek_statement: Output statement_list: size = %zu \n",statement_list.size());
                                           }
#endif
                                        ROSE_ASSERT(statement_list.size() == 1);

                                        SgStatement* statement = statement_list[0];
                                        ROSE_ASSERT(statement != NULL);

#if DEBUG_CONVERT_CONDITION
                                        printf ("In processEntriesRelatedToConditional(): case iek_statement: statement = %p = %s \n",statement,statement->class_name().c_str());
#endif
                                        returnStatementList.push_back(statement);

#if DEBUG_CONVERT_CONDITION
                                        if (returnStatement != NULL)
                                           {
                                             printf ("returnStatement = %p = %s \n",returnStatement,returnStatement->class_name().c_str());
                                           }
#endif
                                        ROSE_ASSERT(returnStatement == NULL);
                                        returnStatement = statement;
                                        singleStatement = statement;

#if DEBUG_CONVERT_CONDITION
                                        printf ("returnStatementList.size() = %zu \n",returnStatementList.size());
#endif
                                        ROSE_ASSERT(returnStatementList.size() == 1);
                                        ROSE_ASSERT(returnStatementList[0] != NULL);
                                      }
                                 }

                              break;
                            }

                      // DQ (7/5/2019): This case should not be handled by the default.
                         case iek_src_seq_end_of_construct: 
                            {
                           // Keep sse pointing to end of construct to get position
#if 1
                           // DQ (8/17/2012): I think that the source position of the closing scope is contained in here (so we don't want to ignore these).
                              printf ("@@@@@@@@@@@@@@@@@ In processEntriesRelatedToConditional(): case iek_src_seq_end_of_construct: sse->entity.ptr = %p \n",sse->entity.ptr);
#endif
                              a_src_seq_end_of_construct_ptr edg_end_of_construct = (a_src_seq_end_of_construct_ptr)(sse->entity.ptr);
                              ROSE_ASSERT(edg_end_of_construct != NULL);

                              break;
                            }

                         case iek_src_seq_secondary_decl:
                            {
#if 0
                              an_il_entry_kind inner_kind = (an_il_entry_kind)(((a_src_seq_secondary_decl_ptr)(sse->entity.ptr))->entity.kind);
                              printf ("@@@@@@@@@@@@@@@@@ In processEntriesRelatedToConditional(): case iek_src_seq_secondary_decl: il_entry_kind_names[inner_kind] = %s (not processed) \n",il_entry_kind_names[inner_kind]);
#endif
                           // printf ("Increment sse \n");
                              ++sse;

                              break;
                            }

                      // This case is found in C++11_tests/test2019_484.C
                         case iek_variable:
                            {
#if DEBUG_CONVERT_CONDITION
                              printf ("Found variable: entry_kind == iek_variable: \n");
#endif
#if 1
                           // DQ (7/6/2019): Maybe we should not process these to avoid duplicate entries (I think we need this 
                           // else variable declarations in other declaration (e.g. structs and unions) will not be processed).
                              SgVariableDeclaration* variableDeclaration = parse_variable_declaration(sse);
                              ROSE_ASSERT(variableDeclaration != NULL);

                           // DQ (7/3/2019): Trigger this variable declaration to be reused in the conditional.
                              variableDeclaration->set_builtFromUseOnly(true);

                           // declaration = variableDeclaration;
                              singleStatement = variableDeclaration;

                              if (returnStatement != NULL)
                                 {
                                   printf ("returnStatement = %p = %s \n",returnStatement,returnStatement->class_name().c_str());
                                 }
                              ROSE_ASSERT(returnStatement == NULL);

                              returnStatement = variableDeclaration;

                              returnStatementList.push_back(variableDeclaration);

                              ROSE_ASSERT(returnStatementList.size() == 1);
                              ROSE_ASSERT(returnStatementList[0] != NULL);
#else
                           // DQ (7/6/2019): But increment the sse.
                              ++sse;
#endif
#if DEBUG_CONVERT_CONDITION
                              printf ("After call to parse_variable_declaration(): variableDeclaration = %p \n",variableDeclaration);
                              printf ("In processEntriesRelatedToConditional(): DONE: Calling parse_variable_declaration(): entry = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
#if 0
                              printf ("Exiting as a test! \n");
                              ROSE_ASSERT(false);
#endif
                              break;
                            }

                         case iek_macro:
                            {
#if 0
                              printf ("Found macro: entry_kind == iek_macro: \n");
#endif
                              while(sse->entity.kind == iek_macro)
                                 {
                                   ++sse;
                                 }

                              break;
                            }

#if 1
                      // DQ (9/22/2020): Added pragma support.
                         case iek_pragma:
                            {
                              a_pragma_ptr p = (a_pragma_ptr)(sse->entity.ptr);
#if DEBUG_CONVERT_CONDITION
                              a_pragma_kind kind = p->kind;
                              printf ("In processEntriesRelatedToConditional(): kind = %s \n",summarize_pragma_kind_in_string((a_pragma_kind)kind));
#endif
                              ++sse;

#if DEBUG_CONVERT_CONDITION
                              const char* pragma_name = pragma_ids[kind];
                              printf ("In processEntriesRelatedToConditional(): pragma_name = %s \n",pragma_name);
#endif
                              string pragma_kind_name = generate_pragma_name(p);
#if DEBUG_CONVERT_CONDITION
                              printf ("In parse_pragma(): pragma_kind_name = %s\n",pragma_kind_name.c_str());
#endif
                           // indent_line(); debug_print("pragma %s \"%s\" ", pragma_name, p->pragma_text); print_source_pos(p->position); debug_print(" associated source_sequence_entry = %p \n",p->source_sequence_entry);
                           // ScopedIndent si(indent);
                           // result = buildPragmaDeclaration(p->pragma_text, topScopeStack()); // FIXME file info
#if DEBUG_CONVERT_CONDITION
                              printf ("In processEntriesRelatedToConditional(): calling summarize_pragma(p = %p) \n",p);
                              summarize_pragma(p);
#endif

                              SgName name = pragma_kind_name;
                              SgPragma* pragma = buildPragma(name);

                              SgScopeStatement* scope = topScopeStack();
                              ROSE_ASSERT(scope != NULL);
#if DEBUG_CONVERT_CONDITION
                              printf ("Setting SgPragma parent to topScopeStack() = %p = %s \n",scope,scope->class_name().c_str());
#endif
                              pragma->set_parent(scope);

                           // Add the pragma to the scope statement.
                              scope->set_pragma(pragma);

                              pragma = set_file_info_for_where_corresp_is_unavailable(pragma, p->position);

#if 0
                              printf ("Exiting as a test! \n");
                              ROSE_ASSERT(false);
#endif
                              break;
                            }
#endif
                         default:
                            {
                              printf ("default reached: This was not a type or variable: entry = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#if 1
                           // DQ (9/23/2020): Increment the source sequence entry (which means that we might be missing something).
                              ++sse;
#endif
#if 1
                           // DQ (9/24/2020): This was previously an infinite loop, so make it an error now.
                              printf ("Exiting as a test! \n");
                              ROSE_ASSERT(false);
#endif
                              break;
                            }
                       }

#if 0
                 // DQ (7/5/2019): We don't want to be incrementing the source sequence pointer directly.
#if 1
                 // DQ (7/1/2019): If we found a valid type then don't increment the sse, I think.
                 // if (declaration == NULL)
                    if (singleStatement == NULL)
                       {
                         ++sse;
                       }
#else
                    ++sse;
#endif
#endif

#if DEBUG_CONVERT_CONDITION && 1
                    printf ("In processEntriesRelatedToConditional(): AFTER: (increment until we reach the target_statement_after_conditional): entry = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
                  }
#if DEBUG_CONVERT_CONDITION && 1
               printf ("In processEntriesRelatedToConditional(): AFTER LAST INCREMENT: entry = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
             }

#if 0
          printf ("Exiting as a test! \n");
          ROSE_ASSERT(false);
#endif
        }
       else
        {
       // DQ (7/7/2019): I think it is an error to not pass in a valid target_statement_after_conditional.
#if 0
          printf ("Note: In processEntriesRelatedToConditional(): target_statement_after_conditional == NULL: this might be an error! \n");
#endif
#if 0
          printf ("Exiting as a test! \n");
          ROSE_ASSERT(false);
#endif
        }

#if DEBUG_CONVERT_CONDITION
     printf ("&&&&&&&&&&&&&&&&&&&& Leaving processEntriesRelatedToConditional(): returnStatementList.size() = %zu \n",returnStatementList.size());
#endif

#if DEBUG_CONVERT_CONDITION
     printf ("&&&&&&&&&&&&&&&&&&&& Leaving processEntriesRelatedToConditional(): returnStatement = %p \n",returnStatement);
     if (returnStatement != NULL)
        {
          printf (" --- returnStatement = %p = %s \n",returnStatement,returnStatement->class_name().c_str());

          SgBasicBlock* returnBlock = isSgBasicBlock(returnStatement);
          if (returnBlock != NULL)
             {
               for (size_t i = 0; i < returnBlock->get_statements().size(); i++)
                  {
                    printf ("returnBlock->get_statements()[%zu] = %p = %s \n",i,returnBlock->get_statements()[i],returnBlock->get_statements()[i]->class_name().c_str());
                  }

            // If the last statement is a statment expression, then we need to wrap the whold block inside of a statement expression.
               size_t lastStatementIndex = returnBlock->get_statements().size();
               printf ("lastStatementIndex = %zu \n",lastStatementIndex);
               ROSE_ASSERT(lastStatementIndex > 0);
               SgStatement* lastStatement = returnBlock->get_statements()[lastStatementIndex-1];
               printf ("lastStatement = %p = %s \n",lastStatement,lastStatement->class_name().c_str());
#if 0
               printf ("Exiting as a test! \n");
               ROSE_ASSERT(false);
#endif
             }
        }
#endif


#if DEBUG_CONVERT_CONDITION
  // DQ (7/5/2019): Debugging information.
     if (returnStatementList.empty() == false)
        {
          printf ("Leaving processEntriesRelatedToConditional(): returnStatementList.size() = %zu \n",returnStatementList.size());
          for (size_t i = 0; i < returnStatementList.size(); i++)
             {
               ROSE_ASSERT(returnStatementList[i] != NULL);
               printf (" --- returnStatementList[%zu] = %p = %s \n",i,returnStatementList[i],returnStatementList[i]->class_name().c_str());
             }
        }
       else
        {
          printf ("returnStatementList is empty \n");
        }
#endif

#if DEBUG_PROCESS_ENTRIES_FOR_CONDITIONAL || 0
     printf ("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ \n");
     printf ("Leaving processEntriesRelatedToConditional() returnStatementList.size() = %zu \n",returnStatementList.size());
     printf ("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ \n");
#endif

#if 0
     printf ("Exiting as a test! \n");
     ROSE_ASSERT(false);
#endif

  // return returnStatement;
     return returnStatementList;
   }

SgCastExp::cast_type_enum typeOfCast(a_constant_ptr constant)
   {
  // DQ (8/22/2016): Adding support for C++ types of casts.
     SgCastExp::cast_type_enum cast_type = SgCastExp::e_C_style_cast;

     ROSE_ASSERT(constant != NULL);

     bool reset_return_value = false;

#if 0
     printf ("In typeOfCast(a_constant_ptr): constant = %p \n",constant);
#endif

#if 1
  // This only applies to EDG a_constant_ptr types.
  // if (e->implicit_cast == true)
     if (constant->implicit_cast == true)
        {
#if 0
          printf ("In typeOfCast(a_constant_ptr): Found a implicit cast, marking cast as implicit cast (defaults to C style cast in AST) ");
          print_source_corresp(constant->source_corresp); debug_print("\n");
#endif
       // DQ (8/22/2016): This is not supported in ROSE and could be if we find it to be required.
       // cast_type = SgCastExp::e_implicit_cast;
        }
#endif

#if 1
  // This only applies to EDG a_constant_ptr types.
  // if (e->explicit_cast_applied == true)
     if (constant->explicit_cast_applied == true)
        {
#if 0
          printf ("In typeOfCast(a_constant_ptr): Found a explicit_cast_applied cast, marking cast as explicit_cast_applied cast (defaults to C style cast in AST) ");
          print_source_corresp(constant->source_corresp); debug_print("\n");
#endif
       // DQ (8/22/2016): This is not supported in ROSE and could be if we find it to be required.
       // cast_type = SgCastExp::e_explicit_cast_applied_cast;
        }
#endif

#if 1
  // This ALSO applies to EDG a_constant_ptr types.
     if (constant->is_reinterpret_cast == true)
        {
#if 0
          printf ("In typeOfCast(a_constant_ptr): Found a reinterpret cast, marking cast as reinterpret cast \n");
#endif
          ROSE_ASSERT(reset_return_value == false);
          cast_type = SgCastExp::e_reinterpret_cast;
          reset_return_value = true;
        }
#endif

#if 0
     bool skip_output = false;
     if (reset_return_value == true && skip_output == false)
        {
          printf ("Leaving typeOfCast(a_constant_ptr): returning cast_type = %d = %s \n",cast_type,SgCastExp::cast_type_to_string(cast_type).c_str());
        }
#endif

     return cast_type;
   }


SgCastExp::cast_type_enum typeOfCast(an_expr_node_ptr e)
   {
  // DQ (8/22/2016): Adding support for C++ types of casts.
     SgCastExp::cast_type_enum cast_type = SgCastExp::e_C_style_cast;

     ROSE_ASSERT(e != NULL);

     bool reset_return_value = false;

#if 0
     printf ("In typeOfCast(an_expr_node_ptr): e = %p \n",e);
#endif

  // Supported tests:
  //    explicit, const, static, reinterpret, reference, rvalue_reference, 
  //    implicit_step_of_explicit, implicit, explicit_cast_applied, 
  //    keep_as_cast_for_cp_gen_be, functional_notation, brace_notation, 
  //    safe, tracking_reference

  // if (e->is_const_cast == true)
     if (e->kind == enk_operation && e->variant.operation.is_const_cast == true)
        {
#if 0
          printf ("In typeOfCast(): Found a const cast, marking cast as const cast \n");
#endif
          ROSE_ASSERT(reset_return_value == false);
          cast_type = SgCastExp::e_const_cast;
          reset_return_value = true;
        }

     if (e->is_static_cast == true)
        {
       // TRUE if this node represents a static_cast in the source.  Set only on edg specific enk_temp_init nodes and on
       // edg specific eok_cast and edg specific eok_XXX_cast enk_operation nodes; will be FALSE everywhere else.
#if 0
          printf ("In typeOfCast(an_expr_node_ptr): Found a static cast, marking cast as static cast \n");
#endif
          ROSE_ASSERT(reset_return_value == false);
          cast_type = SgCastExp::e_static_cast;
          reset_return_value = true;
        }

  // if (e->is_reinterpret_cast == true)
     if (e->kind == enk_operation && e->variant.operation.is_reinterpret_cast == true)
        {
#if 0
          printf ("In typeOfCast(an_expr_node_ptr): Found a reinterpret cast, marking cast as reinterpret cast \n");
#endif
          ROSE_ASSERT(reset_return_value == false);
          cast_type = SgCastExp::e_reinterpret_cast;
          reset_return_value = true;
        }

  // if (e->is_reference_cast == true)
     if (e->kind == enk_operation && e->variant.operation.is_reference_cast == true)
        {
#if 0
          printf ("In typeOfCast(an_expr_node_ptr): Found a reference cast, marking cast as reference cast (not implemented) \n");
#endif
       // DQ (8/22/2016): This is not supported in ROSE and could be if we find it to be required.
       // cast_type = SgCastExp::e_reference_cast;
        }

  // if (e->is_rvalue_reference_cast == true)
     if (e->kind == enk_operation && e->variant.operation.is_rvalue_reference_cast == true)
        {
#if 0
          printf ("In typeOfCast(an_expr_node_ptr): Found a rvalue_reference cast, marking cast as rvalue_reference cast (not implemented) \n");
#endif
       // DQ (8/22/2016): This is not supported in ROSE and could be if we find it to be required.
       // cast_type = SgCastExp::e_rvalue_reference_cast;
        }

  // if (e->implicit_step_of_explicit_cast == true)
     if (e->kind == enk_operation && e->variant.operation.implicit_step_of_explicit_cast == true)
        {
#if 0
          printf ("In typeOfCast(an_expr_node_ptr): Found a implicit_step_of_explicit cast, marking cast as implicit_step_of_explicit cast (not implemented) \n");
#endif
       // DQ (8/22/2016): This is not supported in ROSE and could be if we find it to be required.
       // cast_type = SgCastExp::e_implicit_step_of_explicit_cast;
        }

     if (e->keep_as_cast_for_cp_gen_be == true)
        {
#if 0
          printf ("In typeOfCast(an_expr_node_ptr): Found a keep_as_cast_for_cp_gen_be cast, marking cast as force_explicit cast (not implemented) \n");
#endif
       // DQ (8/22/2016): This is not supported in ROSE and could be if we find it to be required.
       // cast_type = SgCastExp::e_force_explicit_cast;
        }

     if (e->is_functional_notation_cast == true)
        {
       // This records the difference between "int x; float y = (float) x;" and "int x; float y = float(x);"
#if 0
          printf ("In typeOfCast(an_expr_node_ptr): Found a functional_notation cast, marking cast as functional notation cast (not implemented) ");
          print_source_range(e->expr_range); debug_print("\n");
#endif
       // DQ (8/22/2016): This is not supported in ROSE and could be if we find it to be required.
       // cast_type = SgCastExp::e_functional_notation_cast;
        }

     if (e->is_brace_notation_cast == true)
        {
#if 0
          printf ("In typeOfCast(an_expr_node_ptr): Found a brace_notation cast, marking cast as brace notation cast (not implemented) \n");
#endif
       // DQ (8/22/2016): This is not supported in ROSE and could be if we find it to be required.
       // cast_type = SgCastExp::e_brace_notation_cast;
        }

#if MICROSOFT_EXTENSIONS_ALLOWED
     if (e->is_safe_cast == true)
        {
#if 0
          printf ("In typeOfCast(an_expr_node_ptr): Found a MSVC specific safe cast, marking cast as safe cast \n");
#endif
       // DQ (8/22/2016): This is not supported in ROSE and could be if we find it to be required.
          ROSE_ASSERT(reset_return_value == false);
          cast_type = SgCastExp::e_safe_cast;
          reset_return_value = true;
        }

  // if (e->is_tracking_reference_cast == true)
     if (e->kind == enk_operation && e->variant.operation.is_tracking_reference_cast == true)
        {
#if 0
          printf ("In typeOfCast(an_expr_node_ptr): Found a MSVC specific tracking_reference cast, marking cast as tracking_reference cast (not implemented) \n");
#endif
       // DQ (8/22/2016): This is not supported in ROSE and could be if we find it to be required.
       // cast_type = SgCastExp::e_tracking_reference_cast;
        }
#endif

#if 0
     bool skip_output = (cast_type == SgCastExp::e_static_cast) || (cast_type == SgCastExp::e_reinterpret_cast) || (cast_type == SgCastExp::e_const_cast);
     if (reset_return_value == true && skip_output == false)
        {
          printf ("Leaving typeOfCast(an_expr_node_ptr): returning cast_type = %d = %s \n",cast_type,SgCastExp::cast_type_to_string(cast_type).c_str());
        }
#endif

     return cast_type;
   }

#ifdef DEBUG_CONVERSION
bool isInTypeTranslationCache(a_type_ptr p)  
   {
  // DQ (6/9/2013): Added this function to support debugging.

     bool returnValue = false;
     if (type_translation_cache.find(p) == type_translation_cache.end())
        {
#ifdef DEBUG_CONVERSION
           printf ("WARNING: isInTypeTranslationCache(): p = %p is not in the type_translation_cache \n",p);
#endif
        }
       else
        {
          returnValue = true;
        }

     return returnValue;
   }
#endif

#ifdef DEBUG_CONVERSION
bool isInTypeDeclarationMapAsDefiningDeclaration(a_type_ptr p)
   {
  // DQ (6/9/2013): Added this function to support debugging.

      ROSE_ASSERT(p != NULL);

     bool returnValue = false;
     if (type_declaration_map.find(p) == type_declaration_map.end())
        {
#ifdef DEBUG_CONVERSION
           printf ("WARNING: isInTypeDeclarationMapAsDefiningDeclaration(): p = %p is not in the type_declaration_map \n",p);
#endif
        }
       else
        {
       // If it is in the map then make sure that the vector has entries in it.
          ROSE_ASSERT(type_declaration_map[p].empty() == false);

          std::vector<SgDeclarationStatement*>::iterator i = type_declaration_map[p].begin();
          bool foundDefiningDeclaration = false;
          while (i != type_declaration_map[p].end())
             {
               ROSE_ASSERT(*i != NULL);
#if 0
               printf ("In isInTypeDeclarationMapAsDefiningDeclaration(): p = %p declaration = %p = %s \n",p,*i,(*i)->class_name().c_str());
#endif
            // Looking only at classes at the moment.
               SgClassDeclaration* classDeclaration = isSgClassDeclaration(*i);
               if (classDeclaration != NULL)
                  {
                 // There should only be a single defining declaration (defining declarations contain a valid pointer to a definition (SgClassDefinition).
                    if (foundDefiningDeclaration == false)
                       {
                         foundDefiningDeclaration = (classDeclaration->get_definition() != NULL);
                       }
                      else
                       {
                         ROSE_ASSERT(classDeclaration->get_definition() == NULL);
                       }
                  }

               i++;
             }

          returnValue = foundDefiningDeclaration;
        }

     return returnValue;
   }
#endif


// Replace "type_declaration_map[p].push_back(decl);" with call to function to 
// test for already pushed entries.
void push_entry_to_type_declaration_map(a_type_ptr p, SgDeclarationStatement* decl)
   {
     if (type_declaration_map.find(p) == type_declaration_map.end())
        {
          ROSE_ASSERT(isInTypeDeclarationMap(p) == false);
#if 0
          printf ("This declaration is NOT present in the type_declaration_map (adding a list) \n");
          printf (" --- type_declaration_map[p].size() = %zu \n",type_declaration_map[p].size());
#endif
          type_declaration_map[p].push_back(decl);
        }
       else
        {
          ROSE_ASSERT(isInTypeDeclarationMap(p) == true);
          std::vector<SgDeclarationStatement*> & declarationVector = type_declaration_map[p];
          std::vector<SgDeclarationStatement*>::iterator i = find(declarationVector.begin(),declarationVector.end(),decl);
          if (i != declarationVector.end())
             {
#if 0
               printf ("This declaration is ALREADY present in the type_declaration_map \n");
#endif
             }
            else
             {
#if 0
               printf ("This declaration is NOT present in the type_declaration_map (but the list for p = %p is present) \n",p);
               printf (" --- type_declaration_map[p].size() = %zu \n",type_declaration_map[p].size());
#endif
               type_declaration_map[p].push_back(decl);
             }
        }
      
      ROSE_ASSERT(type_declaration_map[p].empty() == false);
   }

void normalizeTemplateName( SgName & name )
   {
  // DQ (2/21/2005): Process the filename to avoid constructions such as "X<Y<int>>" 
  // where the ">>" fools the C++ parser and causes an error in the processing of the 
  // generated (unparsed) file using the filename build from such a string.
  // Note that filtering the function name to put spaces between ">'s" causes spaces 
  // in the function name but this appears to be OK.

     string s = name.str();
     string t = ">>";
  // printf ("Before filtering s = %s \n",s.c_str());

  // AKP (2/17/2007: testing string name here not the SgName
  //                 need to catch "operator>>="in addition to "operator>>"
  // DQ (3/19/2005): We should not be converting "operator>>" to "operator> >"
  // if (name != "operator>>")
     if ((s != "operator>>") && (s != "operator>>=") )
        {
          while (s.find(t) != string::npos)
             {
               s.replace(s.find(t),2,"> >");
             }
        }

  // printf ("After filtering s = %s \n",s.c_str());

  // DQ (3/19/2005): Make sure that we didn't do anything silly :-).
     ROSE_ASSERT (name != "operator> >");

     name = s.c_str();

     string nameString = name.str();
  // Search for common substring of unconverted EDG generated typenames...
     if ( nameString.find("____") != string::npos )
        {
          printf ("Error: \"____\" found in EDG generated type name = %s (likely and unconverted EDG name) \n",nameString.c_str());
          ROSE_ASSERT (false);
        }     
   }

#ifdef DEBUG_CONVERSION
// DQ (3/21/2017): Allow this only in debug mode to improve code coverage.
void testForBadGenerateTemplateName( const std::string & name ) 
   {
  // DQ (2/20/2005): Error checking on existence of substrings that can be common
  // in building template name and which are a source of errors in generated code.
     if ( (name.find("<<") != string::npos) || (name.find(">>") != string::npos) )
        {
       // This could just be the input or output operators so make sure this is not that case!
          if ( (name.find("operator<<") == string::npos) && (name.find("operator>>") == string::npos) )
             {
            // OK, we have some sort of error to report!
               printf ("\n\n");
               printf ("Error: \"<<\" or \">>\" found in generated name, this could lead to an error in compiling the generated code later (in edg47Rose/edgRose.C)\n");
            // printf ("..... name generated in processing a %s IR node \n",sage_class_name());
               printf ("..... generated name = %s \n",name.c_str());
               printf ("\n\n");
               ROSE_ASSERT (false);
            // ROSE_ASSERT (returnName.find("<<") == string::npos);
            // ROSE_ASSERT (returnName.find(">>") == string::npos);
             }
        }

  // Search for common substring of unconverted EDG generated typenames...
     if ( name.find("____") != string::npos )
        {
          printf ("Error: \"____\" found in ROSE generated type name = %s (likely and unconverted EDG name) \n",name.c_str());
       // printf ("Error: SgDeclarationStatement (this) = %p = %s = %s \n",this,this->class_name().c_str(),SageInterface::get_name(this).c_str());
       // get_file_info()->display("Error: likely and unconverted EDG name");
          ROSE_ASSERT (false);
        }
   }
#endif



void initialize (SgSourceFile* inputSourceFile)
   {
  // This function initializes data in the new namespace.
  // Since we are now using a namespace for EDG_ROSE_Translation instead of a
  // struct, we have don't have a constructor in which to initialize member data.
     indent = 0;

     global_source_sequence_number = 0;

     ROSE_ASSERT(inputSourceFile != NULL);
     currentSourceFile = inputSourceFile;

  // DQ (4/14/2014): Added support to set behavior to either build function calls to specify 
  // unparsing of overloaded operator names or use operator syntax.
     global_call_uses_operator_syntax = inputSourceFile->get_unparse_function_calls_using_operator_syntax();
     global_call_uses_operator_names  = inputSourceFile->get_unparse_function_calls_using_operator_names();

#ifdef DEBUG_CONVERSION
     printf ("In initialize(SgSourceFile*): global_call_uses_operator_syntax = %s \n",global_call_uses_operator_syntax ? "true" : "false");
     printf ("In initialize(SgSourceFile*): global_call_uses_operator_names  = %s \n",global_call_uses_operator_names  ? "true" : "false");
#endif

  // DQ (12/28/2019): Reset to default values so that we can support multiple files.
     convert_typedef_last_source_sequence_number            = -1 /* default value (set to MAX unsigned int value) */;
     convert_typedef_last_decl                              = NULL;
     parse_field_declaration_last_source_sequence_number    = -1 /* default value (set to MAX unsigned int value) */;
     parse_field_declaration_last_decl                      = NULL;
     parse_variable_declaration_last_source_sequence_number = -1 /* default value (set to MAX unsigned int value) */;
     parse_variable_declaration_last_decl                   = NULL;
   }

bool isDeclaredInAnInstantiation( a_source_correspondence & scp, bool runTest )
   {
  // DQ (4/28/2012): Added support to detect nesting of declarations in instantations (which 
  // is used to force generation of instantations instead of template declarations). This 
  // function is used in the isTemplateDeclaration() functions (overloaded on edg types).

  // FIXME: DQ (5/3/2012): This is a poor way to optimize the use of this function...
  // Note that this is a more expensive test so optimize it away using the runTest variable.

     bool returnResult = false;

#if DEBUG_GET_DECLARATION_SCOPE
     printf ("In isTemplateDeclaration(edg_ptr): scp.name = %s runTest = %s \n",scp.name,runTest ? "true" : "false");
#endif

  // DQ (4/28/2012): As another marker, if the parent scope is a template instantiation, then the current template must be an instantiation (though it might only be a partial instantiation).
  // if (buildTemplateDeclaration == true && scp.parent_scope != NULL)
     if (runTest == true)
        {
       // a_scope_ptr edg_scopeOfDeclaration = p->parent_scope;
#if DEBUG_GET_DECLARATION_SCOPE
          printf ("In isTemplateDeclaration(edg_ptr): calling getDeclarationScope() \n");
#endif
       // SgScopeStatement* rose_scopeOfDeclaration = getDeclarationScope(p->source_corresp);
          SgScopeStatement* rose_scopeOfDeclaration = getDeclarationScope(scp);
          ROSE_ASSERT(rose_scopeOfDeclaration != NULL);

#if DEBUG_GET_DECLARATION_SCOPE
          printf ("DONE: In isTemplateDeclaration(a_routine_ptr): calling getDeclarationScope() rose_scopeOfDeclaration = %p = %s \n",rose_scopeOfDeclaration,rose_scopeOfDeclaration->class_name().c_str());
#endif

          ROSE_ASSERT(rose_scopeOfDeclaration != NULL);
          SgTemplateInstantiationDefn* templateInstantiationDefn = isSgTemplateInstantiationDefn(rose_scopeOfDeclaration);
          if (templateInstantiationDefn != NULL)
             {
            // Some extra error checking.
               SgClassDeclaration* classDeclaration = templateInstantiationDefn->get_declaration();
               ROSE_ASSERT(classDeclaration != NULL);
               SgTemplateInstantiationDecl* templateInstantiationDeclaration = isSgTemplateInstantiationDecl(classDeclaration);
               ROSE_ASSERT(templateInstantiationDeclaration != NULL);
#if 0
               printf ("In isDeclaredInAnInstantiation(edg_ptr): If scp.parent_scope != NULL, then check for parent as an instantiation (return true) \n");
#endif
               returnResult = true;
             }
        }

#if DEBUG_GET_DECLARATION_SCOPE
     printf ("Leaving isDeclaredInAnInstantiation(edg_ptr): runTest = %s returnResult = %s \n",runTest ? "true" : "false",returnResult ? "true" : "false");
#endif

     return returnResult;
   }

bool templateArgumentsAreTemplateParameters(a_type_ptr p)
   {
  // Iterate over the template argument list and check if all are template parameters.
  // alternative name: areTemplateArgumentsReallyJustTemplateParameters

     bool result = true;

  // DQ (1/15/2016): This function should only apply to classes (or structs or unions).
     ROSE_ASSERT( (p->kind == tk_class) || (p->kind == tk_struct) || (p->kind == tk_union) );

  // DQ (1/15/2016): Not clear if this can be enforced.
     ROSE_ASSERT(p->variant.class_struct_union.extra_info != NULL);

     a_template_arg_ptr template_arg = p->variant.class_struct_union.extra_info->template_arg_list;

#if 0
     printf ("In templateArgumentsAreTemplateParameters(a_type_ptr p = %p): output the template_arg_list: \n",p);
     summarize_template_arg_list(p->variant.class_struct_union.extra_info->template_arg_list);
#endif

     while (template_arg != NULL)
       {
      // bool isTemplateParameter = (template_arg->kind == tk_template_param);
      // bool isTemplateParameter = (template_arg->kind == tak_type && template_arg->variant.type->kind == tk_template_param);
#if 0
         bool isTemplateParameter = (template_arg->kind == tak_nontype && template_arg->variant.constant->kind == ck_template_param) ||
                                    (template_arg->kind == tak_type    && template_arg->variant.type->kind == tk_template_param)     ||
                                    (template_arg->kind != tak_template);
#else
         bool isTemplateParameter = (template_arg->kind == tak_nontype && template_arg->variant.constant->kind == ck_template_param) ||
                                    (template_arg->kind == tak_type    && template_arg->variant.type->kind == tk_template_param);
         if (template_arg->kind == tak_template)
            {
              isTemplateParameter = false;
            }
#endif

         if (isTemplateParameter == false)
            {
              result = false;
            }

         template_arg = template_arg->next;
       }

#if 0
     printf ("Leaving templateArgumentsAreTemplateParameters(): result = %s \n",result ? "true" : "false");
#endif

     return result;
   }


bool isTemplateDeclaration (a_type_ptr p)
   {
  // This refactors the logic to distinguish when a type will be associated with a template declaration or a template class instantiation.

     ROSE_ASSERT(p != NULL);

#define DEBUG_IS_TEMPLATE_CLASS_DECLARATION 0

#if 0
     printf ("In isTemplateDeclaration(a_type_ptr): p = %p \n",p);
#endif

  // DQ (1/15/2016): This function should only apply to classes (or structs or unions).
     ROSE_ASSERT( (p->kind == tk_class) || (p->kind == tk_struct) || (p->kind == tk_union) );

  // DQ (1/15/2016): Not clear if this can be enforced.
     ROSE_ASSERT(p->variant.class_struct_union.extra_info != NULL);

#if DEBUG_IS_TEMPLATE_CLASS_DECLARATION
     printf ("In isTemplateDeclaration(a_type_ptr = %p): p->variant.class_struct_union.is_template_class             = %s \n",p,p->variant.class_struct_union.is_template_class ? "true" : "false");
     printf ("In isTemplateDeclaration(a_type_ptr = %p): p->variant.class_struct_union.is_prototype_instantiation    = %s \n",p,p->variant.class_struct_union.is_prototype_instantiation ? "true" : "false");
     ROSE_ASSERT(p->variant.class_struct_union.extra_info != NULL);
     printf ("In isTemplateDeclaration(a_type_ptr = %p): p->variant.class_struct_union.extra_info->template_arg_list = %p \n",p,p->variant.class_struct_union.extra_info->template_arg_list);
#endif

  // DQ (1/15/2016): Added test of the template arguments because when they are simple template parameters the type should be considered a template (see test2016_05.C).
     bool templateArgumentsReallyJustTemplateParameters = templateArgumentsAreTemplateParameters(p);

#if 0
     bool buildTemplateDeclaration = (p->variant.class_struct_union.is_template_class == true) && 
                                     ( (p->variant.class_struct_union.is_prototype_instantiation == false) && 
                                       (p->variant.class_struct_union.extra_info->template_arg_list == NULL) );
#else
     bool buildTemplateDeclaration = (p->variant.class_struct_union.is_template_class == true) && 
                                     ( (p->variant.class_struct_union.is_prototype_instantiation == false || templateArgumentsReallyJustTemplateParameters == true) && 
                                       (p->variant.class_struct_union.extra_info->template_arg_list == NULL ||
                                         (p->variant.class_struct_union.extra_info->template_arg_list != NULL && templateArgumentsReallyJustTemplateParameters == true)) );
#endif

#if DEBUG_IS_TEMPLATE_CLASS_DECLARATION
     printf ("In isTemplateDeclaration(a_type_ptr = %p): buildTemplateDeclaration = %s is_nonreal_class == %s \n",
          p,buildTemplateDeclaration ? "true" : "false",p->variant.class_struct_union.is_nonreal_class ? "true" : "false");
#endif
#if 0
     printf ("In isTemplateDeclaration(a_type_ptr = %p): calling summarize_type() \n",p);
     summarize_type(p);
#endif

#if 0
  // DQ (2/11/2013): If is_nonreal_class == true then don't let this be a template declaration.
  // This is part of fixing the compilation of the "rose.h" header file (in ROSE compiling ROSE tests).
     if (p->variant.class_struct_union.is_nonreal_class == true)
        {
#if 0
          printf ("In isTemplateDeclaration(a_type_ptr = %p): buildTemplateDeclaration = %s: However this is_nonreal_class == true, so it should NOT be a template declaration (reset buildTemplateDeclaration = false) \n",p,buildTemplateDeclaration ? "true" : "false");
#endif
          buildTemplateDeclaration = false;
        }
#endif

  // DQ (1/30/2018): I want to see if internally we can consider p == p->source_corresp, so that I can access 
  // the value of "p" in getDeclarationScope() without having to pass it akwardly as a 3rd parameter.
     ROSE_ASSERT((void*) p == (void*) &(p->source_corresp));
  // ROSE_ASSERT((void*) p == (void*) &(p->source_corresp.source_sequence_entry));

#if 0
     printf ("In isTemplateDeclaration(a_type_ptr = %p): Calling getDeclarationScope(): &(p->source_corresp) = %p &(p->source_corresp.source_sequence_entry) = %p \n",
          p,&(p->source_corresp),&(p->source_corresp.source_sequence_entry));
#endif

  // Test if this is a function in a template declaration.
     SgScopeStatement* scopeOfFunction = getDeclarationScope(p->source_corresp);
     ROSE_ASSERT(scopeOfFunction != NULL);

#if DEBUG_IS_TEMPLATE_CLASS_DECLARATION
     printf ("In isTemplateDeclaration(a_type_ptr = %p): scopeOfFunction = %p = %s \n",p,scopeOfFunction,scopeOfFunction->class_name().c_str());
#endif

  // DQ (5/17/2014): Fix for test2014_56.C typedef of template instantiation in a class template.
#if 0
  // if (isSgTemplateClassDefinition(scopeOfFunction) != NULL)
     bool isClassStructUnion = (p->kind == tk_class) || (p->kind == tk_struct) || (p->kind == tk_union);
     if ( ( (isClassStructUnion == true && p->variant.class_struct_union.is_nonreal_class == false) || isClassStructUnion == false) && isSgTemplateClassDefinition(scopeOfFunction) != NULL  )
#else
  // DQ (5/17/2014): This code works better since more often any class declared in a template class will be treated as a template (instead of a non-template).
     if (isSgTemplateClassDefinition(scopeOfFunction) != NULL)
#endif
        {
       // DQ (5/16/2014): This is a problem for test2014_56.C, where a typedef is taken of a template instantiation, 
       // but the template instantiation is being interpreted as a template declaration (because of this code).
#if DEBUG_IS_TEMPLATE_CLASS_DECLARATION
          printf ("In isTemplateDeclaration(a_type_ptr): buildTemplateDeclaration = %s: However this is a function defined in a SgTemplateClassDefinition, so it should be a template declaration (reset buildTemplateDeclaration = true) \n",buildTemplateDeclaration ? "true" : "false");
#endif
          buildTemplateDeclaration = true;
        }

#if 0
  // DQ (5/17/2014): Try this.
     if (p->autonomous_primary_tag_decl == false)
        {
#if DEBUG_IS_TEMPLATE_CLASS_DECLARATION
          printf ("In isTemplateDeclaration(a_type_ptr): p->autonomous_primary_tag_decl == false (reset buildTemplateDeclaration = false) \n");
#endif
          buildTemplateDeclaration = false;
        }
#endif

  // DQ (4/27/2012): Added test to support debugging (see test2004_77.C; made symetric with the function handling).
     a_template_arg_ptr templateArgs = p->variant.class_struct_union.extra_info->template_arg_list;
     if (buildTemplateDeclaration == true && templateArgs != NULL)
        {
#ifdef DEBUG_CONVERSION
          printf ("WARNING: This might be a template class instantiation being interpreted as a template declaration \n");
#endif
        }

#if 0
  // DQ (4/27/2012): Fixing test2012_68.C (testing)...
     if (buildTemplateDeclaration == true && p->instance_required == true)
        {
          printf ("In isTemplateDeclaration(a_type_ptr): If p->instance_required == true, then this must be an instantiation (reset buildTemplateDeclaration = false) \n");
          buildTemplateDeclaration = false;
        }
#endif

#if 1
     bool runTest = buildTemplateDeclaration;
     if (isDeclaredInAnInstantiation(p->source_corresp,runTest) == true)
        {
#if DEBUG_IS_TEMPLATE_CLASS_DECLARATION
          printf ("In isTemplateDeclaration(a_type_ptr): buildTemplateDeclaration = %s: If p->source_corresp.parent_scope != NULL, then check for parent as an instantiation (reset buildTemplateDeclaration = false) \n",buildTemplateDeclaration ? "true" : "false");
#endif
          buildTemplateDeclaration = false;
        }
#else
  // DQ (4/28/2012): As another marker, if the parent scope is a template instantiation, then the current template must be an instantiation (though it might only be a partial instantiation).
     if (buildTemplateDeclaration == true && p->source_corresp.parent_scope != NULL)
        {
       // a_scope_ptr edg_scopeOfDeclaration = p->parent_scope;

       // printf ("In isTemplateDeclaration(a_type_ptr): CALLING getDeclarationScope() p = %p \n",p);
          SgScopeStatement* rose_scopeOfDeclaration = getDeclarationScope(p->source_corresp);
       // printf ("DONE: IN isTemplateDeclaration(a_type_ptr): CALLING getDeclarationScope() p = %p \n",p);
          ROSE_ASSERT(rose_scopeOfDeclaration != NULL);
          SgTemplateInstantiationDefn* templateInstantiationDefn = isSgTemplateInstantiationDefn(rose_scopeOfDeclaration);

#error "DEAD CODE!"

          if (templateInstantiationDefn != NULL)
             {
               SgClassDeclaration* classDeclaration = templateInstantiationDefn->get_declaration();
               ROSE_ASSERT(classDeclaration != NULL);
               SgTemplateInstantiationDecl* templateInstantiationDeclaration = isSgTemplateInstantiationDecl(classDeclaration);
               ROSE_ASSERT(templateInstantiationDeclaration != NULL);

               printf ("In isTemplateDeclaration(a_type_ptr): If p->source_corresp.parent_scope != NULL, then check for parent as an instantiation (reset buildTemplateDeclaration = false) \n");
               buildTemplateDeclaration = false;
             }
        }
#endif

#if DEBUG_IS_TEMPLATE_CLASS_DECLARATION || 0
     printf ("Leaving isTemplateDeclaration(a_type_ptr): buildTemplateDeclaration   = %s \n",buildTemplateDeclaration ? "true" : "false");
#endif

     return buildTemplateDeclaration;
   }


bool isTemplateInstantiation (a_type_ptr p)
   {
  // This refactors the logic to distinguish when a type will be associated with a template declaration or a template class instantiation.

     ROSE_ASSERT(p != NULL);

  // DQ (3/7/2012): Debugging a randomly failing test.
  // ROSE_ASSERT(p->variant.class_struct_union.extra_info != NULL);
  // printf ("In isTemplateInstantiation(a_type_ptr): p->variant.class_struct_union.extra_info = %p \n",p->variant.class_struct_union.extra_info);

  // DQ (5/17/2014): Added code to detect when this type is a class.
     bool isClassStructUnion = (p->kind == tk_class) || (p->kind == tk_struct) || (p->kind == tk_union);

#if 0
#if 1
  // DQ (1/1/2012): This is the original version, I think this is OK.
     bool buildTemplateInstantiation = (p->variant.class_struct_union.is_template_class == true) && 
                                       ( (p->variant.class_struct_union.is_prototype_instantiation == true) || 
                                         ( (p->variant.class_struct_union.extra_info != NULL) &&
                                           (p->variant.class_struct_union.extra_info->template_arg_list != NULL)) );
#else
  // DQ (1/1/2012): This is new and I think it is not as good!
     bool buildTemplateInstantiation = (p->variant.class_struct_union.is_template_class == true) && 
                                       (p->variant.class_struct_union.extra_info->template_arg_list != NULL);
#endif
#else
  // DQ (5/17/2014): This is a newer and I hope better test for when a class is a template instantiation.
  // Use  assoc_template instead of template_arg_list, since we can have situations in Boost where it is 
  // a template instantiation, but no explicit template argument as available).
     bool buildTemplateInstantiation = (isClassStructUnion == true) && 
                                       (p->variant.class_struct_union.originally_unnamed == false) && 
                                       ( ( (p->variant.class_struct_union.extra_info != NULL) &&
                                        // (p->variant.class_struct_union.extra_info->template_arg_list != NULL) ) );
                                           (p->variant.class_struct_union.extra_info->assoc_template != NULL) ) );
#endif

#if 0
  // DQ (5/17/2014): Added code to detect when this is an un-named type (which can't be a template instantiation).
     if (p->variant.class_struct_union.originally_unnamed == true && buildTemplateInstantiation == true)
        {
       // We can't have an un-named type that is a template or template instantiation.
          printf ("In isTemplateInstantiation(a_type_ptr): p->variant.class_struct_union.originally_unnamed == true: reset buildTemplateInstantiation == false \n");
          buildTemplateInstantiation = false;
        }
#endif

#if 0
     printf ("In isTemplateInstantiation(a_type_ptr): buildTemplateInstantiation = %s isClassStructUnion = %s \n",buildTemplateInstantiation ? "true" : "false",isClassStructUnion ? "true" : "false");
#endif

  // DQ (5/17/2014): The above initialization of buildTemplateInstantiation shold not be valid for isClassStructUnion == false.
     ROSE_ASSERT(isClassStructUnion == true || (isClassStructUnion == false && buildTemplateInstantiation == false));

#if 0
  // DQ (5/3/2012): This would make the use of isDeclaredInAnInstantiation() symetric, but has not be required yet.
  // Likely we need an example of a non template class in a templated class (debug this later).
     bool runTest = !buildTemplateInstantiation;
     if (isDeclaredInAnInstantiation(p->source_corresp,runTest) == true)
        {
       // It might be that we should be checking the befriending class as well, but not yet.
          printf ("In isTemplateInstantiation(a_type_ptr): If p->source_corresp.parent_scope != NULL, then check for parent as an instantiation (reset buildTemplateInstantiation = true) \n");
          buildTemplateInstantiation = true;
        }
#endif

#if 0
     printf ("Leaving isTemplateInstantiation(a_type_ptr): edg_type p = %p buildTemplateInstantiation = %s \n",p,buildTemplateInstantiation ? "true" : "false");
#endif

     return buildTemplateInstantiation;
   }

bool isTemplateDeclaration (a_routine_ptr p)
   {
  // This refactors the logic to distinguish when a type will be associated with a template declaration or a template class instantiation.

#define DEBUG_IS_TEMPLATE_DECLARATION 0

     ROSE_ASSERT(p != NULL);

  // DQ (1/1/2012): Modified when we interpret functions as a template declaration vs. a template instantiation.
  // bool buildTemplateDeclaration = (p->is_template_function == true) && (p->called == false);

#if 0
     printf ("Calling summarize_routine() from isTemplateDeclaration() \n");
     summarize_routine(p);
#endif

#if 0
     if (p->source_corresp.name_references == NULL)
        {
          printf ("Calling summarize_routine() from isTemplateDeclaration() \n");
          summarize_routine(p);
        }
     ROSE_ASSERT(p->source_corresp.name_references != NULL);

     bool buildTemplateDeclaration = (p->is_template_function == true) && (p->called == false) && (p->source_corresp.name_references->from_prototype_instantiation == true);

     printf ("In isTemplateDeclaration(a_routine_ptr): buildTemplateDeclaration   = %s \n",buildTemplateDeclaration ? "true" : "false");

     if (buildTemplateDeclaration == true)
        {
          ROSE_ASSERT(p->source_corresp.name_references != NULL);
          ROSE_ASSERT(p->source_corresp.name_references->from_prototype_instantiation == true);
        }
#else
  // DQ (2/9/2012): Note that we should not use the name_references field, or at least not depend on it to be a valid pointer.
  // Note that p->source_corresp.name_references == NULL for test2004_30.C
  // bool buildTemplateDeclaration = (p->is_template_function == true) && (p->called == false) && (p->source_corresp.name_references != NULL && p->source_corresp.name_references->from_prototype_instantiation == true);

  // DQ (8/26/2014): This is the original code.
  // bool buildTemplateDeclaration = (p->is_template_function == true) && (p->called == false);

  // This is the test that appears to work well for classes.
  // bool buildTemplateDeclaration = (p->variant.class_struct_union.is_template_class == true) && 
  //                                 ( (p->variant.class_struct_union.is_prototype_instantiation == false) && 
  //                                   (p->variant.class_struct_union.extra_info->template_arg_list == NULL) );
     bool buildTemplateDeclaration = (p->is_template_function == true) && ( (p->assoc_template != NULL) && (p->template_arg_list == NULL) );
#endif

#if DEBUG_IS_TEMPLATE_DECLARATION
     printf ("In isTemplateDeclaration(a_routine_ptr): p                        = %p \n",p);
     printf ("In isTemplateDeclaration(a_routine_ptr): p->is_template_function  = %s \n",p->is_template_function ? "true" : "false");
     printf ("In isTemplateDeclaration(a_routine_ptr): p->assoc_template        = %p \n",p->assoc_template);
     printf ("In isTemplateDeclaration(a_routine_ptr): p->template_arg_list     = %p \n",p->template_arg_list);
     printf ("In isTemplateDeclaration(a_routine_ptr): p->called (not used)     = %s \n",p->called ? "true" : "false");
     printf ("In isTemplateDeclaration(a_routine_ptr): buildTemplateDeclaration = %s \n",buildTemplateDeclaration ? "true" : "false");
  // ROSE_ASSERT(p->source_corresp.name_references != NULL);
  // printf ("In isTemplateDeclaration(a_routine_ptr): p->source_corresp.name_references->from_prototype_instantiation = %s \n",p->source_corresp.name_references->from_prototype_instantiation ? "true" : "false");
#endif

#if 0
  // DQ (8/26/2014): We can't use this function since it can now return either of two scopes depending 
  // on if the scope has been process as both a template declaration and template instantiation.
  // Test if this is a function in a template declaration.
     SgScopeStatement* scopeOfFunction = getDeclarationScope(p->source_corresp);
#else
  // DQ (8/26/2014): Try to look at the current scope to see if we can identify this as being in a template declaraion scope.
     SgScopeStatement* scopeOfFunction = topScopeStack();
#if DEBUG_IS_TEMPLATE_DECLARATION
     printf ("In isTemplateDeclaration(a_routine_ptr): (warning) Using the topScopeStack() = %p = %s \n",topScopeStack(),topScopeStack()->class_name().c_str());
#endif
#endif
     ROSE_ASSERT(scopeOfFunction != NULL);

     if (isSgTemplateClassDefinition(scopeOfFunction) != NULL)
        {
#if DEBUG_IS_TEMPLATE_DECLARATION
          printf ("In isTemplateDeclaration(a_routine_ptr): This is a function defined in a SgTemplateClassDefinition, so it should be a template declaration \n");
          printf ("In isTemplateDeclaration(a_routine_ptr): scopeOfFunction = %p = %s \n",scopeOfFunction,scopeOfFunction->class_name().c_str());
          printf ("In isTemplateDeclaration(a_routine_ptr): topScopeStack() = %p = %s \n",topScopeStack(),topScopeStack()->class_name().c_str());
#endif
          if (scopeOfFunction != topScopeStack())
             {
               printf ("WARNING: In isTemplateDeclaration(a_routine_ptr): scopeOfFunction != topScopeStack(): we might be marking this as a template declaration incorrectly! \n");
             }

          buildTemplateDeclaration = true;
        }
       else
        {
       // Nothing to do here!
          if (isSgTemplateInstantiationDefn(scopeOfFunction) != NULL)
             {
#if DEBUG_IS_TEMPLATE_DECLARATION
               printf ("In isTemplateDeclaration(a_routine_ptr): This is a function defined in a SgTemplateInstantiationDefn, so it should be a template instantiation (unless we can prove (force) that it should be a template declaration) \n");
               printf ("In isTemplateDeclaration(a_routine_ptr): scopeOfFunction = %p = %s \n",scopeOfFunction,scopeOfFunction->class_name().c_str());
               printf ("In isTemplateDeclaration(a_routine_ptr): topScopeStack() = %p = %s \n",topScopeStack(),topScopeStack()->class_name().c_str());
#endif
               buildTemplateDeclaration = false;
             }
        }

#if DEBUG_IS_TEMPLATE_DECLARATION
     printf ("In isTemplateDeclaration(a_routine_ptr): After checking the parent: buildTemplateDeclaration = %s \n",buildTemplateDeclaration ? "true" : "false");
#endif

  // DQ (4/27/2012): Added test to support debugging (see test2004_77.C).
     a_template_arg_ptr templateArgs = p->template_arg_list;

  // DQ (12/29/2014): Use this as a correction for EDG normalized template functions (see test2014_242.C).
  // Note that if this is a problem then we might check the top of the template scope stack using: templateDeclarationStack.
  // if (buildTemplateDeclaration == true && templateArgs != NULL)
  // if (buildTemplateDeclaration == true && templateArgs != NULL && isSgTemplateClassDefinition(scopeOfFunction) == NULL)
  // if (buildTemplateDeclaration == true && templateArgs != NULL)
     if (buildTemplateDeclaration == true && templateArgs != NULL && isSgTemplateClassDefinition(scopeOfFunction) == NULL)
        {
#if defined(DEBUG_CONVERSION) || DEBUG_IS_TEMPLATE_DECLARATION
          printf ("WARNING: This might be a template function instantiation being interpreted as a template declaration \n");
          printf ("   --- Force this to be a template instantiation (marking buildTemplateDeclaration to be false, but might not override other rational) \n");
#endif

          a_class_list_entry_ptr befriending_class = p->friends_or_originator.befriending_classes;
          while(befriending_class != NULL)
             {
               printf ("      befriending class_type = %p \n",befriending_class->class_type);

            // SgClassDeclaration* classDeclaration = NULL;
               a_type_ptr befriending_class = p->friends_or_originator.befriending_classes->class_type;
               if (template_type_translation_cache.find(befriending_class) != template_type_translation_cache.end())
                  {
                    SgType* roseType = template_type_translation_cache[befriending_class];
                    ROSE_ASSERT(roseType != NULL);

                    printf ("   --- roseType = %p = %s \n",roseType,roseType->class_name().c_str());
#if 0
                    printf ("Exiting as a test! \n");
                    ROSE_ASSERT(false);
#endif
                  }
               

               befriending_class = befriending_class->next;
             }

          buildTemplateDeclaration = false;
        }
       else
        {
          if (buildTemplateDeclaration == true && templateArgs == NULL)
             {
#if defined(DEBUG_CONVERSION) || DEBUG_IS_TEMPLATE_DECLARATION
               printf ("WARNING: (buildTemplateDeclaration == true && templateArgs == NULL): This STILL might be a template function instantiation being interpreted as a template declaration \n");
#endif

            // DQ (8/25/2014): Added code to support testing of test2014_161.C.
#if 0
            // Might have to check p->assoc_template->kind == routine
               ROSE_ASSERT(p->assoc_template != NULL);
            // ROSE_ASSERT(p->assoc_template->kind == templk_function);
            // ROSE_ASSERT(p->assoc_template->kind == templk_member_function);
               ROSE_ASSERT(p->assoc_template->kind == templk_function || p->assoc_template->kind == templk_member_function);

               if ( (p->assoc_template->kind == templk_function) || (p->assoc_template->kind == templk_member_function) )
                  {
#if 0
                    printf ("routine kind == templk_function: so we need to check if p == p->routine.extra_info.assoc_routine \n");
#endif
#if 0
                 // DQ (8/25/2014): Commented out to revert to previous working state.
                    if (p == p->assoc_template->prototype_instantiation.routine)
                       {
                         printf ("p == p->assoc_template->prototype_instantiation.routine (reset buildTemplateDeclaration to false) \n");

                         buildTemplateDeclaration = false;
                       }
#endif
#if 0
                    printf ("Exiting as a test! \n");
                    ROSE_ASSERT(false);
#endif
                  }
#endif
             }
        }

  // DQ (4/27/2012): Fixing test2012_68.C (testing)...
     if (buildTemplateDeclaration == true && p->instance_required == true)
        {
#if DEBUG_IS_TEMPLATE_DECLARATION
          printf ("In isTemplateDeclaration(a_routine_ptr): If p->instance_required == true, then this must be an instantiation (reset buildTemplateDeclaration = false) \n");
#endif
          buildTemplateDeclaration = false;
        }

#if 1
     bool runTest = buildTemplateDeclaration;
     if (isDeclaredInAnInstantiation(p->source_corresp,runTest) == true)
        {
#if DEBUG_IS_TEMPLATE_DECLARATION
          printf ("In isTemplateDeclaration(a_routine_ptr): If p->source_corresp.parent_scope != NULL, then check for parent as an instantiation (reset buildTemplateDeclaration = false) \n");
#endif
          buildTemplateDeclaration = false;
        }
#else
  // DQ (4/28/2012): As another marker, if the parent scope is a template instantiation, then the current template must be an instantiation (though it might only be a partial instantiation).
     if (buildTemplateDeclaration == true && p->source_corresp.parent_scope != NULL)
        {
       // a_scope_ptr edg_scopeOfDeclaration = p->parent_scope;

       // printf ("IN isTemplateDeclaration(a_routine_ptr): CALLING getDeclarationScope() p = %p \n",p);
          SgScopeStatement* rose_scopeOfDeclaration = getDeclarationScope(p->source_corresp);
       // printf ("DONE: IN isTemplateDeclaration(a_routine_ptr): CALLING getDeclarationScope() p = %p \n",p);
          ROSE_ASSERT(rose_scopeOfDeclaration != NULL);
          SgTemplateInstantiationDefn* templateInstantiationDefn = isSgTemplateInstantiationDefn(rose_scopeOfDeclaration);

#error "DEAD CODE!"

          if (templateInstantiationDefn != NULL)
             {
               SgClassDeclaration* classDeclaration = templateInstantiationDefn->get_declaration();
               ROSE_ASSERT(classDeclaration != NULL);
            // SgTemplateInstantiationDefn* templateInstantiationDefinition = isSgTemplateInstantiationDefn(classDeclaration);
            // ROSE_ASSERT(templateInstantiationDefinition != NULL);
               SgTemplateInstantiationDecl* templateInstantiationDeclaration = isSgTemplateInstantiationDecl(classDeclaration);
               ROSE_ASSERT(templateInstantiationDeclaration != NULL);

               printf ("In isTemplateDeclaration(a_routine_ptr): If p->source_corresp.parent_scope != NULL, then check for parent as an instantiation (reset buildTemplateDeclaration = false) \n");
               buildTemplateDeclaration = false;
             }
        }
#endif

#if ENFORCE_NO_FUNCTION_TEMPLATE_DECLARATIONS_IN_TEMPLATE_CLASS_INSTANTIATIONS
  // DQ (8/25/2014): Added this test to detect template declarations in template instantiations.
     a_scope_ptr edg_scope = p->source_corresp.parent_scope;
     ROSE_ASSERT(edg_scope != NULL);
     SgScopeStatement* rose_scope = scope_cache[edg_scope];
     ROSE_ASSERT(rose_scope != NULL);

     if (buildTemplateDeclaration == true && isSgTemplateInstantiationDefn(rose_scope) != NULL)
        {
       // DQ (8/25/2014): We need to detect case of partial specialization which would be OK.
          printf ("ERROR: isSgTemplateInstantiationDefn(rose_scope) != NULL: This should be built as an instantiation of a template function instead of a template function \n");

          printf ("Error: this should be an instantiation! \n");
          ROSE_ASSERT(false);
        }

     if (buildTemplateDeclaration == true && isSgTemplateInstantiationDefn(topScopeStack()) != NULL)
        {
       // DQ (8/25/2014): We need to detect case of partial specialization which would be OK.
          printf ("ERROR: isSgTemplateInstantiationDefn(topScopeStack()) != NULL: This should be built as an instantiation of a template function instead of a template function \n");

          buildTemplateDeclaration = false;
#if 0
          printf ("Error: this should be an instantiation! \n");
          ROSE_ASSERT(false);
#endif
        }
#endif

#if DEBUG_IS_TEMPLATE_DECLARATION
     printf ("Leaving isTemplateDeclaration(a_routine_ptr): returning buildTemplateDeclaration = %s \n",buildTemplateDeclaration ? "true" : "false");
#endif

     return buildTemplateDeclaration;
   }


bool isTemplateInstantiation (a_routine_ptr p)
   {
  // This refactors the logic to distinguish when a type will be associated with a template declaration or a template class instantiation.

#define DEBUG_IS_TEMPLATE_INSTANTIATION 0

     ROSE_ASSERT(p != NULL);

  // DQ (1/1/2012): Modified when we interpret functions as a template declaration vs. a template instantiation.
  // bool buildTemplateInstantiation = (p->is_template_function == true) && (p->called == true);
#if 0
     ROSE_ASSERT(p->source_corresp.name_references != NULL);
     bool buildTemplateInstantiation = (p->is_template_function == true) && (p->called == true) && (p->source_corresp.name_references->from_prototype_instantiation == false);

     printf ("In isTemplateInstantiation(a_routine_ptr): buildTemplateInstantiation = %s \n",buildTemplateInstantiation ? "true" : "false");

     if (buildTemplateInstantiation == true)
        {
          ROSE_ASSERT(p->source_corresp.name_references != NULL);
          ROSE_ASSERT(p->source_corresp.name_references->from_prototype_instantiation == false);
        }
#else
  // DQ (2/9/2012): Note that we should not use the name_references field, or at least not depend on it to be a valid pointer.
  // Note that p->source_corresp.name_references == NULL for test2004_30.C
  // bool buildTemplateInstantiation = (p->is_template_function == true) && (p->called == true) && (p->source_corresp.name_references != NULL && p->source_corresp.name_references->from_prototype_instantiation == false);
     bool buildTemplateInstantiation = (p->is_template_function == true) && (p->called == true);
#endif

#if DEBUG_IS_TEMPLATE_INSTANTIATION || 0
     printf ("In isTemplateInstantiation(a_routine_ptr): p->is_template_function       = %s \n",p->is_template_function ? "true" : "false");
     printf ("In isTemplateInstantiation(a_routine_ptr): p->called                     = %s \n",p->called ? "true" : "false");
     printf ("In isTemplateInstantiation(a_routine_ptr): p->is_prototype_instantiation = %s \n",p->is_prototype_instantiation ? "true" : "false");
     printf ("In isTemplateInstantiation(a_routine_ptr): buildTemplateInstantiation    = %s \n",buildTemplateInstantiation ? "true" : "false");
#endif

#if 0
  // DQ (4/27/2012): Added test to support debugging (see test2004_77.C).
     a_template_arg_ptr templateArgs = p->template_arg_list;
     if (buildTemplateInstantiation == false)
        {
       // ROSE_ASSERT(templateArgs != NULL);
#if 0
          printf ("In isTemplateInstantiation(a_routine_ptr): buildTemplateInstantiation == false templateArgs = %p \n",templateArgs);
#endif
        }
#endif

  // DQ (4/27/2012): Fixing test2012_68.C (testing)...
     if (buildTemplateInstantiation == false && p->instance_required == true)
        {
#if DEBUG_IS_TEMPLATE_INSTANTIATION
          printf ("In isTemplateInstantiation(a_routine_ptr): If p->instance_required == true, then this must be an instantiation (reset buildTemplateInstantiation = true) \n");
#endif
          buildTemplateInstantiation = true;
        }

#if DEBUG_IS_TEMPLATE_INSTANTIATION
     printf ("In isTemplateInstantiation(a_routine_ptr): Before call to isDeclaredInAnInstantiation() buildTemplateInstantiation = %s \n",buildTemplateInstantiation ? "true" : "false");
#endif
#if 1
  // DQ (5/2/2012): the 2nd input has to be true for the test to be done....
  // DQ (4/30/2012): Added new criteria to consider as template instantiation (see test2012_58.C: friend function is template class).
  // if (isDeclaredInAnInstantiation(p->source_corresp,buildTemplateInstantiation) == true)
  // if (isDeclaredInAnInstantiation(p->source_corresp,true) == true)
     bool runTest = !buildTemplateInstantiation;
     if (isDeclaredInAnInstantiation(p->source_corresp,runTest) == true)
        {
       // It might be that we should be checking the befriending class as well, but not yet.
#if DEBUG_IS_TEMPLATE_INSTANTIATION
          printf ("In isTemplateInstantiation(a_routine_ptr): If p->source_corresp.parent_scope != NULL, then check for parent as an instantiation (reset buildTemplateInstantiation = true) \n");
#endif
          buildTemplateInstantiation = true;
        }
#endif

  // DQ (4/27/2012): Fixing test2012_68.C (testing)...
     if (buildTemplateInstantiation == false && p->defined_in_friend_decl == true)
        {
#if DEBUG_IS_TEMPLATE_INSTANTIATION
          printf ("In isTemplateInstantiation(a_routine_ptr): If p->defined_in_friend_decl == true, then this might be an instantiation (reset buildTemplateInstantiation = true) \n");
#endif

       // Check if this is part of a friend function defined in a class.
          SgTemplateInstantiationDefn* templateInstantiationDefn = isSgTemplateInstantiationDefn(topScopeStack());
          if (templateInstantiationDefn != NULL)
             {
#if DEBUG_IS_TEMPLATE_INSTANTIATION
               printf ("In isTemplateInstantiation(a_routine_ptr): isSgTemplateInstantiationDefn(topScopeStack()) != NULL \n");
#endif
               buildTemplateInstantiation = true;
             }
        }

  // DQ (8/27/2014): Added test to detect required instantiations.
     if (buildTemplateInstantiation == false && p->template_arg_list != NULL)
        {
#if DEBUG_IS_TEMPLATE_INSTANTIATION
          printf ("In isTemplateInstantiation(a_routine_ptr): If p->template_arg_list == true, then this must be an instantiation (reset buildTemplateInstantiation = true) \n");

       // Not clear if this is impotant criteria.
          printf ("   --- p->expl_template_arg_list_used = %s \n",p->expl_template_arg_list_used ? "true" : "false");
#endif

          buildTemplateInstantiation = true;
        }


#if 0
  // DQ (4/30/2012): Temporary code...
     if (p->befriending_classes != NULL)
        {
          printf ("In isTemplateInstantiation(a_routine_ptr): p->befriending_classes != NULL \n");
          buildTemplateInstantiation = true;
        }
#endif

#if 0
  // DQ (8/25/2014): Commented out to revert to previous working state.

  // DQ (8/25/2014): We need to make this as a template instantiation (see test2014_161.C).
  // Note that if this is a constructor call then the name and the template arguments will be
  // taken from the class template instantiation and not the constructor member function call.
     if ( (p->is_template_function == true) && (p->is_prototype_instantiation == true) )
        {
#if 0
          printf ("In isTemplateInstantiation(a_routine_ptr): If (p->is_template_function == true) && (p->is_prototype_instantiation == true) == true, then this must be an instantiation (reset buildTemplateInstantiation = true) \n");
#endif
          buildTemplateInstantiation = true;
        }
#endif

#if 0
     printf ("In isTemplateInstantiation(a_routine_ptr): Before call to isDeclaredInAnInstantiation() buildTemplateInstantiation = %s \n",buildTemplateInstantiation ? "true" : "false");
#endif

#if DEBUG_IS_TEMPLATE_INSTANTIATION || 0
     printf ("Leaving isTemplateInstantiation(a_routine_ptr): returning buildTemplateInstantiation = %s \n",buildTemplateInstantiation ? "true" : "false");
#endif

     return buildTemplateInstantiation;
   }


void errorCheckingOfTemplateOrTemplateInstantiationModes (const bool buildTemplateDeclaration, const bool buildTemplateInstantiation)
   {
  // Error checking.
     if (buildTemplateDeclaration == true)
        {
       // If this is a templated declaration, then it can't also be a template instantiation.
          ROSE_ASSERT(buildTemplateInstantiation == false);
        }
       else
        {
       // buildTemplateInstantiation can be either true or false in this case.
        }

  // Error checking.
     if (buildTemplateInstantiation == true)
        {
          ROSE_ASSERT(buildTemplateDeclaration == false);
        }
   }

std::string builtin_operation_function_name(a_builtin_operation_kind kind)
   {
     char* s = NULL;
     switch (kind)
        {
          case bok_offsetof:
               s = (char*)"__builtin_offsetof";
               break;
          case bok_has_assign:
               s = (char*)"__has_assign";
               break;
          case bok_has_copy:
               s = (char*)"__has_copy";
               break;
          case bok_has_nothrow_assign:
               s = (char*)"__has_nothrow_assign";
               break;
          case bok_has_nothrow_constructor:
               s = (char*)"__has_nothrow_constructor";
               break;
          case bok_has_nothrow_copy:
               s = (char*)"__has_nothrow_copy";
               break;
          case bok_has_trivial_assign:
               s = (char*)"__has_trivial_assign";
               break;
          case bok_has_trivial_constructor:
               s = (char*)"__has_trivial_constructor";
               break;
          case bok_has_trivial_copy:
               s = (char*)"__has_trivial_copy";
               break;
          case bok_has_trivial_destructor:
               s = (char*)"__has_trivial_destructor";
               break;
          case bok_has_user_destructor:
               s = (char*)"__has_user_destructor";
               break;
          case bok_has_virtual_destructor:
               s = (char*)"__has_virtual_destructor";
               break;
          case bok_is_abstract:
               s = (char*)"__is_abstract";
               break;
          case bok_is_base_of:
               s = (char*)"__is_base_of";
               break;
          case bok_is_class:
               s = (char*)"__is_class";
               break;
          case bok_is_convertible_to:
               s = (char*)"__is_convertible_to";
               break;
          case bok_is_empty:
               s = (char*)"__is_empty";
               break;
          case bok_is_enum:
               s = (char*)"__is_enum";
               break;
          case bok_is_pod:
               s = (char*)"__is_pod";
               break;
          case bok_is_polymorphic:
               s = (char*)"__is_polymorphic";
               break;
          case bok_is_union:
               s = (char*)"__is_union";
               break;
          case bok_types_compatible:
               s = (char*)"__builtin_types_compatible_p";
               break;
          case bok_intaddr:
               s = (char*)"__intaddr";
               break;
          case bok_is_trivial:
               s = (char*)"__is_trivial";
               break;
          case bok_is_standard_layout:
               s = (char*)"__is_standard_layout";
               break;
          case bok_is_trivially_copyable:
               s = (char*)"__is_trivially_copyable";
               break;
          case bok_is_literal_type:
               s = (char*)"__is_literal_type";
               break;
          case bok_has_trivial_move_constructor:
               s = (char*)"__has_trivial_move_constructor";
               break;
          case bok_has_trivial_move_assign:
               s = (char*)"__has_trivial_move_assign";
               break;
          case bok_has_nothrow_move_assign:
               s = (char*)"__has_nothrow_move_assign";
               break;
          case bok_is_constructible:
               s = (char*)"__is_constructible";
               break;
          case bok_is_nothrow_constructible:
               s = (char*)"__is_nothrow_constructible";
               break;
          case bok_has_finalizer:
               s = (char*)"__has_finalizer";
               break;
          case bok_is_delegate:
               s = (char*)"__is_delegate";
               break;
          case bok_is_interface_class:
               s = (char*)"__is_interface_class";
               break;
          case bok_is_ref_array:
               s = (char*)"__is_ref_array";
               break;
          case bok_is_ref_class:
               s = (char*)"__is_ref_class";
               break;
          case bok_is_sealed:
               s = (char*)"__is_sealed";
               break;
          case bok_is_simple_value_class:
               s = (char*)"__is_simple_value_class";
               break;
          case bok_is_final:
               s = (char*)"__is_final";
               break;
          case bok_is_trivially_constructible:
               s = (char*)"__is_trivially_constructible";
               break;
          case bok_is_destructible:
               s = (char*)"__is_destructible";
               break;
          case bok_is_nothrow_destructible:
               s = (char*)"__is_nothrow_destructible";
               break;
          case bok_is_trivially_destructible:
               s = (char*)"__is_trivially_destructible";
               break;
          case bok_is_nothrow_assignable:
               s = (char*)"__is_nothrow_assignable";
               break;
          case bok_is_trivially_assignable:
               s = (char*)"__is_trivially_assignable";
               break;
          case bok_builtin_shuffle:
               s = (char*)"__builtin_shuffle";
               break;
          case bok_builtin_complex:
               s = (char*)"__builtin_complex";
               break;
          case bok_is_valid_winrt_type:
               s = (char*)"__is_valid_winrt_type";
               break;
          case bok_builtin_shufflevector:
               s = (char*)"__builtin_shufflevector";
               break;
          case bok_builtin_convertvector:
               s = (char*)"__builtin_convertvector";
               break;
       // Microsoft
          case bok_is_assignable:
               s = (char*)"__is_assignable";
               break;
          case bok_is_trivially_copy_assignable:
               s = (char*)"__is_trivially_copy_assignable";
               break;
          case bok_builtin_addressof:
               s = (char*)"__builtin_addressof";
               break;
          case bok_has_unique_object_representations:
               s = (char*)"__has_unique_object_representations";
               break;
          case bok_is_aggregate:
               s = (char*)"__is_aggregate";
               break;
          case bok_is_win_class:
               s = (char*)"__is_win_class";
               break;
          case bok_is_win_interface:
               s = (char*)"__is_win_interface";
               break;
          case bok_reference_binds_to_temporary:
               s = (char*)"__reference_binds_to_temporary";
               break;
          case bok_is_same:
               s = (char*)"__is_same";
               break;
          case bok_is_same_as:
               s = (char*)"__is_same_as";
               break;
          case bok_is_function:
               s = (char*)"__is_function";
               break;
          case bok_last:
               s = (char*)"__last";
               ROSE_ABORT();
               break;
          default:
               printf ("Error: builtin_operation_function_name(): kind = %" PRIuPTR " \n",(size_t)kind);
               s = (char*)"**BAD operation kind**";
               ROSE_ABORT();
        }

     return s;
   }

bool isWideChar( a_type_ptr type)
   {
     bool isWideCharacter = false;
     if ( (type->kind == tk_integer) && (type->variant.integer.wchar_t_type == true) )
        {
          isWideCharacter = true;
        }
#if 0
     printf ("In isWideChar(): isWideCharacter = %s target_size = %" PRIuPTR " \n",isWideCharacter ? "true" : "false",(size_t) type->size);
#endif

     return isWideCharacter;
   }

bool is16BitWideChar( a_type_ptr type)
   {
     bool is16BitWideCharacter = false;
     size_t target_size = (unsigned long) type->size;

  // wchar_t can be either 16-bit or 32-bit to support unicode.
     if (target_size == 2)
        {
          is16BitWideCharacter = true;
        }

#if 0
     printf ("In is16BitWideChar(): is16BitWideCharacter = %s target_size = %" PRIuPTR " \n",is16BitWideCharacter ? "true" : "false",target_size);
#endif

     return is16BitWideCharacter;
   }

bool is32BitWideChar( a_type_ptr type)
   {
     bool is32BitWideCharacter = false;
     size_t target_size = (unsigned long) type->size;

  // wchar_t can be either 16-bit or 32-bit to support unicode.
     if (target_size == 4)
        {
          is32BitWideCharacter = true;
        }

#if 0
     printf ("In is32BitWideChar(): is32BitWideCharacter = %s target_size = %" PRIuPTR " \n",is32BitWideCharacter ? "true" : "false",target_size);
#endif

     return is32BitWideCharacter;
   }

#define WARNING_BUILD_SPECULATIVE_FUNCTION_REF_EXP 0
#define DEBUG_BUILD_SPECULATIVE_FUNCTION_REF_EXP 0

SgExpression* buildSpeculativeFunctionRefExp( a_constant_ptr e )
   {
     ROSE_ASSERT(e != NULL);

     SgName name                               = e->source_corresp.name;
     bool isMemberFunction                     = e->source_corresp.is_class_member;

     SgExpression* result = NULL;

#if DEBUG_BUILD_SPECULATIVE_FUNCTION_REF_EXP
     printf ("In buildSpeculativeFunctionRefExp(): case of tpck_unknown_function: this is a call to a template function: name        = %s \n",e->source_corresp.name);
  // printf ("  --- case of tpck_unknown_function: this is a call to a template function: hasAnAssociatdPrototypeInstantiation       = %s \n",hasAnAssociatdPrototypeInstantiation ? "true" : "false");
  // printf ("  --- case of tpck_unknown_function: this is a call to a template function: isGlobalQualifiedName                      = %s \n",isGlobalQualifiedName ? "true" : "false");
  // printf ("  --- case of tpck_unknown_function: this is a call to a template function: isQualifiedName                            = %s \n",isQualifiedName ? "true" : "false");
     SgScopeStatement * top_scope_stack = topScopeStack();
     printf ("  --- case of tpck_unknown_function: this is a call to a template function: current scope                         = %p = %s \n",top_scope_stack,top_scope_stack ? top_scope_stack->class_name().c_str() : "");
     printf ("  --- case of tpck_unknown_function: this is a call to a template function: isMemberFunction                           = %s \n",isMemberFunction ? "true" : "false");
#endif

  // DQ (2/25/2013): This is false for a case tpck_template_ref
  // ROSE_ASSERT(hasAnAssociatdPrototypeInstantiation == true);

  // DQ (2/25/2013): I think we want to generate a function reference expression independent of the value of hasAnAssociatdPrototypeInstantiation.
  // DQ (2/23/2013): We need to build a SgTemplateFunctionRefExp since this case only happens for calls to template functions.
  // if (hasAnAssociatdPrototypeInstantiation == true)
        {
       // Find the SgTemplateSymbol associated with this function.

#if DEBUG_BUILD_SPECULATIVE_FUNCTION_REF_EXP
          printf ("In buildSpeculativeFunctionRefExp(): Try rose_scope = getDeclarationScope() \n");
#endif

          SgScopeStatement* rose_scope = getDeclarationScope(e->source_corresp, true);

#if DEBUG_BUILD_SPECULATIVE_FUNCTION_REF_EXP
          printf (" --- rose_scope (cache) = %p = %s \n", rose_scope, rose_scope ? rose_scope->class_name().c_str() : "");
          printf (" --- isMemberFunction = %s \n",isMemberFunction ? "true" : "false");
#endif

          if ( rose_scope == NULL || ( isMemberFunction && ( isSgGlobal(rose_scope) || isSgNamespaceDefinitionStatement(rose_scope) ) ) )
             {
#if DEBUG_BUILD_SPECULATIVE_FUNCTION_REF_EXP
               printf ("In buildSpeculativeFunctionRefExp(): Fall back on rose_scope = topScopeStack() and searching the parent scopes (it might be a problem) \n");
#endif
               rose_scope = topScopeStack();

#if 0
               printf (" --- After using topScopeStack(): rose_scope = %p = %s \n", rose_scope, rose_scope ? rose_scope->class_name().c_str() : "");
               if (isSgGlobal(rose_scope) != NULL)
                  {
                    printf ("In buildSpeculativeFunctionRefExp(): case of tpck_unknown_function: Not a known function: Calling summarize_constant() \n");
                    summarize_constant(e);
                  }
#endif
            // DQ (9/4/2020): This should not be global scope.
               ROSE_ASSERT(isSgGlobal(rose_scope) == NULL);

            // TV (04/10/2018): Get to a class definition statement (or global scope):
               while (rose_scope != NULL) 
                  {
#if 0
                    printf ("TOP of while loop: rose_scope = %p = %s \n", rose_scope, rose_scope ? rose_scope->class_name().c_str() : "");
#endif
                    if (isSgGlobal(rose_scope) || isSgNamespaceDefinitionStatement(rose_scope) || isSgClassDefinition(rose_scope) )
                         break;
                    rose_scope = rose_scope->get_scope();
#if 0
                    printf ("BOTTOM of while loop: rose_scope = %p = %s \n", rose_scope, rose_scope ? rose_scope->class_name().c_str() : "");
#endif
                  }
             }
#if DEBUG_BUILD_SPECULATIVE_FUNCTION_REF_EXP
          printf ("  --- rose_scope (final) = %p = %s \n", rose_scope, rose_scope ? rose_scope->class_name().c_str() : "");
#endif
          ROSE_ASSERT( rose_scope != NULL );

#if 0
          if (isSgGlobal(rose_scope) != NULL)
             {
               printf ("In buildSpeculativeFunctionRefExp(): case of tpck_unknown_function: Not a known function: Calling summarize_constant() \n");
               summarize_constant(e);
             }
#endif
       // DQ (9/4/2020): This should not be global scope.
       // ROSE_ASSERT(isSgGlobal(rose_scope) == NULL);

          SgFunctionSymbol* functionSymbol = lookupFunctionSymbolInParentScopes(name,rose_scope);

          if (isMemberFunction && functionSymbol != NULL && isSgMemberFunctionSymbol(functionSymbol) == NULL) 
             {
#if WARNING_BUILD_SPECULATIVE_FUNCTION_REF_EXP
               printf("WARNING: In buildSpeculativeFunctionRefExp(): Found a normal function symbol instead of the expected a member function symbol (set to NULL).\n");
#endif
              functionSymbol = NULL;
             }

          if (functionSymbol != NULL)
             {
#if DEBUG_BUILD_SPECULATIVE_FUNCTION_REF_EXP
               printf ("--- case of tpck_unknown_function: functionSymbol = %p = %s \n",functionSymbol,functionSymbol->class_name().c_str());
#endif
               SgMemberFunctionSymbol* memberFunctionSymbol = isSgMemberFunctionSymbol(functionSymbol);
               if (memberFunctionSymbol != NULL)
                  {
                    SgTemplateMemberFunctionSymbol* templateMemberFunctionSymbol = isSgTemplateMemberFunctionSymbol(functionSymbol);
#if DEBUG_BUILD_SPECULATIVE_FUNCTION_REF_EXP
                    printf ("--- case of tpck_unknown_function: templateMemberFunctionSymbol = %p \n",templateMemberFunctionSymbol);
#endif
                    bool virtual_call = false;
                    bool need_qualifier = false;
                    if (templateMemberFunctionSymbol != NULL)
                       {
                         SgTemplateMemberFunctionRefExp* templateMemberFunctionRefExp = buildTemplateMemberFunctionRefExp_nfi(templateMemberFunctionSymbol,virtual_call,need_qualifier);
                         ROSE_ASSERT(templateMemberFunctionRefExp != NULL);

                         ROSE_ASSERT(templateMemberFunctionRefExp->get_symbol() != NULL);
#if DEBUG_BUILD_SPECULATIVE_FUNCTION_REF_EXP
                         printf ("In buildSpeculativeFunctionRefExp(): TEST 1: templateMemberFunctionRefExp = %p = %s \n",templateMemberFunctionRefExp,templateMemberFunctionRefExp->class_name().c_str());
#endif
                         result = templateMemberFunctionRefExp;
                       }
                      else
                       {
                         result = buildMemberFunctionRefExp_nfi(memberFunctionSymbol,virtual_call,need_qualifier);
                         ROSE_ASSERT(result != NULL);
                       }
                  }
                 else
                  {
                    SgTemplateFunctionSymbol* templateFunctionSymbol = isSgTemplateFunctionSymbol(functionSymbol);
                    if (templateFunctionSymbol != NULL)
                       {
                      // result = buildTemplateFunctionRefExp_nfi(templateFunctionSymbol);
                         SgTemplateFunctionRefExp* templateFunctionRefExp = buildTemplateFunctionRefExp_nfi(templateFunctionSymbol);
                         ROSE_ASSERT(templateFunctionRefExp != NULL);

                         ROSE_ASSERT(templateFunctionRefExp->get_symbol() != NULL);

                         result = templateFunctionRefExp;
                       }
                      else
                       {
#if DEBUG_BUILD_SPECULATIVE_FUNCTION_REF_EXP
                         printf ("In buildSpeculativeFunctionRefExp(): case of tpck_unknown_function: Not a template function: Calling summarize_constant() \n");
                         summarize_constant(e);
#endif
#ifdef DEBUG_CONVERSION
                         printf ("ERROR: case tpck_unknown_function: This is not a template function, building a SgFunctionRefExp \n");
#endif
                         ROSE_ASSERT(functionSymbol != NULL);
                         result = buildFunctionRefExp_nfi(functionSymbol);
                       }
                  }
             }
            else
             {
            // In this case the function replace() has not been processed yet in the EDG/ROSE translation, so it is unavailabe in the ROSE symbol tables.
            // Usually what we do is build a prototype for the function so that have a valid symbol to reference.  But we can't do this for an 
            // unknown function since we have no reference to the edg routine that would be required for this.  We could define something in the 
            // symbol table for temporary use, but it would have to be fixed up later (astPostProcessing phase).
#if 0
               printf ("In buildSpeculativeFunctionRefExp(): case of tpck_unknown_function: functionSymbol == NULL: name = %s \n",e->source_corresp.name);
            // printf ("  --- case of tpck_unknown_function: functionSymbol == NULL: hasAnAssociatdPrototypeInstantiation     = %s \n",hasAnAssociatdPrototypeInstantiation ? "true" : "false");
            // printf ("  --- case of tpck_unknown_function: functionSymbol == NULL: isGlobalQualifiedName                    = %s \n",isGlobalQualifiedName ? "true" : "false");
            // printf ("  --- case of tpck_unknown_function: functionSymbol == NULL: isQualifiedName                          = %s \n",isQualifiedName ? "true" : "false");
               printf ("  --- case of tpck_unknown_function: functionSymbol == NULL: current scope                       = %p = %s \n",topScopeStack(),topScopeStack()->class_name().c_str());
               printf ("  --- case of tpck_unknown_function: functionSymbol == NULL: edg_scope                                = %p \n",edg_scope);
#endif
#if 0
               printf ("In buildSpeculativeFunctionRefExp(): case of tpck_unknown_function: Not a known function: Calling summarize_constant() \n");
               summarize_constant(e);
#endif
#ifdef DEBUG_CONVERSION
               printf ("ERROR: case tpck_unknown_function: functionSymbol == NULL\n");
#endif

#if 0
            // I think that e->source_corresp.assoc_info is supposed to be a a_symbol_ptr, but the kind does not make sense (invalid value).
               a_symbol_ptr edg_symbol = symbol_for(e);
            // ROSE_ASSERT(edg_symbol != NULL);
               if (edg_symbol != NULL)
                  {
                    printf ("In buildSpeculativeFunctionRefExp(): case of tpck_unknown_function: Not a known function: Calling summarize_symbol() \n");
                    summarize_symbol(edg_symbol);
                  }
#endif
            // ROSE_ASSERT(edg_scope != NULL);

            // DQ (2/25/2013): If this is a NULL name then we can't really build such a function.
               ROSE_ASSERT(name.is_null() == false);

            // DQ (2/24/2013): This is not always true.
            // ROSE_ASSERT(isGlobalQualifiedName == true || isQualifiedName == true);

            // Calling: SgTemplateMemberFunctionDeclaration* buildNondefiningTemplateMemberFunctionDeclaration (
            //      const SgName & name, SgType* return_type, SgFunctionParameterList *parlist, SgScopeStatement* scope = NULL, 
            //      SgExprListExp* decoratorList = NULL, unsigned int functionConstVolatileFlags = 0);
            // Building a function with void return type and no parameters so that we can build a function reference 
            // expression for an unknown function that at least has the correct name (and it from the correct scope).
               SgType* return_type                     = buildVoidType();
               SgFunctionParameterList *parlist        = buildFunctionParameterList_nfi();
               SgExprListExp* pythonDecoratorList      = NULL;
               unsigned int functionConstVolatileFlags = 0;
#if 0
               printf ("In buildSpeculativeFunctionRefExp(): parlist = %p \n",parlist);
#endif
            // ROSE_ASSERT(isMemberFunction == true);

            // We need to define this here so that can use it in putFunctionInTemplateMap() after it is initialized in either of the two branches below.
               SgFunctionDeclaration* nondefiningFunctionDeclaration = NULL;

               if (isMemberFunction == true)
                  {
                 // It is unclear if this is a template member function or a non-template member function.

#ifdef DEBUG_CONVERSION
                    if (isSgTemplateInstantiationDefn(rose_scope) != NULL)
                       {
                         printf ("ERROR: isSgTemplateInstantiationDefn(rose_scope) != NULL: This should be built as an instantiation of a template member function instead of a template member function \n");
                       }
#endif
                 // DQ (8/12/2013): We need to support the SgTemplateParameterPtrList as a parameter to the buildNondefiningTemplateMemberFunctionDeclaration() function.
                 // SgFunctionDeclaration* nondefiningFunctionDeclaration = buildNondefiningTemplateMemberFunctionDeclaration(name,return_type,parlist,rose_scope,pythonDecoratorList,functionConstVolatileFlags);
                    SgTemplateParameterPtrList templateParameterList;

                 // DQ (8/7/2013): Make this an error so that we can come back and fix this.
                    if (templateParameterList.empty() == true)
                       {
#if 0
                         printf ("In buildSpeculativeFunctionRefExp(): case of tpck_unknown_function: incomplete support for overloaded member function templates: name = %s \n",name.str());
                         summarize_constant(e);
#endif
#ifdef DEBUG_CONVERSION
                         printf ("WARNING: In buildSpeculativeFunctionRefExp(): templateParameterList.empty() == true: incomplete support for overloaded member function templates: name = %s \n",name.str());
#endif
                       }
                 // ROSE_ASSERT(templateParameterList.empty() == false);

                 // DQ (8/15/2013): Modified to use nondefiningFunctionDeclaration defined in the outer scope.
#if 0
                    printf ("Calling buildNondefiningTemplateMemberFunctionDeclaration(): \n");
                    printf (" --- name                                       = %s \n",name.str());
                    printf (" --- return_type = %p = %s \n",return_type,return_type->class_name().c_str());
                    printf (" --- parlist     = %p = %s \n",parlist,parlist->class_name().c_str());
                    printf (" --- rose_scope  = %p = %s \n",rose_scope,rose_scope->class_name().c_str());
#endif

                 // DQ (9/4/2020): This should not be global scope (because member functions are not defined in global scope).
                 // This is related to the fix for Cxx11_tests/test2020_91.C.
                    ROSE_ASSERT(isSgGlobal(rose_scope) == NULL);

                    nondefiningFunctionDeclaration = buildNondefiningTemplateMemberFunctionDeclaration(name,return_type,parlist,rose_scope,pythonDecoratorList,functionConstVolatileFlags,&templateParameterList);
                    ROSE_ASSERT(nondefiningFunctionDeclaration != NULL);
#if 0
                    printf ("DONE: Calling buildNondefiningTemplateMemberFunctionDeclaration(): \n");
#endif
#if 0
                 // DQ (6/28/2013): Set if template arguments should be explicit for function reference expressions.
                    ROSE_ASSERT(isSgTemplateMemberFunctionDeclaration(nondefiningFunctionDeclaration) != NULL);
                    isSgTemplateMemberFunctionDeclaration(nondefiningFunctionDeclaration)->set_template_argument_list_is_explicit(p->expl_template_arg_list_used);
#endif
#if 0
                    printf ("WARNING: need to mark this nondefiningFunctionDeclaration = %p = %s as being generated from an unknown template function within EDG \n",nondefiningFunctionDeclaration,name.str());
#endif
                 // DQ (8/13/2013): The symbol lookup now has to follow newer (more strict) rules.
                 // DQ (2/25/2013): I think we can assert this, unless the buildNondefiningTemplateMemberFunctionDeclaration() has added the symbol, in which case it is a mistake to add one below.
                    ROSE_ASSERT(nondefiningFunctionDeclaration->get_type() != NULL);
                    functionSymbol = rose_scope->lookup_template_member_function_symbol(name,nondefiningFunctionDeclaration->get_type(),&templateParameterList);
                    ROSE_ASSERT(functionSymbol != NULL);

                    SgTemplateMemberFunctionSymbol* templateMemberFunctionSymbol = isSgTemplateMemberFunctionSymbol(functionSymbol);
                    ROSE_ASSERT(templateMemberFunctionSymbol != NULL);

                    bool virtual_call = false;
                    bool need_qualifier = false;
                    SgTemplateMemberFunctionRefExp* templateMemberFunctionRefExp = buildTemplateMemberFunctionRefExp_nfi(templateMemberFunctionSymbol,virtual_call,need_qualifier);
                    ROSE_ASSERT(templateMemberFunctionRefExp != NULL);
#if 0
                    printf ("In buildSpeculativeFunctionRefExp(): TEST 2: templateMemberFunctionRefExp = %p \n",templateMemberFunctionRefExp);
#endif
                    result = templateMemberFunctionRefExp;
                  }
                 else
                  {
                 // It is unclear if this is a template non-member function or a non-template non-member function.
#if 0
                    printf ("WARNING: In buildSpeculativeFunctionRefExp(): building EMPTY templateArgumentsList: incomplete support for overloaded templates \n");
#endif
                    SgTemplateParameterPtrList templateParameterList;

#ifdef DEBUG_CONVERSION
                 // DQ (8/7/2013): Make this an error so that we can come back and fix this.
                    if (templateParameterList.empty() == true)
                       {
                         printf ("WARNING: In buildSpeculativeFunctionRefExp(): templateParameterList.empty() == true: incomplete support for overloaded non-member templates: name = %s \n",name.str());
                       }
#endif
                 // DQ (8/15/2013): Modified to use nondefiningFunctionDeclaration defined in the outer scope.
                 // DQ (8/7/2013): Added support for template functions overloaded on template parameters (see test2013_303.C).
                    nondefiningFunctionDeclaration = buildNondefiningTemplateFunctionDeclaration(name,return_type,parlist,rose_scope,pythonDecoratorList,&templateParameterList);
                    ROSE_ASSERT(nondefiningFunctionDeclaration != NULL);
#if 0
                 // DQ (6/28/2013): Set if template arguments should be explicit for function reference expressions.
                    ROSE_ASSERT(isSgTemplateFunctionDeclaration(nondefiningFunctionDeclaration) != NULL);
                    isSgTemplateFunctionDeclaration(nondefiningFunctionDeclaration)->set_template_argument_list_is_explicit(p->expl_template_arg_list_used);
#endif
#if 0
                    printf ("WARNING: need to mark this nondefiningFunctionDeclaration = %p = %s as being generated from an unknown template function within EDG \n",nondefiningFunctionDeclaration,name.str());
#endif
#if 0
                    printf ("Calling rose_scope->lookup_template_function_symbol(): \n");
                    printf (" --- name                                       = %s \n",name.str());
                    printf (" --- nondefiningFunctionDeclaration->get_type() = %p = %s \n",nondefiningFunctionDeclaration->get_type(),nondefiningFunctionDeclaration->get_type()->class_name().c_str());
#endif
                 // DQ (8/13/2013): The symbol lookup now has to follow newer (more strict) rules.
                 // DQ (2/25/2013): I think we can assert this, unless the buildNondefiningTemplateMemberFunctionDeclaration() has added the symbol, in which case it is a mistake to add one below.
                    ROSE_ASSERT(nondefiningFunctionDeclaration->get_type() != NULL);
                    functionSymbol = rose_scope->lookup_template_function_symbol(name,nondefiningFunctionDeclaration->get_type(),&templateParameterList);

                    ROSE_ASSERT(functionSymbol != NULL);
                    SgTemplateFunctionSymbol* templateFunctionSymbol = isSgTemplateFunctionSymbol(functionSymbol);
                    ROSE_ASSERT(templateFunctionSymbol != NULL);
#if 0
                    printf ("Calling buildTemplateFunctionRefExp_nfi(): templateFunctionSymbol = %p \n",templateFunctionSymbol);
#endif
                    SgTemplateFunctionRefExp* templateFunctionRefExp = buildTemplateFunctionRefExp_nfi(templateFunctionSymbol);
                    ROSE_ASSERT(templateFunctionRefExp != NULL);
#if 0
                    printf ("DONE: Calling buildTemplateFunctionRefExp_nfi(): templateFunctionSymbol = %p \n",templateFunctionSymbol);
#endif
                    result = templateFunctionRefExp;
                  }

#if 0
               printf ("In buildSpeculativeFunctionRefExp(): Need to put new template function or member function into the cache: (not clear what key to use): Calling summarize_constant() \n");
               summarize_constant(e);
#endif
#if 0
               printf ("@@@@@@@@@@@@@@@@@ Not clear how to add new buildNondefiningTemplateMemberFunctionDeclaration to the first_nondefining_declarations_for_templates cache (PART 2) name = %s result = %p = %s \n",name.str(),result,result->class_name().c_str());
#endif
            // DQ (8/15/2013): This needs to be called after building template function and member function declaations.
               ROSE_ASSERT(nondefiningFunctionDeclaration != NULL);
             }
        }

     ROSE_ASSERT(result != NULL);

#if DEBUG_BUILD_SPECULATIVE_FUNCTION_REF_EXP
     printf ("Leaving buildSpeculativeFunctionRefExp(): result = %p = %s \n",result,result->class_name().c_str());
#endif

     if (isMemberFunction == true)
        {
       // ROSE_ASSERT(isSgTemplateMemberFunctionRefExp(result) != NULL);
          ROSE_ASSERT(isSgMemberFunctionRefExp(result) != NULL || isSgTemplateMemberFunctionRefExp(result) != NULL || isSgNonrealRefExp(result) != NULL);
        }
       else
        {
       // ROSE_ASSERT(isSgTemplateFunctionRefExp(result) != NULL);
#ifdef DEBUG_CONVERSION
          if (isSgFunctionRefExp(result) == NULL && isSgTemplateFunctionRefExp(result) == NULL)
             {
            // DQ (2/25/2013): This case happends for test2011_121.C, I think that problem is that it should maybe a constructor and is not marked as a member function.
               printf ("WARNING: buildSpeculativeFunctionRefExp(): should be a template member function reference: result = %p = %s \n",result,result->class_name().c_str());
             }
#endif
       // ROSE_ASSERT(isSgFunctionRefExp(result) != NULL || isSgTemplateFunctionRefExp(result) != NULL);
        }

     return result;
   }

#define DEBUG_UPDATE_SCOPE_CACHE 0

void update_scope_cache(std::map<a_scope_ptr, SgScopeStatement*> & local_scope_cache, a_scope_ptr assoc_scope, SgClassDefinition* classDefinition)
   {
  // DQ (8/26/2014): Added support to update the scope cashe using different scope caches.
  //    1) for template declarations.
  //    2) for template instantations and non-template-instantiaton scopes.
  // This is required because EDG uses the scale scope for both the template declarations 
  // and the template instantiations.

     if (local_scope_cache.find(assoc_scope) != local_scope_cache.end())
        {
#if DEBUG_UPDATE_SCOPE_CACHE
          printf ("In update_scope_cache(): Note assoc_scope = %p is already in scope_cache: scope_cache[assoc_scope] = %p (don't reset to classDefinition = %p) \n",assoc_scope,scope_cache[assoc_scope],classDefinition);
#endif
          if (local_scope_cache[assoc_scope] != NULL)
             {
            // DQ (3/12/2012): I don't like this, so I want to know when this happens, but we will allow it for now.
#ifdef DEBUG_CONVERSION
               printf ("WARNING: scope_cache[assoc_scope] != NULL and will be reset \n");
#endif
               if (local_scope_cache[assoc_scope] != classDefinition)
                  {
#ifdef DEBUG_CONVERSION
                    printf ("*** WARNING: scope_cache[assoc_scope] != NULL and will be reset to a DIFFERENT value! \n");
#endif
#if 0
                    SgScopeStatement* scope_from_cache = local_scope_cache[assoc_scope];
                    printf ("WARNING: In update_scope_cache(): scope_cache[assoc_scope = %p] = %p = %s and is not reset with classDefinition = %p = %s \n",
                         assoc_scope,scope_from_cache,scope_from_cache->class_name().c_str(),classDefinition,classDefinition->class_name().c_str());
#endif
                  }
             }
       // ROSE_ASSERT(scope_cache[assoc_scope] == NULL);

       // DQ (4/22/2012): We should not be resetting the value and it should not be NULL.
       // scope_cache[assoc_scope] = classDefinition;
          ROSE_ASSERT(local_scope_cache[assoc_scope] != NULL);
          if (local_scope_cache[assoc_scope] == NULL)
             {
#if 0
               printf ("Resetting NULL value in scope_cache[assoc_scope = %p] to classDefinition = %p = %s \n",assoc_scope,classDefinition,classDefinition->class_name().c_str());
#endif
               local_scope_cache[assoc_scope] = classDefinition;
             }
        }
       else
        {
#if DEBUG_UPDATE_SCOPE_CACHE
          printf ("In update_scope_cache(): Note assoc_scope = %p not yet defined in scope_cache: will be set to classDefinition = %p = %s \n",assoc_scope,classDefinition,classDefinition->class_name().c_str());
#endif
          local_scope_cache[assoc_scope] = classDefinition;
        }
   }


bool matching_defining_and_nondefining_variants(SgDeclarationStatement* decl)
   {
     ROSE_ASSERT(decl != NULL);

  // DQ (3/3/2012): Added test to make sure that the defining and non defining declarations variants are the same.
  // Note that this could be refactored (used in next fnction: convert_struct_primary())
     VariantT IR_node_variant = decl->variantT();

  // ROSE_ASSERT(decl->get_definingDeclaration() != NULL);
     if (decl->get_definingDeclaration() != NULL)
        {
       // ROSE_ASSERT(IR_node_variant == decl->get_definingDeclaration()->variantT() != NULL);
          if (IR_node_variant != decl->get_definingDeclaration()->variantT())
             {
               printf ("ERROR: In matching_defining_and_nondefining_variants(): non-matching decl = %p = %s and defining declaration = %p = %s \n",
                    decl,decl->class_name().c_str(),decl->get_definingDeclaration(),decl->get_definingDeclaration()->class_name().c_str());
             }
          ROSE_ASSERT(IR_node_variant == decl->get_definingDeclaration()->variantT());
        }

  // Note that this could be refactored (used in next fnction: convert_struct_primary())
     ROSE_ASSERT(decl->get_firstNondefiningDeclaration() != NULL);
     if (decl->get_firstNondefiningDeclaration() != NULL)
        {
       // ROSE_ASSERT(IR_node_variant == decl->get_firstNondefiningDeclaration()->variantT() != NULL);
          if (IR_node_variant != decl->get_firstNondefiningDeclaration()->variantT())
             {
               printf ("In matching_defining_and_nondefining_variants(): decl = %p = %s decl->get_firstNondefiningDeclaration() = %p = %s \n",
                    decl,decl->class_name().c_str(),decl->get_firstNondefiningDeclaration(),decl->get_firstNondefiningDeclaration()->class_name().c_str());
             }
          ROSE_ASSERT(IR_node_variant == decl->get_firstNondefiningDeclaration()->variantT());
        }

     return true;
   }

SgSymbol* retrieveAssociatedClassSymbol (std::string XXX_name, SgScopeStatement* scopeOfDeclaration, bool buildTemplateDeclaration, SgTemplateArgumentPtrList* templateArgumentsList, 
                                                     SgTemplateParameterPtrList* templateParameterList, SgTemplateArgumentPtrList* templateSpecializationArgumentList)
   {
  // DQ (12/26/2011): Refactoring of code to support the retrival of the appropriate symbol representing a class in a specified scope.

  // Note that the input name should not have any template arguments (this function will add them depending on if this is a template or non-template class.

#define DEBUG_RETRIEVE_ASSOCIATED_CLASS_SYMBOL 0

#if DEBUG_RETRIEVE_ASSOCIATED_CLASS_SYMBOL
      printf ("In retrieveAssociatedClassSymbol(): input name = %s buildTemplateDeclaration = %s scopeOfDeclaration = %p = %s \n",XXX_name.c_str(),buildTemplateDeclaration ? "true" : "false",scopeOfDeclaration,scopeOfDeclaration->class_name().c_str());
#endif

#if 1
  // DQ (5/10/2016): Debugging case of C++11 using the Intel v16 compiler and it's associated mutex header file.
  // See Cxx11_tests/test2016_32.C for an example that demonstrates this problem.
     if (XXX_name == "<unnamed>")
        {
          printf ("In retrieveAssociatedClassSymbol(): Identified case of XXX_name == <unnamed> \n");

          XXX_name = "rose_edg_unnamed";
#if 0
          printf ("Exiting as a test! \n");
          ROSE_ASSERT(false);
#endif
        }

     bool test_passes = hasTemplateSyntax(XXX_name);
     if (test_passes == true)
        {
          printf ("Error: failing test: hasTemplateSyntax(XXX_name = %s) == true \n",XXX_name.c_str());

          scopeOfDeclaration->get_file_info()->display("failing test: hasTemplateSyntax(XXX_name): debug");

          printf ("Exiting after error! \n");
          ROSE_ASSERT(false);
        }
#else
     ROSE_ASSERT(hasTemplateSyntax(XXX_name) == false);
#endif

     SgName nameWithOutTemplateArguments               = XXX_name;
     SgName nameWithOutTemplateSpecializationArguments = XXX_name;

     string name_used_to_get_symbol = "";

     SgSymbol* mysymbol = NULL;
     if (buildTemplateDeclaration == true)
        {
#if DEBUG_RETRIEVE_ASSOCIATED_CLASS_SYMBOL
          printf ("In retrieveAssociatedClassSymbol(): Looking in the symbol table for a SgTemplateSymbol scopeOfDeclaration = %p = %s \n",scopeOfDeclaration,scopeOfDeclaration->class_name().c_str());
#endif

       // DQ (9/4/2012): Added assertion (this fails for ROSE compiling ROSE, and test2004_77.C).
       // ROSE_ASSERT(templateArgumentsList == NULL);

       // DQ (9/10/2012): We need to assert this.
          ROSE_ASSERT(templateParameterList != NULL);

       // DQ (9/13/2012): We need to assert this.
          ROSE_ASSERT(templateSpecializationArgumentList != NULL);


       // DQ (9/13/2012): Set the name used for some references to templates (e.g. calls to get the symbol).
          SgName nameWithTemplateSpecializationArguments = appendTemplateArgumentsToName(nameWithOutTemplateSpecializationArguments,*templateSpecializationArgumentList);
#if DEBUG_RETRIEVE_ASSOCIATED_CLASS_SYMBOL
          printf ("In retrieveAssociatedClassSymbol(): Calling lookup_template_class_symbol() using nameWithTemplateSpecializationArguments = %s \n",nameWithTemplateSpecializationArguments.str());
#endif
       // DQ (9/20/2012): We use the name WITH the template specalization; so set this properly so that correct debugging message is generated.
       // name_used_to_get_symbol = nameWithOutTemplateSpecializationArguments;
          name_used_to_get_symbol = nameWithTemplateSpecializationArguments;

       // mysymbol = scopeOfDeclaration->lookup_template_symbol(name);
       // mysymbol = scopeOfDeclaration->lookup_template_class_symbol(name);
       // mysymbol = scopeOfDeclaration->lookup_template_class_symbol(name,templateParameterList,templateSpecializationArgumentList);
          mysymbol = scopeOfDeclaration->lookup_template_class_symbol(nameWithTemplateSpecializationArguments,templateParameterList,templateSpecializationArgumentList);
        }
       else
        {
#if DEBUG_RETRIEVE_ASSOCIATED_CLASS_SYMBOL
          printf ("In retrieveAssociatedClassSymbol(): Looking in the symbol table for a SgClassSymbol scopeOfDeclaration = %p = %s \n",scopeOfDeclaration,scopeOfDeclaration->class_name().c_str());
#endif

       // DQ (9/4/2012): Added assertion.
          ROSE_ASSERT(templateArgumentsList != NULL);

       // DQ (9/13/2012): Generate the name we need to use to access the symbol tabel (for non-template; could be normal class or template instantiation).
          SgName nameWithTemplateArguments = nameWithOutTemplateArguments;
          if (templateArgumentsList->empty() == false)
             {
               nameWithTemplateArguments = appendTemplateArgumentsToName(nameWithOutTemplateArguments,*templateArgumentsList);
             }
#if DEBUG_RETRIEVE_ASSOCIATED_CLASS_SYMBOL
          printf ("In retrieveAssociatedClassSymbol(): Calling lookup_nontemplate_class_symbol() using nameWithTemplateArguments    = %s \n",nameWithTemplateArguments.str());
          printf ("In retrieveAssociatedClassSymbol(): Calling lookup_nontemplate_class_symbol() using templateArgumentsList->size() = %" PRIuPTR " \n",templateArgumentsList->size());
#endif
          name_used_to_get_symbol = nameWithTemplateArguments;

       // mysymbol = scopeOfDeclaration->lookup_class_symbol(name);
       // mysymbol = scopeOfDeclaration->lookup_nontemplate_class_symbol(name);
       // mysymbol = scopeOfDeclaration->lookup_nontemplate_class_symbol(name,templateArgumentsList);
          mysymbol = scopeOfDeclaration->lookup_nontemplate_class_symbol(nameWithTemplateArguments,templateArgumentsList);

       // DQ (12/26/2011): This is either a null pointer, or a pointer to a SgClassSymbol not associated with a template class declaration.
#if DEBUG_RETRIEVE_ASSOCIATED_CLASS_SYMBOL
          printf ("In retrieveAssociatedClassSymbol(): mysymbol = %p \n",mysymbol);
          if (mysymbol != NULL)
             {
               printf ("In retrieveAssociatedClassSymbol(): mysymbol = %p = %s \n",mysymbol,mysymbol->class_name().c_str());
               printf ("In retrieveAssociatedClassSymbol(): isSgClassSymbol(mysymbol)->get_declaration() = %p = %s \n",isSgClassSymbol(mysymbol)->get_declaration(),isSgClassSymbol(mysymbol)->get_declaration()->class_name().c_str());
             }
#endif

       // DQ (8/12/2013): I think that the precision of the symbol lookup is such that symbols are now found 
       // where before they were not found.  So it is OK to now have found the correct symbol and one associated 
       // with a valid template class declaration.
          if (mysymbol != NULL && isSgTemplateClassDeclaration(isSgClassSymbol(mysymbol)->get_declaration()) != NULL)
             {
               printf ("ERROR: This might be an error because there is a template class declaration associated with the non-template class symbol \n");
             }
          ROSE_ASSERT(mysymbol == NULL || isSgTemplateClassDeclaration(isSgClassSymbol(mysymbol)->get_declaration()) == NULL);
        }

#if DEBUG_RETRIEVE_ASSOCIATED_CLASS_SYMBOL
     printf ("In retrieveAssociatedClassSymbol(): name_used_to_get_symbol = %s mysymbol = %p \n",name_used_to_get_symbol.c_str(),mysymbol);
#endif

#ifdef DEBUG_CONVERSION
     if (mysymbol == NULL)
        {
          printf ("In retrieveAssociatedClassSymbol(): mysymbol == NULL: name_used_to_get_symbol = %s \n",name_used_to_get_symbol.c_str());
        }
#endif

     return mysymbol;
   }

void outputDefiningNonDefiningDeclarations(SgDeclarationStatement* first_nondefining_declaration, SgDeclarationStatement* defining_declaration, SgDeclarationStatement* secondary_nondefining_declaration, const string & label)
   {
  // We must have a valid first_nondefining_declaration (by definition).
     ROSE_ASSERT(first_nondefining_declaration != NULL);

#if OUTPUT_DEFINING_AND_NONDEFINING_DECLARATIONS
  // DQ (9/16/2012): Only output this level of detail in the internal debugging mode.

     printf ("outputDefiningNonDefiningDeclarations(label = %s) \n",label.c_str());
     SgType* first_nondefining_declaration_type = getType(first_nondefining_declaration);
     printf ("--- first_nondefining_declaration     = %p first_nondefining_declaration->get_type()     = %p = %s \n",
          first_nondefining_declaration,first_nondefining_declaration_type,first_nondefining_declaration_type ? first_nondefining_declaration_type->class_name().c_str() : "null");
     if (secondary_nondefining_declaration != NULL)
        {
          SgType* secondary_nondefining_declaration_type = getType(secondary_nondefining_declaration);
          printf ("--- secondary_nondefining_declaration = %p secondary_nondefining_declaration->get_type() = %p = %s \n",
               secondary_nondefining_declaration,secondary_nondefining_declaration_type,secondary_nondefining_declaration_type ? secondary_nondefining_declaration_type->class_name().c_str() : "null");
        }
     if (defining_declaration != NULL)
        {
          SgType* defining_declaration_type = getType(defining_declaration);
          printf ("--- defining_declaration              = %p defining_declaration->get_type()              = %p = %s \n",
               defining_declaration,defining_declaration_type,defining_declaration_type ? defining_declaration_type->class_name().c_str() : "null");
        }


     printf ("--- first_nondefining_declaration     = %p first_nondefining_declaration->get_firstNondefiningDeclaration()     = %p \n",first_nondefining_declaration,first_nondefining_declaration->get_firstNondefiningDeclaration());
     if (secondary_nondefining_declaration != NULL)
        {
          printf ("--- secondary_nondefining_declaration = %p secondary_nondefining_declaration->get_firstNondefiningDeclaration() = %p \n",secondary_nondefining_declaration,secondary_nondefining_declaration->get_firstNondefiningDeclaration());
        }
     if (defining_declaration != NULL)
        {
          printf ("--- defining_declaration              = %p defining_declaration->get_firstNondefiningDeclaration()              = %p \n",defining_declaration,defining_declaration->get_firstNondefiningDeclaration());
        }

     printf ("--- first_nondefining_declaration     = %p first_nondefining_declaration->get_definingDeclaration()     = %p \n",first_nondefining_declaration,first_nondefining_declaration->get_definingDeclaration());
     if (secondary_nondefining_declaration != NULL)
        {
          printf ("--- secondary_nondefining_declaration = %p secondary_nondefining_declaration->get_definingDeclaration() = %p \n",secondary_nondefining_declaration,secondary_nondefining_declaration->get_definingDeclaration());
        }
     if (defining_declaration != NULL)
        {
          printf ("--- defining_declaration              = %p defining_declaration->get_definingDeclaration()              = %p \n",defining_declaration,defining_declaration->get_definingDeclaration());
        }
#endif
   }

void testDefiningNonDefiningDeclarations(SgDeclarationStatement* first_nondefining_declaration, SgDeclarationStatement* defining_declaration, SgDeclarationStatement* secondary_nondefining_declaration, const string & label)
   {
  // We must have a valid first_nondefining_declaration (by definition).
     ROSE_ASSERT(first_nondefining_declaration != NULL);

  // ROSE_ASSERT(nondefdecl != NULL);

  // ROSE_ASSERT(first_nondefdecl->get_type() != NULL);
  // ROSE_ASSERT(nondefdecl->get_type()       != NULL);

#if 1
  // Debugging info.
     outputDefiningNonDefiningDeclarations(first_nondefining_declaration,defining_declaration,secondary_nondefining_declaration,label);
#endif

     if (defining_declaration != NULL)
        {
       // DQ (9/4/2012): If defDecl != NULL, I want to assert this for the new EDG/ROSE connection code (at least).
          ROSE_ASSERT(defining_declaration->get_definingDeclaration()                  == defining_declaration);
          ROSE_ASSERT(defining_declaration->get_firstNondefiningDeclaration()          == first_nondefining_declaration);

          ROSE_ASSERT(first_nondefining_declaration->get_firstNondefiningDeclaration() == first_nondefining_declaration);

          ROSE_ASSERT(first_nondefining_declaration->get_definingDeclaration() != NULL);
          ROSE_ASSERT(first_nondefining_declaration->get_definingDeclaration() == defining_declaration);

          if (secondary_nondefining_declaration != NULL)
             {
               ROSE_ASSERT(secondary_nondefining_declaration->get_definingDeclaration() != NULL);
               ROSE_ASSERT(secondary_nondefining_declaration->get_firstNondefiningDeclaration() == first_nondefining_declaration);
               ROSE_ASSERT(secondary_nondefining_declaration->get_definingDeclaration()         == defining_declaration);
             }
        }
       else
        {
       // If decl->get_definingDeclaration() == NULL, then make sure everything else is set consistantly.
          ROSE_ASSERT(defining_declaration == NULL);

          if (first_nondefining_declaration->get_definingDeclaration() != NULL)
             {
               printf ("Error: first_nondefining_declaration->get_definingDeclaration() != NULL: first_nondefining_declaration = %p = %s first_nondefining_declaration->get_definingDeclaration() = %p \n",
                    first_nondefining_declaration,first_nondefining_declaration->class_name().c_str(),first_nondefining_declaration->get_definingDeclaration());
             }
          ROSE_ASSERT(first_nondefining_declaration->get_definingDeclaration() == NULL);
          ROSE_ASSERT(first_nondefining_declaration->get_firstNondefiningDeclaration() == first_nondefining_declaration);
          if (secondary_nondefining_declaration != NULL)
             {
               ROSE_ASSERT(secondary_nondefining_declaration->get_definingDeclaration() == NULL);
               ROSE_ASSERT(secondary_nondefining_declaration->get_firstNondefiningDeclaration() == first_nondefining_declaration);
             }
        }

  // Only check the type if this in NOT a namespace.
     SgNamespaceAliasDeclarationStatement* first_nondefining_declaration_namespace_alias = isSgNamespaceAliasDeclarationStatement(first_nondefining_declaration);
     SgNamespaceDeclarationStatement* first_nondefining_declaration_namespace            = isSgNamespaceDeclarationStatement(first_nondefining_declaration);
     if (first_nondefining_declaration_namespace == NULL && first_nondefining_declaration_namespace_alias == NULL)
        {
       // Now test the associated types that are stored redundantly at each declaration (they should be the same).
          testDefiningNonDefiningDeclarationTypes(first_nondefining_declaration,defining_declaration,secondary_nondefining_declaration);
        }
   }

void testDefiningNonDefiningDeclarationTypes(SgDeclarationStatement* first_nondefining_declaration, SgDeclarationStatement* defining_declaration, SgDeclarationStatement* secondary_nondefining_declaration)
   {
  // We must have a valid first_nondefining_declaration (by definition).
     ROSE_ASSERT(first_nondefining_declaration != NULL);

     SgType* first_nondefining_declaration_type = getType(first_nondefining_declaration);

     ROSE_ASSERT(first_nondefining_declaration_type != NULL);
     if (defining_declaration != NULL)
        {
          SgType* defining_declaration_type = getType(defining_declaration);
          ROSE_ASSERT(defining_declaration_type != NULL);
          ROSE_ASSERT(first_nondefining_declaration_type == defining_declaration_type);

          if (secondary_nondefining_declaration != NULL)
             {
               SgType* secondary_nondefining_declaration_type = getType(secondary_nondefining_declaration);
               ROSE_ASSERT(secondary_nondefining_declaration_type != NULL);
               ROSE_ASSERT(secondary_nondefining_declaration_type == first_nondefining_declaration_type);
             }
        }
       else
        {
       // If decl->get_definingDeclaration() == NULL, then make sure everything else is set consistantly.
          ROSE_ASSERT(getType(first_nondefining_declaration) != NULL);
          if (secondary_nondefining_declaration != NULL)
             {
               SgType* secondary_nondefining_declaration_type = getType(secondary_nondefining_declaration);
               ROSE_ASSERT(secondary_nondefining_declaration_type != NULL);
               ROSE_ASSERT(secondary_nondefining_declaration_type == first_nondefining_declaration_type);
             }
        }
   }

bool verifyExistenceOfSymbol(SgDeclarationStatement* declaration)
   {
  // DQ (12/11/2011): Added new test. this fails because the first non-defining function has been defined already and this is the second one.
  // But we want to register all of the a_routine_ptr associated with this function into the cache with the same symbol, so this is OK to support.
     ROSE_ASSERT(declaration != NULL);
     SgSymbol* symbol = declaration->get_symbol_from_symbol_table();
     if (symbol == NULL)
        {
       // If the symbol was not found then it is because we have to lookup the first non-defining declaration and us it to find the symbol.
          ROSE_ASSERT(declaration->get_firstNondefiningDeclaration() != NULL);
          ROSE_ASSERT(declaration != declaration->get_firstNondefiningDeclaration());
          symbol = declaration->get_firstNondefiningDeclaration()->get_symbol_from_symbol_table();
       }
     return (symbol != NULL);
   }

void testAndResetTemplateDeclarationAndTemplateInstantiation ( a_routine_ptr p, SgScopeStatement* scopeOfFunction, bool & buildTemplateInstantiation, bool & buildTemplateDeclaration )
   {
  // DQ (12/15/2011): I would like to disallow instantions of templates in a template definition scope 
  // (it makes no sense and is difficult to support with a generalization of the ROSE IR to make the 
  // return type of get_class_scope* more general (would have to return a SgDeclarationStatement or 
  // the SgTemplateClassDeclaration would ahve to be derived from a SgClassDeclaration.

     ROSE_ASSERT(p != NULL);
     ROSE_ASSERT(scopeOfFunction != NULL);

#if 0
     printf ("In testAndResetTemplateDeclarationAndTemplateInstantiation(): scopeOfFunction = %p = %s \n",scopeOfFunction,scopeOfFunction->class_name().c_str());
#endif

     if (isSgTemplateClassDefinition(scopeOfFunction) != NULL)
        {
          if (buildTemplateInstantiation == true)
             {
               printf ("ERROR: not allowed to put a template member function instantiation declaration into SgTemplateClassDefinition (setting buildTemplateInstantiation == false) \n");
               buildTemplateInstantiation = false;

            // I think that the template arg list should be null, else we can't really ignore that this is a template instantiation.
               ROSE_ASSERT(p->template_arg_list == NULL);
             }
          ROSE_ASSERT(buildTemplateInstantiation == false);

          if (buildTemplateDeclaration == false)
             {
               printf ("ERROR: not allowed to put a non-template member function declaration into SgTemplateClassDefinition (setting buildTemplateDeclaration == true) \n");
               buildTemplateDeclaration = true;
             }
        }

     if (buildTemplateDeclaration == true)
        {
       // For buildTemplateInstantiation to be false.
          if (buildTemplateInstantiation == true)
             {
               printf ("In testAndResetTemplateDeclarationAndTemplateInstantiation(): Forcing buildTemplateInstantiation to be false, since buildTemplateDeclaration == true \n");
               buildTemplateInstantiation = false;
             }
        }
   }

void testFirstNondefiningDeclarationType(SgFunctionDeclaration* first_nondefining_declaration, bool buildTemplateDeclaration, bool buildTemplateInstantiation)
   {
#if 0
     if (first_nondefining_declaration != NULL)
        {
          printf ("In testFirstNondefiningDeclarationType(): first_nondefining_declaration = %p = %s buildTemplateDeclaration = %s buildTemplateInstantiation = %s \n",
                  first_nondefining_declaration,first_nondefining_declaration->class_name().c_str(),
                  buildTemplateDeclaration ? "true" : "false",buildTemplateInstantiation ? "true" : "false");
        }
#endif

  // DQ (4/3/2012): Test this to make sure it is the correct type (matching what we will build).
  // DQ (4/3/2012): Error checking...
     if (buildTemplateDeclaration == true)
        {
       // If we are building a template declaration then any identified first_nondefining_declaration should match.
          if (first_nondefining_declaration != NULL)
             {
               SgTemplateFunctionDeclaration*       templateFunctionDeclaration       = isSgTemplateFunctionDeclaration(first_nondefining_declaration);
               SgTemplateMemberFunctionDeclaration* templateMemberFunctionDeclaration = isSgTemplateMemberFunctionDeclaration(first_nondefining_declaration);
               if (templateFunctionDeclaration == NULL && templateMemberFunctionDeclaration == NULL)
                  {
                    printf ("Error: Found an odd IR node in testFirstNondefiningDeclarationType() first_nondefining_declaration = %p = %s \n",first_nondefining_declaration,first_nondefining_declaration->class_name().c_str());
                  }
               ROSE_ASSERT(templateFunctionDeclaration != NULL || templateMemberFunctionDeclaration != NULL);
             }
        }

  // DQ (4/3/2012): Error checking...
     if (buildTemplateInstantiation == true)
        {
       // If we are building a template declaration then any identified first_nondefining_declaration should match.
          if (first_nondefining_declaration != NULL)
             {
               SgTemplateInstantiationFunctionDecl*       templateFunctionInstantiationDeclaration       = isSgTemplateInstantiationFunctionDecl(first_nondefining_declaration);
               SgTemplateInstantiationMemberFunctionDecl* templateMemberFunctionInstantiationDeclaration = isSgTemplateInstantiationMemberFunctionDecl(first_nondefining_declaration);
               if (templateFunctionInstantiationDeclaration == NULL && templateMemberFunctionInstantiationDeclaration == NULL)
                  {
                    printf ("ERROR: In testFirstNondefiningDeclarationType(): first_nondefining_declaration = %p = %s \n",first_nondefining_declaration,first_nondefining_declaration->class_name().c_str());
                  }
               ROSE_ASSERT(templateFunctionInstantiationDeclaration != NULL || templateMemberFunctionInstantiationDeclaration != NULL);
             }
        }
   }

void testEdgRoseMaps()
   {
#if 0
     printf ("******************************************************** \n");
     printf ("******************************************************** \n");
     printf ("In testEdgRoseMaps(): \n");
     printf ("******************************************************** \n");
#endif

// DQ (6/26/2013): Test turning off this function completely.
// DQ (2/19/2013): Turn off all of this internal checking when using NDEBUG (which also disables all asserts).
// #ifndef NDEBUG
#if 0

  // DQ (9/16/2012): This is a performance test; I think the internal testing is taking a LONG time (especially the new type testing).
  // This allows us to do a variable amount of testing by running each internal test less frequently (especially the type testing 
  // which is REALLY expensive on largeer input codes).
     static int test_counter = 0;

  // Changing this value allows us to change the sampling rate for the internal testing (actual test per attempted test).
#if 0
     static unsigned long frequency_of_sampling = 1;
#else
  // DQ (6/25/2013): This appears to still be %25 of the cost of running ROSE over large programs (such as the ROSE heard files).
  // static unsigned long frequency_of_sampling = 10000;
     static unsigned long frequency_of_sampling = 10000;
#endif

  // DQ (8/10/2012): Additional tests:
  //    1) We could make sure that we have only unique entries in the function_declaration_map.
  //       This would avoid resetting existing entries and loosing such references between EDG 
  //       and ROSE functions.

     if (test_counter % frequency_of_sampling == 0)
        {
  // Test the non-defining declarations
     std::map<a_routine_ptr, SgFunctionDeclaration*>::iterator i = first_nondefining_declarations_for_functions.begin();
     while (i != first_nondefining_declarations_for_functions.end())
        {
       // These should only be non-template functions.
          SgFunctionDeclaration* functionDeclaration = i->second;

       // This might be NULL if we caught the map in a stage where a function is being 
       // processed (try not to test at these points so that we can make this an error).
          if (functionDeclaration == NULL)
             {
               printf ("Found a NULL reference in the first_nondefining_declarations_for_functions map i->first = %p \n",i->first);
             }
          ROSE_ASSERT(functionDeclaration != NULL);

          SgTemplateFunctionDeclaration*       templateFunctionDeclaration       = isSgTemplateFunctionDeclaration(functionDeclaration);
          SgTemplateMemberFunctionDeclaration* templateMemberFunctionDeclaration = isSgTemplateMemberFunctionDeclaration(functionDeclaration);

       // Verify this is not a template function (member or non-member).
          if (templateFunctionDeclaration != NULL || templateMemberFunctionDeclaration != NULL)
             {
               printf ("Found a NULL reference in the first_nondefining_declarations_for_functions map i->first = %p \n",i->first);
             }
          ROSE_ASSERT(templateFunctionDeclaration == NULL);
          ROSE_ASSERT(templateMemberFunctionDeclaration == NULL);

       // DQ (4/24/2012): We should not have a defining declaration in the first_nondefining_declarations_for_functions map.
          ROSE_ASSERT(functionDeclaration != functionDeclaration->get_definingDeclaration());

          i++;
        }

       // End of test_counter scope.
        }

     if (test_counter % frequency_of_sampling == 0)
        {
  // Test the defining declarations
     std::map<a_routine_ptr, SgFunctionDeclaration*>::iterator j = defining_declarations_for_functions.begin();
     while (j != defining_declarations_for_functions.end())
        {
       // These should only be non-template functions.
          SgFunctionDeclaration* functionDeclaration = j->second;

       // This might be NULL if we caught the map in a stage where a function is being 
       // processed (try not to test at these points so that we can make this an error).
          if (functionDeclaration == NULL)
             {
               printf ("Found a NULL reference in the defining_declarations_for_functions map j->first = %p \n",j->first);
             }
          ROSE_ASSERT(functionDeclaration != NULL);

          SgTemplateFunctionDeclaration*       templateFunctionDeclaration       = isSgTemplateFunctionDeclaration(functionDeclaration);
          SgTemplateMemberFunctionDeclaration* templateMemberFunctionDeclaration = isSgTemplateMemberFunctionDeclaration(functionDeclaration);

       // Verify this is not a template function (member or non-member).
          ROSE_ASSERT(templateFunctionDeclaration == NULL);
          ROSE_ASSERT(templateMemberFunctionDeclaration == NULL);

       // DQ (4/24/2012): We should not have a non-defining declaration in the defining_declarations_for_functions map.
          ROSE_ASSERT(functionDeclaration != functionDeclaration->get_firstNondefiningDeclaration());

          j++;
        }

       // End of test_counter scope.
        }

     if (test_counter % frequency_of_sampling == 0)
        {
  // Test the non-defining declarations
     std::map<a_template_ptr, SgDeclarationStatement*>::iterator k = first_nondefining_declarations_for_templates.begin();
     while (k != first_nondefining_declarations_for_templates.end())
        {
       // These should only be non-template functions.
          SgDeclarationStatement* declaration = k->second;

       // This might be NULL if we caught the map in a stage where a function is being 
       // processed (try not to test at these points so that we can make this an error).
          if (declaration == NULL)
             {
               printf ("Found a NULL reference in the first_nondefining_declarations_for_templates map k->first = %p \n",k->first);
             }
          ROSE_ASSERT(declaration != NULL);

          SgTemplateClassDeclaration*          templateClassDeclaration          = isSgTemplateClassDeclaration(declaration);
          SgTemplateFunctionDeclaration*       templateFunctionDeclaration       = isSgTemplateFunctionDeclaration(declaration);
          SgTemplateMemberFunctionDeclaration* templateMemberFunctionDeclaration = isSgTemplateMemberFunctionDeclaration(declaration);
          SgTemplateVariableDeclaration*       templateVariableDeclaration       = isSgTemplateVariableDeclaration(declaration);

       // DQ (9/10/2014): Can we eliminate this?
       // DQ (7/19/2012): This is new support for template template parameter (and I am not sure how it should best be supported).

       // Verify this IS a template function (member or non-member).
          ROSE_ASSERT(templateClassDeclaration != NULL || templateFunctionDeclaration != NULL || templateMemberFunctionDeclaration != NULL || templateVariableDeclaration != NULL || templateDeclaration != NULL);

       // DQ (4/24/2012): We should not have a defining declaration in the first_nondefining_declarations_for_templates map.
          if (declaration == declaration->get_definingDeclaration())
             {
               printf ("Error: defining declaration in first_nondefining_declarations_for_templates (declaration = %p = %s) \n",declaration,declaration->class_name().c_str());
             }
          ROSE_ASSERT(declaration != declaration->get_definingDeclaration());
          k++;
        }
       // End of test_counter scope.
        }

     if (test_counter % frequency_of_sampling == 0)
        {
  // Test the defining declarations
     std::map<a_template_ptr, SgDeclarationStatement*>::iterator l = defining_declarations_for_templates.begin();
     while (l != defining_declarations_for_templates.end())
        {
       // These should only be non-template functions.
          SgDeclarationStatement* declaration = l->second;

       // This might be NULL if we caught the map in a stage where a function is being 
       // processed (try not to test at these points so that we can make this an error).
          if (declaration == NULL)
             {
               printf ("Found a NULL reference in the defining_declarations_for_templates map l->first = %p \n",l->first);
             }
          ROSE_ASSERT(declaration != NULL);

          SgTemplateClassDeclaration*          templateClassDeclaration          = isSgTemplateClassDeclaration(declaration);
          SgTemplateFunctionDeclaration*       templateFunctionDeclaration       = isSgTemplateFunctionDeclaration(declaration);
          SgTemplateMemberFunctionDeclaration* templateMemberFunctionDeclaration = isSgTemplateMemberFunctionDeclaration(declaration);

       // Verify this IS a template function (member or non-member).
          ROSE_ASSERT(templateClassDeclaration != NULL || templateFunctionDeclaration != NULL || templateMemberFunctionDeclaration != NULL);

       // DQ (4/24/2012): We should not have a nondefining declaration in the defining_declarations_for_templates map.
          ROSE_ASSERT(declaration != declaration->get_firstNondefiningDeclaration());

          l++;
        }

       // End of test_counter scope.
        }

     if (test_counter % frequency_of_sampling == 0)
        {
  // Test the scopes: scope_cache
     std::set<SgScopeStatement*> setOfRoseNodes;
     std::map<a_scope_ptr, SgScopeStatement*>::iterator m = scope_cache.begin();
     while (m != scope_cache.end())
        {
       // These should only be non-template functions.
          SgScopeStatement* scope = m->second;
          ROSE_ASSERT(scope != NULL);

       // DQ (4/22/2012): Make sure that ROSE IR nodes are unique in the scope_cache.
          if (setOfRoseNodes.find(scope) != setOfRoseNodes.end())
             {
            // DQ (9/8/2012): Make this a warning.
            // printf ("Error: ROSE scope %p = %s is repeated (associated second time with edg scope = %p) \n",scope,scope->class_name().c_str(),m->first);
#ifdef DEBUG_CONVERSION
               printf ("WARNING: Allowing multiple EDG scopes to reference a specific single ROSE scope: ROSE scope %p = %s is repeated (associated second time with edg scope = %p) \n",scope,scope->class_name().c_str(),m->first);
#endif
#if 0
               printf ("In testEdgRoseMaps(): Calling summarize_scope() \n");
               summarize_scope(m->first);
#endif
             }
#if 0
       // DQ (9/8/2012): We don't want to build redundant class definitions, so it seem I have to share scopes, however, I am still debugging this.
          if (setOfRoseNodes.find(scope) != setOfRoseNodes.end())
             {
               printf ("WARNING: Allowing multiple EDG scopes to reference a specific single ROSE scope (as a test) \n");
             }
#endif
       // ROSE_ASSERT(setOfRoseNodes.find(scope) == setOfRoseNodes.end());
          if (setOfRoseNodes.find(scope) == setOfRoseNodes.end())
             {
               setOfRoseNodes.insert(scope);
             }
#if 0
          printf ("In testEdgRoseMaps(): edg_scope = %p scope = %p = %s \n",m->first,scope,scope->class_name().c_str());
#endif
#if 0
          printf ("\n******************************************************** \n");
          printf ("In testEdgRoseMaps(): Calling summarize_scope() \n");
          summarize_scope(m->first);
          printf ("******************************************************** \n\n");
#endif
          SgTemplateClassDefinition*   templateClassDefinition   = isSgTemplateClassDefinition(scope);
          SgTemplateInstantiationDefn* templateInstantiationDefn = isSgTemplateInstantiationDefn(scope);
          if (templateClassDefinition != NULL)
             {
            // A template class can be anywhere...
             }
            else
             {
               if (templateInstantiationDefn != NULL)
                  {
                 // But a template instantiation should not be in a template class...
                    SgTemplateClassDefinition*   problem_templateClassDefinition   = isSgTemplateClassDefinition(templateInstantiationDefn->get_scope());
                 // ROSE_ASSERT(isSgTemplateClassDefinition(templateInstantiationDefn->get_scope()) == NULL);
                    if (problem_templateClassDefinition != NULL)
                       {
                         printf ("problem_templateClassDefinition should be NULL, but problem_templateClassDefinition = %p = %s \n",problem_templateClassDefinition,problem_templateClassDefinition->class_name().c_str());
                       }
                    ROSE_ASSERT(problem_templateClassDefinition == NULL);
                  }
             }

       // ROSE_ASSERT(templateClassDefinition != NULL || templateInstantiationDefn != NULL || templateInstantiationDefn != NULL);

          m++;
        }

       // End of test_counter scope.
        }


  // DQ (9/16/2012): Turn off the internal testing to evaluate it's performance (warning is output).
#if 0
     if (test_counter % (frequency_of_sampling*10) == 0)
#else
     printf ("Evaluation of type_translation_cache \n");
  // This is how to make sure we trun this test as much as possible (for debugging).
     if (test_counter % 1 == 0)
#endif
        {
          int outer_counter = 0;

  // DQ (8/12/2012): Added tests.
  // Test the types: type_translation_cache
     std::set<SgType*> setOfRoseTypeNodes;
     std::map<a_type_ptr, SgType*>::iterator n = type_translation_cache.begin();
     while (n != type_translation_cache.end())
        {
       // These should only be non-template functions.
          SgType* type = n->second;

          if (type == NULL)
             {
               printf ("In testEdgRoseMaps(): Failing NULL pointer test in type_translation_cache for n->first = %p \n",n->first);
             }
          ROSE_ASSERT(type != NULL);
#if 0
          printf ("In testEdgRoseMaps(): Evaluate type: n->first = %p = %s type = %p = %s \n",n->first,summarize_type_kind_in_string(n->first),type,type->class_name().c_str());
#endif

       // DQ (9/9/2012): Added additional testing to make sure that each ROSE type is asociated with only one edg type.
          if (setOfRoseTypeNodes.find(type) == setOfRoseTypeNodes.end())
             {
               setOfRoseTypeNodes.insert(type);
             }
            else
             {
#if 0
               printf ("WARNING: %d ROSE type = %p = %s is already present in type_translation_cache (detect 2nd attempt to insert it using edg_type = %p \n",outer_counter,type,type->class_name().c_str(),n->first);
#endif
               SgFunctionType*       functionType       = isSgFunctionType      (n->second);
               SgMemberFunctionType* memberFunctionType = isSgMemberFunctionType(n->second);
               SgTemplateType*       templateType       = isSgTemplateType      (n->second);
               SgPointerType*        pointerType        = isSgPointerType       (n->second);
               SgModifierType*       modifierType       = isSgModifierType      (n->second);
               SgReferenceType*      referenceType      = isSgReferenceType     (n->second);
               SgTypedefType*        typedefType        = isSgTypedefType       (n->second);
               SgTypeChar*           charType           = isSgTypeChar          (n->second);
               SgTypeVoid*           voidType           = isSgTypeVoid          (n->second);
               bool isIntegerType = n->second->isIntegerType();
               bool isFloatType   = n->second->isFloatType();
               
               if (functionType == NULL && memberFunctionType == NULL && templateType == NULL && 
                   pointerType == NULL  && modifierType == NULL       && referenceType == NULL && 
                   typedefType == NULL  && isIntegerType == false     && isFloatType == false &&
                   charType == NULL     && voidType == NULL)
                  {
                    bool trigger_error = false;
                    int counter = 0;

                    std::map<a_type_ptr, SgType*>::iterator db_n = type_translation_cache.begin();
                    while(db_n != type_translation_cache.end())
                       {
                         ROSE_ASSERT(db_n->first  != NULL);
                         ROSE_ASSERT(db_n->second != NULL);
#if 0
                      // DQ (9/12/2012): Decrease the amount of output spew by ignoring cases that we will not consider to be an error.
                         printf ("  --- counter = %d: db_n->first = %p db_n->second = %p = %s \n",counter,db_n->first,db_n->second,db_n->second->class_name().c_str());
#endif
                         SgClassType* classType = isSgClassType(db_n->second);

                         SgClassDeclaration*         classDeclaration          = NULL;
                         SgClassDeclaration*         classDeclaration_in_cache = NULL;
                         SgTemplateClassDeclaration* templateClassDeclaration  = NULL;

                         if (classType != NULL)
                            {
                              classDeclaration         = isSgClassDeclaration(classType->get_declaration());
                              templateClassDeclaration = isSgTemplateClassDeclaration(classDeclaration);
#if 0
                           // DQ (9/12/2012): Moved this output so that we are only reporting errors that we care about.
                              printf ("  --- db_n->first = %p db_n->second = %p = %s \n",db_n->first,db_n->second,db_n->second->class_name().c_str());
                              printf ("  --- --- classType = %p classDeclaration = %p = %s name = %s \n",classType,classDeclaration,classDeclaration->class_name().c_str(),classDeclaration->get_name().str());
                              if (templateClassDeclaration != NULL)
                                 {
                                   printf ("  --- --- classType = %p templateClassDeclaration = %p = %s name = %s #template Parameters = %" PRIuPTR " \n",
                                        classType,templateClassDeclaration,classDeclaration->class_name().c_str(),templateClassDeclaration->get_name().str(),templateClassDeclaration->get_templateParameters().size());
                                 }
#endif
                            }

                         if (type == db_n->second)
                            {
                           // This is the matching SgType IR node.
                              SgClassType* classType_in_cache = isSgClassType(type);
                              if (classType_in_cache == NULL)
                                 {
                                   printf ("In testEdgRoseMaps(): type = %p = %s \n",type,type->class_name().c_str());
                                   SgType* stripped_type = type->stripType();
                                   ROSE_ASSERT(stripped_type != NULL);
                                   printf ("In testEdgRoseMaps(): stripped_type = %p = %s \n",stripped_type,stripped_type->class_name().c_str());

                                   classType_in_cache = isSgClassType(stripped_type);
                                 }
                              ROSE_ASSERT(classType_in_cache != NULL);

                              classDeclaration_in_cache = isSgClassDeclaration(classType_in_cache->get_declaration());
                              ROSE_ASSERT(classDeclaration_in_cache != NULL);

                           // printf ("Calling summarize_type for n->first    = %p \n",db_n->first);
                           // summarize_type(n->first);
#if 0
                              printf ("Calling summarize_type for db_n->first = %p \n",db_n->first);
                              summarize_type(db_n->first);
#endif
                           // DQ (9/12/2012): If these are the same class declaration then it seems that this is OK; EDG uses seperate type IR nodes, but ROSE shares these types.
                           // trigger_error = (classType != NULL);
                              trigger_error = (classType != NULL) && (classDeclaration != classDeclaration_in_cache);
                            }

#if 1
                         if (trigger_error == true)
                            {
#if 1
                           // DQ (9/12/2012): Moved this output so that we are only reporting errors that we care about.
                              printf ("  --- db_n->first = %p db_n->second = %p = %s \n",db_n->first,db_n->second,db_n->second->class_name().c_str());
                              if (classDeclaration != NULL)
                                   printf ("  --- --- classType = %p classDeclaration = %p = %s name = %s \n",classType,classDeclaration,classDeclaration->class_name().c_str(),classDeclaration->get_name().str());
                              if (templateClassDeclaration != NULL)
                                 {
                                   printf ("  --- --- classType = %p templateClassDeclaration = %p = %s name = %s #template Parameters = %" PRIuPTR " \n",
                                        classType,templateClassDeclaration,classDeclaration->class_name().c_str(),templateClassDeclaration->get_name().str(),templateClassDeclaration->get_templateParameters().size());
                                 }
#endif
                              printf ("Make this an error for SgClassType only to support testing! \n");
                              ROSE_ASSERT(false);
                            }
#endif
                         db_n++;
                         counter++;
                       }
#if 0
                    if (trigger_error == true)
                       {
                         printf ("Make this an error for SgClassType only to support testing! \n");
                         ROSE_ASSERT(false);
                       }
#endif
                  }
#if 0
               printf ("Make this an error to support testing! \n");
               ROSE_ASSERT(false);
#endif
             }

          n++;

          outer_counter++;
        }

       // End of test_counter scope.
        }

#ifdef DEBUG_CONVERSION
     if (test_counter % 1000 == 0)
        {
          printf ("WARNING: Skipping tests in testEdgRoseMaps() test_counter = %d \n",test_counter);
        }
#endif
     test_counter++;

// endif for ifndef NDEBUG
#endif

#if 0
     printf ("******************************************************** \n");
     printf ("Leaving testEdgRoseMaps(): \n");
     printf ("******************************************************** \n");
     printf ("******************************************************** \n");
#endif
   }

void verifyFunctionHasSymbolInAssociatedScope(SgFunctionDeclaration* first_nondefining_declaration, SgTemplateParameterPtrList* templateParameterList, SgTemplateArgumentPtrList* templateArgumentsList)
   {
  // DQ (1/27/2013): Added early test for code failing in buildDefiningFunctionDeclaration_T().
  // This function takes any function declaration and verifies that there is a matching symbol that 
  // can be found for it using the name and type of the function and symbol table associated with 
  // the functions declaration's scope.  This should always be true.

  // nameWithTemplateArguments = appendTemplateArgumentsToName(nameWithoutTemplateArguments,templateArgumentsList);
     string nameWithTemplateArguments = first_nondefining_declaration->get_name();
  // SgMemberFunctionType* func_type = isSgMemberFunctionType(first_nondefining_declaration->get_type());
     SgFunctionType* func_type = isSgFunctionType(first_nondefining_declaration->get_type());
     ROSE_ASSERT(func_type != NULL);
     SgScopeStatement* scope = first_nondefining_declaration->get_scope();
     ROSE_ASSERT(scope != NULL);

     SgSymbol* func_symbol = NULL;

  // Note that the testing is based on type, and must respected the type hierarchy (else everything 
  // is a function declaration and we don't use the correct symbol table search function).

  // Test for template instantations declarations or template declarations first (test for member functions before non-member functions).
     if (isSgTemplateInstantiationMemberFunctionDecl(first_nondefining_declaration) != NULL)
        {
       // func_symbol = scope->get_symbol_table()->find_symbol_by_type_of_function<SgTemplateInstantiationMemberFunctionDecl>(nameWithTemplateArguments,func_type);
          func_symbol = scope->find_symbol_by_type_of_function<SgTemplateInstantiationMemberFunctionDecl>(nameWithTemplateArguments,func_type,NULL,templateArgumentsList);
        }
       else
        {
          if (isSgTemplateInstantiationFunctionDecl(first_nondefining_declaration) != NULL)
             {
            // func_symbol = scope->get_symbol_table()->find_symbol_by_type_of_function<SgTemplateInstantiationFunctionDecl>(nameWithTemplateArguments,func_type);
               func_symbol = scope->find_symbol_by_type_of_function<SgTemplateInstantiationFunctionDecl>(nameWithTemplateArguments,func_type,NULL,templateArgumentsList);
             }
            else
             {
#if 0
               printf ("In verifyFunctionHasSymbolInAssociatedScope(): Niether a function nor member function template instantiation: first_nondefining_declaration = %p = %s = %s \n",
                    first_nondefining_declaration,first_nondefining_declaration->get_name().str(),first_nondefining_declaration->class_name().c_str());
#endif
               if (isSgTemplateMemberFunctionDeclaration(first_nondefining_declaration) != NULL)
                  {
                 // DQ (8/7/2013): Adding support for template functions overloaded using template parameters.
                 // func_symbol = scope->get_symbol_table()->find_symbol_by_type_of_function<SgTemplateMemberFunctionDeclaration>(nameWithTemplateArguments,func_type);
                 // func_symbol = scope->find_symbol_by_type_of_function<SgTemplateMemberFunctionDeclaration>(nameWithTemplateArguments,func_type);
                     func_symbol = scope->find_symbol_by_type_of_function<SgTemplateMemberFunctionDeclaration>(nameWithTemplateArguments,func_type,templateParameterList,NULL);
                  }
                 else
                  {
                    if (isSgTemplateFunctionDeclaration(first_nondefining_declaration) != NULL)
                       {
                      // DQ (8/7/2013): Adding support for template functions overloaded using template parameters.
                      // func_symbol = scope->get_symbol_table()->find_symbol_by_type_of_function<SgTemplateFunctionDeclaration>(nameWithTemplateArguments,func_type);
                      // func_symbol = scope->find_symbol_by_type_of_function<SgTemplateFunctionDeclaration>(nameWithTemplateArguments,func_type);
                          func_symbol = scope->find_symbol_by_type_of_function<SgTemplateFunctionDeclaration>(nameWithTemplateArguments,func_type,templateParameterList,NULL);
                       }
                      else
                       {
#if 0
                         printf ("In verifyFunctionHasSymbolInAssociatedScope(): Niether a function nor member function template instantiation or non-template: first_nondefining_declaration = %p = %s = %s \n",
                              first_nondefining_declaration,first_nondefining_declaration->get_name().str(),first_nondefining_declaration->class_name().c_str());
#endif

                      // Test for non-template instantations declarations and non-template declarations last (test for member functions before non-member functions).
                         if (isSgMemberFunctionDeclaration(first_nondefining_declaration) != NULL)
                            {
                           // func_symbol = scope->get_symbol_table()->find_symbol_by_type_of_function<SgMemberFunctionDeclaration>(nameWithTemplateArguments,func_type);
                           // func_symbol = scope->find_symbol_by_type_of_function<SgMemberFunctionDeclaration>(nameWithTemplateArguments,func_type);
                              func_symbol = scope->find_symbol_by_type_of_function<SgMemberFunctionDeclaration>(nameWithTemplateArguments,func_type,NULL,NULL);
                            }
                           else
                            {
                              if (isSgFunctionDeclaration(first_nondefining_declaration) != NULL)
                                 {
                                // func_symbol = scope->get_symbol_table()->find_symbol_by_type_of_function<SgFunctionDeclaration>(nameWithTemplateArguments,func_type);
                                // func_symbol = scope->find_symbol_by_type_of_function<SgFunctionDeclaration>(nameWithTemplateArguments,func_type);
                                   func_symbol = scope->find_symbol_by_type_of_function<SgFunctionDeclaration>(nameWithTemplateArguments,func_type,NULL,NULL);
                                 }
                                else
                                 {
                                   printf ("ERROR: first_nondefining_declaration = %p = %s = %s \n",
                                        first_nondefining_declaration,first_nondefining_declaration->get_name().str(),first_nondefining_declaration->class_name().c_str());
                                   ROSE_ASSERT(false);
                                 }
                            }
                       }
                  }
             }
        }

     if (func_symbol == NULL)
        {
       // printf ("ERROR caught in setDefiningAndFirstNondefiningDeclarations(): nameWithTemplateArguments = %s buildTemplateInstantiation = %s \n",
       //      nameWithTemplateArguments.c_str(),buildTemplateInstantiation ? "true:" : "false");
          printf ("ERROR caught in verifyFunctionHasSymbolInAssociatedScope(): nameWithTemplateArguments = %s \n",nameWithTemplateArguments.c_str());
          printf ("ERROR caught in verifyFunctionHasSymbolInAssociatedScope(): scope = %p = %s \n",scope,scope->class_name().c_str());
          printf ("ERROR caught in verifyFunctionHasSymbolInAssociatedScope(): func_symbol == NULL for first_nondefining_declaration = %p = %s and func_type = %p = %s \n",
               first_nondefining_declaration,first_nondefining_declaration->class_name().c_str(),func_type,func_type->class_name().c_str());
        }

     ROSE_ASSERT(func_symbol != NULL);
   }

void testAndMarkAsSpecialized(a_routine_ptr p, bool buildTemplateInstantiation, SgFunctionDeclaration* definingFunctionDeclaration , SgFunctionDeclaration* nondefiningFunctionDeclaration)
   {
  // DQ (8/10/2012): This function test to see if this should be marked as a specialized template instantiation function, and marks it accordingly.

#if 0
     printf ("In testAndMarkAsSpecialized(): p = %p buildTemplateInstantiation = %s definingFunctionDeclaration = %p nondefiningFunctionDeclaration = %p \n",
          p,buildTemplateInstantiation ? "true" : "false",definingFunctionDeclaration,nondefiningFunctionDeclaration);
     printf ("   --- nondefiningFunctionDeclaration->get_name() = %s \n",nondefiningFunctionDeclaration->get_name().str());
#endif

     if (buildTemplateInstantiation == true)
        {
       // Note that if this is a member function in a specialized class this will be false, yet we wan to make it to be output.
       // Maybe even mark it as a specialization, but this might not be technically correct.
          bool markAsSpecialization = (p->is_specialized == true);
          bool memberOfSpecializedClass = false;
#if 0
          printf ("In testAndMarkAsSpecialized(): markAsSpecialization     = %s \n",markAsSpecialization ? "true" : "false");
          printf ("In testAndMarkAsSpecialized(): memberOfSpecializedClass = %s \n",memberOfSpecializedClass ? "true" : "false");
          printf ("   --- topScopeStack() = %p = %s \n",topScopeStack(),topScopeStack()->class_name().c_str());
          printf ("   --- nondefiningFunctionDeclaration->get_scope()      = %p = %s \n",nondefiningFunctionDeclaration->get_scope(),nondefiningFunctionDeclaration->get_scope()->class_name().c_str());
          printf ("   --- nondefiningFunctionDeclaration->get_parent()     = %p = %s \n",nondefiningFunctionDeclaration->get_parent(),nondefiningFunctionDeclaration->get_parent()->class_name().c_str());
          if (nondefiningFunctionDeclaration->get_definingDeclaration() != NULL)
             {
               printf ("   --- nondefiningFunctionDeclaration->get_definingDeclaration() = %p = %s \n",
                    nondefiningFunctionDeclaration->get_definingDeclaration(),nondefiningFunctionDeclaration->get_definingDeclaration()->class_name().c_str());
             }
#endif

       // DQ (8/8/2012): If this is a member function, check if it is in a specialized class and then, if so, mark it as specialized.
          if (nondefiningFunctionDeclaration != NULL)
             {
#if 0
               printf ("In testAndMarkAsSpecialized(): nondefiningFunctionDeclaration->get_scope() = %p = %s \n",
                    nondefiningFunctionDeclaration->get_scope(),nondefiningFunctionDeclaration->get_scope()->class_name().c_str());
               printf ("In testAndMarkAsSpecialized(): nondefiningFunctionDeclaration->get_parent() = %p = %s \n",
                    nondefiningFunctionDeclaration->get_parent(),nondefiningFunctionDeclaration->get_parent()->class_name().c_str());
#endif
               SgClassDefinition* parentClassDefinition = isSgClassDefinition(nondefiningFunctionDeclaration->get_scope());
               if (parentClassDefinition != NULL)
                  {
                    SgClassDeclaration* parentClassDeclaration = parentClassDefinition->get_declaration();
                    ROSE_ASSERT(parentClassDeclaration != NULL);
#if 0
                    printf ("In testAndMarkAsSpecialized(): parentClassDeclaration->get_specialization()  = %s \n",outputSpecializationString(parentClassDeclaration->get_specialization()).c_str());
#endif
                    if (parentClassDeclaration->get_specialization() == SgDeclarationStatement::e_specialization)
                       {
#if 0
                         printf ("In testAndMarkAsSpecialized(): This is the case of a template member function instantiation in a specialized class (so we need to mark this to be output). \n");
#endif
                         memberOfSpecializedClass = true;
                       }
                  }
                 else
                  {
#if 0
                    printf ("In testAndMarkAsSpecialized(): scope is not a SgClassDefinition \n");
#endif
                  }

            // DQ (11/26/2015): We now allow multiple function definitions (two) for instantiations that are defined in the template scope 
            // but normailzed by EDG to not be represented in the original template (string modified by EDG).
               if (isSgGlobal(nondefiningFunctionDeclaration->get_scope()) != NULL && isSgGlobal(topScopeStack()) != NULL)
                  {
#if 0
                    printf ("nondefiningFunctionDeclaration->get_declarationModifier().isFriend() = %s \n",nondefiningFunctionDeclaration->get_declarationModifier().isFriend() ? "true" : "false");
                    if (definingFunctionDeclaration != NULL)
                       {
                         printf ("definingFunctionDeclaration->get_declarationModifier().isFriend() = %s \n",definingFunctionDeclaration->get_declarationModifier().isFriend() ? "true" : "false");
                       }
#endif
                 // DQ (11/27/2015): I think we need to check that this is a friend function.
                    if (nondefiningFunctionDeclaration->get_declarationModifier().isFriend() == true)
                       {
#if 0
                         printf ("In testAndMarkAsSpecialized(): friend identified in global scope: This is the case of a template member function instantiation in a specialized class (so we need to mark this to be output). \n");
#endif
                         memberOfSpecializedClass = true;
                       }
                  }
             }
            else
             {
            // I don't think this branch is possible!
               ROSE_ASSERT(definingFunctionDeclaration == NULL);
             }
#if 0
          printf ("In testAndMarkAsSpecialized(): markAsSpecialization     = %s \n",markAsSpecialization     ? "true" : "false");
          printf ("In testAndMarkAsSpecialized(): memberOfSpecializedClass = %s \n",memberOfSpecializedClass ? "true" : "false");
#endif
#if 0
          printf ("In testAndMarkAsSpecialized(): definingFunctionDeclaration    = %p \n",definingFunctionDeclaration);
          printf ("In testAndMarkAsSpecialized(): nondefiningFunctionDeclaration = %p \n",nondefiningFunctionDeclaration);
#endif
       // DQ (8/8/2012): Mark this as a specialization if appropriate.
          if (memberOfSpecializedClass == true || markAsSpecialization == true)
             {
            // Don't mark this as a specialization if it isn't (else it will be assigned for output ).
               if (markAsSpecialization == true)
                  {
                    if (definingFunctionDeclaration != NULL)
                       {
                         definingFunctionDeclaration->set_specialization(SgDeclarationStatement::e_specialization);
                         ROSE_ASSERT(definingFunctionDeclaration   ->get_file_info() != NULL);
                         ROSE_ASSERT(definingFunctionDeclaration   ->get_file_info()->isCompilerGenerated() == false);
                       }

                    nondefiningFunctionDeclaration->set_specialization(SgDeclarationStatement::e_specialization);
                  }

               ROSE_ASSERT(nondefiningFunctionDeclaration->get_file_info() != NULL);
               if (nondefiningFunctionDeclaration->get_file_info()->isCompilerGenerated() != false)
                  {
                 // DQ (8/8/2012): This might be OK, but check on it later (see test2004_36.C).
#ifdef DEBUG_CONVERSION
                    printf ("WARNING: memberOfSpecializedClass == true || markAsSpecialization == true: This case of nondefiningFunctionDeclaration is different from the definingFunctionDeclaration (isCompilerGenerated() == true) \n");
#endif
                  }

               if (definingFunctionDeclaration != NULL)
                  {
#if 0
                    printf ("In testAndMarkAsSpecialized(): definingFunctionDeclaration: Mark this for explicit output in code generation \n");
#endif
                  }

#if 0
               printf ("In testAndMarkAsSpecialized(): nondefiningFunctionDeclaration: Skip marking this for explicit output in code generation \n");
#endif
             }
            else
             {
            // DQ (8/3/2012): If this is a template instantiation, but not a specialization, then it should be marked as compiler generated.
#if 0
               printf ("p->is_specialized == false && memberOfSpecializedClass == false: make this as compiler generated. \n");
#endif
               if (definingFunctionDeclaration != NULL)
                  {
                    definingFunctionDeclaration->setCompilerGenerated();

                 // DQ (8/8/2012): Added new test.
                    ROSE_ASSERT(definingFunctionDeclaration->get_file_info()->isOutputInCodeGeneration() == false);
                  }

               nondefiningFunctionDeclaration->setCompilerGenerated();

            // DQ (8/8/2012): Added new test.
               if (nondefiningFunctionDeclaration->get_file_info()->isOutputInCodeGeneration() != false)
                  {
                 // DQ (8/8/2012): This might be OK, but check on it later (see test2004_149.C).
#ifdef DEBUG_CONVERSION
                    printf ("memberOfSpecializedClass != true && markAsSpecialization != true: WARNING: This case of nondefiningFunctionDeclaration is different from the definingFunctionDeclaration (isOutputInCodeGeneration() == true) \n");
#endif
                  }

             }

       // DQ (8/8/2012): I don't know why the templateDeclaration is not present.
          SgTemplateInstantiationMemberFunctionDecl* templateInstantiationMemberFunctionDecl            = isSgTemplateInstantiationMemberFunctionDecl(definingFunctionDeclaration);
          SgTemplateInstantiationMemberFunctionDecl* nondefiningTemplateInstantiationMemberFunctionDecl = isSgTemplateInstantiationMemberFunctionDecl(nondefiningFunctionDeclaration);

       // DQ (8/10/2012): Should this be a test of the defining or non-defining declaration?
          if (templateInstantiationMemberFunctionDecl != NULL)
             {
               if (nondefiningTemplateInstantiationMemberFunctionDecl->get_templateDeclaration() == NULL)
                  {
#ifdef DEBUG_CONVERSION
                    printf ("WARNING: nondefiningTemplateInstantiationMemberFunctionDecl->get_templateDeclaration() == NULL \n");
#endif
                  }
             }
        }
       else
        {
       // DQ (9/6/2014): For cases where we build the template declaration from an EDG routine that is used to build a template instantiation, this can fail.
       // DQ (8/3/2012): Verify that if this is not a template instantiation, then it is not a specialization.
          ROSE_ASSERT(p->is_specialized == false);
        }

#if 0
     printf ("Leaving testAndMarkAsSpecialized(): nondefiningFunctionDeclaration->get_file_info()->isOutputInCodeGeneration() = %s \n",
          nondefiningFunctionDeclaration->get_file_info()->isOutputInCodeGeneration() ? "true" : "false");
     if (definingFunctionDeclaration != NULL)
        {
          printf ("Leaving testAndMarkAsSpecialized(): definingFunctionDeclaration->get_file_info()->isOutputInCodeGeneration() = %s \n",
               definingFunctionDeclaration->get_file_info()->isOutputInCodeGeneration() ? "true" : "false");
        }
#endif
   }

void testTemplateArgumentParents(SgTemplateArgumentPtrList & templateArgumentsList, SgDeclarationStatement* nondefining_templateInstantiation)
   {
  // DQ (7/25/2012): The parent should point to the SgTemplateInstantiationDecl, not the SgGlobal.

  // DQ (9/13/2012): This is the new test in SageBuilder)
     SageBuilder::testTemplateArgumentParents(nondefining_templateInstantiation);

  // This is redundant code (but let the test be done twice to make sure).
     ROSE_ASSERT(nondefining_templateInstantiation != NULL);

     for (size_t i = 0; i < templateArgumentsList.size(); i++)
        {
       // DQ (7/25/2012): I don't know if we can assert these conditions.
          ROSE_ASSERT(templateArgumentsList[i]->get_parent() != NULL);
          ROSE_ASSERT(isSgGlobal(templateArgumentsList[i]->get_parent()) == NULL);

          if (templateArgumentsList[i]->get_parent() != nondefining_templateInstantiation)
             {
               printf ("In testTemplateArgumentParents(): templateArgumentsList[i]               = %p = %s \n",templateArgumentsList[i],templateArgumentsList[i]->class_name().c_str());
               printf ("In testTemplateArgumentParents(): templateArgumentsList[i]->get_parent() = %p = %s \n",templateArgumentsList[i]->get_parent(),templateArgumentsList[i]->get_parent()->class_name().c_str());
               printf ("In testTemplateArgumentParents(): nondefining_templateInstantiation      = %p = %s \n",nondefining_templateInstantiation,nondefining_templateInstantiation->class_name().c_str());
               SgDeclarationStatement* firstNondefiningDeclaration = nondefining_templateInstantiation->get_firstNondefiningDeclaration();
               if (firstNondefiningDeclaration != NULL)
                    printf ("In testTemplateArgumentParents(): firstNondefiningDeclaration            = %p = %s \n",firstNondefiningDeclaration,firstNondefiningDeclaration->class_name().c_str());
               SgDeclarationStatement* definingDeclaration = nondefining_templateInstantiation->get_definingDeclaration();
               if (definingDeclaration != NULL)
                    printf ("In testTemplateArgumentParents(): definingDeclaration                    = %p = %s \n",definingDeclaration,definingDeclaration->class_name().c_str());
             }
          ROSE_ASSERT(templateArgumentsList[i]->get_parent() == nondefining_templateInstantiation);

       // templateArgumentsList[i]->set_parent(nondefining_templateInstantiation);
        }
   }

template <>
SgName generateName<a_routine_ptr>(a_routine_ptr p)
   {
  // This is a simpler version since all functions have to have names.
     ROSE_ASSERT(p != NULL);

  // ROSE_ASSERT(p->source_corresp.name != NULL);
     if (p->source_corresp.name == NULL)
        {
       // DQ (4/2/2013): This case is generated for test2013_86.C.
          mprintf ("NOTE: In generateName<a_routine_ptr>(): This is a function with an empty name (allowed as compiler generated initializing constructors to un-named classes, structs, and unions in C++ \n");
        }

     SgName name = p->source_corresp.name;
  // DQ (8/13/2014): Fix EDG bug in ULD operator names such as "operator "" _w" which have a space removed for form "operator ""_w"
     if (p->special_kind == sfk_udl_operator)
        {
          string string_name = name;
          size_t position = string_name.find("\"\"");

       // Add a space after the pair of double quotes.
          string_name.insert(position+2," ");
#if 0
          printf ("In generateName<a_routine_ptr>(): position = %" PRIuPTR " name = %s string_name = %s \n",position,name.str(),string_name.c_str());
#endif
          name = string_name;
        }

     return name;
   }

template <>
SgName generateName<a_variable_ptr>(a_variable_ptr p)
   {
  // This is a simpler version since most variable have to have names.
     ROSE_ASSERT(p != NULL);

  // DQ (6/8/2012): This fails for test2012_98.C (it is not reasonable to always enforce this).
  // ROSE_ASSERT(p->source_corresp.name != NULL);
  // SgName name = p->source_corresp.name;

  // DQ (6/8/2012): We need the flexability to support vaiables that have no name (see test2012_98.C).
     SgName name = p->source_corresp.name == NULL ? generate_name_for_variable(p) : p->source_corresp.name;

     return name;
   }

template <>
SgName generateName<a_field_ptr>(a_field_ptr p)
   {
  // This is a simpler version since most fields have to have names.
     ROSE_ASSERT(p != NULL);

  // DQ (1/1/2014): test2014_01.c demonstrates a test code where this fails (unnamed unions in a struct).
  // We need the flexability to support vaiables that have no name (see test2014_01.c).
     SgName name = p->source_corresp.name == NULL ? generate_name_for_field(p) : p->source_corresp.name;

     return name;
   }

void test_for_correct_form_of_declaration(bool buildTemplateInstantiation, bool buildTemplateDeclaration, SgDeclarationStatement* decl)
   {
     ROSE_ASSERT(decl != NULL);
#if 0
     printf ("In test_for_correct_form_of_declaration(): buildTemplateInstantiation = %s buildTemplateDeclaration = %s decl = %p = %s \n",
          buildTemplateInstantiation ? "true" : "false",buildTemplateDeclaration ? "true" : "false",decl, decl->class_name().c_str());
#endif
  // DQ (11/27/2011): Adding more testing for when we generate template instantiations, non-template functions and template function declarations.
     if (buildTemplateInstantiation == true)
        {
       // These are two different derived classes from SgFunctionDeclaration (so we have to test for each one separately).
       // ROSE_ASSERT(isSgTemplateInstantiationFunctionDecl(decl) != NULL || isSgTemplateInstantiationMemberFunctionDecl(decl) != NULL);
          if (isSgTemplateInstantiationFunctionDecl(decl) == NULL && isSgTemplateInstantiationMemberFunctionDecl(decl) == NULL)
             {
               printf ("Error: buildTemplateInstantiation == true --- decl = %p = %s \n",decl,decl->class_name().c_str());
             }
          ROSE_ASSERT(isSgTemplateInstantiationFunctionDecl(decl) != NULL || isSgTemplateInstantiationMemberFunctionDecl(decl) != NULL);
        }
       else
        {
          if (buildTemplateDeclaration == true)
             {
            // Or a SgTemplateMemberFunctionDeclaration, which is derived from a SgTemplateFunctionDeclaration.
            // ROSE_ASSERT(isSgTemplateFunctionDeclaration(decl) != NULL);
               ROSE_ASSERT(isSgTemplateFunctionDeclaration(decl) != NULL || isSgTemplateMemberFunctionDeclaration(decl) != NULL);
             }
            else
             {
            // Or a SgMemberFunctionDeclaration, which is derived from a SgFunctionDeclaration.
               ROSE_ASSERT(isSgFunctionDeclaration(decl) != NULL);
             }
        }
   }

SgInitializer* wrap_expr_as_initializer(SgExpression* e)
   {
#if 0
     printf ("In wrap_expr_as_initializer(): e = %p \n",e);
#endif

  // If there is no initializer in the source (e.g. for a variable declaration), 
  // then the pointer to the initializer will be NULL.
     if (e == NULL)
        {
          return NULL;
        }
       else
        {
       // If this is any type of SgInitializer then skip wrapping it ...
#if 0
          printf ("In wrap_expr_as_initializer(): e = %p = %s \n",e,e->class_name().c_str());
#endif
          SgInitializer* initialier = isSgInitializer(e);
          if (initialier != NULL)
             {
               return initialier;
             }
            else
             {
            // Scalar variables (at least) have to be wrapped into a SgAssignInitializer...
#if 0
               printf ("In wrap_expr_as_initializer(): calling buildAssignInitializer_nfi() e = %p \n",e);
               printf ("In wrap_expr_as_initializer(): e->get_file_info()->get_filenameString() = %s \n",e->get_file_info()->get_filenameString().c_str());
#endif
            // DQ (12/3/2012): Added test.
               ROSE_ASSERT(e->get_file_info()->get_filenameString() != "");

               return set_generated_file_info_to_unparse(buildAssignInitializer_nfi(e));
             }
        }
   }

#define DEBUG_use_of_builtin_variable 0

SgVariableSymbol * use_of_builtin_variable(a_variable_ptr p, SgName & name) {
#if DEBUG_use_of_builtin_variable
  printf ("In use_of_builtin_variable():\n");
  printf ("  p = %p : %s\n", p, p->source_corresp.name);
  printf ("  name = %p\n", name);
#endif

  SgType* type = buildPointerType(buildConstType(buildCharType()));
  ROSE_ASSERT(type != NULL);
#if DEBUG_use_of_builtin_variable
  printf ("  type = %p : %s\n", type, type ? type->class_name().c_str() : "");
#endif
  SgScopeStatement* globalScope = getGlobalScopeFromScopeStack();
  ROSE_ASSERT(globalScope != NULL);
#if DEBUG_use_of_builtin_variable
  printf ("  globalScope = %p : %s \n", globalScope, globalScope->class_name().c_str());
#endif
  SgVariableDeclaration * hidden_decl = set_generated_file_info(buildVariableDeclaration_nfi(name, type, NULL, globalScope));
#if DEBUG_use_of_builtin_variable
  printf ("  hidden_decl = %p : %s \n", hidden_decl, hidden_decl->class_name().c_str());
#endif
  // Liao 2013/1/10. We have to insert it to the scope to facilitate handling later on (such as moving the variable to a new block)
  // If this is not inserted into the scope, moveStatementsBetweenBlocks() will not find it (and move the associated symbol)
  SageInterface::prependStatement (hidden_decl, globalScope);

  set_generated_file_info(hidden_decl->get_definition());
  set_generated_file_info(getFirstInitializedName(hidden_decl));

  SgVariableSymbol * sym = getFirstVarSym(hidden_decl);
  ROSE_ASSERT (sym != NULL);
#if DEBUG_use_of_builtin_variable
  printf ("  sym  = %p : %s\n", sym, sym ? sym->get_name().str() : "");
#endif
  ROSE_ASSERT(variable_cache.find(p) == variable_cache.end());
  variable_cache[p] = sym;

  return sym;
}

#define DEBUG_use_of_unseen_variable 0

SgVariableSymbol * use_of_unseen_variable(a_variable_ptr p, SgName & name) {
#if DEBUG_use_of_unseen_variable
  printf ("In use_of_unseen_variable():\n");
  printf ("  p = %p : %s\n", p, p->source_corresp.name);
  printf ("  name = %s\n", name.str());
#endif

// TODO Anything useful in this block? Esp. case of: "dimentions of VLA types"
//      If it made it to master then that can be deleted
#if 0
  SgType* type = convert_type(p->type);
#if DEBUG_use_of_unseen_variable
  printf ("  type = %p : %s\n", type, type ? type->class_name().c_str() : "");
#endif

  SgInitializer* initializer = nullptr;
  if (p->is_compound_literal) {
    ROSE_ASSERT(p->is_member_constant == false);
    initializer = convert_var_initializer(p);
    ROSE_ASSERT(initializer != nullptr);
#if DEBUG_use_of_unseen_variable
    printf ("  initializer = %p : %s \n", initializer, initializer->class_name().c_str());
#endif
  }

  SgInitializedName* initializedName = buildInitializedName_nfi(name, type, initializer);
  ROSE_ASSERT(initializedName != nullptr);
#if DEBUG_use_of_unseen_variable
  printf ("  initializedName = %p : %s \n", initializedName, initializedName->class_name().c_str());
#endif

  // This will have to be set when we have built the declaration for this variable.
  ROSE_ASSERT(initializedName->get_declptr() == nullptr);
  ROSE_ASSERT(initializedName->get_prev_decl_item() == nullptr);

  bool should_get_declscope = p->source_corresp.parent_scope == nullptr || p->source_corresp.parent_scope->kind != sck_function;
  should_get_declscope |= scope_cache.find(p->source_corresp.parent_scope) != scope_cache.end();

  SgScopeStatement * rose_scopeOfDeclaration = should_get_declscope ? getDeclarationScope(p->source_corresp) : nullptr;
#if DEBUG_use_of_unseen_variable
  printf ("  rose_scopeOfDeclaration = %p : %s\n", rose_scopeOfDeclaration, rose_scopeOfDeclaration ? rose_scopeOfDeclaration->class_name().c_str() : "");
#endif

  SgVariableSymbol * sym = nullptr;
  if (rose_scopeOfDeclaration != nullptr) {
    initializedName->set_scope(rose_scopeOfDeclaration);
    initializedName->set_parent(rose_scopeOfDeclaration);
    set_file_info_for_corresp(initializedName,p->source_corresp);

    sym = new SgVariableSymbol(initializedName);
    ROSE_ASSERT(sym != nullptr);
#if DEBUG_use_of_unseen_variable
    printf ("  sym  = %p : %s\n", sym, sym ? sym->get_name().str() : "");
#endif
    rose_scopeOfDeclaration->insert_symbol(name,sym);

  } else {
    // DQ (2/14/2016): Added case for when the current ROSE scope has not been built yet.
    // case of rose_scopeOfDeclaration == NULL, this happens if the input scope is sck_function
    // and only for variable references used in the dimentions of VLA types.
    // We need to define a scope for this to be so that the name qualification can be generated for 
    // the name mangling of the function type for which this is in the parameter list. We might want to
    // have a scope for which name qualification would not be persued (but this would be overly complex).
    rose_scopeOfDeclaration = topScopeStack();
    ROSE_ASSERT(rose_scopeOfDeclaration != nullptr);
#if DEBUG_use_of_unseen_variable
    printf ("  rose_scopeOfDeclaration = %p : %s\n", rose_scopeOfDeclaration, rose_scopeOfDeclaration ? rose_scopeOfDeclaration->class_name().c_str() : "");
#endif

    initializedName->set_scope(rose_scopeOfDeclaration);
    initializedName->set_parent(rose_scopeOfDeclaration);
    set_file_info_for_corresp(initializedName, p->source_corresp);

    sym = new SgVariableSymbol(initializedName);
    ROSE_ASSERT(sym != nullptr);
#if DEBUG_use_of_unseen_variable
    printf ("  sym  = %p : %s\n", sym, sym ? sym->get_name().str() : "");
#endif
  }
#else
  SgVariableDeclaration * vdecl = convert_variable_prototype(p, nullptr);
  ROSE_ASSERT(vdecl);
#if DEBUG_use_of_unseen_variable
  printf ("  vdecl = %p : %s\n", vdecl, vdecl ? vdecl->class_name().c_str() : "");
#endif
  SgInitializedName * iname = getFirstInitializedName(vdecl);
  ROSE_ASSERT(iname);
  ROSE_ASSERT(iname->get_parent());
#if DEBUG_use_of_unseen_variable
  printf ("  iname = %p : %s\n", iname, iname ? iname->class_name().c_str() : "");
#endif
  SgVariableSymbol * sym = isSgVariableSymbol(iname->search_for_symbol_from_symbol_table());
  ROSE_ASSERT(sym);
#if DEBUG_use_of_unseen_variable
  printf ("  sym = %p : %s\n", sym, sym ? sym->class_name().c_str() : "");
  printf ("  sym->get_declaration() = %p : %s\n", sym->get_declaration(), sym->get_declaration() ? sym->get_declaration()->class_name().c_str() : "");
#endif

  // TV: this assertion breaks with some template variable instantiations:
  //     for example, we can have multiple instance of the same template header:
  //          template <typename T, typename W, bool = has_word_t<T>>
  //
  //     TODO special name-mangling for nonreal-type used as template argument
  //          Could use the depth/index pair and parent qualified name
  //     Temporary Fix:
  //          only impose the condition if the variable is *not* non-real
  ROSE_ASSERT(p->is_nonreal || (sym->get_declaration() == iname) );
#endif
  return sym;
}

void process_possible_typeof_expression_arguments( a_scope_ptr scope )
   {
  // DQ (4/5/2015): Added support to find expressions used in typeof() operators.

  // This function builds a map that we can use to associate typeof() operators with expressions.
  // It might be the these types are processed with the first statment of a scope (if it is a variable declaration)
  // independent of if it is associated with the first declaration.  This can cause the sse to be advanced (I think).

     ROSE_ASSERT(scope != NULL);

  // DQ (5/20/2015): I think we need this function since it is the only approach to finding some expressions that 
  // are the input paramters to some typeof operators.  See test2015_77.c and test2015_149.c for examples.
  // However, this function needs to be tailored to be more appropriate for this specific use.

#if 0
  // DQ (5/19/2015): We need to check if this function is being called and if we need it.
  // printf ("WARNING: I don't think that this function: process_possible_typeof_expression_arguments() does anything meaningful! \n");
     printf ("NOTE: function process_possible_typeof_expression_arguments() needs to be fixed to process the hidden expressions to the typeof operator \n");
#endif

#define DEBUG_TYPEOF_EXPRESSION 0

#if DEBUG_TYPEOF_EXPRESSION
     printf ("In process_possible_typeof_expression_arguments(): TOP of function scope = %p \n",scope);
#endif

#if 0
     printf ("In process_possible_typeof_expression_arguments(): calling summarize_scope() \n");
     summarize_scope(scope);
#endif

#if 0
  // DQ (5/23/2015): Added support for listing the elements in the typeof maps.
     output_typeof_maps("In process_possible_typeof_expression_arguments()");
#endif

     if (scope->expr_node_refs != NULL)
        {
          a_local_expr_node_ref_ptr local_expr_node_ref_ptr = scope->expr_node_refs;

#if DEBUG_TYPEOF_EXPRESSION
          printf ("Found local_expr_node_ref = %p \n",local_expr_node_ref_ptr);
#endif

          while (local_expr_node_ref_ptr != NULL)
             {
               ROSE_ASSERT(local_expr_node_ref_ptr->referrer.ptr != NULL);

#if DEBUG_TYPEOF_EXPRESSION
            // void* key = local_expr_node_ref_ptr->referrer.ptr;
               a_type_ptr       type_key       = (local_expr_node_ref_ptr->referrer.kind == tk_typeref) ? (a_type_ptr) local_expr_node_ref_ptr->referrer.ptr : NULL;
               an_expr_node_ptr expression_key = NULL;
#endif
               if (local_expr_node_ref_ptr->referrer.ptr != NULL)
                  {
#if DEBUG_TYPEOF_EXPRESSION
                    printf ("     ---- local_expr_node_ref                        = %p \n",local_expr_node_ref_ptr);

                    printf ("     ---- local_expr_node_ref: expr                  = %p \n",local_expr_node_ref_ptr->expr);
                    printf ("     ---- local_expr_node_ref: kind (value)          = %d \n",(int)local_expr_node_ref_ptr->kind);
                    printf ("     ---- local_expr_node_ref: kind (string)         = %s \n",summarize_local_expr_node_ref_string(local_expr_node_ref_ptr->kind).c_str());

                    printf ("     ---- local_expr_node_ref: referrer.kind (value) = %d \n",local_expr_node_ref_ptr->referrer.kind);
                    printf ("     ---- local_expr_node_ref: referrer.ptr          = %p \n",local_expr_node_ref_ptr->referrer.ptr);
                    printf ("     ---- local_expr_node_ref: referrer (alt)        = %s \n",summarize_entry_kind_in_string_kind(local_expr_node_ref_ptr->referrer,true).c_str());
                    printf ("     ---- Build map of expressions = %p using local_expr_node_ref_ptr->referrer.ptr = %p as key \n",local_expr_node_ref_ptr->expr,local_expr_node_ref_ptr->referrer.ptr);
                 // printf ("     ---- typeof_map.find(key) != typeof_map.end()   = %s \n",typeof_map.find(key) != typeof_map.end() ? "true" : "false");
                 // printf ("     ---- edg_type_to_rose_typeof_type_map.find(type_key = %p) != typeof_map.end()   = %s \n",type_key,edg_type_to_rose_typeof_type_map.find(type_key) != edg_type_to_rose_typeof_type_map.end() ? "true" : "false");
                    printf ("Note: type_key = %p edg_type_to_rose_typeof_type_map.find(type_key) != edg_type_to_rose_typeof_type_map.end() = %s \n",
                         type_key,edg_type_to_rose_typeof_type_map.find(type_key) != edg_type_to_rose_typeof_type_map.end() ? "true" : "false");
                    printf ("Note: expression_key = %p edg_expression_to_rose_typeof_type_map.find(expression_key) != edg_expression_to_rose_typeof_type_map.end() = %s \n",
                         expression_key,edg_expression_to_rose_typeof_type_map.find(expression_key) != edg_expression_to_rose_typeof_type_map.end() ? "true" : "false");
#endif
                  }

               ROSE_ASSERT(local_expr_node_ref_ptr->expr != NULL);

            // DQ (5/20/2015): Found case of hidden expression to typeof operator.
            // EDG 7.0: typeof is represented as lerk_decltype (lerk_typeof no longer exists).
               if (local_expr_node_ref_ptr->kind == lerk_decltype)
                  {
                 // Note that we want to process the EDG expression in local_expr_node_ref_ptr->expr to build 
                 // the ROSE SgExpression subtree to be used in the typeof operator, but we wnat to use the key 
                 // defined by the EDG type local_expr_node_ref_ptr->referrer.ptr (if it is a type and not an 
                 // expresion) to store the typeof operator into the map.
#if 0
                    printf ("In process_possible_typeof_expression_arguments(): Found case of hidden expression to typeof operator: %s \n",
                         summarize_entry_kind_in_string_kind(local_expr_node_ref_ptr->referrer,true).c_str());
                    printf ("In process_possible_typeof_expression_arguments(): Found case of hidden expression to typeof operator: expr = %s \n",
                         summarize_expression_in_string(local_expr_node_ref_ptr->expr).c_str());
#endif
#if 0
                    printf ("In process_possible_typeof_expression_arguments(): calling summarize_expr() \n");
                    summarize_expr(local_expr_node_ref_ptr->expr);
#endif
                    ROSE_ASSERT(local_expr_node_ref_ptr->referrer.kind == iek_type);

                    a_type_ptr edg_type_key = (a_type_ptr)(local_expr_node_ref_ptr->referrer.ptr);
                    ROSE_ASSERT(edg_type_key != NULL);

#if 0
                    printf ("Calling convert_expression(edg_type_key = %p) \n",edg_type_key);
#endif
                    SgExpression* typeof_expr_arg = convert_expression(local_expr_node_ref_ptr->expr);
                    ROSE_ASSERT(typeof_expr_arg != NULL);

#if 0
                    printf ("DONE: Calling convert_expression(edg_type_key = %p) \n",edg_type_key);
#endif
                    SgTypeOfType* typeofOperator = NULL;

                    if (edg_type_to_rose_typeof_type_map.find(edg_type_key) == edg_type_to_rose_typeof_type_map.end())
                       {
                         typeofOperator = SageBuilder::buildTypeOfType(typeof_expr_arg,NULL);
                         ROSE_ASSERT(typeofOperator != NULL);
#if 0
                         printf ("Adding typeofOperator = %p to edg_type_to_rose_typeof_type_map[local_expr_node_ref_ptr->expr = %p] \n",typeofOperator,edg_type_key);
#endif
                         edg_type_to_rose_typeof_type_map[edg_type_key] = typeofOperator;
                       }
                      else
                       {
                         typeofOperator = edg_type_to_rose_typeof_type_map[edg_type_key];
                         ROSE_ASSERT(typeofOperator != NULL);
#if 0
                         printf ("Found typeofOperator = %p in edg_type_to_rose_typeof_type_map using key = %p \n",typeofOperator,edg_type_key);
#endif
                       }
#if 0
                 // Build a typeof operator type using the typeof_expr_arg and put it into the edg_expression_to_rose_typeof_type_map.
                 // Or maybe the edg_type_to_rose_typeof_type_map depending on if the best key to use is an EDG expression or type.
                    if (edg_expression_to_rose_typeof_type_map.find(local_expr_node_ref_ptr->expr) == edg_expression_to_rose_typeof_type_map.end())
                       {
                         typeofOperator = SageBuilder::buildTypeOfType(typeof_expr_arg,NULL);
                         ROSE_ASSERT(typeofOperator != NULL);
#if 0
                         printf ("Adding typeofOperator = %p to edg_expression_to_rose_typeof_type_map[local_expr_node_ref_ptr->expr = %p] \n",typeofOperator,local_expr_node_ref_ptr->expr);
#endif
                         edg_expression_to_rose_typeof_type_map[local_expr_node_ref_ptr->expr] = typeofOperator;
                       }
                      else
                       {
                         typeofOperator = edg_expression_to_rose_typeof_type_map[local_expr_node_ref_ptr->expr];
                         ROSE_ASSERT(typeofOperator != NULL);
#if 0
                         printf ("Found typeofOperator = %p in edg_expression_to_rose_typeof_type_map using key = %p \n",typeofOperator,local_expr_node_ref_ptr->expr);
#endif
                       }
#endif
                  }

               ROSE_ASSERT(local_expr_node_ref_ptr != local_expr_node_ref_ptr->next);

#if DEBUG_TYPEOF_EXPRESSION
               printf ("BASE OF LOOP: local_expr_node_ref       = %p \n",local_expr_node_ref_ptr);
               printf ("BASE OF LOOP: local_expr_node_ref->next = %p \n",local_expr_node_ref_ptr->next);
#endif
               local_expr_node_ref_ptr = local_expr_node_ref_ptr->next;
             }
        }

#if DEBUG_TYPEOF_EXPRESSION
     printf ("Leaving process_possible_typeof_expression_arguments(): BASE of function \n");
#endif
   }

void check_types_and_set_isAssociatedWithDeclarationList( SgVariableDeclaration* last_decl, SgVariableDeclaration* temp_decl )
   {
  // DQ (12/29/2015): Refactored code to support multiple variable declaration per variable declaration.
  // Note that even were we set one variable per declaration we record that the varialbe declarations are
  // associated with a list of variable declarations in the original source code (detected when the variable 
  // declarations are from the same source sequence number).

#define DEBUG_VARIABLE_DECLARATION_TYPE_CHECKING 0

#if DEBUG_VARIABLE_DECLARATION_TYPE_CHECKING
     printf ("In check_types_and_set_isAssociatedWithDeclarationList:\n");
     printf ("  -- last_decl = %p\n", last_decl);
     printf ("  -- temp_decl = %p\n", temp_decl);
#endif

     ROSE_ASSERT(last_decl != NULL);
     ROSE_ASSERT(temp_decl != NULL);

     SgInitializedName* first_var = getFirstInitializedName(last_decl);
     ROSE_ASSERT(first_var != NULL);
     SgInitializedName* last_var = getFirstInitializedName(temp_decl);
     ROSE_ASSERT(last_var != NULL);

#if DEBUG_VARIABLE_DECLARATION_TYPE_CHECKING
     printf ("  -- first_var  = %p\n", first_var);
     printf ("  -- last_var   = %p\n", last_var);
#endif

  // DQ (4/4/2015): We want to have a better test for when we combine types defined in a typedef (see test2015_60.c and test2015_62.c).
     if (last_decl != NULL)
        {
          ROSE_ASSERT(temp_decl != NULL);
          SgType* first_decl_type = first_var->get_type();
          SgType* last_decl_type  = last_var->get_type();
          ROSE_ASSERT(first_decl_type != NULL);
          ROSE_ASSERT(last_decl_type != NULL);
#if 0
       // DQ (6/6/2016): Tracing down use of incorrect typedef alias (inconsistant with original source code) \n");
          printf ("In check_types_and_set_isAssociatedWithDeclarationList(): Calling stripType() \n");
#endif
       // DQ (4/4/2015): We want to avoid striping typedefs.
       // SgType* stripped_type_last_decl_base_type    = last_decl_base_type->stripType(SgType::STRIP_MODIFIER_TYPE|SgType::STRIP_REFERENCE_TYPE|SgType::STRIP_POINTER_TYPE|SgType::STRIP_ARRAY_TYPE|SgType::STRIP_TYPEDEF_TYPE);
       // SgType* stripped_type_local_result_base_type = local_result_base_type->stripType(SgType::STRIP_MODIFIER_TYPE|SgType::STRIP_REFERENCE_TYPE|SgType::STRIP_POINTER_TYPE|SgType::STRIP_ARRAY_TYPE|SgType::STRIP_TYPEDEF_TYPE);
          SgType* stripped_type_last_decl_type  = last_decl_type ->stripType(SgType::STRIP_MODIFIER_TYPE|SgType::STRIP_REFERENCE_TYPE|SgType::STRIP_RVALUE_REFERENCE_TYPE|SgType::STRIP_POINTER_TYPE|SgType::STRIP_ARRAY_TYPE);
          SgType* stripped_type_first_decl_type = first_decl_type->stripType(SgType::STRIP_MODIFIER_TYPE|SgType::STRIP_REFERENCE_TYPE|SgType::STRIP_RVALUE_REFERENCE_TYPE|SgType::STRIP_POINTER_TYPE|SgType::STRIP_ARRAY_TYPE);
          ROSE_ASSERT(stripped_type_first_decl_type != NULL);
          ROSE_ASSERT(stripped_type_last_decl_type != NULL);

          if (stripped_type_last_decl_type == stripped_type_first_decl_type)
             {
#if DEBUG_VARIABLE_DECLARATION_TYPE_CHECKING
               printf ("Identified the SAME types used in typedef declaration for multiple types \n");
#endif
               ROSE_ASSERT(isSgVariableDeclaration(temp_decl) != NULL);
            // isSgVariableDeclaration(temp_decl)->set_isAssociatedWithDeclarationList(true);
               temp_decl->set_isAssociatedWithDeclarationList(true);
               last_decl->set_isAssociatedWithDeclarationList(true);
             }
            else
             {
#if DEBUG_VARIABLE_DECLARATION_TYPE_CHECKING
               printf ("Identified the SEPERATE types used in typedef declaration for multiple types \n");
#endif
             }
        }
   }

#define DEBUG__convert_storage_class 0

SgStorageModifier::storage_modifier_enum convert_storage_class(a_storage_class kind)
   {
#if DEBUG__convert_storage_class
     printf("Enter convert_storage_class()\n");
     printf("  kind = %s\n", db_storage_class_names[kind]);
#endif

     SgStorageModifier::storage_modifier_enum returnValue;
     switch (kind)
        {
          case sc_extern:       returnValue = SgStorageModifier::e_extern;      break;
          case sc_static:       returnValue = SgStorageModifier::e_static;      break;
          case sc_auto:         returnValue = SgStorageModifier::e_auto;        break;
          case sc_unspecified:  returnValue = SgStorageModifier::e_unspecified; break;
          case sc_typedef:      returnValue = SgStorageModifier::e_typedef;     break;
          case sc_register:     returnValue = SgStorageModifier::e_register;    break;
          case sc_asm:          returnValue = SgStorageModifier::e_asm;         break;

          case sc_last:
          default: ERROR_MESSAGE(stderr, "Bad storage modifier %s\n", db_storage_class_names[kind]); ROSE_ASSERT(false);
        }

#if DEBUG__convert_storage_class
     printf("Leave convert_storage_class()\n");
#endif
     return returnValue;
   }

SgScopeStatement* edgScopeOfDeclarationFromEntity(a_tagged_pointer & entity)
   {
  // a_scope_ptr return_scope = NULL;
     SgScopeStatement* return_scope = NULL;

     an_il_entry_kind kind = (an_il_entry_kind)(entity.kind);
     switch(kind)
        {
          case iek_type:
             {
               a_type_ptr type = (a_type_ptr)(entity.ptr);
#if 0
               printf ("In edgScopeOfDeclarationFromEntity(): Handling the case iek_type: type = %p \n",type);
#endif
               return_scope = edgScopeOfDeclarationFromType(type);
               break;
             }

          default:
             {
#if 0
               printf ("Case not handled in edgScopeOfDeclarationFromEntity() \n");
#endif
             }
        }

     return return_scope;
   }

SgScopeStatement* edgScopeOfDeclarationFromType(a_type_ptr & p)
   {
  // a_scope_ptr return_scope = NULL;
     SgScopeStatement* return_scope = NULL;

     a_type_kind kind = p->kind;
     switch(kind)
        {
          case tk_union:
             {
            // bool originally_unnamed = p->variant.class_struct_union.originally_unnamed;
               ROSE_ASSERT(p->variant.class_struct_union.extra_info != NULL);

               a_scope_ptr scope_represented_by_type = p->variant.class_struct_union.extra_info->assoc_scope;
               ROSE_ASSERT(scope_represented_by_type != NULL);
#if 0
               printf ("In edgScopeOfDeclarationFromType(): case tk_union: calling summarize_scope() \n");
               summarize_scope(scope_represented_by_type);
#endif
               if (type_translation_cache.find(p) != type_translation_cache.end())
                  {
                    SgType* existingType = type_translation_cache[p];
                    ROSE_ASSERT(existingType != NULL);
                    SgClassType* classType = isSgClassType(existingType);
                    ROSE_ASSERT(classType != NULL);
                    SgDeclarationStatement* declaration = classType->get_declaration();
                    ROSE_ASSERT(declaration != NULL);
                    SgScopeStatement* rose_scope = declaration->get_scope();
                    ROSE_ASSERT(rose_scope != NULL);
#if 0
                    printf ("In edgScopeOfDeclarationFromType(): case tk_union: rose_scope = %p = %s \n",rose_scope,rose_scope->class_name().c_str());
#endif
                    return_scope = rose_scope;
                  }
                 else
                  {
#if 0
                    printf ("In edgScopeOfDeclarationFromType(): case tk_union: type = %p not found in type_translation_cache \n",p);
#endif
                  }

               break;
             }

          default:
             {
#if 0
               printf ("Case not handled in edgScopeOfDeclarationFromType() p = %p \n",p);
#endif
             }
        }

     return return_scope;
   }

SgStatement* build_basic_block_if_multiple_statements(const vector<SgStatement*>& stmts)
   {
#if 0
     printf ("In build_basic_block_if_multiple_statements(): stmts.size() = %" PRIuPTR " \n",stmts.size());
#endif

     if (stmts.size() == 1)
        {
#if 0
          printf ("In build_basic_block_if_multiple_statements(): returning stmts[0] = %p = %s \n",stmts[0],stmts[0]->class_name().c_str());
#endif

       // DQ (12/31/2014): We want to avoid the normalization of the AST using compiler generated SgBasicBlock IR nodes.
          SgBasicBlock* basicBlock = isSgBasicBlock(stmts[0]);
          if (basicBlock != NULL && basicBlock->isCompilerGenerated() == true)
             {
            // We want to avoid introducing compiler generated SgBasicBlock IR nodes where possible.
#if 0
               printf ("basicBlock->get_statements().size() = %zu \n",basicBlock->get_statements().size());
#endif
               if (basicBlock->get_statements().size() == 1)
                  {
#if 0
                    printf ("In build_basic_block_if_multiple_statements(): detected compiler generated SgBasicBlock: returning basicBlock->get_statements()[0] = %p = %s \n",
                         basicBlock->get_statements()[0],basicBlock->get_statements()[0]->class_name().c_str());
#endif
                 // DQ (12/31/2014): Make sure that there are not any symbols that we would loose track of by ignoring this SgBasicBlock.
                    ROSE_ASSERT(basicBlock->get_symbol_table() != NULL);
                 // ROSE_ASSERT(basicBlock->get_symbol_table()->size() == 0);
                    if (basicBlock->get_symbol_table()->size() != 0)
                       {
#ifdef DEBUG_CONVERSION
                         printf ("WARNING: Ignoring compiler generated wrapper block that has some number of symbols in it's symbol table: basicBlock->get_symbol_table()->size() = %d \n",basicBlock->get_symbol_table()->size());
#endif
                       }

                    return basicBlock->get_statements()[0];
                  }
#if 0
               printf ("Exiting a a test! \n");
               ROSE_ASSERT(false);
#endif
             }
            else
             {
#ifdef DEBUG_CONVERSION
               if (basicBlock != NULL)
                  {
                 // DQ (5/14/2015): For test2015_144.c this is a SgBasicBlock associated with a GNU statement expression.
                    printf ("What is this SgBasicBlock: basicBlock = %p (at least sometimes this is a SgBasicBlock associated with a GNU statement expression) \n",basicBlock);
#if 0
                    basicBlock->get_startOfConstruct()->display("What is this SgBasicBlock: debug");
#endif
                  }
#endif
             }

          return stmts[0];
        }
       else
        {
#if 0
          printf ("In build_basic_block_if_multiple_statements(): found stmts.size() = %zu \n",stmts.size());
#endif
          if (stmts.size() == 2)
             {
#if 0
               printf ("In build_basic_block_if_multiple_statements(): found stmts.size() == 2: check for SgLabelStatement \n");
#endif
               SgLabelStatement* labelStatement = isSgLabelStatement(stmts[0]);
               if (labelStatement != NULL)
                  {
#if 0
                    printf ("In build_basic_block_if_multiple_statements(): found labeled statement as first atement returning stmts[1] = %p = %s \n",stmts[1],stmts[1]->class_name().c_str());
#endif
                    if (labelStatement->get_statement() == stmts[1])
                       {
                      // DQ (7/30/2013): I think we want to return a SgBasicBlock that holds the SgLabelStatement stmts[0] and the associated labelStatement->get_statement() stmts[1].
#if 0
                         printf ("We detected the special case of a SgLabelStatement stmts[0] and an associated labelStatement->get_statement() stmts[1]: but we need to generate a SgBasicBlock to hold them so that we have a simplified CFG handling (see unparseLabelStmt()) \n");
#endif
                       }
                  }
             }
#if 0
          printf ("In build_basic_block_if_multiple_statements(): returning stmts as a SgBasicBlock \n");
          for (size_t i = 0; i < stmts.size(); i++)
             {
               printf ("   --- stmts[%" PRIuPTR "] = %p = %s \n",i,stmts[i],stmts[i]->class_name().c_str());
             }
#endif

       // return set_generated_file_info_to_unparse(buildBasicBlock_nfi(stmts));
          SgBasicBlock* returnBlock = set_generated_file_info_to_unparse(buildBasicBlock_nfi(stmts));
#if 0
          if (stmts.empty() == true)
             {
               printf ("Leaving build_basic_block_if_multiple_statements(): RETURNING EMPTY BLOCK \n");
             }
          printf ("Leaving build_basic_block_if_multiple_statements(): SgBasicBlock returnBlock = %p stmts.size() = %" PRIuPTR " \n",returnBlock,stmts.size());
#endif

          return returnBlock;
        }
  }

void skipOverSourceSequenceEntriesAlreadyVisited(SourceSequenceList& sse, a_statement_ptr p )
   {
  // There are some declarations that can be visited in a slightly different order than the source sequence lists.
  // It is helpful to do this when processing type declarations (defining declarations) that are associated 
  // with a statement (especially for handling of the GNU statement expression).  In this case we visit the 
  // types when we traverse the stmk_decl node in the source sequence list and build the types there (then)
  // so that they will be available when we build the language constructs referencing these types.  Later
  // when they are visited as part of traversing the source sequence lists we want to skip over them.

  // Additional note: Seperate from this function, but related to this is that variable declarations containing 
  // such defining declarations of types can be marked as having an embedded end of construct which must be 
  // detected seperately and not enterpreted as the end of a block.  Since the use of iek_src_seq_end_of_construct
  // is specific to being associated with iek_statement and not iek_variable, I don't think this applies to 
  // it's use in this function (this point effects test2012_130.c).

     while (sse)
        {
#if 0
       // DQ (10/23/2012): This is part of debugging switch statements (case option detection).
          printf ("In skipOverSourceSequenceEntriesAlreadyVisited(): case of iek_statement: loop to iek_src_seq_end_of_construct entry_kind = %p = %s \n",
               sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
#if 0
       // DQ (8/17/2012): I think that the source position of the closing scope is contained in here (so we don't want to ignore these).
          if (sse->entity.kind == iek_src_seq_end_of_construct)
             {
               printf ("@@@@@@@@@@@@@@@@@ In skipOverSourceSequenceEntriesAlreadyVisited(): found iek_src_seq_end_of_construct kind in case iek_statement \n");
             }
#endif
          if (sse->entity.kind == iek_src_seq_end_of_construct &&
               ((a_src_seq_end_of_construct_ptr)(sse->entity.ptr))->entity.kind == iek_statement &&
               ((a_src_seq_end_of_construct_ptr)(sse->entity.ptr))->entity.ptr == (char*)p)
             {
#if 0
               printf ("Breaking out of loop for p = %p = %s \n",p,summarize_statement_in_string(p));
#endif
#if 0
            // This is how we identify what block this might be the end of so that we can avoid incrementing the sse past the relevant block (see test2012_15.c).
               printf ("   --- breaking out of while loop: entry = %p kind = %s \n",
                    ((a_src_seq_end_of_construct_ptr)(sse->entity.ptr))->entity.ptr,summarize_entry_kind_in_string_kind(((a_src_seq_end_of_construct_ptr)(sse->entity.ptr))->entity).c_str());

               a_statement_ptr statement = (a_statement_ptr)((a_src_seq_end_of_construct_ptr)(sse->entity.ptr))->entity.ptr;

               printf ("In skipOverSourceSequenceEntriesAlreadyVisited(): calling summarize_statement(statement = %p) \n",statement);
               summarize_statement(statement);

               if (statement->kind == stmk_block)
                  {
                    a_block_ptr block = (a_block_ptr)(statement->variant.block.extra_info);
                    ROSE_ASSERT(block != NULL);

                    printf ("In skipOverSourceSequenceEntriesAlreadyVisited(): calling summarize_block(block = %p) \n",block);
                    summarize_block(block);
                  }
                 else
                  {
                    printf ("This was not a block %d stmk_block = %d \n",(int)statement->kind,(int)stmk_block);
                  }
#endif
               break;
             }
            else
             {
#if 0
               printf ("Don't break out of loop yet... \n");
#endif
#if 0
               if (sse->entity.kind == iek_src_seq_end_of_construct)
                  {
                 // This is how we identify what block this might be the end of so that we can avoid incrementing the sse past the relevant block (see test2012_15.c).
                    printf ("   --- case iek_src_seq_end_of_construct: of Don't break out of loop yet: entry = %p kind = %s \n",
                         ((a_src_seq_end_of_construct_ptr)(sse->entity.ptr))->entity.ptr,summarize_entry_kind_in_string_kind(((a_src_seq_end_of_construct_ptr)(sse->entity.ptr))->entity).c_str());
                  }
#endif
             }

          ++sse;
        }

     ROSE_ASSERT (sse);
     ++sse;
   }


void testAndSkipOverPreviouslyVisitedSourceSequenceEntries ( SourceSequenceList & sse )
   {
  // DQ (9/9/2013): This is a re-try and fixing the problem of GNU statement expressions with type declarations 
  // that cause extra elements to be added to the source sequence list.

#ifdef DEBUG_CONVERSION
     printf ("In testAndSkipOverPreviouslyVisitedSourceSequenceEntries(): SKIPPING THIS FUNCTION: entry = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif

#if 0
     printf ("In testAndSkipOverPreviouslyVisitedSourceSequenceEntries(): SKIPPING THIS FUNCTION! (return) \n");
#endif

     return;
   }

void skip_over_previously_processed_blocks(SourceSequenceList & sse)
   {
#if 0
     printf ("Top of skip_over_previously_processed_blocks(): sse = %p = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif

     bool status = skip_over_previously_processed_blocks_support(sse);

#if 0
     printf ("After initial call to skip_over_previously_processed_blocks_support(): status = %s sse = %p = %s \n",
          (status == true) ? "true" : "false",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif

     while (status == true)
        {
#if 0
          printf ("In loop calling skip_over_previously_processed_blocks_support(): sse = %p = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
          status = skip_over_previously_processed_blocks_support(sse);
        }

#if 0
     printf ("Base of skip_over_previously_processed_blocks(): sse = %p = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
   }

bool skip_over_previously_processed_blocks_support(SourceSequenceList & sse)
   {
  // DQ (3/23/2015): Refactoring of code to make it callable from elsewhere (test2015_14.c demonstrates the need for this to be more general).
  // The functions's return value is TRUE if something was skipped, and FALSE is nothing was skipped.

  // DQ (3/21/2015): Test test2015_23.c demonstrates a GNU statement expression that is processed in the 
  // for statement initializer.  This has to be skipped over since it was already processed (to get the 
  // sse in sync with the processed AST).
     an_il_entry_kind tmp_kind = (an_il_entry_kind)(sse->entity.kind);
     char* entity_ptr = sse->entity.ptr;

     bool return_status = false;

#if 0
     printf ("Testing for stmk_block between the for loop header and the for loop body: \n");
     printf ("   --- sse entry                     = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
     a_statement_ptr statement = (a_statement_ptr)(sse->entity.ptr);
     if (tmp_kind == iek_statement && statement->kind == stmk_block)
        {
          char* associated_block_entity_ptr = entity_ptr;
#if 0
          printf ("   --- tmp_kind == stmk_block: associated_block_entity_ptr = %p \n",associated_block_entity_ptr);
#endif

#if 0
          printf ("In skip_over_previously_processed_blocks_support(): case stmk_for: has this body stmt_ptr = %p been seen previously = %s (in statement_expression_bodies) \n",
          p,statement_expression_bodies.find(statement) != statement_expression_bodies.end() ? "true" : "false");
#endif
       // DQ (3/22/2015): test2015_27.c demonstrates that we need to test that this is a previously processed EDG stmt_block node (and only then skip it).
          if (statement_expression_bodies.find(statement) != statement_expression_bodies.end())
             {
            // DQ (3/6/2017): Converted to use message logging.
            // If this block has been previously processed then skip past it in the source sequence list.
               mprintf ("This block HAS been previously processed \n");
               while (entity_ptr != NULL)
                  {
#if 0
                    printf ("Advance sse in processing of for loop: (before ++sse): sse entry = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
                    ++sse;

                    return_status = true;

                    an_il_entry_kind local_tmp_kind = (an_il_entry_kind)(sse->entity.kind);
                    if (local_tmp_kind == iek_src_seq_end_of_construct)
                       {
                         a_src_seq_end_of_construct_ptr src_seq_end_of_construct = (a_src_seq_end_of_construct_ptr)(sse->entity.ptr);
                         if (src_seq_end_of_construct->entity.ptr != associated_block_entity_ptr)
                            {
                              entity_ptr = sse->entity.ptr;
#if 0
                              printf ("   --- (after ++sse (local_tmp_kind == iek_src_seq_end_of_construct)): sse entry = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
                            }
                           else
                            {
#if 0
                              printf ("   --- Found associated end of block \n");
#endif
                              entity_ptr = NULL;
                              ++sse;
                            }
                       }
                      else
                       {
                         if (sse->entity.kind == iek_statement)
                            {
#if 0
                              a_statement_ptr associated_statement = (a_statement_ptr)(sse->entity.ptr);
                              printf ("local_tmp_kind != iek_src_seq_end_of_construct: Calling summarize_statement() associated_statement = %p \n",associated_statement);
                              summarize_statement(associated_statement);
#endif
                            }
                           else
                            {
#if 0
                              printf ("   --- (sse->entity.kind != iek_statement): sse entry = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
                            }

                         entity_ptr = sse->entity.ptr;
#if 0
                         printf ("   --- (after ++sse): sse entry = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
                       }
                  }
             }
            else
             {
#if 0
               printf ("This block has been NOT been previously processed (so process the source sequence list) \n");
#endif
             }
        }
       else
        {
#if 0
          printf ("   --- not a stmk_block: sse entry = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
        }

     return return_status;
   }


void updateSwitchSupportMap(a_source_sequence_entry_ptr source_sequence_entry, SgStatement* firstStatementInList)
   {
#define DEBUG_UPDATE_SWITCH_SUPPORT_MAP 0

#if DEBUG_UPDATE_SWITCH_SUPPORT_MAP
     printf ("TOP of updateSwitchSupportMap(): source_sequence_entry = %p firstStatementInList = %p \n",source_sequence_entry,firstStatementInList);
#endif

  // DQ (2/9/2013): Record all of the generated statements into the switch support map.
     if (firstStatementInList != NULL && switchSupportStackOfMaps.empty() == false)
        {
          map<a_source_sequence_entry_ptr,SgStatement*>* switchSupportMap = switchSupportStackOfMaps.back();
          ROSE_ASSERT(switchSupportMap != NULL);

       // vector<SgStatement*>::iterator result_iterator_single = result.begin();
       // if (result.size() == 1)
             {
            // ROSE_ASSERT(parse_statement_function_source_sequence_entry != NULL);

               if (source_sequence_entry != NULL)
                  {
                    map<a_source_sequence_entry_ptr,SgStatement*>::iterator mapEntry = switchSupportMap->find(source_sequence_entry);

                 // ROSE_ASSERT(mapEntry != switchSupportMap->end());
                    if (mapEntry != switchSupportMap->end())
                       {
                      // ROSE_ASSERT(mapEntry->second == NULL);
                         if (mapEntry->second != NULL)
                            {
#if DEBUG_UPDATE_SWITCH_SUPPORT_MAP
                              printf ("We are resetting the ROSE statement entry in the switchSupportMap mapEntry->second = %p = %s firstStatementInList = %p = %s \n",
                                   mapEntry->second,mapEntry->second->class_name().c_str(),firstStatementInList,firstStatementInList->class_name().c_str());
#endif
                            }

                      // ROSE_ASSERT((*result_iterator_single) != NULL);
                      // mapEntry->second = *result_iterator_single;
                         ROSE_ASSERT(firstStatementInList != NULL);
                         mapEntry->second = firstStatementInList;
                       }
                      else
                       {
                      // I think that any nested block of statements in a switch (not just the Duff's Device examples) can take this branch.
#if DEBUG_UPDATE_SWITCH_SUPPORT_MAP
                         printf ("Leaving updateSwitchSupportMap(): add this edg statement to ROSE statement mapping: parse_statement_function_source_sequence_entry = %p firstStatementInList = %p = %s \n",
                              source_sequence_entry,firstStatementInList,firstStatementInList->class_name().c_str());
#endif
                      // (*switchSupportMap)[source_sequence_entry] = *result_iterator_single;
                         (*switchSupportMap)[source_sequence_entry] = firstStatementInList;
#if DEBUG_UPDATE_SWITCH_SUPPORT_MAP
                         printf ("Leaving updateSwitchSupportMap(): switchSupportMap->size() = %" PRIuPTR " \n",switchSupportMap->size());
#endif
                       }
                  }
                 else
                  {
#if 0
                    printf ("ERROR: parse_statement_function_source_sequence_entry == NULL: generated statements are: \n");
                    vector<SgStatement*>::iterator result_iterator = result.begin();
                    while (result_iterator != result.end())
                       {
                         printf ("   --- result contains: %p = %s \n",*result_iterator,(*result_iterator)->class_name().c_str());
                         result_iterator++;
                       }
#endif
                    printf ("Warning: In updateSwitchSupportMap(): firstStatementInList = %p input source_sequence_entry == NULL \n",firstStatementInList);
#if 0
                    printf ("Exiting since we need to set source_sequence_entry properly \n");
                    ROSE_ASSERT(false);
#endif
                  }
             }
        }
   }


SgClassDeclaration::class_types convert_class_kind( a_type_kind kind )
   {
  // Set this to a default value.
     SgClassDeclaration::class_types result = SgClassDeclaration::e_class;

     switch (kind)
        {
          case tk_class:  result = SgClassDeclaration::e_class;  break;
          case tk_struct: result = SgClassDeclaration::e_struct; break;
          case tk_union:  result = SgClassDeclaration::e_union;  break;

          default:
             {
               printf ("Error: default reached in convert_class_type(kind = %d) \n",kind);
               ROSE_ASSERT(false);
             }
        }

     return result;
   }

SgScopeStatement * findAssociatedClassDeclaration(a_field_ptr field_entry)
   {
     a_scope_ptr edg_parent_scope = field_entry->source_corresp.parent_scope;
     ROSE_ASSERT(edg_parent_scope != NULL);

#if 0
     printf ("In findAssociatedClassDeclaration(): calling summarize_scope() edg_parent_scope = %p \n",edg_parent_scope);
     summarize_scope(edg_parent_scope);
#endif

     ROSE_ASSERT(edg_parent_scope->kind == sck_class_struct_union);

     a_type_ptr edg_type = edg_parent_scope->variant.assoc_type;
     ROSE_ASSERT(edg_type != NULL);

     if (scope_cache.find(edg_parent_scope) == scope_cache.end())
        {
       // This could trigger to generation of the defining declaration for the class.
#if 0
          printf ("This could trigger to generation of the defining declaration for the class \n");
#endif
          ROSE_ASSERT(type_translation_cache.find(edg_type) != type_translation_cache.end());

       // SgClassDeclaration* secondaryClassDeclaration = NULL;
          SgType* rose_type = type_translation_cache[edg_type];
          ROSE_ASSERT(rose_type != NULL);

          SgClassType* rose_class_type = isSgClassType(rose_type);
          ROSE_ASSERT(rose_class_type != NULL);

          SgClassDeclaration* secondaryClassDeclaration = isSgClassDeclaration(rose_class_type->get_declaration());
          ROSE_ASSERT(secondaryClassDeclaration != NULL);
#if 0
          printf ("secondaryClassDeclaration->get_definingDeclaration() = %p \n",secondaryClassDeclaration->get_definingDeclaration());
#endif
#if 1
       // DQ (11/5/2020): This is the original code.
          ROSE_ASSERT(secondaryClassDeclaration->get_definingDeclaration() == NULL);
#else
       // DQ (11/4/2020): If the primary declaration is available, e.g. reading the same file twice, as a few tools do, then reuse it.
          if (secondaryClassDeclaration->get_definingDeclaration() != NULL)
             {
               printf ("NOTE: In findAssociatedClassDeclaration(): secondaryClassDeclaration->get_definingDeclaration() != NULL (see tool bug_84.cpp) \n");

            // DQ (11/5/2020): Set this to NULL, this happens when reading the same file twice (which some tools do to generate a dynamic library version).
               secondaryClassDeclaration->set_definingDeclaration(NULL);
             }
          ROSE_ASSERT(secondaryClassDeclaration->get_definingDeclaration() == NULL);
#endif
#if 0
          printf ("In findAssociatedClassDeclarationt(): case iek_field: calling convert_struct_primary() (building a union) \n");
#endif
#if 0
          printf ("In findAssociatedClassDeclaration(): calling summarize_type() edg_type = %p \n",edg_type);
          summarize_type(edg_type);
#endif
       // SgDeclarationStatement* primaryClassDeclaration = convert_struct_primary (edg_type, SgClassDeclaration::e_union, secondaryClassDeclaration);
          SgClassDeclaration::class_types rose_class_kind = convert_class_kind(edg_type->kind);

#if 0
       // DQ (10/11/2015): This is the original code, which was a mistake when the secondaryClassDeclaration was a SgTemplateClassDeclaration.
          SgDeclarationStatement* primaryClassDeclaration = convert_struct_primary (edg_type, rose_class_kind, secondaryClassDeclaration);
          ROSE_ASSERT(primaryClassDeclaration != NULL);
#else
       // DQ (11/5/2020): Switch this code back, but still disable the assertion above.
       // DQ (11/4/2020): If the primary declaration is available (e.g. reading the same file twice, as a few tools do, then reuse it.
       // DQ (10/11/2015): This is the new code, which was a mistake when the secondaryClassDeclaration was a SgTemplateClassDeclaration.
       // SgDeclarationStatement* primaryClassDeclaration = NULL;
       // SgDeclarationStatement* primaryClassDeclaration = secondaryClassDeclaration->get_definingDeclaration();
          SgDeclarationStatement* primaryClassDeclaration = NULL;

          SgTemplateClassDeclaration* secondaryTemplateClassDeclaration = isSgTemplateClassDeclaration(secondaryClassDeclaration);
          if (secondaryTemplateClassDeclaration != NULL)
             {
               mprintf ("In findAssociatedClassDeclarationt(): Need to build a template declaration \n");

            // primaryClassDeclaration = convert_struct_primary (edg_type, rose_class_kind, secondaryClassDeclaration);
               a_template_ptr edg_template = NULL;
            // SgTemplateClassDeclaration* primaryTemplateClassDeclaration = convert_template_struct_primary(edg_type,rose_class_kind,secondaryClassDeclaration,edg_template);

            // DQ (11/5/2020): Switch this code back, but still disable the assertion above.
            // DQ (11/4/2020): If the primary declaration is available (e.g. reading the same file twice, as a few tools do, then reuse it.
#if 1
               primaryClassDeclaration = convert_template_struct_primary(edg_type,rose_class_kind,secondaryTemplateClassDeclaration,edg_template);
#else
               if (primaryClassDeclaration == NULL)
                  {
                    primaryClassDeclaration = convert_template_struct_primary(edg_type,rose_class_kind,secondaryTemplateClassDeclaration,edg_template);
                  }
#endif
               ROSE_ASSERT(primaryClassDeclaration != NULL);
               ROSE_ASSERT(isSgTemplateClassDeclaration(primaryClassDeclaration) != NULL);
             }
            else
             {
               primaryClassDeclaration = convert_struct_primary (edg_type, rose_class_kind, secondaryClassDeclaration);
               ROSE_ASSERT(primaryClassDeclaration != NULL);
             }
#endif
          ROSE_ASSERT(primaryClassDeclaration != NULL);

#if 0
          printf ("In findAssociatedClassDeclaration(): detected need for defining declaration: generated primaryClassDeclaration = %p = %s \n",primaryClassDeclaration,primaryClassDeclaration->class_name().c_str());
#endif
        }

     ROSE_ASSERT(scope_cache.find(edg_parent_scope) != scope_cache.end());
     SgScopeStatement* associatedScope = scope_cache[edg_parent_scope];
     ROSE_ASSERT(associatedScope != NULL);
#if 0
     printf ("associatedScope = %p = %s \n",associatedScope,associatedScope->class_name().c_str());
#endif
     SgClassDefinition* classDefinition = isSgClassDefinition(associatedScope);
     ROSE_ASSERT(classDefinition != NULL);

     return classDefinition;
   }

void process_static_local_variable_initializers(a_scope_ptr scope) {
#ifdef DEBUG_CONVERSION
     indent_line(); debug_print("process_static_local_variable_initializers\n");
     ScopedIndent si(indent);
#endif

     if (scope == NULL)
        {
       // Not a scope
#ifdef DEBUG_CONVERSION
          indent_line(); debug_print("not a scope\n");
#endif
          return;
        }

     for (a_local_static_variable_init_ptr inits = scope->local_static_variable_inits; inits != NULL; inits = inits->next)
        {
          ROSE_ASSERT(inits->variable->source_corresp.name != NULL);
#ifdef DEBUG_CONVERSION
          indent_line(); debug_print("processing var %s\n", inits->variable->source_corresp.name);
#endif
          SgVariableSymbol* sym = convert_variable_use(inits->variable);
          SgInitializedName* var = sym->get_declaration();

          ROSE_ASSERT (var != NULL);

       // DQ (9/4/2013): I think we can enfoce this here (else we should be maybe building compound literals).
          ROSE_ASSERT (inits->variable->is_compound_literal == false);

          var->set_initializer(convert_initializer(inits->init_kind, inits->initializer));
          var->get_initializer()->set_parent(var);
        }
   }

a_statement_ptr last_statement_in_list(a_statement_ptr p)
   {
  // This skips over compiler-generated statements

#if 0
     printf ("In last_statement_in_list(p = %p) \n",p);
#endif

     a_statement_ptr last = NULL;
     for (; p != NULL; p = p->next)
        {
#if 0
          printf ("   --- In last_statement_in_list() in loop p = %p kind = %s (p->next = %p) \n",p,summarize_statement_in_string(p),p->next);
#endif
          if (p->source_sequence_entry != NULL)
             {
               last = p;
             }
            else
             {
            // DQ (12/26/2013): where these are declarations in the source sequence list, we need to iterate to the end of the list of them.
#ifdef DEBUG_CONVERSION
               printf("   --- In last_statement_in_list(): Skipping over compiler-generated statements: p = %p kind = %s (p->next = %p) \n",p,summarize_statement_in_string(p),p->next);
#endif
             }
        }

#if 0
     printf ("In last_statement_in_list() last = %p \n",last);
#endif

     return last;
   }

bool has_end_of_block_terminator(SourceSequenceList& sse, a_statement_ptr enclosing_block )
   {
  // This function searches along the source sequence list to find the iek_src_seq_end_of_construct that 
  // matches it with the current block.  This is for testing, at leat for now, since it is ineffiecient to
  // do this search.  Some blocks do not have a terminating iek_src_seq_end_of_construct in the EDG IR, so
  // this is a problem.  Hopefully we can find a better way to detect these blocks that have no terminating
  // iek_src_seq_end_of_construct, since they need special handling.

     bool return_value = false;

#define DEBUG_HAS_END_OF_BLOCK_TERMINATOR 0

  // Make a copy of the SourceSequenceList so that we can iterate upon it independently.
     SourceSequenceList local_sse = sse;

     if (enclosing_block == NULL)
        {
#if DEBUG_HAS_END_OF_BLOCK_TERMINATOR
          printf ("WARNING: In has_end_of_block_terminator(): enclosing_block == NULL: returning false \n");
#endif
          return false;
        }

     ROSE_ASSERT(enclosing_block != NULL);
     ROSE_ASSERT(enclosing_block->kind == stmk_block);

     a_statement_ptr s = enclosing_block->variant.block.statements;

#if DEBUG_HAS_END_OF_BLOCK_TERMINATOR
     a_statement_ptr last_stmt = enclosing_block->variant.block.statements;
#endif

     int counter = 0;

#if DEBUG_HAS_END_OF_BLOCK_TERMINATOR
     printf ("Statement list for enclosing_block = %p \n",enclosing_block);
#endif
     while(s != NULL)
        {
#if DEBUG_HAS_END_OF_BLOCK_TERMINATOR
          printf ("   -- counter = %d statement = %p \n",counter,s);
#endif
          counter++;

#if DEBUG_HAS_END_OF_BLOCK_TERMINATOR
          last_stmt = s;
#endif

          s = s->next;
        }

#if DEBUG_HAS_END_OF_BLOCK_TERMINATOR
     printf ("In has_end_of_block_terminator(): last_stmt = %p \n",last_stmt);
#endif

     an_il_entry_kind kind = (an_il_entry_kind)(local_sse->entity.kind);
     std::vector<a_statement_ptr> endOfConstructList;

#if DEBUG_HAS_END_OF_BLOCK_TERMINATOR
     printf ("Available end_of_construct entities: \n");
#endif

     while (local_sse)
        {
          kind = (an_il_entry_kind)(local_sse->entity.kind);

#if DEBUG_HAS_END_OF_BLOCK_TERMINATOR
          printf ("   --- local_sse is current entry kind = %p = %s \n",local_sse->entity.ptr,summarize_entry_kind_in_string_kind(local_sse->entity,true).c_str());
#endif
          if (kind == iek_src_seq_end_of_construct)
             {
            // a_tagged_pointer & entity = local_sse->entity;
#if DEBUG_HAS_END_OF_BLOCK_TERMINATOR
               printf ("   --- local_sse->entity = %p = %s \n",local_sse->entity.ptr,summarize_entry_kind_in_string_kind(local_sse->entity,true).c_str());
#endif
               a_src_seq_end_of_construct_ptr end_of_construct = (a_src_seq_end_of_construct_ptr)local_sse->entity.ptr;
               ROSE_ASSERT(end_of_construct != NULL);

#if DEBUG_HAS_END_OF_BLOCK_TERMINATOR
               printf ("   --- end_of_construct = %p \n",end_of_construct);

               printf ("   --- end_of_construct->entity.kind = %" PRIuPTR " \n",(size_t)end_of_construct->entity.kind);
               printf ("   --- stmk_block = %" PRIuPTR " \n",(size_t)stmk_block);
               printf ("   --- iek_src_seq_end_of_construct = %" PRIuPTR " \n",(size_t)iek_src_seq_end_of_construct);

               printf ("summarize_entry_kind_in_string_kind((an_il_entry_kind)stmk_block) = %s \n",summarize_entry_kind_in_string_kind((an_il_entry_kind)stmk_block));
               printf ("summarize_entry_kind_in_string_kind(iek_src_seq_end_of_construct) = %s \n",summarize_entry_kind_in_string_kind(iek_src_seq_end_of_construct));
               printf ("summarize_entry_kind_in_string_kind(end_of_construct->entity.kind) = %s \n",summarize_entry_kind_in_string_kind((an_il_entry_kind)end_of_construct->entity.kind));

               printf ("   --- (before switch) end_of_construct->entity = %p = %s \n",end_of_construct->entity.ptr,summarize_entry_kind_in_string_kind(end_of_construct->entity).c_str());
#endif
               switch (end_of_construct->entity.kind)
                  {
                 // DQ (1/17/2014): I think that this can be more than just a case iek_statement: case stmk_block, 
                 // but we may have to trap these special cases to get a more complete list.
                    case iek_statement:
                       {
#if DEBUG_HAS_END_OF_BLOCK_TERMINATOR
                         printf ("Detected case iek_statement: \n");
#endif
                         a_statement_ptr statement = (a_statement_ptr)end_of_construct->entity.ptr;

                         switch (statement->kind)
                            {
                              case stmk_block:
                                 {
#if DEBUG_HAS_END_OF_BLOCK_TERMINATOR
                                   printf ("In has_end_of_block_terminator(): case stmk_block: enclosing_block = %p local_sse->entity.ptr = %p \n",enclosing_block,end_of_construct->entity.ptr);
#endif
                                // if (enclosing_block == (a_statement_ptr)local_sse->entity.ptr)
                                   if (enclosing_block == (a_statement_ptr)end_of_construct->entity.ptr)
                                      {
#if DEBUG_HAS_END_OF_BLOCK_TERMINATOR
                                        printf ("Identified enclosing_block associated end_of_construct \n");
#endif
                                        return_value = true;
                                      }
                                   break;
                                 }

                              case stmk_decl:
                                 {
#if DEBUG_HAS_END_OF_BLOCK_TERMINATOR
                                   printf ("In has_end_of_block_terminator(): case stmk_decl: enclosing_block = %p local_sse->entity.ptr = %p \n",enclosing_block,end_of_construct->entity.ptr);
#endif
                                   if (enclosing_block == (a_statement_ptr)end_of_construct->entity.ptr)
                                      {
#if DEBUG_HAS_END_OF_BLOCK_TERMINATOR
                                        printf ("Identified enclosing_block associated end_of_construct \n");
#endif
                                        return_value = true;
                                      }
                                   break;
                                 }
                              case stmk_end_test_while:
                                 {
#if 0
                                   printf ("Supporting new case for EDG 4.8: case iek_statement: case stmk_end_test_while \n");
#endif
#if DEBUG_HAS_END_OF_BLOCK_TERMINATOR
                                   printf ("In has_end_of_block_terminator(): case stmk_stmk_end_test_while: enclosing_block = %p local_sse->entity.ptr = %p \n",enclosing_block,end_of_construct->entity.ptr);
#endif
                                   if (enclosing_block == (a_statement_ptr)end_of_construct->entity.ptr)
                                      {
#if DEBUG_HAS_END_OF_BLOCK_TERMINATOR
                                        printf ("Identified enclosing_block associated end_of_construct \n");
#endif
                                        return_value = true;
                                      }
                                   break;
                                 }
                              default:
                                 {
                                   printf ("default reached in inner switch: end_of_construct->entity.kind = %d end_of_construct->entity not handled: entity = %s \n",end_of_construct->entity.kind,summarize_entry_kind_in_string_kind(end_of_construct->entity).c_str());
                                 }
                            }
                         break;
                       }

                    case iek_type:
                       {
#if DEBUG_HAS_END_OF_BLOCK_TERMINATOR
                         printf ("Detected case iek_type: \n");
#endif
                         a_type_ptr type = (a_type_ptr)end_of_construct->entity.ptr;

                         switch (type->kind)
                            {
                              case tk_class:
                              case tk_struct:
                              case tk_union:
                                 {
#if DEBUG_HAS_END_OF_BLOCK_TERMINATOR
                                   printf ("Detected case tk_union: \n");
#endif
                                   if (enclosing_block == (a_statement_ptr)end_of_construct->entity.ptr)
                                      {
#if DEBUG_HAS_END_OF_BLOCK_TERMINATOR
                                        printf ("Identified enclosing_block associated end_of_construct \n");
#endif
                                        return_value = true;
                                      }
                                   break;
                                 }

                           // DQ (1/17/2014): This happens in grep
                              case tk_routine:

                           // DQ (1/17/2014): This happens in grep and xercesc
                              case tk_integer:
                                 {
#if DEBUG_HAS_END_OF_BLOCK_TERMINATOR
                                   printf ("Detected case tk_integer: \n");
#endif
                                   if (enclosing_block == (a_statement_ptr)end_of_construct->entity.ptr)
                                      {
#if DEBUG_HAS_END_OF_BLOCK_TERMINATOR
                                        printf ("Identified enclosing_block associated end_of_construct \n");
#endif
                                        return_value = true;
                                      }
                                   break;
                                 }
                           // DQ (3/30/2017): Adding support to handle test2017_02.c C language test code.
                           // Note: this happens in MySql application.
                              case tk_typeref:
                                 {
                                   if (enclosing_block == (a_statement_ptr)end_of_construct->entity.ptr)
                                      {
#if 1
                                        printf ("Identified enclosing_block associated end_of_construct \n");
#endif
                                        return_value = true;
                                      }
                                   break;
                                 }
                              default:
                                 {
                                   printf ("default reached in inner switch: case iek_type: end_of_construct->entity.kind = %d end_of_construct->entity not handled: entity = %s \n",end_of_construct->entity.kind,summarize_entry_kind_in_string_kind(end_of_construct->entity).c_str());
                                // DQ (1/17/2014): Disable this from being an error.
                                // ROSE_ASSERT(false);
                                   break;
                                 }
                               
                            }
                         break;
                       }

                 // DQ (1/17/2014): This happens in cherokee
                    case iek_variable:
                       {
#if DEBUG_HAS_END_OF_BLOCK_TERMINATOR
                         printf ("Detected case iek_variable: \n");
#endif
                      // a_variable_ptr variable = (a_variable_ptr)end_of_construct->entity.ptr;

                         if (enclosing_block == (a_statement_ptr)end_of_construct->entity.ptr)
                            {
#if DEBUG_HAS_END_OF_BLOCK_TERMINATOR
                              printf ("Identified enclosing_block associated end_of_construct \n");
#endif
                              return_value = true;
                            }
                         break;
                       }

                    default:
                       {
                         printf ("default reached in has_end_of_block_terminator(): end_of_construct->entity.kind = %d end_of_construct->entity not handled: entity = %s \n",end_of_construct->entity.kind,summarize_entry_kind_in_string_kind(end_of_construct->entity).c_str());
                       }
                  }
             }

          ++local_sse;
        }

#if DEBUG_HAS_END_OF_BLOCK_TERMINATOR
     printf ("Leaving has_end_of_block_terminator(): return_value = %s \n",return_value ? "true" : "false");
#endif

     return return_value;
   }


#define DEBUG_RECORD_BLOCK_SEQUENCE_ENTRIES 0

void record_block_source_sequence_entries_for_statements (SourceSequenceList & sse, vector<a_source_sequence_entry_ptr> & block_source_sequence_perspective)
   {
#if DEBUG_RECORD_BLOCK_SEQUENCE_ENTRIES
     printf ("##### In record_block_source_sequence_entries_for_statements(): sse.value() = %p \n",sse.value());
     printf ("##### In record_block_source_sequence_entries_for_statements(): sse = %p = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
     printf ("##### In record_block_source_sequence_entries_for_statements(): block_source_sequence_perspective.size() = %zu \n",block_source_sequence_perspective.size());
#endif

     a_statement_ptr statement = (sse->entity.kind == iek_statement) ? (a_statement_ptr)(sse->entity.ptr) : NULL;

#if DEBUG_RECORD_BLOCK_SEQUENCE_ENTRIES
     if (statement != NULL)
        {
          printf ("In record_block_source_sequence_entries_for_statements(): This is a statement! \n");
          if (statement->kind == stmk_block)
             {
               printf ("In record_block_source_sequence_entries_for_statements(): Calling summerize_statement() \n");
               summarize_statement(statement);
             }

          if (statement->kind == stmk_label)
             {
               printf ("In record_block_source_sequence_entries_for_statements(): Calling summerize_statement() \n");
               summarize_statement(statement);
             }
        }
       else
        {
          a_type_ptr type_statement = (sse->entity.kind == iek_type) ? (a_type_ptr)(sse->entity.ptr) : NULL;
          if ( (type_statement != NULL && type_statement->kind == tk_typeref))
             {
               printf ("In record_block_source_sequence_entries_for_statements(): This is a typedef statement! \n");
             }
        }
#endif

  // DQ (1/2/2018): We need to ignore statement blocks that are in fact statement expressions. This is only defined for GNU, and not Windows mode.
#if GNU_EXTENSIONS_ALLOWED
     bool is_statement_expression = ( (statement != NULL) && (statement->kind == stmk_block) && statement->variant.block.extra_info->is_statement_expression == true) ? true : false;
#else
     bool is_statement_expression = false;
#endif

#if DEBUG_RECORD_BLOCK_SEQUENCE_ENTRIES
     printf ("In record_block_source_sequence_entries_for_statements(): is_statement_expression = %s \n",is_statement_expression ? "true" : "false");
#endif

  // DQ (12/26/2017): We want to permit typedefs to have case labels as well (valid for C++, but not for C code), see test2017_97.C and test2017_21.c.
     if (statement != NULL && is_statement_expression == false)
        {
          block_source_sequence_perspective.push_back((a_source_sequence_entry_ptr)(sse.value()));

       // DQ (1/2/2018): Need to handle the case of a label explicitly.
          if (statement->kind == stmk_label)
             {
               SourceSequenceList sse_body = sse;
               ++sse_body;

#if DEBUG_RECORD_BLOCK_SEQUENCE_ENTRIES
               printf ("In record_block_source_sequence_entries_for_statements(): recursive call \n");
#endif
               record_block_source_sequence_entries_for_statements(sse_body,block_source_sequence_perspective);
             }
        }
       else
        {
#if DEBUG_RECORD_BLOCK_SEQUENCE_ENTRIES
          printf ("Skipping over case of stmk_decl \n");
#endif
        }

#if DEBUG_RECORD_BLOCK_SEQUENCE_ENTRIES
     printf ("##### In record_block_source_sequence_entries_for_statements(): block_source_sequence_perspective.size() = %zu \n",block_source_sequence_perspective.size());
#endif
   }

bool is_unnamed(a_type_ptr p)
   {
     if (p->kind == tk_integer)
        {
          return p->variant.integer.originally_unnamed;
        }

     if (p->kind == tk_class || p->kind == tk_struct || p->kind == tk_union)
        {
          return p->variant.class_struct_union.originally_unnamed;
        }

     return false;
   }

a_type_ptr contains_unnamed_type_at_top_level_ptr_to_member(a_type_ptr p)
   {
  // This has to return an EDG type pointer, so it can't call the convert_ptr_to_member_type() function.
     a_type_ptr returnPointer = NULL; // convert_ptr_to_member_type(p);

  // ROSE_ASSERT (!"tk_ptr_to_member not supported in contains_unnamed_type_at_top_level_ptr_to_member");

#ifdef DEBUG_CONVERSION
     printf ("Note: contains_unnamed_type_at_top_level_ptr_to_member() returning NULL \n");
#endif

     return returnPointer;
   }

a_type_ptr contains_unnamed_type_at_top_level(a_type_ptr p)
   {
  // If there is an unnamed type at the "top level" of p (i.e., either directly
  // or within a pointer or array or typeref), return it; otherwise, return NULL

     ROSE_ASSERT (p != NULL);

#if 0
     printf ("In contains_unnamed_type_at_top_level(): a_type_ptr p = %p \n",p);
#endif

  // I think tha only a subset of these cases are feasible.
     switch (p->kind)
        {
          case tk_error:
          case tk_void: return NULL;
          case tk_integer: if (is_unnamed(p) == true) return p; else return NULL; // For enums
#if FIXED_POINT_ALLOWED
          case tk_fixed_point:
#endif
          case tk_float:
          case tk_imaginary:
          case tk_complex: return NULL;
          case tk_pointer: return contains_unnamed_type_at_top_level(p->variant.pointer.type);
          case tk_routine: return NULL; // FIXME (Can this have a type definition in it?)
          case tk_array: return contains_unnamed_type_at_top_level(p->variant.array.element_type);

          case tk_class:
          case tk_struct:
          case tk_union: if (is_unnamed(p)) return p; else return NULL;

          case tk_typeref: 
             {
            // Skip typedefs (which do not define the types they refer to)
               return p->source_corresp.name == NULL ? contains_unnamed_type_at_top_level(p->variant.typeref.type) : NULL;
             }

          case tk_ptr_to_member: 
             {
            // See test2019.C for an example of this (declaration of pointer to member in typedef).
#if 0
               printf ("In contains_unnamed_type_at_top_level(): Calling contains_unnamed_type_at_top_level_ptr_to_member() which returns a NULL pointer! p = %p \n",p);
#endif
#if 0
               printf ("In contains_unnamed_type_at_top_level(): Calling summarize_type(): \n");
               summarize_type(p);
#endif
               return contains_unnamed_type_at_top_level_ptr_to_member(p);
             }

          case tk_template_param:
             {
#ifdef DEBUG_CONVERSION
               printf ("In contains_unnamed_type_at_top_level(): case tk_template_param: (return NULL) \n");
#endif
               return NULL;
             }

#if GNU_VECTOR_TYPES_ALLOWED
       // DQ (7/27/2014): Added EDG 4.9 specific members.
          case tk_vector:
             {
#ifdef DEBUG_CONVERSION
               printf ("In contains_unnamed_type_at_top_level(): case tk_vector: (return NULL) \n");
#endif
               return NULL;
             }
#endif

       // DQ (7/27/2014): Added EDG 4.9 specific members.
          case tk_nullptr:
             {
#ifdef DEBUG_CONVERSION
               printf ("In contains_unnamed_type_at_top_level(): case tk_nullptr: (return NULL) \n");
#endif
               return NULL;
             }

       // DQ (7/27/2014): This should have been in place before.
          case tk_unknown:
               ROSE_ASSERT (!"Bad type in contains_unnamed_type_at_top_level");

          default: 
               ROSE_ASSERT (!"Bad type in contains_unnamed_type_at_top_level");
        }

     ROSE_ASSERT(!"Should not be reached");
     return NULL;
   }

std::string generate_source_position_string(a_source_correspondence & sc, void* pointer_value)
   {
     a_source_position start;
     string return_string;

     a_decl_position_supplement_ptr sup = sc.decl_pos_info;

     start = sc.decl_position;

  // DQ (11/14/2021): Handle this case seperately, see where this can happen (below).
  // ROSE_ASSERT(sup != NULL);

     if (sup != NULL)
        {
          if (!source_range_null(sup->identifier_range))
             {
#if 0
               printf ("Setting source position using the source_range_null(sup->identifier_range) \n");
#endif
               start = std::min(start, sup->identifier_range.start);
             }

          if (!source_range_null(sup->specifiers_range))
             {
#if 0
               printf ("Setting source position using the source_range_null(sup->specifiers_range) \n");
#endif
               start = std::min(start, sup->specifiers_range.start);
             }

          if (!source_range_null(sup->variant.declarator_range))
             {
#if 0
               printf ("Setting source position using the source_range_null(sup->variant.declarator_range) \n");
#endif
               start = std::min(start, sup->variant.declarator_range.start);
             }

       // bool compiler_generated_start = (start->start_position.seq == 0);
          bool compiler_generated_start = (start.seq == 0);
          ROSE_ASSERT(compiler_generated_start == false);

          Sg_File_Info* fi_start = convert_source_position(start);
          ROSE_ASSERT(fi_start != NULL);

       // DQ (11/12/2021): Use the file_id instead of the filename string.
       // string filename   = fi_start->get_filename();
          int file_id       = fi_start->get_file_id();
          int line_number   = fi_start->get_line();
          int column_number = fi_start->get_col();

#if 0
       // printf ("file_id = %d line_number = %d column_number = %d filename = %s \n",file_id,line_number,column_number,filename.c_str());
          printf ("file_id = %d line_number = %d column_number = %d \n",file_id,line_number,column_number);
#endif

       // stringSupport(filename,"/","_sep_");
       // printf ("file_id = %d line_number = %d column_number = %d filename = %s \n",file_id,line_number,column_number,filename.c_str());

       // return_string = filename + "_" + StringUtility::numberToString(line_number) + "_" + StringUtility::numberToString(column_number);

       // DQ (11/14/2021): Give the name the similar prefix as to what is searched for in the unparser.
       // return_string = StringUtility::numberToString(file_id) + "_" + StringUtility::numberToString(line_number) + "_" + StringUtility::numberToString(column_number);
          return_string = string("0x") + StringUtility::numberToString(file_id) + "_" + StringUtility::numberToString(line_number) + "_" + StringUtility::numberToString(column_number);

          delete fi_start;
          fi_start = NULL;
        }
       else
        {
       // DQ (11/14/2021): It appears that there are times when we just don't have source position 
       // information, for example: test2021_10.c
          ROSE_ASSERT(pointer_value != NULL);
          return StringUtility::numberToString(pointer_value);
        }

#if 0
     printf ("In generate_source_position_string(): return_string = %s \n",return_string.c_str());
#endif

#if 0
     printf ("In generate_source_position_string(): exiting as a test! \n");
     ROSE_ASSERT(false);
#endif

     return return_string;
   }

std::string generate_name_for_type(a_type_ptr p)
   {
#if 0
     printf ("In generate_name_for_type(): building name: __anonymous_%p \n",p);
#endif
  // Do something like EDG does for unnamed objects
  // FIXME: ROSE can't handle a union or such that truly does not have a name
  // (and is just expanded inline where it is used).
  // return "__anonymous_" + StringUtility::numberToString(p);
  // print_source_corresp(p->source_corresp);

#if 0
  // printf ("In generate_name_for_variable(): using pointer value in name \n");

#if 0
     printf ("In generate_name_for_type(): calling summarize_type(): p = %p \n",p);
     summarize_type(p);
#endif
#if 0
     printf ("Exiting as a test! \n");
     ROSE_ASSERT(false);
#endif

     return "__anonymous_" + StringUtility::numberToString(p);
#else
  // DQ (11/12/2021): Avoid using the pointer name in the generated name to support multifile handling.
     return "__anonymous_" + generate_source_position_string(p->source_corresp,(void*)p);
#endif
   }


std::string generate_name_for_variable(a_variable_ptr p)
   {
  // DQ (6/8/2012): Added support for un-named variables (required for test2012_98.C).

  // Do something like EDG does for unnamed objects
  // FIXME: ROSE can't handle a union or such that truly does not have a name
  // (and is just expanded inline where it is used).
#if 1
  // printf ("In generate_name_for_variable(): using pointer value in name \n");
     return "__anonymous_" + StringUtility::numberToString(p);
#else
     return "__anonymous_" + generate_source_position_string(p->source_corresp,(void*)p);
#endif
   }


std::string generate_name_for_field(a_field_ptr p)
   {
  // DQ (1/1/2014): Added support for un-named fields (required for test2014_01.c).

#if 1
  // printf ("In generate_name_for_field(): using pointer value in name \n");
     return "__anonymous_" + StringUtility::numberToString(p);
#else
     return "__anonymous_" + generate_source_position_string(p->source_corresp,(void*)p);
#endif
   }

std::string generate_pragma_name(a_pragma_ptr p)
   {
  // DQ (9/23/2020): This is a refactoring of the code in parse_pragma() so that we can support the 
  // generation of SgPragma IR nodes independently of SgPragmaDeclaration nodes (which can not be 
  // added to the AST in some locations where #pragma declarations can be used, see Cxx11_tests_102.C 
  // for an example).

     a_pragma_kind kind = p->kind;

#if 0
     printf ("In generate_pragma_name(): p = %p \n",p);
#endif

     string pragma_kind_name;

     if (p->pragma_text != NULL)
        {
       // This is the most common case where UPC and OpenMP are not used.
          pragma_kind_name = p->pragma_text;
#if 0
          printf ("In generate_pragma_name(): p->pragma_text != NULL: pragma_kind_name = %s\n",pragma_kind_name.c_str());
#endif
        }
       else
        {
       // Switch #1
          switch (kind)
             {
#if UPC_EXTENSIONS_ALLOWED             
               case pk_upc:
                  {
#ifdef DEBUG_CONVERSION
                    indent_line(); debug_print("UPC pragma\n");
                    ScopedIndent si(indent);
#endif
                    a_upc_pragma_kind inner_kind = p->variant.upc.kind;

                 // Switch #2
                    switch (inner_kind)
                       {
                         case upc_pk_access:
                            {
                              a_upc_access_method tag = p->variant.upc.value.access_method;

                           // Switch #3
                              switch (tag)
                                 {
                                   case upc_access_unspecified: 
                                        ROSE_ASSERT (!"upc_access_unspecified not supported"); 
                                        break;

                                   case upc_access_strict:
                                     // result = buildPragmaDeclaration("upc strict"); 
                                        pragma_kind_name = "upc strict";
                                        break;

                                   case upc_access_relaxed:
                                     // result = buildPragmaDeclaration("upc relaxed");
                                        pragma_kind_name = "upc relaxed";
                                        break;

                                   default:
                                        ERROR_MESSAGE(stderr, "ERROR: Bad UPC access tag %d\n", tag); 
                                        ROSE_ASSERT(false);
                                 }
                              break;
                            }

                         case upc_pk_coherence:
                            {
                              ROSE_ASSERT (!"UPC coherence");
                              break;
                            }

                         default:
                              ERROR_MESSAGE(stderr, "Bad UPC pragma kind %d\n", inner_kind);
                              ROSE_ASSERT(false);
                       }
                    break;
                  }
#endif
#if IDENT_DIRECTIVE_AND_PRAGMA
            // DQ (1/6/2009): Added support for #ident (tests using GNU 3.x compiler in test2006_69.C).
               case pk_ident:
                  {
                    ROSE_ASSERT(p->variant.ident_string != NULL);
                    string ident_string = convert_string_constant(p->variant.ident_string);

                 // return buildPragmaDeclaration("#ident " + ident_string); // FIXME file info
                 // result = buildPragmaDeclaration("#ident " + ident_string); // FIXME file info
                    pragma_kind_name = "#ident " + ident_string;

                    break;
                  }
#endif /* IDENT_DIRECTIVE_AND_PRAGMA */

#if MICROSOFT_EXTENSIONS_ALLOWED
               case pk_comment:
                  {
                    ROSE_ASSERT(p->variant.comment.str != NULL);
                    string comment_string = convert_string_constant(p->variant.comment.str);
                 // result = buildPragmaDeclaration("#comment " + comment_string);
                    pragma_kind_name = "#comment " + comment_string;
#if 0
                    printf ("In parse_pragma(): Defining case pk_comment to return #comment pragma: kind = %d comment_string = %s \n",p->variant.comment.kind,comment_string.c_str());
#endif
                    break;
                  }
#endif

            // DQ (1/22/2018): Added C99 specific pragma support.
               case pk_stdc:
                  {
                 // a_stdc_pragma_kind  kind;  // For the C99 STDC predefined pragmas, indicates the specific pragma being used.
                 // a_stdc_pragma_value value; // Specifies whether the attribute is being turned on, off, or reset to the default value.
#if 0
                    printf ("In parse_pragma(): case pk_stdc: calling summarize_pragma(p = %p) \n",p);
                    summarize_pragma(p);
#endif
                    string kind_name = "STDC ";
                    switch(p->variant.stdc.kind)
                       {
                         case stdc_pk_none:
                            {
                              printf ("In parse_pragma(): case stdc_pk_none: (not yet implemented) \n");
                              ROSE_ASSERT(false);
                              break;
                            }

                         case stdc_pk_fp_contract:
                            {
                           // printf ("In parse_pragma(): case stdc_pk_cx_limited_range: (not yet implemented) \n");
                              kind_name += "FP_CONTRACT";
                              break;
                            }

                         case stdc_pk_fenv_access:
                            {
                           // printf ("In parse_pragma(): case stdc_pk_cx_limited_range: (not yet implemented) \n");
                              kind_name += "FENV_ACCESS";
                              break;
                            }

                         case stdc_pk_cx_limited_range:
                            {
                           // printf ("In parse_pragma(): case stdc_pk_cx_limited_range: (not yet implemented) \n");
                              kind_name += "FP_LIMITED_RANGE";
                              break;
                            }
#if FIXED_POINT_ALLOWED
                         case stdc_pk_fx_full_precision:
                         case stdc_pk_fx_fract_overflow:
                         case stdc_pk_fx_accum_overflow:
                            {
                              printf ("Error: In parse_pragma(): case pk_stdc: cases of fixed point support \n");
                              ROSE_ASSERT(false);
                            }
#endif /* FIXED_POINT_ALLOWED */

                         case stdc_pk_last:
                            {
                              printf ("Error: case stdc_pk_last: \n");
                              ROSE_ASSERT(false);
                            }

                         default: 
                            {
                              printf ("Error: In parse_pragma(): case pk_stdc: kind default reached \n");
                              ROSE_ASSERT(false);
                            }
                       }

                  switch (p->variant.stdc.value)
                     {
                       case stdc_pv_none:
                            {
                              printf ("In parse_pragma(): case stdc_pv_none: (not yet implemented) \n");
                              ROSE_ASSERT(false);
                              break;
                            }
                       case stdc_pv_off:
                            {
                           // printf ("In parse_pragma(): case stdc_pv_off: \n");
                              kind_name += " OFF";
                              break;
                            }
                       case stdc_pv_on:
                            {
                           // printf ("In parse_pragma(): case stdc_pv_on: \n");
                              kind_name += " ON";
                              break;
                            }
#if FIXED_POINT_ALLOWED
                       case stdc_pv_sat:
                            {
                           // Represents the "SAT" option for the "Embedded C" FX_FRACT_OVERFLOW and FX_ACCUM_OVERFLOW variants of "#pragma STDC ...".
                              printf ("Error: In parse_pragma(): case pk_stdc: value case of fixed point support \n");
                              ROSE_ASSERT(false);
                            }
#endif /* FIXED_POINT_ALLOWED */
                       case stdc_pv_default:
                            {
                           // printf ("Error: In parse_pragma(): case pk_stdc: value default reached \n");
                              kind_name += " DEFAULT";
                              break;
                            }

                       default:
                            {
                              printf ("Error: In parse_pragma(): default reached \n");
                              ROSE_ASSERT(false);
                            }
                       }
#if 0
                    printf ("In parse_pragma(): kind_name = %s\n",kind_name.c_str());
#endif
                 // result = buildPragmaDeclaration(kind_name, topScopeStack());
                    pragma_kind_name = kind_name;
#if 0
                    printf ("In parse_pragma(): Defining case pk_stdc pragma: kind = %d \n",p->variant.stdc.kind);
#endif
#if 0
                    printf ("Exiting as a test! \n");
                    ROSE_ASSERT(false);
#endif
                    break;
                  }

               case pk_none:
               case pk_printf_args:
               case pk_scanf_args:
               case pk_lint_argsused:
               case pk_lint_varargs_count:
               case pk_lint_notreached:
               case pk_instantiate:
               case pk_do_not_instantiate:
               case pk_can_instantiate:
               case pk_inline_template:
#if USER_CONTROL_OF_STRUCT_PACKING
               case pk_pack:
#endif
#if PRAGMA_WEAK_ALLOWED
               case pk_weak:
#endif /* PRAGMA_WEAK_ALLOWED */
               case pk_once:
               case pk_hdrstop:
               case pk_no_pch:
               case pk_define_type_info:
            // case pk_stdc:
#if UPC_EXTENSIONS_ALLOWED
            // case pk_upc:
#endif /* UPC_EXTENSIONS_ALLOWED */
#if REDEFINE_EXTNAME_PRAGMA_ENABLED
               case pk_redefine_extname:
#endif /* REDEFINE_EXTNAME_PRAGMA_ENABLED */
#if SUN_EXTENSIONS_ALLOWED
               case pk_enable_ldscope:
               case pk_disable_ldscope:
#endif /* SUN_EXTENSIONS_ALLOWED */
#if GNU_EXTENSIONS_ALLOWED
               case pk_gcc_immediate:
               case pk_gcc_next_token:
#endif
               case pk_diag_suppress:
               case pk_diag_remark:
               case pk_diag_warning:
               case pk_diag_error:
               case pk_diag_once:
               case pk_diag_default:
               case pk_db_opt:
               case pk_db_name:
#if GENERATE_MICROSOFT_IF_EXISTS_ENTRIES
               case pk_if_exists:
#endif
#if MICROSOFT_EXTENSIONS_ALLOWED
               case pk_push_macro:
               case pk_pop_macro:
               case pk_start_map_region:
               case pk_stop_map_region:
#if NATIVE_MULTIBYTE_CHARS_SUPPORTED_WITH_UNICODE
               case pk_setlocale:
#endif /* NATIVE_MULTIBYTE_CHARS_SUPPORTED_WITH_UNICODE */
            // case pk_comment:
               case pk_conform:
               case pk_include_alias:
#endif
#if INCLUDE_UNRECOGNIZED_PRAGMAS_IN_IL
               case pk_unrecognized:
#endif
               case pk_last:
                  {
                    printf ("pragma case not handled! kind = %d pragma_name = %s \n",kind,pragma_kind_name.c_str());
                    ROSE_ASSERT(false);
                    break;
                  }

               default:
                  {
                    ERROR_MESSAGE(stderr, "Bad pragma kind %s\n", pragma_kind_name.c_str());
                    ROSE_ASSERT(false);
                  }
             }

#if 0
          printf ("In generate_pragma_name(): p->pragma_text == NULL: pragma_kind_name = %s\n",pragma_kind_name.c_str());
#endif
        }

#if 0
     printf ("Leaving generate_pragma_name(): pragma_kind_name = %s\n",pragma_kind_name.c_str());
#endif

     return pragma_kind_name;
   }

std::string generate_namespace_name(a_namespace_ptr p) {
  // DQ (8/25/2014): This functionality was refactored across three locations where it is called.

     string namespace_name;
     if (p->source_corresp.name != NULL)
        {
          namespace_name = p->source_corresp.name;
        }

  // DQ (6/16/2013): This is a test for a specially generated named namespace that shows up in only special cases.
     string targetString = "_GLOBAL__N__1";

#if 0
     printf ("In generate_namespace_name(): ##### namespace_name = %s \n",namespace_name.c_str());
     printf ("In generate_namespace_name(): ##### namespace_name.size() = %" PRIuPTR " \n",namespace_name.size());
     printf ("In generate_namespace_name(): ##### targetString = %s targetString.size() = %" PRIuPTR " \n",targetString.c_str(),targetString.size());
     printf ("In generate_namespace_name(): ##### namespace_name.substr(0,targetString.size()) = %s \n",namespace_name.substr(0,targetString.size()).c_str());
#endif

     if (namespace_name.substr(0,targetString.size()) == targetString)
        {
#if 0
          printf ("In generate_namespace_name(): calling summarize_namespace(nsp = %p) \n",p);
          summarize_namespace(p);
#endif
#if 0
          printf ("Exiting as a test! \n");
          ROSE_ASSERT(false);
#endif
        }

     if (p->source_corresp.unnamed_entity_given_fabricated_name == true)
        {
       // isUnnamedNamespace = true;
          namespace_name = "";
        }

#if 0
     printf ("Leaving generate_namespace_name(): (after filtering generated namespace names): a_namespace_ptr p = %p namespace_name = %s) \n",p,namespace_name.c_str());
#endif

     return namespace_name;
   }

void clear_EDG_ROSE_caches() {
  // Starts with some checks

     ROSE_ASSERT(Sg_File_Info::get_nametofileid_map().find("") == Sg_File_Info::get_nametofileid_map().end());

     ROSE_ASSERT(internal_intermediate_scope != NULL);
     if (internal_intermediate_scope->symbol_table_size() > 0) {
       printf ("FATAL: internal_intermediate_scope->symbol_table_size() = %zu \n", internal_intermediate_scope->symbol_table_size());
     }
     ROSE_ASSERT(internal_intermediate_scope->symbol_table_size() == 0);

  // Clearing the cache/stacks

     project_wide_type_translation_cache.insert(type_translation_cache.begin(),type_translation_cache.end());

     type_translation_cache.clear();
     template_type_translation_cache.clear();
     type_declaration_map.clear();

     first_nondefining_declarations_for_functions.clear();
     defining_declarations_for_functions.clear();
     function_declaration_map.clear();

     function_name_cache.clear();

     first_nondefining_declarations_for_template_functions.clear();
     defining_declarations_for_template_functions.clear();

     first_nondefining_declarations_for_templates.clear();
     defining_declarations_for_templates.clear();

     variable_cache.clear();

     field_cache.clear();

     function_cache.clear();

     label_cache.clear();

     types_with_emitted_definitions.clear();

     statement_expression_bodies.clear();

     statement_scope_cache.clear();

     scope_cache.clear();

     namespace_cache.clear();
     namespace_alias_cache.clear();

     currentSourceFile = NULL;
     current_global_scope = NULL;

     templateDeclarationStack.clear();
     
     global_innermost_function_scope = NULL;

     switchSupportStackOfMaps.clear();
     
     outputTypesSet.clear();

     edg_type_to_rose_typeof_type_map.clear();
     edg_expression_to_rose_typeof_type_map.clear();

     delete internal_intermediate_scope;
     internal_intermediate_scope = NULL;

     statement_expression_cache.clear();

     nonreal_scope_cache.clear();
     nonreal_scope_cache_from_template.clear();
     nonreal_scope_for_template_parameters_stack.clear();

     processedTemplateArgsCache.clear();
     processedTemplateParameterCache.clear();
     typesWithoutDeduction.clear();

     // PP (9/24/25): should the following maps be cleared?
     // saved_defining_declarations_from_class_templates
     // switch_case_map
   }

void clear_global_caches() {
  edg_include_file_map.clear();
  edg_include_file_using_names_as_written_map.clear();
  project_wide_type_translation_cache.clear();
}

void build_header_file_tree(SgSourceFile* rose_source_file, SgIncludeFile* parent_include_file, a_source_file_ptr edg_source_file)
   {

#define DEBUG_INCLUDE_TREE_SUPPORT 0

  // DQ (9/17/2018): Build a tree data structure that represents the header file tree.
  // This information will be used in the header file unparsing to note the differences 
  // between the header files that are listed in include directives, vs the header files 
  // that are included to form the AST.

  // NOTE thiat this is one function that is used with either a valid pointer to a SgSourceFile 
  // or a valid pointer to a SgIncludeFile.  As a result it will insert children at the first 
  // level into the SgSourceFile, and children at all lower levels into the specified SgIncludeFile.

  // #if DEBUG_INCLUDE_TREE_SUPPORT || 0
  // DQ (4/5/2020): Added header file unparsing feature specific debug level.
     if (SgProject::get_unparseHeaderFilesDebug() >= 6)
        {
          printf ("\n*************************** \n");
          printf ("In build_header_file_tree(): rose_source_file = %p parent_include_file = %p edg_source_file = %p \n",rose_source_file,parent_include_file,edg_source_file);
          printf ("rose_source_file->getFileName = %s \n",rose_source_file->getFileName().c_str());
          if (edg_source_file->file_name != NULL)
             {
               string filename = edg_source_file->file_name;
            // string parentFilename = parent_include_file != NULL ? parent_include_file->file_name : "null";
               string parentFilename = parent_include_file != NULL ? parent_include_file->get_filename() : "null";
               printf (" --- header file name = %s parentFilename = %s \n",filename.c_str(),parentFilename.c_str());

            // DQ (8/18/2019): Can this ever be NULL? Check it!
            // ROSE_ASSERT(parent_include_file != NULL);
             }
        }
  // #endif

  // Only one of these should be non-NULL (enforced).
     ROSE_ASSERT(rose_source_file != NULL || parent_include_file != NULL);

  // DQ (11/9/2018): Enforce the rose_source_file is always a valid pointer so that we can set 
  // the SgSourceFile associated with the translation unit.
  // ROSE_ASSERT(rose_source_file == NULL || parent_include_file == NULL);
     ROSE_ASSERT(rose_source_file != NULL || parent_include_file == NULL);

     ROSE_ASSERT(edg_source_file != NULL);

     string applicationRootDirectory = rose_source_file->get_project()->get_applicationRootDirectory();

#if DEBUG_INCLUDE_TREE_SUPPORT
     printf ("applicationRootDirectory = %s \n",applicationRootDirectory.c_str());
#endif

     if (parent_include_file == NULL)
        {
       // Build a root node for the SgIncludeFile so that we can define traversals over the SgIncludeFile tree.
       // This will simplify how we can pose queries on the include tree structure.

          string filename = edg_source_file->file_name;

          parent_include_file = new SgIncludeFile(filename);
#if 0
       // DQ (3/28/2021): Test the value of get_source_file().
          SgSourceFile* include_source_file = parent_include_file->get_source_file();
          printf ("Test A \n");
          printf ("include_source_file = %p \n",include_source_file);
          printf ("Test B \n");
       // ROSE_ASSERT(include_source_file != NULL);
          printf ("Test C \n");
#endif
          ROSE_ASSERT(parent_include_file != NULL);
          ROSE_ASSERT(parent_include_file->get_source_file() == NULL);

       // DQ (5/6/2020): Mark this as the root of the include tree (associated with the input source file).
       // Note that we can't support a list of include files in the source file else we would have to build 
       // source files for every header file just to support the list of include files that it includes.
          parent_include_file->set_isRootSourceFile(true);

       // #if DEBUG_INCLUDE_TREE_SUPPORT
       // DQ (4/5/2020): Added header file unparsing feature specific debug level.
          if (SgProject::get_unparseHeaderFilesDebug() >= 8)
             {
               printf ("In build_header_file_tree(): parent_include_file == NULL: build SgIncludeFile: parent_include_file = %p filename = %s \n",parent_include_file,filename.c_str());
             }
       // #endif
       // DQ (11/16/2019): Check that this is not alredy in the map.
          ROSE_ASSERT (edg_include_file_map.find(filename) == edg_include_file_map.end());

          rose_source_file->set_associated_include_file(parent_include_file);

       // DQ (6/3/2019): Need to set the parent in general, because it will be checked in the AST Consistancy Tests.
       // rose_source_file->set_parent(rose_source_file);
       // rose_source_file->set_parent(parent_include_file);
       // rose_source_file->set_parent(NULL);
       // ROSE_ASSERT(rose_source_file->get_parent() != NULL);
          ROSE_ASSERT(rose_source_file->get_parent() != NULL);

       // printf ("build_header_file_tree(): SgIncludeFile: self-reference via parent pointer! \n");

       // DQ (11/20/2019): Make sure this has not been previously set.
          ROSE_ASSERT(parent_include_file->get_source_file() == NULL);

          parent_include_file->set_source_file(rose_source_file);

       // DQ (03/10/2020): Make sure this has not been previously set.
          ROSE_ASSERT(parent_include_file->get_parent() == NULL);

       // DQ (03/10/2020): Set the parent.
          parent_include_file->set_parent(rose_source_file);

       // DQ (03/10/2020): Check that it is set.
          ROSE_ASSERT(parent_include_file->get_parent() != NULL);

#if 0
          printf ("@@@@@@@@@@@@@@@@@@@@@@@@ parent_include_file->set_source_file(): parent_include_file = %p rose_source_file = %p filename = %s \n",parent_include_file,rose_source_file,filename.c_str());
#endif
       // parent_include_file->set_parent(rose_source_file);
       // parent_include_file->set_parent_include_file(rose_source_file);

       // DQ (11/17/2018):: Not applicable to the root level (source file).
       // parent_include_file->set_name_used_in_include_directive("_N/A_");
       // parent_include_file->set_directory_prefix("_N/A_");
          parent_include_file->set_name_used_in_include_directive("");
          parent_include_file->set_directory_prefix("");

#if 0
          printf ("$$$$$$$$$$$$$$$ Calling set_source_file_of_translation_unit(): parent_include_file = %p = %s \n",parent_include_file,parent_include_file->class_name().c_str());
          ROSE_ASSERT(rose_source_file != NULL);
          printf (" --- rose_source_file = %p = %s \n",rose_source_file,rose_source_file->class_name().c_str());
#endif

          parent_include_file->set_source_file_of_translation_unit(rose_source_file);
          parent_include_file->set_parent_include_file(NULL);

       // DQ (11/16/2018): Save the applicationRootDirectory.
          parent_include_file->set_applicationRootDirectory(applicationRootDirectory);
        }

     a_source_file_ptr temp_file = edg_source_file->first_child_file;

     if (SgProject::get_unparseHeaderFilesDebug() >= 8)
        {
          printf ("In build_header_file_tree(): temp_file = %p \n",temp_file);
        }

     while (temp_file != NULL)
        {
          string filename = temp_file->file_name;

       // DQ (11/16/2019): Maybe we should check if an include file already exists for this filename?

#if 0
       // DQ (4/10/2020): If we do this then the filename associated with each statement in the file will not match (edg is reporting non-normalized filenames).
       // DQ (4/8/2020): We need to add a normalization to the filename to prevent using filename such as: test6/subdir/../Simple.h
       // which would not match in later string comparision tests.
          filename = normalize_filename(filename);
#else
       // printf ("Skipping use of normalization on filenames (normalize_filename() function) \n");
#endif

          SgIncludeFile* include_file = new SgIncludeFile(filename);
          ROSE_ASSERT(include_file != NULL);

       // DQ (3/28/2021): Test the value of get_source_file().
          SgSourceFile* include_source_file = include_file->get_source_file();
#if 0
          printf ("Test AA \n");
#endif
          ROSE_ASSERT(include_source_file == NULL);
#if 0
          printf ("Test BB \n");
#endif
       // #if DEBUG_INCLUDE_TREE_SUPPORT || 0
       // DQ (4/5/2020): Added header file unparsing feature specific debug level.
          if (SgProject::get_unparseHeaderFilesDebug() >= 8)
             {
               printf ("In build_header_file_tree(): build SgIncludeFile: include_file = %p temp_file = %p filename = %s \n",include_file,temp_file,temp_file->file_name);
             }
       // #endif

       // DQ (6/3/2019): Need to set the parent in general, because it will be checked in the AST Consistancy Tests.
          include_file->set_parent(parent_include_file);
          ROSE_ASSERT(include_file->get_parent() != NULL);

          if (parent_include_file->get_source_file() == rose_source_file)
             {
            // #if DEBUG_INCLUDE_TREE_SUPPORT
            // DQ (4/5/2020): Added header file unparsing feature specific debug level.
               if (SgProject::get_unparseHeaderFilesDebug() >= 8)
                  {
                    printf ("############ Found location at root include file ############ \n");
                  }
                 // #endif

            // DQ (11/15/2018): This list is redundant and is removed to support the traversals over the SgIncludeFile tree.
            // rose_source_file->get_include_file_list().push_back(include_file);
             }

       // DQ (10/25/2019): Can we enforce this?
       // ROSE_ASSERT(include_file->get_source_file() != NULL);
          ROSE_ASSERT(include_file->get_source_file() == NULL);

       // parent_include_file->get_include_file_list().push_back(include_file);

          SgName name_as_written = temp_file->name_as_written;

       // #if DEBUG_INCLUDE_TREE_SUPPORT
       // DQ (4/5/2020): Added header file unparsing feature specific debug level.
          if (SgProject::get_unparseHeaderFilesDebug() >= 8)
             {
               printf ("In build_header_file_tree(): temp_file = %p name_as_written = %s \n",temp_file,temp_file->name_as_written);
             }
       // #endif

          include_file->set_name_used_in_include_directive(name_as_written);

       // #if DEBUG_INCLUDE_TREE_SUPPORT
       // DQ (4/5/2020): Added header file unparsing feature specific debug level.
          if (SgProject::get_unparseHeaderFilesDebug() >= 8)
             {
               printf ("In build_header_file_tree(): include_file = %p include_file->get_name_used_in_include_directive() = %s \n",include_file,include_file->get_name_used_in_include_directive().str());
             }
       // #endif

       // DQ (11/13/2018): Adding support for when the header file has a path prefix.
          string name_used_in_include_directive = name_as_written;
          SgName directoryPathPrefix = Rose::getPathFromFileName(name_used_in_include_directive);

       // #if DEBUG_INCLUDE_TREE_SUPPORT
       // DQ (4/5/2020): Added header file unparsing feature specific debug level.
          if (SgProject::get_unparseHeaderFilesDebug() >= 8)
             {
               printf ("directoryPathPrefix = %s \n",directoryPathPrefix.str());
             }
       // #endif

          SgName name_without_path = Rose::utility_stripPathFromFileName(name_used_in_include_directive);

       // #if DEBUG_INCLUDE_TREE_SUPPORT
       // DQ (4/5/2020): Added header file unparsing feature specific debug level.
          if (SgProject::get_unparseHeaderFilesDebug() >= 8)
             {
               printf ("name_without_path = %s \n",name_without_path.str());
             }
       // #endif

          include_file->set_directory_prefix(directoryPathPrefix);
          include_file->set_name_without_path(name_without_path);

       // DQ (11/16/2018): Save the applicationRootDirectory.
          include_file->set_applicationRootDirectory(applicationRootDirectory);

       // DQ (11/10/2018): for the top level file, this is always NULL, since it can't be a header file.
       // rose_source_file->set_associated_include_file(NULL);

       // DQ (11/10/2018): This might be redundant with the constructor initialization.
          rose_source_file->set_isHeaderFile(false);
          rose_source_file->set_isHeaderFileIncludedMoreThanOnce(false);

          ROSE_ASSERT(rose_source_file->get_isHeaderFile() == false);

       // DQ (11/21/2018): check if the application root is a substring of the filename (filename with path).
          bool isApplicationFile = false;
          size_t found = filename.find(applicationRootDirectory);
          if (found == 0)
             {
            // The application root directory is at the start of the filename.
               include_file->set_isApplicationFile(true);
               isApplicationFile = true;
             }

       // #if DEBUG_INCLUDE_TREE_SUPPORT
       // DQ (4/5/2020): Added header file unparsing feature specific debug level.
          if (SgProject::get_unparseHeaderFilesDebug() >= 8)
             {
               printf ("isApplicationFile = %s \n",isApplicationFile ? "true" : "false");
             }
       // #endif

#if 1
          string file_hash = "no_hash_computed";
#else
       // Disable use of this hash function.
          string file_hash = rose_md5::compute_hash(filename);
#endif

       // #if DEBUG_INCLUDE_TREE_SUPPORT
       // DQ (4/5/2020): Added header file unparsing feature specific debug level.
          if (SgProject::get_unparseHeaderFilesDebug() >= 8)
             {
               printf ("In build_header_file_tree(): filename = %s file_hash = %s \n",temp_file->file_name,file_hash.c_str());
             }
       // #endif

          include_file->set_file_hash(file_hash);

#if 0
          printf ("$$$$$$$$$$$$$$$ Calling set_source_file_of_translation_unit(): include_file = %p = %s \n",include_file,include_file->class_name().c_str());
          ROSE_ASSERT(rose_source_file != NULL);
          printf (" --- rose_source_file = %p = %s \n",rose_source_file,rose_source_file->class_name().c_str());
#endif

       // DQ (11/9/2018): Set the source file associated with the translation unit.
          include_file->set_source_file_of_translation_unit(rose_source_file);

       // DQ (11/9/2018): OK if this is NULL, which means that the rose_source_file is the parent.
          include_file->set_parent_include_file(parent_include_file);

       // #if DEBUG_INCLUDE_TREE_SUPPORT
       // DQ (4/5/2020): Added header file unparsing feature specific debug level.
          if (SgProject::get_unparseHeaderFilesDebug() >= 8)
             {
            // printf ("parent_include_file = %p \n",parent_include_file);
               if (parent_include_file != NULL)
                  {
                    printf ("parent_include_file = %p = %s \n",parent_include_file,parent_include_file->get_filename().str());
                  }
             }
       // #endif

       // DQ (11/12/2018): Added to support detection of <> vs. "" include file specifications.
          bool included_by_system_include = temp_file->included_by_system_include;
          include_file->set_isSystemInclude(included_by_system_include);

          bool preinclude_macros_only = temp_file->preinclude_macros_only;
          include_file->set_preinclude_macros_only(preinclude_macros_only);

       // #if DEBUG_INCLUDE_TREE_SUPPORT
       // DQ (4/5/2020): Added header file unparsing feature specific debug level.
          if (SgProject::get_unparseHeaderFilesDebug() >= 8)
             {
               printf ("preinclude_macros_only = %s \n",preinclude_macros_only ? "true" : "false");
             }
       // #endif

          bool from_system_include_dir = temp_file->from_system_include_dir;
          include_file->set_from_system_include_dir(from_system_include_dir);

       // #if DEBUG_INCLUDE_TREE_SUPPORT
       // DQ (4/5/2020): Added header file unparsing feature specific debug level.
          if (SgProject::get_unparseHeaderFilesDebug() >= 8)
             {
               printf ("from_system_include_dir = %s \n",from_system_include_dir ? "true" : "false");

               bool is_implicit_include = temp_file->is_implicit_include;
               printf ("is_implicit_include = %s \n",is_implicit_include ? "true" : "false");
               bool related_file_implicit_include_done = temp_file->related_file_implicit_include_done;
               printf ("related_file_implicit_include_done = %s \n",related_file_implicit_include_done ? "true" : "false");
             }
       // #endif

       // DQ (11/12/2018): Added to support detection of included files from preinclude specifications (on the command line).
          bool included_by_preinclude     = temp_file->included_by_preinclude;
          include_file->set_isPreinclude(included_by_preinclude);

       // DQ (11/22/2018): Make this more restrictive to avoid processing of system and compiler specific header files.
       // if (included_by_preinclude == false)
          if (included_by_preinclude == false && isApplicationFile == true)
             {
            // DQ (11/10/2018): Now build the associated SgSourceFile, so that we will have it in the heirarchy.

               if (edg_include_file_map.find(filename) != edg_include_file_map.end())
                  {
                 // Note: redundant entries will not be put into the include file map.
                 // Note clear if this is a correct implementation.

                 // #if DEBUG_INCLUDE_TREE_SUPPORT
                 // DQ (4/5/2020): Added header file unparsing feature specific debug level.
                    if (SgProject::get_unparseHeaderFilesDebug() >= 8)
                       {
                         printf ("This include file has already been seen! \n");
                       }
                 // #endif

                    include_file->set_isIncludedMoreThanOnce(true);

                 // And set the initial entry that was originally put into the map.
                    edg_include_file_map[filename]->set_isIncludedMoreThanOnce(true);

                 // Mark the primary use as a way to, at least initially simplify the implementation.
                    edg_include_file_map[filename]->set_isPrimaryUse(true);

                 // DQ (11/11/2018): Find the associated SgSourceFile and mark it as something???
#if 0
                    printf ("Exiting as a test! \n");
                    ROSE_ASSERT(false);
#endif
                  }
                 else
                  {
                    SgSourceFile* include_sourceFile = new SgSourceFile();

                    include_sourceFile->set_sourceFileNameWithPath(filename);
                    include_sourceFile->set_startOfConstruct(new Sg_File_Info(filename));
                 // include_sourceFile->set_endOfConstruct(new Sg_File_Info(filename));
#if 0
                    printf ("In build_header_file_tree(): include_sourceFile->get_globalScope() = %p \n",include_sourceFile->get_globalScope());
#endif
                 // DQ (11/10/2018): This needs to be set before we check using SageIII/astPostProcessing/resetParentPointers.C
                    include_sourceFile->get_startOfConstruct()->set_parent(include_sourceFile);
                 // include_sourceFile->get_endOfConstruct()->set_parent(include_sourceFile);

                 // DQ (9/23/2019): For C/C++ code this should be false (including all headers).
                    if (include_sourceFile->get_requires_C_preprocessor() == true)
                       {
#if 0
                         printf ("In build_header_file_tree(): Setting include_sourceFile->get_requires_C_preprocessor() to false \n");
#endif
                         include_sourceFile->set_requires_C_preprocessor(false);
                       }
                    ROSE_ASSERT(include_sourceFile->get_requires_C_preprocessor() == false);
#if 0
                    printf ("Need to identify the translation unit so that we can copy the value of isC_only and isCxxOnly \n");
                    printf (" --- include_file->get_source_file_of_translation_unit() = %p \n",include_file->get_source_file_of_translation_unit());
#endif
                    ROSE_ASSERT(include_file->get_source_file_of_translation_unit() != NULL);

                 // DQ (3/3/2021): We need to set more data members of the SgSourceFile.
                    include_sourceFile->set_C_only  (include_file->get_source_file_of_translation_unit()->get_C_only());
                    include_sourceFile->set_Cxx_only(include_file->get_source_file_of_translation_unit()->get_Cxx_only());
#if 0
                    printf ("Exiting as a test! \n");
                    ROSE_ASSERT(false);
#endif
                 // DQ (11/20/2019): Make sure this has not been previously set.
                    ROSE_ASSERT(include_file->get_source_file() == NULL);

                    include_file->set_source_file(include_sourceFile);

                 // DQ (3/28/2021): Having a global scope shared across different header files is a problem for the token-based unparsing.
                 // DQ (5/28/2020): We need to set the global scope (point to the application file's global scope).
                    ROSE_ASSERT (rose_source_file != NULL);
                    if (rose_source_file != NULL)
                       {
                         ROSE_ASSERT(rose_source_file->get_globalScope() != NULL);
#if 1
                      // DQ (4/11/2021): I think it is OK to share the same global scope across all of the header files because each
                      // header file will have it's own source file (which shared the same global scope) and the token sequences
                      // are computed seperatelu for each source file and held in a map using the source file pointer a the key.
                      // DQ (3/28/2021): Original code (sharing the global scope across header files).
                         include_sourceFile->set_globalScope(rose_source_file->get_globalScope());
#else
#if 0
                         printf ("Building a new SgGlobal (global scope) for use in include files: include_sourceFile = %p rose_source_file = %p \n",include_sourceFile,rose_source_file);
                         printf (" --- include_sourceFile filename = %s \n",include_sourceFile->getFileName().c_str());
                         printf (" --- rose_source_file filename   = %s \n",rose_source_file->getFileName().c_str());
#endif
                      // DQ (3/28/2021): Having a global scope shared across different header files is a problem for 
                      // the token-based unparsing. We want to have each global scope be different across each of the 
                      // header file.
                         SgGlobal* includeFileGlobalScope = new SgGlobal();

                         Sg_File_Info* startOfConstruct = new Sg_File_Info();
                         Sg_File_Info* endOfConstruct   = new Sg_File_Info();

                         *startOfConstruct = *(rose_source_file->get_globalScope()->get_startOfConstruct());
                         *endOfConstruct   = *(rose_source_file->get_globalScope()->get_endOfConstruct());

                      // Need to set the file info source position.
                         includeFileGlobalScope->set_startOfConstruct(startOfConstruct);
                         includeFileGlobalScope->set_endOfConstruct(endOfConstruct);
#if 0
                         includeFileGlobalScope->get_startOfConstruct()->display("startOfConstruct: debug");
                         includeFileGlobalScope->get_endOfConstruct()->display("endOfConstruct: debug");
#endif
                         include_sourceFile->set_globalScope(includeFileGlobalScope);

                      // Copy the list of statements in the global scope so that we have the same pointers to the same statements.
                      // This must be copied since the statements in the global scope may change, and if so they are only changed 
                      // through the source file (and not the header files).  This update to the declaration list in the global 
                      // scope of the header files is done, as need before unparsing.  It might have to be updated after the AST 
                      // post processing, but I don't think so (currently).
                         include_sourceFile->get_globalScope()->get_declarations() = rose_source_file->get_globalScope()->get_declarations();
#endif
#if 0
                         printf ("rose_source_file->get_globalScope()->get_declarations().size()   = %zu \n",rose_source_file->get_globalScope()->get_declarations().size());
                         printf ("include_sourceFile->get_globalScope()->get_declarations().size() = %zu \n",include_sourceFile->get_globalScope()->get_declarations().size());
#endif
#if 0
                         printf ("Building a new SgGlobal (global scope) for use in include files: include_sourceFile = %p rose_source_file = %p \n",include_sourceFile,rose_source_file);
                         printf (" --- include_sourceFile filename               = %s \n",include_sourceFile->getFileName().c_str());
                         printf (" --- --- include_sourceFile->get_globalScope() = %p \n",include_sourceFile->get_globalScope());
                         printf (" --- rose_source_file filename                 = %s \n",rose_source_file->getFileName().c_str());
                         printf (" --- --- rose_source_file->get_globalScope()   = %p \n",rose_source_file->get_globalScope());
#endif
                       }
                      else
                       {
                         printf ("Exiting as a test! \n");
                         ROSE_ASSERT(false);
                      // include_file->set_source_file_of_translation_unit(rose_source_file);
                      // ROSE_ASSERT(parent_include_file->get_globalScope() != NULL);
                      // include_sourceFile->set_global_scope(parent_include_file->get_globalScope());
                       }

                 // DQ (5/28/2020): Test that this is set.
                    ROSE_ASSERT(include_sourceFile->get_globalScope() != NULL);
#if 0
                    printf ("@@@@@@@@@@@@@@@@@@@@@@@@ include_file->set_source_file(): include_file = %p include_sourceFile = %p filename = %s \n",include_file,include_sourceFile,filename.c_str());
#endif
                 // DQ (10/25/2019): Can we enforce this?
                    ROSE_ASSERT(include_file->get_source_file() != NULL);

                 // DQ (11/10/2018): It might be enough that it is the parent, but build a more explicit connection.
                    include_sourceFile->set_associated_include_file(include_file);
                    include_sourceFile->set_parent(include_file);

                 // DQ (11/10/2018): This might be redundant with the constructor initialization.
                    include_sourceFile->set_isHeaderFile(true);
                    include_sourceFile->set_isHeaderFileIncludedMoreThanOnce(false);

                 // #if DEBUG_INCLUDE_TREE_SUPPORT
                 // DQ (4/5/2020): Added header file unparsing feature specific debug level.
                    if (SgProject::get_unparseHeaderFilesDebug() >= 8)
                       {
                         printf ("$$$$$$$$$$$$$$$$$$ Building new SgSourceFile (to support include file): include_sourceFile = %p filename = %s \n",include_sourceFile,include_sourceFile->getFileName().c_str());
                       }
                 // #endif

                 // DQ (11/10/2018): Thisis not built in the EDG/ROSE translation.
                 // DQ (9/26/2018): This can not be set yet.
                 // ROSE_ASSERT(include_file->get_source_file() == NULL);
                    ROSE_ASSERT(include_file->get_source_file() != NULL);
                    ROSE_ASSERT(include_file->get_include_file_list().empty() == true);

#if 0
                    printf ("In build_header_file_tree(): build new SgSourceFile: include_sourceFile = %p filename = %s \n",include_sourceFile,include_sourceFile->getFileName().c_str());
#endif
                 // DQ (4/25/2020): I think this is marked incorrectly.
                    if (include_sourceFile->get_unparseHeaderFiles() == false)
                       {
                      // DQ (4/25/2020): When this is not set, we need to check if it should be set.

                         ROSE_ASSERT(rose_source_file != NULL);
#if 0
                         printf ("rose_source_file = %p name = %s \n",rose_source_file,rose_source_file->getFileName().c_str());
#endif
                         SgProject* project = rose_source_file->get_project();
                      // SgFileList & fileList = project->get_files();
                         SgFileList* fileList = project->get_fileList_ptr();
                         for (size_t i = 0; i < fileList->get_listOfFiles().size(); i++)
                            {
#if 0
                              printf (" --- i = %zu \n",i);
#endif
                              SgSourceFile* sourceFile = isSgSourceFile(fileList->get_listOfFiles()[i]);
                              ROSE_ASSERT(sourceFile != NULL);
#if 0
                              printf (" --- sourceFile = %s sourceFile->get_unparseHeaderFiles() = %s \n",sourceFile->getFileName().c_str(),sourceFile->get_unparseHeaderFiles() ? "true" : "false");
#endif
                              if (sourceFile->get_unparseHeaderFiles() == true)
                                 {
#if 0
                                   printf ("In build_header_file_tree(): Setting include_sourceFile->set_unparseHeaderFiles(true) \n");
#endif
                                // DQ (4/25/2020): There is not global mechanism to see if we are unparsing header files, 
                                // it is set in each source file (and in the source file (SgSourceFile) constructed to for 
                                // each include file.
                                   include_sourceFile->set_unparseHeaderFiles(true);
                                 }
                            }
#if 0
                         if (project->get_unparseHeaderFiles() == true)
                            {
                              printf ("In build_header_file_tree(): Setting include_sourceFile->set_unparseHeaderFiles(true) \n");
                              include_sourceFile->set_unparseHeaderFiles(true);
                            }
#endif
#if 0
                         printf ("ERROR: include_sourceFile->get_unparseHeaderFiles() == false and should be true for testing unparseHeaderTests test8 \n");
                         ROSE_ASSERT(false);
#endif
                       }
                  }
             }
            else
             {
            // #if DEBUG_INCLUDE_TREE_SUPPORT
            // DQ (4/5/2020): Added header file unparsing feature specific debug level.
               if (SgProject::get_unparseHeaderFilesDebug() >= 8)
                  {
                    printf ("Skipping the build of SgSourceFile for header files included via preinclude! \n");
                  }
            // #endif

               if (name_used_in_include_directive == "rose_edg_required_macros_and_functions.h")
                 {
                   include_file->set_isRoseSystemInclude(true);
#if 0
                   printf ("Exiting as a test! \n");
                   ROSE_ASSERT(false);
#endif
                 }

#if 0
               if (included_by_preinclude == false && isApplicationFile == false)
                  {
                    printf ("Adding header file to the list of header files in edg_include_file_map \n");

#if 0
                   printf ("Exiting as a test! \n");
                   ROSE_ASSERT(false);
#endif
                  }
#endif
            // DQ (10/25/2019): Can we enforce this?
               ROSE_ASSERT(include_file->get_source_file() == NULL);

#if 0
               printf ("Exiting as a test! \n");
               ROSE_ASSERT(false);
#endif
             }

#if 0
       // DQ (10/26/2019): Can we enforce this?
       // ROSE_ASSERT(include_file->get_source_file() != NULL);
          if (include_file->get_source_file() == NULL)
            {
              printf ("NOTE: include_file->get_source_file() == NULL: include_file = %p include_file->get_filename() = %s \n",include_file,include_file->get_filename().str());
              printf (" --- included_by_preinclude = %s \n",included_by_preinclude ? "true" : "false");
              printf (" --- isApplicationFile = %s \n",isApplicationFile ? "true" : "false");
              if (included_by_preinclude == false)
                 {
                   printf ("A SgSourceFile will be added in the unparser if this header file is required to be unparsed \n");
#if 0
                   printf ("Exiting as a test! \n");
                   ROSE_ASSERT(false);
#endif
                 }
            }
#endif

#if 0
       // DQ (11/12/2018): Moved to the included_by_preinclude == false branch above.
          if (edg_include_file_map.find(filename) != edg_include_file_map.end())
             {
            // Note: redundant entries will not be put into the include file map.
            // Note clear if this is a correct implementation.
#if 0
               printf ("This include file has already been seen! \n");
#endif
               include_file->set_isIncludedMoreThanOnce(true);

            // And set the initial entry that was originally put into the map.
               edg_include_file_map[filename]->set_isIncludedMoreThanOnce(true);

            // Mark the primary use as a way to, at least initially simplify the implementation.
               edg_include_file_map[filename]->set_isPrimaryUse(true);

            // DQ (11/11/2018): Find the associated SgSourceFile and mark it as something???
#if 0
               printf ("Exiting as a test! \n");
               ROSE_ASSERT(false);
#endif
             }
#endif

       // #if DEBUG_INCLUDE_TREE_SUPPORT || 0
       // DQ (4/5/2020): Added header file unparsing feature specific debug level.
          if (SgProject::get_unparseHeaderFilesDebug() >= 8)
             {
               printf ("@@@@@@@@@@@@@@ Adding to edg_include_file_map: filename = %s include_file = %p \n",filename.c_str(),include_file);
             }
       // #endif

       // DQ (11/16/2019): Check that this is not alredy in the map.
       // ROSE_ASSERT (edg_include_file_map.find(filename) == edg_include_file_map.end());

       // DQ (11/16/2019): Check that this is not alredy in the map.
          if (edg_include_file_map.find(filename) == edg_include_file_map.end())
             {
               ROSE_ASSERT(include_file != NULL);
#if 0
               printf ("Adding SgIncludeFile* to edg_include_file_map: filename = %s include_file = %p = %s \n",filename.c_str(),include_file,include_file->get_filename().str());
#endif
               edg_include_file_map.insert(pair<string,SgIncludeFile*>(filename,include_file));

            // #if DEBUG_INCLUDE_TREE_SUPPORT || 0
            // DQ (4/5/2020): Added header file unparsing feature specific debug level.
               if (SgProject::get_unparseHeaderFilesDebug() >= 8)
                  {
                    printf ("@@@@@@@@@@@@@@ edg_include_file_map.size() = %zu inserted include_file = %p using filename = %s \n",edg_include_file_map.size(),include_file,filename.c_str());
                  }
            // #endif

               a_seq_number first_seq_number = temp_file->first_seq_number;
               a_seq_number last_seq_number  = temp_file->last_seq_number;

            // #if DEBUG_INCLUDE_TREE_SUPPORT
            // DQ (4/5/2020): Added header file unparsing feature specific debug level.
               if (SgProject::get_unparseHeaderFilesDebug() >= 6)
                  {
                    printf ("first_seq_number = %u \n",first_seq_number);
                    printf ("last_seq_number  = %u \n",last_seq_number);
                  }
               // #endif

               include_file->set_first_source_sequence_number(first_seq_number);
               include_file->set_last_source_sequence_number(last_seq_number);

            // if (rose_source_file != NULL)
               ROSE_ASSERT(rose_source_file != NULL);

            // DQ (11/14/2018): We can assert this case in the new design!
               ROSE_ASSERT(parent_include_file != NULL);

               if (parent_include_file == NULL)
                  {
                    printf ("This case can't happen in the new design! \n");
                    ROSE_ASSERT(false);

                    ROSE_ASSERT(parent_include_file == NULL);
                 // DQ (11/15/2018): removed the include list from SgSourceFile so that we could support traversals on the inlcude tree.
                 // rose_source_file->get_include_file_list().push_back(include_file);
                  }
                 else
                  {
                    ROSE_ASSERT(parent_include_file != NULL);
                    parent_include_file->get_include_file_list().push_back(include_file);
                  }

            // DQ (11/12/2018): Detect case where we can't currently support token based unparsing of the header files.
            // Two different files included using the same name, specified differently as both "" (non-system) and <> (system) includes.
               string include_filename_key = name_as_written;

               if (edg_include_file_using_names_as_written_map.find(include_filename_key) != edg_include_file_using_names_as_written_map.end())
                  {
                 // #if DEBUG_INCLUDE_TREE_SUPPORT
                 // DQ (4/5/2020): Added header file unparsing feature specific debug level.
                    if (SgProject::get_unparseHeaderFilesDebug() >= 8)
                       {
                         printf ("Note: two different include files have been included using the same name \n");
                       }
                 // #endif

                    SgIncludeFile* current_include_file  = include_file;
                    SgIncludeFile* previous_include_file = edg_include_file_using_names_as_written_map[include_filename_key];
                    ROSE_ASSERT(current_include_file != NULL);
                    ROSE_ASSERT(previous_include_file != NULL);

                    bool isUsingConsistantSyntax = true;
                    if (current_include_file->get_isSystemInclude() != previous_include_file->get_isSystemInclude())
                       {
                      // DQ (11/30/2018): This setting is only an issue when using the token based unparsing.
                      // isUsingConsistantSyntax = false;

                      // #if DEBUG_INCLUDE_TREE_SUPPORT
                      // DQ (4/5/2020): Added header file unparsing feature specific debug level.
                         if (SgProject::get_unparseHeaderFilesDebug() >= 8)
                            {
                              printf ("rose_source_file->get_unparse_tokens() = %s \n",rose_source_file->get_unparse_tokens() ? "true" : "false");
                            }
                      // #endif
                         if (rose_source_file->get_unparse_tokens() == true)
                            {
                           // These are not marked consistantly as isSystemInclude
                              printf ("These are not marked consistantly as isSystemInclude(): \n");
                              printf ("current_include_file->isSystemInclude()  = %s \n",current_include_file->get_isSystemInclude() ? "true" : "false");
                              printf ("previous_include_file->isSystemInclude() = %s \n",previous_include_file->get_isSystemInclude() ? "true" : "false");

                              isUsingConsistantSyntax = false;
#if 0
                              printf ("Exiting as a test! \n");
                              ROSE_ASSERT(false);
#endif
                            }
                       }

                    current_include_file->set_can_be_supported_using_token_based_unparsing(isUsingConsistantSyntax);
                    previous_include_file->set_can_be_supported_using_token_based_unparsing(isUsingConsistantSyntax);

                    if (isUsingConsistantSyntax == false)
                       {
                      // Warn about header file name that is inconsistantly used to refer to two 
                      // different files using two different forms of include syntax.
                         printf ("\n\n");
                         printf ("****************************************************************************** \n");
                         printf ("WARNING: different files with the same name using inconsistant inlcude syntax: \n");
                         printf ("   --- current_include_file  = %p = %s \n",current_include_file,current_include_file->get_filename().str());
                         printf ("   --- previous_include_file = %p = %s \n",previous_include_file,previous_include_file->get_filename().str());
                         printf ("****************************************************************************** \n");
                         printf ("\n\n");
                       }
                  }

               edg_include_file_using_names_as_written_map.insert(pair<string,SgIncludeFile*>(include_filename_key,include_file));

            // Process this include for any nested include files.
            // build_header_file_tree(NULL,include_file,temp_file);
               build_header_file_tree(rose_source_file,include_file,temp_file);

            // DQ (10/25/2019): Can we enforce this?
            // ROSE_ASSERT(include_file->get_source_file() != NULL);
             }
            else
             {
#if 0
               printf ("Skipping overwriting an entry in edg_include_file_map filename = %s include_file = %p \n",filename.c_str(),include_file);
#endif
             }

          temp_file = temp_file->next;
        }

  // #if DEBUG_INCLUDE_TREE_SUPPORT
  // DQ (4/5/2020): Added header file unparsing feature specific debug level.
     if (SgProject::get_unparseHeaderFilesDebug() >= 7)
        {
          printf ("Leaving build_header_file_tree(): rose_source_file = %p parent_include_file = %p edg_source_file = %p \n",rose_source_file,parent_include_file,edg_source_file);
        }
  // #endif
   }

/////////////////////////////
// TODO mode to SageBuilder

#define DEBUG_buildReferenceExpressionFromSymbol 0

SgExpression * buildReferenceExpressionFromSymbol(SgSymbol * symbol) {
#if DEBUG_buildReferenceExpressionFromSymbol
  printf("Enter buildReferenceExpressionFromSymbol():");
  printf("  symbol = %p = %s\n", symbol, symbol->class_name().c_str());
#endif
  SgExpression * result = nullptr;
  switch (symbol->variantT()) {
    case V_SgFunctionSymbol:
      result = SageBuilder::buildFunctionRefExp_nfi((SgFunctionSymbol *)symbol);
      break;
    case V_SgMemberFunctionSymbol:
      result = SageBuilder::buildMemberFunctionRefExp_nfi((SgMemberFunctionSymbol *)symbol, false, false);
      break;
    case V_SgTemplateFunctionSymbol:
      result = SageBuilder::buildTemplateFunctionRefExp_nfi((SgTemplateFunctionSymbol *)symbol);
      break;
    case V_SgTemplateMemberFunctionSymbol:
      result = SageBuilder::buildTemplateMemberFunctionRefExp_nfi((SgTemplateMemberFunctionSymbol *)symbol, false, false);
      break;
    default: 
      printf ("Fatal: default reached in switch symbol = %p = %s\n", symbol, symbol->class_name().c_str());
      ROSE_ABORT();
  }
#if DEBUG_buildReferenceExpressionFromSymbol
  printf ("result = %p = %s\n", result, result->class_name().c_str());
#endif

  return result;
}

}
