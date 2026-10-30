
#include "edgRose.h"

SgSymbol* EDG_ROSE_Translation::convert_class_use(a_variable_ptr p)
   {
  // This function converts a variable in EDG to a SgClassSymbol for use in building the "this" pointer expression.
  // Note that the variable must be a "this" pointer (should be checked before calling).

  // DQ (11/29/2011): Observation about this function:
  // If this is variable from a template class the type will resolve to an instantiated template (SgTemplateInstantiationDecl) 
  // of that class instead of the template declaration.  This is not a serious problem, but it is just a bit wrong.  Plus, I
  // can't seem to figure out when it happens to enable me to force the correct behavior.  So we will leave 
  // this for the moment and proceed with other work.  The result is that for test2011_161.C is that we end up
  // with a template instantiation in the AST (visible only from the Whole AST graph, I think) that is there
  // even though there is no driving template instantiation.  This might mean that it should be detectable from 
  // the template arguments (which would not have any thnk but template parameters, if that is possible; no concrete types).

#ifdef DEBUG_CONVERSION
     indent_line(); debug_print("class use "); print_source_corresp(p->source_corresp); debug_print("\n");
#endif

#if 0
     printf ("In convert_class_use(): Calling summarize_variable(p = %p) \n",p);
     summarize_variable(p);
#endif

  // Verify this is a "this" pointer reference.
     ROSE_ASSERT(p->is_this_parameter == true);

  // This will be a "this" pointer expression (and no name is available)
     ROSE_ASSERT(p->source_corresp.name == NULL);

#if 0
     printf ("In convert_class_use(): calling summarize_type() on p->type \n");
     summarize_type(p->type);
#endif

  // I think that the class declaration must have been seen at the point where the "this" pointer can be referenced, so this should be available.
     a_type_ptr base_type = skip_typerefs(p->type); // pointer to some class
     ROSE_ASSERT(base_type != NULL);

  // This "this" variable is a pointer type by definition, so get the associated class type.
     a_type_ptr  original_class_type = base_type->variant.pointer.type;
     ROSE_ASSERT(original_class_type != NULL);
#if 0
     printf ("Checking the type_translation_cache using original_class_type = %p \n",original_class_type);
#endif

#if 0
     printf ("In convert_class_use(): calling summarize_type() \n");
     summarize_type(original_class_type);
#endif

#if 0
     printf ("In convert_class_use(): calling convert_type(): original_class_type = %p \n",original_class_type);
#endif

     SgType* possibleClassType = convert_type(original_class_type);
     ROSE_ASSERT(possibleClassType != NULL);

#if 0
     printf ("In convert_class_use(): possibleClassType = %p = %s \n",possibleClassType,possibleClassType->class_name().c_str());
#endif

#if 0
  // DQ (6/6/2016): Tracing down use of incorrect typedef alias (inconsistant with original source code) \n");
     printf ("In convert_class_use(): Calling stripType() \n");
#endif

  // DQ (12/10/2011): Reduce to the base type (see test2004_35.C)
     possibleClassType = possibleClassType->stripType();

     SgClassType* classType = isSgClassType(possibleClassType);
     SgNonrealType* nrtype = isSgNonrealType(possibleClassType);

     SgSymbol* sym = NULL;
     if (classType != NULL) {

       ROSE_ASSERT(classType->get_declaration() != NULL);
    // printf ("classType->get_declaration() = %p = %s \n",classType->get_declaration(),classType->get_declaration()->class_name().c_str());

    // DQ (3/6/2012): Added test.
       ROSE_ASSERT (classType->get_declaration() != NULL);
       SgClassDeclaration* local_classDeclaration = isSgClassDeclaration(classType->get_declaration());
       ROSE_ASSERT (local_classDeclaration != NULL);
       ROSE_ASSERT (local_classDeclaration->get_firstNondefiningDeclaration() != NULL);
       ROSE_ASSERT (local_classDeclaration->get_firstNondefiningDeclaration() == local_classDeclaration);

    // Get the declaration from the SgClassType, is there a more direct way?
       SgClassDeclaration* classDeclaration = isSgClassDeclaration(classType->get_declaration());
       ROSE_ASSERT(classDeclaration != NULL);

    // Get the symbol from the symbol table where the declaration was defined (defining declaration is not required).
       sym = isSgClassSymbol(classDeclaration->get_symbol_from_symbol_table());
     } else if (nrtype != NULL) {

       ROSE_ASSERT(nrtype->get_declaration() != NULL);

    // DQ (3/6/2012): Added test.
       SgNonrealDecl* nrdecl = isSgNonrealDecl(nrtype->get_declaration());
       ROSE_ASSERT (nrdecl != NULL);
       ROSE_ASSERT (nrdecl->get_firstNondefiningDeclaration() != NULL);
       ROSE_ASSERT (nrdecl->get_firstNondefiningDeclaration() == nrdecl);

    // Get the symbol from the symbol table where the declaration was defined (defining declaration is not required).
       sym = isSgNonrealSymbol(nrdecl->get_symbol_from_symbol_table());
     } else {
       ROSE_ASSERT(false);
     }
     ROSE_ASSERT(sym != NULL);

     return sym;
   }
