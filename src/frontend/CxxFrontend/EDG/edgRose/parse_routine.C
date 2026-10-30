
#include "edgRose.h"

SgDeclarationStatement*
EDG_ROSE_Translation::parse_routine(SourceSequenceList& sse, bool forceTemplateDeclaration, a_template_ptr edg_template, bool forceSecondaryDeclaration)
   {
  // DQ (5/8/2012): We want to be able to force the generation of secondary declaration (function prototypes
  // instead of defining functions) where we detect that we are in a class template declaration.  This is because
  // when we use the template declaration as a string in the unparser we want to use an AST that matches the
  // string which EDG builds.  Importantly, this EDG string represents all defining functions as function 
  // prototypes.

     a_routine_ptr p = (a_routine_ptr)(sse->entity.ptr);
     ROSE_ASSERT (p != NULL);
     ++sse;

  // DQ (8/11/2012): Update the current position being processed.
     a_seq_number function_source_sequence_number = p->source_corresp.decl_position.seq;
     global_source_sequence_number = function_source_sequence_number;

     if (edg_template != NULL && p->assoc_template != edg_template)
        {
       // Report when these are different and use the one from p->assoc_template where possible.
#ifdef DEBUG_CONVERSION
          printf("WARNING: Report when these are different and use the one from p->assoc_template where possible. edg_template = %p p->assoc_template = %p \n",edg_template,p->assoc_template);
#endif
       // Assign the function associated templae, even if it is NULL (the class template will not work).
          edg_template = p->assoc_template;
        }

#if 0
     printf ("In parse_routine(): Calling convert_routine(p = %p) forceTemplateDeclaration = %s edg_template = %p forceSecondaryDeclaration = %s \n",p,forceTemplateDeclaration ? "true" : "false",edg_template,forceSecondaryDeclaration ? "true" : "false");
#endif

  // return convert_routine(p,forceTemplateDeclaration,edg_template);
     SgDeclarationStatement* result = NULL;
     if (forceSecondaryDeclaration == true)
        {
#if 0
          printf ("Forcing the generation of a secondary declartion to match EDG generated template string representation \n");
#endif
          result = convert_routine_prototype(p,forceTemplateDeclaration,edg_template);
        }
       else
        {
#if 0
          printf ("In parse_routine(): Calling convert_routine() \n");
#endif
          result = convert_routine(p,forceTemplateDeclaration,edg_template);
#if 0
          printf ("DONE: In parse_routine(): Calling convert_routine() \n");
#endif
        }

  // DQ (2/16/2013): Added assertion.
     ROSE_ASSERT(result != NULL);

#if 0
     result->get_startOfConstruct()->display("Leaving parse_routine(): START: debug");
     result->get_endOfConstruct()->display("Leaving parse_routine(): END: debug");
#endif

  // DQ (2/16/2013): Added support for access modifiers.
     unsigned int access_level = p->source_corresp.access;
     set_access_modifier(result,access_level);

#if 0
     printf ("Leaving parse_routine(): result = %p = %s: access = %s \n",result,result->class_name().c_str(),result->get_declarationModifier().get_accessModifier().displayString().c_str());
#endif

#if 0
     printf ("Leaving parse_routine(): p = %p result = %p = %s \n",p,result,result->class_name().c_str());
#endif

     return result;
   }
