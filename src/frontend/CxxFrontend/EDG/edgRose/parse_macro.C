
#include "edgRose.h"

SgDeclarationStatement*
EDG_ROSE_Translation::parse_macro(SourceSequenceList& sse)
   {
  // This function only sees the #define and #undef macros, so it is not so interesting for use within ROSE.

     a_macro_ptr p = (a_macro_ptr)(sse->entity.ptr);
     ROSE_ASSERT(p != NULL);

#ifdef DEBUG_CONVERSION
     indent_line(); debug_print("macro %s \"%s\" ", p->is_undef ? "true" : "false", p->text); print_source_corresp(p->source_corresp); debug_print("\n");
#endif

  // DQ (8/11/2012): Update the current position being processed.
     a_seq_number macro_source_sequence_number = p->source_corresp.decl_position.seq;
     global_source_sequence_number = macro_source_sequence_number;

     ++sse;

     ROSE_ASSERT(p != NULL);

#if 0
     indent_line(); debug_print("macro (not supported)\n");

     printf ("p->source_corresp.name        = %s \n",p->source_corresp.name != NULL ? p->source_corresp.name : "(null)");
     printf ("p->is_undef                   = %s \n",p->is_undef ? "true" : "false");
     printf ("p->is_command_line_definition = %s \n",p->is_command_line_definition ? "true" : "false");
     printf ("p->is_predefined              = %s \n",p->is_predefined ? "true" : "false");
#if EXTRA_SOURCE_POSITIONS_IN_IL
     printf ("skip output of saved source range \n");
#endif
     printf ("p->text = %s \n",p->text != NULL ? p->text : "(null)");
#endif

#ifdef DEBUG_CONVERSION
     printf ("     --- p->source_corresp.name = %s p->text = %s \n",p->source_corresp.name != NULL ? p->source_corresp.name : "(null)",p->text != NULL ? p->text : "(null)");
#endif

#if 0
     printf ("In parse_macro(): calling summarize_macro() \n");
     summarize_macro(p);
#endif

     return NULL;
   }
