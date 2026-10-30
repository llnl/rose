
#include "edgRose.h"

// DQ (4/21/2018): Add support to build a SgDeclarationScope if it is detected in the parameter type constrcution 
// that we need one for types that are defined in the function parameter list.  See test2018_41.c for an example.
SgFunctionParameterList*
EDG_ROSE_Translation::convert_function_parameter_list(a_routine_ptr p, bool skipDefaultArguments, a_type_ptr declared_type, SgDeclarationScope* & declarationScope )
   {
  // DQ (4/11/2018): I think we need to modify this function to take the declared type instead of the a_routine_ptr p, or in addition to it.
  // This would allow this function to be called from the parse_secondary_declaration() function so that the declared function parameters 
  // could be accumulated.  This is essential to the handling of the C_tests/test2018_32.c example code.

  // DQ (8/11/2012): This function uses the type information to obtain the function parameter data (as a result it is not 
  // precise in evaluating default arguments where they are used).

  // DQ (12/1/2011): This is a refactored function to support the generation of the function parameter lists.

#ifdef DEBUG_CONVERSION
     indent_line(); debug_print("function param list skipDefaultArguments = %s \n", (skipDefaultArguments ? "true" : "false")); 
#endif

  // print_source_range(p->decl_pos_info ? p->decl_pos_info->identifier_range : null_source_range); debug_print("\n");

#if 0
     printf ("In convert_function_parameter_list() p = %p skipDefaultArguments = %s \n",p,skipDefaultArguments ? "true" : "false");
     a_seq_number function_source_sequence_number = p->source_corresp.decl_position.seq;
  // printf ("function_source_sequence_number = %" PRIuPTR " \n",function_source_sequence_number);
     printf ("function_source_sequence_number = %u \n",function_source_sequence_number);
#endif

#if 0
     printf ("In convert_function_parameter_list(): calling summarize_routine() \n");
     summarize_routine(p);
#endif

     a_variable_ptr param_var = NULL;

     a_type_ptr p_type = p->type;
     ROSE_ASSERT(p_type != NULL);

#if 0
  // printf ("In convert_function_parameter_list() p->type = %p p->declared_type = %p \n",p->type,p->declared_type);
     printf ("In convert_function_parameter_list() p->type = %p declared_type = %p \n",p->type,declared_type);
     printf ("In convert_function_parameter_list() using p_type = %p \n",p_type);
#endif

  // DQ (8/19/2012): We need to strip off any typedefs that hide the a_routine type.
  // ROSE_ASSERT(p_type->kind == tk_routine);

     if (p_type->kind != tk_routine)
        {
       // DQ (9/18/2012): This is temporary code (FIXME!).
       // I think we need to stip off any number of typedefs used to hide the a_routine_type (see test2012_192.C).
          ROSE_ASSERT(p_type->kind == tk_typeref);

#if defined(DEBUG_CONVERSION) || 0
          printf ("UNCONVENTIONAL CODE: In convert_function_parameter_list(): (function declared using typedef of function type) p_type = %p p_type->variant.typeref.type = %p \n",p_type,p_type->variant.typeref.type);
#endif
       // We are efectively stripping off the typedef since it is not clear how to represent a function with this in place.
       // We don't have a concept of a function declaration that is a name using a typedef of a function type.
       // So we have to peel away the typedef and represent the function directly (using the typedef's base type).
          p_type = p_type->variant.typeref.type;
          ROSE_ASSERT(p_type->kind == tk_routine);
        }

     a_routine_type_supplement_ptr sup = p_type->variant.routine.extra_info;
     ROSE_ASSERT(sup != NULL);

#if 0
     printf ("p_type->variant.routine.extra_info = %p \n",p_type->variant.routine.extra_info);
#endif

  // DQ (4/11/2018): Use the type supplement from the declared type instead (where it is avaiable).
  // if (p->declared_type != NULL)
     if (declared_type != NULL)
        {
#if 0
          printf ("@@@@@@@@@@@@@ Found declared_type: using p->declared_type->variant.routine.extra_info instead \n");
#endif
       // sup = p->declared_type->variant.routine.extra_info;
          sup = declared_type->variant.routine.extra_info;
          ROSE_ASSERT(sup != NULL);
#if 0
       // printf ("p->declared_type->variant.routine.extra_info = %p \n",p->declared_type->variant.routine.extra_info);
          printf ("declared_type->variant.routine.extra_info = %p \n",declared_type->variant.routine.extra_info);
#endif
        }

#if 0
     printf ("In convert_function_parameter_list(): calling summarize_a_routine_type_supplement(): sup = %p \n",sup);
     summarize_routine_type_supplement(sup);
#endif

     a_param_type_ptr params = sup->param_type_list;

#if 0
  // DQ (1/9/2014): Added debbung output (debugging test2014_29.c, but not useful).
     if (params == NULL)
        {
          printf ("WARNING: In convert_function_parameter_list(): sup->param_type_list == NULL \n");
          ROSE_ASSERT(false);
        }
#endif

  // SgFunctionParameterList* param_list = set_generated_file_info_to_unparse(buildFunctionParameterList_nfi());
     SgFunctionParameterList* param_list = set_file_info_for_corresp(buildFunctionParameterList_nfi(),p->source_corresp);
     ROSE_ASSERT(param_list != NULL);

#if 0
     printf ("In convert_function_parameter_list(): param_list = %p \n",param_list);
#endif

  // DQ (11/11/2012): It is better that this be marked as compiler generated.
  // This avoids the paramter list as appearing to come before the function declaration.
  // This also allows the actual function parameters to have a more meaningful source position.
  // Plus this is a type of IR node with which we would like to not associated CPP directives and comments.
  // This is also important to fixing UnparseHeadersTests/Simple0.C
     param_list->get_startOfConstruct()->setCompilerGenerated();
     param_list->get_endOfConstruct()  ->setCompilerGenerated();

  // DQ (5/1/2012): Debugging where this is set as a transformation.
     ROSE_ASSERT(param_list->get_startOfConstruct() != NULL);
     ROSE_ASSERT(param_list->get_endOfConstruct()   != NULL);
     ROSE_ASSERT(param_list->get_startOfConstruct()->isTransformation() == false);
     ROSE_ASSERT(param_list->get_endOfConstruct()->isTransformation()   == false);

  // DQ (1/23/2013): Since we don't use the param_types list we don't need to save it.
  // vector<SgType*> param_types;

  // DQ (5/16/2012): Need to finish this (have this be passed to the convert_function_parameter() function.
  // ROSE_ASSERT(skipDefaultArguments == false);

     for (a_param_type_ptr param = params; param != NULL; param = param->next)
        {
          ROSE_ASSERT(param != NULL);
          string temp_parameter_name = param->name != NULL ? param->name : "";
#if 0
          printf ("In convert_function_parameter_list(): processing function parameter: name = %s \n",temp_parameter_name.c_str());
       // string name = param->name;
          if (temp_parameter_name == "array")
             {
               printf ("In convert_function_parameter_list(): in loop over parameters: calling summarize_param_type() \n");
               summarize_param_type(param);
             }
#endif
#if 0
          printf ("In convert_function_parameter_list(): in loop over parameters: param->type: calling summarize_type() \n");
          summarize_type(param->type);
#endif
#if 0
          if (param->type != param->declared_type)
             {
               printf ("In convert_function_parameter_list(): in loop over parameters: param->declared_type: calling summarize_type() \n");
               summarize_type(param->declared_type);
             }
#endif
#if 0
          printf ("&&&&&&&&&&&&&&&&&&&&&&& In convert_function_parameter_list(): Calling convert_function_parameter(): name = %s \n",temp_parameter_name.c_str());
#endif

       // DQ (4/21/2018): Add support to build a SgDeclarationScope if it is detected in the parameter type constrcution 
       // that we need one for types that are defined in the function parameter list.  See test2018_41.c for an example.
       // SgDeclarationScope* declarationScope = NULL;

       // SgInitializedName* this_param = convert_function_parameter(param);
          SgInitializedName* this_param = convert_function_parameter(param,skipDefaultArguments,declarationScope);
#if 0
          printf ("DONE: In convert_function_parameter_list(): Calling convert_function_parameter() \n");
#endif
#if 0
          printf ("this_param->get_type() = %p = %s \n",this_param->get_type(),this_param->get_type()->unparseToString().c_str());
#endif
#if 0
          this_param->get_file_info()->display("In convert_function_parameter_list(): function parameter: debug");
#endif
       // param_types.push_back(this_param->get_type());
          appendArg(param_list, this_param);

       // DQ (5/15/2012): I want to set this but I don't have the function declaration yet...so we might have to fix it up afterward.
       // this_param->set_declptr(functionDeclaration);

// #if GNU_EXTENSIONS_ALLOWED
       // DQ (8/16/2014): These functions are now called as part of an API supporting both GNU and Microsoft attrivutes.
       // I am not clear on what gnu attributes are allowed on function parameters.
          if (param_var != NULL)
             {
               set_variable_attributes(param_var,this_param);
             }
#if 0
       // DQ (1/6/2014): Adding support for noreturn attribute for function parameters.
          ROSE_ASSERT(param->type != NULL);
          if (param->type->kind == tk_routine && param->type->variant.routine.extra_info != NULL)
             {
               printf ("Exiting as a test! \n");
               ROSE_ASSERT(false);
             }
#endif
// #endif

#if ENABLE_CUDA_SUPPORT
          if (param_var != NULL)
             {
               set_variable_cuda_attributes(param_var, this_param);
             }
#endif /* ENABLE_CUDA_SUPPORT */

          if (param_var != NULL)
             {
               param_var = param_var->next;
             }
#if 0
          printf ("In convert_function_parameter_list(): base of loop over function parameters \n");
#endif
        }

     bool has_ellipsis = sup->has_ellipsis;
     if (has_ellipsis == true)
        {
          appendArg(param_list, set_generated_file_info_to_unparse(buildInitializedName_nfi("", SgTypeEllipse::createType(), NULL)));
        }

#if 0
  // Note that param_types is not used anywhere...(so commented out above).
     for (size_t i = 0; i < param_types.size(); i++)
        {
          printf ("param_types[%" PRIuPTR "] = %s \n",i,param_types[i]->class_name().c_str());
        }
#endif
#if 0
     for (size_t i = 0; i < param_list->get_args().size(); i++)
        {
          printf ("param_list->get_args()[%" PRIuPTR "] = %s \n",i,param_list->get_args()[i]->class_name().c_str());
        }
#endif

     ROSE_ASSERT(param_list != NULL);

  // DQ (5/1/2012): Debugging where this isset as a transformation.
     ROSE_ASSERT(param_list->get_startOfConstruct() != NULL);
     ROSE_ASSERT(param_list->get_endOfConstruct()   != NULL);
     ROSE_ASSERT(param_list->get_startOfConstruct()->isTransformation() == false);
     ROSE_ASSERT(param_list->get_endOfConstruct()->isTransformation()   == false);

#if DEBUG_STRUCTURAL_DESIGN || 0
     printf ("Leaving convert_function_parameter_list(): param_list = %p param_list->get_args().size() = %" PRIuPTR " \n",param_list,param_list->get_args().size());
#endif
#if 0
     param_list->get_startOfConstruct()->display("Leaving convert_function_parameter_list(): START: debug");
     param_list->get_endOfConstruct()  ->display("Leaving convert_function_parameter_list(): END: debug");
#endif
     return param_list;
   }
