
#include "edgRose.h"

SgExpression* EDG_ROSE_Translation::convert_lambda(a_lambda_ptr lambda_info)
   {
  // DQ (7/21/2012): Support for lambda expression handling.
  // DQ (9/3/2014): Note that the source position of the "mutable" keyword is also available in EDG, but ignored in ROSE.
     SgExpression* result = NULL;

#ifdef DEBUG_CONVERSION
  // DQ (7/10/2020): Adding debug information.
     indent_line(); debug_print("lambda expression: lambda_info = %p \n",lambda_info);
     ScopedIndent si(indent);
#endif

     ROSE_ASSERT(lambda_info != NULL);

#if 0
     printf ("In convert_lambda_expression(): Calling summarize_lambda_expr() \n");
     summarize_lambda_expr(lambda_info);
#endif
#if 0
     if (lambda_info->capture_list != NULL)
        {
          printf ("In convert_lambda_expression(): Calling summarize_lambda_capture() \n");
          summarize_lambda_capture(lambda_info->capture_list);
        }
#endif
#if 0
     if (lambda_info->lambda_routine != NULL)
        {
          printf ("In convert_lambda_expression(): Calling summarize_routine() \n");
          summarize_routine(lambda_info->lambda_routine);
        }
#endif
#if 0
     printf ("In convert_lambda_expression(): Calling summarize_dynamic_init() \n");
     summarize_dynamic_init(lambda_initialization_dip);
#endif

#if 0
     printf ("@@@@@@@@@@@@@ Building the lambda capture list \n");
#endif
     SgLambdaCaptureList* lambdaCaptureList = buildLambdaCaptureList_nfi();

     set_file_info_for_where_corresp_is_unavailable(lambdaCaptureList, lambda_info->start_position);

#if EXTRA_SOURCE_POSITIONS_IN_IL
     set_file_info_end(lambdaCaptureList, lambda_info->capture_end_position);
#endif

     a_lambda_capture_ptr capture_ptr = lambda_info->capture_list;
     while(capture_ptr != NULL)
        {
#ifdef DEBUG_CONVERSION
          printf ("Building the lambda capture \n");
#endif
       // summarize_lambda_capture(capture_list_ptr);
       // SgInitializedName* capture_variable = NULL;
          SgExpression* capture_variable = NULL;
          if (capture_ptr->captured.variable != NULL)
             {
#ifdef DEBUG_CONVERSION
               printf ("Building the lambda capture variable: capture_ptr->variable->is_this_parameter = %s \n",capture_ptr->captured.variable->is_this_parameter ? "true" : "false");
#endif
            // DQ (2/15/2015): Added support for this paramber in the lambda capture list.
               if (capture_ptr->captured.variable->is_this_parameter == true)
                  {
#if 0
                    printf ("In convert_lambda_expression(): Calling summarize_variable() \n");
                    summarize_variable(capture_ptr->variable);
#endif
                    bool pointerToMember = false;
                    ROSE_ASSERT(capture_ptr->captured.variable->type != NULL);
                    SgType* this_type = convert_type(capture_ptr->captured.variable->type,pointerToMember);
                    ROSE_ASSERT(this_type != NULL);
#if 0
                    printf ("this_type = %p = %s \n",this_type,this_type->class_name().c_str());
#endif
                    SgType* base_type = this_type->findBaseType();
                    ROSE_ASSERT(base_type != NULL);
                    SgClassType* classType = isSgClassType(base_type);
                    ROSE_ASSERT(classType != NULL);
                    SgDeclarationStatement* declaration = classType->get_declaration();
                    ROSE_ASSERT(declaration != NULL);
                    SgClassDeclaration* classDeclaration = isSgClassDeclaration(declaration);
                    ROSE_ASSERT(classDeclaration != NULL);
                    SgSymbol* symbol = classDeclaration->get_symbol_from_symbol_table();
                    ROSE_ASSERT(symbol != NULL);
                    SgClassSymbol* classSymbol = isSgClassSymbol(symbol);
                    ROSE_ASSERT(classSymbol != NULL);
                    SgThisExp* thisExpression = buildThisExp(classSymbol);
                    ROSE_ASSERT(thisExpression != NULL);
                    capture_variable = thisExpression;
#if 0
                    printf ("Build a this expression (not yet implemented) \n");
                    ROSE_ASSERT(false);
#endif
                  }
                 else
                  {
                 // DQ (2/15/2015): Older version of code before seperating out the "this" expression handling.
                    SgVariableSymbol* capture_variable_symbol = convert_variable_use(capture_ptr->captured.variable);
                    capture_variable = buildVarRefExp_nfi(capture_variable_symbol);
                  }
               ROSE_ASSERT(capture_variable != NULL);
               set_file_info_for_corresp(capture_variable,capture_ptr->captured.variable->source_corresp);
             }

       // SgInitializedName* source_closure_variable = NULL;
          SgExpression* source_closure_variable = NULL;
          if (capture_ptr->capture_info.source_closure_field != NULL)
             {
#ifdef DEBUG_CONVERSION
               printf ("Building the lambda capture source closure variable \n");
#endif
               SgVariableSymbol* source_closure_variable_symbol = convert_field_use(capture_ptr->capture_info.source_closure_field);
               source_closure_variable = buildVarRefExp_nfi(source_closure_variable_symbol);
               set_file_info_for_corresp(source_closure_variable,capture_ptr->capture_info.source_closure_field->source_corresp);
             }

       // SgInitializedName* closure_variable = NULL;
          SgExpression* closure_variable = NULL;
          if (capture_ptr->closure_field != NULL)
             {
#ifdef DEBUG_CONVERSION
               printf ("Building the lambda capture closure variable \n");
#endif
            // closure_variable = convert_field(capture_ptr->closure_field);
               SgVariableSymbol* closure_variable_symbol = convert_field_use(capture_ptr->closure_field);
            // closure_variable = closure_variable_symbol->get_declaration();
               closure_variable = buildVarRefExp_nfi(closure_variable_symbol);

               set_file_info_for_corresp(closure_variable,capture_ptr->closure_field->source_corresp);
             }

          SgLambdaCapture* lambdaCapture = buildLambdaCapture_nfi(capture_variable,source_closure_variable,closure_variable);
          ROSE_ASSERT(lambdaCapture != NULL);

       // set_file_info_for_expr(lambdaCapture, capture_ptr);
          set_file_info_for_where_corresp_is_unavailable(lambdaCapture, capture_ptr->position);

#if EXTRA_SOURCE_POSITIONS_IN_IL
          set_file_info_end(lambdaCapture, capture_ptr->end_position);
#endif

#ifdef DEBUG_CONVERSION
       // Set associated flags
          printf ("          capture_by_reference         = %s \n",capture_ptr->capture_by_reference ? "true" : "false");
          printf ("          is_implicit                  = %s \n",capture_ptr->is_implicit ? "true" : "false");
          printf ("          is_pack_expansion            = %s \n",capture_ptr->is_pack_expansion ? "true" : "false");
#endif
          if (capture_ptr->capture_by_reference == true)
             {
#ifdef DEBUG_CONVERSION
               printf ("capture_ptr->capture_by_reference == true \n");
#endif
               lambdaCapture->set_capture_by_reference(true);
             }

          if (capture_ptr->is_implicit == true)
             {
#ifdef DEBUG_CONVERSION
               printf ("capture_ptr->is_implicit == true \n");
#endif
               lambdaCapture->set_implicit(true);
             }

          if (capture_ptr->is_pack_expansion == true)
             {
#ifdef DEBUG_CONVERSION
               printf ("capture_ptr->is_pack_expansion == true \n");
#endif
               lambdaCapture->set_pack_expansion(true);
             }

          lambdaCaptureList->get_capture_list().push_back(lambdaCapture);

          capture_ptr = capture_ptr->next;
        }

#if 0
     printf ("@@@@@@@@@@@@@ lambda_info->closure_class = %p \n",lambda_info->closure_class);
#endif

     SgClassDeclaration* lambda_closure_class = NULL;
     if (lambda_info->closure_class != NULL)
        {
#ifdef DEBUG_CONVERSION
          printf ("Building the lambda closure class \n");
#endif
          SgType* lambda_closure_type = convert_type(lambda_info->closure_class);
          ROSE_ASSERT(lambda_closure_type != NULL);

          SgClassType* lambda_closure_class_type = isSgClassType(lambda_closure_type);
          ROSE_ASSERT(lambda_closure_class_type != NULL);

          SgDeclarationStatement* declarationStatement = lambda_closure_class_type->get_declaration();
          ROSE_ASSERT(declarationStatement != NULL);
          lambda_closure_class = isSgClassDeclaration(declarationStatement);
          ROSE_ASSERT(lambda_closure_class != NULL);
        }

#if 0
     printf ("@@@@@@@@@@@@@ lambda_closure_class = %p (%s)\n", lambda_closure_class, lambda_closure_class ? lambda_closure_class->class_name().c_str() : "");
     printf ("@@@@@@@@@@@@@     ->get_parent()   = %p (%s)\n", lambda_closure_class ? lambda_closure_class->get_parent() : NULL, ( lambda_closure_class && lambda_closure_class->get_parent() ) ? lambda_closure_class->get_parent()->class_name().c_str() : "");
#endif

#if 0
     printf ("@@@@@@@@@@@@@ lambda_info->lambda_routine = %p \n",lambda_info->lambda_routine);
#endif

     SgFunctionDeclaration* lambda_function = NULL;
     if (lambda_info->lambda_routine != NULL)
        {
#ifdef DEBUG_CONVERSION
          printf ("Building the lambda function \n");
#endif
          if (lambda_info->is_generic) {
            a_template_ptr assoc_template = lambda_info->lambda_routine->assoc_template;
            ROSE_ASSERT(assoc_template != NULL);

            templateDeclarationStack.push_back(assoc_template);

            SgDeclarationScope * nonreal_decl_scope = get_nonreal_declaration_scope_for_template(assoc_template, "convert_lambda_expression");

            lambda_function = convert_routine(lambda_info->lambda_routine, true, assoc_template);

            attach_nonreal_declaration_scope_to_template(assoc_template, lambda_function, nonreal_decl_scope, "convert_lambda_expression");

            templateDeclarationStack.pop_back();
          } else {
            lambda_function = convert_routine(lambda_info->lambda_routine);
          }
          ROSE_ASSERT(lambda_function != NULL);

          ROSE_ASSERT(lambda_function->get_firstNondefiningDeclaration() != NULL);
          ROSE_ASSERT(lambda_function->get_definingDeclaration() != NULL);
#if 0
          printf ("lambda_function = %p = %s \n",lambda_function,lambda_function->class_name().c_str());
          printf ("lambda_function->get_firstNondefiningDeclaration() = %p = %s \n",lambda_function->get_firstNondefiningDeclaration(),lambda_function->get_firstNondefiningDeclaration()->class_name().c_str());
          printf ("lambda_function->get_definingDeclaration()         = %p = %s \n",lambda_function->get_definingDeclaration(),lambda_function->get_definingDeclaration()->class_name().c_str());
#endif

       // DQ (9/3/2014): The starting and ending source positions are abby-normal for this EDG node.
          set_file_info_for_where_corresp_is_unavailable(lambda_function,lambda_info->lambda_routine->source_corresp.decl_position);
#if EXTRA_SOURCE_POSITIONS_IN_IL
          ROSE_ASSERT(lambda_info->lambda_routine->source_corresp.decl_pos_info != NULL);
          set_file_info_end(lambda_function, lambda_info->lambda_routine->source_corresp.decl_pos_info->variant.declarator_range.end);
#endif
        }

     ROSE_ASSERT(lambda_function != NULL);
     ROSE_ASSERT(lambda_function->get_file_info() != NULL);

#if 0
     printf ("Souce position for lambda function: \n");
     lambda_function->get_file_info()->display("lambda_function");
#endif

     SgLambdaExp* lambdaExp = buildLambdaExp_nfi(lambdaCaptureList,lambda_closure_class,lambda_function);

     set_starting_file_info_for_lambda(lambdaExp,lambda_info);

#if 0
  // Set associated flags
     printf ("     is_mutable                               = %s \n",lambda_info->is_mutable ? "true" : "false");
     printf ("     has_capture_default                      = %s \n",lambda_info->has_capture_default ? "true" : "false");
     printf ("     default_is_by_reference                  = %s \n",lambda_info->default_is_by_reference ? "true" : "false");
     printf ("     explicit_return_type                     = %s \n",lambda_info->explicit_return_type ? "true" : "false");
     printf ("     has_parameter_decl                       = %s \n",lambda_info->has_parameter_decl ? "true" : "false");
#endif

     if (lambda_info->is_mutable == true)
        {
#ifdef DEBUG_CONVERSION
          printf ("lambda_info->is_mutable == true \n");
#endif
          lambdaExp->set_is_mutable(true);
        }

     if (lambda_info->has_capture_default == true)
        {
#ifdef DEBUG_CONVERSION
          printf ("lambda_info->has_capture_default == true \n");
#endif
          lambdaExp->set_capture_default(true);
        }

     if (lambda_info->default_is_by_reference == true)
        {
#ifdef DEBUG_CONVERSION
          printf ("lambda_info->default_is_by_reference == true \n");
#endif
          lambdaExp->set_default_is_by_reference(true);
        }

     if (lambda_info->explicit_return_type == true)
        {
#ifdef DEBUG_CONVERSION
          printf ("lambda_info->explicit_return_type == true \n");
#endif
       // The use of C++ new return type syntax should also be a part of the lambda function.
          lambdaExp->set_explicit_return_type(true);
        }

     if (lambda_info->has_parameter_decl == true)
        {
#ifdef DEBUG_CONVERSION
          printf ("lambda_info->has_parameter_decl == true \n");
#endif
          lambdaExp->set_has_parameter_decl(true);
        }

     result = lambdaExp;

#if 0
  // printf ("Leaving convert_lambda_expression(): expr = %p result = %p = %s \n",expr,result,result->class_name().c_str());
     printf ("Leaving convert_lambda_expression(): lambda_info = %p result = %p = %s \n",lambda_info,result,result->class_name().c_str());
#endif

     return result;
   }
