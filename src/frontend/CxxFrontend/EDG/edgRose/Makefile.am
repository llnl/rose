include $(top_srcdir)/config/Makefile.for.ROSE.includes.and.libs

libedgSageSources = \
     $(top_builddir)/src/frontend/SageIII/Cxx_Grammar.h \
     $(top_srcdir)/src/frontend/CxxFrontend/EDG/EDG_@ROSE_EDG_MAJOR_VERSION_NUMBER@.@ROSE_EDG_MINOR_VERSION_NUMBER@/src/cp_gen_be.cc \
     $(top_srcdir)/src/frontend/CxxFrontend/EDG/EDG_@ROSE_EDG_MAJOR_VERSION_NUMBER@.@ROSE_EDG_MINOR_VERSION_NUMBER@/src/fe_init.cc

noinst_LTLIBRARIES = libnewsage.la

libnewsage_la_SOURCES = \
     main.C \
     construct_compound_literal.C \
     construct_implicit_namespace.C \
     construct_namespace.C \
     construct_nonreal_declaration_scope.C \
     convert_argument_list.C \
     convert_base_class_list.C \
     convert_class_use.C \
     convert_condition.C \
     convert_constant.C \
     convert_constant_in_enumerator.C \
     convert_constant_template_parameter.C \
     convert_ctor_initializers.C \
     convert_dynamic_constructor_initializer.C \
     convert_dynamic_initializer.C \
     convert_expression__enk_new_delete.C \
     convert_expression__enk_operation.C \
     convert_expression.C \
     convert_field_use.C \
     convert_function_parameter.C \
     convert_function_parameter_list.C \
     convert_function_type.C \
     convert_function_use.C \
     convert_initializer.C \
     convert_integer_constant.C \
     convert_integer_type.C \
     convert_float_constant.C \
     convert_float_type.C \
     convert_float_type_from_kind.C \
     convert_goto.C \
     convert_label_use.C \
     convert_lambda.C \
     convert_namespace.C \
     convert_nonreal_type.C \
     convert_ptr_to_member_type.C \
     convert_routine.C \
     convert_routine_prototype.C \
     convert_statement.C \
     convert_string_constant.C \
     convert_struct_primary.C \
     convert_struct_secondary.C \
     convert_switch_case.C \
     convert_template_prototype.C \
     convert_template_struct_primary.C \
     convert_template_struct_secondary.C \
     convert_templk_class.C \
     convert_templk_class_for_class.C \
     convert_templk_class_for_typeref.C \
     convert_templk_function.C \
     convert_templk_variable.C \
     convert_try_block_statement.C \
     convert_type.C \
     convert_typedef.C \
     convert_type_as_secondary_decl.C \
     convert_variable_declaration.C \
     convert_variable_prototype.C \
     convert_variable_use.C \
     convert_var_initializer.C \
     parse_asm.C \
     parse_enumerators.C \
     parse_field_declaration.C \
     parse_for_initializer.C \
     parse_global_or_namespace_scope_entity.C \
     parse_global_scope.C \
     parse_instantiation_directive.C \
     parse_macro.C \
     parse_microsoft_attribute.C \
     parse_namespace.C \
     parse_pragma.C \
     parse_routine.C \
     parse_secondary_declaration.C \
     parse_statement.C \
     parse_statement_list.C \
     parse_statement_list_without_terminator.C \
     parse_static_assertion_declaration.C \
     parse_structlike.C \
     parse_template.C \
     parse_template_arguments.C \
     parse_template_parameters.C \
     parse_using_declaration.C \
     parse_variable_declaration.C \
     fixup.C \
     setup.C \
     accessors.C \
     utilities.C \
     debugging.C \
     edgRose.C edgRoseAttributes.C SourcePosition.C SourceSequenceList.C edgSupport.C edgGraph.C 

libnewsage_la_CXXFLAGS = \
   $(CXXFLAGS) \
   -DMAKE_FRONT_END_CALLABLE=TRUE \
   $(ROSE_INCLUDES) \
   -I$(top_srcdir)/src/frontend/CxxFrontend/EDG/EDG_@ROSE_EDG_MAJOR_VERSION_NUMBER@.@ROSE_EDG_MINOR_VERSION_NUMBER@/src \
   -I../EDG_@ROSE_EDG_MAJOR_VERSION_NUMBER@.@ROSE_EDG_MINOR_VERSION_NUMBER@/src 

libnewsage_la-SourcePosition_DEPENDENCIES: SourcePosition.C SourcePosition.h
libnewsage_la-edgRose_DEPENDENCIES: edgRose.C SourcePosition.h
libnewsage_la_DEPENDENCIES  = $(libedgSageSources)

EXTRA_DIST = \
  CMakeLists.txt \
  edgRose.h macro_defs.h \
  edgRoseAttributes.h edgRoseOMP.h \
  SourcePosition.h SourceSequenceList.h debugging.h 

DISTCLEANFILES = 
