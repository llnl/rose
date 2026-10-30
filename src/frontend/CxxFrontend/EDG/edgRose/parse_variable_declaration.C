
#include "edgRose.h"

#define DEBUG__parse_variable_declaration 0

namespace EDG_ROSE_Translation {

SgVariableDeclaration * parse_variable_declaration(SourceSequenceList& sse) {
  ROSE_ASSERT (sse->entity.kind == iek_variable);
  a_variable_ptr edg_var_ptr = (a_variable_ptr)(sse->entity.ptr);
  ++sse;
  ROSE_ASSERT(edg_var_ptr != NULL);

#if DEBUG__parse_variable_declaration
  printf("Enter parse_variable_declaration()\n");
  printf("  edg_var_ptr = %p\n", edg_var_ptr);
  printf("    ->type          = %p\n", edg_var_ptr->type);
  printf("    ->declared_type = %p\n", edg_var_ptr->declared_type);
  printf("    ->is_constexpr         = %s\n", edg_var_ptr->is_constexpr ? "true" : "false");
  printf("    ->is_template_variable = %s\n", edg_var_ptr->is_template_variable ? "true" : "false");
  printf("    ->is_member_constant   = %s\n", edg_var_ptr->is_member_constant ? "true" : "false");
  printf("    ->is_thread_local      = %s\n", edg_var_ptr->is_thread_local ? "true" : "false");
  printf("    ->has_direct_braced_initializer = %s\n", edg_var_ptr->has_direct_braced_initializer ? "true" : "false");
  printf("    ->has_parenthesized_initializer = %s\n", edg_var_ptr->has_parenthesized_initializer ? "true" : "false");
#endif

  a_seq_number variable_declaration_source_sequence_number = edg_var_ptr->source_corresp.decl_position.seq;
  global_source_sequence_number = variable_declaration_source_sequence_number;

  bool suppress_variable_declaration_normalization = global_suppress_variable_declaration_normalization;
  if (suppress_variable_declaration_normalization && edg_var_ptr->is_template_variable) {
    suppress_variable_declaration_normalization = false;
  }

  bool addingVariableToExistingVariableDeclaration = false;
  SgVariableDeclaration* temp_decl = NULL;
  do {
    if (edg_var_ptr->source_corresp.parent_scope != NULL) {
      process_possible_typeof_expression_arguments(edg_var_ptr->source_corresp.parent_scope);
    }
    ROSE_ASSERT(edg_var_ptr->type != NULL);
    a_seq_number current_source_sequence_number = edg_var_ptr->source_corresp.decl_position.seq;

    // DQ (11/6/2014): Adding C++11 support to record the use of C++11 syntax (for the unparser).
    // if (edg_var_ptr->has_direct_braced_initializer == true) {}

    if (addingVariableToExistingVariableDeclaration) {
      ROSE_ASSERT(temp_decl != NULL);
    } else {
      ROSE_ASSERT(temp_decl == NULL);
    }

    if (current_source_sequence_number == parse_variable_declaration_last_source_sequence_number) {
      SgInitializedName* last_var = NULL;
      if (suppress_variable_declaration_normalization == false) {
        ROSE_ASSERT(addingVariableToExistingVariableDeclaration == false);

        ROSE_ASSERT(temp_decl == NULL);
        temp_decl = convert_variable_prototype(edg_var_ptr, NULL);
        ROSE_ASSERT(temp_decl != NULL);

        last_var = getFirstInitializedName(temp_decl);
        ROSE_ASSERT(last_var != NULL);
      } else {
        ROSE_ASSERT(temp_decl != NULL);
        ROSE_ASSERT(addingVariableToExistingVariableDeclaration);

        SgInitializedName* first_var = getFirstInitializedName(temp_decl);

        ROSE_ASSERT(first_var != NULL);
        ROSE_ASSERT(first_var->get_scope() != NULL);

        SgName name = generateName(edg_var_ptr);

        a_type_ptr edg_variable_type = edg_var_ptr->declared_type ? edg_var_ptr->declared_type : edg_var_ptr->type;
        ROSE_ASSERT(edg_variable_type != NULL);

        SgType* type = convert_type(edg_variable_type);

        SgInitializer* init = NULL;
        if (edg_var_ptr->is_member_constant == false) {
          init = convert_var_initializer(edg_var_ptr);
        }

        last_var = buildInitializedName_nfi(name,type,init);

        SgScopeStatement* scope = getDeclarationScope(edg_var_ptr->source_corresp);
        ROSE_ASSERT(scope != NULL);

        SgVariableSymbol* variableSymbol = new SgVariableSymbol(last_var);
        scope->insert_symbol(name,variableSymbol);

        last_var->set_parent(parse_variable_declaration_last_decl);

        last_var->set_scope(topScopeStack());

        parse_variable_declaration_last_decl->get_variables().push_back(last_var);

        // Liao, 1/20/2016, patch up SgVariableDefinition , similar code from SgVariableDeclaration::append_variable()
        SageBuilder::buildVariableDefinition_nfi (parse_variable_declaration_last_decl, last_var, init);
      }
      ROSE_ASSERT(temp_decl != NULL);
      ROSE_ASSERT(last_var != NULL);

      if (suppress_variable_declaration_normalization == false) {
        check_types_and_set_isAssociatedWithDeclarationList(parse_variable_declaration_last_decl, temp_decl);
      }
    } else {
      ROSE_ASSERT(temp_decl == NULL);
      temp_decl = convert_variable_prototype(edg_var_ptr, NULL);
      ROSE_ASSERT(temp_decl != NULL);
    }

    ROSE_ASSERT(temp_decl != NULL);
    ROSE_ASSERT(temp_decl->get_parent() != NULL);

    SgVariableDeclaration* decl = temp_decl;
    ROSE_ASSERT(decl != NULL);

    SgStorageModifier::storage_modifier_enum sm = convert_storage_class(edg_var_ptr->declared_storage_class);
#if ENABLE_CUDA_SUPPORT
    set_variable_cuda_attributes(edg_var_ptr, decl);
#endif /* ENABLE_CUDA_SUPPORT */

    fixup_variable_or_field_declaration(decl, edg_var_ptr->type);

    if (current_source_sequence_number != parse_variable_declaration_last_source_sequence_number || suppress_variable_declaration_normalization == false) {
      SgInitializedName* var = getFirstInitializedName(decl);
      if (edg_var_ptr->has_parenthesized_initializer == false) {
        var->set_using_assignment_copy_constructor_syntax(true);
      }
      var->get_storageModifier().set_modifier(sm);
      decl->get_declarationModifier().get_storageModifier().set_modifier(sm);

#if ENABLE_CUDA_SUPPORT
      set_variable_cuda_attributes(edg_var_ptr, var);
#endif /* ENABLE_CUDA_SUPPORT */

#if GNU_EXTENSIONS_ALLOWED || REDEFINE_EXTNAME_PRAGMA_ENABLED
      if (edg_var_ptr->asm_name_is_valid == true && edg_var_ptr->asm_name_or_reg.name != NULL) {
        // This is a declaration of the form: "register int *p1 asm ("ax") = 0;"
        var->set_register_name_string(edg_var_ptr->asm_name_or_reg.name);
      }
#endif

#if GNU_EXTENSIONS_ALLOWED && NAMED_REGISTERS_ALLOWED
      if (edg_var_ptr->asm_name_is_valid == false && edg_var_ptr->has_named_register_storage_class == false) {
        // This is a declaration of the form: "register int *p1 asm ("ax") = 0;"
        var->set_register_name_code((SgInitializedName::asm_register_name_enum)edg_var_ptr->asm_name_or_reg.reg);
      }
#endif

      if (edg_var_ptr->declared_with_auto_type_specifier == true) {
        var->set_using_auto_keyword(true);
        ROSE_ASSERT(edg_var_ptr->declared_type != NULL);
        if (edg_var_ptr->declared_type != edg_var_ptr->type) {
          SgType * auto_type = convert_type(edg_var_ptr->declared_type,false, true);
          var->set_auto_decltype(auto_type);
        }
      }

      SgInitializer* init = NULL;
      if (edg_var_ptr->is_member_constant == false) {
        init = convert_var_initializer(edg_var_ptr);
      }
      if (init != NULL) {
        var->set_initializer(init);
        init->set_parent(var);
      }

      set_variable_attributes(edg_var_ptr, var);
      if (variable_cache.find(edg_var_ptr) == variable_cache.end()) {
        SgVariableSymbol* sym = getFirstVarSym(decl);
        ROSE_ASSERT(sym != NULL);
        variable_cache[edg_var_ptr] = sym;
      }
    } else {
      decl->get_declarationModifier().get_storageModifier().set_modifier(sm);
    }

    ROSE_ASSERT(decl != NULL);
    ROSE_ASSERT(decl->get_parent() != NULL);
    if (suppress_variable_declaration_normalization == false) {
      parse_variable_declaration_last_source_sequence_number = current_source_sequence_number;
      parse_variable_declaration_last_decl = decl;
    }

    addingVariableToExistingVariableDeclaration = false;

    if (suppress_variable_declaration_normalization == true && sse.isNull() == false && sse->entity.kind == iek_variable) {
      ROSE_ASSERT (sse->entity.kind == iek_variable);
      a_variable_ptr next_var_ptr = (a_variable_ptr)(sse->entity.ptr);
      ROSE_ASSERT(next_var_ptr != NULL);

      a_seq_number local_current_source_sequence_number = edg_var_ptr->source_corresp.decl_position.seq;
      a_seq_number local_next_source_sequence_number = next_var_ptr->source_corresp.decl_position.seq;

      if (local_current_source_sequence_number == local_next_source_sequence_number) {
        addingVariableToExistingVariableDeclaration = true;

        global_source_sequence_number = local_current_source_sequence_number;

        parse_variable_declaration_last_source_sequence_number = current_source_sequence_number;
        parse_variable_declaration_last_decl = decl;

        ++sse;

        edg_var_ptr = next_var_ptr;
      }
    }
  } while (addingVariableToExistingVariableDeclaration == true);

  if (edg_var_ptr->is_thread_local == true) {
    temp_decl->set_is_thread_local(true);
  }

  if (edg_var_ptr->is_constexpr == true) {
    temp_decl->set_is_constexpr(true);
  }

#if DEBUG__parse_variable_declaration
  printf("  temp_decl = %p = %s\n", temp_decl, temp_decl ? temp_decl->class_name().c_str() : "");
  printf("Leave parse_variable_declaration()\n");
#endif
  return temp_decl;
}

}

