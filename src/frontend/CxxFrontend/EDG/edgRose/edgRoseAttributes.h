// *********************************************************************
// *********** START GNU and MS Attribute Handling Support *************
// *********************************************************************

#if GNU_VISIBILITY_ATTRIBUTE_ALLOWED
SgDeclarationModifier::gnu_declaration_visability_enum get_ELF_visibility_attribute( an_ELF_visibility_kind visibility, a_source_correspondence_ptr scp );
#endif /* GNU_VISIBILITY_ATTRIBUTE_ALLOWED */

void set_alignment_attributes( a_type_ptr type, SgDeclarationStatement* decl );
void set_alignment_attributes( a_variable_ptr type, SgInitializedName* initializedName );

void set_routine_type_attributes( a_type_ptr type, SgFunctionDeclaration* decl );
void set_type_attributes( a_type_ptr type, SgDeclarationStatement* decl );

// We might want to provide a reference to a SgDeclarationModifier instead of a SgInitializedName!
void set_variable_attributes( a_variable_ptr var, SgInitializedName* initializedName );

// DQ (1/24/2016): Adding specialized support for function parameters to be set using the attributes on the associated type.
void set_variable_attributes( an_attribute_ptr attribute_list, SgInitializedName* initializedName );

void set_field_attributes( a_field_ptr field, SgVariableDeclaration* decl );
void set_routine_attributes( a_routine_ptr rout, SgFunctionDeclaration* decl );
void set_label_attributes( a_label_ptr label, SgLabelStatement* labelstmt);

// DQ (8/17/2014): Adding more support for Microsoft declspec annotations.
void set_ms_attributes(SgDeclarationModifier & declarationModifier,an_attribute_ptr ptr );
void set_ms_attributes(SgInitializedName* initializedName, an_attribute_ptr ptr );

// DQ (8/18/2014): refactored support for retrival of the string literal in the Microsoft declspec uuid.
std::string get_uuid_string(bool & uuid_string_detected, an_attribute_arg_ptr attribute_arg);

// *********************************************************************
// ************* START CUDA Attribute Handling Support *****************
// *********************************************************************
       
#if ENABLE_CUDA_SUPPORT
void set_routine_cuda_attributes( a_routine_ptr rout, SgFunctionDeclaration* decl);
void set_variable_cuda_attributes( a_variable_ptr var, SgDeclarationStatement * decl_stmt);
void set_variable_cuda_attributes( a_variable_ptr var, SgInitializedName * initializedName);
void set_variable_cuda_attributes( a_variable_ptr var, SgStorageModifier & storage_modifier);
#endif /* ENABLE_CUDA_SUPPORT */

