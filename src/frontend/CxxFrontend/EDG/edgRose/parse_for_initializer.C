
#include "edgRose.h"

SgForInitStatement*
EDG_ROSE_Translation::parse_for_initializer(SourceSequenceList& sse)
   {
#ifdef DEBUG_CONVERSION
     indent_line(); debug_print("for or upc_forall initializer\n");
     ScopedIndent si(indent);
#endif

     SgForInitStatement* returnForInitStatement = NULL;

     an_il_entry_kind outer_kind = (an_il_entry_kind)(sse->entity.kind);
     ROSE_ASSERT (outer_kind == iek_statement);

     a_statement_ptr stmt = (a_statement_ptr)(sse->entity.ptr);
     a_statement_kind kind = stmt->kind;

#if 0
     printf ("In parse_for_initializer(): case of iek_statement: before switch over statement kinds: p = %p kind = %s \n",stmt,summarize_statement_in_string(stmt));
#endif

#if 0
     printf ("In parse_for_initializer(): calling summarize_statement() \n");
     summarize_statement(stmt);
#endif

     switch (kind) 
        {
          case stmk_decl:
             {
            // DQ (8/4/2012): We might want to push a SgForStatement onto the stack so that we 
            // would have a proper scope for these variables when they are part of declarations.

               vector<SgStatement*> decls = parse_statement_list(sse, iek_statement, stmt);
#if 0
               printf ("In parse_for_initializer(): decls.size() = %" PRIuPTR " \n",decls.size());
               for (size_t i = 0; i < decls.size(); i++)
                  {
                    printf ("In parse_statement_list(): decls[%" PRIuPTR "] = %p = %s \n",i,decls[i],decls[i]->class_name().c_str());
                  }
#endif
            // DQ (8/4/2012): Detect where multiple statements have been build into the result of parse_statement_list().
               vector<SgStatement*> correct_decls;
               if (decls.size() < 2)
                  {
                 // This is the typical case.
                    correct_decls = decls;
                  }
                 else
                  {
                 // If there are multiple declarations then just use the last one (the SgVariableDeclaration).
                 // We should consider a more elegant way to handle this. In the case of test2012_161.C the class 
                 // declaration should not be put into the SgForInitStmt.

#ifdef DEBUG_CONVERSION
                    printf ("WARNING: attempt to build a SgForInitiStmt with more than one declaration in it \n");
#endif
                 // correct_decls.push_back(decls[decls.size()-1]);
                    for (size_t i = 0; i < decls.size(); i++)
                       {
#ifdef DEBUG_CONVERSION
                         printf ("In parse_statement_list(): decls[%" PRIuPTR "] = %p = %s \n",i,decls[i],decls[i]->class_name().c_str());
#endif
                         if (isSgVariableDeclaration(decls[i]) != NULL)
                            {
#ifdef DEBUG_CONVERSION
                              printf ("Save this SgVariableDeclaration \n");
#endif
#if 0
                           // DQ (6/18/2013): I now think this is not the correct way to fix the problem noticed in test2013_214.C.
                           // DQ (6/18/2013): The way that multiple variable declarations in a for loop initializer is handled 
                           // is different from that of multiple SgInitializedName IR nodes in a single SgVariableDeclaration.
                           // Though perhaps it should be the same in ROSE for simplicity.
                              SgVariableDeclaration* variableDeclaration = isSgVariableDeclaration(decls[i]);
                              ROSE_ASSERT(variableDeclaration != NULL);
                              if (variableDeclaration->get_isAssociatedWithDeclarationList() == true)
                                 {
                                   printf ("In parse_statement_list(): Forcing reset of isAssociatedWithDeclarationList to FALSE \n");
                                   variableDeclaration->set_isAssociatedWithDeclarationList(false);
                                 }
                              ROSE_ASSERT(variableDeclaration->get_isAssociatedWithDeclarationList() == false);
#endif
                           // DQ (8/30/2014): Set this to be true so that the cases of pointer types can be unparsed correctly.
                              SgVariableDeclaration* variableDeclaration = isSgVariableDeclaration(decls[i]);
                              ROSE_ASSERT(variableDeclaration != NULL);
                              variableDeclaration->set_isAssociatedWithDeclarationList(true);

                              correct_decls.push_back(decls[i]);
                            }
                           else
                            {
#ifdef DEBUG_CONVERSION
                              printf ("Ignore this declaration = %p = %s in building the SgForInitStmt \n",decls[i],decls[i]->class_name().c_str());
#endif
                            }
                       }
                  }

            // DQ (8/4/2012): It is an error to have multiple statments in the for loop initialization statement
            // (it can be a simple declaration even with embedded type declarations); see test2012_161.C.
            // ROSE_ASSERT(correct_decls.size() < 2);

            // DQ (11/4/2012): This IR node can't be given a legitimate source code position and should be marked 
            // as compiler generated because it will not be unparsed directly and so is not allowed to have attached 
            // comments or CPP directives.
            // return set_file_info_for_stmt(buildForInitStatement_nfi(decls), stmt);
            // return set_file_info_for_stmt(buildForInitStatement_nfi(correct_decls), stmt);
            // return set_generated_file_info_to_unparse(buildForInitStatement_nfi(correct_decls));
               returnForInitStatement = buildForInitStatement_nfi(correct_decls);

               break;
             }

          case stmk_expr: 
             {
               vector<SgStatement*> exprs = parse_statement(sse);

            // DQ (11/4/2012): This IR node can't be given a legitimate source code position and should be marked 
            // as compiler generated because it will not be unparsed directly and so is not allowed to have attached 
            // comments or CPP directives.
            // return set_file_info_for_stmt(buildForInitStatement_nfi(exprs), stmt);
            // return set_generated_file_info_to_unparse(buildForInitStatement_nfi(exprs));
               returnForInitStatement = buildForInitStatement_nfi(exprs);
               break;
             }

   // DQ (12/29/2008): Added EDG 4.0 cases not handled
          case stmk_if:
          case stmk_while:
          case stmk_goto:
          case stmk_label:
          case stmk_return:
          case stmk_block:
          case stmk_end_test_while:
          case stmk_for:
          case stmk_switch_case:
          case stmk_switch:
          case stmk_init:
          case stmk_asm:
#if ASM_FUNCTION_ALLOWED
          case stmk_asm_func_body:
#endif
          case stmk_try_block:
          case stmk_set_vla_size:
          case stmk_vla_decl:
#if UPC_EXTENSIONS_ALLOWED
          case stmk_upc_notify:
          case stmk_upc_wait:
          case stmk_upc_barrier:
          case stmk_upc_fence:
          case stmk_upc_forall:
#endif
#if GNU_EXTENSIONS_ALLOWED
          case stmk_assigned_goto:
#endif
          case stmk_empty:
#if MICROSOFT_EXTENSIONS_ALLOWED
          case stmk_microsoft_try:
#endif
          case stmk_last:
             {
               printf ("Case not handled in parse_for_initializer! kind = %d) \n", kind);
               ROSE_ASSERT(false);
               break;
             }

          default:
             {
               ERROR_MESSAGE(stderr, "Bad statement kind in parse_for_initializer (kind = %d) \n", kind);
               ROSE_ASSERT(false);
             }
        }

     ROSE_ASSERT(returnForInitStatement != NULL);

  // DQ (1/28/2013): We previously didn't generate source position information for this construct, we fix this here.
  // set_generated_file_info_to_unparse(returnForInitStatement);

  // DQ (1/28/2013): I think we can assert this.
     ROSE_ASSERT(returnForInitStatement->get_init_stmt().empty() == false);

#if 1
  // DQ (1/2/2015): Set the source position to include better ending position whenre itializers are used.
     set_file_info_for_stmt(returnForInitStatement,stmt);

     Sg_File_Info* start = returnForInitStatement->get_startOfConstruct();
     Sg_File_Info* end   = returnForInitStatement->get_endOfConstruct();
     ROSE_ASSERT(start != NULL && end != NULL);
#if 0
     start->display("In parse_for_initializer(): start: debug");
     end  ->display("In parse_for_initializer(): end: debug");
#endif
#else
  // DQ (1/28/2013): Dig into the asociated statements and set the source position information for the SgForInitStatement.
     if (returnForInitStatement->get_init_stmt().empty() == false)
        {
          SgStatementPtrList::iterator         firstStatement = returnForInitStatement->get_init_stmt().begin();
          SgStatementPtrList::reverse_iterator lastStatement  = returnForInitStatement->get_init_stmt().rbegin();

#error "DEAD CODE!"

          Sg_File_Info* start = (*firstStatement)->get_startOfConstruct();
          Sg_File_Info* end   = (*lastStatement) ->get_endOfConstruct();
          ROSE_ASSERT(start != NULL && end != NULL);
#if 0
          start->display("In parse_for_initializer(): start: debug");
          end  ->display("In parse_for_initializer(): end: debug");
#endif
       // Since the returnForInitStatement was constructed using the builder interface, there should be valid 
       // startOfConstruct and endOfConstruct objects, but set to default values (no source position).
          ROSE_ASSERT(returnForInitStatement->get_startOfConstruct() != NULL);
          ROSE_ASSERT(returnForInitStatement->get_endOfConstruct() != NULL);

       // Use the assignment operator to reset the values (more direct implementation).
          *(returnForInitStatement->get_startOfConstruct()) = *start;
          *(returnForInitStatement->get_endOfConstruct())   = *end;
        }
#endif
#if 0
       else
        {
          printf ("Note: empty SgForInitStatement: no source position provided (marked as generated) \n");
          set_generated_file_info_to_unparse(returnForInitStatement);
        }
#endif

  // DQ (12/29/2008): Added return value for function to avoid compiler warning.
  // return NULL;
     return returnForInitStatement;
  }
