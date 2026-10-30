
#include "edgRose.h"

SgDeclarationStatement*
EDG_ROSE_Translation::parse_namespace(SourceSequenceList& sse)
   {
     a_namespace_ptr nsp = (a_namespace_ptr)(sse->entity.ptr);
     ++sse;

#ifdef DEBUG_CONVERSION
  // indent_line(); debug_print("namespace declaration statement"); debug_print("\n");
     indent_line(); debug_print("namespace declaration statement (parse) name = %s is_namespace_alias = %s ",nsp->source_corresp.name, nsp->is_namespace_alias ? "true" : "false"); print_source_corresp(nsp->source_corresp); debug_print("\n");
     ScopedIndent si(indent);
#endif

     SgScopeStatement* current_scope = topScopeStack();
     ROSE_ASSERT(current_scope != NULL);

  // This is the return value.
     SgDeclarationStatement* namespaceDeclaration = NULL;

     string namespace_name = generate_namespace_name(nsp);

     bool isUnnamedNamespace = false;
     if (namespace_name == "")
        {
#ifdef DEBUG_CONVERSION
          printf ("In parse_namespace(): detected un-named namespace \n");
#endif
          isUnnamedNamespace = true;
        }

     if (nsp->is_namespace_alias == true)
        {
       // A namespace alias declaration, e.g., namespace alias_name = existing_name;
       // gen_name(&nsp->variant.assoc_namespace->source_corresp,iek_namespace, GN_NO_OPTIONS, (a_boolean *)NULL);

          ROSE_ASSERT(nsp->variant.assoc_namespace != NULL);
#if 0
          printf ("In parse_namespace(): (nsp->is_namespace_alias == true): calling summarize_namespace(nsp = %p) \n",nsp);
          summarize_namespace(nsp);
#endif
       // DQ (4/8/2018): Moved this declaration from below.
          a_namespace_ptr aliased_nsp = nsp->variant.assoc_namespace;
          ROSE_ASSERT(aliased_nsp != NULL);

       // A namespace alias declaration, e.g., namespace alias_name = existing_name;
       // SgName aliasedName = sage_gen_name(&nsp->variant.assoc_namespace->source_corresp, iek_namespace, GN_NO_OPTIONS, (a_boolean *)NULL /* ,listOfScopeSymbols */ );
          SgName aliasedName = nsp->variant.assoc_namespace->source_corresp.name;
          ROSE_ASSERT(aliasedName.is_null() == false);
#if 0
          printf ("nsp->is_namespace_alias == true: namespace_name = %s aliasedName = %s \n",namespace_name.c_str(),aliasedName.str());
#endif
       // SgDeclarationStatement* aliasedNamespace = convert_namespace(nsp->variant.assoc_namespace,sse);

          SgName name = namespace_name;
          ROSE_ASSERT(name.is_null() == false);

       // DQ (4/8/2018): Implementing this to have simpler logic (hope that will not be a mistake).
          SgNamespaceDeclarationStatement*      aliased_namespace = NULL;
          SgNamespaceAliasDeclarationStatement* aliased_aliased_namespace = NULL;

          SgNamespaceAliasDeclarationStatement* tmp_namespaceAliasDeclarationStatement = NULL;

#if 0
          printf ("namespace_cache.find(aliased_nsp) != namespace_cache.end()             = %s \n",namespace_cache.find(aliased_nsp) != namespace_cache.end() ? "true" : "false");
          printf ("namespace_alias_cache.find(aliased_nsp) != namespace_alias_cache.end() = %s \n",namespace_alias_cache.find(aliased_nsp) != namespace_alias_cache.end() ? "true" : "false");
#endif
#if 0
          printf ("aliased_nsp->is_namespace_alias = %s \n",aliased_nsp->is_namespace_alias ? "true" : "false");
#endif

       // DQ (4/10/2018): These are two different caches into which we search for the aliased_nsp to get the different 
       // ROSE IR node (either a SgNamespaceDeclarationStatement or a SgNamespaceAliasDeclarationStatement).
          if (aliased_nsp->is_namespace_alias == true)
             {
            // This is the special case of a namespace alias of a namespace alias (which is allowed).
#if 0
               printf ("This is the special case of a namespace alias of a namespace alias (which is allowed) \n");
#endif
               ROSE_ASSERT(namespace_alias_cache.find(aliased_nsp) != namespace_alias_cache.end());

            // Should this also be true?
               ROSE_ASSERT(namespace_cache.find(aliased_nsp) != namespace_cache.end());

            // DQ (4/10/2018): This is the SgNamespaceDeclarationStatement at the end of any chain of namespace aliasing.
               aliased_namespace = namespace_cache[aliased_nsp];
               ROSE_ASSERT(aliased_namespace != NULL);

               tmp_namespaceAliasDeclarationStatement = new SgNamespaceAliasDeclarationStatement(name,aliased_namespace);
               ROSE_ASSERT(tmp_namespaceAliasDeclarationStatement != NULL);

               if (namespace_alias_cache.find(aliased_nsp) != namespace_alias_cache.end())
                  {
                    aliased_aliased_namespace = namespace_alias_cache[aliased_nsp];
                    ROSE_ASSERT(aliased_aliased_namespace != NULL);

                    tmp_namespaceAliasDeclarationStatement->set_is_alias_for_another_namespace_alias(true);
                    tmp_namespaceAliasDeclarationStatement->set_namespaceAliasDeclaration(aliased_aliased_namespace);
                  }
                 else
                  {
                    printf ("The namespace being aliased (which is itself a namespace alias) should have been seen (and built) before now! \n");
                    ROSE_ASSERT(false);
                  }
             }
            else
             {
               if (namespace_cache.find(aliased_nsp) == namespace_cache.end())
                  {
                 // This can be the case of "namespace s = std;" where std is only implicitly defined.
#if 0
                    printf ("Building a namespace for this namespace that is being aliased: name = %s \n",name.str());
#endif
                    aliased_namespace = construct_namespace(aliased_nsp);
                    ROSE_ASSERT(aliased_namespace != NULL);
#if 0
                    printf ("Put the new namespace (name = %s) into the namespace_cache at %p \n",name.str(),aliased_nsp);
#endif
                    namespace_cache[aliased_nsp] = aliased_namespace;
                  }
                 else
                  {
#if 0
                    printf ("namespace_cache.find(aliased_nsp) != namespace_cache.end() \n");
#endif
                  }

               if (namespace_cache.find(aliased_nsp) != namespace_cache.end())
                  {
                    aliased_namespace = namespace_cache[aliased_nsp];
                    ROSE_ASSERT(aliased_namespace != NULL);
                    tmp_namespaceAliasDeclarationStatement = new SgNamespaceAliasDeclarationStatement(name,aliased_namespace);
                  }
                 else
                  {
                    printf ("The namespace being aliased should have been seen (and built) before now! \n");
                    ROSE_ASSERT(false);
                  }
             }

          ROSE_ASSERT(aliased_namespace != NULL);
          ROSE_ASSERT(aliased_namespace != NULL || aliased_aliased_namespace != NULL);

       // SgNamespaceAliasDeclarationStatement* tmp_namespaceAliasDeclarationStatement = new SgNamespaceAliasDeclarationStatement(name,aliased_namespace);
          ROSE_ASSERT(tmp_namespaceAliasDeclarationStatement != NULL);

       // DQ (4/8/2018): Add the aliased namespace currently being processed to the cache od namespace aliases.
       // if (namespace_alias_cache.find(aliased_nsp) == namespace_alias_cache.end())
          if (namespace_alias_cache.find(nsp) == namespace_alias_cache.end())
             {
            // namespace_alias_cache[aliased_nsp] = tmp_namespaceAliasDeclarationStatement;
               namespace_alias_cache[nsp] = tmp_namespaceAliasDeclarationStatement; 
             }

          SgNamespaceAliasDeclarationStatement* aliased_namespace_in_source = NULL;
       // DQ (4/8/2018): If the aliased namespace is itself an alias, then we need to handle this differently.
          if (nsp->variant.assoc_namespace->is_namespace_alias == true)
             {
               if (namespace_alias_cache.find(aliased_nsp) != namespace_alias_cache.end())
                  {
                    aliased_namespace_in_source = namespace_alias_cache[aliased_nsp];
                    ROSE_ASSERT(aliased_namespace_in_source != NULL);
                  }
                 else
                  {
                    printf ("Error: reference to aliased_namespace_in_source that has not yet been seen! \n");
                    ROSE_ASSERT(false);
                  }

               ROSE_ASSERT(aliased_namespace_in_source != NULL);
             }


#if 0
          printf ("Building namespace alias named %s to namespace name = %s \n",name.str(),aliasedName.str());
          printf ("   --- aliased_namespace           = %p = %s name = %s \n",aliased_namespace,aliased_namespace->class_name().c_str(),aliased_namespace->get_name().str());
          if (aliased_namespace_in_source != NULL)
             {
               printf ("   --- aliased_namespace_in_source = %p = %s name = %s \n",aliased_namespace_in_source ,aliased_namespace_in_source ->class_name().c_str(),aliased_namespace_in_source ->get_name().str());
             }
#endif
          if (aliased_namespace_in_source != NULL)
             {
            // DQ (4/10/2018): I think this has already been set above (so this is redundant code).
               ROSE_ASSERT(tmp_namespaceAliasDeclarationStatement->get_is_alias_for_another_namespace_alias() == true);
#if 0
               printf ("I think this has already been set above (so this is redundant code) \n");
#endif
               tmp_namespaceAliasDeclarationStatement->set_is_alias_for_another_namespace_alias(true);
               tmp_namespaceAliasDeclarationStatement->set_namespaceAliasDeclaration(aliased_namespace_in_source);
#if 0
               printf ("Set namespace alias to record the namespace alias it is an alias to ... \n");
#endif
             }

       // DQ (4/8/2018): We need this declaration with this name for the code to work that we are removing (clean this up afterward)..
          SgNamespaceDeclarationStatement* namespaceDeclarationFromSymbol = aliased_namespace; // symbol->get_declaration();
          ROSE_ASSERT (namespaceDeclarationFromSymbol != NULL);

#if 0
          printf ("In parse_namespace(): calling summarize_namespace(nsp = %p) \n",nsp);
          summarize_namespace(nsp);
#endif

          a_scope_ptr nsp_scope = nsp->source_corresp.parent_scope;
          ROSE_ASSERT (scope_cache.find(nsp_scope) != scope_cache.end());

          SgScopeStatement* scope = scope_cache[nsp_scope];
          ROSE_ASSERT(scope != NULL);

       // DQ (11/3/2012): Setting default source position (normally handled by SageBuilder when using those functions).
          setSourcePosition(tmp_namespaceAliasDeclarationStatement);

          SgNamespaceAliasDeclarationStatement* namespaceAliasDeclarationStatement = set_file_info_for_corresp(tmp_namespaceAliasDeclarationStatement, nsp->source_corresp);

          namespaceDeclaration = namespaceAliasDeclarationStatement;
          ROSE_ASSERT (namespaceDeclaration != NULL);
#if 0
          printf ("namespaceDeclaration = %p = %s \n",namespaceDeclaration,namespaceDeclaration->class_name().c_str());
#endif
          namespaceDeclaration->set_firstNondefiningDeclaration(namespaceDeclaration);

       // DQ (8/6/2012): Add the namespace to the namespace map.
       // ROSE_ASSERT(namespace_cache.find(nsp) == namespace_cache.end());
       // namespace_cache[nsp] = namespaceDeclaration;
       // namespace_cache[nsp] = namespaceDeclarationFromSymbol;
          if (namespace_cache.find(nsp) == namespace_cache.end())
             {
               namespace_cache[nsp] = namespaceDeclarationFromSymbol;
             }
            else
             {
               printf ("WARNING: Namespace symbol is already present in the namespace_cache \n");
             }

          ROSE_ASSERT(namespace_cache.find(nsp) != namespace_cache.end());

       // sourcePositionData.initializeSourcePositionInformation(result);

#if TRACE_NAMESPACE_DECLARATION
          printf ("NNNNNNN #3 new namespace SgNamespaceAliasDeclarationStatement = %p name = %s aliasedName = %s \n",result, name.str(),aliasedName.str());
#endif
       // DQ (8/28/2009): Set the parent before calling get_scope().
          namespaceDeclaration->set_parent(topScopeStack());

       // printf ("SgNamespaceAliasDeclarationStatement: result->get_scope() = %p = %s \n",result->get_scope(),result->get_scope()->class_name().c_str());

       // We need a SgNamespaceSymbol because the using directive has to find the symbol for it 
       // referenced namespace and so the SgNamespaceAliasDeclarationStatement should generate 
       // a SgNamespaceSymbol.

       // The SgNamespaceSymbol has to accept a SgNamespaceAliasDeclarationStatement so that it 
       // can pass the test for a declaration's scope to match that of the scope of the symbol 
       // table where it lives (as a symbol).
       // SgNamespaceSymbol* namespaceSymbol = new SgNamespaceSymbol(name,namespaceDeclaration);
       // Liao 11/12/2010, multiple alias namespace declarations are allowed.
       // SgSymbol* symbol = get_symbol_ptr(scp);
       // if (check_symbol_in_current_scope( name, nsp->source_corresp ) == false )  


#if 0
          printf ("Build the symbol for the name space alias current_scope = %p = %s \n",current_scope,current_scope->class_name().c_str());
#endif
          SgNamespaceSymbol* namespaceAliasSymbol = isSgNamespaceSymbol(current_scope->lookup_symbol(name,NULL,NULL));
          if (namespaceAliasSymbol == NULL)
             {
#if 0
               printf ("Build a SgNamespaceSymbol: name = %s \n",name.str());
#endif
               SgNamespaceSymbol* namespaceSymbol = new SgNamespaceSymbol(name,NULL,namespaceAliasDeclarationStatement);
               ROSE_ASSERT (namespaceSymbol != NULL);

#if 0
               printf ("Putting the namespace alias symbol = %p into scope = %p = %s \n",namespaceSymbol,current_scope,current_scope->class_name().c_str());
#endif
               current_scope->insert_symbol(name,namespaceSymbol);
             }


#if 0
          printf ("aliasedName = %s name = %s \n",aliasedName.str(),name.str());
#endif

       // DQ (4/8/2018): Added assertion.
          ROSE_ASSERT(namespaceDeclaration != NULL);

       // DQ (7/8/2014): Fixed to use the correct scope (not the current scope of the alias, but the scope containing the alised namespace; a subtle point).
       // DQ (8/16/2013): Modified to reflect new API.
       // ROSE_ASSERT(current_scope->lookup_symbol(aliasedName) != NULL);
       // ROSE_ASSERT(current_scope->lookup_symbol(aliasedName,NULL,NULL) != NULL);

#if 0
       // DQ (4/8/2018): I now think it is wrong to have reference to a symbol in the scope using the aliased name.
          if (scope->lookup_symbol(aliasedName,NULL,NULL) == NULL)
             {
               printf ("WARNING: Should the aliased name be in the scopes symbol table? (change for 4/8/2018) \n");
             }
       // ROSE_ASSERT(scope->lookup_symbol(aliasedName,NULL,NULL) != NULL);
#endif
          ROSE_ASSERT(namespaceDeclaration->get_startOfConstruct() != NULL);
          ROSE_ASSERT(namespaceDeclaration->get_endOfConstruct()   != NULL);
#if 0
          printf ("Namespace Alias support not implemented! \n");
          ROSE_ASSERT(false);
#endif
        }
       else
        {
       /* Not a namespace alias, and therefore a namespace definition (either the initial one or an extension thereof). */
       // write_tok_str(" { ");
       // push_name_context(nsp->variant.assoc_scope);
#if 0
          printf ("Namespace is not an alias \n");
#endif
       // DQ (9/1/2012): Handle the case where we may have seen this namespace previously (e.g. in processing template 
       // instantations which referenced declaations in this namespace).  In this case we want to reuse the declaration 
       // we build then (we built it then so that we would have a scope to use; instead of defaulting to the topOfScopeStack()).
          SgNamespaceDeclarationStatement* tmp_namespaceDeclaration = NULL;

       // DQ (1/30/2013): Don't reuse a previously used SgNamespaceDeclarationStatement (this causes a problem 
       // detected in the unique statement test in AST consistancy tests).  The only example demonstrating this
       // problem so far is ROSE compiling the ROSE header files.
       // if (namespace_cache.find(nsp) != namespace_cache.end())
          if ((namespace_cache.find(nsp) != namespace_cache.end()) && false)
             {
               tmp_namespaceDeclaration = namespace_cache[nsp];
               ROSE_ASSERT(tmp_namespaceDeclaration != NULL);
#if 0
               printf ("Reusing SgNamespaceDeclarationStatement from namespace_cache: tmp_namespaceDeclaration = %p \n",tmp_namespaceDeclaration);
#endif
#if 0
               printf ("If we saw this previously did we set the source position??? tmp_namespaceDeclaration = %p \n",tmp_namespaceDeclaration);
               tmp_namespaceDeclaration->get_startOfConstruct()->display("If we saw this previously did we set the source position???: debug");
#endif
             }
            else
             {
#ifdef DEBUG_CONVERSION
            // DQ (1/30/2013): Output a message about how we are no longer, but would have been sharing this SgNamespaceDeclarationStatement.
               if (namespace_cache.find(nsp) != namespace_cache.end())
                  {
                    printf ("Previously we would have reused the namespace declaration = %p found in namespace_cache but this is a problem for ROSE compiling ROSE \n",namespace_cache[nsp]);
                  }

               printf ("WARNING: namespace declaration being build using scope == NULL (will default to global scope) \n");
#endif
            // DQ (5/16/2013): We want to use the build interface where possible.
            // tmp_namespaceDeclaration = set_file_info_for_corresp(new SgNamespaceDeclarationStatement(namespace_name,NULL,isUnnamedNamespace), nsp->source_corresp);
            // SgNamespaceDeclarationStatement* namespaceDecl_nfi = new SgNamespaceDeclarationStatement(namespace_name,NULL,isUnnamedNamespace);
               SgNamespaceDeclarationStatement* namespaceDecl_nfi = buildNamespaceDeclaration_nfi(namespace_name,isUnnamedNamespace,NULL);

#if 0
            // DQ (8/25/2014): I don't think this is required any more.
#if 0
               printf ("WARNING: It should not be required to call setSourcePosition from the edgRose.C file for the SgNamespaceDeclarationStatement built using the SageBuilder function. \n");
#endif
            // DQ (11/3/2012): Added setting default source position info.
               setSourcePosition(namespaceDecl_nfi);
#endif
               tmp_namespaceDeclaration = set_file_info_for_corresp(namespaceDecl_nfi, nsp->source_corresp);
             }

          namespaceDeclaration = tmp_namespaceDeclaration;
          ROSE_ASSERT (namespaceDeclaration != NULL);

#if 0
          tmp_namespaceDeclaration->get_file_info()->display("tmp_namespaceDeclaration file_info: debug");
#endif
#if 0
       // DQ (5/16/2013): This is handled the the build function.
          namespaceDeclaration->set_parent(topScopeStack());
          namespaceDeclaration->set_firstNondefiningDeclaration(namespaceDeclaration);
#else
       // DQ (5/16/2013): Added tests.
          ROSE_ASSERT(namespaceDeclaration->get_parent() == topScopeStack());

       // DQ (5/16/2013): I don't think this should be true with the new namespace handling.
          if (namespaceDeclaration->get_firstNondefiningDeclaration() != namespaceDeclaration)
             {
               ROSE_ASSERT(namespaceDeclaration->get_firstNondefiningDeclaration() != NULL);
#ifdef DEBUG_CONVERSION
               printf ("Note that namespaceDeclaration->get_firstNondefiningDeclaration() != namespaceDeclaration: namespaceDeclaration = %p namespaceDeclaration->get_firstNondefiningDeclaration() = %p \n",
                    namespaceDeclaration,namespaceDeclaration->get_firstNondefiningDeclaration());
#endif
             }
       // ROSE_ASSERT(namespaceDeclaration->get_firstNondefiningDeclaration() == namespaceDeclaration);
#endif
       // DQ (9/1/2012): In rare cases we will have already processed this namespace (because we were forced 
       // by EDG to process some template instantations placed at the top of the namespace or referencing 
       // classes defined in the namespace).
       // DQ (8/6/2012): Add the namespace to the namespace map.
       // ROSE_ASSERT(namespace_cache.find(nsp) == namespace_cache.end());
          if (namespace_cache.find(nsp) == namespace_cache.end())
             {
               namespace_cache[nsp] = tmp_namespaceDeclaration;
             }

       // DQ (6/16/2013): Test for use of empty namespace name in symbol lookup (this test fails for test2013_211.C).
       // ROSE_ASSERT(namespace_name.empty() == false);
          if (namespace_name.empty() == true)
             {
            // There is a generated name that could alternatively be used instead of the empty namespace name string.
#ifdef DEBUG_CONVERSION
               printf ("WARNING: empty namespace name (for un-named namespace) used for symbol table lookup (not an error, but something to handle better) \n");
#endif
             }

       // DQ (8/16/2013): Modified to reflect new API.
       // FIXME: Need to setup SgNamespaceSymbol!
       // SgSymbol* namespaceSymbol = current_scope->lookup_symbol(namespace_name);
          SgSymbol* namespaceSymbol = current_scope->lookup_symbol(namespace_name,NULL,NULL);
#if 0
          printf ("namespaceSymbol = %p \n",namespaceSymbol);
#endif
          if (namespaceSymbol == NULL)
             {
               SgName name = namespace_name;
               namespaceSymbol = new SgNamespaceSymbol(name,tmp_namespaceDeclaration,NULL);
               ROSE_ASSERT (namespaceSymbol != NULL);
#if 0
               printf ("Putting the namespace (name = %s) symbol = %p into scope = %p = %s \n",name.str(),namespaceSymbol,current_scope,current_scope->class_name().c_str());
#endif
#if 1
            // DQ (5/16/2013): The new namespace handlign is hidden in the SgScopeStatement::insert_symbol() function for simplicity.
               current_scope->insert_symbol(namespace_name,namespaceSymbol);
#else
            // DQ (5/9/2013): If this is a namespace scope then we need so handle that case there there can be many since it is reentrant.
               SgScopeStatement* associatedScope = NULL;
               SgNamespaceDefinitionStatement* namespaceDefinition = isSgNamespaceDefinitionStatement(current_scope);
               if (namespaceDefinition != NULL)
                  {
                    associatedScope = namespaceDefinition->get_global_definition();

#error "DEAD CODE!"

                  }
                 else
                  {
                    associatedScope = current_scope;
                  }
               ROSE_ASSERT(associatedScope != NULL);

               associatedScope->insert_symbol(namespace_name,namespaceSymbol);
            // type_decl->set_scope(associatedScope);
#endif
             }
          ROSE_ASSERT(namespaceSymbol != NULL);

       // DQ (8/16/2013): Modified to reflect new API.
       // ROSE_ASSERT(current_scope->lookup_symbol(namespace_name) != NULL);
          ROSE_ASSERT(current_scope->lookup_symbol(namespace_name,NULL,NULL) != NULL);

          ROSE_ASSERT(current_scope->lookup_namespace_symbol(namespace_name) != NULL);
#if 0
          printf ("In parse_namespace(): This is a namespace definition \n");
#endif
#if 1
       // DQ (5/16/2013): In the new namespace support the definition is always present, but for the first non-defining declaration it might be empty.
          SgNamespaceDefinitionStatement* namespaceDefinition = tmp_namespaceDeclaration->get_definition();
          ROSE_ASSERT(namespaceDefinition != NULL);
#else
#if 1
       // DQ (9/1/2012): This version of the code accounts for the case where the namespace may have been previously 
       // processed and so the SgNamespaceDefinitionStatement was already built.
          SgNamespaceDefinitionStatement* namespaceDefinition = NULL;
          if (tmp_namespaceDeclaration->get_definition() == NULL)
             {
            // We need to build a SgNamespaceDefinitionStatement (namespaceDeclaration was not previously built and reused above).
               namespaceDefinition = new SgNamespaceDefinitionStatement(tmp_namespaceDeclaration);
               ROSE_ASSERT (namespaceDefinition != NULL);

#error "DEAD CODE!"

            // DQ (11/4/2012): Added setting default source position info.
               setSourcePosition(namespaceDefinition);

               namespaceDefinition->set_parent(namespaceDeclaration);

            // DQ (9/1/2012): I think this might be violated where we have previously processed a namespace.
               ROSE_ASSERT(tmp_namespaceDeclaration->get_definition() == NULL);

#error "DEAD CODE!"

            // set the definition
               tmp_namespaceDeclaration->set_definition(namespaceDefinition);
             }
            else
             {
               namespaceDefinition = tmp_namespaceDeclaration->get_definition();
               ROSE_ASSERT (namespaceDefinition != NULL);
             }
#else
       // DQ (8/18/2012): We want to use the iek_src_seq_end_of_construct (which holds the position of the closing brace) in setting the source position information.
       // SgNamespaceDefinitionStatement* namespaceDefinition = set_file_info_for_corresp(new SgNamespaceDefinitionStatement(tmp_namespaceDeclaration), nsp->source_corresp);
          SgNamespaceDefinitionStatement* namespaceDefinition = new SgNamespaceDefinitionStatement(tmp_namespaceDeclaration);
          ROSE_ASSERT (namespaceDefinition != NULL);
          namespaceDefinition->set_parent(namespaceDeclaration);

#error "DEAD CODE!"

       // DQ (9/1/2012): I think this might be violated where we have previously processed a namespace.
          ROSE_ASSERT(namespaceDeclaration->get_definition() == NULL);

       // set the definition
          tmp_namespaceDeclaration->set_definition(namespaceDefinition);
#endif

#if 0
          printf ("Calling summarize_namespace() \n");
          summarize_namespace(nsp);
#endif

#if 0
       // DQ (8/17/2012): Test the std namespace for line number info (I expect this to fail).
          ROSE_ASSERT(namespaceDefinition->get_startOfConstruct() != NULL);
          if (namespaceDefinition->get_startOfConstruct()->get_line() == namespaceDefinition->get_endOfConstruct()->get_line())
             {
#if 0
               namespaceDefinition->get_startOfConstruct()->display("namespaceDefinition->get_startOfConstruct(): debug");
               namespaceDefinition->get_endOfConstruct()  ->display("namespaceDefinition->get_endOfConstruct(): debug");
#endif
               printf ("ERROR: likely incorrect ending source line position for namespace \n");
               ROSE_ASSERT(false);
             }
#endif
#endif
       // DQ (11/20/2011): Add the name space scope so that we can find it later 
       // (when evaluating the scope of variables refreenced outside of the namespace's scope).
          ROSE_ASSERT(nsp->variant.assoc_scope != NULL);

#if 1
       // DQ (9/1/2012): This is to handle the case where we have previously processed the namespace and we are reusing that associated SgNamespaceDefinitionStatement.
          if (scope_cache.find(nsp->variant.assoc_scope) == scope_cache.end())
             {
            // If we have not see it previously then add it to the scope cache.
               scope_cache[nsp->variant.assoc_scope] = namespaceDefinition;
             }
            else
             {
            // DQ (1/30/2013): Skipping this test to avoid reuse of previously declared SgNamespaceDeclarationStatement.
#ifdef DEBUG_CONVERSION
               printf ("In parse_namespace(): Commented out assertion of scope_cache[nsp->variant.assoc_scope] == namespaceDefinition to avoid reuse of existing namespace: namespaceDefinition = %p \n",namespaceDefinition);
#endif
            // If we have processed in previously (it is in the scope cache), then make sure this is the correct one.
            // ROSE_ASSERT(scope_cache[nsp->variant.assoc_scope] == namespaceDefinition);
             }
#else
       // DQ (3/11/2012): check that this is not already in the list before we add it (or
       // verify it is NULL if it is supposed to be in the list but just not initialized yet).
          ROSE_ASSERT (scope_cache.find(nsp->variant.assoc_scope) == scope_cache.end());

          scope_cache[nsp->variant.assoc_scope] = namespaceDefinition;
#endif
       // printf ("nsp->variant.assoc_scope = %p \n",nsp->variant.assoc_scope);
#if 0
          printf ("@@@@@ In scope_cache (parse_namespace): nsp->variant.assoc_scope = %p  is associated with namespaceDefinition = %p = %s \n",nsp->variant.assoc_scope,namespaceDefinition,namespaceDefinition->get_namespaceDeclaration()->get_name().str());
#endif
#if 0
          printf ("Exiting in parse_namespace() \n");
          ROSE_ASSERT(false);
#endif

#if DEBUG_PUSH_SCOPE
          printf ("In parse_namespace(): push scope stack: namespace definiton = %p = %s \n",namespaceDefinition,namespaceDefinition->class_name().c_str());
#endif
          pushScopeStack(namespaceDefinition);

#ifdef DEBUG_CONVERSION
          ScopedIndent si(indent);
#endif
#if 0
          printf ("\n@@@@@ PROCESSING NAMESPACE DEFINITION (parse_namespace()) \n");
#endif
       // Process all global scope declarations
       // while (sse->entity.kind != iek_src_seq_end_of_construct)
          while (sse)
             {
               an_il_entry_kind kind = (an_il_entry_kind)(sse->entity.kind);
#if 0
            // DQ (8/17/2012): I think that the source position of the closing scope is contained in here (so we don't want to ignore these).
               printf ("@@@@@@@@@@@@@@@@@ In parse_namespace(): found a iek_src_seq_end_of_construct kind \n");
               printf ("@@@@@@@@@@@@@@@@@ In parse_namespace(): sse->entity.ptr = %p \n",sse->entity.ptr);

               printf ("@@@@@@@@@@@@@@@@@ In parse_namespace(): sse->entity.ptr = %s \n",summarize_entry_kind_in_string_kind(sse->entity).c_str());

               printf ("@@@@@@@@@@@@@@@@@ In the namespace = %s found statement of kind = %s \n",namespace_name.c_str(),il_entry_kind_names[kind]);
#endif

               if (kind == iek_src_seq_end_of_construct)
                  {
                    ROSE_ASSERT (sse->entity.kind == iek_src_seq_end_of_construct);
                    a_src_seq_end_of_construct_ptr p = (a_src_seq_end_of_construct_ptr)(sse->entity.ptr);
                    ROSE_ASSERT(p != NULL);
#if 0
                    printf ("@@@@@@@@@@@@@@@@@ In the namespace = %s found statement of kind = iek_src_seq_end_of_construct \n",namespace_name.c_str());
#endif
                 // DQ (8/18/2012): I think this is the better way to make sure that the ending position is correctly marked.
                    set_file_info_for_corresp_and_end(namespaceDefinition,nsp->source_corresp,sse);

                 // DQ (6/26/2018): Need to test for if this is the end of the current construct, or some other one.
                 // Note that a macro in a specific place (e.g. in the initialization for a variable) can trigger an added 
                 // source sequence point and thus an end of construct for the variable.  As a result we have to check that
                 // we are handling the end of construct associated with this namespace.  If it is not associated with this
                 // namespace, then we increment over it, if it is then we increment over it and "goto end".
                 // if (((char*)nsp) == sse->entity.ptr)
                    if (((char*)nsp) == p->entity.ptr)
                       {
                         check_end_of_construct(iek_namespace, nsp, sse);
#if 0
                         printf ("@@@@@@@@@@@@@@@@@ In parse_namespace(): increment sse \n");
#endif
                         ++sse;
#if 0
                         printf ("@@@@@@@@@@@@@@@@@ In parse_namespace(): goto end \n");
#endif
                         goto end;
                       }
                      else
                       {
                      // DQ (6/26/2018): This is a part of another end of construct (e.g. variable declaration with an initializer and an embedded macro (test2018_109.C).
#if 0
                      // printf ("################# In parse_namespace(): nsp = %p p->entity.ptr = %p \n",nsp,p->entity.ptr);
                         printf ("################# In parse_namespace(): nsp = %p p->entity.ptr = %p sse->entity.ptr = %s \n",nsp,p->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif

#if 0
                      // if (sse->entity.kind == iek_namespace)
                         if (p->entity.kind == iek_namespace)
                            {
#if 1
                              printf ("################# In parse_namespace(): IS a namespace increment sse and goto end \n");
#endif
                              ++sse;
#if 1
                              goto end;
#endif
                            }
                           else
                            {
#if 1
                              printf ("################# In parse_namespace(): NOT a namespace increment sse \n");
#endif
                           // if (sse->entity.kind == iek_variable)
                              if (p->entity.kind == iek_variable)
                                 {
                                   ++sse;
#if 0
                                   goto end;
#endif
                                 }
                                else
                                 {
                                   ++sse;
#if 1
                                   goto end;
#endif
                                 }
                            }
#else
                         ++sse;
#if 0
                         goto end;
#endif
#endif
                       }
                  }
                 else
                  {
                    SgDeclarationStatement* s = parse_global_or_namespace_scope_entity(sse);
#if 0
                    printf ("@@@@@@@@@@@@@@@@@ In parse_namespace(): declaration s = %p = %s \n",s,(s != NULL) ? s->class_name().c_str() : "NULL");
#endif
                 // Not all source sequence entries will translate into a valid IR node.
                 // As part of general EDG translation documentation we should make a 
                 // list of example that don't translate.
                    if (s != NULL)
                       {
#if MAKE_REDUNDANT_CALLS_TO_DETECT_TRANSFORAMTIONS
                      // DQ (5/1/2012): Make sure that we don't have IR nodes marked as transformations.
                         detectTransformations_local(s);
#endif
                         appendStatement(s, namespaceDefinition);
                       }
                  }
             }

#if 0
     printf ("@@@@@@@@@@@@@@@@@ In parse_namespace(): exited while loop over sse normally \n");
#endif

// Note Label
end: ;

#if DEBUG_POP_SCOPE
          printf ("In parse_namespace(): calling popScopeStack() \n");
#endif
       // Pop the global scope
          popScopeStack();

#if 0
          printf ("@@@@@ DONE: PROCESSING NAMESPACE DEFINITION (parse_namespace()) \n");
#endif
#if 0
          printf ("topScopeStack() = %p = %s = %s \n",topScopeStack(),topScopeStack()->class_name().c_str(),SageInterface::get_name(topScopeStack()).c_str());
#endif
#if 0
          printf ("Exiting as a test! \n");
          ROSE_ASSERT(false);
#endif

       // Error checking, there should be nothing left on the scope stack.
          ROSE_ASSERT(topScopeStack() != NULL);

#if 0
       /* Go through the source sequence list and generate the members of the namespace. */
          while (ss_entry_kind(curr_source_sequence_entry) != iek_src_seq_end_of_construct)
             {
            // gen_declaration(/*for_init=*/FALSE);

               ++sse;
               indent_line(); debug_print("namespace declaration statement"); debug_print("\n");
             }

       /* This should be the end-of-construct marker for the namespace. */
             {
               a_src_seq_end_of_construct_ptr ssecp = ss_entry_ptr(curr_source_sequence_entry,a_src_seq_end_of_construct_ptr);
               check_assertion_str(ss_entry_kind(ssecp) == iek_namespace && ss_entry_ptr(ssecp, a_namespace_ptr) == nsp,"gen_namespace: bad end-of-construct");
            /* Set the position for the closing "}". */
               set_output_position(&ssecp->position);
               adv_curr_source_sequence_entry();
             }
          pop_name_context();
          write_tok_ch('}');
#endif

#if 0
       // DQ (8/17/2012): Test the std namespace for line number info (this should pass now that we have moved it, so long as there are declarations).
       // Note that the trivial case for this test could still fail: "namespace X{};", but this happens in debug.h (system header file).
          ROSE_ASSERT(namespaceDefinition->get_startOfConstruct() != NULL);
          if (namespaceDefinition->get_startOfConstruct()->get_line() == namespaceDefinition->get_endOfConstruct()->get_line())
             {
#if 0
               namespaceDefinition->get_startOfConstruct()->display("namespaceDefinition->get_startOfConstruct(): debug");
               namespaceDefinition->get_endOfConstruct()  ->display("namespaceDefinition->get_endOfConstruct(): debug");
#endif
               printf ("ERROR: likely incorrect ending source line position for namespace \n");
               ROSE_ASSERT(false);
             }
#endif
          set_file_info_for_seconary_namespace_definition(namespaceDefinition,nsp->source_corresp.decl_pos_info);
#if 0
       // SgNamespaceDeclarationStatement* namespaceDeclaration = isSgNamespaceDeclarationStatement(result);
       // ROSE_ASSERT(namespaceDeclaration != NULL);
       // SgNamespaceDefinitionStatement* namespaceDefinition = namespaceDeclaration->get_definition();
       // ROSE_ASSERT(namespaceDefinition != NULL);
          printf ("In parse_namespace(): namespaceDefinition = %p at (%d,%d) -> (%d,%d) \n",namespaceDefinition,
               namespaceDefinition->get_startOfConstruct()->get_line(),namespaceDefinition->get_startOfConstruct()->get_col(),
               namespaceDefinition->get_endOfConstruct()->get_line(),namespaceDefinition->get_endOfConstruct()->get_col());
#endif

       // DQ (4/8/2018): Added assertion.
          ROSE_ASSERT(namespaceDeclaration != NULL);

       // DQ (11/20/2011): This is fixed now!
       // printf ("Namespace definition support not implemented yet! \n");
       // ROSE_ASSERT(false);
        }

     ROSE_ASSERT(namespaceDeclaration != NULL);

  // Debugging code (assertion testing).
  // Note that assoc_scope is only valid (union) if is_namespace_alias == false.
  // ROSE_ASSERT(nsp->is_namespace_alias == false);
     if (nsp->is_namespace_alias == false)
        {
          ROSE_ASSERT(nsp->variant.assoc_scope != NULL);

          a_scope_ptr edg_scope = nsp->variant.assoc_scope; // source_corresp.parent_scope;
          ROSE_ASSERT(edg_scope != NULL);

       // DQ (12/28/2011): This test avoids adding a null pointer to the scope_cache (since it is permited that there not be a scope available).
          if (scope_cache.find(edg_scope) != scope_cache.end())
             {
            // DQ (12/9/2011): Added test...(fails for test2004_33.C)
               SgScopeStatement* scopeOfReference = scope_cache[edg_scope];

            // DQ (12/28/2011): If this fails then we need to fix the reference to scope_cache to avoid adding a NULL pointer.
               ROSE_ASSERT(scopeOfReference != NULL);

               if (scopeOfReference == NULL)
                  {
                 // This is a potential problem.
                    SgNamespaceDeclarationStatement* local_namespaceDeclaration = isSgNamespaceDeclarationStatement(namespaceDeclaration);
                    if (local_namespaceDeclaration != NULL)
                       {
#if 0
                         SgNamespaceDefinitionStatement* namespaceDefinition = local_namespaceDeclaration->get_definition();
                         printf ("@@@@@ In scope_cache (BOTTOM of parse_namespace (scope_cache[edg_scope] == NULL)): nsp->variant.assoc_scope = %p  is associated with namespaceDefinition = %p = %s \n",
                              nsp->variant.assoc_scope,namespaceDefinition,(namespaceDefinition != NULL) ? namespaceDefinition->get_namespaceDeclaration()->get_name().str() : "null");
#endif
                       }
                      else
                       {
#if 0
                         printf ("@@@@@ In scope_cache (BOTTOM of parse_namespace (scope_cache[edg_scope] == NULL)): local_namespaceDeclaration == NULL \n");
#endif
                       }
                  }

            // ROSE_ASSERT(scopeOfReference != NULL);
             }
            else
             {
               ROSE_ASSERT(scope_cache.find(edg_scope) == scope_cache.end());
             }

          SgScopeStatement* associatedScope = scope_cache[edg_scope];
          ROSE_ASSERT(associatedScope != NULL);
#if 0
          printf ("At BASE of parse_namespace() (primary): associatedScope = %p = %s \n",associatedScope,associatedScope->class_name().c_str());
#endif
        }

  // DQ (8/12/2014): Adding support for C++11 inlined namespaces.
     if (nsp->is_inline == true)
        {
#if 0
          printf ("In parse_namespace(): (nsp->is_inline == true) \n");
#endif
          SgNamespaceDeclarationStatement* tmp_namespaceDeclaration = isSgNamespaceDeclarationStatement(namespaceDeclaration);
          ROSE_ASSERT(tmp_namespaceDeclaration != NULL);
          tmp_namespaceDeclaration->set_isInlinedNamespace(true);
        }

#if 0
     printf ("At BASE of parse_namespace() (primary): current scope = %p = %s \n",topScopeStack(),topScopeStack()->class_name().c_str());
#endif
#if 0
     printf ("At BASE of parse_namespace() (primary): namespaceDeclaration = %p \n",namespaceDeclaration);
#endif

     return namespaceDeclaration;
   }
