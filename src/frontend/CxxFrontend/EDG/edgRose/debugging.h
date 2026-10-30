// Header file for debugging code 

#include <exprutil.h>

void summarize_constant(a_constant *cp);
void summarize_class ( a_type *type );
void summarize_src_seq_secondary_decl(a_src_seq_secondary_decl_ptr sec_decl);
void summarize_type     ( a_type *tp );
void summarize_routine  ( a_routine_ptr ptr );
void summarize_template ( a_template_ptr ptr );
void summarize_variable ( a_variable_ptr ptr );
void summarize_template_parameter ( a_template_parameter_ptr ptr );
void summarize_expr     ( an_expr_node_ptr ptr );
void summarize_name_reference ( a_name_reference_ptr ptr );
void summarize_statement ( a_statement_ptr ptr );
void summarize_dynamic_init ( a_dynamic_init_ptr ptr );
void summarize_namespace ( a_namespace_ptr ptr );
void summarize_using_declaration ( a_using_decl_ptr ptr );
void summarize_source_correspondence ( const a_source_correspondence & X );
void summarize_field ( a_field_ptr ptr );
void summarize_asm_entry ( an_asm_entry_ptr ptr );
void summarize_operand ( an_operand* ptr );
void summarize_scope ( a_scope_ptr ptr );
void summarize_param_type ( a_param_type_ptr ptr );
void summarize_lambda_expr (a_lambda_ptr ptr);
void summarize_lambda_capture (a_lambda_capture_ptr ptr);
void summarize_template_arg_list (a_template_arg_ptr arg_list);
void summarize_template_arg (a_template_arg_ptr ptr);
void summarize_switch_descriptor (a_switch_stmt_descr_ptr ptr);
void summarize_switch_case (a_switch_case_entry_ptr ptr);
void summarize_range_based_for_loop (a_range_based_for_loop_ptr ptr);
void summarize_label (a_label_ptr ptr);
void summarize_block (a_block_ptr ptr);
void summarize_pragma ( a_pragma_ptr ptr );
void summarize_new_delete_supplement ( a_new_delete_supplement_ptr ptr );
void summarize_symbol ( a_symbol_ptr ptr );
void summarize_exception_specification ( an_exception_specification_ptr ptr );
void summarize_exception_specification_type ( an_exception_specification_type_ptr ptr );
void summarize_routine_type_supplement ( a_routine_type_supplement_ptr ptr );
void summarize_source_position ( const a_source_position & sourcePosition );
void summarize_source_range ( const a_source_range & sourceRange );
void summarize_decl_pos_info ( const a_decl_position_supplement_ptr & ptr );
void summarize_source_sequence ( const a_seq_number sourceSequenceNumber );
void summarize_pack_expansion_descr (a_pack_expansion_descr_ptr ptr);
void summarize_macro ( a_macro_ptr ptr );
void summarize_il_header ( an_il_header & p );
void summarize_source_file ( const a_source_file_ptr & ptr );
void summarize_macro_invocation_record_block ( a_macro_invocation_record_block_ptr ptr );
void summarize_attribute_group ( an_attribute_group_ptr ptr );
void summarize_attribute_arg ( an_attribute_arg_ptr ptr );
void summarize_attribute ( an_attribute_ptr ptr );
void summarize_static_assertion ( a_static_assertion_ptr ptr );
void summarize_base_class ( a_base_class_ptr ptr );
void summarize_integer_type_supplement(an_integer_type_supplement_ptr ptr);
void summarize_constructor_init ( a_constructor_init_ptr ptr );
void summarize_scope_orphaned_list_header ( a_scope_orphaned_list_header_ptr ptr );
void summarize_macro_invocation_record (a_macro_invocation_record_ptr ptr);
void summarize_local_scope_ref ( a_local_scope_ref_ptr ptr );
void summarize_instantiation_directive ( an_instantiation_directive_ptr ptr );
void summarize_vla_dimension(a_vla_dimension_ptr ptr);
#if MICROSOFT_EXTENSIONS_ALLOWED
void summarize_ms_attribute ( an_ms_attribute_ptr ptr );
void summarize_ms_attribute_arg ( an_ms_attribute_arg_ptr ptr );
#endif
#if FULLY_RESOLVED_MACRO_POSITIONS || RECORD_MACRO_INVOCATIONS
void summarize_simple_source_position ( a_simple_source_position_ptr ptr );
#endif

std::string summarize_entry_kind_in_string_kind (const a_tagged_pointer & entry, bool includePointerValue = true );
std::string summarize_element_position_kind ( an_element_position_kind kind );
std::string summarize_qualifiers_in_string ( size_t qualifiers );
std::string summarize_expression_in_string (an_expr_node_ptr ptr);
std::string summarize_expr_operator (an_expr_operator_kind x);
std::string summarize_integer_string(an_integer_kind kind);
std::string summarize_local_expr_node_ref_string(a_local_expr_node_ref_kind kind);
std::string summarize_token_kind(a_token_kind kind);
std::string summarize_builtin_operation_kind_in_string(a_builtin_operation_kind kind);

char* summarize_string_literal_kind(a_string_or_char_literal_kind kind);
char* summarize_decl_modifier(a_decl_modifier kind);
char* summarize_pragma_kind_in_string (a_pragma_kind kind);
char* summarize_template_kind_in_string (a_pragma_kind kind);
char* summarize_templ_arg_kind_in_string(a_templ_arg_kind kind);
char* summarize_template_kind_in_string (a_template_kind kind);
char* summarize_expr_kind (an_expr_node_kind x);
char* summarize_storage_class_in_string(a_storage_class X);
char* summarize_operand_kind ( an_operand_kind kind );
char* summarize_template_param_constant_kind_in_string(a_template_param_constant_kind kind);
char* summarize_constant_kind_in_string ( a_constant_ptr ptr );
char* summarize_address_base_kind_in_string ( an_address_base_kind ptr );
char* summarize_init_kind_in_string ( an_init_kind kind );
char* summarize_scope_in_string         ( a_scope_ptr ptr );
char* summarize_symbol_kind_in_string(a_symbol_ptr ptr);
char* summarize_symbol_kind_in_string_kind(a_symbol_kind kind);
char* summarize_type_kind_in_string     ( a_type_ptr ptr );
char* summarize_type_kind_in_string_kind     ( a_type_kind kind );
char* summarize_constant_in_string      ( a_constant *cp );
char* summarize_statement_in_string     ( a_statement_ptr ptr );
char* summarize_base_type_kind_in_string( a_based_type_list_member_ptr basedType );
char* summarize_template_parameter_kind_in_string ( a_template_parameter_ptr ptr );
char* summarize_float_kind_in_string ( a_float_kind kind );
const char* summarize_entry_kind_in_string_kind    ( an_il_entry_kind entry_kind );

std::string summarizeTypeModifier            ( const a_type_qualifier_set    & modifiers );
std::string summarizeStorageModifier         ( const a_storage_class         & modifiers );
std::string summarizeAccessModifier          ( const an_access_specifier     & modifiers );
std::string summarizeLinkageModifier         ( const a_name_linkage_kind     & modifiers );
std::string summarizeSpecialFunctionModifier ( const a_special_function_kind & modifiers );
std::string summarizeDynamicInitialization   ( const a_dynamic_init_kind & x );
#if UPC_EXTENSIONS_ALLOWED
std::string summarizeUPC_Modifier            ( const a_upc_access_method     & modifiers );
#endif

std::string attribute_kind_to_string (an_attribute_kind kind);
std::string attribute_family_to_string (an_attribute_family family);
std::string attribute_syntactic_location_to_string (an_attribute_location location);
std::string attribute_arg_kind_to_string (an_attribute_arg_kind kind);

void output_decl_modifier_enum_values(a_decl_modifier_set decl_modifier);

void printScopeStack( std::string label );
void printTypeInfo(a_type_ptr type);
void printDeclarationInType ( SgType* type );

std::string sourcePositionInformation ( SgNode* node );

std::string getLocationName ( SgScopeStatement* scope );

std::string generateScopeName( const a_source_correspondence & source_corresp );
std::string generateScopeName( a_type_ptr edgType );
std::string generateScopeName( a_routine_ptr edgRoutine );
std::string generateScopeName ( a_source_sequence_entry_ptr ssp );

void testForBadEdgTemplateName( const std::string & name );
