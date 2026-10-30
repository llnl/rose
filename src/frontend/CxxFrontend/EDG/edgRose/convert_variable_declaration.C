
#include "edgRose.h"

#define DEBUG__convert_variable_declaration 0

SgVariableDeclaration* 
EDG_ROSE_Translation::convert_variable_declaration(a_variable_ptr range)
   {
  // Change the name from "range" to "variable" (or better, after we get this working).

#if DEBUG__convert_variable_declaration
  printf("Enter convert_variable_declaration()");
#endif
       
#define DEBUG_CONVERT_VARIABLE_DECLARATIONS 0

  // I am not sure yet that we need to support multiple variables per declaration, but 
  // this feature might make it more useful in the parse_variable_declaration() function 
  // (using the source sequence list).
     bool addingVariableToExistingVariableDeclaration = false;

     SgVariableDeclaration* decl = NULL;

  // DQ (3/27/2018): This is designed to support multiple variables (not clear if this can happen for the range declaration).
     do {

#if DEBUG_RANGE_BASED_FOR_STATEMENT || 0
          printf ("In convert_variable_declaration(): case stmk_range_based_for: calling summarize_variable() \n");
          summarize_variable(range);
#endif

#if 0
          printf ("In convert_variable_declaration(): case stmk_range_based_for: calling summarize_scope() \n");
          ROSE_ASSERT(range->source_corresp.parent_scope != NULL);
          summarize_scope(range->source_corresp.parent_scope);
#endif

#if DEBUG_CONVERT_VARIABLE_DECLARATIONS
       // printf ("In parse_statement(): case stmk_range_based_for: first_var->get_scope() = %p = %s \n",first_var->get_scope(),first_var->get_scope()->class_name().c_str());
#endif
          SgName name = generateName(range);
#if 0
          printf ("In convert_variable_declaration(): case stmk_range_based_for: name                 = %s \n",name.str());
          printf ("In convert_variable_declaration(): case stmk_range_based_for: range->type          = %p \n",range->type);
          printf ("In convert_variable_declaration(): case stmk_range_based_for: range->declared_type = %p \n",range->declared_type);
#endif
       // DQ (6/19/2012): Added support for autonomous types (e.g. defining types embedded in variable declarations); see test2012_107.C.
          a_type_ptr edg_variable_type = range->declared_type ? range->declared_type : range->type;
          ROSE_ASSERT(edg_variable_type != NULL);

          SgType* type = convert_type(edg_variable_type);

          SgInitializer* init = NULL;
       // init = convert_var_initializer(p);
       // if (variable_cache[p] != NULL)
#if 0
          printf ("range->is_member_constant = %s \n",range->is_member_constant ? "true" : "false");
#endif
          if (range->is_member_constant == false)
             {
               init = convert_var_initializer(range);
#if 0
               printf ("Processing a member constant! init = %p \n",init);
#endif
             }

       // SgInitializedName* anotherInitializedName = buildInitializedName_nfi(name,type,init);

#if DEBUG_GET_DECLARATION_SCOPE
          printf ("In convert_variable_declaration(): case stmk_range_based_for: calling getDeclarationScope() \n");
#endif
       // DQ (1/3/2015): We need to do a better job of computing the scope (see test2015_152.C).
       // SgScopeStatement* scope = first_var->get_scope();
          SgScopeStatement* scope = getDeclarationScope(range->source_corresp);
          ROSE_ASSERT(scope != NULL);

#if DEBUG_CONVERT_VARIABLE_DECLARATIONS || 0
          printf ("In convert_variable_declaration(): case stmk_range_based_for: scope computed for each variable seperately (required for template variables): scope = %p = %s \n",scope,scope->class_name().c_str());
#endif

#if 0
          printf ("Exiting as a test until we compute the scope correctly \n");
          ROSE_ASSERT(false);
#endif
       // SgVariableSymbol* variableSymbol = new SgVariableSymbol(anotherInitializedName);
       // scope->insert_symbol(name,variableSymbol);

       // Used below with a different name.
       // last_var = anotherInitializedName;

       // SgVariableDeclaration* decl = SageBuilder::buildVariableDefinition_nfi (last_decl, anotherInitializedName, init);
       // SgVariableDeclaration* decl = SageBuilder::buildVariableDeclaration_nfi(name,type,init,scope);
          if (decl == NULL)
             {
               decl = SageBuilder::buildVariableDeclaration_nfi(name,type,init,scope);
             }
            else
             {
               printf ("ERROR: We are not supporting multiple variables per variable declaration just yet. \n");
               ROSE_ASSERT(false);
             }

       // anotherInitializedName->set_parent(last_decl);
       // anotherInitializedName->set_parent(decl);
#if 0
          printf ("WARNING: In convert_variable_declaration(): case stmk_range_based_for: using topScopeStack() \n");
#endif
       // We need to compute the scope properly (note that the scope should be available).
       // anotherInitializedName->set_scope(topScopeStack());

       // last_decl->get_variables().push_back(anotherInitializedName);
       // decl->get_variables().push_back(anotherInitializedName);

       // Liao, 1/20/2016, patch up SgVariableDefinition , similar code from SgVariableDeclaration::append_variable()
       // SageBuilder::buildVariableDefinition_nfi (last_decl, anotherInitializedName, init);


          SgStorageModifier::storage_modifier_enum sm = convert_storage_class(range->declared_storage_class);

       // Set this the same for all of the declarations associated with a single declaration.
       // decl->get_declarationModifier().get_storageModifier().set_modifier(sm);

#if ENABLE_CUDA_SUPPORT
       // DQ (7/28/2012): This is set here for the whole declaration (and each declaration associated with a single declaration with a list of variables.
          set_variable_cuda_attributes(range, decl);
#endif /* ENABLE_CUDA_SUPPORT */

#if DEBUG_CONVERT_VARIABLE_DECLARATIONS || 0
           printf ("In parse_variable_declaration(): Calling fixup_variable_or_field_declaration() instead of markAsVariableDeclarationContainsBaseTypeDefiningDeclaration() \n");
#endif

       // DQ (10/15/2012): Refactored this code so it can be called from here and convert_variable_prototype().
       // This function figures out if this variable declaration (also called for secondary declarations) requires 
       // the type declaration as part of (nested inside) the variable declaration.
       // markAsVariableDeclarationContainsBaseTypeDefiningDeclaration(p,decl);
          fixup_variable_or_field_declaration(decl,range->type);

#if 0
          printf ("In convert_variable_declaration(): range->type = %p \n",range->type);
       // printf ("In convert_variable_declaration(): decl->get_type() = %p = %s \n",decl->get_type(),decl->get_type()->class_name().c_str());
          printf ("In convert_variable_declaration(): decl->get_type() = %p \n",decl->get_type());
#endif
#if 0
          printf ("Exiting as a test! \n");
          ROSE_ASSERT(false);
#endif

       // This is the variable in the variable declaration (note that a variable declaration can declare more than one variable).
       // It is unclear if in EDG this is supported, I think it is, but the information is less than obvious.
          SgInitializedName* var = getFirstInitializedName(decl);

       // Because of the grammar to modifiers (see Appendex B of C++ reference) the modifiers are in two locations, 
       // we only really want to set one of then.  I have to look in the unparser to see which one is given preference.
          var->get_storageModifier().set_modifier(sm);
          decl->get_declarationModifier().get_storageModifier().set_modifier(sm);

#if ENABLE_CUDA_SUPPORT
       // DQ (7/28/2012): This is set above for the whole declaration (and each declaration associated with a single declaration with a list of variables.
       // set_variable_cuda_attributes(p, decl);
          set_variable_cuda_attributes(range, var);
#endif /* ENABLE_CUDA_SUPPORT */

#if 0
       // DQ (7/25/2006): Added to support code such as: "register int *p1 asm ("ax") = 0;" (see test2006_98.C)
          printf ("range->asm_name_is_valid                = %s \n",range->asm_name_is_valid ? "true" : "false");
          printf ("range->has_named_register_storage_class = %s \n",range->has_named_register_storage_class ? "true" : "false");
          printf ("range->asm_name_or_reg.name             = %p \n",range->asm_name_or_reg.name);
          printf ("range->asm_name_or_reg.reg              = %d \n",range->asm_name_or_reg.reg);
#endif

#if GNU_EXTENSIONS_ALLOWED || REDEFINE_EXTNAME_PRAGMA_ENABLED
       // Note: asm_name_is_valid is the relevant GNU extension here and if NAMED_REGISTERS_ALLOWED 
       // is defined then has_named_register_storage_class is available.
          if (range->asm_name_is_valid == true && range->asm_name_or_reg.name != NULL)
             {
            // This is a declaration of the form: "register int *p1 asm ("ax") = 0;"
            // If we were not to handle the GNU extensions then we would have to store
            // the string represnting the register name.  Instead we will likely use
            // a machine specific code and develop them separately for a number of architectures.
               var->set_register_name_string(range->asm_name_or_reg.name);
#if 0
               printf ("Set register name from p->asm_name_or_reg.name = %s \n",p->asm_name_or_reg.name);
#endif
            }
#endif

// DQ (4/21/2016): The use of "reg" is only protected by GNU_EXTENSIONS_ALLOWED.
// #if GNU_EXTENSIONS_ALLOWED || REDEFINE_EXTNAME_PRAGMA_ENABLED
#if GNU_EXTENSIONS_ALLOWED
       // Note: asm_name_is_valid is the relevant GNU extension here and if NAMED_REGISTERS_ALLOWED 
       // is defined then has_named_register_storage_class is available.
#if NAMED_REGISTERS_ALLOWED
          if (range->asm_name_is_valid == false && range->has_named_register_storage_class == false)
             {
            // This is a declaration of the form: "register int *p1 asm ("ax") = 0;"
               var->set_register_name_code((SgInitializedName::asm_register_name_enum)range->asm_name_or_reg.reg);

#if 0
            // DQ (12/12/2012): If GNU_X86_ASM_EXTENSIONS_ALLOWED is true, then it appers that the register names are picked up correctly.
            // EDG BUG: This appears to be an EDG bug, the register is always set to "ax"
               printf ("EDG BUG: This appears to be an EDG bug, the register is always set to 'ax' p->asm_name_or_reg.reg = %d \n",(int)p->asm_name_or_reg.reg);
#if GNU_X86_ASM_EXTENSIONS_ALLOWED
               ROSE_ASSERT(range->asm_name_or_reg.reg == anr_a);
#endif
#endif
             }
#endif
#endif

#if NAMED_REGISTERS_ALLOWED
          if (range->has_named_register_storage_class == true)
             {
            // This is a declaration similar to the form: "register int *p1 asm ("ax") = 0;" 
            // but I have not seen this specific case yet!

               printf ("Sorry, case of (p->has_named_register_storage_class == true) not implemented \n");
            // var->set_register_name_code((SgInitializedName::asm_register_id_enum)p->asm_name_or_reg.id);
             }
#endif

       // DQ (8/2/2014): added to support C++11 auto keyword.
          if (range->declared_with_auto_type_specifier == true)
             {
#if 0
               printf ("Detected C++11 auto keyword \n");
#endif
               var->set_using_auto_keyword(true);
               ROSE_ASSERT(range->declared_type != NULL);
               if (range->declared_type != range->type) {
                 SgType * auto_type = convert_type(range->declared_type,false, true);
//               printf("Found auto typed variable: auto_type = %p (%s) = %s\n", auto_type, auto_type->class_name().c_str(), auto_type->unparseToString().c_str());
                 var->set_auto_decltype(auto_type);
               }
#if 0
               printf ("Exiting as a test! \n");
               ROSE_ASSERT(false);
#endif
             }


#if 0
       // Setup the initializer if it is present in the variable declaration
       // SgInitializer* init = convert_var_initializer(p);
          SgInitializer* init = NULL;
       // if (variable_cache[p] == NULL)
          if (range->is_member_constant == false)
             {
#if DEBUG_CONVERT_VARIABLE_DECLARATIONS
               printf ("In convert_variable_declaration(): case stmk_range_based_for: calling convert_var_initializer(range = %p) \n",range);
#endif
               init = convert_var_initializer(range);
#if DEBUG_CONVERT_VARIABLE_DECLARATIONS
               printf ("In convert_variable_declaration(): case stmk_range_based_for: DONE: calling convert_var_initializer(range = %p) init = %p = %s \n",range,init, (init != NULL) ? init->class_name().c_str() : "null");
#endif
             }

          if (init != NULL)
             {
               var->set_initializer(init);
               init->set_parent(var);
             }
#endif
#if 0
          if (variable_cache[range] != NULL)
             {
               printf ("In convert_variable_declaration(): case stmk_range_based_for: This variable has been defined previously (may exist in another scope) \n");
               printf ("In convert_variable_declaration(): case stmk_range_based_for: var = %p var->get_initptr() = %p \n",var,var->get_initptr());
            // var->set_initializer(NULL);
             }
#endif

// #if GNU_EXTENSIONS_ALLOWED || USER_CONTROL_OF_STRUCT_PACKING
       // DQ (8/16/2014): Now using the same API for both GNU and Microsoft attribute handling.
       // DQ (2/27/2013): Added support for gnu attributes specific to variables.
          set_variable_attributes(range,var);
// #endif

       // DQ (8/29/2020): I think we can enforce this now.
       // ROSE_ASSERT(variable_cache.find(range) == variable_cache.end());

       // SgVariableSymbol* sym = getFirstVarSym(decl);
#if 0
       // printf ("In convert_variable_declaration(): Putting sym name = %s into variable_cache using key range = %p EDG name = %s \n",sym->get_name().str(),range,range->source_corresp.name);
#endif
       // DQ (8/29/2020): Make sure we don't put NULL pointes into the variable_cache.
       // ROSE_ASSERT(sym != NULL);

       // Save the SgVariableSymbol into the map (I think that it might still be unclear which symbol table to put it in, 
       // so I think this is done later).
       // variable_cache[range] = getFirstVarSym(decl);
       // variable_cache[range] = sym;
       // DQ (8/29/2020): I think we can enforce this now.
          if (variable_cache.find(range) == variable_cache.end())
             {
            // Save the SgVariableSymbol into the map (I think that it might still be unclear which symbol table to put it in, 
            // so I think this is done later).

               SgVariableSymbol* sym = getFirstVarSym(decl);
#if 0
               printf ("In convert_variable_declaration(): Putting sym name = %s into variable_cache using key range = %p EDG name = %s \n",sym->get_name().str(),range,range->source_corresp.name);
#endif
            // DQ (8/29/2020): Make sure we don't put NULL pointes into the variable_cache.
               ROSE_ASSERT(sym != NULL);
#if 0
               printf ("In parse_variable_declaration(): Putting sym name = %s into variable_cache using key p = %p EDG name = %s \n",sym->get_name().str(),p,p->source_corresp.name);
#endif
            // variable_cache[p] = getFirstVarSym(decl);
            // variable_cache[p] = sym;
               variable_cache[range] = sym;
             }
            else
             {
#if 0
               printf ("In parse_variable_declaration(): variable_cache already has a entry for range = %p name = %s \n",range,range->source_corresp.name);
#endif
             }

#if 0
          var->get_startOfConstruct()->display("SgInitializedName in parse_variable_declaration(): debug");
          var->get_endOfConstruct()  ->display("SgInitializedName in parse_variable_declaration(): debug");
#endif

#if DEBUG_RANGE_BASED_FOR_STATEMENT
       // DQ (12/29/2015): Adding support for multiple variables in a veraible declaration (optional behavior).
          printf ("In parse_statement(): case stmk_range_based_for: At base of loop over variables in range variable declaration: addingVariableToExistingVariableDeclaration = %s \n",
               addingVariableToExistingVariableDeclaration ? "true" : "false");
#endif
        }
     while (addingVariableToExistingVariableDeclaration == true);

     ROSE_ASSERT(decl != NULL);

#if DEBUG__convert_variable_declaration
  printf ("  decl = %p = %s\n", decl, decl->class_name().c_str());
  printf("Leave convert_variable_declaration()");
#endif
     return decl;
   }
