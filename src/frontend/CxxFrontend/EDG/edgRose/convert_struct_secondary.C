
#include "edgRose.h"

SgDeclarationStatement* EDG_ROSE_Translation::convert_struct_secondary(a_type_ptr p, SgClassDeclaration::class_types t )
   {
  // This function will build either a SgClassDeclaration, an instantiation of a class declaration template, of a SgTemplateClassDeclaration.

#if defined(DEBUG_CONVERSION) || 0
     indent_line();
  // DQ (9/28/2022): Remove this compiler warning (unused parameter).
  // debug_print("%s secondary from_template_parameter = %s \n", 
  //             (t == SgClassDeclaration::e_union ? "union" : t == SgClassDeclaration::e_struct ? "struct" : t == SgClassDeclaration::e_class ? "class" : "???"), 
  //             from_template_parameter ? "true" : "false");
     debug_print("%s secondary \n", 
                 (t == SgClassDeclaration::e_union ? "union" : t == SgClassDeclaration::e_struct ? "struct" : t == SgClassDeclaration::e_class ? "class" : "???"));
#endif

     ROSE_ASSERT(p != NULL);

  // DQ (1/2/2012): Refactoring to generate common name used in this function.
     SgName nameWithOutTemplateArguments = generateName(p);

     bool originally_unnamed = p->variant.class_struct_union.originally_unnamed;

  // DQ (3/2/2019): If this is un-named then don't allow a name to be used.
     if (originally_unnamed == true)
        {
          nameWithOutTemplateArguments = generate_name_for_type(p);
#if 0
          printf ("In convert_struct_secondary(): Reset name because originally_unnamed == true: nameWithOutTemplateArguments = %s \n", nameWithOutTemplateArguments.str());
#endif
#if 0
          printf ("Exiting as a test! \n");
          ROSE_ASSERT(false);
#endif
        }

     SgName nameWithTemplateArguments    = nameWithOutTemplateArguments;

  // DQ (9/13/2012): These are used for template declarations.
     SgName nameWithOutTemplateSpecializationArguments = nameWithOutTemplateArguments;
     SgName nameWithTemplateSpecializationArguments    = nameWithOutTemplateSpecializationArguments;
#ifdef DEBUG_CONVERSION
     indent_line(); debug_print("convert_struct_secondary(): p = %p name = %s : primary at ",p,nameWithOutTemplateArguments.str()); print_source_corresp(p->source_corresp); debug_print("\n");
#endif

#if 0
     if (nameWithOutTemplateArguments == "REQUESTED_PUSH_BACK_SPECIALIZATION_FOR_SEQUENCE_DOES_NOT_EXIST")
        {
          printf ("Debugging output for specific class \n");
          summarize_type(p);
        }
#endif

#if DEBUG_STRUCTURAL_DESIGN || 0
     printf ("******************************************************************************************* \n");
     printf ("In convert_struct_secondary(p=%p): (TOP) nameWithOutTemplateArguments = %s originally_unnamed = %s \n",p,nameWithOutTemplateArguments.str(),originally_unnamed ? "true" : "false");
     printf ("******************************************************************************************* \n");
#endif

#if 0
     printf ("In convert_struct_secondary(): calling summarize_type() \n");
     summarize_type(p);
#endif

#ifdef DEBUG_CONVERSION
  // This outputs more accurate source position information (where the secondary is used instead of where the primary was declared).

  // DQ (7/17/2012): we want to output the source position of the secondary declaration (not the primary).
  // this is part of fixing the incorrect interleaving of CPP macros into test test2012_125.C.
     if (p->source_corresp.decl_pos_info != NULL)
        {
       // DQ (5/13/2012): But we add the options in EDG to save more source position information (and thus the start AND end position is available to us at p->decl_pos_info).
          indent_line(); debug_print("-- secondary identifier_range start at:"); print_source_pos(p->source_corresp.decl_pos_info->identifier_range.start); debug_print("\n");
          indent_line(); debug_print("-- secondary identifier_range end at  :"); print_source_pos(p->source_corresp.decl_pos_info->identifier_range.end); debug_print("\n");

          indent_line(); debug_print("-- secondary specifiers_range start at:"); print_source_pos(p->source_corresp.decl_pos_info->specifiers_range.start); debug_print("\n");
          indent_line(); debug_print("-- secondary specifiers_range end at  :"); print_source_pos(p->source_corresp.decl_pos_info->specifiers_range.end); debug_print("\n");
        }
       else
        {
       // Sometimes the source position is just not available.
          indent_line(); debug_print("-- p->decl_pos_info == NULL \n");
        }
#endif

     ROSE_ASSERT (topScopeStack() != NULL);

#if 0
  // DQ (12/21/2011): We don't want to handle this by forcing declarations to be template declarations (see test2011_198.C).
  // DQ (12/19/2011): This is part of support to avoid template instantiations inside of template declarations.
     bool forceTemplateDeclaration = false;
     if (templateDeclarationStack.empty() == false)
        {
          printf ("Note: Inside of convert_struct_secondary(): templateDeclarationStack non-empty --- template declaration setting forceTemplateDeclaration = true \n");
          forceTemplateDeclaration = true;
        }
#endif

  // DQ (4/22/2012): Test the maps that we use internally (see function for documentation).
     testEdgRoseMaps();

#if DEBUG_GET_DECLARATION_SCOPE
     printf ("In convert_struct_secondary(): calling getDeclarationScope() \n");
#endif

  // The scope of a declaration (in C++) can be different from the current scope (where it is placed structurally).
  // SgScopeStatement* scopeOfDeclaration = getDeclarationScope(p->source_corresp);
     SgScopeStatement* scopeOfDeclaration = getDeclarationScope(p->source_corresp);
     ROSE_ASSERT(scopeOfDeclaration != NULL);

#if 0
     printf ("In convert_struct_secondary(): scopeOfDeclaration = %p = %s \n",scopeOfDeclaration,scopeOfDeclaration->class_name().c_str());
#endif

#if 0
     printf ("In convert_struct_secondary(): Exiting as a test! \n");
     ROSE_ASSERT(false);
#endif

#if 0
     printf ("In convert_struct_secondary(): Exiting as a test! \n");
     ROSE_ASSERT(false);
#endif

  // DQ (4/22/2012): Test the maps that we use internally (see function for documentation).
     testEdgRoseMaps();

     bool buildTemplateDeclaration   = false;
     bool buildTemplateInstantiation = false;
     bool forceTemplateDeclaration   = false;
     setTemplateOrTemplateInstantiationClassGeneration(p,forceTemplateDeclaration,buildTemplateDeclaration,buildTemplateInstantiation);

#if 0
     printf ("In convert_struct_secondary(): buildTemplateDeclaration   = %s \n",buildTemplateDeclaration   ? "true" : "false");
     printf ("In convert_struct_secondary(): buildTemplateInstantiation = %s \n",buildTemplateInstantiation ? "true" : "false");
#endif

#if 0
     printf ("In convert_struct_secondary(): Calling parse_template_parameters() \n");
#endif

  // DQ (9/10/2012): Setup the template parameters (required to disambiguate templates when the names are not enough). See test2012_209.C
     SgTemplateParameterPtrList templateParameterList;
     if (buildTemplateDeclaration == true) {
       templateParameterList = parse_template_parameters(p,scopeOfDeclaration,NULL);
     }

#if 0
     printf ("DONE: In convert_struct_secondary(): Calling parse_template_parameters() \n");
#endif

  // DQ (9/11/2012): Setup the template specialization arguments (these also have to be passed to the SageBuilder API (since they are also needed to disambiguate template names).
  // a_template_arg_ptr templateSpecializationArgs = p->variant.class_struct_union.extra_info->partial_spec_template_arg_list;
  // This is the list of template arguments on which the list of "actual" arguments is based.
     a_template_arg_ptr templateSpecializationArgs        = p->variant.class_struct_union.extra_info->partial_spec_template_arg_list;
  // This is the list of "actual" template arguments.
     a_template_arg_ptr templateSpecializationArgs_actual = p->variant.class_struct_union.extra_info->template_arg_list;
     SgTemplateArgumentPtrList templateSpecializationArgumentList;
     if (templateSpecializationArgs != NULL)
        {
       // DQ (9/12/2012): This is tricky: If this is a partial specialization then the "actual" template arguments are in 
       // the p->variant.class_struct_union.extra_info->template_arg_list and the associated names of the template arguments 
       // are in the p->variant.class_struct_union.extra_info->partial_spec_template_arg_list list.  This is because the names
       // need not match that of where they are defined in an outer template.
          ROSE_ASSERT(templateSpecializationArgs_actual != NULL);
       // templateSpecializationArgumentList = parse_template_arguments(templateSpecializationArgs,scopeOfDeclaration);
#if 0
          printf ("In convert_struct_secondary(): Calling parse_template_arguments to support templateSpecializationArgumentList \n");
#endif
#if 0
       // DQ (2/11/2019): Adding support to track the min_template_arguments.
          a_class_type_supplement_ptr classTypeSupplement = p->variant.class_struct_union.extra_info;
          ROSE_ASSERT((p->kind == tk_class || p->kind == tk_struct || p->kind == tk_union) && classTypeSupplement != NULL);
          printf ("In convert_struct_secondary(): classTypeSupplement->min_template_arguments = %d \n",classTypeSupplement->min_template_arguments);
#endif
          templateSpecializationArgumentList = parse_template_arguments(templateSpecializationArgs_actual,scopeOfDeclaration);
#if 0
          printf ("In convert_struct_secondary(): DONE: Calling parse_template_arguments to support templateSpecializationArgumentList \n");
#endif
        }

  // DQ (9/13/2012): Set the name used for some references to templates (e.g. calls to get the symbol).
     nameWithTemplateSpecializationArguments = appendTemplateArgumentsToName(nameWithOutTemplateSpecializationArguments,templateSpecializationArgumentList);

  // DQ (1/2/2012): The templateArgumentsList is computed once and used twice (1st to fixup the name, 2nd to append template arguments to the template instantiation).
     ROSE_ASSERT(p->variant.class_struct_union.extra_info != NULL);
     a_template_arg_ptr templateArgs = p->variant.class_struct_union.extra_info->template_arg_list;
     SgTemplateArgumentPtrList templateArgumentsList;
     if (buildTemplateInstantiation == true && templateArgs != NULL)
        {
#if 0
          printf ("In convert_struct_secondary(): Calling parse_template_arguments to support templateArgumentList \n");
#endif
#if 0
       // DQ (2/11/2019): Adding support to track the min_template_arguments.
          a_class_type_supplement_ptr classTypeSupplement = p->variant.class_struct_union.extra_info;
          ROSE_ASSERT((p->kind == tk_class || p->kind == tk_struct || p->kind == tk_union) && classTypeSupplement != NULL);
          printf ("In convert_struct_secondary(): classTypeSupplement->min_template_arguments = %d \n",classTypeSupplement->min_template_arguments);
#endif
          templateArgumentsList = parse_template_arguments(templateArgs,scopeOfDeclaration);
#if 0
          printf ("In convert_struct_secondary(): DONE: Calling parse_template_arguments to support templateArgumentList \n");
#endif

       // DQ (7/25/2012): Added test. We can't test this until the parent is set for each SgTemplateArgument.
       // testTemplateArgumentParents(templateArgumentsList,NULL);

       // DQ (6/6/2012): Make sure that this is valid (might not always be true).
          ROSE_ASSERT(templateArgumentsList.size() > 0);
        }
       else
        {
          if (buildTemplateInstantiation == true)
             {
#ifdef DEBUG_CONVERSION
               printf ("WARNING: In convert_struct_secondary() templateArgs == NULL \n");
#endif
             }
        }

     if (buildTemplateInstantiation == true)
        {
          nameWithTemplateArguments = appendTemplateArgumentsToName(nameWithOutTemplateArguments,templateArgumentsList);
        }

#if 0
     printf ("In convert_struct_secondary(): nameWithOutTemplateArguments            = %s \n",nameWithOutTemplateArguments.str());
     printf ("In convert_struct_secondary(): nameWithTemplateArguments               = %s \n",nameWithTemplateArguments.str());
     printf ("In convert_struct_secondary(): nameWithTemplateSpecializationArguments = %s \n",nameWithTemplateSpecializationArguments.str());
#endif

  // We need to be able to return either a SgClassDeclaration (or class instantiation) or a SgTemplateClassDeclaration.
     SgClassDeclaration* decl = NULL;
  // SgDeclarationStatement* decl = NULL;

     SgDeclarationStatement* firstNondefdecl = NULL;

  // SgClassDeclaration* decl = set_file_info_for_corresp(buildNondefiningClassDeclaration_nfi(name, t, topScopeStack()), p->source_corresp);
  // SgClassDeclaration* decl = set_file_info_for_corresp(buildNondefiningClassDeclaration_nfi(name, t, scopeOfDeclaration), p->source_corresp);
     if (decl == NULL)
        {
       // Get the nondefining declaration from the symbol if it has been built (if this works, 
       // then we likely don't need the "SgClassDeclaration* nonDefiningDecl" parameter).
       // SgClassDeclaration* firstNondefdecl = NULL;
       // SgDeclarationStatement* firstNondefdecl = NULL;

          ROSE_ASSERT(scopeOfDeclaration != NULL);

       // DQ (8/27/2014): This is a higher bar to pass on the type in the cache.
       // ROSE_ASSERT (type_translation_cache.find(p) == type_translation_cache.end());
#if 0
          printf ("In convert_struct_secondary(): Calling retrieveAssociatedClassSymbol() \n");
          printf ("scopeOfDeclaration = %p = %s \n",scopeOfDeclaration,scopeOfDeclaration->class_name().c_str());
          SgTemplateClassDefinition* tmp_templateClassDefinition = isSgTemplateClassDefinition(scopeOfDeclaration);
          if (tmp_templateClassDefinition != NULL)
            {
              printf ("tmp_templateClassDefinition = %p \n",tmp_templateClassDefinition);
              printf ("tmp_templateClassDefinition->get_members().size() = %zu \n",tmp_templateClassDefinition->get_members().size());
              SgTemplateClassDeclaration* tmp_templateClassDeclaration = isSgTemplateClassDeclaration(tmp_templateClassDefinition->get_declaration());
              ROSE_ASSERT(tmp_templateClassDeclaration != NULL);
              printf ("tmp_templateClassDeclaration->get_name() = %s \n",tmp_templateClassDeclaration->get_name().str());
            }
#endif
       // SgSymbol*      tmp_symbol = retrieveAssociatedClassSymbol(nameWithTemplateArguments,scopeOfDeclaration,buildTemplateDeclaration,&templateArgumentsList,&templateParameterList,&templateSpecializationArgumentList);
          SgSymbol*      tmp_symbol = retrieveAssociatedClassSymbol(nameWithOutTemplateArguments,scopeOfDeclaration,buildTemplateDeclaration,&templateArgumentsList,&templateParameterList,&templateSpecializationArgumentList);
          SgClassSymbol* mysymbol   = isSgClassSymbol(tmp_symbol);
#if 0
          printf ("DONE: In convert_struct_secondary(): Calling retrieveAssociatedClassSymbol(): mysymbol = %p \n",mysymbol);
#endif
       // DQ (4/22/2012): Test the maps that we use internally (see function for documentation).
          testEdgRoseMaps();
#if 0
          printf ("In convert_struct_secondary(): tmp_symbol = %p = %s -- mysymbol = %p = %s \n",
               tmp_symbol,(tmp_symbol != NULL) ? tmp_symbol->class_name().c_str() : "null" ,mysymbol,(mysymbol != NULL) ? mysymbol->class_name().c_str() : "null");
#endif

          if (mysymbol != NULL)
             {
#if 0
               printf ("In convert_struct_secondary(): scopeOfDeclaration = %p = %s mysymbol = %p = %s \n",scopeOfDeclaration,scopeOfDeclaration->class_name().c_str(),mysymbol,mysymbol->class_name().c_str());
#endif
               firstNondefdecl = isSgClassDeclaration(mysymbol->get_declaration());

            // DQ (9/5/2012): Added assertion.
               ROSE_ASSERT(firstNondefdecl != NULL);

#if 0
               printf ("buildTemplateDeclaration = %s \n",buildTemplateDeclaration ? "true" : "false");
#endif

            // Build the nondefining declaration (build functions will connect to the correct symbols).
               if (buildTemplateDeclaration == true)
                  {
#if 0
                    printf ("############## BEFORE Calling buildNondefiningTemplateClassDeclaration_nfi(): nameWithOutTemplateArguments = %s \n",nameWithOutTemplateArguments.str());
#endif
                 // This must be a separate secondary declaration (to avoid multiple references to the same IR node in the AST).
                    decl = set_file_info_for_corresp(buildNondefiningTemplateClassDeclaration_nfi(nameWithOutTemplateArguments, t, scopeOfDeclaration, &templateParameterList,&templateSpecializationArgumentList), p->source_corresp);
                    ROSE_ASSERT(decl != NULL);
                    ROSE_ASSERT(isSgTemplateClassDeclaration(decl) != NULL);

                    for (SgTemplateParameterPtrList::iterator it = templateParameterList.begin(); it != templateParameterList.end(); it++) {
                      if ((*it)->get_parent() != NULL) {
                        SgDeclarationStatement * declstmt_parent = isSgDeclarationStatement((*it)->get_parent());
                        ROSE_ASSERT(declstmt_parent != NULL);
                        ROSE_ASSERT(declstmt_parent->get_firstNondefiningDeclaration() == decl->get_firstNondefiningDeclaration());
                      } else {
                        (*it)->set_parent(decl);
                      }
                    }

                 // DQ (12/29/2011): Add this template to the cache. We must do so as soon as possible to permit references inside the class to be resolved.
                 // ROSE_ASSERT(p->variant.class_struct_union.extra_info->assoc_template != NULL);
                 // TV (08/23/12): 
                    if (p->variant.class_struct_union.extra_info->assoc_template != NULL)
                       {
                      // DQ (8/15/2015): We want to always use the first non-defining declaration.
                      // first_nondefining_declarations_for_templates[p->variant.class_struct_union.extra_info->assoc_template] = decl;
                         ROSE_ASSERT(decl->get_firstNondefiningDeclaration() != NULL);
                         setFirstNondefDeclForTemplate(p->variant.class_struct_union.extra_info->assoc_template, decl->get_firstNondefiningDeclaration());
                       }
                      else
                       {
#ifdef DEBUG_CONVERSION
                         printf ("Warning: p->variant.class_struct_union.extra_info->assoc_template == NULL \n");
#endif
                       }

                 // DQ (12/12/2011): Added call to get type to force generation of SgClassType using template member function.
                    SgClassType* templateClassType = isSgTemplateClassDeclaration(decl)->get_type();
                    ROSE_ASSERT(templateClassType != NULL);

#if 0
                    printf ("############## AFTER Calling buildNondefiningTemplateClassDeclaration_nfi(): nameWithOutTemplateArguments = %s \n",nameWithOutTemplateArguments.str());
#endif
                 // DQ (3/6/2012): Added test.
                    SgClassType* classType = templateClassType;
                    ROSE_ASSERT (classType != NULL);
                    ROSE_ASSERT (classType->get_declaration() != NULL);
                    SgClassDeclaration* local_classDeclaration = isSgClassDeclaration(classType->get_declaration());
                    ROSE_ASSERT (local_classDeclaration != NULL);
                    ROSE_ASSERT (local_classDeclaration->get_firstNondefiningDeclaration() != NULL);
                    ROSE_ASSERT (local_classDeclaration->get_firstNondefiningDeclaration() == local_classDeclaration);

#if 0
                 // DQ (9/9/2012): It appears that the template parameter list is not set properly (frequently zero, at least).
                    printf ("In convert_struct_secondary(): decl->get_templateParameters().size() = %" PRIuPTR " \n",isSgTemplateClassDeclaration(decl)->get_templateParameters().size());
#endif
                 // DQ (9/9/2012): It appears that the template parameter list is not set properly (frequently zero, at least).
                 // ROSE_ASSERT(isSgTemplateClassDeclaration(decl)->get_templateParameters().size() > 0);
                  }
                 else
                  {
#if 0
                 // DQ (5/18/2014): Added debugging code.
                    printf ("Before call to buildNondefiningClassDeclaration_nfi(): nameWithOutTemplateArguments = %s \n",nameWithOutTemplateArguments.str());
                    printf ("Before call to buildNondefiningClassDeclaration_nfi(): buildTemplateInstantiation = %s \n",buildTemplateInstantiation ? "true" : "false");
                    printf ("Before call to buildNondefiningClassDeclaration_nfi(): firstNondefdecl = %p = %s \n",firstNondefdecl,firstNondefdecl->class_name().c_str());
#endif
                 // DQ (6/6/2012): Added support to pass the template arguments (used only if buildTemplateInstantiation == true).
                 // decl = set_file_info_for_corresp(buildNondefiningClassDeclaration_nfi(nameWithTemplateArguments, t, scopeOfDeclaration, buildTemplateInstantiation), p->source_corresp);
                 // decl = buildNondefiningClassDeclaration_nfi(nameWithTemplateArguments, t, scopeOfDeclaration, buildTemplateInstantiation);
                 // decl = buildNondefiningClassDeclaration_nfi(nameWithTemplateArguments, t, scopeOfDeclaration, buildTemplateInstantiation, &templateArgumentsList);
                    decl = buildNondefiningClassDeclaration_nfi(nameWithOutTemplateArguments, t, scopeOfDeclaration, buildTemplateInstantiation, &templateArgumentsList);
                    ROSE_ASSERT(decl != NULL);
#if 0
                    printf ("Build a secondary declaration (buildNondefiningClassDeclaration_nfi()) decl = %p = %s \n",decl,decl->class_name().c_str());
#endif
                 // DQ (2/11/2012): Set this to be compiler generated if it is a template instantiation.
                    if (buildTemplateInstantiation == true)
                       {
                      // Note that the source position info in EDG for template instantations refers to the template 
                      // declaration (so to be more precise we want to mark the source position as compiler generated instead).
                         decl = set_generated_file_info(decl);
                       }
                      else
                       {
                         decl = set_file_info_for_corresp(decl, p->source_corresp);
                       }

                    ROSE_ASSERT(decl != NULL);
                    ROSE_ASSERT(isSgClassDeclaration(decl) != NULL);
                  }

#if 0
               decl->get_file_info()->display("In convert_struct_secondary(): (mysymbol != NULL) debug");
#endif

#if 0
               printf ("Declaration found from symbol = %p = %s \n",firstNondefdecl,firstNondefdecl->class_name().c_str());
#endif
            // DQ (11/29/2011): Adding checks...
               ROSE_ASSERT(firstNondefdecl == firstNondefdecl->get_firstNondefiningDeclaration());

               ROSE_ASSERT(firstNondefdecl != NULL);
            // ROSE_ASSERT(firstNondefdecl->get_parent() != NULL);
               if (firstNondefdecl->get_parent() == NULL)
                  {
#ifdef DEBUG_CONVERSION
                    printf ("In convert_struct_secondary(): Note that firstNondefdecl->get_parent() == NULL, this might be OK. \n");
#endif
                  }

            // Currently the decl->get_firstNondefiningDeclaration() == decl, but we want to reference the first non-defining declaration.
               decl->set_firstNondefiningDeclaration(firstNondefdecl);

            // DQ (11/29/2011): Adding checks...
            // ROSE_ASSERT(decl->get_firstNondefiningDeclaration() == decl);
#if 0
               printf ("In convert_struct_secondary(): set decl = %p to have firstNondefiningDeclaration = %p \n",decl,decl->get_firstNondefiningDeclaration());
#endif
            // Make the defining declaration consistant, even if it is NULL (because it has not been set yet.
            // firstNondefdecl->set_definingDeclaration(decl);
#if 0
               printf ("In convert_struct_secondary(): firstNondefdecl->get_definingDeclaration() = %p \n",firstNondefdecl->get_definingDeclaration());
#endif

#if 0
            // DQ (9/5/2012): I think this is wrong (backwards), we have to set the new declaration's defining declaration.
            // But this is the only version of this code that apepars to pass all of the tests.
               firstNondefdecl->set_definingDeclaration(decl->get_definingDeclaration());
#else
            // DQ (9/5/2012): Was this set by the SageBuilder function (e.g. buildNondefiningClassDeclaration_nfi()).
            // printf ("In convert_struct_secondary(): (Was this set by the SageBuilder function) decl->get_definingDeclaration() = %p \n",decl->get_definingDeclaration());
            // ROSE_ASSERT(decl->get_definingDeclaration() == NULL);
               if (decl->get_definingDeclaration() != NULL)
                  {
#ifdef DEBUG_CONVERSION
                    printf ("In convert_struct_secondary(): (Was this set by the SageBuilder function) decl->get_definingDeclaration() = %p = %s \n",decl->get_definingDeclaration(),decl->get_definingDeclaration()->class_name().c_str());
#endif
                  }
               
            // DQ (9/5/2012): This is the code we want, but it causes test2004_85.C to fail.
               if (firstNondefdecl->get_definingDeclaration() != NULL)
                  {
                    if (decl->get_definingDeclaration() != NULL)
                       {
                      // DQ (9/5/2012): If this already has a valid value, then make sure it is the same value (that we will be using).
                         ROSE_ASSERT(firstNondefdecl->get_definingDeclaration() == decl->get_definingDeclaration());
                       }

                    decl->set_definingDeclaration(firstNondefdecl->get_definingDeclaration());
                  }
                 else
                  {
                 // If the definingDeclaration has not bee updated with the identified definingDeclaration then update it now.
                    if (decl->get_definingDeclaration() != NULL)
                       {
                         printf ("ERROR: The processing of the defining declaration should have caused the first non-defining declaration's defining declaration to be updated. \n");
                         ROSE_ASSERT(false);
                       }

                    ROSE_ASSERT(decl->get_definingDeclaration() == NULL);
                 // firstNondefdecl->set_definingDeclaration(decl->get_definingDeclaration()); 
                  }
#endif
            // DQ (9/5/2012): I think this can be asserted.
               ROSE_ASSERT(firstNondefdecl->get_definingDeclaration() == decl->get_definingDeclaration());

            // *********************************************************************
            // NOTE: SAME CODE AS BELOW (REFACTOR).
            // DQ (12/11/2011): Adding the scope of the class (assoc_scope) to the scope_cache.
#if 0
               printf ("Make sure this is tk_class... \n");
#endif

            // DQ (12/28/2011): This is the proper scope for the class.

            // DQ (4/22/2012): WRONG, is the the associated scope of the class, but the scope of class is in parent_scope.
            // DQ (12/21/2011): I think this (assoc_scope) is a linked list of scopes associated with the member functions of the class and
            // not anything associated with the class definition scope.  So this is not the scope that we are looking for...
            // a_scope_ptr edg_scope = p->variant.class_struct_union.extra_info->assoc_scope;
               a_scope_ptr edg_scope = p->source_corresp.parent_scope;

            // ROSE_ASSERT(edg_scope != NULL);
               if (edg_scope != NULL)
                  {
                    SgScopeStatement* currentScope = topScopeStack();
                    ROSE_ASSERT(currentScope != NULL);
#if 0
                    printf ("In convert_struct_secondary(): (part 1) currentScope = %p = %s \n",currentScope,currentScope->class_name().c_str());
#endif
                 // DQ (4/22/2012): Test the maps that we use internally (see function for documentation).
                    testEdgRoseMaps();

                 // ROSE_ASSERT(isSgClassDefinition(currentScope) != NULL);
                 // scope_cache[edg_scope] = currentScope;
                    if (scope_cache.find(edg_scope) == scope_cache.end())
                       {
                      // ROSE_ASSERT(scope_cache[edg_scope] != NULL);
                      // ROSE_ASSERT(isSgClassDefinition(currentScope) != NULL);
                         scope_cache[edg_scope] = currentScope;
                       }
                      else
                       {
                         ROSE_ASSERT(scope_cache[edg_scope] != NULL);
                       }

                 // DQ (4/22/2012): Test the maps that we use internally (see function for documentation).
                    testEdgRoseMaps();
                  }
            // *********************************************************************
#if 0
               printf ("Exiting after setting the scope_cache \n");
               ROSE_ASSERT(false);
#endif
#ifdef DEBUG_CONVERSION
               printf ("In convert_struct_secondary(): set decl = %p to have definingDeclaration = %p \n",decl,decl->get_definingDeclaration());
#endif
            // In this case, decl is a nondefining declaration, but might not be the first non-defining declaration!
               ROSE_ASSERT(firstNondefdecl->get_definingDeclaration() != decl);
#if 0
               printf ("Marking class declaration decl = %p as a forward declaration \n",decl);
#endif
               decl->setForward();

            // Testing to make sure we have the correct IR nodes.
               if (buildTemplateDeclaration == true)
                  {
                    ROSE_ASSERT(isSgTemplateClassDeclaration(decl) != NULL);
                  }
                 else
                  {
                    if (buildTemplateInstantiation == true)
                       {
                         ROSE_ASSERT(isSgTemplateInstantiationDecl(decl) != NULL);
                       }
                      else
                       {
                         ROSE_ASSERT(isSgClassDeclaration(decl) != NULL);
                       }
                  }

#if 0
               if (buildTemplateDeclaration == true)
                  {
                 // I think this may be the wrong name to use: we want to use the name with template specialization arguments.
                    printf ("WARNING: I think this may be the wrong name to use: we want to use the name with template specialization arguments \n");
                    printf ("In convert_struct_secondary(): Calling lookup_template_class_symbol() using nameWithTemplateSpecializationArguments = %s \n",nameWithTemplateSpecializationArguments.str());
                  }
#endif
            // DQ (12/27/2011): Added new assertion.
            // ROSE_ASSERT(buildTemplateDeclaration ? decl->get_scope()->lookup_template_class_symbol(nameWithOutTemplateArguments) != NULL : decl->get_scope()->lookup_nontemplate_class_symbol(nameWithTemplateArguments,&templateArgumentsList) != NULL );
            // ROSE_ASSERT(buildTemplateDeclaration ? decl->get_scope()->lookup_template_class_symbol(nameWithOutTemplateArguments,&templateParameterList,&templateSpecializationArgumentList) != NULL : 
            //                                        decl->get_scope()->lookup_nontemplate_class_symbol(nameWithTemplateArguments,&templateArgumentsList) != NULL );
               ROSE_ASSERT(buildTemplateDeclaration ? decl->get_scope()->lookup_template_class_symbol(nameWithTemplateSpecializationArguments,&templateParameterList,&templateSpecializationArgumentList) != NULL : 
                                                      decl->get_scope()->lookup_nontemplate_class_symbol(nameWithTemplateArguments,&templateArgumentsList) != NULL );

            // DQ (9/6/2012): Added assertion.
               ROSE_ASSERT(firstNondefdecl != NULL);

            // DQ (9/9/2012): Test the maps that we use internally (see function for documentation).
               testEdgRoseMaps();
             }
            else
             {
               ROSE_ASSERT(mysymbol == NULL);
#if 0
               printf ("In convert_struct_secondary(): buildTemplateDeclaration = %s scopeOfDeclaration = %p = %s mysymbol == NULL \n",
                    buildTemplateDeclaration ? "true" : "false",scopeOfDeclaration,scopeOfDeclaration->class_name().c_str());
#endif

            // DQ (11/29/2011): There was no declaration found via the symbol table, so build one.
               if (buildTemplateDeclaration == true)
                  {
#if 0
                 // DQ (12/15/2019): Debugging Cxx11_tests/test2019_488.C
                    if (template_type_translation_cache[p] != NULL)
                       {
                         printf ("Error: failing assertion: template_type_translation_cache[p] == NULL \n");
                         printf ("In convert_struct_secondary(): p = %p \n",p);
                         printf ("type_translation_cache[p] = %p = %s \n",template_type_translation_cache[p],template_type_translation_cache[p]->class_name().c_str());

                      // DQ (12/14/2019): Added debugging code.
                         SgClassType* tmp_classType = isSgClassType(template_type_translation_cache[p]);
                         if (tmp_classType != NULL)
                            {
                              SgClassDeclaration* classDeclaration = isSgClassDeclaration(tmp_classType->get_declaration());
                              ROSE_ASSERT(classDeclaration != NULL);
                              printf ("classDeclaration->get_name() = %s \n",classDeclaration->get_name().str());
                            }
                           else
                            {
                              printf ("template_type_translation_cache[p] is not a SgClassType \n");
                            }
                         printf ("In convert_struct_secondary(): calling sumarize_type(): p = %p \n",p);
                         summarize_type(p);
                       }
#endif
                    ROSE_ASSERT (template_type_translation_cache[p] == NULL);

                 // decl = set_file_info_for_corresp(buildNondefiningTemplateClassDeclaration_nfi(nameWithOutTemplateArguments, t, scopeOfDeclaration), p->source_corresp);
                    decl = set_file_info_for_corresp(buildNondefiningTemplateClassDeclaration_nfi(nameWithOutTemplateArguments, t, scopeOfDeclaration, &templateParameterList,&templateSpecializationArgumentList), p->source_corresp);
                    ROSE_ASSERT(decl != NULL);
                    ROSE_ASSERT(isSgTemplateClassDeclaration(decl) != NULL);

                    for (SgTemplateParameterPtrList::iterator it = templateParameterList.begin(); it != templateParameterList.end(); it++) {
                      if ((*it)->get_parent() != NULL) {
                        SgDeclarationStatement * declstmt_parent = isSgDeclarationStatement((*it)->get_parent());
                        ROSE_ASSERT(declstmt_parent != NULL);
                        ROSE_ASSERT(declstmt_parent->get_firstNondefiningDeclaration() == decl->get_firstNondefiningDeclaration());
                      } else {
                        (*it)->set_parent(decl);
                      }
                    }

#if 0
                    printf ("################ decl = %p = %s \n",decl,decl->class_name().c_str());
                    printf ("################ decl->get_type() = %p = %s \n",decl->get_type(),decl->get_type()->class_name().c_str());
#endif

                 // I think this may be the wrong name to use: we want to use the name with template specialization arguments.
                 // printf ("WARNING: I think this may be the wrong name to use: we want to use the name with template specialization arguments (shoudl use nameWithTemplateSpecializationArguments) \n");
#ifdef DEBUG_CONVERSION
                    printf ("In convert_struct_secondary(): Calling lookup_template_class_symbol() using nameWithTemplateSpecializationArguments = %s \n",nameWithTemplateSpecializationArguments.str());
#endif
                 // ROSE_ASSERT(decl->get_scope()->lookup_template_class_symbol(nameWithOutTemplateArguments) != NULL);
                 // ROSE_ASSERT(decl->get_scope()->lookup_template_class_symbol(nameWithOutTemplateArguments,&templateParameterList,&templateSpecializationArgumentList) != NULL);
                    ROSE_ASSERT(decl->get_scope()->lookup_template_class_symbol(nameWithTemplateSpecializationArguments,&templateParameterList,&templateSpecializationArgumentList) != NULL);

                    if (p->variant.class_struct_union.extra_info->assoc_template != NULL)
                       {
                      // DQ (8/15/2015): We want to always use the first non-defining declaration.
                      // first_nondefining_declarations_for_templates[p->variant.class_struct_union.extra_info->assoc_template] = decl;
                         ROSE_ASSERT(decl->get_firstNondefiningDeclaration() != NULL);
                         setFirstNondefDeclForTemplate(p->variant.class_struct_union.extra_info->assoc_template, decl->get_firstNondefiningDeclaration());
                       }
                  }
                 else
                  {
                 // DQ (6/6/2012): Added support to pass the template arguments (used only if buildTemplateInstantiation == true).
                 // DQ (5/31/2012): Use the name without template arguments.
                 // decl = set_file_info_for_corresp(buildNondefiningClassDeclaration_nfi(nameWithTemplateArguments, t, scopeOfDeclaration, buildTemplateInstantiation), p->source_corresp);
                 // decl = buildNondefiningClassDeclaration_nfi(nameWithTemplateArguments, t, scopeOfDeclaration, buildTemplateInstantiation);
                 // decl = buildNondefiningClassDeclaration_nfi(nameWithOutTemplateArguments, t, scopeOfDeclaration, buildTemplateInstantiation);
                 // decl = buildNondefiningClassDeclaration_nfi(nameWithTemplateArguments, t, scopeOfDeclaration, buildTemplateInstantiation);
                 // decl = buildNondefiningClassDeclaration_nfi(nameWithTemplateArguments, t, scopeOfDeclaration, buildTemplateInstantiation, &templateArgumentsList);
                    decl = buildNondefiningClassDeclaration_nfi(nameWithOutTemplateArguments, t, scopeOfDeclaration, buildTemplateInstantiation, &templateArgumentsList);
                    ROSE_ASSERT(decl != NULL);

#ifdef DEBUG_CONVERSION
                    printf ("In convert_struct_secondary(): after buildNondefiningClassDeclaration_nfi(): decl->get_type() = %p = %s \n",decl->get_type(),decl->get_type()->class_name().c_str());
#endif
#if 0
                    printf ("In convert_struct_secondary(): after buildNondefiningClassDeclaration_nfi(): decl->get_type() = %p = %s \n",decl->get_type(),decl->get_type()->class_name().c_str());
#endif
                 // DQ (5/31/2012): Set the template name before we generate the type.
                 // isSgTemplateInstantiationDecl(nondefdecl)->set_templateName(name);

                 // DQ (8/27/2014): This is a higher bar to pass on the type in the cache.
                 // ROSE_ASSERT (type_translation_cache.find(p) == type_translation_cache.end());

                    if (buildTemplateInstantiation == true)
                       {
                      // DQ (2/9/2012): Mark template instantations as compiler generated, though some may not be if they are explicit and we need to detect that case.
                         decl = set_generated_file_info(decl);

                      // DQ (5/31/2012): Added test for properly set templateName (name withouth template arguments; used to construct proper name with template arguments).
                         ROSE_ASSERT(isSgTemplateInstantiationDecl(decl)->get_templateName().is_null() == false);
                       }
                      else
                       {
                         decl = set_file_info_for_corresp(decl, p->source_corresp);
                       }
                    ROSE_ASSERT(decl != NULL);
                    ROSE_ASSERT(isSgClassDeclaration(decl) != NULL);

#ifdef DEBUG_CONVERSION
                    printf ("In convert_struct_secondary(): after buildNondefiningClassDeclaration_nfi(): nameWithTemplateArguments = %s \n",nameWithTemplateArguments.str());
#endif
                    ROSE_ASSERT(decl->get_scope()->lookup_nontemplate_class_symbol(nameWithTemplateArguments,&templateArgumentsList) != NULL);
                  }

               ROSE_ASSERT(decl != NULL);

            // DQ (9/6/2012): Set this here.
               firstNondefdecl = decl;

#if 0
               decl->get_file_info()->display("In convert_struct_secondary(): (mysymbol == NULL) debug");
#endif

            // *********************************************************************
            // NOTE: SAME CODE AS ABOVE (REFACTOR).
            // DQ (12/11/2011): Adding the scope of the class (assoc_scope) to the scope_cache (see test2011_181.C).
#if 0
               printf ("Make sure this is tk_class... \n");
#endif
            // DQ (12/28/2011): This is the proper scope for the class.

            // DQ (4/22/2012): WRONG, is the the associated scope of the class, but the scope of class is in parent_scope.
            // DQ (12/21/2011): I think this (assoc_scope) is a linked list of scopes associated with the member functions of the class and
            // not anything associated with the class definition scope.  So this is not the scope that we are looking for...
            // a_scope_ptr edg_scope = p->variant.class_struct_union.extra_info->assoc_scope;
               a_scope_ptr edg_scope = p->source_corresp.parent_scope;
#if 0
               printf ("In convert_struct_secondary(): edg_scope = %p \n",edg_scope);
#endif
            // ROSE_ASSERT(edg_scope != NULL);
               if (edg_scope != NULL)
                  {
                    SgScopeStatement* currentScope = topScopeStack();
                    ROSE_ASSERT(currentScope != NULL);
#if 0
                    printf ("In convert_struct_secondary(): (part 2) edg_scope = %p currentScope = %p = %s \n",edg_scope,currentScope,currentScope->class_name().c_str());
#endif
                 // DQ (4/22/2012): Test the maps that we use internally (see function for documentation).
                    testEdgRoseMaps();

                    if (scope_cache.find(edg_scope) == scope_cache.end())
                       {
                      // ROSE_ASSERT(scope_cache[edg_scope] != NULL);
                      // ROSE_ASSERT(isSgClassDefinition(currentScope) != NULL);
                         scope_cache[edg_scope] = currentScope;
#if 0
                         printf ("In convert_struct_secondary(): (part 2) calling summarize_type() \n");
                         summarize_type(p);
#endif
#if 0
                         printf ("In convert_struct_secondary(): (part 2) Calling summarize_scope() \n");
                         summarize_scope(edg_scope);
#endif
                       }
                      else
                       {
                         ROSE_ASSERT(scope_cache[edg_scope] != NULL);
                       }

                 // DQ (4/22/2012): Test the maps that we use internally (see function for documentation).
                    testEdgRoseMaps();
                  }
            // *********************************************************************

#if 0
               if (buildTemplateDeclaration == true)
                  {
                 // I think this may be the wrong name to use: we want to use the name with template specialization arguments.
                 // printf ("WARNING: I think this may be the wrong name to use: we want to use the name with template specialization arguments (should be using nameWithTemplateSpecializationArguments) \n");
                    printf ("In convert_struct_secondary(): Calling lookup_template_class_symbol() using nameWithTemplateSpecializationArguments = %s \n",nameWithTemplateSpecializationArguments.str());

                  }
#endif

            // DQ (12/27/2011): Added new assertion.
            // ROSE_ASSERT(buildTemplateDeclaration ? decl->get_scope()->lookup_template_class_symbol(nameWithOutTemplateArguments) != NULL : decl->get_scope()->lookup_nontemplate_class_symbol(nameWithTemplateArguments,&templateArgumentsList) != NULL );
            // ROSE_ASSERT(buildTemplateDeclaration ? decl->get_scope()->lookup_template_class_symbol(nameWithOutTemplateArguments,&templateParameterList,&templateSpecializationArgumentList) != NULL : 
            //                                        decl->get_scope()->lookup_nontemplate_class_symbol(nameWithTemplateArguments,&templateArgumentsList) != NULL );
               ROSE_ASSERT(buildTemplateDeclaration ? decl->get_scope()->lookup_template_class_symbol(nameWithTemplateSpecializationArguments,&templateParameterList,&templateSpecializationArgumentList) != NULL : 
                                                      decl->get_scope()->lookup_nontemplate_class_symbol(nameWithTemplateArguments,&templateArgumentsList) != NULL );

            // DQ (9/6/2012): Added assertion.
               ROSE_ASSERT(firstNondefdecl != NULL);

            // DQ (9/9/2012): Test the maps that we use internally (see function for documentation).
               testEdgRoseMaps();

#if 0
               printf ("Exiting after setting the scope_cache \n");
               ROSE_ASSERT(false);
#endif
             }

       // DQ (9/9/2012): Test the maps that we use internally (see function for documentation).
          testEdgRoseMaps();

       // If this is a template instantiation check that the associated template declaration is present.
          SgTemplateInstantiationDecl* templateInstantiation = isSgTemplateInstantiationDecl(decl);
#if 0
          printf ("In convert_struct_secondary(): templateInstantiation = %p \n",templateInstantiation);
#endif
          if (templateInstantiation != NULL)
             {
               ROSE_ASSERT(templateInstantiation->get_templateDeclaration() == NULL);

            // Make a note, since this is an opportunity to improve the build interface to take the 
            // SgTemplateDeclaration directly (but we need to think about this a bit more).
#if 0
               printf ("Note: In convert_struct_secondary(): Setting the templateDeclaration in the definingTemplateInstantiation after construction \n");
#endif
            // DQ (8/27/2014): This is a higher bar to pass on the type in the cache.
            // ROSE_ASSERT (type_translation_cache.find(p) == type_translation_cache.end());

            // DQ (5/18/2014): Handle the case where the EDG type does not include a valid assoc_template pointer.
            // This happens for Boost code where the first time the class is correctly interpreted to be a template instantiation, 
            // but the second time it is missing the pointer to an associated template, but it still needs to be interpreted as a 
            // template instantiation.  In this case we get the ROSE SgTemplateClassDeclaration from the associated first Nondefining 
            // SgTemplateInstantiationDecl.
               ROSE_ASSERT(p->variant.class_struct_union.extra_info != NULL);
            // ROSE_ASSERT(p->variant.class_struct_union.extra_info->assoc_template != NULL);
               SgTemplateClassDeclaration* templateDeclaration = NULL;
               if (p->variant.class_struct_union.extra_info->assoc_template == NULL)
                  {
                    SgTemplateInstantiationDecl* firstNondefiningTemplateInstantiationDeclaration = isSgTemplateInstantiationDecl(decl->get_firstNondefiningDeclaration());
                    ROSE_ASSERT(firstNondefiningTemplateInstantiationDeclaration != NULL);
                    templateDeclaration = firstNondefiningTemplateInstantiationDeclaration->get_templateDeclaration();
                    ROSE_ASSERT(templateDeclaration != NULL);
                    templateInstantiation->set_templateDeclaration(templateDeclaration);
                  }
                 else
                  {
                 // DQ (8/27/2014): This is a higher bar to pass on the type in the cache.
                 // ROSE_ASSERT (type_translation_cache.find(p) == type_translation_cache.end());

                    SgDeclarationStatement * declstmt = getTemplateDeclaration(p);
                    templateDeclaration = isSgTemplateClassDeclaration(declstmt);
                    ROSE_ASSERT(templateDeclaration != NULL);

                 // DQ (8/27/2014): This is a higher bar to pass on the type in the cache.
                 // ROSE_ASSERT (type_translation_cache.find(p) == type_translation_cache.end());

                    templateInstantiation->set_templateDeclaration(templateDeclaration);
                  }

            // DQ (8/27/2014): This is a higher bar to pass on the type in the cache.
            // ROSE_ASSERT (type_translation_cache.find(p) == type_translation_cache.end());

               ROSE_ASSERT(templateDeclaration != NULL);

               ROSE_ASSERT(templateInstantiation->get_templateDeclaration() != NULL);

            // DQ (9/13/2012): Calling the test in SageInterface.
            // DQ (7/25/2012): Added test.
            // testTemplateArgumentParents(templateArgumentsList,templateInstantiation);
               SageBuilder::testTemplateArgumentParents(templateInstantiation);

            // DQ (7/15/2012): This is now done in the SageBuilder functions (so we might not need this function).
            // Attach an associated template arguments.
            // process_template_arguments(p,templateInstantiation,templateArgumentsList);

            // DQ (3/27/2012): Normalize the names so that they can be used for code generation.
               normalizeTemplateName(nameWithOutTemplateArguments);

            // DQ (9/12/2012): We don't want to be setting the template name here (should be set by SageBuilder functions.
#ifdef DEBUG_CONVERSION
               printf ("WARNING: In convert_struct_secondary(): setting templateName for templateInstantiation = %p (should already be set) \n",templateInstantiation);
#endif
               ROSE_ASSERT(templateInstantiation->get_templateName().is_null() == false);

               templateInstantiation->set_templateName(nameWithOutTemplateArguments);

            // DQ (5/31/2012): Find locations where this is set and include template syntax.
            // ROSE_ASSERT(nameWithOutTemplateArguments.getString().find('<') == string::npos);
               ROSE_ASSERT(hasTemplateSyntax(nameWithOutTemplateArguments) == false);

               ROSE_ASSERT(templateInstantiation->get_templateName().is_null() == false);
               ROSE_ASSERT(isSgTemplateInstantiationDecl(templateInstantiation->get_firstNondefiningDeclaration())->get_templateName().is_null() == false);

            // DQ (8/27/2014): This is a higher bar to pass on the type in the cache.
            // ROSE_ASSERT (type_translation_cache.find(p) == type_translation_cache.end());

            // DQ (9/1/2012): If the templateDeclaration is not set in the firstNondefiningDeclaration, then set it explicitly.
               if (isSgTemplateInstantiationDecl(templateInstantiation->get_firstNondefiningDeclaration())->get_templateDeclaration() == NULL)
                  {
#ifdef DEBUG_CONVERSION
                    printf ("WARNING: In convert_struct_secondary(): I would have expected this to have been set templateInstantiation->get_firstNondefiningDeclaration() = %p templateDeclaration = %p \n",
                         templateInstantiation->get_firstNondefiningDeclaration(),templateDeclaration);
                    templateInstantiation->get_firstNondefiningDeclaration()->get_startOfConstruct()->display("I would have expected the templateDeclaration to have been set: firstNondefiningDeclaration: debug");
                    templateDeclaration->get_startOfConstruct()->display("I would have expected the templateDeclaration to have been set: templateDeclaration: debug");
#endif
                    isSgTemplateInstantiationDecl(templateInstantiation->get_firstNondefiningDeclaration())->set_templateDeclaration(templateDeclaration);
                  }
               ROSE_ASSERT(isSgTemplateInstantiationDecl(templateInstantiation->get_firstNondefiningDeclaration())->get_templateDeclaration() != NULL);

               if (templateInstantiation->get_definingDeclaration() != NULL)
                  {
                    ROSE_ASSERT(isSgTemplateInstantiationDecl(templateInstantiation->get_definingDeclaration())->get_templateName().is_null() == false);
                    if (isSgTemplateInstantiationDecl(templateInstantiation->get_definingDeclaration())->get_templateDeclaration() == NULL)
                       {
#ifdef DEBUG_CONVERSION
                         printf ("Can't find template declaration for templateInstantiation->get_definingDeclaration() = %p = %s \n",
                              templateInstantiation->get_definingDeclaration(),isSgTemplateInstantiationDecl(templateInstantiation->get_definingDeclaration())->get_name().str());
#endif
                         SgTemplateClassDeclaration* templateClassDeclaration = isSgTemplateInstantiationDecl(templateInstantiation->get_firstNondefiningDeclaration())->get_templateDeclaration();
                         ROSE_ASSERT(templateClassDeclaration != NULL);
                         ROSE_ASSERT(templateClassDeclaration->get_firstNondefiningDeclaration() != NULL);
                         ROSE_ASSERT(templateClassDeclaration->get_definingDeclaration() != NULL);
                         SgTemplateClassDeclaration* defining_templateClassDeclaration = isSgTemplateClassDeclaration(templateClassDeclaration->get_definingDeclaration());
                         ROSE_ASSERT(defining_templateClassDeclaration != NULL);

                      // DQ (8/25/2012): I would like to set this earlier if possible (does it make sense?).
#ifdef DEBUG_CONVERSION
                         printf ("WARNING: In convert_struct_secondary(): Setting the defining declaration's template declaration: defining_templateClassDeclaration = %p \n",defining_templateClassDeclaration);
#endif
                         isSgTemplateInstantiationDecl(templateInstantiation->get_definingDeclaration())->set_templateDeclaration(defining_templateClassDeclaration);
                       }
                    ROSE_ASSERT(isSgTemplateInstantiationDecl(templateInstantiation->get_definingDeclaration())->get_templateDeclaration() != NULL);
                  }

            // DQ (3/3/2012): Added test for matching variants.
               ROSE_ASSERT(matching_defining_and_nondefining_variants(templateDeclaration) == true);

            // DQ (8/27/2014): This is a higher bar to pass on the type in the cache.
            // ROSE_ASSERT (type_translation_cache.find(p) == type_translation_cache.end());

            // DQ (9/9/2012): Test the maps that we use internally (see function for documentation).
               testEdgRoseMaps();
             }

#if 0
          printf ("In convert_struct_secondary(): Skipped test of decl->get_parent() != NULL \n");
#endif
       // ROSE_ASSERT(decl->get_parent() != NULL);

       // DQ (11/29/2011): Adding checks...
       // ROSE_ASSERT(decl->get_firstNondefiningDeclaration() == decl);
        }

  // DQ (9/6/2012): Added assertion.
     ROSE_ASSERT(firstNondefdecl != NULL);

#if 0
     printf ("In convert_struct_secondary(): decl = %p = %s -- Calling lookup_template_class_symbol() or lookup_nontemplate_class_symbol() \n",decl,decl->class_name().c_str());
#endif

#if 0
     if (buildTemplateDeclaration == true)
        {
       // I think this may be the wrong name to use: we want to use the name with template specialization arguments.
          printf ("WARNING: I think this may be the wrong name to use: we want to use the name with template specialization arguments (should be using nameWithTemplateSpecializationArguments)\n");
          printf ("In convert_struct_secondary(): Calling lookup_template_class_symbol() using nameWithTemplateSpecializationArguments = %s \n",nameWithTemplateSpecializationArguments.str());
        }
#endif

  // DQ (12/27/2011): Added new assertion.
  // ROSE_ASSERT(buildTemplateDeclaration ? decl->get_scope()->lookup_template_class_symbol(nameWithOutTemplateArguments) != NULL : decl->get_scope()->lookup_nontemplate_class_symbol(nameWithTemplateArguments,&templateArgumentsList) != NULL );
  // ROSE_ASSERT(buildTemplateDeclaration ? decl->get_scope()->lookup_template_class_symbol(nameWithOutTemplateArguments,&templateParameterList,&templateSpecializationArgumentList)  != NULL : 
  //                                        decl->get_scope()->lookup_nontemplate_class_symbol(nameWithTemplateArguments,&templateArgumentsList) != NULL );
     ROSE_ASSERT(buildTemplateDeclaration ? decl->get_scope()->lookup_template_class_symbol(nameWithTemplateSpecializationArguments,&templateParameterList,&templateSpecializationArgumentList)  != NULL : 
                                            decl->get_scope()->lookup_nontemplate_class_symbol(nameWithTemplateArguments,&templateArgumentsList) != NULL );

  // DQ (8/27/2014): This is a higher bar to pass on the type in the cache.
  // ROSE_ASSERT (type_translation_cache.find(p) == type_translation_cache.end());

  // DQ (6/9/2013): Use new function to avoid pushing duplicate SgDeclarationStatements for each type.
  // type_declaration_map[p].push_back(decl);
     push_entry_to_type_declaration_map(p,decl);

  // DQ (8/27/2014): This is a higher bar to pass on the type in the cache.
  // Note that push_entry_to_type_declaration_map records all of the declarations 
  // and does not modify the type_translation_cache.
  // ROSE_ASSERT (type_translation_cache.find(p) == type_translation_cache.end());

  // Note that at this point the SgClassSymbol has been built and is
  // in the symbol table for the scope defined by "topScopeStack()"

     SgClassDeclaration*         classDeclaration         = isSgClassDeclaration(decl);
     SgTemplateClassDeclaration* templateClassDeclaration = isSgTemplateClassDeclaration(decl);
     if (is_unnamed(p))
        {
       // DQ (11/29/2011): This is specific to a SgClassDeclaration
       // decl->set_isUnNamed(true);

       // DQ (7/28/2012): I think this is always true, so we could simplify the code below.
          ROSE_ASSERT(classDeclaration != NULL);

          if (classDeclaration != NULL)
             {
#if 0
               printf ("In convert_struct_secondary(): Mark class declaration = %p = %s as UN-NAMED \n",decl,decl->class_name().c_str());
#endif
               classDeclaration->set_isUnNamed(true);

            // DQ (7/31/2012): Set the non-defining declaration
               if (classDeclaration->get_firstNondefiningDeclaration() != NULL)
                  {
                    isSgClassDeclaration(classDeclaration->get_firstNondefiningDeclaration())->set_isUnNamed(true);
                  }

            // DQ (7/31/2012): Set the defining declaration
               if (classDeclaration->get_definingDeclaration() != NULL)
                  {
                    isSgClassDeclaration(classDeclaration->get_definingDeclaration())->set_isUnNamed(true);
                  }
             }
        }

  // DQ (9/9/2012): Test the maps that we use internally (see function for documentation).
     testEdgRoseMaps();

     if (templateClassDeclaration != NULL) {
        // DQ (3/3/2012): Added test for matching variants.
        ROSE_ASSERT(matching_defining_and_nondefining_variants(templateClassDeclaration) == true);

        SgClassType* classType = templateClassDeclaration->get_type();

          if (template_type_translation_cache[p] == NULL)
             {
#ifdef DEBUG_CONVERSION
               printf ("In convert_struct_secondary(): Adding to template_type_translation_cache: p = %p classType = %p = %s \n",p,classType,classType->class_name().c_str());
#endif
               template_type_translation_cache[p] = classType;
             }
          ROSE_ASSERT (isSgClassType(template_type_translation_cache[p]) != NULL);

          if (template_type_translation_cache[p] != classType)
             {
               printf ("Error: failing assertion: template_type_translation_cache[p] == classType \n");
               printf ("In convert_struct_secondary(): p = %p \n",p);
               printf ("classType = %p = %s = %s \n",classType,classType->class_name().c_str(),classType->get_name().str());
               printf ("type_translation_cache[p] = %p = %s \n",template_type_translation_cache[p],template_type_translation_cache[p]->class_name().c_str());

            // DQ (12/14/2019): Added debugging code.
               SgClassType* tmp_classType = isSgClassType(template_type_translation_cache[p]);
               if (tmp_classType != NULL)
                  {
                    SgClassDeclaration* classDeclaration = isSgClassDeclaration(tmp_classType->get_declaration());
                    ROSE_ASSERT(classDeclaration != NULL);
                    printf ("classDeclaration->get_name() = %s \n",classDeclaration->get_name().str());
                  }
                 else
                  {
                    printf ("template_type_translation_cache[p] is not a SgClassType \n");
                  }
               printf ("In convert_struct_secondary(): calling sumarize_type(): p = %p \n",p);
               summarize_type(p);
             }
          ROSE_ASSERT (template_type_translation_cache[p] == classType);

     } else if (classDeclaration != NULL) {
          SgClassType* classType = classDeclaration->get_type();

       // DQ (8/12/2012): Added test.
          ROSE_ASSERT(classType != NULL);

#ifdef DEBUG_CONVERSION
          printf ("In convert_struct_secondary(): classType = %p = %s \n",classType,classType->class_name().c_str());
          bool typeIsPresentInTypeCache = (type_translation_cache.find(p) != type_translation_cache.end());
          printf ("In convert_struct_secondary(): typeIsPresentInTypeCache = %s \n",typeIsPresentInTypeCache ? "true" : "false");
#endif

       // DQ (10/10/2015): I think this should be true (debugging where it is false later).
          if (classDeclaration->get_type()->get_declaration() != classDeclaration)
             {
#if 0
            // DQ (10/10/2015): I think this is OK (the case of multiple protptype declarations would be an example).
               printf ("NOTE: classDeclaration->get_type()->get_declaration() != classDeclaration (not sure if this is always OK) \n");
#endif
             }
       // ROSE_ASSERT(classDeclaration->get_type()->get_declaration() == classDeclaration);

          if (type_translation_cache[p] == NULL)
             {
#ifdef DEBUG_CONVERSION
               printf ("In convert_struct_secondary(): Adding to type_translation_cache: p = %p classType = %p = %s \n",p,classType,classType->class_name().c_str());
#endif
               type_translation_cache[p] = classType;
             }
       // TV (09/15/2018): (ROSE-1376) this happened in CompileTests/RoseExample_tests:
       //       classType = 0x60881b70 = SgClassType = position_iterator < ForwardIteratorT  , PositionT  , template position_iterator2< ForwardIteratorT  , PositionT  >   >  
       //       type_translation_cache[p] = 0x641d2120 = SgNonrealType
          if (isSgNonrealType(type_translation_cache[p])) {
            printf ("WARNING: Found a non-real type in type_translation_cache when a class type was expected. This was only seen on CompileTests/RoseExample_tests, let it pass for now...\n");
          }
          ROSE_ASSERT (isSgClassType(type_translation_cache[p]) != NULL || isSgNonrealType(type_translation_cache[p]) != NULL);

       // DQ (8/27/2014): This is a higher bar to pass on the type in the cache.
          ROSE_ASSERT (type_translation_cache.find(p) != type_translation_cache.end());
          ROSE_ASSERT (type_translation_cache[p] != NULL);

       // DQ (1/26/2018): This is part of debugging a failed Plum Hall test.
       // TV (09/16/2018): non-real type case see (ROSE-1376)
          if (type_translation_cache[p] != classType && isSgNonrealType(type_translation_cache[p]) == NULL)
             {
               printf ("Error: failing assertion: type_translation_cache[p] == classType \n");
               printf ("In convert_struct_secondary(): p = %p \n",p);
               printf ("classType = %p = %s = %s \n",classType,classType->class_name().c_str(),classType->get_name().str());
               printf ("type_translation_cache[p] = %p = %s \n",type_translation_cache[p],type_translation_cache[p]->class_name().c_str());
               printf ("In convert_struct_secondary(): calling sumarize_type(): p = %p \n",p);
               summarize_type(p);
             }
       // TV (09/16/2018): non-real type case see (ROSE-1376)
          ROSE_ASSERT (type_translation_cache[p] == classType || isSgNonrealType(type_translation_cache[p]) != NULL);

       // SgClassDeclaration* firstNondefiningDecl = isSgClassDeclaration(isSgClassType(type_translation_cache[p])->get_declaration());

       // DQ (3/6/2012): Added test.
          ROSE_ASSERT (classType != NULL);
          ROSE_ASSERT (classType->get_declaration() != NULL);
          SgClassDeclaration* local_classDeclaration = isSgClassDeclaration(classType->get_declaration());
          ROSE_ASSERT (local_classDeclaration != NULL);
          ROSE_ASSERT (local_classDeclaration->get_firstNondefiningDeclaration() != NULL);

          ROSE_ASSERT (local_classDeclaration->get_firstNondefiningDeclaration() == local_classDeclaration);
          if (local_classDeclaration->get_firstNondefiningDeclaration() != local_classDeclaration)
             {
               printf ("ERROR: In convert_struct_secondary(): resetting the type's declaration: local_classDeclaration->get_firstNondefiningDeclaration() != local_classDeclaration: local_classDeclaration = %p local_classDeclaration->get_firstNondefiningDeclaration() = %p \n",
                    local_classDeclaration,local_classDeclaration->get_firstNondefiningDeclaration());

            // This case is not call (I think).
               ROSE_ASSERT(false);

            // DQ (3/21/2012): Reset the declaration in the class type.
               classType->set_declaration(local_classDeclaration->get_firstNondefiningDeclaration());
               local_classDeclaration = isSgClassDeclaration(local_classDeclaration->get_firstNondefiningDeclaration());
             }
          ROSE_ASSERT (local_classDeclaration->get_firstNondefiningDeclaration() == local_classDeclaration);

       // DQ (9/9/2012): Test the maps that we use internally (see function for documentation).
          testEdgRoseMaps();
        }

  // DQ (9/9/2012): Test the maps that we use internally (see function for documentation).
     testEdgRoseMaps();

  // The non-defining declaration used in the type is NOT a forward declaration ans should not be marked as such!
  // See test2009_10.C for a valid example where this assertion fails (simple class prototype with no defining declaration).
  // ROSE_ASSERT (firstNondefiningDecl->get_forward() == false);

  // DQ: This is meaningless, since we get the classType from decl->get_type()
  // decl->set_type(classType);

     ROSE_ASSERT(decl != NULL);

#if 0
     printf ("In convert_struct_secondary(): Skipped test of decl->get_parent() != NULL \n");
#endif

  // ROSE_ASSERT(decl->get_parent() != NULL);

     if (buildTemplateDeclaration == true)
        {
          ROSE_ASSERT(isSgTemplateClassDeclaration(decl) != NULL);
        }
       else
        {
          if (buildTemplateInstantiation == true)
             {
               ROSE_ASSERT(isSgTemplateInstantiationDecl(decl) != NULL);
             }
            else
             {
               ROSE_ASSERT(isSgClassDeclaration(decl) != NULL);
             }
        }

#if 0
     if (buildTemplateDeclaration == true)
        {
       // I think this may be the wrong name to use: we want to use the name with template specialization arguments.
          printf ("WARNING: I think this may be the wrong name to use: we want to use the name with template specialization arguments (should be using nameWithTemplateSpecializationArguments) \n");
          printf ("In convert_struct_secondary(): Calling lookup_template_class_symbol() using nameWithTemplateSpecializationArguments = %s \n",nameWithTemplateSpecializationArguments.str());
        }
#endif

  // DQ (12/27/2011): Added new assertion.
  // ROSE_ASSERT(buildTemplateDeclaration ? decl->get_scope()->lookup_template_class_symbol(nameWithOutTemplateArguments) != NULL : decl->get_scope()->lookup_nontemplate_class_symbol(nameWithTemplateArguments,&templateArgumentsList) != NULL );
  // ROSE_ASSERT(buildTemplateDeclaration ? decl->get_scope()->lookup_template_class_symbol(nameWithOutTemplateArguments,&templateParameterList,&templateSpecializationArgumentList) != NULL : 
  //                                        decl->get_scope()->lookup_nontemplate_class_symbol(nameWithTemplateArguments,&templateArgumentsList) != NULL );
     ROSE_ASSERT(buildTemplateDeclaration ? decl->get_scope()->lookup_template_class_symbol(nameWithTemplateSpecializationArguments,&templateParameterList,&templateSpecializationArgumentList) != NULL : 
                                            decl->get_scope()->lookup_nontemplate_class_symbol(nameWithTemplateArguments,&templateArgumentsList) != NULL );

#if DEBUG_STRUCTURAL_DESIGN || 0
     printf ("***************************************************************************************** \n");
     printf ("Leaving convert_struct_secondary(): decl = %p = %s nameWithTemplateArguments = %s summarize_type_kind_in_string_kind(p = %p p->kind) = %s \n",
          decl,decl->class_name().c_str(),nameWithTemplateArguments.str(),p,summarize_type_kind_in_string_kind(p->kind));
     printf ("   --- decl->get_definingDeclaration()         = %p \n",decl->get_definingDeclaration());
     printf ("   --- decl->get_firstNondefiningDeclaration() = %p \n",decl->get_firstNondefiningDeclaration());
     printf ("   --- decl->get_type()                        = %p = %s \n",decl->get_type(),decl->get_type()->class_name().c_str());
     SgClassType* classType = isSgClassType(decl->get_type());
     ROSE_ASSERT(classType != NULL);
     ROSE_ASSERT(classType->get_declaration() != NULL);
  // printf ("   --- classType->get_declaration()  = %p \n",classType->get_declaration());
     printf ("   --- classType->get_declaration()  = %p = %s \n",classType->get_declaration(),classType->get_declaration()->class_name().c_str());
     ROSE_ASSERT(type_translation_cache.find(p) != type_translation_cache.end());
     ROSE_ASSERT(type_translation_cache[p] != NULL);
     printf ("   --- p = %p type_translation_cache[p] = %p = %s \n",p,type_translation_cache[p],type_translation_cache[p]->class_name().c_str());
     SgClassType* class_type = isSgClassType(type_translation_cache[p]);
     ROSE_ASSERT(class_type != NULL);
     printf ("   --- class_type = %p = %s \n",class_type,class_type->class_name().c_str());
     printf ("   --- class_type->get_declaration() = %p = %s \n",class_type->get_declaration(),class_type->get_declaration()->class_name().c_str());
     SgScopeStatement* temp_scope = decl->get_scope();
     printf ("   --- decl->get_scope() = %p = %s \n",temp_scope,temp_scope->class_name().c_str());
     printf ("***************************************************************************************** \n");
#endif

  // DQ (8/27/2014): Can we assert this?
     if (templateClassDeclaration != NULL) {
       ROSE_ASSERT(template_type_translation_cache[p] == decl->get_type());
     } else {
    // TV (09/16/2018): non-real type case see (ROSE-1376)
       ROSE_ASSERT(type_translation_cache[p] == decl->get_type() || isSgNonrealType(type_translation_cache[p]));
     }

  // DQ (9/6/2012): Tests for first non-defining, defining, and secondary declarations.
  // testDefiningNonDefiningDeclarations(firstNondefdecl,firstNondefdecl->get_definingDeclaration(),decl,"testing convert_struct_secondary()");
     testDefiningNonDefiningDeclarations(firstNondefdecl->get_firstNondefiningDeclaration(),firstNondefdecl->get_definingDeclaration(),decl,"testing convert_struct_secondary() (part 1)");
     testDefiningNonDefiningDeclarations(firstNondefdecl,firstNondefdecl->get_definingDeclaration(),decl,"testing convert_struct_secondary() (part 2)");

#if 0
     decl->get_file_info()->display("At base of convert_struct_secondary(): debug");
#endif

  // DQ (4/9/2012): Test the maps that we use internally (see function for documentation).
     testEdgRoseMaps();

  // DQ (4/22/2012): Added this to capture some missing scopes that are needed later to support references to members in instantiated templates.
     a_scope_ptr needed_scope = p->variant.class_struct_union.extra_info->assoc_scope;
     if (needed_scope != NULL)
        {
       // Note that test2012_190.C is a simple case of triggering this branch.

       // It might be better to grab these now rather than waiting to later.  If this is an associates scope then we should consider
       // processing it as early as possible.  Or should we???  It appears that we process nested classes in templated classes as
       // secondary declarations which I would like to avoid where they are primate declarations (defining declarations).
#if 0
          printf ("WARNING: In convert_struct_secondary(): We may need to handle this edg scope if not handled elsewhere: needed_scope = %p \n",needed_scope);
#endif
#if 0
          printf ("In convert_struct_secondary(): calling summarize_scope() needed_scope = %p \n",needed_scope);
          summarize_scope(needed_scope);
#endif

       // DQ (8/12/2012): Could this be helpful?
          a_type_ptr associatedTypeForNeededScope = NULL;
          if (needed_scope->kind == sck_class_struct_union)
             {
            // Actually valid when kind == sck_func_prototype, sck_class_struct_union, sck_class_reactivation, or sck_enum.
               associatedTypeForNeededScope = needed_scope->variant.assoc_type;
               ROSE_ASSERT(associatedTypeForNeededScope != NULL);
             }
            else
             {
               printf ("Unclear now to process the needed_scope = %p (exiting) \n",needed_scope);
               ROSE_ASSERT(false);
             }

          ROSE_ASSERT(associatedTypeForNeededScope != NULL);

          bool buildTemplateDeclaration   = false;
          bool buildTemplateInstantiation = false;
          bool forceTemplateDeclaration   = false;
          setTemplateOrTemplateInstantiationClassGeneration(associatedTypeForNeededScope,forceTemplateDeclaration,buildTemplateDeclaration,buildTemplateInstantiation);
#if 0
          printf ("Computed values of needed scope: buildTemplateDeclaration = %s buildTemplateInstantiation = %s forceTemplateDeclaration = %s \n",
               buildTemplateDeclaration ? "true" : "false", buildTemplateInstantiation ? "true" : "false", forceTemplateDeclaration ? "true" : "false");
#endif
       // DQ (4/22/2012): I think the EDG scope has the associated functions that we want to process and that are 
       // referenced later.  I think this is a bit of new insight into how EDG handled instantiated templates...
       // XXX print out the routines in this EDG scope... XXX
#if 0
          printf ("In convert_struct_secondary(): Exiting as a test! \n");
          ROSE_ASSERT(false);
#endif
        }

  // DQ (8/11/2014): Added to support C++11 final keyword.
     if (p->variant.class_struct_union.final == true)
        {
#ifdef DEBUG_CONVERSION
          printf ("In convert_struct_secondary(): detected final keyword \n");
#endif
          decl->get_declarationModifier().setFinal();

#if 0
          printf ("Exiting as a test! \n");
          ROSE_ASSERT(false);
#endif
        }

  // DQ (4/22/2012): Test the maps that we use internally (see function for documentation).
     testEdgRoseMaps();

  // DQ (8/18/2014): Setup any possible Microsoft declspec attributes.
     if (p->source_corresp.attributes != NULL)
        {
          set_ms_attributes(decl->get_declarationModifier(),p->source_corresp.attributes);
        }

  // DQ (2/5/2015): Check and mark this as un-named.
     if (originally_unnamed == true)
        {
#if 0
          printf ("In convert_struct_secondary(): Marking class decl = %p as un-named \n",decl);
          printf ("   --- decl->get_definingDeclaration()         = %p \n",decl->get_definingDeclaration());
          printf ("   --- decl->get_firstNondefiningDeclaration() = %p \n",decl->get_firstNondefiningDeclaration());
#endif
       // DQ (2/5/2015): I would like to think that this has alread been set.
          ROSE_ASSERT(decl->get_isUnNamed() == true);

          decl->set_isUnNamed(true);

       // DQ (7/31/2012): Set the non-defining declaration
          if (decl->get_firstNondefiningDeclaration() != NULL)
             {
            // DQ (2/5/2015): I would like to think that this has alread been set.
               ROSE_ASSERT(isSgClassDeclaration(decl->get_firstNondefiningDeclaration())->get_isUnNamed() == true);

               isSgClassDeclaration(decl->get_firstNondefiningDeclaration())->set_isUnNamed(true);
             }

       // DQ (7/31/2012): Set the defining declaration
          if (decl->get_definingDeclaration() != NULL)
             {
            // DQ (2/5/2015): I would like to think that this has alread been set.
               ROSE_ASSERT(isSgClassDeclaration(decl->get_definingDeclaration())->get_isUnNamed() == true);

               isSgClassDeclaration(decl->get_definingDeclaration())->set_isUnNamed(true);
             }
        }

     return decl;
   }
