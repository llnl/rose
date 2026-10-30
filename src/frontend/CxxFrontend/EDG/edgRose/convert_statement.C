
#include "edgRose.h"

SgStatement*
EDG_ROSE_Translation::convert_statement(a_statement_ptr stmt)
   {
  // DQ (8/21/2020): Let's see if we can build this simple of an interface so that we can use it for the statements in a switch statement.
  // The goal might be to refactor the code in the parse_statement() function below to make it more usable in this function for processing switch statements.

     SgStatement* resultStatement = NULL;

#if DEBUG_PARSE_STATEMENT
     printf ("In convert_statement(a_statement_ptr stmt): case of iek_statement: stmt = %p statement kind = %s \n",stmt,summarize_statement_in_string(stmt));
#endif

  // Build something simple to return.
     resultStatement = buildNullStatement_nfi();



#if DEBUG_PARSE_STATEMENT
     printf ("Leaving convert_statement(): case of iek_statement: p = %p statement kind = %s \n",p,summarize_statement_in_string(p));
#endif

     return resultStatement;
   }
