
#include "edgRose.h"

void
EDG_ROSE_Translation::parse_enumerators(SourceSequenceList& sse, SgEnumDeclaration* decl, a_type_ptr t)
   {
#if 0
     printf ("In parse_enumerators(): decl = %p \n",decl);
#endif

     while (sse)
        {
          an_il_entry_kind kind = (an_il_entry_kind)(sse->entity.kind);
#if 0
          printf ("In parse_enumerators(): an_il_entry_kind kind = %s \n",summarize_entry_kind_in_string_kind(kind));
          printf ("   --- entry = %p kind = %s \n",sse->entity.ptr,summarize_entry_kind_in_string_kind(sse->entity).c_str());
#endif
          switch (kind)
             {
               case iek_constant:
                  {
                    a_constant_ptr p = (a_constant_ptr)(sse->entity.ptr);
#if 0
                    printf ("In parse_enumerators(): calling convert_constant_in_enumerator()! \n");
#endif
                    SgExpression* value = convert_constant_in_enumerator(p);
                    ROSE_ASSERT (value != NULL);
                    ++sse;
#if 0
                    printf ("In parse_enumerators(): build the SgInitializedName for the enum value! \n");
#endif
                    SgName name = p->source_corresp.name;
#if 0
                    printf ("In parse_enumerators(): build the SgInitializedName for the enum value: using topScopeStack() \n");
#endif
                    SgScopeStatement* scope = topScopeStack();
                    ROSE_ASSERT(scope != NULL);

                 // DQ (5/14/2018): Need to fixup the scope when this is a C++11 enum class.
                    if (t->variant.integer.is_scoped_enum == true)
                       {
                      // Use the declaration scope as the location for symbols when this is a C++11 enum class declaration.
                         ROSE_ASSERT(internal_intermediate_scope != NULL);
#if 0
                         printf ("internal_intermediate_scope->get_symbol_table()->size() = %d \n",internal_intermediate_scope->get_symbol_table()->size());
#endif
                         scope = internal_intermediate_scope;
#if 0
                         printf ("Exiting as a test! \n");
                         ROSE_ASSERT(false);
#endif
                       }

#if 0
                    printf ("In parse_enumerators(): calling buildAssignInitializer_nfi(name = %s, scope = %p = %s) p = %p \n",name.str(),scope,scope->class_name().c_str(),p);
#endif
                 // SgInitializedName* initializedName = set_file_info_for_corresp(buildInitializedName_nfi(p->source_corresp.name, value->get_type(), set_file_info_for_enumerator(buildAssignInitializer_nfi(value), p)), p->source_corresp);
                 // SgInitializedName* initializedName = set_file_info_for_corresp(buildInitializedName_nfi(name, value->get_type(), set_file_info_for_enumerator(buildAssignInitializer_nfi(value), p)), p->source_corresp);
                    SgAssignInitializer* assignInitializer = buildAssignInitializer_nfi(value);
                    ROSE_ASSERT(assignInitializer->get_startOfConstruct() != NULL);
                    ROSE_ASSERT(assignInitializer->get_endOfConstruct() != NULL);
                    SgInitializedName* initializedName = set_file_info_for_corresp(buildInitializedName_nfi(name, value->get_type(), set_file_info_for_enumerator(assignInitializer, p)), p->source_corresp);

                    ROSE_ASSERT(decl != NULL);
                    decl->get_enumerators().push_back(initializedName);

                    initializedName->set_parent(decl);
                 // initializedName->set_scope(topScopeStack());
                    initializedName->set_scope(scope);

                    initializedName->set_declptr(decl);
#if 0
                    printf ("In parse_enumerators(): Put the name in = %p = %s in the symbol table \n",initializedName,initializedName->get_name().str());
#endif
                 // DQ (6/10/2012): Add an asociated symbol to the symbol table (see test2012_102.C).
                    SgEnumFieldSymbol* enumFieldSymbol = scope->lookup_enum_field_symbol(name);

                 // DQ (1/26/2013): We had code to almost handle the case of the symbol not being present in the scope, but it needs 
                 // to be conditional on finding it already present or not.  This is an issue in compiling ROSE with ROSE.

                 // ROSE_ASSERT(enumFieldSymbol == NULL);
                    if (enumFieldSymbol == NULL)
                       {
                         enumFieldSymbol = new SgEnumFieldSymbol(initializedName);

                      // DQ (1/26/2013): We only want to insert the associated symbol if we din't find it already present in the associated scope.
                         scope->insert_symbol(name,enumFieldSymbol);
                       }
                    ROSE_ASSERT(enumFieldSymbol != NULL);

                 // DQ (1/26/2013): We don't want to insert the associated symbol unconditionally!
                 // scope->insert_symbol(name,enumFieldSymbol);

                    break;
                  }

               case iek_macro: ++sse; break; // FIXME: do something else?

            // A "#pragma" inside an enum body (e.g. "#pragma clang diagnostic ..." in libc++ headers).
            // There is no place for a SgPragmaDeclaration among enumerators, so it is skipped.
               case iek_pragma: ++sse; break;

               case iek_src_seq_end_of_construct:
                  {
                 // DQ (10/29/2012): This logic may be effected by the additional complexity of compiler 
                 // generated iek_src_seq_end_of_construct associated with variable declarations that 
                 // define types (such as those in GNU statement expressions).

#if 0
                 // DQ (8/17/2012): I think that the source position of the closing scope is contained in here (so we don't want to ignore these).
                    if (sse->entity.kind == iek_src_seq_end_of_construct)
                       {
                         printf ("@@@@@@@@@@@@@@@@@ In parse_enumerators(): case iek_src_seq_end_of_construct: \n");
                       }
#endif
                    check_end_of_construct(iek_type, t, sse);
                    ++sse;
                    goto end;
                  }

            // DQ (12/2/2013): Testing busybox source code.
               case iek_type:
                  {
                 // printf ("In parse_enumerators(): Found unimplemented case iek_type \n");
                    ROSE_ASSERT(t != NULL);
#if 0
                    bool pointerToMember = false;
                    SgType* type = convert_type(t,pointerToMember);
#endif
#if 0
                 // SgClassDeclaration* parse_structlike(SourceSequenceList& sse, a_type_ptr p, SgClassDeclaration::class_types t, bool forceTemplateClassDeclaration = false, a_template_ptr edg_template = NULL);
                    bool forceTemplateClassDeclaration = false;
                    a_template_ptr edg_template = NULL;
                    SgClassDeclaration* classDeclaration = parse_structlike(sse,t, SgClassDeclaration::e_union,forceTemplateClassDeclaration,edg_template);
#endif
                 // SgClassDeclaration* decl = convert_struct_primary(p, t, NULL);
                 // SgDeclarationStatement* s = convert_type_as_secondary_decl(sse);
                 // ROSE_ASSERT(s != NULL);
                 // ROSE_ASSERT(s->get_scope() != NULL);

                    SgStatement* stmt = parse_type_declaration(sse);
                 // ROSE_ASSERT(stmt != NULL);
                 // ROSE_ASSERT(stmt == NULL);
                    if (stmt != NULL)
                       {
                      // Not clear what to do in this case (but test2013_103.c demonstrates this).
                       }
                      else
                       {
                      // Not clear what to do in this case (but test2013_101.c demonstrates this).
                       }
#if 0
                    printf ("Exiting as a test! \n");
                    ROSE_ASSERT(false);
#endif
                 // DQ (12/2/2013): Testcode test2013_101.c demonstrates that this should be commented out.
                 // ++sse;
                    break;
                  }

#if MICROSOFT_EXTENSIONS_ALLOWED
               case iek_ms_attribute:
                  {
                 // DQ (8/15/2014): I don't think this case can happen (unable to build example test code).
                    printf ("In parse_enumerators: case iek_ms_attribute: Defining case iek_ms_attribute to return (result == NULL) \n");

                    ++sse;
#if 1
                    printf ("Error: case iek_ms_attribute: not implemented here (should not exist in this location) \n");
                    ROSE_ASSERT(false);
#endif
                    break;
                  }
#endif

            // DQ (1/29/2019): Added case to support Cxx11_tests/test2019_52.C).
               case iek_src_seq_secondary_decl:
                  {
#if 0
                    printf ("In parse_enumerators: case iek_src_seq_secondary_dec: (class declaration referenced ahead of defining declaration within enum declaration \n");
#endif
                 // DQ (1/29/2019): This causes Cxx11_tests/test2019_52.C to generate "e = sizeof( (struct A*)0 )" as "e=8" 
                 // (or at leasst unparse the code as "e=8".  This might even be correct or semantically equivalent, but that 
                 // might require additional research into this.  At the moment this is at least no longer a bug in the frontend, 
                 // and appears to unparse to a semantically equivelent version of the input (original) code.
                    ++sse;
#if 0
                    printf ("Error: case iek_src_seq_secondary_dec: not implemented here (should not exist in this location) \n");
                    ROSE_ASSERT(false);
#endif
                    break;
                  }

   // DQ (12/29/2008): Added EDG 4.0 cases not handled
               case iek_none:
               case iek_source_file:
               case iek_param_type:
               case iek_routine_type_supplement:
               case iek_based_type_list_member:
            // case iek_type:
               case iek_variable:
               case iek_field:
               case iek_exception_specification:
               case iek_exception_specification_type:
               case iek_routine:
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
               case iek_namespace:
               case iek_using_decl:
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
               case iek_asm_entry:
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
            // case iek_src_seq_secondary_decl:
               case iek_src_seq_sublist:
               case iek_instantiation_directive:
               case iek_scope_orphaned_list_header:
               case iek_template:
               case iek_decl_position_supplement:
               case iek_template_decl:
               case iek_template_parameter:
               case iek_seq_number_lookup_entry:
               case iek_local_expr_node_ref:
               case iek_static_assertion:
               case iek_local_scope_ref:
               case iek_il_entity_list_entry:

   // DQ (7/21/2012): lambda cases that we want to cover (but might not be used here).
               case iek_lambda:
               case iek_lambda_capture:

               case iek_name_reference:
               case iek_name_qualifier:
               case iek_last:
                  {
                 // printf ("Case not handled! \n");
                    printf ("Case not handled! kind = %s \n",summarize_entry_kind_in_string_kind(kind));
                 // Advance so that a build without assertions cannot loop forever on this entry.
                    ++sse;
                    ROSE_ASSERT(false);
                    break;
                  }

               default: ERROR_MESSAGE(stderr, "Bad IL node kind in parse_enumerators %d\n", kind); abort();
             }
        }
end: ;

  // TV (04/20/11): Comment this as an enum can be the last statement of a source file.
  // ROSE_ASSERT (sse);

#if 0
     printf ("Leaving parse_enumerators(): decl = %p \n",decl);
#endif
   }
