
#include "edgRose.h"

SgDeclarationStatement* EDG_ROSE_Translation::convert_struct_primary(a_type_ptr p, SgClassDeclaration::class_types t, SgClassDeclaration* nonDefiningDecl)
   {
  // Note: nonDefiningDecl can sometimes be NULL, see note below.

#define DEBUG_CONVERT_STRUCT_PRIMARY 0

  // DQ (1/2/2012): Refactoring to generate common name used in this function.
  // SgName name = e->source_corresp.name;
  // SgName name = p->source_corresp.name == NULL ? generate_name_for_type(p) : p->source_corresp.name;
     SgName nameWithOutTemplateArguments = generateName(p);
     SgName nameWithTemplateArguments    = nameWithOutTemplateArguments;

  // DQ (9/13/2012): These are used for template declarations.
     SgName nameWithOutTemplateSpecializationArguments = nameWithOutTemplateArguments;
     SgName nameWithTemplateSpecializationArguments    = nameWithOutTemplateSpecializationArguments;

     bool originally_unnamed = p->variant.class_struct_union.originally_unnamed;

#if DEBUG_STRUCTURAL_DESIGN || DEBUG_CONVERT_STRUCT_PRIMARY || 0
     printf ("******************************************************************************************* \n");
     printf ("In convert_struct_primary(): (TOP) nameWithOutTemplateArguments = %s originally_unnamed = %s \n",nameWithOutTemplateArguments.str(),originally_unnamed ? "true" : "false");
     printf ("In convert_struct_primary(): summarize_type_kind_in_string_kind(p->kind)                = %s \n",summarize_type_kind_in_string_kind(p->kind));
     printf ("In convert_struct_primary(): nonDefiningDecl                                            = %p \n",nonDefiningDecl);
     if (nonDefiningDecl != NULL)
        {
       // If this was a class built from a template parameter, then the scope will be a SgDeclarationScope
          printf ("In convert_struct_primary(): nonDefiningDecl = %p nonDefiningDecl->get_name()           = %s \n",nonDefiningDecl,nonDefiningDecl->get_name().str());
          SgScopeStatement* tmp_scope = nonDefiningDecl->get_scope();
          printf ("In convert_struct_primary(): nonDefiningDecl->get_scope()                               = %p = %s \n",tmp_scope,tmp_scope->class_name().c_str());

       // DQ (7/31/2019): Test this because it appears to be the earliest point, identified so far, where the non defining declaration is NULL.
       // ROSE_ASSERT(nonDefiningDecl->get_parent() != NULL);
        }
     printf ("******************************************************************************************* \n");
#endif

     if (p->kind == tk_typeref)
        {
          printf ("In convert_struct_primary(): This is the case of a C++11 typedef of a class (we might want to have caught this before getting here) \n");
#if 1
          printf ("Exiting as a test! \n");
          ROSE_ASSERT(false);
#endif
        }

  // DQ (10/30/2012): Test this, but I don't think it will always be true (e.g. a forward reference would invalidate this test).
  // ROSE_ASSERT(type_translation_cache.find(p) == type_translation_cache.end());

  // DQ (8/2/2012): For unnamed types we want to generate a unique name (not reuse the typedef name as EDG appears to be doing).
     if (originally_unnamed == true)
        {
       // We want to generate a internal name to use instead of borrowing the typedef name as a name for the class.
          nameWithOutTemplateArguments = generate_name_for_type(p);

       // Note that I don't think this will be a template instantiation since that would not make sense for an nunamed type.
          nameWithTemplateArguments    = nameWithOutTemplateArguments;
        }

#ifdef DEBUG_CONVERSION
     indent_line(); debug_print("%s primary\n", (t == SgClassDeclaration::e_union ? "union" : t == SgClassDeclaration::e_struct ? "struct" : t == SgClassDeclaration::e_class ? "class" : "???"));
  // string name = p->source_corresp.name == NULL ? generate_name_for_type(p) : p->source_corresp.name;
     indent_line(); debug_print("convert_struct_primary(): p = %p name = %s nonDefiningDecl = %p : ",p,nameWithOutTemplateArguments.str(),nonDefiningDecl); print_source_corresp(p->source_corresp); debug_print("\n");

  // DQ (9/24/2012): tracing down superious output...
  // printf ("In convert_struct_primary(): After convert_struct_primary(): p and print_source_corresp() \n");
#endif

     ROSE_ASSERT (topScopeStack() != NULL);

#if 0
     printf ("In convert_struct_primary(): topScopeStack() = %p = %s \n",topScopeStack(),topScopeStack()->class_name().c_str());
#endif

  // DQ (4/26/2012): Test the maps that we use internally (see function for documentation).
     testEdgRoseMaps();

#if 0
     printf ("In convert_struct_primary(): calling summarize_type() \n");
     summarize_type(p);
#endif

#if DEBUG_GET_DECLARATION_SCOPE
     printf ("In convert_struct_primary(): calling getDeclarationScope() \n");
#endif
  // The scope of a declaration (in C++) can be different from the current scope (where it is placed structurally).
     SgScopeStatement* scopeOfDeclaration = getDeclarationScope(p->source_corresp);
     ROSE_ASSERT(scopeOfDeclaration != NULL);

#if 0
     printf ("In convert_struct_primary(): scopeOfDeclaration = %p = %s \n",scopeOfDeclaration,scopeOfDeclaration->class_name().c_str());
#endif

  // DQ (7/25/2017): If this was a class built from a template parameter, then the scope will be a SgDeclarationScope
  // Maybe the getDeclarationScope() function should take the nonDefiningDecl as an optional parameter so that it can 
  // refactor this special case elsewhere.  Although it might not be useful because only classes can be built from 
  // template parameters when template types as not generated.
     if (nonDefiningDecl != NULL)
        {
          SgScopeStatement* nonDefiningDecl_scope = nonDefiningDecl->get_scope();
#if 0
          printf ("In convert_struct_primary(): reset scopeOfDeclaration: nonDefiningDecl->get_scope() = %p = %s \n",nonDefiningDecl_scope,nonDefiningDecl_scope->class_name().c_str());
#endif
          scopeOfDeclaration = nonDefiningDecl_scope;
          ROSE_ASSERT(scopeOfDeclaration != NULL);
        }
       else
        {
       // DQ (7/5/2019): If this is an un-named struct (or class or union), then it can't have any name qualification,
       // so the topScopeStack() should work well.
          if (is_unnamed(p) == true)
             {
#if 0
               printf ("In convert_struct_primary(): Using topScopeStack() = %p = %s for un-named struct (or class or union) \n",topScopeStack(),topScopeStack()->class_name().c_str());
#endif
#if 0
            // DQ (7/7/2019): Note that for C language support all types are set in global scope (same for functions).
            // But why is the result returned from getDeclarationScope() wrong?
               SgScopeStatement* topOfScopeStackScope = topScopeStack();
               if (scopeOfDeclaration != topOfScopeStackScope)
                  {
                    printf ("In convert_struct_primary(): why is the result returned from getDeclarationScope() wrong? (using topScopeStack()) \n");

                    ROSE_ASSERT(scopeOfDeclaration != NULL);
                    ROSE_ASSERT(topOfScopeStackScope != NULL);
                    printf (" --- scopeOfDeclaration   = %p = %s \n",scopeOfDeclaration,scopeOfDeclaration->class_name().c_str());
                    printf (" --- topOfScopeStackScope = %p = %s \n",topOfScopeStackScope,topOfScopeStackScope->class_name().c_str());
#if 0
                    topOfScopeStackScope->get_file_info()->display("topOfScopeStackScope: for un-named construct, scopeOfDeclaration != topOfScopeStackScope: debug");
#endif
                 // DQ (7/6/2019): debugging test2019_09.c.
                    printf ("########################### In convert_struct_primary(): NO LONGER: Skipping resetting of scopeOfDeclaration \n");
                 // scopeOfDeclaration = topScopeStack();
#if 0
                    printf ("Exiting as a test! \n");
                    ROSE_ASSERT(false);
#endif
                  }
#endif
             }
            else
             {
#if 0
               printf ("In convert_struct_primary(): We have not good way to determin the scope: topScopeStack() = %p = %s \n",topScopeStack(),topScopeStack()->class_name().c_str());
#endif
             }
        }

#if DEBUG_CONVERT_STRUCT_PRIMARY
     printf ("***** In convert_struct_primary(): evaluate topScopeStack() = %p = %s before call buildStructDeclaration_nfi() \n",topScopeStack(),topScopeStack()->class_name().c_str());
     printf ("***** In convert_struct_primary(): using scopeOfDeclaration = %p = %s to call buildStructDeclaration_nfi() \n",scopeOfDeclaration,scopeOfDeclaration->class_name().c_str());
#endif

     bool buildTemplateDeclaration   = false;
     bool buildTemplateInstantiation = false;
     bool forceTemplateDeclaration   = false;
     setTemplateOrTemplateInstantiationClassGeneration(p,forceTemplateDeclaration,buildTemplateDeclaration,buildTemplateInstantiation);

#if DEBUG_CONVERT_STRUCT_PRIMARY
     printf ("In convert_struct_primary(): buildTemplateDeclaration   = %s \n",buildTemplateDeclaration ? "true" : "false");
     printf ("In convert_struct_primary(): buildTemplateInstantiation = %s \n",buildTemplateInstantiation ? "true" : "false");
#endif

  // DQ (2/11/2013): Debugging support for compiling rose.h header file.
     if (p->variant.class_struct_union.is_nonreal_class == true)
        {
#if 0
          printf ("In convert_struct_primary(): p->variant.class_struct_union.is_nonreal_class = %s \n",p->variant.class_struct_union.is_nonreal_class ? "true" : "false");
          printf ("In convert_struct_primary(): buildTemplateDeclaration   = %s \n",buildTemplateDeclaration   ? "true" : "false");
          printf ("In convert_struct_primary(): buildTemplateInstantiation = %s \n",buildTemplateInstantiation ? "true" : "false");
#endif
          if (buildTemplateDeclaration == true)
             {
            // DQ (2/11/2013): I am not clear if we should perhaps mark this as a buildTemplateInstantiation = true.
#if 0
               printf ("ERROR: we cannot support this nonreal class as a template declaration: reset buildTemplateDeclaration = false \n");
#endif
               buildTemplateDeclaration = false;
#if 0
            // This path is only taken on ROSE compiling the rose.h header file (I need more information about this).
               printf ("ERROR: In convert_struct_primary(): buildTemplateDeclaration == true: calling summarize_type() \n");
               summarize_type(p);
#endif
             }
        }

  // If this assertion is false, then we should have likely called a different function to build the 
  // template declaration. Or we should have forced a template instantiation instead.
     if (buildTemplateDeclaration == true)
        {
       // This path is only taken on ROSE compiling the rose.h header file (I need more information about this).
          printf ("ERROR: In convert_struct_primary(): buildTemplateDeclaration == true: calling summarize_type() \n");
          summarize_type(p);
        }
     ROSE_ASSERT(buildTemplateDeclaration == false);

  // DQ (12/29/2011): Can we assert this? NO! This fails for test2001_01.C.
  // The reason is that sometimes only the defining declaration is the first class declaration to be 
  // seen, then this triggers the construction of a non-defining declaration in the AST build interface.
  // ROSE_ASSERT(nonDefiningDecl != NULL);

#if DEBUG_CONVERT_STRUCT_PRIMARY
     printf ("In convert_struct_primary(): nonDefiningDecl = %p = %s (passed in as input to convert_struct_primary()) \n",nonDefiningDecl,nonDefiningDecl != NULL ? nonDefiningDecl->class_name().c_str() : "null");
     printf ("In convert_struct_primary(): type_translation_cache.find(p) == type_translation_cache.end() = %s \n",(type_translation_cache.find(p) == type_translation_cache.end()) ? "true" : "false");
#endif

#if 0
     printf ("In convert_struct_primary(): Calling parse_template_parameters() \n");
#endif

  // TV (07/23/2018): This does not make sense as it retrieve the template parameter of the associated template (we are NOT parsing a template)
  //                  Replaced by NULL in the two places where it was used to retrive symbol for printing debug info
  // DQ (9/10/2012): Setup the template parameters (required to disambiguate templates when the names are not enough). See test2012_209.C
  // SgTemplateParameterPtrList templateParameterList = parse_template_parameters(p,scopeOfDeclaration,NULL);

#if 0
     printf ("DONE: In convert_struct_primary(): Calling parse_template_parameters() \n");
#endif

#if 0
  // DQ (9/10/2012): Setup the template specialization arguments (these also have to be passed to the SageBuilder API (since they are also needed to disambiguate template names).
  // a_template_arg_ptr templateSpecializationArgs = p->variant.class_struct_union.extra_info->partial_spec_template_arg_list;
  // This is the list of template arguments on which the list of "actual" arguments is based.
     a_template_arg_ptr templateSpecializationArgs        = p->variant.class_struct_union.extra_info->partial_spec_template_arg_list;
  // This is the list of "actual" template arguments.
     a_template_arg_ptr templateSpecializationArgs_actual = p->variant.class_struct_union.extra_info->template_arg_list;
#else
  // DQ (8/12/2014): New for C++ we can now have templated typedefs (see Cxx11_tests/test2014_38.C).
  // Note that this can be a tk_typeref (without template arguments).
     a_template_arg_ptr templateSpecializationArgs        = NULL;
     a_template_arg_ptr templateSpecializationArgs_actual = NULL;
     if (p->kind == tk_class || p->kind == tk_struct || p->kind == tk_union)
        {
       // This is the list of template arguments on which the list of "actual" arguments is based.
          templateSpecializationArgs        = p->variant.class_struct_union.extra_info->partial_spec_template_arg_list;
       // This is the list of "actual" template arguments.
          templateSpecializationArgs_actual = p->variant.class_struct_union.extra_info->template_arg_list;
        }
#endif
     SgTemplateArgumentPtrList templateSpecializationArgumentList;
     if (templateSpecializationArgs != NULL)
        {
       // DQ (9/12/2012): This is tricky: If this is a partial specialization then the "actual" template arguments are in 
       // the p->variant.class_struct_union.extra_info->template_arg_list and the associated names of the template arguments 
       // are in the p->variant.class_struct_union.extra_info->partial_spec_template_arg_list list.  This is because the names
       // need not match that of where they are defined in an outer template.
          ROSE_ASSERT(templateSpecializationArgs_actual != NULL);
#if 0
          printf ("In convert_struct_primary(): Calling parse_template_arguments to support templateSpecializationArgumentList \n");
#endif
#if 0
       // DQ (2/11/2019): Adding support to track the min_template_arguments.
          a_class_type_supplement_ptr classTypeSupplement = p->variant.class_struct_union.extra_info;
          ROSE_ASSERT((p->kind == tk_class || p->kind == tk_struct || p->kind == tk_union) && classTypeSupplement != NULL);
          printf ("In convert_template_struct_primary(): classTypeSupplement->min_template_arguments = %d \n",classTypeSupplement->min_template_arguments);
#endif
       // templateSpecializationArgumentList = parse_template_arguments(templateSpecializationArgs,scopeOfDeclaration);
          templateSpecializationArgumentList = parse_template_arguments(templateSpecializationArgs_actual,scopeOfDeclaration);
#if 0
          printf ("In convert_struct_primary(): DONE: Calling parse_template_arguments to support templateSpecializationArgumentList \n");
#endif
        }

  // DQ (9/13/2012): Set the name used for some references to templates (e.g. calls to get the symbol).
     nameWithTemplateSpecializationArguments = appendTemplateArgumentsToName(nameWithOutTemplateSpecializationArguments,templateSpecializationArgumentList);

#if DEBUG_CLASSES_FROM_TEMPLATE_PARAMETERS
     printf ("In convert_struct_primary(): nameWithOutTemplateSpecializationArguments = %s \n",nameWithOutTemplateSpecializationArguments.str());
     printf ("In convert_struct_primary(): nameWithTemplateSpecializationArguments    = %s \n",nameWithTemplateSpecializationArguments.str());
#endif

#if DEBUG_CONVERT_STRUCT_PRIMARY
     printf ("In convert_struct_primary(): nonDefiningDecl = %p = %s nameWithTemplateSpecializationArguments = %s \n",
          nonDefiningDecl,nonDefiningDecl != NULL ? nonDefiningDecl->class_name().c_str() : "null",nameWithTemplateSpecializationArguments.str());
#endif

  // DQ (1/2/2012): The templateArgumentsList is computed once and used twice (1st to fixup the name, 2nd to append template arguments to the template instantiation).
     a_template_arg_ptr templateArgs = p->variant.class_struct_union.extra_info->template_arg_list;
     SgTemplateArgumentPtrList templateArgumentsList;
#if 0
     if (buildTemplateInstantiation == true && templateArgs == NULL)
        {
       // DQ (5/18/2014): We no long want to assert this (fails for Boost examples under the new changes in 
       // identification of template declarations vs. template instantiations.
       // DQ (5/8/2013): This case happens when explicit template instantiations is specified in the source code.
       // examples such as test2013_159.C using "template<>" syntax.
          printf ("WARNING: buildTemplateInstantiation == true && templateArgs == NULL (case of explicit template instantiation specified in source code) \n");
       // ROSE_ASSERT(false);
#if 0
          printf ("WARNING: buildTemplateInstantiation == true && templateArgs == NULL (case of explicit template instantiation specified in source code): calling summarize_type() \n");
          summarize_type(p);
#endif
        }
#endif

#if 0
     if (buildTemplateInstantiation == true && templateArgs != NULL)
        {
          templateArgumentsList = parse_template_arguments(templateArgs,scopeOfDeclaration);
        }
#else
  // DQ (1/30/2013): We need to be careful about the use of template arguments, since the symbol table lookup requires 
  // that the pointers to SgTemplateArguments must match by value.  The is a problem below where there are assertions 
  // using the lookup via the function: retrieveAssociatedClassSymbol().
     bool templateArgumentListSetup = false;
     if (nonDefiningDecl != NULL)
        {
          SgTemplateInstantiationDecl* templateInstantiationDecl = isSgTemplateInstantiationDecl(nonDefiningDecl);
          if (templateInstantiationDecl != NULL)
             {
            // STL list copy
               templateArgumentsList = templateInstantiationDecl->get_templateArguments();
               templateArgumentListSetup = true;
             }
        }

     if (buildTemplateInstantiation == true && templateArgs != NULL && templateArgumentListSetup == false)
        {
#if 0
          printf ("In convert_struct_primary(): Calling parse_template_arguments to support templateArgumentsList \n");
#endif
#if 0
       // DQ (2/11/2019): Adding support to track the min_template_arguments.
          a_class_type_supplement_ptr classTypeSupplement = p->variant.class_struct_union.extra_info;
          ROSE_ASSERT((p->kind == tk_class || p->kind == tk_struct || p->kind == tk_union) && classTypeSupplement != NULL);
          printf ("In convert_template_struct_primary(): classTypeSupplement->min_template_arguments = %d \n",classTypeSupplement->min_template_arguments);
#endif
          templateArgumentsList = parse_template_arguments(templateArgs,scopeOfDeclaration);
#if 0
          printf ("In convert_struct_primary(): DONE: Calling parse_template_arguments to support templateArgumentsList \n");
#endif
        }
#endif

     if (buildTemplateInstantiation == true)
        {
       // ROSE_ASSERT(isSgTemplateInstantiationDecl(nonDefiningDecl) != NULL);
          if (nonDefiningDecl != NULL)
             {
               ROSE_ASSERT(isSgTemplateInstantiationDecl(nonDefiningDecl) != NULL);
             }

          nameWithTemplateArguments = appendTemplateArgumentsToName(nameWithOutTemplateArguments,templateArgumentsList);

       // DQ (8/2/2012): I don't think it makes sense for a template declaration to be associated with an unnamed type (so let's try to enforce this).
          ROSE_ASSERT(originally_unnamed == false);
        }

#if DEBUG_CONVERT_STRUCT_PRIMARY
     printf ("In convert_struct_primary(): nameWithOutTemplateArguments = %s \n",nameWithOutTemplateArguments.str());
     printf ("In convert_struct_primary(): nameWithTemplateArguments    = %s \n",nameWithTemplateArguments.str());
#if DEBUG_CONVERT_STRUCT_PRIMARY
     printf ("In convert_struct_primary(): evaluate topScopeStack() = %p = %s before call buildStructDeclaration_nfi() \n",topScopeStack(),topScopeStack()->class_name().c_str());
     printf ("In convert_struct_primary(): using scopeOfDeclaration = %p = %s to call buildStructDeclaration_nfi() \n",scopeOfDeclaration,scopeOfDeclaration->class_name().c_str());
#endif
#endif
#if DEBUG_CONVERT_STRUCT_PRIMARY
     printf ("In convert_struct_primary(): Calling buildClassDeclaration_nfi() to build defining declaration (will generate a non-defining declaration if nonDefiningDecl == NULL). \n");
#endif
#if DEBUG_CONVERT_STRUCT_PRIMARY
     printf ("In convert_struct_primary(): type_translation_cache.find(p) != type_translation_cache.end() = %s \n",type_translation_cache.find(p) != type_translation_cache.end() ? "true" : "false");
#endif

  // DQ (9/8/2012): Need to see if this has been previously process, and if a defining declaration was generated (if so we need to use it instead of building another one).
     SgClassDeclaration* definingDeclaration = NULL;
     if (type_translation_cache.find(p) != type_translation_cache.end())
        {
#if DEBUG_CONVERT_STRUCT_PRIMARY
          printf ("In convert_struct_primary(): edg_type p = %p HAS been processed and IS in type_translation_cache as type_translation_cache[p] = %p \n",p,type_translation_cache[p]);
#endif
       // This has been translated previously, look up the declaration and see if there is a defining declaration 
       // that we should reuse (to avoid building a second defining declaration which would not be allowed).
          SgType* tmp_type = type_translation_cache[p];
          ROSE_ASSERT(tmp_type != NULL);
          SgClassType* classType = isSgClassType(tmp_type);
          ROSE_ASSERT(classType != NULL);
          SgClassDeclaration* declaration_from_type = isSgClassDeclaration(classType->get_declaration());
          ROSE_ASSERT(declaration_from_type != NULL);

#if DEBUG_CONVERT_STRUCT_PRIMARY
          printf ("In convert_struct_primary(): classType = %p = %s = %s \n",classType,classType->class_name().c_str(),classType->unparseToString().c_str());
          printf ("In convert_struct_primary(): declaration_from_type = %p = %s = %s \n",declaration_from_type,declaration_from_type->class_name().c_str(),SageInterface::get_name(declaration_from_type).c_str());
#endif
          SgClassDeclaration* defining_declaration_from_type = isSgClassDeclaration(declaration_from_type->get_definingDeclaration());

       // DQ (12/23/2019): If this is a declaration from a different file, then w need to reset 
       // defining_declaration_from_type to avoid sharing the type across translation units.
          if (defining_declaration_from_type != NULL)
             {
               SgGlobal * global_scope = SageInterface::getGlobalScope(defining_declaration_from_type);
               bool already_seen_in_previous_file = global_scope != NULL && global_scope != current_global_scope;
#if 0
               printf ("global_scope         = %p \n",global_scope);
               printf ("current_global_scope = %p \n",current_global_scope);
               printf ("In convert_struct_primary(): already_seen_in_previous_file = %s \n",already_seen_in_previous_file ? "true" : "false");
#endif
               if (already_seen_in_previous_file == true)
                  {
#if 0
                    printf ("already_seen_in_previous_file == true: Reset defining_declaration_from_type = NULL \n");
#endif
                    defining_declaration_from_type = NULL;
                  }
#if 0
               printf ("Exiting as a test! \n");
               ROSE_ASSERT(false);
#endif
             }

#if DEBUG_CONVERT_STRUCT_PRIMARY
          printf ("In convert_struct_primary(): Found first_nondefiningDeclaration = %p = %s \n",declaration_from_type,declaration_from_type->class_name().c_str());
          printf ("In convert_struct_primary(): defining_declaration_from_type = %p = %s (might not be valid if defining declaration has not been seen yet) \n",
               defining_declaration_from_type,defining_declaration_from_type != NULL ? defining_declaration_from_type->class_name().c_str() : "null");
          printf ("In convert_struct_primary(): declaration_from_type->get_name() = %s \n",declaration_from_type->get_name().str());
          printf ("In convert_struct_primary(): declaration_from_type->get_type() = %p = %s \n",declaration_from_type->get_type(),declaration_from_type->get_type()->class_name().c_str());
          ROSE_ASSERT(declaration_from_type->get_type() != NULL);
          ROSE_ASSERT(declaration_from_type->get_type()->get_declaration() != NULL);
          printf ("In convert_struct_primary(): declaration_from_type->get_type()->get_declaration() = %p = %s \n",
               declaration_from_type->get_type()->get_declaration(),declaration_from_type->get_type()->get_declaration()->class_name().c_str());
          printf ("In convert_struct_primary(): declaration_from_type->get_firstNondefiningDeclaration() = %p \n",declaration_from_type->get_firstNondefiningDeclaration());
          printf ("In convert_struct_primary(): declaration_from_type->get_definingDeclaration()         = %p \n",declaration_from_type->get_definingDeclaration());
#endif
       // DQ (9/8/2012): Make this an error for the moment (verify that we fail as expected on test2004_77.C).
       // ROSE_ASSERT(defining_declaration_from_type == NULL);

       // definingDeclaration = defining_declaration_from_type;
          if (buildTemplateInstantiation == true)
             {
            // We want to build a template instantiation, so if this is a SgTemplateClassDeclaration, then we don't want it's defining declaration.
               ROSE_ASSERT(buildTemplateDeclaration == false);
               if (isSgTemplateClassDeclaration(defining_declaration_from_type) == NULL)
                  {
                    if (defining_declaration_from_type != NULL && nameWithTemplateArguments == defining_declaration_from_type->get_name())
                       {
#if 0
                         printf ("Setting the definingDeclaration to defining_declaration_from_type = %p \n",defining_declaration_from_type);
#endif

                         SgName tmp_name = defining_declaration_from_type->get_name();
#if 0
                         printf ("Check if this has the same name: defining_declaration_from_type->get_name() = %s \n",tmp_name.str());
                         printf ("   ---------------------------------------------- nameWithTemplateArguments = %s \n",nameWithTemplateArguments.str());
#endif
                      // definingDeclaration = defining_declaration_from_type;
                         if (tmp_name == nameWithTemplateArguments)
                           {
                             definingDeclaration = defining_declaration_from_type;
                           }
                          else
                           {
                             printf ("############## In convert_struct_primary(): Skipping the use of defining_declaration_from_type = %p \n",defining_declaration_from_type);
                           }
                       }
                      else
                       {
#if DEBUG_CONVERT_STRUCT_PRIMARY
                         printf ("In convert_struct_primary(): Non-matching names: nameWithTemplateArguments = %s defining_declaration_from_type->get_name() = %s \n",
                              nameWithTemplateArguments.str(),defining_declaration_from_type != NULL ? defining_declaration_from_type->get_name().str() : "null");
                         printf ("In convert_struct_primary(): Names of template instantiations didn't match so build a new templae instantiation. \n");
#endif
                         definingDeclaration = NULL;
                       }
                  }
                 else
                  {
                 // If we only found the template declaration then set this to NULL so that we will build the template instantation that we need.
                    definingDeclaration = NULL;
                  }
             }
            else
             {
            // If this is a non-template declaration and non-template instantiation, then we use the defining declaration (if it is non-null).
               if (buildTemplateDeclaration == false)
                  {
#if DEBUG_CONVERT_STRUCT_PRIMARY
                    printf ("Setting the definingDeclaration to defining_declaration_from_type = %p \n",defining_declaration_from_type);
#endif
                    definingDeclaration = defining_declaration_from_type;
                  }
                 else
                  {
                 // If we only found the template declaration then set this to NULL so that we will build the template instantation that we need.
#if DEBUG_CONVERT_STRUCT_PRIMARY
                    printf ("NOTE: If we only found the template declaration then set this to NULL so that we will build the template instantation that we need. \n");
                    printf ("***** But we might want to avoid having a non-template declaration be generated in this case! \n");
#endif
                    definingDeclaration = NULL;

                  }
             }
        }
       else
        {
#if DEBUG_CONVERT_STRUCT_PRIMARY
          printf ("In convert_struct_primary(): edg_type p = %p has NOT been processed and is NOT in type_translation_cache \n",p);
#endif
        }

  // We need to ignore the pointer tests if this is a reused defining declaration (which has already been attached to the AST and assigned a parent).
     bool reusedDefiningDeclaration = (definingDeclaration != NULL);

#if DEBUG_CONVERT_STRUCT_PRIMARY
     printf ("In convert_struct_primary(): reusedDefiningDeclaration = %s \n",reusedDefiningDeclaration ? "true" : "false");
#endif

  // SgClassDeclaration* decl = set_file_info_for_corresp(buildClassDeclaration_nfi(name, t, topScopeStack(),    nonDefiningDecl), p->source_corresp);
  // SgClassDeclaration* decl = set_file_info_for_corresp(buildClassDeclaration_nfi(name, t, scopeOfDeclaration, nonDefiningDecl), p->source_corresp);
  // SgClassDeclaration* decl = set_file_info_for_corresp(buildClassDeclaration_nfi(nameWithTemplateArguments, t, scopeOfDeclaration, nonDefiningDecl, buildTemplateInstantiation), p->source_corresp);
  // SgClassDeclaration* decl = buildClassDeclaration_nfi(nameWithTemplateArguments, t, scopeOfDeclaration, nonDefiningDecl, buildTemplateInstantiation);
  // SgClassDeclaration* decl = buildClassDeclaration_nfi(nameWithTemplateArguments, t, scopeOfDeclaration, nonDefiningDecl, buildTemplateInstantiation, &templateArgumentsList);
  // SgClassDeclaration* decl = buildClassDeclaration_nfi(nameWithOutTemplateArguments, t, scopeOfDeclaration, nonDefiningDecl, buildTemplateInstantiation, &templateArgumentsList);
     SgClassDeclaration* decl = NULL;
     if (definingDeclaration == NULL)
        {
#if 0
          printf ("In convert_struct_primary(): There is no existing defining declaration, so we need to build one! nonDefiningDecl = %p \n",nonDefiningDecl);
#endif

       // DQ (6/8/2013): If this is true then we have some sort of problem (see test2013_198.C).
          if (nonDefiningDecl != NULL && nonDefiningDecl->get_definingDeclaration() != NULL)
             {
               printf ("Error: the nondefining declaration = %p = %s has get_definingDeclaration() != NULL \n",nonDefiningDecl,nonDefiningDecl->class_name().c_str());
               ROSE_ASSERT(false);
             }

       // DQ (10/11/2015): Trying to enforce that template class declarations not be mixed with non-template class declarations
          if (isSgTemplateClassDeclaration(nonDefiningDecl) != NULL)
             {
               printf ("WARNING: nonDefiningDecl is a SgTemplateClassDeclaration, we should be calling buildTemplateClassDeclaration_nfi() instead of buildClassDeclaration_nfi() \n");
             }
#if 0
       // DQ (12/27/2018): Adding more testing.
          if (nonDefiningDecl == NULL)
             {
               printf ("NOTE: expecting that the nondefining declaration is a valid pointer: nonDefiningDecl = %p \n",nonDefiningDecl);
             }

          printf ("In convert_struct_primary(): calling buildClassDeclaration_nfi(): templateArgumentsList.size() = %zu \n",templateArgumentsList.size());
#endif

          decl = buildClassDeclaration_nfi(nameWithOutTemplateArguments, t, scopeOfDeclaration, nonDefiningDecl, buildTemplateInstantiation, &templateArgumentsList);
          ROSE_ASSERT(decl != NULL);
#if 0
          printf ("In convert_struct_primary(): After calling buildClassDeclaration_nfi(): decl = %p nonDefiningDecl = %p \n",decl,nonDefiningDecl);
#endif
       // DQ (1/22/2013): This is failing for some Boost code used in ROSE compiling ROSE.  In this case the defining declaration 
       // is reused (so the parent had alread been set).
       // DQ (10/15/2012): Added new test. This fails for test2012_231.C (where we have a class declared in both the true and falue 
       // branches of an "if" statement without SgBasicBlocks defining new explicit scopes, namely the two definig declarations are 
       // in the same scope.  Likely one should be using a mangled name includung "TRUE_BRANCH" and the other "FALSE_BRANCH".)
       // ROSE_ASSERT(decl->get_parent() == NULL);
          if (decl->get_parent() != NULL)
             {
#ifdef DEBUG_CONVERSION
               printf ("In convert_struct_primary(): Where the decl->get_parent() != NULL, the defining declaration appears to have been reused: decl = %p = %s \n",decl,decl->class_name().c_str());
#endif
             }
       // DQ (6/8/2013): Added as part of testing for test2013_198.C.
       // ROSE_ASSERT(decl->get_parent() == NULL);

       // DQ (7/31/2019): This should have been built by buildClassDeclaration_nfi() if it didn't exist.
          ROSE_ASSERT(decl->get_firstNondefiningDeclaration() != NULL);

       // DQ (7/31/2019): This should have been built by buildClassDeclaration_nfi() if it didn't exist.
       // ROSE_ASSERT(decl->get_parent() != NULL);
#if 0
          printf ("In convert_struct_primary(): After calling buildClassDeclaration_nfi(): decl->get_parent() = %p \n",decl->get_parent());
#endif
       // DQ (7/31/2019): Test this since we want it to have been set to the scope if it was only used in a symbol.
       // ROSE_ASSERT(decl->get_firstNondefiningDeclaration()->get_parent() != NULL);
        }
       else
        {
#if DEBUG_CONVERT_STRUCT_PRIMARY
          printf ("In convert_struct_primary(): We have detected that there exists a previously built defining declaration so we need to use it. definingDeclaration = %p name = %s \n",
               definingDeclaration,definingDeclaration->get_name().str());
          printf ("In convert_struct_primary(): This borrowed defining declaration has scope = %p = %s \n",definingDeclaration->get_scope(),definingDeclaration->get_scope()->class_name().c_str());
          printf ("In convert_struct_primary(): definingDeclaration->get_parent() = %p \n",definingDeclaration->get_parent());
#endif
          decl = definingDeclaration;
          ROSE_ASSERT(decl != NULL);
        }

  // DQ (9/13/2012): Added testing.
     SageBuilder::testTemplateArgumentParents(decl);

     ROSE_ASSERT(decl != NULL);

#if DEBUG_CONVERT_STRUCT_PRIMARY
     printf ("DONE: In convert_struct_primary(): Calling buildClassDeclaration_nfi() to build defining declaration (will generate a non-defining declaration if nonDefiningDecl == NULL) decl = %p = %s \n",decl,decl->class_name().c_str());

     printf ("In convert_struct_primary(): decl             = %p = %s \n",decl,decl->class_name().c_str());
     printf ("  --- decl->get_name()                        = %s \n",decl->get_name().str());
     printf ("  --- decl->get_type()                        = %p \n",decl->get_type());
     ROSE_ASSERT(decl->get_type() != NULL);
     SgClassType* classTypeFromDeclaration        = isSgClassType(decl->get_type());
     ROSE_ASSERT(classTypeFromDeclaration != NULL);
     SgClassDeclaration* classDeclarationFromType = isSgClassDeclaration(classTypeFromDeclaration->get_declaration());
     ROSE_ASSERT(classDeclarationFromType != NULL);
     printf ("   --- name from declaration from type = %s \n",classDeclarationFromType->get_name().str());

     printf ("  --- decl->get_firstNondefiningDeclaration() = %p \n",decl->get_firstNondefiningDeclaration());
     if (decl->get_firstNondefiningDeclaration() != NULL)
        {
          SgClassDeclaration* temp_classDeclaration = isSgClassDeclaration(decl->get_firstNondefiningDeclaration());
          ROSE_ASSERT(temp_classDeclaration != NULL);
          printf ("  --- decl->get_firstNondefiningDeclaration()->get_name() = %s \n",temp_classDeclaration->get_name().str());

          SgScopeStatement* associatedNondefiningDeclarationScope = temp_classDeclaration->get_scope();
          printf ("  --- associatedNondefiningDeclarationScope = %p = %s \n",associatedNondefiningDeclarationScope,associatedNondefiningDeclarationScope->class_name().c_str());
        }
     printf ("  --- decl->get_definingDeclaration()         = %p \n",decl->get_definingDeclaration());
     if (decl->get_definingDeclaration() != NULL)
        {
          SgClassDeclaration* temp_classDeclaration = isSgClassDeclaration(decl->get_definingDeclaration());
          ROSE_ASSERT(temp_classDeclaration != NULL);
          printf ("  --- decl->get_definingDeclaration()->get_name() = %s \n",temp_classDeclaration->get_name().str());

          SgScopeStatement* associatedDefiningDeclarationScope = temp_classDeclaration->get_scope();
          printf ("  --- associatedDefiningDeclarationScope = %p = %s \n",associatedDefiningDeclarationScope,associatedDefiningDeclarationScope->class_name().c_str());
        }
     printf ("  --- scopeOfDeclaration                      = %p = %s \n",scopeOfDeclaration,scopeOfDeclaration->class_name().c_str());
     printf ("  --- nameWithOutTemplateArguments = %s \n",nameWithOutTemplateArguments.str());
     printf ("  --- nameWithTemplateArguments    = %s \n",nameWithTemplateArguments.str());
#endif

  // DQ (3/5/2018): I just noticed an example (Cxx11_tests/test2015_08.C) where the names are different.
  //    allocator_traits < allocator< uint_least32_t  >  > 
  // vs.
  //    allocator_traits < allocator< const uint_least32_t  >  >
  // So we need to test for this.
     if (decl->get_firstNondefiningDeclaration() != NULL && decl->get_definingDeclaration() != NULL)
        {
          SgClassDeclaration* temp_firstnondefining_classDeclaration = isSgClassDeclaration(decl->get_firstNondefiningDeclaration());
          SgClassDeclaration* temp_defining_classDeclaration         = isSgClassDeclaration(decl->get_definingDeclaration());
#if 0
       // DQ (8/3/2019): Output debugging info for failing test: test2019_16a.c and test2019_16b.c read in this order as two files on the command line.
          printf ("temp_firstnondefining_classDeclaration->get_type() = %p = %s = %s \n",temp_firstnondefining_classDeclaration->get_type(),
               temp_firstnondefining_classDeclaration->get_type()->class_name().c_str(),temp_firstnondefining_classDeclaration->get_type()->unparseToString().c_str());
          printf ("temp_defining_classDeclaration->get_type()         = %p = %s = %s \n",temp_defining_classDeclaration->get_type(),
               temp_defining_classDeclaration->get_type()->class_name().c_str(),temp_defining_classDeclaration->get_type()->unparseToString().c_str());
#endif
          ROSE_ASSERT(temp_firstnondefining_classDeclaration->get_name() == temp_defining_classDeclaration->get_name());

       // DQ (7/6/2019): Consider changing the scopeOfDeclaration to the the scope of the declaration instead of that computed 
       // from the topScopeStack(), as returned from getDeclarationScope().  This problem is caused by the implementation of
       // the getDeclarationScope to test for C language use and then return the topScopeStack() result.  This is new semantics
       // that is better than returning the SgGlobal scope (which was the previous behavior), but it complicated by the new
       // support for conditionals and declarations in conditionals that are initially processed with one value returned from 
       // the topScopeStack() function, and then reprocessed after the declarations in the conditional to translate the 
       // associated expressions within the conditional.

          SgScopeStatement* originalDeclarationScope = temp_firstnondefining_classDeclaration->get_scope();

       // DQ (8/3/2019): Adding debugging information (output).
          SgScopeStatement* temp_defining_classDeclaration_scope = temp_defining_classDeclaration->get_scope();
#if 0
       // DQ (8/3/2019): Output debugging info for failing test: test2019_16a.c and test2019_16b.c read in this order as two files on the command line.
          if (temp_defining_classDeclaration_scope != originalDeclarationScope)
             {
               printf ("Error: temp_defining_classDeclaration_scope != originalDeclarationScope: \n");
               printf (" --- originalDeclarationScope = %p = %s \n",
                    originalDeclarationScope,originalDeclarationScope->class_name().c_str());
               printf (" --- temp_defining_classDeclaration_scope = %p = %s \n",
                    temp_defining_classDeclaration_scope,temp_defining_classDeclaration_scope->class_name().c_str());
             }
#endif
       // DQ (8/3/2019): This fails for test2019_16a.c and test2019_16b.c read in this order as two files on the command line.
       // The result is that there are two global scopes (one for each translation unit).
       // ROSE_ASSERT(temp_defining_classDeclaration->get_scope() == originalDeclarationScope);
          SgGlobal* originalDeclarationGlobalScope       = isSgGlobal(originalDeclarationScope);
          SgGlobal* temp_defining_DeclarationGlobalScope = isSgGlobal(temp_defining_classDeclaration_scope);
          if (originalDeclarationGlobalScope != NULL && temp_defining_DeclarationGlobalScope != NULL && temp_defining_classDeclaration_scope != originalDeclarationScope)
             {
            // This is OK to have two different global scopes within multiple file support.
             }
            else
             {
            // DQ (12/22/2019): Now the we want to avoid sharing the class declaration across two files, I think this asertion
            // is no longer valid.  Debugging test2019_510a.C and test2019_510b.C used on the same command line.
            // ROSE_ASSERT(temp_defining_classDeclaration_scope == originalDeclarationScope);
#if 0
               if (temp_defining_classDeclaration_scope != originalDeclarationScope)
                  {
                 // This is only output in the case of multiple files on the command line reading the same class declarations.
                    printf ("In convert_struct_primary(): Commented out assertion: temp_defining_classDeclaration_scope == originalDeclarationScope (avoiding sharing across multiple files) \n");
                  }
#endif
             }
#if 0
          printf ("Resetting the scopeOfDeclaration to be the original scope used to build the defining and non-defining declarations: originalDeclarationScope = %p = %s \n",
               originalDeclarationScope,originalDeclarationScope->class_name().c_str());
#endif
          scopeOfDeclaration = originalDeclarationScope;
#if 0
          printf ("Exiting as a test! \n");
          ROSE_ASSERT(false);
#endif
        }

#if 0
     printf ("In convert_struct_primary(): (part 1) Calling retrieveAssociatedClassSymbol(nameWithOutTemplateArguments = %s, buildTemplateDeclaration = %s) scopeOfDeclaration = %p = %s \n",
             nameWithOutTemplateArguments.str(),buildTemplateDeclaration ? "true" : "false",scopeOfDeclaration,scopeOfDeclaration->class_name().c_str());
#endif

  // SgSymbol* debug_tmp_symbol = retrieveAssociatedClassSymbol(nameWithTemplateArguments,scopeOfDeclaration,buildTemplateDeclaration,&templateArgumentsList,&templateParameterList,&templateSpecializationArgumentList);
     SgSymbol* debug_tmp_symbol = retrieveAssociatedClassSymbol(nameWithOutTemplateArguments,scopeOfDeclaration,buildTemplateDeclaration,&templateArgumentsList,NULL,&templateSpecializationArgumentList);
  // DQ (9/8/2012): Test this (case below is failing for test2005_133.C).
#if 0
     if (debug_tmp_symbol == NULL)
        {
          printf ("Output the symbol table for this scope = %p = %s \n",scopeOfDeclaration,scopeOfDeclaration->class_name().c_str());
          scopeOfDeclaration->get_symbol_table()->print("Error: In convert_struct_primary()");
        }
#endif

  // DQ (1/29/2013): This fails for ROSE compiling ROSE (and nothing else so far).
  // ROSE_ASSERT(debug_tmp_symbol != NULL);
     if (debug_tmp_symbol == NULL)
        {
          printf ("WARNING: In convert_struct_primary(): commented out assertion of debug_tmp_symbol != NULL buildTemplateDeclaration = %s nameWithOutTemplateArguments = %s \n",
               buildTemplateDeclaration ? "true" : "false",nameWithOutTemplateArguments.str());
#if 0
          printf ("Output the symbols in the symbol table: \n");
          scopeOfDeclaration->print_symboltable("In convert_struct_primary()");
#endif
        }
     ROSE_ASSERT(debug_tmp_symbol != NULL);

  // DQ (6/9/2013): Adding debugging code.
     SgClassSymbol* debug_tmp_class_symbol = isSgClassSymbol(debug_tmp_symbol);
     ROSE_ASSERT(debug_tmp_class_symbol != NULL);
     ROSE_ASSERT(debug_tmp_class_symbol->get_declaration() != NULL);
  // DQ (6/9/2013): The declaration in the symbol should be a non-defining declaration.
     ROSE_ASSERT(debug_tmp_class_symbol->get_declaration()->get_definition() == NULL);

#if 0
     printf ("In convert_struct_primary(): debug_tmp_class_symbol = %p = %s debug_tmp_class_symbol->get_declaration() = %p = %s = %s \n",
          debug_tmp_class_symbol,debug_tmp_class_symbol->class_name().c_str(),debug_tmp_class_symbol->get_declaration(),
          debug_tmp_class_symbol->get_declaration()->class_name().c_str(),SageInterface::get_name(debug_tmp_class_symbol->get_declaration()).c_str());
#endif

#if 0
     scopeOfDeclaration->get_symbol_table()->print("Symbol table in convert_struct_primary()");
#endif

  // DQ (9/7/2012): Added assertions.
     ROSE_ASSERT(decl->get_firstNondefiningDeclaration() != NULL);
     ROSE_ASSERT(decl->get_definingDeclaration() != NULL);
     ROSE_ASSERT(decl->get_firstNondefiningDeclaration()->get_definingDeclaration() == decl);

  // DQ (9/7/2012): Added more assertions.
     ROSE_ASSERT(decl->get_definingDeclaration() == decl);
     ROSE_ASSERT(decl->get_definingDeclaration()->get_firstNondefiningDeclaration() == decl->get_firstNondefiningDeclaration());
     ROSE_ASSERT(decl->get_firstNondefiningDeclaration()->get_firstNondefiningDeclaration() == decl->get_firstNondefiningDeclaration());

  // DQ (9/8/2012): Fix this to allow the defining declaration to be reused (where the parent pointer may have been previously set).
  // DQ (7/12/2012): I think this is always true (to support the case of a non-autonomous class 
  // declaration (type defined in a type reference (e.g. nested in a variable or typedef declaration)).
  // ROSE_ASSERT(decl->get_parent() == NULL);
     if (decl->get_parent() != NULL && reusedDefiningDeclaration == false)
        {
#ifdef DEBUG_CONVERSION
          printf ("ERROR: In convert_struct_primary(): decl->get_parent() = %p reusedDefiningDeclaration = %s \n",decl->get_parent(),reusedDefiningDeclaration ? "true" : "false");
#endif
        }

  // DQ (1/24/2013): Commented out as part of testing ROSE compiling ROSE.
  // ROSE_ASSERT(decl->get_parent() == NULL || reusedDefiningDeclaration);

  // if (isSgTemplateInstantiationDecl(decl) != NULL || isSgTemplateInstantiationFunctionDecl(decl) != NULL || isSgTemplateInstantiationMemberFunctionDecl(decl) != NULL)
     if (isSgTemplateInstantiationDecl(decl) != NULL)
        {
#if 0
          printf ("This is a SgTemplateInstantiationDecl: decl = %p = %s \n",decl,decl->class_name().c_str());
#endif
#if 0
          printf ("Calling summarize_source_correspondence() in convert_struct_primary() \n");
          summarize_source_correspondence(p->source_corresp);
#endif

       // DQ (5/2/2012): Instead of setting this as compiler generated without also providing any source position infomation, 
       // instead set it using the source positon of the original template and additionally mark it as compiler generated.
          set_file_info_for_corresp(decl,p->source_corresp);

       // DQ (5/15/2012): Added assertion (this should ahve been built by the builder function.
          ROSE_ASSERT(decl->get_firstNondefiningDeclaration() != NULL);
          set_file_info_for_corresp(decl->get_firstNondefiningDeclaration(),p->source_corresp);
       // decl->get_firstNondefiningDeclaration()->get_file_info()->setCompilerGenerated();

       // DQ (8/7/2012): Mark classes as specialized (if so and not compiler generated)
       // decl->get_file_info()->setCompilerGenerated();
          if (p->variant.class_struct_union.is_specialized == true)
             {
#if 0
               printf ("Mark this class as specialized (and not compiler generated) decl = %p \n",decl);
#endif
               decl->set_specialization(SgDeclarationStatement::e_specialization);
            // decl->get_firstNondefiningDeclaration()->set_specialization(SgDeclarationStatement::e_specialization);
               isSgClassDeclaration(decl->get_firstNondefiningDeclaration())->set_specialization(SgDeclarationStatement::e_specialization);

            // Verify that if this is specialized, that it is not marked as compiler generated.
               ROSE_ASSERT(decl->get_file_info()->isCompilerGenerated() == false);
             }
            else
             {
               decl->get_file_info()->setCompilerGenerated();
               decl->get_firstNondefiningDeclaration()->get_file_info()->setCompilerGenerated();
             }

#if 0
          decl->get_file_info()->display("In convert_struct_primary(): decl debug");
          decl->get_firstNondefiningDeclaration()->get_file_info()->display("In convert_struct_primary(): decl->get_firstNondefiningDeclaration() debug");
#endif
       // printf ("Error: Template instantiations should not have a source position so we want to investigate this further. \n");
       // ROSE_ASSERT(false);
        }
       else
        {
       // DQ (4/29/2012): This can be a compiler generated class for a function template instantiation 
       // (It will not be a template class instantiation (thought I think it should be) but it should 
       // be marked as a compiler generated IR node). See test 2012_51.C for an example of this case.
#if 0
       // DQ (2/26/2019): This logic has ben moved into the resetSourcePostion() function (called from 
       // in set_file_info_for_corresp()).
       // DQ (2/26/2019): If this is a declaration that has been reused from another file as part of 
       // multiple file handling, then it will have a valid source position already.
          bool containsSourcePositionInformationFromAlternatedFile = false;
          if (decl->get_file_info() != NULL)
             {
               decl->get_file_info()->display("Before being reset to source position information from current file");
               containsSourcePositionInformationFromAlternatedFile = true;
             }
#endif
          decl = set_file_info_for_corresp(decl,p->source_corresp);

       // DQ (4/29/2012): This is code from the older version of the EDG translation (special case).
       // However, I would prefer to handle this as a template instantation (instead of a class 
       // declaration from a template (which seems redundant).
       // DQ (12/18/2005): If this is a nested class in a template then we have to mark it as such!
          if (p->variant.class_struct_union.is_template_class == true)
             {
            // Set both of the class declarations (defining and non-defining as being from a template.
            // This means that the class is a non-template class defined inside of a template (function 
            // or class template).
               decl->set_from_template(true);
               decl->setCompilerGenerated();

               if (nonDefiningDecl != NULL)
                  {
                    nonDefiningDecl->set_from_template(true);
                    nonDefiningDecl->setCompilerGenerated();
                  }
#if 0
            // DQ (4/29/2012): Added debugging code.
               printf ("Reset the source position information for decl (defining Class Declaration) = %p \n",decl);
               decl->get_file_info()->display("type->variant.class_struct_union.is_template_class == true");
#endif
             }
#if 0
       // DQ (2/26/2019): If this is a declaration that has been reused from another file as part of 
       // multiple file handling, then it will have a valid source position already.
          if (containsSourcePositionInformationFromAlternatedFile == true)
             {
               ROSE_ASSERT(decl->get_file_info() != NULL);
               decl->get_file_info()->display("After being reset to source position information from current file");
             }
#endif
        }

     ROSE_ASSERT(decl != NULL);

  // DQ (3/5/2012): Make sure that for a template instantiation the class definition is a SgTemplateInstantiationDefn.
     if (buildTemplateInstantiation == true)
        {
          ROSE_ASSERT(decl->get_definition() != NULL);
          ROSE_ASSERT(isSgTemplateInstantiationDefn(decl->get_definition()) != NULL);
        }

  // DQ (3/3/2012): Commented out this test (to allow us to set the parent correctly later).
#if 0
     printf ("In convert_struct_primary(): Commented out test for decl->get_parent() != NULL, decl->get_parent() = %p \n",decl->get_parent());
#endif

  // DQ (2/29/2012): Added test for non-null parent.
  // ROSE_ASSERT(decl->get_parent() != NULL);

  // DQ (9/8/2012): Fix this to allow the defining declaration to be reused (where the parent pointer may have been previously set).
  // DQ (3/20/2012): At this point the parent should still be NULL.
  // ROSE_ASSERT(decl->get_parent() == NULL);

  // DQ (1/24/2013): Added debugging code.
  // ROSE_ASSERT(decl->get_parent() == NULL || reusedDefiningDeclaration);
     if (decl->get_parent() != NULL && reusedDefiningDeclaration == false)
        {
#ifdef DEBUG_CONVERSION
          printf ("ERROR: In convert_struct_primary(): decl->get_parent() = %p reusedDefiningDeclaration = %s \n",decl->get_parent(),reusedDefiningDeclaration ? "true" : "false");
#endif
        }

  // DQ (1/24/2013): Commented out as part of testing ROSE compiling ROSE.
  // ROSE_ASSERT(decl->get_parent() == NULL || reusedDefiningDeclaration);

  // DQ (3/20/2012): And at this point the scope should have been set.
     ROSE_ASSERT(decl->get_scope() != NULL);

     decl->set_forward(false);

#if 0
     printf ("In convert_struct_primary(): buildClassDeclaration_nfi() returned decl = %p = %s \n",decl,decl->class_name().c_str());
#endif

     ROSE_ASSERT(decl->get_firstNondefiningDeclaration() != NULL);

#if 0
     printf ("In convert_struct_primary(): decl->get_firstNondefiningDeclaration()   = %p = %s \n",decl->get_firstNondefiningDeclaration(),decl->get_firstNondefiningDeclaration()->class_name().c_str());
#endif

     ROSE_ASSERT(decl->get_definingDeclaration() != NULL);

#if 0
     printf ("In convert_struct_primary(): decl->get_definingDeclaration()           = %p = %s \n",decl->get_definingDeclaration(),decl->get_definingDeclaration()->class_name().c_str());
#endif

  // Make sure that these are different IR nodes.
     ROSE_ASSERT(decl->get_firstNondefiningDeclaration() != decl);

     if (buildTemplateInstantiation == true)
        {
       // We need to enforce that the first non-defining declaration is constant type of IR node with the decl.
          ROSE_ASSERT(isSgTemplateInstantiationDecl(decl) != NULL);
          ROSE_ASSERT(isSgTemplateInstantiationDecl(decl->get_firstNondefiningDeclaration()) != NULL);

       // DQ (3/27/2012): Normalize the names so that they can be used for code generation.
          normalizeTemplateName(nameWithOutTemplateArguments);

       // DQ (9/12/2012): We don't want to be setting the template name here (should be set by SageBuilder functions.
#if 0
          printf ("WARNING: In convert_struct_primary(): setting templateName for decl = %p (should already be set) \n",decl);
#endif
          ROSE_ASSERT(isSgTemplateInstantiationDecl(decl)->get_templateName().is_null() == false);

          isSgTemplateInstantiationDecl(decl)->set_templateName(nameWithOutTemplateArguments);
          isSgTemplateInstantiationDecl(decl->get_firstNondefiningDeclaration())->set_templateName(nameWithOutTemplateArguments);

       // DQ (5/31/2012): Find locations where this is set and include template syntax.
       // ROSE_ASSERT(nameWithOutTemplateArguments.getString().find('<') == string::npos);
          ROSE_ASSERT(hasTemplateSyntax(nameWithOutTemplateArguments) == false);

       // Error checking.
          string test_name_1 = nameWithTemplateArguments.str();
          string test_name_2 = isSgTemplateInstantiationDecl(decl)->get_templateName().str();
       // ROSE_ASSERT(test_name_1.find('<') != string::npos);
       // ROSE_ASSERT(test_name_1.find('>') != string::npos);
       // ROSE_ASSERT(test_name_2.find('<') == string::npos);
       // ROSE_ASSERT(test_name_2.find('>') == string::npos);

#if 0
       // DQ (5/8/2013): This fails for test2013_159.C, and it appears that we have been overly restrictive here.
          if (hasTemplateSyntax(test_name_1) == false)
             {
               printf ("WARNING: In convert_struct_primary(): test_name_1 = %s is not using template syntax \n",test_name_1.c_str());
             }
#endif
       // ROSE_ASSERT(hasTemplateSyntax(test_name_1) == true);
          ROSE_ASSERT(hasTemplateSyntax(test_name_2) == false);

          ROSE_ASSERT(isSgTemplateInstantiationDecl(decl)->get_templateName() == isSgTemplateInstantiationDecl(decl->get_firstNondefiningDeclaration())->get_templateName());
        }

     ROSE_ASSERT(decl->get_scope() != NULL);
     ROSE_ASSERT(decl->get_type() != NULL);

  // Test for the associated template declaration in the SgTemplateInstantiationDecl
     if (buildTemplateInstantiation == true)
        {
          SgTemplateInstantiationDecl* definingTemplateInstantiation    = isSgTemplateInstantiationDecl(decl);
          SgTemplateInstantiationDecl* nondefiningTemplateInstantiation = isSgTemplateInstantiationDecl(decl->get_firstNondefiningDeclaration());
#if 0
          printf ("In convert_struct_primary(): definingTemplateInstantiation = %s \n",definingTemplateInstantiation ? "true" : "false");
#endif
          ROSE_ASSERT(definingTemplateInstantiation != NULL);
          ROSE_ASSERT(nondefiningTemplateInstantiation != NULL);

          if (definingTemplateInstantiation->get_templateDeclaration() == NULL)
             {
            // Make a note, since this is an opportunity to improve the build interface to take the 
            // SgTemplateDeclaration directly (but we need to think about this a bit more).
#if 0
               printf ("Note: In convert_struct_primary(): Setting the templateDeclaration in the definingTemplateInstantiation after construction \n");
#endif
#if 0
            // DQ (1/24/2013): Added debug info.
               printf ("In convert_struct_primary(): definingTemplateInstantiation                                    = %p = %s \n",definingTemplateInstantiation,definingTemplateInstantiation->class_name().c_str());
               printf ("In convert_struct_primary(): definingTemplateInstantiation->get_firstNondefiningDeclaration() = %p = %s \n",definingTemplateInstantiation->get_firstNondefiningDeclaration(),
                    (definingTemplateInstantiation->get_firstNondefiningDeclaration() != NULL) ? definingTemplateInstantiation->get_firstNondefiningDeclaration()->class_name().c_str() : "null");
               printf ("In convert_struct_primary(): definingTemplateInstantiation->get_definingDeclaration()         = %p = %s \n",definingTemplateInstantiation->get_definingDeclaration(),
                    (definingTemplateInstantiation->get_definingDeclaration() != NULL) ? definingTemplateInstantiation->get_definingDeclaration()->class_name().c_str() : "null");
#endif

            // DQ (1/24/2013): Check that this is the same for the nondefining declaration if it is available.
               if (nondefiningTemplateInstantiation != NULL)
                  {
                 // ROSE_ASSERT(nondefiningTemplateInstantiation->get_templateDeclaration() == NULL);
                    SgTemplateClassDeclaration* templateClassDeclaration = nondefiningTemplateInstantiation->get_templateDeclaration();
#if 0
                    printf ("nondefiningTemplateInstantiation->get_templateDeclaration() = %p \n",templateClassDeclaration);
#endif
                    if (templateClassDeclaration != NULL)
                       {
#if 0
                         printf ("nondefiningTemplateInstantiation->get_templateDeclaration() = %p = %s \n",templateClassDeclaration,
                              templateClassDeclaration->class_name().c_str());
                         templateClassDeclaration->get_file_info()->display("nondefiningTemplateInstantiation->get_templateDeclaration(): debug");
#endif
                      // Check if this is the defining declaration for the template.
                         if (templateClassDeclaration->get_definingDeclaration() == templateClassDeclaration)
                            {
#if 0
                              printf ("WARNING: The nondefiningTemplateInstantiation = %p references the defining declaration for the template declaration = %p \n",
                                   nondefiningTemplateInstantiation,templateClassDeclaration);
#endif
                              definingTemplateInstantiation->set_templateDeclaration(templateClassDeclaration);

                           // DQ (1/24/2013): Added test for matching variants.
                              ROSE_ASSERT(matching_defining_and_nondefining_variants(templateClassDeclaration) == true);
                              ROSE_ASSERT(matching_defining_and_nondefining_variants(definingTemplateInstantiation) == true);
                            }
                       }

                 // ROSE_ASSERT(nondefiningTemplateInstantiation->get_templateDeclaration() == NULL);
                  }

            // DQ (1/24/2013): Check if it is still NULL, if so use the getTemplateDeclaration() function.
               if (definingTemplateInstantiation->get_templateDeclaration() == NULL)
                  {
#if 0
                 // SgTemplateDeclaration* templateDeclaration = getTemplateDeclaration(p);
                    SgTemplateClassDeclaration* templateDeclaration = getTemplateDeclaration(p);
#error "DEAD CODE!"
                    ROSE_ASSERT(templateDeclaration != NULL);
                    definingTemplateInstantiation->set_templateDeclaration(templateDeclaration);
#else
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
                         SgTemplateInstantiationDecl* firstNondefiningTemplateInstantiationDeclaration = isSgTemplateInstantiationDecl(definingTemplateInstantiation->get_firstNondefiningDeclaration());
                         ROSE_ASSERT(firstNondefiningTemplateInstantiationDeclaration != NULL);
                         templateDeclaration = firstNondefiningTemplateInstantiationDeclaration->get_templateDeclaration();
                         ROSE_ASSERT(templateDeclaration != NULL);
                         definingTemplateInstantiation->set_templateDeclaration(templateDeclaration);
                       }
                      else
                       {
                         SgDeclarationStatement * decl = getTemplateDeclaration(p);
                         templateDeclaration = isSgTemplateClassDeclaration(decl);
                         ROSE_ASSERT(templateDeclaration != NULL);
                         definingTemplateInstantiation->set_templateDeclaration(templateDeclaration);
                       }

                    ROSE_ASSERT(templateDeclaration != NULL);
#endif
                 // DQ (3/3/2012): Added test for matching variants.
                    ROSE_ASSERT(matching_defining_and_nondefining_variants(templateDeclaration) == true);
                    ROSE_ASSERT(matching_defining_and_nondefining_variants(definingTemplateInstantiation) == true);
                  }
             }
          ROSE_ASSERT(definingTemplateInstantiation->get_templateDeclaration() != NULL);

          if (nondefiningTemplateInstantiation->get_templateDeclaration() == NULL)
             {
            // The call to getTemplateDeclaration(p) could be refactored, I think.
               SgDeclarationStatement * decl = getTemplateDeclaration(p);
               SgTemplateClassDeclaration* templateDeclaration = isSgTemplateClassDeclaration(decl);
               ROSE_ASSERT(templateDeclaration != NULL);
               nondefiningTemplateInstantiation->set_templateDeclaration(templateDeclaration);

            // DQ (3/3/2012): Added test for matching variants.
               ROSE_ASSERT(matching_defining_and_nondefining_variants(templateDeclaration) == true);
               ROSE_ASSERT(matching_defining_and_nondefining_variants(nondefiningTemplateInstantiation) == true);
             }
          ROSE_ASSERT(nondefiningTemplateInstantiation->get_templateDeclaration() != NULL);

       // Attach an associated template arguments.
       // process_template_arguments(p,nondefiningTemplateInstantiation,templateArgumentsList);

          ROSE_ASSERT(nondefiningTemplateInstantiation->get_templateName().is_null() == false);
          ROSE_ASSERT(isSgTemplateInstantiationDecl(nondefiningTemplateInstantiation->get_firstNondefiningDeclaration())->get_templateName().is_null() == false);
          ROSE_ASSERT(isSgTemplateInstantiationDecl(nondefiningTemplateInstantiation->get_definingDeclaration())->get_templateName().is_null() == false);

       // DQ (1/1/2012): Mark these to be output so that we can debug the unparsing of the template instantiations.
       // printf ("************ convert_struct_primary(): Marking decl for output decl = %p = %s \n",decl,decl->class_name().c_str());
       // nondefiningTemplateInstantiation->get_file_info()->setOutputInCodeGeneration();
       // nondefiningTemplateInstantiation->get_definingDeclaration()->get_file_info()->setOutputInCodeGeneration();
#if 0
          printf ("################  p->variant.class_struct_union.is_specialized = %s \n",p->variant.class_struct_union.is_specialized ? "true" : "false");
#endif
       // DQ (1/2/2012): We only want to mark template instantiations for output when we detect that there are transformations to them.
       // However, the helps to force them to be output which allows for debugging then (as if they had been detected as being transformed).
       // if (p->variant.class_struct_union.is_specialized == true)
       // printf ("Mark this as a template specialization \n");
       // nondefiningTemplateInstantiation->set_specialization(SgDeclarationStatement::e_specialization);
       // definingTemplateInstantiation->set_specialization(SgDeclarationStatement::e_specialization);

       // DQ (9/13/2012): Added testing.
          SageBuilder::testTemplateArgumentParents(nondefiningTemplateInstantiation);
          SageBuilder::testTemplateArgumentParents(definingTemplateInstantiation);
        }

  // DQ (8/12/2014): Adding support for C++11 template type references (test2014_38.C).
  // DQ (11/27/2011): Set the scope of the new defining declaration's SgClassDefinition in the scope_cache.
  // ROSE_ASSERT(p->kind == tk_class || p->kind == tk_struct || p->kind == tk_union);
  // ROSE_ASSERT(p->variant.class_struct_union.extra_info != NULL);
  // a_scope_ptr assoc_scope = p->variant.class_struct_union.extra_info->assoc_scope;
     a_scope_ptr assoc_scope = NULL;
     if (p->kind == tk_class || p->kind == tk_struct || p->kind == tk_union)
        {
          ROSE_ASSERT(p->variant.class_struct_union.extra_info != NULL);

       // DQ (12/28/2011): This is a proper pointer to the scope for the class, it edg scope is a list of the scopes for
       // each routine, but this is not a scope for the first routine in the class.

       // DQ (12/21/2011): I think this is a linked list of scopes associated with the member functions of the class and
       // not anything associated with the class definition scope.  So this is not the scope that we are looking for...
          assoc_scope = p->variant.class_struct_union.extra_info->assoc_scope;
        }

  // ROSE_ASSERT(assoc_scope != NULL);
     if (assoc_scope != NULL)
        {
          SgClassDefinition* classDefinition = decl->get_definition();
          ROSE_ASSERT(classDefinition != NULL);

       // DQ (8/26/2014): Refactored the support to update the scope cache for template declaration 
       // scopes and template instantiations and class declaration scopes.

       // NOTE: update_scope_cache() does not reset the entry if it is already cached.
#if 0
          printf ("In convert_struct_primary(): Call update_scope_cache() with:\n");
          printf (" --- assoc_scope = %p\n", assoc_scope);
          printf (" --- classDefinition = %p (%s)\n", classDefinition, classDefinition->class_name().c_str());
          printf (" --- scope_cache[assoc_scope] = %p\n", scope_cache.find(assoc_scope) != scope_cache.end() ? scope_cache[assoc_scope] : NULL);
#endif
          update_scope_cache(scope_cache,assoc_scope,classDefinition);
          ROSE_ASSERT(scope_cache[assoc_scope] != NULL);
#if 0
          printf (" --- scope_cache[assoc_scope] = %p\n", scope_cache[assoc_scope]);
#endif

       // DQ (4/22/2012): Skip this test now that we don't reset the value.
       // ROSE_ASSERT(classDefinition == scope_cache[assoc_scope]);
#if 0
          printf ("In convert_struct_primary(): Skipped test for classDefinition == scope_cache[assoc_scope] \n");
#endif
        }
       else
        {
       // DQ (12/28/2011): Also failing test2003_27.C
       // DQ (12/11/2011): Added test since we need a valid scope cache ... to build constants in templates...
       // Sometimes the assoc_scope is NULL.  Not clear exactly why... see test2004_35.C.
#ifdef DEBUG_CONVERSION
          printf ("WARNING: In convert_struct_primary(): SgClassDefinition not added to scope_cache[] (assoc_scope == NULL) \n");
#endif
       // ROSE_ASSERT(false);
        }

  // DQ (6/9/2013): Use new function to avoid pushing duplicate SgDeclarationStatements for each type.
  // There may already be a secondary (non-defining) declaration!
  // type_declaration_map[p].push_back(decl);
     push_entry_to_type_declaration_map(p,decl);

  // Note that at this point the SgClassSymbol has been built and is
  // in the symbol table for the scope defined by "topScopeStack()"

     if (is_unnamed(p))
        {
#if 0
          printf ("In convert_struct_primary(): Mark class declaration = %p = %s as UN-NAMED \n",decl,decl->class_name().c_str());
#endif
          decl->set_isUnNamed(true);

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

     SgClassDeclaration* firstNondefiningDecl = NULL; // isSgClassDeclaration(classType->get_declaration());

#if 0
     printf ("In convert_struct_primary(): Looking up the class symbol for nameWithTemplateArguments = %s in scope = %p = %s \n",nameWithTemplateArguments.str(),scopeOfDeclaration,scopeOfDeclaration->class_name().c_str());
#endif

  // DQ (12/26/2012): We are now using a new SgTemplateClass to represent symbols for SgTemplateClassDeclaration.
  // bool buildTemplateDeclaration = false;
     ROSE_ASSERT(buildTemplateDeclaration == false);

  // DQ (9/13/2012): Test before failing point in template argument without valid parent pointer.
     SageBuilder::testTemplateArgumentParents(decl);

#if 0
     printf ("In convert_struct_primary(): Calling retrieveAssociatedClassSymbol with nameWithTemplateArguments = %s (I think maybe it should be nameWithOutTemplateArguments = %s (?)) \n",nameWithTemplateArguments.str(),nameWithOutTemplateArguments.str());
     printf ("In convert_struct_primary(): (part 2) Calling retrieveAssociatedClassSymbol(nameWithOutTemplateArguments = %s, buildTemplateDeclaration = %s) \n",nameWithOutTemplateArguments.str(),buildTemplateDeclaration ? "true" : "false");
#endif

  // DQ (9/6/2012): I think we are correct to use the nameWithTemplateArguments to look up the symbol (and also pass 
  // the templateArgumentsList (needed to resolve names that differ only in the unqualified template arguments).
  // SgSymbol*      tmp_symbol = retrieveAssociatedClassSymbol(nameWithTemplateArguments,scopeOfDeclaration,buildTemplateDeclaration,&templateArgumentsList);
  // SgSymbol*      tmp_symbol = retrieveAssociatedClassSymbol(nameWithTemplateArguments,scopeOfDeclaration,buildTemplateDeclaration,&templateArgumentsList,&templateParameterList,&templateSpecializationArgumentList);
     SgSymbol*      debug_tmp_symbol_2 = retrieveAssociatedClassSymbol(nameWithOutTemplateArguments,scopeOfDeclaration,buildTemplateDeclaration,&templateArgumentsList,NULL,&templateSpecializationArgumentList);

  // DQ (1/29/2013): This fails for ROSE compiling ROSE.
  // DQ (9/8/2012): Test this (case below is failing for test2005_133.C).
  // ROSE_ASSERT(tmp_symbol != NULL);
     if (debug_tmp_symbol_2 == NULL)
        {
          printf ("In convert_struct_primary(): Calling retrieveAssociatedClassSymbol with nameWithTemplateArguments = %s (I think maybe it should be nameWithOutTemplateArguments = %s (?)) \n",nameWithTemplateArguments.str(),nameWithOutTemplateArguments.str());
          printf ("In convert_struct_primary(): (part 2) Calling retrieveAssociatedClassSymbol(nameWithOutTemplateArguments = %s, buildTemplateDeclaration = %s) \n",nameWithOutTemplateArguments.str(),buildTemplateDeclaration ? "true" : "false");
#if 0
       // scopeOfDeclaration->get_symbol_table()->print("In convert_struct_primary()");
          scopeOfDeclaration->print_symboltable("In convert_struct_primary()");
#endif
        }
     ROSE_ASSERT(debug_tmp_symbol_2 != NULL);

  // DQ (6/9/2013): Adding debugging code.
     SgClassSymbol* debug_tmp_class_symbol_2 = isSgClassSymbol(debug_tmp_symbol_2);
     ROSE_ASSERT(debug_tmp_class_symbol_2 != NULL);
     ROSE_ASSERT(debug_tmp_class_symbol_2->get_declaration() != NULL);
  // DQ (6/9/2013): The declaration in the symbol should be a non-defining declaration.
     ROSE_ASSERT(debug_tmp_class_symbol_2->get_declaration()->get_definition() == NULL);

     SgClassSymbol* mysymbol = isSgClassSymbol(debug_tmp_symbol_2);

#if 0
     printf ("In convert_struct_primary(): After looking up the class symbol: mysymbol = %p \n",mysymbol);
#endif

  // DQ (12/27/2011): the if below will enforce that mysymbol != NULL.
     ROSE_ASSERT(mysymbol != NULL);

     if (mysymbol != NULL)
        {
          firstNondefiningDecl = isSgClassDeclaration(mysymbol->get_declaration());
#if 0
          printf ("In convert_struct_primary(): retrieved firstNondefiningDecl from symbol: firstNondefiningDecl = %p = %s \n",firstNondefiningDecl,firstNondefiningDecl->class_name().c_str());
#endif
          ROSE_ASSERT(firstNondefiningDecl != NULL);

       // DQ (9/6/2012): I think we are getting the wrong first non-defining declaration here.
#if 0
          printf ("In convert_struct_primary(): Getting firstNondefiningDecl = %p from symbol = %p \n",firstNondefiningDecl,mysymbol);
#endif
       // DQ (2/27/2012): Test setting the parent correctly (an not just to the current scope).
#if 0
          printf ("*** WARNING: In convert_struct_primary(): Commented out the test of parent to be non-null \n");
#endif
       // ROSE_ASSERT(firstNondefiningDecl->get_parent() != NULL);

       // DQ (9/6/2012): This may be the cause of the problems with haskell tests!
#if 0
          printf ("WARNING: In convert_struct_primary(): Resetting firstNondefiningDecl = %p definingDeclaration (currently %p) to decl = %p \n",firstNondefiningDecl,firstNondefiningDecl->get_definingDeclaration(),decl);
#endif
       // DQ (9/7/2012): If this is true then resetting the definingDeclaration below is pointless.
          if (firstNondefiningDecl->get_definingDeclaration() != decl)
             {
               printf ("WARNING: In convert_struct_primary(): Resetting firstNondefiningDecl = %p definingDeclaration (currently %p) to decl = %p \n",firstNondefiningDecl,firstNondefiningDecl->get_definingDeclaration(),decl);
             }
       // ROSE_ASSERT(firstNondefiningDecl->get_definingDeclaration() == decl);
          ROSE_ASSERT(firstNondefiningDecl->get_definingDeclaration() == NULL || firstNondefiningDecl->get_definingDeclaration() == decl);

          firstNondefiningDecl->set_definingDeclaration(decl);

       // DQ (9/6/2012): This may be the cause of the problems with haskell tests!
#if 0
          printf ("WARNING: In convert_struct_primary(): Resetting decl = %p firstNondefiningDeclaration (currently %p) to firstNondefiningDecl = %p \n",decl,decl->get_firstNondefiningDeclaration(),firstNondefiningDecl);
#endif
       // DQ (9/7/2012): If this is true then resetting the firstNondefiningDeclaration below is pointless.
          if (decl->get_firstNondefiningDeclaration() != firstNondefiningDecl)
             {
               printf ("WARNING: In convert_struct_primary(): Resetting decl = %p firstNondefiningDeclaration (currently %p) to firstNondefiningDecl = %p \n",decl,decl->get_firstNondefiningDeclaration(),firstNondefiningDecl);
               printf ("firstNondefiningDecl = %p = %s = %s \n",firstNondefiningDecl,firstNondefiningDecl->class_name().c_str(),SageInterface::get_name(firstNondefiningDecl).c_str());
               firstNondefiningDecl->get_file_info()->display("failing test: firstNondefiningDecl: debug");
               printf ("decl = %p = %s \n",decl,decl->class_name().c_str());
               decl->get_file_info()->display("failing test: decl : debug");
               printf ("decl->get_firstNondefiningDeclaration() = %p = %s \n",decl->get_firstNondefiningDeclaration(),decl->get_firstNondefiningDeclaration()->class_name().c_str());
               decl->get_firstNondefiningDeclaration()->get_file_info()->display("failing test: decl->get_firstNondefiningDeclaration(): debug");
             }
          ROSE_ASSERT(decl->get_firstNondefiningDeclaration() == firstNondefiningDecl);

       // If this is a forward declaration, not associated with the original 
       // first-nondefining declaration then point to the original non-defining 
       // declaration for consistancy!
          decl->set_firstNondefiningDeclaration(firstNondefiningDecl);

       // DQ (9/6/2012): This may be the cause of the problems with haskell tests!
#if 0
          printf ("WARNING: In convert_struct_primary(): Resetting decl = %p definingDeclaration (currently %p) to firstNondefiningDecl->get_definingDeclaration() = %p \n",
               decl,decl->get_definingDeclaration(),firstNondefiningDecl->get_definingDeclaration());
#endif
       // DQ (9/7/2012): If this is true then resetting the definingDeclaration below is pointless.
          ROSE_ASSERT(decl->get_definingDeclaration() == firstNondefiningDecl->get_definingDeclaration());

       // This might only be setting the definin declaration to NULL, if we have not see the defining declaration yet!
          decl->set_definingDeclaration(firstNondefiningDecl->get_definingDeclaration());

          ROSE_ASSERT(firstNondefiningDecl->get_definingDeclaration() == decl);
          ROSE_ASSERT(firstNondefiningDecl->get_firstNondefiningDeclaration() != decl);

       // DQ (8/12/2012): Added test.
       // ROSE_ASSERT(type_translation_cache.find(p) == type_translation_cache.end());
          if (type_translation_cache.find(p) != type_translation_cache.end())
             {
               ROSE_ASSERT(type_translation_cache[p] != NULL);
             }

       // DQ (9/9/2012): Added assertion.
       // ROSE_ASSERT(type_translation_cache.find(p) != type_translation_cache.end());

       // DQ (9/9/2012): Using the find() function on type_translation_cache to protect from adding NULL entroies (safer approach).
       // Set the type map, even though we would like to use the symbol table as a reference in the future.
       // if (type_translation_cache[p] == NULL)
          if (type_translation_cache.find(p) == type_translation_cache.end())
             {
#if 0
               printf ("In convert_struct_primary(): Adding to type_translation_cache: p = %p decl->get_type() = %p = %s \n",
                    p,firstNondefiningDecl->get_type(),firstNondefiningDecl->get_type()->class_name().c_str());
#endif
               type_translation_cache[p] = firstNondefiningDecl->get_type();
             }

       // DQ (8/12/2012): Added test.
          ROSE_ASSERT(type_translation_cache.find(p) != type_translation_cache.end());

       // Make sure that types are shared between defining and non-defining declarations
          ROSE_ASSERT(decl->get_type() != NULL);
          ROSE_ASSERT(firstNondefiningDecl->get_type() != NULL);
          if (firstNondefiningDecl->get_type() != decl->get_type())
             {
               printf ("Fixing up class types to be shared between the defining and non-defining declaration decl->get_type() = %p = %s \n",decl->get_type(),decl->get_type()->class_name().c_str());
               delete decl->get_type();
               decl->set_type(firstNondefiningDecl->get_type());

            // DQ (12/13/2011): Is this executed...
               printf ("Is this code executed (deleting an IR node) \n");
               ROSE_ASSERT(false);
             }
          ROSE_ASSERT(firstNondefiningDecl->get_type() == decl->get_type());
        }
       else
        {
          printf ("There should have been a symbol! \n");
          ROSE_ASSERT(false);
        }

     ROSE_ASSERT (firstNondefiningDecl != NULL);

  // ROSE_ASSERT (firstNondefiningDecl->get_forward() == true);
  // ROSE_ASSERT (firstNondefiningDecl->get_forward() == false);

     ROSE_ASSERT(decl->get_firstNondefiningDeclaration() != decl);

     ROSE_ASSERT(decl != firstNondefiningDecl);

  // DQ (9/6/2012): This may be the cause of the problems with haskell tests!
#if 0
     printf ("WARNING: In convert_struct_primary(): 2nd reset: Resetting decl = %p firstNondefiningDeclaration (currently %p) to firstNondefiningDecl = %p \n",decl,decl->get_firstNondefiningDeclaration(),firstNondefiningDecl);
#endif

  // DQ (9/7/2012): If this is true then resetting the firstNondefiningDeclaration below is pointless.
     ROSE_ASSERT(decl->get_firstNondefiningDeclaration() == firstNondefiningDecl);

     decl->set_firstNondefiningDeclaration(firstNondefiningDecl);
     ROSE_ASSERT(decl->get_firstNondefiningDeclaration() != decl);

  // DQ (9/6/2012): This may be the cause of the problems with haskell tests!
#if 0
     printf ("WARNING: In convert_struct_primary(): 2nd reset: Resetting firstNondefiningDecl = %p definingDeclaration (currently %p) to decl = %p \n",firstNondefiningDecl,firstNondefiningDecl->get_definingDeclaration(),decl);
#endif

  // DQ (9/7/2012): If this is true then resetting the definingDeclaration below is pointless.
     ROSE_ASSERT(firstNondefiningDecl->get_definingDeclaration() == decl);

     firstNondefiningDecl->set_definingDeclaration(decl);

  // Since we get the type from decl, this is meaningless...
  // decl->set_type(classType);

     ROSE_ASSERT (decl != NULL);
  // printf ("Leaving convert_struct_primary() decl = %p = %s \n",decl,decl->class_name().c_str());

  // printf ("decl                                            = %p \n",decl);
  // printf ("firstNondefiningDecl->get_definingDeclaration() = %p \n",firstNondefiningDecl->get_definingDeclaration());
     ROSE_ASSERT(firstNondefiningDecl->get_definingDeclaration() == decl);
     ROSE_ASSERT(firstNondefiningDecl->get_firstNondefiningDeclaration() == firstNondefiningDecl);
     ROSE_ASSERT(firstNondefiningDecl->get_definingDeclaration()->get_firstNondefiningDeclaration() == firstNondefiningDecl);

     ROSE_ASSERT(matching_defining_and_nondefining_variants(decl) == true);

  // DQ (2/27/2012): Test setting the parent correctly (an not just to the current scope).
  // printf ("*** WARNING: In convert_struct_primary(): (at base) Commented out the test of parent to be non-null \n");
  // ROSE_ASSERT(firstNondefiningDecl->get_parent() != NULL);
  // ROSE_ASSERT(firstNondefiningDecl->get_definingDeclaration()->get_parent() != NULL);

     ROSE_ASSERT(decl->get_firstNondefiningDeclaration() != decl);

  // DQ (12/29/2011): Make sure this is not interpreted as a template.
  // ROSE_ASSERT(p->variant.class_struct_union.extra_info->assoc_template == NULL);
     if (p->variant.class_struct_union.extra_info->assoc_template != NULL)
        {
       // DQ (8/11/2012): This fails for test2012_175.C.  Not clear if this is reasonable so I output a warning for now.
          if (!hasFirstNondefDeclForTemplate(p->variant.class_struct_union.extra_info->assoc_template))
             {
#if 0
               printf ("WARNING: template has not been seen yet, might be a problem (can't say just yet). \n");
#endif
             }
        }

  // DQ (1/1/2012: New test.
     ROSE_ASSERT(decl->get_definingDeclaration() == decl);

#if 0
     decl->get_file_info()->display("At base of convert_struct_primary(): debug");
#endif

  // DQ (4/8/2012): Test the maps that we use internally (see function for documentation).
     testEdgRoseMaps();

     ROSE_ASSERT(decl->get_definition() != NULL);

#if DEBUG_STRUCTURAL_DESIGN || DEBUG_CONVERT_STRUCT_PRIMARY || 0
     printf ("********************************************************************************** \n");
     printf ("Leaving convert_struct_primary(): p = %p scopeOfDeclaration = %p = %s decl = %p = %s decl->get_definition() = %p = %s nameWithTemplateArguments = %s summarize_type_kind_in_string_kind(p->kind) = %s \n",
             p,scopeOfDeclaration,scopeOfDeclaration->class_name().c_str(),decl,decl->class_name().c_str(),
             decl->get_definition(),decl->get_definition()->class_name().c_str(),nameWithTemplateArguments.str(),summarize_type_kind_in_string_kind(p->kind));

     printf ("decl->get_firstNondefiningDeclaration() = %p \n",decl->get_firstNondefiningDeclaration());
     if (decl->get_firstNondefiningDeclaration() != NULL)
        {
          printf ("decl->get_firstNondefiningDeclaration() = %p = %s \n",decl->get_firstNondefiningDeclaration(),decl->get_firstNondefiningDeclaration()->class_name().c_str());

       // DQ (7/31/2019): Check the parent of the non-defining declaration (debugging test2019_501.C).
          printf ("decl->get_firstNondefiningDeclaration()->get_parent() = %p \n",decl->get_firstNondefiningDeclaration()->get_parent());

       // DQ (7/31/2019): Comment out so that I can generate a graph on a more trivial case.
       // The question is what should the parent be set to if the nondefining declaration is used with a sysmbol, and not a part of the AST.
       // ROSE_ASSERT(decl->get_firstNondefiningDeclaration()->get_parent() != NULL);

          if (decl->get_firstNondefiningDeclaration()->get_parent() != NULL)
             {
               printf ("decl->get_firstNondefiningDeclaration()->get_parent() = %p = %s \n",
                    decl->get_firstNondefiningDeclaration()->get_parent(),decl->get_firstNondefiningDeclaration()->get_parent()->class_name().c_str());
             }
        }
     printf ("********************************************************************************** \n");
#endif

  // DQ (9/6/2012): Added support for testing first non-defining, defining, and secondary declarations.
     testDefiningNonDefiningDeclarations(firstNondefiningDecl,decl,NULL,"convert_struct_primary()");

  // DQ (9/6/2012): Added assertion.
     ROSE_ASSERT(decl->get_type() != NULL);

#if 0
  // DQ (4/29/2012): Debugging why the unparter is putting out this class when it is instantiated from a template.
     decl->get_file_info()->display("In convert_struct_primary(): debug");
#endif
#if 0
     decl->get_file_info()->display("In convert_struct_primary(): decl: debug");
     if (decl->get_firstNondefiningDeclaration() != NULL)
        {
          printf ("decl->get_firstNondefiningDeclaration() = %p = %s \n",decl->get_firstNondefiningDeclaration(),decl->get_firstNondefiningDeclaration()->class_name().c_str());
          decl->get_firstNondefiningDeclaration()->get_file_info()->display("In convert_struct_primary(): decl->get_firstNondefiningDeclaration(): debug");
        }
#endif

#if 0
     printf ("In convert_struct_primary(): p->autonomous_primary_tag_decl = %s \n",p->autonomous_primary_tag_decl ? "true" : "false");
#endif

  // DQ (7/5/2012): Swapped true and false branches based on test2012_108.C.
  // if (p->autonomous_primary_tag_decl == true)
     if (p->autonomous_primary_tag_decl == true)
        {
       // This is a simple (typical) declaration (referenced directly in a scope) used to define a type.
          if (decl->get_parent() == NULL)
             {
            // printf ("WARNING: It appears that we don't have enough information to set the parent to the variable declaration where this type is declared \n");

               SgScopeStatement* rose_scopeOfDeclaration = scopeOfDeclaration; // getDeclarationScope(scp);
               ROSE_ASSERT(rose_scopeOfDeclaration != NULL);
#if 0
               printf ("In convert_struct_primary(): Setting the parent of decl = %p = %s to scope = %p = %s \n",decl,decl->class_name().c_str(),rose_scopeOfDeclaration,rose_scopeOfDeclaration->class_name().c_str());
#endif
               decl->set_parent(rose_scopeOfDeclaration);
             }
          ROSE_ASSERT(decl->get_parent() != NULL);

       // printf ("In convert_struct_primary(): Mark declaration as AUTONOMOUS \n");
          decl->set_isAutonomousDeclaration(true);
        }
       else
        {
       // DQ (9/8/2012): Fix this to allow the defining declaration to be reused (where the parent pointer may have been previously set).
       // This is a part of a type nested/referenced in a declaration.
       // We have to wait until we see the associated outer declaration 
       // before we can set this parent (or we leave it unset, as I recall).
       // ROSE_ASSERT(decl->get_parent() == NULL);
       // TV (2/3/14): When merging file, the decl might already have been seen in the previous file. In this case, the parent would have been set
       // ROSE_ASSERT(decl->get_parent() == NULL || reusedDefiningDeclaration == true);

       // printf ("Exiting as a test when this is a NESTED type \n");
       // ROSE_ASSERT(false);

          decl->set_isAutonomousDeclaration(false);
        }

  // DQ (7/9/2012): Verify settings.
     if (p->autonomous_primary_tag_decl == false)
        {
       // This is a type declaration such as: "class A0 {public: int foo (int x) {return x;}} x2;"
       // printf ("This is a class declaration inside of a type reference \n");

       // DQ (7/9/2012): Setting this to assert NULL since this is not an autonomous declaration (see test2012_107.C).
       // ROSE_ASSERT(decl->get_parent() != NULL);
          ROSE_ASSERT(decl != NULL);

       // DQ (9/8/2012): Fix this to allow the defining declaration to be reused (where the parent pointer may have been previously set).
       // ROSE_ASSERT(decl->get_parent() == NULL);
       // TV (2/3/14): When merging file, the decl might already have been seen in the previous file. In this case, the parent would have been set
       // ROSE_ASSERT(decl->get_parent() == NULL || reusedDefiningDeclaration == true);
        }
       else
        {
       // This is a type declaration such as: "class A {public: int foo (int x) {return x;}}; A x2;"
       // printf ("This is a class declaration NOT defined in a type reference (so it's structural parent is the associated scope) \n");
          ROSE_ASSERT(decl->get_parent() != NULL);
        }

  // DQ (8/11/2014): Added to support C++11 final keyword.
     if (p->variant.class_struct_union.final == true)
        {
#ifdef DEBUG_CONVERSION
          printf ("In convert_struct_primary(): detected final keyword \n");
#endif
          decl->get_declarationModifier().setFinal();

#if 0
          printf ("Exiting as a test! \n");
          ROSE_ASSERT(false);
#endif
        }

#if DEBUG_STRUCTURAL_DESIGN || DEBUG_CONVERT_STRUCT_PRIMARY || 0
     printf ("Leaving convert_struct_primary(): decl = %p decl->get_parent() = %p \n",decl,decl->get_parent());
     printf ("   --- decl->get_name()                                   = %s \n",decl->get_name().str());
     printf ("   --- decl->get_definingDeclaration()                    = %p \n",decl->get_definingDeclaration());
     printf ("   --- decl->get_firstNondefiningDeclaration()            = %p \n",decl->get_firstNondefiningDeclaration());
     printf ("   --- decl->get_isAutonomousDeclaration()                = %s \n",decl->get_isAutonomousDeclaration() ? "true" : "false");
     printf ("   --- decl = %p = %s: access = %s \n",decl,decl->class_name().c_str(),decl->get_declarationModifier().get_accessModifier().displayString().c_str());

     printf ("   --- decl->get_scope()                                  = %p = %s \n",decl->get_scope(),decl->get_scope()->class_name().c_str());

  // DQ (3/7/2015): Debugging null pointer in reference to templateDeclaration specific to EDG 4.7 (works with EDG 4.9 using g++ 4.4.7 and g++ 4.8.1).
     SgTemplateInstantiationDecl* tmp_nondefiningDeclaration = isSgTemplateInstantiationDecl(decl->get_firstNondefiningDeclaration());
     SgTemplateInstantiationDecl* tmp_definingDeclaration    = isSgTemplateInstantiationDecl(decl->get_definingDeclaration());
     if (tmp_nondefiningDeclaration != NULL)
        {
           printf ("tmp_nondefiningDeclaration: name = %s get_templateDeclaration() = %p \n",tmp_nondefiningDeclaration->get_name().str(),tmp_nondefiningDeclaration->get_templateDeclaration());
        }
     if (tmp_definingDeclaration != NULL)
        {
          printf ("tmp_definingDeclaration: name = %s get_templateDeclaration() = %p \n",tmp_definingDeclaration->get_name().str(),tmp_definingDeclaration->get_templateDeclaration());
        }
#endif

     return decl;
   }
