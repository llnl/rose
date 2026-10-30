
#include "edgRose.h"

SgInitializedName* EDG_ROSE_Translation::convert_function_parameter(a_param_type_ptr p, bool skipDefaultArguments, SgDeclarationScope* & declarationScope)
   {
     ROSE_ASSERT(p != NULL);

#ifdef DEBUG_CONVERSION
     indent_line(); debug_print("function param skipDefaultArguments = %s ",skipDefaultArguments ? "true" : "false"); print_source_range(p->decl_pos_info ? p->decl_pos_info->identifier_range : null_source_range); debug_print("\n");
     ScopedIndent si(indent);
#endif

#if 0
     printf ("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! \n");
     printf ("convert_function_parameter(p = %p,skipDefaultArguments = %s,declarationScope = %p \n",p,skipDefaultArguments ? "true" : "false",declarationScope);
     printf ("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! \n");
#endif

#if 0
  // DQ (12/16/2020): Temporary check for where skipDefaultArguments == true, debugging test2020_79.C.
     if (skipDefaultArguments == true)
        {
          printf ("Exiting because of temporary check for skipDefaultArguments == true \n");
          ROSE_ASSERT(false);
        }
#endif

#if GNU_EXTENSIONS_ALLOWED
  // DQ (9/29/2012): Added detection of transparent unions
     if (p->is_transparent == true)
        {
       // For a parameter of union type, TRUE if the union is transparent.
       // This is a C only feature, transparent unions are not yet supported
          printf ("WARNING: This is a C only feature, transparent unions are not yet supported p = %p \n",p);
          ROSE_ASSERT(false);
        }

  // DQ (9/29/2012): Added detection of non-null pointers.
     if (p->nonnull == true)
        {
       // TRUE if this represents a parameter of pointer type that must be passed a non-NULL argument.
#ifdef DEBUG_CONVERSION
          printf ("WARNING: non-NULL argument specification is not yet supported p = %p \n",p);
#endif
       // ROSE_ASSERT(false);
        }
#endif

     SgName name = (p->name == NULL ? "" : p->name);

     a_type_ptr edg_type = p->declared_type == NULL ? p->type : p->declared_type;
     ROSE_ASSERT(edg_type != NULL);

#if 0
     printf ("In convert_function_parameter() name = %s \n",name.str());
     printf ("In convert_function_parameter() p->type = %p p->declared_type = %p \n",p->type,p->declared_type);
     printf ("In convert_function_parameter() using edg_type = %p \n",edg_type);
#endif

#if 0
  // DQ (12/16/2020): This was previously commented out and should remain so.
  // DQ (4/9/2018): We don't want this here.
     if (edg_type->declared_in_function_prototype == true)
        {
       // DQ (12/16/2020): This is an issue for test2012_04.c

       // Allocated a declaration scope IR node, build the type to have the associated declaration in the 
       // declaration instead of the global ro function scopes.
#if 1
          printf ("Exiting after building a SgDeclarationScope \n");
          ROSE_ASSERT(false);
#endif
        }
#endif

#ifdef DEBUG_CONVERSION
     if (skipDefaultArguments == true)
        {
          printf ("NOTE: In convert_function_parameter(): skipDefaultArguments == true (not clear why we would skip default parameters) \n");
        }
#endif

#if 0
     printf ("In convert_function_parameter(): Calling summarize_parameter_type: p = %p name = %s \n",p,name.str());
     summarize_param_type(p);
#endif

#ifdef DEBUG_CONVERSION
  // DQ (7/31/2014): This is the first step in detecting move constructors and move assignment operators.
     if (p->move_ctor_or_assign_parameter == true)
        {
          printf ("Detected C++11 move constructor or move asignment operator. \n");
        }
#endif

#if 0
     if (p->declared_type != NULL)
        {
          printf ("In convert_function_parameter(): Calling summarize_type() on parameter (p->declared_type): p = %p name = %s \n",p,name.str());
          summarize_type(p->declared_type);
        }
#endif

#if 0
     if (p->type != NULL)
        {
          printf ("In convert_function_parameter(): Calling summarize_type() on parameter (p->type): p = %p name = %s \n",p,name.str());
          summarize_type(p->type);
        }
#endif

#if 0
     printf ("************ In convert_function_parameter(): p->name = %s p->declared_type = %p \n",(p->name == NULL ? "null" : p->name),p->declared_type);
#endif

  // DQ (11/21/2012): Note that for test2012_168.c the compilation is recursive in convert_type() starting at this point, and never returns.
  // SgType* type = convert_type(p->declared_type == NULL ? p->type : p->declared_type);
     SgType* type = convert_type(edg_type);

#if 0
     printf ("************ In convert_function_parameter(): DONE with call to convert_type(): p->name = %s type = %p = %s type name = %s \n",
             (p->name == NULL ? "null" : p->name),type,type->class_name().c_str(),SageInterface::get_name(type).c_str());
#endif

#if 0
  // DQ (4/21/2018): Adding debugging for Plum Hall tests.
     SgClassType* classType = isSgClassType(type);
     if (classType != NULL)
        {
          SgClassDeclaration* classDeclaration = isSgClassDeclaration(classType->get_declaration());
          ROSE_ASSERT(classDeclaration != NULL);
#if 0
          printf ("   --- classDeclaration->get_name() = %s \n",classDeclaration->get_name().str());
#endif
          SgScopeStatement* scope = classDeclaration->get_scope();
          ROSE_ASSERT(scope != NULL);
#if 0
          printf ("   --- scope = %p classDeclaration->get_scope() = %s \n",scope,scope->class_name().c_str());
#endif
        }
#endif
     SgInitializer* def_arg = NULL;

#if 0
     printf ("p (associatedParameter) = %p \n",p);
#endif

     if (p->has_default_arg == true && skipDefaultArguments == false)
        {
#if 0
          printf ("This parameter has a default argument: p->default_arg_expr = %p \n",p->default_arg_expr);
#endif
          an_expr_node_ptr defaultArgExpr = p->default_arg_expr;
          if (defaultArgExpr != NULL)
             {
               a_seq_number defaultArgExpr_source_sequence_number = defaultArgExpr->expr_range.start.seq;

               ROSE_ASSERT(skipDefaultArguments == false);

            // If the default expression is a reference operator then EDG does not store the source position. 
            // Since ROSE's translation will recover a better representation for for the subtree we can get 
            // a valid source sequence number from the ROSE IR node more directly.
               if (defaultArgExpr_source_sequence_number == 0)
                  {
#if 0
                    printf ("In convert_function_parameter(): Calling summarize_expression: defaultArgExpr = %p for variable name = %s \n",defaultArgExpr,name.str());
                    summarize_expr(defaultArgExpr);
#endif
                 // Note that we are building the default argument here so that we can generate a propoer non-zero source sequence number fo use below.
                    SgInitializer* temp_default_argument = set_file_info_for_expr(buildAssignInitializer_nfi(convert_expression(p->default_arg_expr)),defaultArgExpr);
                    ROSE_ASSERT(temp_default_argument != NULL);

                 // if (temp_default_argument != NULL)
                 //    {
#if 0
                    printf ("In convert_function_parameter(): temp_default_argument = %p = %s \n",temp_default_argument,temp_default_argument->class_name().c_str());
#endif
                    defaultArgExpr_source_sequence_number = temp_default_argument->get_file_info()->get_source_sequence_number();
#if 0
                 // printf ("In convert_function_parameter(): Reset the defaultArgExpr_source_sequence_number to %" PRIuPTR " \n",defaultArgExpr_source_sequence_number);
                    printf ("In convert_function_parameter(): Reset the defaultArgExpr_source_sequence_number to %u \n",defaultArgExpr_source_sequence_number);
#endif
                    if (defaultArgExpr_source_sequence_number == 0)
                       {
                      // Call the new mechanism to support collection of source sequence entries.
                         set<unsigned int> sourceSequenceEntries = SageInterface::collectSourceSequenceNumbers(temp_default_argument);
#if 0
                         printf ("In convert_function_parameter(): (still zero): sourceSequenceEntries.size() = %" PRIuPTR " \n",sourceSequenceEntries.size());
#endif
                         unsigned int nonZeroEntry = 0;
                         for (set<unsigned int>::iterator i = sourceSequenceEntries.begin(); i != sourceSequenceEntries.end(); i++)
                            {
#if 0
                              printf ("   --- sourceSequenceEntries: i = %u \n",*i);
#endif
                              if (*i != 0)
                                 {
                                   nonZeroEntry = *i;
                                 }
                            }
#if 0
                         printf ("In convert_function_parameter(): (still zero): nonZeroEntry = %d \n",nonZeroEntry);
#endif
                         defaultArgExpr_source_sequence_number = nonZeroEntry;
                       }
                    // }

                 // Set the default argument so we don't have to recompute it below.
                    def_arg = temp_default_argument;

                 // ROSE_ASSERT(skipDefaultArguments == true || defaultArgExpr_source_sequence_number != 0);
                    ROSE_ASSERT(skipDefaultArguments == false);
                  }

               ROSE_ASSERT(skipDefaultArguments == false);
               if (defaultArgExpr_source_sequence_number >= global_source_sequence_number)
                  {
#if 0
                    printf ("HERE IS THE DEFAULT ARGUMENT ASSOCIATED WITH THE CURRENT FUNCTION DECLARATION \n");
#endif
                    ROSE_ASSERT(defaultArgExpr_source_sequence_number != 0);

                 // DQ (8/11/2012): Note that test2012_187.C has defaultArgExpr_source_sequence_number == 0.
                    if (defaultArgExpr_source_sequence_number == 0)
                       {
#ifdef DEBUG_CONVERSION
                         printf ("WARNING: defaultArgExpr_source_sequence_number == 0: allowing this to be processed as default argument. \n");
#endif
                       }

#if 0
                    printf ("In convert_function_parameter(): calling buildAssignInitializer_nfi() if (def_arg == NULL): def_arg = %p \n",def_arg);
#endif

                 // DQ (4/25/2013): Note that because of the if predicate (above) p->has_default_arg is always true.
                    if (def_arg == NULL)
                       {
                         def_arg = set_file_info_for_expr(buildAssignInitializer_nfi(convert_expression(p->default_arg_expr)),defaultArgExpr);
                         ROSE_ASSERT(def_arg != NULL);
                       }

                 // We might have just built the default argument or built it as part of generating a proper source sequence number above.
                    if (def_arg != NULL)
                       {
#if 0
                         def_arg->get_file_info()->display("default argument source position info: debug");
#endif
                      // However, if the source sequence in the default argument is zero, we need to reset it to a proper value.
                         if (def_arg->get_file_info()->get_source_sequence_number() == 0)
                            {
#if 0
                           // printf ("Reset the source sequence number in the default argument to %" PRIuPTR " \n",defaultArgExpr_source_sequence_number);
                              printf ("Reset the source sequence number in the default argument to %u \n",defaultArgExpr_source_sequence_number);
#endif
                              def_arg->get_file_info()->set_source_sequence_number(defaultArgExpr_source_sequence_number);

#if 0
                              def_arg->get_file_info()->display("default argument source position info (after reset): debug");
#endif
                            }

                         ROSE_ASSERT(def_arg->get_file_info()->get_source_sequence_number() != 0);
                       }

                    ROSE_ASSERT(def_arg != NULL);
                    ROSE_ASSERT(def_arg->get_file_info()->get_source_sequence_number() != 0);
                  }
                 else
                  {
                    if (def_arg != NULL)
                       {
                      // We don't need this default argument since it appears after any possible use by a relevant function declaration.
                      // ROSE_ASSERT(def_arg->get_file_info()->get_source_sequence_number() != 0);
#if 0
                         printf ("delete the AST represented by the default argument def_arg = %p = %s \n",def_arg,def_arg->class_name().c_str());
#endif
                     //  SageInterface::deleteAST(def_arg);
                         def_arg = NULL;
                       }
                  }
             }
            else
             {
            // DQ (8/11/2012): Output a warning for this (strange) case.
            // This can be NULL if the default argument value has not yet been evaluated, or for a template default
            // argument value whose value was never needed.
#ifdef DEBUG_CONVERSION
               printf ("WARNING: p->has_default_arg == true, but could not find the default arg (defaultArgExpr == NULL) \n");
#endif
             }
        }

#if 0
     printf ("In convert_function_parameter(a_param_type_ptr p = %p, skipDefaultArguments = %s): def_arg = %p = %s \n",
          p,skipDefaultArguments ? "true" : "false",def_arg,(def_arg != NULL) ? (def_arg->class_name().c_str()) : "NULL");
#endif

     if (def_arg != NULL)
        {
#if 0
          def_arg->get_file_info()->display("default argument: debug");
#endif
       // Set these to be marked as default arguments (in the function parameter list).  This is a bit redundant since from
       // the structure of being initializes to paramters in the function paramter list they coudl not be anything but default 
       // arguments.  The marking as default parameters is more significant in the function call's argument list where it is
       // used to avoid unparsing those parameters (so that we have a better quality source to source handling).  In the
       // context of default parameters in the function call argument list, the default arguments are used non unparsed,
       // but are correctly represented in the AST (to support analysis).
          def_arg->get_file_info()->setDefaultArgument();
          def_arg->get_startOfConstruct()->setDefaultArgument();
          def_arg->get_endOfConstruct()->setDefaultArgument();
        }

#if 0
     printf ("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ \n");
     printf ("Before calling buildInitializedName(): name = %s type = %p = %s type name = %s \n",name.str(),type,type->class_name().c_str(),SageInterface::get_name(type).c_str());
     printf ("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ \n");
#endif

  // DQ (10/22/2012): The position of the function parameters is taken from the defining declaration. This is a problem for 
  // function prototypes that might use the defining declaration source position for the function prototype.  So set the source 
  // position to compiler generated for function prototypes and the correct source position for the defining function declarations.
     SgInitializedName* initializedName = buildInitializedName_nfi(name, type, def_arg);

     if (edg_type->declared_in_function_prototype) 
        {
          initializedName->set_needs_definitions(true);
        }

     set_file_info_for_decl_pos(initializedName, p->decl_pos_info);

     initializedName->set_scope(topScopeStack());

#if DEBUG_STRUCTURAL_DESIGN || 0
     printf ("Leaving convert_function_parameter(): name = %s SgInitializedName = %p \n",name.str(),initializedName);
#endif
#if 0
     initializedName->get_startOfConstruct()->display("Leaving convert_function_parameter(): start: debug");
     initializedName->get_endOfConstruct()->display("Leaving convert_function_parameter(): end: debug");
#endif

  // DQ (1/18/2014): Refctoring code so I can call it from convert_variable() as well.
     set_noreturn_attribute(p->type,initializedName);

  // DQ (1/18/2014): Added assertion.
     ROSE_ASSERT(initializedName != NULL);

  // DQ (8/18/2014): Setup any possible Microsoft declspec attributes.
     if (p->attributes != NULL)
        {
          set_ms_attributes(initializedName,p->attributes);
        }

  // DQ (1/24/2016): Added custom (non-standard) support to read a GNU attribute on the 
  // SgModifierType in the parameter's type, and use it to mark this as a CUDA device).
     if (edg_type->source_corresp.attributes != NULL)
        {
#if 0
          printf ("In convert_function_parameter(): calling set_type_attributes() \n");
#endif
       // This could be named: set_function_parameter_using_attributes()
          set_variable_attributes(edg_type->source_corresp.attributes,initializedName);
#if 0
          printf ("In convert_function_parameter(): DONE: calling set_type_attributes() \n");
#endif
#if 0
          printf ("Exiting as a test! \n");
          ROSE_ASSERT(false);
#endif
        }
#if 0
     printf ("initializedName->get_name() = %s \n",initializedName->get_name().str());
     initializedName->get_file_info()->display("Leaving convert_function_parameter(): initializedName: debug");
#endif

     return initializedName;
   }
