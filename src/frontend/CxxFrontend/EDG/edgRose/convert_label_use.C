
#include "edgRose.h"

SgLabelStatement* EDG_ROSE_Translation::convert_label_use(a_label_ptr p)
   {
#ifdef DEBUG_CONVERSION
     indent_line(); debug_print("label use "); print_source_corresp(p->source_corresp); debug_print("\n");
#endif

     if (p->source_corresp.name == NULL)
        {
#ifdef DEBUG_CONVERSION
          indent_line(); debug_print("- compiler-generated\n");
#endif
          return NULL;
        }

     SgLabelStatement*& sym = label_cache[p];

     if (sym == NULL)
        {
       // Make a stub label to be inserted into the tree later

#ifdef DEBUG_CONVERSION
          printf ("In convert_label_use(): Building a SgLabelStatement \n");
#endif
          ROSE_ASSERT(p->source_corresp.name != NULL);
          sym = set_generated_file_info(buildLabelStatement_nfi(p->source_corresp.name, set_generated_file_info(buildNullStatement_nfi()), topScopeStack()));
        }
     ROSE_ASSERT (sym != NULL);

     set_label_attributes(p,sym);

     return sym;
   }
