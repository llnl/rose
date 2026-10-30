
#include "edgRose.h"

#define DEBUG_parse_static_assertion_declaration 0

SgStaticAssertionDeclaration*
EDG_ROSE_Translation::parse_static_assertion_declaration(SourceSequenceList& sse)
   {
#if DEBUG_parse_static_assertion_declaration
     printf("Enter parse_static_assertion_declaration:\n");
#endif
     a_static_assertion_ptr static_assertion = (a_static_assertion_ptr)(sse->entity.ptr);
     ROSE_ASSERT(static_assertion != NULL);

  // I think we need to write a parse_static_assertion() function.
     ROSE_ASSERT(static_assertion->condition != NULL);
     SgExpression* constant_condition = convert_constant(static_assertion->condition);
     ROSE_ASSERT(constant_condition != NULL);

  // TV (02/11/2019): this can be null if there is no second argument in the static_assert (no text to be emited upon failure)
     SgName string_literal;
     if (static_assertion->string_literal != NULL) {
       SgExpression* constant_string_literal = convert_constant(static_assertion->string_literal);
       ROSE_ASSERT(constant_string_literal != NULL);

#if DEBUG_parse_static_assertion_declaration
       printf ("  constant_string_literal = %p = %s \n",constant_string_literal,constant_string_literal->class_name().c_str());
#endif

       SgStringVal* stringValue = isSgStringVal(constant_string_literal);
       ROSE_ASSERT(stringValue != NULL);

       string_literal = stringValue->get_value();

    // DQ (1/21/2018): We only needed the literal, however, likely the IR should be changed to accept a SgStringVal instead.
       delete stringValue;
     }

#if DEBUG_parse_static_assertion_declaration
     printf ("  string_literal = %s \n",string_literal.str());
#endif

     SgStaticAssertionDeclaration* result = SageBuilder::buildStaticAssertionDeclaration(constant_condition, string_literal);
     ROSE_ASSERT(result != NULL);
     ROSE_ASSERT(result->get_parent() == NULL);

     constant_condition->set_parent(result);
     result->set_parent(topScopeStack());

     result = set_file_info_for_where_corresp_is_unavailable(result, static_assertion->position);

     ++sse;

     return result;
   }
