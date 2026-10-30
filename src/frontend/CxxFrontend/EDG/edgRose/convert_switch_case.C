
#include "edgRose.h"

SgStatement*
EDG_ROSE_Translation::convert_switch_case(a_switch_case_entry_ptr edg_case_stmt)
   {
  // SgCaseOptionStmt* rose_case_stmt = NULL;
     SgStatement* rose_case_stmt = NULL;

     ROSE_ASSERT(edg_case_stmt != NULL);

  // The stmk_switch_case statement pointing to this entry.
     a_statement_ptr edg_switch_stmt = edg_case_stmt->stmt;

#if 0
     printf ("Calling summarize_switch (edg_switch_stmt = %p) \n",edg_switch_stmt);
     summarize_statement (edg_switch_stmt);
#endif

#if 0
     printf ("In convert_switch_case(): switch case statements: edg_switch_stmt = %s \n",summarize_statement_in_string(edg_switch_stmt));
#endif

     a_switch_stmt_descr_ptr edg_switch_extra_info = edg_switch_stmt->variant.switch_stmt.extra_info;
     ROSE_ASSERT(edg_switch_extra_info != NULL);

     bool isDefaultCase = (edg_switch_extra_info->default_case != NULL && edg_case_stmt == edg_switch_extra_info->default_case);

#if 0
     printf ("In convert_switch_case(): case stmk_switch: cases: isDefaultCase = %s \n",isDefaultCase ? "true" : "false");
#endif

#if 0
     printf ("Calling summarize_switch_descriptor (edg_switch_extra_info = %p) \n",edg_switch_extra_info);
     summarize_switch_descriptor (edg_switch_extra_info);
#endif

  // The integer value associated with this case.  (Or the value of the start
  // of the range if this is a GNU case range.)  NULL for a "default:" case.
  // a_constant_ptr edg_case_value = edg_case_stmt->case_value;

#if GNU_EXTENSIONS_ALLOWED
  // If this is a GNU case range, the value of the end of the range; otherwise, NULL.
  // a_constant_ptr range_end = NULL;
#endif

     SgStatement* case_body_with_statements = NULL;
#if 0
     printf ("NOTE: In convert_switch_case(): case_body_with_statements set to NULL for initial implementation! \n");
#endif

#if 0
     printf ("In convert_switch_case(): edg_case_stmt->case_value = %p \n",edg_case_stmt->case_value);
#endif
     if (isDefaultCase == false && edg_case_stmt->case_value == NULL)
        {
#if 0
          printf ("WARNING: Using secondary mechanism to detect default case being used (edg_case_stmt->case_value == NULL) \n");
#endif
          isDefaultCase = true;
        }

     if (isDefaultCase == false)
        {
#if 0
          printf ("In convert_switch_case(): case stmk_switch: cases: Found a NON-DEFAULT case! edg_case_stmt = %p \n",edg_case_stmt);
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
            // SgExpression* case_constant_expr_range_end = convert_constant(edg_case_stmt->range_end).value();
            // isSgCaseOptionStmt(rose_case_stmt)->get_range_end(case_constant_expr_range_end);

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
#if 0
          printf ("In convert_switch_case(): cases: Found the DEFAULT case! edg_case_stmt = %p \n",edg_case_stmt);
#endif
          rose_case_stmt = buildDefaultOptionStmt_nfi(case_body_with_statements);

       // DQ (12/26/2014): Assertion added.
          ROSE_ASSERT(rose_case_stmt != NULL);

       // DQ (3/25/2021): Assertion added.
       // ROSE_ASSERT(rose_case_stmt->get_parent() != NULL);
#if 0
          if (rose_case_stmt->get_parent() == NULL)
             {
               printf ("In convert_switch_case(): rose_case_stmt->get_parent() == NULL \n");
             }
#endif
#if 0
          printf ("Exiting at the base of the first default statement in a switch stmt \n");
          ROSE_ASSERT(false);
#endif
        }

     ROSE_ASSERT(rose_case_stmt != NULL);

  // ROSE_ASSERT(case_body_with_statements != NULL);
     if (case_body_with_statements != NULL)
        {
          case_body_with_statements->set_parent(rose_case_stmt);

          ROSE_ASSERT(case_body_with_statements->get_parent() == rose_case_stmt);
#if 0
          printf ("case_body_with_statements source position: \n");
          case_body_with_statements->get_startOfConstruct()->display("case_body_with_statements source position: start");
          case_body_with_statements->get_endOfConstruct()  ->display("case_body_with_statements source position: end");
#endif
        }

  // DQ (12/26/2014): Set the source position.
     set_file_info_for_case_option_stmt(rose_case_stmt,edg_case_stmt);

     ROSE_ASSERT(rose_case_stmt != NULL);

  // This will be set by the caller of this function.
  // ROSE_ASSERT(rose_case_stmt->get_body() == NULL);

     return rose_case_stmt;
   }
