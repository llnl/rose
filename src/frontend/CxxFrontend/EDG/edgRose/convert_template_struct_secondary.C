
#include "edgRose.h"

SgDeclarationStatement* EDG_ROSE_Translation::convert_template_struct_secondary(a_type_ptr p, SgClassDeclaration::class_types t, a_template_ptr edg_template)
   {

  // DQ (9/12/2012): We now want to support using the template name with and without template specialization arguments.
  // DQ (1/2/2012): Refactoring to generate common name used in this function.
     SgName nameWithOutTemplateSpecializationArguments = generateName(p);
     SgName nameWithTemplateSpecializationArguments    = nameWithOutTemplateSpecializationArguments;

#ifdef DEBUG_CONVERSION
     indent_line(); debug_print("%s template secondary p = %p \n", (t == SgClassDeclaration::e_union ? "union" : t == SgClassDeclaration::e_struct ? "struct" : t == SgClassDeclaration::e_class ? "class" : "???"),p);
#endif

  // string name = p->source_corresp.name == NULL ? generate_name_for_type(p) : p->source_corresp.name;
     ROSE_ASSERT (topScopeStack() != NULL);

#if 0
     printf ("In convert_template_struct_secondary(): calling summarize_type() \n");
     summarize_type(p);
#endif
#if DEBUG_STRUCTURAL_DESIGN
     printf ("***** In convert_template_struct_secondary(): p = %p edg_template = %p \n",p,edg_template);
#endif

  // DQ (8/27/2014): This is a higher bar to pass on the type in the cache.
  // ROSE_ASSERT (type_translation_cache.find(p) == type_translation_cache.end());

  // DQ (4/14/2012): Added support to have the edg_template be passed so that we can set the EDG/ROSE maps ASAP.
     if (edg_template == NULL)
        {
          printf ("In convert_template_struct_secondary(): (edg_template == NULL) calling summarize_type() \n");
          summarize_type(p);
        }
     ROSE_ASSERT (edg_template != NULL);

  // DQ (11/30/2011): Some points to test upon...
  // DQ (12/10/2011): added tk_struct and tk_union options to test...
  // ROSE_ASSERT(p->kind == tk_class);
     if (p->kind != tk_class && p->kind != tk_struct && p->kind != tk_union)
        {
          printf ("In convert_template_struct_secondary(): (p->kind != tk_class && p->kind != tk_struct && p->kind != tk_union) calling summarize_type() \n");
          summarize_type(p);
        }
     ROSE_ASSERT(p->kind == tk_class || p->kind == tk_struct || p->kind == tk_union);

     ROSE_ASSERT(p->variant.class_struct_union.extra_info != NULL);
     ROSE_ASSERT(p->variant.class_struct_union.extra_info->assoc_template != NULL);

  // DQ (6/1/2014): I think it is no longer true that all template structs will be marked as a template class.
  // ROSE_ASSERT(p->variant.class_struct_union.is_template_class == true);
     if (p->variant.class_struct_union.is_template_class == false)
        {
       // This appears to only happen using the tutorial/inputCode_generatingUniqueNamesFromDeclaration.C test code.
       // But to be safe, lets output a note about this for now.
          printf ("NOTE: Found input type to convert_template_struct_secondary() marked as p->variant.class_struct_union.is_template_class == false (I don't think this is a problem) \n");
        }

#if DEBUG_GET_DECLARATION_SCOPE
     printf ("In convert_template_struct_secondary(): calling getDeclarationScope() \n");
#endif
  // The scope of a declaration (in C++) can be different from the current scope (where it is placed structurally).
     SgScopeStatement* scopeOfDeclaration = getDeclarationScope(p->source_corresp);
     ROSE_ASSERT(scopeOfDeclaration != NULL);

#if 0
     printf ("***** In convert_template_struct_secondary(): evaluate topScopeStack() = %s before call buildStructDeclaration_nfi() \n",topScopeStack()->class_name().c_str());
     printf ("***** In convert_template_struct_secondary(): using scopeOfDeclaration = %s to call buildStructDeclaration_nfi() \n",scopeOfDeclaration->class_name().c_str());
#endif

#if 0
     printf ("In convert_template_struct_secondary(): templk_class: Calling parse_template_parameters() \n");
#endif

  // DQ (9/10/2012): Setup the template parameters (required to disambiguate templates when the names are not enough). See test2012_209.C
     SgTemplateParameterPtrList templateParameterList = parse_template_parameters(p,scopeOfDeclaration,NULL);

#if 0
     printf ("DONE: In convert_template_struct_secondary(): templk_class: Calling parse_template_parameters() \n");
#endif

  // DQ (9/11/2012): Adding support for partial specialization.
     SgTemplateArgumentPtrList templateSpecializationArgumentList;
  // a_template_arg_ptr partial_spec_template_arg_list = p->variant.class_struct_union.extra_info->partial_spec_template_arg_list;
  // This is the list of template arguments on which the list of "actual" arguments is based.
     a_template_arg_ptr templateSpecializationArgs        = p->variant.class_struct_union.extra_info->partial_spec_template_arg_list;
  // This is the list of "actual" template arguments.
     a_template_arg_ptr templateSpecializationArgs_actual = p->variant.class_struct_union.extra_info->template_arg_list;
  // if (partial_spec_template_arg_list != NULL)
     if (templateSpecializationArgs != NULL)
        {
       // DQ (9/12/2012): This is tricky: If this is a partial specialization then the "actual" template arguments are in 
       // the p->variant.class_struct_union.extra_info->template_arg_list and the associated names of the template arguments 
       // are in the p->variant.class_struct_union.extra_info->partial_spec_template_arg_list list.  This is because the names
       // need not match that of where they are defined in an outer template.
          ROSE_ASSERT(templateSpecializationArgs_actual != NULL);
       // templateSpecializationArgumentList = parse_template_arguments(partial_spec_template_arg_list,scopeOfDeclaration);
#if 0
          printf ("In convert_template_struct_secondary(): calling parse_template_arguments() \n");
#endif
#if 0
       // DQ (2/11/2019): Adding support to track the min_template_arguments.
          a_class_type_supplement_ptr classTypeSupplement = p->variant.class_struct_union.extra_info;
          ROSE_ASSERT((p->kind == tk_class || p->kind == tk_struct || p->kind == tk_union) && classTypeSupplement != NULL);
          printf ("In convert_template_struct_secondary(): classTypeSupplement->min_template_arguments = %d \n",classTypeSupplement->min_template_arguments);
#endif
          templateSpecializationArgumentList = parse_template_arguments(templateSpecializationArgs_actual,scopeOfDeclaration);
        }
     
  // DQ (9/12/2012): Adding support for template specialization.
     nameWithTemplateSpecializationArguments = appendTemplateArgumentsToName(nameWithOutTemplateSpecializationArguments,templateSpecializationArgumentList);

#if DEBUG_CLASSES_FROM_TEMPLATE_PARAMETERS
     printf ("In convert_template_struct_secondary(): nameWithOutTemplateSpecializationArguments = %s \n",nameWithOutTemplateSpecializationArguments.str());
     printf ("In convert_template_struct_secondary(): nameWithTemplateSpecializationArguments    = %s \n",nameWithTemplateSpecializationArguments.str());
#endif

  // DQ (9/12/2012): Adding support for template specialization.
  // SgTemplateClassDeclaration* decl = NULL;
  // SgTemplateClassDeclaration* decl = set_file_info_for_corresp(buildTemplateClassDeclaration_nfi(name, t, scopeOfDeclaration, nonDefiningDecl), p->source_corresp);
  // SgTemplateClassDeclaration* decl = set_file_info_for_corresp(buildTemplateClassDeclaration_nfi(name, t, scopeOfDeclaration, NULL), p->source_corresp);
  // SgTemplateClassDeclaration* decl = set_file_info_for_corresp(buildNondefiningTemplateClassDeclaration_nfi(name, t, scopeOfDeclaration), p->source_corresp);
  // SgTemplateClassDeclaration* decl = set_file_info_for_corresp(buildNondefiningTemplateClassDeclaration_nfi(name, t, scopeOfDeclaration, &templateParameterList), p->source_corresp);
  // SgTemplateClassDeclaration* decl = set_file_info_for_corresp(buildNondefiningTemplateClassDeclaration_nfi(name, t, scopeOfDeclaration, &templateParameterList, &templateSpecializationArgumentList), p->source_corresp);
     SgTemplateClassDeclaration* decl = set_file_info_for_corresp(buildNondefiningTemplateClassDeclaration_nfi(nameWithOutTemplateSpecializationArguments, t, scopeOfDeclaration, &templateParameterList, &templateSpecializationArgumentList), p->source_corresp);

     ROSE_ASSERT(decl != NULL);

  // DQ (8/27/2014): Added test on associated type.
     ROSE_ASSERT(decl->get_type() != NULL);

  // DQ (8/27/2014): Why is this not a SgTemplateType (because SgTemplateType are types reserved 
  // for template parameters, not template declarations IR nodes (which should be a SgClassType)).
  // ROSE_ASSERT(isSgTemplateType(decl->get_type()) != NULL);
     ROSE_ASSERT(isSgClassType(decl->get_type()) != NULL);

     for (SgTemplateParameterPtrList::iterator it = templateParameterList.begin(); it != templateParameterList.end(); it++) {
       if ((*it)->get_parent() != NULL) {
         SgDeclarationStatement * declstmt_parent = isSgDeclarationStatement((*it)->get_parent());
         ROSE_ASSERT(declstmt_parent != NULL);
         ROSE_ASSERT(declstmt_parent->get_firstNondefiningDeclaration() == decl->get_firstNondefiningDeclaration());
       } else {
         (*it)->set_parent(decl);
       }
     }


  // DQ (9/6/2012): Added to support testing the first non-defining, defining, and secondary declarations below.
     SgDeclarationStatement* firstNondefdecl = decl->get_firstNondefiningDeclaration();

  // DQ (7/19/2017): I think this is true because we would have processed the template parameters before 
  // building the first nondefining declaration, well before processing the defining declaration.  However,
  // if there are multiple forward declarations then the first one would process the template paramters 
  // and it would be false for the second prototype declaration.  Both C and C++ permit there to be an 
  // arbitrary number of prototype declarations.
  // ROSE_ASSERT(firstNondefdecl->get_definingDeclaration() == NULL);

  // DQ (7/18/2017): Adding support for class declarations built from template parameters.  The scope for 
  // these must be the SgDeclarationScope that is contained in the nondefining template declaration.
     setupDeclarationScope(firstNondefdecl);

  // DQ (7/15/2012): Added testing specific to autonomous declearations for templates.
     ROSE_ASSERT(decl != NULL);
     ROSE_ASSERT(decl->get_parent() == NULL);
     if (p->autonomous_primary_tag_decl == false)
        {
          if (decl->get_parent() != NULL)
             {
               printf ("decl = %p = %s decl->get_parent() = %p = %s \n",decl,decl->class_name().c_str(),decl->get_parent(),decl->get_parent()->class_name().c_str());
             }
          ROSE_ASSERT(decl->get_parent() == NULL);
        }
       else
        {
       // DQ (7/15/2012): fails for test2004_26.C
       // ROSE_ASSERT(decl->get_parent() != NULL);
          ROSE_ASSERT(decl->get_parent() == NULL);
        }

  // DQ (12/4/2011): We need a concept for type for SgTemplateClassDeclaration so that we can construct SgMemberFunctionType nodes for template member functions.
  // We use a SgClassType which has been fixed to permit it to hold either a SgClassDeclaration or an SgTemplateClassDeclaration.
     ROSE_ASSERT(decl->get_type() != NULL);

     decl->set_forward(false);

#if 0
     printf ("In convert_template_struct_secondary(): buildClassDeclaration_nfi() returned decl = %p \n",decl);
     printf ("In convert_template_struct_secondary(): decl->get_firstNondefiningDeclaration()   = %p \n",decl->get_firstNondefiningDeclaration());
     printf ("In convert_template_struct_secondary(): decl->get_definingDeclaration()           = %p \n",decl->get_definingDeclaration());
#endif

  // This may or may not be true, I think; but it should be non-null.
  // ROSE_ASSERT(decl->get_firstNondefiningDeclaration() != decl);
     ROSE_ASSERT(decl->get_firstNondefiningDeclaration() != NULL);

#if 0
     decl->get_firstNondefiningDeclaration()->get_file_info()->display("In convert_template_struct_secondary(): debug");
#endif

     ROSE_ASSERT(decl->get_scope() != NULL);
  // ROSE_ASSERT(decl->get_type() != NULL);

  // There may already be a secondary (non-defining) declaration!
  // type_declaration_map[p].push_back(decl);

  // Note that at this point the SgClassSymbol has been built and is
  // in the symbol table for the scope defined by "topScopeStack()"

#if 0
  // DQ (3/22/2012): This is moved to allow both defining and nondefining to be set uniformally.
     if (is_unnamed(p))
        {
          decl->set_isUnNamed(true);
        }
#endif

  // SgClassDeclaration* firstNondefiningDecl = NULL; // isSgClassDeclaration(classType->get_declaration());
  // SgClassSymbol* mysymbol = scopeOfDeclaration->lookup_class_symbol(name);
     SgTemplateClassDeclaration* firstNondefiningDecl = NULL; // isSgClassDeclaration(classType->get_declaration());

#if 1
  // DQ (12/26/2012): We are now using a new SgTemplateClass to represent symbols for SgTemplateClassDeclaration.
     bool buildTemplateDeclaration = true;

#if 0
     printf ("In convert_template_struct_secondary(): (part 1) Calling retrieveAssociatedClassSymbol(nameWithOutTemplateSpecializationArguments = %s, buildTemplateDeclaration = %s) \n",
          nameWithOutTemplateSpecializationArguments.str(),buildTemplateDeclaration ? "true" : "false");
#endif

  // DQ (9/12/2012): Adding support for template specialization.
  // DQ (9/10/2012): The template parameter list is required to disambiguate template names that are the same.
  // SgSymbol* tmp_symbol = retrieveAssociatedClassSymbol(name,scopeOfDeclaration,buildTemplateDeclaration,NULL);
  // SgSymbol* tmp_symbol = retrieveAssociatedClassSymbol(nameWithTemplateSpecializationArguments,scopeOfDeclaration,buildTemplateDeclaration,NULL,&templateParameterList,&templateSpecializationArgumentList);
     SgSymbol* tmp_symbol = retrieveAssociatedClassSymbol(nameWithOutTemplateSpecializationArguments,scopeOfDeclaration,buildTemplateDeclaration,NULL,&templateParameterList,&templateSpecializationArgumentList);
     ROSE_ASSERT(tmp_symbol != NULL);

     SgTemplateClassSymbol* mysymbol = isSgTemplateClassSymbol(tmp_symbol);
#else
  // DQ (12/26/2012): This is older code now that we are now using a new SgTemplateClass to represent symbols for SgTemplateClassDeclaration.
  // SgTemplateSymbol* mysymbol = scopeOfDeclaration->lookup_template_symbol(name);
     SgTemplateSymbol* mysymbol = scopeOfDeclaration->lookup_template_symbol(name,NULL,NULL);
#endif

     ROSE_ASSERT(mysymbol != NULL);

#if 0
     printf ("In convert_template_struct_secondary(): mysymbol = %p \n",mysymbol);
#endif

     if (mysymbol != NULL)
        {
          firstNondefiningDecl = isSgTemplateClassDeclaration(mysymbol->get_declaration());

          ROSE_ASSERT(firstNondefiningDecl != NULL);

#if 0
          printf ("In convert_template_struct_secondary(): (part 1) Skipping test for valid parent pointer. \n");
#endif
       // ROSE_ASSERT(firstNondefiningDecl->get_parent() != NULL);

       // firstNondefiningDecl->set_definingDeclaration(decl);

       // If this is a forward declaration, not associated with the original 
       // first-nondefining declaration then point to the original non-defining 
       // declaration for consistancy!
          decl->set_firstNondefiningDeclaration(firstNondefiningDecl);

       // This might only be setting the definin declaration to NULL, if we have not see the defining declaration yet!
          decl->set_definingDeclaration(firstNondefiningDecl->get_definingDeclaration());

       // ROSE_ASSERT(firstNondefiningDecl->get_definingDeclaration() == decl);

       // This may or may not be true, I think; but it should be non-null.
       // ROSE_ASSERT(firstNondefiningDecl->get_firstNondefiningDeclaration() != decl);
          ROSE_ASSERT(firstNondefiningDecl->get_firstNondefiningDeclaration() != NULL);

#if 0
       // Set the type map, even though we would like to use the symbol table as a reference in the future.
          if (type_translation_cache[p] == NULL)
               type_translation_cache[p] = firstNondefiningDecl->get_type();

#error "DEAD CODE!"

       // Make sure that types are shared between defining and non-defining declarations
          ROSE_ASSERT(decl->get_type() != NULL);
          ROSE_ASSERT(firstNondefiningDecl->get_type() != NULL);
          if (firstNondefiningDecl->get_type() != decl->get_type())
             {
               printf ("Fixing up class types to be shared between the defining and non-defining declaration \n");
            // delete decl->get_type();
               decl->set_type(firstNondefiningDecl->get_type());
             }
          ROSE_ASSERT(firstNondefiningDecl->get_type() == decl->get_type());
#else

       // DQ (8/27/2014): This is a higher bar to pass on the type in the cache.
       // ROSE_ASSERT (type_translation_cache.find(p) == type_translation_cache.end());

       // DQ (12/28/2011): Fixed type ("==" was "!=") I think this is a proper fix.
       // DQ (12/21/2011): Make sure that the type associated with this class declaration is in the type_translation_cache.
       // if (type_translation_cache.find(p) != type_translation_cache.end())
          if (template_type_translation_cache.find(p) == template_type_translation_cache.end())
             {
               ROSE_ASSERT(decl->get_type() != NULL);
#if 0
               printf ("In convert_template_struct_secondary(): add entry for p = %p to type_translation_cache: decl->get_type() = %p = %s \n",p,decl->get_type(),decl->get_type()->class_name().c_str());
#endif
#if 0
               printf ("In convert_template_struct_secondary(): skip putting the types associated with template declarations into the type_translation_cache \n");
#endif
            // DQ (8/27/2014): Note that types associated with templates are not real type yet because they 
            // have not been instantiated into classes, functions, etc.  So we need to store a seperate map 
            // for these types.  They are given a type in ROSE that is a SgClassType (for the case of template 
            // class declarations), and they also use the same EDG a_type_ptr value.  So having two seperate 
            // maps allows us to store the map to the ROSE types from the EDG type pointers and not have to 
            // interfer with the types build for non-template classes (using the same key).
#if 0
               printf ("In convert_template_struct_secondary(): Adding to template_type_translation_cache: p = %p decl->get_type() = %p = %s \n",p,decl->get_type(),decl->get_type()->class_name().c_str());
#endif
            // type_translation_cache[p] = decl->get_type();
               template_type_translation_cache[p] = decl->get_type();

            // DQ (8/27/2014): This is a higher bar to pass on the type in the cache.
            // ROSE_ASSERT (type_translation_cache.find(p) == type_translation_cache.end());

             }
       // ROSE_ASSERT(type_translation_cache.find(p) != type_translation_cache.end());
          ROSE_ASSERT(template_type_translation_cache.find(p) != template_type_translation_cache.end());
          ROSE_ASSERT(template_type_translation_cache[p] != NULL);
#if 0
       // DQ (8/27/2014): This code is redundant with the code above, since we test now for a NULL pointer entry (so commented out).

       // DQ (2/25/2012): Add the declaration directly if it is not yet in place.
          if (type_translation_cache[p] == NULL)
             {
            // We don't really want to have to fixup entries in the type_translation_cache like this, but I think it is required sometimes.
            // We want to later check that this is not symtomatic of a larger problem.  Hence the message that we want to have be output.
            // See test2012_08.C for where this is failing (before this fix).
               printf ("WARNING: This type is in the list as a NULL pointer (setting it to be a valid pointer), \n");

               ROSE_ASSERT(decl->get_type() != NULL);
               type_translation_cache[p] = decl->get_type();
             }
          ROSE_ASSERT(type_translation_cache[p] != NULL);
#endif
#endif
        }
       else
        {
          printf ("There should have been a symbol! \n");
          ROSE_ASSERT(false);
        }

     ROSE_ASSERT (firstNondefiningDecl != NULL);

  // DQ (3/22/2012): Set these together to be uniform.
     if (is_unnamed(p))
        {
#if 0
          printf ("In convert_template_struct_secondary(): Mark class declaration = %p = %s as UN-NAMED \n",decl,decl->class_name().c_str());
#endif
          decl->set_isUnNamed(true);
          firstNondefiningDecl->set_isUnNamed(true);

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

#if 0
     printf ("decl                                            = %p \n",decl);
     printf ("firstNondefiningDecl->get_definingDeclaration() = %p \n",firstNondefiningDecl->get_definingDeclaration());
#endif
     ROSE_ASSERT(firstNondefiningDecl->get_firstNondefiningDeclaration() != NULL);

#if 0
     printf ("In convert_template_struct_secondary(): (part 2) Skipping test for valid parent pointer. \n");
#endif
     ROSE_ASSERT(decl->get_firstNondefiningDeclaration() != NULL);

#if 0
     printf ("In convert_template_struct_secondary(): (part 3) Skipping test for valid parent pointer. \n");
#endif

  // ROSE_ASSERT(decl->get_parent() != NULL);

#if 0
  // DQ (9/10/2012): Template parameter lists are now setup as part of the template declaration as it is constrcuted (handled to the SageBuilder API.
  // Add Template parameters to the templateDeclstatement (could not be added when the SgTemplateDeclaration was first built)
     ROSE_ASSERT(p->variant.class_struct_union.extra_info != NULL);
     if (p->variant.class_struct_union.extra_info->assoc_template != NULL)
        {
#if 0
          printf ("Setting template parameters: found assoc_template = %p \n",p->variant.class_struct_union.extra_info->assoc_template);
#endif
       // DQ (2/11/2012): Check if we have a valid template_decl and only build template parameter list if so.
       // See test2012_02.C for a case where this is important (also in STL example test2004_35.C).
       // ROSE_ASSERT(p->variant.class_struct_union.extra_info->assoc_template->template_decl != NULL);
          if (p->variant.class_struct_union.extra_info->assoc_template->template_decl != NULL)
             {
               a_template_parameter_ptr templateParameter = p->variant.class_struct_union.extra_info->assoc_template->template_decl->param_list;
               if (templateParameter != NULL)
                  {
                    printf ("Setting template parameters: found valid template parameter list = %p \n",templateParameter);
                    SgTemplateParameterPtrList templateParameterList = parse_template_parameters(templateParameter,decl);
                    ROSE_ASSERT(decl != NULL);
                 // decl->set_templateParameters(templateParameterList);
                    decl->get_templateParameters() = templateParameterList;
                  }
             }
        }
#endif

  // DQ (4/14/2012): Put the template into the EDG/ROSE map.
  // Note that p->variant.class_struct_union.extra_info->assoc_template is the primary template
  // and that edg_template is the secondary template.
     a_template_ptr primary_edg_template = p->variant.class_struct_union.extra_info->assoc_template;
     ROSE_ASSERT(primary_edg_template != NULL);

  // Make sure the primary template is present.
     if (!hasFirstNondefDeclForTemplate(primary_edg_template))
        {
          setFirstNondefDeclForTemplate(primary_edg_template, decl->get_firstNondefiningDeclaration());
        }
     ROSE_ASSERT(hasFirstNondefDeclForTemplate(primary_edg_template));

  // Make sure the secondary template is NOT present, but add it.
     if (!hasFirstNondefDeclForTemplate(edg_template))
        {
          setFirstNondefDeclForTemplate(edg_template, decl->get_firstNondefiningDeclaration());
        }
     ROSE_ASSERT(hasFirstNondefDeclForTemplate(edg_template));

  // DQ (8/27/2014): Modified to use the new template_type_translation_cache instead of the previous type_translation_cache.
  // DQ (12/21/2011): Make sure that the type associated with this class declaration is in the type_translation_cache.
     ROSE_ASSERT(template_type_translation_cache.find(p) != template_type_translation_cache.end());
     ROSE_ASSERT(template_type_translation_cache[p] != NULL);

  // DQ (3/3/2012): Added test for matching variants.
     ROSE_ASSERT(matching_defining_and_nondefining_variants(decl) == true);

#if 0
     printf ("edg_template = %p p->variant.class_struct_union.extra_info->assoc_template = %p \n",edg_template,p->variant.class_struct_union.extra_info->assoc_template);
#endif

  // ROSE_ASSERT(p->variant.class_struct_union.extra_info->assoc_template == edg_template);

#if DEBUG_STRUCTURAL_DESIGN || DEBUG_CLASSES_FROM_TEMPLATE_PARAMETERS || 0
     printf ("********************************************************************************** \n");
     printf ("Leaving convert_template_struct_secondary(): decl = %p = %s nameWithTemplateSpecializationArguments = %s summarize_type_kind_in_string_kind(p = %p p->kind) = %s \n",
          decl,decl->class_name().c_str(),nameWithTemplateSpecializationArguments.str(),p,summarize_type_kind_in_string_kind(p->kind));
     printf ("********************************************************************************** \n");
#endif

  // DQ (9/6/2012): Added support for testing first non-defining, defining, and secondary declarations.
     testDefiningNonDefiningDeclarations(firstNondefdecl,firstNondefdecl->get_definingDeclaration(),decl,"convert_template_struct_secondary()");

  // DQ (4/24/2012): Added test.
     ROSE_ASSERT(hasFirstNondefDeclForTemplate(edg_template));

  // DQ (7/14/2012): Adding test on parent (since this is a reference for which we have to build a declaration, we don't know where the declaration should be placed structurally).
     ROSE_ASSERT(decl->get_parent() == NULL);

#if 0
     decl->get_startOfConstruct()->display("Leaving convert_template_struct_secondary(): start: debug");
     decl->get_endOfConstruct  ()->display("Leaving convert_template_struct_secondary(): end: debug");
#endif

#if 0
  // DQ (9/9/2012): It appears that the template parameter list is not set properly (frequently zero, at least).
     printf ("In convert_template_struct_secondary(): decl->get_templateParameters().size() = %" PRIuPTR " \n",decl->get_templateParameters().size());
#endif

  // DQ (8/11/2014): Added to support C++11 final keyword.
     if (p->variant.class_struct_union.final == true)
        {
#ifdef DEBUG_CONVERSION
          printf ("In convert_template_struct_secondary(): detected final keyword \n");
#endif
          decl->get_declarationModifier().setFinal();
        }

     return decl;
   }
