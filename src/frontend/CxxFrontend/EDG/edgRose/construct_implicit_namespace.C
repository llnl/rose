
#include "edgRose.h"

namespace EDG_ROSE_Translation {

SgNamespaceDeclarationStatement * construct_implicit_namespace(a_namespace_ptr p)
   {
     ROSE_ASSERT(p->source_corresp.name != NULL);
     SgName namespace_name = p->source_corresp.name;
     ROSE_ASSERT(namespace_name == "std" || namespace_name == "__cxxabiv1");

     SgNamespaceDeclarationStatement * implicit_namespace_declaration = NULL;

     if (namespace_name == "std") {
       if (global_std_namespace == NULL) {
         global_std_namespace = construct_namespace(p);
       }
       ROSE_ASSERT(global_std_namespace != NULL);
       implicit_namespace_declaration = global_std_namespace;
     }

     if (namespace_name == "__cxxabiv1") {
       if (global_cxxabiv1_namespace == NULL) {
         global_cxxabiv1_namespace = construct_namespace(p);
       }
       ROSE_ASSERT(global_cxxabiv1_namespace != NULL);
       implicit_namespace_declaration = global_cxxabiv1_namespace;
     }

     ROSE_ASSERT(implicit_namespace_declaration != NULL);

     ROSE_ASSERT(implicit_namespace_declaration->isCompilerGenerated() == true);
     
     implicit_namespace_declaration->get_startOfConstruct()->unsetOutputInCodeGeneration();
     implicit_namespace_declaration->get_endOfConstruct()->unsetOutputInCodeGeneration();

     // When opening multiple files on the same command line `global_*_namespace` will have been set in the first file
     if (namespace_cache.find(p) == namespace_cache.end()) {
       namespace_cache[p] = implicit_namespace_declaration;
     }
     if (scope_cache.find(p->variant.assoc_scope) == scope_cache.end()) {
       ROSE_ASSERT(implicit_namespace_declaration->get_definition() != NULL);
       scope_cache[p->variant.assoc_scope] = implicit_namespace_declaration->get_definition();
     }

     // Make sure that these caches were not over-wrote somewhere else
     ROSE_ASSERT(namespace_cache[p] == implicit_namespace_declaration);
     ROSE_ASSERT(scope_cache[p->variant.assoc_scope] == implicit_namespace_declaration->get_definition());

     return implicit_namespace_declaration;
   }
}
