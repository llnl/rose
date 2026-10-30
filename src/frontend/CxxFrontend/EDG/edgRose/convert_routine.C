
#include "edgRose.h"

SgFunctionDeclaration* EDG_ROSE_Translation::convert_routine(a_routine_ptr p, bool forceTemplateDeclaration, a_template_ptr edg_template, SgFunctionDeclaration* optional_nondefiningTemplateDeclaration )
   {
  // This function will process an EDG routine from either a template function, instantiated template function, 
  // or a regular function (including the member function versions).  As a result it is a very general (single) 
  // implementation.  However, it is a bit too complex for my tastes, so we want to simplify this as much as 
  // possible (this will happen in a series of iterations, so that I can maintain the correctness).

#define DEBUG_CONVERT_ROUTINE 0

#if DEBUG_STRUCTURAL_DESIGN || DEBUG_CONVERT_ROUTINE
     printf ("**********************************************************************************************\n");
     printf ("**********************************************************************************************\n");
     printf ("Inside of convert_routine(a_routine_ptr p = %p, forceTemplateDeclaration = %s, edg_template = %p) \n",p,forceTemplateDeclaration ? "true" : "false",edg_template);
     printf ("   --- optional_nondefiningTemplateDeclaration = %p \n",optional_nondefiningTemplateDeclaration);
     printf ("**********************************************************************************************\n");
     printf ("**********************************************************************************************\n");
#endif

#if DEBUG_CLASSES_FROM_TEMPLATE_PARAMETERS
     printf ("   --- internal_intermediate_scope = %p \n",internal_intermediate_scope);
     size_t numberOfClassesBuiltFromTemplateParameters_1 = internal_intermediate_scope->symbol_table_size();
  // if (numberOfClassesBuiltFromTemplateParameters_1 > 0)
     printf ("   --- number of numberOfClassesBuiltFromTemplateParameters = %zu \n",numberOfClassesBuiltFromTemplateParameters_1);
#endif

     ROSE_ASSERT(p != NULL);

#ifdef DEBUG_CONVERSION
     indent_line(); debug_print(p->source_corresp.is_class_member ? "member function definition " : "function definition "); print_source_corresp(p->source_corresp); debug_print("\n");
#endif

#if 0
     debug_print(p->source_corresp.is_class_member ? "member function definition " : "function definition "); print_source_corresp(p->source_corresp); debug_print("\n");
#endif

#if 0
  // DQ (9/22/2022): Added debugging code.
     SgName temp_name = p->source_corresp.name;
  // if (p->source_corresp.name == "foobar")
     if (temp_name == "foobar")
        {
          printf ("Found foobar function: exiting as a test! \n");
          ROSE_ASSERT(false);
        }
       else
        {
          printf ("Note: foobar function not found: p->source_corresp.name = %s \n",p->source_corresp.name);
        }
#endif

  // DQ (2/11/2016): Moved this from the end of the function to the top.
  // DQ (3/31/2012): allow for the case where p->assoc_scope == NULL (seen in test2012_58.C).
  // EDG maintains separate regions for each function (I think so that the frontend can work efficiently 
  // on machines with small memories), so get the function scope representing the current function.
  // a_scope_ptr scope = il_header.region_scope_entry[p->assoc_scope];
     a_scope_ptr scope = NULL;
  // DQ (10/5/2016): lookup of scopes for function definitions (etc.) are now implemented differently in EDG 4.11.
  // printf ("In convert_routine(): p->function_def_number = %d \n",p->function_def_number);
     if (p->function_def_number != NULL_function_def_number)
        {
       // Note that p->assoc_scope is an EDG specific scope number (not a pointr to a scope).
       // scope = il_header.region_scope_entry[p->function_def_number];
          scope = il_header.function_def_table[p->function_def_number].scope;
        }
       else
        {
       // This case is used for test2012_58.C
          ROSE_ASSERT(p->source_corresp.parent_scope != NULL);
          scope = p->source_corresp.parent_scope;
        }

  // DQ (2/12/2016): I think that we can assert this.
     ROSE_ASSERT(scope != NULL);

#if DEBUG_CONVERT_ROUTINE
     printf ("In convert_routine(): scope = %p \n",scope);
#endif

  // DQ (2/12/2016): This needs to be set before the processing of the loop body so that VLA function parameters can be evaluated.
     global_innermost_function_scope = scope;
     ROSE_ASSERT(global_innermost_function_scope != NULL);

#if 0
     printf ("In convert_routine(): Calling summarize_scope() \n");
     summarize_scope(scope);
#endif

#if 0
     ROSE_ASSERT(p->type != NULL);
     SgType* edg_routine_type = convert_type(p->type);
     ROSE_ASSERT(p->declared_type != NULL);
     SgType* edg_routine_declared_type = convert_type(p->declared_type);

     printf ("In convert_routine(): edg_routine_type          = %p = %s \n",p->type,edg_routine_type->unparseToString().c_str());
     printf ("In convert_routine(): edg_routine_declared_type = %p = %s \n",p->declared_type,edg_routine_declared_type->unparseToString().c_str());
#endif
#if 0
     printf ("Exiting as a test! \n");
     ROSE_ASSERT(false);
#endif

  // DQ (9/6/2014): When provided this is either a SgTemplateFunctionDeclaration or a SgTemplateMemberFunctionDeclaration.
  // This is part of recursive support for template declarations edited out of the template class definitions where they 
  // are represented by function prototypes.
     if (optional_nondefiningTemplateDeclaration != NULL)
        {
          ROSE_ASSERT(isSgTemplateFunctionDeclaration(optional_nondefiningTemplateDeclaration) != NULL || isSgTemplateMemberFunctionDeclaration(optional_nondefiningTemplateDeclaration) != NULL);
        }

  // DQ (1/2/2012): Refactoring generation of names.
     ROSE_ASSERT(p->source_corresp.name != NULL);
  // SgName name = p->source_corresp.name;
  // SgName name = generateName(p);
     SgName nameWithOutTemplateArguments = generateName(p);
     SgName nameWithTemplateArguments    = nameWithOutTemplateArguments;

#if DEBUG_CONVERT_ROUTINE
     printf ("In convert_routine(): p->assoc_template = %p nameWithOutTemplateArguments = %s \n",p->assoc_template,nameWithOutTemplateArguments.str());
     if (optional_nondefiningTemplateDeclaration != NULL)
        {
          printf ("   --- optional_nondefiningTemplateDeclaration->get_name() = %s \n",optional_nondefiningTemplateDeclaration->get_name().str());
        }
       else
        {
          printf ("   --- optional_nondefiningTemplateDeclaration == NULL \n");
        }
#endif

#if 0
  // DQ (3/5/2017): Testing the mlog mechanism.
     mprintf ("MPRINTF: In convert_routine(): p->assoc_template = %p nameWithOutTemplateArguments = %s \n",p->assoc_template,nameWithOutTemplateArguments.str());
     mlog[DEBUG] << "MLOG: In convert_routine(): xxx " << endl;
#endif

     bool buildTemplateDeclaration   = false;
     bool buildTemplateInstantiation = false;
     setTemplateOrTemplateInstantiationFunctionGeneration(p,forceTemplateDeclaration,buildTemplateDeclaration,buildTemplateInstantiation);

#if DEBUG_CONVERT_ROUTINE
     printf ("In convert_routine(): buildTemplateInstantiation = %s \n",buildTemplateInstantiation ? "true" : "false");
     printf ("In convert_routine(): buildTemplateDeclaration   = %s \n",buildTemplateDeclaration ? "true" : "false");

     if (p->source_corresp.is_class_member == true)
          printf ("THIS ROUTINE IS A MEMBER FUNCTION \n");
       else
          printf ("THIS ROUTINE IS NOT A MEMBER FUNCTION \n");
#endif

#if DEBUG_CONVERT_ROUTINE
     printf ("In convert_routine(): p->defined_in_friend_decl     = %s \n",(p->defined_in_friend_decl == true) ? "true" : "false");
     printf ("In convert_routine(): p->is_prototype_instantiation = %s \n",(p->is_prototype_instantiation == true) ? "true" : "false");
     printf ("In convert_routine(): topScopeStack()               = %p = %s \n",topScopeStack(),topScopeStack()->class_name().c_str());
#endif

  // DQ (5/22/2013): Reset the buildTemplateInstantiation.
     if (isSgTemplateInstantiationDefn(topScopeStack()) != NULL && p->defined_in_friend_decl == true)
        {
       // This is a case of a friend function defined in a template declaration (definition) (see test2013_180.C).
#if 0
       // DQ (7/7/2014): We can force this to be reset, but it is the right thing to do? It will just fail later on!
          printf ("In convert_routine(): (BEFORE RESET) buildTemplateInstantiation = %s \n",buildTemplateInstantiation ? "true" : "false");
          buildTemplateInstantiation = true;
          printf ("In convert_routine(): (RESET) buildTemplateInstantiation = %s \n",buildTemplateInstantiation ? "true" : "false");
#else
       // DQ (7/7/2014): Note that the function can be a template declaration (as in test2014_79.C and test2014_80.C), we have to allow this case.
          if (buildTemplateInstantiation == false)
             {
#ifdef DEBUG_CONVERSION
               printf ("In convert_routine(): warning: buildTemplateInstantiation == false (case of template declaration inside a template instantiation) \n");
#endif
            // printf ("In convert_routine(): warning: buildTemplateInstantiation == false calling summarize_routine() \n");
            // summarize_routine(p);
             }
       // ROSE_ASSERT(buildTemplateInstantiation == true);
#endif
        }

  // DQ (4/4/2012): I think that which map we use should depend on if forceTemplateDeclaration is true or false.
  // Alternatively, we might want to use the computed setting of buildTemplateDeclaration (declared below) as
  // the trigger.  We should also implement the testing required to make sure that the maps 
  // (e.g. first_nondefining_declarations_for_functions) don't have references to template functions (member 
  // and non-member), and that template specific maps (e.g. first_nondefining_declarations_for_templates)
  // don't contain non-template functions.

  // DQ (4/8/2012): When this function generates a template function we need to modify the EDG/ROSE map for 
  // templates, not for functions.  So this work below (building a NULL entry in the EDG/ROSE map) needs to 
  // be undone or not generated in the first place until we know what we will be generating.  I don't like
  // this aspect of the define of the new EDG/ROSE translation.

  // DQ (4/9/2012): Test the maps that we use internally (see function for documentation).
     testEdgRoseMaps();

  // DQ (4/9/2012): I want to change how this is handled.  The use of the map this way causes a NULL pointer 
  // entry into the map and this is a problem.  Another problem is that we want to alternatively use the 
  // template map and at this point we don't know which map to use.
  // This function will correctly return a SgMemberFunctionDeclaration if p is a member function.
  // SgFunctionDeclaration*& first_nondefining_declaration = first_nondefining_declarations_for_functions[p];
  // SgFunctionDeclaration*& first_nondefining_declaration = first_nondefining_declarations_for_functions[p];
  // SgFunctionDeclaration*& return_defining_declaration   = defining_declarations_for_functions[p];
     SgFunctionDeclaration* first_nondefining_declaration = NULL;
     SgFunctionDeclaration* return_defining_declaration   = NULL;

  // DQ (4/9/2012): We need to find a correct value for this pointer.
  // a_template_ptr edg_template = NULL;

  // DQ (4/10/2012): I think this is the correct value to use to initialize this.
  // edg_template = p->assoc_template;

  // DQ (5/10/2012): Refactored code to set the first_nondefining_declaration and return_defining_declaration function declarations.
  // setDefiningAndFirstNondefiningDeclarations(p,buildTemplateDeclaration,buildTemplateInstantiation,forceTemplateDeclaration,first_nondefining_declaration,return_defining_declaration,edg_template);
     setDefiningAndFirstNondefiningDeclarations(p,forceTemplateDeclaration,first_nondefining_declaration,return_defining_declaration,edg_template);

#if DEBUG_CONVERT_ROUTINE
     printf ("part 1: buildTemplateInstantiation = %s first_nondefining_declaration = %p \n",buildTemplateInstantiation ? "true" : "false",first_nondefining_declaration);
#endif

  // DQ (5/20/2016): Test the template arguments match here.
  // DQ (5/19/2016): Adding test on template arguments and those in the first_nondefining_declaration.
     if (buildTemplateInstantiation == true && first_nondefining_declaration != NULL)
        {
       // ROSE_ASSERT(first_nondefining_declaration != NULL);
       // SgTemplateInstantiationMemberFunctionDecl*first_nondefiningTemplateInstantiationMemberFunctionDeclaration = 
       //      isSgTemplateInstantiationMemberFunctionDecl(first_nondefiningMemberFunctionDeclaration);
          ROSE_ASSERT(first_nondefining_declaration == first_nondefining_declaration->get_firstNondefiningDeclaration());
          SgTemplateInstantiationMemberFunctionDecl* first_nondefiningTemplateInstantiationMemberFunctionDeclaration = 
               isSgTemplateInstantiationMemberFunctionDecl(first_nondefining_declaration->get_firstNondefiningDeclaration());
#if 0
          printf ("Testing of template arguments: part 1: first_nondefining_declaration = %p \n",first_nondefining_declaration);
          printf ("Testing of template arguments: part 1: first_nondefiningTemplateInstantiationMemberFunctionDeclaration = %p if non-null \n",
               first_nondefiningTemplateInstantiationMemberFunctionDeclaration);
#endif
          if (first_nondefiningTemplateInstantiationMemberFunctionDeclaration != NULL)
             {
            // ROSE_ASSERT(first_nondefiningTemplateInstantiationMemberFunctionDeclaration != NULL);
               ROSE_ASSERT(first_nondefiningTemplateInstantiationMemberFunctionDeclaration->get_firstNondefiningDeclaration() != NULL);
               ROSE_ASSERT(first_nondefiningTemplateInstantiationMemberFunctionDeclaration->get_firstNondefiningDeclaration() == first_nondefining_declaration);

               SgTemplateArgumentPtrList & templateArgumentsList_from_first_nondefining_declaration = first_nondefiningTemplateInstantiationMemberFunctionDeclaration->get_templateArguments();

               a_template_arg_ptr tmp_templateArgs = p->template_arg_list;
               SgScopeStatement* tmp_scopeOfFunction = NULL;
               if (optional_nondefiningTemplateDeclaration != NULL)
                  {
                 // This is the special case to force construction of template member and non-member function defining declarations.
                    tmp_scopeOfFunction = optional_nondefiningTemplateDeclaration->get_scope();
                    ROSE_ASSERT(tmp_scopeOfFunction != NULL);
#if 0
                    printf ("Resetting the scopeOfFunction using the optional_nondefiningTemplateDeclaration: tmp_scopeOfFunction = %p = %s \n",
                         tmp_scopeOfFunction,tmp_scopeOfFunction->class_name().c_str());
#endif
                  }
                 else
                  {
                 // This is the more common case.
#if DEBUG_GET_DECLARATION_SCOPE
                    printf ("In convert_routine(): calling getDeclarationScope() \n");
#endif
                    tmp_scopeOfFunction = getDeclarationScope(p->source_corresp);
                    ROSE_ASSERT(tmp_scopeOfFunction != NULL);
                  }

               ROSE_ASSERT(tmp_scopeOfFunction != NULL);

               SgTemplateArgumentPtrList tmp_templateArgumentsList = 
                    setupMatchingTemplateArguments(buildTemplateInstantiation,first_nondefining_declaration, tmp_templateArgs, tmp_scopeOfFunction);
#if 0
               printf ("Testing of template argument list \n");
#endif
               ROSE_ASSERT(SageInterface::templateArgumentListEquivalence(tmp_templateArgumentsList, templateArgumentsList_from_first_nondefining_declaration));
             }
            else
             {
#if 0
            // DQ (7/6/2016): Adding more debugging information.
               printf ("In convert_routine(): This is not a member function template instantiation: first_nondefining_declaration = %p = %s \n",
                    first_nondefining_declaration,first_nondefining_declaration->class_name().c_str());
#endif
             }
        }

  // DQ (11/23/2015): Added test for consistanty of the generate function (in the case of the defining friend function in test2015_121.C).
     if (isSgTemplateInstantiationFunctionDecl(first_nondefining_declaration) != NULL && buildTemplateInstantiation == false)
        {
#if 0
       // DQ (7/6/2016): I want to see this debugging info.
          printf ("WARNING: In convert_routine(): detected inconsistanty in the type of function that will be built \n");
          printf ("   --- buildTemplateInstantiation = %s \n",buildTemplateInstantiation ? "true" : "false");
          printf ("   --- first_nondefining_declaration = %p = %s \n",first_nondefining_declaration,first_nondefining_declaration->class_name().c_str());
#else
          mprintf ("WARNING: In convert_routine(): detected inconsistanty in the type of function that will be built \n");
          mprintf ("   --- buildTemplateInstantiation = %s \n",buildTemplateInstantiation ? "true" : "false");
          mprintf ("   --- first_nondefining_declaration = %p = %s \n",first_nondefining_declaration,first_nondefining_declaration->class_name().c_str());

       // DQ (1/6/2020): I want to see this debugging info.
          printf ("WARNING: In convert_routine(): detected inconsistanty in the type of function that will be built \n");
          printf ("   --- buildTemplateInstantiation = %s \n",buildTemplateInstantiation ? "true" : "false");
          printf ("   --- first_nondefining_declaration = %p = %s \n",first_nondefining_declaration,first_nondefining_declaration->class_name().c_str());
#endif
       // ROSE_ASSERT(false);
#if 0
          printf ("In convert_routine(): RESET buildTemplateInstantiation = true \n");
#endif
       // DQ (3/2/2019): Can we assert this?
          ROSE_ASSERT(p->is_template_function == true);

          buildTemplateInstantiation = true;
        }
       else
        {
       // DQ (3/2/2019): Can we assert this? No!
       // ROSE_ASSERT(p->is_template_function == false);
        }

  // DQ (9/6/2014): When provided this is either a SgTemplateFunctionDeclaration or a SgTemplateMemberFunctionDeclaration.
  // This is part of recursive support for template declarations edited out of the template class definitions where they 
  // are represented by function prototypes.
     if (optional_nondefiningTemplateDeclaration != NULL)
        {
#if 0
          printf ("In convert_routine(): Setting first_nondefining_declaration to non-null optional_nondefiningTemplateDeclaration = %p \n",optional_nondefiningTemplateDeclaration);
          printf ("   --- optional_nondefiningTemplateDeclaration = %p = %s \n",optional_nondefiningTemplateDeclaration,optional_nondefiningTemplateDeclaration->class_name().c_str());
          printf ("   --- first_nondefining_declaration           = %p = %s \n",first_nondefining_declaration,(first_nondefining_declaration != NULL) ? first_nondefining_declaration->class_name().c_str() : "null");
          printf ("   --- return_defining_declaration             = %p = %s \n",return_defining_declaration,(return_defining_declaration != NULL) ? return_defining_declaration->class_name().c_str() : "null");
#endif
       // first_nondefining_declaration = optional_nondefiningTemplateDeclaration;
          if (first_nondefining_declaration == NULL)
             {
               first_nondefining_declaration = optional_nondefiningTemplateDeclaration;
             }

       // DQ (9/6/2014): If is is not the same pointer, then force it to be so (fails for test2004_81.C).
       // ROSE_ASSERT(first_nondefining_declaration == optional_nondefiningTemplateDeclaration);
          if (first_nondefining_declaration != optional_nondefiningTemplateDeclaration)
             {
#if 0
               printf ("Forcing the first_nondefining_declaration to be reset to optional_nondefiningTemplateDeclaration = %p = %s \n",
                    optional_nondefiningTemplateDeclaration,optional_nondefiningTemplateDeclaration->class_name().c_str());
#endif
               first_nondefining_declaration = optional_nondefiningTemplateDeclaration;
             }
          ROSE_ASSERT(first_nondefining_declaration == optional_nondefiningTemplateDeclaration);
        }

     if (first_nondefining_declaration != NULL && first_nondefining_declaration->get_firstNondefiningDeclaration() != first_nondefining_declaration)
        {
       // This is an error.
          printf ("Error: In convert_routine(): (first_nondefining_declaration != NULL && first_nondefining_declaration->get_firstNondefiningDeclaration() != first_nondefining_declaration) \n");
          printf ("   --- first_nondefining_declaration = %p = %s = %s \n",first_nondefining_declaration,
               first_nondefining_declaration->class_name().c_str(),first_nondefining_declaration->get_name().str());

          printf ("   --- first_nondefining_declaration->get_mangled_name() = %s \n",first_nondefining_declaration->get_mangled_name().str());

          ROSE_ASSERT(isSgFunctionDeclaration(first_nondefining_declaration->get_firstNondefiningDeclaration()) != NULL);
          printf ("   --- first_nondefining_declaration->get_firstNondefiningDeclaration() = %p = %s = %s \n",first_nondefining_declaration->get_firstNondefiningDeclaration(),
               first_nondefining_declaration->get_firstNondefiningDeclaration()->class_name().c_str(),
               isSgFunctionDeclaration(first_nondefining_declaration->get_firstNondefiningDeclaration())->get_name().str());
          printf ("   --- first_nondefining_declaration->get_firstNondefiningDeclaration()->get_mangled_name() = %s \n",first_nondefining_declaration->get_firstNondefiningDeclaration()->get_mangled_name().str());

       // DQ (9/6/2014): This is debugging code only called in case of an error (caught by the ROSE_ASSERT below).
          printf ("In convert_routine(): calling summarize_routine() before checking for if this has previously been translated into return_defining_declaration!\n");
          summarize_routine(p);
        }
     ROSE_ASSERT(first_nondefining_declaration == NULL || first_nondefining_declaration->get_firstNondefiningDeclaration() == first_nondefining_declaration);

#if DEBUG_CONVERT_ROUTINE
     printf ("In convert_routine(): first_nondefining_declaration = %p = %s \n",first_nondefining_declaration,first_nondefining_declaration != NULL ? first_nondefining_declaration->class_name().c_str() : "NULL");
#endif

     if (first_nondefining_declaration != NULL)
        {
       // DQ (5/23/2013): If this has already been defined, then it should be built to be consistant with its original first non-declaration.
          if (buildTemplateInstantiation == true && isSgTemplateInstantiationFunctionDecl(first_nondefining_declaration) == NULL && isSgTemplateInstantiationMemberFunctionDecl(first_nondefining_declaration) == NULL)
             {
#if 0
               printf ("In convert_routine(): Detected earlier declaration as non-template instantiation, so make the new function being built be consitant: reset buildTemplateInstantiation = false \n");
#endif
               buildTemplateInstantiation = false;
             }
        }

  // DQ (6/28/2013): Adding support for marking template_argument_list_is_explicit.
     if (first_nondefining_declaration != NULL)
        {
          SgTemplateInstantiationFunctionDecl* templateInstantiationFunctionDecl = isSgTemplateInstantiationFunctionDecl(first_nondefining_declaration);
          if (templateInstantiationFunctionDecl != NULL)
             {
#if 0
               printf ("templateInstantiationFunctionDecl->get_template_argument_list_is_explicit() = %s \n",templateInstantiationFunctionDecl->get_template_argument_list_is_explicit() ? "true" : "false");
#endif
             }

          SgTemplateInstantiationMemberFunctionDecl* templateInstantiationMemberFunctionDecl = isSgTemplateInstantiationMemberFunctionDecl(first_nondefining_declaration);
          if (templateInstantiationMemberFunctionDecl != NULL)
             {
#if 0
               printf ("templateInstantiationMemberFunctionDecl->get_template_argument_list_is_explicit() = %s \n",templateInstantiationMemberFunctionDecl->get_template_argument_list_is_explicit() ? "true" : "false");
#endif
             }
        }

#if 0
     printf ("In convert_routine(): calling summarize_routine() \n");
     summarize_routine(p);
#endif

#if DEBUG_CONVERT_ROUTINE
     printf ("In convert_routine(): p->defined_outside_of_parent = %s \n",p->defined_outside_of_parent ? "true" : "false");
#endif

  // DQ (4/9/2012): Test the maps that we use internally (see function for documentation).
     testEdgRoseMaps();

#if DEBUG_CONVERT_ROUTINE
     printf ("In convert_routine(): return_defining_declaration = %p \n",return_defining_declaration);
     printf ("In convert_routine(): topScopeStack() = %p = %s \n",topScopeStack(),topScopeStack()->class_name().c_str());
#endif

  // DQ (11/24/2015): Record that we are building a 2nd defining function declaration 
  // to make up for the original one being normalized away in EDG's representation of 
  // the string for the associated template declaration.
     bool is2ndDefiningDeclarationFromNormalizedEdgTemplateDeclaration               = false;
     bool is1stDefiningDeclarationFromNormalizedEdgTemplateDeclarationMarkedAsFriend = false;

     if (return_defining_declaration != NULL)
        {
#ifdef DEBUG_CONVERSION
          indent_line(); debug_print("-- already translated\n");
#endif
#if DEBUG_CONVERT_ROUTINE
          printf ("In convert_routine(): Note that p = %p edg_template = %p has already been translated into return_defining_declaration = %p = %s \n",p,edg_template,return_defining_declaration,return_defining_declaration->class_name().c_str());
#endif
       // DQ (4/27/2012): Added test to make sure this is present in the function cache.
          ROSE_ASSERT(function_cache.find(p) != function_cache.end());
#if DEBUG_CONVERT_ROUTINE
          printf ("In convert_routine(): first_nondefining_declaration = %p = %s = %s name = %s \n",first_nondefining_declaration,first_nondefining_declaration->class_name().c_str(),SageInterface::generateUniqueName(first_nondefining_declaration,false).c_str(),first_nondefining_declaration->get_name().str());
#endif

       // DQ (4/3/2012): This is a problem for some template code (see test2012_59.C).
       // In this case we want to provide another defining declaration (the first will 
       // have been for the function in the templated class, and the second will be for 
       // the function in the specialized class).  To EDG they are pointers to the same 
       // function (the same pointer value), but to ROSE we want them to be different 
       // defining functions (one for the template class declaration, and the other for 
       // the template class instantiation declaration).  In each case they will be a 
       // function (SgFunctionDeclaration in ROSE).
#if 1
       // DQ (11/24/2015): In this case the non-defining declaration may not exist in 
       // the code and if so then it's parent will be marked as SgGlobal. We need a
       // way to record this 2nd defining declaration as something that should be
       // unparsed or not unparsed as part of AST in the 2nd location were it is 
       // present where the defining declaration was normalized away in EDG's 
       // representation of the template string.
          is2ndDefiningDeclarationFromNormalizedEdgTemplateDeclaration = true;

          if (return_defining_declaration->get_declarationModifier().isFriend() == true)
             {
            // Save the information about the EDG normalized defining declaration being 
            // marked as a friend declaration. So that we can mark the 2nd defining declaration
            // without marking the first non-defining declaration (which would cause the 
            // non-defining declaration to be unparsed incorrectly).
#if DEBUG_CONVERT_ROUTINE
               printf ("In convert_routine(): The defining declaration was marked as friend, we need to reproduce this in the new defining declaration, or the non-definng declaration \n");
               printf ("   --- (before reset): first_nondefining_declaration->get_declarationModifier().isFriend() = %s \n",
                    first_nondefining_declaration->get_declarationModifier().isFriend() ? "true" : "false");
#endif
               is1stDefiningDeclarationFromNormalizedEdgTemplateDeclarationMarkedAsFriend = true;
             }
#if DEBUG_CONVERT_ROUTINE
          printf ("In convert_routine(): This is a special case of building multiple defining declarations (of different functions from ROSE's perspective but using the same function pointer from EDG's perspective) (see test2012_59.C for an example). \n");
#endif
#if 0
          printf ("In convert_routine(): Skip returning the previously built return_defining_declaration = %p = %s \n",
               return_defining_declaration,return_defining_declaration->class_name().c_str());

       // DQ (4/3/2012): Reset this to NULL...test this...
       // printf ("Resetting return_defining_declaration AND first_nondefining_declaration to NULL so that we can rebuild using a new declaration \n");
          printf ("Resetting return_defining_declaration to NULL so that we can rebuild using a new declaration \n");
#endif
          return_defining_declaration   = NULL;

          ROSE_ASSERT(first_nondefining_declaration != NULL);
          ROSE_ASSERT(first_nondefining_declaration->get_definingDeclaration() != NULL);

#if 1
          if (first_nondefining_declaration->get_definingDeclaration()->get_declarationModifier().isFriend() == true)
             {
#if DEBUG_CONVERT_ROUTINE
               printf ("In convert_routine(): The defining declaration was marked as friend, we need to reproduce this in the new defining declaration, or the non-definng declaration \n");
               printf ("   --- (before reset): first_nondefining_declaration->get_declarationModifier().isFriend() = %s \n",first_nondefining_declaration->get_declarationModifier().isFriend() ? "true" : "false");
#endif
               first_nondefining_declaration->get_declarationModifier().setFriend();
#if DEBUG_CONVERT_ROUTINE
               printf ("   --- (after reset): first_nondefining_declaration->get_declarationModifier().isFriend()  = %s \n",first_nondefining_declaration->get_declarationModifier().isFriend() ? "true" : "false");
#endif
#if 0
               printf ("Exiting as a test! \n");
               ROSE_ASSERT(false);
#endif
             }
#endif

       // DQ (11/23/2015): Skip resetting the first_nondefining_declaration.
       // first_nondefining_declaration = NULL;

       // printf ("This is a problem for some template code (see test2012_59.C). \n");
       // ROSE_ASSERT(false);
#if 0
          printf ("In convert_routine(): calling summarize_routine() special case of building multiple defining declarations: before building a new defining declaration \n");
          summarize_routine(p);
#endif
       // return return_defining_declaration;
#endif
        }
       else
        {
#if DEBUG_CONVERT_ROUTINE
          printf ("In convert_routine(): p->defined = %s \n",p->defined ? "true" : "false");
#endif

       // if (!p->defined)
          if (p->defined == false)
             {
            // DQ (4/30/2012): We see this in test2012_58.C (smaller test code that test2004_35.C), but we don't want to return a NULL pointer....

            // DQ (12/11/2011): This is see in test2004_35.C, but we don't want to return a NULL pointer....

            // It appears that this is a secondary declaration but we may have called the wrong function 
            // (we want to build a secondary template function declaration instead).

#ifdef DEBUG_CONVERSION
               indent_line(); debug_print("-- never defined\n");
#endif
               return_defining_declaration = NULL;
#if DEBUG_CONVERT_ROUTINE
               printf ("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ \n");
               printf ("$$$$$$$$$$$$$$$$$ In convert_routine(): first_nondefining_declaration = %p \n",first_nondefining_declaration);
               printf ("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ \n");
#endif

            // DQ (8/14/2014): This is only violated under MS Windows mode (in processing the iostream header file).
            // ROSE_ASSERT(first_nondefining_declaration == NULL);
               if (first_nondefining_declaration != NULL)
                  {
                 // DQ (3/6/2017): Converted to use message logging.
                    mprintf ("WARNING: In convert_routine(): This is only violated under MS Windows mode (in processing the iostream header file). \n");
                  }

            // DQ (3/31/2012): I would like to continue to process this routine since test2012_58.C 
            // demonstrates that code should be generated to support the current function.
#if 0
            // DQ (9/7/2014): Debugging code.
               if (buildTemplateDeclaration == true)
                  {
                    printf ("In convert_routine(): (buildTemplateDeclaration == true): calling summarize_routine() I don't want to return a NULL pointer value here!\n");
                    summarize_routine(p);

                    if (p->assoc_template != NULL)
                       {
                         printf ("In convert_routine(): (buildTemplateDeclaration == true): calling summarize_template() for p->assoc_template = %p I don't want to return a NULL pointer value here!\n",p->assoc_template);
                         summarize_template(p->assoc_template);
                       }
                  }
#endif
             }
            else
             {
#if DEBUG_CONVERT_ROUTINE
               printf ("In convert_routine(): This is a defined function (p->defined == true), but this might be the prototype: p->is_prototype_instantiation = %s \n",(p->is_prototype_instantiation == true) ? "true" : "false");
#endif
             }
        }

     bool addSymbolWhenFirstNondefiningDeclarationIsBuilt = false;

  // DQ (12/9/2011): Adding test...debugging test2006_55.C
#if DEBUG_CONVERT_ROUTINE
     printf ("Inside of convert_routine(): first_nondefining_declaration = %p \n",first_nondefining_declaration);
#endif

     if (first_nondefining_declaration != NULL)
        {
#if 0
          printf ("Inside of convert_routine(): first_nondefining_declaration = %p = %s \n",first_nondefining_declaration,first_nondefining_declaration->class_name().c_str());
#endif

       // DQ (1/27/2013): Added more debugging support designed to catch an error noticed in ROSE compiling ROSE. This is redundant!
       // verifyFunctionHasSymbolInAssociatedScope(first_nondefining_declaration);

       // DQ (12/17/2011): We should not be adding null pointers to the cache in our tests!
       // DQ (12/9/2011): If this was previously built then there should be a symbol for it in the function_cache.
       // ROSE_ASSERT(function_cache[p] != NULL);
          if (function_cache.find(p) == function_cache.end())
             {
#if 0
               printf ("In convert_routine(): calling summarize_routine() (function_cache.find(p) == function_cache.end()) \n");
               summarize_routine(p);
#endif
             }
          ROSE_ASSERT(function_cache.find(p) != function_cache.end());
        }
       else
        {
       // This function will have to build the symbol.
          addSymbolWhenFirstNondefiningDeclarationIsBuilt = true;

       // DQ (12/17/2011): This is OK to fail if the function was used before the declaration was seen.
       // It should not have been built yet.
       // ROSE_ASSERT(function_cache.find(p) == function_cache.end());
          if (function_cache.find(p) != function_cache.end())
             {
#if 0
               printf ("This function has been seen, but before it was defined (e.g. in a class definition) \n");
#endif
               ROSE_ASSERT(function_cache[p] != NULL);
             }
        }

#if DEBUG_CONVERT_ROUTINE
     printf ("In convert_routine(): first_nondefining_declaration = %p = %s \n",first_nondefining_declaration,first_nondefining_declaration != NULL ? first_nondefining_declaration->class_name().c_str() : "NULL");
     printf ("In convert_routine(): defining_declaration          = %p = %s \n",return_defining_declaration,  return_defining_declaration   != NULL ? return_defining_declaration->class_name().c_str() : "NULL");
#endif

#if 0
     printf ("In convert_routine(): calling summarize_routine() \n");
     summarize_routine(p);

     printf ("In convert_routine(): calling summarize_template() for p->assoc_template = %p \n",p->assoc_template);
     if (p->assoc_template != NULL)
        {
          summarize_template(p->assoc_template);
        }
#endif

  // DQ (9/5/2014): This adds the associated template declaration function (member or non-member function) to the AST
  // if it has not yet been previously processed.  This step fixes a problem where EDG rewrites the template class 
  // declarations to make functions defined in the class be function prototypes (which had caused the defining template
  // declaration to get lost.  This was never a problem for compiling code, except for the Intel compiler which caused 
  // an error for test2014_186.C and test2014_189.C (not reproducable with the GNU g++ compiler, and alo specific to 
  // the use within un-named namespace (liekly this was a bug in the Intel compiler).
     if (p->assoc_template != NULL && buildTemplateInstantiation == true)
        {
       // DQ (9/5/2014): Check if the associated template has been translated to a ROSE AST.
#if 0
          printf ("Check if the associated template has been translated to a ROSE AST: p->assoc_template = %p \n",p->assoc_template);
#endif
          SgDeclarationStatement* templateDeclaration = NULL;

#ifdef DEBUG_CONVERSION
          bool definingTemplateDeclarationPreviouslyBuilt = false;
#endif
          if (hasDefiningDeclForTemplate(p->assoc_template))
             {
#if 0
               printf ("Found a defining template declaration for p->assoc_template = %p \n",p->assoc_template);
#endif
#if 0
               templateDeclaration = defining_declarations_for_templates[p->assoc_template];
               printf ("Found a template declaration (1st attempt) in defining_declarations_for_templates: templateDeclaration = %p = %s \n",templateDeclaration,templateDeclaration->class_name().c_str());

               SgDeclarationStatement* nondefiningDeclaration = templateDeclaration->get_firstNondefiningDeclaration();
               printf ("   --- non-defining declaration = %p = %s \n",nondefiningDeclaration,(nondefiningDeclaration != NULL) ? nondefiningDeclaration->class_name().c_str() : "null");
               SgDeclarationStatement* definingDeclaration = templateDeclaration->get_definingDeclaration();
               printf ("   --- defining declaration = %p = %s \n",definingDeclaration,(definingDeclaration != NULL) ? definingDeclaration->class_name().c_str() : "null");
#endif
#ifdef DEBUG_CONVERSION
               definingTemplateDeclarationPreviouslyBuilt = true;
#endif
             }
       // ROSE_ASSERT(templateDeclaration != NULL);

          if (hasFirstNondefDeclForTemplate(p->assoc_template))
             {
               templateDeclaration = getFirstNondefDeclForTemplate(p->assoc_template);
#if 0
               printf ("Found a template declaration in first_nondefining_declarations_for_templates: templateDeclaration = %p = %s \n",templateDeclaration,templateDeclaration->class_name().c_str());
#endif
#ifdef DEBUG_CONVERSION
               SgDeclarationStatement* definingDeclaration = templateDeclaration->get_definingDeclaration();
#endif
#if 0
               SgDeclarationStatement* nondefiningDeclaration = templateDeclaration->get_firstNondefiningDeclaration();
               printf ("   --- non-defining declaration = %p = %s \n",nondefiningDeclaration,(nondefiningDeclaration != NULL) ? nondefiningDeclaration->class_name().c_str() : "null");
               printf ("   --- defining declaration = %p = %s \n",definingDeclaration,(definingDeclaration != NULL) ? definingDeclaration->class_name().c_str() : "null");
#endif

            // DQ (9/8/2014): Check for a saved defining decalaration (processed but output into the AST yet as a result of following EDG template class normalization rules).
               if (saved_defining_declarations_from_class_templates.find(p->assoc_template) != saved_defining_declarations_from_class_templates.end())
                  {
                 // Get the saved version of the defining template declaration from the cache.
                    SgFunctionDeclaration* generatedFunctionFromTemplate = saved_defining_declarations_from_class_templates[p->assoc_template];

                 // This is where we will insert the function declaration (not associated with it's scope, as in member functions declared outside of a class definition).
                    SgScopeStatement* currentScope = topScopeStack();
#if 0
                    printf ("In convert_routine(): topScopeStack() = %p = %s \n",currentScope,currentScope->class_name().c_str());
                    printf ("Retrieved defining function from saved_defining_declarations_from_class_templates map: generatedFunctionFromTemplate = %p = %s = %s \n",
                         generatedFunctionFromTemplate,generatedFunctionFromTemplate->class_name().c_str(),generatedFunctionFromTemplate->get_name().str());
#endif
                    bool skip_saving_template_string = false;

                 // DQ (9/8/2014): The use of multiple instantiations from a single template will cause it to be added ahead 
                 // of the first template instantiation.  See test2014_199.C for a simple example.
                 // Note that if it has previously been added to the AST, then it will have a valid parent pointer (so check).
                 // ROSE_ASSERT(generatedFunctionFromTemplate->get_parent() == NULL);
                    if (generatedFunctionFromTemplate->get_parent() == NULL)
                       {
                         appendStatement(generatedFunctionFromTemplate, currentScope);
                         ROSE_ASSERT(generatedFunctionFromTemplate->get_parent() != NULL);

                      // DQ (9/6/2015): This is failing for test2012_65.C (I think because it is a null pointer at times).
                      // string edg_template_string = edg_template->text;
                         string edg_template_string;
                         if (p->assoc_template->text != NULL)
                            {
                              edg_template_string = p->assoc_template->text;
                            }
                           else
                            {
                           // DQ (9/7/2014): I think we should supress the generation of a defining template member or non-member function declaration in this case.
#if 0
                              printf ("If the template function body has not beeen saved then we likely don't need to build a defining declaration \n");
#endif
                           // Check p->inline_in_class_definition

                              skip_saving_template_string = true;
                           // ROSE_ASSERT(false);
                            }
#if 0
                         printf ("In convert_routine(): edg_template_string = %s \n",edg_template_string.c_str());
#endif
                      // This is required for the C++11 tests (test2014_04.C).
                         if (skip_saving_template_string == false)
                            {
                              SgTemplateFunctionDeclaration*       templateFunctionDeclaration       = isSgTemplateFunctionDeclaration(generatedFunctionFromTemplate);
                              SgTemplateMemberFunctionDeclaration* templateMemberFunctionDeclaration = isSgTemplateMemberFunctionDeclaration(generatedFunctionFromTemplate);
                              if (templateFunctionDeclaration != NULL)
                                 {
                                   templateFunctionDeclaration->set_string(edg_template_string);
                                   templateFunctionDeclaration->set_string_represents_function_body(true);
                                 }
                                else
                                 {
                                   if (templateMemberFunctionDeclaration == NULL)
                                      {
                                        printf ("Error: What is this if not a template function or templae member function generatedFunctionFromTemplate = %p = %s \n",
                                             generatedFunctionFromTemplate,generatedFunctionFromTemplate->class_name().c_str());
                                      }
                                   ROSE_ASSERT(templateMemberFunctionDeclaration != NULL);
                                   templateMemberFunctionDeclaration->set_string(edg_template_string);
                                   templateMemberFunctionDeclaration->set_string_represents_function_body(true);
                                 }
                            }
#if 0
                         printf ("Exiting as a test! \n");
                         ROSE_ASSERT(false);
#endif
                       }

                 // ROSE_ASSERT(generatedFunctionFromTemplate->get_parent() != NULL);
                    ROSE_ASSERT(skip_saving_template_string == true || generatedFunctionFromTemplate->get_parent() != NULL);
#if 0
                    printf ("In convert_routine(): after appendStatement(): generatedFunctionFromTemplate = %p = %s \n",generatedFunctionFromTemplate,generatedFunctionFromTemplate->class_name().c_str());
#endif

                 // Remove the entry from the saved_defining_declarations_from_class_templates (we have a 
                 // test of the parent to make sure it has not been previously added, so it is not required 
                 // to remove the map entry).
#if 0
                 // DQ (1/20/2016): We need to remove this from the map to avoid it being reused again (I think).
                    printf ("Please remove generatedFunctionFromTemplate from saved_defining_declarations_from_class_templates map \n");
                    ROSE_ASSERT(false);
#endif
                  }
                 else
                  {
// #ifdef DEBUG_CONVERSION
#if 0
                    printf ("WARNING: The associated saved defining template function (member or non-member) declaration was not found (might not exist or is defined outside of the class) \n");
                 // ROSE_ASSERT(false);
#endif
                  }

#ifdef DEBUG_CONVERSION
            // DQ (9/8/2014): Note that if the definingDeclaration is NULL; then we want to address this after searching the defining_declarations_for_templates cache.
               if (definingDeclaration == NULL && definingTemplateDeclarationPreviouslyBuilt == false)
                  {
                    printf ("Note: This case might be OK: if there is no defining declaration available \n");
                    printf ("   --- definingTemplateDeclarationPreviouslyBuilt = %s \n",definingTemplateDeclarationPreviouslyBuilt ? "true" : "false");
                    printf ("   --- definingDeclaration = %p = %s \n",definingDeclaration,(definingDeclaration != NULL) ? definingDeclaration->class_name().c_str() : "null");
                  }
            // ROSE_ASSERT( (definingDeclaration == NULL && definingTemplateDeclarationPreviouslyBuilt == false) == false);
#endif
             }
       // ROSE_ASSERT(templateDeclaration != NULL);

          if (hasDefiningDeclForTemplate(p->assoc_template))
             {
               templateDeclaration = getDefiningDeclForTemplate(p->assoc_template);
#if DEBUG_CONVERT_ROUTINE
               printf ("Found a template declaration (2nd attempt) in defining_declarations_for_templates: templateDeclaration = %p = %s \n",templateDeclaration,templateDeclaration->class_name().c_str());

               SgDeclarationStatement* nondefiningDeclaration = templateDeclaration->get_firstNondefiningDeclaration();
               printf ("   --- non-defining declaration = %p = %s \n",nondefiningDeclaration,(nondefiningDeclaration != NULL) ? nondefiningDeclaration->class_name().c_str() : "null");
               SgDeclarationStatement* definingDeclaration = templateDeclaration->get_definingDeclaration();
               printf ("   --- defining declaration = %p = %s \n",definingDeclaration,(definingDeclaration != NULL) ? definingDeclaration->class_name().c_str() : "null");
#endif
               ROSE_ASSERT(templateDeclaration != NULL);
             }
            else
             {
#if 0
               printf ("Warning: (defining_declarations_for_templates.find(p->assoc_template) == defining_declarations_for_templates.end()): p->assoc_template = %p \n",p->assoc_template);
               printf ("templateDeclaration = %p \n",templateDeclaration);

               printf ("In convert_routine(): calling summarize_routine() \n");
               summarize_routine(p);

               printf ("In convert_routine(): calling summarize_template() for p->assoc_template = %p \n",p->assoc_template);
               summarize_template(p->assoc_template);
#endif
             }

       // DQ (9/6/2014): Make this a warning instead of an error.
          if (templateDeclaration == NULL)
             {
               mprintf ("Warning: templateDeclaration was not built (appears to happen for boost code) \n");
            // printf ("Warning: templateDeclaration was not built (appears to happen for boost code) \n");
             }
       // ROSE_ASSERT(templateDeclaration != NULL);

#if 0
          printf ("Exiting as a test! \n");
          ROSE_ASSERT(false);
#endif
        }

  // DQ (5/21/2013): test2013_178.C demonstrates that this test is not acceptable for a firent function.
  // DQ (4/3/2012): This is tests again below...but only for the case of non-class members...so restrict our test here...
  // DQ (4/3/2012): Test this to make sure it is the correct type (matching what we will build).
  // testFirstNondefiningDeclarationType(first_nondefining_declaration,buildTemplateDeclaration,buildTemplateInstantiation);
  // if (p->source_corresp.is_class_member == false)
     if (p->source_corresp.is_class_member == false && p->defined_in_friend_decl == false)
        {
          testFirstNondefiningDeclarationType(first_nondefining_declaration,buildTemplateDeclaration,buildTemplateInstantiation);
        }

  // ******************************* Refactorizable Code **********************************
  // DQ (9/6/2014): Where we have passed in the optional non-defining template declaration (member or non-member) 
  // we want to set the scope to be the scope of the non-defining declaration (a template class definition where 
  // it was originally defined in the template class but was edited out as a normalization by EDG.
     SgScopeStatement* scopeOfFunction = NULL;
     if (optional_nondefiningTemplateDeclaration != NULL)
        {
       // This is the special case to force construction of template member and non-member function defining declarations.
          scopeOfFunction = optional_nondefiningTemplateDeclaration->get_scope();
          ROSE_ASSERT(scopeOfFunction != NULL);
#if 0
          printf ("Resetting the scopeOfFunction using the optional_nondefiningTemplateDeclaration: scopeOfFunction = %p = %s \n",scopeOfFunction,scopeOfFunction->class_name().c_str());
#endif
        }
       else
        {
       // This is the more common case.
#if DEBUG_GET_DECLARATION_SCOPE
          printf ("In convert_routine(): part 2: calling getDeclarationScope() \n");
#endif
          scopeOfFunction = getDeclarationScope(p->source_corresp);
          ROSE_ASSERT(scopeOfFunction != NULL);
        }

     ROSE_ASSERT(scopeOfFunction != NULL);

#if DEBUG_CONVERT_ROUTINE
     printf ("In convert_routine(): scopeOfFunction = %p = %s \n",scopeOfFunction,scopeOfFunction->class_name().c_str());
     printf ("In convert_routine(): p->defined_in_friend_decl = %s \n",(p->defined_in_friend_decl == true) ? "true" : "false");
     printf ("In convert_routine(): buildTemplateDeclaration   = %s \n",(buildTemplateDeclaration == true) ? "true" : "false");
     printf ("In convert_routine(): buildTemplateInstantiation = %s \n",(buildTemplateInstantiation == true) ? "true" : "false");
     printf ("In convert_routine(): topScopeStack() = %p = %s \n",topScopeStack(),topScopeStack()->class_name().c_str());
#endif

  // DQ (7/22/2012): Moving template argument generation to earlier in the generation of functions so that template names with arguments can be passed to the AST builder interface.
     a_template_arg_ptr templateArgs = p->template_arg_list; // p->variant.class_struct_union.extra_info->template_arg_list;
     SgTemplateArgumentPtrList templateArgumentsList;

  // DQ (1/30/2013): Added assertion. This fails for test2004_11.C.
  // ROSE_ASSERT(buildTemplateInstantiation == false || (buildTemplateInstantiation == true && templateArgs != NULL) );

#if DEBUG_CONVERT_ROUTINE
     printf ("In convert_routine(): Calling parse_template_parameters(): p = %p scopeOfFunction = %p = %s \n",p,scopeOfFunction,scopeOfFunction->class_name().c_str());
#endif

     SgTemplateParameterPtrList templateParameterList = parse_template_parameters(p,scopeOfFunction,NULL);

#if DEBUG_CONVERT_ROUTINE
     printf ("DONE: In convert_routine(): Calling parse_template_parameters() \n");
#endif

  // DQ (8/11/2013): This can of course be empty for non-template functions.
  // ROSE_ASSERT(templateParameterList.empty() == false);
#if DEBUG_CONVERT_ROUTINE
     printf ("In convert_routine(): templateParameterList.size() = %" PRIuPTR " \n",templateParameterList.size());
#endif

  // DQ (8/13/2013): Adding test of template parameter lists.
     SgTemplateFunctionDeclaration* first_nondefining_templateFunctionDeclaration = isSgTemplateFunctionDeclaration(first_nondefining_declaration);
     if (first_nondefining_templateFunctionDeclaration != NULL)
        {
#if DEBUG_CONVERT_ROUTINE
          printf ("   --- first_nondefining_templateFunctionDeclaration->get_name() = %s \n",first_nondefining_templateFunctionDeclaration->get_name().str());
          printf ("   --- first_nondefining_templateFunctionDeclaration->get_templateParameters().size() = %" PRIuPTR " \n",first_nondefining_templateFunctionDeclaration->get_templateParameters().size());
          printf ("   --- templateParameterList.size()                                                   = %" PRIuPTR " \n",templateParameterList.size());
#endif
          ROSE_ASSERT(templateParameterList.size() == first_nondefining_templateFunctionDeclaration->get_templateParameters().size());
        }

  // DQ (5/21/2016): Calling refactored code
     templateArgumentsList = setupMatchingTemplateArguments(buildTemplateInstantiation,first_nondefining_declaration,templateArgs,scopeOfFunction);

#if DEBUG_CONVERT_ROUTINE
     printf ("In convert_routine(): buildTemplateInstantiation = %s \n",buildTemplateInstantiation ? "true" : "false");
     printf ("In convert_routine(): templateArgumentsList.size() = %" PRIuPTR " \n",templateArgumentsList.size());
#endif

  // DQ (8/6/2013): If this is the case of building a template then we need to get the template parameters.  
  // However, sometimes these are only available in the template argument list.
     if (buildTemplateDeclaration == true)
        {
       // Even if this is a template declaration, we may have to interpret the template parameters to distinquish possible overloading.
#if 0
          printf ("p->template_arg_list (templateArgs) = %p \n",p->template_arg_list);
#endif
          if ( templateArgs != NULL)
             {
#if 0
               printf ("We have to build template parameters (maybe from the template arguments) so that we can handle overloaded templates \n");
#endif
            // DQ (8/6/2013): This should certainly be marked as a template function.
               ROSE_ASSERT(p->is_template_function == true);

            // DQ (5/21/2016): We can't test the templateArgumentListSetup, since that code was refactored to another scope.
            // DQ (8/8/2013): This should not have been setup yet.
               ROSE_ASSERT(templateArgumentsList.empty() == true);

            // DQ (7/6/2016): Is it possible that we are confusing template arguments and template parameters.
            // the template declaration has teplate parameters, but maybe for specializations include what EDG 
            // is calling template arguments.
            // STL list copy
               templateArgumentsList = parse_template_arguments(templateArgs,scopeOfFunction);
             }

       // DQ (7/6/2016): I am hping we can enforce this (but maybe not for partial specializations).
          ROSE_ASSERT(buildTemplateInstantiation == false);
        }

     if (buildTemplateInstantiation == true)
        {
#if 0
       // ROSE_ASSERT(isSgTemplateInstantiationDecl(nonDefiningDecl) != NULL);
          if (nonDefiningDecl != NULL)
             {
               ROSE_ASSERT(isSgTemplateInstantiationDecl(nonDefiningDecl) != NULL);
             }
#endif
          nameWithTemplateArguments = appendTemplateArgumentsToName(nameWithOutTemplateArguments,templateArgumentsList);

#if DEBUG_CONVERT_ROUTINE
          printf ("In convert_routine(): after appendTemplateArgumentsToName(): nameWithTemplateArguments = %s \n",nameWithTemplateArguments.str());
#endif

       // DQ (1/29/2013): Keep track of names associated with functions, it appears that in some rare cases the computed names 
       // of template instantiations can change.  As part of debugging this we keep a static map of EDG function string names.
       // We record the names with template arguments, since this is the aspect that appears to be different.
          if (function_name_cache.find(p) != function_name_cache.end())
             {
               SgName saved_function_name = function_name_cache[p];
               if (saved_function_name != nameWithTemplateArguments)
                  {
                 // Note that this error has only so far been seen in ROSE compiling the ROSE header files. In this case we 
                 // need to use the original name (which I think is type equivalent, but not string equivalent). Using
                 // the original name is important because the symbol table handling requires consistant naming.

                    printf ("WARNING: In convert_routine(): function_name_cache[p = %p] = %s nameWithTemplateArguments = %s \n",p,saved_function_name.str(),nameWithTemplateArguments.str());
                 // ROSE_ASSERT(false);

                    nameWithTemplateArguments = saved_function_name;
#if 0
                    printf ("In convert_routine(): calling summarize_routine() for insert function \n");
                    summarize_routine(p);
#endif
                  }
             }
            else
             {
#if 0
               if (nameWithTemplateArguments == "insert < __normal_iterator< SgInitializedName ** , __type  >  > ")
                  {
                    printf ("templateArgs = %p \n",templateArgs);
                    printf ("SgTemplateArgumentPtrList: (size = %" PRIuPTR ") \n",templateArgumentsList.size());
                    for (size_t i = 0; i < templateArgumentsList.size(); i++)
                       {
                         printf ("templateArgumentsList[i = %" PRIuPTR "] = %p \n",i,templateArgumentsList[i]);
                       }

                    printf ("In convert_routine(): calling summarize_routine() for insert function (for insert < __normal_iterator< SgInitializedName ** , __type  >  >) \n");
                    summarize_routine(p);
                  }
#endif
               function_name_cache[p] = nameWithTemplateArguments;
             }
        }

#if DEBUG_CONVERT_ROUTINE
     printf ("In convert_routine(): nameWithOutTemplateArguments = %s \n",nameWithOutTemplateArguments.str());
     printf ("In convert_routine(): nameWithTemplateArguments    = %s \n",nameWithTemplateArguments.str());
#endif

#if 0
  // DQ (1/27/2013): Added debugging code.
  // if (nameWithOutTemplateArguments == "insert")
     if (nameWithOutTemplateArguments == "insert" && buildTemplateInstantiation == true)
        {
          printf ("In convert_routine(): nameWithOutTemplateArguments = %s \n",nameWithOutTemplateArguments.str());
          printf ("In convert_routine(): nameWithTemplateArguments    = %s \n",nameWithTemplateArguments.str());

          printf ("In convert_routine(): calling summarize_routine() for insert function \n");
          summarize_routine(p);
        }
#endif

#if DEBUG_CONVERT_ROUTINE
     printf ("In convert_routine(): Before templateArgumentListEquivalence() test: buildTemplateInstantiation = %s first_nondefining_declaration = %p \n",
          buildTemplateInstantiation ? "true" : "false",first_nondefining_declaration);
#endif

  // DQ (5/19/2016): Adding test on template arguments and those in the first_nondefining_declaration.
     if (buildTemplateInstantiation == true && first_nondefining_declaration != NULL)
        {
       // ROSE_ASSERT(first_nondefining_declaration != NULL);
          SgTemplateInstantiationMemberFunctionDecl*first_nondefiningTemplateInstantiationMemberFunctionDeclaration = 
               isSgTemplateInstantiationMemberFunctionDecl(first_nondefining_declaration->get_firstNondefiningDeclaration());
          if (first_nondefiningTemplateInstantiationMemberFunctionDeclaration != NULL)
             {
            // ROSE_ASSERT(first_nondefiningTemplateInstantiationMemberFunctionDeclaration != NULL);
               ROSE_ASSERT(first_nondefiningTemplateInstantiationMemberFunctionDeclaration->get_firstNondefiningDeclaration() != NULL);
               ROSE_ASSERT(first_nondefiningTemplateInstantiationMemberFunctionDeclaration->get_firstNondefiningDeclaration() == first_nondefining_declaration);

               SgTemplateArgumentPtrList & templateArgumentsList_from_first_nondefining_declaration = first_nondefiningTemplateInstantiationMemberFunctionDeclaration->get_templateArguments();

               ROSE_ASSERT(SageInterface::templateArgumentListEquivalence(templateArgumentsList, templateArgumentsList_from_first_nondefining_declaration));
             }
        }

  // Generate the name for this function (with template arguments if appropriate).
  // SgName fullNameWithPossibleTemplateArguments    = nameWithTemplateArguments;
  // SgName fullNameWithoutPossibleTemplateArguments = nameWithOutTemplateArguments;

     ROSE_ASSERT(nameWithTemplateArguments != "");
     ROSE_ASSERT(nameWithOutTemplateArguments != "");
  // ******************************* Refactorizable Code **********************************

  // DQ (9/6/2014): We want to define this only once in an outer scope instead of twice in different scopes.
     SgScopeStatement* classDefinition    = NULL;
     SgScopeStatement*  scopeOfDeclaration = NULL;

  // I forget how friend functions appear, but we will want to check that.
  // Note that EDG will mark a static member function as (is_class_member == true).
  // Also, test2007_30.C has an example where (first_nondefining_declaration == NULL)
  // if (p->source_corresp.is_class_member == true)
     if (p->source_corresp.is_class_member == true && first_nondefining_declaration != NULL)
        {
          ROSE_ASSERT(first_nondefining_declaration != NULL);
#if 0
          printf ("In convert_routine(): p->source_corresp.is_class_member == true && first_nondefining_declaration = %p = %s \n",first_nondefining_declaration,first_nondefining_declaration->class_name().c_str());
#endif
       // Make sure that we built a member function declaration
          SgMemberFunctionDeclaration* nondefiningMemberFunctionDeclaration                 = isSgMemberFunctionDeclaration(first_nondefining_declaration);
          SgTemplateMemberFunctionDeclaration* nondefiningTemplateMemberFunctionDeclaration = isSgTemplateMemberFunctionDeclaration(first_nondefining_declaration);

       // ROSE_ASSERT(nondefiningMemberFunctionDeclaration != NULL);
          if (nondefiningMemberFunctionDeclaration != NULL)
             {
               ROSE_ASSERT(nondefiningMemberFunctionDeclaration->get_associatedClassDeclaration() != NULL);
               ROSE_ASSERT(isSgMemberFunctionType(nondefiningMemberFunctionDeclaration->get_type()) != NULL);

            // DQ (12/29/2011): This is true for test2011_195.C, so I think it is an error to assert that it is false.
            // ROSE_ASSERT(buildTemplateInstantiation == false);
             }
            else
             {
               if (nondefiningTemplateMemberFunctionDeclaration != NULL)
                  {
                 // ROSE_ASSERT(nondefiningTemplateMemberFunctionDeclaration->get_associatedClassDeclaration() != NULL);
                    ROSE_ASSERT(isSgMemberFunctionType(nondefiningTemplateMemberFunctionDeclaration->get_type()) != NULL);
                 // ROSE_ASSERT(buildTemplateInstantiation == true);
                    ROSE_ASSERT(buildTemplateInstantiation == false);
                  }
             }

       // DQ (11/27/2011): Adding tests for debugging test2011_162.C.
          ROSE_ASSERT(first_nondefining_declaration->get_scope() != NULL);
          ROSE_ASSERT(isSgClassDefinition(first_nondefining_declaration->get_scope()) != NULL || isSgDeclarationScope(first_nondefining_declaration->get_scope()) != NULL);

       // DQ (1/27/2013): Added more debugging support designed to catch an error noticed in ROSE compiling ROSE. This is redundant!
       // verifyFunctionHasSymbolInAssociatedScope(first_nondefining_declaration);

       // DQ (9/6/2014): We want to define this only once in an outer scope instead of twice in different scopes.
       // DQ (5/10/2012): Added a new test here, but this is a failing test farther down in this function (for test2012_73.C).
       // This test was failing for defining declaration in template declaration (because the EDG string will only output the non-defining prototype declaration.
       // SgClassDefinition* classDefinition = getAssociatedClassDefinitionForMemberFunction(p);
       // SgClassDefinition* classDefinition = getAssociatedClassDefinitionForMemberFunction(p,forceTemplateDeclaration);
          classDefinition = getAssociatedClassDefinitionForMemberFunction(p,forceTemplateDeclaration);
          ROSE_ASSERT(classDefinition != NULL);

       // DQ (9/6/2014): When we specify the optional non-defining template declaration we want to use its scope.
          if (optional_nondefiningTemplateDeclaration != NULL)
             {
            // classDefinition = optional_nondefiningTemplateDeclaration->get_scope();
               classDefinition = scopeOfFunction;
               ROSE_ASSERT(classDefinition != NULL);

               ROSE_ASSERT(optional_nondefiningTemplateDeclaration == first_nondefining_declaration);
#if DEBUG_CONVERT_ROUTINE
               printf ("In convert_routine(): Reset the classDefinition: classDefinition = %p = %s \n",classDefinition,classDefinition->class_name().c_str());
#endif
             }

       // DQ (5/10/2012): This is the essential issue, the scope is not computed correctly when the first_nondefining_declaration is known.
          ROSE_ASSERT(first_nondefining_declaration->get_scope() != NULL);
          if (classDefinition != first_nondefining_declaration->get_scope())
             {
               printf ("Error: classDefinition != first_nondefining_declaration->get_scope() \n");
               printf ("   --- classDefinition = %p = %s \n",classDefinition,classDefinition->class_name().c_str());
               printf ("   --- first_nondefining_declaration->get_scope() = %p = %s \n",first_nondefining_declaration->get_scope(),first_nondefining_declaration->get_scope()->class_name().c_str());
             }
          ROSE_ASSERT(classDefinition == first_nondefining_declaration->get_scope());

#if 0
          classDefinition->get_file_info()->display("classDefinition in convert_routine()");
          classDefinition->get_symbol_table()->print("SgClassDefinition scope for new member function (before calling build function)");
#endif

// #ifdef DEBUG_CONVERSION
#if 0
       // DQ (1/27/2013): This is not a good test since this may not be a template member function (see verifyFunctionHasSymbolInAssociatedScope() for a better test).

       // SgSymbol* associatedSymbol = classDefinition->lookup_template_member_function_symbol(name,nondefiningMemberFunctionDeclaration->get_type());
       // SgSymbol* associatedSymbol = classDefinition->lookup_template_member_function_symbol(nameWithTemplateArguments,nondefiningMemberFunctionDeclaration->get_type());
          SgSymbol* associatedSymbol = classDefinition->lookup_template_member_function_symbol(nameWithTemplateArguments,nondefiningMemberFunctionDeclaration->get_type(),NULL,NULL);
          if (associatedSymbol == NULL)
             {
               printf ("ERROR: associatedSymbol == NULL \n");
               printf ("   --- nondefiningMemberFunctionDeclaration = %p = %s \n",nondefiningMemberFunctionDeclaration,nondefiningMemberFunctionDeclaration->class_name().c_str());
               printf ("   --- classDefinition = %p = %s \n",classDefinition,classDefinition->class_name().c_str());
            // printf ("   --- name = %s \n",name.str());
               printf ("   --- nameWithTemplateArguments = %s \n",nameWithTemplateArguments.str());
            // printf ("   --- nondefiningTemplateMemberFunctionDeclaration->get_type() = %p = %s \n",nondefiningTemplateMemberFunctionDeclaration->get_type(),nondefiningTemplateMemberFunctionDeclaration->get_type()->class_name().c_str());
            // printf ("   --- nondefiningTemplateMemberFunctionDeclaration->get_type()->get_mangled() = %s \n",nondefiningTemplateMemberFunctionDeclaration->get_type()->get_mangled().str());
               printf ("   --- nondefiningMemberFunctionDeclaration->get_type() = %p = %s \n",nondefiningMemberFunctionDeclaration->get_type(),nondefiningMemberFunctionDeclaration->get_type()->class_name().c_str());
               printf ("   --- nondefiningMemberFunctionDeclaration->get_type()->get_mangled() = %s \n",nondefiningMemberFunctionDeclaration->get_type()->get_mangled().str());
             }
       // ROSE_ASSERT(associatedSymbol != NULL);
          if (associatedSymbol == NULL)
             {
               printf ("######## Commented out test for associatedSymbol for nondefiningMemberFunctionDeclaration = %p \n",nondefiningMemberFunctionDeclaration);
             }
#endif
        }
       else
        {
       // DQ (9/6/2014): If first_nondefining_declaration == NULL, we still need to set the classDefinition (now that this has been refactored).
          if (p->source_corresp.is_class_member == true)
             {
#if 0
               printf ("In convert_routine(): (p->source_corresp.is_class_member == true): first_nondefining_declaration = %p \n",first_nondefining_declaration);
#endif
               classDefinition = getAssociatedClassDefinitionForMemberFunction(p,forceTemplateDeclaration);
               ROSE_ASSERT(classDefinition != NULL);
             }
            else
             {
#if 0
               printf ("In convert_routine(): (p->source_corresp.is_class_member == false): first_nondefining_declaration = %p = %s \n",
                    first_nondefining_declaration,(first_nondefining_declaration != NULL) ? first_nondefining_declaration->class_name().c_str() : "null");
#endif

            // DQ (9/6/2014): This is now computed once above (in an outer scope) and used in all inner scopes.
            // For non-member functions we also want to compute the scope one at the top.
            // scopeOfDeclaration = getDeclarationScope(p->source_corresp);

               scopeOfDeclaration = scopeOfFunction;

               ROSE_ASSERT(scopeOfDeclaration != NULL);
             }
        }

  // Use this stored information in EDG to know that we should have a valid non-defining declaration available at this point
     ROSE_ASSERT(p->type->kind == tk_routine);
     if (p->type->variant.routine.extra_info->prototyped == true)
        {
          if (first_nondefining_declaration == NULL)
             {
#ifdef DEBUG_CONVERSION
               printf ("Error: We should have seen the first_nondefining_declaration already, but first_nondefining_declaration == NULL \n");
#endif
             }
       // ROSE_ASSERT(first_nondefining_declaration != NULL);
        }

#if 0
     printf ("In convert_routine(): Generate the function type: calling convert_type(p->type = %p) \n",p->type);
#endif
#if 0
     printf ("In convert_routine(): Calling summarize_type(): p->type = %p \n",p->type);
     summarize_type(p->type);
#endif

  // DQ (7/29/2014): It appears that EDG does not mark the type associated with the function so that 
  // p->type.source_corresp.is_class_member == true when p->source_corresp.is_class_member == true.
     bool forceToBeMemberFunctionType = false;
     if (p->source_corresp.is_class_member == true)
        {
          if (p->type->source_corresp.is_class_member == false)
             {
               a_routine_type_supplement_ptr sup = p->type->variant.routine.extra_info;
               ROSE_ASSERT(sup != NULL);
#if 0
               bool is_member_function = (sup->assoc_routine_is_ctor || sup->assoc_routine_is_dtor);
               if (p->is_defaulted == false && p->is_deleted == false)
                  {
                    printf ("In convert_routine(): (p->is_defaulted == false && p->is_deleted == false): Calling summarize_routine(): p = %p \n",p);
                    summarize_routine(p);
                  }
#endif
            // ROSE_ASSERT(p->is_defaulted || p->is_deleted);

            // When (p->is_defaulted || p->is_deleted), there is a BUG in EDG that prevents the type from 
            // being marked as a class member (thus inconsistant with other constructors and destructors).
#if 0
               printf ("WARNING: Resetting forceToBeMemberFunctionType to supported defaulted or deleted member functions \n");
#endif
               forceToBeMemberFunctionType = true;

#if 0
               printf ("Error: EDG BUG: function type not marked consistantly with the function to be a member function type \n");
               ROSE_ASSERT(false);
#endif
             }
        }

  // DQ (8/8/2014): For C++11 support for the new function return type syntax we want to process the return type 
  // after the function parameters. In this case the function type is generated and it should be that we can 
  // process the return type after the function parameters within the convert_type() function.
  // DQ (8/3/2014): Added to support C++11 new syntax for specification of return types.
     if (p->type->variant.routine.extra_info->trailing_return_type == true)
        {
#if 0
          printf ("In convert_routine(): detected C++11 new syntax for specification of return types (not implemented) \n");
#endif
        }

  // Generate the SgFunctionType.
     SgType* unstrippedFunctionType = nullptr;
     SgType* unstrippedFunctionTypeUndeduced = nullptr;

     std::tie(unstrippedFunctionType, unstrippedFunctionTypeUndeduced) = convert_type2(p->type, true /*with non deduced types*/, forceToBeMemberFunctionType, false);

     ROSE_ASSERT(unstrippedFunctionType != NULL);

#if 0
     printf ("unstrippedFunctionType = %p = %s \n",unstrippedFunctionType,unstrippedFunctionType->class_name().c_str());
     printf ("In convert_routine(): Calling stripType() \n");
#endif

  // SgType* tempFunctionType = convert_type(p->type)->stripType();
     SgType* tempFunctionType = unstrippedFunctionType->stripType();
     ROSE_ASSERT(tempFunctionType != NULL);
     SgFunctionType* function_type = isSgFunctionType(tempFunctionType);

  // DQ (3/12/2019): Adding debugging support (fails for mergeAST_tests/mergeTest_05.C).
     if (function_type == NULL)
        {
          printf ("In convert_routine(): tempFunctionType                          = %p = %s \n",tempFunctionType,tempFunctionType->class_name().c_str());
          SgType* type = unstrippedFunctionType->stripType();
          printf ("In convert_routine(): unstrippedFunctionType->stripType(): type = %p = %s \n",type,type->class_name().c_str());
          printf ("In convert_routine(): forceToBeMemberFunctionType               = %s \n",forceToBeMemberFunctionType ? "true" : "false");
#if 0
          printf ("In convert_routine(): Calling summarize_type(): p->type = %p \n",p->type);
          summarize_type(p->type);
#endif
#if 0
          printf ("In convert_routine(): recursive cal: calling summarize_routine() \n");
          summarize_routine(p);
#endif
        }
     ROSE_ASSERT(function_type != NULL);

#if 0
     printf ("In convert_routine(): function_type = %p = %s \n",function_type,function_type->class_name().c_str());
#endif

     SgType* return_type = convert_type(p->type->variant.routine.return_type);

  // DQ (11/25/2011): Adding support for uniform handling of SgFunctionDeclaration and SgTemplateFunctionDeclaration IR nodes.
     SgFunctionDeclaration*               nondefiningFunctionDeclaration               = NULL;
     SgFunctionDeclaration*               definingFunctionDeclaration                  = NULL;

  // DQ (12/28/2011): These are required for both defining and nondefining and function and member function cases.
  // The template function and template member function cases are required because a SgTemplateMemberFunctionDeclaration 
  // is not derived from a SgTemplateFunctionDeclaration.
     SgTemplateFunctionDeclaration*       nondefiningTemplateFunctionDeclaration       = NULL;
     SgTemplateMemberFunctionDeclaration* nondefiningTemplateMemberFunctionDeclaration = NULL;
     SgTemplateFunctionDeclaration*       definingTemplateFunctionDeclaration          = NULL;
     SgTemplateMemberFunctionDeclaration* definingTemplateMemberFunctionDeclaration    = NULL;


     SgExprListExp* decoratorList            = NULL;  // Python specific option.
     unsigned int functionConstVolatileFlags = 0;

  // DQ (3/8/2012): Added support for function type modifiers (nte that these have to match those associated with the non-defining declaration).
     a_routine_type_supplement_ptr sup = p->type->variant.routine.extra_info;
     ROSE_ASSERT(sup != NULL);
     functionConstVolatileFlags = setupFunctionTypeModifiers(sup);

  // This is the in the case of a member function (and when the constructor preinitialization list is available).
     SgCtorInitializerList* ctor = NULL;

  // DQ (7/27/2014): For C++11 using EDG 4.9, we might have to also check the (routine.extra_info.assoc_routine_is_ctor == true).
  // if (p->source_corresp.is_class_member == true || sup->assoc_routine_is_ctor == true || sup->assoc_routine_is_dtor == true)

  // DQ (11/12/2020): Added support for defining functions to have extern "C" style specifications.
     bool routine_name_linkage_is_explicit = sup->routine_name_linkage_is_explicit;

#if DEBUG_CONVERT_ROUTINE || 0
     printf ("In convert_routine(): p->is_defaulted                   = %s \n",p->is_defaulted            ? "true" : "false");
     printf ("In convert_routine(): p->is_deleted                     = %s \n",p->is_deleted              ? "true" : "false");
     printf ("In convert_routine(): p->source_corresp.is_class_member = %s \n",p->source_corresp.is_class_member ? "true" : "false");
     printf ("In convert_routine(): sup->assoc_routine_is_ctor        = %s \n",sup->assoc_routine_is_ctor ? "true" : "false");
     printf ("In convert_routine(): sup->assoc_routine_is_dtor        = %s \n",sup->assoc_routine_is_dtor ? "true" : "false");
     printf ("In convert_routine(): routine_name_linkage_is_explicit  = %s \n",routine_name_linkage_is_explicit ? "true" : "false");
#endif

     // Static function must be built as such so that symbol get properly set (limited to file scope)
     SgStorageModifier::storage_modifier_enum storage_modifier = SgStorageModifier::e_default;
     if (buildTemplateDeclaration == false && buildTemplateInstantiation == false && p->source_corresp.is_class_member == false) {
       storage_modifier = convert_storage_class(p->storage_class);
     }

#if 0
  // printf ("In convert_routine(): The explicit keyword was used with this function   = %s \n",(p->explicit_storage_class == true) ? "true" : "false");
     printf ("In convert_routine(): The storage class used with this function          = %s \n",summarizeStorageModifier(p->storage_class).c_str());
     printf ("In convert_routine(): The declared storage class used with this function = %s \n",summarizeStorageModifier(p->declared_storage_class).c_str());
     printf ("In convert_routine(): routine_name_linkage_is_explicit                   = %s \n",routine_name_linkage_is_explicit ? "true" : "false");
#endif

#if 0
  // DQ (11/12/2020): Adding code to est the extern "C" lingage support (similar as for the function prototype).
  // if (p->explicit_storage_class == false)
  // if ( (p->explicit_storage_class == false) && (routine_name_linkage_is_explicit == false) )
     if ( routine_name_linkage_is_explicit == true)
        {
          printf ("This is where for a function prototype we set additional fieds in the SgFunctionDeclaration \n");
#if 0
          printf ("Exiting as a test! \n");
          ROSE_ASSERT(false);
#endif
        }
#endif

  // Check if this is a C++ member function
     if (p->source_corresp.is_class_member == true)
        {
       // The SgMemberFunctionType we will build needs extra information about it's associated class definition, 
       // so we get this from the non-defining declaration (since the current topofScope might not be the class 
       // definition if the defining declaration is declared in a different scope.

       // SgMemberFunctionDeclaration* nondefiningMemberFunctionDeclaration = isSgMemberFunctionDeclaration(first_nondefining_declaration);
       // ROSE_ASSERT(nondefiningMemberFunctionDeclaration != NULL);
       // SgClassDefinition* classDefinition = isSgClassDefinition(nondefiningMemberFunctionDeclaration->get_scope());
       // ROSE_ASSERT(classDefinition != NULL);

      // DQ (7/27/2014): This can be an error for a construction in EDG 4.9, it seems that is_class_membr 
      // is false and routine.extra_info.assoc_routine_is_ctor is true.  So this has to be looked into shortly 
      // (see Cxx11_tests/test2014_05.C).
       // DQ (12/1/2011): I think this should be a SgMemberFunctionType.
          if (isSgMemberFunctionType(function_type) == NULL)
             {
               printf ("In convert_routine(): case of member function --- function_type = %p = %s \n",function_type,function_type->class_name().c_str());
             }
          ROSE_ASSERT(isSgMemberFunctionType(function_type) != NULL);

          ROSE_ASSERT(classDefinition != NULL);
#if 0
          classDefinition->get_file_info()->display("classDefinition in convert_routine()");
          classDefinition->get_symbol_table()->print("SgClassDefinition scope for new member function (before calling build function)");
#endif

       // DQ (12/17/2011): Refactored code
          testAndResetTemplateDeclarationAndTemplateInstantiation(p,classDefinition,buildTemplateInstantiation,buildTemplateDeclaration);

       // DQ (4/3/2012): Test this to make sure it is the correct type (matching what we will build).
          testFirstNondefiningDeclarationType(first_nondefining_declaration,buildTemplateDeclaration,buildTemplateInstantiation);

          SgMemberFunctionDeclaration*         nondefiningMemberFunctionDeclaration         = NULL;
          SgMemberFunctionDeclaration*         definingMemberFunctionDeclaration            = NULL;

       // SgMemberFunctionDeclaration* definingMemberFunctionDeclaration                 = new SgMemberFunctionDeclaration(p->source_corresp.name, function_type, NULL);
#if 0
          printf ("p->source_corresp.is_class_member == true: buildTemplateInstantiation = %s \n",buildTemplateInstantiation ? "true" : "false");
          printf ("p->source_corresp.is_class_member == true: buildTemplateDeclaration   = %s \n",buildTemplateDeclaration   ? "true" : "false");
#endif
       // DQ (12/1/2011): This is a rewrite to use the Sage Build Interface.
          if (buildTemplateDeclaration == false)
             {
            // definingMemberFunctionDeclaration = new SgMemberFunctionDeclaration(p->source_corresp.name, function_type, NULL);
               if (first_nondefining_declaration == NULL)
                  {
                 // DQ (12/4/2011): If we build a separate non-defining function, it need it's own paramter list.
#if 0
                    printf ("In convert_routine(): Building a first_nondefining_declaration \n");
#endif
#if 0
                    printf ("In convert_routine(): Calling convert_function_parameter_list() for non-defining non-template member function declaration (skip default arguments = true) \n");
#endif
                 // DQ (4/21/2018): Add support to build a SgDeclarationScope if it is detected in the parameter type constrcution 
                 // that we need one for types that are defined in the function parameter list.  See test2018_41.c for an example.
                    SgDeclarationScope* declarationScope = NULL;

                    bool skipDefaultArguments = true;
                 // SgFunctionParameterList* nondefining_function_param_list = convert_function_parameter_list(p,skipDefaultArguments);
                    SgFunctionParameterList* nondefining_function_param_list = convert_function_parameter_list(p,skipDefaultArguments,NULL,declarationScope);
                    ROSE_ASSERT(nondefining_function_param_list != NULL);
#if 0
                    printf ("In convert_routine(): DONE: Calling convert_function_parameter_list() for non-defining non-template member function declaration (skip default arguments = true) \n");
                    printf ("   --- nondefining_function_param_list = %p \n",nondefining_function_param_list);
#endif

#if 0
                    printf ("Exiting as a test! \n");
                    ROSE_ASSERT(false);
#endif

#if 0
                    printf ("In convert_routine(): calling summarize_routine() (building a member function) \n");
                    summarize_routine(p);
#endif
                 // nondefiningMemberFunctionDeclaration = buildNondefiningMemberFunctionDeclaration(p->source_corresp.name, return_type, nondefining_function_param_list, classDefinition, decoratorList, functionConstVolatileFlags, buildTemplateInstantiation);
                 // nondefiningMemberFunctionDeclaration = buildNondefiningMemberFunctionDeclaration(name, return_type, nondefining_function_param_list, classDefinition, decoratorList, functionConstVolatileFlags, buildTemplateInstantiation);
                 // nondefiningMemberFunctionDeclaration = buildNondefiningMemberFunctionDeclaration(nameWithTemplateArguments, return_type, nondefining_function_param_list, classDefinition, decoratorList, functionConstVolatileFlags, buildTemplateInstantiation);
                 // nondefiningMemberFunctionDeclaration = buildNondefiningMemberFunctionDeclaration(nameWithTemplateArguments, return_type, nondefining_function_param_list, classDefinition, decoratorList, functionConstVolatileFlags, buildTemplateInstantiation,&templateArgumentsList);

                 // DQ (7/5/2016): Where this fails it appears to be building a new nondefining declaration which 
                 // is not using the same template arguments as are in the specified template argument list.
                    nondefiningMemberFunctionDeclaration = buildNondefiningMemberFunctionDeclaration(nameWithOutTemplateArguments, return_type, nondefining_function_param_list, classDefinition, decoratorList, functionConstVolatileFlags, buildTemplateInstantiation, &templateArgumentsList);

                 // DQ (5/21/2016): Add test of equivalence for template arguments.
                 // DQ (5/19/2016): Adding test on template arguments and those in the first_nondefining_declaration.
                    if (buildTemplateInstantiation == true)
                       {
                      // ROSE_ASSERT(first_nondefining_declaration != NULL);
                         SgTemplateInstantiationMemberFunctionDecl* first_nondefiningTemplateInstantiationMemberFunctionDeclaration = 
                              isSgTemplateInstantiationMemberFunctionDecl(nondefiningMemberFunctionDeclaration->get_firstNondefiningDeclaration());
#if 0
                         printf ("Testing of template arguments: part 1.5: first_nondefining_declaration = %p \n",first_nondefining_declaration);
                         printf ("Testing of template arguments: part 1.5: nondefiningMemberFunctionDeclaration = %p \n",nondefiningMemberFunctionDeclaration);
                         printf ("Testing of template arguments: part 1.5: first_nondefiningTemplateInstantiationMemberFunctionDeclaration = %p if non-null \n",
                              first_nondefiningTemplateInstantiationMemberFunctionDeclaration);
#endif
                         if (first_nondefiningTemplateInstantiationMemberFunctionDeclaration != NULL)
                            {
                           // ROSE_ASSERT(first_nondefiningTemplateInstantiationMemberFunctionDeclaration != NULL);
                              ROSE_ASSERT(first_nondefiningTemplateInstantiationMemberFunctionDeclaration->get_firstNondefiningDeclaration() != NULL);

                              if (first_nondefiningTemplateInstantiationMemberFunctionDeclaration->get_firstNondefiningDeclaration() != nondefiningMemberFunctionDeclaration)
                                 {
                                // Test the nondefinig declaration just built.
                                   SgTemplateInstantiationMemberFunctionDecl* nondefiningTemplateInstantiationMemberFunctionDeclaration = 
                                        isSgTemplateInstantiationMemberFunctionDecl(nondefiningMemberFunctionDeclaration);
                                   ROSE_ASSERT(nondefiningTemplateInstantiationMemberFunctionDeclaration != NULL);
#if 0
                                   printf ("Testing nondefiningTemplateInstantiationMemberFunctionDeclaration = %p \n",nondefiningTemplateInstantiationMemberFunctionDeclaration);
#endif
                                   SgTemplateArgumentPtrList & templateArgumentsList_from_nondefining_declaration = nondefiningTemplateInstantiationMemberFunctionDeclaration->get_templateArguments();
                                   ROSE_ASSERT(SageInterface::templateArgumentListEquivalence(templateArgumentsList, templateArgumentsList_from_nondefining_declaration));
                                 }
                           // ROSE_ASSERT(first_nondefiningTemplateInstantiationMemberFunctionDeclaration->get_firstNondefiningDeclaration() == nondefiningMemberFunctionDeclaration);

                              SgTemplateArgumentPtrList & templateArgumentsList_from_first_nondefining_declaration = first_nondefiningTemplateInstantiationMemberFunctionDeclaration->get_templateArguments();

#if 0
                              printf ("Testing of template argument list \n");
#endif

                              bool templateArgListsAreEquivalent = SageInterface::templateArgumentListEquivalence(templateArgumentsList, templateArgumentsList_from_first_nondefining_declaration);
#if 0
                              printf ("part 1.5: templateArgListsAreEquivalent = %s \n",templateArgListsAreEquivalent ? "true" : "false");
#endif
                              if (templateArgListsAreEquivalent == false)
                                 {
                                   if (first_nondefining_declaration != NULL && first_nondefining_declaration->get_file_info() != NULL)
                                        first_nondefining_declaration->get_file_info()->display("first_nondefining_declaration: debug");
                                   if (first_nondefiningTemplateInstantiationMemberFunctionDeclaration != NULL && first_nondefiningTemplateInstantiationMemberFunctionDeclaration->get_file_info() != NULL)
                                        first_nondefiningTemplateInstantiationMemberFunctionDeclaration->get_file_info()->display("first_nondefiningTemplateInstantiationMemberFunctionDeclaration: debug");
                                   printf ("In convert_routine(): calling summarize_routine() after failing templateArgumentListEquivalence() test \n");
                                   summarize_routine(p);

                                   ROSE_ASSERT(templateArgumentsList.size() == templateArgumentsList_from_first_nondefining_declaration.size());
                                   for (size_t i = 0; i < templateArgumentsList.size(); i++)
                                      {
                                        printf ("   --- templateArgumentsList[%zu] = %p = %s (and from 1st non-defining) = %p = %s \n",
                                             i,templateArgumentsList[i],templateArgumentsList[i]->class_name().c_str(),
                                             templateArgumentsList_from_first_nondefining_declaration[i],
                                             templateArgumentsList_from_first_nondefining_declaration[i]->class_name().c_str());
                                      }
                                 }
                              ROSE_ASSERT(templateArgListsAreEquivalent == true);
                           // ROSE_ASSERT(SageInterface::templateArgumentListEquivalence(templateArgumentsList, templateArgumentsList_from_first_nondefining_declaration));
                            }
                       }
#if 0
                    printf ("In convert_routine(): Building a first_nondefining_declaration: nondefiningMemberFunctionDeclaration = %p \n",nondefiningMemberFunctionDeclaration);
                    printf ("   nameWithOutTemplateArguments = %s  \n",nameWithOutTemplateArguments.str());
                    printf ("   nameWithTemplateArguments    = %s  \n",nameWithTemplateArguments.str());
                    printf ("   templateArgumentsList.size() = %zu \n",templateArgumentsList.size());
                    for (size_t i = 0; i < templateArgumentsList.size(); i++)
                       {
                         printf ("   --- templateArgumentsList[%zu] = %p = %s \n",i,templateArgumentsList[i],templateArgumentsList[i]->class_name().c_str());
                       }
#endif
                 // DQ (12/27/20134): Added these to permit testing earlier than in the buildDefiningFunctionDeclaration_T() function.
                    ROSE_ASSERT(nondefiningMemberFunctionDeclaration->get_firstNondefiningDeclaration() != NULL);
                 // ROSE_ASSERT(nondefiningMemberFunctionDeclaration->get_firstNondefiningDeclaration() == nondefiningMemberFunctionDeclaration);

                    nondefiningFunctionDeclaration       = nondefiningMemberFunctionDeclaration;

                 // ROSE_ASSERT(classDefinition->lookup_function_symbol(p->source_corresp.name) != NULL);
                 // ROSE_ASSERT(classDefinition->lookup_function_symbol(name) != NULL);
                    ROSE_ASSERT(classDefinition->lookup_function_symbol(nameWithTemplateArguments) != NULL);
                    ROSE_ASSERT(nondefining_function_param_list->get_parent() != NULL);

                 // Set the source position for the parameter list.
                    set_file_info_for_corresp(nondefiningMemberFunctionDeclaration->get_parameterList(), p->source_corresp);

                    ROSE_ASSERT(nondefiningFunctionDeclaration != NULL);
                    ROSE_ASSERT(nondefiningFunctionDeclaration->get_definition() == NULL);

                 // This is a redundant test (I think).
                 // ROSE_ASSERT(classDefinition->lookup_function_symbol(p->source_corresp.name) != NULL);
                 // ROSE_ASSERT(classDefinition->lookup_function_symbol(name) != NULL);
                    ROSE_ASSERT(classDefinition->lookup_function_symbol(nameWithTemplateArguments) != NULL);

                    if (buildTemplateInstantiation == true)
                       {
                      // Set the template declaration in the template instantiation.
                         setTemplateFunctionDeclaration (p,nondefiningMemberFunctionDeclaration);

                      // DQ (6/28/2013): Set if template arguments should be explicit for function reference expressions.
                         isSgTemplateInstantiationMemberFunctionDecl(nondefiningMemberFunctionDeclaration)->set_template_argument_list_is_explicit(p->expl_template_arg_list_used);
                       }
                  }
                 else
                  {
                 // DQ (5/12/2012): Set nondefiningMemberFunctionDeclaration in this branch as well.
                    nondefiningMemberFunctionDeclaration = isSgMemberFunctionDeclaration(first_nondefining_declaration);
                    ROSE_ASSERT(nondefiningMemberFunctionDeclaration != NULL);

                 // DQ (12/27/20134): Added these to permit testing earlier than in the buildDefiningFunctionDeclaration_T() function.
                    ROSE_ASSERT(nondefiningMemberFunctionDeclaration->get_firstNondefiningDeclaration() != NULL);
                    ROSE_ASSERT(nondefiningMemberFunctionDeclaration->get_firstNondefiningDeclaration() == nondefiningMemberFunctionDeclaration);

                 // nondefiningFunctionDeclaration = first_nondefining_declaration;
                    nondefiningFunctionDeclaration = isSgMemberFunctionDeclaration(first_nondefining_declaration);
                    ROSE_ASSERT(nondefiningFunctionDeclaration != NULL);

                 // ROSE_ASSERT(classDefinition->lookup_function_symbol(p->source_corresp.name) != NULL);
                 // ROSE_ASSERT(classDefinition->lookup_function_symbol(name) != NULL);

#if  MAKE_REDUNDANT_CALLS_TO_SYMBOL_TABLE_LOOKUP
                 // DQ (1/27/2013): Look up the symbol and save value to support debugging.  It appears that we
                 // are finding the symbol when looked up by name, but when looked up by name and function type 
                 // later, the symbol is not found.  This is a bug in ROSE compiling ROSE (in STL vector handling).
                 // if (classDefinition->lookup_function_symbol(nameWithTemplateArguments) == NULL)
                    SgSymbol* tempFunctionSymbol = classDefinition->lookup_function_symbol(nameWithTemplateArguments);
#if 0
                    printf ("In convert_routine(): Testing that function nameWithTemplateArguments = %s is in symbol table in scope = %p = %s tempFunctionSymbol = %p = %s \n",
                         nameWithTemplateArguments.str(),classDefinition,classDefinition->class_name().c_str(),tempFunctionSymbol,tempFunctionSymbol->class_name().c_str());
#endif
                    if (tempFunctionSymbol == NULL)
                       {
#ifdef DEBUG_CONVERSION
                         printf ("ERROR: lookup_function_symbol(nameWithTemplateArguments = %s) == NULL nondefiningMemberFunctionDeclaration = %p = %s \n",nameWithTemplateArguments.str(),nondefiningMemberFunctionDeclaration,nondefiningMemberFunctionDeclaration->get_name().str());
#endif
#if 0
                         classDefinition->get_file_info()->display("classDefinition in convert_routine()");
                         classDefinition->get_symbol_table()->print("SgClassDefinition scope for new member function (before calling build function)");
#endif
                       }
#endif
                    ROSE_ASSERT(classDefinition->lookup_function_symbol(nameWithTemplateArguments) != NULL);

                    if (buildTemplateInstantiation == true)
                       {
                      // DQ (4/3/2012): Added test.
                         ROSE_ASSERT(nondefiningFunctionDeclaration != NULL);
                         SgTemplateInstantiationMemberFunctionDecl* memberFunctionTemplateInstationation = isSgTemplateInstantiationMemberFunctionDecl(nondefiningFunctionDeclaration);
                         if (memberFunctionTemplateInstationation == NULL)
                            {
                              ROSE_ASSERT(nondefiningFunctionDeclaration != NULL);
                              printf ("Error: nondefiningFunctionDeclaration = %p = %s \n",nondefiningFunctionDeclaration,nondefiningFunctionDeclaration->class_name().c_str());
                            }
                         ROSE_ASSERT(memberFunctionTemplateInstationation != NULL);
                         ROSE_ASSERT(memberFunctionTemplateInstationation->get_templateDeclaration() != NULL);
                       }
                  }
#if 0
               classDefinition->get_symbol_table()->print("SgClassDefinition scope for new member function (after calling non-defining build function)");
#endif
            // DQ (12/27/2013): Added code to make sure we are always getting the correct firstNondefiningDeclaration.
            // first_nondefining_declaration = nondefiningFunctionDeclaration;
               ROSE_ASSERT(nondefiningFunctionDeclaration != NULL);
               first_nondefining_declaration = isSgFunctionDeclaration(nondefiningFunctionDeclaration->get_firstNondefiningDeclaration());

               ROSE_ASSERT(first_nondefining_declaration != NULL);

               ROSE_ASSERT(nondefiningMemberFunctionDeclaration != NULL);

#if 1
            // DQ (8/7/2013): Adding support for template functions overloaded using template parameters.
            // DQ (1/27/2013): Added more debugging support designed to catch an error noticed in ROSE compiling ROSE.  This is redundant!
            // verifyFunctionHasSymbolInAssociatedScope(first_nondefining_declaration);
               verifyFunctionHasSymbolInAssociatedScope(first_nondefining_declaration,NULL,&templateArgumentsList);

            // DQ (1/26/2013): Added early test for code failing in buildDefiningFunctionDeclaration_T().
                  {
                 // nameWithTemplateArguments = appendTemplateArgumentsToName(nameWithoutTemplateArguments,templateArgumentsList);
                    SgMemberFunctionType* func_type = isSgMemberFunctionType(first_nondefining_declaration->get_type());
                    ROSE_ASSERT(func_type != NULL);

                 // DQ (8/7/2013): Adding support for template functions overloaded using template parameters.
                 // SgSymbol* func_symbol = classDefinition->get_symbol_table()->find_symbol_by_type_of_function<SgTemplateInstantiationMemberFunctionDecl>(nameWithTemplateArguments,func_type);
                 // SgSymbol* func_symbol = classDefinition->find_symbol_by_type_of_function<SgTemplateInstantiationMemberFunctionDecl>(nameWithTemplateArguments,func_type);
                    SgSymbol* func_symbol = classDefinition->find_symbol_by_type_of_function<SgTemplateInstantiationMemberFunctionDecl>(nameWithTemplateArguments,func_type,NULL,&templateArgumentsList);
                    if (func_symbol == NULL)
                       {
                         printf ("ERROR caught in convert_routine(): nameWithTemplateArguments = %s buildTemplateInstantiation = %s \n",
                              nameWithTemplateArguments.str(),buildTemplateInstantiation ? "true:" : "false");
                         printf ("ERROR caught in convert_routine(): func_symbol == NULL for first_nondefining_declaration = %p = %s = %s and func_type = %p = %s \n",
                              first_nondefining_declaration,first_nondefining_declaration->get_name().str(),first_nondefining_declaration->class_name().c_str(),func_type,func_type->class_name().c_str());
                       }
                 // ROSE_ASSERT(func_symbol != NULL);

                 // DQ (1/27/2013): I think this is the root cause of the problem with ROSE compiling ROSE.  These appear to not be equivalent within some deel STL vector member function processing.
                    if (nameWithTemplateArguments != first_nondefining_declaration->get_name())
                       {
                      // printf ("ERROR: function names are different: nameWithTemplateArguments = %s first_nondefining_declaration->get_name() = %s \n",nameWithTemplateArguments.str(),first_nondefining_declaration->get_name().str());
                         printf ("ERROR: function names are different: nameWithTemplateArguments                 = %s \n",nameWithTemplateArguments.str());
                         printf ("ERROR: function names are different: first_nondefining_declaration->get_name() = %s \n",first_nondefining_declaration->get_name().str());

                      // DQ (9/6/2014): This is debugging code (execution ends in an error afterware).
                         printf ("In convert_routine(): calling summarize_routine() error function names are different between the first-nondefinng and the function we are building \n");
                         summarize_routine(p);

                         printf ("Exiting since this is a serious error! \n");
                         ROSE_ASSERT(false);
                       }
                  }
#endif

            // DQ (12/27/20134): Added these to permit testing earlier than in the buildDefiningFunctionDeclaration_T() function.
               ROSE_ASSERT(nondefiningMemberFunctionDeclaration->get_firstNondefiningDeclaration() != NULL);
            // ROSE_ASSERT(nondefiningMemberFunctionDeclaration->get_firstNondefiningDeclaration() == nondefiningMemberFunctionDeclaration);

            // DQ (12/27/20134): Added these to permit testing earlier than in the buildDefiningFunctionDeclaration_T() function.
               SgMemberFunctionDeclaration* first_nondefiningMemberFunctionDeclaration = isSgMemberFunctionDeclaration(nondefiningMemberFunctionDeclaration->get_firstNondefiningDeclaration());
               ROSE_ASSERT(first_nondefiningMemberFunctionDeclaration != NULL);
               ROSE_ASSERT(first_nondefiningMemberFunctionDeclaration->get_firstNondefiningDeclaration() != NULL);
               ROSE_ASSERT(first_nondefiningMemberFunctionDeclaration->get_firstNondefiningDeclaration() == first_nondefiningMemberFunctionDeclaration);

#if 1
            // DQ (9/9/2014): Build this seperately for each local scope and SgFunctionDeclaration so that we can control where
            // we need these to be build with and without default parameters.
#if 0
               printf ("In convert_routine(): Calling convert_function_parameter_list() to build function type and used in defining declaration (allow default arguments) \n");
#endif
               bool skipDefaultArguments = false;
#if 0
               printf ("In convert_routine(): skipDefaultArguments = %s \n",skipDefaultArguments ? "true" : "false");
#endif
            // DQ (4/21/2018): Add support to build a SgDeclarationScope if it is detected in the parameter type constrcution 
            // that we need one for types that are defined in the function parameter list.  See test2018_41.c for an example.
               SgDeclarationScope* declarationScope = NULL;

            // SgFunctionParameterList* param_list = convert_function_parameter_list(p);
            // SgFunctionParameterList* param_list = convert_function_parameter_list(p,skipDefaultArguments);
               SgFunctionParameterList* param_list = convert_function_parameter_list(p,skipDefaultArguments,NULL,declarationScope);
               ROSE_ASSERT(param_list != NULL);
#if 0
               printf ("In convert_routine(): DONE: Calling convert_function_parameter_list() to build function type and used in defining declaration (allow default arguments) \n");
               printf ("   --- param_list = %p \n",param_list);
#endif
#endif

#if 0
               printf ("In convert_routine(): Building the definingMemberFunctionDeclaration: definingMemberFunctionDeclaration = %p \n",definingMemberFunctionDeclaration);
               printf ("   nameWithOutTemplateArguments = %s  \n",nameWithOutTemplateArguments.str());
               printf ("   nameWithTemplateArguments    = %s  \n",nameWithTemplateArguments.str());
               printf ("   templateArgumentsList.size() = %zu \n",templateArgumentsList.size());
               for (size_t i = 0; i < templateArgumentsList.size(); i++)
                  {
                    printf ("   --- templateArgumentsList[%zu] = %p = %s \n",i,templateArgumentsList[i],templateArgumentsList[i]->class_name().c_str());
                  }
#endif
#if 0
               printf ("part 2: buildTemplateInstantiation = %s first_nondefining_declaration = %p \n",buildTemplateInstantiation ? "true" : "false",first_nondefining_declaration);
#endif
            // DQ (5/19/2016): Adding test on template arguments and those in the first_nondefining_declaration.
               if (buildTemplateInstantiation == true && first_nondefining_declaration != NULL)
                  {
                 // ROSE_ASSERT(first_nondefining_declaration != NULL);
                    SgTemplateInstantiationMemberFunctionDecl*first_nondefiningTemplateInstantiationMemberFunctionDeclaration = 
                         isSgTemplateInstantiationMemberFunctionDecl(first_nondefiningMemberFunctionDeclaration);
#if 0
                    printf ("Testing of template arguments: part 2: first_nondefining_declaration = %p \n",first_nondefining_declaration);
                    printf ("Testing of template arguments: part 2: first_nondefiningTemplateInstantiationMemberFunctionDeclaration = %p if non-null \n",first_nondefiningTemplateInstantiationMemberFunctionDeclaration);
#endif
                    if (first_nondefiningTemplateInstantiationMemberFunctionDeclaration != NULL)
                       {
                      // ROSE_ASSERT(first_nondefiningTemplateInstantiationMemberFunctionDeclaration != NULL);
                         ROSE_ASSERT(first_nondefiningTemplateInstantiationMemberFunctionDeclaration->get_firstNondefiningDeclaration() != NULL);
                         ROSE_ASSERT(first_nondefiningTemplateInstantiationMemberFunctionDeclaration->get_firstNondefiningDeclaration() == first_nondefining_declaration);

                         SgTemplateArgumentPtrList & templateArgumentsList_from_first_nondefining_declaration = first_nondefiningTemplateInstantiationMemberFunctionDeclaration->get_templateArguments();

#if 0
                         printf ("Testing of template argument list \n");
#endif

                         bool templateArgListsAreEquivalent = SageInterface::templateArgumentListEquivalence(templateArgumentsList, templateArgumentsList_from_first_nondefining_declaration);
                         if (templateArgListsAreEquivalent == false)
                            {
                              first_nondefining_declaration->get_file_info()->display("first_nondefining_declaration: debug");
                              first_nondefiningTemplateInstantiationMemberFunctionDeclaration->get_file_info()->display("first_nondefiningTemplateInstantiationMemberFunctionDeclaration: debug");
                              printf ("In convert_routine(): calling summarize_routine() after failing templateArgumentListEquivalence() test \n");
                              summarize_routine(p);
                            }
                         ROSE_ASSERT(templateArgListsAreEquivalent == true);
                      // ROSE_ASSERT(SageInterface::templateArgumentListEquivalence(templateArgumentsList, templateArgumentsList_from_first_nondefining_declaration));
                       }
                  }
#if 0
               printf ("Now build the defining declaration expected for the call to this function \n");
#endif
            // Now build the defining declaration expected for the call to this function.
            // definingMemberFunctionDeclaration = buildDefiningMemberFunctionDeclaration(p->source_corresp.name, return_type, param_list, classDefinition, decoratorList, buildTemplateInstantiation);
            // definingMemberFunctionDeclaration = buildDefiningMemberFunctionDeclaration(name, return_type, param_list, classDefinition, decoratorList, buildTemplateInstantiation, functionConstVolatileFlags);
            // definingMemberFunctionDeclaration = buildDefiningMemberFunctionDeclaration(name, return_type, param_list, classDefinition, decoratorList, buildTemplateInstantiation, functionConstVolatileFlags,nondefiningMemberFunctionDeclaration);
            // definingMemberFunctionDeclaration = buildDefiningMemberFunctionDeclaration(nameWithTemplateArguments, return_type, param_list, classDefinition, decoratorList, buildTemplateInstantiation, functionConstVolatileFlags,nondefiningMemberFunctionDeclaration);
            // definingMemberFunctionDeclaration = buildDefiningMemberFunctionDeclaration(nameWithTemplateArguments, return_type, param_list, classDefinition, decoratorList, buildTemplateInstantiation, functionConstVolatileFlags,nondefiningMemberFunctionDeclaration,&templateArgumentsList);

            // DQ (12/27/20134): Fixed to use the first non-defining declaration.
            // definingMemberFunctionDeclaration = buildDefiningMemberFunctionDeclaration(nameWithOutTemplateArguments, return_type, param_list, classDefinition, decoratorList, buildTemplateInstantiation, functionConstVolatileFlags, nondefiningMemberFunctionDeclaration, &templateArgumentsList);
               definingMemberFunctionDeclaration = buildDefiningMemberFunctionDeclaration(nameWithOutTemplateArguments, return_type, param_list, classDefinition, decoratorList, buildTemplateInstantiation, functionConstVolatileFlags, first_nondefiningMemberFunctionDeclaration, &templateArgumentsList);
#if 0
               classDefinition->get_symbol_table()->print("SgClassDefinition scope for new member function (after calling defining build function)");
#endif
            // This is the SgFunctionDeclaration (used internally in this function).
               definingFunctionDeclaration = definingMemberFunctionDeclaration;

            // DQ (4/3/2012): Allow the generation of multiple defining function (different from
            // ROSE's perspective) but from the same routine pointer in EDG (see test2012_59.C).
            // This is the SgDeclarationStatement that will be returned from this function.
            // return_defining_declaration = definingFunctionDeclaration;
               if (return_defining_declaration == NULL)
                  {
#if 0
                    printf ("In convert_routine(): return_defining_declaration has not be set, setting it to definingFunctionDeclaration = %p = %s \n",definingFunctionDeclaration,definingFunctionDeclaration->class_name().c_str());
#endif
                    return_defining_declaration = definingFunctionDeclaration;
                  }
                 else
                  {
#if 0
                    printf ("In convert_routine(): Note that return_defining_declaration was previously set, so we don't want to reset it \n");
                    printf ("   --- return_defining_declaration = %p = %s \n",return_defining_declaration,return_defining_declaration->class_name().c_str());
#endif
                    ROSE_ASSERT(definingFunctionDeclaration != NULL);
#if 0
                    printf ("   --- definingFunctionDeclaration = %p = %s \n",definingFunctionDeclaration,definingFunctionDeclaration->class_name().c_str());
#endif
                 // DQ (4/3/2012): Reset this as a test.
                    printf ("Reset return_defining_declaration as a test \n");
                    return_defining_declaration = definingFunctionDeclaration;
                  }

            // DQ (2/9/2012): Make this dependent on being a template instantiation.
               if (buildTemplateInstantiation == true)
                  {
                 // DQ (1/2/2012): Added refactored function test set the template declaration inside of the template instantiation.
                    setTemplateFunctionDeclaration (p,definingMemberFunctionDeclaration);

                 // DQ (6/28/2013): Set if template arguments should be explicit for function reference expressions.
                    ROSE_ASSERT(isSgTemplateInstantiationMemberFunctionDecl(definingMemberFunctionDeclaration) != NULL);
                    isSgTemplateInstantiationMemberFunctionDecl(definingMemberFunctionDeclaration)->set_template_argument_list_is_explicit(p->expl_template_arg_list_used);
                  }

            // DQ (2/9/2012): Make this dependent on being a template instantiation.
               if (buildTemplateInstantiation == true)
                  {
                 // If this is a template instantiation then mark all parts (including the parameter list) and compiler generated.
                    set_generated_file_info(definingMemberFunctionDeclaration->get_parameterList());
                  }
                 else
                  {
                 // Set the source position for the parameter list.
                    set_file_info_for_corresp(definingMemberFunctionDeclaration->get_parameterList(), p->source_corresp);
                  }

               ROSE_ASSERT(definingTemplateFunctionDeclaration == NULL);
               ROSE_ASSERT(definingFunctionDeclaration != NULL);

               ROSE_ASSERT(param_list->get_parent() != NULL);

               ROSE_ASSERT(definingFunctionDeclaration->get_definition() != NULL);

               ROSE_ASSERT(definingFunctionDeclaration    != NULL);
               ROSE_ASSERT(nondefiningFunctionDeclaration != NULL);
               ROSE_ASSERT(nondefiningFunctionDeclaration != NULL && definingFunctionDeclaration != NULL);

            // DQ (1/2/2012): Added new test.
               if (buildTemplateInstantiation == true)
                  {
                    ROSE_ASSERT(isSgTemplateInstantiationMemberFunctionDecl(definingFunctionDeclaration)    != NULL);
                    ROSE_ASSERT(isSgTemplateInstantiationMemberFunctionDecl(nondefiningFunctionDeclaration) != NULL);
                    if (isSgTemplateInstantiationMemberFunctionDecl(nondefiningFunctionDeclaration)->get_templateDeclaration() == NULL)
                       {
                      // In this case we don't have a template declaration.
                         ROSE_ASSERT(p->assoc_template == NULL);
                       }
                 // ROSE_ASSERT(isSgTemplateInstantiationMemberFunctionDecl(nondefiningFunctionDeclaration)->get_templateDeclaration() != NULL);
                 // ROSE_ASSERT(isSgTemplateInstantiationMemberFunctionDecl(definingFunctionDeclaration)->get_templateDeclaration()    != NULL);

                 // DQ (1/17/2013): See test2013_08.C for an example that fails this test.
                 // DQ (1/16/2013): Added test...to detect unset templateDeclaration member.
                 // ROSE_ASSERT(isSgTemplateInstantiationMemberFunctionDecl(nondefiningFunctionDeclaration)->get_templateDeclaration() != NULL);
                 // ROSE_ASSERT(isSgTemplateInstantiationMemberFunctionDecl(definingFunctionDeclaration)->get_templateDeclaration()    != NULL);
                  }

            // ROSE_ASSERT(classDefinition->lookup_function_symbol(p->source_corresp.name) != NULL);
            // ROSE_ASSERT(classDefinition->lookup_function_symbol(name) != NULL);
               ROSE_ASSERT(classDefinition->lookup_function_symbol(nameWithTemplateArguments) != NULL);

#if 1
            // DQ (4/9/2012): This this case we should have passed the template into this function (we have to modify the API).
               ROSE_ASSERT(edg_template == NULL);

            // DQ (3/25/2017): Eliminate Clang warning (which caught a bug): warning: if statement has empty body [-Wempty-body]
            // DQ (4/9/2012): Add this to the template cache
            // DQ (4/21/2012): This assertion fails for the case of test2001_01.C and test2012_63.C (case of function with prototype before its definition).
            // ROSE_ASSERT(first_nondefining_declarations_for_functions.find(p) == first_nondefining_declarations_for_functions.end());
            // if (first_nondefining_declarations_for_functions.find(p) == first_nondefining_declarations_for_functions.end());
               if (first_nondefining_declarations_for_functions.find(p) == first_nondefining_declarations_for_functions.end())
                  {
#if 0
                    printf ("In convert_routine(): Adding nondefiningFunctionDeclaration = %p = %s = %s to first_nondefining_declarations_for_functions[p = %p] \n",
                         nondefiningFunctionDeclaration,nondefiningFunctionDeclaration->class_name().c_str(),nondefiningFunctionDeclaration->get_name().str(),p);
                    printf ("   --- Changed to used nondefiningFunctionDeclaration->get_firstNondefiningDeclaration() = %p \n",nondefiningFunctionDeclaration->get_firstNondefiningDeclaration());
#endif
                 // first_nondefining_declarations_for_functions[p] = nondefiningFunctionDeclaration;
                    first_nondefining_declarations_for_functions[p] = isSgFunctionDeclaration(nondefiningFunctionDeclaration->get_firstNondefiningDeclaration());
                  }
               ROSE_ASSERT(first_nondefining_declarations_for_functions[p] != NULL);

               if (defining_declarations_for_functions.find(p) != defining_declarations_for_functions.end())
                  {
#if 0
                    printf ("WARNING: In convert_routine(): p WAS previously process and put into defining_declarations_for_functions: p = %p \n",p);
#endif
#if 0
                    printf ("The definingFunctionDeclaration = %p = %s name = %s may have been built redundently \n",
                            definingFunctionDeclaration,definingFunctionDeclaration->class_name().c_str(),definingFunctionDeclaration->get_name().str());
#endif
#if 0
                    printf ("In convert_routine(): calling summarize_routine() before checking for if this has previously been translated into return_defining_declaration!\n");
                    summarize_routine(p);
#endif
                  }

            // DQ (1/27/2019): We cannot assert this (see Cxx11_tests/test2019_31.C)
            // ROSE_ASSERT(defining_declarations_for_functions.find(p) == defining_declarations_for_functions.end());

            // DQ (4/9/2012): Add this to the template cache
               if (defining_declarations_for_functions.find(p) == defining_declarations_for_functions.end())
                  {
                    defining_declarations_for_functions[p] = definingFunctionDeclaration;
                  }
               ROSE_ASSERT(defining_declarations_for_functions[p] != NULL);

               ROSE_ASSERT( (first_nondefining_declarations_for_functions.find(p) != first_nondefining_declarations_for_functions.end()) || 
                            (defining_declarations_for_functions.find(p) != defining_declarations_for_functions.end()) );
#endif
             }
            else
             {
            // This is the case where buildTemplateDeclaration != false
               ROSE_ASSERT(buildTemplateDeclaration == true);
#if 0
               printf ("In convert_routine(): buildTemplateDeclaration == true: first_nondefining_declaration = %p \n",first_nondefining_declaration);
#endif
            // definingTemplateMemberFunctionDeclaration = new SgTemplateMemberFunctionDeclaration(p->source_corresp.name, function_type, NULL);
               if (first_nondefining_declaration == NULL)
                  {
#if 0
                    printf ("In convert_routine(): Calling convert_function_parameter_list() for non-defining template member function declaration (skip the default arguments = true) \n");
#endif
                 // DQ (4/21/2018): Add support to build a SgDeclarationScope if it is detected in the parameter type constrcution 
                 // that we need one for types that are defined in the function parameter list.  See test2018_41.c for an example.
                    SgDeclarationScope* declarationScope = NULL;

                 // DQ (12/16/2020): Fixing test2020_80.C (support for default arguments in the AST).
                 // DQ (12/4/2011): If we build a separate non-defining function, it need it's own paramter list.
                 // bool skipDefaultArguments = true;
                    bool skipDefaultArguments = false;

                 // SgFunctionParameterList* nondefining_function_param_list = convert_function_parameter_list(p,skipDefaultArguments);
                    SgFunctionParameterList* nondefining_function_param_list = convert_function_parameter_list(p,skipDefaultArguments,NULL,declarationScope);
                    ROSE_ASSERT(nondefining_function_param_list != NULL);
#if 0
                    printf ("In convert_routine(): DONE: Calling convert_function_parameter_list() for non-defining template member function (skip the default arguments = true) \n");
                    printf ("   --- nondefining_function_param_list = %p \n",nondefining_function_param_list);
#endif

                 // printf ("Need to add templateParameterList to function arguments for buildNondefiningTemplateMemberFunctionDeclaration() \n");

                 // DQ (8/12/2013): We need to support passing the template parameter list to construct a template member function.
                 // nondefiningTemplateMemberFunctionDeclaration = buildNondefiningTemplateMemberFunctionDeclaration(nameWithOutTemplateArguments, return_type, nondefining_function_param_list, classDefinition, decoratorList, functionConstVolatileFlags);
                    nondefiningTemplateMemberFunctionDeclaration = buildNondefiningTemplateMemberFunctionDeclaration(nameWithOutTemplateArguments, return_type, nondefining_function_param_list, classDefinition, decoratorList, functionConstVolatileFlags, &templateParameterList);

#if 0
                    printf ("In convert_routine(): nondefiningTemplateMemberFunctionDeclaration = %p \n", nondefiningTemplateMemberFunctionDeclaration);
                    printf ("In convert_routine(): nondefiningTemplateMemberFunctionDeclaration->get_firstNondefiningDeclaration() = %p \n", nondefiningTemplateMemberFunctionDeclaration->get_firstNondefiningDeclaration());
#endif

                 // DQ (8/22/2013): This assertion appears to fail for only one AST merge test code. Not clear why, but for 
                 // multiple files on the command line this might make sense (only want one first nondefining declaration).
                 // ROSE_ASSERT(nondefiningTemplateMemberFunctionDeclaration->get_firstNondefiningDeclaration() == nondefiningTemplateMemberFunctionDeclaration);
                    if (nondefiningTemplateMemberFunctionDeclaration->get_firstNondefiningDeclaration() != nondefiningTemplateMemberFunctionDeclaration)
                       {
                      // DQ (10/11/2015): This also fails for test2015_87.C.
                      // I think that this fails for the mergeTest_56.C example.
                      // printf ("WARNING: is appears to only fail for the mergeTest_56.C example in mergeAST_tests \n");
                       }
                 // ROSE_ASSERT(nondefiningTemplateMemberFunctionDeclaration->get_firstNondefiningDeclaration() == nondefiningTemplateMemberFunctionDeclaration);

                 // definingTemplateFunctionDeclaration          = nondefiningTemplateMemberFunctionDeclaration;
                    ROSE_ASSERT(nondefiningTemplateMemberFunctionDeclaration != NULL);

                    for (SgTemplateParameterPtrList::iterator it = templateParameterList.begin(); it != templateParameterList.end(); it++) {
                      if ((*it)->get_parent() != NULL) {
                        SgDeclarationStatement * declstmt_parent = isSgDeclarationStatement((*it)->get_parent());
                        ROSE_ASSERT(declstmt_parent != NULL);
                        ROSE_ASSERT(declstmt_parent->get_firstNondefiningDeclaration() == nondefiningTemplateMemberFunctionDeclaration->get_firstNondefiningDeclaration());
                      } else {
                        (*it)->set_parent(nondefiningTemplateMemberFunctionDeclaration);
                      }
                    }

                 // DQ (12/22/2011): This cannot be supported in the new design.
                 // nondefiningTemplateFunctionDeclaration       = nondefiningTemplateMemberFunctionDeclaration;

                 // Set the source position for the parameter list.
                    set_file_info_for_corresp(nondefiningTemplateMemberFunctionDeclaration->get_parameterList(), p->source_corresp);

                 // ROSE_ASSERT(nondefiningTemplateFunctionDeclaration != NULL);
                 // ROSE_ASSERT(nondefiningTemplateFunctionDeclaration->get_definition() == NULL);
#if 0
                    printf ("Looking up nameWithOutTemplateArguments = %s in scope = %p = %s \n",nameWithOutTemplateArguments.str(),classDefinition,classDefinition->class_name().c_str());
#endif
                    ROSE_ASSERT(nondefiningTemplateMemberFunctionDeclaration->get_type() != NULL);
                 // ROSE_ASSERT(classDefinition->lookup_template_member_function_symbol(p->source_corresp.name,nondefiningTemplateMemberFunctionDeclaration->get_type()) != NULL);
                 // ROSE_ASSERT(classDefinition->lookup_template_member_function_symbol(name,nondefiningTemplateMemberFunctionDeclaration->get_type()) != NULL);
                 // ROSE_ASSERT(classDefinition->lookup_template_member_function_symbol(nameWithOutTemplateArguments,nondefiningTemplateMemberFunctionDeclaration->get_type()) != NULL);
                    ROSE_ASSERT(classDefinition->lookup_template_member_function_symbol(nameWithOutTemplateArguments,nondefiningTemplateMemberFunctionDeclaration->get_type(),&templateParameterList) != NULL);
                    ROSE_ASSERT(nondefining_function_param_list->get_parent() != NULL);

                 // DQ (12/12/2011): Added call to get type to force generation of SgFunctionType using template member function.
                    SgFunctionType* templateFunctionType = nondefiningTemplateMemberFunctionDeclaration->get_type();
                    ROSE_ASSERT(templateFunctionType != NULL);

                    ROSE_ASSERT(verifyExistenceOfSymbol(nondefiningTemplateMemberFunctionDeclaration) == true);



#if  MAKE_REDUNDANT_CALLS_TO_SYMBOL_TABLE_LOOKUP
                 // DQ (5/10/2012): Debugging code.
                 // SgSymbol* associatedSymbol = classDefinition->lookup_template_member_function_symbol(name,nondefiningTemplateMemberFunctionDeclaration->get_type());
                 // SgSymbol* associatedSymbol = classDefinition->lookup_template_member_function_symbol(nameWithOutTemplateArguments,nondefiningTemplateMemberFunctionDeclaration->get_type());
                    SgSymbol* associatedSymbol = classDefinition->lookup_template_member_function_symbol(nameWithOutTemplateArguments,nondefiningTemplateMemberFunctionDeclaration->get_type(),NULL,NULL);
                 // if (classDefinition->lookup_template_member_function_symbol(name,nondefiningTemplateMemberFunctionDeclaration->get_type()) == NULL)
                    if (associatedSymbol == NULL)
                       {
                         printf ("ERROR: associatedSymbol == NULL \n");
                         printf ("   --- classDefinition = %p = %s \n",classDefinition,classDefinition->class_name().c_str());
                      // printf ("   --- name = %s \n",name.str());
                         printf ("   --- nameWithOutTemplateArguments = %s \n",nameWithOutTemplateArguments.str());
                         printf ("   --- nondefiningTemplateMemberFunctionDeclaration->get_type() = %p = %s \n",nondefiningTemplateMemberFunctionDeclaration->get_type(),nondefiningTemplateMemberFunctionDeclaration->get_type()->class_name().c_str());
                         printf ("   --- nondefiningTemplateMemberFunctionDeclaration->get_type()->get_mangled() = %s \n",nondefiningTemplateMemberFunctionDeclaration->get_type()->get_mangled().str());
                       }
                 // ROSE_ASSERT(classDefinition->lookup_template_member_function_symbol(name,nondefiningTemplateMemberFunctionDeclaration->get_type()) != NULL);
#endif
                 // DQ (5/10/2012): Commented out this test for tessting purposes.
                 // ROSE_ASSERT(associatedSymbol != NULL);
                 // ROSE_ASSERT(classDefinition->lookup_template_member_function_symbol(nameWithOutTemplateArguments,nondefiningTemplateMemberFunctionDeclaration->get_type()) != NULL);
                    ROSE_ASSERT(classDefinition->lookup_template_member_function_symbol(nameWithOutTemplateArguments,nondefiningTemplateMemberFunctionDeclaration->get_type(),&templateParameterList) != NULL);
                  }
                 else
                  {
                    ROSE_ASSERT(first_nondefining_declaration != NULL);
#if 0
                    printf ("first_nondefining_declaration = %p = %s \n",first_nondefining_declaration,first_nondefining_declaration->class_name().c_str());
#endif
                 // definingTemplateFunctionDeclaration = isSgTemplateMemberFunctionDeclaration(first_nondefining_declaration);
                 // ROSE_ASSERT(definingTemplateFunctionDeclaration != NULL);
                 // nondefiningTemplateFunctionDeclaration = isSgTemplateMemberFunctionDeclaration(first_nondefining_declaration);
                 // ROSE_ASSERT(nondefiningTemplateFunctionDeclaration != NULL);
                    nondefiningTemplateMemberFunctionDeclaration = isSgTemplateMemberFunctionDeclaration(first_nondefining_declaration);
                    ROSE_ASSERT(nondefiningTemplateMemberFunctionDeclaration != NULL);
                    ROSE_ASSERT(nondefiningTemplateMemberFunctionDeclaration->get_firstNondefiningDeclaration() == nondefiningTemplateMemberFunctionDeclaration);

                 // ROSE_ASSERT(classDefinition->lookup_template_symbol(p->source_corresp.name) != NULL);
                    ROSE_ASSERT(nondefiningTemplateMemberFunctionDeclaration->get_type() != NULL);
                 // ROSE_ASSERT(classDefinition->lookup_template_member_function_symbol(p->source_corresp.name,nondefiningTemplateMemberFunctionDeclaration->get_type()) != NULL);

                 // DQ (4/25/2012): Make sure that the nondefiningTemplateMemberFunctionDeclaration is the correct type (SgTemplateMemberFunctionDeclaration).
                    ROSE_ASSERT(isSgTemplateMemberFunctionDeclaration(nondefiningTemplateMemberFunctionDeclaration) != NULL);
#if 0
                 // DQ (4/25/2012): Make sure that the nondefiningTemplateMemberFunctionDeclaration has the correct type of symbol (SgTemplateMemberFunctionSymbol).
                    if (isSgTemplateMemberFunctionType(nondefiningTemplateMemberFunctionDeclaration->get_type()) == NULL)
                       {
                         printf ("nondefiningTemplateMemberFunctionDeclaration = %p = %s \n",nondefiningTemplateMemberFunctionDeclaration,nondefiningTemplateMemberFunctionDeclaration->class_name().c_str());
                         printf ("nondefiningTemplateMemberFunctionDeclaration->get_type() = %p = %s \n",nondefiningTemplateMemberFunctionDeclaration->get_type(),nondefiningTemplateMemberFunctionDeclaration->get_type()->class_name().c_str());
                       }
                    ROSE_ASSERT(isSgTemplateMemberFunctionType(nondefiningTemplateMemberFunctionDeclaration->get_type()) != NULL);
#endif
#if  MAKE_REDUNDANT_CALLS_TO_SYMBOL_TABLE_LOOKUP
                 // DQ (4/25/2012): Debugging code.
                 // SgSymbol* associatedSymbol = classDefinition->lookup_template_member_function_symbol(name,nondefiningTemplateMemberFunctionDeclaration->get_type());
                 // SgSymbol* associatedSymbol = classDefinition->lookup_template_member_function_symbol(nameWithOutTemplateArguments,nondefiningTemplateMemberFunctionDeclaration->get_type());
                    SgSymbol* associatedSymbol = classDefinition->lookup_template_member_function_symbol(nameWithOutTemplateArguments,nondefiningTemplateMemberFunctionDeclaration->get_type(),NULL,NULL);
                 // if (classDefinition->lookup_template_member_function_symbol(name,nondefiningTemplateMemberFunctionDeclaration->get_type()) == NULL)
                    if (associatedSymbol == NULL)
                       {
                         printf ("ERROR: associatedSymbol == NULL \n");
                         printf ("   --- classDefinition = %p = %s \n",classDefinition,classDefinition->class_name().c_str());
                      // printf ("   --- name = %s \n",name.str());
                         printf ("   --- nameWithOutTemplateArguments = %s \n",nameWithOutTemplateArguments.str());
                         printf ("   --- nondefiningTemplateMemberFunctionDeclaration->get_type() = %p = %s \n",nondefiningTemplateMemberFunctionDeclaration->get_type(),nondefiningTemplateMemberFunctionDeclaration->get_type()->class_name().c_str());
                         printf ("   --- nondefiningTemplateMemberFunctionDeclaration->get_type()->get_mangled() = %s \n",nondefiningTemplateMemberFunctionDeclaration->get_type()->get_mangled().str());
                       }
                 // ROSE_ASSERT(classDefinition->lookup_template_member_function_symbol(name,nondefiningTemplateMemberFunctionDeclaration->get_type()) != NULL);
#endif
                 // DQ (5/10/2012): Commented out this test for testing purposes.
                 // ROSE_ASSERT(associatedSymbol != NULL);
                 // ROSE_ASSERT(classDefinition->lookup_template_member_function_symbol(nameWithOutTemplateArguments,nondefiningTemplateMemberFunctionDeclaration->get_type()) != NULL);
                    if (classDefinition->lookup_template_member_function_symbol(nameWithOutTemplateArguments,nondefiningTemplateMemberFunctionDeclaration->get_type(),&templateParameterList) == NULL)
                       {
                         printf ("nameWithOutTemplateArguments                                                   = %s \n",nameWithOutTemplateArguments.str());
                         printf ("classDefinition                                                                = %p = %s \n",classDefinition,classDefinition->class_name().c_str());
                         printf ("templateParameterList.size()                                                   = %" PRIuPTR " \n",templateParameterList.size());
                         printf ("nondefiningTemplateMemberFunctionDeclaration->get_templateParameters()->size() = %" PRIuPTR " \n",nondefiningTemplateMemberFunctionDeclaration->get_templateParameters().size()); 
                         ROSE_ASSERT(templateParameterList.size() == nondefiningTemplateMemberFunctionDeclaration->get_templateParameters().size());
                         for (size_t i = 0; i < templateParameterList.size(); i++)
                            {
                              printf ("templateParameterList[i = %" PRIuPTR "] = %p nondefiningTemplateMemberFunctionDeclaration->get_templateParameters()[i = %" PRIuPTR "] = %p \n",
                                   i,templateParameterList[i],i,nondefiningTemplateMemberFunctionDeclaration->get_templateParameters()[i]);
                            }
                       }
                    ROSE_ASSERT(classDefinition->lookup_template_member_function_symbol(nameWithOutTemplateArguments,nondefiningTemplateMemberFunctionDeclaration->get_type(),&templateParameterList) != NULL);
#if 0
                    if (associatedSymbol == NULL)
                       {
#ifdef DEBUG_CONVERSION
                         printf ("######## Commented out test for associatedSymbol for nondefiningTemplateMemberFunctionDeclaration = %p \n",nondefiningTemplateMemberFunctionDeclaration);
#endif
                       }
#endif
#ifdef DEBUG_CONVERSION
                 // DQ (5/11/2012): Test alternative function to verify it is equivalent.
                 // printf ("In convert_routine(): name = %s func_type = %p = %s \n",name.str(),nondefiningTemplateMemberFunctionDeclaration->get_type(),nondefiningTemplateMemberFunctionDeclaration->get_type()->get_mangled().str());
                    printf ("In convert_routine(): nameWithOutTemplateArguments = %s func_type = %p = %s \n",nameWithOutTemplateArguments.str(),
                         nondefiningTemplateMemberFunctionDeclaration->get_type(),nondefiningTemplateMemberFunctionDeclaration->get_type()->get_mangled().str());
#endif
#if  MAKE_REDUNDANT_CALLS_TO_SYMBOL_TABLE_LOOKUP
                 // DQ (8/7/2013): Added support for template function overloading using template parameters.
                    SgSymbol* func_symbol_1 = classDefinition->find_symbol_by_type_of_function<SgTemplateMemberFunctionDeclaration>(nameWithOutTemplateArguments,nondefiningTemplateMemberFunctionDeclaration->get_type(),&templateParameterList,NULL);
#endif
                 // DQ (8/7/2013): Added support for template function overloading using template parameters.
                    ROSE_ASSERT(classDefinition->find_symbol_by_type_of_function<SgTemplateMemberFunctionDeclaration>(nameWithOutTemplateArguments,nondefiningTemplateMemberFunctionDeclaration->get_type(),&templateParameterList,NULL) != NULL);
                  }

            // DQ (12/11/2011): Added new test.
            // ROSE_ASSERT(verifyExistenceOfSymbol(nondefiningTemplateFunctionDeclaration) == true);
               ROSE_ASSERT(verifyExistenceOfSymbol(nondefiningTemplateMemberFunctionDeclaration) == true);

               nondefiningFunctionDeclaration = nondefiningTemplateMemberFunctionDeclaration;

            // first_nondefining_declaration = nondefiningTemplateFunctionDeclaration;
               first_nondefining_declaration = nondefiningTemplateMemberFunctionDeclaration;
               ROSE_ASSERT(first_nondefining_declaration != NULL);

#if MAKE_REDUNDANT_CALLS_TO_SYMBOL_TABLE_LOOKUP
               SgSymbol* func_symbol_1 = classDefinition->find_symbol_by_type_of_function<SgTemplateMemberFunctionDeclaration>(nameWithOutTemplateArguments,nondefiningTemplateMemberFunctionDeclaration->get_type(),&templateParameterList,NULL);
               ROSE_ASSERT(func_symbol_1 != NULL);

               SgSymbol* associatedSymbol = classDefinition->lookup_template_member_function_symbol(nameWithOutTemplateArguments,nondefiningTemplateMemberFunctionDeclaration->get_type(),NULL,NULL);
               ROSE_ASSERT(associatedSymbol != NULL);
#else
            // DQ (8/7/2013): Added support for template function overloading using template parameters.
            // DQ (2/17/2013): This implementation permits -DNDEBUG to eliminate the checking and thus the redundant symbol table lookup.
               ROSE_ASSERT(classDefinition->find_symbol_by_type_of_function<SgTemplateMemberFunctionDeclaration>(nameWithOutTemplateArguments,nondefiningTemplateMemberFunctionDeclaration->get_type(),&templateParameterList,NULL) != NULL);
               ROSE_ASSERT(classDefinition->lookup_template_member_function_symbol(nameWithOutTemplateArguments,nondefiningTemplateMemberFunctionDeclaration->get_type(),&templateParameterList) != NULL);
#endif
            // DQ (8/22/2013): This assertion appears to fail for only one AST merge test code. Not clear why, but for 
            // multiple files on the command line this might make sense (only want one first nondefining declaration).
            // Now build the defining declaration expected for the call to this function.
            // assert(nondefiningTemplateMemberFunctionDeclaration->get_firstNondefiningDeclaration() == nondefiningTemplateMemberFunctionDeclaration);

               if (nondefiningTemplateMemberFunctionDeclaration->get_firstNondefiningDeclaration() != nondefiningTemplateMemberFunctionDeclaration)
                  {
                 // I think that this fails for the mergeTest_56.C example.
                    mprintf ("WARNING: it appears to only fail for the mergeTest_56.C example in mergeAST_tests: calling buildDefiningTemplateMemberFunctionDeclaration() \n");
                  }
#ifdef DEBUG_CONVERSION
               printf ("In convert_routine(): We need to skip the default function parameters in building the defining template function declaration \n");
#endif
            // DQ (12/16/2020): Fixing test2020_79.C (default arguments for template declarations in the AST).
            // DQ (9/9/2014): We need to skip the default arguments in building the defining declaration.
               bool skipDefaultArguments = false;
#if 0
               printf ("In convert_routine(): Calling convert_function_parameter_list() (skip the default arguments = true) \n");
#endif
            // DQ (4/21/2018): Add support to build a SgDeclarationScope if it is detected in the parameter type constrcution 
            // that we need one for types that are defined in the function parameter list.  See test2018_41.c for an example.
               SgDeclarationScope* declarationScope = NULL;

               SgFunctionParameterList* param_list = convert_function_parameter_list(p,skipDefaultArguments,NULL,declarationScope);
               ROSE_ASSERT(param_list != NULL);
#if 0
               printf ("In convert_routine(): DONE: Calling convert_function_parameter_list() (skip the default arguments = true) \n");
               printf ("   --- param_list = %p \n",param_list);
#endif

#if 0
               printf ("((((((((((((((((((((((( In convert_routine(): calling buildDefiningTemplateMemberFunctionDeclaration() \n");
               printf ("   --- nondefiningTemplateMemberFunctionDeclaration                                    = %p \n",nondefiningTemplateMemberFunctionDeclaration);
               printf ("   --- nondefiningTemplateMemberFunctionDeclaration->get_firstNondefiningDeclaration() = %p \n",nondefiningTemplateMemberFunctionDeclaration->get_firstNondefiningDeclaration());
               printf ("   --- nondefiningTemplateMemberFunctionDeclaration->get_declarationScope()            = %p \n",nondefiningTemplateMemberFunctionDeclaration->get_declarationScope());
#endif
               ROSE_ASSERT(nondefiningTemplateMemberFunctionDeclaration->get_firstNondefiningDeclaration() != NULL);
               SgTemplateMemberFunctionDeclaration* firstTemplateMemberFunctionDeclaration = isSgTemplateMemberFunctionDeclaration(nondefiningTemplateMemberFunctionDeclaration->get_firstNondefiningDeclaration());
               definingTemplateMemberFunctionDeclaration = buildDefiningTemplateMemberFunctionDeclaration(nameWithOutTemplateArguments, return_type, param_list, classDefinition, decoratorList, functionConstVolatileFlags,firstTemplateMemberFunctionDeclaration);

#if 0
               printf ("))))))))))))))))))))))) In convert_routine(): DONE: calling buildDefiningTemplateMemberFunctionDeclaration() \n");
#endif
#if 0
            // Ouput the symbol table of the scope in the defining function declaration
               SgScopeStatement* definingFunctionScope = definingTemplateMemberFunctionDeclaration->get_definition();
               ROSE_ASSERT(definingFunctionScope != NULL);
               definingFunctionScope->get_symbol_table()->print();
#endif
            // This is the SgTemplateFunctionDeclaration (used internally in this function).
            // definingTemplateFunctionDeclaration = definingTemplateMemberFunctionDeclaration;

            // This is the SgDeclarationStatement that will be returned from this function.
               return_defining_declaration = definingTemplateMemberFunctionDeclaration;

            // Set the source position for the parameter list.
               set_file_info_for_corresp(definingTemplateMemberFunctionDeclaration->get_parameterList(), p->source_corresp);

               ROSE_ASSERT(definingFunctionDeclaration == NULL);
               ROSE_ASSERT(definingTemplateMemberFunctionDeclaration != NULL);

            // DQ (12/28/2011): We can now support this since the template versions are derived from the non template versions.
               definingMemberFunctionDeclaration = definingTemplateMemberFunctionDeclaration;

               definingFunctionDeclaration = definingMemberFunctionDeclaration;

               ROSE_ASSERT(definingTemplateMemberFunctionDeclaration->get_definition() != NULL);
               ROSE_ASSERT(nondefiningTemplateMemberFunctionDeclaration != NULL);
               ROSE_ASSERT(nondefiningTemplateMemberFunctionDeclaration != NULL && definingTemplateMemberFunctionDeclaration != NULL);

               ROSE_ASSERT(nondefiningTemplateMemberFunctionDeclaration->get_type() != NULL);
               ROSE_ASSERT(definingTemplateMemberFunctionDeclaration->get_type() != NULL);

#if MAKE_REDUNDANT_CALLS_TO_SYMBOL_TABLE_LOOKUP
            // DQ (2/17/2013): This is a repeated test from above.
            // DQ (5/10/2012): Commented out this test for tessting purposes.
               ROSE_ASSERT(classDefinition->lookup_template_member_function_symbol(nameWithOutTemplateArguments,nondefiningTemplateMemberFunctionDeclaration->get_type(),NULL,NULL) != NULL);
               if (classDefinition->lookup_template_member_function_symbol(nameWithOutTemplateArguments,nondefiningTemplateMemberFunctionDeclaration->get_type(),NULL,NULL) == NULL)
                  {
                    printf ("######## Commented out test for symbol assertion for nondefiningTemplateMemberFunctionDeclaration = %p \n",nondefiningTemplateMemberFunctionDeclaration);
                  }
#endif
               ROSE_ASSERT(param_list->get_parent() != NULL);

            // TV (4/2/14): need to pass first nondef decl to putFunctionInTemplateMap()
               SgFunctionDeclaration * first_nondef_function_decl = isSgFunctionDeclaration(nondefiningFunctionDeclaration->get_firstNondefiningDeclaration());
               ROSE_ASSERT(first_nondef_function_decl != NULL);
               putFunctionInTemplateMap(p, edg_template, first_nondef_function_decl, definingFunctionDeclaration);
             }

       // Get the constructor preinitialization list (only relevant for constructors, but present as an empty 
       // list on all member functions).  We can set the ctor pointer here, but we need to call the function 
       // to build the body (which builds the function parameter symbols) before we can build the constructor 
       // preinitialization list.
       // DQ (12/28/2011): This can now be made more uniform under the new design.
          if (isSgClassDefinition(classDefinition)) {
            definingMemberFunctionDeclaration->set_associatedClassDeclaration(isSgClassDefinition(classDefinition)->get_declaration());
          } else if (isSgDeclarationScope(classDefinition)) {
            SgDeclarationStatement * declstmt = isSgDeclarationStatement(isSgDeclarationScope(classDefinition)->get_parent());
            definingMemberFunctionDeclaration->set_associatedClassDeclaration(declstmt);
          } else {
            ROSE_ASSERT(false);
          }
          ROSE_ASSERT(definingMemberFunctionDeclaration->get_associatedClassDeclaration() != NULL);
          ctor = definingMemberFunctionDeclaration->get_CtorInitializerList();
#if 0
          printf ("definingMemberFunctionDeclaration->get_CtorInitializerList() = %p \n",ctor);
#endif
          ROSE_ASSERT(ctor != NULL);
          ROSE_ASSERT(ctor->get_startOfConstruct() != NULL);
          ROSE_ASSERT(ctor->get_endOfConstruct() != NULL);

       // Error checking
          if (buildTemplateDeclaration == false)
             {
               ROSE_ASSERT(definingFunctionDeclaration    != NULL);
               ROSE_ASSERT(nondefiningFunctionDeclaration != NULL);
               ROSE_ASSERT(nondefiningFunctionDeclaration != NULL && definingFunctionDeclaration != NULL);
             }
            else
             {
               ROSE_ASSERT(definingTemplateFunctionDeclaration != NULL || definingTemplateMemberFunctionDeclaration != NULL);

               ROSE_ASSERT(nondefiningTemplateFunctionDeclaration != NULL || nondefiningTemplateMemberFunctionDeclaration != NULL);

               ROSE_ASSERT( (nondefiningTemplateFunctionDeclaration       != NULL && definingTemplateFunctionDeclaration       != NULL) ||
                            (nondefiningTemplateMemberFunctionDeclaration != NULL && definingTemplateMemberFunctionDeclaration != NULL));
             }

          ROSE_ASSERT(nondefiningFunctionDeclaration != NULL);
          ROSE_ASSERT(definingFunctionDeclaration    != NULL);
        }
       else
        {
       // Case of non-member function.
          ROSE_ASSERT(p->source_corresp.is_class_member == false);
#if 0
          printf ("branch for NON-MEMBER functions buildTemplateDeclaration = %s \n",buildTemplateDeclaration ? "true" : "false");
#endif
          ROSE_ASSERT (function_type != NULL);

       // Call the build function instead

       // DQ (9/6/2014): Added test to match non-defining (if available) with the defining declaration that we are trying to build.
          if (first_nondefining_declaration != NULL)
             {
               if (buildTemplateDeclaration == true)
                  {
                    ROSE_ASSERT(isSgTemplateFunctionDeclaration(first_nondefining_declaration) != NULL);
                  }
                 else
                  {
                    ROSE_ASSERT(isSgTemplateFunctionDeclaration(first_nondefining_declaration) == NULL);
                  }
             }

       // DQ (12/1/2011): This is a rewrite to use the Sage Build Interface.

       // DQ (9/6/2014): This is now computed once above (in an outer scope) and used in all inner scopes.
       // The scope of a declaration (in C++) can be different from the current scope (where it is placed structurally).
          ROSE_ASSERT(scopeOfDeclaration != NULL);

          SgScopeStatement* scope = scopeOfDeclaration;
          if (buildTemplateDeclaration == false)
             {
               ROSE_ASSERT(scope != NULL);
#if 0
               printf ("In convert_routine(): (buildTemplateDeclaration == false): p = %p first_nondefining_declaration = %p \n",p,first_nondefining_declaration);
#endif
#if 0
               printf ("In convert_routine(): (buildTemplateDeclaration == false): buildTemplateInstantiation    = %s \n",buildTemplateInstantiation ? "true" : "false");
               printf ("In convert_routine(): (buildTemplateDeclaration == false): nameWithTemplateArguments     = %s \n",nameWithTemplateArguments.str());
               printf ("In convert_routine(): (buildTemplateDeclaration == false): nameWithOutTemplateArguments  = %s \n",nameWithOutTemplateArguments.str());
               printf ("In convert_routine(): (buildTemplateDeclaration == false): scope                         = %p  = %s \n",scope,scope->class_name().c_str());
#endif

            // This was set at the top of the function if it was known from the EDG/ROSE caches.
               if (first_nondefining_declaration == NULL)
                  {
#if 0
                    printf ("In convert_routine(): Calling convert_function_parameter_list() for non-defining non-template non-member function declaration (skip the default arguments = true) scope = %p = %s \n",scope,scope->class_name().c_str());
#endif
                 // DQ (4/21/2018): Add support to build a SgDeclarationScope if it is detected in the parameter type constrcution 
                 // that we need one for types that are defined in the function parameter list.  See test2018_41.c for an example.
                    SgDeclarationScope* declarationScope = NULL;

                 // DQ (12/4/2011): If we build a separate non-defining function, it need it's own paramter list (without default arguments).
                    bool skipDefaultArguments = true;
                    SgFunctionParameterList* nondefining_function_param_list = convert_function_parameter_list(p,skipDefaultArguments,NULL,declarationScope);
                    ROSE_ASSERT(nondefining_function_param_list != NULL);

                    if (nondefiningFunctionDeclaration == NULL)
                       {
                         nondefiningFunctionDeclaration = buildNondefiningFunctionDeclaration(nameWithOutTemplateArguments, return_type, nondefining_function_param_list, scope, decoratorList, buildTemplateInstantiation, &templateArgumentsList, storage_modifier);
                       }
                      else
                       {
                      // DQ (9/20/2013): The parent is tested below, so we need to set it even though it was not used directly 
                      // (it was used to build the function type so we likely can't delete it easily).
                         nondefining_function_param_list->set_parent(nondefiningFunctionDeclaration);

                         printf ("WARNING: RARE CASE of nondefiningFunctionDeclaration found in symbol table but not found in the cache under the original key (will be added to cache below) \n");
                       }

                 // DQ (8/20/2013): Enforce this after the special case of finding a declaration in the symbol table that was not in the cache.
                    ROSE_ASSERT(nondefiningFunctionDeclaration != NULL);
                    ROSE_ASSERT(nondefiningFunctionDeclaration->get_firstNondefiningDeclaration() != NULL);

                    first_nondefining_declaration = isSgFunctionDeclaration(nondefiningFunctionDeclaration->get_firstNondefiningDeclaration());

                 // Set the source position for the parameter list.
                    set_file_info_for_corresp(nondefiningFunctionDeclaration->get_parameterList(), p->source_corresp);

                    ROSE_ASSERT(nondefiningFunctionDeclaration != NULL);
                    ROSE_ASSERT(nondefiningFunctionDeclaration->get_definition() == NULL);

                    ROSE_ASSERT(scope != NULL);
                    ROSE_ASSERT(scope->lookup_function_symbol(nameWithTemplateArguments) != NULL);
                    ROSE_ASSERT(nondefining_function_param_list->get_parent() != NULL);

                 // DQ (2/10/2012): Check for the associated template declaration.
                    if (buildTemplateInstantiation == true)
                       {
                         SgTemplateInstantiationFunctionDecl* templateInstantiationFunctionDecl = isSgTemplateInstantiationFunctionDecl(nondefiningFunctionDeclaration);
                         ROSE_ASSERT(templateInstantiationFunctionDecl != NULL);

                      // DQ (6/28/2013): Set if template arguments should be explicit for function reference expressions.
                         templateInstantiationFunctionDecl->set_template_argument_list_is_explicit(p->expl_template_arg_list_used);
#if 0
                         printf ("(non-defining) templateInstantiationFunctionDecl->get_template_argument_list_is_explicit() = %s edg routine = %p \n",
                              templateInstantiationFunctionDecl->get_template_argument_list_is_explicit() ? "true" : "false",p);
#endif
                         if (templateInstantiationFunctionDecl->get_templateDeclaration() == NULL)
                            {
#if 0
                              printf ("In convert_routine(): calling setTemplateFunctionDeclaration(p = %p, templateInstantiationFunctionDecl = %p = %s) \n",
                                   p,templateInstantiationFunctionDecl,templateInstantiationFunctionDecl->class_name().c_str());
#endif
                              setTemplateFunctionDeclaration (p,templateInstantiationFunctionDecl);
                            }

                      // DQ (4/30/2012): Relax this test for test2012_58.C.
                         if (templateInstantiationFunctionDecl->get_templateDeclaration() == NULL)
                            {
                              mprintf ("In convert_routine(): Allowing templateInstantiationFunctionDecl->get_templateDeclaration() == NULL \n");
                              mprintf ("In convert_routine(): templateInstantiationFunctionDecl->get_mangled_name() = %s \n",templateInstantiationFunctionDecl->get_mangled_name().str());
                            }
                      // ROSE_ASSERT(templateInstantiationFunctionDecl->get_templateDeclaration() != NULL);
                       }
                  }
                 else
                  {
                    nondefiningFunctionDeclaration = isSgFunctionDeclaration(first_nondefining_declaration);
                    ROSE_ASSERT(nondefiningFunctionDeclaration != NULL);
#if 0
                    printf ("In convert_routine(): using an existing nondefiningFunctionDeclaration = %p = %s fron scope = %p = %s \n",nondefiningFunctionDeclaration,nondefiningFunctionDeclaration->class_name().c_str(),scope,scope->class_name().c_str());
                    printf ("   --- nondefiningFunctionDeclaration->get_name() = %s \n",nondefiningFunctionDeclaration->get_name().str());
#endif
                    if (scope->lookup_function_symbol(nameWithTemplateArguments) == NULL)
                       {
                         printf ("Error: Can't find nameWithTemplateArguments = %s in scope = %p = %s \n",nameWithTemplateArguments.str(),scope,scope->class_name().c_str());
#if 0
                         printf ("In convert_routine(): first_nondefining_declaration = %p = %s = %s \n",first_nondefining_declaration,first_nondefining_declaration->class_name().c_str(),first_nondefining_declaration->get_name().str());
#endif
                         if (first_nondefining_declaration->get_scope() != scope)
                            {
                              printf ("WARNING: The expected scope and the scope of the first_nondefining_declaration are different! \n");
                              printf ("--- scope = %p = %s \n",scope,scope->class_name().c_str());
                              ROSE_ASSERT(first_nondefining_declaration->get_scope() != NULL);
                              printf ("--- first_nondefining_declaration->get_scope() = %p = %s \n",first_nondefining_declaration->get_scope(),first_nondefining_declaration->get_scope()->class_name().c_str());
                              ROSE_ASSERT(first_nondefining_declaration->get_scope()->lookup_function_symbol(nameWithTemplateArguments) != NULL);

                              printf ("NOTE: Reset the scope to match the non-defining declaration! \n");
                              if (isSgNamespaceDefinitionStatement(scope) != NULL)
                                 {
                                   scope = first_nondefining_declaration->get_scope();
                                 }
                                else
                                 {
                                   printf ("Error: if this is not a namespace then the scopes should match! \n");
                                   ROSE_ASSERT(false);
                                 }
                            }
                         ROSE_ASSERT(isSgNamespaceDefinitionStatement(scope) != NULL || first_nondefining_declaration->get_scope() == scope);
                       }

                 // DQ (2/11/2012): If this is a part of a namespace then there could be many of them for a namespace and we don't unify them as they are built (yet).
                    ROSE_ASSERT(isSgNamespaceDefinitionStatement(scope) != NULL || scope->lookup_function_symbol(nameWithTemplateArguments) != NULL);
                  }

               first_nondefining_declaration = isSgFunctionDeclaration(nondefiningFunctionDeclaration->get_firstNondefiningDeclaration());
               ROSE_ASSERT(first_nondefining_declaration != NULL);
#if 0
               printf ("In convert_routine(): first_nondefining_declaration = %p = %s \n",first_nondefining_declaration,first_nondefining_declaration->class_name().c_str());
               printf ("In convert_routine(): first_nondefining_declaration->get_mangled_name() = %s \n",first_nondefining_declaration->get_mangled_name().str());
#endif
            // DQ (7/24/2012): Note that this fails for test2004_149.C; I think this is OK.
            // DQ (7/23/2012): Added a new test (buildDefiningFunctionDeclaration() can't locate the symbol for test2004_77.C).
               SgSymbol* testAccessToFunctionSymbol_1 = first_nondefining_declaration->get_symbol_from_symbol_table();
               if (testAccessToFunctionSymbol_1 == NULL)
                  {
                    printf ("This appears to be a rare case where the symbol is not found (see test2004_149.C) first_nondefining_declaration = %p = %s \n",first_nondefining_declaration,first_nondefining_declaration->class_name().c_str());
                  }

            // DQ (8/19/2013): Added tests ahead of failing test in buildDefiningFunctionDeclaration().
               ROSE_ASSERT(first_nondefining_declaration != NULL);
               ROSE_ASSERT(first_nondefining_declaration->get_firstNondefiningDeclaration() != NULL);

            // DQ (8/19/2013): This fails for test3 of RoseExamples_tests.
               ROSE_ASSERT(first_nondefining_declaration->get_firstNondefiningDeclaration() == first_nondefining_declaration);

#if 1
            // DQ (9/9/2014): Build this seperately for each local scope and SgFunctionDeclaration so that we can control where
            // we need these to be build with and without default parameters.
#if 0
               printf ("In convert_routine(): Calling convert_function_parameter_list() to build function type and used in defining declaration (allow default arguments) \n");
#endif
               bool skipDefaultArguments = false;
#if 0
               printf ("In convert_routine(): skipDefaultArguments = %s \n",skipDefaultArguments ? "true" : "false");
#endif
            // DQ (4/21/2018): Add support to build a SgDeclarationScope if it is detected in the parameter type constrcution 
            // that we need one for types that are defined in the function parameter list.  See test2018_41.c for an example.
               SgDeclarationScope* declarationScope = NULL;

               SgFunctionParameterList* param_list = convert_function_parameter_list(p,skipDefaultArguments,NULL,declarationScope);
               ROSE_ASSERT(param_list != NULL);
#if 0
               printf ("In convert_routine(): DONE: Calling convert_function_parameter_list() to build function type and used in defining declaration (allow default arguments) \n");
               printf ("   --- param_list = %p \n",param_list);
#endif
#endif
#if 0
               printf ("In convert_routine(): nameWithOutTemplateArguments  = %s \n",nameWithOutTemplateArguments.str());
               printf ("In convert_routine(): scope                         = %p = %s \n",scope,scope->class_name().c_str());
               printf ("In convert_routine(): buildTemplateInstantiation    = %s \n",buildTemplateInstantiation ? "true" : "false");
               printf ("In convert_routine(): first_nondefining_declaration = %p = %s \n",first_nondefining_declaration,first_nondefining_declaration->class_name().c_str());
               printf ("In convert_routine(): first_nondefining_declaration->get_name() = %s \n",first_nondefining_declaration->get_name().str());
#endif
            // Now build the defining declaration expected for the call to this function.
               definingFunctionDeclaration = buildDefiningFunctionDeclaration(nameWithOutTemplateArguments, return_type, param_list, scope, decoratorList, buildTemplateInstantiation, first_nondefining_declaration, &templateArgumentsList);

#if 0
               printf ("In convert_routine(): buildTemplateInstantiation = %s definingFunctionDeclaration = %p = %s \n",
                    buildTemplateInstantiation ? "true" : "false",definingFunctionDeclaration,definingFunctionDeclaration->class_name().c_str());
#endif
            // DQ (2/10/2012): If the non-defining declaration is a template instantiation, then make sure that the defining declaration is also.
               if (isSgTemplateInstantiationFunctionDecl(first_nondefining_declaration) != NULL)
                  {
                    SgTemplateInstantiationFunctionDecl* definingTemplateInstantiationFunctionDecl = isSgTemplateInstantiationFunctionDecl(definingFunctionDeclaration);
                    ROSE_ASSERT(definingTemplateInstantiationFunctionDecl != NULL);

                 // DQ (6/28/2013): Set if template arguments should be explicit for function reference expressions.
                    definingTemplateInstantiationFunctionDecl->set_template_argument_list_is_explicit(p->expl_template_arg_list_used);
#if 0
                    printf ("definingTemplateInstantiationFunctionDecl->get_template_argument_list_is_explicit() = %s edg routine = %p \n",definingTemplateInstantiationFunctionDecl->get_template_argument_list_is_explicit() ? "true" : "false",p);
#endif
                 // DQ (4/30/2012): the template is not always available (e.g. if it is listed as non defined in EDG: see test2012_58.C).
                 // DQ (2/10/2012): Check for the associated template declaration.
                 // ROSE_ASSERT(isSgTemplateInstantiationFunctionDecl(first_nondefining_declaration)->get_templateDeclaration() != NULL);
                    if (isSgTemplateInstantiationFunctionDecl(first_nondefining_declaration)->get_templateDeclaration() != NULL)
                       {
                      // Use the template declaration from the non-definng declaration to set the associated template declaration in the defining declaration.

                      // DQ (2/10/2012): Check that the associated template declaration is set.
                      // setTemplateFunctionDeclaration (p,nondefiningMemberFunctionDeclaration);
                         if (isSgTemplateInstantiationFunctionDecl(definingFunctionDeclaration)->get_templateDeclaration() == NULL)
                            {
                           // Find the associated template declaration (in the nondefining declaration) and set the reference to it in the template instantiation.
                              SgTemplateFunctionDeclaration* templateFunctionDeclaration = isSgTemplateInstantiationFunctionDecl(first_nondefining_declaration)->get_templateDeclaration();
                              ROSE_ASSERT(templateFunctionDeclaration != NULL);
#if 0
                              printf ("Getting the template from the nondefining declaration \n");
#endif
                              isSgTemplateInstantiationFunctionDecl(definingFunctionDeclaration)->set_templateDeclaration(templateFunctionDeclaration);
#if 0
                              ROSE_ASSERT(definingFunctionDeclaration->get_scope() != NULL);
                           // SgTemplateFunctionDeclaration* templateFunctionDeclaration = definingFunctionDeclaration->get_scope()->lookup_template_function_symbol();
                              SgName name = definingFunctionDeclaration->get_name();
                              printf ("definingFunctionDeclaration: function name = %s \n",name.str());
                           // SgTemplateFunctionDeclaration* templateFunctionDeclaration = definingFunctionDeclaration->get_scope()->lookup_template_function_symbol();
#endif
                            }
                         ROSE_ASSERT(isSgTemplateInstantiationFunctionDecl(definingFunctionDeclaration)->get_templateDeclaration() != NULL);
                       }
                      else
                       {
#ifdef DEBUG_CONVERSION
                         printf ("In convert_routine(): (nondefining declaration's template declaration was not set): definingFunctionDeclaration->get_mangled_name() = %s \n",definingFunctionDeclaration->get_mangled_name().str());
#endif
                       }
#if 0
                    printf ("isSgTemplateInstantiationFunctionDecl(first_nondefining_declaration)->get_templateName() = %s \n",isSgTemplateInstantiationFunctionDecl(first_nondefining_declaration)->get_templateName().str());
                    printf ("isSgTemplateInstantiationFunctionDecl(definingFunctionDeclaration)->get_templateName()   = %s \n",isSgTemplateInstantiationFunctionDecl(definingFunctionDeclaration)->get_templateName().str());
#endif
                    ROSE_ASSERT(isSgTemplateInstantiationFunctionDecl(definingFunctionDeclaration)->get_templateName().is_null() == false);
                    ROSE_ASSERT(isSgTemplateInstantiationFunctionDecl(first_nondefining_declaration)->get_templateName().is_null() == false);
                  }

            // This is the SgFunctionDeclaration (used internally in this function).
            // definingFunctionDeclaration = definingFunctionDeclaration;

            // This is the SgDeclarationStatement that will be returned from this function.
               return_defining_declaration = definingFunctionDeclaration;

            // Set the source position for the parameter list.
               set_file_info_for_corresp(definingFunctionDeclaration->get_parameterList(), p->source_corresp);

               ROSE_ASSERT(definingTemplateFunctionDeclaration == NULL);
               ROSE_ASSERT(definingFunctionDeclaration != NULL);

               ROSE_ASSERT(definingFunctionDeclaration->get_definition() != NULL);

               ROSE_ASSERT(definingFunctionDeclaration    != NULL);
               ROSE_ASSERT(nondefiningFunctionDeclaration != NULL);

            // ROSE_ASSERT(scope->lookup_function_symbol(p->source_corresp.name) != NULL);
            // ROSE_ASSERT(scope->lookup_function_symbol(name) != NULL);
               ROSE_ASSERT(scope->lookup_function_symbol(nameWithTemplateArguments) != NULL);
               ROSE_ASSERT(param_list->get_parent() != NULL);

#if 1
            // DQ (4/9/2012): This this case we should have passed the template into this function (we have to modify the API).
               ROSE_ASSERT(edg_template == NULL);

            // DQ (3/25/2017): Eliminate Clang warning (which caught a bug): warning: if statement has empty body [-Wempty-body]
            // DQ (4/9/2012): Add this to the template cache
            // ROSE_ASSERT(first_nondefining_declarations_for_functions.find(p) == first_nondefining_declarations_for_functions.end());
            // if (first_nondefining_declarations_for_functions.find(p) == first_nondefining_declarations_for_functions.end());
               if (first_nondefining_declarations_for_functions.find(p) == first_nondefining_declarations_for_functions.end())
                  {
#if 0
                    printf ("In convert_routine(): Adding nondefiningFunctionDeclaration = %p = %s = %s to first_nondefining_declarations_for_functions[p = %p] \n",
                         nondefiningFunctionDeclaration,nondefiningFunctionDeclaration->class_name().c_str(),nondefiningFunctionDeclaration->get_name().str(),p);
                    printf ("   --- Changed to use nondefiningFunctionDeclaration->get_firstNondefiningDeclaration() = %p \n",nondefiningFunctionDeclaration->get_firstNondefiningDeclaration());
#endif
                 // first_nondefining_declarations_for_functions[p] = nondefiningFunctionDeclaration;
                    first_nondefining_declarations_for_functions[p] = isSgFunctionDeclaration(nondefiningFunctionDeclaration->get_firstNondefiningDeclaration());
                  }
               ROSE_ASSERT(first_nondefining_declarations_for_functions[p] != NULL);

            // DQ (11/23/2015): We have to allow for multiple defining declarations for functions (which is counter intuative) to be
            // build for friend functions in class template definitions and the scope (namespace or global) where they can be injected.
            // DQ (4/9/2012): Add this to the template cache
            // ROSE_ASSERT(defining_declarations_for_functions.find(p) == defining_declarations_for_functions.end());
               if (defining_declarations_for_functions.find(p) == defining_declarations_for_functions.end())
                  {
                    defining_declarations_for_functions[p] = definingFunctionDeclaration;
                  }
                 else
                  {
                 // DQ (11/23/2015): Output at lease a note about this rare case.
                    mprintf ("NOTE: Detected extra function declaration in ROSE mapped to routine pointer in EDG (required to represent friend template functions instantiations) \n");
                    mprintf ("   --- definingFunctionDeclaration = %p = %s \n",definingFunctionDeclaration,definingFunctionDeclaration->class_name().c_str());
                  }
               ROSE_ASSERT(defining_declarations_for_functions[p] != NULL);

               ROSE_ASSERT( (first_nondefining_declarations_for_functions.find(p) != first_nondefining_declarations_for_functions.end()) || 
                            (defining_declarations_for_functions.find(p) != defining_declarations_for_functions.end()) );
#endif
             }
            else
             {
            // DQ (7/22/2012): Added assertion to make the code more clear.
               ROSE_ASSERT(buildTemplateDeclaration == true);
#if 0
               printf ("In convert_routine(): buildTemplateDeclaration == true: first_nondefining_declaration = %p \n",first_nondefining_declaration);
#endif
            // definingTemplateMemberFunctionDeclaration = new SgTemplateMemberFunctionDeclaration(p->source_corresp.name, function_type, NULL);
               if (first_nondefining_declaration == NULL)
                  {
#if 0
#if 0
                     printf ("In convert_routine(): Calling parse_template_parameters() \n");
#endif
                     SgTemplateParameterPtrList templateParameterList = parse_template_parameters(p,scope,NULL);
                     ROSE_ASSERT(templateParameterList.empty() == false);
#if 0
                     printf ("Exiting as a test (need template parameter list) \n");
                     ROSE_ASSERT(false);
#endif
#endif

#if 0
                    printf ("In convert_routine(): Calling convert_function_parameter_list() for non-defining template non-member function declaration (skip the default arguments = true) \n");
#endif
                 // DQ (4/21/2018): Add support to build a SgDeclarationScope if it is detected in the parameter type constrcution 
                 // that we need one for types that are defined in the function parameter list.  See test2018_41.c for an example.
                    SgDeclarationScope* declarationScope = NULL;

                 // DQ (12/15/2020): We need to capture the default arguments.
                 // DQ (12/4/2011): If we build a separate non-defining function, it need it's own paramter list (without default arguments).
                 // bool skipDefaultArguments = true;
                    bool skipDefaultArguments = false;

                 // SgFunctionParameterList* nondefining_function_param_list = convert_function_parameter_list(p,skipDefaultArguments);
                    SgFunctionParameterList* nondefining_function_param_list = convert_function_parameter_list(p,skipDefaultArguments,NULL,declarationScope);
                    ROSE_ASSERT(nondefining_function_param_list != NULL);
#if 0
                    printf ("In convert_routine(): DONE: Calling convert_function_parameter_list() for non-defining template non-member function declaration (skip the default arguments = true) \n");
                    printf ("   --- nondefining_function_param_list = %p \n",nondefining_function_param_list);
#endif

                 // DQ (5/2/2012): Test this to make sure we have SgInitializedNames set properly.
                    SageInterface::setSourcePosition(nondefining_function_param_list);

#if MAKE_REDUNDANT_CALLS_TO_DETECT_TRANSFORAMTIONS
                    detectTransformations_local(nondefining_function_param_list);
#endif
#if 0
                    printf ("In convert_routine(): Calling buildNondefiningTemplateFunctionDeclaration(): nameWithOutTemplateArguments = %s \n",nameWithOutTemplateArguments.str());
#endif
                 // DQ (8/9/2013): Since we can't have template specialization of template functions, we don't need the templateArgumentsList for this function to build the template function.
                 // nondefiningTemplateFunctionDeclaration = buildNondefiningTemplateFunctionDeclaration(p->source_corresp.name, return_type, nondefining_function_param_list, scope, decoratorList);
                 // nondefiningTemplateFunctionDeclaration = buildNondefiningTemplateFunctionDeclaration(name, return_type, nondefining_function_param_list, scope, decoratorList);
                 // nondefiningTemplateFunctionDeclaration = buildNondefiningTemplateFunctionDeclaration(nameWithOutTemplateArguments, return_type, nondefining_function_param_list, scope, decoratorList,&templateArgumentsList);
                    nondefiningTemplateFunctionDeclaration = buildNondefiningTemplateFunctionDeclaration(nameWithOutTemplateArguments, return_type, nondefining_function_param_list, scope, decoratorList, &templateParameterList);
                 // first_nondefining_declaration = nondefiningTemplateFunctionDeclaration;
                    ROSE_ASSERT(nondefiningTemplateFunctionDeclaration != NULL);
#if 0
                    printf ("DONE: In convert_routine(): call to buildNondefiningTemplateFunctionDeclaration(): need to fold template argument list into function name \n");
#endif
                 // DQ (2/12/2015): Added assertions earlier before calling buildDefiningFunctionDeclaration_T<>().
                    ROSE_ASSERT(nondefiningTemplateFunctionDeclaration->get_firstNondefiningDeclaration() != NULL);
#if 0
                 // DQ (10/22/2016): This is allows in the case of the AST merge mechanism.
                    if (nondefiningTemplateFunctionDeclaration->get_firstNondefiningDeclaration() != nondefiningTemplateFunctionDeclaration)
                       {
#ifdef DEBUG_CONVERSION
                         printf ("In convert_routine(): nondefiningTemplateFunctionDeclaration->get_firstNondefiningDeclaration() != nondefiningTemplateFunctionDeclaration: this is likely an additional prototype function \n");
#endif
                       }
#endif
                 // DQ (3/7/2015): This failes for mergeTest_04.C (comment out for now).
                 // ROSE_ASSERT(nondefiningTemplateFunctionDeclaration->get_firstNondefiningDeclaration() == nondefiningTemplateFunctionDeclaration);
#if 0
                    printf ("Exiting as a test! \n");
                    ROSE_ASSERT(false);
#endif
                 // Set the source position for the parameter list.
                    set_file_info_for_corresp(nondefiningTemplateFunctionDeclaration->get_parameterList(), p->source_corresp);

                 // DQ (8/13/2013): Adding test of template parameter lists.
                    ROSE_ASSERT(templateParameterList.size() == nondefiningTemplateFunctionDeclaration->get_templateParameters().size());

                    ROSE_ASSERT(nondefiningTemplateFunctionDeclaration != NULL);
                    ROSE_ASSERT(nondefiningTemplateFunctionDeclaration->get_definition() == NULL);

                    ROSE_ASSERT(nondefiningTemplateFunctionDeclaration->get_type() != NULL);
                 // ROSE_ASSERT(scope->lookup_template_function_symbol(p->source_corresp.name,nondefiningTemplateFunctionDeclaration->get_type()) != NULL);
                 // ROSE_ASSERT(scope->lookup_template_function_symbol(name,nondefiningTemplateFunctionDeclaration->get_type()) != NULL);
                 // ROSE_ASSERT(scope->lookup_template_function_symbol(nameWithOutTemplateArguments,nondefiningTemplateFunctionDeclaration->get_type()) != NULL);
                 // ROSE_ASSERT(scope->lookup_template_function_symbol(nameWithOutTemplateArguments,nondefiningTemplateFunctionDeclaration->get_type(),NULL,&templateArgumentsList) != NULL);
                    ROSE_ASSERT(scope->lookup_template_function_symbol(nameWithOutTemplateArguments,nondefiningTemplateFunctionDeclaration->get_type(),&templateParameterList) != NULL);
                    ROSE_ASSERT(nondefining_function_param_list->get_parent() != NULL);

                 // DQ (12/11/2011): Added new test.
                 // SgSymbol* symbol = nondefiningTemplateFunctionDeclaration->get_symbol_from_symbol_table();
                 // ROSE_ASSERT(symbol != NULL);
                    ROSE_ASSERT(verifyExistenceOfSymbol(nondefiningTemplateFunctionDeclaration) == true);
#if 0
                    printf ("DONE: In convert_routine(): call to verifyExistenceOfSymbol(): need to fold template argument list into function name \n");
#endif
#if 0
                    printf ("Exiting as a test! \n");
                    ROSE_ASSERT(false);
#endif
                 // DQ (12/12/2011): Added call to get type to force generation of SgFunctionType using template member function.
                    SgFunctionType* templateFunctionType = nondefiningTemplateFunctionDeclaration->get_type();
                    ROSE_ASSERT(templateFunctionType != NULL);
#if 0
                    printf ("first_nondefining_declaration          = %p \n",first_nondefining_declaration);
                    printf ("nondefiningTemplateFunctionDeclaration = %p \n",nondefiningTemplateFunctionDeclaration);
                    printf ("nondefiningTemplateFunctionDeclaration->get_firstNondefiningDeclaration() = %p \n",
                            (nondefiningTemplateFunctionDeclaration != NULL) ? nondefiningTemplateFunctionDeclaration->get_firstNondefiningDeclaration() : NULL);
#endif
                 // DQ (9/24/2015): Adding test of template parameter lists.
                    if (templateParameterList.size() != nondefiningTemplateFunctionDeclaration->get_templateParameters().size())
                       {
                         printf ("Error: size mismatch: case nondefining_declaration == NULL: templateParameterList.size() = %zu \n",templateParameterList.size());
                         printf ("Error: size mismatch: case nondefining_declaration == NULL: nondefiningTemplateFunctionDeclaration->get_templateParameters().size() = %zu \n",
                              nondefiningTemplateFunctionDeclaration->get_templateParameters().size());
                       }
                    ROSE_ASSERT(templateParameterList.size() == nondefiningTemplateFunctionDeclaration->get_templateParameters().size());

                    SgTemplateFunctionDeclaration* first_nondefiningTemplateFunctionDeclaration = isSgTemplateFunctionDeclaration(nondefiningTemplateFunctionDeclaration->get_firstNondefiningDeclaration());
                    ROSE_ASSERT(first_nondefiningTemplateFunctionDeclaration != NULL);

                 // DQ (9/24/2015): Adding test of template parameter lists.
                    if (templateParameterList.size() != first_nondefiningTemplateFunctionDeclaration->get_templateParameters().size())
                       {
                         printf ("Error: size mismatch: case first_nondefining_declaration == NULL: templateParameterList.size() = %zu \n",templateParameterList.size());
                         printf ("Error: size mismatch: case first_nondefining_declaration == NULL: first_nondefiningTemplateFunctionDeclaration->get_templateParameters().size() = %zu \n",
                              first_nondefiningTemplateFunctionDeclaration->get_templateParameters().size());

                         first_nondefiningTemplateFunctionDeclaration->get_file_info()->display("first_nondefiningTemplateFunctionDeclaration");
                         nondefiningTemplateFunctionDeclaration->get_file_info()->display("nondefiningTemplateFunctionDeclaration");
                       }
                    ROSE_ASSERT(templateParameterList.size() == first_nondefiningTemplateFunctionDeclaration->get_templateParameters().size());

                 // DQ (7/20/2017): Adding support for classes built in the declaration scope added to the template declaration.
                    setupDeclarationScope(nondefiningTemplateFunctionDeclaration);
                  }
                 else
                  {
                 // first_nondefining_declaration = isSgTemplateFunctionDeclaration(first_nondefining_declaration);
                 // ROSE_ASSERT(first_nondefining_declaration != NULL);
                    nondefiningTemplateFunctionDeclaration = isSgTemplateFunctionDeclaration(first_nondefining_declaration);
                    ROSE_ASSERT(nondefiningTemplateFunctionDeclaration != NULL);

                 // ROSE_ASSERT(scope->lookup_template_symbol(p->source_corresp.name) != NULL);

                // DQ (3/8/2012): test2012_26.C demonstrates that the correct scope might be that of the associated first_nondefining_declaration!
#if 0
                    printf ("In convert_routine(): scope = %p = %s nameWithOutTemplateArguments = %s \n",scope,scope->class_name().c_str(),nameWithOutTemplateArguments.str());
#endif

                    ROSE_ASSERT(first_nondefining_declaration->get_scope() != NULL);
                    if (first_nondefining_declaration->get_scope() != scope)
                       {
                      // This can be because of a friend function or because the function was defined in another instance of the same namespace.
                      // We might want to fixup the handling of multiple instance of namespaces so that the lookup_template_function_symbol() 
                      // function will use the SgAliasSymbols.
                         printf ("first_nondefining_declaration->get_scope() != scope: This can happend for friend functions (it is OK) \n");
                         printf ("   scope                                      = %p = %s \n",scope,scope->class_name().c_str());
                         printf ("   first_nondefining_declaration->get_scope() = %p = %s \n",first_nondefining_declaration->get_scope(),first_nondefining_declaration->get_scope()->class_name().c_str());
                       }
                 // ROSE_ASSERT(first_nondefining_declaration->get_scope() == scope);
                 // scope->get_symbol_table()->print("In convert_routine()");
                 // ROSE_ASSERT(scope->lookup_template_symbol(name) != NULL);
                 // ROSE_ASSERT(scope->lookup_template_function_symbol(name) != NULL);
                 // printf ("SHOULD WE BE USING THE MANGLED NAME? \n");

                 // DQ (3/8/2012): We need to use the scope of the first defining declaration if it exists.
                 // printf ("Could this be a friend function? \n");
#if 0
                    printf ("Using function type = %p to lookup the template function symbol \n",nondefiningTemplateFunctionDeclaration->get_type());
#endif
                 // ROSE_ASSERT(scope->lookup_template_function_symbol(name,nondefiningTemplateFunctionDeclaration->get_type()) != NULL);
                 // ROSE_ASSERT(first_nondefining_declaration->get_scope()->lookup_template_function_symbol(name,nondefiningTemplateFunctionDeclaration->get_type()) != NULL);
                 // ROSE_ASSERT(first_nondefining_declaration->get_scope()->lookup_template_function_symbol(nameWithOutTemplateArguments,nondefiningTemplateFunctionDeclaration->get_type()) != NULL);
                    ROSE_ASSERT(first_nondefining_declaration->get_scope()->lookup_template_function_symbol(nameWithOutTemplateArguments,nondefiningTemplateFunctionDeclaration->get_type(),&templateParameterList) != NULL);

                 // ROSE_ASSERT(scope->lookup_template_function_symbol(name,nondefiningTemplateFunctionDeclaration->get_type()) != NULL);

                 // DQ (8/13/2013): Adding test of template parameter lists.
                    ROSE_ASSERT(templateParameterList.size() == nondefiningTemplateFunctionDeclaration->get_templateParameters().size());
                  }

            // DQ (12/11/2011): Added new test.
            // SgSymbol* symbol = nondefiningTemplateFunctionDeclaration->get_symbol_from_symbol_table();
            // ROSE_ASSERT(symbol != NULL);
               ROSE_ASSERT(verifyExistenceOfSymbol(nondefiningTemplateFunctionDeclaration) == true);

               nondefiningFunctionDeclaration = nondefiningTemplateFunctionDeclaration;

#if 0
               printf ("nondefiningTemplateFunctionDeclaration      = %p \n",nondefiningTemplateFunctionDeclaration);
               printf ("BEFORE reset: first_nondefining_declaration = %p \n",first_nondefining_declaration);
#endif

            // TV (2/4/14) : in the case of merge, we can build a nondef decl (believed to be the first from EDG point of view) 
            //               but have the actual first nondef decl be in a previously parsed compilation unit.
               first_nondefining_declaration = isSgFunctionDeclaration(nondefiningTemplateFunctionDeclaration->get_firstNondefiningDeclaration());
               ROSE_ASSERT(first_nondefining_declaration != NULL);

#if 0
               printf ("AFTER reset: first_nondefining_declaration  = %p \n",first_nondefining_declaration);
#endif

            // DQ (7/31/2013): Added assertions earlier before calling buildDefiningFunctionDeclaration_T<>().
               ROSE_ASSERT(first_nondefining_declaration != NULL);
               ROSE_ASSERT(first_nondefining_declaration->get_firstNondefiningDeclaration() != NULL);
               ROSE_ASSERT(first_nondefining_declaration->get_firstNondefiningDeclaration() == first_nondefining_declaration);

               SgTemplateFunctionDeclaration* first_nondefiningTemplateFunctionDeclaration = isSgTemplateFunctionDeclaration(first_nondefining_declaration);
               ROSE_ASSERT(first_nondefiningTemplateFunctionDeclaration != NULL);

            // DQ (9/24/2015): Adding test of template parameter lists.
               if (nondefiningTemplateFunctionDeclaration->get_templateParameters().size() != first_nondefiningTemplateFunctionDeclaration->get_templateParameters().size())
                  {
                    ROSE_ASSERT(first_nondefiningTemplateFunctionDeclaration != NULL);
                    ROSE_ASSERT(nondefiningTemplateFunctionDeclaration != NULL);
                    printf ("Error: size mismatch: first_nondefiningTemplateFunctionDeclaration->get_templateParameters().size() = %zu \n",
                         first_nondefiningTemplateFunctionDeclaration->get_templateParameters().size());
                    printf ("Error: size mismatch: nondefiningTemplateFunctionDeclaration->get_templateParameters().size()       = %zu \n",
                         nondefiningTemplateFunctionDeclaration->get_templateParameters().size());
                    printf ("Error: size mismatch: templateParameterList.size()                                                  = %zu \n",templateParameterList.size());
                  }
            // DQ (9/24/2015): If this fails then we know that the first_nondefining_template_function_declaration was not setup properly.
               ROSE_ASSERT(nondefiningTemplateFunctionDeclaration->get_templateParameters().size() == first_nondefiningTemplateFunctionDeclaration->get_templateParameters().size());

#if 1
            // DQ (9/9/2014): Changed the default value of skipDefaultArguments to TRUE.
            // DQ (9/9/2014): Build this seperately for each local scope and SgFunctionDeclaration so that we can control where
            // we need these to be build with and without default parameters.
#if 0
               printf ("In convert_routine(): Calling convert_function_parameter_list() to build function type and used in defining declaration (allow default arguments) \n");
#endif
            // DQ (12/16/2020): Fixing test2020_79.C (default arguments for template declarations in the AST).
            // bool skipDefaultArguments = true;
               bool skipDefaultArguments = false;
#if 0
               printf ("In convert_routine(): skipDefaultArguments = %s \n",skipDefaultArguments ? "true" : "false");
#endif
            // DQ (4/21/2018): Add support to build a SgDeclarationScope if it is detected in the parameter type constrcution 
            // that we need one for types that are defined in the function parameter list.  See test2018_41.c for an example.
               SgDeclarationScope* declarationScope = NULL;

            // SgFunctionParameterList* param_list = convert_function_parameter_list(p);
            // SgFunctionParameterList* param_list = convert_function_parameter_list(p,skipDefaultArguments);
               SgFunctionParameterList* param_list = convert_function_parameter_list(p,skipDefaultArguments,NULL,declarationScope);
               ROSE_ASSERT(param_list != NULL);
#if 0
               printf ("In convert_routine(): DONE: Calling convert_function_parameter_list() to build function type and used in defining declaration (allow default arguments) \n");
               printf ("   --- param_list = %p \n",param_list);
#endif
#endif
            // TV (2/4/14) : we need it to be a SgTemplateFunctionDeclaration for SageBuilder::buildDefiningTemplateFunctionDeclaration()
               SgTemplateFunctionDeclaration * first_nondefining_template_function_declaration = isSgTemplateFunctionDeclaration(first_nondefining_declaration);
               ROSE_ASSERT(first_nondefining_template_function_declaration != NULL);
#if 0
               printf ("((((((((((((((((((((((( In convert_routine(): calling buildDefiningTemplateFunctionDeclaration() \n");
#endif
            // Now build the defining declaration expected for the call to this function.
            // definingTemplateMemberFunctionDeclaration = buildDefiningTemplateMemberFunctionDeclaration(p->source_corresp.name, return_type, param_list, classDefinition, decoratorList, functionConstVolatileFlags);
            // definingTemplateFunctionDeclaration = buildDefiningTemplateFunctionDeclaration(p->source_corresp.name, return_type, param_list, scope, decoratorList);
            // definingTemplateFunctionDeclaration = buildDefiningTemplateFunctionDeclaration(name, return_type, param_list, scope, decoratorList);
               ROSE_ASSERT(first_nondefining_declaration->get_scope() != NULL);
            // definingTemplateFunctionDeclaration = buildDefiningTemplateFunctionDeclaration(name, return_type, param_list, first_nondefining_declaration->get_scope(), decoratorList, nondefiningTemplateFunctionDeclaration);
               definingTemplateFunctionDeclaration = buildDefiningTemplateFunctionDeclaration(nameWithOutTemplateArguments, return_type, param_list, first_nondefining_declaration->get_scope(), decoratorList, first_nondefining_template_function_declaration);

#if 0
               printf ("))))))))))))))))))))))) In convert_routine(): DONE: calling buildDefiningTemplateFunctionDeclaration() \n");
#endif
            // DQ (3/25/2017): Eliminate Clang warning (which caught a bug): explicitly assigning value of variable of type 'SgTemplateFunctionDeclaration *' to itself [-Wself-assign]
            // This is the SgTemplateFunctionDeclaration (used internally in this function).
            // definingTemplateFunctionDeclaration = definingTemplateFunctionDeclaration;

            // This is the SgDeclarationStatement that will be returned from this function.
               return_defining_declaration = definingTemplateFunctionDeclaration;

            // Set the source position for the parameter list.
               set_file_info_for_corresp(definingTemplateFunctionDeclaration->get_parameterList(), p->source_corresp);

            // DQ (8/13/2013): Adding test of template parameter lists.
               if (templateParameterList.size() != definingTemplateFunctionDeclaration->get_templateParameters().size())
                  {
                    ROSE_ASSERT(first_nondefining_template_function_declaration != NULL);
                    ROSE_ASSERT(nondefiningTemplateFunctionDeclaration != NULL);
                    printf ("Error: size mismatch: first_nondefining_template_function_declaration->get_templateParameters().size() = %zu \n",
                         first_nondefining_template_function_declaration->get_templateParameters().size());
                    printf ("Error: size mismatch: nondefiningTemplateFunctionDeclaration->get_templateParameters().size() = %zu \n",
                         nondefiningTemplateFunctionDeclaration->get_templateParameters().size());
                    printf ("Error: size mismatch: templateParameterList.size() = %zu \n",templateParameterList.size());
                    printf ("Error: size mismatch: definingTemplateFunctionDeclaration->get_templateParameters().size() = %zu \n",
                         definingTemplateFunctionDeclaration->get_templateParameters().size());
                  }
               ROSE_ASSERT(templateParameterList.size() == definingTemplateFunctionDeclaration->get_templateParameters().size());

               ROSE_ASSERT(definingFunctionDeclaration == NULL);
               ROSE_ASSERT(definingTemplateFunctionDeclaration != NULL);

            // DQ (12/28/2011): Set the common variable definingFunctionDeclaration (common to both the template and non-template cases).
               definingFunctionDeclaration = definingTemplateFunctionDeclaration;
               ROSE_ASSERT(definingFunctionDeclaration != NULL);

               ROSE_ASSERT(definingTemplateFunctionDeclaration->get_definition() != NULL);

               ROSE_ASSERT(definingTemplateFunctionDeclaration    != NULL);
               ROSE_ASSERT(nondefiningTemplateFunctionDeclaration != NULL);

               ROSE_ASSERT(nondefiningTemplateFunctionDeclaration->get_type() != NULL);
            // ROSE_ASSERT(scope->lookup_template_function_symbol(p->source_corresp.name,nondefiningTemplateFunctionDeclaration->get_type()) != NULL);
            // ROSE_ASSERT(scope->lookup_template_function_symbol(name,nondefiningTemplateFunctionDeclaration->get_type()) != NULL);
            // ROSE_ASSERT(first_nondefining_declaration->get_scope()->lookup_template_function_symbol(name,nondefiningTemplateFunctionDeclaration->get_type()) != NULL);
            // ROSE_ASSERT(first_nondefining_declaration->get_scope()->lookup_template_function_symbol(nameWithOutTemplateArguments,nondefiningTemplateFunctionDeclaration->get_type()) != NULL);
               ROSE_ASSERT(first_nondefining_declaration->get_scope()->lookup_template_function_symbol(nameWithOutTemplateArguments,nondefiningTemplateFunctionDeclaration->get_type(),&templateParameterList) != NULL);
               ROSE_ASSERT(param_list->get_parent() != NULL);

               ROSE_ASSERT(nondefiningFunctionDeclaration != NULL);

            // DQ (3/30/2012): Check that these are using the template arguments in the name where required (e.g. where no template function parameters are used).
            // printf ("))))))))))) definingTemplateFunctionDeclaration->get_templateName()    = %s \n",definingTemplateFunctionDeclaration->get_templateName().str());
            // printf ("))))))))))) nondefiningTemplateFunctionDeclaration->get_templateName() = %s \n",nondefiningTemplateFunctionDeclaration->get_templateName().str());
               if (isSgTemplateInstantiationFunctionDecl(definingFunctionDeclaration) != NULL)
                  {
                     printf ("))))))))))) isSgTemplateInstantiationFunctionDecl(definingFunctionDeclaration)->get_templateName()    = %s \n",isSgTemplateInstantiationFunctionDecl(definingFunctionDeclaration)->get_templateName().str());
                  }
               if (isSgTemplateInstantiationFunctionDecl(nondefiningFunctionDeclaration) != NULL)
                  {
                     printf ("))))))))))) isSgTemplateInstantiationFunctionDecl(nondefiningFunctionDeclaration)->get_templateName() = %s \n",isSgTemplateInstantiationFunctionDecl(nondefiningFunctionDeclaration)->get_templateName().str());
                  }

            // TV (2/4/14): we only want to insert first nondefining into the template map
               SgFunctionDeclaration * first_nondef_template_function_decl = isSgFunctionDeclaration(nondefiningFunctionDeclaration->get_firstNondefiningDeclaration());
               ROSE_ASSERT(first_nondef_template_function_decl != NULL);
            // DQ (8/10/2012): Refactored code.
               putFunctionInTemplateMap(p, edg_template, first_nondef_template_function_decl, definingFunctionDeclaration);

            // DQ (7/20/2017): Adding support for classes built in the declaration scope added to the template declaration.
               setupDeclarationScope(definingTemplateFunctionDeclaration);
             }

          ROSE_ASSERT(nondefiningFunctionDeclaration != NULL);
          ROSE_ASSERT(definingFunctionDeclaration    != NULL);
        }

  // DQ (9/9/2014): The parameter list is now only declared and build in inner scopes where it is used.
  // This is so that we can more accurately set the use of default function parameter values and also avoid sharing 
  // function parameter lists acorss multiple function declarations.  This is important for the unparsing of 
  // template functions due to redundnat unparsing of default function parameter values.
  // ROSE_ASSERT(param_list->get_parent() != NULL);

     if (buildTemplateDeclaration == false)
        {
          ROSE_ASSERT(definingFunctionDeclaration    != NULL);
          ROSE_ASSERT(nondefiningFunctionDeclaration != NULL);
          ROSE_ASSERT(nondefiningFunctionDeclaration != NULL && definingFunctionDeclaration != NULL);

       // Setup the SgSpecialFunctionModifiers (think that these different calls can be refactored with the ones below).
          set_special_function_modifier (nondefiningFunctionDeclaration->get_specialFunctionModifier(), p->special_kind);
          set_special_function_modifier (definingFunctionDeclaration->get_specialFunctionModifier(), p->special_kind);
        }
       else
        {
       // ROSE_ASSERT(definingTemplateFunctionDeclaration    != NULL);
       // ROSE_ASSERT(nondefiningTemplateFunctionDeclaration != NULL);
       // ROSE_ASSERT(nondefiningTemplateFunctionDeclaration != NULL && definingTemplateFunctionDeclaration != NULL);

       // printf ("No call for set_special_function_modifier() for SgTemplateFunctionDeclaration \n");

       // DQ (12/28/2011): We have to seperate the cases of SgTemplateFunctionDeclaration from SgTemplateMemberFunctionDeclaration
          if (definingTemplateFunctionDeclaration != NULL)
             {
               ROSE_ASSERT(nondefiningTemplateFunctionDeclaration != NULL);
               ROSE_ASSERT(nondefiningTemplateFunctionDeclaration != NULL && definingTemplateFunctionDeclaration != NULL);

               ROSE_ASSERT(definingTemplateMemberFunctionDeclaration    == NULL);
               ROSE_ASSERT(nondefiningTemplateMemberFunctionDeclaration == NULL);

            // Setup the SgSpecialFunctionModifiers
               set_special_function_modifier (nondefiningTemplateFunctionDeclaration->get_specialFunctionModifier(), p->special_kind);
               set_special_function_modifier (definingTemplateFunctionDeclaration->get_specialFunctionModifier(), p->special_kind);
             }
            else
             {
               ROSE_ASSERT(definingTemplateMemberFunctionDeclaration    != NULL);
               ROSE_ASSERT(nondefiningTemplateMemberFunctionDeclaration != NULL);
               ROSE_ASSERT(nondefiningTemplateMemberFunctionDeclaration != NULL && definingTemplateMemberFunctionDeclaration != NULL);

               ROSE_ASSERT(nondefiningTemplateFunctionDeclaration       == NULL);

            // Setup the SgSpecialFunctionModifiers
               set_special_function_modifier (nondefiningTemplateMemberFunctionDeclaration->get_specialFunctionModifier(), p->special_kind);
               set_special_function_modifier (definingTemplateMemberFunctionDeclaration->get_specialFunctionModifier(), p->special_kind);
             }
        }

  // One of the two sets pointer pairs should be valid.
     ROSE_ASSERT( (nondefiningFunctionDeclaration               != NULL && definingFunctionDeclaration               != NULL) || 
                  (nondefiningTemplateFunctionDeclaration       != NULL && definingTemplateFunctionDeclaration       != NULL) ||
                  (nondefiningTemplateMemberFunctionDeclaration != NULL && definingTemplateMemberFunctionDeclaration != NULL) );

     ROSE_ASSERT(first_nondefining_declaration != NULL);
     ROSE_ASSERT(return_defining_declaration != NULL);

     return_defining_declaration->set_parent(topScopeStack());

     ROSE_ASSERT(return_defining_declaration->get_parent() != NULL);
     if (first_nondefining_declaration != NULL)
        {
          ROSE_ASSERT(first_nondefining_declaration->get_parent() != NULL);
        }

#if 0
     printf ("In convert_routine(): return_defining_declaration               = %p = %s \n",return_defining_declaration,return_defining_declaration->class_name().c_str());
     printf ("In convert_routine(): return_defining_declaration->get_parent() = %p = %s \n",return_defining_declaration->get_parent(),return_defining_declaration->get_parent()->class_name().c_str());
#endif

  // *********************************************************************
  // *********************************************************************
     if (buildTemplateDeclaration == false)
        {
          ROSE_ASSERT(nondefiningFunctionDeclaration != NULL);
          ROSE_ASSERT(definingFunctionDeclaration != NULL);

       // Setup the SgFunctionModifiers
          set_function_modifier(nondefiningFunctionDeclaration->get_functionModifier(), p);
          set_function_modifier(definingFunctionDeclaration->get_functionModifier(), p);

       // DQ (8/11/2012): If this is a definition outside of the class where it is declared then supress the virtual modifier.
       // This sort of fixup after calling set_function_modifier() is ugly...
       // printf ("In convert_routine(): p->defined_outside_of_parent = %s \n",p->defined_outside_of_parent ? "true" : "false");
          if (definingFunctionDeclaration->get_functionModifier().isVirtual() == true && p->defined_outside_of_parent == true)
             {
               definingFunctionDeclaration->get_functionModifier().unsetVirtual();
             }

// #if GNU_EXTENSIONS_ALLOWED
       // DQ (8/16/2014): Now using the same API for both GNU and Microsoft attribute handling.
       // Handle GNU and MS specific attributes
#if 0
          printf ("In convert_routine(): buildTemplateDeclaration == false: calling set_routine_attributes(): p = %p \n",p);
#endif
          set_routine_attributes(p,nondefiningFunctionDeclaration);
          set_routine_attributes(p,definingFunctionDeclaration);
// #endif

#if ENABLE_CUDA_SUPPORT
       // Handle CUDA specific attributes
          set_routine_cuda_attributes(p,nondefiningFunctionDeclaration);
          set_routine_cuda_attributes(p,definingFunctionDeclaration);
#endif
        }
       else
        {
       // DQ (3/2/2013): Can we assert this here.
          ROSE_ASSERT(nondefiningFunctionDeclaration != NULL);
          ROSE_ASSERT(definingFunctionDeclaration != NULL);

#ifdef DEBUG_CONVERSION
          printf ("WARNING: function attributes for template declarations not set definingFunctionDeclaration = %p nondefiningFunctionDeclaration = %p \n",definingFunctionDeclaration,nondefiningFunctionDeclaration);
#endif
#if 0
          printf ("In convert_routine(): function attributes for template declarations not set: calling summarize_routine() \n");
          summarize_routine(p);
#endif
#if 0
       // This should be wrong for case I am debugging presently.
          printf ("p->source_corresp.access = %d \n",(int)p->source_corresp.access);
          printf ("as_public       = %d \n",(int)as_public);
          printf ("as_protected    = %d \n",(int)as_protected);
          printf ("as_private      = %d \n",(int)as_private);
          printf ("as_inaccessible = %d \n",(int)as_inaccessible);
#endif
       // ROSE_ASSERT(p->source_corresp.access == as_public);
#if 0
          printf ("Note that function attribute handling for template and non-template functions should be refactored \n");
#endif
       // Setup the SgFunctionModifiers
          set_function_modifier(nondefiningFunctionDeclaration->get_functionModifier(), p);
          set_function_modifier(definingFunctionDeclaration->get_functionModifier(), p);

// #if GNU_EXTENSIONS_ALLOWED
       // DQ (8/16/2014): Now using the same API for both GNU and Microsoft attribute handling.
       // Handle GNU and MS specific attributes
#if 0
          printf ("In convert_routine(): buildTemplateDeclaration == true: calling set_routine_attributes(): p = %p \n",p);
#endif
          set_routine_attributes(p,nondefiningFunctionDeclaration);
          set_routine_attributes(p,definingFunctionDeclaration);
// #endif

#if ENABLE_CUDA_SUPPORT
       // Handle CUDA specific attributes
          set_routine_cuda_attributes(p,nondefiningFunctionDeclaration);
          set_routine_cuda_attributes(p,definingFunctionDeclaration);
#endif
        }

  // DQ (3/2/2013): Adding support for specification of access level.
     unsigned int access_level = p->source_corresp.access;
     set_access_modifier(nondefiningFunctionDeclaration,access_level);
     set_access_modifier(definingFunctionDeclaration,access_level);

#if 0
     printf ("In convert_routine(): access_level = %d definingFunctionDeclaration = %p = %s: access = %s \n",(int)access_level,
          definingFunctionDeclaration,definingFunctionDeclaration->class_name().c_str(),definingFunctionDeclaration->get_declarationModifier().get_accessModifier().displayString().c_str());
     printf ("In convert_routine(): access_level = %d nondefiningFunctionDeclaration = %p = %s: access = %s \n",(int)access_level,
          nondefiningFunctionDeclaration,nondefiningFunctionDeclaration->class_name().c_str(),nondefiningFunctionDeclaration->get_declarationModifier().get_accessModifier().displayString().c_str());
#endif
#if 0
     printf ("In convert_routine(): function attributes for template declarations ARE NOW set: definingFunctionDeclaration = %p nondefiningFunctionDeclaration = %p \n",definingFunctionDeclaration,nondefiningFunctionDeclaration);
#endif

  // DQ (10/11/2015): This is a failing test in the AST consistancy testing, so we need code here to ttrack this issue down.
     SgAccessModifier::access_modifier_enum definingDeclaration_access_modifier         = 
          return_defining_declaration->get_declarationModifier().get_accessModifier().get_modifier();
     SgAccessModifier::access_modifier_enum firstNondefiningDeclaration_access_modifier = 
          return_defining_declaration->get_firstNondefiningDeclaration()->get_declarationModifier().get_accessModifier().get_modifier();
     if (definingDeclaration_access_modifier != firstNondefiningDeclaration_access_modifier)
        {
          printf ("Error: SgAccessModifier::access_modifier_enum's don't match between the defining and non-defining declarations \n");

          printf ("return_defining_declaration = %p = %s \n",return_defining_declaration,return_defining_declaration->class_name().c_str());
          printf ("return_defining_declaration->get_firstNondefiningDeclaration() = %p = %s \n",
               return_defining_declaration->get_firstNondefiningDeclaration(),return_defining_declaration->get_firstNondefiningDeclaration()->class_name().c_str());

          printf ("Warning: definingDeclaration_access_modifier         = %d \n",(int)definingDeclaration_access_modifier);
          printf ("Warning: firstNondefiningDeclaration_access_modifier = %d \n",(int)firstNondefiningDeclaration_access_modifier);

          printf ("In convert_routine(): return_defining_declaration: access_level = %s \n",
               return_defining_declaration->get_declarationModifier().get_accessModifier().displayString().c_str());
          printf ("In convert_routine(): return_defining_declaration->get_firstNondefiningDeclaration(): access_level = %s \n",
               return_defining_declaration->get_firstNondefiningDeclaration()->get_declarationModifier().get_accessModifier().displayString().c_str());

       // DQ (10/11/2015): Make the previously build non-defining declaration referenced by the defining declaration match the defining declaration.
       // return_defining_declaration->get_firstNondefiningDeclaration()->get_declarationModifier().set_accessModifier(return_defining_declaration->get_declarationModifier().get_accessModifier());
          return_defining_declaration->get_firstNondefiningDeclaration()->get_declarationModifier().get_accessModifier() = return_defining_declaration->get_declarationModifier().get_accessModifier();

          definingDeclaration_access_modifier         = 
               return_defining_declaration->get_declarationModifier().get_accessModifier().get_modifier();
          firstNondefiningDeclaration_access_modifier = 
               return_defining_declaration->get_firstNondefiningDeclaration()->get_declarationModifier().get_accessModifier().get_modifier();
        }
     ROSE_ASSERT(definingDeclaration_access_modifier == firstNondefiningDeclaration_access_modifier);

// *********************************************************************
// *********************************************************************

  // DQ (12/9/2011): Adding tests for symbols...
     if (addSymbolWhenFirstNondefiningDeclarationIsBuilt == true)
        {
       // It should not have been built yet, or should it!.
       // ROSE_ASSERT(function_cache.find(p) == function_cache.end());
          if (function_cache.find(p) != function_cache.end())
             {
#ifdef DEBUG_CONVERSION
               printf ("WARNING: function_cache.find(p) != function_cache.end(), I am not clear if this is allowed! \n");
#endif
             }
        }

  // return_defining_declaration = new SgFunctionDeclaration(p->source_corresp.name, function_type, NULL);
     ROSE_ASSERT (return_defining_declaration != NULL);

#if 0
     printf ("In convert_routine(): calling summarize_routine() before setting the source position \n");
     summarize_routine(p);
#endif
#if 0
     printf ("BEFORE setting the source position: \n");
     return_defining_declaration->get_file_info()->display("BEFORE setting the source position: debug");
#endif

#if 0
     printf ("In convert_routine(): test 1: return_defining_declaration = %p = %s \n",return_defining_declaration,return_defining_declaration->class_name().c_str());
     return_defining_declaration->get_startOfConstruct()->display("return_defining_declaration: START: debug");
     return_defining_declaration->get_endOfConstruct()  ->display("return_defining_declaration: END: debug");
#endif

  // This should already be set.
  // DQ  (3/16/2017): This is not yet set for the start of construct where at least this is a template member function.
  // set_file_info_for_corresp(return_defining_declaration, p->source_corresp);

  // DQ (1/6/2020): Need to check if this function should be marked as compiler generated (or the somewhat skitchy source position used).
  // See Cxx11_tests/test2020_18.C for an example.
     bool isCompilerGenerated = false;
     ROSE_ASSERT(return_defining_declaration->get_firstNondefiningDeclaration() != NULL);
     if (return_defining_declaration->get_firstNondefiningDeclaration() != NULL)
        {
          SgTemplateInstantiationFunctionDecl* templateInstantiationFunctionDeclaration = isSgTemplateInstantiationFunctionDecl(return_defining_declaration->get_firstNondefiningDeclaration());
       // ROSE_ASSERT(templateInstantiationFunctionDeclaration != NULL);
#if 0
          printf ("return_defining_declaration->get_firstNondefiningDeclaration() = %p = %s \n",
               return_defining_declaration->get_firstNondefiningDeclaration(),
               return_defining_declaration->get_firstNondefiningDeclaration()->class_name().c_str());
          printf ("templateInstantiationFunctionDeclaration = %p \n",templateInstantiationFunctionDeclaration);
#endif
          if (templateInstantiationFunctionDeclaration != NULL)
             {
#if 0
               printf ("templateInstantiationFunctionDeclaration = %p = %s \n",templateInstantiationFunctionDeclaration,templateInstantiationFunctionDeclaration->class_name().c_str());
#endif
               SgScopeStatement* scope = isSgScopeStatement(templateInstantiationFunctionDeclaration->get_parent());
               ROSE_ASSERT(scope != NULL);
               if (scope != NULL)
                  {
                    isCompilerGenerated = scope->isCompilerGenerated();
#if 0
                    printf ("scope = %p = %s \n",scope,scope->class_name().c_str());
                    printf ("isCompilerGenerated = %s \n",isCompilerGenerated ? "true" : "false");
#endif
#if 0
                    scope->get_file_info()->display("In convert_routine(): scope: debug");
#endif

                    SgTemplateInstantiationDefn* templateInstantiationDefinition = isSgTemplateInstantiationDefn(scope);
                    if (templateInstantiationDefinition != NULL)
                       {
                         SgTemplateInstantiationDecl* templateInstantiationDeclaration = isSgTemplateInstantiationDecl(templateInstantiationDefinition->get_declaration());
                         if (templateInstantiationDeclaration->isCompilerGenerated() == true)
                            {
#if 0
                              printf ("templateInstantiationDeclaration->isCompilerGenerated() == true \n");
#endif
                              isCompilerGenerated = true;
                            }
                       }
                  }
             }
        }

     if (isCompilerGenerated == true)
        {
#if 0
          printf ("Calling return_defining_declaration->setCompilerGenerated() and unsetOutputInCodeGeneration() \n");
#endif
          return_defining_declaration->setCompilerGenerated(); 
          return_defining_declaration->unsetOutputInCodeGeneration();
          return_defining_declaration->unsetSourcePositionUnavailableInFrontend();
        }
       else
        {
#if 0
          printf ("Calling set_file_info_for_corresp(return_defining_declaration, p->source_corresp); \n");
#endif
          set_file_info_for_corresp(return_defining_declaration, p->source_corresp);
        }

#if 0
     printf ("In convert_routine(): test 2: return_defining_declaration = %p = %s \n",return_defining_declaration,return_defining_declaration->class_name().c_str());
     return_defining_declaration->get_startOfConstruct()->display("return_defining_declaration: START: debug");
     return_defining_declaration->get_endOfConstruct()  ->display("return_defining_declaration: END: debug");
#endif

#if 0
     printf ("AFTER setting the source position: \n");
     return_defining_declaration->get_file_info()->display("AFTER setting the source position: debug");
#endif

#if 0
  // set_file_info_for_corresp(return_defining_declaration->get_parameterList(), p->source_corresp);
     if (nondefiningFunctionDeclaration != NULL)
        {
          set_file_info_for_corresp(nondefiningFunctionDeclaration->get_parameterList(), p->source_corresp);
        }

     if (first_nondefining_declaration == NULL)
        {
       // DQ (4/4/2012): Removed unused 2nd parameter in function convert_routine_prototype().
       // first_nondefining_declaration = convert_routine_prototype(p, true);
       // SgDeclarationStatement* declaration = convert_routine_prototype(p, true);
          SgDeclarationStatement* declaration = convert_routine_prototype(p);

       // This is equivalent to just "first_nondefining_declaration = declaration;", but with a little more checking.
       // first_nondefining_declaration = declaration;
          first_nondefining_declaration = isSgFunctionDeclaration(declaration);
          if (first_nondefining_declaration == NULL)
             {
               ROSE_ASSERT(isSgTemplateFunctionDeclaration(declaration) != NULL);
               first_nondefining_declaration = isSgTemplateFunctionDeclaration(declaration);
#if 0
               printf ("This is a SgTemplateFunctionDeclaration \n");
               ROSE_ASSERT(false);
#endif
             }
        }
#endif

  // DQ (10/7/2012): Adding support to mark function as static (fixing this for defining function declarations).
  // This does allow test2012_26.c to mark defining function declarations properly; but there appear to be other problems.
  // This appears to fail for C++ functions in classes that are defined in SgBasicBlock scopes (or nested in variable 
  // declarations) where EDG marks the functions as static even when they are not explicitly marked as such).
#if 0
     printf ("In convert_routine(): setup support for static keyword: buildTemplateDeclaration          = %s \n",buildTemplateDeclaration   ? "true" : "false");
     printf ("In convert_routine(): setup support for static keyword: buildTemplateInstantiation        = %s \n",buildTemplateInstantiation ? "true" : "false");
     printf ("In convert_routine(): setup support for static keyword: p->source_corresp.is_class_member = %s \n",p->source_corresp.is_class_member ? "true" : "false");
#endif

  // DQ (11/12/2020): I just noticed that we are setting this here plus up above in this function (this might not be a real issue).
  // printf ("In convert_routine(): We are setting the storage class support again! \n");

     if (
       storage_modifier == SgStorageModifier::e_default &&
       p->source_corresp.is_class_member == true &&
       sup->this_class == NULL &&
       return_defining_declaration->get_parent() == return_defining_declaration->get_scope()
     ) {
       storage_modifier = convert_storage_class(sc_static);
     }
     return_defining_declaration->get_declarationModifier().get_storageModifier().set_modifier(storage_modifier);

#if 0
  // DQ (1/22/2013): Check the value of the static keyword.
     printf ("Just marked function as static if it was supposed to be marked static: return_defining_declaration->get_declarationModifier().get_storageModifier().isStatic() = %s \n",
          return_defining_declaration->get_declarationModifier().get_storageModifier().isStatic() ? "true" : "false");
#endif

  // DQ (3/23/2012): Adding friend support for member functions. It is not a good idea to mark all of the declarations uniformally (so comment out some of this code).
  // if (p->defined_in_friend_decl == true)
  // if (p->defined_in_friend_decl == true || return_defining_declaration->get_firstNondefiningDeclaration()->get_declarationModifier().isFriend() == true)
     if (p->defined_in_friend_decl == true)
        {
       // DQ (4/3/2012): We need to know what scope this is so that we can know when to mark this as a friend function.
          SgScopeStatement* currentScope = topScopeStack();

#ifdef DEBUG_CONVERSION
          printf ("In convert_routine(): Setting as FRIEND return_defining_declaration = %p = %s currentScope = %p = %s \n",return_defining_declaration,return_defining_declaration->class_name().c_str(),currentScope,currentScope->class_name().c_str());
#endif
       // DQ (4/3/2012): Only set as a friend function if this is within the class declaration (not allowed outside of class scope).
       // return_defining_declaration->get_declarationModifier().setFriend();
          ROSE_ASSERT(currentScope != NULL);
          if (isSgClassDefinition(currentScope) != NULL)
             {
#ifdef DEBUG_CONVERSION
               printf ("Marking this function as a friend function return_defining_declaration = %p = %s currentScope = %p = %s \n",return_defining_declaration,return_defining_declaration->class_name().c_str(),currentScope,currentScope->class_name().c_str());
#endif
               return_defining_declaration->get_declarationModifier().setFriend();
             }
#if 0
          if (return_defining_declaration->get_firstNondefiningDeclaration() != NULL)
               return_defining_declaration->get_firstNondefiningDeclaration()->get_declarationModifier().setFriend();
          if (return_defining_declaration->get_definingDeclaration() != NULL)
               return_defining_declaration->get_definingDeclaration()->get_declarationModifier().setFriend();
#endif

       // DQ (4/3/2012): Modified the test.
       // ROSE_ASSERT(return_defining_declaration->get_declarationModifier().isFriend() == true);
          if (isSgClassDefinition(currentScope) != NULL)
             {
               ROSE_ASSERT(return_defining_declaration->get_declarationModifier().isFriend() == true);
             }
            else
             {
               ROSE_ASSERT(return_defining_declaration->get_declarationModifier().isFriend() == false);
             }
        }
  // ROSE_ASSERT(p->defined_in_friend_decl == return_defining_declaration->get_declarationModifier().isFriend());
  // ROSE_ASSERT( (p->defined_in_friend_decl || return_defining_declaration->get_firstNondefiningDeclaration()->get_declarationModifier().isFriend() == true) == return_defining_declaration->get_declarationModifier().isFriend());

  // DQ (11/27/2015): Adding support to mark the defining declaration as friend when it is normalized by EDG.
     if (is2ndDefiningDeclarationFromNormalizedEdgTemplateDeclaration == true && is1stDefiningDeclarationFromNormalizedEdgTemplateDeclarationMarkedAsFriend == true)
        {
       // Note that without the friend marking we will not be able to get the name qualification correct.
       // But this also means that we need to fixup the unparsing to supress the friend keyword when not 
       // being unpsrsed in the class definition.
#if 0
          mprintf ("In convert_routine(): Setting the 2nd defining declaration to be a friend function \n");
          printf ("In convert_routine(): Setting the 2nd defining declaration to be a friend function \n");
#endif
          ROSE_ASSERT(return_defining_declaration != NULL);
          return_defining_declaration->get_declarationModifier().setFriend();
        }

#if 0
  // If the first non-defining declaration is marked as friend then mark all of the declarations consistantly.
     if (return_defining_declaration->get_firstNondefiningDeclaration()->get_declarationModifier().isFriend() == true)
        {
          if (return_defining_declaration->get_definingDeclaration() != NULL)
               return_defining_declaration->get_definingDeclaration()->get_declarationModifier().setFriend();
        }
#endif
#if 0
  // DQ (3/23/2012): Error checking... Added test (also at the bottom of this function).
     ROSE_ASSERT( (p->defined_in_friend_decl || return_defining_declaration->get_firstNondefiningDeclaration()->get_declarationModifier().isFriend() == true) == return_defining_declaration->get_declarationModifier().isFriend());
     if (return_defining_declaration->get_firstNondefiningDeclaration() != NULL)
        {
          if ( (p->defined_in_friend_decl || return_defining_declaration->get_firstNondefiningDeclaration()->get_declarationModifier().isFriend() == true) != return_defining_declaration->get_firstNondefiningDeclaration()->get_declarationModifier().isFriend())
             {
               printf ("p->defined_in_friend_decl = %s \n",p->defined_in_friend_decl ? "true" : "false");
               printf ("return_defining_declaration->get_firstNondefiningDeclaration()->get_declarationModifier().isFriend() = %s \n",
                    return_defining_declaration->get_firstNondefiningDeclaration()->get_declarationModifier().isFriend() ? "true" : "false");
             }
          ROSE_ASSERT( (p->defined_in_friend_decl || return_defining_declaration->get_firstNondefiningDeclaration()->get_declarationModifier().isFriend() == true) == return_defining_declaration->get_firstNondefiningDeclaration()->get_declarationModifier().isFriend());
        }
     if (return_defining_declaration->get_definingDeclaration() != NULL)
        {
       // ROSE_ASSERT(p->defined_in_friend_decl == return_defining_declaration->get_definingDeclaration()->get_declarationModifier().isFriend());
          ROSE_ASSERT( (p->defined_in_friend_decl || return_defining_declaration->get_firstNondefiningDeclaration()->get_declarationModifier().isFriend() == true) == return_defining_declaration->get_firstNondefiningDeclaration()->get_declarationModifier().isFriend());
        }
#endif

#if 0
     printf ("In convert_routine() (before reset): return_defining_declaration = %p return_defining_declaration->get_firstNondefiningDeclaration() = %p and return_defining_declaration->get_definingDeclaration() = %p \n",
          return_defining_declaration,return_defining_declaration->get_firstNondefiningDeclaration(),return_defining_declaration->get_definingDeclaration());
     printf ("In convert_routine(): return_defining_declaration = %p setting firstNondefiningDeclaration to %p and definingDeclaration to %p \n",
          return_defining_declaration,first_nondefining_declaration,return_defining_declaration);
#endif
#if 0
  // DQ (3/13/2012): this will cause the firstNondefiningDeclaration to be reset and this causes the associated symbol to not be found in the AST consistancy tests.
     return_defining_declaration->set_firstNondefiningDeclaration(first_nondefining_declaration);
     return_defining_declaration->set_definingDeclaration(return_defining_declaration);
#else
  // DQ (3/13/2012): We only want to check that these are set, not reset them (set by AST Build functions).
     ROSE_ASSERT(return_defining_declaration->get_firstNondefiningDeclaration() != NULL);
     ROSE_ASSERT(return_defining_declaration->get_definingDeclaration() != NULL);
#endif

#if 0
  // DQ (3/31/2012): allow for the case where p->assoc_scope == NULL (seen in test2012_58.C).
  // EDG maintains separate regions for each function (I think so that the frontend can work efficiently 
  // on machines with small memories), so get the function scope representing the current function.
  // a_scope_ptr scope = il_header.region_scope_entry[p->assoc_scope];
     a_scope_ptr scope = NULL;
     if (p->assoc_scope != /*NULL_region_number*/ 0)
        {
       // Note that p->assoc_scope is an EDG specific scope number (not a pointr to a scope).
          scope = il_header.region_scope_entry[p->assoc_scope];
        }
       else
        {
       // This case is used for test2012_58.C
          ROSE_ASSERT(p->source_corresp.parent_scope != NULL);
          scope = p->source_corresp.parent_scope;
        }
#endif

  // DQ (3/31/2012): Added new test.
     ROSE_ASSERT(scope != NULL);

     ROSE_ASSERT(return_defining_declaration->get_definition() != NULL);
     ROSE_ASSERT(return_defining_declaration->get_definition()->get_body() != NULL);

#if 0
  // DQ (3/11/2012): check that this is not already in the list before we add it (or
  // verify it is NULL if it is supposed to be in the list but just not initialized yet).
     ROSE_ASSERT (scope_cache.find(scope) == scope_cache.end());

  // scope_cache[scope] = return_defining_declaration->get_definition();
     scope_cache[scope] = return_defining_declaration->get_definition()->get_body();
     ROSE_ASSERT(scope_cache[scope] != NULL);
#else
  // DQ (3/13/2012): I think we want to allow this to be nonequal (instead of an assertion error).
     ROSE_ASSERT(scope != NULL);
     if (scope_cache.find(scope) == scope_cache.end())
        {
          scope_cache[scope] = return_defining_declaration->get_definition()->get_body();
          ROSE_ASSERT(scope_cache[scope] != NULL);
        }
     ROSE_ASSERT(scope_cache[scope] != NULL);
#endif

  // Handle the function body for all functions.
  // DQ (12/28/2011): This can now be made more uniform since template functions and template member functions are derived from SgFunctionDeclaration.
     ROSE_ASSERT(definingFunctionDeclaration != NULL);
     ROSE_ASSERT(definingFunctionDeclaration->get_definition() != NULL);

     ROSE_ASSERT(definingFunctionDeclaration->get_definition()->get_startOfConstruct() != NULL);
     ROSE_ASSERT(definingFunctionDeclaration->get_definition()->get_endOfConstruct() != NULL);

#if 0
     printf ("In convert_routine(): test 3: return_defining_declaration = %p = %s \n",return_defining_declaration,return_defining_declaration->class_name().c_str());
     return_defining_declaration->get_startOfConstruct()->display("return_defining_declaration: START: debug");
     return_defining_declaration->get_endOfConstruct()  ->display("return_defining_declaration: END: debug");
#endif

  // DQ (1/6/2020): This is a redundnt call to set the source position (called above).
  // DQ (5/13/2012): Set the source code position of the function definition.
  // set_file_info_for_corresp(definingFunctionDeclaration,p->source_corresp);

#if 0
     printf ("In convert_routine(): test 4: return_defining_declaration = %p = %s \n",return_defining_declaration,return_defining_declaration->class_name().c_str());
     return_defining_declaration->get_startOfConstruct()->display("return_defining_declaration: START: debug");
     return_defining_declaration->get_endOfConstruct()  ->display("return_defining_declaration: END: debug");
#endif

  // DQ (8/18/2012): We need to see the end of the function before we can set the source position correctly for the start and end position.
#if 0
     printf ("Setup the source position for the function definition when we see the end of the function. \n");
#endif

  // DQ (7/29/2014): C++11 member functions that are marked as delete or default are not represented 
  // a secondary declaration in EDG.  So we have to avoid treating them too much like functions and 
  // defining a function body for them.
  // parse_function_body(scope->source_sequence_list, p, definingFunctionDeclaration);
     bool skip_body_of_cxx11_defaulted_or_deleted_member_function = false;
     skip_body_of_cxx11_defaulted_or_deleted_member_function = (p->is_defaulted == true || p->is_deleted == true);
#if DEBUG_CLASSES_FROM_TEMPLATE_PARAMETERS
     printf ("In convert_routine(): Calling parse_function_body(): skip_body_of_cxx11_defaulted_or_deleted_member_function = %s \n",skip_body_of_cxx11_defaulted_or_deleted_member_function ? "true" : "false");
#endif
#if DEBUG_CLASSES_FROM_TEMPLATE_PARAMETERS
     printf ("@@@@@@@@@@@@@@@@@@ Should have called setupDeclarationScope() before now! numberOfClassesBuiltFromTemplateParameters = %zu \n",internal_intermediate_scope->symbol_table_size());
#endif

  // DQ (7/21/2017): Associate the declaration scope for any classes built from template parameters 
  // with the defining function declaration.  Note that if we don't do this here, then and the declaration 
  // scope will be attached to some class constructed as a result of processing the function body.
     setupDeclarationScope(definingFunctionDeclaration);

  // DQ (7/21/2017): Any possible template parameters built as classes should have been 
  // attached to the template declaration before processing the template function body!
     ROSE_ASSERT(internal_intermediate_scope != NULL);
     ROSE_ASSERT(internal_intermediate_scope->symbol_table_size() == 0);

  // DQ (4/30/2012): Only generate the body if the function is defined (see test2012_58.C).
  // parse_function_body(scope->source_sequence_list, p, definingFunctionDeclaration);
  // ROSE_ASSERT(p->defined == true);
     if (p->defined == true)
        {
          if (skip_body_of_cxx11_defaulted_or_deleted_member_function == false)
             {
               parse_function_body(scope->source_sequence_list, p, definingFunctionDeclaration);
             }
        }
       else
        {
       // DQ (2/18/2013): This branch is taken in compiling rose.h header file.
#ifdef DEBUG_CONVERSION
          printf ("WARNING: In convert_routine(): body of function was skipped \n");
#endif
        }

  // DQ (1/28/2013): Set the source position of the end of the defining declaration to that from the end of the associated SgFunctionDefinition.
     ROSE_ASSERT(definingFunctionDeclaration->get_definition() != NULL);
     ROSE_ASSERT(definingFunctionDeclaration->get_definition()->get_endOfConstruct() != NULL);

#if 0
     printf ("Reset source position at end of defining function declaration from: \n");
     definingFunctionDeclaration->get_endOfConstruct()->display("BEFORE RESET: definingFunctionDeclaration->get_endOfConstruct(): debug");
     printf (" --- to: \n");
#endif

     *(definingFunctionDeclaration->get_endOfConstruct()) = *(definingFunctionDeclaration->get_definition()->get_endOfConstruct());

#if 0
     definingFunctionDeclaration->get_endOfConstruct()->display("AFTER RESET: definingFunctionDeclaration->get_endOfConstruct(): debug");
#endif

  // DQ (1/14/2020): For this to be compiler generated if it is a SgTemplateInstantiationDecl.
  // If we detect it's use in an instantiation directive then we can reverse this step.
     SgTemplateInstantiationFunctionDecl* templateInstantiationFunctionDecl = isSgTemplateInstantiationFunctionDecl(definingFunctionDeclaration);
     if (templateInstantiationFunctionDecl != NULL)
        {
          bool markAsSpecialization = (p->is_specialized == true);
          if (markAsSpecialization == false)
             {
            // This is a non specialization that should be marked as compiler generated.
               if (templateInstantiationFunctionDecl->isCompilerGenerated() == false)
                  {
                 // This function should be marked as compiler generated but has not been so force it to be marked as such.
                 // This is a fix for the last issue specific to Cxx11_tests/test2020_44.C (and the reduced size test2020_48.C).
#if 0
                    printf ("In convert_routine(): Force the SgTemplateInstantiationFunctionDecl to be compiler generated: name = %s \n",templateInstantiationFunctionDecl->get_name().str());
#endif
                    templateInstantiationFunctionDecl->setCompilerGenerated();
                  }
             }
        }

#if 0
     printf ("In convert_routine(): test 5: return_defining_declaration = %p = %s \n",return_defining_declaration,return_defining_declaration->class_name().c_str());
     return_defining_declaration->get_startOfConstruct()->display("return_defining_declaration: START: debug");
     return_defining_declaration->get_endOfConstruct()  ->display("return_defining_declaration: END: debug");
#endif

  // **********************************************************************
  // Fill in the constructor preinitialization list (after the symbols for 
  // the function paramters have been build in parse_function_body()).
     if (ctor != NULL)
        {
       // DQ (7/29/2014): Where we skip the function body because this is a C++11 defaulted or deleted
       // member function, we want to skip the handling of the constructor initializers as well.
       // convert_ctor_initializers(p,ctor);
          if (skip_body_of_cxx11_defaulted_or_deleted_member_function == false)
             {
               convert_ctor_initializers(p,ctor);
             }
        }
  // **********************************************************************

     ROSE_ASSERT(return_defining_declaration != NULL);

     if (nondefiningFunctionDeclaration != NULL)
        {
       // The nondefining declaration should not have a pointer to the associated defining declaration's function definition.
          ROSE_ASSERT(nondefiningFunctionDeclaration->get_definition() == NULL);

          ROSE_ASSERT(nondefiningFunctionDeclaration->get_startOfConstruct() != NULL);
          ROSE_ASSERT(nondefiningFunctionDeclaration->get_endOfConstruct() != NULL);
        }

     if (definingFunctionDeclaration != NULL)
        {
       // The defining declaration should have a valid pointer to the function definition.
          ROSE_ASSERT(definingFunctionDeclaration->get_definition() != NULL);
        }

  // DQ (1/18/2014): Adding support for old style function parameter declarations.
  // This support was noticed as missing when the source position info on function 
  // parameters because important in when we output the function parameters, since 
  // C allows function prototypes to be output without function parameters.
     ROSE_ASSERT(p != NULL);
     ROSE_ASSERT(p->type != NULL);
     ROSE_ASSERT(p->type->variant.routine.extra_info != NULL);
     if (p->type->variant.routine.extra_info->old_style_params_scanned == true)
        {
       // Mark this function as using old style c function parameters (declarations).
#if 0
          printf ("############################################################################################## \n");
          printf ("Mark this function as using old style C function parameters: definingFunctionDeclaration = %p \n",definingFunctionDeclaration);
          printf ("############################################################################################## \n");
#endif
          definingFunctionDeclaration->set_oldStyleDefinition(true);
        }

#if 0
     if (function_cache[p] == NULL)
        {
          displayFunctionCache("In convert_routine(): adding nondefiningFunctionDeclaration");

#error "DEAD CODE!"

          add_function_to_symbol_table_and_cache(p, nondefiningFunctionDeclaration);
        }
#else

  // DQ (12/17/2011): We need to avoid adding null pointers to the cache as part of tests.
  // if (nondefiningFunctionDeclaration == NULL)
  // if (function_cache[p] == NULL)
     if (function_cache.find(p) == function_cache.end())
        {
       // DQ (6/20/2009): Added assertion
       // ROSE_ASSERT(nondefiningFunctionDeclaration != NULL);
#if 0
          printf ("In convert_routine(): function_cache.find(p) == function_cache.end() \n");
#endif
          ROSE_ASSERT(nondefiningFunctionDeclaration != NULL);
          if (nondefiningFunctionDeclaration != NULL)
             {
            // This is the case for non-template functions and non-template member functions (and all template instantiations for functions and member functions).

               ROSE_ASSERT(nondefiningFunctionDeclaration->get_firstNondefiningDeclaration() != NULL);
               ROSE_ASSERT(nondefiningFunctionDeclaration->get_firstNondefiningDeclaration()->get_symbol_from_symbol_table() != NULL);

            // DQ (12/18/2011): test2004_35.C demonstrates a case there this is not true, I think this is correct that it is not always 
            // true if the use of a function appears before its declaration (defining or non-defining) (e.g. in a class definition).
            // ROSE_ASSERT(nondefiningFunctionDeclaration->get_firstNondefiningDeclaration() == nondefiningFunctionDeclaration);
            // ROSE_ASSERT(nondefiningFunctionDeclaration->get_symbol_from_symbol_table() != NULL);

            // DQ (12/11/2011): New test...(redundant with the one immediatedly above).
               ROSE_ASSERT(verifyExistenceOfSymbol(nondefiningFunctionDeclaration) == true);

               SgSymbol* symbol = nondefiningFunctionDeclaration->get_firstNondefiningDeclaration()->get_symbol_from_symbol_table();
               ROSE_ASSERT(symbol != NULL);

            // This should handle both normal functions and member functions
            // function_cache[p] = isSgFunctionSymbol(decl->get_symbol_from_symbol_table());
            // function_cache[p] = isSgSymbol(nondefiningFunctionDeclaration->get_symbol_from_symbol_table());
               function_cache[p] = symbol;
               ROSE_ASSERT(function_cache[p] != NULL);
             }
            else
             {
            // This is the case for template functions and template member functions.

            // DQ (12/4/2011): Not clear if we have to do something here or not!
            // ROSE_ASSERT(nondefiningTemplateFunctionDeclaration != NULL);
            // ROSE_ASSERT(definingTemplateFunctionDeclaration != NULL);
               ROSE_ASSERT(nondefiningTemplateFunctionDeclaration != NULL || nondefiningTemplateMemberFunctionDeclaration != NULL);
               ROSE_ASSERT(definingTemplateFunctionDeclaration    != NULL || definingTemplateMemberFunctionDeclaration    != NULL);

            // DQ (12/11/2011): New test...
               ROSE_ASSERT(verifyExistenceOfSymbol(nondefiningTemplateFunctionDeclaration) == true);

            // DQ (12/11/2011): We need to get the symbol from the first non-defining declaration (as recorded in the 
            // get_firstNondefiningDeclaration(), since it can be different from the variable nondefiningTemplateFunctionDeclaration).
            // This is an unfortunate choice of names local to this function.  They can be different because the variable
            // nondefiningTemplateFunctionDeclaration is set via the function_cache that is using a_routine_ptr type variables and there
            // can be many of these.
            // SgSymbol* symbol = nondefiningTemplateFunctionDeclaration->get_symbol_from_symbol_table();
               ROSE_ASSERT(nondefiningTemplateFunctionDeclaration->get_firstNondefiningDeclaration() != NULL);
               SgSymbol* symbol = nondefiningTemplateFunctionDeclaration->get_firstNondefiningDeclaration()->get_symbol_from_symbol_table();
               ROSE_ASSERT(symbol != NULL);
#if 0
               printf ("symbol = %p = %s \n",symbol,symbol->class_name().c_str());
#endif
            // DQ (12/11/2011): I think we have to use a different cache for template functions...
            // function_cache[p] = isSgSymbol(nondefiningTemplateFunctionDeclaration->get_symbol_from_symbol_table());
               function_cache[p] = symbol;
               ROSE_ASSERT(function_cache[p] != NULL);
             }

       // DQ (6/20/2009): Added assertion
          ROSE_ASSERT(function_cache[p] != NULL);
        }
       else
        {
#if 0
          printf ("In convert_routine(): function_cache.find(p) != function_cache.end() \n");
#endif
        }
#endif

     ROSE_ASSERT(function_cache[p] != NULL);

  // ROSE_ASSERT(return_defining_declaration->get_definition() != NULL);

  // DQ (11/27/2011): Adding tests for debugging test2011_162.C.
     ROSE_ASSERT(return_defining_declaration->get_scope() != NULL);
  // ROSE_ASSERT(isSgClassDefinition(defining_declaration->get_scope()) != NULL);

     test_for_correct_form_of_declaration(buildTemplateInstantiation,buildTemplateDeclaration,return_defining_declaration);

     if (buildTemplateInstantiation == true)
        {
#if 0
       // DQ (1/3/2012): Mark these to be output so that we can debug the unparsing of the template instantiations.
          first_nondefining_declaration->get_file_info()->setOutputInCodeGeneration();
          return_defining_declaration->get_file_info()->setOutputInCodeGeneration();
#endif
       // DQ (1/2/2012): We only want to mark template instantiations for output when we detect that there are transformations to them.
       // However, the helps to force them to be output which allows for debugging then (as if they had been detected as being transformed).
       // printf ("Mark this as a template specialization \n");
       // first_nondefining_declaration->set_specialization(SgDeclarationStatement::e_specialization);
       // return_defining_declaration->set_specialization(SgDeclarationStatement::e_specialization);
        }

#if 0
  // DQ (4/30/2012): I think this is relevant for test2012_51.C.
     if (p->is_template_function == true)
        {
          printf ("In convert_routine(): p->is_template_function == true (p = %p) \n",p);

          return_defining_declaration->get_file_info()->display("Leaving convert_routine(): return_defining_declaration");

       // Set both of the class declarations (defining and non-defining as being from a template.
       // This means that the class is a non-template class defined inside of a template (function 
       // or class template).
       // return_defining_declaration->set_from_template(true);
          return_defining_declaration->setCompilerGenerated();

          if (first_nondefining_declaration != NULL)
             {
            // first_nondefining_declaration->set_from_template(true);
               first_nondefining_declaration->setCompilerGenerated();
             }
        }
#else
  // printf ("Disabled mechanism to mark the function as compiler generated if it is from a template \n");
#endif


// #if 0
#if DEBUG_STRUCTURAL_DESIGN || DEBUG_CONVERT_ROUTINE || 0
     printf ("***********************************************************************************************\n");
     printf ("Leaving convert_routine(a_routine_ptr p = %p edg_template = %p name (without template arguments) = %s (with template arguments) = %s return_defining_declaration = %p = %s ) \n",
          p,edg_template,nameWithOutTemplateArguments.str(),nameWithTemplateArguments.str(),return_defining_declaration,return_defining_declaration->class_name().c_str());
     SgTemplateInstantiationFunctionDecl* templateInstantiationFunctionDeclaration = isSgTemplateInstantiationFunctionDecl(return_defining_declaration);
     if (templateInstantiationFunctionDeclaration != NULL)
        {
          printf ("   --- templateInstantiationFunctionDeclaration->get_templateDeclaration() = %p \n",templateInstantiationFunctionDeclaration->get_templateDeclaration());
        }
     SgTemplateFunctionDeclaration* templateFunctionDeclaration = isSgTemplateFunctionDeclaration(return_defining_declaration);
     if (templateFunctionDeclaration != NULL)
        {
          printf ("   --- templateFunctionDeclaration->get_templateParameters().size() = %" PRIuPTR " \n",templateFunctionDeclaration->get_templateParameters().size());
          SgTemplateFunctionDeclaration* firstNondefining_templateFunctionDeclaration = isSgTemplateFunctionDeclaration(return_defining_declaration->get_firstNondefiningDeclaration());
          printf ("   --- firstNondefining_templateFunctionDeclaration->get_templateParameters().size() = %" PRIuPTR " \n",firstNondefining_templateFunctionDeclaration->get_templateParameters().size());
        }
     SgTemplateMemberFunctionDeclaration* templateMemberFunctionDeclaration = isSgTemplateMemberFunctionDeclaration(return_defining_declaration);
     if (templateMemberFunctionDeclaration != NULL)
        {
          printf ("   --- templateMemberFunctionDeclaration->get_templateParameters().size() = %" PRIuPTR " \n",templateMemberFunctionDeclaration->get_templateParameters().size());
          SgTemplateMemberFunctionDeclaration* firstNondefining_templateMemberFunctionDeclaration = isSgTemplateMemberFunctionDeclaration(return_defining_declaration->get_firstNondefiningDeclaration());
          printf ("   --- firstNondefining_templateMemberFunctionDeclaration->get_templateParameters().size() = %" PRIuPTR " \n",firstNondefining_templateMemberFunctionDeclaration->get_templateParameters().size());
        }
  // DQ (11/23/2015): Added more detail for debugging friend function injection.
     SgScopeStatement* function_scope = return_defining_declaration->get_scope();
     ROSE_ASSERT(function_scope != NULL);
     printf ("   --- associated scope = %p = %s = %s \n",function_scope,function_scope->class_name().c_str(),SageInterface::generateUniqueName(function_scope,true).c_str());
     printf ("   --- return_defining_declaration->get_declarationModifier().isFriend() = %s \n",return_defining_declaration->get_declarationModifier().isFriend() ? "true" : "false");
     printf ("   --- return_defining_declaration->get_firstNondefiningDeclaration()    = %p \n",return_defining_declaration->get_firstNondefiningDeclaration());

     SgFunctionDeclaration* tmp_nondefiningDeclaration = isSgFunctionDeclaration(return_defining_declaration->get_firstNondefiningDeclaration());
     ROSE_ASSERT(tmp_nondefiningDeclaration != NULL);
     printf ("   --- tmp_nondefiningDeclaration->get_declarationModifier().isFriend() = %s \n",tmp_nondefiningDeclaration->get_declarationModifier().isFriend() ? "true" : "false");

     printf ("   --- return_defining_declaration->get_file_info()->isCompilerGenerated() = %s \n",return_defining_declaration->get_file_info()->isCompilerGenerated() ? "true" : "false");
     printf ("   --- tmp_nondefiningDeclaration->get_file_info()->isCompilerGenerated()  = %s \n",tmp_nondefiningDeclaration->get_file_info()->isCompilerGenerated() ? "true" : "false");
     printf ("***********************************************************************************************\n");
#endif
#if DEBUG_CLASSES_FROM_TEMPLATE_PARAMETERS || DEBUG_CONVERT_ROUTINE
     printf ("   --- return_defining_declaration->get_declarationScope           = %p \n",return_defining_declaration->get_declarationScope());
     printf ("   --- internal_intermediate_scope = %p \n",internal_intermediate_scope);
     size_t numberOfClassesBuiltFromTemplateParameters_2 = internal_intermediate_scope->symbol_table_size();
  // if (numberOfClassesBuiltFromTemplateParameters_2 > 0)
     printf ("   --- number of numberOfClassesBuiltFromTemplateParameters = %zu \n",numberOfClassesBuiltFromTemplateParameters_2);
#endif

     ROSE_ASSERT(definingFunctionDeclaration != NULL);
     ROSE_ASSERT(nondefiningFunctionDeclaration != NULL);

#if 0
     definingFunctionDeclaration   ->get_file_info()->display("Leaving convert_routine(): definingFunctionDeclaration: debug");

  // DQ (8/3/2012): At least for template declarations this will be marked as isOutputInCodeGeneration and isSourcePositionUnavailableInFrontend 
  // in the Sg_File_Info IR node.  I don't know if this is what we want.
     nondefiningFunctionDeclaration->get_file_info()->display("Leaving convert_routine(): nondefiningFunctionDeclaration: debug");
#endif


#if 0
     definingFunctionDeclaration->get_startOfConstruct()->display("Leaving convert_routine(): definingFunctionDeclaration: START: debug");
     definingFunctionDeclaration->get_endOfConstruct()  ->display("Leaving convert_routine(): definingFunctionDeclaration: END:   debug");
#endif

#if DEBUG_CONVERT_ROUTINE
     printf ("In convert_routine(): optional_nondefiningTemplateDeclaration = %p \n",optional_nondefiningTemplateDeclaration);
#endif

  // DQ (9/6/2014): We don't want to call this if we have forced this to be a template member or non-member function declaration.
  // DQ (8/10/2012): Refactored this code.
  // testAndMarkAsSpecialized(p, buildTemplateInstantiation, definingFunctionDeclaration, nondefiningFunctionDeclaration);
     if (optional_nondefiningTemplateDeclaration == NULL)
        {
#if defined(DEBUG_CONVERSION) || 0
          printf ("In convert_routine(): calling testAndMarkAsSpecialized() \n");
#endif
          testAndMarkAsSpecialized(p, buildTemplateInstantiation, definingFunctionDeclaration, nondefiningFunctionDeclaration);

#if defined(DEBUG_CONVERSION) || 0
          printf ("DONE: In convert_routine(): calling testAndMarkAsSpecialized() \n");
#endif
        }

  // DQ (8/3/2012): The non-defining declaration in the SgTemplateInstantiationDirectiveStatement is not the same
  // and the first non-defining declaration for the function.  This is a better design; but it make is 
  // difficult to know how to setup this defining declaration to not be output.  So this has to be setup 
  // in post-processing.
     if (isSgTemplateInstantiationDirectiveStatement(nondefiningFunctionDeclaration->get_parent()) != NULL)
        {
          printf ("This is associated with a template instantiation directive, mark as compiler generated and not to be output \n");
          ROSE_ASSERT(false);
        }

#if 0
  // DQ (8/3/2012): Output the sepecialization details...
     printf ("Leaving convert_routine(): return_defining_declaration->get_specialization()                                    = %s \n",outputSpecializationString(definingFunctionDeclaration->get_specialization()).c_str());
     printf ("Leaving convert_routine(): return_defining_declaration->get_firstNondefiningDeclaration()->get_specialization() = %s \n",outputSpecializationString(nondefiningFunctionDeclaration->get_specialization()).c_str());
#endif

     ROSE_ASSERT(return_defining_declaration->get_startOfConstruct() != NULL);
     ROSE_ASSERT(return_defining_declaration->get_endOfConstruct() != NULL);

  // DQ (7/25/2012): This is a refactoring of the support to copy the template arguments from the non-defining declaration to the defining declaration.
     if (isSgTemplateInstantiationFunctionDecl(return_defining_declaration) != NULL)
        {
          copyTemplateArgumentsFromNondefiningToDefiningDeclaration<SgTemplateInstantiationFunctionDecl>(return_defining_declaration);
        }
       else
        {
          if (isSgTemplateInstantiationMemberFunctionDecl(return_defining_declaration) != NULL)
             {
               copyTemplateArgumentsFromNondefiningToDefiningDeclaration<SgTemplateInstantiationMemberFunctionDecl>(return_defining_declaration);
             }
            else
             {
#if 0
               printf ("Not a template instantiation (so no template arguments to fixup) return_defining_declaration = %p = %s \n",return_defining_declaration,return_defining_declaration->class_name().c_str());
#endif
             }
        }

  // DQ (4/3/2012): Fixup test to reflect functions built outside of the class definition scope.
  // DQ (3/23/2012): Added test.
  // ROSE_ASSERT(p->defined_in_friend_decl == return_defining_declaration->get_declarationModifier().isFriend());
     ROSE_ASSERT(return_defining_declaration != NULL);
     if (isSgClassDefinition(topScopeStack()) != NULL)
        {
          ROSE_ASSERT(p->defined_in_friend_decl == return_defining_declaration->get_declarationModifier().isFriend());
        }
       else
        {
       // DQ (11/27/2015): This is no long true, since we set the defining declaration as a friend if
       // the original EDG normalized defining declaration was marked as friend.  This is superior to
       // passing such markings though the nondefining decaration (which caused test2013_178.C to fail).
       // ROSE_ASSERT(return_defining_declaration->get_declarationModifier().isFriend() == false);
        }

#if 0
     if (return_defining_declaration->get_firstNondefiningDeclaration() != NULL)
          ROSE_ASSERT(p->defined_in_friend_decl == return_defining_declaration->get_firstNondefiningDeclaration()->get_declarationModifier().isFriend());
     if (return_defining_declaration->get_definingDeclaration() != NULL)
          ROSE_ASSERT(p->defined_in_friend_decl == return_defining_declaration->get_definingDeclaration()->get_declarationModifier().isFriend());
#endif

#if DEBUG_CONVERT_ROUTINE
     printf ("Leaving convert_routine(): Calling testEdgRoseMaps() \n");
#endif

  // DQ (4/4/2012): Test the maps that we use internally (see function for documentation).
     testEdgRoseMaps();

     if (buildTemplateDeclaration == true)
        {
       // DQ (4/30/2012): This fails for test2012_56.C (friend template defined in templated class).
       // ROSE_ASSERT(edg_template != NULL);
          if (edg_template != NULL)
             {
            // DQ (4/10/2012): Added test to make sure this was added.
               ROSE_ASSERT( hasFirstNondefDeclForTemplate(edg_template) || hasDefiningDeclForTemplate(edg_template) );
             }
        }

  // DQ (4/27/2012): Added test to make sure this is present in the function cache.
     ROSE_ASSERT(function_cache.find(p) != function_cache.end());

#if MAKE_REDUNDANT_CALLS_TO_DETECT_TRANSFORAMTIONS
  // DQ (5/1/2012): Make sure that we don't have IR nodes marked as transformations.
     detectTransformations_local(return_defining_declaration);
#endif

  // DQ (2/26/2013): Adding support for exception handling specifications ("throw()" specification).
     if (p->type->kind == tk_routine && sup->exception_specification != NULL)
        {
       // DQ (10/4/2015): Added improved support for throw handling (only availabe in EDG versions 4.7 and later, I think).
          if (sup->exception_specification->is_noexcept == true)
             {
               if (sup->exception_specification->compiler_generated == true)
                  {
                 // In C++11 mode the absense of an explicit throw() is marked as a compiler generated noexcept() specification.
                  }
                 else
                  {
#if 0
                 // DQ (10/21/2015): This results in a large amount of output spew from C++11 example test codes.
                    printf ("Detected non-compiler generated noexcept (C++11) \n");
#endif
                  }
             }
            else
             {
               if (sup->exception_specification->compiler_generated == true)
                  {
#if 0
                 // DQ (10/21/2015): This results in a large amount of output spew from C++11 example test codes.
                    printf ("Detected compiler-generated throw() (I don't think these exist) \n");
                 // ROSE_ASSERT(false);
#endif
                  }
                 else
                  {
#if 0
                    printf ("Detected non-compiler generated throw \n");
#endif
#if 0
                    printf ("In convert_routine(): FOUND throw() exception specification: function name = %s \n",return_defining_declaration->get_name().str());
#endif
#if 0
                    printf ("In convert_routine(): checking on throw() usage: calling summarize_routine() \n");
                    summarize_routine(p);
                    printf ("DONE: In convert_routine(): checking on throw() usage: calling summarize_routine() \n\n");
#endif
                 // There is at least a simple throw specification.
                    return_defining_declaration->get_declarationModifier().setThrow();
#ifdef DEBUG_CONVERSION
                    printf ("In convert_routine(): FOUND throw() exception specification, but parameters and not filed in yet: function name = %s \n",return_defining_declaration->get_name().str());
#endif
#if 0
                    printf ("In convert_routine(): Exiting after having detected throw specification in function prototype: function name = %s \n",return_defining_declaration->get_name().str());
                    ROSE_ASSERT(false);
#endif
                  }
             }
        }

  // DQ (8/1/2014): Adding support for C++11 constexpr.
     if (p->is_declared_constexpr == true)
        {
#if DEBUG_CONVERT_ROUTINE
          printf ("In convert_routine(): Mark function as constexpr \n");
#endif
          return_defining_declaration->set_is_constexpr(true);

          if (p->is_constexpr == false)
             {
            // This is not an error, but is an interesting case that I would like to look at more closely (if there is a test code that demonstrates it).
#if DEBUG_CONVERT_ROUTINE
               printf ("In convert_routine(): detected case of (p->is_declared_constexpr == true) and (p->is_constexpr == false) \n");
#endif
             }
        }

#if 0
     printf ("Leaving convert_routine(): return_defining_declaration = %p \n",return_defining_declaration);     
     printf ("Exiting as a test! \n");
     ROSE_ASSERT(false);
#endif

#if 0
  // DQ (1/18/2014): Debugging use of optional function parameters in function protypes (C only, see test2014_29.c).
     ROSE_ASSERT(nondefiningFunctionDeclaration != NULL);
     SgInitializedNamePtrList::iterator i = nondefiningFunctionDeclaration->get_parameterList()->get_args().begin();
     while (i != nondefiningFunctionDeclaration->get_parameterList()->get_args().end())
        {
          (*i)->get_startOfConstruct()->display("In convert_routine(): nondefiningFunctionDeclaration parameter: startOfConstruct: debug");
          i++;
        }
     ROSE_ASSERT(definingFunctionDeclaration != NULL);
     SgInitializedNamePtrList::iterator j = definingFunctionDeclaration->get_parameterList()->get_args().begin();
     while (j != definingFunctionDeclaration->get_parameterList()->get_args().end())
        {
          (*j)->get_startOfConstruct()->display("In convert_routine(): definingFunctionDeclaration parameter: startOfConstruct: debug");
          j++;
        }
#endif

  // DQ (8/3/2014): Added to support C++11 new syntax for specification of return types.
     if (sup->trailing_return_type == true)
        {
#if 0
          printf ("In convert_routine(): detected C++11 new syntax for specification of return types \n");
#endif
          return_defining_declaration->set_using_new_function_return_type_syntax(true);
        }

  // DQ (8/11/2014): Added to support C++11 overridden keyword.
     if (p->override == true)
        {
#ifdef DEBUG_CONVERSION
          printf ("In convert_routine(): detected override keyword \n");
#endif
          return_defining_declaration->get_declarationModifier().setOverride();

#if 0
          printf ("Exiting as a test! \n");
          ROSE_ASSERT(false);
#endif
        }

  // DQ (8/11/2014): Added to support C++11 final keyword.
     if (p->final == true)
        {
#ifdef DEBUG_CONVERSION
          printf ("In convert_routine(): detected final keyword \n");
#endif
          return_defining_declaration->get_declarationModifier().setFinal();

#if 0
          printf ("Exiting as a test! \n");
          ROSE_ASSERT(false);
#endif
        }

#if 1
  // DQ (10/11/2015): This is a failing test in the AST consistancy testing, so we need code here to track this issue down.
     SgAccessModifier::access_modifier_enum tmp_definingDeclaration_access_modifier         = 
          return_defining_declaration->get_declarationModifier().get_accessModifier().get_modifier();
     SgAccessModifier::access_modifier_enum tmp_firstNondefiningDeclaration_access_modifier = 
          return_defining_declaration->get_firstNondefiningDeclaration()->get_declarationModifier().get_accessModifier().get_modifier();
     if (tmp_definingDeclaration_access_modifier != tmp_firstNondefiningDeclaration_access_modifier)
        {
          printf ("Error: SgAccessModifier::access_modifier_enum's don't match between the defining and non-defining declarations \n");
        }
     ROSE_ASSERT(tmp_definingDeclaration_access_modifier == tmp_firstNondefiningDeclaration_access_modifier);
#endif

#if 0
     printf ("In convert_routine(): return_defining_declaration->get_firstNondefiningDeclaration() = %p = %s \n",
          return_defining_declaration->get_firstNondefiningDeclaration(),return_defining_declaration->get_firstNondefiningDeclaration()->class_name().c_str());
     return_defining_declaration->get_firstNondefiningDeclaration()->get_startOfConstruct()->display("return_defining_declaration->get_firstNondefiningDeclaration(): START: debug");
     return_defining_declaration->get_firstNondefiningDeclaration()->get_endOfConstruct  ()->display("return_defining_declaration->get_firstNondefiningDeclaration(): END: debug");
#endif

#if 0
     printf ("Leaving convert_routine(): test 6: return_defining_declaration = %p = %s \n",return_defining_declaration,return_defining_declaration->class_name().c_str());
     return_defining_declaration->get_startOfConstruct()->display("return_defining_declaration: START: debug");
     return_defining_declaration->get_endOfConstruct()  ->display("return_defining_declaration: END: debug");
#endif


  // DQ (2/13/2016): Make sure that the function scope has been processed into the scope cache.
     ROSE_ASSERT(global_innermost_function_scope != NULL);
     ROSE_ASSERT(scope_cache.find(global_innermost_function_scope) != scope_cache.end());
     ROSE_ASSERT(scope_cache[global_innermost_function_scope] != NULL);

#if 0
  // DQ (11/12/2020): Unclear if this is set if it should be unset as a result of routine_name_linkage_is_explicit (above).
     printf ("In convert_routine(): return_defining_declaration->isExternBrace()                                            = %s \n",
          return_defining_declaration->isExternBrace() ? "true" : "false");
     printf ("In convert_routine(): return_defining_declaration->get_declarationModifier().get_storageModifier().isExtern() = %s \n",
          return_defining_declaration->get_declarationModifier().get_storageModifier().isExtern() ? "true" : "false");
#endif
#if 0
     bool resetStrorageModifier = ( ( (isSgFunctionDeclaration(result) != NULL) || (p->friend_decl == true) ) && (result->get_declarationModifier().get_storageModifier().isExtern() == true) );
     printf ("In convert_routine(): (p->explicit_storage_class == false): resetStrorageModifier = %s \n",resetStrorageModifier ? "true" : "false");
#endif
#if 0
     printf ("In convert_routine(): routine_name_linkage_is_explicit                   = %s \n",routine_name_linkage_is_explicit ? "true" : "false");
#endif

  // DQ (11/12/2020): Set the extern "C" linkage if required.
     if (routine_name_linkage_is_explicit == true)
        {
          set_linkage(p, return_defining_declaration);
        }

#if DEBUG_CONVERT_ROUTINE || 0
     printf ("Leaving convert_routine(): %p \n", p);
#endif
     if (unstrippedFunctionTypeUndeduced)
     {
       SgFunctionType* syntaxTy = isSgFunctionType(unstrippedFunctionTypeUndeduced->stripType());

       return_defining_declaration->set_type_syntax(syntaxTy);
       return_defining_declaration->set_type_syntax_is_available(syntaxTy != nullptr);
     }

     return return_defining_declaration;
   }

