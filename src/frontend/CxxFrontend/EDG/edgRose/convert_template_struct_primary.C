
#include "edgRose.h"

SgTemplateClassDeclaration* EDG_ROSE_Translation::convert_template_struct_primary(a_type_ptr p, SgClassDeclaration::class_types t, SgTemplateClassDeclaration* nonDefiningDecl, a_template_ptr edg_template)
   {
     ROSE_ASSERT(p != NULL);

  // DQ (1/2/2012): Refactoring to generate common name used in this function.
  // SgName name = e->source_corresp.name;
  // SgName name = p->source_corresp.name == NULL ? generate_name_for_type(p) : p->source_corresp.name;
  // SgName name = generateName(p);
     SgName nameWithOutTemplateSpecializationArguments = generateName(p);
  // DQ (3/2/2019): If this is un-named then don't allow a name to be used.
  // TV (5/20/19): Copied from convert_struct_secondary to fix ROSE-2008
     if ( p->variant.class_struct_union.originally_unnamed == true )
        {
          nameWithOutTemplateSpecializationArguments = generate_name_for_type(p);
        }
     SgName nameWithTemplateSpecializationArguments    = nameWithOutTemplateSpecializationArguments;

#ifdef DEBUG_CONVERSION
     indent_line(); debug_print("%s template primary p = %p \n", (t == SgClassDeclaration::e_union ? "union" : t == SgClassDeclaration::e_struct ? "struct" : t == SgClassDeclaration::e_class ? "class" : "???"),p);
#endif

  // string name = p->source_corresp.name == NULL ? generate_name_for_type(p) : p->source_corresp.name;

     ROSE_ASSERT (topScopeStack() != NULL);

#if DEBUG_STRUCTURAL_DESIGN || 0
     printf ("******************************************************************** \n");
     printf ("In convert_template_struct_primary(): nonDefiningDecl = %p = %s \n",nonDefiningDecl,nonDefiningDecl != NULL ? nonDefiningDecl->class_name().c_str() : "(not set)");
     printf ("******************************************************************** \n");
#endif

#if 0
     printf("In convert_template_struct_primary(p = %p): \n", p);
#endif

  // DQ (8/26/2012): commented out to permit better template support (non-real classes).
  // DQ (4/8/2012): Added support to have the edg_template be passed so that we can set the EDG/ROSE maps ASAP.
  // ROSE_ASSERT (edg_template != NULL);

  // DQ (8/26/2012): We now allow this to be NULL to permit processing of non-real classes for template declaration support.
     if (edg_template == NULL)
        {
          mprintf ("STILL NEED TO --- MARK THIS CLASS AS A MINIMAL INTERFACE CLASS USED! \n");
          ROSE_ASSERT(p->variant.class_struct_union.is_nonreal_class == true);
          ROSE_ASSERT(nonDefiningDecl != NULL);
       // ROSE_ASSERT(nonDefiningDecl->get_isRepresentingTemplateParameterInTemplateDeclaration() == true);
          if (nonDefiningDecl->get_isRepresentingTemplateParameterInTemplateDeclaration() == false)
             {
            // DQ (10/11/2015): Usually this is bacause this is a class that is not an explicit template defined inside of a template class declaration (see test2015_89.C).
               mprintf ("WARNING: Unclear why the non-defining declaration is not marked as isRepresentingTemplateParameterInTemplateDeclaration == true nonDefiningDecl = %p = %s \n",nonDefiningDecl,nonDefiningDecl->class_name().c_str());
             }
       // TV (05/03/2018): if `edg_template` is not provided we use the associated template of the class/struct/union
          ROSE_ASSERT(p->kind == tk_class || p->kind == tk_struct || p->kind == tk_union);
          ROSE_ASSERT(p->variant.class_struct_union.extra_info != NULL);
          ROSE_ASSERT(p->variant.class_struct_union.extra_info->assoc_template != NULL);
          edg_template = p->variant.class_struct_union.extra_info->assoc_template;
        }

//     else
        {
          ROSE_ASSERT (edg_template != NULL);
#if 0
       // DQ (9/7/2014): Added optional debugging support.
          printf ("In convert_template_struct_primary(): calling summarize_template() \n");
          summarize_template(edg_template);
#endif
       // DQ (4/21/2012): Check to make sure this template has not already been built.
          if (hasFirstNondefDeclForTemplate(edg_template))
             {
               SgDeclarationStatement* declaration = getFirstNondefDeclForTemplate(edg_template);
               ROSE_ASSERT(declaration != NULL);
#if 0
               printf ("This class has already been build as a nondefining declaration (might be OK). edg_template = %p ROSE template declaration = %p = %s nonDefiningDecl = %p \n",edg_template,declaration,declaration->class_name().c_str(),nonDefiningDecl);
#endif
               if (nonDefiningDecl == NULL)
                  {
#if 0
                    printf ("Setting the nonDefiningDecl (not set in calling parameters) to declaration = %p = %s \n",declaration,declaration->class_name().c_str());
#endif
                 // nonDefiningDecl = isSgTemplateClassDeclaration(declaration);

                    ROSE_ASSERT(declaration->get_firstNondefiningDeclaration() != NULL);
                    nonDefiningDecl = isSgTemplateClassDeclaration(declaration->get_firstNondefiningDeclaration());

                 // DQ (4/24/2012): We should not have a defining declaration in the first_nondefining_declarations_for_templates map.
                    ROSE_ASSERT(declaration != declaration->get_definingDeclaration());

                    ROSE_ASSERT(nonDefiningDecl != NULL);
                  }
             }
       // ROSE_ASSERT(first_nondefining_declarations_for_templates.find(edg_template) == first_nondefining_declarations_for_templates.end());
        }

#if 0
  // DQ (9/10/2012): This can be output in lower level functions making it a bit redundant here.
     printf ("In convert_template_struct_primary(): calling summarize_type() \n");
     summarize_type(p);
#endif

  // DQ (11/29/2011): Added checks...
     if (nonDefiningDecl != NULL)
        {
          if (nonDefiningDecl->get_firstNondefiningDeclaration() != nonDefiningDecl)
             {
               printf ("nonDefiningDecl = %p = %s \n",nonDefiningDecl,nonDefiningDecl->class_name().c_str());
               ROSE_ASSERT(nonDefiningDecl->get_firstNondefiningDeclaration() != NULL);
               printf ("nonDefiningDecl->get_firstNondefiningDeclaration() = %p = %s \n",nonDefiningDecl->get_firstNondefiningDeclaration(),nonDefiningDecl->get_firstNondefiningDeclaration()->class_name().c_str());

            // DQ (9/8/2012): I think that the first non defining declaration and the nonDefiningDecl were not constructed at the same time.
               printf ("WARNING: (commented out assertion) nonDefiningDecl->get_firstNondefiningDeclaration() = %p != nonDefiningDecl = %p \n",nonDefiningDecl->get_firstNondefiningDeclaration(),nonDefiningDecl);
             }

       // DQ (9/8/2012): commented out this assertion (I think it is overly conservative).  This fails for test2005_142.C.
       // I think this is not fixed in the function calling this function.
          ROSE_ASSERT(nonDefiningDecl->get_firstNondefiningDeclaration() == nonDefiningDecl);
        }
       else
        {
#if 0
          printf ("Input SgTemplateClassDeclaration* nonDefiningDecl == NULL \n");
#endif
        }

#if 0
     printf ("***** In convert_template_struct_primary(): evaluate topScopeStack() = %s before call buildStructDeclaration_nfi() \n",topScopeStack()->class_name().c_str());
#endif
#if DEBUG_GET_DECLARATION_SCOPE
     printf ("In convert_template_struct_primary(): calling getDeclarationScope() \n");
#endif
  // The scope of a declaration (in C++) can be different from the current scope (where it is placed structurally).
     SgScopeStatement* scopeOfDeclaration = getDeclarationScope(p->source_corresp);
     ROSE_ASSERT(scopeOfDeclaration != NULL);

#if 0
     printf ("***** In convert_template_struct_primary(): evaluate topScopeStack() = %s before call buildStructDeclaration_nfi() \n",topScopeStack()->class_name().c_str());
     printf ("***** In convert_template_struct_primary(): using scopeOfDeclaration = %s to call buildStructDeclaration_nfi() \n",scopeOfDeclaration->class_name().c_str());
     printf ("***** In convert_template_struct_primary(): calling parse_template_parameters \n");
#endif

  // DQ (5/13/2012): Can we assert this! NO!
  // ROSE_ASSERT(nonDefiningDecl != NULL);

#if 0
     printf ("In convert_template_struct_primary(): templk_class: Calling parse_template_parameters() \n");
#endif

  // DQ (9/10/2012): Setup the template parameters (required to disambiguate templates when the names are not enough). See test2012_209.C
  // TV (05/03/2018): Use the `edg_template` argument instead of `p`. Calling `parse_template_parameters` on a type causes to use its `assoc_template` which would be the first non-defining 
  // TV (05/03/2018): any class declared in a template class is a template class (whether or not it has his own template header)
     SgTemplateParameterPtrList templateParameterList;
     if (edg_template->template_decl != NULL) 
        {
          templateParameterList = parse_template_parameters(edg_template->template_decl->param_list,NULL,NULL);
        }

#if 0
     printf ("DONE: In convert_template_struct_primary(): templk_class: Calling parse_template_parameters() \n");
#endif

  // DQ (9/10/2012): Test the just built template parameters.
     if (templateParameterList.size() == 0)
        {
#ifdef DEBUG_CONVERSION
          printf ("WARNING: templateParameterList is empty for p = %p \n",p);
#endif
#if 0
       // DQ (9/13/2012): We need to know why this is an empty parameter list (likely OK).
          printf ("In convert_template_struct_primary(): templateParameterList.size() == 0: calling summarize_type() \n");
          summarize_type(p);
#endif
        }
  // ROSE_ASSERT(templateParameterList.size() > 0);

  // DQ (9/10/2012): Setup the template specialization arguments (these also have to be passed to the SageBuilder API (since they are also needed to disambiguate template names).
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
          printf ("In convert_template_struct_secondary(): calling parse_template_arguments() \n");
#endif
#if 0
       // DQ (2/11/2019): Adding support to track the min_template_arguments.
          a_class_type_supplement_ptr classTypeSupplement = p->variant.class_struct_union.extra_info;
          ROSE_ASSERT((p->kind == tk_class || p->kind == tk_struct || p->kind == tk_union) && classTypeSupplement != NULL);
          printf ("In convert_template_struct_secondary(): classTypeSupplement->min_template_arguments = %d \n",classTypeSupplement->min_template_arguments);
#endif
          templateSpecializationArgumentList = parse_template_arguments(templateSpecializationArgs_actual,scopeOfDeclaration);
#if 0
          printf ("In convert_template_struct_primary(): templateSpecializationArgs != NULL: templateParameterList.size()              = %" PRIuPTR " \n",templateParameterList.size());
          printf ("In convert_template_struct_primary(): templateSpecializationArgs != NULL: templateSpecializationArgumentList.size() = %" PRIuPTR " \n",templateSpecializationArgumentList.size());
          for (size_t i = 0; i < templateSpecializationArgumentList.size(); i++) {
            if (templateSpecializationArgumentList[i]->get_argumentType() == SgTemplateArgument::start_of_pack_expansion_argument) {
              printf ("  -- templateSpecializationArgumentList[%zd] = \"start_of_pack_expansion_argument\" \n", i);
            } else {
              printf ("  -- templateSpecializationArgumentList[%zd] = %s \n", i, templateSpecializationArgumentList[i]->unparseToString().c_str());
            }
          }
#endif
        }
#if 0
     printf ("In convert_template_struct_primary(): templateParameterList.size()              = %" PRIuPTR " \n",templateParameterList.size());
     printf ("In convert_template_struct_primary(): templateSpecializationArgumentList.size() = %" PRIuPTR " \n",templateSpecializationArgumentList.size());
#endif

  // DQ (9/12/2012): Adding support for template specialization.
     nameWithTemplateSpecializationArguments = appendTemplateArgumentsToName(nameWithOutTemplateSpecializationArguments,templateSpecializationArgumentList);

#if DEBUG_CLASSES_FROM_TEMPLATE_PARAMETERS
     printf ("In convert_template_struct_primary(): nameWithOutTemplateSpecializationArguments = %s \n",nameWithOutTemplateSpecializationArguments.str());
     printf ("In convert_template_struct_primary(): nameWithTemplateSpecializationArguments    = %s \n",nameWithTemplateSpecializationArguments.str());
#endif

     if (templateSpecializationArgs != NULL)
        {
#if 0
          printf ("This is the case of partial template specialization \n");
#endif
#if 0
          printf ("Exiting as a test! \n");
          ROSE_ASSERT(false);
#endif
        }

#if 0
     printf (" - scopeOfDeclaration = %p (%s) \n",scopeOfDeclaration,scopeOfDeclaration->class_name().c_str());
#endif

  // DQ (9/12/2012): Adding support for template specialization. 
  // SgTemplateClassDeclaration* decl = NULL; 
  // SgTemplateClassDeclaration* decl = set_file_info_for_corresp(buildTemplateClassDeclaration_nfi(name, t, scopeOfDeclaration, nonDefiningDecl), p->source_corresp);
  // SgTemplateClassDeclaration* decl = set_file_info_for_corresp(buildTemplateClassDeclaration_nfi(name, t, scopeOfDeclaration, nonDefiningDecl, &templateParameterList, &templateSpecializationArgumentList), p->source_corresp);
     SgTemplateClassDeclaration* decl = set_file_info_for_corresp(buildTemplateClassDeclaration_nfi(nameWithOutTemplateSpecializationArguments, t, scopeOfDeclaration, nonDefiningDecl, &templateParameterList, &templateSpecializationArgumentList), p->source_corresp);
     ROSE_ASSERT(decl != NULL);

#if 0
     printf ("DONE: Calling buildTemplateClassDeclaration_nfi() scopeOfDeclaration = %p = %s \n",scopeOfDeclaration,scopeOfDeclaration->class_name().c_str());
#endif

#if 0
     printf ("In convert_template_struct_primary(): decl->get_templateParameters().size()              = %" PRIuPTR " \n",decl->get_templateParameters().size());
     printf ("In convert_template_struct_primary(): decl->get_templateSpecializationArguments().size() = %" PRIuPTR " \n",decl->get_templateSpecializationArguments().size());
#endif

  // DQ (5/13/2012): I think this is true now! And that the source position information was changed by the buildTemplateClassDeclaration_nfi().
     ROSE_ASSERT(decl->get_firstNondefiningDeclaration() != NULL);

     for (SgTemplateParameterPtrList::iterator it = templateParameterList.begin(); it != templateParameterList.end(); it++) {
       if ((*it)->get_parent() != NULL) {
         SgDeclarationStatement * declstmt_parent = isSgDeclarationStatement((*it)->get_parent());
         ROSE_ASSERT(declstmt_parent != NULL);
      // TV (09/15/2018): FIXME fails in CompileTests/mergeAST_tests/mergeTest_04.C
      // ROSE_ASSERT(declstmt_parent->get_firstNondefiningDeclaration() == decl->get_firstNondefiningDeclaration());
       } else {
         (*it)->set_parent(decl);
       }
     }

  // DQ (7/19/2017): Fixup the declaration scope is required because class declarations have been generated in the processing of the template parameters.
  // Note that this code is similar to that in the convert_template_struct_secondary(), so it might be that we can refactor this.
  // DQ (7/18/2017): Adding support for class declarations built from template parameters.  The scope for 
  // these must be the SgDeclarationScope that is contained in the nondefining template declaration.
     setupDeclarationScope(decl);

#if 0
     printf ("Exiting after fixing up the declaration scope \n");
     ROSE_ASSERT(false);
#endif

  // DQ (9/10/2012): Test the just built template with its template parameters.
     if (decl->get_templateParameters().size() == 0)
        {
#ifdef DEBUG_CONVERSION
          printf ("WARNING: In convert_template_struct_primary(): decl->get_templateParameters().size() == 0: OK for nested classes/structs in template declarations decl = %p \n",decl);
#endif
        }
  // ROSE_ASSERT(decl->get_templateParameters().size() > 0);

  // DQ (7/15/2012): Added testing specific to autonomous declearations for templates.
     ROSE_ASSERT(decl != NULL);

  // DQ (9/9/2012): Added call to setup the template parameters.
  // setupTemplateParameters(edg_template,decl);

#if 0
     printf ("In convert_template_struct_primary(): p->autonomous_primary_tag_decl = %s \n",p->autonomous_primary_tag_decl ? "true" : "false");
#endif

  // DQ (7/15/2012): fails for test2006_01.C
  // ROSE_ASSERT(decl->get_parent() == NULL);
     if (decl->get_parent() != NULL)
        {
#if PRINT_DEVELOPER_WARNINGS
          printf ("WARNING: decl->get_parent() != NULL: (defining declaration may have been reused) decl = %p \n",decl);
#endif
          ROSE_ASSERT(nonDefiningDecl != NULL);
          ROSE_ASSERT(nonDefiningDecl->get_definingDeclaration() != NULL);
          ROSE_ASSERT(nonDefiningDecl->get_definingDeclaration()->get_parent() != NULL);
        }

     if (p->autonomous_primary_tag_decl == false)
        {
#if 0
          if (decl->get_parent() != NULL)
             {
               printf ("decl = %p = %s decl->get_parent() = %p = %s \n",decl,decl->class_name().c_str(),decl->get_parent(),decl->get_parent()->class_name().c_str());
             }
#endif
       // ROSE_ASSERT(decl->get_parent() == NULL);
          if (decl->get_parent() != NULL)
             {
#ifdef DEBUG_CONVERSION
               printf ("WARNING (skipping 2nd test): decl->get_parent() != NULL: (defining declaration may have been reused) decl = %p \n",decl);
#endif
             }
        }
       else
        {
       // ROSE_ASSERT(decl->get_parent() != NULL);

       // DQ (1/21/2013): This is a problem for BoxLib (captured in test2013_18.C)
       // ROSE_ASSERT(decl->get_parent() == NULL);
       // decl->set_parent(scopeOfDeclaration);
       // ROSE_ASSERT(decl->get_parent() != NULL);
          if (decl->get_parent() == NULL)
             {
               decl->set_parent(scopeOfDeclaration);
               ROSE_ASSERT(decl->get_parent() != NULL);
             }
            else
             {
#if PRINT_DEVELOPER_WARNINGS
               printf ("In convert_template_struct_primary(): Note that decl->get_parent() != NULL: likely because this declaration is reused \n");
#endif
            // DQ (1/21/2013): If this is reused then I think that we can assert this.
               ROSE_ASSERT(decl->get_definingDeclaration() != NULL);
               ROSE_ASSERT(decl == decl->get_definingDeclaration());
               
             }
        }

#if 0
     if (nonDefiningDecl != NULL)
        {
          printf ("In convert_template_struct_primary(): nonDefiningDecl = %p \n",nonDefiningDecl);
          nonDefiningDecl->get_file_info()->display("In convert_template_struct_primary(): nonDefiningDecl debug");
        }
     decl->get_file_info()->display("In convert_template_struct_primary(): decl debug");
#endif
#if 0
     decl->get_firstNondefiningDeclaration()->get_file_info()->display("In convert_template_struct_primary(): decl->get_firstNondefiningDeclaration() debug");
#endif

#if 0
     printf ("In convert_template_struct_primary(): nameWithTemplateSpecializationArguments = %s \n",nameWithTemplateSpecializationArguments.str());
#endif

     decl->set_forward(false);

  // DQ (12/4/2011): We need a concept for type for SgTemplateClassDeclaration so that we can construct SgMemberFunctionType nodes for template member functions.
  // We use a SgClassType which has been fixed to permit it to hold either a SgClassDeclaration or an SgTemplateClassDeclaration.
     ROSE_ASSERT(decl->get_type() != NULL);

#if 0
     printf ("In convert_template_struct_primary(): buildClassDeclaration_nfi() returned decl = %p \n",decl);
     printf ("In convert_template_struct_primary(): decl->get_firstNondefiningDeclaration()   = %p \n",decl->get_firstNondefiningDeclaration());
     printf ("In convert_template_struct_primary(): decl->get_definingDeclaration()           = %p \n",decl->get_definingDeclaration());
#endif

  // DQ (5/2/2012): Added support for debugging the source position information for template declarations.
     ROSE_ASSERT(decl->get_firstNondefiningDeclaration() != NULL);

  // DQ (5/13/2012): Don't reset since this will reset the secondary (non-defining declaration) 
  // to match the primary (defining declaration) and cause comments and CPP declarations to be
  // matched to the incorrect lines (see bug for test2007_47.C).
  // We have to set the source positiuon for the non-definng template declaration...
  // set_file_info_for_corresp(decl->get_firstNondefiningDeclaration(), p->source_corresp);
     ROSE_ASSERT(decl->get_firstNondefiningDeclaration()->get_file_info() != NULL);

#if 0
     decl->get_firstNondefiningDeclaration()->get_file_info()->display("In convert_template_struct_primary(): decl->get_firstNondefiningDeclaration() debug");
#endif

     ROSE_ASSERT(decl->get_firstNondefiningDeclaration() != decl);

  // DQ (8/26/2012): This fails for non-real classes.
  // DQ (12/29/2011): Make sure this is not interpreted as a template.
  // ROSE_ASSERT(p->variant.class_struct_union.extra_info->assoc_template != NULL);
     if (p->variant.class_struct_union.extra_info->assoc_template != NULL)
        {
          if (!hasFirstNondefDeclForTemplate(p->variant.class_struct_union.extra_info->assoc_template))
             {
            // DQ (12/30/2011): This might be a clue that we have to process this class directly before proceeding...
#if 0
               printf("In convert_template_struct_primary(): Note that we have not seen the non-defining declaration for this before (rare code in stl_list.h:303) \n");
#endif
             }

       // DQ (12/30/2011): This fails for test2011_200.C
          if (hasDefiningDeclForTemplate(p->variant.class_struct_union.extra_info->assoc_template))
             {
               // implied => ROSE_ASSERT(getDefiningDeclForTemplate(p->variant.class_struct_union.extra_info->assoc_template) != NULL);
             }
            else
             {
            // DQ (12/30/2011): Don't reset this if it is already set.
#if 0
               printf ("Adding decl = %p to defining_declarations_for_templates[p->variant.class_struct_union.extra_info->assoc_template = %p] \n",decl,p->variant.class_struct_union.extra_info->assoc_template);
#endif
               setDefiningDeclForTemplate(p->variant.class_struct_union.extra_info->assoc_template, decl);
             }
        }
       else
        {
       // DQ (8/26/2012): Since assoc_template == NULL, verify that this is a non-real class.
          ROSE_ASSERT(p->variant.class_struct_union.is_nonreal_class == true);

#ifdef DEBUG_CONVERSION
          printf ("Should we been using a map for the nonreal classes that we build? decl = %p = %s \n",decl,decl->class_name().c_str());
#endif
        }

     ROSE_ASSERT(decl->get_scope() != NULL);
  // ROSE_ASSERT(decl->get_type() != NULL);

  // DQ (8/27/2014): Moved to using seperate cache for mapping EDG types to types associated with template declarations.
  // This is required because the same key (EDG a_type_ptr value) is used to build both ROSE template declarations and
  // ROSE template instantiations.  This avoids collisions that were not being detected and have caused problems (bugs).
  // See test2014_161.C for an example of a bug caused by this issue.

  // DQ (12/21/2011): Make sure that the type associated with this class declaration is in the type_translation_cache.
     if (template_type_translation_cache.find(p) == template_type_translation_cache.end())
        {
          ROSE_ASSERT(decl->get_type() != NULL);
          template_type_translation_cache[p] = decl->get_type();
          ROSE_ASSERT(template_type_translation_cache[p] != NULL);
        }
     ROSE_ASSERT(template_type_translation_cache.find(p) != template_type_translation_cache.end());
     ROSE_ASSERT(template_type_translation_cache[p] != NULL);

  // DQ (11/11/2014): I think this is a problem for the merge tests which compile the same file twice on the command line.
  // DQ (8/12/2012): Added test...
  // ROSE_ASSERT(template_type_translation_cache[p] == decl->get_type());
     if (template_type_translation_cache[p] != decl->get_type())
        {
          printf ("Note: template_type_translation_cache[p] != decl->get_type(): this happens for the mergeAST tests (mergeTest_111.C) \n");
        }

  // ********************************************************************
  // DQ (12/11/2011): Copy of code from convert_struct_primary():
  // DQ (11/27/2011): Set the scope of the new defining declaration's SgClassDefinition in the scope_cache.
     ROSE_ASSERT(p->kind == tk_class || p->kind == tk_struct || p->kind == tk_union);
     ROSE_ASSERT(p->variant.class_struct_union.extra_info != NULL);

#if 1
  // DQ (12/28/2011): This is a proper pointer to the scope for the class, it edg scope is a list of the scopes for
  // each routine, but this is not a scope for the first routine in the class.

  // DQ (12/21/2011): I think this is a linked list of scopes associated with the member functions of the class and
  // not anything associated with the class definition scope.  So this is not the scope that we are looking for...
     a_scope_ptr assoc_scope = p->variant.class_struct_union.extra_info->assoc_scope;

  // ROSE_ASSERT(assoc_scope != NULL);
     if (assoc_scope != NULL)
        {
          SgClassDefinition* classDefinition = decl->get_definition();
          ROSE_ASSERT(classDefinition != NULL);
#if 0
          printf ("===== Putting classDefinition = %p = %s into scope_cache[assoc_scope = %p] \n",classDefinition,classDefinition->class_name().c_str(),assoc_scope);
#endif
#if 0
       // DQ (3/12/2012): I think I set this incorrectly yesterday.
       // DQ (3/11/2012): check that this is not already in the list before we add it (or
       // verify it is NULL if it is supposed to be in the list but just not initialized yet).
       // ROSE_ASSERT (scope_cache.find(assoc_scope) == scope_cache.end());
          ROSE_ASSERT (scope_cache.find(assoc_scope) != scope_cache.end());

       // ROSE_ASSERT(isSgClassDefinition(decl->get_scope()) != NULL);
       // scope_cache[assoc_scope] = decl->get_scope();
          scope_cache[assoc_scope] = classDefinition;
#else
       // DQ (3/12/2012): It appears that sometimes the entry is in the scope_cache and sometime not.
       // So we will implement this two ways so that I can better understand this and evaluate if
       // it might be an error of some sort.
       // if (scope_cache.find(assoc_scope) != scope_cache.end())
#if 1
       // DQ (8/26/2014): Refactored the support to update the scope cache for template declaration 
       // scopes and template instantiations and class declaration scopes.
          update_scope_cache(scope_cache,assoc_scope,classDefinition);
          ROSE_ASSERT(scope_cache[assoc_scope] != NULL);
#else
          if (scope_cache.find(assoc_scope) != scope_cache.end())
             {
#if 0
               printf ("In convert_template_struct_primary(): Note assoc_scope = %p is already in scope_cache: scope_cache[assoc_scope] = %p (don't reset to classDefinition = %p) \n",assoc_scope,scope_cache[assoc_scope],classDefinition);
#endif
               if (scope_cache[assoc_scope] != NULL)
                  {
                 // DQ (3/12/2012): I don't like this, so I want to know when this happens, but we will allow it for now.
#ifdef DEBUG_CONVERSION
                    printf ("WARNING: scope_cache[assoc_scope] != NULL and will be reset \n");
#endif
                    if (scope_cache[assoc_scope] != classDefinition)
                       {
#ifdef DEBUG_CONVERSION
                         printf ("*** WARNING: scope_cache[assoc_scope] != NULL and will be reset to a DIFFERENT value! \n");
#endif
#if 0
                         SgScopeStatement* scope_from_cache = scope_cache[assoc_scope];
                         printf ("WARNING: In convert_template_struct_primary(): scope_cache[assoc_scope = %p] = %p = %s and is not reset with classDefinition = %p = %s \n",
                              assoc_scope,scope_from_cache,scope_from_cache->class_name().c_str(),classDefinition,classDefinition->class_name().c_str());
#endif
                       }
                  }
            // ROSE_ASSERT(scope_cache[assoc_scope] == NULL);

            // DQ (4/22/2012): We should not be resetting the value and it should not be NULL.
            // scope_cache[assoc_scope] = classDefinition;
               ROSE_ASSERT(scope_cache[assoc_scope] != NULL);
               if (scope_cache[assoc_scope] == NULL)
                  {
#if 0
                    printf ("Resetting NULL value in scope_cache[assoc_scope = %p] to classDefinition = %p = %s \n",assoc_scope,classDefinition,classDefinition->class_name().c_str());
#endif
                    scope_cache[assoc_scope] = classDefinition;
                  }
             }
            else
             {
#if 0
               printf ("In convert_template_struct_primary(): Note assoc_scope = %p not yet defined in scope_cache: will be set to classDefinition = %p = %s \n",assoc_scope,classDefinition,classDefinition->class_name().c_str());
#endif
               scope_cache[assoc_scope] = classDefinition;
             }

          ROSE_ASSERT(scope_cache[assoc_scope] != NULL);
#endif
#endif

       // scopeOfDeclaration = scope_cache[scp.parent_scope];
       // ROSE_ASSERT(scope_cache[p->source_corresp.parent_scope] != NULL);
       // ROSE_ASSERT(decl->get_scope() == scope_cache[p->source_corresp.parent_scope]);
       // ROSE_ASSERT(decl->get_scope() == scope_cache[assoc_scope]);
       // ROSE_ASSERT(scope_cache[assoc_scope] != NULL);

       // DQ (4/22/2012): Skip this test now that we don't reset the value.
       // ROSE_ASSERT(classDefinition == scope_cache[assoc_scope]);
#if 0
          printf ("In convert_template_struct_primary(): Skipped test for classDefinition == scope_cache[assoc_scope] \n");
#endif
        }
       else
        {
       // DQ (4/27/2012): There is not always a valid p->variant.class_struct_union.extra_info->assoc_scope pointer (see test2005_12.C).
       // ROSE_ASSERT(false);
        }
#endif
  // ********************************************************************


  // DQ (4/8/2012): This function is building a template class declaration and must add this declaration to 
  // the EDG/ROSE maps ASAP.  The current entry associated with the input EDG type "p" is likely already in
  // the map, but is NULL.
     ROSE_ASSERT(p->kind == tk_class || p->kind == tk_struct || p->kind == tk_union);
#if 1

  // DQ (4/24/2012): Test the maps that we use internally (see function for documentation).
     testEdgRoseMaps();

  // DQ (8/26/2012): Added test to support non-real template class declarations.
  // ROSE_ASSERT (edg_template != NULL);

  // DQ (8/26/2012): Added conditional to support processing on non-real classes (associated with SgTemplatType IR nodes within (nested) template declarations)
     if (edg_template == NULL)
        {
       // This case fails for non-real classes.
          mprintf ("STILL NEED TO --- MARK THIS CLASS AS A MINIMAL INTERFACE CLASS USED! decl = %p \n",decl);
          ROSE_ASSERT(p->variant.class_struct_union.is_nonreal_class == true);
          ROSE_ASSERT(nonDefiningDecl != NULL);
       // ROSE_ASSERT(nonDefiningDecl->get_isRepresentingTemplateParameterInTemplateDeclaration() == true);
          if (nonDefiningDecl->get_isRepresentingTemplateParameterInTemplateDeclaration() == false)
             {
#ifdef DEBUG_CONVERSION
               mprintf ("WARNING: Unclear why the non-defining declaration is not marked as isRepresentingTemplateParameterInTemplateDeclaration == true nonDefiningDecl = %p = %s \n",decl,decl->class_name().c_str());
#endif
             }
        }
       else
        {
       // Only set this as the non-defining declaration if it was not previosly set in the first_nondefining_declarations_for_templates map.
          if (!hasFirstNondefDeclForTemplate(edg_template))
             {
            // Now associate a valid ROSE template class declaration in the map.
               ROSE_ASSERT(decl != NULL);
               ROSE_ASSERT(edg_template != NULL);

               setFirstNondefDeclForTemplate(edg_template, decl->get_firstNondefiningDeclaration());
             }
            else
             {
#ifdef DEBUG_CONVERSION
               printf ("Note: the first_nondefining_declarations_for_templates was already present \n");
#endif
             }

       // DQ (4/24/2012): Added code to record the mapping of the defining declaration to the ROSE defining declaration.
          if (!hasDefiningDeclForTemplate(edg_template))
             {
            // Now associate a valid ROSE template class declaration in the map.
               ROSE_ASSERT(decl != NULL);
               ROSE_ASSERT(edg_template != NULL);

            // DQ (4/24/2012): Don't put the defining declaration into the nondefining declaration map.
#if 0
               printf ("Adding decl = %p to defining_declarations_for_templates[edg_template = %p] \n",decl,edg_template);
#endif
               setDefiningDeclForTemplate(edg_template, decl);

            // And make sure that this is the defining declaration.
               ROSE_ASSERT(decl == decl->get_definingDeclaration());
             }
            else
             {
#ifdef DEBUG_CONVERSION
               printf ("Note: the defining_declarations_for_templates was already present \n");
#endif
             }
        }
#endif
#if 0
     printf ("Exiting after test for template declaration in cache \n");
     ROSE_ASSERT(false);
#endif

  // DQ (4/24/2012): Test the maps that we use internally (see function for documentation).
     testEdgRoseMaps();

  // There may already be a secondary (non-defining) declaration!
  // type_declaration_map[p].push_back(decl);

  // Note that at this point the SgClassSymbol has been built and is
  // in the symbol table for the scope defined by "topScopeStack()"

#if 0
  // DQ (3/22/2012): This is set below so that it is set together...
     if (is_unnamed(p))
        {
          decl->set_isUnNamed(true);
        }
#endif

#if 0
     printf(" - decl = %p (%s)\n", decl, decl ? decl->class_name().c_str() : "");
#endif

     bool buildTemplateDeclaration = true;

     SgSymbol* tmp_symbol = retrieveAssociatedClassSymbol(nameWithOutTemplateSpecializationArguments,scopeOfDeclaration,buildTemplateDeclaration,NULL,&templateParameterList,&templateSpecializationArgumentList);
     if (tmp_symbol == NULL)
        {
#if 1
          printf ("In convert_template_struct_primary(): tmp_symbol == NULL: scopeOfDeclaration = %p = %s \n",scopeOfDeclaration,scopeOfDeclaration->class_name().c_str());
          printf (" --- name = %s \n",nameWithOutTemplateSpecializationArguments.str());
          printf (" --- scopeOfDeclaration = %p = %s name = %s \n",scopeOfDeclaration,scopeOfDeclaration->class_name().c_str(),SageInterface::get_name(scopeOfDeclaration).c_str());
#endif
        }
     ROSE_ASSERT(tmp_symbol != NULL);

#if 0
     printf (" - tmp_symbol = %p (%s\n", tmp_symbol, tmp_symbol->class_name().c_str());
#endif

     SgTemplateClassSymbol* mysymbol = isSgTemplateClassSymbol(tmp_symbol);
     ROSE_ASSERT(mysymbol != NULL);

#if 0
     printf (" - mysymbol = %p (%s\n", mysymbol, tmp_symbol->class_name().c_str());
#endif

     SgTemplateClassDeclaration* firstNondefiningDecl = isSgTemplateClassDeclaration(mysymbol->get_declaration());

     ROSE_ASSERT(firstNondefiningDecl != NULL);

#if 0
     printf ("In convert_template_struct_primary(): Skipped test of nondefdecl->get_parent() != NULL \n");
#endif
  // ROSE_ASSERT(firstNondefiningDecl->get_parent() != NULL);

     firstNondefiningDecl->set_definingDeclaration(decl);

  // If this is a forward declaration, not associated with the original 
  // first-nondefining declaration then point to the original non-defining 
  // declaration fo consistancy!
     decl->set_firstNondefiningDeclaration(firstNondefiningDecl);

  // This might only be setting the definin declaration to NULL, if we have not see the defining declaration yet!
     decl->set_definingDeclaration(firstNondefiningDecl->get_definingDeclaration());

     ROSE_ASSERT(firstNondefiningDecl->get_definingDeclaration() == decl);
     ROSE_ASSERT(firstNondefiningDecl->get_firstNondefiningDeclaration() != decl);

  // DQ (11/29/2011): Added checks...
     ROSE_ASSERT(firstNondefiningDecl->get_firstNondefiningDeclaration() == firstNondefiningDecl);

     ROSE_ASSERT (firstNondefiningDecl != NULL);

  // DQ (3/22/2012): Make this uniform so that both are set together.
     if (is_unnamed(p))
        {
#if 0
          printf ("In convert_template_struct_primary(): Mark class declaration = %p = %s as UN-NAMED \n",decl,decl->class_name().c_str());
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

  // ROSE_ASSERT (firstNondefiningDecl->get_forward() == true);
  // ROSE_ASSERT (firstNondefiningDecl->get_forward() == false);

     ROSE_ASSERT(decl->get_firstNondefiningDeclaration() != decl);

     ROSE_ASSERT(decl != firstNondefiningDecl);

     decl->set_firstNondefiningDeclaration(firstNondefiningDecl);
     ROSE_ASSERT(decl->get_firstNondefiningDeclaration() != decl);

     firstNondefiningDecl->set_definingDeclaration(decl);

  // Since we get the type from decl, this is meaningless...
  // decl->set_type(classType);

#if 0
     printf ("decl                                            = %p \n",decl);
     printf ("firstNondefiningDecl->get_definingDeclaration() = %p \n",firstNondefiningDecl->get_definingDeclaration());
#endif

     ROSE_ASSERT(firstNondefiningDecl->get_definingDeclaration() == decl);
     ROSE_ASSERT(firstNondefiningDecl->get_firstNondefiningDeclaration() == firstNondefiningDecl);
     ROSE_ASSERT(firstNondefiningDecl->get_definingDeclaration()->get_firstNondefiningDeclaration() == firstNondefiningDecl);

#if 0
     printf ("In convert_template_struct_primary(): Skipped test of nondefdecl->get_parent() != NULL \n");
#endif
  // ROSE_ASSERT(firstNondefiningDecl->get_parent() != NULL);

  // DQ (7/15/2012): test2006_01.C fails here.
  // ROSE_ASSERT(firstNondefiningDecl->get_definingDeclaration()->get_parent() != NULL);

     ROSE_ASSERT(decl->get_firstNondefiningDeclaration() != decl);

  // DQ (7/15/2012): test2006_01.C fails here.
  // ROSE_ASSERT(decl->get_parent() != NULL);

  // DQ (8/27/2014): Modified to use the new template_type_translation_cache instead of the previous type_translation_cache.
  // DQ (12/21/2011): Make sure that the type associated with this class declaration is in the type_translation_cache.
  // ROSE_ASSERT(type_translation_cache.find(p) != type_translation_cache.end());
  // ROSE_ASSERT(type_translation_cache[p] != NULL);
     ROSE_ASSERT(template_type_translation_cache.find(p) != template_type_translation_cache.end());
     ROSE_ASSERT(template_type_translation_cache[p] != NULL);

  // DQ (3/3/2012): Added test to make sure that the defining and non defining declarations variants are the same.
     ROSE_ASSERT(matching_defining_and_nondefining_variants(decl) == true);

  // DQ (4/8/2012): Test the maps that we use internally (see function for documentation).
     testEdgRoseMaps();

#if DEBUG_STRUCTURAL_DESIGN || DEBUG_CLASSES_FROM_TEMPLATE_PARAMETERS || 0
     printf ("********************************************************************************** \n");
     printf ("Leaving convert_template_struct_primary(): decl = %p = %s nameWithTemplateSpecializationArguments = %s summarize_type_kind_in_string_kind(p = %p p->kind) = %s \n",
          decl,decl->class_name().c_str(),nameWithTemplateSpecializationArguments.str(),p,summarize_type_kind_in_string_kind(p->kind));
     printf ("********************************************************************************** \n");
#endif

#if 0
     printf ("Source position for defining template class declaration: \n");
     decl->get_startOfConstruct()->display("Source position for defining template class declaration: START: debug");
     decl->get_endOfConstruct  ()->display("Source position for defining template class declaration: END: debug");
#endif

#if 0
     printf ("Exiting as a test! \n");
     ROSE_ASSERT(false);
#endif

  // DQ (9/6/2012): Added support for testing first non-defining, defining, and secondary declarations.
     testDefiningNonDefiningDeclarations(firstNondefiningDecl,decl,NULL,"convert_template_struct_primary()");

#if 0
     decl->get_file_info()->display("In convert_template_struct_primary(): decl");
     if (decl->get_firstNondefiningDeclaration() != NULL)
        {
          printf ("decl->get_firstNondefiningDeclaration() = %p = %s \n",decl->get_firstNondefiningDeclaration(),decl->get_firstNondefiningDeclaration()->class_name().c_str());
          decl->get_firstNondefiningDeclaration()->get_file_info()->display("In convert_template_struct_primary(): decl");
        }
#endif

  // DQ (7/15/2012): Added testing specific to autonomous declearations for templates.
     ROSE_ASSERT(decl != NULL);
     if (p->autonomous_primary_tag_decl == false)
        {
          if (decl->get_parent() != NULL)
             {
            // DQ (1/13/2013): Added to the comment.
               printf ("WARNING (this happens when an defining declaration is reused): decl = %p = %s decl->get_parent() = %p = %s \n",decl,decl->class_name().c_str(),decl->get_parent(),decl->get_parent()->class_name().c_str());
             }
       // DQ (1/13/2013): We need to allow the parent to have been set if this is a reused defining declaration.
       // ROSE_ASSERT(decl->get_parent() == NULL);
       // ROSE_ASSERT(firstNondefiningDecl->get_definingDeclaration()->get_parent() == NULL);
        }
       else
        {
          ROSE_ASSERT(decl->get_parent() != NULL);

          ROSE_ASSERT(firstNondefiningDecl->get_definingDeclaration()->get_parent() != NULL);
        }

#if 0
  // DQ (9/9/2012): It appears that the template parameter list is not set properly (frequently zero, at least).
     printf ("In convert_template_struct_primary(): decl->get_templateParameters().size() = %" PRIuPTR " \n",decl->get_templateParameters().size());
#endif

#if 0
  // DQ (9/9/2012): It appears that the template parameter list is not set properly (frequently zero, at least).
     printf ("In convert_template_struct_primary(): (after setting the template parameters) decl->get_templateParameters().size() = %" PRIuPTR " \n",decl->get_templateParameters().size());
#endif

  // ROSE_ASSERT(decl->get_templateParameters().size() > 0);


  // DQ (8/11/2014): Added to support C++11 final keyword.
     if (p->variant.class_struct_union.final == true)
        {
#ifdef DEBUG_CONVERSION
          printf ("In convert_template_struct_primary(): detected final keyword \n");
#endif
          decl->get_declarationModifier().setFinal();

#if 0
          printf ("Exiting as a test! \n");
          ROSE_ASSERT(false);
#endif
        }

     return decl;
   }
