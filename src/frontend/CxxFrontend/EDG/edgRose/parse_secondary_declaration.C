
#include "edgRose.h"

#define DEBUG__parse_secondary_declaration 0

namespace EDG_ROSE_Translation {

SgDeclarationStatement * parse_secondary_declaration(SourceSequenceList& sse)
   {
     a_src_seq_secondary_decl_ptr p = (a_src_seq_secondary_decl_ptr)(sse->entity.ptr);
     an_il_entry_kind kind = (an_il_entry_kind)(p->entity.kind);
#if DEBUG__parse_secondary_declaration
     printf ("ENTER parse_secondary_declaration():\n");
     printf ("  p = %p\n", p);
     printf ("  kind = %s\n", il_entry_kind_names[kind]);
#endif

     a_seq_number secondary_source_sequence_number = p->decl_position.seq;
     global_source_sequence_number = secondary_source_sequence_number;

     SgDeclarationStatement* result = NULL;
     switch (kind) {
          case iek_type:
             {
               if (!p->declared_in_func_prototype) {
                 result = convert_type_as_secondary_decl(sse);
               }
               ++sse;
               break;
             }

          case iek_routine:
             {
               a_routine_ptr edg_routine = (a_routine_ptr)(p->entity.ptr);
               result = convert_routine_prototype(edg_routine);

            // DQ (8/13/2020): Enforce that all member declarations are assigned an access.
               ROSE_ASSERT(result->get_declarationModifier().get_accessModifier().isDefault() == false);
               ++sse;

            // DQ (3/7/2012): Added testing (debugging test2004_77.C).
               ROSE_ASSERT(result->get_firstNondefiningDeclaration() != NULL);
               ROSE_ASSERT(result->get_firstNondefiningDeclaration()->get_symbol_from_symbol_table() != NULL);

            // DQ (4/30/2012): Added new tests (for test2012_58.C, but fails for test2012_51.C).
               if (result->get_definingDeclaration() != NULL)
                  {
                    ROSE_ASSERT(result->variant() == result->get_definingDeclaration()->variant());
                  }

            // DQ (11/25/2011): The new function declaration could be either a SgFunctionDeclaration or a SgTemplateFunctionDeclaration.
            // So we have to look in both caches...
            // ROSE_ASSERT(function_cache.find((a_routine_ptr)(p->entity.ptr)) != function_cache.end());
            // a_routine_ptr edg_routine = (a_routine_ptr)(p->entity.ptr);
               ROSE_ASSERT(edg_routine != NULL);
               if (isSgFunctionDeclaration(result) != NULL)
                  {
                    ROSE_ASSERT(function_cache.find(edg_routine) != function_cache.end());
                  }
                 else
                  {
                    ROSE_ASSERT(result != NULL);
                    ROSE_ASSERT(isSgTemplateFunctionDeclaration(result) != NULL);
                    ROSE_ASSERT(function_cache.find(edg_routine) != function_cache.end());
                  }
               ROSE_ASSERT(edg_routine->type != NULL);
               a_routine_type_supplement_ptr sup = edg_routine->type->variant.routine.extra_info;
               ROSE_ASSERT(sup != NULL);
               bool routine_name_linkage_is_explicit = sup->routine_name_linkage_is_explicit;
               if ( (p->explicit_storage_class == false) && (routine_name_linkage_is_explicit == false) )
                  {
                    bool resetStrorageModifier = ( ( (isSgFunctionDeclaration(result) != NULL) || (p->friend_decl == true) ) && (result->get_declarationModifier().get_storageModifier().isExtern() == true) );
                    if (resetStrorageModifier == true)
                       {
                         result->get_declarationModifier().get_storageModifier().setDefault();
                         result->unsetExternBrace();
                       }
                  }
               if (isSgMemberFunctionDeclaration(result) != NULL)
                  {
                    ROSE_ASSERT(isSgMemberFunctionType(isSgMemberFunctionDeclaration(result)->get_type()) != NULL);
                  }
               ROSE_ASSERT(result->get_firstNondefiningDeclaration() != NULL);
               ROSE_ASSERT(result->get_firstNondefiningDeclaration()->get_symbol_from_symbol_table() != NULL);

               if (p->friend_decl == true)
                  {
                    result->get_declarationModifier().setFriend();
                    ROSE_ASSERT(result->get_declarationModifier().get_accessModifier().isDefault() == false);
                  }
               if (result->get_definingDeclaration() != NULL)
                  {
                    ROSE_ASSERT(result->variant() == result->get_definingDeclaration()->variant());
                  }

               ROSE_ASSERT(p->declared_type != NULL);
               if (p->declared_type->kind == tk_routine && p->declared_type->variant.routine.extra_info->param_type_list == NULL)
                  {
                    SgFunctionDeclaration* functionDeclaration = isSgFunctionDeclaration(result);
                    ROSE_ASSERT(functionDeclaration != NULL);

                    functionDeclaration->set_prototypeIsWithoutParameters(true);
                  }


            // DQ (8/13/2020): Enforce that all member declarations are assigned an access.
               ROSE_ASSERT(result->get_declarationModifier().get_accessModifier().isDefault() == false);

            // DQ (7/10/2014): Added support for saving the declared type syntax (type equivalent to what is in the 
            // defining declaration, but sometimes syntactically different due to constant expression evaluation.
               if (p->declared_type != NULL && edg_routine->type != NULL && p->declared_type != edg_routine->type)
                  {

                 // We need to call the convert_function_parameter_list() function instead of the convert_type(p->declared_type) 
                 // function so that we can collect the function prototype's function parameter names and type.

                 // We need to add this SgFunctionParameterList to the generated function declaration as the parameter 
                 // list to go with the syntax_type information (which could be different for each function prototype, 
                 // but would always be an equivalent type).
                    bool skipDefaultArguments = false;
                    SgDeclarationScope* declarationScope = NULL;
                    SgFunctionParameterList* param_list = convert_function_parameter_list(edg_routine, skipDefaultArguments, p->declared_type, declarationScope);
                    ROSE_ASSERT(param_list != NULL);
                 // DQ (4/12/2018): Set the scope on each of the function parameters representing the syntax used with the type.
                    for (size_t i = 0; i < param_list->get_args().size(); i++)
                       {
                         SgFunctionDeclaration* functionDeclaration = isSgFunctionDeclaration(result);
                         ROSE_ASSERT(functionDeclaration != NULL);
                         ROSE_ASSERT(functionDeclaration->get_parameterList() != NULL);

                         SgInitializedName* semantic_paramter  = functionDeclaration->get_parameterList()->get_args()[i];
                         SgInitializedName* syntactic_paramter = param_list->get_args()[i];
                         ROSE_ASSERT(semantic_paramter != NULL);
                         ROSE_ASSERT(syntactic_paramter != NULL);

                      // TV (11/15/2018): that might be a regression in EDG 5.0 (SEE C_tests/test2014_18.c)
                         if (semantic_paramter->isGnuAttributeNoReturn()) {
                           syntactic_paramter->setGnuAttributeNoReturn();
                         }

                         SgScopeStatement* scope = semantic_paramter->get_scope();
                         ROSE_ASSERT(scope != NULL);
                         syntactic_paramter->set_scope(scope);
                       }

                    SgType* declared_type_syntax = convert_type(p->declared_type);
                    SgFunctionDeclaration* functionDeclaration = isSgFunctionDeclaration(result);
                    ROSE_ASSERT(functionDeclaration != NULL);

                    SgFunctionType* declared_function_type = isSgFunctionType(declared_type_syntax);
                    if (declared_function_type != NULL)
                       {
                         functionDeclaration->set_type_syntax(declared_function_type);
                         functionDeclaration->set_type_syntax_is_available(true);
                         functionDeclaration->set_parameterList_syntax(param_list);
                         param_list->set_parent(functionDeclaration);
                       }
                      else
                       {
                         mprintf ("WARNING: declared_function_type == NULL) in setting function type_syntax \n");
                         param_list->set_parent(functionDeclaration);
                       }

                    ROSE_ASSERT(!p->declared_type->declared_in_function_prototype);

                 // DQ (9/14/2015): Added test which appears to work except within the C++11 mode.
                    if (functionDeclaration->get_type_syntax_is_available() == true)
                       {
                         ROSE_ASSERT(functionDeclaration->get_type() != NULL);
                         ROSE_ASSERT(functionDeclaration->get_type_syntax() != NULL);
                       }
                  }
               ROSE_ASSERT(result->get_declarationModifier().get_accessModifier().isDefault() == false);
               break;
             }

          case iek_variable:
             {
               // FIXME 5 lines below...
               char* inner_ptr = ((a_src_seq_secondary_decl_ptr)(sse->entity.ptr))->entity.ptr;
               ROSE_ASSERT(inner_ptr != NULL);
               a_variable_ptr secondary_variable_declaration = (a_variable_ptr)inner_ptr;
               ROSE_ASSERT(secondary_variable_declaration != NULL);
               ROSE_ASSERT((a_variable_ptr)(p->entity.ptr) == secondary_variable_declaration);

               result = convert_variable_prototype(secondary_variable_declaration, NULL);
               ROSE_ASSERT(result != NULL);

            // DQ (8/13/2020): Enforce that all member declarations are assigned an access.
               ROSE_ASSERT(result->get_declarationModifier().get_accessModifier().isDefault() == false);

               if (p->declared_type->kind == tk_array)
                  {
                 // Note that it does not appear to be possible to do this test to identify if empty bracked syntax is
                 // used in the convert_variable_prototype because there is not the same access to the p->declared_type.
#if 0
                    printf ("In parse_secondary_declaration(): case iek_variable: Secondary variable declared type is an array \n");
#endif
                    a_type_ptr tp = p->declared_type;
                    if (tp->variant.array.is_variable_size_array == false && tp->variant.array.is_template_dependent_size_array == false)
                       {
#if 0
                         printf ("   --- array: sizeof(a_targ_size_t) = %" PRIuPTR " \n",(size_t)sizeof(a_targ_size_t));
                         printf ("   --- array: number_of_elements = %" PRIuPTR " \n",(size_t)tp->variant.array.variant.number_of_elements);
#endif
                         if ((size_t)tp->variant.array.variant.number_of_elements == 0)
                            {
#if 0
                              printf ("In parse_secondary_declaration(): case iek_variable: p->declared_type->kind == tk_array: array specified using [] syntax \n");
#endif
                              SgVariableDeclaration* variableDeclaration = isSgVariableDeclaration(result);
                              ROSE_ASSERT(variableDeclaration != NULL);

                              SgInitializedName* initializedName = getFirstInitializedName(variableDeclaration);
                              ROSE_ASSERT(initializedName != NULL);

                              initializedName->set_hasArrayTypeWithEmptyBracketSyntax(true);
                            }
                       }
                  }

               ++sse;
               ROSE_ASSERT(variable_cache.find((a_variable_ptr)(p->entity.ptr)) != variable_cache.end());
               break;
             }

          case iek_template:
             {
               result = convert_template_prototype((a_template_ptr)(p->entity.ptr));

               ROSE_ASSERT(result->get_declarationModifier().get_accessModifier().isDefault() == false);
               ROSE_ASSERT(result->get_startOfConstruct() != NULL);
               ROSE_ASSERT(result->get_endOfConstruct() != NULL);

            // DQ (3/8/2012): We would like to enforce that we only generate a secondary (not a primary) declaration).
               if (result == result->get_definingDeclaration()) {
                 printf("Error: This is a defining declaration. result = %p : %s\n", result, result->class_name().c_str());
               }
               ROSE_ASSERT(result != result->get_definingDeclaration());

            // DQ (12/28/2011): This is a new test.
               ROSE_ASSERT(result != NULL);
               ++sse;

               if (p->friend_decl == true)
                  {
                    result->get_declarationModifier().setFriend();
                    if (result->get_firstNondefiningDeclaration() != NULL)
                         result->get_firstNondefiningDeclaration()->get_declarationModifier().setFriend();
                    if (result->get_definingDeclaration() != NULL)
                         result->get_definingDeclaration()->get_declarationModifier().setFriend();
                    ROSE_ASSERT(result->get_declarationModifier().isFriend() == true);
                  }
               ROSE_ASSERT(p->friend_decl == result->get_declarationModifier().isFriend());
               result->setForward();
               break;
             }

          case iek_namespace:
             {
               char* inner_ptr = ((a_src_seq_secondary_decl_ptr)(sse->entity.ptr))->entity.ptr;
               ROSE_ASSERT(inner_ptr != NULL);
               a_namespace_ptr secondary_namespace_declaration = (a_namespace_ptr)inner_ptr;
               ROSE_ASSERT(secondary_namespace_declaration != NULL);
               result = convert_namespace(secondary_namespace_declaration,sse);
               ROSE_ASSERT(result != NULL);

               ROSE_ASSERT(result->get_declarationModifier().get_accessModifier().isDefault() == true);
               if (secondary_namespace_declaration->source_corresp.decl_position.seq != 0)
                  {
                    ++sse;
                  }
               break;
             }

           default: {
             printf ("Case not handled! il_entry_kind_names[kind=%d] = %s \n", kind, il_entry_kind_names[kind]);
             ROSE_ABORT();
           }              
         }

     if (p->declared_in_func_prototype == true)
        {
          mprintf ("In parse_secondary_declaration(): If this was a type defined in a function parameter then result == NULL \n");
          ROSE_ASSERT (result == NULL);

          return result;
        }
       else
        {
          ROSE_ASSERT (result != NULL);
        }

#if MAKE_REDUNDANT_CALLS_TO_DETECT_TRANSFORAMTIONS
  // DQ (5/1/2012): Make sure that we don't have IR nodes marked as transformations.
     detectTransformations_local(result);
#endif

  // DQ (1/30/2015): Avoid resetting the case of a SgNamespaceDeclarationStatement since this more general function will not handle it properly.
  // This will delete the previously build Sg_File_Info object and build new ones (some properties of the original Sg_File_Info object are transfered, but not all make sense).
  // result = set_file_info_for_secondary_declaration(result,p);
  // fixupSubTreeSourcePositions(result);
     SgNamespaceDeclarationStatement* namespaceDeclarationStatement = isSgNamespaceDeclarationStatement(result);
     if (namespaceDeclarationStatement == NULL)
        {

       // DQ (1/13/2020): Friend functions are secondary declarations that should not be marked as compiler 
       // generated (where they have a valid source position from the secondary declaration source position).
       // Though at the moment they may be unparsed using the incorrect syntax.
          bool markedAsCompilerGenerated = false;
       // if (p->friend_decl != result->get_firstNondefiningDeclaration()->get_declarationModifier().isFriend())
          bool isFriendDeclaration = p->friend_decl;
          if (isFriendDeclaration == true)
             {
               markedAsCompilerGenerated = result->get_startOfConstruct()->isCompilerGenerated();
             }

          result = set_file_info_for_secondary_declaration(result,p);

          if (isFriendDeclaration == true && markedAsCompilerGenerated == true)
             {
               result->unsetCompilerGenerated();
             }

          fixupSubTreeSourcePositions(result);
        }


     testDefiningNonDefiningDeclarations(result->get_firstNondefiningDeclaration(),result->get_definingDeclaration(),result,"parse_secondary_declaration()");

     SgNamespaceDeclarationStatement* namespaceDeclaaration = isSgNamespaceDeclarationStatement(result);
     bool isAcceptedNodeToBeMarkedAsDefault = (namespaceDeclaaration != NULL);
     if ( (isAcceptedNodeToBeMarkedAsDefault == false) && (result->get_declarationModifier().get_accessModifier().isDefault() == true) )
        {
          printf ("Note: result = %p = %s marked with default access \n",result,result->class_name().c_str());
        }

#if DEBUG__parse_secondary_declaration
     printf ("LEAVE parse_secondary_declaration():\n");
     printf ("  result = %p = %s\n", result, result->class_name().c_str());
     printf ("    isStatic() = %s \n", result->get_declarationModifier().get_storageModifier().isStatic() ? "true" : "false");
     printf ("    isExtern() = %s \n", result->get_declarationModifier().get_storageModifier().isExtern() ? "true" : "false");
     printf ("    get_linkage().empty() = %s \n", result->get_linkage().empty() ? "true" : "false");
     printf ("    get_linkage() = %s \n", result->get_linkage().c_str());
     printf ("    isExternBrace() = %s \n", result->isExternBrace() ? "true" : "false");
#endif
     return result;
   }
}