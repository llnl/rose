
#include "edgRose.h"

// DQ (9/10/2012): This is a new function to support gathering the template parameter for input to the 
// SageBuilder function (before the SgTemplateClassDeclaration is built, instead of fixing it out afterward).
// This is required because the symbol lookup in the SageBuild functions need the template parameters to 
// resolver disambiguation of templates overloaded by name and disambiguated only by template parameter 
// details (e.g. the number of template parameters, at least, and maybe template parameter positon and depth).
// See test2012_209.C for an example of overloaded templates requiring disambiguation using the number of 
// template parameters.
// SgTemplateParameterPtrList EDG_ROSE_Translation::parse_template_parameters( a_template_parameter_ptr tpp, SgScopeStatment* scope, SgNode* parent )
SgTemplateParameterPtrList
EDG_ROSE_Translation::parse_template_parameters( a_type_ptr p, SgScopeStatement* scope, SgNode* parent )
   {
  // Note that the case of a SgInitializedName requires the scope to be passed.

#define DEBUG_TEMPLATE_PARAMETERS 0

  // SgTemplateParameterPtrList templateParameterList;

#if DEBUG_TEMPLATE_PARAMETERS
     printf ("In parse_template_parameters( a_type_ptr p, SgScopeStatement* scope, SgNode* parent ): p = %p \n",p);
#endif

     a_template_parameter_ptr tpp = NULL;

  // DQ (8/12/2014): Added C++11 support for templated typerefs (see Cxx11_tests/test2014_38.C).
  // Only execute if we are a class, struct, or union.
     if (p->kind == tk_class || p->kind == tk_struct || p->kind == tk_union)
        {
          ROSE_ASSERT(p->variant.class_struct_union.extra_info != NULL);
          if (p->variant.class_struct_union.extra_info->assoc_template != NULL)
             {
#if DEBUG_TEMPLATE_PARAMETERS
               printf ("In parse_template_parameters(): Setting template parameters: found assoc_template = %p \n",p->variant.class_struct_union.extra_info->assoc_template);
#endif
            // DQ (2/11/2012): Check if we have a valid template_decl and only build template parameter list if so.
            // See test2012_02.C for a case where this is important (also in STL example test2004_35.C).
            // ROSE_ASSERT(p->variant.class_struct_union.extra_info->assoc_template->template_decl != NULL);
               if (p->variant.class_struct_union.extra_info->assoc_template->template_decl != NULL)
                  {
                    tpp = p->variant.class_struct_union.extra_info->assoc_template->template_decl->param_list;
                  }
             }
        }

     ROSE_ASSERT(scope != NULL);

     if (tpp != NULL)
        {
#if DEBUG_TEMPLATE_PARAMETERS
           printf ("In parse_template_parameters( a_type_ptr p, SgScopeStatement* scope, SgNode* parent ): Calling parse_template_parameters(): returned value \n");
#endif
          return parse_template_parameters(tpp,scope,parent);
        }

     return SgTemplateParameterPtrList();
   }

// DQ (8/8/2013): This is the support for template parameters specific to functions and member functions.
SgTemplateParameterPtrList
EDG_ROSE_Translation::parse_template_parameters( a_routine_ptr p, SgScopeStatement* scope, SgNode* parent )
   {
  // Note that the case of a SgInitializedName requires the scope to be passed.

  // SgTemplateParameterPtrList templateParameterList;

#if DEBUG_TEMPLATE_PARAMETERS
     printf ("In parse_template_parameters( a_routine_ptr p, SgScopeStatement* scope, SgNode* parent ): p = %p \n",p);
#endif

     a_template_parameter_ptr tpp = NULL;

     if (p->assoc_template != NULL)
        {
#if 0
          printf ("In parse_template_parameters(): Setting template parameters: found assoc_template = %p \n",p->assoc_template);
#endif
       // DQ (2/11/2012): Check if we have a valid template_decl and only build template parameter list if so.
       // See test2012_02.C for a case where this is important (also in STL example test2004_35.C).
       // ROSE_ASSERT(p->variant.class_struct_union.extra_info->assoc_template->template_decl != NULL);
          if (p->assoc_template->template_decl != NULL)
             {
               tpp = p->assoc_template->template_decl->param_list;
             }
        }
       else
        {
#if 0
          printf ("In parse_template_parameters(): p->assoc_template == NULL: returning an empty list \n");
#endif
        }

     ROSE_ASSERT(scope != NULL);

     if (tpp != NULL)
        {
#if DEBUG_TEMPLATE_PARAMETERS
          printf ("In parse_template_parameters(): calling and returning result of parse_template_parameters(tpp,scope,parent) \n");
#endif
          return parse_template_parameters(tpp,scope,parent);
        }

#if 0
     printf ("In parse_template_parameters(): tpp == NULL: returning an empty list \n");
#endif

     return SgTemplateParameterPtrList();
   }

SgTemplateParameterPtrList
EDG_ROSE_Translation::parse_template_parameters( a_template_parameter_ptr tpp, SgScopeStatement* scope, SgNode* parent )
   {

  // This was not asserted in the previous verison of this function.
  // We can't assert this for a forward template declaration.
  // ROSE_ASSERT(scope != NULL); // TV (05/04/2018): `scope` is never referenced in this function

#if DEBUG_TEMPLATE_PARAMETERS
     printf ("In parse_template_parameters( a_template_parameter_ptr tpp, SgScopeStatement* scope, SgNode* parent ): tpp = %p parent = %p \n",tpp,parent);
     if (parent != NULL)
        {
          printf ("   --- parent = %p = %s \n",parent,parent->class_name().c_str());
        }
     if (scope != NULL)
        {
          printf ("   --- scope = %p = %s \n",scope,scope->class_name().c_str());
        }
#endif

#if 0
  // DQ (8/12/2013): This is done now!
     printf ("########################################################################################################################################### \n");
     printf ("##### We might need a cache for template parameters (SgTemplateParameter) similar to that for template arguments (SgTemplateArgument) ##### \n");
     printf ("########################################################################################################################################### \n");
#endif

  // DQ (7/23/2017): Before processing any new template parameter list, we should have absorbed 
  // any existing entries and associated them with their associated template declaration. This 
  // test should help debug where we are getting the mangled name incorrect because the the 
  // declaration scope is not being handled properly.
     ROSE_ASSERT(internal_intermediate_scope != NULL);
     bool first_template_parameter = true;
  // Save the original size, since we want to make it an error to only add new entries.
     size_t original_intermediate_scope_size = 0;
     if (tpp != NULL)
        {
          original_intermediate_scope_size = internal_intermediate_scope->symbol_table_size();
          if (internal_intermediate_scope->symbol_table_size() != 0)
             {
#if DEBUG_TEMPLATE_PARAMETERS
               printf ("@@@@@@@@@@@@@@@@@@@ WARNING: In parse_template_parameters(): the declaration scope has already been setup with classes constructed from parameters of another template declaration \n");
               printf ("   --- internal_intermediate_scope->symbol_table_size() = %zu (non-zero) \n",internal_intermediate_scope->symbol_table_size());
#endif
             }
            else
             {
#if DEBUG_TEMPLATE_PARAMETERS
               printf ("@@@@@@@@@@@@@@@@@@@ In parse_template_parameters(): internal_intermediate_scope->symbol_table_size() = %zu (zero) \n",internal_intermediate_scope->symbol_table_size());
#endif
             }
       // ROSE_ASSERT(internal_intermediate_scope->symbol_table_size() == 0);
        }
       else
        {
#if DEBUG_TEMPLATE_PARAMETERS
          printf ("@@@@@@@@@@@@@@@@@@@ In parse_template_parameters(): tpp == NULL: so internal_intermediate_scope->symbol_table_size() = %zu (can be non-zero) \n",internal_intermediate_scope->symbol_table_size());
#endif
        }

     SgTemplateParameterPtrList templateParameterList;

     while (tpp != NULL)
        {
#if DEBUG_TEMPLATE_PARAMETERS
          printf ("In parse_template_parameters(): Processing a template parameter (in while loop) tpp = %p kind = %s \n",tpp,summarize_template_parameter_kind_in_string(tpp));
#endif
          SgTemplateParameter* result_templateParameter = NULL;

       // DQ (8/12/2013): Adding support for caching of template parameters so that we will have a one-to-one 
       // corespondence with edg a_template_parameter_ptr values and thus simplify the symbol table lookup of 
       // template functions and classes that have equivalent template parameters in there template parameter 
       // lists.  This avoid the complexity of handling equivalent of arbitrary expressions and matches the 
       // implementation design for template arguments.
       // DQ (1/30/2013): We are using the SgTemplateParameter pointers in the symbol lookup for template 
       // declarations, so we need to make sure that each different EDG a_template_parameter_ptr only maps 
       // to a single SgTemplateParameter object.  So we check this here.
          if (processedTemplateParameterCache.find(tpp) != processedTemplateParameterCache.end())
             {
               result_templateParameter = processedTemplateParameterCache[tpp];

               if (parent != NULL && result_templateParameter->get_parent() == NULL) {
                 result_templateParameter->set_parent(parent);
               } else if (parent != NULL && result_templateParameter->get_parent() != NULL) {
#if DEBUG_TEMPLATE_PARAMETERS
                 printf ("In parse_template_parameters():\n");
                 printf (" --- parent = %p (%s)\n", parent, parent ? parent->class_name().c_str() : "");
                 printf (" --- result_templateParameter->get_parent() = %p (%s)\n", result_templateParameter->get_parent(), result_templateParameter->get_parent() ? result_templateParameter->get_parent()->class_name().c_str() : "");
#endif
                 SgDeclarationStatement * declstmt = isSgDeclarationStatement(parent);
                 ROSE_ASSERT(declstmt != NULL);
                 SgDeclarationStatement * rparent = isSgDeclarationStatement(result_templateParameter->get_parent());
                 ROSE_ASSERT(rparent != NULL);
                 if (isSgNonrealDecl(parent) && isSgNonrealDecl(rparent)) {
                   SgNonrealDecl * nr_parent = isSgNonrealDecl(parent);
                   SgNonrealDecl * nr_rparent = isSgNonrealDecl(rparent);

#if DEBUG_TEMPLATE_PARAMETERS
                   printf ("  nr_parent = %p : %s\n", nr_parent, nr_parent ? nr_parent->get_name().getString().c_str() : "");
                   printf ("  nr_rparent = %p : %s\n", nr_rparent, nr_rparent ? nr_rparent->get_name().getString().c_str() : "");
#endif
                   ROSE_ASSERT(nr_parent->get_name() == nr_rparent->get_name());
//                 printf ("WARNING: Case of a template parameter whose parent is a SgNonrealDecl of same name as the prescribed parent. Need minimal test case.\n");
                 } else {
                   ROSE_ASSERT(declstmt->get_firstNondefiningDeclaration() == rparent->get_firstNondefiningDeclaration());
                 }
               }
#if 0
               printf ("In parse_template_parameters(): Retriving the template parameter from the cache of previously process template parameter: tpp = %p result_templateParameter = %p \n",tpp,result_templateParameter);
               printf ("In parse_template_parameters(): result_templateParameter = %s \n",result_templateParameter->unparseToString().c_str());
#endif
             }
            else
             {
            // DQ (9/24/2012): If we want to output something for this function we need t output more than this
            // (causes what appears to be surious output, difficult to trace to this functions).
            // if (tpp != NULL) {print_source_corresp(tpp->source_corresp);}
            // debug_print("\n");

               SgTemplateParameter::template_parameter_enum parameter_kind = SgTemplateParameter::parameter_undefined;

               SgType*                typeParameter              = NULL;
               SgType*                defaultTypeParameter       = NULL;

               SgExpression*          expressionParameter        = NULL; 
               SgExpression*          defaultExpressionParameter = NULL;

            // SgTemplateDeclaration* templateParameter          = NULL;
            // SgTemplateDeclaration* defaultTemplateParameter   = NULL;
               SgDeclarationStatement* templateParameter         = NULL;
               SgDeclarationStatement* defaultTemplateParameter  = NULL;

            // DQ (11/21/2011): Added new support to handle template parameters that are "int U = 42" (kind of a constant valued declaration).
               SgInitializedName*     initializedNameParameter   = NULL;
#if 0
               printf ("In parse_template_parameters(): calling summarize_template_parameter() \n");
               summarize_template_parameter (tpp);
#endif
               switch (tpp->kind)
                  {
                    case tpk_type:
                       {
                         parameter_kind = SgTemplateParameter::type_parameter;

                         a_type_ptr edg_type        = tpp->variant.type.ptr;
                         a_type_ptr edg_default_arg = tpp->variant.type.default_arg_type;

                      // SgType* parameter = sage_form_type( tpp->variant.type.ptr, octl, /*a_SgClassDefinition_ptr under_a_class=*/NULL, definingDeclaration);
                         ROSE_ASSERT(edg_type != NULL);

                         typeParameter = convert_type(edg_type);

//                       typeParameter = convert_nonreal_type(edg_type, false, false, NULL);

#if DEBUG_CLASSES_FROM_TEMPLATE_PARAMETERS || DEBUG_TEMPLATE_PARAMETERS || 0
                         size_t numberOfClassesBuiltFromTemplateParameters_1 = internal_intermediate_scope->symbol_table_size();
                         printf ("################## In parse_template_parameters(): after convert_type(): numberOfClassesBuiltFromTemplateParameters = %zu \n",numberOfClassesBuiltFromTemplateParameters_1);
#endif
#if DEBUG_CLASSES_FROM_TEMPLATE_PARAMETERS || DEBUG_TEMPLATE_PARAMETERS || 0
                         printf ("In parse_template_parameters(): typeParameter = %p = %s \n",typeParameter,typeParameter->class_name().c_str());
#endif

                         if (first_template_parameter == true && original_intermediate_scope_size > 0 && internal_intermediate_scope->symbol_table_size() > original_intermediate_scope_size)
                            {
                           // DQ (7/23/2017): If this happens, then we may need to keep a stack of SgDeclarationScope IR nodes so that
                           // we can handle classes generated from template parameters while processing nested templat declarations.
                           // I think an example of this is test2017_28.C
                              printf ("ERROR: class added to a declaration scope that was being used for a previous template declaration: original_intermediate_scope_size = %zu \n",original_intermediate_scope_size);
#if 1
                              printf ("Exiting as a test! \n");
                              ROSE_ASSERT(false);
#endif
                            }

                         first_template_parameter = false;

                      // DQ (7/16/2017): Find a way to mark this type as being generated from a parameter so that we can fixup the scope 
                      // to be a declaration scope (new concept) and also make sure that it will not use name qualification when ask for 
                      // it's fully qualified name.  Also, in the case of a nonreal class (that is also a prozy class?) the associated declaration 
                      // will be given a global scope (which is always incorrect).  But since we don't have the declaration until after the 
                      // template parameters are constructed, we can't fix this up here and must defer it to later processing.

                      // TV (03/30/2018): Now all types coming from Template Parameter are SgNonrealType
                         ROSE_ASSERT(isSgNonrealType(typeParameter) != NULL);

                         if (edg_default_arg != NULL)
                            {
                           // DQ (7/18/2017): Note that this might be another location where a class migh be generated from a template parameter.
#if DEBUG_CLASSES_FROM_TEMPLATE_PARAMETERS || DEBUG_TEMPLATE_PARAMETERS || 0
                              printf ("Default parameter found (case tpk_type) \n");
#endif
                           // defaultParameter = sage_form_type( tpp->variant.type.default_arg_type, octl, /*a_SgClassDefinition_ptr under_a_class=*/NULL, definingDeclaration);
                              defaultTypeParameter = convert_type(edg_default_arg);
                            }
                         break;
                       }

                    case tpk_nontype:
                       {
                         parameter_kind = SgTemplateParameter::nontype_parameter;

                         a_constant_ptr edg_constant    = tpp->variant.nontype.constant;
                         a_constant_ptr edg_default_arg = tpp->variant.nontype.default_arg_constant;

                         ROSE_ASSERT(edg_constant != NULL);
#if DEBUG_TEMPLATE_PARAMETERS
                         printf ("In parse_template_parameters(): expressionParameter = %p (%s)\n", expressionParameter, expressionParameter ? expressionParameter->class_name().c_str() : "");
#endif
                         if (edg_constant->kind == ck_template_param) {
                           SgVarRefExp * vref = isSgVarRefExp(convert_constant(edg_constant));
                           ROSE_ASSERT(vref != NULL);
                           
                           SgVariableSymbol * vsym = vref->get_symbol();
                           ROSE_ASSERT(vsym != NULL);

                           initializedNameParameter = vsym->get_declaration();
                           ROSE_ASSERT(initializedNameParameter != NULL);

                           if (edg_default_arg != NULL) {
                             SgAssignInitializer * assign_init = SageBuilder::buildAssignInitializer(convert_constant(edg_default_arg));
                             initializedNameParameter->set_initializer(assign_init);
                             assign_init->set_parent(initializedNameParameter);
                           }
                         } else {
                           ROSE_ASSERT(false); // TV: just checking...
                         }
                         break;
                       }

                    case tpk_template:
                       {
                         parameter_kind = SgTemplateParameter::template_parameter;

                      // The placeholder template representing the parameter
                         a_template_ptr edg_template    = tpp->variant.templ.class_template;
                         a_template_ptr edg_default_arg = tpp->variant.templ.default_arg_template;

                      // templateParameter = convert_template(edg_template);
                         templateParameter = convert_template_prototype(edg_template);

                         if (edg_default_arg != NULL)
                            {
#if 0
                              printf ("Default parameter found (case tpk_template) \n");
#endif
                              defaultTemplateParameter = convert_template_prototype(edg_default_arg);
                            }
                         break;
                       }

                    case tpk_error:
                       {
                         printf ("Error: case tpk_error reached in parse_template_parameters \n");
                         ROSE_ASSERT (false);
                         break;
                       }

                    default:
                       {
                         printf ("Error: Default reached in parse_template_parameters \n");
                         ROSE_ASSERT (false);
                         break;
                       }
                  }

            // Make sure this is not an undefined value.
               ROSE_ASSERT(parameter_kind != SgTemplateParameter::parameter_undefined);

               result_templateParameter = new SgTemplateParameter(parameter_kind, typeParameter, defaultTypeParameter, expressionParameter, defaultExpressionParameter, templateParameter, defaultTemplateParameter, initializedNameParameter);
#if DEBUG_TEMPLATE_PARAMETERS
               printf ("In parse_template_parameters(): building result_templateParameter = %p from tpp = %p \n",result_templateParameter,tpp);
#endif

            // Make sure this is not an undefined value.
               ROSE_ASSERT(result_templateParameter->get_parameterType() != SgTemplateParameter::parameter_undefined);

               if (initializedNameParameter != NULL)
                  {
                    initializedNameParameter->set_parent(result_templateParameter);
                  }


            // DQ (7/25/2012): Enforce this.
               if (expressionParameter != NULL)
                  {
                    ROSE_ASSERT(expressionParameter->get_parent() != NULL);
                  }

            // DQ (7/25/2012): Enforce this.
               if (defaultExpressionParameter != NULL)
                  {
                    ROSE_ASSERT(defaultExpressionParameter->get_parent() != NULL);
                  }

            // DQ (7/25/2012): Enforce this.
               if (templateParameter != NULL)
                  {
                    ROSE_ASSERT(templateParameter->get_parent() != NULL);
                  }

            // DQ (7/25/2012): Enforce this.
               if (defaultTemplateParameter != NULL)
                  {
                    ROSE_ASSERT(defaultTemplateParameter->get_parent() != NULL);
                  }

            // result_templateParameter->set_parent(templateDeclaration);
               result_templateParameter->set_parent(parent);

            // DQ (8/14/2013): The source position information could be added to the template parameter 
            // (except that ROSE does not store a Sg_File_Info object in the SgTemplateParameter.  This 
            // could be fixed in the future. Note that template arguments have not associated source 
            // position information so it is not possible there.
            // printf ("WARNING: In parse_template_parameters(): source position information for SgTemplateParameter is available and not set! \n");
            // set_file_info_for_corresp(result_templateParameter,tpp->source_corresp);
#if 0
               printf ("In parse_template_parameters(): result_templateParameter = %s \n",SageInterface::get_name(result_templateParameter).c_str());
#endif
            // DQ (8/12/2013): Add the template parameter to the cache.
               processedTemplateParameterCache[tpp] = result_templateParameter;
             }

       // Add to the list.
          templateParameterList.push_back(result_templateParameter);

       // DQ (8/8/2013): This function is not implemented.
       // printf ("In parse_template_parameters(): result_templateParameter = %s \n",result_templateParameter->unparseToString().c_str());

          tpp = tpp->next;
        }

// #if 0
#if DEBUG_STRUCTURAL_DESIGN || DEBUG_TEMPLATE_PARAMETERS || 0
     printf ("Leaving parse_template_parameters( a_template_parameter_ptr tpp, SgScopeStatement* scope, SgNode* parent ) \n");
#endif

#if 0
     printf ("Exiting as a test! \n");
     ROSE_ASSERT(false);
#endif

     return templateParameterList;
   }
