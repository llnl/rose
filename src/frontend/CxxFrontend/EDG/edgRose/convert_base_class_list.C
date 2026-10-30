
#include "edgRose.h"

void EDG_ROSE_Translation::convert_base_class_list( a_type_ptr p, SgClassDefinition* classDefinition )
   {
  // Put out the base class list.

  // DQ (8/12/2014): Adding support for C++11 template type references (test2014_38.C).
  // a_class_type_supplement_ptr ctsp  = p->variant.class_struct_union.extra_info;
  // a_base_class_ptr bcp              = ctsp->base_classes;
     a_class_type_supplement_ptr ctsp  = NULL;
     a_base_class_ptr bcp              = NULL;

  // DQ (8/14/2014): This was a bug that eliminated the generation of base classes.
  // if ((bcp != NULL) && (bcp->type->kind == tk_class || bcp->type->kind == tk_struct || bcp->type->kind == tk_union))
     if (p->kind == tk_class || p->kind == tk_struct || p->kind == tk_union)
        {
          ctsp = p->variant.class_struct_union.extra_info;

       // DQ (8/14/2014): I think we can assert this.
          ROSE_ASSERT(ctsp != NULL);

          bcp  = ctsp->base_classes;
        }

  // a_boolean        first_base_class = TRUE;

     ROSE_ASSERT(classDefinition != NULL);

     if (bcp != NULL)
        {
          for (; bcp != NULL; bcp = bcp->next)
             {
            // The list contains all base classes, but record only the direct base classes into the base class list for the class declaration.
               if (bcp->direct)
                  {
                    a_base_class_derivation_ptr bcdp = bcp->derivation;
                 // If this is not the first base class, put a comma between the base classes.
                 // first_base_class = FALSE;
                    if (bcp->is_virtual)
                       {
                      // Find the direct derivation for a virtual base class.
                         for (; !bcdp->direct; bcdp = bcdp->next) {}
                       }

                 // DQ (8/12/2012): Added test.
                    ROSE_ASSERT(bcp->type != NULL);

                 // DQ (8/12/2012): Added test (this fails, so we need the code below).
                 // ROSE_ASSERT(type_translation_cache.find(bcp->type) != type_translation_cache.end());

                 // DQ (8/12/2012): We can't just reference type_translation_cache if there is not a vaild entry and we have to test 
                 // for a valid entry (else NULL pointer entries will be added).
                 // Display the derivation access.
                 // SgType* tmp_type = type_translation_cache[bcp->type];
                    SgType* tmp_type = NULL;
                    if (type_translation_cache.find(bcp->type) != type_translation_cache.end())
                       {
                         tmp_type = type_translation_cache[bcp->type];
                       }
                    else
                       {
                         tmp_type = convert_type(bcp->type,false,false);
                       }

                    SgClassDeclaration * classDeclaration = NULL;
                    SgNonrealDecl * nrdecl = NULL;
                       {
                         ROSE_ASSERT(tmp_type != NULL);

                         SgClassType* base_class_type = isSgClassType(tmp_type);
                         SgNonrealType* base_nr_type  = isSgNonrealType(tmp_type);

                         ROSE_ASSERT(base_class_type != NULL || base_nr_type != NULL);

                         if (base_class_type != NULL) {
                           classDeclaration = isSgClassDeclaration(base_class_type->get_declaration());
                           ROSE_ASSERT(classDeclaration != NULL);
                         }
                         if (base_nr_type != NULL) {
#if 0
                           printf("In convert_base_class_list(): case of a nonreal class (SgNonrealType)");
#endif
                           nrdecl = isSgNonrealDecl(base_nr_type->get_declaration());
                         }
                       }
                    ROSE_ASSERT(classDeclaration != NULL || nrdecl != NULL);
#if 0
                    printf ("In convert_base_class_list(): Building possible SgBaseClass IR node: classDeclaration = %p \n",classDeclaration);
#endif
                 // SgBaseClass* baseclass = new SgBaseClass ( classDeclaration, bcp->direct );
                 // SgBaseClass* baseclass = NULL;
                    if (classDeclaration != NULL)
                       {
                      // DQ (5/6/2013): Refactors this support to the builder API.
                         SgBaseClass* baseclass = buildBaseClass(classDeclaration, classDefinition, bcp->is_virtual, bcp->direct );
                         ROSE_ASSERT(baseclass != NULL);

                      // DQ (1/21/2019): Moved to using ROSETTA generated access functions which return a pointer.
                         ROSE_ASSERT(baseclass->get_baseClassModifier() != NULL);

                      // DQ (8/11/2020): changed name from set_access_modifiers() to set_access_modifiers_for_base_class() 
                      // to avoid confusion with set_access_modifier().
                      // baseclass->get_baseClassModifier()->get_accessModifier() = set_access_modifiers(bcdp->access);
                         baseclass->get_baseClassModifier()->get_accessModifier() = set_access_modifiers_for_base_class(bcdp->access);
                       }
                      else
                       {
                      // DQ (12/30/2011): This case is demonstrated in test2004_35.C (which fails in STL list class: cpp_type_traits.h).
#ifdef DEBUG_CONVERSION
                         printf ("Skipped handling of base class specification (nonreal class (template strange)). \n");
#endif
                         SgNonrealBaseClass * baseclass = buildNonrealBaseClass(nrdecl, classDefinition, bcp->is_virtual, bcp->direct );
                         ROSE_ASSERT(baseclass != NULL);

                      // DQ (8/11/2020): changed name from set_access_modifiers() to set_access_modifiers_for_base_class() 
                      // to avoid confusion with set_access_modifier().
                      // baseclass->get_baseClassModifier()->get_accessModifier() = set_access_modifiers(bcdp->access);
                         baseclass->get_baseClassModifier()->get_accessModifier() = set_access_modifiers_for_base_class(bcdp->access);
                       }
                  }
             }
        }

  // ROSE_ASSERT(!"convert_base_class_list() not yet implemented");
   }
