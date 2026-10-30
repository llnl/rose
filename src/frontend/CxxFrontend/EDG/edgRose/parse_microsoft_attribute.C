
#include "edgRose.h"

#if MICROSOFT_EXTENSIONS_ALLOWED

SgMicrosoftAttributeDeclaration*
EDG_ROSE_Translation::parse_microsoft_attribute(SourceSequenceList& sse)
   {
     SgName name;

     an_ms_attribute_ptr ms_attribute = (an_ms_attribute_ptr)(sse->entity.ptr);
     ROSE_ASSERT(ms_attribute != NULL);
#if 0
     printf ("In global_or_namespace_scope_entity: case iek_ms_attribute: Defining case iek_ms_attribute to return (ms_attribute = %p) \n",ms_attribute);
#endif
#if 0
     printf ("In global_or_namespace_scope_entity: case iek_ms_attribute: calling summarize_ms_attribute() \n");
     summarize_ms_attribute(ms_attribute);
#endif

     ++sse;

     if (ms_attribute->kind != msak_custom)
        {
          name = ms_attribute->variant.info.string;
        }
       else
        {
          printf ("ms_attribute->kind == msak_custom: ms attribute is not supported in ROSE \n");
        }

     SgMicrosoftAttributeDeclaration* microsift_attribute = buildMicrosoftAttributeDeclaration(name);

     ROSE_ASSERT(microsift_attribute != NULL);

     microsift_attribute->set_parent(topScopeStack());
     ROSE_ASSERT(microsift_attribute->get_parent() != NULL);

     return microsift_attribute;
   }
#endif
