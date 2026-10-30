
#include "edgRose.h"

SgStatement* EDG_ROSE_Translation::convert_goto(a_label_ptr p)
   {
#ifdef DEBUG_CONVERSION
      indent_line(); debug_print("label use for goto break = %s continue = %s ",p->break_label ? "true" : "false",p->continue_label ? "true" : "false"); print_source_corresp(p->source_corresp); debug_print("\n");
     ScopedIndent si(indent);
#endif

     if (p->source_corresp.name == NULL)
        {
       // This is a compiler-generated label
          if (p->break_label)
             {
#if 0
               printf ("In convert_goto(): Calling buildBreakStmt_nfi() \n");
#endif
               return buildBreakStmt_nfi();
             }
            else
             {
               if (p->continue_label)
                  {
#if 0
                    printf ("In convert_goto(): Calling buildContinueStmt_nfi() \n");
#endif
                    return buildContinueStmt_nfi();
                  }
                 else
                  {
                    ROSE_ASSERT (!"Bad case in convert_goto"); abort();
                  }
             }
        }
       else
        {
#if 0
          printf ("In convert_goto(): p->source_corresp.name = %s Calling convert_label_use(p = %p) \n",p->source_corresp.name,p);
#endif
          SgLabelStatement* ls = convert_label_use(p);
          ROSE_ASSERT (ls != NULL);
#if 0
          printf ("In convert_goto(): p->source_corresp.name = %s Done calling convert_label_use(p = %p) calling buildGotoStatement_nfi(ls = %p) \n",p->source_corresp.name,p,ls);
#endif
          return buildGotoStatement_nfi(ls);
        }


     ROSE_ASSERT(!"should be unreachable code in convert_goto");
     return NULL;
   }
