
#include "edgRose.h"

SgDeclarationStatement*
EDG_ROSE_Translation::parse_using_declaration(SourceSequenceList& sse)
   {
  // Generate code for a class member or nonmember using-declaration.

#define DEBUG_PARSE_USING_DECLARATION 0

     a_using_decl_ptr udp = (a_using_decl_ptr)(sse->entity.ptr);
     ++sse;

     ROSE_ASSERT(udp != NULL);

#ifdef DEBUG_CONVERSION
     indent_line(); debug_print("using declaration statement: "); print_source_pos(udp->position); debug_print("\n");
     ScopedIndent si(indent);
#endif

#if DEBUG_PARSE_USING_DECLARATION
     printf ("In parse_using_declaration(): calling summarize_using_declaration(udp = %p) \n",udp);
     summarize_using_declaration(udp);
#endif

  // a_source_correspondence  *scp = NULL;
     an_il_entry_kind         entry_kind = (an_il_entry_kind)udp->entity.kind;;

  // SgUsingDeclarationStatement* result = NULL;
     SgDeclarationStatement* result = NULL;

  // DataRequiredForComputationOfSourcePostionInformation sourcePositionData(udp->position);

  // Using directives are handled by a separate function, parse_using_directive()
  // ROSE_ASSERT(udp->is_using_directive == false);

#if 0
  /* Advance past the source sequence entry for the "using" directive. */
#if TRACE_ADVANCE_CURRENT_SOURCE_SEQUENCE_ENTRY
     printf ("Calling adv_curr_source_sequence_entry from sage_gen_using_declaration() \n");
#endif
     adv_curr_source_sequence_entry();

  /* Position the output file to the "using" position. */
     set_output_position(&udp->position);

  /* Get the source correspondence entry for the entity. */
     entry_kind = (an_il_entry_kind)udp->entity.kind;

     scp = source_corresp_for_il_entry(udp->entity.ptr, entry_kind);
     check_assertion(scp != NULL);
#endif

  // a_using_decl_ptr udp = (a_using_decl_ptr)(sse->entity.ptr);
  // entry_kind = (an_il_entry_kind)udp->entity.kind;

#if DEBUG_PARSE_USING_DECLARATION
     printf ("In parse_using_declaration(): entry_kind = %s \n",summarize_entry_kind_in_string_kind(entry_kind));
#endif

  // DQ (9/11/2004): We require a declaration to setup the using-declaration.
  // we will get the declaration from a symbol previously stored in the EDG AST.
     SgDeclarationStatement* declarationStatement = NULL;

     switch (entry_kind)
        {
          case iek_variable:
             {
               ROSE_ASSERT(udp->is_using_directive == false);

               a_variable_ptr edg_variable = (a_variable_ptr)(udp->entity.ptr);
               ROSE_ASSERT(edg_variable != NULL);

#if DEBUG_PARSE_USING_DECLARATION
               printf ("In parse_using_declaration(): calling summarize_variable(edg_variable = %p) \n",edg_variable);
               summarize_variable(edg_variable);
#endif
               SgName name = edg_variable->source_corresp.name;
               a_scope_ptr edg_scope = edg_variable->source_corresp.parent_scope;
               ROSE_ASSERT(edg_scope != NULL);

            // DQ (3/11/2012): check that this is not already in the list before we add it (or
            // verify it is NULL if it is supposed to be in the list but just not initialized yet).
               ROSE_ASSERT (scope_cache.find(edg_scope) != scope_cache.end());

               SgScopeStatement* scopeOfReference = scope_cache[edg_scope];
               ROSE_ASSERT(scopeOfReference != NULL);

               SgVariableSymbol* variableSymbol = scopeOfReference->lookup_variable_symbol(name);
               ROSE_ASSERT(variableSymbol != NULL);

               ROSE_ASSERT(variableSymbol->get_declaration() != NULL );
               ROSE_ASSERT(variableSymbol->get_declaration()->get_declaration() != NULL );
               declarationStatement = variableSymbol->get_declaration()->get_declaration();
               ROSE_ASSERT(declarationStatement != NULL);
            // result = set_file_info_for_corresp(new SgUsingDeclarationStatement(declarationStatement,NULL), udp->source_corresp);
            // result = set_file_info_for_where_corresp_is_unavailable(new SgUsingDeclarationStatement(declarationStatement,NULL), udp->position);
               SgUsingDeclarationStatement* usingDeclaration = new SgUsingDeclarationStatement(declarationStatement,NULL);

            // DQ (11/3/2012): Added call to set file info to default settings.
               setSourcePosition(usingDeclaration);

               result = set_file_info_for_where_corresp_is_unavailable(usingDeclaration, udp->position);

               ROSE_ASSERT (result != NULL);
#if 0
               ROSE_ASSERT(!"using declaration: iek_variable not supported");
#endif
               break;
             }

          case iek_constant:
             {
               ROSE_ASSERT(udp->is_using_directive == false);

               a_constant_ptr edg_constant = (a_constant_ptr)(udp->entity.ptr);
               ROSE_ASSERT(edg_constant != NULL);

#if DEBUG_PARSE_USING_DECLARATION
               printf ("In parse_using_declaration(): calling summarize_constant(edg_constant = %p) \n",edg_constant);
               summarize_constant(edg_constant);
#endif
               SgName name = edg_constant->source_corresp.name;
               a_scope_ptr edg_scope = edg_constant->source_corresp.parent_scope;
               ROSE_ASSERT(edg_scope != NULL);

            // DQ (12/29/2011): This fails for test2011_191.C so we need to visit the scope of the associated template class (not a template instantiation).
            // SgScopeStatement* scopeOfReference = scope_cache[edg_scope];
               SgScopeStatement* scopeOfReference = NULL;
               if (scope_cache.find(edg_scope) == scope_cache.end())
                  {
#if DEBUG_PARSE_USING_DECLARATION
                    printf ("This scope has never been seen before so we have to build it's declaration \n");
#endif
                    scopeOfReference = NULL;
                  }
                 else
                  {
                    scopeOfReference = scope_cache[edg_scope];
                  }

#if DEBUG_PARSE_USING_DECLARATION
               printf ("In parse_using_declaration(): scopeOfReference = %p = %s \n",scopeOfReference,scopeOfReference != NULL ? scopeOfReference->class_name().c_str() : "null");
#endif
            // DQ (12/29/2011): I am going the skip this issue for now and go on...
            // ROSE_ASSERT(scopeOfReference != NULL);
               bool skipByUsingAnAlternativeDeclaration = false;
               if (scopeOfReference != NULL)
                  {
                 // printf ("In parse_using_declaration(): calling scopeOfReference->lookup_variable_symbol(name) for name = %s \n",name.str());
                    SgVariableSymbol* variableSymbol = scopeOfReference->lookup_variable_symbol(name);
                 // printf ("In parse_using_declaration(): variableSymbol = %p \n",variableSymbol);

                 // ROSE_ASSERT(variableSymbol != NULL);
                    if (variableSymbol != NULL)
                       {
                         ROSE_ASSERT(variableSymbol->get_declaration() != NULL );
                         ROSE_ASSERT(variableSymbol->get_declaration()->get_declaration() != NULL );
                         declarationStatement = variableSymbol->get_declaration()->get_declaration();
                         ROSE_ASSERT(declarationStatement != NULL);
                      // result = set_file_info_for_corresp(new SgUsingDeclarationStatement(declarationStatement,NULL), udp->source_corresp);
                         result = set_file_info_for_where_corresp_is_unavailable(new SgUsingDeclarationStatement(declarationStatement,NULL), udp->position);
                         ROSE_ASSERT (result != NULL);
                       }
                      else
                       {
                      // This fails for test2004_35.C (use of STL list header file)
                      // skipByUsingAnAlternativeDeclaration = true;

                         SgEnumFieldSymbol* enumFieldSymbol = scopeOfReference->lookup_enum_field_symbol(name);
                      // ROSE_ASSERT(enumFieldSymbol != NULL);
                         if (enumFieldSymbol != NULL)
                            {

                           // scope->insert_symbol(name,enumFieldSymbol);
                              SgInitializedName* initializedName = enumFieldSymbol->get_declaration();
                              ROSE_ASSERT(initializedName != NULL);

                              ROSE_ASSERT(declarationStatement == NULL);

                           // result = set_file_info_for_where_corresp_is_unavailable(new SgUsingDeclarationStatement(declarationStatement,initializedName), udp->position);
                              SgUsingDeclarationStatement* usingDeclaration = new SgUsingDeclarationStatement(declarationStatement,initializedName);

                           // DQ (11/4/2012): Added call to set file info to default settings.
                              setSourcePosition(usingDeclaration);

                              result = set_file_info_for_where_corresp_is_unavailable(usingDeclaration, udp->position);
                            }
                           else
                            {
                           // This fails for test2012_07.C (use of STL list header file)
                              skipByUsingAnAlternativeDeclaration = true;
                            }
                       }
                  }
                 else
                  {
                    skipByUsingAnAlternativeDeclaration = true;
                  }

            // DQ (12/30/2011): This allows a fall back position so that we can revisit the problem later (when it may be more obvious or at least a bit more clear).
               if (skipByUsingAnAlternativeDeclaration == true)
                  {
                 // FIXME: Can't identify the scope of the referenced constant.
#ifdef DEBUG_CONVERSION
                 // DQ (12/29/2011): This is demonstrated by test2011_191.C ("using ::A<T>::abc;" in a template class definition).
                    printf ("ERROR: I can't associate this constant with the variable declaration (in the associated template --- building a dummy variable declaration) \n");
#endif
#ifdef DEBUG_CONVERSION
                 // DQ (5/9/2013): I want to warn about this.
                    printf ("WARNING: In parse_using_declaration(): Using topScopeStack() as scope for SgVariableDeclaration \n");
#endif
                 // DQ (5/9/2013): If this is a namespace then test to make sure we are inserting the symbol into the correct scope.
                    SgNamespaceDefinitionStatement* namespaceDefinitionStatement = isSgNamespaceDefinitionStatement(topScopeStack());
                    if (namespaceDefinitionStatement != NULL)
                       {
                         ROSE_ASSERT(namespaceDefinitionStatement == namespaceDefinitionStatement->get_global_definition());
                       }

                 // Build a value so that we can go on (template is currently unparsed as a string so not a crisis).
                    SgVariableDeclaration* variableDeclaration = buildVariableDeclaration_nfi(name, SgTypeInt::createType(), NULL, topScopeStack());
                    ROSE_ASSERT(variableDeclaration != NULL);

#if DEBUG_PARSE_USING_DECLARATION
                    printf ("In parse_using_declaration(): variableDeclaration = %p = %s \n",variableDeclaration,variableDeclaration->class_name().c_str());
#endif
                    set_file_info_for_where_corresp_is_unavailable(variableDeclaration->get_definition(), udp->position);
                    set_file_info_for_where_corresp_is_unavailable(variableDeclaration->get_variables()[0], udp->position);

                    declarationStatement = set_file_info_for_where_corresp_is_unavailable(variableDeclaration, udp->position);
                    ROSE_ASSERT(declarationStatement != NULL);

                 // result = set_file_info_for_where_corresp_is_unavailable(new SgUsingDeclarationStatement(declarationStatement,NULL), udp->position);
                    SgUsingDeclarationStatement* usingDeclaration = new SgUsingDeclarationStatement(declarationStatement,NULL);
               
                 // DQ (11/3/2012): Added call to set file info to default settings.
                    setSourcePosition(usingDeclaration);

                    result = set_file_info_for_where_corresp_is_unavailable(usingDeclaration, udp->position);
                  }
#if 0
               ROSE_ASSERT(!"using declaration: iek_constant not supported");
#endif
               break;
             }

          case iek_namespace:
             {
            // Enforce that if this is a namespace then udp->is_using_directive == true
               ROSE_ASSERT(udp->is_using_directive == true);

            // DQ (12/16/2011): Added support for templates to be specified in using declarations.
               a_namespace_ptr edg_namespace = (a_namespace_ptr)(udp->entity.ptr);
               ROSE_ASSERT(edg_namespace != NULL);

#if DEBUG_PARSE_USING_DECLARATION
               printf ("In parse_using_declaration(): calling summarize_namespace(edg_namespace = %p) \n",edg_namespace);
               summarize_namespace(edg_namespace);
#endif
               SgName name = edg_namespace->source_corresp.name;

            // DQ (3/11/2012): This is a newer (simpler) implementation of the using namespace declaration support.
               SgNamespaceDeclarationStatement* namespaceDeclaration = NULL;

            // DQ (8/6/2012): Added support for using declarations to reference namespace aliases.
               if (edg_namespace->is_namespace_alias == true)
                  {
                    a_namespace_ptr assoc_namespace = edg_namespace->variant.assoc_namespace;
                    ROSE_ASSERT(assoc_namespace != NULL);

                 // Note that to support this we need a map of namespaces...
                    ROSE_ASSERT(namespace_cache.find(assoc_namespace) != namespace_cache.end());
                    SgNamespaceDeclarationStatement* associatedNamespace = isSgNamespaceDeclarationStatement(namespace_cache[assoc_namespace]);
                    ROSE_ASSERT(associatedNamespace != NULL);
#if DEBUG_PARSE_USING_DECLARATION
                    mprintf ("In parse_using_declaration(): edg_namespace->is_namespace_alias == true: reusing a SgNamespaceDeclarationStatement = %p \n",associatedNamespace);
#endif
                    namespaceDeclaration = associatedNamespace;
                    ROSE_ASSERT(namespaceDeclaration != NULL);
                  }
                 else
                  {
                 // The assoc_scope is only a valid pointer (of a union) if is_namespace_alias == false.
                    ROSE_ASSERT(edg_namespace->is_namespace_alias == false);
                    ROSE_ASSERT(edg_namespace->variant.assoc_scope != NULL);

                    a_scope_ptr edg_scope = edg_namespace->variant.assoc_scope;
                    ROSE_ASSERT(edg_scope != NULL);

                 // Find the ROSE SgNamespaceDefinitionStatement associated with the first declaration of the namespace (appearing in the using declaration).
                 // Note that the case of "std" is a special case and if we see it AND it does not exist, we need to build it as a namespace.
                    SgScopeStatement* scopeOfReference = NULL;

                 // DQ (3/11/2012): check that this is not already in the list before we add it (or
                 // verify it is NULL if it is supposed to be in the list but just not initialized yet).
                 // ROSE_ASSERT (scope_cache.find(edg_scope) != scope_cache.end());
                    if (scope_cache.find(edg_scope) != scope_cache.end())
                       {
                      // Find the ROSE SgNamespaceDefinitionStatement associated with the first declaration of the namespace (appearing in the using declaration).
                      // Note that the case of "std" is a special case and if we see it AND it does not exist, we need to build it as a namespace.
                         scopeOfReference = scope_cache[edg_scope];
                       }

                 // ROSE_ASSERT(scopeOfReference != NULL);
                    if (scopeOfReference == NULL)
                       {
                         if (name == "std")
                            {
                           // If this is NULL then it could be a reference to the "std" namespace (implicitly defined in C++).
                           // In this case we sould build a proper namespace and add it to the symbol table.

                              mprintf ("Building the implicit 'std' namespace to support the reference to it in parse_using_declaration() name = %s \n",name.str());

                              bool unnamednamespace = false;

                           // DQ (3/6/2012): I think this should be the global scope if not then we should find the global scope for this case of the implicit "std" namespace.
                              scopeOfReference = getGlobalScopeFromScopeStack();
                              ROSE_ASSERT(isSgGlobal(scopeOfReference) != NULL);

                              SgNamespaceDeclarationStatement* std_namespaceDeclaration = set_generated_file_info(buildNamespaceDeclaration_nfi(name,unnamednamespace,scopeOfReference));
                              ROSE_ASSERT(std_namespaceDeclaration != NULL);
                              ROSE_ASSERT(std_namespaceDeclaration->get_startOfConstruct() != NULL);

                              ROSE_ASSERT(std_namespaceDeclaration->get_firstNondefiningDeclaration() != NULL);
                              set_generated_file_info(std_namespaceDeclaration->get_firstNondefiningDeclaration());
                              ROSE_ASSERT(std_namespaceDeclaration->get_firstNondefiningDeclaration()->get_startOfConstruct() != NULL);

                              ROSE_ASSERT(std_namespaceDeclaration->get_definingDeclaration() == NULL);

                              namespaceDeclaration = std_namespaceDeclaration;
                            }
                           else
                            {
                              printf ("ERROR: The namespace should have been seen previously for it to be referenced in a using declaration. \n");
                              ROSE_ASSERT(false);
                            }
                       }
                      else
                       {
                         ROSE_ASSERT(scopeOfReference != NULL);
                         SgNamespaceDefinitionStatement* namespaceDefinition = isSgNamespaceDefinitionStatement(scopeOfReference);
                         ROSE_ASSERT(namespaceDefinition != NULL);

                      // This is now the first declaration for the namespace and what we want to use consistantly even though other 
                      // declarations are allowed in C++ because namespaces are reintrant.
                         namespaceDeclaration = namespaceDefinition->get_namespaceDeclaration();
                         ROSE_ASSERT(namespaceDeclaration != NULL);
                       }
                  }

               ROSE_ASSERT(namespaceDeclaration != NULL);
            // result = set_file_info_for_where_corresp_is_unavailable(new SgUsingDirectiveStatement(namespaceDeclaration), udp->position);
               SgUsingDirectiveStatement* usingDirective = new SgUsingDirectiveStatement(namespaceDeclaration);

            // DQ (11/3/2012): Added call to set file info to default settings.
               setSourcePosition(usingDirective);

               result = set_file_info_for_where_corresp_is_unavailable(usingDirective, udp->position);

               ROSE_ASSERT (result != NULL);

               ROSE_ASSERT(isSgUsingDirectiveStatement(result) != NULL);
#if 0
               ROSE_ASSERT(!"using declaration: iek_namespace not supported");
#endif
               break;
             }

       // Handles case of non-member function and member functions.
          case iek_routine:
             {
               ROSE_ASSERT(udp->is_using_directive == false);

               a_routine_ptr edg_routine = (a_routine_ptr)(udp->entity.ptr);
               ROSE_ASSERT(edg_routine != NULL);

#if DEBUG_PARSE_USING_DECLARATION
               printf ("In parse_using_declaration(): calling summarize_routine(edg_routine = %p) \n",edg_routine);
               summarize_routine(edg_routine);
#endif
               SgName name = edg_routine->source_corresp.name;
               a_scope_ptr edg_scope = edg_routine->source_corresp.parent_scope;
               ROSE_ASSERT(edg_scope != NULL);

#if DEBUG_PARSE_USING_DECLARATION
               printf ("using declaration: name = %s \n",name.str());
               printf ("@@@@@ In parse_using_declaration(): case iek_routine -- edg_scope = %p \n",edg_scope);
               printf ("@@@@@ In parse_using_declaration(): case iek_routine -- edg_scope = %s \n",summarize_scope_in_string(edg_scope));
               printf ("topScopeStack() = %p = %s \n",topScopeStack(),topScopeStack()->class_name().c_str());
#endif

            // DQ (3/11/2012): check that this is not already in the list before we add it (or
            // verify it is NULL if it is supposed to be in the list but just not initialized yet).
               ROSE_ASSERT (scope_cache.find(edg_scope) != scope_cache.end());

               SgScopeStatement* scopeOfReference = scope_cache[edg_scope];

#if 0
               if (scopeOfReference == NULL)
                  {
                    printf ("WARNING: scope_cache[edg_scope] == NULL using topScopeStack() \n");
                    scopeOfReference = topScopeStack();
                  }
#endif
               ROSE_ASSERT(scopeOfReference != NULL);

               SgNamespaceDefinitionStatement* namespaceDefinitionStatement = isSgNamespaceDefinitionStatement(scopeOfReference);
               if (namespaceDefinitionStatement != NULL)
                  {
#if DEBUG_PARSE_USING_DECLARATION
                    printf ("Found the SgNamespaceDefinitionStatement \n");
#endif
                    ROSE_ASSERT(namespaceDefinitionStatement->get_namespaceDeclaration() != NULL);
#if DEBUG_PARSE_USING_DECLARATION
                    printf ("namespaceDefinitionStatement->get_declaration()->get_name() = %s \n",namespaceDefinitionStatement->get_namespaceDeclaration()->get_name().str());
#endif
                  }

            // DQ (7/23/2012): Check if this function has been see before (it should have been processed to be referenced in a using declaration).
               if (first_nondefining_declarations_for_functions.find(edg_routine) == first_nondefining_declarations_for_functions.end()) 
                  {
#if DEBUG_PARSE_USING_DECLARATION
                    printf("WARNING: using declaration refers to non-existing method \"%s\".\n  See Cxx_tests/rose-1540-0.C\n",name.str());
#endif
                    declarationStatement = convert_routine_prototype(edg_routine, false, NULL);
                    ROSE_ASSERT(declarationStatement != NULL);

                    ROSE_ASSERT(declarationStatement->get_startOfConstruct() != NULL);
                    declarationStatement->get_startOfConstruct()->setCompilerGenerated();

                    ROSE_ASSERT(declarationStatement->get_endOfConstruct() != NULL);
                    declarationStatement->get_endOfConstruct()->setCompilerGenerated();
                  }
                 else
                  {
                    declarationStatement = first_nondefining_declarations_for_functions[edg_routine];
                  }

               ROSE_ASSERT(declarationStatement != NULL);
            // result = set_file_info_for_corresp(new SgUsingDeclarationStatement(declarationStatement,NULL), udp->source_corresp);
            // result = set_file_info_for_where_corresp_is_unavailable(new SgUsingDeclarationStatement(declarationStatement,NULL), udp->position);
               SgUsingDeclarationStatement* usingDeclaration = new SgUsingDeclarationStatement(declarationStatement,NULL);
               
            // DQ (11/3/2012): Added call to set file info to default settings.
               setSourcePosition(usingDeclaration);

               result = set_file_info_for_where_corresp_is_unavailable(usingDeclaration, udp->position);

               ROSE_ASSERT (result != NULL);
#if 0
               ROSE_ASSERT(!"using declaration: iek_routine not supported");
#endif
               break;
             }

       // Handles case of class, enum, and typedef
          case iek_type:
             {
               ROSE_ASSERT(udp->is_using_directive == false);

               a_type_ptr edg_type = (a_type_ptr)(udp->entity.ptr);
               ROSE_ASSERT(edg_type != NULL);

#if DEBUG_PARSE_USING_DECLARATION
               printf ("In parse_using_declaration(): calling summarize_type(edg_type = %p) \n",edg_type);
               summarize_type(edg_type);
#endif
               SgName name = edg_type->source_corresp.name;
               a_scope_ptr edg_scope = edg_type->source_corresp.parent_scope;
               ROSE_ASSERT(edg_scope != NULL);

            // DQ (3/11/2012): check that this is not already in the list before we add it (or
            // verify it is NULL if it is supposed to be in the list but just not initialized yet).
            // ROSE_ASSERT (scope_cache.find(edg_scope) != scope_cache.end());
               if (scope_cache.find(edg_scope) == scope_cache.end())
                  {
                 // DQ (1/28/2019): This appears to be part of the scope of the template declaration (assoc_scope).
#if DEBUG_PARSE_USING_DECLARATION
                    printf ("In parse_using_declaration(): case iek_type: We have not seen this scope = %p previously, so it might need to be processed \n",edg_scope);
                    printf ("   --- name = %s \n",name.str());
#endif
                    bool pointerToMember         = false;
                    bool from_template_parameter = false;
                    SgType* rose_type = convert_type(edg_type,pointerToMember,from_template_parameter);
                    ROSE_ASSERT(rose_type != NULL);
                  }

               SgScopeStatement* scopeOfReference = getDeclarationScope (edg_type->source_corresp, edg_type->kind == tk_template_param);
               ROSE_ASSERT(scopeOfReference != NULL);
#if 0
            // DQ (5/20/2013): This is not handled within the SgScopeStatement level of abstraction where for symbol table handling we 
            // check if the scope is a SgNamespaceDefinition, and then use a specially constructed SgNamespaceDefinition that holds 
            // SgAliasSymbols to all of the declarations associated with the many reentrant namespace defintion scopes.
            // DQ (5/9/2013): Added test to allow detection of where we want to direct declarations to a specific scope where namespace 
            // scoped declarations will be accumulated.  This will at first be the first non-definig declaration's associated namespace 
            // definition.  But later it may be a especially set aside SgNamespaceDefinitionStatement for just this purpose.
               SgNamespaceDefinitionStatement* namespaceDefinitionStatement = isSgNamespaceDefinitionStatement(scopeOfReference);
               if (namespaceDefinitionStatement != NULL)
                  {
                 // DQ (5/9/2013): We need to correct the scope of reference to be the first non-defining declaration since there can be many namespace definitions (reentrant).
                    printf ("ERROR: need to make scopeOfReference = %p to namespaceDefinitionStatement = %p \n",namespaceDefinitionStatement,namespaceDefinitionStatement->get_global_definition());
                    ROSE_ASSERT(false);
                  }
#endif
               switch(edg_type->kind)
                  {
                 // DQ (1/29/2019): Adding new case (see Cxx11_tests/test2019_46.C).
                    case tk_template_param:
                       {
                         SgNonrealSymbol* nrSymbol = scopeOfReference->lookup_nonreal_symbol(name);
                         ROSE_ASSERT(nrSymbol != NULL);

                         ROSE_ASSERT(nrSymbol->get_declaration() != NULL);
                         declarationStatement = nrSymbol->get_declaration();
                         ROSE_ASSERT(declarationStatement != NULL);

                         break;
                       }

                    case tk_class:
                    case tk_struct:
                    case tk_union:
                       {
                         SgClassSymbol* classSymbol = scopeOfReference->lookup_class_symbol(name);
                         ROSE_ASSERT(classSymbol != NULL);

                         ROSE_ASSERT(classSymbol->get_declaration() != NULL);
                         declarationStatement = classSymbol->get_declaration();
                         ROSE_ASSERT(declarationStatement != NULL);
                         break;
                       }

                    case tk_typeref:
                       {
                      // DQ (9/2/2012): Note that we have to be careful when searching for symbols in namespaces...since there can be many of SgNamespaceDefinitionStatements.
                         SgTypedefSymbol* typedefSymbol = scopeOfReference->lookup_typedef_symbol(name);
                         if (typedefSymbol == NULL)
                            {
                           // DQ (9/2/2012): This is an alternative appraoch (we realy only need one, but since typedefSymbol
                           // can be NULL we need the backup mechanism (to handle boost in the projects/haskell_port test)).
#if DEBUG_PARSE_USING_DECLARATION
                              printf ("In parse_using_declaration(): SgTypedefSymbol not found in scope = %p = %s = %s for name = %s \n",scopeOfReference,scopeOfReference->class_name().c_str(),SageInterface::get_name(scopeOfReference).c_str(),name.str());
                              scopeOfReference->get_startOfConstruct()->display("In parse_using_declaration(): SgTypedefSymbol not found in scope: debug");
#endif
#if DEBUG_PARSE_USING_DECLARATION
                              printf ("In parse_using_declaration(): calling summarize_using_declaration(udp = %p) \n",udp);
                              summarize_using_declaration(udp);
#endif
#if DEBUG_PARSE_USING_DECLARATION
                              printf ("In parse_using_declaration(): case iek_type: case tk_typeref: calling summarize_type(edg_type = %p) \n",edg_type);
                              summarize_type(edg_type);
#endif
                              bool previouslyProcessed = (type_translation_cache.find(edg_type) != type_translation_cache.end());
#if DEBUG_PARSE_USING_DECLARATION
                              printf ("previouslyProcessed = %s \n",previouslyProcessed ? "true" : "false");
#endif
                           // DQ (8/14/2014): If this is for MS Windows, then the size_t has been implicitly defined internal to the
                           // compiler and there is no associated typedef declaration to use with the SgUsingDeclarationStatement.
                           // string name = edg_type->source_corresp.name;
                              bool name_is_size_t = (name == "size_t");
                              if (name_is_size_t == true)
                                 {
                                   printf ("Detected SgUsingDeclarationStatement with size_t \n");
                                 }

                              SgType* rose_type = NULL;
                              if (previouslyProcessed == true)
                                 {
#if DEBUG_PARSE_USING_DECLARATION
                                   printf ("This type has been processed already! \n");
#endif
                                   rose_type = type_translation_cache[edg_type];
                                   ROSE_ASSERT(rose_type != NULL);
                                 }
                                else
                                 {
                                   rose_type = convert_type(edg_type);
                                   ROSE_ASSERT(rose_type != NULL);
                                 }
#if DEBUG_PARSE_USING_DECLARATION
                              printf ("rose_type = %p = %s \n",rose_type,rose_type->class_name().c_str());
#endif
                              SgTypedefType* typedefType = isSgTypedefType(rose_type);

                           // DQ (8/14/2014): This fails for MS Windows iostream header file.
                           // ROSE_ASSERT(typedefType != NULL);
                              if (typedefType == NULL)
                                 {
                                // DQ (8/14/2014): For Windows the size_t type is not installation dependent and is 
                                // known to be cached as a builtin type (so there is not associated declaration).
                                   ROSE_ASSERT(edg_type->is_instantiation_dependent == false);
                                   ROSE_ASSERT(edg_type->is_instantiation_dependent_cached == true);
                                   SgModifierType* modifierType = isSgModifierType(rose_type);
                                   if (modifierType != NULL)
                                      {
#if DEBUG_PARSE_USING_DECLARATION
                                        printf ("modifierType->get_base_type() = %p = %s \n",modifierType->get_base_type(),modifierType->get_base_type()->class_name().c_str());
#endif
                                        if (name_is_size_t == true)
                                           {
                                          // DQ (8/14/2014): This happens only under MS Windows (but can be reproduced on a Linux machine in EDG's microsoft mode.
                                          // This solution is to generate a typedef and put it into the set the correct scope but not put it into the scopes declaration list.

                                             SgType* base_type       = modifierType->get_base_type();
                                             SgScopeStatement* scope = scopeOfReference;

                                             SgTypedefDeclaration* typedefDeclaration_for_builtin_size_t = buildTypedefDeclaration_nfi(name,base_type,scope);

                                             typedefType = typedefDeclaration_for_builtin_size_t->get_type();
                                             ROSE_ASSERT(typedefType != NULL);

                                          // We need to add this to the symbol table as well (if not done by the builder).

                                          // Set the parent.
                                             typedefDeclaration_for_builtin_size_t->set_parent(scope);
#if 0
                                             printf ("ERROR: Detected implicitly declared size_t (not clear what declaration should be used with the SgUsingDeclarationStatement) \n");
                                             ROSE_ASSERT(false);
#endif
                                           }
                                      }
                                 }
                              ROSE_ASSERT(typedefType != NULL);

                              declarationStatement = typedefType->get_declaration();
                              ROSE_ASSERT(declarationStatement != NULL);
                            }
                           else
                            {
                              ROSE_ASSERT(typedefSymbol != NULL);

                              ROSE_ASSERT(typedefSymbol->get_declaration() != NULL);
                              declarationStatement = typedefSymbol->get_declaration();
                            }

                         ROSE_ASSERT(declarationStatement != NULL);
                         break;
                       }

                    case tk_enum:
                       {
                         SgEnumSymbol* enumSymbol = scopeOfReference->lookup_enum_symbol(name);
                         ROSE_ASSERT(enumSymbol != NULL);

                         ROSE_ASSERT(enumSymbol->get_declaration() != NULL);
                         declarationStatement = enumSymbol->get_declaration();
                         ROSE_ASSERT(declarationStatement != NULL);
                         break;
                       }

                    default:
                       {
                         printf ("Error: default reached in switch in parse_using_declaration() case iek_type \n");
                         ROSE_ASSERT(false);
                       }
                  }

               ROSE_ASSERT(declarationStatement != NULL);
            // result = set_file_info_for_where_corresp_is_unavailable(new SgUsingDeclarationStatement(declarationStatement,NULL), udp->position);
               SgUsingDeclarationStatement* usingDeclaration = new SgUsingDeclarationStatement(declarationStatement,NULL);

            // DQ (11/4/2012): Added call to set file info to default settings.
               setSourcePosition(usingDeclaration);

               result = set_file_info_for_where_corresp_is_unavailable(usingDeclaration, udp->position);

            // DQ (1/11/2019): Adding support for inheriting constructor feature (C++11).
               if (udp->is_inheriting_ctor == true)
                  {
                    usingDeclaration->set_is_inheriting_constructor(true);
                  }

               ROSE_ASSERT (result != NULL);
#if 0
               ROSE_ASSERT(!"using declaration: iek_type not supported");
#endif
               break;
             }

          case iek_template:
             {
               ROSE_ASSERT(udp->is_using_directive == false);

            // DQ (12/16/2011): Added support for templates to be specified in using declarations.
               a_template_ptr edg_template = (a_template_ptr)(udp->entity.ptr);
               ROSE_ASSERT(edg_template != NULL);

#if DEBUG_PARSE_USING_DECLARATION
               printf ("In parse_using_declaration(): calling summarize_template(edg_template = %p) \n",edg_template);
               summarize_template(edg_template);
#endif
#if DEBUG_PARSE_USING_DECLARATION
               printf ("Calling getAssociatedTemplateSymbol(): edg_template = %p declarationStatement = %p = %s \n",
                       edg_template,declarationStatement,(declarationStatement != NULL) ? declarationStatement->class_name().c_str() : "null");
#endif
            // Refactored this code to a seperate function.
               SgSymbol* templateSymbol = getAssociatedTemplateSymbol(edg_template,declarationStatement);
               ROSE_ASSERT(templateSymbol != NULL);
               ROSE_ASSERT(declarationStatement != NULL);
#if 0
               printf ("Exiting after call to getAssociatedTemplateSymbol() \n");
               ROSE_ASSERT(false);
#endif
            // result = set_file_info_for_where_corresp_is_unavailable(new SgUsingDeclarationStatement(declarationStatement,NULL), udp->position);
               SgUsingDeclarationStatement* usingDeclaration = new SgUsingDeclarationStatement(declarationStatement,NULL);

            // DQ (11/4/2012): Added call to set file info to default settings.
               setSourcePosition(usingDeclaration);

               result = set_file_info_for_where_corresp_is_unavailable(usingDeclaration, udp->position);

               ROSE_ASSERT (result != NULL);
#if 0
               ROSE_ASSERT(!"using declaration: iek_template not supported");
#endif
               break;
             }

       // DQ (2/11/2012): Added support for using declarations with references to data fields.
          case iek_field:
             {
               ROSE_ASSERT(udp->is_using_directive == false);

               a_field_ptr edg_field = (a_field_ptr)(udp->entity.ptr);
               ROSE_ASSERT(edg_field != NULL);

#if DEBUG_PARSE_USING_DECLARATION
               printf ("In parse_using_declaration(): calling summarize_field(edg_field = %p) \n",edg_field);
               summarize_field(edg_field);
#endif
               SgName name = edg_field->source_corresp.name;
               a_scope_ptr edg_scope = edg_field->source_corresp.parent_scope;
               ROSE_ASSERT(edg_scope != NULL);

#if DEBUG_PARSE_USING_DECLARATION
               printf ("using declaration: name = %s \n",name.str());
               printf ("@@@@@ In parse_using_declaration(): case iek_field -- edg_scope = %p \n",edg_scope);
               printf ("@@@@@ In parse_using_declaration(): case iek_field -- edg_scope = %s \n",summarize_scope_in_string(edg_scope));
               printf ("topScopeStack() = %p = %s \n",topScopeStack(),topScopeStack()->class_name().c_str());
#endif
            // DQ (3/11/2012): check that this is not already in the list before we add it (or
            // verify it is NULL if it is supposed to be in the list but just not initialized yet).
               ROSE_ASSERT (scope_cache.find(edg_scope) != scope_cache.end());

               SgScopeStatement* scopeOfReference = scope_cache[edg_scope];
               ROSE_ASSERT(scopeOfReference != NULL);
#if 0
               scopeOfReference->get_symbol_table()->print("using declaration");
#endif
               SgVariableSymbol* variableSymbol = scopeOfReference->lookup_variable_symbol(name);
               ROSE_ASSERT(variableSymbol != NULL);

               ROSE_ASSERT(variableSymbol->get_declaration() != NULL);

            // DQ (2/11/2012): As I recall we have to look two levels deep for variable declarations.
            // declarationStatement = variableSymbol->get_declaration();
               declarationStatement = variableSymbol->get_declaration()->get_declaration();

               ROSE_ASSERT(declarationStatement != NULL);
            // result = set_file_info_for_where_corresp_is_unavailable(new SgUsingDeclarationStatement(declarationStatement,NULL), udp->position);
               SgUsingDeclarationStatement* usingDeclaration = new SgUsingDeclarationStatement(declarationStatement,NULL);

            // DQ (11/4/2012): Added call to set file info to default settings.
               setSourcePosition(usingDeclaration);

               result = set_file_info_for_where_corresp_is_unavailable(usingDeclaration, udp->position);


               ROSE_ASSERT (result != NULL);
#if 0
               ROSE_ASSERT(!"using declaration: iek_field not supported");
#endif
               break;
             }

       // DQ (7/21/2012): lambda cases that we want to cover (but might not be used here).
          case iek_lambda:
             {
               ROSE_ASSERT(!"using declaration: iek_lambda not supported");
               break;
             }

       // DQ (7/21/2012): lambda cases that we want to cover (but might not be used here).
          case iek_lambda_capture:
             {
               ROSE_ASSERT(!"using declaration: iek_lambda_capture not supported");
               break;
             }

       // DQ (9/19/2022): Added new case appearing in use of EDG 6.3 for the first time.
          case iek_base_class:
             {
               ROSE_ASSERT(udp->is_using_directive == false);

               a_base_class_ptr edg_base_class = (a_base_class_ptr)(udp->entity.ptr);
               ROSE_ASSERT(edg_base_class != NULL);

#if DEBUG_PARSE_USING_DECLARATION && 0
               printf ("In parse_using_declaration(): calling summarize_base_class(edg_base_class = %p) \n",edg_base_class);
               summarize_base_class(edg_base_class);
#endif

#if DEBUG_PARSE_USING_DECLARATION
               a_type_ptr edg_class_type          = edg_base_class->type;
               a_type_ptr edg_original_class_type = edg_base_class->orig_type;
               a_type_ptr edg_derived_class_type  = edg_base_class->derived_class;
#if 0
            // DQ (9/20/2022): This is the class that we want...
               printf ("In parse_using_declaration(): calling summarize_type(edg_class_type = %p) \n",edg_class_type);
               summarize_type(edg_class_type);
#endif
#if 0
            // DQ (9/20/2022): This is just the class where the using directive is located.
               printf ("In parse_using_declaration(): calling summarize_type(edg_derived_class_type = %p) \n",edg_derived_class_type);
               summarize_type(edg_derived_class_type);
#endif
#endif
               a_type_ptr edg_type   = edg_base_class->type;
               ROSE_ASSERT(edg_type != NULL);
               a_scope_ptr edg_scope = edg_type->source_corresp.parent_scope;
               ROSE_ASSERT(edg_scope != NULL);

               // printf ("In parse_using_declaration(): calling summarize_type(edg_type = %p) \n",edg_type);
               // summarize_type(edg_type);

               // printf ("In parse_using_declaration(): calling summarize_type(edg_type = %p) \n",edg_type);
               // summarize_scope(edg_scope);

            // DQ (3/11/2012): check that this is not already in the list before we add it (or
            // verify it is NULL if it is supposed to be in the list but just not initialized yet).
            // ROSE_ASSERT (scope_cache.find(edg_scope) != scope_cache.end());
               if (scope_cache.find(edg_scope) == scope_cache.end())
                  {
                 // DQ (1/28/2019): This appears to be part of the scope of the template declaration (assoc_scope).
#if DEBUG_PARSE_USING_DECLARATION
                    printf ("In parse_using_declaration(): case iek_type: We have not seen this scope = %p previously, so it might need to be processed \n",edg_scope);
                    printf ("   --- name = %s \n",name.str());
#endif
                    bool pointerToMember         = false;
                    bool from_template_parameter = false;
                    SgType* rose_type = convert_type(edg_type,pointerToMember,from_template_parameter);
                    ROSE_ASSERT(rose_type != NULL);
                  }

               SgScopeStatement * scopeOfReference = nullptr;
               bool is_nonreal_class = (
                                   edg_type->kind == tk_struct ||
                                   edg_type->kind == tk_class  ||
                                   edg_type->kind == tk_union
                                 ) && edg_type->variant.class_struct_union.is_nonreal_class;
               if (is_nonreal_class) {
                   if (nonreal_scope_cache.find(edg_scope) != nonreal_scope_cache.end()) {
                       scopeOfReference = nonreal_scope_cache[edg_scope];
                   } else {
                       ROSE_ASSERT(nonreal_scope_for_template_parameters_stack.size() > 0);
                       scopeOfReference = nonreal_scope_for_template_parameters_stack[nonreal_scope_for_template_parameters_stack.size()-1];
                   }
               } else {
                   scopeOfReference = getDeclarationScope (edg_type->source_corresp, (edg_type->kind == tk_template_param));
               }
               ROSE_ASSERT(scopeOfReference != NULL);
               // printf ("scopeOfReference = %p = %s \n",scopeOfReference, scopeOfReference->class_name().c_str());

               SgName name = edg_type->source_corresp.name;
               SgSymbol * sym = scopeOfReference->lookup_symbol(name);
               ROSE_ASSERT(sym != nullptr);
               // printf ("sym = %p = %s \n",sym, sym->class_name().c_str());

               if (isSgClassSymbol(sym)) {
                 declarationStatement = ((SgClassSymbol*)sym)->get_declaration();
               } else if (isSgNonrealSymbol(sym)) {
                 declarationStatement = ((SgNonrealSymbol*)sym)->get_declaration();
               }
               ROSE_ASSERT(declarationStatement != NULL);

               SgUsingDeclarationStatement * usingDeclaration = new SgUsingDeclarationStatement(declarationStatement,NULL);

            // DQ (9/20/2022): Note sure if we should not set the source position more directly.
               setSourcePosition(usingDeclaration);

               ROSE_ASSERT(udp->is_inheriting_ctor == true);
               usingDeclaration->set_is_inheriting_constructor(true);

               result = set_file_info_for_where_corresp_is_unavailable(usingDeclaration, udp->position);

            // ROSE_ASSERT(!"using declaration: iek_base_class not supported (note: was never required until EDG 6.3)");
               break;
             }

          default:
             {
               printf ("Error: default reached in switch in parse_using_declaration(): entry_kind = %s \n",summarize_entry_kind_in_string_kind(entry_kind));
               ROSE_ASSERT(false);
               break;
             }
        }

  // ROSE_ASSERT(udp->is_using_directive == false);

#if 0
  // The source_correspondence_pointer for what is being used was computed above
  // SgSymbol* symbol = get_symbol_ptr(*scp);
     SgSymbol* symbol = get_symbol_ptr(*scp);
     ROSE_ASSERT(symbol != NULL);

#error "DEAD CODE!"

     switch (symbol->variantT())
        {
          case V_SgVariableSymbol:
             {
               ROSE_ASSERT( isSgVariableSymbol(symbol) != NULL );
               ROSE_ASSERT( isSgVariableSymbol(symbol)->get_declaration() != NULL );
               ROSE_ASSERT( isSgVariableSymbol(symbol)->get_declaration()->get_declaration() != NULL );
               declarationStatement = isSgVariableSymbol(symbol)->get_declaration()->get_declaration();
               ROSE_ASSERT(declarationStatement != NULL);
               result = new SgUsingDeclarationStatement(declarationStatement,NULL);
               ROSE_ASSERT (result != NULL);
               break;
             }

#error "DEAD CODE!"

          case V_SgNamespaceSymbol:
             {
               SgNamespaceSymbol* namespaceSymbol = isSgNamespaceSymbol(symbol);
               ROSE_ASSERT(namespaceSymbol != NULL);

            // DQ (8/30/2009): This code does not yet support the new namespace aliasing features!
#ifndef _MSC_VER
// DQ (11/29/2009): MSVC does not support "#warning" as a CPP directive.
#warning "DQ (8/30/2009): This code does not yet support the new namespace aliasing features!"
#endif
               declarationStatement = namespaceSymbol->get_declaration();
               ROSE_ASSERT(declarationStatement != NULL);
               result = new SgUsingDeclarationStatement(declarationStatement,NULL);
               ROSE_ASSERT (result != NULL);
               break;
             }

#error "DEAD CODE!"

          case V_SgFunctionSymbol:
             {
               SgFunctionSymbol* functionSymbol = isSgFunctionSymbol(symbol);
               ROSE_ASSERT(functionSymbol != NULL);
               declarationStatement = functionSymbol->get_declaration();
               ROSE_ASSERT(declarationStatement != NULL);
               result = new SgUsingDeclarationStatement(declarationStatement,NULL);
               ROSE_ASSERT (result != NULL);
               break;
             }

          case V_SgMemberFunctionSymbol:
             {
               SgMemberFunctionSymbol* memberFunctionSymbol = isSgMemberFunctionSymbol(symbol);
               ROSE_ASSERT(memberFunctionSymbol != NULL);
               declarationStatement = memberFunctionSymbol->get_declaration();
               ROSE_ASSERT(declarationStatement != NULL);
               result = new SgUsingDeclarationStatement(declarationStatement,NULL);
               ROSE_ASSERT (result != NULL);
               break;
             }

#error "DEAD CODE!"

          case V_SgClassSymbol:
             {
               SgClassSymbol* classSymbol = isSgClassSymbol(symbol);
               ROSE_ASSERT(classSymbol != NULL);
               declarationStatement = classSymbol->get_declaration();
               ROSE_ASSERT(declarationStatement != NULL);
               result = new SgUsingDeclarationStatement(declarationStatement,NULL);
               ROSE_ASSERT (result != NULL);
               break;
             }

          case V_SgTypedefSymbol:
             {
               SgTypedefSymbol* typedefSymbol = isSgTypedefSymbol(symbol);
               ROSE_ASSERT(typedefSymbol != NULL);
               declarationStatement = typedefSymbol->get_declaration();
               ROSE_ASSERT(declarationStatement != NULL);
               result = new SgUsingDeclarationStatement(declarationStatement,NULL);
               ROSE_ASSERT (result != NULL);
               break;
             }

#error "DEAD CODE!"

          case V_SgTemplateSymbol:
             {
               SgTemplateSymbol* templateSymbol = isSgTemplateSymbol(symbol);
               ROSE_ASSERT(templateSymbol != NULL);
               declarationStatement = templateSymbol->get_declaration();
               ROSE_ASSERT(declarationStatement != NULL);
               result = new SgUsingDeclarationStatement(declarationStatement,NULL);
               ROSE_ASSERT (result != NULL);
               break;
             }

       // DQ (7/20/2005): Added this case to support test2005_114.C (from Peter)
       // "struct T { enum c { i }; }; struct U : T { using T::i; };"
          case V_SgEnumFieldSymbol:
             {
               SgEnumFieldSymbol* enumFieldSymbol = isSgEnumFieldSymbol(symbol);
               ROSE_ASSERT(enumFieldSymbol != NULL);
               SgInitializedName* initializedName = enumFieldSymbol->get_declaration();
               ROSE_ASSERT(initializedName != NULL);
               result = new SgUsingDeclarationStatement(NULL,initializedName);
               ROSE_ASSERT (result != NULL);
               break;
             }

#error "DEAD CODE!"

       // DQ (5/22/2007): Added support for enum types in using declaration (test2007_50.C).
          case V_SgEnumSymbol:
             {
               SgEnumSymbol* enumSymbol = isSgEnumSymbol(symbol);
               ROSE_ASSERT(enumSymbol != NULL);
               declarationStatement = enumSymbol->get_declaration();
               ROSE_ASSERT(declarationStatement != NULL);
               result = new SgUsingDeclarationStatement(declarationStatement,NULL);
               ROSE_ASSERT (result != NULL);
               break;
             }

#error "DEAD CODE!"

          default:
             {
               printf ("ERROR: Default reached in parse_using_declaration(): symbol = %s \n",symbol->sage_class_name());
               declarationStatement = NULL;
               ROSE_ASSERT(false);
             }
        }
#endif

  // DQ (7/20/2005): Changed to handle SgEnumFieldSymbol where the declaration is really a SgInitializedName!
     ROSE_ASSERT (result != NULL);

  // sourcePositionData.initializeSourcePositionInformation(result);

     ROSE_ASSERT (result->get_startOfConstruct() != NULL);
     ROSE_ASSERT (result->get_endOfConstruct()   != NULL);

  // DQ (7/14/2012): Enforce a few rules.
     ROSE_ASSERT(isSgUsingDeclarationStatement(result) != NULL || isSgUsingDirectiveStatement(result) != NULL);

  // DQ (7/14/2012): Set the parent (always structural, so OK to use the topScopeStack()).
     ROSE_ASSERT(result->get_parent() == NULL);
     result->set_parent(topScopeStack());
     ROSE_ASSERT(result->get_parent() != NULL);

#if DEBUG_PARSE_USING_DECLARATION
     printf ("Leaving parse_using_declaration(): result = %p = %s setting firstNondefiningDeclaration to %p and definingDeclaration to %p \n",result,result->class_name().c_str(),result,result);
#endif

  // Liao, 10/31/2008
     result->set_firstNondefiningDeclaration(result);
     result->set_definingDeclaration(result);

  // DQ (8/13/2020): Added support for access modifiers.
     unsigned int access_level = udp->access;
     set_access_modifier(result,access_level);

#if 0
     printf ("Exiting as a test! \n");
     ROSE_ASSERT(false);
#endif

     return result;
   }
