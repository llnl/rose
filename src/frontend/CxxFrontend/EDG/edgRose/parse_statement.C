
#include "edgRose.h"

std::vector<SgStatement*>
EDG_ROSE_Translation::parse_statement(SourceSequenceList& sse, SgBasicBlock* existingBasicBlock)
   {
  // This can return multiple statements because some EDG compiler-generated
  // blocks (such as for loops) were not in the input code and are not
  // necessary to correct semantics of the output code

  // Note: the 2nd parameter is ONLY used when called from parse_function_body(); so that the already 
  // constructed SgBasicBlock, in the SgFunctionDefinition, can be used directly.

  // DQ (7/30/2013): Keep track of the depth of the number of recursive calls.
     static int depthCounter = 0;
     depthCounter++;

     vector<SgStatement*> result;

     ROSE_ASSERT (sse);
     an_il_entry_kind outer_kind = (an_il_entry_kind)(sse->entity.kind);

  // DQ (2/9/2013): The switch support requires a map of switch statement body entries (typically statements, sometimes declarations).
  // This map thus can't be a map between EDG a_statement_ptr and ROSE SgStatements, it must be something more gneral in EDG.
  // So we are using EDG's a_source_sequence_entry_ptr type for the map, and so we have to save this value to fill in the map 
  // at the base of this function.
     a_source_sequence_entry_ptr parse_statement_function_source_sequence_entry = NULL;

// DQ (3/23/2014): Control output of debugging info.
#define DEBUG_PARSE_STATEMENT 0

     switch (outer_kind)
        {
          case iek_statement:
             {
               a_statement_ptr p = (a_statement_ptr)(sse->entity.ptr);

               parse_statement_function_source_sequence_entry = p->source_sequence_entry;
               ROSE_ASSERT(parse_statement_function_source_sequence_entry != NULL);
#if DEBUG_PARSE_STATEMENT
               printf ("In parse_statement(): case of iek_statement: p = %p statement kind = %s \n",p,summarize_statement_in_string(p));
#endif

            // This is the better version of the code (though it fails for test2012_12.c (it passes all tests for everything else).
            // However, while test2012_14.c passes for C89, it fails for C99 (so that is strange).

            // DQ (10/23/2012): Test commenting out advancing the source sequence pointer (bad idea; fails to terminate).
               ++sse;
#if 0
               printf ("In parse_statement(): case of iek_statement: After sse increment: entry = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
#if 0
               printf ("##### In parse_statement(): has this body stmt_ptr = %p been seen previously = %s (in statement_expression_bodies) \n",
                    p,statement_expression_bodies.find(p) != statement_expression_bodies.end() ? "true" : "false");
#endif
               if (statement_expression_bodies.find(p) != statement_expression_bodies.end())
                  {
                 // Skip the source sequence entries, since the generated statement
                 // was already converted as part of a statement expression.
#ifdef DEBUG_CONVERSION
                    indent_line(); debug_print("skipping statement "); print_source_pos(p->position); debug_print("\n");
#endif
                 // DQ (10/25/2012): Refactored code.
                    skipOverSourceSequenceEntriesAlreadyVisited(sse,p);
#if 0
                    printf ("In parse_statement(): case of iek_statement: Jumping to END label of GOTO END: entry = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
                    goto end;
                  }

#ifdef DEBUG_CONVERSION
            // DQ (12/31/2008): EDG 4.0 does not work with the summarize_statement_in_string() function!
            // indent_line(); debug_print("statement: %s ",summarize_statement_in_string(p)); print_source_pos(p->position); debug_print("\n");
               indent_line(); debug_print("statement: start: "); print_source_pos(p->position); debug_print(" end: "); print_source_pos(p->end_position); debug_print("\n");
               ScopedIndent si(indent);
#endif
#if 0
               printf ("In parse_statement(): case of iek_statement: Calling summarize_statement() \n");
               summarize_statement(p);
#endif
               a_statement_kind kind = p->kind;

#if DEBUG_PARSE_STATEMENT || 0
               printf ("In parse_statement(): case of iek_statement: before switch over statement kinds: p = %p kind = %s \n",p,summarize_statement_in_string(p));
#endif
               switch (kind)
                  {
                    case stmk_expr:
                       {
                      // This is the normal expression statment in C and C++, not the GNU specific statement expression.
#ifdef DEBUG_CONVERSION
                         indent_line(); debug_print("expr statement p = %p\n",p);
#endif
                            {
#ifdef DEBUG_CONVERSION
                              ScopedIndent si(indent);
#endif
#if 0
                              printf ("In parse_statement(): case of iek_statement: case stmk_expr: Building an expression statment \n");
#endif
#if 0
                              printf ("In parse_statement(): case of iek_statement: Calling summarize_statement() \n");
                              summarize_statement(p);
#endif
#if 0
                              printf ("In parse_statement(): case of iek_statement: Calling summarize_expr(p->expr) \n");
                              summarize_expr(p->expr);
#endif
                              result.push_back(set_file_info_for_stmt(buildExprStatement_nfi(convert_expression(p->expr)), p));
#if 0
                              printf ("DONE: In parse_statement(): case of iek_statement: case stmk_expr: Building an expression statment \n");
#endif
#if 0
                           // DQ (10/29/2012): I don't this this is relevant.
                           // printf ("In parse_statement(): case stmk_expr: calling testAndSkipOverPreviouslyVisitedSourceSequenceEntries() p = %p \n",p);
                              testAndSkipOverPreviouslyVisitedSourceSequenceEntries(sse);
                           // printf ("DONE: In parse_statement(): case stmk_expr: calling testAndSkipOverPreviouslyVisitedSourceSequenceEntries() p = %p \n",p);
#endif
                            }
                         break;
                       }

                    case stmk_if:
                       {
#define DEBUG_IF_STATEMENT 0

#ifdef DEBUG_CONVERSION
                         indent_line(); debug_print("if statement (p = %p)\n",p);
                         ScopedIndent si(indent);
#endif
#if 0
                         printf ("In parse_statement(): case of iek_statement: case of stmk_if: Calling summarize_statement() \n");
                         summarize_statement(p);
#endif
                      // DQ (2/13/2012): Build the "if" scope first, so that we can use the correct scope (if the test contains a variable declaration).
                      // SgIfStmt* ifs = buildIfStmt_nfi(NULL,NULL,NULL);
                         SgIfStmt* ifs = new SgIfStmt((SgStatement*)NULL,(SgStatement*)NULL,(SgStatement*)NULL);
                         ROSE_ASSERT(ifs != NULL);

                      // DQ (3/15/2012): Set the parent as early as possible (so that any declaration can have propely computed mangled names for their types).
                         ifs->set_parent(topScopeStack());
#if DEBUG_PUSH_SCOPE
                         printf ("In parse_statement(): case stmk_if: push scope stack: if statement scope = %p = %s \n",ifs,ifs->class_name().c_str());
#endif
                      // Temporarily push the new scope onto the scope stack.
                         pushScopeStack (ifs);
#if DEBUG_IF_STATEMENT
                         printf ("In parse_statement(): case stmk_if: Built SgIfStmt and pushed that scope onto the stack \n");
                         printf ("In parse_statement(): case stmk_if: Calling convert_condition() \n");
#endif
                      // DQ (9/6/2013): Added support to skipping some entries between the conditional and the next statement ("then" branch).
                      // It is a problem that the scope of the SgIfStmt is build after the test is build, so the 
                      // SgIfStmtm can not be on the current scope stack. If the test is a simple variable declaration
                      // then the scope of the variable can not be properly represented.
                      // SgStatement* test = convert_condition(p->expr);
                      // SgStatement* test = convert_condition(p->expr,sse);
                         SgStatement* test = convert_condition(p->expr,sse,p->variant.if_stmt.then_statement);
#if DEBUG_IF_STATEMENT
                         printf ("In parse_statement(): case stmk_if: calling testAndSkipOverPreviouslyVisitedSourceSequenceEntries() after processing conditional expression: entry = %p kind = %s \n",
                              sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
                      // DQ (9/6/2013): I don't think this does anything here.  It is not a part of the solution for the GNU statement expression droppings.
                         testAndSkipOverPreviouslyVisitedSourceSequenceEntries(sse);
                      // printf ("In case stmk_if: skipping call to testAndSkipOverPreviouslyVisitedSourceSequenceEntries() \n");
#if DEBUG_IF_STATEMENT
                         printf ("***** In parse_statement(): case stmk_if: Building the true branch: p->variant.if_stmt.then_statement = %p \n",p->variant.if_stmt.then_statement);
#endif
                      // DQ (7/28/2013): Examples such as test2012_50.c demonstrate that we need to parse any type declarations plus 
                      // the final true branch given by p->variant.if_stmt.then_statement.  If we don't then the true branch will appear
                      // as a block after the if statement (incorrect code).
                      // SgStatement* then_stmt = p->variant.if_stmt.then_statement == NULL ? set_generated_file_info_to_unparse(buildBasicBlock_nfi()) : build_basic_block_if_multiple_statements(parse_statement(sse));

                      // DQ (7/30/2013): This causes a problem for test2013_34.c (double SgBasicBlock), but fixes test2013_35.c.
#if DEBUG_IF_STATEMENT
                         printf ("This causes a problem for test2013_34.c (double SgBasicBlock), but fixes test2013_35.c \n");
#endif
                      // DQ (7/30/2013): For the processing of the else case we want to call parse_statement() instead of parse_statement_list_without_terminator() 
                      // since if there is a label we will process the statement associated with the label and then have incremented past the last statement.
                      // This is important for test2013_35.c (and other C test codes as well).
                      // SgStatement* then_stmt = p->variant.if_stmt.then_statement == NULL ? set_generated_file_info_to_unparse(buildBasicBlock_nfi()) : build_basic_block_if_multiple_statements(parse_statement(sse));
                         SgStatement* then_stmt = NULL;
                         if (p->variant.if_stmt.then_statement != NULL)
                            {
                           // DQ (9/6/2013): This support for the entries to skip after the use of a GNU statement expression
                           // containing a type declaration is refactored into the base of the convert_conditional() function.
                           // DQ (7/30/2013): If this is not the then statement, then read off the first statement so that we can get to the then statement (see test2013_34.c).
                           // if (p->variant.if_stmt.then_statement != (a_statement*)sse->entity.ptr)
                           // if (p->variant.if_stmt.then_statement != (a_statement*)sse->entity.kind == iek_type)
                              if (sse->entity.kind == iek_type)
                                 {
#if DEBUG_IF_STATEMENT
                                // printf ("In parse_statement(): case stmk_if: (p->variant.if_stmt.then_statement != sse->entry->ptr): entry = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
                                   printf ("In parse_statement(): case stmk_if: (sse->entity.kind == iek_type): entry = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
                                   vector<SgStatement*> dummy_stmts = parse_statement(sse);

                                   printf ("In parse_statement(): case stmk_if: dummy_stmts = %" PRIuPTR " \n",dummy_stmts.size());

                                // ROSE_ASSERT(dummy_stmts.size() == 0);
                                 }

                           // vector<SgStatement*> stmts = parse_statement(sse);
                              then_stmt = build_basic_block_if_multiple_statements(parse_statement(sse));
                            }
                           else
                            {
                           // DQ (9/6/2013): I would like to know if this case can really happen.  Perhaps it is silly like: "if(1) else;"
                              printf ("NOTE: In parse_statement(): case stmk_if: Example of empty true case in stmk_if (p->variant.if_stmt.then_statement == NULL) \n");

                              then_stmt = set_generated_file_info_to_unparse(buildBasicBlock_nfi());
                            }

#if DEBUG_IF_STATEMENT
                         printf ("In parse_statement(): case stmk_if: then_stmt = %p = %s \n",then_stmt,then_stmt != NULL ? then_stmt->class_name().c_str() : "null");
#endif
#if DEBUG_IF_STATEMENT
                         printf ("In parse_statement(): case stmk_if: calling testAndSkipOverPreviouslyVisitedSourceSequenceEntries() after processing TRUE branch: entry = %p kind = %s \n",
                              sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
#if DEBUG_IF_STATEMENT
                         printf ("In parse_statement(): case stmk_if: calling testAndSkipOverPreviouslyVisitedSourceSequenceEntries() before processing FALSE branch: entry = %p kind = %s \n",
                              sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
                      // DQ (9/6/2013): I don't think this does anything here.  It is not a part of the solution for the GNU statmen expression droppings.
                         testAndSkipOverPreviouslyVisitedSourceSequenceEntries(sse);
#if DEBUG_IF_STATEMENT
                         printf ("In parse_statement(): case stmk_if: after calling testAndSkipOverPreviouslyVisitedSourceSequenceEntries() before processing FALSE branch: entry = %p kind = %s \n",
                              sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
#if DEBUG_IF_STATEMENT
                         printf ("**************************************************************************************************** \n");
                         printf ("**************************************************************************************************** \n");
                         printf ("**************************************************************************************************** \n");
                         printf ("***** In parse_statement(): case stmk_if: Building the false branch (if it exists) p->variant.if_stmt.else_statement = %p \n",p->variant.if_stmt.else_statement);
#endif
                      // DQ (7/30/2013): For the processing of the else case we want to call parse_statement() instead of parse_statement_list_without_terminator() 
                      // since if there is a label we will process the statement associated with the label and then have incremented past the last statement.
                      // This is important for test2013_35.c (and other C test codes as well).
                         SgStatement* else_stmt = p->variant.if_stmt.else_statement == NULL ? NULL : build_basic_block_if_multiple_statements(parse_statement(sse));
#if DEBUG_IF_STATEMENT
                         printf ("In parse_statement(): case stmk_if: else_stmt = %p = %s \n",else_stmt,else_stmt != NULL ? else_stmt->class_name().c_str() : "null");
                         printf ("**************************************************************************************************** \n");
                         printf ("**************************************************************************************************** \n");
                         printf ("**************************************************************************************************** \n");
#endif
#if DEBUG_IF_STATEMENT
                         printf ("In parse_statement(): case stmk_if: calling testAndSkipOverPreviouslyVisitedSourceSequenceEntries() after processing FALSE branch: entry = %p kind = %s \n",
                              sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
#if DEBUG_IF_STATEMENT
                         printf ("In parse_statement(): case stmk_if: calling SageInterface::initializeIfStmt() (true body = %p, false body = %p) \n",then_stmt,else_stmt);
#endif
#if 0
                      // DQ (1/6/2018): Let's try to avoid excessive normalization of the if false block (if it was not explicitly specified as a block).
                         printf ("Exiting as a test! \n");
                         ROSE_ASSERT(false);
#endif
                      // SgIfStmt* ifs = buildIfStmt_nfi(test, then_stmt, else_stmt);
                      // This is a SageInterface function: SageInterface::initializeIfStmt().
                         initializeIfStmt(ifs, test, then_stmt, else_stmt);
#if DEBUG_IF_STATEMENT
                         printf ("ifs->get_true_body()  = %p = %s \n",ifs->get_true_body(), ifs->get_true_body()  != NULL ? ifs->get_true_body() ->class_name().c_str() : "null");
                         printf ("ifs->get_false_body() = %p = %s \n",ifs->get_false_body(),ifs->get_false_body() != NULL ? ifs->get_false_body()->class_name().c_str() : "null");
#endif
#if DEBUG_POP_SCOPE
                         printf ("In parse_statement(): case stmk_if: calling popScopeStack() \n");
#endif
                      // Now pop the scope so that the normal mechanism can handle the scope stack.
                         popScopeStack();

                         set_file_info_for_stmt(ifs, p);
                         result.push_back(ifs);
#if DEBUG_IF_STATEMENT
                         printf ("DONE: In parse_statement(): case stmk_if: p = %p ifs = %p \n",p,ifs);
#endif
                         break;
                       }

                 // DQ (7/26/2020): Adding support for C++17 constexpr if-then-else.
                    case stmk_constexpr_if:
                       {
#define DEBUG_CONSTEXPR_IF_STATEMENT 0

#ifdef DEBUG_CONVERSION
                         indent_line(); debug_print("if statement (p = %p)\n",p);
                         ScopedIndent si(indent);
#endif
#if 0
                         printf ("In parse_statement(): case of iek_statement: case of stmk_constexpr_if: Calling summarize_statement() \n");
                         summarize_statement(p);
#endif
                      // DQ (2/13/2012): Build the "if" scope first, so that we can use the correct scope (if the test contains a variable declaration).
                      // SgIfStmt* ifs = buildIfStmt_nfi(NULL,NULL,NULL);
                         SgIfStmt* ifs = new SgIfStmt((SgStatement*)NULL,(SgStatement*)NULL,(SgStatement*)NULL);
                         ROSE_ASSERT(ifs != NULL);

                      // DQ (3/15/2012): Set the parent as early as possible (so that any declaration can have propely computed mangled names for their types).
                         ifs->set_parent(topScopeStack());
#if DEBUG_PUSH_SCOPE
                         printf ("In parse_statement(): case stmk_constexpr_if: push scope stack: if statement scope = %p = %s \n",ifs,ifs->class_name().c_str());
#endif
                      // Temporarily push the new scope onto the scope stack.
                         pushScopeStack (ifs);
#if DEBUG_CONSTEXPR_IF_STATEMENT
                         printf ("In parse_statement(): case stmk_constexpr_if: Built SgIfStmt and pushed that scope onto the stack \n");
                         printf ("In parse_statement(): case stmk_constexpr_if: Calling convert_condition() \n");
#endif
                      // DQ (9/6/2013): Added support to skipping some entries between the conditional and the next statement ("then" branch).
                      // It is a problem that the scope of the SgIfStmt is build after the test is build, so the 
                      // SgIfStmtm can not be on the current scope stack. If the test is a simple variable declaration
                      // then the scope of the variable can not be properly represented.
                      // SgStatement* test = convert_condition(p->expr);
                      // SgStatement* test = convert_condition(p->expr,sse);
                      // SgStatement* test = convert_condition(p->expr,sse,p->variant.if_stmt.then_statement);
                         ROSE_ASSERT(p->variant.constexpr_if != NULL);
                         SgStatement* test = convert_condition(p->expr,sse,p->variant.constexpr_if->then_statement);
#if DEBUG_CONSTEXPR_IF_STATEMENT
                         printf ("In parse_statement(): case stmk_constexpr_if: calling testAndSkipOverPreviouslyVisitedSourceSequenceEntries() after processing conditional expression: entry = %p kind = %s \n",
                              sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
                      // DQ (9/6/2013): I don't think this does anything here.  It is not a part of the solution for the GNU statement expression droppings.
                         testAndSkipOverPreviouslyVisitedSourceSequenceEntries(sse);
                      // printf ("In case stmk_constexpr_if: skipping call to testAndSkipOverPreviouslyVisitedSourceSequenceEntries() \n");
#if DEBUG_CONSTEXPR_IF_STATEMENT
                         printf ("***** In parse_statement(): case stmk_constexpr_if: Building the true branch: p->variant.constexpr_if->then_statement = %p \n",
                              p->variant.constexpr_if->then_statement);
#endif
                      // DQ (7/28/2013): Examples such as test2012_50.c demonstrate that we need to parse any type declarations plus 
                      // the final true branch given by p->variant.if_stmt.then_statement.  If we don't then the true branch will appear
                      // as a block after the if statement (incorrect code).
                      // SgStatement* then_stmt = p->variant.stmk_constexpr_if->then_statement == NULL ? set_generated_file_info_to_unparse(buildBasicBlock_nfi()) : build_basic_block_if_multiple_statements(parse_statement(sse));

                      // DQ (7/30/2013): This causes a problem for test2013_34.c (double SgBasicBlock), but fixes test2013_35.c.
#if DEBUG_CONSTEXPR_IF_STATEMENT
                         printf ("This causes a problem for test2013_34.c (double SgBasicBlock), but fixes test2013_35.c \n");
#endif
                      // DQ (7/30/2013): For the processing of the else case we want to call parse_statement() instead of parse_statement_list_without_terminator() 
                      // since if there is a label we will process the statement associated with the label and then have incremented past the last statement.
                      // This is important for test2013_35.c (and other C test codes as well).
                      // SgStatement* then_stmt = p->variant.stmk_constexpr_if->then_statement == NULL ? set_generated_file_info_to_unparse(buildBasicBlock_nfi()) : build_basic_block_if_multiple_statements(parse_statement(sse));
                         SgStatement* then_stmt = NULL;
                      // if (p->variant.if_stmt.then_statement != NULL)
                         if (p->variant.constexpr_if->then_statement != NULL)
                            {
                           // DQ (9/6/2013): This support for the entries to skip after the use of a GNU statement expression
                           // containing a type declaration is refactored into the base of the convert_conditional() function.
                           // DQ (7/30/2013): If this is not the then statement, then read off the first statement so that we can get to the then statement (see test2013_34.c).
                           // if (p->variant.if_stmt.then_statement != (a_statement*)sse->entity.ptr)
                           // if (p->variant.if_stmt.then_statement != (a_statement*)sse->entity.kind == iek_type)
                              if (sse->entity.kind == iek_type)
                                 {
#if DEBUG_CONSTEXPR_IF_STATEMENT
                                // printf ("In parse_statement(): case stmk_constexpr_if: (p->variant.if_stmt.then_statement != sse->entry->ptr): entry = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
                                   printf ("In parse_statement(): case stmk_constexpr_if: (sse->entity.kind == iek_type): entry = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
                                   vector<SgStatement*> dummy_stmts = parse_statement(sse);

                                   printf ("In parse_statement(): case stmk_constexpr_if: dummy_stmts = %" PRIuPTR " \n",dummy_stmts.size());

                                // ROSE_ASSERT(dummy_stmts.size() == 0);
                                 }

                           // vector<SgStatement*> stmts = parse_statement(sse);
                              then_stmt = build_basic_block_if_multiple_statements(parse_statement(sse));
                            }
                           else
                            {
                           // DQ (9/6/2013): I would like to know if this case can really happen.  Perhaps it is silly like: "if(1) else;"
                              printf ("NOTE: In parse_statement(): case stmk_constexpr_if: Example of empty true case in stmk_constexpr_if (p->variant.constexpr_if->then_statement == NULL) \n");

                              then_stmt = set_generated_file_info_to_unparse(buildBasicBlock_nfi());
                            }

#if DEBUG_CONSTEXPR_IF_STATEMENT
                         printf ("In parse_statement(): case stmk_constexpr_if: then_stmt = %p = %s \n",then_stmt,then_stmt != NULL ? then_stmt->class_name().c_str() : "null");
#endif
#if DEBUG_CONSTEXPR_IF_STATEMENT
                         printf ("In parse_statement(): case stmk_constexpr_if: calling testAndSkipOverPreviouslyVisitedSourceSequenceEntries() after processing TRUE branch: entry = %p kind = %s \n",
                              sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
#if DEBUG_CONSTEXPR_IF_STATEMENT
                         printf ("In parse_statement(): case stmk_constexpr_if: calling testAndSkipOverPreviouslyVisitedSourceSequenceEntries() before processing FALSE branch: entry = %p kind = %s \n",
                              sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
                      // DQ (9/6/2013): I don't think this does anything here.  It is not a part of the solution for the GNU statmen expression droppings.
                         testAndSkipOverPreviouslyVisitedSourceSequenceEntries(sse);
#if DEBUG_CONSTEXPR_IF_STATEMENT
                         printf ("In parse_statement(): case stmk_constexpr_if: after calling testAndSkipOverPreviouslyVisitedSourceSequenceEntries() before processing FALSE branch: entry = %p kind = %s \n",
                              sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
#if DEBUG_CONSTEXPR_IF_STATEMENT
                         printf ("**************************************************************************************************** \n");
                         printf ("**************************************************************************************************** \n");
                         printf ("**************************************************************************************************** \n");
                         printf ("***** In parse_statement(): case stmk_constexpr_if: Building the false branch (if it exists) p->variant.if_stmt.else_statement = %p \n",
                              p->variant.constexpr_if->else_statement);
#endif
                      // DQ (7/30/2013): For the processing of the else case we want to call parse_statement() instead of parse_statement_list_without_terminator() 
                      // since if there is a label we will process the statement associated with the label and then have incremented past the last statement.
                      // This is important for test2013_35.c (and other C test codes as well).
                      // SgStatement* else_stmt = p->variant.if_stmt.else_statement == NULL ? NULL : build_basic_block_if_multiple_statements(parse_statement(sse));
                         SgStatement* else_stmt = p->variant.constexpr_if->else_statement == NULL ? NULL : build_basic_block_if_multiple_statements(parse_statement(sse));
#if DEBUG_CONSTEXPR_IF_STATEMENT
                         printf ("In parse_statement(): case stmk_constexpr_if: else_stmt = %p = %s \n",else_stmt,else_stmt != NULL ? else_stmt->class_name().c_str() : "null");
                         printf ("**************************************************************************************************** \n");
                         printf ("**************************************************************************************************** \n");
                         printf ("**************************************************************************************************** \n");
#endif
#if DEBUG_CONSTEXPR_IF_STATEMENT
                         printf ("In parse_statement(): case stmk_constexpr_if: calling testAndSkipOverPreviouslyVisitedSourceSequenceEntries() after processing FALSE branch: entry = %p kind = %s \n",
                              sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
#if DEBUG_CONSTEXPR_IF_STATEMENT
                         printf ("In parse_statement(): case stmk_constexpr_if: calling SageInterface::initializeIfStmt() (true body = %p, false body = %p) \n",then_stmt,else_stmt);
#endif
#if 0
                      // DQ (1/6/2018): Let's try to avoid excessive normalization of the if false block (if it was not explicitly specified as a block).
                         printf ("Exiting as a test! \n");
                         ROSE_ASSERT(false);
#endif
                      // SgIfStmt* ifs = buildIfStmt_nfi(test, then_stmt, else_stmt);
                      // This is a SageInterface function: SageInterface::initializeIfStmt().
                         initializeIfStmt(ifs, test, then_stmt, else_stmt);
#if DEBUG_CONSTEXPR_IF_STATEMENT
                         printf ("ifs->get_true_body()  = %p = %s \n",ifs->get_true_body(), ifs->get_true_body()  != NULL ? ifs->get_true_body() ->class_name().c_str() : "null");
                         printf ("ifs->get_false_body() = %p = %s \n",ifs->get_false_body(),ifs->get_false_body() != NULL ? ifs->get_false_body()->class_name().c_str() : "null");
#endif
#if DEBUG_POP_SCOPE
                         printf ("In parse_statement(): case stmk_constexpr_if: calling popScopeStack() \n");
#endif
                      // Now pop the scope so that the normal mechanism can handle the scope stack.
                         popScopeStack();

                         set_file_info_for_stmt(ifs, p);
                         result.push_back(ifs);

#if DEBUG_CONSTEXPR_IF_STATEMENT
                         printf ("In parse_statement(): case stmk_constexpr_if: value_known = %s \n",p->variant.constexpr_if->value_known ? "true" : "false");
                         printf ("In parse_statement(): case stmk_constexpr_if: value       = %s \n",p->variant.constexpr_if->value ? "true" : "false");
#endif
                         ifs->set_is_if_constexpr_statement(true);
                         ifs->set_if_constexpr_value_known(p->variant.constexpr_if->value_known);
                         ifs->set_if_constexpr_value(p->variant.constexpr_if->value);

#if DEBUG_CONSTEXPR_IF_STATEMENT
                         printf ("DONE: In parse_statement(): case stmk_constexpr_if: p = %p ifs = %p \n",p,ifs);
#endif
#if 0
                         printf ("In parse_statement(): case stmk_constexpr_if: not implemented! \n");
                         ROSE_ASSERT(false);
#endif
                         break;
                       }

                    case stmk_while:
                       {
#ifdef DEBUG_CONVERSION
                         indent_line(); debug_print("while p = %p\n",p);
                         ScopedIndent si(indent);
#endif
                      // DQ (2/15/2012): Build the "switch" scope first, so that we can use the correct scope (if the test contains a variable declaration).
                         SgWhileStmt* ws = new SgWhileStmt((SgStatement*)NULL,(SgStatement*)NULL);

                      // DQ (6/19/2012): Need to set the parent so that the scope can be computed for nested types generated in test2012_105.C.
                         ws->set_parent(topScopeStack());
#if 0
                         printf ("In parse_statement(): ws = %p = %s \n",ws,ws->class_name().c_str());
#endif
#if DEBUG_PUSH_SCOPE
                         printf ("In parse_statement(): case stmk_while: push scope stack: while statement scope = %p = %s \n",ws,ws->class_name().c_str());
#endif
                      // Temporarily push the new scope onto the scope stack.
                         pushScopeStack (ws);
#if 0
                         printf ("In parse_statement(): case stmk_while: calling summarize_statement() for while statement p = %p \n",p);
                         summarize_statement(p);
#endif
#if 0
                         printf ("In parse_statement(): case stmk_while: calling summarize_expr() for conditional test p->expr = %p \n",p->expr);
                         summarize_expr(p->expr);
#endif
#if 0
                         printf ("Calling convert_condition(): entry = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
                      // DQ (9/6/2013): Added support to skipping some entries between the conditional and the next statement ("loop_statement").
                      // SgStatement* test = convert_condition(p->expr);
                      // SgStatement* test = convert_condition(p->expr,sse);
                         SgStatement* test = convert_condition(p->expr,sse,p->variant.loop_statement);
#if 0
                         printf ("DONE: Calling convert_condition(): entry = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
#if 0
                      // DQ (9/6/2013): If a declaration in an statement expression was in the conditional, then the next source sequence entry will 
                      // point to the type and then the end fo the block which we have to skip over. It is not yet clear how to skip over this. 
                      // The consequence is that the processing of the body (below) will skip over the type (correctly) and build an empty block when 
                      // it ends the collection of statements at the src-seq-end-of-construct (after the type).  It might be a better solution to have 
                      // the processing of the type increment past the src-seq-end-of-construct and stop.

#error "DEAD CODE!"

                         a_statement_ptr loop_statement = p->variant.loop_statement;
                         if (loop_statement != NULL)
                            {
                           // DQ (9/6/2013): Now we know that we have a valid loop body for the while statement, so if this is a type we need 
                           // to skip over it (similar to the processing for the stmk_if case (above).
                              printf ("Now we know we have a valid loop body for the while statement \n");
#if 0
                              printf ("In parse_statement(): case stmk_while: (sse->entity.kind == iek_type): entry = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
                              if (sse->entity.kind == iek_type)
                                 {
#if 0
                                   printf ("In parse_statement(): case stmk_while: (sse->entity.kind == iek_type): entry = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
                                   vector<SgStatement*> dummy_stmts = parse_statement(sse);

#error "DEAD CODE!"

                                   printf ("In parse_statement(): case stmk_while: dummy_stmts = %" PRIuPTR " \n",dummy_stmts.size());

                                // ROSE_ASSERT(dummy_stmts.size() == 0);
                                 }
                                else
                                 {
                                // For more complex cases we need to just increment the sse until we reach the next statement (which we know is the loop_statement).
                                   printf ("We might want to increment the sse until we reach the loop_statement = %p \n",loop_statement);

#error "DEAD CODE!"

                                // DQ (9/6/2013): I think it is dangerous to loop while not equal since this could be a significant problem 
                                // if the loop_statement is not ahead in the source sequence list.
                                   while ((a_statement_ptr)(sse->entity.ptr) != loop_statement)
                                      {
                                        printf ("Increment sse until we reach loop_statement = %p \n",loop_statement);
#if 0
                                        printf ("In parse_statement(): case stmk_while: BEFORE: (increment until we reach the loop_statement): entry = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
                                        ++sse;
#if 0
                                        printf ("In parse_statement(): case stmk_while: AFTER: (increment until we reach the loop_statement): entry = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
                                      }
#error "DEAD CODE!"

                                 }
#if 0
                              printf ("Exiting as a test! \n");
                              ROSE_ASSERT(false);
#endif
                            }
#endif
#if 0
                         printf ("In parse_statement(): case stmk_while: after convert_condition(): calling testAndSkipOverPreviouslyVisitedSourceSequenceEntries() after processing conditional expression: entry = %p kind = %s \n",
                              sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
                      // DQ (9/5/2013): Need to handle where statement expressions may have had a declaration and added a scope to support the declaration.
                         testAndSkipOverPreviouslyVisitedSourceSequenceEntries(sse);
#if 0
                         printf ("In parse_statement(): case stmk_while: PROCESS BODY: Calling parse_statement() entry = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
                      // SgStatement* body = build_basic_block_if_multiple_statements(parse_statement(sse));
                         const vector<SgStatement*> & statementList = parse_statement(sse);
#if 0
                         printf ("In parse_statement(): case stmk_while: PROCESS BODY: Calling build_basic_block_if_multiple_statements() entry = %p kind = %s \n",
                              sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
                      // SgStatement* body = build_basic_block_if_multiple_statements(parse_statement(sse));
                         SgStatement* body = build_basic_block_if_multiple_statements(statementList);
#if 0
                         printf ("In parse_statement(): case stmk_while: AFTER PROCESS BODY: calling testAndSkipOverPreviouslyVisitedSourceSequenceEntries() after processing conditional expression: entry = %p kind = %s \n",
                              sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
                      // DQ (9/5/2013): Need to handle where statement expressions may have had a declaration and added a scope to support the declaration.
                         testAndSkipOverPreviouslyVisitedSourceSequenceEntries(sse);
#if 0
                         printf ("In parse_statement(): case stmk_while: Calling initializeWhileStatement(): test = %p = %s body = %p = %s \n",test,test->class_name().c_str(),body,body->class_name().c_str());
#endif
                      // SgWhileStmt* ws = buildWhileStmt_nfi(test, body);
                      // This is a SageInterface function: SageInterface::initializeSwitchStatement().
                         initializeWhileStatement(ws, test, body, NULL);
#if DEBUG_POP_SCOPE
                         printf ("In parse_statement(): case stmk_while: calling popScopeStack() \n");
#endif
                      // Now pop the scope so that the normal mechanism can handle the scope stack.
                         popScopeStack();

                         set_file_info_for_stmt(ws, p);
                         result.push_back(ws);
                         break;
                       }

// DQ (2/3/2013): Control output of debugging info for case stmk_switch.
#define DEBUG_SWITCH 0

                    case stmk_switch:
                       {
#ifdef DEBUG_CONVERSION
                         indent_line(); debug_print("switch statement p = %p\n",p);
                         ScopedIndent si(indent);
#endif
                      // DQ (2/15/2012): Build the "switch" scope first, so that we can use the correct scope (if the test contains a variable declaration).
                         SgSwitchStatement* sw = new SgSwitchStatement((SgStatement*)NULL,(SgStatement*)NULL);
                         ROSE_ASSERT(sw != NULL);

                     // DQ (11/3/2012): Set the default source position (usually called from SageBuilder interface functions).
                        setSourcePosition(sw);

#if DEBUG_PUSH_SCOPE
                         printf ("In parse_statement(): case stmk_switch: push scope stack: switch statement scope = %p = %s \n",sw,sw->class_name().c_str());
#endif
                      // DQ (10/30/2012): Set the parent as early as possible (so that any declaration can have propely computed mangled names for their types).
                         sw->set_parent(topScopeStack());

                      // Temporarily push the new scope onto the scope stack.
                         pushScopeStack(sw);

#if NEW_DUFF_DEVICE_SWITCH_SUPPORT
                      // DQ (11/22/2017): New version of code, to support handling duff's device. Make this a command line option to use this older version of the switch implementation.

                         an_expr_node_ptr        edg_switch_key     = p->expr;
                         a_statement_ptr         edg_body_statement = p->variant.switch_stmt.body_statement;
                         a_switch_stmt_descr_ptr edg_extra_info     = p->variant.switch_stmt.extra_info;

                         ROSE_ASSERT(edg_body_statement != NULL);
                         ROSE_ASSERT(edg_extra_info != NULL);

                         SgStatement* switch_key  = convert_condition(edg_switch_key,sse,edg_body_statement);
                         ROSE_ASSERT(switch_key != NULL);

                         sw->set_item_selector(switch_key);

#if DEBUG_SWITCH
                         printf ("In parse_statement(): case stmk_switch: current sse: (a_statement_ptr)sse->entity.ptr = %p = %s \n",
                              (a_statement_ptr)sse->entity.ptr,summarize_statement_in_string((a_statement_ptr)sse->entity.ptr));
#endif
#if DEBUG_PRINT_SCOPE_STACK
                         printScopeStack("In parse_statement(): case stmk_switch");
#endif

                      // SgStatement* block = parse_statement(sse);
                      // ROSE_ASSERT(block != NULL);
                         std::vector<SgStatement*> statement_list = parse_statement(sse);

#if DEBUG_SWITCH
                         printf ("In parse_statement(): case stmk_switch: statement_list.size() = %zu \n",statement_list.size());
#endif

                      // DQ (11/24/2017): Add the body statement to the switch statement.
                         SgStatement* body = build_basic_block_if_multiple_statements(statement_list);
                         ROSE_ASSERT(body != NULL);

                         sw->set_body(body);

                      // And set the parent.
                         body->set_parent(sw);

                         ROSE_ASSERT (sw->get_body() != NULL);

                      // Put the switch statement into the ROSE AST with source position information.
                         result.push_back(set_file_info_for_stmt(sw, p));

#if DEBUG_POP_SCOPE
                         printf ("In parse_statement(): case stmk_switch: (pop SgSwitchStatement) calling popScopeStack() \n");
#endif
                      // Now pop the scope so that the normal mechanism can handle the scope stack.
                         popScopeStack();

#else
                      // DQ (11/22/2017): Old version of code, make this a command line option to use this older version 
                      // of the switch implementation (not handling duff's device) (default to old version).

                      // DQ (10/23/2012): Added body to switch.
                         SgBasicBlock* body_of_switch = buildBasicBlock_nfi();
                         ROSE_ASSERT(body_of_switch != NULL);

                         sw->set_body(body_of_switch);

                         body_of_switch->set_parent(sw);

                         ROSE_ASSERT(body_of_switch->get_parent() == sw);

                      // DQ (12/28/2014): Set the source position of the body of the switch (initial value for the startOfConstruct, and final value for the endOfConstruct).
                      // *(body_of_switch->get_startOfConstruct()) = *(sw->get_startOfConstruct());
                      // *(body_of_switch->get_endOfConstruct())   = *(sw->get_endOfConstruct());

#if DEBUG_SWITCH
                         printf ("In parse_statement(): case stmk_switch: calling convert_condition() for p->expr = %p \n",p->expr);
#endif
                      // DQ (9/9/2013): Using the new block mechanism (passing 3rd non-null argument to convert_condition() function) 
                      // fails for test2012_84.c.
                      // DQ (9/6/2013): Added support to handle skipping entries between the end of the conditional and the switch statement body.
                      // When the GNU statement expression appears in the switch statement this new code will skip a bit too much and we generate 
                      // the wrong code (slightly different).  So put back the use of NULL (default argument for convert_condition()).
                      // DQ (11/5/2012): Build the conditional before push the switch body onto the stack.
                      // SgStatement* key  = convert_condition(p->expr,sse);
                         SgStatement* key  = convert_condition(p->expr,sse,p->variant.switch_stmt.body_statement);

                         sw->set_item_selector(key);
#if DEBUG_SWITCH
                         printf ("In parse_statement(): case stmk_switch: body_of_switch = %p not put into scope_cache \n",body_of_switch);
#endif
                         ROSE_ASSERT(statement_scope_cache.find(p) == statement_scope_cache.end());
                         statement_scope_cache[p] = body_of_switch;

#if DEBUG_PUSH_SCOPE
                         printf ("In parse_statement(): case stmk_switch: push scope stack: switch statement body_of_switch = %p = %s \n",body_of_switch,body_of_switch->class_name().c_str());
#endif
                         pushScopeStack(body_of_switch);

                      // The statement (called "substatement" in the C and C++ standards) controlled by the switch.  This is
                      // almost always a compound statement that itself contains stmk_switch_case statements.
                         a_statement_ptr switch_body_statement = p->variant.switch_stmt.body_statement;
#if 0
                         printf ("In parse_statement(): case stmk_switch: p->variant.switch_stmt.body_statement: calling summarize_statement(p->variant.switch_stmt.body_statement = %p) \n",p->variant.switch_stmt.body_statement);
                         summarize_statement(p->variant.switch_stmt.body_statement);
#endif
                      // DQ (12/28/2014): Set the source position of the body of the switch (initial value for the startOfConstruct, and final value for the endOfConstruct).
                         set_file_info_for_stmt(body_of_switch,switch_body_statement);
#if 0
                         body_of_switch->get_startOfConstruct()->display("startOfConstruct: body_of_switch");
                         body_of_switch->get_endOfConstruct  ()->display("endOfConstruct: body_of_switch");
#endif
#if 0
                         printf ("Exiting as a test! \n");
                         ROSE_ASSERT(false);
#endif
                      // DQ (2/8/2013): Adding switch support for duff's device (this is a vector of pointers to maps that map 
                      // edg statements to ROSE statements so that we can weave the SgCaseOptionStatement IR nodes into place 
                      // as a post-processing step as the end of the switch statement processing).
                         map<a_source_sequence_entry_ptr,SgStatement*>* switchSupportMap = new map<a_source_sequence_entry_ptr,SgStatement*>();
                         ROSE_ASSERT(switchSupportMap != NULL);
                         switchSupportStackOfMaps.push_back(switchSupportMap);
#if DEBUG_SWITCH
                         printf ("switchSupportStackOfMaps.size() = %" PRIuPTR " \n",switchSupportStackOfMaps.size());
#endif
                      // **************************************************************
                      // DQ (10/31/2012): I think it is useless to organize scopes relative to the edg a_scope_ptr values.
                      // They are hard to find and we only want to relate back to the SgScopeStatement, so we could just use 
                      // a map from void* pointers (or entity pointers) back to the SgScopeStatement values instead.
                      // The best edg pointers to use would be statements, since they capture all of the concepts of scope 
                      // that we need.  We we will have a seperate map of a_statement_ptr to SgScopeStatements (or modify 
                      // the existing map to use a_statement_ptr instead of a_scope_ptr).

                      // DQ (10/31/2012): This is the simple way to record the ROSE scope associated with any edg statement.
                         statement_scope_cache[p] = body_of_switch;

                      // Information about this switch statement (including fields describing the associated stmk_switch_case statements).
                         a_switch_stmt_descr_ptr switch_extra_info = p->variant.switch_stmt.extra_info;
#if DEBUG_SWITCH
                         printf ("In parse_statement(): case stmk_switch: body_statement = %p = %s extra_info = %p \n",switch_body_statement,summarize_statement_in_string(switch_body_statement),switch_extra_info);
#endif
#if DEBUG_SWITCH
                         printf ("In parse_statement(): case stmk_switch: calling summarize_statement(p = %p) \n",p);
                         summarize_statement(p);
#endif
#if DEBUG_SWITCH
                         printf ("In parse_statement(): case stmk_switch: calling summarize_switch_descriptor(switch_extra_info = %p) \n",switch_extra_info);
                         summarize_switch_descriptor(switch_extra_info);
#endif
#if 0
                         SgStatement* key  = convert_condition(p->expr,sse);
#error "DEAD CODE!"
                         sw->set_item_selector(key);
#endif
#if DEBUG_SWITCH
                         printf ("In parse_statement(): case stmk_switch: cases (in source code order, including default case) = %p \n",switch_extra_info->cases);
#endif
                         a_switch_case_entry_ptr edg_case_stmt = switch_extra_info->cases;

#if DEBUG_PRINT_SCOPE_STACK
                         printScopeStack("In parse_statement(): case stmk_switch");
#endif
                         if (edg_case_stmt == NULL)
                            {
                           // This is a case of "switch(i);"
#if DEBUG_SWITCH
                              printf ("In parse_statement(): case stmk_switch: There are no cases for this switch \n");
#endif
                            }
                           else
                            {
                           // Start of case where there exist cases in the switch.

                           // DQ (2/10/2013): I think we can assert this.
                              ROSE_ASSERT(edg_case_stmt->stmt != NULL);
#if DEBUG_SWITCH
                               printf ("In parse_statement(): case stmk_switch: first statement of case = %p = %s \n",edg_case_stmt->stmt,summarize_statement_in_string(edg_case_stmt->stmt));
#endif
#if DEBUG_SWITCH
                               printf ("Calling summarize_switch_case \n");
                               summarize_switch_case(edg_case_stmt);
#endif
#if 0
                           // a_statement_ptr p = (a_statement_ptr)(sse->entity.ptr);
                              while ((a_statement_ptr)sse->entity.ptr != edg_case_stmt->stmt)
                                 {
#error "DEAD CODE!"
                                   printf ("Increment sse to the start of the case option statement sse->entity.ptr = %p \n",sse->entity.ptr);
                                   ++sse;
                                 }
#endif
#if DEBUG_SWITCH
                              printf ("In parse_statement(): case stmk_switch: current sse: (a_statement_ptr)sse->entity.ptr = %p = %s \n",
                                   (a_statement_ptr)sse->entity.ptr,summarize_statement_in_string((a_statement_ptr)sse->entity.ptr));
#endif
                           // bool advancedOverStmtBlock = false;

                           // DQ (2/10/2013): Change this to check the switch_body_statement instead of the sse.
                           // DQ (2/9/2012): This test works for C++, but not for C language tests (see test2013_59.C and test2012_84.c).
                           // Note that this means that EDG is for C++ only, inserting a stmt_block as the body of the switch statement.
                           // For C code we need to handle the case of no stmt_block used for the body of the switch statement.
                           // if (sse->entity.kind == iek_statement && ((a_statement_ptr)sse->entity.ptr)->kind == stmk_block)
                           // if (sse->entity.kind == iek_statement && ((a_statement_ptr)sse->entity.ptr)->kind == stmk_block && (il_header.source_language == sl_Cplusplus))
                              if (switch_body_statement->kind == stmk_block)
                                 {
                                // If this is a stmk_block then we will just collect the whole block of statements instead of just the once associated with the first switch case.
                                   bool compilerGenerated = (switch_body_statement->position.seq == 0);
#if DEBUG_SWITCH
                                   printf ("Current entry is a block (already addressed by default design of switch in ROSE) (initial pass): so increment sse if not compiler generated: compilerGenerated = %s \n",compilerGenerated ? "true" : "false");
                                   debug_print("switch body statement: start: "); print_source_pos(switch_body_statement->position); debug_print(" end: "); print_source_pos(switch_body_statement->end_position); debug_print("\n");
#endif
                                // ROSE_ASSERT((sse->entity.kind == iek_statement) && ((a_statement_ptr)sse->entity.ptr)->kind == stmk_block);

                                // DQ (2/10/2013): Check if this stmt_block is in the source sequence list (if so then increment sse,
                                // else it is a compiler generated stmt_block (commonly generated by EDG for C).
                                // ++sse;
                                   if (compilerGenerated == false)
                                      {
                                        ROSE_ASSERT((sse->entity.kind == iek_statement) && ((a_statement_ptr)sse->entity.ptr)->kind == stmk_block);
                                        ++sse;
                                      }

                                // advancedOverStmtBlock = true;

                                // This is also true in this case.
                                   ROSE_ASSERT(switch_body_statement->kind == stmk_block);
                                 }
#if DEBUG_SWITCH
                              printf ("In parse_statement(): case stmk_switch: after increment over stmt_block: current sse: (a_statement_ptr)sse->entity.ptr = %p = %s \n",
                                      (a_statement_ptr)sse->entity.ptr,(sse->entity.kind == iek_statement) ? summarize_statement_in_string((a_statement_ptr)sse->entity.ptr) : "not a iek_statement");
#endif
                           // Traverse all of the statments in the statement body.
                              if (switch_body_statement->kind == stmk_block)
                                 {
                                // Note that the stmk_switch_case statements are in the list of statements of the switch body, but not in the source sequence list (what is up with that).
#if DEBUG_SWITCH
                                   printf ("In parse_statement(): case stmk_switch: Make a list of all statements in the switch body: \n");
#endif
                                   a_statement_ptr temp_statement = switch_body_statement->variant.block.statements;
                                   while (temp_statement != NULL)
                                      {
#if DEBUG_SWITCH
                                     // bool presentInSourceSequenceList = (((temp_statement->kind == stmk_switch_case) || (temp_statement->kind == stmk_label)) == false);
                                        bool presentInSourceSequenceList = (((temp_statement->kind == stmk_switch_case) || (temp_statement->kind == stmk_label) || (temp_statement->kind == stmk_init)) == false);
                                        printf ("   --- stmt = %p = %s (present in sse list = %s) \n",temp_statement,summarize_statement_in_string(temp_statement),presentInSourceSequenceList ? "true" : "false");
#endif
                                     // DQ (2/8/2013): Fill in the map with all of the EDG statement pointers.
#if DEBUG_SWITCH
                                        printf ("Adding edg statement = %p with source_sequence_entry = %p to the switchSupportMap = %p \n",temp_statement,temp_statement->source_sequence_entry,switchSupportMap);
#endif
                                     // switchSupportMap->[temp_statement] = NULL;
                                     // (*switchSupportMap)[temp_statement] = NULL;
                                     // ROSE_ASSERT(temp_statement->source_sequence_entry != NULL);
                                        if (temp_statement->source_sequence_entry != NULL)
                                           {
                                             (*switchSupportMap)[temp_statement->source_sequence_entry] = NULL;
                                           }
                                          else
                                           {
#if DEBUG_SWITCH
                                              printf ("NOTE: that temp_statement->source_sequence_entry == NULL \n");
#endif
                                           // DQ (5/13/2015): We can't assert this (see test2015_138.c).
                                           // DQ (2/9/2013): I assume that this is false if temp_statement->source_sequence_entry == NULL, check it...
                                           // ROSE_ASSERT(presentInSourceSequenceList == false);
                                           }

                                        temp_statement = temp_statement->next;
                                      }
                                 }
                                else
                                 {
                                   printf ("ERROR: switch_body_statement->kind != stmk_block \n");
                                   ROSE_ASSERT(false);
                                 }

                              a_statement_ptr next_statement = NULL;
                              if ((an_il_entry_kind)sse->entity.kind == iek_statement)
                                 {
                                // Check if this is the same as the first statement in the case option.  If so then this is a typical switch, 
                                // if not then there are statements before the first case option statement.
                                   next_statement = (a_statement_ptr)sse->entity.ptr;
#if DEBUG_SWITCH
                                   printf ("+++++++++++++++ next_statement = %p = %s \n",next_statement,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif

                                   a_statement_ptr first_statement_after_case_label = NULL;
                                   if (edg_case_stmt->stmt != NULL)
                                      {
                                        first_statement_after_case_label = edg_case_stmt->stmt->next;
#if DEBUG_SWITCH
                                        printf ("+++++++++++++++ initialized value: first_statement_after_case_label = %p \n",first_statement_after_case_label);
#endif
                                     // bool presentInSourceSequenceList = false;
                                     // presentInSourceSequenceList = (((first_statement_after_case_label->kind == stmk_switch_case) || (first_statement_after_case_label->kind == stmk_label) || (first_statement_after_case_label->kind == stmk_init)) == false);
                                        while (first_statement_after_case_label != NULL && (((first_statement_after_case_label->kind == stmk_switch_case) || (first_statement_after_case_label->kind == stmk_label) || (first_statement_after_case_label->kind == stmk_init)) == true))
                                           {
                                             first_statement_after_case_label = first_statement_after_case_label->next;
#if DEBUG_SWITCH
                                             printf ("+++++++++++++++ RE-initialized value: first_statement_after_case_label = %p \n",first_statement_after_case_label);
#endif
                                           }
                                      }

#if DEBUG_SWITCH
                                   printf ("+++++++++++++++ final value: first_statement_after_case_label = %p \n",first_statement_after_case_label);
#endif

                                   if (first_statement_after_case_label != NULL && next_statement == first_statement_after_case_label)
                                      {
                                     // This is a typical switch with no statements before the first case option statement.
#if DEBUG_SWITCH
                                        printf ("+++++++++++++++ This is a typical switch with no statements before the first case option statement. first_statement_after_case_label = %p \n",first_statement_after_case_label);
#endif
                                      }
                                     else
                                      {
                                        if (first_statement_after_case_label != NULL && next_statement != first_statement_after_case_label)
                                           {
#if 1
   #ifdef DEBUG_CONVERSION
                                             printf ("++++++++++++++ Supporting handling of statements before the first case statement in switch! \n");
   #endif 
   #if 0
                                             printf ("Detected statements before the first case option statement! \n");
   #endif
                                          // We want the last statement appearing before edg_case_stmt->stmt.
                                          // a_statement_ptr last_statement_before_first_case_label = find_statement_before_target(next_statement,first_statement_after_case_label);
                                             a_statement_ptr last_statement_before_first_case_label = find_statement_before_target(next_statement,edg_case_stmt->stmt);
   #if DEBUG_SWITCH
                                          // DEBUGGING: Output more data.
                                             printf ("next_statement = %p = %s \n",next_statement,summarize_statement_in_string(next_statement));
                                             printf ("last_statement_before_first_case_label = %p = %s \n",last_statement_before_first_case_label,summarize_statement_in_string(last_statement_before_first_case_label));

                                          // Debugging info.
                                             a_statement_ptr temp_statement = next_statement;
                                          // while ((temp_statement->kind != stmk_switch_case) && temp_statement != first_statement_after_case_label)
                                          // while ((temp_statement->kind != stmk_switch_case) && temp_statement != last_case_body_statement)
                                          // while (temp_statement != last_statement_before_first_case_label)
                                          // while (temp_statement != edg_case_stmt->stmt)

                                             printf ("In parse_statement(): case stmk_switch: Detected statements before the first case option statement: \n");
                                             while (temp_statement != last_statement_before_first_case_label->next)
                                                {
                                                  printf ("   --- Detected statements before the first case option statement: temp_statement = %p = %s \n",
                                                       temp_statement,summarize_statement_in_string(temp_statement));

                                                  temp_statement = temp_statement->next;
                                                }
   #endif
                                        if (sse->entity.kind == iek_statement && ((a_statement_ptr)sse->entity.ptr)->kind == stmk_decl)
                                           {
                                          // If this is a stmk_decl then it may contain typed declared as part of this switch sstatement.
   #if DEBUG_SWITCH
                                             printf ("Current entry is a stmk_decl (skipping it for now, but later we want to call parse_statement() at this point) \n");
   #endif
                                             a_statement_ptr statement = (a_statement_ptr)sse->entity.ptr;
   #if DEBUG_SWITCH
                                             printf ("BEFORE WHILE LOOP: statement = %p last_statement_before_first_case_label = %p \n",statement,last_statement_before_first_case_label);
   #endif
                                          // if (statement == last_statement_before_first_case_label)
                                          // while (statement != last_statement_before_first_case_label && statement->kind == stmk_decl)
                                             while (statement->kind == stmk_decl)
                                                {
   #if DEBUG_SWITCH
                                                  printf ("Increment the sse past the stmk_decl to get to the declarations that are next...\n");
                                                  printf ("In parse_statement(): statements before first case in switch: entry = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
   #endif
                                                  ++sse;
   #if 0
                                               // If the last statment is the first statement (and it is a stmk_decl) then we have to look at the list of entities in the stmk_decl).
                                                  an_il_entity_list_entry_ptr entities = statement->variant.decl.entities;
      #error "DEAD CODE!"
                                                  while (entities != NULL)
                                                     {
      #if 0
                                                       printf ("In parse_statement(): statements before first case in switch: entry = %p kind = %s \n",entities->entity.ptr,summarize_entry_kind_in_string_kind(entities->entity).c_str());
      #endif
                                                       if (entities->entity.kind == iek_statement)
                                                          {
                                                            last_statement_before_first_case_label = (a_statement_ptr)entities->entity.ptr;
                                                          }
      #error "DEAD CODE!"
                                                         else
                                                          {
                                                            if (entities->entity.kind == iek_variable)
                                                               {
      #if 0
                                                                 printf ("Not an edg a_statement_ptr, but handling the restricted case of an iek_variable \n");
      #endif
                                                              // last_statement_before_first_case_label = (a_statement_ptr)entities->entity.ptr;
                                                                 SgVariableDeclaration* variable = parse_variable_declaration(sse);
                                                                 appendStatement(variable,body_of_switch);
                                                               }
                                                              else
      #error "DEAD CODE!"
                                                               {
                                                                 printf ("The last_statement_before_first_case_label must be a statement and this entity is a %s \n",summarize_entry_kind_in_string_kind(entities->entity).c_str());

                                                                 printf ("Exiting until we can take care of this case... \n");
                                                                 ROSE_ASSERT(false);
                                                               }
                                                          }

      #error "DEAD CODE!"
                                                       entities = entities->next;
                                                     }
   #else
                                               // We are only handling variables in the entries of a switch before the first case option statement.
                                                  if (sse->entity.kind == iek_variable)
                                                     {
      #if DEBUG_SWITCH
                                                       printf ("Not an edg a_statement_ptr, but handling the restricted case of an iek_variable \n");
      #endif
      #if 0
                                                    // last_statement_before_first_case_label = (a_statement_ptr)entities->entity.ptr;
                                                       SgVariableDeclaration* variable = parse_variable_declaration(sse);
                                                       appendStatement(variable,body_of_switch);
      #else
                                                       while (sse->entity.kind == iek_variable)
                                                          {
         #if DEBUG_SWITCH
                                                            printf ("Repeat intil we have not more in a sequences of iek_variable entries \n");
         #endif
         #if DEBUG_SWITCH
                                                            printf ("Calling parse_variable_declaration(sse) as part of handling declarations before switch cases \n");
         #endif
                                                         // We need to make sure that any collected statements are set into the switchSupportMap (so call parse_statement() instead of parse_variable_declaration()).
                                                         // SgVariableDeclaration* variable = parse_variable_declaration(sse);
                                                            vector<SgStatement*> variable_declarations = parse_statement(sse,NULL);
                                                            ROSE_ASSERT(variable_declarations.size() == 1);
                                                            SgStatement* statement = *(variable_declarations.begin());
                                                            ROSE_ASSERT(statement != NULL);
                                                            SgVariableDeclaration* variable = isSgVariableDeclaration(statement);
                                                            ROSE_ASSERT(variable != NULL);
         #if DEBUG_SWITCH
                                                            printf ("At base of inner loop over contigous sequence of iek_variable entries: next entity is a %s \n",summarize_entry_kind_in_string_kind(sse->entity).c_str());
         #endif
                                                            appendStatement(variable,body_of_switch);
                                                          }
      #endif
                                                     }
                                                    else
                                                     {
                                                       printf ("The last_statement_before_first_case_label must be a statement and this entity is a %s \n",summarize_entry_kind_in_string_kind(sse->entity).c_str());

                                                       printf ("Exiting until we can take care of this case... \n");
                                                       ROSE_ASSERT(false);
                                                     }
   #endif
                                               // This might not be a statement...
                                                  statement = (a_statement_ptr)sse->entity.ptr;
   #if DEBUG_SWITCH
                                                  printf ("statement = %p = %s \n",statement,summarize_statement_in_string(statement));
                                                  printf ("last_statement_before_first_case_label = %p = %s \n",last_statement_before_first_case_label,summarize_statement_in_string(last_statement_before_first_case_label));
   #endif
                                                }
   #if DEBUG_SWITCH
                                             printf ("last_statement_before_first_case_label = %p = %s \n",last_statement_before_first_case_label,summarize_statement_in_string(last_statement_before_first_case_label));
   #endif
                                          // last_statement_before_first_case_label = statement->variant.decl->entities;
                                          // Now we need to file the last entity.
                                             
                                           // vector<SgStatement*> stmts = parse_statement(sse);
                                           // ++sse;
                                           }
                                          else
                                           {
   #if DEBUG_SWITCH
                                             printf ("++++++++++ Non-declaration statement found before the first switch case (e.g. while statement) \n");
   #endif
                                          // DQ (2/6/2013): This is a problem since it picks up statements that are after the first case statement and thus is causing the AST to be generated wrong.
                                             if (sse->entity.kind == iek_statement && ((a_statement_ptr)sse->entity.ptr)->kind != stmk_decl)
                                                {
                                               // This is some other kind of statement: not a declaration.
   #if DEBUG_SWITCH
                                                  printf ("This is some other kind of statement: not a declaration: entry = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
   #endif
                                               // DQ (2/2/2013): Unifing the support with parse_statement_list_without_terminator() so that we can support Duff's Devices.
                                               // vector<SgStatement*> stmts = parse_statement_list_in_switch_body(sse,last_statement_before_first_case_label);
                                               // vector<SgStatement*> stmts = parse_statement_list_in_switch_body(sse,switch_body_statement,last_statement_before_first_case_label);
                                               // vector<SgStatement*> stmts = parse_statement_list_without_terminator(sse,switch_body_statement,last_statement_before_first_case_label);
                                                  vector<SgStatement*> stmts = parse_statement_list_without_terminator(sse,NULL,NULL,last_statement_before_first_case_label);

                                                  appendStatementList(stmts,body_of_switch);
#if 0
                                                  printf ("Exiting as a test! \n");
                                                  ROSE_ASSERT(false);
#endif
                                                }
                                           }
#else
   #ifdef DEBUG_CONVERSION
                                             printf ("WARNING: suppressing handling of statements before the first case statement in switch! \n");
   #endif
#endif

#if 0
                                             printf ("Exiting after collecting statements before case option statement. \n");
                                             ROSE_ASSERT(false);
#endif
                                           }
                                      }
                                 }

                           // If there are any SgStatement IR nodes before the first case/or default statement, then we need to find 
                           // them here (and add them before the case/default statement).
#if DEBUG_SWITCH
                              printf ("@@@@@@@@@@ switch_body_statement = %p = %s \n",switch_body_statement,summarize_statement_in_string(switch_body_statement));
#endif
#if 0
                           // a_statement_ptr p = (a_statement_ptr)(sse->entity.ptr);
                           // while ((a_statement_ptr)sse->entity.ptr != edg_case_stmt->stmt)
                              while (sse->entity.kind == iek_statement && ((a_statement_ptr)sse->entity.ptr)->kind == stmk_block)
                                 {
#error "DEAD CODE!"
                                   printf ("Increment sse to the start of the case option statement sse->entity.ptr = %p \n",sse->entity.ptr);
                                   ++sse;
                                 }
#endif
                           // DQ (1/17/2014): Where possilbe compute the block so it can be passed to the parse_statement_list_without_terminator() function.
                              a_statement_ptr block_statement = NULL;
                              if (sse->entity.kind == iek_statement)
                                 {
                                   a_statement_ptr statement = (a_statement_ptr)sse->entity.ptr;
                                   if (statement->kind == stmk_block)
                                      {
                                        block_statement = statement;
                                        a_block_ptr block = (a_block_ptr)(p->variant.block.extra_info);
                                        ROSE_ASSERT(block != NULL);
#if 0
                                        printf ("In parse_statement(): case stmk_switch: Calling summarize_block() block = %p \n",block);
                                        summarize_block(block);
#endif
                                      }
                                 }

                              int caseStatementCounter = 0;
                              while (edg_case_stmt != NULL)
                                 {
#if DEBUG_SWITCH
                                   printf ("========== In parse_statement(): case stmk_switch: TOP of loop over cases: calling summarize_switch_case(case_stmt = %p) \n",edg_case_stmt);
                                   summarize_switch_case(edg_case_stmt);
#endif
                                   SgStatement* rose_case_stmt = NULL;

                                   bool isDefaultCase = (switch_extra_info->default_case != NULL && edg_case_stmt == switch_extra_info->default_case);
#if DEBUG_SWITCH
                                   printf ("In parse_statement(): case stmk_switch: cases: caseStatementCounter = %d isDefaultCase = %s \n",caseStatementCounter,isDefaultCase ? "true" : "false");
#endif
                                // **************************************************************
                                // Process the statements in the case (case or default statement)
                                // **************************************************************

                                // This is the start of the range of statements in this case.
                                   a_statement_ptr case_statement = edg_case_stmt->stmt;
                                   ROSE_ASSERT(case_statement != NULL);
                                   a_statement_ptr next_case_statement = (edg_case_stmt->next != NULL) ? edg_case_stmt->next->stmt : NULL;
#if DEBUG_SWITCH
                                   printf ("In parse_statement(): case stmk_switch: cases: case_statement = %p next_case_statement = %p \n",case_statement,next_case_statement);
#endif
                                // Since this points at the stmk_switch_case statement by definition, we want the 
                                // next statement (but only if the next statement is NOT another stmk_switch_case).
                                // a_statement_ptr case_body_statement = (case_statement != NULL) ? case_statement->next : NULL;
                                   a_statement_ptr case_body_statement = (case_statement != NULL && case_statement->next != NULL && case_statement->next->kind != stmk_switch_case) ? case_statement->next : NULL;
#if DEBUG_SWITCH
                                   printf ("In parse_statement(): case stmk_switch: cases: case_body_statement = %p next_case_statement = %p (not inclusive) \n",case_body_statement,next_case_statement);
#endif
                                   a_statement_ptr last_case_body_statement = NULL;
                                   if (case_body_statement != NULL)
                                      {
                                     // To correct for test2012_125.c where a lable is associated with the last statment in the 
                                     // case, we have to addess this in the find_statement_before_target() function.
                                        last_case_body_statement = find_statement_before_target(case_body_statement, next_case_statement);
                                      }
#if DEBUG_SWITCH
                                   printf ("In parse_statement(): case stmk_switch: cases: case_body_statement = %p last_case_body_statement = %p (inclusive) \n",case_body_statement,last_case_body_statement);
#endif
                                   if (last_case_body_statement != NULL)
                                      {
                                     // This can happen if there are switch statements nested in other switch statements and compiler generated stmk_label uses as separators.
                                        if (next_case_statement != last_case_body_statement->next)
                                           {
#ifdef DEBUG_CONVERSION
                                             printf ("WARNING: next_case_statement != last_case_body_statement->next \n");
#endif
                                           }
                                     // ROSE_ASSERT(next_case_statement == last_case_body_statement->next);
                                      }
#if DEBUG_SWITCH
                                   printf ("In parse_statement(): case stmk_switch: cases: case_body_statement calling summarize_statement(case_body_statement = %p) \n",case_body_statement);
                                   if (case_body_statement != NULL)
                                      {
                                        summarize_statement(case_body_statement);
                                      }
                                   printf ("In parse_statement(): case stmk_switch: cases: last_case_body_statement calling summarize_statement(last_case_body_statement = %p) \n",last_case_body_statement);
                                   if (last_case_body_statement != NULL)
                                      {
                                        summarize_statement(last_case_body_statement);
                                      }
#endif
#if DEBUG_SWITCH
                                   printf ("In parse_statement(): case stmk_switch: cases: next_case_statement calling summarize_statement(next_case_body_statement = %p) \n",next_case_statement);
                                   if (next_case_statement != NULL)
                                      {
                                        summarize_statement(next_case_statement);
                                      }
#endif
#if DEBUG_SWITCH
                                   printf ("********************************************************************************* \n");
                                   printf ("TOP: Collecting statements in case/default option statement caseStatementCounter = %3d \n",caseStatementCounter);
                                   printf ("********************************************************************************* \n");
                                   printf ("In parse_statement(): case stmk_switch: case %d: START: entry = %p kind = %s \n",caseStatementCounter,sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif

                                // DQ (10/28/2012): Assert this...
                                   if ((an_il_entry_kind)sse->entity.kind != iek_statement)
                                      {
#ifdef DEBUG_CONVERSION
                                        printf ("WARNING: In parse_statement(): case stmk_switch: case %d: sse->entity.kind != iek_statement: entry = %p kind = %s \n",
                                             caseStatementCounter,sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
                                      }
                                // ROSE_ASSERT((an_il_entry_kind)sse->entity.kind == iek_statement);
                                // if (case_body_statement != (a_statement_ptr)sse->entity.ptr)
                                   if ( ((an_il_entry_kind)sse->entity.kind == iek_statement) && (case_body_statement != (a_statement_ptr)sse->entity.ptr) )
                                      {
#ifdef DEBUG_CONVERSION
                                        printf ("WARNING: In parse_statement(): case stmk_switch: case %d: case_body_statement != (a_statement_ptr)sse->entity.ptr: case_body_statement = %p sse->entity.ptr = %p \n",
                                             caseStatementCounter,case_body_statement,sse->entity.ptr);
#endif
                                      }
                                // ROSE_ASSERT(case_body_statement == (a_statement_ptr)sse->entity.ptr);

                                // We need to iterate over the statement from case_body_statement, upto, but not including, next_case_body_statement.
                                // vector<SgStatement*> stmts = parse_statement_list_in_switch_body(sse,next_case_body_statement,switch_body_statement);
                                // vector<SgStatement*> stmts = parse_statement_list_in_switch_body(sse,next_case_body_statement);
                                // vector<SgStatement*> stmts = parse_statement_list_in_switch_body(sse,last_case_body_statement);
                                   vector<SgStatement*> stmts;
                                // if (case_body_statement != last_case_body_statement)
                                   if (last_case_body_statement != NULL)
                                      {
#if DEBUG_SWITCH
                                        printf ("^^^^^^^^^^ In parse_statement(): case stmk_switch: last_case_body_statement != NULL: calling parse_statement_list_in_switch_body \n");
#endif

                                     // DQ (2/9/2013): If the current value of sse is a reference to a stmk_block then last_case_body_statement can be 
                                     // pointing to the block_stmt, and the parse_statement_list_without_terminator() function will not collect the 
                                     // block statement (but only for C, it works fine for C++).  See test2013_59.C for the C++ example and test2012_84.c 
                                     // for the C language example.
#if DEBUG_SWITCH
                                        printf ("In parse_statement(): case stmk_switch: case %d: START: entry = %p kind = %s \n",caseStatementCounter,sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
                                        printf ("In parse_statement(): case stmk_switch: block_statement = %p \n",block_statement);
#endif
#if 0
                                        printf ("Exiting as a test! \n");
                                        ROSE_ASSERT(false);
#endif
                                     // DQ (1/17/2014): Use the block_statement in the paremter list to the parse_statement_list_without_terminator() when it is non-NULL.
                                     // DQ (2/2/2013): Unifing the support with parse_statement_list_without_terminator() so that we can support Duff's Devices.
                                     // stmts = parse_statement_list_in_switch_body(sse,last_case_body_statement);
                                     // stmts = parse_statement_list_in_switch_body(sse,switch_body_statement,last_case_body_statement);
                                     // stmts = parse_statement_list_without_terminator(sse,switch_body_statement,last_case_body_statement);
                                     // stmts = parse_statement_list_without_terminator(sse,NULL,NULL,last_case_body_statement);
                                        stmts = parse_statement_list_without_terminator(sse,block_statement,NULL,last_case_body_statement);
#if 0
                                        printf ("Exiting as a test! \n");
                                        ROSE_ASSERT(false);
#endif
                                      }
                                     else
                                      {
#if DEBUG_SWITCH
                                        printf ("^^^^^^^^^^ Note: last_case_body_statement == NULL: no statements to extract from this case option statement \n");
#endif
                                      }

                                // Generate the body for the switch statement.
                                   SgStatement* case_body_with_statements = build_basic_block_if_multiple_statements(stmts);
                                   ROSE_ASSERT(case_body_with_statements != NULL);
#if DEBUG_SWITCH
                                   printf ("************************************************************************************ \n");
                                   printf ("Collected %" PRIuPTR " statements in case/default option statement caseStatementCounter = %3d \n",stmts.size(),caseStatementCounter);
                                   printf ("   --- case_body_with_statements = %p = %s \n",case_body_with_statements,case_body_with_statements->class_name().c_str());
                                   printf ("************************************************************************************ \n");

                                   printf ("In parse_statement(): case stmk_switch: case %d: END: entry = %p kind = %s \n",caseStatementCounter,sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
                                   if (stmts.empty() == true)
                                      {
#if DEBUG_SWITCH
                                        printf ("The case option label must be associated with a previously parsed statement \n");
#endif
#if DEBUG_SWITCH
                                        printf ("In parse_statement(): case stmk_switch: cases: case_body_statement = %p last_case_body_statement = %p (inclusive) \n",case_body_statement,last_case_body_statement);
#endif
#if 0
                                        printf ("Exiting as a test! \n");
                                        ROSE_ASSERT(false);
#endif
                                      }

                                // *********************************************************
                                // Process the end of the case (case or default statement)
                                // *********************************************************

                                   if (isDefaultCase == false)
                                      {
#if DEBUG_SWITCH
                                        printf ("In parse_statement(): case stmk_switch: cases: Found a NON-DEFAULT case! edg_case_stmt = %p \n",edg_case_stmt);
#endif
                                     // The integer value associated with this case.  (Or the value of the start of the range if this is a
                                     // GNU case range.)  NULL for a "default:" case.
                                        SgExpression* case_constant_expr = convert_constant(edg_case_stmt->case_value);
                                        ROSE_ASSERT(case_constant_expr != NULL);

                                        rose_case_stmt = buildCaseOptionStmt_nfi(case_constant_expr,case_body_with_statements);

                                     // DQ (12/26/2014): Assertion added.
                                        ROSE_ASSERT(rose_case_stmt != NULL);

                                        ROSE_ASSERT(case_constant_expr->get_parent() == rose_case_stmt);
#if GNU_EXTENSIONS_ALLOWED
                                     // If this is a GNU case range, the value of the end of the range; otherwise, NULL.
                                        if (edg_case_stmt->range_end != NULL)
                                           {
#if 0
                                             printf ("GNU case range not yet supported for edg4x, but already supported within ROSE \n");
#endif

                                          // DQ (1/31/2014): Make this a SgCaseOptionStmt so that we can more easily add the GNU case range support.
                                             SgCaseOptionStmt* rose_case_option_stmt = isSgCaseOptionStmt(rose_case_stmt);
                                             ROSE_ASSERT(rose_case_option_stmt != NULL);

                                             SgExpression* case_range_end_constant_expr = convert_constant(edg_case_stmt->range_end);
                                             ROSE_ASSERT(case_range_end_constant_expr != NULL);

                                             rose_case_option_stmt->set_key_range_end(case_range_end_constant_expr);
                                             case_range_end_constant_expr->set_parent(rose_case_option_stmt);
#if 0
                                             printf ("Exiting at GNU case range not yet supported for edg4x \n");
                                             ROSE_ASSERT(false);
#endif
                                           }
#endif
#if 0
                                        printf ("Exiting at the base of the first non-default statement in a switch stmt \n");
                                        ROSE_ASSERT(false);
#endif
                                      }
                                     else
                                      {
#if DEBUG_SWITCH || 1
                                        printf ("In parse_statement(): case stmk_switch: cases: Found the DEFAULT case! edg_case_stmt = %p \n",edg_case_stmt);
#endif
                                        rose_case_stmt = buildDefaultOptionStmt_nfi(case_body_with_statements);

                                     // DQ (12/26/2014): Assertion added.
                                        ROSE_ASSERT(rose_case_stmt != NULL);

                                     // DQ (3/25/2021): Assertion added.
                                        ROSE_ASSERT(rose_case_stmt->get_parent() != NULL);
                                      }

                                   ROSE_ASSERT(rose_case_stmt != NULL);
                                   ROSE_ASSERT(case_body_with_statements->get_parent() == rose_case_stmt);

                                // DQ (12/26/2014): Set the source position.
                                   set_file_info_for_case_option_stmt(rose_case_stmt,edg_case_stmt);
#if DEBUG_SWITCH
                                   printf ("case_body_with_statements source position: \n");
                                   case_body_with_statements->get_startOfConstruct()->display("case_body_with_statements source position: start");
                                   case_body_with_statements->get_endOfConstruct()  ->display("case_body_with_statements source position: end");
#endif
                                // DQ (12/26/2014): Set the source position for the end of the case statement.
                                   ROSE_ASSERT(rose_case_stmt->get_endOfConstruct() != NULL);
                                   ROSE_ASSERT(case_body_with_statements->get_endOfConstruct() != NULL);
                                   if (case_body_with_statements->get_endOfConstruct()->isCompilerGenerated() == false)
                                      {
#if DEBUG_SWITCH
                                        printf ("   --- update the ending source position for the case body \n");
#endif
                                        *(rose_case_stmt->get_endOfConstruct()) = *(case_body_with_statements->get_endOfConstruct());
                                      }
                                     else
                                      {
#if DEBUG_SWITCH
                                        printf ("   --- case_body_with_statements->get_endOfConstruct()->isCompilerGenerated() == true \n");
#endif
#if 0
                                     // DQ (1/22/2015): COMMENTED OUT: Until we can get around the token mapping of the compiler generated SgBasicBlock that is the body of the SgCaseOptionStmt.

                                     // DQ (1/22/2015): Find and use the ending source position of the last statement in the compiler generated basic block.
                                     // SgStatementPtrList::reverse_iterator last = case_body_with_statements->get_statements().rbegin();
                                        SgBasicBlock* basicBlock = isSgBasicBlock(case_body_with_statements);
                                        ROSE_ASSERT(basicBlock != NULL);
                                        SgStatementPtrList::reverse_iterator last = basicBlock->get_statements().rbegin();

                                        if (last != basicBlock->get_statements().rend())
                                           {
                                              SgStatement* lastStatementOfCompilerGeneratedBlock = *last;
                                              ROSE_ASSERT(lastStatementOfCompilerGeneratedBlock != NULL);
#if DEBUG_SWITCH
                                              printf ("lastStatementOfCompilerGeneratedBlock = %p = %s \n",lastStatementOfCompilerGeneratedBlock,lastStatementOfCompilerGeneratedBlock->class_name().c_str());
#endif
                                             *(rose_case_stmt->get_endOfConstruct()) = *(lastStatementOfCompilerGeneratedBlock->get_endOfConstruct());

#if DEBUG_SWITCH
                                             printf ("case_body_with_statements source position (after reset for compiler-generated block): \n");
                                             case_body_with_statements->get_startOfConstruct()->display("case_body_with_statements source position: start");
                                             case_body_with_statements->get_endOfConstruct()  ->display("case_body_with_statements source position: end");
#endif
                                           }
                                          else
                                           {
                                             printf ("Error: compiler generated block for switch case option statements appears to be empty \n");
                                             ROSE_ASSERT(false);
                                           }
#endif
                                      }

                                // DQ (12/27/2014): Tracking down a zero value for the end of a case statement.
                                // ROSE_ASSERT(case_body_with_statements->get_endOfConstruct()->isCompilerGenerated() == false);
                                   ROSE_ASSERT(rose_case_stmt->get_endOfConstruct()->isCompilerGenerated() == false);

                                // DQ (2/8/2013): Fill-in the ROSE IR node pointer in the map of EDG statements to ROSE statements.
                                // a_statement_ptr edg_case_stmt_associated_statement = (a_statement_ptr)edg_case_stmt;
                                // a_statement_ptr edg_case_stmt_associated_statement = edg_case_stmt->stmt;
                                // a_statement_ptr edg_case_stmt_associated_statement = case_body_statement;
                                // a_source_sequence_entry_ptr edg_case_stmt_associated_entry = case_body_statement->source_sequence_entry;
                                   a_source_sequence_entry_ptr edg_case_stmt_associated_entry = (case_body_statement != NULL) ? case_body_statement->source_sequence_entry : NULL;
#if DEBUG_SWITCH
                                   printf ("In parse_statement(): case stmk_switch: cases: Computed edg_case_stmt_associated_entry = %p kind = %s \n",
                                        edg_case_stmt_associated_entry,(edg_case_stmt_associated_entry != NULL) ? summarize_entry_kind_in_string_kind(edg_case_stmt_associated_entry->entity).c_str() : "NULL");
#endif
                                // if (edg_case_stmt_associated_entry != NULL && edg_case_stmt_associated_entry->entity.kind == stmk_decl)
                                   if ((edg_case_stmt_associated_entry != NULL) && (edg_case_stmt_associated_entry->entity.kind == iek_statement) && (((a_statement_ptr)(edg_case_stmt_associated_entry->entity.ptr))->kind == stmk_decl))
                                      {
                                     // printf ("In parse_statement(): case stmk_switch: cases: Calling summarize_statement(edg_case_stmt_associated_statement); \n");
                                     // summarize_statement(edg_case_stmt_associated_statement);

                                     // printf ("     case stmk_decl: edg_case_stmt_associated_statement: source_sequence_entry = %p \n",edg_case_stmt_associated_statement->source_sequence_entry);
                                     // a_source_sequence_entry_ptr listElement = edg_case_stmt_associated_statement->source_sequence_entry;
#if DEBUG_SWITCH
                                        printf ("     case stmk_decl: edg_case_stmt_associated_entry = %p \n",edg_case_stmt_associated_entry);
#endif
                                        a_source_sequence_entry_ptr listElement = edg_case_stmt_associated_entry;

                                        bool done = false;
                                        while (listElement != NULL && done == false)
                                           {
#if DEBUG_SWITCH
                                             printf ("          listElement->entity = %p (kind = %s) \n",listElement,summarize_entry_kind_in_string_kind(listElement->entity).c_str());
#endif
                                          // if (edg_case_stmt_associated_entry != (a_statement_ptr)listElement->entity.ptr)
                                             if (edg_case_stmt_associated_entry != listElement)
                                                {
                                               // DQ (2/10/2013): We don't want to use the source_sequence_entry represented by a iek_src_seq_sublist (see test2013_61.C).
                                               // edg_case_stmt_associated_entry = (a_statement_ptr)listElement->entity.ptr;
                                               // edg_case_stmt_associated_entry = listElement;
                                                  ROSE_ASSERT(edg_case_stmt_associated_entry != NULL);
                                               // if (edg_case_stmt_associated_entry != NULL && edg_case_stmt_associated_entry->entity.kind != iek_src_seq_sublist)
                                               // if (edg_case_stmt_associated_entry->entity.kind != iek_src_seq_sublist)
                                                  if (listElement->entity.kind != iek_src_seq_sublist)
                                                     {
                                                       edg_case_stmt_associated_entry = listElement;
                                                       done = true;
                                                     }
                                                    else
                                                     {
                                                       edg_case_stmt_associated_entry = ((a_src_seq_sublist_ptr)(listElement->entity.ptr))->source_sequence_list;
                                                       done = true;

                                                    // DQ (4/23/2017): Converted to used message streams (appears only for case of where Microsoft extensions are used).
                                                       mprintf ("Found a edg_case_stmt_associated_entry->entity.kind == iek_src_seq_sublist edg_case_stmt_associated_entry = %p \n",edg_case_stmt_associated_entry);
                                                    // ROSE_ASSERT(false);
                                                     }
#if DEBUG_SWITCH
                                                   printf ("In parse_statement(): case stmk_switch: cases: resetting the edg_case_stmt_associated_entry = %p \n",edg_case_stmt_associated_entry);
#endif
                                                }
                                               else
                                                {
#if DEBUG_SWITCH
                                                  printf ("Found edg_case_stmt_associated_entry != listElement (common in first iteration) \n");
#endif
                                                }

                                             listElement = listElement->next;
                                           }

                                     // ROSE_ASSERT(edg_case_stmt_associated_statement != NULL);
                                        ROSE_ASSERT(edg_case_stmt_associated_entry != NULL);

                                     // If this is not a statement (i.e. a variable declaration) then this makes no sense.
                                     // printf ("In parse_statement(): case stmk_switch: cases: Calling summarize_statement(edg_case_stmt_associated_statement); \n");
                                     // summarize_statement(edg_case_stmt_associated_statement);
#if DEBUG_SWITCH
                                     // Output the map to support debugging.
                                        map<a_source_sequence_entry_ptr,SgStatement*>::iterator i = switchSupportMap->begin();
                                        int counter = 0;
                                        while(i != switchSupportMap->end())
                                           {
                                             printf ("switchSupportMap[%d] first = %p second = %p = %s \n",counter,i->first,i->second,(i->second != NULL) ? i->second->class_name().c_str() : "NULL");

                                             counter++;
                                             i++;
                                           }
#endif
#if 0
                                        printf ("Need to handle the case where edg_case_stmt_associated_entry is a stmk_decl edg_case_stmt_associated_entry = %p \n",edg_case_stmt_associated_entry);
                                        ROSE_ASSERT(false);
#endif
                                      }
                                     else
                                      {
#if DEBUG_SWITCH
                                        printf ("In parse_statement(): case stmk_switch: cases: edg_case_stmt_associated_entry = %p is not a stmk_decl \n",edg_case_stmt_associated_entry);
#endif
                                      }

                                // ROSE_ASSERT(edg_case_stmt_associated_statement != NULL);
                                // ROSE_ASSERT(case_body_statement != NULL);
                                // if (case_body_statement != NULL)
                                   if (edg_case_stmt_associated_entry != NULL)
                                      {
#if DEBUG_SWITCH
                                     // printf ("In parse_statement(): case stmk_switch: switchSupportMap->size() = %" PRIuPTR " edg_case_stmt_associated_statement = %p \n",switchSupportMap->size(),edg_case_stmt_associated_statement);
                                     // printf ("In parse_statement(): case stmk_switch: switchSupportMap->size() = %" PRIuPTR " case_body_statement = %p \n",switchSupportMap->size(),case_body_statement);
                                        printf ("In parse_statement(): case stmk_switch: switchSupportMap->size() = %" PRIuPTR " edg_case_stmt_associated_entry = %p \n",switchSupportMap->size(),edg_case_stmt_associated_entry);
#endif
                                     // map<a_statement_ptr,SgStatement*>::iterator mapEntry = switchSupportMap->find(edg_case_stmt_associated_statement);
                                     // map<a_statement_ptr,SgStatement*>::iterator mapEntry = switchSupportMap->find(case_body_statement);
                                        map<a_source_sequence_entry_ptr,SgStatement*>::iterator mapEntry = switchSupportMap->find(edg_case_stmt_associated_entry);

                                     // ROSE_ASSERT(mapEntry != switchSupportMap->end());
                                        if (mapEntry != switchSupportMap->end())
                                           {
                                          // ROSE_ASSERT(mapEntry->second != NULL);
                                             if (mapEntry->second == NULL)
                                                {
                                               // I think this is the case of a declaration at the top of a switch statement.
                                               // printf ("Error: I think this is the case of a declaration at the top of a switch statement: case_body_statement = %p kind = %s \n",case_body_statement,summarize_statement_in_string(case_body_statement));
                                                  printf ("Error: I think this is the case of a declaration at the top of a switch statement: edg_case_stmt_associated_entry = %p kind = %s \n",
                                                          edg_case_stmt_associated_entry,summarize_entry_kind_in_string_kind(edg_case_stmt_associated_entry->entity).c_str());

                                                  int counter = 0;
                                                  map<a_source_sequence_entry_ptr,SgStatement*>::iterator mapIterator = switchSupportMap->begin();
                                                  while (mapIterator != switchSupportMap->end())
                                                     {
                                                       printf ("   --- switchSupportMap[%d] contains: first = %p second %p = %s \n",counter,mapIterator->first,mapIterator->second,(mapIterator->second != NULL) ? mapIterator->second->class_name().c_str() : "NULL");
                                                       counter++;
                                                       mapIterator++;
                                                     }

                                                  printf ("Exiting after detecing an error! \n");
                                                  ROSE_ASSERT(false);
                                                }
                                          // mapEntry->second = rose_case_stmt;
#if DEBUG_SWITCH
                                             printf ("Found associated edg statement mapEntry->first = %p and set map to ROSE case statement mapEntry->second = %p = %s \n",mapEntry->first,mapEntry->second,mapEntry->second->class_name().c_str());
#endif
                                           }
                                          else
                                           {
                                          // DQ (4/23/2017): Converted to used message streams (appears only for case of where Microsoft extensions are used).
                                          // I think that any switch with nested blocks of statements (not only the Duff's Device examples) can take this branch.
                                             mprintf ("ERROR: edg_case_stmt_associated_entry = %p = %s not found in switchSupportMap: generated statements are: \n",
                                                  edg_case_stmt_associated_entry,summarize_entry_kind_in_string_kind(edg_case_stmt_associated_entry->entity).c_str());

                                             int counter = 0;
                                             map<a_source_sequence_entry_ptr,SgStatement*>::iterator mapIterator = switchSupportMap->begin();
                                             while (mapIterator != switchSupportMap->end())
                                                {
                                               // DQ (4/23/2017): Converted to used message streams (appears only for case of where Microsoft extensions are used).
                                                  mprintf ("   --- switchSupportMap[%d] contains: first = %p second %p = %s \n",counter,mapIterator->first,
                                                       mapIterator->second,(mapIterator->second != NULL) ? mapIterator->second->class_name().c_str() : "NULL");
                                                  counter++;
                                                  mapIterator++;
                                                }
#if 1
                                          // DQ (4/23/2017): Converted to used message streams (appears only for case of where Microsoft extensions are used).
                                             mprintf ("ERROR: In parse_statement(): case stmk_switch: cases: statement associated with switch case (%p) not found in switchSupportMap \n",edg_case_stmt_associated_entry);
                                          // ROSE_ASSERT(false);
#endif
                                           }
                                      }
                                     else
                                      {
                                     // DQ (2/8/2013): This is the case of an empty case statement (fall through to new case).
#if DEBUG_SWITCH
                                        printf ("In parse_statement(): case stmk_switch: cases: case_body_statement == NULL (OK to have an empty case statement) \n");
#endif
                                      }
#if 0
                                   printf ("Appending rose_case_stmt = %p = %s to switch body = %p = %s \n",rose_case_stmt,rose_case_stmt->class_name().c_str(),body_of_switch,body_of_switch->class_name().c_str());
#endif
                                // Put case or default statement into the body of the switch statement.
                                   body_of_switch->append_statement(rose_case_stmt);
#if 0
                                   printf ("In parse_statement(): case stmk_switch: switch statement body source position: caseStatementCounter = %d \n",caseStatementCounter);
                                   body_of_switch->get_startOfConstruct()->display("body_of_switch source position: start");
                                   body_of_switch->get_endOfConstruct()  ->display("body_of_switch source position: end");
#endif
#if 0
                                // DQ (12/28/2014): Commented out: This is now set using better infor from EDG.

                                // DQ (12/26/2014): Set the source position for the switch statement body statement.
                                   if (caseStatementCounter == 0)
                                      {
                                     // DQ (12/26/2014): Set the source position for the start of the switch body statement.
                                        ROSE_ASSERT(body_of_switch->get_startOfConstruct() != NULL);
                                        ROSE_ASSERT(rose_case_stmt->get_startOfConstruct() != NULL);
                                        *(body_of_switch->get_startOfConstruct()) = *(rose_case_stmt->get_startOfConstruct());
                                      }

                                // DQ (12/26/2014): Set the source position for the end of the switch body statement.
                                   ROSE_ASSERT(body_of_switch->get_endOfConstruct() != NULL);
                                   ROSE_ASSERT(rose_case_stmt->get_endOfConstruct() != NULL);
                                // *(body_of_switch->get_endOfConstruct()) = *(rose_case_stmt->get_endOfConstruct());
#endif
#if DEBUG_SWITCH
                                   printf ("In parse_statement(): case stmk_switch: switch statement body source position: (after reset): caseStatementCounter = %d \n",caseStatementCounter);
                                   body_of_switch->get_startOfConstruct()->display("body_of_switch source position: (after reset): start");
                                   body_of_switch->get_endOfConstruct()  ->display("body_of_switch source position: (not reset): end");
#endif
                                // Iterate to the next case in the source sequence order.
                                   edg_case_stmt = edg_case_stmt->next;
                                   caseStatementCounter++;
                                 }
#if 0
                              if (advancedOverStmtBlock == true)
                                 {
                                // DQ (10/25/2012):This needs to match when we advanced over the block, so that we advance over the associated end of block ().
#if DEBUG_SWITCH
                                   printf ("We need to match when we advanced over the block at the top of the switch statement handling. \n");
#endif
                                // DQ (2/2/2013): This fixes a problem in the unification of the parse_statement_list_without_terminator() function.
                                // ++sse;
                                 }
#endif
                           // End of case where there exist cases in the switch.
                            }
#if DEBUG_SWITCH
                         printf ("In parse_statement(): case stmk_switch: calling parse_statement(sse) switch_body_statement = %p \n",switch_body_statement);
#endif

#if 0
                      // SgStatement* body = build_basic_block_if_multiple_statements(parse_statement(sse));
                         vector<SgStatement*> stmts = parse_statement(sse);

#error "DEAD CODE!"

                         printf ("In parse_statement(): case stmk_switch: stmts for body stmts.size() = %" PRIuPTR " \n",stmts.size());

                      // Generate the body for the switch statement.
                         SgStatement* body = build_basic_block_if_multiple_statements(stmts);

#error "DEAD CODE!"

                      // SgSwitchStatement* sw = buildSwitchStatement_nfi(key, body);
                      // This is a SageInterface function: SageInterface::initializeSwitchStatement().
                         initializeSwitchStatement(sw, key, body);
#endif

#if DEBUG_POP_SCOPE
                         printf ("In parse_statement(): case stmk_switch: (pop SgBasicBlock switch body) calling popScopeStack() \n");
#endif
                      // Now pop the SgBasicBlock (switch body) off of the scoep stack.
                         popScopeStack();
#if DEBUG_POP_SCOPE
                         printf ("In parse_statement(): case stmk_switch: (pop SgSwitchStatement) calling popScopeStack() \n");
#endif
                      // Now pop the scope so that the normal mechanism can handle the scope stack.
                         popScopeStack();

                      // DQ (10/30/2012): Added test for parent to be non-NULL (required to be set early so that any nested types can have mangled names computed).
                      // DQ (7/9/2012): Added test for parent to be NULL.
                      // ROSE_ASSERT(sw->get_parent() == NULL);
                         ROSE_ASSERT(sw->get_parent() != NULL);

                         ROSE_ASSERT(sw->hasExplicitScope() == false);

                      // DQ (7/9/2012): We can set the parent directly to the scope for the case of a switch statement (now set uniformally below).
                      // sw->set_parent(topScopeStack());

                         set_file_info_for_stmt(sw, p);

                         result.push_back(sw);
#if DEBUG_SWITCH
                         printf ("LEAVING SWITCH \n");
                         printf ("In parse_statement(): case stmk_switch: entry = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif

                         if (sse->entity.kind == iek_src_seq_end_of_construct)
                            {
                           // DQ (2/3/2013): If this is the end of the block then since we processed the block directly in this 
                           // switch case increment past the end of the block so that if we are already in a function collecting 
                           // statements from a block it will not misinterpret the end of the switch body (block) with the end 
                           // of the block is it going to terminate at.
#if DEBUG_SWITCH
                              printf ("switch_body_statement = %p \n",switch_body_statement);
#endif
                              a_src_seq_end_of_construct_ptr src_seq_end_of_construct = (a_src_seq_end_of_construct_ptr)(sse->entity.ptr);
#if DEBUG_SWITCH
                              printf ("This sse->entity.kind == iek_src_seq_end_of_construct is associated with %p = %s \n",src_seq_end_of_construct->entity.ptr,summarize_entry_kind_in_string_kind(src_seq_end_of_construct->entity).c_str());
#endif
                           // We only want to test if these are matching valid pointers.
                              if (switch_body_statement != NULL && switch_body_statement == (a_statement_ptr)(src_seq_end_of_construct->entity.ptr))
                                 {
#if DEBUG_SWITCH
                                   printf ("Need to clean up after the switch block = %p \n",switch_body_statement);
#endif
                                // Increment past the iek_src_seq_end_of_construct at the end of the switch_body_statement (block).
                                   ++sse;
                                 }
#if 0
                              printf ("Exiting as a test, found end of the switch block (I think) switch_body_statement = %p \n",switch_body_statement);
                              ROSE_ASSERT(false);
#endif
                            }
#if DEBUG_SWITCH
                         printf ("This is some other kind of statement: not a declaration: entry = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
                      // DQ (2/8/2013): delete the map and pop the stack of maps.
#if DEBUG_SWITCH
                         printf ("Leaving case stmk_switch: switchSupportMap->size() = %" PRIuPTR " \n",switchSupportMap->size());
#endif

#if DEBUG_SWITCH
                         map<a_source_sequence_entry_ptr,SgStatement*>::iterator i = switchSupportMap->begin();
                         while (i != switchSupportMap->end())
                            {
                              printf ("switchSupportMap: i->first = %p i->second = %p \n",i->first,i->second);
                              i++;
                            }
#endif
                         delete switchSupportMap;
                         switchSupportMap = NULL;
                         switchSupportStackOfMaps.pop_back();
#if DEBUG_SWITCH
                         printf ("Leaving case stmk_switch: switchSupportStackOfMaps.size() = %" PRIuPTR " \n",switchSupportStackOfMaps.size());
#endif
                      // DQ (11/22/2017): endif for old version of switch implementation (not handling duff's device).
#endif

#if DEBUG_PRINT_SCOPE_STACK
                         printScopeStack("Leaving case stmk_switch");
#endif
                         break;
                       }


                    case stmk_goto:
                       {
#ifdef DEBUG_CONVERSION
                         indent_line(); debug_print("goto p = %p\n",p);
                         ScopedIndent si(indent);
#endif

                         a_label_ptr label = p->variant.label.ptr;
#if 0
                         printf ("In parse_statement(): case stmk_goto: stmk_goto = %d stmk_label = %d calling convert_goto() p->expr = %p \n",stmk_goto,stmk_label,label);
#endif
#if 0
                         printf ("In parse_statement(): case stmk_goto: calling summarize_label(label = %p) \n",label);
                         summarize_label(label);
#endif
#if 0
                         if (label->switch_break_label == true)
                            {
                              printf ("This is a break within a switch statment \n");
                            }
#endif
                      // This may be either a break or continue statement.
                         SgStatement* gs = convert_goto(label); 
                         ROSE_ASSERT (gs != NULL);

                         set_file_info_for_stmt(gs, p);
                         result.push_back(gs);
                         break;
                       }

                    case stmk_label:
                       {
#ifdef DEBUG_CONVERSION
                         indent_line(); debug_print("label p = %p\n",p);
                         ScopedIndent si(indent);
#endif
#if 0
                         printf ("In parse_statement(): case stmk_label: Building a SgLabelStatement \n");
#endif
#if 0
                         printf ("In parse_statement(): case stmk_label: exiting if we ever get here! \n");
                         ROSE_ASSERT(false);
#endif
                         a_label_ptr label = p->variant.label.ptr;

#if 0
                         printf ("In parse_statement(): case stmk_label: calling summarize_label(label = %p) \n",label);
                         summarize_label(label);
#endif
                      // DQ (1/6/2018): Added assertion that every lable will have a name.
                         ROSE_ASSERT(label->source_corresp.name != NULL);

                         if (label->source_corresp.name == NULL)
                            {
#if 0
                              printf ("In parse_statement(): case stmk_label: Detected a compiler generated label (trigger for end of case options statement) \n");
#endif
#ifdef DEBUG_CONVERSION
                              indent_line(); debug_print("- compiler-generated\n");
#endif
                              printf ("In parse_statement(): case stmk_label: Exiting if we find a compiler-generated label (I don't think these exist) \n");
                              ROSE_ASSERT(false);

                              printf ("In parse_statement(): case stmk_label: Skip to end label (using goto) \n");
                           // return NULL;
                           // break;
                              goto end;
                            }
#if 0
                         printf ("In parse_statement(): case stmk_label: calling convert_label_use(label = %p) \n",label);
#endif
                         SgLabelStatement* ls = convert_label_use(label); // This builds the label if it does not already exist
                         ROSE_ASSERT(ls != NULL);
#if 0
                         printf ("In parse_statement(): case stmk_label: ls = %p \n",ls);
#endif
                      // DQ (7/18/2012): Test and set the parent.
                         ROSE_ASSERT(ls->get_parent() == NULL);
                         ls->set_parent(topScopeStack());
                         ROSE_ASSERT(ls->get_parent() != NULL);
#if 0
                         printf ("Building the associated statement to the label: p->variant.label.exec_stmt = %p (may have already been processed) \n",label->exec_stmt);
#endif
                         ROSE_ASSERT(label->exec_stmt != NULL);
                      // ls->set_statement(parse_statement(label->exec_stmt));
                      // ls->set_statement(parse_statement(sse));
                      // SgStatement* body = build_basic_block_if_multiple_statements(parse_statement(sse));
                         if (sse->entity.kind != iek_src_seq_end_of_construct)
                            {
#if 0
                              printf ("In parse_statement(): case stmk_label: (sse->entity.kind != iek_src_seq_end_of_construct): Calling build_basic_block_if_multiple_statements(parse_statement(sse)); \n");
#endif
                              SgStatement* body = build_basic_block_if_multiple_statements(parse_statement(sse));

                              if (ls->get_statement() != NULL && isSgNullStatement(ls->get_statement()) != NULL)
                                 {
                                   delete ls->get_statement();
                                   ls->set_statement(NULL);
                                 }

                              ls->set_statement(body);

                           // DQ (1/7/2018): Need to set the parent of the body to the label statement.
                              body->set_parent(ls);
                            }
                           else
                            {
#if 0
                              printf ("Note: detected label without association to statement (sse->entity.kind == iek_src_seq_end_of_construct) \n");
                              printf ("SgLabelStatement: ls->get_statement() = %p = %s \n",ls->get_statement(),ls->get_statement() ? ls->get_statement()->class_name().c_str() : "null");
#endif
                           // DQ (12/23/2012): This should be true (SgLabelStatement is built this way by default).
                              ROSE_ASSERT(ls->get_statement() != NULL && isSgNullStatement(ls->get_statement()) != NULL);

                              if (ls->get_statement() != NULL && isSgNullStatement(ls->get_statement()) != NULL)
                                 {
                                   if (ls->get_statement()->get_startOfConstruct()->isCompilerGenerated() == true && ls->get_statement()->get_startOfConstruct()->isOutputInCodeGeneration() == false)
                                      {
                                        ls->get_statement()->get_startOfConstruct()->setOutputInCodeGeneration();

                                        ROSE_ASSERT(ls->get_statement()->get_endOfConstruct()->isCompilerGenerated() == true && ls->get_statement()->get_endOfConstruct()->isOutputInCodeGeneration() == false);
                                        ls->get_statement()->get_endOfConstruct()->setOutputInCodeGeneration();
                                      }
                                 }
                            }
#if 0
                         printf ("In parse_statement(): case stmk_label: ls->get_statement() = %p = %s \n",ls->get_statement(),ls->get_statement() != NULL ? ls->get_statement()->class_name().c_str() : "null");
#endif
                         if (ls != NULL)
                            {
                           // DQ (7/30/2013): Note that because this case will return a vector of at least size 2, it will 
                           // force a block to be built in the function build_basic_block_if_multiple_statements(), which 
                           // is inconsistant with the original source code.

                           // If ls is NULL, the label is compiler-generated and does not appear in ROSE
                              set_file_info_for_stmt(ls, p);
                              result.push_back(ls);

                              ROSE_ASSERT(ls->get_statement() != NULL);

                           // DQ (1/6/2018): We want to handle the label statment as a proper compound statement which means it should not be added to the block.
                           // DQ (1/14/2013): We need to also add the statement associated with the label statement to the list of statement 
                           // (this is a bug reported by Robb and also an issue with the wget application, a test code for this is in test2013_01.c).
                           // result.push_back(ls->get_statement());
#if 0
                              printf ("In parse_statement(): case stmk_label: Adding SgLabelStatement ls = %p and the associated ls->get_statement() = %p = %s to the result vector of statements \n",
                                   ls,ls->get_statement(),ls->get_statement()->class_name().c_str());
                              printf ("In parse_statement(): case stmk_label: Supressing the redundent addition of the statment wrapped by the lable statement into the result vector \n");
                              printf ("   --- result.size() = %zu \n",result.size());
#endif
                            }

#if 0
                         printf ("In parse_statement(): case stmk_label: leaving case: ls->get_statement() = %p = %s \n",ls->get_statement(),ls->get_statement() != NULL ? ls->get_statement()->class_name().c_str() : "null");
#endif
                         break;
                       }

                    case stmk_return:
                       {
#ifdef DEBUG_CONVERSION
                         indent_line(); debug_print("return statement\n");
#endif
#if 0
                         printf ("Calling summarize_statement() for stmk_return \n");
                         summarize_statement(p);
#endif
#if 0
                      // printf ("string generateScopeName (sse ) = %s \n",generateScopeName ((a_source_sequence_entry_ptr)sse ).c_str());
                      // printf ("string generateScopeName (sse ) = %s \n",generateScopeName (sse.operator->()).c_str());

// SgFunctionDefinition* 

                         printf ("Calling summarize_sse() for stmk_return \n");
                      // summarize_sse(p);
#endif
                            {
                              SgExpression* return_expr = NULL;
                              if (p->variant.return_dynamic_init != NULL)
                                 {
#ifdef DEBUG_CONVERSION
                                   indent_line(); debug_print("+ by dynamic init\n");
                                   ScopedIndent si(indent);
#endif
                                // DQ (2/25/2012): EDG specifies that this should be NULL when return_dynamic_init is non-NULL.
                                   ROSE_ASSERT(p->expr == NULL);
#if 0
                                // DQ (2/25/2012): I think we want to have a non-null edgType here (see test2012_07.C).
                                   a_type_ptr edgType = NULL;
                                   printf ("Calling convert_dynamic_initializer() with edgType = %p \n",edgType);
#endif
                                // We need to handle the case of a cast to a class in the return statement.  The new version of EDG (4.0+) 
                                // does not have a reference to the associated constructor call or the associated class.
#if 0
                                   printf ("Calling convert_dynamic_initializer(): global_innermost_function_scope = %p \n",global_innermost_function_scope);
#endif
                                   ROSE_ASSERT(global_innermost_function_scope != NULL);
                                   a_routine_ptr curr_routine = global_innermost_function_scope->variant.routine.ptr;
                                   a_type_ptr curr_routine_type = skip_typerefs(curr_routine->type);
                                   a_type_ptr return_type = curr_routine_type->variant.routine.return_type;
                                   ROSE_ASSERT(return_type != NULL);

                                   return_expr = convert_dynamic_initializer(p->variant.return_dynamic_init,return_type);
                                 }
                                else
                                 {
                                   if (p->expr != NULL)
                                      {
#ifdef DEBUG_CONVERSION
                                        indent_line(); debug_print("+ by expression\n");
                                        ScopedIndent si(indent);
#endif
                                        return_expr = convert_expression(p->expr);
                                      }
                                     else
                                      {
#ifdef DEBUG_CONVERSION
                                        indent_line(); debug_print("+ nothing\n");
#endif
#if 0
                                        printf ("If nothing was specified then build a SgNullExpression IR node \n");
#endif
                                     // DQ (3/26/2012): Added construction of a SgNullExpression for this case of "return;" in the source code (see test2006_125.C).
                                        return_expr = set_generated_file_info_to_unparse(buildNullExpression_nfi());
                                      }
                                 }

                              result.push_back(set_file_info_for_stmt(buildReturnStmt_nfi(return_expr), p));

                           // DQ (3/26/2012): Added test.
                              ROSE_ASSERT(return_expr != NULL);

                              if (return_expr != NULL)
                                 {
                                   ROSE_ASSERT(return_expr->get_parent() != NULL);
                                 }
                            }
                         break;
                       }

                    case stmk_block:
                       {
                      // DQ (5/17/2015): If this block is used as part of a GNU statement expression then it will 
                      // be marked as such in the EDG node, but I am not clear on how to use this information.

#define DEBUG_BLOCK 0

                         bool is_generated = (p->position.seq == 0);
#ifdef DEBUG_CONVERSION
                         indent_line(); debug_print("block is: %s\n", is_generated ? "generated" : "user supplied");
#endif
#if DEBUG_BLOCK
                         if (p->variant.block.statements == NULL)
                            {
                              printf ("In parse_statement(): case stmk_block: EMPTY BLOCK FOUND! \n");
                            }
#endif

                      // DQ (11/24/2017): Detect the case of a switch body's stmk_switch_case statement.
                         bool stop_after_procesing_statements_in_block = false;

#if DEBUG_BLOCK && defined(DEBUG_CONVERSION)
                         for (a_statement_ptr s = p->variant.block.statements; s != NULL; s = s->next)
                            {
                           // indent_line(); fprintf(stderr, "In parse_statement(): case stmk_block: + "); db_statement(s); debug_print("\n");
                           // indent_line(); printf("In parse_statement(): case stmk_block: + "); db_statement(s); debug_print("\n");
                              indent_line(); printf("In parse_statement(): case stmk_block: call db_statement(s = %p = %s) \n",s,summarize_statement_in_string(s)); // debug_print("\n");

                           // DQ (11/24/2017): Detect the case of a switch body's stmk_switch_case statement.
                              if (s->kind == stmk_switch_case)
                                 {
                                   printf ("WARNING: case of switch body containing stmk_switch_case is not handled in new duff's device support \n");
                                // ROSE_ASSERT(false);

                                   printf ("In parse_statement(): case stmk_block: Calling summarize_statement(s = %p) \n",p);
                                   summarize_statement(s);

                                   stop_after_procesing_statements_in_block = true;
                                 }
                            }
#endif
#if 0
                      // DQ (9/5/2013): Added debugging support to detect empty blocks from declarations in statement expressions.
                         if (p->kind == stmk_block)
                            {
                              a_block_ptr block = (a_block_ptr)(p->variant.block.extra_info);
                              ROSE_ASSERT(block != NULL);

                              printf ("In parse_statement(): case stmk_block: Calling summarize_block() block = %p \n",block);
                              summarize_block(block);
                              
                              printf ("In parse_statement(): case stmk_block: block->is_statement_expression = %s \n",block->is_statement_expression ? "true" : "false");
                            }
#endif
#ifdef DEBUG_CONVERSION
                         ScopedIndent si(indent);
#endif
#if DEBUG_BLOCK
                      // DQ (10/23/2012): Added test for switch statement (this will not work because 
                      // there may be a SgBasicBlock as the body of the switch at the top of the stack).
                         bool topScopeIsSwitchStatement = (isSgSwitchStatement(topScopeStack()) != NULL);
                         printf ("In parse_statement(): case stmk_block: generated block: topScopeIsSwitchStatement = %s \n",topScopeIsSwitchStatement ? "true" : "false");
#endif
#if DEBUG_BLOCK && 0
                         printf ("In parse_statement(): case stmk_block: calling summarize_statement() p = %p \n",p);
                         summarize_statement(p);
#endif
#if DEBUG_BLOCK
                         printf ("In parse_statement(): case stmk_block: is_generated = %s \n",is_generated ? "true" : "false");
#endif

                         if (is_generated == true)
                            {
                           // Generated block for scoping things that are already implicitly scopes in the language anyway; these do not have end-of-construct entries
#if DEBUG_BLOCK
                              printf ("In parse_statement(): case stmk_block: this is a compiler-generated block (NOT present in source code) p = %p \n",p);
#endif
                           // DQ (12/23/2013): Added to detect blocks that are only associated with implicit declaration 
                           // (e.g. implicit functions).  See test2013_108.c for an example.
                              ROSE_ASSERT(p->kind == stmk_block);
#if DEBUG_BLOCK
                              a_block_ptr block = (a_block_ptr)(p->variant.block.extra_info);
                              ROSE_ASSERT(block != NULL);
#endif
#if DEBUG_BLOCK
                              if (block->assoc_scope == NULL)
                                 {
                                   printf ("WARNING: detected case of stmk_block = %p supporting implicit declarations \n",block);
                                // ROSE_ASSERT(false);
                                 }
#endif
                           // DQ (10/15/2012): This is a test and it appears to work well at fixing the problem with loosing statements in compiler generated blocks.
                           // printf ("Increment sse as test before calling parse_statement_list_without_terminator() p = %p \n",p);
#if DEBUG_BLOCK
                              printf ("In parse_statement(): sse->entity.kind = %s \n",summarize_entry_kind_in_string_kind((an_il_entry_kind)(sse->entity.kind)));
#endif
#if 0
                              if ((an_il_entry_kind)(sse->entity.kind) == iek_statement)
                                 {
                                   a_statement_ptr stmt = (a_statement_ptr)(sse->entity.ptr);
                                   printf ("In parse_statement(): case stmk_block: is_generated == true: calling summarize_statement() \n");
                                   summarize_statement(stmt);
                                 }
#endif
                           // DQ (10/15/2012): This is an error to increment this here!
                           // ++sse;
#if DEBUG_BLOCK
                              printf ("In parse_statement(): case stmk_block: sse->entity.kind = %s \n",summarize_entry_kind_in_string_kind((an_il_entry_kind)(sse->entity.kind)));
#endif
#if DEBUG_BLOCK
                              printf ("In parse_statement(): case stmk_block: topScopeStack() = %p = %s \n",topScopeStack(),topScopeStack()->class_name().c_str());
#endif

                           // DQ (12/27/2013): This special case is required because the "true branch" and the "false branch" represent different scopes even 
                           // when no explicit "{}" are use to wrap the statement.  This is a strange aspect of C++ (not present in C).  Currently, the ROSE
                           // IR cannot support this, since the C++ if_stmt has a total of 3 scopes, with the true and false branch scopes not allowed to 
                           // redefine variables in the "if_stmt" scope, but allowed to define new types (e.g. class declarations).  This is demonstrated in
                           // C++ regression tests: test2013_291-296.C
                           // DQ (10/15/2012): Added SgBasicBlock so that we can use the proper scope when processing the associated statment.  This is only 
                           // really required for where the current scope is a SgIfStmt because the true and false branches require their own seperate scopes 
                           // to avoid type names from being mixed (see test2012_231.C).
                           // bool topScopeIsIfStmt = (isSgIfStmt(topScopeStack()) != NULL) || (isSgWhileStmt(topScopeStack()) != NULL);
                           // bool topScopeIsIfStmt = false;

                           // DQ (7/7/2019): This is required code, it is special processing for the if statement (above) is required for some test codes (listed above).
                           // DQ (7/6/2019): Disable this special handling (it is likely wrong and causing us to see differences 
                           // between different kinds of constructs that accept conditional statements).
                              bool topScopeIsIfStmt = (isSgIfStmt(topScopeStack()) != NULL);
#if DEBUG_BLOCK
                              printf ("In parse_statement(): case stmk_block: Remove special handling for SgIfStmt \n");
#endif
                           // DQ (7/7/2019): I think that the special processing for the if statement (above) is required for some test codes (listed above).
                           // bool topScopeIsIfStmt = false;

#if DEBUG_BLOCK
                              printf ("In parse_statement(): case stmk_block: generated block: topScopeIsIfStmt = %s \n",topScopeIsIfStmt ? "true" : "false");
#endif

                              SgBasicBlock* bb = NULL;
                              if (topScopeIsIfStmt == true)
                                 {
                                   bb = set_generated_file_info_to_unparse(buildBasicBlock_nfi());
                                   bb->set_parent(topScopeStack());

                                // DQ (9/6/2014): This might have already been processed for this scope as a result of now processing 
                                // the template member and non-member function declarations.
                                // DQ (10/31/2012): This is the simple way to record the ROSE scope associated with any edg statement.
                                // ROSE_ASSERT(statement_scope_cache.find(p) == statement_scope_cache.end());
                                // statement_scope_cache[p] = bb;
                                   if (statement_scope_cache.find(p) == statement_scope_cache.end())
                                      {
                                        statement_scope_cache[p] = bb;
                                      }
                                     else
                                      {
#if 0
                                        printf ("Warning: (statement_scope_cache.find(p) != statement_scope_cache.end()): scope was previously processed: p = %p statement_scope_cache[p] = %p = %s \n",
                                             p,statement_scope_cache[p],statement_scope_cache[p]->class_name().c_str());
#endif
                                      }
#if DEBUG_PUSH_SCOPE
                                   printf ("@@@@@ In parse_statement(): case stmk_block: topScopeIsIfStmt == true: push scope stack for generated block: block scope = %p = %s \n",bb,bb->class_name().c_str());
#endif
                                   pushScopeStack(bb);
                                 }

                           // result = parse_statement_list_without_terminator(sse, p, p->variant.block.statements);
                              vector<SgStatement*> stmts = parse_statement_list_without_terminator(sse, p, p->variant.block.statements, NULL);
#if DEBUG_BLOCK
                              printf ("AFTER call to parse_statement_list_without_terminator(): stmts.size() = %zu \n",stmts.size());
                              printf ("In parse_statement(): case of stmk_block: block = %p topScopeIsIfStmt = %s (if true CALLING appendStatementList()) stmts.size() = %" PRIuPTR " \n",block,topScopeIsIfStmt ? "true" : "false",stmts.size());
                              if (stmts.size() == 1)
                                 {
                                   printf ("This compiler generated block has a single statement: stmts[0] = %p = %s \n",stmts[0],stmts[0]->class_name().c_str());
                                 }
#endif
                           // DQ (12/23/2013): This special handling might be a mistake, test2013_108.c might be demonstrating this (or maybe not).
                              if (topScopeIsIfStmt == true)
                                 {
                                   ROSE_ASSERT(bb != NULL);
                                   appendStatementList(stmts, bb);
#if DEBUG_POP_SCOPE
                                   printf ("@@@@@ In parse_statement(): case stmk_block for generated block: calling popScopeStack() \n");
#endif
                                   popScopeStack();
                                   result.push_back(bb);
                                 }
                                else
                                 {
                                // Case of none ifStmt (which is more complex because it has true and false branches).
                                   result = stmts;
                                 }
#if DEBUG_BLOCK
                              printf ("DONE: In parse_statement(): case stmk_block: this is a compiler-generated block (NOT present in source code) p = %p \n",p);
#endif
                            }
                           else
                            {
#if DEBUG_BLOCK
                              printf ("In parse_statement(): case stmk_block: this is a NON-compiler-generated block (it IS present in source code) p = %p existingBasicBlock = %p \n",p,existingBasicBlock);
#endif
                           // DQ (8/21/2012): We want to reuse the existing SgBasicBlock if it exists.
                           // SgBasicBlock* bb = set_file_info_for_stmt(buildBasicBlock_nfi(), p);
                              SgBasicBlock* bb = NULL;
                              if (existingBasicBlock != NULL)
                                 {
                                // Reuse the input block (e.g. from body of SgFunctionDefinition).
                                // This permits the existing body of the SgFunctionDefinition to be put onto the top of stack.
                                   bb = set_file_info_for_stmt(existingBasicBlock, p);
#if 0
                                // DQ (10/31/2012): Not clear if we need to add this block or if it should already be present.
                                   ROSE_ASSERT(scope_cache.find(p) != scope_cache.end());
#endif
                                 }
                                else
                                 {
                                // Where no existing block is input, build a new one.
#if 0
                                // DQ (7/6/2019): New code. Reuse the existing SgBasicBlock if is was previously generated (e.g. as part of processing the declaration in a conditional statement.
                                   if (statement_scope_cache.find(p) != statement_scope_cache.end())
                                      {
                                        SgScopeStatement* associatedScope = statement_scope_cache[p];
                                        ROSE_ASSERT(associatedScope != NULL);
                                        bb = isSgBasicBlock(associatedScope);
                                        ROSE_ASSERT(bb != NULL);
#if 1
                                     // DQ (7/6/2019): Debugging test2019_05.c (I think we need to reuse the SgBasicBlock that is already in the cache).
                                        printf ("Exiting as a test! \n");
                                        ROSE_ASSERT(false);
#endif
                                      }
                                     else
#endif
                                      {
                                     // DQ (7/6/2019): Old code, which did not reuse the previously generated SgBasicBlock.
                                        bb = set_file_info_for_stmt(buildBasicBlock_nfi(), p);
#if DEBUG_BLOCK
                                        printf ("In parse_statement(): case stmk_block: this is a NON-compiler-generated block (it IS present in source code): bb = %p \n",bb);
#endif
                                     // DQ (2/11/2013): This assertion is violated when compiling "rose.h" (ROSE compiling ROSE).
                                     // Maybe it was previously set to this value.
                                     // DQ (10/31/2012): This is the simple way to record the ROSE scope associated with any edg statement.
                                     // ROSE_ASSERT(statement_scope_cache.find(p) == statement_scope_cache.end());
                                     // statement_scope_cache[p] = bb;
                                        if (statement_scope_cache.find(p) == statement_scope_cache.end())
                                           {
                                             statement_scope_cache[p] = bb;
                                           }
                                          else
                                           {
                                          // DQ (2/11/2013): Check that this is the same value.
                                             if (statement_scope_cache[p] != bb)
                                                {
                                               // DQ (7/5/2019): This is an issue for test2019_05.c
                                                  printf ("Note: In parse_statement(): case stmk_block: existingBasicBlock = %p SgBasicBlock (bb = %p) does not match that already assigned to cache (statement_scope_cache[p=%p] = %p) \n",existingBasicBlock,bb,p,statement_scope_cache[p]);

                                               // DQ (7/6/2019): Reset the scope in the cache.
                                                  statement_scope_cache[p] = bb;
#if 0
                                               // DQ (7/6/2019): Debugging test2019_09.c (I think we need to reuse the SgBasicBlock that is already in the cache).
                                                  printf ("Exiting as a test! \n");
                                                  ROSE_ASSERT(false);
#endif
                                                }
                                          // ROSE_ASSERT(statement_scope_cache[p] == bb);
                                           }
                                      }
                                 }

                           // DQ (7/9/2012): Let's not do this, so that we can be uniform in not setting the parent until later.
                           // DQ (12/30/2011): We might want to set the parent to be the top of the top of the scope stack before pushing the new SgBasicBlock.
                              bb->set_parent(topScopeStack());

#if DEBUG_PUSH_SCOPE
                              printf ("In parse_statement(): case stmk_block: push scope stack: block scope = %p = %s \n",bb,bb->class_name().c_str());
#endif
                              pushScopeStack(bb);

                           // DQ (7/9/2012): Let's not do this, so that we can be uniform in not setting the parent until later.
                           // DQ (12/30/2011): This might be too much to expect...
                              ROSE_ASSERT(bb->get_parent() != NULL);

                              vector<SgStatement*> stmts = parse_statement_list(sse, iek_statement, p);
                           // vector<SgStatement*> stmts = parse_statement_list(sse, iek_statement, p, p->variant.block.statements);

#if DEBUG_BLOCK || 0
                              printf ("In parse_statement(): case of stmk_block: CALLING appendStatementList() stmts.size() = %" PRIuPTR " \n",stmts.size());
                              if (stmts.size() >= 1)
                                 {
                                   printf ("In parse_statement(): case of stmk_block: CALLING appendStatementList() stmts.size() = %" PRIuPTR " stmts[0] = %p = %s bb = %p \n",
                                           stmts.size(),stmts[0],stmts[0]->class_name().c_str(),bb);
                                 }
#endif
#if 0
                              printf ("In parse_statement(): case stmk_block: stmts.size() = %zu \n",stmts.size());
#endif
#if 0
                           // DQ (3/25/2021): I don't think we need this now.
                           // DQ (3/25/2021): For EDG 6.x, we have to handle the support for case statements (and the default statement) differently,
                           // as a result we need to set the parent of any default statement explicitly.
                              for (size_t i = 0; i < stmts.size(); i++)
                                 {
                                   SgStatement* stmt = stmts[i];
                                   ROSE_ASSERT(stmt != NULL);

                                   SgDefaultOptionStmt* defaultStatement = isSgDefaultOptionStmt(stmt);
                                   if (defaultStatement != NULL)
                                      {
                                        if (defaultStatement->get_parent() == NULL)
                                           {
                                             printf ("In parse_statement(): case stmk_block: defaultStatement = %p = %s has null parent (set to bb = %p) \n",stmt,stmt->class_name().c_str(),bb);
                                          // defaultStatement->set_parent(bb);
                                           }

                                     // ROSE_ASSERT(defaultStatement->get_parent() != NULL);
#if 0
                                        printf ("Exiting as a test! \n");
                                        ROSE_ASSERT(false);
#endif
                                      }
                                     else
                                      {
                                        if (stmt->get_parent() == NULL)
                                           {
                                             printf ("In parse_statement(): case stmk_block: stmt = %p = %s has null parent \n",stmt,stmt->class_name().c_str());
                                           }
                                      }
                                // stmt->set_parent(bb);
                                 }
#endif

                              appendStatementList(stmts, bb);

#if DEBUG_POP_SCOPE
                              printf ("In parse_statement(): case stmk_block: calling popScopeStack() \n");
#endif
                              popScopeStack();
                              result.push_back(bb);
#if DEBUG_BLOCK
                              printf ("DONE: In parse_statement(): case stmk_block: this is a NON-compiler-generated block (it IS present in source code) p = %p \n",p);
#endif
                            }

                      // DQ (11/24/2017): Added support for testing new switch statement support (includes duff's device examples).
                         if (stop_after_procesing_statements_in_block == true)
                            {
                              printf ("End of case after procesing block statements when switch case statement was detected \n");
#if 0
                              printf ("Exiting after procesing block statements when case statement was detected \n");
                              ROSE_ASSERT(false);
#endif
                            }

                         process_static_local_variable_initializers(p->variant.block.extra_info->assoc_scope);
                         break;
                       }

                    case stmk_end_test_while:
                       {
#ifdef DEBUG_CONVERSION
                         indent_line(); debug_print("do-while loop\n");
                         ScopedIndent si(indent);
#endif
                      // ********** This is required for EDG 4.8 **********
                      // DQ (3/22/2014): The new EDG 4.8 adds an end_of_construct to match the case of a stmk_end_test_while.  
                      // So we have to push a scope so that there will be a scope to pop when we process the end-of-construct.

                      // DQ (2/15/2012): Build the "switch" scope first, so that we can use the correct scope (if the test contains a variable declaration).
                         SgDoWhileStmt* ws = new SgDoWhileStmt((SgStatement*)NULL,(SgStatement*)NULL);

                      // DQ (6/19/2012): Need to set the parent so that the scope can be computed for nested types generated in test2012_105.C.
                         ws->set_parent(topScopeStack());
#if 0
                         printf ("In parse_statement(): ws = %p = %s \n",ws,ws->class_name().c_str());
#endif
#if DEBUG_PUSH_SCOPE
                         printf ("In parse_statement(): case stmk_while: push scope stack: while statement scope = %p = %s \n",ws,ws->class_name().c_str());
#endif
                      // Temporarily push the new scope onto the scope stack.
                         pushScopeStack (ws);
                      // ********** This is required for EDG 4.8 **********


                      // SgStatement* test = convert_condition(p->expr);
                         SgStatement* test = convert_condition(p->expr,sse);
#if 0
                         printf ("@@@@@ Using EDG 4.8: case stmk_end_test_while: we might have to push a new scope here! \n");
#endif
                         SgStatement* body = build_basic_block_if_multiple_statements(parse_statement(sse));
#if 0
                         printf ("@@@@@ Using EDG 4.8: case stmk_end_test_while: we might have to pop a new scope here! \n");
#endif
#if DEBUG_PRINT_SCOPE_STACK
                         printScopeStack("In parse_statement(): After building do-while body in case stmk_end_test_while: List the scopes on the stack:");
#endif

                      // SgDoWhileStmt* ws = buildDoWhileStmt_nfi(body, test);
                         ROSE_ASSERT(ws != NULL);

                      // ws->set_body(body);
                      // ws->set_condition(test);
                      // DQ (3/22/2014): Now we want to use the scope that is already on the stack (instead of adding a new one).
                         buildDoWhileStatement_nfi(ws, body, test);
#if 0
                         printf ("Calling set_file_info_for_stmt(ws = %p, p) \n",ws);
#endif
                         set_file_info_for_stmt(ws, p);

#if DEBUG_POP_SCOPE
                         printf ("In parse_statement(): case stmk_end_test_while: calling popScopeStack() \n");
#endif
                      // DQ (3/22/104): Added call to popScopeStack()
                         popScopeStack();

                         result.push_back(ws);

#if DEBUG_PRINT_SCOPE_STACK
                         printScopeStack("In parse_statement(): END OF CASE stmk_end_test_while: List the scopes on the stack:");
#endif
                         if ((an_il_entry_kind)(sse->entity.kind) == iek_src_seq_end_of_construct)
                            {
                              ++sse;
                            }
                         break;
                       }

                    case stmk_for:
                       {
#ifdef DEBUG_CONVERSION
                         indent_line(); debug_print("for loop\n");
                         ScopedIndent si(indent);
#endif

#define DEBUG_FOR_LOOP 0

                         a_for_loop_ptr for_loop = p->variant.for_loop.extra_info;
#if DEBUG_FOR_LOOP
                         printf ("Building SgForStatement on scope stack \n");
#endif
#if 0
                         printf ("Calling summarize_loop_info() \n");
                         ROSE_ASSERT(for_loop != NULL);

                         if (for_loop->initialization != NULL)
                            {
                              summarize_statement(for_loop->initialization);
                            }
                         if (for_loop->for_init_scope != NULL)
                            {
                              summarize_scope(for_loop->for_init_scope);
                            }
#endif
                      // DQ (9/26/2012): Add the scope early so that any possible declarations in the for loop initializer will be put into 
                      // the correct scope and not use types or variables from the outer scope (see test2012_106.C and test2012_223.C).
                         SgForStatement* forStatementScope = new SgForStatement((SgStatement*)NULL,(SgExpression*)NULL,(SgStatement*)NULL);

                      // DQ (9/26/2012): This assertion fails for test2004_85.C
                      // ROSE_ASSERT(isSgBasicBlock(topScopeStack()) != NULL);

                      // DQ (9/9/26/2012): We have to set the parent (so that traversals back through scopes will succeed).
                         forStatementScope->set_parent(topScopeStack());
#if DEBUG_PUSH_SCOPE
                         printf ("In parse_statement(): case stmk_for: push scope stack: for statement scope = %p = %s \n",forStatementScope,forStatementScope->class_name().c_str());
#endif
                         pushScopeStack(forStatementScope);
#if DEBUG_FOR_LOOP
                         printf ("Calling parse_for_initializer() topScopeStack() = %p = %s \n",topScopeStack(),topScopeStack()->class_name().c_str());
#endif
                      // DQ (1/28/2013): Provide better source position information for case were for_loop->initialization == NULL.
                      // SgForInitStatement * for_loop_initialization = for_loop->initialization == NULL ? set_generated_file_info_to_unparse(buildForInitStatement(buildNullStatement_nfi())) : parse_for_initializer(sse);
                         SgForInitStatement * for_loop_initialization = for_loop->initialization == NULL ? set_file_info_for_stmt(buildForInitStatement(buildNullStatement_nfi()),p) : parse_for_initializer(sse);
                         ROSE_ASSERT(for_loop_initialization != NULL);

                      // DQ (1/28/2013): Set the end source position to be the same as the start (of the for loop).  Note that the for loop
                      // will span several lines and this does not make for a good source position when the for loop initializer is not present.
                         if (for_loop->initialization == NULL)
                            {
                              *(for_loop_initialization->get_endOfConstruct()) = *(for_loop_initialization->get_startOfConstruct());
                            }
#if DEBUG_FOR_LOOP
                         printf ("In parse_statement(): case stmk_for: calling convert_condition(): for statement scope = %p = %s \n",forStatementScope,forStatementScope->class_name().c_str());
#endif
#if DEBUG_FOR_LOOP
                         printf ("   --- sse before convert_condition(): entry = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
                      // DQ (1/28/2013): Provide better source position information for case were for_loop->initialization == NULL.
                      // DQ (10/12/2012): Make sure that we build proper null expressions and not just return ASTs with NULL pointers.
                      // SgStatement* test = p->expr == NULL ? set_generated_file_info_to_unparse(buildNullStatement_nfi()) : convert_condition(p->expr);
                      // SgStatement* test = p->expr == NULL ? set_generated_file_info_to_unparse(buildNullStatement_nfi()) : convert_condition(p->expr,sse);

                      // DQ (7/7/2019): Specify the target_statement_after_conditional (note: the increment does not count since it is not a part of the source sequence list).
                      // DQ (7/7/2019): I also think that the statement body should never be NULL.
                      // SgStatement* test = p->expr == NULL ? set_file_info_for_stmt(buildNullStatement_nfi(),p) : convert_condition(p->expr,sse);
                         ROSE_ASSERT(p->variant.for_loop.statement != NULL);
                         SgStatement* test = p->expr == NULL ? set_file_info_for_stmt(buildNullStatement_nfi(),p) : convert_condition(p->expr,sse,p->variant.for_loop.statement);

                         ROSE_ASSERT(test != NULL);
#if DEBUG_FOR_LOOP
                         printf ("DONE: In parse_statement(): case stmk_for: calling convert_condition(): for statement scope = %p = %s \n",forStatementScope,forStatementScope->class_name().c_str());
#endif
#if DEBUG_FOR_LOOP
                         printf ("   --- sse after convert_condition(): entry = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
                      // DQ (1/28/2013): Set the end source position to be the same as the start (of the for loop).  Note that the for loop
                      // will span several lines and this does not make for a good source position when the for loop test is not present.
                         if (p->expr == NULL)
                            {
                              *(test->get_endOfConstruct()) = *(test->get_startOfConstruct());
                            }
#if DEBUG_FOR_LOOP
                         printf ("Calling convert_expression() \n");
#endif
                         SgExpression* incr = for_loop->increment == NULL ? set_generated_file_info_to_unparse(buildNullExpression_nfi()) : convert_expression(for_loop->increment);
                         ROSE_ASSERT(incr != NULL);
#if DEBUG_FOR_LOOP
                         printf ("Calling build_basic_block_if_multiple_statements(parse_statement(sse)) \n");
#endif
#if DEBUG_FOR_LOOP
                         printf ("   --- sse before build_basic_block_if_multiple_statements(): entry = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
                      // DQ (11/4/2012): if there one or more macros here after the "for" and before the loop body, then we have to iterate past them.
                      // This is a general problem and should perhaps be best implemented in either the top of parse_statement() or in the iterator for 
                      // the source sequence support (though that would seem to loose the information about their existence, which we don't use however).
                      // NOTE: we might or might not want to process #pragma statements in the same way...
                         if (sse->entity.kind == iek_macro)
                            {
                              while(sse->entity.kind == iek_macro)
                                 {
                                   ++sse;
                                 }
                            }

#if 1
                      // DQ (3/19/2015): Also loop over the sse entries that are associated with previously processed type 
                      // (see test2015_13.c, to avoid extra block before loop body). Note this is only an issue for C code (not C++).
                      // Note: I had thought that this shuld be a part of the conditional support function, but since these types 
                      // can be used almost anywhere it is less clear where the single location where this support be located should be.
                         an_il_entry_kind kind = (an_il_entry_kind)(sse->entity.kind);
                         a_type_ptr type = (kind == iek_type) ? (a_type_ptr) sse->entity.ptr : NULL;
                         if (type != NULL && type_translation_cache.find(type) != type_translation_cache.end())
                            {
                              ROSE_ASSERT(type != NULL);
#if DEBUG_FOR_LOOP
                              printf ("Handle orphaned scructure declaration: type = %p \n",type);
#endif
                           // DQ (3/19/2015): I hope that where this is a type it is always terminated by a iek_src_seq_end_of_construct.
                           // We could also check that we iterate until we reach the iek_src_seq_end_of_construct associated with the type we are trying to skip over.
                              while(kind != iek_src_seq_end_of_construct)
                                 {
#if DEBUG_FOR_LOOP
                                   printf ("At start of loop: Iterate past any types that have been previously processed (though in some cases this is supposed to be the defining declaration) \n");
                                   printf ("   --- sse after detection of previously processed type: entry = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
                                   ++sse;

                                   kind = (an_il_entry_kind)(sse->entity.kind);
#if DEBUG_FOR_LOOP
                                   printf ("At end of loop: \n");
                                   printf ("   --- sse after detection of previously processed type: end of loop: entry = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
                                 }

                              ROSE_ASSERT(kind == iek_src_seq_end_of_construct);
                              ++sse;

                           // Note that we might have several of these to process (this code only processes one of them!
#if DEBUG_FOR_LOOP
                              printf ("At end of processing the orphaned scructure declaration: \n");
                              printf ("   --- sse after processing the orphaned scructure declaration: entry = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
                            }
#else
                         printf ("Skipping loop over previously defined types in for loop processing \n");
#endif

#if 1
#if 0
                         printf ("Calling skip_over_previously_processed_blocks in for loop processing \n");
#endif
                      // DQ (3/23/2015): Refactoring of code to make it callable from elsewhere (test2015_14.c demonstrates the need for this to be more general).
                         skip_over_previously_processed_blocks(sse);
#if 0
                         printf ("DONE: Calling skip_over_previously_processed_blocks in for loop processing \n");
#endif
#else

#error "DEAD CODE!"

                      // DQ (3/21/2015): Test test2015_23.c demonstrates a GNU statement expression that is processed in the 
                      // for statement initializer.  This has to be skipped over since it was already processed (to get the 
                      // sse in sync with the processed AST).
                         an_il_entry_kind tmp_kind = (an_il_entry_kind)(sse->entity.kind);
                      // a_type_ptr type = (kind == iek_type) ? (a_type_ptr) sse->entity.ptr : NULL;
                         char* entity_ptr = sse->entity.ptr;
                         ROSE_ASSERT(p->variant.for_loop.statement != NULL);
#if 0
                         printf ("Testing for stmk_block between the for loop header and the for loop body: \n");
                         printf ("   --- p->variant.for_loop.statement = %p kind = %s \n",p->variant.for_loop.statement,summarize_statement_in_string(p->variant.for_loop.statement));
                         printf ("   --- sse entry                     = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif

#error "DEAD CODE!"

#if 1
                      // if (tmp_kind == stmk_block)
                         a_statement_ptr statement = (a_statement_ptr)(sse->entity.ptr);
                         if (tmp_kind == iek_statement && statement->kind == stmk_block)
                            {
                              char* associated_block_entity_ptr = entity_ptr;
#if DEBUG_FOR_LOOP
                              printf ("   --- tmp_kind == stmk_block: associated_block_entity_ptr = %p \n",associated_block_entity_ptr);
#endif
#if 0
                              printf ("Calling summarize_statement() \n");
                              summarize_statement(statement);
#endif

#if DEBUG_FOR_LOOP
                              printf ("In parse_statement(): case stmk_for: has this body stmt_ptr = %p been seen previously = %s (in statement_expression_bodies) \n",
                                   p,statement_expression_bodies.find(statement) != statement_expression_bodies.end() ? "true" : "false");
#endif
                           // DQ (3/22/2015): test2015_27.c demonstrates that we need to test that this is a previously processed EDG stmt_block node (and only then skip it).
                              if (statement_expression_bodies.find(statement) != statement_expression_bodies.end())
                                 {
                                // DQ (3/6/2017): Converted to use message logging.
                                // If this block has been previously processed then skip past it in the source sequence list.
                                   mprintf ("This block HAS been previously processed \n");

#if 0
                                   printf ("Exiting as a test! \n");
                                   ROSE_ASSERT(false);
#endif
#if 1
                              while (entity_ptr != NULL && entity_ptr != (char*)p->variant.for_loop.statement)
                                 {
#if DEBUG_FOR_LOOP
                                   printf ("Advance sse in processing of for loop: entity_ptr != (char*)for_loop->statement: Need to skip over this sse entity: entity_ptr = %p for_loop->statement = %p \n",entity_ptr,p->variant.for_loop.statement);
                                   printf ("   --- (before ++sse): sse entry = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
                                   ++sse;

                                   an_il_entry_kind local_tmp_kind = (an_il_entry_kind)(sse->entity.kind);
                                   if (local_tmp_kind == iek_src_seq_end_of_construct)
                                      {
                                     // a_src_seq_end_of_construct_ptr src_seq_end_of_construct = (a_src_seq_end_of_construct_ptr)(sse->entry.ptr);
                                     // a_src_seq_end_of_construct_ptr src_seq_end_of_construct = (a_src_seq_end_of_construct_ptr)(sse->entry.ptr);
                                     // a_src_seq_end_of_construct_ptr src_seq_end_of_construct = (a_src_seq_end_of_construct_ptr)(entry.ptr);
                                        a_src_seq_end_of_construct_ptr src_seq_end_of_construct = (a_src_seq_end_of_construct_ptr)(sse->entity.ptr);


                                     // if (src_seq_end_of_construct->entity.ptr != associated_block_entity_ptr)
                                        if (src_seq_end_of_construct->entity.ptr != associated_block_entity_ptr)
                                           {
                                             entity_ptr = sse->entity.ptr;
#if DEBUG_FOR_LOOP
                                             printf ("   --- (after ++sse (local_tmp_kind == iek_src_seq_end_of_construct)): sse entry = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
                                           }
                                          else
                                           {
#if DEBUG_FOR_LOOP
                                             printf ("   --- Found associated end of block \n");
#endif
                                             entity_ptr = NULL;
                                             ++sse;
                                           }
                                      }
                                     else
                                      {
                                     // ROSE_ASSERT(src_seq_end_of_construct->kind == stmk_block);
                                     // ROSE_ASSERT(sse->entity.kind == iek_statement);
                                        if (sse->entity.kind == iek_statement)
                                           {
                                             a_statement_ptr associated_statement = (a_statement_ptr)(sse->entity.ptr);
#if 0
                                             printf ("local_tmp_kind != iek_src_seq_end_of_construct: Calling summarize_statement() associated_statement = %p \n",associated_statement);
                                             summarize_statement(associated_statement);
#endif
                                           }
                                          else
                                           {
#if DEBUG_FOR_LOOP
                                             printf ("   --- (sse->entity.kind != iek_statement): sse entry = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
                                           }

                                        entity_ptr = sse->entity.ptr;
#if DEBUG_FOR_LOOP
                                        printf ("   --- (after ++sse): sse entry = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
                                      }
#if 0
                                   printf ("Exiting as a test! \n");
                                   ROSE_ASSERT(false);
#endif
                                 }
#endif
                                 }
                                else
                                 {
#if DEBUG_FOR_LOOP
                                   printf ("This block has been NOT been previously processed (so process the source sequence list) \n");
#endif
                                 }
                            }
                           else
                            {
#if DEBUG_FOR_LOOP
                           // printf ("   --- not a stmk_block \n");
                              printf ("   --- not a stmk_block: sse entry = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
                            }
#else
                         printf ("Skipping loop over previously defined blocks in for loop processing \n");
                         printf ("   --- p->variant.for_loop.statement = %p kind = %s \n",p->variant.for_loop.statement,summarize_statement_in_string(p->variant.for_loop.statement));
                         printf ("   --- sse entry                     = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
#endif

#if DEBUG_FOR_LOOP
                         printf ("Process the loop body: sse entry = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
                         SgStatement* body = build_basic_block_if_multiple_statements(parse_statement(sse));
                         ROSE_ASSERT(body != NULL);
#if DEBUG_FOR_LOOP
                         printf ("DONE: Calling build_basic_block_if_multiple_statements(parse_statement(sse)) body = %p = %s \n",body,body->class_name().c_str());
#endif
#if 0
                         printf ("   --- sse after build_basic_block_if_multiple_statements(): entry = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif

                      // DQ (10/12/2012): Added assertion. This shows that there is a SgForInitStatement build automatically (semantics I don't like).
                         ROSE_ASSERT(forStatementScope->get_for_init_stmt() != NULL);

                      // DQ (9/26/2012): Now we want to use the scope that is already on the stack (instead of adding a new one).
                         buildForStatement_nfi(forStatementScope, for_loop_initialization, test, incr, body);

                         set_file_info_for_stmt(forStatementScope, p);

#if DEBUG_POP_SCOPE
                         printf ("In parse_statement(): case stmk_for: calling popScopeStack() \n");
#endif
                         popScopeStack();
                         result.push_back(forStatementScope);
#if DEBUG_FOR_LOOP
                         printf ("Calling process_static_local_variable_initializers() \n");
#endif
                         process_static_local_variable_initializers(p->variant.for_loop.extra_info->for_init_scope);

                         ROSE_ASSERT(forStatementScope->get_for_init_stmt() != NULL);
                         ROSE_ASSERT(forStatementScope->get_test()          != NULL);
                         ROSE_ASSERT(forStatementScope->get_increment()     != NULL);
                         ROSE_ASSERT(forStatementScope->get_loop_body()     != NULL);
#if DEBUG_FOR_LOOP
                         printf ("DONE: case stmk_for forStatementScope = %p for_loop_initialization = %p \n",forStatementScope,for_loop_initialization);
                         forStatementScope->get_file_info()->display("case stmk_for forStatementScope");
#endif
                         break;
                       }

                    case stmk_init: ROSE_ASSERT (!"stmk_init not supported"); abort();

                    case stmk_asm:
                       {
                         an_asm_entry_ptr asm_entry = p->variant.asm_entry;
#ifdef DEBUG_CONVERSION
                         indent_line(); debug_print("asm statement (stmk_asm) "); print_source_corresp(asm_entry->source_corresp); debug_print("\n");
                         ScopedIndent si(indent);
#endif
                      // DQ (5/6/2013): Don't process this as a string constant (since we don't want to escape the "\n" type characters).
                      // string asm_string = convert_string_constant(asm_entry->asm_string);
                         ROSE_ASSERT (asm_entry->asm_string->kind == ck_string);

                      // DQ (9/14/2013): Length of the string, in bytes.  Includes the trailing null, if any.
                      // Be careful: string literals may have more than one null, or none at all -- use the length.
                         string asm_string(asm_entry->asm_string->variant.string.value, asm_entry->asm_string->variant.string.length);
#if 0
                         printf ("In parse_statement(): case stmk_asm: asm_string               = %s \n",asm_string.c_str());
                         printf ("In parse_statement(): case stmk_asm: escapeString(asm_string) = %s \n",escapeString(asm_string).c_str());
#endif
#if 0
                      // DQ (9/14/2013): Note that test2013_72.c demonstrate a stray null at the end of the asm string which is
                      // then unparsed as "\000" trash at the end and then fails in the assembler phase within the GNU tool chain.
                         for (size_t i = 0; i < asm_string.length(); i++)
                            {
                              printf ("In parse_statement(): case stmk_asm: ascii value for asm_string[i=%" PRIuPTR "] = %u \n",i,asm_string[i]);
                            }
#endif
                      // DQ (9/14/2013): Trim any trailing null characters (see test2013_72.c).
                         size_t length = asm_string.length();
                         ROSE_ASSERT(length > 0);
#if 0
                         printf ("In parse_statement(): case stmk_asm: Starting length = %" PRIuPTR " \n",length);
                         printf ("In parse_statement(): case stmk_asm: BEFORE TRIMMING TRAILING NULL CHAR: escapeString(asm_string) = %s \n",escapeString(asm_string).c_str());
#endif
                         while (length > 0 && asm_string[length-1] == '\0')
                            {
#if 0
                              printf ("In parse_statement(): case stmk_asm: BEFORE TRIMMING TRAILING NULL CHAR: loop: length = %" PRIuPTR " \n",length);
                              printf ("In parse_statement(): case stmk_asm: BEFORE TRIMMING TRAILING NULL CHAR: loop: escapeString(asm_string) = %s \n",escapeString(asm_string).c_str());
#endif
                              asm_string = asm_string.substr(0,length-1);
                              length--;
#if 0
                              printf ("In parse_statement(): case stmk_asm: AFTER TRIMMING TRAILING NULL CHAR: loop: length = %" PRIuPTR " \n",length);
                              printf ("In parse_statement(): case stmk_asm: AFTER TRIMMING TRAILING NULL CHAR: loop: escapeString(asm_string) = %s \n",escapeString(asm_string).c_str());
#endif
                            }
#if 0
                         printf ("In parse_statement(): case stmk_asm: AFTER TRIMMING TRAILING NULL CHAR: asm_string               = %s \n",asm_string.c_str());
                         printf ("In parse_statement(): case stmk_asm: AFTER TRIMMING TRAILING NULL CHAR: escapeString(asm_string) = %s \n",escapeString(asm_string).c_str());
#endif
                      // SgAsmStmt* asmStmt = set_file_info_for_stmt(new SgAsmStmt(), p); // FIXME build function
                         SgAsmStmt* asmStmt = new SgAsmStmt();

                      // DQ (11/3/2012): Added call to set file info to default settings.
                         setSourcePosition(asmStmt);

                         asmStmt = set_file_info_for_stmt(asmStmt, p); // FIXME build function

                         ROSE_ASSERT(asmStmt != NULL);

                         asmStmt->set_definingDeclaration(asmStmt);
                         asmStmt->set_firstNondefiningDeclaration(asmStmt);
                         asmStmt->set_assemblyCode(asm_string);

                      // DQ (7/18/2012): Set the parent.
                         ROSE_ASSERT(asmStmt->get_parent() == NULL);
                         asmStmt->set_parent(topScopeStack());
                         ROSE_ASSERT(asmStmt->get_parent() != NULL);
#if 0
                         printf ("GNU_EXTENSIONS_ALLOWED = %d \n",(int)GNU_EXTENSIONS_ALLOWED);
#endif
#if GNU_EXTENSIONS_ALLOWED
#ifdef DEBUG_CONVERSION
                         if (asm_entry->is_volatile == true)
                            {
                              printf ("Mark asm statement AS volatile \n");
                            }
                           else
                            {
                              printf ("Mark asm statement as NON volatile \n");
                            }
#endif
                      // volatile is a GNU extension (so it need not force SgAsmStmt to be a declaration, if should perhaps be a SgStatement).
                      // DQ (12/12/2012): We put this out in the unpasered code, but it is not explicitly represented in the input code.
                      // asmStmt->set_isVolatile(asm_entry->is_volatile == true);
                         if (asm_entry->is_volatile == true)
                            {
                              asmStmt->set_isVolatile(true);

                           // DQ (8/31/2013): This has to be set as well (checked by the unparser).
                              asmStmt->get_declarationModifier().get_typeModifier().get_constVolatileModifier().setVolatile();
                            }

                      // DQ (5/6/2013): This should not be set if it was not in the original source code.
                      // An "asm" statement is a declaration in ROSE, but it is not clear if it really should be (review this detail).
                      // asmStmt->get_declarationModifier().get_typeModifier().get_constVolatileModifier().setVolatile();

#ifdef DEBUG_CONVERSION
                         printf ("Note: Set both local flag 'isVolatile' flag and 'volatile' flag in declarationModifier().get_typeModifier().get_constVolatileModifier() \n");
#endif
#if 0
                         printf ("asm_entry->gnu_asm_form = %s \n",asm_entry->gnu_asm_form ? "true" : "false");
#endif
                         if (asm_entry->gnu_asm_form == true)
                            {
                           // The asm declaration used the extended GNU syntax in which the asm string is followed by a colon.
                           // This matters even when the operands and clobbers lists are empty because the meaning of the asm
                           // string may be subtly different if this flag is TRUE.

                              asmStmt->set_useGnuExtendedFormat(true);
#if 0
                              printf ("asm_entry->operands = %p \n",asm_entry->operands);
#endif
                           // FIXME: handle all of these
                              for (an_asm_operand_ptr asm_operand = asm_entry->operands; asm_operand != NULL; asm_operand = asm_operand->next)
                                 {
                                // Not implemented!
#ifdef DEBUG_CONVERSION
                                   printf ("**********\nprocess an asm operand: name = %p = %s \n",asm_operand->name, asm_operand->name != NULL ? asm_operand->name : "NULL");
#endif

// DQ (1/9/2009): This may be a bug in EDG (all operands are maked as output operands).
#if RECORD_RAW_ASM_OPERAND_DESCRIPTIONS
#if 0
  #warning "When RECORD_RAW_ASM_OPERAND_DESCRIPTIONS == TRUE, all the operands are marked as output operands..."
#endif
#ifdef DEBUG_CONVERSION
                                   printf ("RECORD_RAW_ASM_OPERAND_DESCRIPTIONS is %s \n",RECORD_RAW_ASM_OPERAND_DESCRIPTIONS ? "TRUE" : "FALSE");
#endif
#else
#ifdef DEBUG_CONVERSION
                                   printf ("RECORD_RAW_ASM_OPERAND_DESCRIPTIONS is NOT DEFINED \n");
#endif
#endif
                                // This is a byte sized bit vector (see EDG: il_def.h file for more details)
                                // printf ("asm_operand_ptr->modifiers   = %o \n",asm_operand_ptr->modifiers);
                                // printf ("asm_operand_ptr->expression  = %p \n",asm_operand_ptr->expression);
                                // generateFileInfo(asm_operand_ptr->position)->display("location of asm operand");
                                   ROSE_ASSERT(asm_operand->expression != NULL);
                                   SgExpression* tempExp = convert_expression(asm_operand->expression);
                                   ROSE_ASSERT(tempExp != NULL);
#ifdef DEBUG_CONVERSION
                                   printf ("tempExp = %p = %s = %s \n",tempExp,tempExp->class_name().c_str(),SageInterface::get_name(tempExp).c_str());
#endif
#if !RECORD_RAW_ASM_OPERAND_DESCRIPTIONS
                                // An operand constraint for the GNU extended assembly syntax.
                                   an_asm_operand_constraint_ptr operand_constraint_ptr = asm_operand->constraints;

                                // See test2006_98.C; this code has some unknown constrant codes which are ignored 
                                // by EDG, and I think result in (asm_operand->constraints == NULL).
                                // ROSE_ASSERT(operand_constraint_ptr != NULL);
                                   if (asm_operand->constraints == NULL)
                                      {
                                        printf ("############## asm_operand->constraints == NULL #################\n");
                                      }

                                // Note that we use the same values in ROSE and EDG so we can just cast from the EDG enum type to the ROSE enum type.
                                // SgAsmOp::asm_operand_constraint_enum constraint = (SgAsmOp::asm_operand_constraint_enum) operand_constraint_ptr->kind;
                                   SgAsmOp::asm_operand_constraint_enum constraint = (operand_constraint_ptr != NULL) ? (SgAsmOp::asm_operand_constraint_enum) operand_constraint_ptr->kind : SgAsmOp::e_reg_q /* or e_invalid? */;
                                   SgAsmOp::asm_operand_modifier_enum   modifiers  = (SgAsmOp::asm_operand_modifier_enum)   asm_operand->modifiers;

                                   SgAsmOp* sageAsmOp = set_file_info_for_stmt(new SgAsmOp(constraint,modifiers,tempExp),p);
                                   tempExp->set_parent(sageAsmOp);
                                   sageAsmOp->set_recordRawAsmOperandDescriptions(false);
#else
                                // SgAsmOp* sageAsmOp = set_file_info_for_stmt(new SgAsmOp(SgAsmOp::e_invalid,SgAsmOp::e_unknown,tempExp),p);
                                   SgAsmOp* sageAsmOp = new SgAsmOp(SgAsmOp::e_invalid,SgAsmOp::e_unknown,tempExp);
                                   tempExp->set_parent(sageAsmOp);

                                // DQ (11/3/2012): Added call to set file info to default settings.
                                   setSourcePosition(sageAsmOp);

                                   sageAsmOp = set_file_info_for_stmt(sageAsmOp,p);

                                   sageAsmOp->set_recordRawAsmOperandDescriptions(true);

                                   if (asm_operand->is_output_operand == true)
                                      {
#if 0
                                        printf ("This is an output operand \n");
#endif
                                        sageAsmOp->set_isOutputOperand(true);
                                      }
                                   if (asm_operand->constraints_string != NULL)
                                      {
#if 0
                                        printf ("This is an output operand: asm_operand->constraints_string = %s \n",asm_operand->constraints_string);
#endif
                                        sageAsmOp->set_constraintString(asm_operand->constraints_string);
                                      }
#endif
                                // Operands with names are fairly rare (but it happens in Linux; see test2007_20.C)
                                   if (asm_operand->name != NULL)
                                      {
                                     // If the name is available we should likely be saving it in the SgAsmOp IR node.
                                     // The symbolic name indicated for this operand (using the "[ <identifier> ]" syntax), or NULL if none was given.
#if 0
                                        printf ("This is an output operand name = %s \n",asm_operand->name);
#endif
                                        sageAsmOp->set_name(asm_operand->name);
                                      }

                                   asmStmt->get_operands().push_back(sageAsmOp);
                                   sageAsmOp->set_parent(asmStmt);
                                 }
#if 0
                              printf ("Linked list of clobbers: asm_entry->clobbers = %p \n",asm_entry->clobbers);
#endif
                              for (a_named_register_list_ptr clobbers = asm_entry->clobbers; clobbers != NULL; clobbers = clobbers->next)
                                 {
                                // Not implemented!
#if 0
                                   printf ("process an asm clobber list entry! asm_entry->clobbers = %p \n",asm_entry->clobbers);
#endif
                                // DQ (12/12/2012): Bug fix for test2012_173.c and test2012_175.c.
                                   a_named_register & registerName = clobbers->reg;
#if 0
                                   printf ("registerName = %d (anr_last = %d) GNU_X86_ASM_EXTENSIONS_ALLOWED = %d \n",registerName,anr_last,(int)GNU_X86_ASM_EXTENSIONS_ALLOWED);
#endif
#if GNU_X86_ASM_EXTENSIONS_ALLOWED
                                // DQ (12/12/2012): Introduced x86 mode.
                                   SgInitializedName::asm_register_name_enum sageRegisterName = (SgInitializedName::asm_register_name_enum) registerName;
#else
                                // DQ (12/12/2012): Introduced non-x86 mode.
                                   ROSE_ASSERT(registerName == anr_unrecognized || registerName == anr_memory);
                                   SgInitializedName::asm_register_name_enum sageRegisterName = (registerName == anr_unrecognized) ? SgInitializedName::e_unrecognized_register : SgInitializedName::e_memory_register;
#endif
                                   asmStmt->get_clobberRegisterList().push_back(sageRegisterName);
                                 }
                            }
                           else
                            {
                           // DQ (3/6/2017): Added support for message logging to control output from ROSE tools.
                           // DQ (8/31/2013): Added this message for asm processing. I don't think this happens for GNU code.
                              mprintf ("NOTE: In parse_statement(): case stmk_asm: (asm_entry->gnu_asm_form == false) \n");
                            }
#endif
                         ROSE_ASSERT(asmStmt != NULL);
                         ROSE_ASSERT(asmStmt->get_parent() != NULL);

                         result.push_back(asmStmt);

                      // FIXME GNU extensions
                         break;
                       }

#if ASM_FUNCTION_ALLOWED
                    case stmk_asm_func_body: ROSE_ASSERT (!"stmk_asm_func_body not supported"); ROSE_ASSERT(false);
#endif

                    case stmk_try_block:
                       {
#ifdef DEBUG_CONVERSION
                         ScopedIndent si(indent);
#endif
                         result.push_back(set_file_info_for_stmt(convert_try_block_statement(sse,p), p));
#if 0
                         ROSE_ASSERT (!"stmk_try_block not supported"); 
                         ROSE_ASSERT(false);
#endif
                      // DQ (10/15/2012): Added break which appears to have been missing (likely fell through to break in case stmk_decl (which was not implemented).
                         break;
                       }

                 // case stmk_microsoft_try: ROSE_ASSERT (!"stmk_microsoft_try not supported"); abort();

                    case stmk_decl:
                       {
                      // Skipping these may not work if there are compiler-generated decls FIXME
#define DEBUG_STMT_DECL 0

#ifdef DEBUG_CONVERSION
                         indent_line(); debug_print("decl statement (in later source sequence entries p = %p)\n",p);
#endif
                      // This is a error (most declarations are critical to process).
                      // ++sse;
#if DEBUG_STMT_DECL
                         printf ("In parse_statement(): case stmk_decl: p->variant.decl.entities = %p \n",p->variant.decl.entities);
#endif
                      // A list of tagged pointers to the entities declared by this statement.
                         an_il_entity_list_entry_ptr entities = p->variant.decl.entities;
                      // DQ (10/16/2012): List the different kinds of entities declared in this statement.
                         if (entities != NULL)
                            {
                           // DQ (10/29/2012): We might want to skip over these entries if we ever visit them later.
#if DEBUG_STMT_DECL
                              printf ("WARNING: In parse_statement(): case stmk_decl: there are entities declared as part of this statement \n");
#endif
                              while (entities != NULL)
                                 {
                                   a_tagged_pointer entity = entities->entity;
#if DEBUG_STMT_DECL
                                   printf ("   --- entities = %p \n",entities);
                                // printf ("      --- entity      = %p \n",entity);
                                   printf ("      --- entity.kind = %d = %s \n",entity.kind,summarize_entry_kind_in_string_kind((an_il_entry_kind)(entity.kind)));
                                   printf ("      --- entity.ptr  = %p \n",entity.ptr);
#endif

                                // DQ (5/14/2015): This is more general handling of the different kinds of entities can can be see here.
                                // Previously on the case of iek_type was supported, now we need to add iek_variable (see test2015_144.c).
                                // All other cases were previously ignored, we might want to still support that default behavior, but now 
                                // we can issue better warning/error messages in the future.
                                   switch (entity.kind)
                                      {
                                     // DQ (5/14/2015): This was the previously existing case (the only one supported).
                                        case iek_type:
                                           {
                                          // In the cases where types are declared in stmt_decl's then we want to build the type so that it can be 
                                          // referenced in the statement.  An example is: int x = (((union ABC { int __in; int __i; }) { .__in = 42 }).__i);
                                          // where the union type will be visited after the variable declaration for "x".  But to build the cast in 
                                          // the SgAssignmentInitializer we need to have the type available.  I would like to avoid building a reference 
                                          // to the type before it exists (e.g. typedef).
#if DEBUG_STMT_DECL
                                             printf ("In parse_statement(): case stmk_decl: Found a type declared in this stmt_decl entity.ptr  = %p \n",entity.ptr);
#endif
                                             a_type_ptr edg_type = (a_type_ptr)(entity.ptr);
                                             ROSE_ASSERT(edg_type != NULL);

                                             bool pointerToMember = false;
#if DEBUG_STMT_DECL
                                             printf ("In parse_statement(): case stmk_decl: calling convert_type() in this stmt_decl = %p for entity.ptr  = %p \n",p,entity.ptr);
#endif
                                             SgType* rose_type = convert_type(edg_type,pointerToMember);
                                             ROSE_ASSERT(rose_type != NULL);
#if DEBUG_STMT_DECL
                                             printf ("In parse_statement(): case stmk_decl: rose_type = %p = %s \n",rose_type,rose_type->class_name().c_str());
#endif
#if DEBUG_STMT_DECL
                                             printf ("DONE: In parse_statement(): case stmk_decl: calling convert_type() in this stmt_decl = %p for entity.ptr  = %p \n",p,entity.ptr);
#endif
#if DEBUG_STMT_DECL
                                             SgClassType* classType = isSgClassType(rose_type);
                                             if (classType != NULL)
                                                {
                                                  SgDeclarationStatement* declaration = classType->get_declaration();
                                                  printf ("In parse_statement(): case stmk_decl: declaration = %p declaration->get_parent()     = %p \n",declaration,declaration->get_parent());
                                                  if (declaration->get_parent() != NULL)
                                                     {
                                                       printf ("   --- declaration->get_parent()                      = %s \n",declaration->get_parent()->class_name().c_str());
                                                     }
                                                  printf ("   --- declaration->get_definingDeclaration()         = %p \n",declaration->get_definingDeclaration());
                                                  printf ("   --- declaration->get_firstNondefiningDeclaration() = %p \n",declaration->get_firstNondefiningDeclaration());
                                                }
#endif
                                             break;
                                           }

                                     // DQ (5/14/2015): This is the new case that has cause a change the design of the support for different kinds.
                                        case iek_variable:
                                           {
#if 0
                                             printf ("Adding support for variable declaration in the entity list in a stmk_decl \n");
#endif
#if 0
                                             ROSE_ASSERT(sse->entity.ptr != NULL);
                                             a_variable_ptr variable = (a_variable_ptr)(sse->entity.ptr);
                                             ROSE_ASSERT(variable != NULL);

                                             printf ("Adding support for variable declaration in the entity list in a stmk_decl: variable = %p \n",variable);
#endif
#if 0
                                             ROSE_ASSERT(sse->entity.ptr != NULL);
                                             parse_statement_function_source_sequence_entry = ((a_variable_ptr)(sse->entity.ptr))->source_corresp.source_sequence_entry;
                                             ROSE_ASSERT(parse_statement_function_source_sequence_entry != NULL);

                                             a_variable_ptr edg_variable = (a_variable_ptr)(sse->entity.ptr);
                                             ROSE_ASSERT(edg_variable != NULL);
#endif
#if 0
                                             printf ("########################################################### \n");
                                             printf ("########################################################### \n");
#endif
                                          // SgBasicBlock* existingBasicBlock = NULL;
                                          // vector<SgStatement*> statement_list;
                                          // vector<SgStatement*> statement_list = parse_statement(sse,existingBasicBlock);
                                          // Only the sse is used, but the last parameter should be the EDG block that will be 
                                          // match the end-of-construct found at the end of the list of statements in the block.
                                          // vector<SgStatement*> statement_list = parse_statement_list(sse, iek_statement, p);
                                          // vector<SgStatement*> statement_list = parse_statement(sse,existingBasicBlock);
#if 0
                                             printf ("In parse_statement(): case stmk_decl: case iek_variable: statement_list.size() = %zu \n",statement_list.size());
#endif
#if 0
                                             printf ("########################################################### \n");
                                             printf ("########################################################### \n");
#endif
#if 0
                                          // ROSE_ASSERT(statement_list.size() == 1);
                                             if (statement_list.size() >= 1)
                                                {
                                                  printf ("In parse_statement(): case stmk_decl: case iek_variable: basicBlock->get_statements()[0] = %p = %s \n",statement_list[0],statement_list[0]->class_name().c_str());
                                                }
#endif
#if 0
                                             printf ("Exiting as a test! \n");
                                             ROSE_ASSERT(false);
#endif
                                             break;
                                           }

                                     // DQ (5/10/2018): These are the remaining unsupported kinds.
                                        case iek_routine:
                                          {
                                         // Note: that implicit functions seen here are handled elsewhere in the EDG/ROSE translation.
                                         // This case is demonstrated by test2009_03.c (which might be a case of an implicit function, but I think not exactly).
                                         // Also demonstrated in test2005_49.C, test2005_88.C, test2006_78.C, and others (all C++ cases).
#if 0
                                             printf ("Warning: In parse_statement(): case stmk_decl: case iek_routine: Note that we are not handling this node in the case stmk_decl entity = %p kind = %s \n",
                                                  entities,summarize_entry_kind_in_string_kind(entities->entity).c_str());
#endif
#if 0
                                             printf ("Exiting as a test! \n");
                                             ROSE_ASSERT(false);
#endif
                                            break;
                                          }

                                     // DQ (5/14/2015): These are the remaining unsupported kinds.
                                        case iek_lambda:
                                        case iek_lambda_capture:

                                     // DQ (5/14/2015): This may not be a complete list.
                                        case iek_none:
                                        case iek_source_file:
                                        case iek_constant:
                                        case iek_param_type:
                                        case iek_routine_type_supplement:
                                        case iek_based_type_list_member:
                                        case iek_field:
                                        case iek_exception_specification:
                                        case iek_exception_specification_type:
                                        case iek_label:
                                        case iek_expr_node:
                                        case iek_for_loop:
                                     // case iek_switch_clause:
                                        case iek_handler:
                                        case iek_try_supplement:
                                        case iek_block:
                                        case iek_statement:
                                        case iek_object_lifetime:
                                        case iek_scope:
                                        case iek_id_name:
                                        case iek_string_text:
                                        case iek_other_text:
                                        case iek_internal_complex_value:
                                        case iek_namespace:
                                        case iek_using_decl:
                                        case iek_dynamic_init:
                                        case iek_local_static_variable_init:
                                        case iek_vla_dimension:
                                        case iek_overriding_virtual_function:
                                        case iek_derivation_step:
                                        case iek_base_class_derivation:
                                        case iek_base_class:
                                        case iek_class_list_entry:
                                        case iek_routine_list_entry:
                                        case iek_class_type_supplement:
                                        case iek_template_param_type_supplement:
                                        case iek_constructor_init:
                                        case iek_asm_entry:
                                        case iek_template_arg:
                                        case iek_new_delete_supplement:
                                        case iek_throw_supplement:
                                        case iek_condition_supplement:
                                        case iek_source_sequence_entry:
                                        case iek_src_seq_secondary_decl:
                                        case iek_src_seq_end_of_construct:
                                        case iek_src_seq_sublist:
                                        case iek_instantiation_directive:
                                        case iek_scope_orphaned_list_header:
#if BACK_END_IS_CP_GEN_BE
                                     // This is only defined for the CP backend.
                                        case iek_hidden_name:
#endif
                                        case iek_pragma:
                                        case iek_template:
                                        case iek_decl_position_supplement:
                                        case iek_switch_case_entry:
                                        case iek_template_decl:
                                        case iek_template_parameter:
                                        case iek_name_reference:
                                        case iek_name_qualifier:
                                        case iek_static_assertion:
                                        case iek_last:
                                           {
                                             mprintf ("Warning: In parse_statement(): case stmk_decl: Note that we are not handling this node in the case stmk_decl entity = %p kind = %s \n",
                                                  entities,summarize_entry_kind_in_string_kind(entities->entity).c_str());
                                          // ROSE_ASSERT(false);
                                             break;
                                           }

                                        default:
                                           {
                                          // DQ (5/14/2015): It might be that this should be a warning.
                                             printf ("Error: In parse_statement(): case stmk_decl: Note that we don't have this kind in our list of cases in the case stmk_decl entity = %p kind = %s \n",
                                                  entities,summarize_entry_kind_in_string_kind(entities->entity).c_str());
                                             ROSE_ASSERT(false);
                                             break;
                                           }
                                      }
                                // Increment through the linked list of entities.
                                   entities = entities->next;
                                 }
                            }
#if DEBUG_STMT_DECL
                         printf ("Leaving parse_statement(): case stmk_decl: where entities that may have been processed as part of this statement \n");
#endif
                         break;
                       }

                    case stmk_set_vla_size: ROSE_ASSERT (!"stmk_set_vla_size not supported"); abort();
                    case stmk_vla_decl:     ROSE_ASSERT (!"stmk_vla_decl not supported");     abort();

#if UPC_EXTENSIONS_ALLOWED
                    case stmk_upc_notify:
                       {
#ifdef DEBUG_CONVERSION
                         indent_line(); debug_print("UPC notify statement "); print_statement_position(p); debug_print("\n");
#endif
                         result.push_back(set_file_info_for_stmt(buildUpcNotifyStatement_nfi(convert_expression(p->expr)), p)); // FIXME: build function
                         break;
                       }

                    case stmk_upc_wait:
                       {
#ifdef DEBUG_CONVERSION
                         indent_line(); debug_print("UPC wait statement "); print_statement_position(p); debug_print("\n");
#endif
                         result.push_back(set_file_info_for_stmt(buildUpcWaitStatement_nfi(convert_expression(p->expr)), p)); // FIXME: build function
                         break;
                       }

                    case stmk_upc_barrier:
                       {
#ifdef DEBUG_CONVERSION
                         indent_line(); debug_print("UPC barrier statement "); print_statement_position(p); debug_print("\n");
#endif
                         result.push_back(set_file_info_for_stmt(buildUpcBarrierStatement_nfi(convert_expression(p->expr)), p)); // FIXME: build function

                         break;
                       }

                    case stmk_upc_fence:
                       {
#ifdef DEBUG_CONVERSION
                         indent_line(); debug_print("UPC fence statement "); print_statement_position(p); debug_print("\n");
#endif
                         result.push_back(set_file_info_for_stmt(buildUpcFenceStatement_nfi(), p)); // FIXME: build function
                         break;
                       }

                    case stmk_upc_forall:
                       {
#ifdef DEBUG_CONVERSION
                         indent_line(); debug_print("UPC forall statement\n");
                         ScopedIndent si(indent);
#endif
                         a_for_loop_ptr for_loop = p->variant.for_loop.extra_info;
#ifdef DEBUG_CONVERSION
                         indent_line(); debug_print("+ initializer\n");
#endif
                         SgForInitStatement* init = for_loop->initialization == NULL ? buildForInitStatement(buildNullStatement_nfi()) : parse_for_initializer(sse);

#ifdef DEBUG_CONVERSION
                         indent_line(); debug_print("+ test\n");
#endif
                         SgStatement* test = p->expr == NULL ? set_generated_file_info_to_unparse(buildNullStatement_nfi()) : convert_condition(p->expr,sse);
#ifdef DEBUG_CONVERSION
                         indent_line(); debug_print("+ increment\n");
#endif
                         SgExpression* incr = for_loop->increment == NULL ? set_generated_file_info_to_unparse(buildNullExpression_nfi()) : convert_expression(for_loop->increment);
#ifdef DEBUG_CONVERSION
                         indent_line(); debug_print("+ affinity\n");
#endif
                         SgExpression* affinity = for_loop->affinity == NULL ? set_generated_file_info_to_unparse(buildNullExpression_nfi()) : convert_expression(for_loop->affinity);
#ifdef DEBUG_CONVERSION
                         indent_line(); debug_print("+ body\n");
#endif
                         SgStatement* body = build_basic_block_if_multiple_statements(parse_statement(sse));
                         SgUpcForAllStatement* fs = buildUpcForAllStatement_nfi(init, test, incr, affinity, body);
                         set_file_info_for_stmt(fs, p);

                      // DQ (10/12/2012): Added assertion.
                         ROSE_ASSERT(fs->get_for_init_stmt() != NULL);

                         set_generated_file_info_to_unparse(fs->get_for_init_stmt());
                         result.push_back(fs);
                         process_static_local_variable_initializers(p->variant.for_loop.extra_info->for_init_scope);
                         break;
                       }
#endif

#if GNU_EXTENSIONS_ALLOWED
                 // DQ (11/21/2017): An example of this is in test2017_68.C.
                 // DQ (1/23/2004): Discovered one of these in the mono C application code.
                 // Some C compilers (e.g., gcc) support goto with a label variable using the label value operator. The label value 
                 // operator && returns the address of its operand, which must be a label defined in the current function or a 
                 // containing function. The value is a constant of type void * and should be used only in a computed goto statement. 
                 // The feature is an extension to C and C++, implemented to facilitate porting programs developed with GNU C.[19] 
                 // This C extension is sometimes referred to as a Computed goto, but it has the semantics of an assigned goto.

                    case stmk_assigned_goto: 
                       {
#ifdef DEBUG_CONVERSION
                         indent_line(); debug_print("GNU extension: assigned goto statement\n");
                         ScopedIndent si(indent);
#endif
                         ROSE_ASSERT(p->expr != NULL);

                         SgExpression* selector_expression = convert_expression(p->expr);

                         ROSE_ASSERT(selector_expression != NULL);
#if 0
                         printf ("case stmk_assigned_goto: selector_expression = %p = %s \n",selector_expression,selector_expression->class_name().c_str());
#endif
                         SgStatement* statement = buildGotoStatement_nfi(selector_expression);
                         ROSE_ASSERT(statement != NULL);
                         selector_expression->set_parent(statement);

                      // We need to build a goto statement that takes an variable as a parameter to support the computed goto, I think.
                      // Or maybe just a SgLableRefExpr.  But more than just a SgLabelStatement.

                         result.push_back(statement);
                         break;
                       }
#endif

                    case stmk_empty:
                       {
#ifdef DEBUG_CONVERSION
                         indent_line(); debug_print("null statement\n");
#endif
                      // This does appear to always compute a correct source position into the ROSE AST.
                         result.push_back(set_file_info_for_stmt(buildExprStatement_nfi(set_generated_file_info_to_unparse(buildNullExpression_nfi())), p));
                         break;
                       }

#if GNU_EXTENSIONS_ALLOWED
                 // DQ (10/6/2016): Adding support for new EDG 4.11 specfic statement kind.
                    case stmk_stmt_expr_result:
                       {
#ifdef DEBUG_CONVERSION
                         indent_line(); debug_print("stmk_stmt_expr_result statement\n");
#endif
                      // A statement in a GNU statement expression producing the result 
                      // value of that expression.  Always the last statement of its block.

                      // For a GNU statement expression that produces a value by calling a copy constructor, this points to a
                      // dynamic initialization entry that initializes the result value (dik_class_result_via_ctor or
                      // dik_constructor).  NULL otherwise.  This is non-NULL (C++ only) when expr is NULL, and vice versa.
                         a_dynamic_init_ptr edg_initializer = p->variant.stmt_expr_result.dynamic_init;
                      // ROSE_ASSERT(edg_initializer != NULL);

                         if (edg_initializer != NULL)
                            {
                              SgExpression* expression = convert_dynamic_initializer(edg_initializer,NULL);
                              ROSE_ASSERT(expression != NULL);
                            }
                           else
                            {
                           // DQ (10/6/2016): Need to review if this is the correct behavior.
                           // We need to access the expression directly!
#if 0
                              printf ("When edg_initializer == NULL, build an expression using the expr data member (will be non-NULL) \n");
#endif
                              ROSE_ASSERT(p->expr != NULL);
                              SgExpression* expression = convert_expression(p->expr);
                              ROSE_ASSERT(expression != NULL);
#if 0
                              printf ("In parse_statement(): case stmk_stmt_expr_result: expression = %p = %s \n",expression,expression->class_name().c_str());
#endif

                              SgExprStatement* expressionStatement = set_file_info_for_stmt(buildExprStatement_nfi(set_generated_file_info_to_unparse(expression)), p);
                              ROSE_ASSERT(expressionStatement != NULL);

                              result.push_back(expressionStatement);
#if 0
                              printf ("In parse_statement(): case stmk_stmt_expr_result: result.size() = %zu \n",result.size());
                              printf ("In parse_statement(): case stmk_stmt_expr_result: BUILD THIS TO BE A GNU STATEMENT-EXPRESSION \n");
#endif
                            }

                         break;
                       }
#endif /* GNU_EXTENSIONS_ALLOWED */

                 // DQ (7/7/2020): This EDG IR node only appears when using EDG 6.0 (which makes the case statements source sequence points).
                 // DQ (10/24/2012): I have never seen this kind of statement (though in implementing the switch statement support I have been looking for it).
                    case stmk_switch_case:
                       {
                      // DQ (11/24/2017): Note that this is handled within the iteration of statements in a block.
                      // This is more complex because the stmk_switch_case is not a source sequence point (and we 
                      // fundamentally iterate over the source sequence points).
                         ROSE_ASSERT(p->variant.switch_case.switch_statement != NULL);
                         ROSE_ASSERT(p->variant.switch_case.extra_info != NULL);

                         a_statement_ptr edg_switch_statement = p->variant.switch_case.switch_statement;
                         ROSE_ASSERT(edg_switch_statement != NULL);

                         a_switch_case_entry_ptr edg_switch_case_info = p->variant.switch_case.extra_info;
                         ROSE_ASSERT(edg_switch_case_info != NULL);

                         SgStatement* case_statement = convert_switch_case(edg_switch_case_info);
                         ROSE_ASSERT(case_statement != NULL);
#if 0
                         printf ("case_statement = %p = %s \n",case_statement,case_statement->class_name().c_str());
#endif
                         switch_case_map.insert(pair<a_switch_case_entry_ptr,SgStatement*>(edg_switch_case_info,case_statement));
                         break;
                       }

#if MICROSOFT_EXTENSIONS_ALLOWED
                    case stmk_microsoft_try:
                       {
                      // DQ (11/24/2017): This needs to be isolated out to avoid fall-through to the next case.
                         printf ("Found a stmk_microsoft_try (not implemented, not required in this location) \n");
                         ROSE_ASSERT(false);

                         break;
                       }
#endif

                 // DQ (1/27/2017): Added support for more C++11 constructs.
                    case stmk_range_based_for:
                       {
#if 0
                         printf ("In parse_statement(): case stmk_range_based_for: calling summarize_statement(): p = %p \n",p);
                         summarize_statement(p);
#endif

                      // Pointer to the statement that is the body of the loop (commonly but not necessarily an stmk_block)
                      // that is to be executed on each iteration of the loop; NULL if there is none. 
                      // a_statement_ptr statement = p->variant.range_based_for_loop.statement;
#if 0
                      // SgStatement* for_loop_body = convert_statement(statement);
                         SgStatement* body = build_basic_block_if_multiple_statements(parse_statement(sse));
                         ROSE_ASSERT(body != NULL);
#if 0
                         printf ("In parse_statement(): case stmk_range_based_for: body = %p = %s \n",body,body->class_name().c_str());
#endif
#endif
                      // Information about the loop control constructs used in the range-based-for.  The expr field in the statement is unused.
                         a_range_based_for_loop_ptr extra_info = p->variant.range_based_for_loop.extra_info;
                         ROSE_ASSERT(extra_info != NULL);
#if 0
                         printf ("In In parse_statement(): case stmk_range_based_for: calling summarize_range_based_for_loop(): extra_info = %p \n",extra_info);
                         summarize_range_based_for_loop (extra_info);
#endif

                      // Build the internal support for the rage based for loop

                      // Pointer to the iteration variable declared in for-range-declaration.  Dynamically initialized
                      // to *__begin.  NULL if there was an error.
                         a_variable_ptr iterator = extra_info->iterator;

                      // Pointer to the __range temporary variable above. Initialized to (expression). 
                         a_variable_ptr range = extra_info->range;

                      // DQ (5/8/2018): Node that this variable is not used (commented out to avoid compiler warning).
                      // An sck_block scope added to surround the range-based-for statement.  It corresponds to the
                      // outermost set of braces shown above, and the __range variable is declared in this scope.
                      // The expression is also evaluated in this scope. 
                      // a_scope_ptr   range_based_for_scope = extra_info->range_based_for_scope;

                      // DQ (5/8/2018): Node that this variable is not used (commented out to avoid compiler warning).
                      // An sck_block scope added to surround the __begin and __end variables.
                      // a_scope_ptr   begin_end_scope = extra_info->begin_end_scope;

                      // DQ (5/8/2018): Node that this variable is not used (commented out to avoid compiler warning).
                      // An sck_block scope added immediately inside the loop, in which the iterator variable is declared.
                      // The dependent statement of the loop is enclosed by this scope.
                      // a_scope_ptr   iterator_scope = extra_info->iterator_scope;

                      // Pointer to the variable representing the temporary variable __begin above.  Dynamically initialized to
                      // one of: __range, __range.begin(), or begin(__range) as appropriate.  NULL if there was an error.
                         a_variable_ptr begin = extra_info->begin;

                      // Pointer to the variable representing the temporary variable __end above.  Dynamically initialized to
                      // one of: __range + __bound, __range.end(), or end(__range) as appropriate.  NULL if there was an error.
                         a_variable_ptr end = extra_info->end;

                      // Expression for the "__begin != __end" test.
                         an_expr_node_ptr ne_call_expr = extra_info->ne_call_expr;

                      // Expression for the "++__begin" increment.
                         an_expr_node_ptr incr_call_expr = extra_info->incr_call_expr;

#if COROUTINES_ALLOWED
                      // TRUE in the case of a "for await (...)" stement.
                         bool use_await = extra_info->use_await;
#endif /* COROUTINES_ALLOWED */

#define DEBUG_RANGE_BASED_FOR_STATEMENT 0

#if 0
                         printf ("#################################################################################################### \n");
                         printf ("Evaluating the iterator declaration (make sure we build the dynamic initializer for the iterator variable) \n");
#endif
                         ROSE_ASSERT(iterator != NULL);

                      // DQ (5/8/2018): Node that this variable is not used (commented out to avoid compiler warning).
                      // a_template_ptr edg_template = NULL;

                         SgRangeBasedForStatement* rangeBasedForStatement = SageBuilder::buildRangeBasedForStatement_nfi(NULL,NULL,NULL,NULL,NULL,NULL,NULL);

                         SgScopeStatement* scopeOfRangeBasedForStatement = topScopeStack();
                         rangeBasedForStatement->set_parent(scopeOfRangeBasedForStatement);

                      // DQ (6/26/2019): Push the new scope onto the stack.
                         pushScopeStack(rangeBasedForStatement);

                      // SgVariableDeclaration* iterator_declaration = convert_variable(iterator);
                      // SgVariableDeclaration* iterator_declaration = convert_variable_prototype(iterator, edg_template);
                         SgVariableDeclaration* iterator_declaration = parse_variable_declaration(sse);
                         ROSE_ASSERT(iterator_declaration != NULL);

                         iterator_declaration->set_parent(rangeBasedForStatement);
#if 0
                      // This will have a name such as: index1 (from the source code).
                         SgInitializedNamePtrList & iterator_variableList = iterator_declaration->get_variables();
                         for (size_t i = 0; i < iterator_variableList.size(); i++)
                            {
                              printf ("iterator_variableList[%zu] = %p name = %s \n",i,iterator_variableList[i],iterator_variableList[i]->get_name().str());
                            }
#endif

                      // DQ (3/27/2018): Note that the range declaration is not available in the source sequence list (compiler generated).
                      // So we need to handle it with a different level of support.  Ultimately this might mean refactoring some of the
                      // code in the parse_variable_declaration() function which takes the source sequence entry, so that we can avoid
                      // duplicating the code to setup the variable declarations for the compiler generated variables supporting the 
                      // range-based for loop.
#if 0
                         printf ("#################################################################################################### \n");
                         printf ("Evaluating the range declaration (make sure we build the dynamic initializer for the range variable) \n");
#endif
                         ROSE_ASSERT(range != NULL);

                      // DQ (3/27/2018): This function takes a a_variable_ptr instead of using the source sequence list (sse), 
                      // because these are the first variables that I have seen that are not a part of the source sequence list.
                         SgVariableDeclaration* range_declaration = convert_variable_declaration(range);

                      // SgVariableDeclaration* range_declaration = convert_variable_prototype(range, edg_template);
                      // SgVariableDeclaration* range_declaration = parse_variable_declaration(sse);
                      // SgVariableSymbol* range_variable_symbol = convert_variable_use(range);
                      // range_reference_exp = buildVarRefExp_nfi(range_variable_symbol);
                         ROSE_ASSERT(range_declaration != NULL);

                         range_declaration->set_parent(rangeBasedForStatement);

#if 0
                      // This will have a name such as: __anonymous_0xe06380
                         SgInitializedNamePtrList & range_variableList = range_declaration->get_variables();
                         for (size_t i = 0; i < range_variableList.size(); i++)
                            {
                              printf ("range_variableList[%zu] = %p name = %s \n",i,range_variableList[i],range_variableList[i]->get_name().str());
                            }
#endif
#if 0
                         printf ("Exiting as a test! (make sure we build the dynamic initializer for the range variable) \n");
                         ROSE_ASSERT(false);
#endif
                      // ROSE_ASSERT(begin != NULL);
                         SgVariableDeclaration* begin_declaration = NULL;
                         if (begin != NULL)
                            {
                           // DQ (3/27/2018): This function takes a a_variable_ptr instead of using the source sequence list (sse), 
                           // because these are the first variables that I have seen that are not a part of the source sequence list.
                           // SgVariableDeclaration* begin_declaration = convert_variable_prototype(begin, edg_template);
                              begin_declaration = convert_variable_declaration(begin);
                              ROSE_ASSERT(begin_declaration != NULL);

                              begin_declaration->set_parent(rangeBasedForStatement);
                           }
#if 0
                      // This will have a name such as: __anonymous_0xe06380
                         SgInitializedNamePtrList & begin_variableList = begin_declaration->get_variables();
                         for (size_t i = 0; i < begin_variableList.size(); i++)
                            {
                              printf ("begin_variableList[%zu] = %p name = %s \n",i,begin_variableList[i],begin_variableList[i]->get_name().str());
                            }
#endif
                      // ROSE_ASSERT(end != NULL);
                         SgVariableDeclaration* end_declaration = NULL;
                         if (end != NULL)
                            {
                           // SgVariableDeclaration* end_declaration = convert_variable_prototype(end, edg_template);
                              end_declaration = convert_variable_declaration(end);
                              ROSE_ASSERT(end_declaration != NULL);

                              end_declaration->set_parent(rangeBasedForStatement);
                            }
#if 0
                      // This will have a name such as: __anonymous_0xe06380
                         SgInitializedNamePtrList & end_variableList = end_declaration->get_variables();
                         for (size_t i = 0; i < end_variableList.size(); i++)
                            {
                              printf ("end_variableList[%zu] = %p name = %s \n",i,end_variableList[i],end_variableList[i]->get_name().str());
                            }
#endif
                      // ROSE_ASSERT(ne_call_expr != NULL);
                         SgExpression* not_equal_call_expression = NULL;
                         if (ne_call_expr != NULL)
                            {
                              not_equal_call_expression = convert_expression(ne_call_expr);
                              ROSE_ASSERT(not_equal_call_expression != NULL);

                              not_equal_call_expression->set_parent(rangeBasedForStatement);
#if 0
                              printf ("not_equal_call_expression = %p = %s \n",not_equal_call_expression,not_equal_call_expression->class_name().c_str());
#endif
                            }

                      // ROSE_ASSERT(incr_call_expr != NULL);
                         SgExpression* increment_call_expression = NULL;
                         if (incr_call_expr != NULL)
                            {
                              increment_call_expression = convert_expression(incr_call_expr);
                              ROSE_ASSERT(increment_call_expression != NULL);

                              increment_call_expression->set_parent(rangeBasedForStatement);
#if 0
                              printf ("increment_call_expression = %p = %s \n",increment_call_expression,increment_call_expression->class_name().c_str());
#endif
                            }
#if 1
                         SgStatement* body = build_basic_block_if_multiple_statements(parse_statement(sse));
                         ROSE_ASSERT(body != NULL);

                         body->set_parent(rangeBasedForStatement);

#if 0
                         printf ("In parse_statement(): case stmk_range_based_for: body = %p = %s \n",body,body->class_name().c_str());
#endif
#endif
#if 0
                         SgRangeBasedForStatement* rangeBasedForStatement = 
                           SageBuilder::buildRangeBasedForStatement_nfi(iterator_declaration,range_declaration,
                                begin_declaration,end_declaration,not_equal_call_expression,increment_call_expression,body);
#else
                      // DQ (6/26/2019): This change is required to support building the SgRangeBasedForStatement scope before building the children.
                         rangeBasedForStatement->set_iterator_declaration(iterator_declaration);
                         rangeBasedForStatement->set_range_declaration(range_declaration);
                         rangeBasedForStatement->set_begin_declaration(begin_declaration);
                         rangeBasedForStatement->set_end_declaration(end_declaration);
                         rangeBasedForStatement->set_not_equal_expression(not_equal_call_expression);
                         rangeBasedForStatement->set_increment_expression(increment_call_expression);
                         rangeBasedForStatement->set_loop_body(body);
#endif
                         ROSE_ASSERT(rangeBasedForStatement != NULL);
#if 0
                         printf ("Processing the scope assertions for after call to SageBuilder::buildRangeBasedForStatement_nfi() \n");
#endif
                      // DQ (6/26/2019): Check the scope of all of the inputs to the SgRangeBasedForStatement.
                         ROSE_ASSERT(body->get_scope()                      == rangeBasedForStatement);

                      // ROSE_ASSERT(end_declaration != NULL);
                         if (end_declaration != NULL)
                            {
                              ROSE_ASSERT(end_declaration->get_scope()           == rangeBasedForStatement);
                            }

                         if (begin_declaration != NULL)
                            {
                              ROSE_ASSERT(begin_declaration->get_scope()         == rangeBasedForStatement);
                            }

                         if (range_declaration != NULL)
                            {
                              ROSE_ASSERT(range_declaration->get_scope()         == rangeBasedForStatement);
                            }

                         if (iterator_declaration != NULL)
                            {
                              ROSE_ASSERT(iterator_declaration->get_scope()      == rangeBasedForStatement);
                            }
#if 0
                         printf ("DONE: Processing the scope assertions for after call to SageBuilder::buildRangeBasedForStatement_nfi() \n");
#endif
                         result.push_back(set_file_info_for_stmt(rangeBasedForStatement, p));

                      // DQ (6/26/2019): Pop the new scope from the stack.
                         popScopeStack();
#if 0
                         printf ("In parse_statement(): case stmk_range_based_for not handled! \n");
                         ROSE_ASSERT(false);
#endif
                         break;
                       }

                    case stmk_last:
                       {
                         printf ("Case not handled! \n");
                         ROSE_ASSERT(false);
                         break;
                       }

                    default: 
                       {
                         printf ("ERROR: parse_statement(): unknown case of iek_statement kinds: p = %p kind = %s \n",p,summarize_statement_in_string(p));
                         ERROR_MESSAGE(stderr, "Bad statement kind %d\n", kind); ROSE_ASSERT(false);
                       }
                  }
#if 0
             printf ("Case of iek_statement: result.size() = %" PRIuPTR " \n",result.size());
#endif
            // DQ (7/9/2012): This is a copy of the test below...
            // DQ (7/9/2012): Added test for parent.  This should be the first location in the call stack where the
            // parent could be set. It should be FALSE by default and set here where it is structurally defined, and 
            // not set where it is dependent on name qualification. Test all of the statements in the STL container.
            // ROSE_ASSERT(result.empty() == false);
               vector<SgStatement*>::iterator result_iterator = result.begin();
               while (result_iterator != result.end())
                  {
#if 0
                    printf ("Case of iek_statement: result.size() = %" PRIuPTR " (*result_iterator) = %p = %s (*result_iterator)->get_parent() = %p \n",result.size(),(*result_iterator),(*result_iterator)->class_name().c_str(),(*result_iterator)->get_parent());
                    if ((*result_iterator)->get_parent() == NULL)
                       {
                         printf ("Case of iek_statement: result.size() = %" PRIuPTR " (*result_iterator) = %p = %s \n",result.size(),(*result_iterator),(*result_iterator)->class_name().c_str());
                         printf ("Case of iek_statement: ----- case reached in parse_statement(): kind = %s \n",summarize_entry_kind_in_string_kind(outer_kind));
                         printf ("Case of iek_statement: ----- statement kind = %s \n",summarize_statement_in_string(p));
                       }
#endif
#if 1
                 // ROSE_ASSERT((*result_iterator)->get_parent() == NULL);
                 // ROSE_ASSERT((*result_iterator)->get_parent() != NULL);
#else
                    if ((*result_iterator)->hasExplicitScope() == false)
                       {
                      // DQ (7/9/2012): We can set the parent directly to the scope for the case of a statement which 
                      // has not explicit scope (e.g. ClassDeclarations can have an explicit scope; or even a structural 
                      // representation inside of a type reference).
                         ROSE_ASSERT((*result_iterator)->get_parent() != NULL);

                      // DQ (7/9/2012): But don't set the parent here (only uniformally below).
                      // (*result_iterator)->set_parent(topScopeStack());
                       }
#endif
                    result_iterator++;
                  }

#if 0
               printf ("switchSupportStackOfMaps.size() = %" PRIuPTR " result.size() = %" PRIuPTR " \n",switchSupportStackOfMaps.size(),result.size());
#endif
#if 0
            // DQ (2/9/2013): Moved to the base of the function now that we support a more general map based on a_source_sequence_entry_ptr instead of a_statement_ptr.
            // DQ (2/8/2013): Record all of the generated statements into the switch support map.
               if (switchSupportStackOfMaps.empty() == false)
                  {
                    vector<SgStatement*>::iterator result_iterator_single = result.begin();
                    if (result.size() == 1)
                       {
                         map<a_source_sequence_entry_ptr,SgStatement*>* switchSupportMap = switchSupportStackOfMaps.back();
                         ROSE_ASSERT(switchSupportMap != NULL);

                         map<a_source_sequence_entry_ptr,SgStatement*>::iterator mapEntry = switchSupportMap->find(p->source_sequence_entry);
                      // ROSE_ASSERT(mapEntry != switchSupportMap->end());
                         if (mapEntry != switchSupportStackOfMaps.back()->end())
                            {
                              ROSE_ASSERT(mapEntry->second == NULL);

                              ROSE_ASSERT((*result_iterator_single) != NULL);
                              mapEntry->second = *result_iterator_single;
                            }
                           else
                            {
                           // I think that only the Duff's Device examples can take this branch.
#if 0
                              printf ("Leaving parse_statment(): case iek_statement: add this edg statement to ROSE statement mapping: p->source_sequence_entry = %p *result_iterator_single = %p = %s \n",
                                   p->source_sequence_entry,*result_iterator_single,(*result_iterator_single)->class_name().c_str());
#endif
                              (*switchSupportMap)[p->source_sequence_entry] = *result_iterator_single;

                              printf ("Leaving parse_statment(): case iek_statement: switchSupportMap->size() = %" PRIuPTR " \n",switchSupportMap->size());
                            }
                       }
                      else
                       {
#ifdef DEBUG_CONVERSION
                         printf ("Not clear how to add groups of statements to the switch map support! \n");
#endif
                      // ROSE_ASSERT(false);
                       }
                  }
#endif
               break;
             }

          case iek_variable:
             {
#if 0
               printf ("In parse_statement(): case iek_variable: processing entity.ptr = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
            // a_variable_ptr p = (a_variable_ptr)(sse->entity.ptr);
            // ROSE_ASSERT(p != NULL);
            // parse_statement_function_source_sequence_entry = p->source_sequence_entry;

            // DQ (2/9/2013): Set the parse_statement_function_source_sequence_entry so that we can setup the switch support map if it is required (if we are in a switch).
               ROSE_ASSERT(sse->entity.ptr != NULL);
               parse_statement_function_source_sequence_entry = ((a_variable_ptr)(sse->entity.ptr))->source_corresp.source_sequence_entry;
               ROSE_ASSERT(parse_statement_function_source_sequence_entry != NULL);

               a_variable_ptr edg_variable = (a_variable_ptr)(sse->entity.ptr);
               ROSE_ASSERT(edg_variable != NULL);

               SgStatement* stmt = parse_variable_declaration(sse);
               if (stmt != NULL)
                  {
                    result.push_back(stmt);

                 // DQ (7/9/2012): Added test for parent (should be set uniformally below).
                 // ROSE_ASSERT(stmt->get_parent() == NULL);
                    ROSE_ASSERT(stmt->get_parent() != NULL);

                 // DQ (5/7/2013): I think we can assert this, debugging test2013_152.C.
#if 0
                    stmt->get_file_info()->display("In parse_statement(): case iek_variable: debug");
#endif
                    if (stmt->get_file_info()->isCompilerGenerated() == true)
                       {
                      // Fix this using source position inform from the type declaration.
#if 0
                         printf ("WARNING: In parse_statement(): case iek_variable: variable declaration marked as compiler generated. \n");
#endif
#if 0
                         printf ("In parse_statement(): case iek_variable: calling summarize_variable() \n");
                         summarize_variable(edg_variable);
#endif
                         if (edg_variable->source_corresp.name == NULL)
                            {
                           // Check if this is a special case of a unnamed variable of type union{}.
#if 0
                              printf ("This is a variable with no valid name! \n");
#endif
                              SgVariableDeclaration* variableDeclaration = isSgVariableDeclaration(stmt);
                              ROSE_ASSERT(variableDeclaration != NULL);
                              if (variableDeclaration->get_variableDeclarationContainsBaseTypeDefiningDeclaration() == true)
                                 {
                                // Get the source position from the type's defining declaration.
                                   SgInitializedName* firstInitializedName = getFirstInitializedName(variableDeclaration);
                                   ROSE_ASSERT(firstInitializedName != NULL);
                                   
                                   SgType* type = firstInitializedName->get_type();
#if 0
                                   printf ("In parse_statement(): case iek_variable: type = %p = %s \n",type,type->class_name().c_str());
#endif
                                   SgClassType* classType = isSgClassType(type);
                                   if (classType != NULL)
                                      {
                                        SgClassDeclaration* classDeclaration = isSgClassDeclaration(classType->get_declaration());
                                        if (classDeclaration != NULL && classDeclaration->get_class_type() == SgClassDeclaration::e_union)
                                           {
                                          // This is a special case where EDG does not have source position and yet we can fix it.
                                             *(variableDeclaration->get_startOfConstruct()) = *(classDeclaration->get_startOfConstruct());
                                             *(variableDeclaration->get_endOfConstruct())   = *(classDeclaration->get_endOfConstruct());
#if 0
                                             printf ("Exiting as a test! \n");
                                             ROSE_ASSERT(false);
#endif
                                           }
                                      }
                                 }
                            }

                      // DQ (11/14/2016): Not celar what to do with un-named variable using brance initializer syntax (need and example).
                         if (edg_variable->has_direct_braced_initializer == true)
                            {
                              printf ("In parse_statement(): case iek_variable: Need to mark as using C++11 braced initializer syntax \n");
                              ROSE_ASSERT(false);
                            }
                       }

                 // DQ (11/6/2014): Adding C++11 support to record the use of C++11 syntax (for the unparser).
                    if (edg_variable->has_direct_braced_initializer == true)
                       {
                         SgVariableDeclaration* variableDeclaration = isSgVariableDeclaration(stmt);
                         ROSE_ASSERT(variableDeclaration != NULL);

                         SgInitializedName* firstInitializedName = getFirstInitializedName(variableDeclaration);
                         ROSE_ASSERT(firstInitializedName != NULL);

                      // DQ (11/14/2016): This uses a new data member to record this and is not yet inplemented 
                      // in the unparser to output the brace syntax (but does output a semantically correct 
                      // initialization using the default initializer).
                         firstInitializedName->set_is_braced_initialized(true);
#if 0
                         printf ("In parse_statement(): case iek_variable: Need to mark as using C++11 braced initializer syntax \n");
                         ROSE_ASSERT(false);
#endif
                       }

                 // ROSE_ASSERT(stmt->get_file_info()->isCompilerGenerated() == false);
                  }
#if 0
               printf ("Leaving parse_statement(): case iek_variable: next sse entry = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
               break;
             }

          case iek_pragma:
             {
#ifdef DEBUG_CONVERSION
               indent_line(); debug_print("pragma statement\n");
#endif
            // DQ (2/9/2013): Set the parse_statement_function_source_sequence_entry so that we can setup the switch support map if it is required (if we are in a switch).
               ROSE_ASSERT(sse->entity.ptr != NULL);
               parse_statement_function_source_sequence_entry = ((a_pragma_ptr)(sse->entity.ptr))->source_sequence_entry;
               ROSE_ASSERT(parse_statement_function_source_sequence_entry != NULL);

               SgStatement* stmt = parse_pragma(sse);

            // DQ (11/25/2012): Added assertion, if this is true then I can simplify the code below
            // and we will not have any chance of conditional fall through to the "case iek_type".
               ROSE_ASSERT(stmt != NULL);

#if 0
               if (stmt != NULL)
                  {
                    result.push_back(stmt); break;
                  }
#else
            // DQ (11/26/2012): Simplified (better) implementation.
               result.push_back(stmt);

#if 0
               printf ("In parse_statement(): case iek_pragma: before processing pragmas: next sse entry = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif

            // DQ (11/26/2012): The pragma handling is sufficently complex to support in switch statements that in this case we will skip this.
               bool inSwitchStatement = inSwitchScope();
#if 0
               printf ("In parse_statement(): case iek_pragma: inSwitchStatement = %s topScopeStack() = %p = %s \n",inSwitchStatement ? "true" : "false",topScopeStack(),topScopeStack()->class_name().c_str());
#endif
               if (inSwitchStatement == false)
                  {
                 // Accumulate any additional pragmas AND the next non-pragma statement. Note that sse is presently pointing 
                 // to the next entry, so we can't include the first call to "parse_pragma(sse);" in this loop.
                    while (sse->entity.kind == iek_pragma)
                       {
                         SgStatement* stmt = parse_pragma(sse);
                         ROSE_ASSERT(stmt != NULL);

                         result.push_back(stmt);
                       }
#if 0
                    printf ("In parse_statement(): case iek_pragma: after processing pragmas: next sse entry = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
                 // DQ (11/26/2012): This is needed to permit test2004_15.C to pass.
                 // vector<SgStatement*> next_set_of_statements = parse_statement(sse);
                    vector<SgStatement*> next_set_of_statements;
                    if (sse->entity.kind != iek_src_seq_end_of_construct)
                       {
                         next_set_of_statements = parse_statement(sse);
                       }
#if 0
                    printf ("In parse_statement(): case iek_pragma: next_set_of_statements.size() = %" PRIuPTR " \n",next_set_of_statements.size());
#endif
                 // DQ (11/26/2012): This can't be asserted and an example is test2005_125.C (in C subset of C++ tests).
                 // DQ (11/26/2012): I think we can assert that this is always length == 1.
                 // ROSE_ASSERT(next_set_of_statements.size() == 1 || next_set_of_statements.empty());
                 // ROSE_ASSERT(next_set_of_statements.size() == 1);

                 // Insert the vector of next statements (typically a single statement) at the end of the result vector.
                    result.insert(result.end(),next_set_of_statements.begin(),next_set_of_statements.end());
                  }
                 else
                  {
#ifdef DEBUG_CONVERSION
                    printf ("WARNING: skipping new pragma handling in scope of switch statement \n");
#endif
                  }

#if 0
               printf ("In parse_statement(): case iek_pragma: (LEAVING) result.size() = %" PRIuPTR " \n",result.size());
#endif
            // DQ (11/26/2012): This can't be asserted and an example is test2005_125.C (in C subset of C++ tests).
            // ROSE_ASSERT(result.size() > 1);

               break;
#endif
             }

          case iek_type:
             {
#ifdef DEBUG_CONVERSION
               indent_line(); debug_print("type statement\n");
#endif
#if 0
               printf ("In parse_statement(): case iek_type: calling parse_type_declaration() \n");
#endif
               char* ptr             = (char*)(sse->entity.ptr);
#if 0
               an_il_entry_kind kind = (an_il_entry_kind)(sse->entity.kind);
#endif
#if 0
               printf ("      --- entity.kind = %d = %s \n",kind,summarize_entry_kind_in_string_kind(kind));
               printf ("      --- entity.ptr  = %p \n",ptr);
#endif

            // DQ (9/6/2013): I think we do need this since it will otherwise fail for test2013_61.C.
            // DQ (2/10/2013): I don't think this is used since the support for building the Switch Support Map is implemented in parse_type_declaration().
            // However, if we do set it then we only have to deal with the case of empty result vector which will provide a more uniform API.
            // DQ (2/10/2013): Set the parse_statement_function_source_sequence_entry so that we can setup the switch support map if it is required (if we are in a switch).
               ROSE_ASSERT(sse->entity.ptr != NULL);
               parse_statement_function_source_sequence_entry = ((a_type_ptr)(sse->entity.ptr))->source_corresp.source_sequence_entry;
               ROSE_ASSERT(parse_statement_function_source_sequence_entry != NULL);
#if 1
               a_type_ptr type = (a_type_ptr) ptr;
               if (type_translation_cache.find(type) != type_translation_cache.end())
                  {
                 // DQ (10/18/2012): This branch is critical to the processing of test2012_47.c (conditional with union cast of designated initializer).
#if 0
                    printf ("This type HAS been processed previously: type = %p \n",type);
                    printf ("But build the defining declaration so that it is available...but don't add it to the return vector or SgStatements... \n");
#endif
#if 0
                    printf ("In parse_statement(): case iek_type: calling summarize_type(type = %p) \n",type);
                    summarize_type(type);
#endif
                 // If this is an unnamed type then stmt will be be NULL (parse_type_declaration() returns NULL pointer for this case) (see test2012_50.c).
                 // SgStatement* stmt = parse_type_declaration(sse);
                 // ROSE_ASSERT(stmt != NULL);

                 // New version to try to fix test2012_233.C
                 // For test2012_233.C this (before this fix) failed when the class/struct/union was in the function body's scope.
                    SgStatement* stmt = parse_type_declaration(sse);

                 // DQ (10/19/2012): Control which version to use based on if it is compiler generated...one works for test2012_47.c 
                 // and fails for test2012_233.C and the other is the reverse.

                 // DQ (10/19/2012): There are a few rules about when to output the type declaration.
                 //    1) When this is an autonomous primary declaration
                 //    2) When this is a typedef (since these only have what we consider non-defining declarations; no defining declarations).
                 // Note: an enum type is handled by the autonomous_primary_tag_decl flag.
                    bool appendStatement = ( (type->autonomous_primary_tag_decl == true) || (type->kind == tk_typeref) );
#if 0
                    printf ("In parse_statement(): case iek_type: stmt = %p = %s appendStatement = %s \n",stmt,(stmt != NULL) ? stmt->class_name().c_str() : "null",appendStatement ? "true" : "false");
#endif
                    if ( (stmt != NULL) && (appendStatement == true) )
                       {
                         result.push_back(stmt);
                       }
                  }
                 else
                  {
#if 0
                    printf ("This type has NOT been processed previously \n");
#endif
#if 0
                    printf ("In parse_statement(): case iek_type: calling summarize_type(type = %p) \n",type);
                    summarize_type(type);
#endif
                    SgStatement* stmt = parse_type_declaration(sse);
#if 0
                    printf ("In parse_statement(): case iek_type: stmt = %p = %s \n",stmt,(stmt != NULL) ? stmt->class_name().c_str() : "null");
#endif
                    if (stmt != NULL)
                         result.push_back(stmt);
                  }
#else
            // DQ (10/18/2012): Previous code...
               SgStatement* stmt = parse_type_declaration(sse);

#error "DEAD CODE!"

               if (stmt != NULL)
                    result.push_back(stmt);
#endif
#if 0
               printf ("In parse_statement(): case iek_type: before leaving the case: associated entry to end_of_construct (after increment): entry = %p kind = %s \n",
                    sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
#if 0
               a_src_seq_end_of_construct_ptr src_seq_end_of_construct = (a_src_seq_end_of_construct_ptr)(sse->entity.ptr);
               printf ("In parse_statement(): case iek_type: before leaving the case: associated entry to end_of_construct (after increment): sse = %p = %s \n",
                    src_seq_end_of_construct->entity.ptr,summarize_entry_kind_in_string_kind(src_seq_end_of_construct->entity).c_str());
#endif
#ifdef DEBUG_CONVERSION
            // DQ (9/6/2013): Added code to detect where we might want to increment sse if we detect that we are at the end of the 
            // source sequence for this block. This is important to the handling of types defined in statement expressions where they
            // appear in conditionals and are presently causing extra empty blocks to be used as the body of the SgIfStmt, SgWhileStmt, etc.).
               if ((an_il_entry_kind)(sse->entity.kind) == iek_src_seq_end_of_construct)
                  {
                    printf ("In parse_statement(): case iek_type: Detected ((an_il_entry_kind)(sse->entity.kind) == iek_src_seq_end_of_construct) \n");
                  }
#endif
               break;
             }

          case iek_src_seq_secondary_decl:
             {
#if 0
               printf ("In parse_statement(): case iek_src_seq_secondary_decl: entry = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
            // DQ (10/29/2012): Note that if this is for an implicit function declarration we might not want it to be called.
               SgStatement* stmt = parse_secondary_declaration(sse);
               if (stmt != NULL)
                  {
                    result.push_back(stmt);
                  }
               break;
             }

          case iek_src_seq_end_of_construct:
             {
            // It is an error to reach this case (unexpected_end_of_construct() simply prints a message and asserts failure).
               printf ("ERROR: In parse_statement(): case iek_src_seq_end_of_construct: entry = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#if 0
            // DQ (8/17/2012): I think that the source position of the closing scope is contained in here (so we don't want to ignore these).
               if (sse->entity.kind == iek_src_seq_end_of_construct)
                  {
                    printf ("@@@@@@@@@@@@@@@@@ In parse_statement(): case iek_src_seq_end_of_construct: \n");
                  }
#endif

            // DQ (3/29/2013): This is an error message to communicate the most likely reason why this translation unit is failing.
               printf ("\n\n****************************************************************************************************************** \n");
               printf ("ERROR: This file is likely a C89 file that is being compiled using the default C99 mode (use the -rose:C89 option) \n");
               printf ("****************************************************************************************************************** \n\n\n");

               unexpected_end_of_construct(sse);

            // DQ (9/28/2022): This break was missing and was a compiler warning (added since it is certainly required).
               break;
             }
     

          case iek_macro:
             {
               SgDeclarationStatement* stmt = parse_macro(sse);

               if (stmt != NULL)
                    result.push_back(stmt);

            // case iek_macro: return parse_macro(sse);
            // vector<SgStatement*> stmts = parse_statement(sse);
            // result.insert(result.end(), stmts.begin(), stmts.end());
            // printf ("case iek_macro not implemented \n");
            // ROSE_ASSERT(false);
               break;
             }

       // DQ (3/11/2012): Added support for using declarations (see test2004_78.C).
          case iek_using_decl:
             {
#if 0
#ifdef DEBUG_CONVERSION
               indent_line(); debug_print("using declaration "); print_source_corresp(field_entry->source_corresp); debug_print("\n");
               ScopedIndent si(indent);
#endif
#endif
               SgDeclarationStatement* stmt = parse_using_declaration(sse);
               ROSE_ASSERT(stmt != NULL);
               result.push_back(stmt);

            // printf ("Case of iek_using_decl not yet supported! \n");
            // ROSE_ASSERT(false);
               break;
             }

       // DQ (10/30/2012): Added support for used in test2012_142.c, but never used previously.
       // These are filed members that have not bee added to the declaration and are visited 
       // in the source sequence list after the processing of the block.  As a result we can't just
       // add them to the current top of scope stack, and we have to carefully compute the scope 
       // (finding the correct class definition).  This is a very strange feature of the source
       // sequence lists, but we are compiling the most bizzare C code that I have ever seen.
          case iek_field:
             {
               a_field_ptr field_entry = (a_field_ptr)(sse->entity.ptr);
               ROSE_ASSERT(field_entry != NULL);

#ifdef DEBUG_CONVERSION
               indent_line(); debug_print("field declaration "); print_source_corresp(field_entry->source_corresp); debug_print("\n");
               ScopedIndent si(indent);
#endif
#if 1
            // DQ (1/20/2014): This was added so that it would be more uniformally set (noticed while debugging compilation of valgrind using ROSE).
               parse_statement_function_source_sequence_entry = ((a_field_ptr)(sse->entity.ptr))->source_corresp.source_sequence_entry;
               ROSE_ASSERT(parse_statement_function_source_sequence_entry != NULL);
#endif
#if 1
               printf ("Calling parse_field_declaration() (iek_field) \n");
#endif
               SgDeclarationStatement* stmt = parse_field_declaration(sse);
#if 1
               printf ("DONE: Calling parse_field_declaration() (iek_field) \n");
#endif
               ROSE_ASSERT(stmt != NULL);

            // There is an exiting class definition that should be used for fields referenced from this case.
               ROSE_ASSERT(stmt->get_scope() != NULL);

            // DQ (11/6/2014): Adding C++11 support to record the use of C++11 syntax (for the unparser).
               if (field_entry->has_direct_braced_initializer == true)
                  {
                    printf ("In parse_field_declaration(): case iek_field: Need to mark as using C++11 braced initializer syntax \n");
                    ROSE_ASSERT(false);
                  }

               break;
             }

       // DQ (7/7/2014): Adding support for a namespace declaration.
          case iek_namespace:
             {
               a_namespace_ptr namespace_entry = (a_namespace_ptr)(sse->entity.ptr);
               ROSE_ASSERT(namespace_entry != NULL);

#ifdef DEBUG_CONVERSION
               indent_line(); debug_print("namespace declaration "); print_source_corresp(namespace_entry->source_corresp); debug_print("\n");
               ScopedIndent si(indent);
#endif
               SgDeclarationStatement* stmt = parse_namespace(sse);
               ROSE_ASSERT(stmt != NULL);
               result.push_back(stmt);

#if 1
               SgNamespaceDeclarationStatement* namespaceDeclaration = isSgNamespaceDeclarationStatement(stmt);
            // ROSE_ASSERT(namespaceDeclaration != NULL);
               if (namespaceDeclaration != NULL)
                  {
                    printf ("In parse_statement: case iek_namespace: namespace declaration: name = %s at (%d,%d) -> (%d,%d) \n",namespaceDeclaration->get_name().str(),
                         namespaceDeclaration->get_startOfConstruct()->get_line(),namespaceDeclaration->get_startOfConstruct()->get_col(),
                         namespaceDeclaration->get_endOfConstruct()->get_line(),namespaceDeclaration->get_endOfConstruct()->get_col());
                  }
#endif

#if 0
               printf ("Case of iek_namespace not yet supported! \n");
               ROSE_ASSERT(false);
#endif
               break;
             }

          case iek_static_assertion:
             {
#ifdef DEBUG_CONVERSION
               indent_line(); debug_print("parse_field_declaration: case iek_static_assertion"); debug_print("\n");
#endif
               SgDeclarationStatement* stmt = parse_static_assertion_declaration(sse);
               ROSE_ASSERT(stmt != NULL);

               result.push_back(stmt);

               break;
             }

#if MICROSOFT_EXTENSIONS_ALLOWED
          case iek_ms_attribute:
             {
#if 0
               printf ("In parse_statement: case iek_ms_attribute: Defining case iek_ms_attribute \n");
#endif
               SgMicrosoftAttributeDeclaration* microsoft_attribute = parse_microsoft_attribute(sse);
               ROSE_ASSERT(microsoft_attribute != NULL);
               result.push_back(microsoft_attribute);
#if 0
               printf ("Error: case iek_ms_attribute: not implemented \n");
               ROSE_ASSERT(false);
#endif
               break;
             }
#endif

       // DQ (12/29/2008): Added EDG 4.0 cases not handled
          case iek_none:
          case iek_source_file:
          case iek_constant:
          case iek_param_type:
          case iek_routine_type_supplement:
          case iek_based_type_list_member:
       // case iek_field:
          case iek_exception_specification:
          case iek_exception_specification_type:
          case iek_routine:
          case iek_label:
          case iek_expr_node:
          case iek_for_loop:
          case iek_switch_case_entry:
          case iek_switch_stmt_descr:
          case iek_handler:
          case iek_try_supplement:
#if MICROSOFT_EXTENSIONS_ALLOWED
          case iek_microsoft_try_supplement:
       // case iek_ms_attribute:
          case iek_ms_attribute_arg:
          case iek_ms_if_exists:
#endif
          case iek_block:
          case iek_object_lifetime:
          case iek_scope:
          case iek_id_name:
          case iek_string_text:
          case iek_other_text:
          case iek_internal_complex_value:
       // case iek_namespace:
       // case iek_using_decl:
          case iek_dynamic_init:
          case iek_local_static_variable_init:
          case iek_vla_dimension:
          case iek_overriding_virtual_function:
          case iek_derivation_step:
          case iek_base_class_derivation:
          case iek_base_class:
          case iek_class_list_entry:
          case iek_routine_list_entry:
          case iek_class_type_supplement:
          case iek_template_param_type_supplement:
          case iek_constructor_init:
          case iek_asm_entry:
#if GNU_EXTENSIONS_ALLOWED
          case iek_asm_operand:
#if !RECORD_RAW_ASM_OPERAND_DESCRIPTIONS
          case iek_asm_operand_constraint:
#endif
          case iek_named_register_list:
#endif
          case iek_template_arg:
          case iek_new_delete_supplement:
          case iek_throw_supplement:
          case iek_condition_supplement:
          case iek_source_sequence_entry:
          case iek_src_seq_sublist:
          case iek_instantiation_directive:
          case iek_scope_orphaned_list_header:
          case iek_template:
          case iek_decl_position_supplement:
          case iek_template_decl:
          case iek_template_parameter:
          case iek_seq_number_lookup_entry:
          case iek_local_expr_node_ref:
       // case iek_static_assertion:
          case iek_local_scope_ref:
          case iek_il_entity_list_entry:

       // DQ (7/21/2012): lambda cases that we want to cover (but might not be used here).
          case iek_lambda:
          case iek_lambda_capture:
          case iek_name_reference:
          case iek_name_qualifier:
          case iek_last:
             {
               printf ("Case not handled! kind = %s \n",summarize_entry_kind_in_string_kind(outer_kind));
               ROSE_ASSERT(false);
               break;
             }

          default:
             {
               printf ("Default case reached in parse_statement(): kind = %s \n",summarize_entry_kind_in_string_kind(outer_kind));
               ROSE_ASSERT(false);
            // ERROR_MESSAGE(stderr, "Bad outer kind in parse_statement %d\n", outer_kind);
            // abort();
             }
        }
#if 0
     testAndSkipOverPreviouslyVisitedSourceSequenceEntries(sse);
     printf ("DONE: Calling testAndSkipOverPreviouslyVisitedSourceSequenceEntries() after switch in parse_statement() \n");
     printf ("AFTER SWITCH in parse_statement(): part 2: entry = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif

// Note Label
end: ;

#if DEBUG_PRINT_SCOPE_STACK
     printScopeStack("Leaving parse_statement(): List the scopes on the stack:");
#endif

  // DQ (7/9/2012): Added test for parent.  This should be the first location in the call stack where the
  // parent could be set. It should be FALSE by default and set here where it is structurally defined, and 
  // not set where it is dependent on name qualification. Test all of the statements in the STL container.
     vector<SgStatement*>::iterator result_iterator = result.begin();
     while (result_iterator != result.end())
        {
       // DQ (6/26/2019): Added assertion.
          ROSE_ASSERT((*result_iterator) != NULL);
          if ((*result_iterator)->hasExplicitScope() == false)
             {
            // DQ (7/9/2012): We can set the parent directly to the scope for the case of a statement which 
            // has not explicit scope (e.g. ClassDeclarations can have an explicit scope; or even a structural 
            // representation inside of a type reference).
               if ((*result_iterator)->get_parent() == NULL)
                  {
                    SgScopeStatement* scope = topScopeStack();
#if 0
                    printf ("In parse_statement(): Setting parent of %p = %s to scope = %p = %s \n",(*result_iterator),(*result_iterator)->class_name().c_str(),scope,scope->class_name().c_str());
#endif
                    (*result_iterator)->set_parent(scope);
                  }
             }
          result_iterator++;
        }

     SgStatement* firstStatementInList = (result.empty() == false) ? *(result.begin()) : NULL;
     updateSwitchSupportMap(parse_statement_function_source_sequence_entry,firstStatementInList);

     depthCounter--;

#if 0
     printf ("Leaving parse_statement(): existingBasicBlock = %p \n",existingBasicBlock);
#endif

     return result;
   }

