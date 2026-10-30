
#include "edgRose.h"

namespace EDG_ROSE_Translation {

SgNamespaceDeclarationStatement * construct_namespace(a_namespace_ptr p)
   {
#ifdef DEBUG_CONVERSION
     indent_line(); debug_print("construct_namespace"); debug_print("\n");
     ScopedIndent si(indent);
#endif

     ROSE_ASSERT(p != NULL);
     ROSE_ASSERT(p->is_namespace_alias == false);

  // DQ (7/23/2017): Before processing any new template parameter list, we should have absorbed 
  // any existing entries and associated them with their associated template declaration. This 
  // test should help debug where we are getting the mangled name incorrect because the the 
  // declaration scope is not being handled properly.
     ROSE_ASSERT(internal_intermediate_scope != NULL);
     ROSE_ASSERT(internal_intermediate_scope->symbol_table_size() == 0);

     a_scope_ptr edg_scope = p->variant.assoc_scope;
     ROSE_ASSERT(edg_scope != NULL);

     SgNamespaceDeclarationStatement * namespaceDeclaration = NULL;
     if (scope_cache.find(edg_scope) != scope_cache.end()) {
       SgScopeStatement* associated_scope = scope_cache[edg_scope];
       ROSE_ASSERT(associated_scope != NULL);

       SgNamespaceDefinitionStatement* associated_namespace_scope = isSgNamespaceDefinitionStatement(associated_scope);
       ROSE_ASSERT(associated_namespace_scope != NULL);
       namespaceDeclaration = associated_namespace_scope->get_namespaceDeclaration();
     } else {
       SgName namespace_name = generate_namespace_name(p);

       SgScopeStatement* parent_scope = getDeclarationScope(p->source_corresp);
       ROSE_ASSERT(parent_scope != NULL);

       namespaceDeclaration = set_file_info_for_corresp(buildNamespaceDeclaration_nfi(namespace_name,false,parent_scope),p->source_corresp);

       namespace_cache[p] = namespaceDeclaration;

       SgNamespaceDefinitionStatement*  namespaceDefinition = namespaceDeclaration->get_definition();
       ROSE_ASSERT(namespaceDefinition != NULL);

       scope_cache[edg_scope] = namespaceDefinition;

#if MAKE_REDUNDANT_CALLS_TO_DETECT_TRANSFORAMTIONS
       detectTransformations_local(namespaceDeclaration);
#endif

       appendStatement(namespaceDeclaration, parent_scope);
     }

     return namespaceDeclaration;
   }

}
