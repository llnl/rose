
#include "edgRose.h"

SgDeclarationStatement*
EDG_ROSE_Translation::convert_namespace(a_namespace_ptr p, SourceSequenceList& sse)
   {
  // This function has to be different from the parse_namespace() function because it is used to 
  // process secondary declaration (additional namespaces see after the first one with the same name).

  // That we are seeing multiple namespaces with the same name might be the problem why we can't find the associated symbols.

#ifdef DEBUG_CONVERSION
  // indent_line(); debug_print("namespace declaration statement"); 
     indent_line(); debug_print("namespace declaration statement (convert) name = %s is_namespace_alias = %s ",p->source_corresp.name, p->is_namespace_alias ? "true" : "false"); print_source_corresp(p->source_corresp); debug_print("\n");

  // print_source_pos(p->decl_position); 
     debug_print("\n");
     ScopedIndent si(indent);
#endif

#if 0
     printf ("In convert_namespace(): calling summarize_namespace(nsp = %p) \n",p);
     summarize_namespace(p);
#endif

     string namespace_name = generate_namespace_name(p);

     a_src_seq_secondary_decl_ptr secondary_p = (a_src_seq_secondary_decl_ptr)(sse->entity.ptr);

#ifdef DEBUG_CONVERSION
  // ROSE_ASSERT(p->decl_pos_info != NULL);
     if (secondary_p->decl_pos_info != NULL)
        {
          indent_line(); debug_print("-- secondary identifier_range start at:"); print_source_pos(secondary_p->decl_pos_info->identifier_range.start); debug_print("\n");
          indent_line(); debug_print("-- secondary identifier_range end at  :"); print_source_pos(secondary_p->decl_pos_info->identifier_range.end); debug_print("\n");

          indent_line(); debug_print("-- secondary specifiers_range start at:"); print_source_pos(secondary_p->decl_pos_info->specifiers_range.start); debug_print("\n");
          indent_line(); debug_print("-- secondary specifiers_range end at  :"); print_source_pos(secondary_p->decl_pos_info->specifiers_range.end); debug_print("\n");

       // DQ (3/11/2012): This is another form ofjust the starting position information, as I recall.
          indent_line(); debug_print("-- secondary (decl_position) start at:"); print_source_pos(secondary_p->decl_position); debug_print("\n");

        }
#endif

  // DQ (8/6/2012): New code to support secondary namespace alias.
     SgDeclarationStatement* result = NULL;

  // ROSE_ASSERT(p->is_namespace_alias == false);
     if (p->is_namespace_alias == true)
        {
          a_namespace_ptr assoc_namespace = p->variant.assoc_namespace;
          ROSE_ASSERT(assoc_namespace != NULL);
#if 0
          printf ("In convert_namespace(): assoc_namespace = %p \n",assoc_namespace);
#endif
          a_namespace_ptr assoc_root_namespace = getRootNamespace(assoc_namespace);
#if 0
          printf ("In convert_namespace(): assoc_root_namespace = %p \n",assoc_root_namespace);
#endif
          ROSE_ASSERT(assoc_root_namespace != NULL);
          if (namespace_cache.find(assoc_root_namespace) == namespace_cache.end())
             {
               namespace_cache[assoc_root_namespace] = construct_implicit_namespace(assoc_root_namespace);
             }
          ROSE_ASSERT(namespace_cache.find(assoc_root_namespace) != namespace_cache.end());

          ROSE_ASSERT(namespace_cache.find(assoc_namespace) != namespace_cache.end());
          SgNamespaceDeclarationStatement* previous_namespace_declaration = namespace_cache[assoc_namespace];
          ROSE_ASSERT(previous_namespace_declaration != NULL);
#if 0
          printf ("In convert_namespace(): Calling construct_namespace(assoc_namespace = %p) \n",assoc_namespace);
#endif
       // DQ (9/1/2012): This function is more general than just for "std" namespaces.
          SgNamespaceDeclarationStatement* std_namespaceDeclaration = construct_namespace(assoc_namespace);
          ROSE_ASSERT(std_namespaceDeclaration != NULL);
#if 0
          printf ("In convert_namespace(): DONE: Calling construct_namespace(assoc_namespace = %p) \n",assoc_namespace);
#endif
          SgNamespaceAliasDeclarationStatement* namespaceAliasDeclaration = set_file_info_for_secondary_declaration(new SgNamespaceAliasDeclarationStatement(namespace_name,previous_namespace_declaration), secondary_p);
          ROSE_ASSERT(namespaceAliasDeclaration != NULL);

          namespaceAliasDeclaration->set_firstNondefiningDeclaration(namespaceAliasDeclaration);
          namespaceAliasDeclaration->set_definingDeclaration(namespaceAliasDeclaration);

          namespaceAliasDeclaration->set_parent(topScopeStack());

       // DQ (4/8/2018): Build a cache of SgNamespaceAliasDeclarationStatement declarations to support when
       // a namespace alias is to another namespace alias (now supported).
          ROSE_ASSERT(assoc_namespace != NULL);
          if (namespace_alias_cache.find(assoc_namespace) == namespace_alias_cache.end())
             {
               namespace_alias_cache[assoc_namespace] = namespaceAliasDeclaration;
             }
          ROSE_ASSERT(namespace_alias_cache.find(assoc_namespace) != namespace_alias_cache.end());

          result = namespaceAliasDeclaration;
          ROSE_ASSERT(result != NULL);
        }
       else
        {
          SgScopeStatement* currentScope = topScopeStack();
          ROSE_ASSERT(currentScope != NULL);
          
#ifdef DEBUG_CONVERSION
          printf ("WARNING: using topScopeStack() to build namespace declaration: currentScope = %p = %s \n",currentScope,currentScope->class_name().c_str());
#endif

          SgNamespaceDeclarationStatement* namespaceDeclaration = set_file_info_for_secondary_declaration(buildNamespaceDeclaration_nfi(namespace_name, /* bool unnamednamespace = */ false, currentScope), secondary_p);
          ROSE_ASSERT(namespaceDeclaration != NULL);

          SgNamespaceDefinitionStatement* namespaceDefinition = namespaceDeclaration->get_definition();
          ROSE_ASSERT(namespaceDefinition != NULL);

       // DQ (1/30/2015): Provide better source position information.
          set_file_info_for_seconary_namespace_definition(namespaceDeclaration->get_definition(),secondary_p->decl_pos_info);

       // DQ (1/30/2015): Set the end of the namespaceDeclaration to be the same as that of the end of the namespaceDefinition.
          (*namespaceDeclaration->get_endOfConstruct()) = (*namespaceDefinition ->get_endOfConstruct());

          ROSE_ASSERT(namespaceDeclaration->isCompilerGenerated() == false);

          result = namespaceDeclaration;
          ROSE_ASSERT(result != NULL);

          if (p->source_corresp.decl_position.seq == 0)
             {
#ifdef DEBUG_CONVERSION
               printf ("secondary namespace declaration does not have source position information \n");
#endif
             }

          namespaceDeclaration->set_parent(topScopeStack());

       // DQ (8/6/2012): If this is a secondary then this namespace should already exist.
          if (namespace_cache.find(p) == namespace_cache.end())
             {
               namespace_cache[p] = namespaceDeclaration;
             }
          ROSE_ASSERT(namespace_cache.find(p) != namespace_cache.end());

       // DQ (9/1/2012): Added assertion to avoid posability of NULL added as index into scope map.
          ROSE_ASSERT(p->variant.assoc_scope != NULL);

       // ROSE_ASSERT(scope_cache.find(p->variant.assoc_scope) != scope_cache.end());
          if (scope_cache.find(p->variant.assoc_scope) == scope_cache.end())
             {
               scope_cache[p->variant.assoc_scope] = namespaceDefinition;
             }
          ROSE_ASSERT(scope_cache.find(p->variant.assoc_scope) != scope_cache.end());
          ROSE_ASSERT(scope_cache[p->variant.assoc_scope] != NULL);

          SgScopeStatement* namespace_definition_scope = scope_cache[p->variant.assoc_scope];
          ROSE_ASSERT(namespace_definition_scope != NULL);
#if 0
          printf ("Preadjusted (associated namespace definition) namespace_definition_scope = %p = %s \n",namespace_definition_scope,namespace_definition_scope->class_name().c_str());
#endif
          SgNamespaceDefinitionStatement* first_namespaceDefinition = isSgNamespaceDefinitionStatement(namespace_definition_scope);
          ROSE_ASSERT(first_namespaceDefinition != NULL);

          SgScopeStatement* scope_of_first_namespace_declaration = first_namespaceDefinition->get_namespaceDeclaration()->get_scope();
          ROSE_ASSERT(scope_of_first_namespace_declaration != NULL);
#if 0
          printf ("scope of associated namespace definition: scope = %p = %s \n",scope_of_first_namespace_declaration,scope_of_first_namespace_declaration->class_name().c_str());
#endif
          SgNamespaceSymbol* namespaceSymbol = scope_of_first_namespace_declaration->lookup_namespace_symbol(namespace_name);
          ROSE_ASSERT(namespaceSymbol != NULL);

          pushScopeStack(namespaceDefinition);

#ifdef DEBUG_CONVERSION
          ScopedIndent si(indent);
#endif
          ++sse;

#if 0
          printf ("In convert_namespace(): @@@@@ PROCESSING NAMESPACE DEFINITION (convert_namespace()) \n");
#endif

       // DQ (4/26/2012): Test the maps that we use internally (see function for documentation).
          testEdgRoseMaps();

       // Process all global scope declarations
       // while (sse->entity.kind != iek_src_seq_end_of_construct)
          while (sse)
             {
               an_il_entry_kind kind = (an_il_entry_kind)(sse->entity.kind);
#if 0
               printf ("In the namespace = %s found statement of kind = %s \n",namespace_name.c_str(),il_entry_kind_names[kind]);
#endif
            // if (kind == src-seq-end-of-construct)
               if (kind == iek_src_seq_end_of_construct)
                  {
                 // DQ (8/17/2012): I think that the source position of the closing scope is contained in here (so we don't want to ignore these).
                 // printf ("@@@@@@@@@@@@@@@@@ In convert_namespace(): found a iek_src_seq_end_of_construct kind \n");

                 // printf ("In the namespace = %s found statement of kind = iek_src_seq_end_of_construct \n",namespace_name.c_str());
                 // printf ("In the namespace = %s found statement of kind = src-seq-end-of-construct \n",namespace_name.c_str());

                    a_src_seq_end_of_construct_ptr edg_end_of_construct = (a_src_seq_end_of_construct_ptr)(sse->entity.ptr);
                    ROSE_ASSERT(edg_end_of_construct != NULL);

                 // debug_print("-- end of construct at :"); print_source_pos(edg_end_of_construct->position); debug_print("\n");

                 // printf ("Coresponding EDG entity is: edg_end_of_construct->entity.ptr = %p kind = %s \n",edg_end_of_construct->entity.ptr,il_entry_kind_names[edg_end_of_construct->entity.kind]);
#if 0
                 // DQ (8/18/2012): These are not yet set.
                    ROSE_ASSERT(namespaceDefinition->get_startOfConstruct() != NULL);
                    namespaceDefinition->get_startOfConstruct()->display("In convert_namespace(): case iek_src_seq_end_of_construct: startOfConstruct debug");
                    namespaceDefinition->get_endOfConstruct()  ->display("In convert_namespace(): case iek_src_seq_end_of_construct: endOfConstruct debug");
#endif
                    check_end_of_construct(iek_namespace, p, sse);

                 // DQ (2/20/2012): This causes the next statement to disappear (see test2012_05.C).
                 // ++sse;

                    goto end;
                  }
                 else
                  {
                    SgDeclarationStatement* s = parse_global_or_namespace_scope_entity(sse);
#if 0
                    printf ("In convert_namespace(): declaration s = %p = %s \n",s,(s != NULL) ? s->class_name().c_str() : "NULL");
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
#if 0
                         printf ("In convert_namespace(): appendStatement(s = %p = %s,namespaceDefinition = %p) \n",s,s->class_name().c_str(),namespaceDefinition);
#endif
                         appendStatement(s,namespaceDefinition);
                       }
                  }
             }

// Note Label
end: ;

#if 0
          printf ("In convert_namespace(): POP the current scope --- topScopeStack() = %p = %s \n",topScopeStack(),topScopeStack()->class_name().c_str());
#endif

#if DEBUG_POP_SCOPE
          printf ("In convert_namespace(): calling popScopeStack() \n");
#endif
       // Pop the global scope
          popScopeStack();

#if 0
          printf ("@@@@@ DONE: PROCESSING NAMESPACE DEFINITION (convert_namespace()) \n");
#endif

       // Error checking, there should be nothing left on the scope stack.
          ROSE_ASSERT(topScopeStack() != NULL);
        }

  // DQ (12/21/2011): This is the correct scope associated with the namespace.
     if (p->is_namespace_alias == false)
        {
          a_scope_ptr edg_scope = p->variant.assoc_scope; // source_corresp.parent_scope;
          ROSE_ASSERT(edg_scope != NULL);

       // DQ (3/11/2012): check that this is not already in the list before we add it (or
       // verify it is NULL if it is supposed to be in the list but just not initialized yet).
          ROSE_ASSERT (scope_cache.find(edg_scope) != scope_cache.end());

       // DQ (12/9/2011): Added test...
          SgScopeStatement* scopeOfReference = scope_cache[edg_scope];
          ROSE_ASSERT(scopeOfReference != NULL);

          SgScopeStatement* associatedScope = scope_cache[edg_scope];
          ROSE_ASSERT(associatedScope != NULL);
#if 0
          printf ("At BASE of convert_namespace() (secondary): associatedScope = %p = %s \n",associatedScope,associatedScope->class_name().c_str());
#endif
        }

  // DQ (4/26/2012): Test the maps that we use internally (see function for documentation).
     testEdgRoseMaps();

  // DQ (5/19/2013): There should always be proper source file position infomation so this should not be required.
     ROSE_ASSERT(result->get_file_info()->isOutputInCodeGeneration() == false);

     ROSE_ASSERT(internal_intermediate_scope != NULL);
     ROSE_ASSERT(internal_intermediate_scope->symbol_table_size() == 0);

     return result;
   }
