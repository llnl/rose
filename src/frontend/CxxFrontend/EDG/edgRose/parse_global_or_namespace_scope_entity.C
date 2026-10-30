
#include "edgRose.h"

SgDeclarationStatement*
EDG_ROSE_Translation::parse_global_or_namespace_scope_entity(SourceSequenceList& sse)
   {
     SgDeclarationStatement* result = NULL;

     an_il_entry_kind kind = (an_il_entry_kind)(sse->entity.kind);
     SgScopeStatement* scope = topScopeStack();

#ifdef DEBUG_CONVERSION
     if (DEBUG_ROSE_EDG_CONNECTION)
        {
          printf ("\n\n>>>>> At TOP  of parse_global_or_namespace_scope_entity(): top of scope = %p = %s ptr = %p summarize_entry_kind_in_string_kind(kind = %d) = %s \n",scope,scope->class_name().c_str(),sse->entity.ptr,kind,summarize_entry_kind_in_string_kind(kind));
        }
#endif

#if 0
  // printf ("In parse_global_or_namespace_scope_entity(): global_source_sequence_number = %" PRIuPTR " \n",global_source_sequence_number);
     printf ("In parse_global_or_namespace_scope_entity(): global_source_sequence_number = %u \n",global_source_sequence_number);
#endif

#if 0
     printf ("In parse_global_or_namespace_scope_entity(): entry_kind = %s \n",summarize_entry_kind_in_string_kind(kind));
#endif

     switch (kind)
        {
          case iek_src_seq_secondary_decl:
#ifdef DEBUG_CONVERSION
               indent_line(); debug_print("global_or_namespace_scope_entity: case iek_src_seq_secondary_decl"); debug_print("\n");
#endif
               result = parse_secondary_declaration(sse);
#if 0
               result->get_startOfConstruct()->display("After parse_secondary_declaration(): SgVariableDeclaration decl: START: debug");
               result->get_endOfConstruct()  ->display("After parse_secondary_declaration(): SgVariableDeclaration decl: END: debug");
#endif
#if 0
               printf ("In parse_global_or_namespace_scope_entity(): case iek_src_seq_secondary_decl: found a secondary declaration \n");

               printf ("Exiting as a test! \n");
               ROSE_ASSERT(false);
#endif
               break;

          case iek_type:
#ifdef DEBUG_CONVERSION
               indent_line(); debug_print("global_or_namespace_scope_entity: case iek_type"); debug_print("\n");
#endif
               result = parse_type_declaration(sse);

            // DQ (2/27/2012): Tracing down parent pointer set incorrectly.
               if (result != NULL)
                  {
                    if (result->get_parent() == NULL)
                       {
#if 0
                      // DQ (3/21/2012): This was only temporary code.
                      // result->set_parent(scope);
                         printf ("*** ERROR: parent set to self! \n");
                         result->set_parent(result->get_file_info());
#else
#ifdef DEBUG_CONVERSION
                         printf ("In parse_global_or_namespace_scope_entity(): result->get_parent() == NULL: result = %p = %s \n",result,result->class_name().c_str());
#endif
#endif
                       }
                      else
                       {
#if 0
                         printf ("In parse_global_or_namespace_scope_entity(): result->get_parent() = %p = %s: result = %p = %s \n",result->get_parent(),result->get_parent()->class_name().c_str(),result,result->class_name().c_str());
#endif
                       }

                 // DQ (3/21/2012): OK to not have the parent set, but make sure the scope is known.
                 // ROSE_ASSERT(result->get_parent() != NULL);
                    ROSE_ASSERT(result->get_scope() != NULL);
                  }
               break;

          case iek_routine:
             {
#ifdef DEBUG_CONVERSION
               indent_line(); debug_print("global_or_namespace_scope_entity: case iek_routine"); debug_print("\n");
#endif
            // DQ (4/21/2012): Added support for cases where this is a template.
            // result = parse_routine(sse);
               bool forceTemplateDeclaration = false;
               result = parse_routine(sse,forceTemplateDeclaration);
#if 0
               printf ("global_or_namespace_scope_entity: case iek_routine: result->get_scope() = %p = %s \n",result->get_scope(),result->get_scope()->class_name().c_str());
#endif
               if (scope != result->get_scope())
                  {
#ifdef DEBUG_CONVERSION
                    printf ("global_or_namespace_scope_entity: case iek_routine: The scope of the function does not match where it is being placed in the AST (OK for member functions). \n");
                    printf ("   --- scope               = %p = %s = %s \n",scope,scope->class_name().c_str(),SageInterface::generateUniqueName(scope,true).c_str());
                    printf ("   --- result->get_scope() = %p = %s = %s \n",result->get_scope(),result->get_scope()->class_name().c_str(),SageInterface::generateUniqueName(result->get_scope(),true).c_str());
#endif
                  }

               break;
             }

          case iek_variable:
#ifdef DEBUG_CONVERSION
               indent_line(); debug_print("global_or_namespace_scope_entity: case iek_variable"); debug_print("\n");
#endif
#if 0
               printf ("Calling parse_variable_declaration(sse) as part of global_or_namespace_scope_entity: case iek_variable \n");
#endif
               result = parse_variable_declaration(sse);

            // DQ (5/25/2015): The ending source position is computed incorrectly when there are multiple 
            // declarations of the same variable (see test2015_160.c).
            // Note that it might be useful to consider an internal analysis that would connect all of the
            // variables that are repeated declarations.  The mechanism to reset the first declaration 
            // (primary declaration) would then use this information to know when the ending position most 
            // likely needs to be reset.  However, in practice, repeated declarations of non-definition 
            // variables is a bit rare. Note that secondary declarations are set using information specific 
            // to the secondary declaration's source position.
               ROSE_ASSERT(result->get_startOfConstruct() != NULL);
               ROSE_ASSERT(result->get_endOfConstruct()   != NULL);
               if (result->get_startOfConstruct()->get_line() < result->get_endOfConstruct()->get_line())
                  {
#if 0
                    printf ("global_or_namespace_scope_entity: case iek_variable: Identified variable marked with starting postion on line = %d and ending position on line = %d \n",
                         result->get_startOfConstruct()->get_line(),result->get_endOfConstruct()->get_line());
#endif
                    *(result->get_endOfConstruct()) = *(result->get_startOfConstruct());
                    SgVariableDeclaration* variableDeclaration = isSgVariableDeclaration(result);
                    ROSE_ASSERT(variableDeclaration != NULL);
                    SgInitializedName* initializedName = SageInterface::getFirstInitializedName(variableDeclaration);
                    ROSE_ASSERT(initializedName != NULL);
                    unsigned int variableLength = initializedName->get_name().get_length();
                    if (variableLength > 0)
                       {
                      // Subtract one so that we get the addition correct: end position = start position + (variable length - 1).
                         variableLength -= 1;
#if 0
                         printf ("Reset the variable ending position: variableLength = %d \n",variableLength);
#endif
                      // Set the position of the column to reflect the size of the variable (which has to be on the same line as the start of the variable name).
                         result->get_endOfConstruct()->set_col(result->get_startOfConstruct()->get_col()+variableLength);

                      // Plus set the ending position in the SgInitializedName.
                         *(initializedName->get_endOfConstruct()) = *(result->get_endOfConstruct());
#if 0
                         result->get_startOfConstruct()->display("global_or_namespace_scope_entity: case iek_variable: After parse_variable_declaration(): reset the ending position of the variable: START: debug");
                         result->get_endOfConstruct()  ->display("global_or_namespace_scope_entity: case iek_variable: After parse_variable_declaration(): reset the ending position of the variable: END: debug");
#endif
                       }
                  }
#if 0
               result->get_startOfConstruct()->display("After parse_variable_declaration(): SgVariableDeclaration decl: START: debug");
               result->get_endOfConstruct()  ->display("After parse_variable_declaration(): SgVariableDeclaration decl: END: debug");
#endif
               break;

          case iek_macro:
#ifdef DEBUG_CONVERSION
            // Supress this debugging information.
               indent_line(); debug_print("global_or_namespace_scope_entity: case iek_macro"); debug_print("\n");
#endif
               result = parse_macro(sse);
               break;

          case iek_pragma:
#ifdef DEBUG_CONVERSION
               indent_line(); debug_print("global_or_namespace_scope_entity: case iek_pragma"); debug_print("\n");
#endif
               result = parse_pragma(sse);
               break;

          case iek_asm_entry:
             {
#ifdef DEBUG_CONVERSION
               indent_line(); debug_print("global_or_namespace_scope_entity: case iek_asm_entry"); debug_print("\n");
#endif
            // DQ (12/30/2008): See test2007_98.C
            // printf ("In parse_global_or_namespace_scope_entity(): calling summarize_asm_entry() \n");
               ROSE_ASSERT(sse->entity.ptr != NULL);
            // summarize_asm_entry((an_asm_entry*)(sse->entity.ptr));
            // printf ("DONE: calling summarize_asm_entry() \n");

               result = parse_asm(sse);
               break;
             }

          case iek_namespace:
             {
#ifdef DEBUG_CONVERSION
               indent_line(); debug_print("global_or_namespace_scope_entity: case iek_namespace"); debug_print("\n");
#endif
               result = parse_namespace(sse);
#if 0
               SgNamespaceDeclarationStatement* namespaceDeclaration = isSgNamespaceDeclarationStatement(result);
               ROSE_ASSERT(namespaceDeclaration != NULL);
               printf ("global_or_namespace_scope_entity: namespace declaration: name = %s at (%d,%d) -> (%d,%d) \n",namespaceDeclaration->get_name().str(),
                    namespaceDeclaration->get_startOfConstruct()->get_line(),namespaceDeclaration->get_startOfConstruct()->get_col(),
                    namespaceDeclaration->get_endOfConstruct()->get_line(),namespaceDeclaration->get_endOfConstruct()->get_col());
#endif
               break;
             }

          case iek_template:
             {
            // DQ (11/19/2011): Adding support for templates
#ifdef DEBUG_CONVERSION
               indent_line(); debug_print("global_or_namespace_scope_entity: case iek_template"); debug_print("\n");
#endif
               result = parse_template(sse);
               break;
             }

          case iek_field:
             {
            // DQ (11/19/2011): Adding support for templates (filed declaration using template parameter).
#ifdef DEBUG_CONVERSION
               indent_line(); debug_print("global_or_namespace_scope_entity: case iek_field"); debug_print("\n");
#endif
               result = parse_field_declaration(sse);
               break;
             }

          case iek_using_decl:
             {
#ifdef DEBUG_CONVERSION
               indent_line(); debug_print("global_or_namespace_scope_entity: case iek_using_decl"); debug_print("\n");
#endif
               result = parse_using_declaration(sse);
               ROSE_ASSERT(result != NULL);
               break;
             }

          case iek_instantiation_directive:
             {
#ifdef DEBUG_CONVERSION
               indent_line(); debug_print("global_or_namespace_scope_entity: case iek_instantiation_directive"); debug_print("\n");
#endif
               result = parse_instantiation_directive(sse);
               ROSE_ASSERT(result != NULL);
               break;
             }

#define HANDLE_SOURCE_SEQUENCE_END 1
#if HANDLE_SOURCE_SEQUENCE_END
       // DQ (12/9/2011): Readded...test2004_35.C
          case iek_src_seq_end_of_construct:
             {
            // DQ (11/19/2011): Adding support for templates.

#ifdef DEBUG_CONVERSION
               printf ("Processing case iek_src_seq_end_of_construct (part of new template support) \n");
#endif
            // check_end_of_construct(iek_type, t, sse);
               ++sse;
               break;
            // goto end;
             }
#endif

          case iek_static_assertion:
             {
#ifdef DEBUG_CONVERSION
               indent_line(); debug_print("global_or_namespace_scope_entity: case iek_static_assertion"); debug_print("\n");
#endif
#if 1
               result = parse_static_assertion_declaration(sse);
#else
            // an_il_entry_kind kind = (an_il_entry_kind)(sse->entity.kind);
               a_static_assertion_ptr static_assertion = (a_static_assertion_ptr)(sse->entity.ptr);
               ROSE_ASSERT(static_assertion != NULL);

#error "DEAD CODE!"

#if 0
               printf ("In global_or_namespace_scope_entity: case iek_static_assertion: calling summarize_static_assertion() \n");
               summarize_static_assertion(static_assertion);
#endif
            // I think we need to write a parse_static_assertion() function.
               ROSE_ASSERT(static_assertion->condition      != NULL);
               SgExpression* constant_condition = convert_constant(static_assertion->condition).value();
               ROSE_ASSERT(constant_condition != NULL);

               ROSE_ASSERT(static_assertion->string_literal != NULL);
               SgExpression* constant_string_literal = convert_constant(static_assertion->string_literal).value();
               ROSE_ASSERT(constant_string_literal != NULL);

               printf ("In global_or_namespace_scope_entity: case iek_static_assertion: constant_string_literal = %p = %s \n",constant_string_literal,constant_string_literal->class_name().c_str());

               SgStringVal* stringValue = isSgStringVal(constant_string_literal);
               ROSE_ASSERT(stringValue != NULL);

#error "DEAD CODE!"

               SgName string_literal    = stringValue->get_value();

               printf ("In global_or_namespace_scope_entity: case iek_static_assertion: string_literal = %s \n",string_literal.str());

            // SgStaticAssertionDeclaration* staticAssertionDeclaration = SageBuilder::buildStaticAssertionDeclaration(constant_condition, "test string literal");
               SgStaticAssertionDeclaration* staticAssertionDeclaration = SageBuilder::buildStaticAssertionDeclaration(constant_condition,string_literal);
               ROSE_ASSERT(staticAssertionDeclaration != NULL);

            // Set the parent.
               ROSE_ASSERT(staticAssertionDeclaration->get_parent() == NULL);
               staticAssertionDeclaration->set_parent(topScopeStack());
               ROSE_ASSERT(staticAssertionDeclaration->get_parent() != NULL);

               result = staticAssertionDeclaration;
               ROSE_ASSERT(result != NULL);

#error "DEAD CODE!"

               ++sse;
#endif
#if 0
               printf ("Error: case iek_static_assertion: not implemented \n");
               ROSE_ASSERT(false);
#endif
               break;
             }

#if MICROSOFT_EXTENSIONS_ALLOWED
          case iek_ms_attribute:
             {
#if 0
               printf ("In global_or_namespace_scope_entity: case iek_ms_attribute: Defining case iek_ms_attribute to return (ms_attribute = %p) \n",ms_attribute);
#endif
               result = parse_microsoft_attribute(sse);
#if 0
               printf ("Error: case iek_ms_attribute: not implemented \n");
               ROSE_ASSERT(false);
#endif
               break;
             }
#endif

   // DQ (12/29/2008): Added EDG 4.0 cases not handled
          case iek_none:
          case iek_source_file:
          case iek_constant:
          case iek_param_type:
          case iek_routine_type_supplement:
          case iek_based_type_list_member:
          case iek_exception_specification:
          case iek_exception_specification_type:
          case iek_label:
          case iek_expr_node:
          case iek_for_loop:
          case iek_switch_case_entry:
          case iek_switch_stmt_descr:
          case iek_handler:
          case iek_try_supplement:
#if MICROSOFT_EXTENSIONS_ALLOWED
          case iek_microsoft_try_supplement:
       // case iek_ms_attribute:
          case iek_ms_attribute_arg:
          case iek_ms_if_exists:
#endif
          case iek_block:
          case iek_statement:
          case iek_object_lifetime:
          case iek_scope:
          case iek_id_name:
          case iek_string_text:
          case iek_other_text:
          case iek_internal_complex_value:
       // case iek_using_decl:
          case iek_dynamic_init:
          case iek_local_static_variable_init:
          case iek_vla_dimension:
          case iek_overriding_virtual_function:
          case iek_derivation_step:
          case iek_base_class_derivation:
          case iek_base_class:
          case iek_class_list_entry:
          case iek_routine_list_entry:
          case iek_class_type_supplement:
          case iek_template_param_type_supplement:
          case iek_constructor_init:
#if GNU_EXTENSIONS_ALLOWED
          case iek_asm_operand:
#if !RECORD_RAW_ASM_OPERAND_DESCRIPTIONS
          case iek_asm_operand_constraint:
#endif
          case iek_named_register_list:
#endif
          case iek_template_arg:
          case iek_new_delete_supplement:
          case iek_throw_supplement:
          case iek_condition_supplement:
          case iek_source_sequence_entry:
#if !HANDLE_SOURCE_SEQUENCE_END
          case iek_src_seq_end_of_construct:
#endif
          case iek_src_seq_sublist:
       // case iek_instantiation_directive:
          case iek_scope_orphaned_list_header:
          case iek_decl_position_supplement:
          case iek_template_decl:
          case iek_template_parameter:
          case iek_seq_number_lookup_entry:
          case iek_local_expr_node_ref:
       // case iek_static_assertion:
          case iek_local_scope_ref:
          case iek_il_entity_list_entry:

       // DQ (7/21/2012): Added lambda cases (but might not be required here).
          case iek_lambda:
          case iek_lambda_capture:
          case iek_name_reference:
          case iek_name_qualifier:
          case iek_last:
             {
               printf ("Case not handled! kind = %s \n",il_entry_kind_names[kind]);
               ROSE_ASSERT(false);
               break;
             }

          default: debug_print("parse_global_or_namespace_scope_entity: %d\n", kind); abort();
        }

// Note label
// end: ;

  // SgScopeStatement* last_scope = topScopeStack();
#if 0
     if (sse.isNull() == false)
        {
          an_il_entry_kind last_kind = (an_il_entry_kind)(sse->entity.kind);
          printf (">>>>> At BASE of parse_global_or_namespace_scope_entity(): top of scope = %p = %s ptr = %p summarize_entry_kind_in_string_kind(kind = %d) = %s \n",
               last_scope,last_scope->class_name().c_str(),sse->entity.ptr,last_kind,summarize_entry_kind_in_string_kind(last_kind));
        }
       else
        {
          printf (">>>>> At BASE of parse_global_or_namespace_scope_entity(): top of scope = %p = %s *** sse == NULL *** \n",last_scope,last_scope->class_name().c_str());
        }
#endif

  // DQ (1/11/2009): Need to know when we are returning NULL pointers (enum declaration in test2007_27.C should not).
     if (result != NULL)
        {
#if 0
          printf ("################ In parse_global_or_namespace_scope_entity(): result = %p = %s \n",result,result->class_name().c_str());
#endif
#if 0
          printf ("################ In parse_global_or_namespace_scope_entity(): result = %p = %s \n",result,result->class_name().c_str());
          SgFunctionDeclaration* functionDeclaration = isSgFunctionDeclaration(result);
          if (functionDeclaration != NULL)
             {
               printf ("   --- functionDeclaration = %s \n",functionDeclaration->get_name().str());
             }
#endif
          ROSE_ASSERT(result->get_file_info() != NULL);
#if 0
          result->get_file_info()->display("source position in parse_global_or_namespace_scope_entity()");
#endif
        }

  // Do tests on the return result
     if (result != NULL)
        {
          ROSE_ASSERT(result->get_file_info()        != NULL);
          ROSE_ASSERT(result->get_startOfConstruct() != NULL);
          ROSE_ASSERT(result->get_endOfConstruct()   != NULL);

       // DQ (3/3/2012): Added test for matching variants.
          ROSE_ASSERT(matching_defining_and_nondefining_variants(result) == true);
        }

     return result;
   }
