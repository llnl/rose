
#include "edgRose.h"

namespace EDG_ROSE_Translation {

#define DEBUG__convert_dynamic_initializer 0

SgExpression * convert_dynamic_initializer(a_dynamic_init_ptr dip, a_type_ptr edgType)
   {
     ROSE_ASSERT(dip != NULL);
#if DEBUG__convert_dynamic_initializer
     printf ("Enter convert_dynamic_initializer():\n");
     printf ("  dip                               = %p \n", dip);
     printf ("    ->kind                          = %s \n", summarizeDynamicInitialization(dip->kind).c_str());
     printf ("    ->is_compound_literal           = %s \n", dip->is_compound_literal ? "true" : "false");
     printf ("    ->is_braced_initializer         = %s \n", dip->is_braced_initializer ? "true" : "false");
     printf ("    ->is_partially_initialized      = %s \n", dip->is_partially_initialized ? "true" : "false");
     printf ("    ->is_explicit_cast              = %s \n", dip->is_explicit_cast ? "true" : "false");
     printf ("    ->is_constructor_init           = %s \n", dip->is_constructor_init ? "true" : "false");
     printf ("    ->suppress_init_list_arg_braces = %s \n", dip->suppress_init_list_arg_braces ? "true" : "false");
     printf ("  edgType                           = %p \n", edgType);
#endif

     SgExpression * result = nullptr;
     switch (dip->kind)
        {
          case dik_none:
             {
            // No initialization.
            // This is the case of where given "class X;" we specify "X x;" instead of "X x();" "X x();" would be an 
            // error if there was no defined public default constructor in class "X". Interestingly "X x;" should 
            // always be correct, I think.  There is nothing to do here, unless we wanted to record the use of "()".
#if DEBUG__convert_dynamic_initializer
               printf ("  Case of dik_none\n");
#endif
               break;
             }

           case dik_zero: 
             {
            // Zero initialization, as in "A()" when A has no constructor.
#if DEBUG__convert_dynamic_initializer
               printf ("  Case of dik_zero:\n");
#endif
               ROSE_ASSERT (edgType != NULL);

               SgType * type = convert_type(edgType);
               ROSE_ASSERT(type != NULL);
#if DEBUG__convert_dynamic_initializer
               printf ("    type = %p = %s\n", type, type->class_name().c_str());
#endif

               SgConstructorInitializer * constInit = SageBuilder::buildConstructorInitializer_nfi(
                   /* declaration = */ nullptr,
                   /* args = */ nullptr,
                   type,
                   /* need_name = */ false,
                   /* need_qualifier = */ false,
                   /* need_parenthesis_after_name = */ true,
                   isSgClassType(type) == NULL ? true : false
               );
               constInit = set_generated_file_info_to_unparse(constInit);
               ROSE_ASSERT(constInit != NULL);
               constInit->set_is_explicit_cast(dip->is_explicit_cast);

               constInit->set_args(set_generated_file_info_to_unparse(constInit->get_args()));
               if (constInit->get_args()->get_expressions().empty() || constInit->get_declaration() != NULL || constInit->get_class_decl() != NULL)
                  {
                    constInit->set_need_name(true);
                  }
               result = constInit;
               break;
             }

          case dik_constant:
             {
#if DEBUG__convert_dynamic_initializer
               printf ("  Case of dik_constant\n");
#endif
               result = convert_constant(dip->variant.constant.ptr);
               break;
             }

          case dik_expression:
             {
#if DEBUG__convert_dynamic_initializer
               printf ("  Case of dik_expression\n");
#endif

               result = convert_expression(dip->variant.expression);
               SgNewExp* newExp = isSgNewExp(result);
               if (newExp != NULL)
                  {
                    SgConstructorInitializer* constructorInitializer = newExp->get_constructor_args();
                    ROSE_ASSERT(constructorInitializer != NULL);
                    constructorInitializer->set_is_explicit_cast(dip->is_explicit_cast);
                    constructorInitializer->set_is_braced_initialized(dip->is_braced_initializer);
                    constructorInitializer->set_associated_class_unknown(false);
                    constructorInitializer->set_need_parenthesis_after_name(false);
                    constructorInitializer->set_associated_class_unknown(true);
                  }
               break;
             }

          case dik_call_returning_class_via_cctor:
             {
            // Used for function calls that return a value via a copy constructor, only under enk_temp_init nodes.
#if DEBUG__convert_dynamic_initializer
               printf ("  Case of dik_call_returning_class_via_cctor \n");
#endif
               result = convert_expression(dip->variant.expression);
               break;
             }

          case dik_constructor:
             {
#if DEBUG__convert_dynamic_initializer
               printf ("  Case of dik_constructor \n");
#endif
               if (edgType == NULL)
                  {
                    printf ("I think it is a problem to call convert_dynamic_constructor_initializer() with edgType == NULL \n");
                    ROSE_ABORT();
                  }

               result = convert_dynamic_constructor_initializer(dip,edgType);
               if (dip->is_constructor_init == false)
                 {
                // DQ (1/16/2019): I think we want this to always be true for this case dik_constructor. 
                   SgConstructorInitializer * constructorInitializer = isSgConstructorInitializer(result);
                   ROSE_ASSERT(constructorInitializer != NULL);
                   constructorInitializer->set_need_name(true);
                 }
               break;
             }

          case dik_nonconstant_aggregate:
             {
            // Nonconstant aggregate constant, used in cases like: "int a[3] = {1, i+j, 3};"
#if DEBUG__convert_dynamic_initializer
               printf ("  Case of dik_nonconstant_aggregate\n");
#endif
               result = convert_constant(dip->variant.constant.ptr);
               break;
             }
          case dik_lambda:
             {
            // EDG provides two different views here.  The lambda member is the
            // source lambda and defines its shape through a_lambda::capture_list,
            // each a_lambda_capture::closure_field, and the closure class.  That
            // is the single representation consumed by the ROSE bridge.
            //
            // dip->variant.constant.ptr is an unshared ck_aggregate recipe for
            // initializing the materialized closure fields; it is deliberately
            // not converted into another SgLambdaExp.  Its ordered constants and
            // ck_dynamic_init entries can describe value-copy, reference, this
            // or *this capture, implicit conversion/copy construction, and C++14
            // init-capture initialization.  These elements generally correspond
            // to materialized closure fields, but not necessarily one-to-one with
            // capture records because a closure field can be pending or absent.
            //
            // Likewise, convert_lambda does not currently translate
            // a_lambda_capture::captured.initializer for an is_init_capture.
            // SgLambdaCapture retains the source variable, source closure field,
            // closure field expressions, and capture flags, but not the complete
            // field-initialization operation.  Analyses needing exact init-capture
            // or implicit copy/conversion semantics must consult these EDG nodes
            // until ROSE has explicit AST support for that operation.
#if DEBUG__convert_dynamic_initializer
               printf ("  Case of dik_lambda\n");
#endif
               result = convert_lambda(dip->variant.constant.lambda);
               break;
             }

          case dik_bitwise_copy:
          default: 
             {
               printf("Fatal: Bad dynamic initializer kind %s\n", summarizeDynamicInitialization(dip->kind).c_str()); 
               ROSE_ABORT();
             }
        }
#if DEBUG__convert_dynamic_initializer
     printf ("   result = %p = %s\n", result, result ? result->class_name().c_str() : "");
#endif
     SgInitializer* initializer = isSgInitializer(result);
     if (initializer != NULL)
        {
#if DEBUG__convert_dynamic_initializer
          printf ("  dip->is_explicit_cast               = %s \n", dip->is_explicit_cast ? "true" : "false");
          printf ("  initializer->get_is_explicit_cast() = %s \n", initializer->get_is_explicit_cast() ? "true" : "false");
#endif
          if (initializer->get_is_explicit_cast() != dip->is_explicit_cast) {
            initializer->set_is_explicit_cast(dip->is_explicit_cast);
            initializer->set_is_braced_initialized(dip->is_braced_initializer);
          }
        }

#if DEBUG__convert_dynamic_initializer
     printf("Leave convert_dynamic_initializer(%p)\n", dip);
#endif
     return result;
   }
}
