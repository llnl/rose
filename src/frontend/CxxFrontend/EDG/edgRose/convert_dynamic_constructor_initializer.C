
#include "edgRose.h"

namespace EDG_ROSE_Translation {

SgExpression * convert_dynamic_constructor_initializer(a_dynamic_init_ptr dip, a_type_ptr edgType)
   {
  // This function supports the case of "dik_constructor" in convert_dynamic_initializer().

  // DQ (12/11/2011): FIXME: This function is not laid out well (the special case of (ctor == NULL) which has 
  // it's own return is not a great design. And then the subcase of that which gets the edgType from the  
  // variable is not well designed either.  So this function is a canidate for being refactored (at least). 
  // Note that I am still discovering how complex this function can be from its use in the new template support 
  // in the AST.  So part of the reason why we have the current code is that I am still learning how complex
  // the template handling needs to be...

  // Initial value of a simple object is established by a constructor call, 
  // not a function (as in a defined constructor). Example: "A objectA1;"

#ifdef DEBUG_CONVERSION
     indent_line(); debug_print("%sdynamic constructor initializer\n", (dip == NULL ? "null " : ""));
#endif

     ROSE_ASSERT(dip != NULL);

  // DQ (12/11/2011): This is not alwasy true (fails for test2001_01.C).
  // ROSE_ASSERT(edgType != NULL);

#if 0
     printf ("In convert_dynamic_constructor_initializer(): Calling summarize_dynamic_init(dip = %p) \n",dip);
     summarize_dynamic_init(dip);
#endif

#if 0
     printf ("Exiting as a test! \n");
     ROSE_ASSERT(false);
#endif

     ROSE_ASSERT(dip->kind == dik_constructor);

  // DQ (12/10/2011): Added new test. (fails for test2001_01.C)
  // ROSE_ASSERT(dip->is_constructor_init == true);

  // If there is a function that is a constructor, then this should be a valid pointer.
     a_routine_ptr    ctor = dip->variant.constructor.ptr;

  // If there are no arguments then this will be a NULL pointer.
     an_expr_node_ptr args = dip->variant.constructor.args;

     a_variable_ptr edg_variable = dip->variable;

#if 0
     printf ("In convert_dynamic_constructor_initializer(): ctor = %p args = %p edg_variable = %p \n",ctor,args,edg_variable);
#endif

#if 0
  // DQ (2/7/2016): Adding support for C++11 initialization lists.
     if (dip->is_creation_of_initializer_list_object == true)
        {
          printf ("WARNING: Detected use of C++11 initialization lists (now implemented) \n");
        }
#endif

  // DQ (6/27/2013): Build this here so that it can be set where the constructor member function is conditionally built below. 
     SgFunctionDeclaration* ctorFunction = NULL;

  // DQ (12/10/2011): ctor == NULL for test2004_35.C.
     if (ctor == NULL)
        {
       // This is likely a primative type without a valid constructor member function.
#if 0
          printf ("In convert_dynamic_constructor_initializer(): ctor == NULL \n");
#endif
#if 0
          printf ("In convert_dynamic_constructor_initializer(): This is likely a primative type without a valid constructor member function \n");
#endif
       // I think that this case has to set "associated_class_unknown == true" and use zero as a default value?
       // Not clear how to gether the value that we need to use.

       // Debug this in the morning...

       // I think this is true in this case...(fails test2004_35.C)
       // ROSE_ASSERT(dip->is_constructor_init == true);

       // Generate an empty constructor argument list
          SgExprListExp* expressionList = set_generated_file_info_to_unparse(buildExprListExp_nfi());

       // Add any available arguments to the constructor argument list
          for (an_expr_node_ptr arg = args; arg != NULL; arg = arg->next)
             {
            // DQ (3/23/2012): We only want to support the actual arguments in the AST (I think).
            // An analysis might however want to see all arguments. Thus this could be a normalization
            // to provide all arguments instead of just non-default arguments.
#if 0
               printf ("In convert_dynamic_constructor_initializer(): (part 1) arg->generated_default_arg = %s \n",arg->generated_default_arg ? "true" : "false");
#endif
               if (arg->generated_default_arg == false)
                  {
                    SgExpression* expression = convert_expression(arg);
#if 0
                    printf ("In convert_dynamic_constructor_initializer(): expression in expresion list = %p = %s \n",expression,expression->class_name().c_str());
#endif
                    appendExpression(expressionList, expression);
                  }
             }

       // This (edgType == NULL) is demonstrated by test2004_35.C.
          if (edgType == NULL)
             {
            // ROSE_ASSERT(edg_variable != NULL);
               if (edg_variable != NULL)
                  {
                    edgType = edg_variable->declared_type ? edg_variable->declared_type : edg_variable->type;
                    ROSE_ASSERT(edgType != NULL);
                  }

               if (edgType == NULL)
                 {
                   printf ("Error: In convert_dynamic_constructor_initializer(): edgType input is NULL, and cannot be computed (or is not sucessfully computed yet) \n");

                   printf ("In convert_dynamic_constructor_initializer(): Calling summarize_dynamic_init(dip = %p) \n",dip);
                   summarize_dynamic_init(dip);
                 }
             }

       // ROSE_ASSERT(edgType != NULL);
          SgType* type = NULL;
          if (edgType != NULL)
             {
#if 0
               printf ("In convert_dynamic_constructor_initializer(): calling convert_type(edgType = %p) \n",edgType);
#endif
               type = convert_type(edgType);
               ROSE_ASSERT (type != NULL);
             }

          bool need_name                   = false;
          bool need_qualifier              = false;
          bool need_parenthesis_after_name = false;

          bool associated_class_unknown    = true;

       // DQ (2/22/2012): We need this but it fails for test2012_07.C
          ROSE_ASSERT (type != NULL);

       // SgConstructorInitializer* constructorInitializer = buildConstructorInitializer(memberFunction,expressionList,constructorType,need_name,need_qualifier,need_parenthesis_after_name,associated_class_unknown);
          SgConstructorInitializer* constructorInitializer = buildConstructorInitializer(NULL,expressionList,type,need_name,need_qualifier,need_parenthesis_after_name,associated_class_unknown);
          ROSE_ASSERT(constructorInitializer != NULL);
#if 0
          printf ("After call to buildConstructorInitializer() 3: constructorInitializer = %p \n",constructorInitializer);
#endif
       // DQ (2/12/2012): Set the source code position info.
          ROSE_ASSERT(constructorInitializer->get_args()->get_startOfConstruct() != NULL);

       // ROSE_ASSERT(false);
#if 0
          printf ("Leaving convert_dynamic_constructor_initializer(): where building SgConstructorInitializer \n");
#endif
          return constructorInitializer;
        }
       else
        {
#if 0
          printf ("In convert_dynamic_constructor_initializer(): ctor != NULL \n");
#endif
#if 0
          printf ("In convert_dynamic_constructor_initializer(): Calling summarize_routine(ctor = %p) \n",ctor);
          summarize_routine(ctor);
#endif

       // DQ (8/25/2014): Need to look in both the first_nondefining_declarations_for_functions AND the first_nondefining_declarations_for_template_functions 
       // caches.  See test2014_161.C for an example where this is critical.
       // DQ (3/14/2012): Make sure this has not be built previously.
       // ROSE_ASSERT(first_nondefining_declarations_for_functions.find(ctor) == first_nondefining_declarations_for_functions.end());

          if (first_nondefining_declarations_for_functions.find(ctor) == first_nondefining_declarations_for_functions.end())
             {
            // DQ (8/25/2014): Note that test2014_161.C is an example of this case.
#if 0
               printf ("In convert_dynamic_constructor_initializer(): ctor was NOT found in cache \n");
#endif
            // DQ (8/25/2014): Test for a possible template or a way to find the template instantiation.
               if (ctor->assoc_template != NULL)
                  {
#if 0
                    printf ("In convert_dynamic_constructor_initializer(): found a valid p->assoc_template = %p \n",ctor->assoc_template);
#endif
                 // DQ (8/25/2014): Commented out to revert to previous working state.
                 // DQ (8/25/2014): It might be that we should have looked here!
                 // ROSE_ASSERT(first_nondefining_declarations_for_template_functions.find(ctor) == first_nondefining_declarations_for_template_functions.end());
                 // ROSE_ASSERT(defining_declarations_for_template_functions.find(ctor) == defining_declarations_for_template_functions.end());
                  }

            // DQ (3/14/2012): Build an explicit function prototype for the implicit constructor (since it is called (referenced) and so we require it).
            // SgFunctionDeclaration* functionDeclaration = convert_routine_prototype(ctor,false,false);
               SgDeclarationStatement* decl_stmt = NULL;
               if (ctor->assoc_template && ctor->assoc_template->source_corresp.parent_scope == ctor->source_corresp.parent_scope) {
                 decl_stmt = convert_template_prototype(ctor->assoc_template);
               } else {
                 decl_stmt = convert_routine_prototype(ctor,false);
               }
               SgFunctionDeclaration* functionDeclaration = isSgFunctionDeclaration(decl_stmt);
               ROSE_ASSERT(functionDeclaration != NULL);

            // DQ (6/27/2013): Set the ctorFunction so that it can be used below.
               ctorFunction = functionDeclaration;
#if 0
            // DQ (6/27/2013): Added assertion.
               if ( first_nondefining_declarations_for_functions.find(ctor) == first_nondefining_declarations_for_functions.end() )
                  {
                    printf ("ERROR: failing test ofr building ctor = %p \n",ctor);
#if 0
                    printf ("ERROR: In convert_dynamic_constructor_initializer(): Calling summarize_routine(ctor = %p) \n",ctor);
                    summarize_routine(ctor);
#endif
                  }
               ROSE_ASSERT( first_nondefining_declarations_for_functions.find(ctor) != first_nondefining_declarations_for_functions.end() );
#endif
#if 0
               printf ("In convert_dynamic_constructor_initializer(): (after convert_routine_prototype()) functionDeclaration = %p = %s \n",functionDeclaration,functionDeclaration->class_name().c_str());
#endif
            // DQ (4/24/2012): Added test.
            // ROSE_ASSERT(first_nondefining_declarations_for_functions.find(ctor) != first_nondefining_declarations_for_functions.end());
               if ( !(first_nondefining_declarations_for_functions.find(ctor) != first_nondefining_declarations_for_functions.end()) &&
                    !(ctor->assoc_template != NULL && hasFirstNondefDeclForTemplate(ctor->assoc_template)) )
                  {
#ifdef DEBUG_CONVERSION
                    printf ("In convert_dynamic_constructor_initializer(): functionDeclaration = %p = %s name = %s \n",functionDeclaration,functionDeclaration->class_name().c_str(),functionDeclaration->get_name().str());
                    printf ("In convert_dynamic_constructor_initializer(): functionDeclaration->get_firstNondefiningDeclaration() = %p \n",functionDeclaration->get_firstNondefiningDeclaration());
                    printf ("In convert_dynamic_constructor_initializer(): functionDeclaration->get_definingDeclaration()         = %p \n",functionDeclaration->get_definingDeclaration());
#endif
#ifdef DEBUG_CONVERSION
                    printf ("ERROR: In convert_dynamic_constructor_initializer(): Calling summarize_dynamic_init(dip = %p) \n",dip);
                    summarize_dynamic_init(dip);
#endif
#if 0
                    printf ("ERROR: In convert_dynamic_constructor_initializer(): Calling summarize_routine(ctor = %p) \n",ctor);
                    summarize_routine(ctor);
#endif
                  }

            // DQ (6/27/2013): Modified to support special case of ctor->assoc_template == NULL for SgTemplateMemberFunctionDeclaration IR nodes.
            // I think the template is often unavailable or was part of a child class in another parent template.
            // ROSE_ASSERT( (first_nondefining_declarations_for_functions.find(ctor) != first_nondefining_declarations_for_functions.end()) ||
            //              (ctor->assoc_template != NULL && first_nondefining_declarations_for_templates.find(ctor->assoc_template) != first_nondefining_declarations_for_templates.end()) );
               ROSE_ASSERT( (first_nondefining_declarations_for_functions.find(ctor) != first_nondefining_declarations_for_functions.end()) ||
                            (ctor->assoc_template != NULL && hasFirstNondefDeclForTemplate(ctor->assoc_template)) || 
                            (ctor->assoc_template == NULL && isSgTemplateMemberFunctionDeclaration(functionDeclaration) != NULL) );

               SgMemberFunctionDeclaration* memberFunctionDeclaration = isSgMemberFunctionDeclaration(functionDeclaration);
               ROSE_ASSERT(memberFunctionDeclaration != NULL);

            // Check that this is a constructor.
               ROSE_ASSERT(memberFunctionDeclaration->get_specialFunctionModifier().isConstructor() == true);
             }
#if 0
       // DQ (8/25/2014): Commented out to revert to previous working state.
            else
             {
#if 0
               printf ("In convert_dynamic_constructor_initializer(): ctor WAS found in cache \n");
#endif

               if (first_nondefining_declarations_for_functions.find(ctor) != first_nondefining_declarations_for_functions.end())
                  {
#if 0
                    printf ("In convert_dynamic_constructor_initializer(): ctor WAS found in non-template instantiation cache: ctorFunction = %p = %s \n",ctorFunction,ctorFunction->class_name().c_str());
#endif
                    ctorFunction = first_nondefining_declarations_for_functions[ctor];
                  }
                 else
                  {
                    if (first_nondefining_declarations_for_template_functions.find(ctor) != first_nondefining_declarations_for_template_functions.end())
                       {
                         ctorFunction = first_nondefining_declarations_for_template_functions[ctor];
#if 0
                         printf ("In convert_dynamic_constructor_initializer(): ctor WAS found in template instantiation cache: ctorFunction = %p = %s \n",ctorFunction,ctorFunction->class_name().c_str());
#endif
                       }
                      else
                       {
                         printf ("ERROR: could not find ctor in either function and template function caches \n");
                         ROSE_ASSERT(false);
                       }
                  }

               ROSE_ASSERT(ctorFunction != NULL);
             }

          ROSE_ASSERT(ctorFunction != NULL);
#endif
        }
       
     ROSE_ASSERT(ctor != NULL);
     a_type_ptr type = ctor->declared_type ? ctor->declared_type : ctor->type;
     ROSE_ASSERT(type != NULL);

  // Values associated with SgConstructorInitializer construction: bool need_name, bool need_qualifier, bool need_parenthesis_after_name, bool associated_class_unknown
     bool need_name                   = false;
     bool need_qualifier              = false;

  // For the initial case in test2001_01.C assume this is false.
     bool need_parenthesis_after_name = false;
     if (dip->variable != NULL)
        {
          need_parenthesis_after_name = dip->variable->has_parenthesized_initializer;
        }

     bool associated_class_unknown    = false;

#if 0
  // Call the EDG function for this!
     bool default_init = is_default_dynamic_init(dip);
     printf ("Building a pointer to the member function declaration default_init = %s args = %p need_parenthesis_after_name = %s \n",
          default_init ? "true" : "false",args,need_parenthesis_after_name ? "true" : "false");
#endif

  // DQ (3/14/2012): This is the case of an implicit (default) constructor being called, see test2004_89.C.
  // DQ (12/30/2011): Put back so we can debug this in the morning...
  // DQ (12/30/2011): If it is not in the list of functions then it might be in the list if templates.
  // Which makes the point that we should unify these lists under the new design of template declaration support.
  // Make sure that ctor is in the list...
     if (first_nondefining_declarations_for_functions.find(ctor) == first_nondefining_declarations_for_functions.end())
        {
       // ROSE_ASSERT(first_nondefining_declarations_for_templates.find(ctor) != first_nondefining_declarations_for_templates.end());
#if 0
          printf ("############# Check for the template version of the constructor functions ctor = %p \n",ctor);
          displayMap(first_nondefining_declarations_for_functions,"Check for the template version of the constructor functions");
#endif

       // This function has not been seen before (likely it is part of a template instantiation for which the class definition 
       // has not been read (is compiler instantiated and not in the source code).  So we have to force the class definition 
       // to be converted into the AST.
#ifdef DEBUG_CONVERSION
#if TEMPLATE_INSTANTIATIONS_IN_SOURCE_SEQUENCE_LISTS
          printf ("TEMPLATE_INSTANTIATIONS_IN_SOURCE_SEQUENCE_LISTS == true \n");
#else
          printf ("TEMPLATE_INSTANTIATIONS_IN_SOURCE_SEQUENCE_LISTS == false \n");
#endif
#endif
       // DQ (3/14/2012): This is the case of an implicit (default) constructor being called, see test2004_89.C.
       // the solution is to add a whole new constructo to the class for which the implicit constructor is being 
       // called.

#if 0
          printf ("In convert_dynamic_constructor_initializer(): Calling summarize_routine(dip = %p) \n",dip);
          summarize_routine(ctor);
#endif
       // Note that the function is likely in the template map:

       // DQ (6/27/2013): This happens rarely and there is nothing to do about it.
          if (ctor->assoc_template == NULL)
             {
               mprintf ("(ctor->assoc_template == NULL): This is allowed for cases where the template is a part of a parent template  (happens in ROSE compiling test2013_241.C) \n");
             }
            else
             {
               ROSE_ASSERT(hasFirstNondefDeclForTemplate(ctor->assoc_template));
             }
        }

  // DQ (4/24/2012): Note that the ctor could be a template function declaration, so we have to look in the template map.
     ROSE_ASSERT( (first_nondefining_declarations_for_functions.find(ctor) != first_nondefining_declarations_for_functions.end()) ||
                  (ctor->assoc_template != NULL && hasFirstNondefDeclForTemplate(ctor->assoc_template)) || 
                  (ctor->assoc_template == NULL && ctorFunction != NULL) );

  // DQ (8/25/2014): Un-Commented out to revert to previous working state.
     if (first_nondefining_declarations_for_functions.find(ctor) != first_nondefining_declarations_for_functions.end())
        {
          ctorFunction = first_nondefining_declarations_for_functions[ctor];
          ROSE_ASSERT(ctorFunction != NULL);
        }
       else
        {
       // DQ (6/27/2013): Added assertion.
          ROSE_ASSERT(ctorFunction != NULL);
        }

  // DQ (6/27/2013): The ctorFunction was not found above (we could have saved it where it was conditionally build above).
  // DQ (5/26/2013): Moved this to before the "if (ctorFunction == NULL)", and commented that code out.
#if 0
     if (ctorFunction == NULL)
        {
          printf ("(ctorFunction == NULL): Can't access the ctorFunction (though it was conditionally build above) \n");
        }
#endif
     ROSE_ASSERT(ctorFunction != NULL);

#if 0
  // We need to handle this case.
     if (ctorFunction == NULL)
        {
       // This constructor has not been seen yet. So we will have to build the constructor for it.
          printf ("This constructor has not been seen yet. So we will have to build the constructor for it. \n");

          if (ctor->is_template_function == true)
             {
               ROSE_ASSERT(ctor->compiler_generated == false);
             }
            else
             {
             }

          ROSE_ASSERT(ctorFunction != NULL);
        }
#endif

     ROSE_ASSERT(ctorFunction != NULL);

#if 0
  // DQ (5/26/2013): if the template basic_string is using the name "string" then I think that 
  // routine_name_linkage_is_explicit == false and then dip->is_explicit_cast == true.
  // Think that this is what it means for these to be false and true respectively.
     bool routine_name_linkage_is_explicit = ctor->type->variant.routine.extra_info->routine_name_linkage_is_explicit;
     printf ("In convert_dynamic_constructor_initializer(): routine_name_linkage_is_explicit = %s \n",routine_name_linkage_is_explicit ? "true" : "false");
     printf ("In convert_dynamic_constructor_initializer(): dip->is_explicit_cast            = %s \n",dip->is_explicit_cast       ? "true" : "false");
#endif

  // Values previously passed in from outside function.
     bool force_parens       = false;
     bool parenthesized_init = true;

  // DQ (3/27/2012): Set this to false for now...so we can handle the more general case first.
     bool is_condition = false;

  // Use the parenthesized initialization form, e.g., A x(y);
  // for classes with constructors, and the "=" form, e.g., A x = y;
  // Otherwise.  Conditions allow only the "=" form. */
     if ( (is_condition == false) &&
          (dip->kind == (a_dynamic_init_kind)dik_constructor ||
           dip->kind == (a_dynamic_init_kind)dik_zero ||
           dip->kind == (a_dynamic_init_kind)dik_none))
        {
          parenthesized_init = true;
        }
       else
        {
          parenthesized_init = false;
        }

#if 0
     printf ("parenthesized initialization: parenthesized_init = %s \n",parenthesized_init ? "true" : "false");
#endif

  // Refactored cases:
  // need_name                   = dip->is_explicit_cast == true;
     need_name                   = false;
     need_qualifier              = false;
     need_parenthesis_after_name = true;
     associated_class_unknown    = false;

  // If parenthesized_init is TRUE, put parentheses around the initializer; this is the parenthesized form of initialization, e.g., A x(y);
  // if (!parenthesized_init)
     if (parenthesized_init == false)
        {
       // This is the copy constructor elision case -- we don't have to write the 
       // copy constructor because it's implied.  Just write the source argument.
#if 0
          printf ("This is the copy constructor elision case \n");
#endif
#if 0
          need_name                   = false;
          need_qualifier              = false;
          need_parenthesis_after_name = true;
          associated_class_unknown    = false;

          result->set_is_explicit_cast(dip->is_explicit_cast);
          isSgConstructorInitializer(result)->set_need_name(true);
          isSgConstructorInitializer(result)->set_need_qualifier(true);
#endif
       // Refactored cases:
          need_name                   = true;
          need_qualifier              = true;

       // DQ (3/27/2012): Added this setting.
          need_parenthesis_after_name = true;
        }
       else
        {
       // This is the non-elision case.
#if 0
          printf ("This is the non-elision case \n");
#endif
          if ( (parenthesized_init == true) && (args == NULL || args->generated_default_arg) )
             {
            // This is a default constructor, so do not list the
            // initialization except when specifically asked to.
#if 0
               printf ("This is a default constructor non-elision case \n");
#endif
#if 0
               need_name                   = false;
               need_qualifier              = false;
               need_parenthesis_after_name = force_parens;
               associated_class_unknown    = false;

               result->set_is_explicit_cast(dip->is_explicit_cast);
#endif
            // Refactored cases:
               need_parenthesis_after_name = force_parens;

            // DQ (3/27/2012): Added this setting.
               need_name                   = dip->is_explicit_cast == true;
             }
            else
             {
#if 0
               printf ("Non-copy constructor non-elision case! \n");
#endif
#if 0
               need_name                   = dip->is_explicit_cast == true;
               need_qualifier              = false;
               need_parenthesis_after_name = true;
               associated_class_unknown    = false;

            // Added support for specification of explicit vs. implicit constructor initializers
               result->set_is_explicit_cast(dip->is_explicit_cast);
#endif
            // Refactored cases:
               need_name                   = (dip->is_explicit_cast == true);

            // DQ (3/27/2012): Added this setting.
               need_parenthesis_after_name = true;
             }
        }

     SgMemberFunctionDeclaration* memberFunction = isSgMemberFunctionDeclaration(ctorFunction);
     ROSE_ASSERT(memberFunction != NULL);
#if 0
     printf ("memberFunction = %p (%s) \n",memberFunction,memberFunction->class_name().c_str());
#endif

     ROSE_ASSERT(memberFunction != memberFunction->get_definingDeclaration());

  // DQ (1/4/2009): Error checking
     ROSE_ASSERT(memberFunction->get_associatedClassDeclaration() != NULL);

  // Generate an empty constructor argument list
     SgExprListExp* expressionList = set_generated_file_info_to_unparse(buildExprListExp_nfi());

  // Add any available arguments to the constructor argument list
     for (an_expr_node_ptr arg = args; arg != NULL; arg = arg->next)
        {
       // DQ (3/23/2012): We only want to support the actual arguments in the AST (I think).
       // An analysis might however want to see all arguments. Thus this could be a normalization
       // to provide all arguments instead of just non-default arguments.
#if 0
          printf ("In convert_dynamic_constructor_initializer(): (part 2) arg->generated_default_arg = %s \n",arg->generated_default_arg ? "true" : "false");
#endif
          if (arg->generated_default_arg == false)
             {
               SgExpression* expression = convert_expression(arg);
#if 0
               printf ("In convert_dynamic_constructor_initializer(): expression in expresion list = %p = %s \n",expression,expression->class_name().c_str());
#endif
               appendExpression(expressionList, expression);
             }
        }

  // Build the type, might not be a SgClassType in the case of vacuous constructors!
     SgType* constructorType = convert_type(type);
     ROSE_ASSERT(constructorType != NULL);

  // DQ (3/26/2012): It is a problem in test2012_48.C if this is a SgMemberFunctionType, it should be a SgClassType instead.
     SgType* classType = isSgClassType(constructorType);
     if (classType == NULL)
        {
          SgMemberFunctionType* memberFunctionType = isSgMemberFunctionType(constructorType);
          if (memberFunctionType != NULL)
             {
               ROSE_ASSERT(memberFunctionType->get_class_type() != NULL);
               if (memberFunctionType->get_class_type() != NULL)
                  {
                    classType = isSgClassType(memberFunctionType->get_class_type());
                  }
                 else
                  {
                    associated_class_unknown = true;
                  }
             }
            else
             {
               ROSE_ASSERT(isSgFunctionType(constructorType) != NULL);
            // printf ("Unclear how to build the class type constructorType = %p = %s \n",constructorType,constructorType->class_name().c_str());
            // ROSE_ASSERT(false);

               classType = constructorType;
               
             }
        }

     ROSE_ASSERT(classType != NULL || isSgFunctionType(constructorType) != NULL);

  // Build the SgConstructorInitializer
     SgConstructorInitializer* constructorInitializer = buildConstructorInitializer(memberFunction,expressionList,classType,need_name,need_qualifier,need_parenthesis_after_name,associated_class_unknown);

     ROSE_ASSERT(constructorInitializer != NULL);

#if 0
     printf ("In convert_dynamic_constructor_initializer(): After call to buildConstructorInitializer_nfi() 4: constructorInitializer = %p \n",constructorInitializer);
     printf ("constructorInitializer->get_args() = %p \n",constructorInitializer->get_args());
     printf ("memberFunction->get_name() = %s \n",memberFunction->get_name().str());
#endif

     if (constructorInitializer->get_args() != NULL)
        {
       // printf ("constructorInitializer->get_args()->get_file_info() = %p \n",constructorInitializer->get_args()->get_file_info());
          ROSE_ASSERT(constructorInitializer->get_args()->get_file_info() != NULL);
        }

#if 0
     printf ("In convert_dynamic_constructor_initializer(): dip->is_explicit_cast                          = %s \n",dip->is_explicit_cast       ? "true" : "false");
     printf ("In convert_dynamic_constructor_initializer(): need_name                                      = %s \n",need_name                   ? "true" : "false");
     printf ("In convert_dynamic_constructor_initializer(): need_qualifier                                 = %s \n",need_qualifier              ? "true" : "false");
     printf ("In convert_dynamic_constructor_initializer(): need_parenthesis_after_name                    = %s \n",need_parenthesis_after_name ? "true" : "false");
     printf ("In convert_dynamic_constructor_initializer(): associated_class_unknown                       = %s \n",associated_class_unknown    ? "true" : "false");
     printf ("In convert_dynamic_constructor_initializer(): constructorInitializer->get_need_name()        = %s \n",constructorInitializer->get_need_name() ? "true" : "false");
     printf ("In convert_dynamic_constructor_initializer(): constructorInitializer->get_is_explicit_cast() = %s \n",constructorInitializer->get_is_explicit_cast() ? "true" : "false");
#endif

  // DQ (11/6/2014): Adding C++11 support to record the use of C++11 syntax (for the unparser).
     if (edg_variable != NULL && edg_variable->has_direct_braced_initializer == true)
        {
#if 0
       // DQ (3/6/2017): Converted to use message logging.
          mprintf ("In convert_dynamic_constructor_initializer(): Need to mark as using C++11 braced initializer syntax \n");
       // ROSE_ASSERT(false);
#endif
          constructorInitializer->set_is_braced_initialized(true);
        }

  // DQ (2/7/2016): Adding support for C++11 initialization lists.
     if (dip->is_creation_of_initializer_list_object == true)
        {
       // Not clear how to handle this case!
#if 0
          printf ("In convert_dynamic_constructor_initializer(): returning constructorInitializer = %p = %s \n",constructorInitializer,constructorInitializer->class_name().c_str());
#endif
        }

     return constructorInitializer;
   }
}