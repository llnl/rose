
#include "edgRose.h"

SgGlobal*
EDG_ROSE_Translation::parse_global_scope(SgGlobal* inputGlobalScope, SourceSequenceList& sse, bool skip_ast_translation )
   {
  // This function processes the global scope. However it does not setup the 
  // Sg_File_Info object in the SgGlobal, this is done in the sage_back_end().
  // Normally the SgSourceFile is setup with a valid SgGlobal before calling 
  // the sage_back_end(), this detail is fixed in sage_back_end().

      ROSE_ASSERT(inputGlobalScope != NULL);

  // SgGlobal* globalScope = set_generated_file_info_to_unparse(new SgGlobal());
  // SgGlobal* globalScope = set_generated_file_info_to_unparse(inputGlobalScope);
     SgGlobal* globalScope = inputGlobalScope;
     ROSE_ASSERT(globalScope != NULL);

     current_global_scope = globalScope;

#if 0
     printf ("In parse_global_scope(): globalScope = %p \n",globalScope);
#endif

  // DQ (7/18/2017): Add initialization of the intermediate scope used to hold class declarations 
  // built for template parameters before then template declarations.  Note that these declarations 
  // are moved from this intermediate scope to a scope built in the template declaration.  This is 
  // required because template parameter must be processed as part of (before) building the template 
  // the template declaration.  See test2017_44.C and test2017_47.C for an example of this.
     Sg_File_Info *startOfConstruct = new Sg_File_Info (globalScope->get_startOfConstruct()->get_filenameString());
     ROSE_ASSERT(startOfConstruct != NULL);

  // internal_intermediate_scope = new SgGlobal(startOfConstruct);
     internal_intermediate_scope = new SgDeclarationScope(startOfConstruct);
     ROSE_ASSERT(internal_intermediate_scope != NULL);

  // We need this to have a valid parent, and global scope might be all that is available.
  // Or maybe we need to fixup the get_scope() function to detect the case of a SgDeclarationScope and return NULL (instead of an error).
     internal_intermediate_scope->set_parent(globalScope);
     ROSE_ASSERT(internal_intermediate_scope->get_parent() != NULL);

#if DEBUG_CLASSES_FROM_TEMPLATE_PARAMETERS
     printf ("In parse_global_scope(): internal_intermediate_scope = %p \n",internal_intermediate_scope);
#endif

  // If we want to record scopes this is one place to do it, and I think this is how it should be done.
  // an_il_entry_kind kind = (an_il_entry_kind)(sse->entity.kind);
  // printf ("In parse_global_scope(): initial kind = %s \n",summarize_entry_kind_in_string(kind));

  // const an_il_header & header = il_header;
  // a_scope_ptr main_scope = header.primary_scope;

     ROSE_ASSERT(il_header.primary_scope != NULL);

  // DQ (3/11/2012): check that this is not already in the list before we add it (or
  // verify it is NULL if it is supposed to be in the list but just not initialized yet).
     ROSE_ASSERT (scope_cache.find(il_header.primary_scope) == scope_cache.end());

     scope_cache[il_header.primary_scope] = globalScope;

#if 0
     printf ("In parse_global_scope(): il_header.primary_scope (EDG global scope) = %p \n",il_header.primary_scope);
#endif

#if 0
     printf ("In parse_global_scope(): Calling summarize_il_header \n");
     summarize_il_header(il_header);
#endif

#if 0
     printf ("In parse_global_scope(): Calling summarize_scope \n");
     summarize_scope(il_header.primary_scope);
#endif

#if DEBUG_PUSH_SCOPE
     printf ("In parse_global_scope(): push scope stack: global scope = %p = %s \n",globalScope,globalScope->class_name().c_str());
#endif
     pushScopeStack(globalScope);

#if DEBUG_PRINT_SCOPE_STACK
     printScopeStack("In parse_global_scope(): after pushing global scope ");
#endif

#ifdef DEBUG_CONVERSION
     ScopedIndent si(indent);
#endif

  // DQ (11/1/11): Added support to skip ROSE specific processing so that the EDG frontend could be separately evaluated.
     if (skip_ast_translation == false)
        {
       // Process all global scope declarations
          while (sse)
             {
               SgDeclarationStatement* declarationStatement = parse_global_or_namespace_scope_entity(sse);
#if 0
               if (declarationStatement != NULL)
                  {
                    printf ("In parse_global_scope(): declaration s = %p = %s \n",declarationStatement,(declarationStatement != NULL) ? declarationStatement->class_name().c_str() : "NULL");
                  }
#endif
            // Not all source sequence entries will translate into a valid IR node.
            // As part of general EDG translation documentation we should make a 
            // list of example that don't translate.
               if (declarationStatement != NULL)
                  {
                    bool appendStatementToScope = true;

                 // DQ (4/27/2012): There are conditions where we want to not append this declaration, for example 
                 // when the declaration is defined in a class.  See test2012_57.C for an example of this.
                    SgFunctionDeclaration* functionDeclaration = isSgFunctionDeclaration(declarationStatement);
#if 0
                    printf ("In parse_global_scope(): functionDeclaration = %p = %s \n",functionDeclaration,declarationStatement->class_name().c_str());
#endif
                    if (functionDeclaration != NULL)
                       {
                         ROSE_ASSERT(functionDeclaration->get_parent() != NULL);

                      // DQ (8/7/2012): Member functions can frequenly be declared outside of their class and so we have to allow this (see test2012_172.C).
                      // appendStatementToScope = (functionDeclaration->get_scope() == functionDeclaration->get_parent());
                         SgMemberFunctionDeclaration* memberFunctionDeclaration = isSgMemberFunctionDeclaration(declarationStatement);
#if 0
                         printf ("memberFunctionDeclaration = %p = %s \n",memberFunctionDeclaration,functionDeclaration->class_name().c_str());
#endif
                         if (memberFunctionDeclaration == NULL)
                            {
                           // DQ (5/5/2012): This is fixed again to reflect test2012_71.C.
                           // appendStatementToScope = (globalScope == functionDeclaration->get_parent());
                              appendStatementToScope = (functionDeclaration->get_scope() == functionDeclaration->get_parent());

                           // DQ (1/14/2013): Added support to place the instantiated template were ever it is declared (instantiated template
                           // may go into global scope (and be name qualified) even when the template declaration is within a namespace).
                           // This should fix the 2nd bug reported by Robb for the case where template instantations were not being properly 
                           // traversed (e.g. when the template declaration was in a namespace).
                              SgTemplateInstantiationFunctionDecl* templateInstantiationFunctionDeclaration = isSgTemplateInstantiationFunctionDecl(declarationStatement);
                              if (templateInstantiationFunctionDeclaration != NULL)
                                 {
                                // Template instantiated have to be able to be placed into global scope (even if there scopes don't match).
#if 0
                                   printf ("templateInstantiationFunctionDeclaration = %p = %s \n",templateInstantiationFunctionDeclaration,templateInstantiationFunctionDeclaration->get_name().str());
                                   printf ("--- templateInstantiationFunctionDeclaration->get_declarationModifier().isFriend() = %s \n",templateInstantiationFunctionDeclaration->get_declarationModifier().isFriend() ? "true" : "false");
#endif
                                // DQ (1/16/2013): If we just set "appendStatementToScope = true" then we fail test2013_03.C in the copyAST_tests
                                // (because the declaration will be added twice to the AST).  I think this is an issue for friend functions of
                                // classes but not for template instantiations in namespaces (see test2013_02.C).
                                   if (templateInstantiationFunctionDeclaration->get_declarationModifier().isFriend() == false)
                                      {
#ifdef DEBUG_CONVERSION
                                        printf ("Mark this for input into AST (SgTemplateInstantiationFunctionDecl non-friend function): templateInstantiationFunctionDeclaration = %p = %s \n",
                                             templateInstantiationFunctionDeclaration,templateInstantiationFunctionDeclaration->class_name().c_str());
#endif
                                        appendStatementToScope = true;
                                      }
                                     else
                                      {
#ifdef DEBUG_CONVERSION
                                        printf ("In parse_global_scope(): templateInstantiationFunctionDeclaration = %p = %s appendStatementToScope = %s \n",
                                             templateInstantiationFunctionDeclaration,templateInstantiationFunctionDeclaration->get_name().str(),appendStatementToScope ? "true" : "false");
#endif
                                      }
                                 }
                                else
                                 {
                                // DQ (4/10/2014): Added case to handle SgFunctionDeclaration that is not a member function or template function.
                                   appendStatementToScope = (functionDeclaration->get_scope() == functionDeclaration->get_parent());

                                // DQ (4/10/2014): If this is a function from a namespace then it can be declared in a different scope.
                                // See test2014_47.C
                                   SgNamespaceDefinitionStatement* namespaceDefinition_scope = isSgNamespaceDefinitionStatement(functionDeclaration->get_scope());
#if 0
                                   printf ("case non-memberfunction: appendStatementToScope     = %s \n",appendStatementToScope ? "true" : "false");
                                   printf ("case non-memberfunction: namespaceDefinition_scope = %p \n",namespaceDefinition_scope);
#endif
                                   if (appendStatementToScope == false && namespaceDefinition_scope != NULL)
                                      {
                                        mprintf ("case memberfunction: reset appendStatementToScope = true for non-member functions defined in namespaces \n");
                                        appendStatementToScope = true;
                                      }
                                 }
                            }
                           else
                            {
                           // This is the case of a member function.

                           // DQ (8/9/2012): If this is a template member function then it might have already been put out into global scope as part of a previous class member.
                              SgTemplateMemberFunctionDeclaration* templateMemberFunctionDeclaration = isSgTemplateMemberFunctionDeclaration(memberFunctionDeclaration);
#if 0
                              printf ("templateMemberFunctionDeclaration = %p = %s \n",templateMemberFunctionDeclaration,memberFunctionDeclaration->class_name().c_str());
#endif
                              if (templateMemberFunctionDeclaration != NULL)
                                 {
                                // For now we want the simple rule that we will not put template member functions into the global scope.
                                // printf ("Skipping appending of SgTemplateMemberFunctionDeclaration to global scope for templateMemberFunctionDeclaration = %p = %s \n",templateMemberFunctionDeclaration,templateMemberFunctionDeclaration->get_name().str());
#if 0
                                // DQ (8/10/2012): This allows test2012_173.C and test2012_174.C to work.
                                   appendStatementToScope = false;
#else
                                // DQ (8/10/2012): This permits test2007_38.C to work.
                                   appendStatementToScope = (functionDeclaration->get_parent() == globalScope);
#endif
#if 0
                                   bool definedInClass = (functionDeclaration->get_parent() == functionDeclaration->get_scope());
                                   printf ("definedInClass = %s \n",definedInClass ? "true" : "false");
#endif
                                 }
                                else
                                 {
#if 0
                                   SgTemplateInstantiationMemberFunctionDecl* templateInstantiationMemberFunctionDeclaration = isSgTemplateInstantiationMemberFunctionDecl(memberFunctionDeclaration);
                                   printf ("templateInstantiationMemberFunctionDeclaration = %p = %s \n",templateInstantiationMemberFunctionDeclaration,memberFunctionDeclaration->class_name().c_str());
#endif
#if 0
                                   appendStatementToScope = (functionDeclaration->get_scope() == functionDeclaration->get_parent());
                                   if (appendStatementToScope == true)
                                      {
                                        appendStatementToScope = (functionDeclaration->get_parent() == globalScope);
                                      }
#else
                                // DQ (8/10/2012): Trying this for symetry...
                                   appendStatementToScope = (functionDeclaration->get_parent() == globalScope);
#endif
                                 }
                            }
#if 0
                         printf ("In parse_global_scope():   --- appendStatementToScope            = %s \n",appendStatementToScope ? "true" : "false");
                         printf ("In parse_global_scope():   --- functionDeclaration->get_parent() = %p \n",functionDeclaration->get_parent());
                         printf ("In parse_global_scope():   --- functionDeclaration->get_scope()  = %p \n",functionDeclaration->get_scope());
#endif
                       }

                 // DQ (6/19/2012): We have to handle variable declarations that contain the type declaration (e.g. class declarations).
                 // these should not be added to the associated scope (even though they support a constructed type. See test2012_107.C.
                    SgClassDeclaration* classDeclaration = isSgClassDeclaration(declarationStatement);
                    if (classDeclaration != NULL)
                       {
                      // In this case the parent will not be set (will be NULL).
                      // ROSE_ASSERT(classDeclaration->get_parent() != NULL);
                         appendStatementToScope = (classDeclaration->get_scope() == classDeclaration->get_parent());

#if 0
                         printf ("In parse_global_scope():   --- classDeclaration               = %p \n",classDeclaration);
                         printf ("In parse_global_scope():   --- appendStatementToScope         = %s \n",appendStatementToScope ? "true" : "false");
                         printf ("In parse_global_scope():   --- classDeclaration->get_parent() = %p \n",classDeclaration->get_parent());
                         printf ("In parse_global_scope():   --- classDeclaration->get_scope()  = %p \n",classDeclaration->get_scope());
#endif

                      // DQ (7/14/2012): Only a few cases are allowed to not have a valid parent.
                         if (classDeclaration->get_parent() == NULL)
                            {
#ifdef DEBUG_CONVERSION
                              printf ("In parse_global_scope(): classDeclaration->get_parent() == NULL is allowed for a non-autonomous class declaration (nested in a type). \n");
#endif
                            }
                      // ROSE_ASSERT(classDeclaration->get_parent() != NULL);

                         if (appendStatementToScope == false)
                            {
                           // DQ (9/8/2012): Now that some SgClassDeclaration can be reused (if they are defining declarations), they might have already had their parent pointers set.
                           // Unclear if where this happens we should have built a secondary declaration instead of reusing the existing defining declaration.
                              if (classDeclaration->get_parent() != NULL)
                                 {
#ifdef DEBUG_CONVERSION
                                   printf ("NOTE: In parse_global_scope(): classDeclaration->get_parent() != NULL: Likely this is a reused declaration already attached to the AST. classDeclaration = %p = %s \n",
                                        classDeclaration,classDeclaration->class_name().c_str());
#endif
                                   ROSE_ASSERT(classDeclaration == classDeclaration->get_definingDeclaration());

                                // DQ (12/22/2019): Reset appendStatementToScope = true to support seperate defining declarations across multiple files.
                                   appendStatementToScope = true;
#if 0
                                   printf ("For classDeclaration = %p = %s name = %s Reset appendStatementToScope = true \n",
                                        classDeclaration,classDeclaration->class_name().c_str(),classDeclaration->get_name().str());
#endif
                                 }
                           // ROSE_ASSERT(classDeclaration->get_parent() == NULL);

                           // This needs to be set in the variable declaration.
                           // classDeclaration->set_variableDeclarationContainsBaseTypeDefiningDeclaration(true);
                            }
                       }
                      else
                       {
                         SgEnumDeclaration* enumDeclaration = isSgEnumDeclaration(declarationStatement);
                         if (enumDeclaration != NULL)
                            {
                           // DQ (2/13/2019): The parent ans scope need not be the same, see Cxx11_tests/test2019_120.C).
                           // appendStatementToScope = (enumDeclaration->get_scope() == enumDeclaration->get_parent());
                              appendStatementToScope = true;

#if 0
                              printf ("In parse_global_scope():   --- appendStatementToScope        = %s \n",appendStatementToScope ? "true" : "false");
                              printf ("In parse_global_scope():   --- enumDeclaration->get_parent() = %p \n",enumDeclaration->get_parent());
                              printf ("In parse_global_scope():   --- enumDeclaration->get_scope()  = %p \n",enumDeclaration->get_scope());
#endif

                           // DQ (7/14/2012): Only a few cases are allowed to not have a valid parent.
                              if (enumDeclaration->get_parent() == NULL)
                                 {
#ifdef DEBUG_CONVERSION
                                   printf ("In parse_global_scope(): enumDeclaration->get_parent() == NULL is allowed for a non-autonomous class declaration (nested in a type). \n");
#endif
                                 }
                           // ROSE_ASSERT(classDeclaration->get_parent() != NULL);
                            }
                           else
                            {
                           // DQ (12/28/2015): If this is a variable declaration we are adding multiple variables to, then we want to avoid adding it to the AST twice.
                              SgVariableDeclaration* variableDeclaration = isSgVariableDeclaration(declarationStatement);
                              if (variableDeclaration != NULL)
                                 {
#if 0
                                   printf ("In parse_global_scope():   --- Found variableDeclaration = %p size = %zu \n",variableDeclaration,variableDeclaration->get_variables().size());
#endif
                                   if (variableDeclaration->get_variables().size() > 1)
                                      {
#if 0
                                        printf ("In parse_global_scope():   --- Adding this case of non-normalized variableDeclaration with multiple variables: size = %zu \n",
                                             variableDeclaration->get_variables().size());
#endif
#if 0
                                     // DQ (12/29/2015): This code is no long required since we are moving to a design that processes all 
                                     // of the variables in a variable declaration before returning from parse_variable_declaration().
                                        appendStatementToScope = false;
#endif
                                      }
                                 }

                           // DQ (7/14/2012): All other declarations should have a valid parent (maybe not enum declarations).
                              if (declarationStatement->get_parent() == NULL)
                                 {
                                   printf ("In parse_global_scope(): declarationStatement->get_parent() == NULL declarationStatement = %p = %s \n",declarationStatement,declarationStatement->class_name().c_str());
                                 }
                              ROSE_ASSERT(declarationStatement->get_parent() != NULL);
                            }
                       }

#if 0
                    printf ("In parse_global_scope(): appendStatement to global scope: s = %p = %s appendStatementToScope = %s \n",
                         declarationStatement,declarationStatement->class_name().c_str(),appendStatementToScope ? "true" : "false");
                    printf ("  --- statement being appended to scope = %p = %s \n",declarationStatement,declarationStatement->class_name().c_str());
                    printf ("  --- associated scope = %p = %s \n",globalScope,globalScope->class_name().c_str());
                    printf ("  --- appendStatementToScope = %s \n",appendStatementToScope ? "true" : "false");
#endif

                 // Mark this as compiler generated (this must be handled more generally).
                 // Note that there is no ROSE IR node for a variable template instantiation.
                    SgTemplateInstantiationDecl*               classTemplateInstantiation          = isSgTemplateInstantiationDecl(declarationStatement);
                    SgTemplateInstantiationFunctionDecl*       functionTemplateInstantiation       = isSgTemplateInstantiationFunctionDecl(declarationStatement);
                    SgTemplateInstantiationMemberFunctionDecl* memberFunctionTemplateInstantiation = isSgTemplateInstantiationMemberFunctionDecl(declarationStatement);

                 // Mark this as compiler generated.
                    if ( (appendStatementToScope == true) && (classTemplateInstantiation != NULL || functionTemplateInstantiation != NULL || memberFunctionTemplateInstantiation != NULL) )
                       {
                         if (declarationStatement->get_file_info()->isCompilerGenerated() == false)
                            {
#ifdef DEBUG_CONVERSION
                              printf ("ERROR: template instantiations should be marked as compiler generated (though maybe not for template specializations) declarationStatement = %p = %s \n",
                                   declarationStatement,declarationStatement->class_name().c_str());
#endif
#if 0
                              declarationStatement->get_file_info()->display("template instantiations should be marked as compiler generated (though maybe not for template specializations): debug");
#endif
                            }
                      // ROSE_ASSERT(declarationStatement->get_file_info()->isCompilerGenerated() == true);
                      // declarationStatement->get_file_info()->setCompilerGenerated();
                       }

                 // appendStatement(declarationStatement, globalScope);
                    if (appendStatementToScope == true)
                       {
#if MAKE_REDUNDANT_CALLS_TO_DETECT_TRANSFORAMTIONS
                      // DQ (5/1/2012): Make sure that we don't have IR nodes marked as transformations.
                         detectTransformations_local(declarationStatement);
#endif
#if 0
                         printf ("Appending the statement to global scope: declarationStatement = %p = %s \n",declarationStatement,declarationStatement->class_name().c_str());
#endif
                         appendStatement(declarationStatement, globalScope);
                       }
                      else
                       {
                      // DQ (4/30/2012): This happens rarely.
#ifdef DEBUG_CONVERSION
                         printf ("WARNING: In parse_global_scope(): Declaration will not be appended to the global scope (declarationStatement = %p = %s) \n",
                              declarationStatement,declarationStatement->class_name().c_str());
#endif
                       }
#if 0
                    printf ("declarationStatement->get_parent() = %p \n",declarationStatement->get_parent());
#endif
                  }

               std::map<a_template_ptr, SgFunctionDeclaration*>::const_iterator i;
               for (std::map<a_template_ptr, SgFunctionDeclaration*>::const_iterator i = saved_defining_declarations_from_class_templates.begin(); i != saved_defining_declarations_from_class_templates.end(); ++i) {
                 ROSE_ASSERT(i->second != NULL);
                 appendStatement(i->second, globalScope);
               }
               saved_defining_declarations_from_class_templates.clear();
             }
        }

#if DEBUG_POP_SCOPE
     printf ("In parse_global_scope(): calling popScopeStack() \n");
#endif
#if DEBUG_PRINT_SCOPE_STACK
     printScopeStack("In parse_global_scope(): before poping global scope ");
#endif

  // Pop the global scope
     popScopeStack();

  // Error checking, there should be nothing left on the scope stack.
     ROSE_ASSERT(topScopeStack() == NULL);

#if 0
     printf ("In parse_global_scope(): suppress_detection_of_transformations = %s \n",suppress_detection_of_transformations ? "true" : "false");
#endif

  // DQ (6/3/2019): The case of outlining to a seperate file will have transformations 
  // that this checking will fail on because it is for the typical case of checking the 
  // AST for transformations after construction of the AST from an typical input file.
  // DQ (2/17/2013): Make sure that we don't have IR nodes marked as transformations.
  // This is now implemented as a single pass of the final AST to avoid the redundant
  // calls that were being done when a finer level of error checking was previously 
  // required. The redundant calls to this function are not turned off using a macro:
  // MAKE_REDUNDANT_CALLS_TO_DETECT_TRANSFORAMTIONS.
  // detectTransformations_local(globalScope);
     if (suppress_detection_of_transformations == false)
        {
          detectTransformations_local(globalScope);
        }

#if 0
     printf ("Leaving parse_global_scope() \n");
#endif

     return globalScope;
   }
