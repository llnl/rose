
#include "edgRose.h"

void EDG_ROSE_Translation::convert_ctor_initializers ( a_routine_ptr p, SgCtorInitializerList* ctor_initialization_list )
   {
  // This is the constructor preinitialization list handling.

#ifdef DEBUG_CONVERSION
     indent_line(); debug_print("convert_ctor_initializers\n");
     ScopedIndent si(indent);
#endif

     ROSE_ASSERT(ctor_initialization_list != NULL);

#if 0
     printf ("In convert_ctor_initializers(): p = %p ctor_initialization_list = %p \n",p,ctor_initialization_list);
#endif
  // Code to permit this file to compile, as long as it will not run.
     a_scope_ptr scope = NULL;
     if (p->function_def_number != NULL_function_def_number)
        {
       // Note that p->assoc_scope is an EDG specific scope number (not a pointer to a scope).
          scope = il_header.function_def_table[p->function_def_number].scope;
        }
     if (scope == NULL)
        {
       // DQ (4/23/2017): Converted to used message streams (appears only for case of where Microsoft extensions are used).
          mprintf ("In convert_ctor_initializers(): p->function_def_number = %d \n",(int)p->function_def_number);
        }

     if (scope == NULL)
        {
#if 0
          printf ("In convert_ctor_initializers(): calling summarize_routine() \n");
          summarize_routine(p);
#endif
       // DQ (8/14/2014): For MS Windows search for this alternative location to find the scope.
          scope = p->source_corresp.parent_scope;
          ROSE_ASSERT(scope != NULL);
          if (scope->kind != sck_function)
             {
#if 0
               printf ("There is not associated definition for this function yet, so we can return early! \n");
#endif
               return;
             }
        }
     ROSE_ASSERT(scope != NULL);
     ROSE_ASSERT(scope->kind == sck_function);
     a_constructor_init_ptr ctor_init = scope->variant.routine.constructor_inits;

#if 0
     printf ("ctor_init is in the scope: scope->variant.routine.constructor_inits = %p \n",scope->variant.routine.constructor_inits);
#endif

     if (ctor_init == NULL)
        {
       // DQ (12/28/2011): Let's see if we can assert this? No, it fails for test2001_01.C.
#if 0
          printf ("There is no a_constructor_init_ptr ctor_init for this function ctor_initialization_list = %p \n",ctor_initialization_list);
#endif
          return;
        }

#if 0
     printf ("In convert_ctor_initializers(): calling summarize_constructor_init(p = %p): \n",ctor_init);
     summarize_constructor_init (ctor_init);
#endif
     a_field_ptr field      = NULL;
#if DEBUG_GET_DECLARATION_SCOPE
     printf ("In convert_ctor_initializers(): calling getDeclarationScope() \n");
#endif

     SgScopeStatement* scopeOfDeclaration = getDeclarationScope(p->source_corresp);
     ROSE_ASSERT(scopeOfDeclaration != NULL);

#if 0
     printf ("In convert_ctor_initializers(): scopeOfDeclaration = %p = %s \n",scopeOfDeclaration,scopeOfDeclaration->class_name().c_str());
#endif

     ROSE_ASSERT(isSgClassDefinition(scopeOfDeclaration) != NULL || isSgDeclarationScope(scopeOfDeclaration) != NULL);

     for (; ctor_init != NULL; ctor_init = ctor_init->next)
        {
          ROSE_ASSERT (ctor_init != NULL);

#ifdef DEBUG_CONVERSION
          indent_line(); debug_print("ctor initializer at "); print_source_range(ctor_init->ctor_init_range); debug_print("\n");
#endif
          SgInitializedName* preconstruction_initializer = NULL;
#if 0
          printf ("##### TOP of loop over ctor initializers ctor_init->compiler_generated = %s \n",ctor_init->compiler_generated ? "true" : "false");
#endif
          if (ctor_init->compiler_generated == false)
             {
               switch (ctor_init->kind)
                  {
                    case cik_virtual_base_class:
                    case cik_direct_base_class:
                       {
                      // Initializing a base class.

                      // DQ (12/30/2011): For the case of template classes this is harder to debug since we output the original template string.

                         a_type_ptr edg_type = ctor_init->variant.base_class->type;
                         ROSE_ASSERT(edg_type != NULL);
                         a_dynamic_init_ptr edg_initializer = ctor_init->initializer;
                         ROSE_ASSERT(edg_initializer != NULL);
#if 0
                         printf ("building the base_class_type \n");
#endif
                         SgType* base_class_type = convert_type(edg_type);
                         ROSE_ASSERT(base_class_type != NULL);

                         SgClassType * class_type = isSgClassType(base_class_type);
                         SgNonrealType * nr_type = isSgNonrealType(base_class_type);

                         ROSE_ASSERT(class_type != NULL || nr_type != NULL);

                         SgName name;
                         if (class_type != NULL) {
                           SgClassDeclaration* local_classDeclaration = isSgClassDeclaration(class_type->get_declaration());
                           ROSE_ASSERT (local_classDeclaration != NULL);
                           
                           ROSE_ASSERT (local_classDeclaration->get_firstNondefiningDeclaration() != NULL);
                           ROSE_ASSERT (local_classDeclaration->get_firstNondefiningDeclaration() == local_classDeclaration);

                           name = local_classDeclaration->get_name();
                         } else if (nr_type != NULL) {
                           SgNonrealDecl * local_classDeclaration = isSgNonrealDecl(nr_type->get_declaration());
                           ROSE_ASSERT (local_classDeclaration != NULL);

//                         ROSE_ASSERT (local_classDeclaration->get_firstNondefiningDeclaration() != NULL);
//                         ROSE_ASSERT (local_classDeclaration->get_firstNondefiningDeclaration() == local_classDeclaration);

                           name = local_classDeclaration->get_name();
                         } else {
                           ROSE_ASSERT(false);
                         }

                      // SgInitializer* base_class_initializer = NULL;
#if 0
                         printf ("building the base_class_initializer_expression \n");
#endif
                         SgExpression* base_class_initializer_expression = convert_dynamic_initializer(ctor_init->initializer,edg_type);
                         ROSE_ASSERT(base_class_initializer_expression != NULL);
#if 0
                         printf ("base_class_initializer_expression = %p = %s \n",base_class_initializer_expression,base_class_initializer_expression->class_name().c_str());
#endif
                         SgInitializer* base_class_initializer = isSgInitializer(base_class_initializer_expression);
                         if (base_class_initializer == NULL)
                            {
                           // Need to wrap this in an SgAssignInitalizer.
#if 0
                              printf ("In convert_ctor_initializers(): case cik_direct_base_class: calling buildAssignInitializer_nfi() base_class_initializer_expression = %p \n",base_class_initializer_expression);
#endif
                              base_class_initializer = buildAssignInitializer_nfi(base_class_initializer_expression);
                            }
                         ROSE_ASSERT(base_class_initializer != NULL);

                      // DQ (3/216/2012): Calling get_type to generate type internally (actually we should not need the field internally).
#if 0
                         printf ("Calling SgInitializer::get_type to generate type internally \n");
#endif
                         ROSE_ASSERT(base_class_initializer->get_type() != NULL);
#if 0
                      // DQ (12/5/2011): This is an obscure detail...
                         SgConstructorInitializer* constructorInitializer = isSgConstructorInitializer(base_class_initializer);
                         if (constructorInitializer != NULL)
                            {
                              constructorInitializer->set_need_parenthesis_after_name(true);
                            }
#endif
#if 0
                         printf ("building the initializedName \n");
#endif
                         SgInitializedName* initializedName = buildInitializedName(name, base_class_type, base_class_initializer);

                      // DQ (2/25/2012): Added test.
                         ROSE_ASSERT(base_class_initializer->get_parent() != NULL);
#if 0
                         printf ("set the scope of the initializedName \n");
#endif
                         initializedName->set_scope(scopeOfDeclaration);
                         ROSE_ASSERT(initializedName->get_scope() != NULL);

                         preconstruction_initializer = initializedName;
#if 0
                         printf ("Error: cik_direct_base_class not supported \n");
                         ROSE_ASSERT(false);
#endif
                         break;
                       }

                    case cik_field:
                       {
                      /* Initializing a nonstatic data member. */
#if 0
                         printf ("Processing a preinitialization element: cik_field \n");
#endif
                         field = ctor_init->variant.field;
                         ROSE_ASSERT(field != NULL);

                         a_type_ptr edg_field_type = field->type;

                         ROSE_ASSERT(edg_field_type != NULL);

                         SgName name = field->source_corresp.name;
#if 0
                         printf ("building the field_initializer_expression: name = %s \n",name.str());
#endif
                         SgExpression* field_initializer_expression = convert_dynamic_initializer(ctor_init->initializer,edg_field_type);
                         ROSE_ASSERT(field_initializer_expression != NULL);
#if 0
                         printf ("field_initializer_expression = %p = %s \n",field_initializer_expression,field_initializer_expression->class_name().c_str());
#endif
                         SgInitializer* field_initializer = isSgInitializer(field_initializer_expression);
                         if (field_initializer == NULL)
                            {
                           // Need to wrap this in an SgAssignInitalizer.
#if 0
                              printf ("In convert_ctor_initializers(): case cik_field: calling buildAssignInitializer_nfi() \n");
#endif
                              field_initializer = buildAssignInitializer_nfi(field_initializer_expression);
                            }
                         ROSE_ASSERT(field_initializer != NULL);
#if 0
                         printf ("field_initializer = %p = %s \n",field_initializer,field_initializer->class_name().c_str());
#endif
                      // DQ (12/5/2011): This is an obscure detail...
                         SgConstructorInitializer* constructorInitializer = isSgConstructorInitializer(field_initializer);
                         if (constructorInitializer != NULL)
                            {
                              constructorInitializer->set_need_parenthesis_after_name(true);
                            }
#if 0
                         printf ("building the field_initializer \n");
#endif
                      // SgInitializer* field_initializer = buildAssignInitializer_nfi(field_initializer_expression);
                      // ROSE_ASSERT(isSgInitializer(field_initializer != NULL);
                      // SgInitializer* field_initializer = field_initializer;
                      // ROSE_ASSERT(field_initializer != NULL);
#if 0
                         printf ("building the field_type \n");
#endif
                         SgType* field_type = convert_type(edg_field_type);
                         ROSE_ASSERT(field_type != NULL);
#if 0
                         printf ("building the initializedName \n");
#endif
                         SgInitializedName* initializedName = buildInitializedName(name, field_type, field_initializer);

                      // DQ (2/25/2012): Added test.
                         ROSE_ASSERT(field_initializer != NULL);
                         ROSE_ASSERT(field_initializer->get_parent() != NULL);
#if 0
                         SgScopeStatement* scopeOfDeclaration = getDeclarationScope(field->source_corresp);
                         ROSE_ASSERT(scopeOfDeclaration != NULL);

                         printf ("In convert_ctor_initializers(): (cik_field) scopeOfDeclaration = %p = %s \n",scopeOfDeclaration,scopeOfDeclaration->class_name().c_str());
#endif
                         ROSE_ASSERT(initializedName != NULL);
                         ROSE_ASSERT(initializedName->get_type() != NULL);
#if 0
                         printf ("set the scope of the initializedName \n");
#endif
                         initializedName->set_scope(scopeOfDeclaration);
                         ROSE_ASSERT(initializedName->get_scope() != NULL);

                         preconstruction_initializer = initializedName;

                      // SgVariableSymbol* variableSymbol = convert_field_use(field);
                      // ROSE_ASSERT(variableSymbol != NULL);

                      // DQ (11/6/2014): Adding C++11 support to record the use of C++11 syntax (for the unparser).
                      // TV (06/27/2018): we are not translating the field but the ctor initializer of the field
                      // if (field->has_direct_braced_initializer == true)
                         if (ctor_init->is_braced == true)
                            {
                              field_initializer->set_is_braced_initialized(true);
                           // TV: only set the flag for the initializer, not the initialized name (which is an artifact of the translation of the ctor init.)
                           // initializedName->set_is_braced_initialized(true);
                            }

#if 0
                         printf ("Error: cik_field not supported \n");
                         ROSE_ASSERT(false);
#endif
                         break;
                       }

                 // DQ (11/6/2014): Added new case that is C++11 specific (I think).
                    case cik_delegation:
                       {
                      // Initialization is delegated to another constructor.
#if 0
                         printf ("In convert_ctor_initializers(): case cik_delegation: calling summarize_constructor_init() \n");
                         summarize_constructor_init(ctor_init);
#endif
#if 0
                         printf ("building the base_class_initializer_expression \n");
#endif
                         ROSE_ASSERT(ctor_init->initializer != NULL);
                         ROSE_ASSERT(ctor_init->orig_type != NULL);

                         SgType* delegate_type = convert_type(ctor_init->orig_type);
                         ROSE_ASSERT(delegate_type != NULL);

                         SgName name;

                         SgClassType * class_type = isSgClassType(delegate_type);
                         SgTypedefType * td_type = isSgTypedefType(delegate_type);
                         if (class_type != NULL) {
                           ROSE_ASSERT(isSgClassDeclaration(class_type->get_declaration()) != NULL);
                           name = isSgClassDeclaration(class_type->get_declaration())->get_name();
                         } else if (td_type != NULL) {
                           ROSE_ASSERT(isSgTypedefDeclaration(td_type->get_declaration()) != NULL);
                           name = isSgTypedefDeclaration(td_type->get_declaration())->get_name();
                         } else {
                           printf("  delegate_type = %p (%s)\n", delegate_type, delegate_type ? delegate_type->class_name().c_str() : "");
                           ROSE_ASSERT(false);
                         }
#if 0
                         printf ("In convert_ctor_initializers(): case cik_delegation: name = %s \n",name.str());
#endif
                         SgExpression* initializer_expression = convert_dynamic_initializer(ctor_init->initializer,ctor_init->orig_type);
                         ROSE_ASSERT(initializer_expression != NULL);
#if 0
                         printf ("initializer_expression = %p = %s \n",initializer_expression,initializer_expression->class_name().c_str());
#endif
                         SgInitializer* initializer = isSgInitializer(initializer_expression);
                      // TV (09/14/2018): That fixes rose-1353
                         if (initializer == NULL)
                            {
                           // Need to wrap this in an SgAssignInitalizer.
#if 0
                              printf ("In convert_ctor_initializers(): case cik_delegation: calling buildAssignInitializer_nfi() \n");
#endif
                              initializer = buildAssignInitializer_nfi(initializer_expression);
                            }
                         ROSE_ASSERT(initializer != NULL);

                      // DQ (12/5/2011): This is an obscure detail...
                         SgConstructorInitializer* constructorInitializer = isSgConstructorInitializer(initializer);
                         if (constructorInitializer != NULL)
                            {
                              constructorInitializer->set_need_parenthesis_after_name(true);
                            }
#if 0
                         printf ("building the initializedName \n");
#endif
                         SgInitializedName* initializedName = buildInitializedName(name, delegate_type, initializer);

                      // DQ (2/25/2012): Added test.
                         ROSE_ASSERT(initializer->get_parent() != NULL);
#if 0
                         printf ("set the scope of the initializedName \n");
#endif
                         initializedName->set_scope(scopeOfDeclaration);
                         ROSE_ASSERT(initializedName->get_scope() != NULL);

                         preconstruction_initializer = initializedName;
#if 0
                         printf ("Error: cik_delegation not supported \n");
                         ROSE_ASSERT(false);
#endif
                         break;
                       }

                    default:
                       {
                         printf ("In convert_ctor_initializers(): default reached \n");
                         ROSE_ASSERT (false);
                       }
                  }

               ROSE_ASSERT(preconstruction_initializer != NULL);
               ROSE_ASSERT(preconstruction_initializer->get_preinitialization() == SgInitializedName::e_unknown_preinitialization);
               switch (ctor_init->kind )
                  {
                    case cik_virtual_base_class:
                         preconstruction_initializer->set_preinitialization(SgInitializedName::e_virtual_base_class);
                         break;

                    case cik_direct_base_class:
                         preconstruction_initializer->set_preinitialization(SgInitializedName::e_nonvirtual_base_class);
                         break;

                    case cik_field:
                      /* Initializing a nonstatic data member. */
                         preconstruction_initializer->set_preinitialization(SgInitializedName::e_data_member);
                         break;

                 // DQ (11/6/2014): Added new case that is C++11 specific (I think).
                    case cik_delegation:
                      /* Initializing a delegation constructor. */
                         preconstruction_initializer->set_preinitialization(SgInitializedName::e_delegation_constructor);
                         break;

                    default:
                         printf ("In convert_ctor_initializers(): preconstruction_initializer->set_preinitialization() default reached \n");
                         ROSE_ASSERT (false);
                  }
               ROSE_ASSERT(preconstruction_initializer != NULL);
#if 0
               printf ("Appending the preinitialization element to the list \n");
#endif
               ctor_initialization_list->get_ctors().push_back(preconstruction_initializer);
             }
#if 0
          printf ("##### BOTTOM of loop over ctor initializers ctor_init->compiler_generated = %s \n",ctor_init->compiler_generated ? "true" : "false");
#endif
        }
#if 0
      printf ("Leaving convert_ctor_initializers() \n");
#endif
   }
