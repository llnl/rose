
#include "edgRose.h"

SgDeclarationStatement* EDG_ROSE_Translation::convert_type_as_secondary_decl(SourceSequenceList& sse)
   {
  // DQ (7/17/2012): Fixup the API.
     a_src_seq_secondary_decl_ptr source_sequence_secondary_declaration = (a_src_seq_secondary_decl_ptr)(sse->entity.ptr);
     ROSE_ASSERT(source_sequence_secondary_declaration != NULL);
     a_type_ptr p = (a_type_ptr)(source_sequence_secondary_declaration->entity.ptr);

#ifdef DEBUG_CONVERSION
     indent_line(); debug_print("%stype to secondary_decl p = %p ", (p == NULL ? "null " : ""),p);
  // if (p != NULL) {print_source_corresp(p->source_corresp); debug_print("\n");}
#endif

  // DQ (9/2/2012): I would like to assert that p != NULL (but not just now). This return path is part of older code...
     if (p == NULL) return NULL;

#ifdef DEBUG_CONVERSION
     ScopedIndent si(indent);
#endif

#ifdef DEBUG_CONVERSION
  // DQ (7/17/2012): we want to output the source position of the secondary declaration (not the primary).
  // this is part of fixing the incorrect interleaving of CPP macros into test test2012_125.C.
     indent_line(); debug_print("-- secondary start at:"); print_source_pos(source_sequence_secondary_declaration->decl_position); debug_print("\n");
     if (source_sequence_secondary_declaration->decl_pos_info != NULL)
        {
       // DQ (5/13/2012): But we add the options in EDG to save more source position information (and thus the start AND end position is available to us at p->decl_pos_info).
          indent_line(); debug_print("-- secondary identifier_range start at:"); print_source_pos(source_sequence_secondary_declaration->decl_pos_info->identifier_range.start); debug_print("\n");
          indent_line(); debug_print("-- secondary identifier_range end at  :"); print_source_pos(source_sequence_secondary_declaration->decl_pos_info->identifier_range.end); debug_print("\n");

          indent_line(); debug_print("-- secondary specifiers_range start at:"); print_source_pos(source_sequence_secondary_declaration->decl_pos_info->specifiers_range.start); debug_print("\n");
          indent_line(); debug_print("-- secondary specifiers_range end at  :"); print_source_pos(source_sequence_secondary_declaration->decl_pos_info->specifiers_range.end); debug_print("\n");
        }
       else
        {
       // Sometimes the source position is just not available.
          indent_line(); debug_print("-- source_sequence_secondary_declaration->decl_pos_info == NULL \n");
        }
#endif

#if 0
     printf ("In convert_type_as_secondary_decl(): Calling summarize_secondary_decl() \n");
     summarize_src_seq_secondary_decl(source_sequence_secondary_declaration);
#endif

     SgDeclarationStatement* result = NULL;
     a_type_kind kind = (a_type_kind)p->kind;
     switch (kind)
        {
          case tk_integer:
             {
               ROSE_ASSERT (p->variant.integer.enum_type);
#ifdef DEBUG_CONVERSION
               indent_line(); debug_print("enum\n");
#endif
            // DQ (1/2/2012): Refactoring to generate common name used in this function.
            // SgName name = e->source_corresp.name;
               SgName name = p->source_corresp.name == NULL ? generate_name_for_type(p) : p->source_corresp.name;

            // string name = p->source_corresp.name == NULL ? generate_name_for_type(p) : p->source_corresp.name;
               SgEnumDeclaration* decl = set_file_info_for_corresp(buildNondefiningEnumDeclaration_nfi(name, topScopeStack()), p->source_corresp);
#if 0
               printf ("##### In convert_type_as_secondary_decl(): Building a nondefining EnumDeclaration: decl = %p \n",decl);
#endif

            // DQ (1/11/2019): Adding support for C++11 enum type specification in enum prototypes.
               if (p->variant.integer.extra_info->base_type != NULL)
                  {
                    ROSE_ASSERT(p->variant.integer.extra_info->base_type != NULL);
                    SgType* enum_type = convert_type(p->variant.integer.extra_info->base_type);
                    ROSE_ASSERT(enum_type != NULL);

                    decl->set_field_type(enum_type);

                 // DQ (1/11/2019): I think this is always true for C++11 enum prototypes.
                 // ROSE_ASSERT(p->variant.integer.is_scoped_enum == true);
#if 0
                    printf ("In convert_type_as_secondary_decl(): Exiting as a test (specification of base type for enum fields) \n");
                    ROSE_ASSERT(false);
#endif
                  }

            // DQ (1/11/2019): I think this is always true for C++11 enum prototypes.
            // ROSE_ASSERT(p->variant.integer.is_scoped_enum == true);

            // DQ (1/11/2019): If we have a C++11 enum prototype, then is must be a scoped enum.
               if (p->variant.integer.is_scoped_enum == true)
                  {
                 // This is the case of a C++11 "enum class" construct.
                    decl->set_isScopedEnum(true);
                  }

               if (is_unnamed(p))
                  {
                    printf ("In convert_type_as_secondary_decl(): Mark enum declaration = %p = %s as UN-NAMED \n",decl,decl->class_name().c_str());
                    decl->set_isUnNamed(true);

                 // DQ (7/31/2012): Set the non-defining declaration
                    if (decl->get_firstNondefiningDeclaration() != NULL)
                       {
                         isSgClassDeclaration(decl->get_firstNondefiningDeclaration())->set_isUnNamed(true);
                       }

                 // DQ (7/31/2012): Set the defining declaration
                    if (decl->get_definingDeclaration() != NULL)
                       {
                         isSgClassDeclaration(decl->get_definingDeclaration())->set_isUnNamed(true);
                       }
                  }

            // DQ (8/12/2012): Added test.
               ROSE_ASSERT(decl->get_type() != NULL);
#if 0
            // DQ (8/12/2012): Added test.
               ROSE_ASSERT(type_translation_cache.find(p) == type_translation_cache.end());
               type_translation_cache[p] = decl->get_type();
#else
            // DQ (8/12/2012): This is a safer version of the code (prevents insertion of NULL pointers into the type_translation_cache).
               if (type_translation_cache.find(p) == type_translation_cache.end())
                  {
                    type_translation_cache[p] = decl->get_type();
                    ROSE_ASSERT(type_translation_cache.find(p) != type_translation_cache.end());
                  }
                 else
                  {
                    ROSE_ASSERT(type_translation_cache[p] != NULL);

                 // This fails for test2004_108.C
                 // ROSE_ASSERT(type_translation_cache[p] == decl->get_type());
                    if (type_translation_cache[p] != decl->get_type())
                       {
                         printf ("WARNING: In convert_type_as_secondary_decl(): case tk_integer: the enum type in the type_translation_cache at p = %p is not the same as decl->get_type() = %p = %s type_translation_cache[p] = %p = %s \n",
                            p,decl->get_type(),decl->get_type()->class_name().c_str(),type_translation_cache[p],type_translation_cache[p]->class_name().c_str());
                      // ROSE_ASSERT(false);
#if 0
                         printf ("In convert_type_as_secondary_decl(): Adding to type_translation_cache: p = %p decl->get_type() = %p = %s \n",p,decl->get_type(),decl->get_type()->class_name().c_str());
#endif
                      // Restore the previous behavior (at least for now, look in to this more later).
                         type_translation_cache[p] = decl->get_type();
                       }
                 // ROSE_ASSERT(type_translation_cache[p] == decl->get_type());
                  }
#endif
            // DQ (8/12/2012): Added tests.
               ROSE_ASSERT(type_translation_cache.find(p) != type_translation_cache.end());
               ROSE_ASSERT(type_translation_cache[p] != NULL);

               result = decl;
               break;
             }

          case tk_class:
             {
#if 0
               printf ("In convert_type_as_secondary_decl(): case tk_class: calling convert_struct_secondary() \n");
#endif

#if 0
            // DQ (12/15/2019): Check if this type has been processed already.
            // Although I have supported it for the template declaration below, I don't know if we should return 
            // a previously declared declaration for a non-template type that was processed.
               if (type_translation_cache.find(p) != type_translation_cache.end())
                  {
#if 0
                    printf (" --- type_translation_cache[p]          = %p \n",type_translation_cache[p]);
                    printf ("type_translation_cache[p] = %p = %s \n",type_translation_cache[p],type_translation_cache[p]->class_name().c_str());

                    printf ("NOTE: If we have previously processed this type and we need the associated ROSE declaration then we should perhaps get the declaration from this type \n");
#endif
#if 0
                    printf ("Exiting as a test! \n");
                    ROSE_ASSERT(false);
#endif
                  }
                 else
                  {
#if 0
                    printf ("p = %p not found within type_translation_cache \n",p);
#endif
                  }
#endif

            // DQ (12/15/2019): Enforce this so that we know that the convert_struct_secondary() function will be called 
            // (except for the non-template case were the type was previously processed).
               ROSE_ASSERT(result == NULL);

            // DQ (12/15/2019): If we can find the existing class declaration then use it directly (at least for a template class).
            // This is a fix for the bug represented in Cxx11_tests/test2019_488.C.
               if (template_type_translation_cache.find(p) != template_type_translation_cache.end())
                  {
#if 0
                    printf (" --- template_type_translation_cache[p] = %p \n",template_type_translation_cache[p]);
                    printf ("template_type_translation_cache[p] = %p = %s \n",template_type_translation_cache[p],template_type_translation_cache[p]->class_name().c_str());
#endif
                    SgClassType* classType = isSgClassType(template_type_translation_cache[p]);
                    ROSE_ASSERT(classType != NULL);

                    SgDeclarationStatement* declarationStatement = classType->get_declaration();
                    ROSE_ASSERT(declarationStatement != NULL);

                    SgClassDeclaration* classDeclaration = isSgClassDeclaration(declarationStatement);
                    ROSE_ASSERT(classDeclaration != NULL);
#if 0
                    printf ("classDeclaration = %p = %s \n",classDeclaration,classDeclaration->class_name().c_str());
#endif
                    SgTemplateClassDeclaration* templateClassDeclaration = isSgTemplateClassDeclaration(classDeclaration);
                    ROSE_ASSERT(templateClassDeclaration != NULL);

                    ROSE_ASSERT(result == NULL);
                    result = templateClassDeclaration;
#if 0
                    printf ("Exiting as a test! \n");
                    ROSE_ASSERT(false);
#endif
                  }
                 else
                  {
#if 0
                    printf ("p = %p not found within template_type_translation_cache \n",p);
#endif
                  }

            // DQ (12/15/2019): If we can find the existing class declaration then use it directly (at least for a template class).
            // This is part of work to support Cxx11_tests/test2019_488.C.
            // result = convert_struct_secondary(p, SgClassDeclaration::e_class);
               if (result == NULL)
                  {
                    result = convert_struct_secondary(p, SgClassDeclaration::e_class);
                  }
                 else
                  {
#if 0
                 // DQ (12/15/2019): I'm not clear if this idea of reusing the declaration assoeicated with the previously 
                 // build type is a god plan for non-template types, but this is especially helpful for the case represented 
                 // by Cxx11_tests/test2019_488.C.
                    printf ("Reusing the existing declaration for result = %p = %s \n",result,result->class_name().c_str());
#endif
                  }
               break;
             }

          case tk_struct:
             {
#if 0
               printf ("In convert_type_as_secondary_decl(): case tk_struct: calling convert_struct_secondary() \n");
#endif
               result = convert_struct_secondary(p, SgClassDeclaration::e_struct);
               break;
             }

          case tk_union:
             {
#if 0
               printf ("In convert_type_as_secondary_decl(): case tk_union: calling convert_struct_secondary() \n");
#endif
               result = convert_struct_secondary(p, SgClassDeclaration::e_union);
               break;
             }

          case tk_typeref: result = convert_typedef(p); break;

          case tk_template_param: {
            auto lp = p->variant.template_param.extra_info->class_type;
            switch (lp->kind) {
              case tk_class: {
                result = convert_struct_secondary(lp, SgClassDeclaration::e_class);
                break;
              }
              case tk_struct: {
                result = convert_struct_secondary(lp, SgClassDeclaration::e_struct);
                break;
              }
              case tk_union: {
                result = convert_struct_secondary(lp, SgClassDeclaration::e_union);
                break;
              }
              default: {
                printf ("Case not handled! kind = %s \n",output_type_enum(kind).c_str());
                ROSE_ASSERT(false);
              }
            }
            break;
          }

          default:
             {
               printf ("Case not handled! kind = %s \n",output_type_enum(kind).c_str());
               ROSE_ASSERT(false);
             }
        }

  // DQ (1/25/2009): This is now set correctly before we get this this point.
  // DQ (1/24/2009): Added assertion.  non-defining declarations in types should not be marked as forward declarations.
  // Only explicitly declared declarations (not used in types) appearing in appropriate scopes can be marked as forward.
  // ROSE_ASSERT(result->isForward() == false);

  // ROSE_ASSERT(source_sequence_secondary_declaration->decl_pos_info != NULL);
     if (source_sequence_secondary_declaration->decl_pos_info != NULL)
        {
          if (isSgTemplateInstantiationDecl(result) == NULL)
             {
            // DQ (7/17/2012): Delete the just setup source position information and re-build it using the secondary source position information.
               delete result->get_startOfConstruct();
               result->set_startOfConstruct(NULL);

               delete result->get_endOfConstruct();
               result->set_endOfConstruct(NULL);
#if 0
               printf ("In convert_type_as_secondary_decl(): This is resetting the source position so make sure that the subtree is consistant \n");
#endif
               result = set_file_info_for_secondary_declaration(result,source_sequence_secondary_declaration);

            // DQ (7/29/2012): We have to fixup the subtree source positions to have the weaving of CPP directives into place work properly.
               fixupSubTreeSourcePositions(result);
             }
            else
             {
#if 0
               printf ("This is a SgTemplateInstantiationDecl (and we don't reset the source position based on the secondary information for these) \n");
#endif
             }
        }
       else
        {
       // When this happens we don't have enough information to reset the source position.
#if 0
          printf ("In convert_type_as_secondary_decl(): source_sequence_secondary_declaration->decl_pos_info == NULL (can't reset the source position) \n");
#endif
       // Unless this is where it is located...
#ifdef DEBUG_CONVERSION
          indent_line(); debug_print("-- secondary start at source_sequence_secondary_declaration->decl_position:"); print_source_pos(source_sequence_secondary_declaration->decl_position); debug_print("\n");
#endif
        }

     if (source_sequence_secondary_declaration->friend_decl == true)
        {
#if 0
          printf ("In convert_type_as_secondary_decl(): secondary declaration is marked as friend_decl == true \n");
#endif
       // result->set_friend(true);
          result->get_declarationModifier().setFriend();
        }

     ROSE_ASSERT(result->get_firstNondefiningDeclaration() != NULL);
     ROSE_ASSERT(getType(result->get_firstNondefiningDeclaration()) != NULL);

  // DQ (9/6/2012): Added support for testing first non-defining, defining, and secondary declarations.
     testDefiningNonDefiningDeclarations(result->get_firstNondefiningDeclaration(),result->get_definingDeclaration(),result,"convert_type_as_secondary_decl()");

  // DQ (2/16/2013): Added support for access modifiers.
     unsigned int access_level = p->source_corresp.access;
     set_access_modifier(result,access_level);

#if 0
     printf ("In convert_type_as_secondary_decl(): result = %p = %s: access = %s \n",result,result->class_name().c_str(),result->get_declarationModifier().get_accessModifier().displayString().c_str());
#endif

     return result;
   }
