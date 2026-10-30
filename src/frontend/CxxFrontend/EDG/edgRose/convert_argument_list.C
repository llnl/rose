
#include "edgRose.h"

#define DEBUG_convert_argument_list 0

SgExprListExp* EDG_ROSE_Translation::convert_argument_list (a_new_delete_supplement_ptr ndsp, an_expr_node_ptr arg )
   {
  // Generate an empty constructor argument list
     SgExprListExp* arguementList = set_generated_file_info_to_unparse(buildExprListExp_nfi());

#if DEBUG_convert_argument_list
     printf ("In convert_argument_list(): ndsp->placement_new = %s \n",ndsp->placement_new ? "true" : "false");
#endif

#if DEBUG_convert_argument_list
     printf ("In convert_argument_list(): ndsp->number_of_elements = %p \n",ndsp->number_of_elements);
     printf ("In convert_argument_list(): ndsp->arg = %p \n",ndsp->arg);
#endif

  // DQ (6/12/2013): For the case of EDG 4.7 we have to handle that the number_of_elements is not in 
  // the list and so needs to be added seperately. Note that this code could be refactored.
  // ROSE_ASSERT(ndsp->number_of_elements != NULL);
     if (ndsp->number_of_elements != NULL)
        {
          bool isCompilerGeneratedConstant = ( (ndsp->number_of_elements->kind == enk_constant) && (ndsp->number_of_elements->variant.constant.ptr->source_corresp.decl_position.seq == 0) );

#if DEBUG_convert_argument_list
          printf ("(placement) isCompilerGeneratedConstant = %s \n",isCompilerGeneratedConstant ? "true" : "false");
#endif
          if (isCompilerGeneratedConstant == false)
             {
               SgExpression* expression = set_generated_file_info_to_unparse(convert_expression(ndsp->number_of_elements));

#if DEBUG_convert_argument_list
               printf ("In convert_argument_list(): expression (in expression list) = %p = %s \n",expression,expression->class_name().c_str());
#endif
               appendExpression(arguementList, expression);
             }
        }

     for (an_expr_node_ptr argument = arg; argument != NULL; argument = argument->next)
        {
       // DQ (1/19/2020): We need to append the arguments to the list independent of status as compiler generated.
       // I think that the source position has already been set so we need not reset it.
          SgExpression* expression = convert_expression(argument);
          ROSE_ASSERT(expression != NULL);

          appendExpression(arguementList, expression);
#if 0
          expression->get_startOfConstruct()->display("In convert_argument_list(): expression");
#endif
        }

#if DEBUG_convert_argument_list
     printf ("Leaving convert_argument_list(): expression list = %p size = %zu \n",arguementList,arguementList->get_expressions().size());
#endif

     return arguementList;
   }
