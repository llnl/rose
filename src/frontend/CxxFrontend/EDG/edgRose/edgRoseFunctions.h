
// Entry point
int edg_main(int argc, char *argv[], SgSourceFile& sageFile);

namespace EDG_ROSE_Translation {
  // This is the main translator class that constains the functionality required to 
  // translate the EDG AST into one for ROSE to use internally (into an object oriented IR).

  // Data member such as STL maps, save the translation from pointers to EDG data strcutures
  // and the associated SAGE object-oriented data structures where there is a correspondence.
  // This avoids any modification of the EDG data structures to save intermediate data.

  // Note that after the translation for EDG to SAGE no information in EDG is accessible.
  // All analysis and transformation of the AST in ROSE is independent of the EDG datastructures.

     extern Sawyer::Message::Facility mlog;
     void initDiagnostics();

  // **********************************
  // Global variables used for tracking
  // **********************************

  // Current indent level for debugging
     extern int indent;

  // DQ (8/11/2012): I think this will be useful in knowing if we have processed default arguments before 
  // the function where they occured.  There does not appear to be any other way to know when to associate
  // the default arguments (stored in the type) with the function declaration.  There could be many
  // function declarations and only one where the function's default arguments should be associated (output).
     extern a_seq_number global_source_sequence_number;

  // DQ (4/14/2013): Added support for whole translation unit specification of there using the overloaded 
  // operator names or the operator syntax.  This is mostly used for testing of the unparser to make sure
  // that both are supported equally.  The default is to use the behavior specified in the original source code.
     extern bool global_call_uses_operator_syntax;
     extern bool global_call_uses_operator_names;

  // DQ (9/18/2018): Build a map so that the CPP include directives, processed in later stages of the frontend,
  // can initialize the pointer in the SgIncludeBody or the SGIncludeDirective so the SgInclude data structure.
     extern std::map<std::string, SgIncludeFile*> edg_include_file_map;
     extern std::map<std::string, SgIncludeFile*> edg_include_file_using_names_as_written_map;

  // DQ (8/6/2012): Use a global variable to support a single "std" namespace in ROSE. Even though it appears that
  // EDG will return a different one for each reference to std (at least in secondary namespace alias statements).
     extern SgNamespaceDeclarationStatement* global_std_namespace;

  // DQ (6/12/2013): EDG 4.5 adds this as an internal namespace, so the first time ROSE sees on it has to 
  // be able to interpret it as a secondary declaration.  So we have to support the special case of building
  // a primary declaration when we see that we don't have one (same problem as for "std").
     extern SgNamespaceDeclarationStatement* global_cxxabiv1_namespace;

  // ******************************
  // Caches used during translation
  // ******************************

  // Translation from EDG types to ROSE types
  // DQ (12/30/2011): Note that when a type can't be found here, often the associated template 
  // to the type can be found via first_nondefining_declarations_for_templates.
     extern std::map<a_type_ptr, SgType*> type_translation_cache;

  // DQ (8/27/2014): We need a seperate cache for ROSE types associated with template declarations
  // because the same EDG a_type_ptr is used and there were collesions in the storage of ROSE
  // types in the type_translation_cache map.  This is also a generally good idea because types
  // associated with template declarations are not really complete (since the type is not associated
  // with a full instantiation).  Thus we have a seperate map for types associated with template 
  // declarations.  The bug that demonstrated that this was a problem was test2014_161.C.
     extern std::map<a_type_ptr, SgType*> template_type_translation_cache;

  // List of all declarations of an EDG class, struct, union, or enum type,
  // used to fix up declarations that may initially have incorrect scopes that
  // must be changed to the scope of the definition if and when it is seen
     extern std::map<a_type_ptr, std::vector<SgDeclarationStatement*> > type_declaration_map;

  // DQ (11/25/2012): This map is updated after the processing of each file (when the type_translation_cache map 
  // used per file is cleared).  This map is used in the AST post processing to support connecting typedefs to 
  // proper types than might not have been processed as types when first encountered (see function: 
  // FixupTypeReferencesOnMemoryPool::visit() in this file below fro more details).
     extern std::map<a_type_ptr, SgType*> project_wide_type_translation_cache;

  // Translation from EDG routines to ROSE function declarations
     extern std::map<a_routine_ptr, SgFunctionDeclaration*> first_nondefining_declarations_for_functions;
     extern std::map<a_routine_ptr, SgFunctionDeclaration*> defining_declarations_for_functions;
     extern std::map<a_routine_ptr, std::vector<SgDeclarationStatement*> > function_declaration_map;

  // DQ (1/29/2013): Keep track of names associated with functions, it appears that in some rare cases the computed names 
  // of template instantiations can change.  As part of debugging this we keep a static map of EDG function string names.
  // We record the names with template arguments, since this is the aspect that appears to be different.
     extern std::map<a_routine_ptr, std::string> function_name_cache;

  // DQ (8/10/2012): This holds template functions only, indexed using a_routine_ptr values instead of a_template_ptr.
  // This is designed to address the problem of multiple member functions template being mapped to the same EDG 
  // a_template_ptr value (as demonstrated in test2012_174).  These maps only apply to template functions and not 
  // template classes or template variables.
     extern std::map<a_routine_ptr, SgFunctionDeclaration*> first_nondefining_declarations_for_template_functions;
     extern std::map<a_routine_ptr, SgFunctionDeclaration*> defining_declarations_for_template_functions;

  // DQ (11/19/2011): Added new map to support template handling.
     extern std::map<a_template_ptr, SgDeclarationStatement*> first_nondefining_declarations_for_templates;
     extern std::map<a_template_ptr, SgDeclarationStatement*> defining_declarations_for_templates;

  // Translation from EDG variables or static member variables to ROSE variables
     extern std::map<a_variable_ptr, SgVariableSymbol*> variable_cache;

  // Translation from EDG fields to ROSE variables
     extern std::map<a_field_ptr, SgVariableSymbol*> field_cache;

  // Translation from EDG routines to ROSE function symbols
     extern std::map<a_routine_ptr, SgSymbol*> function_cache;

  // Translation from EDG labels to ROSE labels
     extern std::map<a_label_ptr, SgLabelStatement*> label_cache;

  // Has the definition of the type been put somewhere in the AST where it will
  // be unparsed (either in a type declaration, the type of a variable
  // declaration marked as unparsing its type's definition, or the same for a
  // typedef)?
     extern std::map<a_type_ptr, bool> types_with_emitted_definitions;

  // Statements that are the bodies of GNU statement expressions and have already
  // been converted as part of those expressions.  This table is used to
  // prevent creating them again when they are seen later in a source sequence
  // list.  This is somewhat of a hack, but I didn't want to turn
  // convert_expression, convert_type, etc. into parse functions.
     extern std::set<a_statement_ptr> statement_expression_bodies;

  // DQ (1/4/2008):  Added support for mapping EDG scope pointers to Sage scopes.
  // C++ can have declaration that literally have one scope but be declared in 
  // an alternative scope, so part of the required support for C++ is a scope map.
  // The first version of the EDG/Sage translation stored the Sage scope in the EDG
  // AST, this is not required with a spearate map.
     extern std::map<a_scope_ptr, SgScopeStatement*> scope_cache;

  // DQ (10/31/2012): I think it is useless to organize scopes relative to the edg a_scope_ptr values.
  // They are hard to find and we only want to relate back to the SgScopeStatement, so we could just use 
  // a map from void* pointers (or entity pointers) back to the SgScopeStatement values instead. The best 
  // choice would be edg pointers to use would be statements, since they capture all of the concepts of scope 
  // that we need.  We we will have a seperate map of a_statement_ptr to SgScopeStatements (or modify 
  // the existing map to use a_statement_ptr instead of a_scope_ptr).  Based on a review of the use of
  // the scope_cache, I would like to add a cache specific to a_statement_ptr values that would also
  // record the mapping of edg IR nodes (edg statements) to ROSE IR nodes (SgScopeStatements).
     extern std::map<a_statement_ptr, SgScopeStatement*> statement_scope_cache;

  // DQ (8/6/2012): We need a map for namespaces...
     extern std::map<a_namespace_ptr, SgNamespaceDeclarationStatement*> namespace_cache;

  // DQ (4/8/2018): Build a cache of SgNamespaceAliasDeclarationStatement declarations to support when
  // a namespace alias is to another namespace alias (now supported).
     extern std::map<a_namespace_ptr, SgNamespaceAliasDeclarationStatement*> namespace_alias_cache;

     extern SgSourceFile* currentSourceFile;
     extern SgGlobal * current_global_scope;

  // DQ (12/19/2011): This is used to prevent the creation of template instantiations inside of template declarations
  // (which don't make sense).  So we keep a stack of template declarations being processed.  If a function is
  // being process and the stack is non-empty, then if the function would be interpreted as a template instantiation, 
  // it will instead be interpreted as a template declaration.  This avoids putting template instantiations into the
  // template declarations by mistake.  It is important since different SgSymbols are used for template instantiations
  // (which appear as functions using SgFunctionSymbol) and template declarations (which appear with SgTemplateSymbol).
     extern std::vector<a_template_ptr> templateDeclarationStack;

  // DQ (2/25/2012): We need this to in some rare cases where we have refer to the function (or it's return type as in test2012_08.C)
     extern a_scope_ptr global_innermost_function_scope;

  // DQ (2/8/2013): Adding support for Duff's Device (more complex switch statements).
  // This is a stack of maps used to support the weaving of SgCaseStmt IR nodes into the list of statements 
  // in the body of a switch.  This weaving is required as part of the more general level of support required
  // for more complex forms of the switch statement such as are found in the Duff's Device.  See test2006_49.C,
  // and also related test codes: test2013_16.C, test2013_17.C, test2013_52.C.
     extern std::vector<std::map<a_source_sequence_entry_ptr,SgStatement*>*> switchSupportStackOfMaps;

  // DQ (10/14/2012): We need to keep track of types that have already been output into declarations.
  // This avoids problems with code such as test2012_150.C: "struct {} var1, *var2;", namely that the
  // declaration of the type would be output twice (because ROSE currently represents multiple variables
  // using multiple variable declaration (one SgInitializedName per SgVariableDeclaration).  When this 
  // is changed to allow multiple SgInitializedName IR nodes per SgVariableDeclaration, then we will not
  // have the problem that is forcing us to record types nested in variable declarations.
     extern std::set<a_type_ptr> outputTypesSet;

  // DQ (4/7/2015): Save the mapping between the typeof operators and the associated expressions where they exist.
  // We use the EDG typeref node (a_type_ptr) as the key and the associated SgStatementExpression or other SgExpression 
  // when it is used. Note that we could have either a more general typeof_map, or multiple typeof maps (using both
  // EDG types (a_type_ptr) and EDG expressions (an_expr_node_ptr) as keys.
     extern std::map<a_type_ptr,      SgTypeOfType*> edg_type_to_rose_typeof_type_map;
     extern std::map<an_expr_node_ptr,SgTypeOfType*> edg_expression_to_rose_typeof_type_map;

  // DQ (7/17/2017): This is the intermediate scope that we need to support building (in rare cases) some 
  // declarations (e.g. class declarations) as part of processing the template paremeters of a non-defining 
  // template declaration.  Declarations are first placed in this scope so that they can be removed from this 
  // scope after the declaration has been constructed. Note that they are moved to a new scope that is provided 
  // within the declaration (SgDeclarationStatement) so that we can support template parameters that are known 
  // to be classes.  Examples of where this happens are in test2017_44.C and test2017_47.C.
     extern SgDeclarationScope* internal_intermediate_scope;

  // DQ (7/6/2019): Adding a map of expressions to GNU statement expressions so that we can reuse them as needed.
  // We will build the SgStatementExpression IR nodes when we traverse the declarations in conditional statements,
  // but we don't have access to the associated an_expr_node_ptr at that point.  What we do have access to is the 
  // associated a_block_ptr (or maybe it is the a_statement_ptr for be more general).
     extern std::map<a_statement_ptr, SgStatementExpression*> statement_expression_cache;

  // Fields for translation of non-real type/decl/scope/...
     extern std::map<a_scope_ptr, SgDeclarationScope*> nonreal_scope_cache;
     extern std::map<a_template_ptr, SgDeclarationScope*> nonreal_scope_cache_from_template;
     extern std::vector<SgDeclarationScope *> nonreal_scope_for_template_parameters_stack;
     extern SgDeclarationScope * global_nonreal_scope;

     extern std::map<a_template_arg_ptr,SgTemplateArgument*> processedTemplateArgsCache;
     extern std::map<a_template_parameter_ptr,SgTemplateParameter*> processedTemplateParameterCache;

  // DQ (9/8/2014): EDG normalizes the template class definitions to remove the defining non-template functions. 
  // We process these in a normal order to save them in the AST, but we have to save this list and insert the
  // defining declarations ahead of the associated template instantiations (if they have not been previously output).
  // This is a step required while we are using the text string form of the template declarations for unparsing.
  // We do however also represent the template declarations in AST form as preparations for unparing them as an
  // AST in the near future (one step at a time).
     extern std::map<a_template_ptr, SgFunctionDeclaration*> saved_defining_declarations_from_class_templates;

  // DQ (3/24/2021): Save the case option statements and insert then in a later step (when using EDG 6.x).
     extern std::map<a_switch_case_entry_ptr,SgStatement*> switch_case_map;

  // PP (9/23/25): Save the conversion of undeduced types.
     /// map storing mappings between EDG types and types that
     ///   have an undeduced (i.e., auto) component.
     extern std::unordered_map<a_type_ptr, SgType*> typesWithoutDeduction;

  // PP (9/24/25): \todo try unordered_maps
  // **********************
  // Other global variables
  // **********************

  // DQ (6/3/2019): We need to allow for the frontend to be called after an AST has been generated.
  // The effect is that a new SgProject is constructed.  This is doe to support outlining to a seperate 
  // file, for example.  In this case the new file is a transformation, or contains transformations and 
  // we can't enforce that the new AST constructed is free of transforamtions (as we want to do for the
  // typical case where an AST is constructed from an input file.  So this flag will turn off some
  // internal testing that is used to support the testing. The flag is used for the case were we are
  // outlinng code to a seperate file as part of more complex transformations.
     extern bool suppress_detection_of_transformations;

     extern a_seq_number convert_typedef_last_source_sequence_number;
     extern SgTypedefDeclaration* convert_typedef_last_decl;

     extern a_seq_number parse_field_declaration_last_source_sequence_number;
     extern SgVariableDeclaration* parse_field_declaration_last_decl;

     extern a_seq_number parse_variable_declaration_last_source_sequence_number;
     extern SgVariableDeclaration* parse_variable_declaration_last_decl;

     extern bool global_suppress_variable_declaration_normalization;

  // **************************************************************************
  //              Functions
  // **************************************************************************

     void initialize (SgSourceFile* inputSourceFile);

     void clear_EDG_ROSE_caches();
     void clear_global_caches();

#ifdef DEBUG_CONVERSION
     void indent_line();
#endif
     void displayFunctionCache( const std::string & label );

     void outputTypesSetDisplay( const std::string & label );

     void testForBadGenerateTemplateName( const std::string & name );

     void normalizeTemplateName( SgName & name );

     template <class T>
     void displayMap( const T & m, const std::string & label );

     void checkForMatchingType (SgDeclarationStatement* declaration);

     void testEdgRoseMaps();

     bool isTemplateDeclaration   (a_type_ptr p);
     bool isTemplateInstantiation (a_type_ptr p);
     bool isTemplateDeclaration   (a_routine_ptr p);
     bool isTemplateInstantiation (a_routine_ptr p);

     void errorCheckingOfTemplateOrTemplateInstantiationModes (const bool buildTemplateDeclaration, const bool buildTemplateInstantiation);
     void setTemplateOrTemplateInstantiationClassGeneration   (a_type_ptr    p, bool forceTemplateDeclaration, bool & buildTemplateDeclaration, bool & buildTemplateInstantiation);
     void setTemplateOrTemplateInstantiationFunctionGeneration(a_routine_ptr p, bool forceTemplateDeclaration, bool & buildTemplateDeclaration, bool & buildTemplateInstantiation);

     void setDefiningAndFirstNondefiningDeclarations (a_routine_ptr p, bool forceTemplateDeclaration,SgFunctionDeclaration* & first_nondefining_declaration,SgFunctionDeclaration* & return_defining_declaration, a_template_ptr & edg_template);

     bool isDeclaredInAnInstantiation( a_source_correspondence & scp, bool buildTemplateDeclaration );

     void testFirstNondefiningDeclarationType(SgFunctionDeclaration* first_nondefining_declaration, bool buildTemplateDeclaration, bool buildTemplateInstantiation);

     template <typename T>
     SgName generateName( T p);

     a_type_ptr getBaseTypeOfEdgArrayOrPointerType(a_type_ptr type);

     template <>
     SgName generateName<a_routine_ptr>( a_routine_ptr p);

     template <>
     SgName generateName<a_variable_ptr>( a_variable_ptr p);

     template <>
     SgName generateName<a_field_ptr>( a_field_ptr p);

     template < class SgTemplateDeclType >
     void copyTemplateArgumentsFromNondefiningToDefiningDeclaration(SgFunctionDeclaration* defining_declaration);

     void setTemplateArgumentParents (SgTemplateArgumentPtrList & templateArgumentsList, SgDeclarationStatement* nondefining_templateInstantiation);
     void testTemplateArgumentParents(SgTemplateArgumentPtrList & templateArgumentsList, SgDeclarationStatement* nondefining_templateInstantiation);

     void test_for_correct_form_of_declaration(bool buildTemplateInstantiation, bool buildTemplateDeclaration, SgDeclarationStatement* decl);

     bool verifyExistenceOfSymbol(SgDeclarationStatement* declaration);

     bool matching_defining_and_nondefining_variants(SgDeclarationStatement* decl);

     SgDeclarationStatement* getTemplateDeclaration(a_type_ptr p);

     void setTemplateFunctionDeclaration (a_routine_ptr p, SgFunctionDeclaration* functionDeclaration );

     SgDeclarationStatement* parse_secondary_declaration(SourceSequenceList& sse);
     SgType* convert_integer_type(a_type_ptr p);
     SgType* convert_float_type_from_kind(a_float_kind kind);
     SgType* convert_float_type(a_type_ptr p);
     std::string get_name_for_enumerator(a_type_ptr enum_type, an_integer_value value);
     SgValueExp* convert_integer_constant(a_constant_ptr e, a_type_ptr p, an_integer_value value, const char* str, bool in_enumerator);
     SgValueExp* convert_float_constant(a_type_ptr p, a_host_fp_value value, const char* str);
     SgType* apply_qualifiers(SgType* result, a_bit_field qualifiers, a_upc_block_size block_size);

     SgType* convert_type(a_type_ptr p, bool pointerToMember = false, bool from_template_parameter = false);
     /// converts an EDG type to a ROSE type (or a pair of ROSE types).
     /// \param  p_param the EDG type
     /// \param  withNondeducedTypes true, if a second undeduced type for p_param should be generated.
     /// \param  pointerToMember
     /// \param  from_template_parameter
     /// \return a pair of types. The first type is the same that convert_type returns. The second type
     ///         is not null only if (withNondeducedTypes == true) and the type p_param contains an "auto" type
     ///         (possibly hidden under cv modifiers or decltype).
     /// \details similar to convert_type, generates a dual return type of concrete and undeduced return type
     ///          (if requested and available).
     std::tuple<SgType*, SgType*>
     convert_type2(a_type_ptr p_param, bool withNondeducedTypes, bool pointerToMember, bool from_template_parameter);

     SgType* convert_ptr_to_member_type(a_type_ptr p);

     SgStatement* convert_condition(an_expr_node_ptr e, SourceSequenceList& sse, a_statement_ptr target_statement_after_conditional = NULL);

     SgExpression * convert_expression(an_expr_node_ptr e);

     SgExpression* convert_lambda(a_lambda_ptr lambda_info);


     std::string convert_string_constant(a_constant_ptr e);
     SgExpression * convert_constant_in_enumerator(a_constant_ptr e);
     SgExpression * convert_constant(a_constant_ptr e);

     SgDeclarationStatement* parse_structlike(SourceSequenceList& sse, a_type_ptr p, SgClassDeclaration::class_types t, bool forceTemplateClassDeclaration = false, a_template_ptr edg_template = NULL);

     SgDeclarationStatement* convert_struct_secondary(a_type_ptr p, SgClassDeclaration::class_types t);

     SgDeclarationStatement* convert_type_as_secondary_decl(SourceSequenceList& sse);

     void fixup_scopes_of_earlier_declarations(const std::vector<SgDeclarationStatement*>& decls, SgScopeStatement* new_scope);
     void fixup_defining_declarations_of_earlier_declarations(const std::vector<SgDeclarationStatement*>& decls, SgDeclarationStatement* def_decl);
     SgDeclarationStatement* get_declaration_of_unnamed_type(SgType* t);
     void fixup_variable_or_field_declaration(SgVariableDeclaration* decl, a_type_ptr type);
     void fixup_typedef_declaration(SgTypedefDeclaration* decl, a_type_ptr type);
     SgDeclarationStatement* parse_field_declaration(SourceSequenceList& sse);
     void add_function_to_symbol_table_and_cache(a_routine_ptr p, SgFunctionDeclaration* decl);

     void fixup_variable_field_typedef_support(SgDeclarationStatement* decl, a_type_ptr type);

     SgDeclarationStatement* parse_routine(SourceSequenceList& sse, bool forceTemplateClassDeclaration, a_template_ptr edg_template = NULL, bool forceSecondaryDeclaration = false);

     template<class FunctionDeclaration>
     void parse_function_body(const SourceSequenceList& sse_base, a_routine_ptr p, FunctionDeclaration* decl);

     SgScopeStatement* getAssociatedClassDefinitionForMemberFunction(a_routine_ptr p, bool forceTemplateDeclaration);

     SgFunctionParameterList* convert_function_parameter_list(a_routine_ptr p, bool skipDefaultArguments, a_type_ptr declared_type, SgDeclarationScope* & declarationScope );

     void convert_ctor_initializers ( a_routine_ptr p, SgCtorInitializerList* ctor_initialization_list );

     SgInitializedName* convert_function_parameter(a_param_type_ptr p , bool skipDefaultArguments, SgDeclarationScope* & declarationScope );

     SgFunctionDeclaration* convert_routine_prototype(a_routine_ptr p, bool forceTemplateDeclaration = false, a_template_ptr edg_template = NULL);

      SgFunctionDeclaration* convert_routine(a_routine_ptr p, bool forceTemplateDeclaration = false, a_template_ptr edg_template = NULL, SgFunctionDeclaration* optional_nondefiningTemplateDeclaration = NULL );

     SgInitializer* wrap_expr_as_initializer(SgExpression* e);

     SgExpression * convert_dynamic_initializer(a_dynamic_init_ptr p, a_type_ptr edgType);

     SgExpression * convert_dynamic_constructor_initializer(a_dynamic_init_ptr dip, a_type_ptr edgType);

     SgInitializer* convert_initializer(an_init_kind kind, an_initializer initializer);
     SgInitializer* convert_var_initializer(a_variable_ptr p);
     SgVariableSymbol* convert_variable_use(a_variable_ptr p);
     SgVariableSymbol* convert_field_use(a_field_ptr p);

     SgSymbol* convert_function_use(a_routine_ptr p);

     SgLabelStatement* convert_label_use(a_label_ptr p);
     SgSymbol* convert_class_use(a_variable_ptr p);
     SgStatement* convert_goto(a_label_ptr p);

     SgStatement* convert_switch_case(a_switch_case_entry_ptr p);

     SgVariableDeclaration* parse_variable_declaration(SourceSequenceList& sse);

     SgVariableDeclaration* convert_variable_declaration(a_variable_ptr variable);

     void check_types_and_set_isAssociatedWithDeclarationList( SgVariableDeclaration* last_decl, SgVariableDeclaration* temp_decl );

     SgVariableDeclaration* convert_variable_prototype(a_variable_ptr p, a_template_ptr edg_template);

     SgStorageModifier::storage_modifier_enum convert_storage_class(a_storage_class kind);
     void set_special_function_modifier(SgSpecialFunctionModifier & modifier, a_special_function_kind kind);
     void set_function_modifier(SgFunctionModifier & modifier, a_routine_ptr p);
     SgForInitStatement* parse_for_initializer(SourceSequenceList& sse);
     SgStatement* build_basic_block_if_multiple_statements(const std::vector<SgStatement*>& stmts);

     std::vector<SgStatement*> parse_statement(SourceSequenceList& sse, SgBasicBlock* existingBasicBlock = NULL);

     SgStatement* convert_statement(a_statement_ptr stmt);
     void process_static_local_variable_initializers(a_scope_ptr scope);
     std::vector<SgStatement*> parse_statement_list(SourceSequenceList& sse, an_il_entry_kind orig_kind, void* orig_ptr);
     a_statement_ptr last_statement_in_list(a_statement_ptr p);

     std::vector<SgStatement*> parse_statement_list_without_terminator(SourceSequenceList& sse, a_statement_ptr enclosing_block, a_statement_ptr statement_list, a_statement_ptr last_stmt = NULL);

     bool is_unnamed(a_type_ptr p);

     a_type_ptr contains_unnamed_type_at_top_level(a_type_ptr p);

     a_type_ptr contains_unnamed_type_at_top_level_ptr_to_member(a_type_ptr p);

     std::string generate_name_for_type(a_type_ptr p);
     std::string generate_name_for_variable(a_variable_ptr p);
     std::string generate_name_for_field(a_field_ptr p);

     SgDeclarationStatement* parse_type_declaration(SourceSequenceList& sse);
     void parse_enumerators(SourceSequenceList& sse, SgEnumDeclaration* decl, a_type_ptr t);
     SgDeclarationStatement* parse_macro(SourceSequenceList& sse);
     SgPragmaDeclaration* parse_pragma(SourceSequenceList& sse);

     std::string generate_pragma_name(a_pragma_ptr p);

     SgAsmStmt* parse_asm(SourceSequenceList& sse);
     SgDeclarationStatement* parse_global_or_namespace_scope_entity(SourceSequenceList& sse);
     SgGlobal* parse_global_scope(SgGlobal* inputGlobalScope, SourceSequenceList& sse, bool skip_ast_translation);

     /// converts an EDG function type to a ROSE function type (or a pair of ROSE function types).
     /// \param   p the EDG type
     /// \param   withNondeducedTypes true, if a second undeduced type for p_param should be generated.
     /// \param   pointerToMember
     /// \return  a pair of types. The first type is the a function types with a deduced return type. The second type
     ///          is not null only if requested (withNondeducedTypes == true) and the type \p p has a return
     ///          type that makes use of auto (possibly hidden under cv modifiers or decltype).
     /// \details generates a secondary function type with an undeduced return type if requested (\p withNondeducedTypes)
     ///          and the function type makes use of an auto return type.
     std::tuple<SgFunctionType*, SgFunctionType*>
     convert_function_type(a_type_ptr p, bool withNondeducedTypes, bool pointerToMember = false);

     SgDeclarationStatement* convert_struct_primary (a_type_ptr p, SgClassDeclaration::class_types t, SgClassDeclaration* nonDefiningDecl);

     SgTemplateClassDeclaration* convert_template_struct_primary(a_type_ptr p, SgClassDeclaration::class_types t, SgTemplateClassDeclaration* nonDefiningDecl, a_template_ptr edg_template);

     SgDeclarationStatement* convert_template_struct_secondary(a_type_ptr p, SgClassDeclaration::class_types t, a_template_ptr edg_template);

     SgScopeStatement* getDeclarationScope ( a_source_correspondence & scp , bool expect_nonreal_scope = false );

     std::string generate_namespace_name(a_namespace_ptr p);

     SgDeclarationStatement* parse_namespace(SourceSequenceList& sse);

     SgDeclarationStatement* convert_namespace(a_namespace_ptr p,SourceSequenceList& sse);

     SgNamespaceDeclarationStatement* construct_implicit_namespace(a_namespace_ptr p);

     SgNamespaceDeclarationStatement* construct_namespace(a_namespace_ptr p);

     a_scope_ptr     getRootScope    (a_namespace_ptr nsp);
     a_namespace_ptr getRootNamespace(a_namespace_ptr nsp);

     SgDeclarationStatement* parse_using_declaration(SourceSequenceList& sse);

     SgTypedefDeclaration * convert_typedef(a_type_ptr p);

     SgTryStmt* convert_try_block_statement(SourceSequenceList& sse, a_statement_ptr statement);

  //////////
  // Template functions

     SgDeclarationStatement* parse_template(SourceSequenceList& sse);
     SgDeclarationStatement* convert_template_prototype(a_template_ptr t);

     SgDeclarationScope * get_nonreal_declaration_scope_for_template_decl(a_template_decl_ptr template_decl);
     SgDeclarationScope * get_nonreal_declaration_scope_for_template(a_template_ptr p, const char * callee);
     void attach_nonreal_declaration_scope_to_template(a_template_ptr p, SgDeclarationStatement * tpldecl, SgDeclarationScope * nrscope, const char * callee);

     SgSymbol* getAssociatedTemplateSymbol(a_template_ptr edg_template, SgDeclarationStatement* & declarationStatement);

     SgType* convert_nonreal_type(a_type_ptr p, bool pointerToMember, bool from_template_parameter, const char * constant_name);

     SgExpression* convert_constant_template_parameter(a_constant_ptr e);

     SgTemplateArgumentPtrList parse_template_arguments( a_template_arg_ptr arg, SgScopeStatement* associatedScope );

  //////////
  // Utility functions

     bool is_implicitly_cast_integral_constant(a_constant_ptr con);
     bool pm_cast_is_unambiguous(a_constant_ptr constant);
     bool default_class_array_initialization( a_dynamic_init_ptr dip, a_boolean *is_value_init);
     bool is_default_dynamic_init(a_dynamic_init_ptr dip);

     void testAndResetTemplateDeclarationAndTemplateInstantiation ( a_routine_ptr p, SgScopeStatement* scopeOfFunction, bool & buildTemplateInstantiation, bool & buildTemplateDeclaration );

     SgSymbol* retrieveAssociatedClassSymbol (std::string name, SgScopeStatement* scopeOfDeclaration, bool buildTemplateDeclaration, SgTemplateArgumentPtrList* templateArgumentsList, 
                                              SgTemplateParameterPtrList* templateParameterList, SgTemplateArgumentPtrList* templateSpecializationArgumentList);

     void convert_base_class_list( a_type_ptr p, SgClassDefinition* classDefinition );

     SgStorageModifier set_storage_modifiers ( a_storage_class storage_class );

     SgAccessModifier set_access_modifiers_for_base_class ( an_access_specifier access );

#if UPC_EXTENSIONS_ALLOWED
     SgType* add_modifiers ( SgType *type, a_type_ptr edg_type, const a_type_qualifier_set type_modifiers, const a_storage_class storage_modifiers, 
                             const an_access_specifier access_modifiers, const a_name_linkage_kind linkage_modifiers, 
                             const a_special_function_kind special_function_modifiers, const a_upc_access_method upc_modifiers );
     void add_modifiers ( SgDeclarationStatement* declarationStatement, const a_storage_class storage_modifiers, 
                          const an_access_specifier access_modifiers, const a_name_linkage_kind linkage_modifiers, 
                          const a_special_function_kind special_function_modifiers, const a_upc_access_method upc_modifiers );
#endif
     void set_access_modifier(SgDeclarationStatement* declarationStatement, unsigned int access_modifiers);

     SgTemplateInstantiationDirectiveStatement* parse_instantiation_directive(SourceSequenceList& sse);

     unsigned int setupFunctionTypeModifiers(a_routine_type_supplement_ptr sup);

     void set_linkage(a_routine_ptr p, SgFunctionDeclaration* function);

     void fixupSubTreeSourcePositions(SgDeclarationStatement* declaration);

     std::string outputSpecializationString(SgDeclarationStatement::template_specialization_enum spec);

     void testAndMarkAsSpecialized(a_routine_ptr p, bool buildTemplateInstantiation, SgFunctionDeclaration* definingFunctionDeclaration , SgFunctionDeclaration* nondefiningFunctionDeclaration);

     void putFunctionInTemplateMap(a_routine_ptr p, a_template_ptr edg_template, SgFunctionDeclaration* nondefiningFunctionDeclaration, SgFunctionDeclaration* definingFunctionDeclaration);

     void testDefiningNonDefiningDeclarations    (SgDeclarationStatement* first_nondefining_declaration, SgDeclarationStatement* defining_declaration, SgDeclarationStatement* secondary_nondefining_declaration, const std::string & label);
     void testDefiningNonDefiningDeclarationTypes(SgDeclarationStatement* first_nondefining_declaration, SgDeclarationStatement* defining_declaration, SgDeclarationStatement* secondary_nondefining_declaration);
     void outputDefiningNonDefiningDeclarations  (SgDeclarationStatement* first_nondefining_declaration, SgDeclarationStatement* defining_declaration, SgDeclarationStatement* secondary_nondefining_declaration, const std::string & label);

     SgType* getType(SgDeclarationStatement* declaration);

     SgTemplateParameterPtrList parse_template_parameters( a_type_ptr p, SgScopeStatement* scope, SgNode* parent );
     SgTemplateParameterPtrList parse_template_parameters( a_template_parameter_ptr tpp, SgScopeStatement* scope, SgNode* parent );
     SgTemplateParameterPtrList parse_template_parameters( a_template_parameter_ptr tpp, SgDeclarationStatement* templateDeclaration);

     SgTemplateParameterPtrList parse_template_parameters( a_routine_ptr p, SgScopeStatement* scope, SgNode* parent );

     a_statement_ptr find_statement_before_target(a_statement_ptr starting_statement, a_statement_ptr target_statement);

     void skipOverSourceSequenceEntriesAlreadyVisited(SourceSequenceList& sse, a_statement_ptr p );

     void testAndSkipOverPreviouslyVisitedSourceSequenceEntries(SourceSequenceList& sse);

     SgScopeStatement* edgScopeOfDeclarationFromEntity(a_tagged_pointer & entity);
     SgScopeStatement* edgScopeOfDeclarationFromType(a_type_ptr & p);

     SgScopeStatement* findAssociatedClassDeclaration(a_field_ptr field_entry);

     SgExprListExp* convert_argument_list (a_new_delete_supplement_ptr ndsp, an_expr_node_ptr arg );

     void verifyFunctionHasSymbolInAssociatedScope(SgFunctionDeclaration* first_nondefining_declaration, SgTemplateParameterPtrList* templateParameterList, SgTemplateArgumentPtrList* templateArgumentsList);

     void updateSwitchSupportMap(a_source_sequence_entry_ptr source_sequence_entry, SgStatement* firstStatementInList);

     SgExpression* buildSpeculativeFunctionRefExp( a_constant_ptr e );
     SgExpression* buildNonrealFunctionRefExp( a_constant_ptr e , a_template_arg_ptr arg_list = NULL );
     SgExpression * buildNonrealFunctionRefExp_base( SgName name, SgDeclarationScope * nrscope, SgTemplateArgumentPtrList * tpl_args=NULL);

     SgClassDeclaration::class_types convert_class_kind( a_type_kind kind );

     bool isInTypeTranslationCache(a_type_ptr p);
     bool isInTypeDeclarationMap(a_type_ptr p);
     bool isInTypeDeclarationMapAsDefiningDeclaration(a_type_ptr p);
     bool isInDefiningDeclarationTemplateCache(a_template_ptr p);
     bool isInFirstNondefiningDeclarationTemplateCache(a_template_ptr p);

     void push_entry_to_type_declaration_map(a_type_ptr p, SgDeclarationStatement* decl);

     std::string builtin_operation_function_name(a_builtin_operation_kind kind);

     bool has_end_of_block_terminator(SourceSequenceList& sse, a_statement_ptr enclosing_block );

     void set_noreturn_attribute( a_type_ptr type, SgInitializedName* initializedName );

     bool isWideChar( a_type_ptr type);

     bool is16BitWideChar( a_type_ptr type);
     bool is32BitWideChar( a_type_ptr type);

     SgStaticAssertionDeclaration* parse_static_assertion_declaration( SourceSequenceList & sse );

     SgMicrosoftAttributeDeclaration* parse_microsoft_attribute(SourceSequenceList & sse);

     void update_scope_cache(std::map<a_scope_ptr, SgScopeStatement*> & local_scope_cache, a_scope_ptr assoc_scope, SgClassDefinition* classDefinition);

     void skip_over_previously_processed_blocks(SourceSequenceList & sse);
     bool skip_over_previously_processed_blocks_support(SourceSequenceList & sse);

     void process_possible_typeof_expression_arguments( a_scope_ptr scope );

     void output_typeof_maps(const std::string & label);

     SgCompoundLiteralExp* construct_compound_literal(an_expr_node_ptr e, SgExpression* expression);

     bool templateArgumentsAreTemplateParameters(a_type_ptr p);

     SgTemplateArgumentPtrList setupMatchingTemplateArguments(bool buildTemplateInstantiation, SgFunctionDeclaration* first_nondefining_declaration, 
                                                              a_template_arg_ptr templateArgs, SgScopeStatement* scopeOfFunction);

     SgCastExp::cast_type_enum typeOfCast(an_expr_node_ptr e);
     SgCastExp::cast_type_enum typeOfCast(a_constant_ptr constant);

     void setupDeclarationScope( SgDeclarationStatement* decl);

     void setup_duffs_device_switch_data(std::vector<a_statement_ptr>                       & block_list_perspective,
                                         std::vector<a_source_sequence_entry_ptr>           & block_source_sequence_perspective,
                                         std::map<a_source_sequence_entry_ptr,SgStatement*> & block_statement_map,
                                         std::vector<SgStatement*>                          & result );

     void record_block_source_sequence_entries_for_statements (SourceSequenceList & sse, std::vector<a_source_sequence_entry_ptr> & block_source_sequence_perspective);

     void setup_block_statement_map ( a_source_sequence_entry_ptr                        sse_value,
                                      std::vector<SgStatement*>                          & stmts,
                                      std::map<a_source_sequence_entry_ptr,SgStatement*> & block_statement_map );

     void fixup_case_statements( std::vector<SgStatement*> & result );

     void build_header_file_tree(SgSourceFile* rose_source_file, SgIncludeFile* parent_include_file, a_source_file_ptr edg_source_file);

     bool hasFirstNondefDeclForTemplate(a_template_ptr p);
     SgDeclarationStatement * getFirstNondefDeclForTemplate(a_template_ptr p);
     void setFirstNondefDeclForTemplate(a_template_ptr p, SgDeclarationStatement * decl);

     bool hasDefiningDeclForTemplate(a_template_ptr p);
     SgDeclarationStatement * getDefiningDeclForTemplate(a_template_ptr p);
     void setDefiningDeclForTemplate(a_template_ptr p, SgDeclarationStatement * decl);

     std::vector<SgStatement*> processEntriesRelatedToConditional( SourceSequenceList& sse, a_statement_ptr target_statement_after_conditional );

     std::string normalize_filename (std::string input_filename );

     std::string generate_source_position_string(a_source_correspondence & sc, void* pointer_value);

     SgFunctionDeclaration * convert_templk_function(a_template_ptr p);
     SgVariableDeclaration * convert_templk_variable(a_template_ptr p);
     SgTemplateTypedefDeclaration * convert_templk_class_for_typeref(a_template_ptr p);
     SgDeclarationStatement * convert_templk_class_for_class(a_template_ptr p);
     SgDeclarationStatement * convert_templk_class(a_template_ptr p);
     SgDeclarationScope * construct_nonreal_declaration_scope();

     SgDeclarationScope * get_nonreal_declaration_scope(SgNode * n);
     void set_nonreal_declaration_scope(SgNode * n, SgDeclarationScope * nrscope);

     SgVariableSymbol * use_of_unseen_variable(a_variable_ptr p, SgName & name);
     SgVariableSymbol * use_of_builtin_variable(a_variable_ptr p, SgName & name);

     void sage_back_end ( SgSourceFile & sageFile ); 
     std::string edgVersionString();
     void cfe_main(int argc, char *argv[], SgSourceFile & sageFile);

     /////////////////////////////
     // TODO move to SageBuilder

     SgExpression* buildReferenceExpressionFromSymbol(SgSymbol * symbol);
   }
