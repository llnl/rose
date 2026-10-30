
#include "edgRose.h"

SgTemplateInstantiationDirectiveStatement*
EDG_ROSE_Translation::parse_instantiation_directive(SourceSequenceList& sse)
   {
  // Generate code for an instantiation directive.

     an_instantiation_directive_ptr idp = (an_instantiation_directive_ptr)(sse->entity.ptr);
     ++sse;

#ifdef DEBUG_CONVERSION
     indent_line(); debug_print("instantiation directive statement: "); print_source_pos(idp->position); debug_print("\n");
     ScopedIndent si(indent);
#endif

#if 0
     printf ("In parse_instantiation_directive(): calling summarize_instantiation_directive(idp = %p) \n",idp);
     summarize_instantiation_directive(idp);
#endif

     an_il_entry_kind kind = (an_il_entry_kind)idp->entity.kind;
     SgDeclarationStatement* declarationStatement = NULL;
     bool do_not_instantiate = idp->do_not_instantiate;
#if 0
     printf ("In parse_instantiation_directive(): kind = %s \n",summarize_entry_kind_in_string_kind(kind));
#endif

     switch (kind)
        {
          case iek_routine:
             {
               a_routine_ptr p = (a_routine_ptr)idp->entity.ptr;
               ROSE_ASSERT(p != NULL);

               bool forceTemplateDeclaration = false;
               a_template_ptr edg_template = NULL;
               SgFunctionDeclaration* functionDeclaration = convert_routine_prototype(p,forceTemplateDeclaration,edg_template);
               ROSE_ASSERT(functionDeclaration != NULL);

            // DQ (5/15/2012): Note that the SgTemplateInstantiationMemberFunctionDecl is marked as static, but this can't be right!
               if (functionDeclaration->get_declarationModifier().get_storageModifier().isStatic() == true)
                  {
#if 0
                    printf ("WARNING: In parse_instantiation_directive(): I don't know why this is marked as static (mark it as non static \n");
#endif
                    functionDeclaration->get_declarationModifier().get_storageModifier().setDefault();
                  }
               ROSE_ASSERT(functionDeclaration->get_declarationModifier().get_storageModifier().isStatic() == false);

            // DQ (5/15/2012): Note that the SgTemplateInstantiationMemberFunctionDecl is marked as inline, but this can't be right!
               if (functionDeclaration->get_functionModifier().isInline() == true)
                  {
#if 0
                    printf ("WARNING: In parse_instantiation_directive(): I don't know why this is marked as inline (mark it as non inline \n");
#endif
                    functionDeclaration->get_functionModifier().unsetInline();
                  }
               ROSE_ASSERT(functionDeclaration->get_functionModifier().isInline() == false);

               declarationStatement = functionDeclaration;
               break;
             }

          case iek_variable:
             {
            // This interface will not require the sse and thus will simplify access to the variable after the 
            // sse variable has been incremented (above).
               a_variable_ptr var = (a_variable_ptr)idp->entity.ptr;

            // DQ (1/18/2019): We need a secondary variable declaration (prototype).  Note this will not have an initializer.
            // Not clear what to do with the edg_template parameter (unavailable from the template instantiation directive).
               a_template_ptr edg_template = NULL;
               SgVariableDeclaration* variableDeclaration = convert_variable_prototype(var,edg_template);
               declarationStatement = variableDeclaration;

               break;
             }

          case iek_type:
             {
               a_type_ptr class_type = (a_type_ptr)idp->entity.ptr;

               SgType* associatedType = convert_type(class_type);
               ROSE_ASSERT(associatedType != NULL);
               SgClassType* classType = isSgClassType(associatedType);
               ROSE_ASSERT(classType != NULL);

            // DQ (3/6/2012): Added test.
               ROSE_ASSERT (classType->get_declaration() != NULL);
               SgClassDeclaration* local_classDeclaration = isSgClassDeclaration(classType->get_declaration());
               ROSE_ASSERT (local_classDeclaration != NULL);
               ROSE_ASSERT (local_classDeclaration->get_firstNondefiningDeclaration() != NULL);
               ROSE_ASSERT (local_classDeclaration->get_firstNondefiningDeclaration() == local_classDeclaration);

            // DQ (11/23/2015): Make this a unique object so that we avoid multiple references to IR nodes in the AST.
               SgClassDeclaration::class_types rose_class_kind = convert_class_kind(class_type->kind);
               declarationStatement = convert_struct_secondary(class_type,rose_class_kind);
               ROSE_ASSERT(declarationStatement != NULL);

               ROSE_ASSERT(declarationStatement->get_firstNondefiningDeclaration() != NULL);

               break;
             }

          default:
             {
            // unexpected_condition_str("gen_instantiation_directive: bad ent kind");
               printf ("Error: default reached in parse_instantiation_directive() \n");
               ROSE_ASSERT(false);
             }
        }

     ROSE_ASSERT(declarationStatement != NULL);

  // TV (7/21/13): To handle the case when it is the second time we see the instantiation directive
     SgTemplateInstantiationDirectiveStatement * templateInstantiationDirective = isSgTemplateInstantiationDirectiveStatement(declarationStatement->get_parent());
     if (templateInstantiationDirective != NULL)
        {
#if 1
          printf ("In parse_instantiation_directive(): I think that this fix by Tristan will cause multiple entries in the AST: returning templateInstantiationDirective = %p \n",templateInstantiationDirective);
#endif
          printf ("In parse_instantiation_directive(): Make this an error to support debugging \n");
          ROSE_ASSERT(false);

          return templateInstantiationDirective;
        }
  // assert(declarationStatement->get_parent() == NULL);

     templateInstantiationDirective = new SgTemplateInstantiationDirectiveStatement(declarationStatement);

  // DQ (11/4/2012): Added call to set file info to default settings.
     setSourcePosition(templateInstantiationDirective);

     SgTemplateInstantiationDirectiveStatement* result = set_file_info_for_where_corresp_is_unavailable(templateInstantiationDirective,idp->position);

     ROSE_ASSERT(result != NULL);

     declarationStatement->set_parent(result);

     if (do_not_instantiate == true)
        {
       // Not clear if there is something special to do here or not...
#ifdef DEBUG_CONVERSION
          printf ("Interesting (C++11 feature) case of do_not_instantiate == true \n");
#endif
          result->set_do_not_instantiate(true);
        }
#if 0
     printf ("In parse_instantiation_directive(): result = %p setting firstNondefiningDeclaration to %p \n",result,result);
#endif
     result->set_firstNondefiningDeclaration(result);

     ROSE_ASSERT(result->get_firstNondefiningDeclaration() != NULL);

#if 0
     printf ("DDDDDDD #1 new SgTemplateInstantiationDirectiveStatement = %p declarationStatement = %p \n",result,declarationStatement);
#endif

     ROSE_ASSERT(result->get_parent() == NULL);
     result->set_parent(topScopeStack());
     ROSE_ASSERT(result->get_parent() != NULL);

     return result;
   }