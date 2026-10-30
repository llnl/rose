
// This code is to support building DOT graphs of the EDG AST ans associated nodes
// so that we can better understand issues in the translation of the EDG representation
// into ROSE.  The modivating problem has been the handling of GNU statement expressions
// where types are declared and extra source sequence points are added to the AST and
// we need to understnad this better so that we can skip over them in the translation.

namespace EDG_ROSE_Graph
   {
     EXTERN std::ofstream file;

     void graph ( a_scope_ptr scope, const std::string & scope_name);
     void graph ( a_statement_ptr statement );
     void graph ( an_expr_node_ptr expression );
     void graph ( a_constant_ptr constant );
     void graph ( a_dynamic_init_ptr dynamic_initializer );
     void graph ( a_type_ptr type );
     void graph ( a_field_ptr field );
     void graph ( a_variable_ptr variable );
     void graph ( a_routine_ptr routine );
     void graph ( a_template_ptr template_node );
     void graph ( a_namespace_ptr namespace_node );
     void graph ( a_source_sequence_entry_ptr src_seq_entry );
     void graph ( a_param_type_ptr type );
     void graph ( a_template_symbol_supplement_ptr template_sup );
     void graph ( a_template_decl_ptr template_decl );
     void graph ( a_template_param_type_supplement_ptr extra_info );
     void graph ( a_template_parameter_ptr template_parameter );
     void graph ( a_template_arg_ptr template_argument );
     void graph ( a_using_decl_ptr udp );
     void graph ( a_vla_dimension_ptr vla_dimension );
     void graph ( a_base_class_ptr base_class );
     void graph ( a_symbol_ptr symbol );
     void graph ( a_new_delete_supplement_ptr new_delete_supplement );
     void graph ( a_block_ptr block );
     void graph ( a_switch_case_entry_ptr switch_case_entry );
     void graph ( a_switch_stmt_descr_ptr switch_stmt_descr );
     void graph ( an_attribute_ptr attributes );
     void graph ( a_variable_template_info_ptr);

     bool outputInGraph ( SourceSequenceList & sse );
     bool outputInGraph( const a_source_position & pos );
     bool outputInGraph( a_statement_ptr statement );
     bool outputInGraph( a_block_ptr block );

     void graph ( a_constructor_init_ptr ctor_init );

     void insertGraphNode (void* node);

     void graph_headers();
     void graph_headers( a_source_file_ptr file );

     void graph_edg_ast();
   }

