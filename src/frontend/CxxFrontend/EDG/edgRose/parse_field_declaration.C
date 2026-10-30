
#include "edgRose.h"

SgDeclarationStatement* EDG_ROSE_Translation::parse_field_declaration(SourceSequenceList& sse)
   {
     a_field_ptr p = (a_field_ptr)(sse->entity.ptr);
     ROSE_ASSERT(p != NULL);

#ifdef DEBUG_CONVERSION
     indent_line(); debug_print("field "); print_source_corresp(p->source_corresp); debug_print("\n");
     ScopedIndent si(indent);
#endif

     a_seq_number current_source_sequence_number = p->source_corresp.decl_position.seq;

#if 0
     printf ("In parse_field_declaration(): current_source_sequence_number                      = %d \n",current_source_sequence_number);
     printf ("In parse_field_declaration(): parse_field_declaration_last_source_sequence_number = %d \n",parse_field_declaration_last_source_sequence_number);
#endif

  // DQ (1/2/2012): Refactoring to generate common name used in this function.
     SgName name = p->source_corresp.name;

#if 0
     printf ("In parse_field_declaration(): calling summarize_field() \n");
     summarize_field(p);
#endif

     ++sse;
     ROSE_ASSERT (sse);

#if 0
     SgVariableSymbol* variableSymbol = convert_field_use(p);
     SgInitializedName* in = variableSymbol->get_declaration();
     ROSE_ASSERT(in != NULL);

     ROSE_ASSERT(in->get_parent() != NULL);
     SgVariableDeclaration* decl = isSgVariableDeclaration(in->get_parent());
#else

#if 0
     printf ("In parse_field_declaration(): calling convert_type for p->type = %p \n",p->type);
#endif

     SgType* type = convert_type(p->type);

#if 0
     printf ("DONE: In parse_field_declaration(): calling convert_type for p->type = %p \n",p->type);
     printf ("In parse_field_declaration(): type = %p = %s \n",type,type->class_name().c_str());
#endif

  // a_byte bit_size     = p->bit_size;
     bool   is_bit_field = p->is_bit_field;
     bool   is_mutable   = p->is_mutable;

  // DQ (4/20/2014): Added support for 4.8 (C++11 specific) initializers of non-static data members.
     SgInitializer* initializer = NULL;
     bool has_initializer = p->has_initializer;
     if (has_initializer == true)
        {
#if 0
          printf ("building the field_initializer_expression \n");
#endif
          if (p->initializer == NULL) {
#if WARNING_FOR_NONREAL_DEVEL
            printf("WARNING: an initializer is expected for field %p (see Cxx11_tests/sizeof_template_pack.C)\n", p);
#endif
          } else {

          SgExpression* field_initializer_expression = convert_dynamic_initializer(p->initializer,p->type);
          ROSE_ASSERT(field_initializer_expression != NULL);
#if 0
          printf ("field_initializer_expression = %p = %s \n",field_initializer_expression,field_initializer_expression->class_name().c_str());
#endif
          SgInitializer* field_initializer = isSgInitializer(field_initializer_expression);
          if (field_initializer == NULL)
             {
#if 0
               printf ("building the fSgAssignInitializer expression \n");
#endif
            // DQ (4/20/2014): I am not clear that we alwasy want to be using an SgAssignInitializer.
               SgAssignInitializer* assignInitializer = buildAssignInitializer(field_initializer_expression,type);
               initializer = assignInitializer;
               ROSE_ASSERT(initializer != NULL);
             }

       // DQ (8/29/2014): This appears to be a problem when using EDG 4.9 and GNU 4.8.1 on test2014_76.C.
          if (initializer == NULL)
             {
               mprintf ("ERROR: EDG specifies and initializer, but defines NULL pointer field_initializer (I think) \n");
             }
       // ROSE_ASSERT(initializer != NULL);
          }
        }

#if 0
     SgScopeStatement* scope_of_variable_declaration = topScopeStack();
     printf ("################ In parse_field_declaration(): calling buildVariableDeclaration_nfi(): name = %s scope = %p = %s \n",
          name.str(),scope_of_variable_declaration,scope_of_variable_declaration->class_name().c_str());
#endif

  // Build the filed (data member) declaration.
  // SgVariableDeclaration* decl = set_file_info_for_corresp(buildVariableDeclaration_nfi(name, type, NULL, topScopeStack()), p->source_corresp);
     SgVariableDeclaration* decl = set_file_info_for_corresp(buildVariableDeclaration_nfi(name, type, initializer, topScopeStack()), p->source_corresp);

#if 0
     printf ("################ In parse_field_declaration(): after call to buildVariableDeclaration_nfi(): decl = %p = %s: \n",decl,decl->class_name().c_str());
     printf (" --- initializer = %p \n",initializer);
#endif

  // DQ (4/4/2015): Interesting that this is true (it is true because the get_type() function is not 
  // virtual and the type is stored in the SgInitializedName instead of the SgVariableDeclaration.
  // ROSE_ASSERT(decl->get_type() == NULL);

#if 0
     decl->get_file_info()->display("In parse_field_declaration(): this should be a valid source position");
#endif

  // DQ (5/7/2013): I think we can assert this, debugging test2013_152.C.
     ROSE_ASSERT(decl->get_file_info()->isCompilerGenerated() == false);

  // DQ (2/16/2013): Added support for access modifiers.
     unsigned int access_level = p->source_corresp.access;
     set_access_modifier(decl,access_level);

#if 0
     printf ("In parse_field_declaration(): decl = %p = %s: access = %s \n",decl,decl->class_name().c_str(),decl->get_declarationModifier().get_accessModifier().displayString().c_str());
#endif
#if 0
     printf ("In parse_field_declaration(): decl = %p = %s \n",decl,decl->class_name().c_str());
#endif
#if 0
     printf ("In parse_field_declaration(): edg_type = %p type = %p = %s Calling fixup_variable_or_field_declaration() \n",p->type,type,type->class_name().c_str());
#endif

     fixup_variable_or_field_declaration(decl, p->type);

  // Add the variable (data member), represented as a SgInitializedName, to the SgVariableDeclaration.
     SgInitializedName* in = set_file_info_for_corresp(getFirstInitializedName(decl), p->source_corresp);
     set_file_info_for_corresp(decl->get_definition(), p->source_corresp);
     ROSE_ASSERT (in != NULL);

     ROSE_ASSERT(in->get_type() != NULL);

     if (is_mutable)
        {
       // DQ (1/22/2013): This was already set, I think it is fine to leave it set.
          in->get_storageModifier().setMutable();

       // DQ (1/22/2013): Set the mutable flag in the SgVariableDeclaration.
          decl->get_declarationModifier().get_storageModifier().setMutable();
        }

     if (is_bit_field)
        {
#if 0
           printf ("Setting the bit field width for the field bit_size = %u p->bit_size_constant = %p \n",bit_size,p->bit_size_constant);
#endif
       // DQ (1/20/2014): We need to save the bit field width as a constant expression rather than as a value.
          ROSE_ASSERT(p->bit_size_constant != NULL);
          SgExpression* bit_field_expression = convert_constant(p->bit_size_constant);
          ROSE_ASSERT(bit_field_expression != NULL);
          bit_field_expression->set_parent(decl);
#if 0
          printf ("bit_field_expression = %p = %s \n",bit_field_expression,bit_field_expression->class_name().c_str());
#endif

          decl->set_bitfield(bit_field_expression);
#if 0
          printf ("DONE: Setting the bit field width for the field bit_size = %u p->bit_size_constant = %p \n",bit_size,p->bit_size_constant);
#endif
        }

  // DQ (12/30/2008): Added test for parent (see test2003_30.C)
     if (decl->get_bitfield() != NULL)
        {
          decl->get_bitfield()->set_parent(decl);
          ROSE_ASSERT(decl->get_bitfield()->get_parent() != NULL);
        }

     if (p->source_corresp.name != NULL)
        {
       // Skip anonymous bit fields
          field_cache[p] = getFirstVarSym(decl);
        }
#endif

// #if GNU_EXTENSIONS_ALLOWED
  // DQ (8/16/2014): Now using the same API for both GNU and Microsoft attribute handling.
  // DQ (1/4.2009): Setup GNU attributes
     set_field_attributes( p, decl );
// #endif

#if 0
  // DQ (1/2/2010): Enforce this here, since appears to be an error later in the SageInterface functions.
     ROSE_ASSERT(decl->get_definingDeclaration() != NULL);
#endif

  // DQ (4/4/2015): I think we can assert this, debugging test2013_152.C.
     ROSE_ASSERT(decl->get_file_info()->isCompilerGenerated() == false);

  // DQ (4/4/2015): We want to handle multiple variables associated with the same declaration (see test2015_60.c and test2015_62.c).
     if (current_source_sequence_number == parse_field_declaration_last_source_sequence_number)
        {
          SgInitializedName* first_var = getFirstInitializedName(parse_field_declaration_last_decl);
          ROSE_ASSERT(first_var != NULL);
          SgInitializedName* last_var = getFirstInitializedName(decl);
          ROSE_ASSERT(last_var != NULL);
#if 0
          printf ("In parse_field_declaration(): Adding a variable to a previous variable declaration: parse_field_declaration_last_decl = %p \n",parse_field_declaration_last_decl);
          printf ("   --- first_var = %p name = %s \n",first_var,first_var->get_name().str());
          printf ("   --- last_var = %p name  = %s \n",last_var,last_var->get_name().str());
#endif
       // addingVariableToExistingVariableDeclaration = true;

       // ROSE_ASSERT(temp_decl == NULL);
       // temp_decl = last_decl;
       // ROSE_ASSERT(temp_decl != NULL);

       // DQ (4/4/2015): This is handled above.
       // DQ (7/28/2012): We don't want to support the new approach just yet, but this will permit us to support un-named type declarations.
       // temp_decl = convert_variable_prototype(p,NULL);
       // temp_decl = decl;

       // DQ (12/31/2015): This is now not always true (e.g. when we handle multiple variables in a single declaration using the normalized mode.
       // DQ (12/29/2015): Before supporting multiple variable in the same declaration, I think this is alway true.
       // ROSE_ASSERT(last_decl == NULL);

       // DQ (4/4/2015): We want to have a better test for when we combine types defined in a typedef (see test2015_60.c and test2015_62.c).
          if (parse_field_declaration_last_decl != NULL)
             {
            // DQ (4/4/2015): Debugging.
            // ROSE_ASSERT(last_decl->get_type() == NULL);
            // ROSE_ASSERT(decl->get_type() == NULL);

            // ROSE_ASSERT(temp_decl != NULL);
            // SgType* last_decl_type = last_decl->get_type();
               SgType* first_decl_type = first_var->get_type();
               SgType* last_decl_type = last_var->get_type();
               ROSE_ASSERT(first_decl_type != NULL);
               ROSE_ASSERT(last_decl_type != NULL);
#if 0
            // DQ (6/6/2016): Tracing down use of incorrect typedef alias (inconsistant with original source code) \n");
               printf ("In parse_field_declaration(): Calling stripType() \n");
#endif
            // DQ (4/4/2015): We want to avoid striping typedefs.
            // SgType* stripped_type_last_decl_base_type    = last_decl_base_type->stripType(SgType::STRIP_MODIFIER_TYPE|SgType::STRIP_REFERENCE_TYPE|SgType::STRIP_POINTER_TYPE|SgType::STRIP_ARRAY_TYPE|SgType::STRIP_TYPEDEF_TYPE);
            // SgType* stripped_type_local_result_base_type = local_result_base_type->stripType(SgType::STRIP_MODIFIER_TYPE|SgType::STRIP_REFERENCE_TYPE|SgType::STRIP_POINTER_TYPE|SgType::STRIP_ARRAY_TYPE|SgType::STRIP_TYPEDEF_TYPE);
               SgType* stripped_type_first_decl_type = first_decl_type->stripType(SgType::STRIP_MODIFIER_TYPE|SgType::STRIP_REFERENCE_TYPE|SgType::STRIP_RVALUE_REFERENCE_TYPE|SgType::STRIP_POINTER_TYPE|SgType::STRIP_ARRAY_TYPE);
               SgType* stripped_type_last_decl_type  = last_decl_type ->stripType(SgType::STRIP_MODIFIER_TYPE|SgType::STRIP_REFERENCE_TYPE|SgType::STRIP_RVALUE_REFERENCE_TYPE|SgType::STRIP_POINTER_TYPE|SgType::STRIP_ARRAY_TYPE);
               ROSE_ASSERT(stripped_type_first_decl_type != NULL);
               ROSE_ASSERT(stripped_type_last_decl_type != NULL);

               if (stripped_type_last_decl_type == stripped_type_first_decl_type)
                  {
#if 0
                    printf ("In parse_field_declaration(): Identified the SAME types used in variable declaration for multiple types \n");
#endif
                 // ROSE_ASSERT(isSgVariableDeclaration(temp_decl) != NULL);
                 // isSgVariableDeclaration(temp_decl)->set_isAssociatedWithDeclarationList(true);
                    decl->set_isAssociatedWithDeclarationList(true);
                    parse_field_declaration_last_decl->set_isAssociatedWithDeclarationList(true);
#if 0
                    printf ("In parse_field_declaration(): isAssociatedWithDeclarationList == true for temp_decl = %p parse_field_declaration_last_decl = %p \n",
                         decl,parse_field_declaration_last_decl);
#endif
                  }
                 else
                  {
#if 0
                    printf ("In parse_field_declaration(): Identified the SEPERATE types used in variable declaration for multiple types \n");
#endif
                  }
#if 0
               printf ("Exiting as a test! \n");
               ROSE_ASSERT(false);
#endif
             }
        }

  // DQ (4/4/2015): Update the saved version of the last_source_sequence_number.
     parse_field_declaration_last_source_sequence_number = current_source_sequence_number;
     parse_field_declaration_last_decl                   = isSgVariableDeclaration(decl);

  // DQ (4/4/2015): Added assertion.
     ROSE_ASSERT(decl != NULL);
     ROSE_ASSERT(parse_field_declaration_last_decl != NULL);

#if 0
     printf ("Leaving parse_field_declaration(): edg_type = %p type = %p = %s SgInitializedName in = %p \n",p->type,type,type->class_name().c_str(),in);
#endif

     return decl;
   }
