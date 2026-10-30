
#include "edgRose.h"

#define DEBUG__convert_variable_prototype 0
#define DEBUG__determine_storage_class 0

namespace EDG_ROSE_Translation {

static SgStorageModifier::storage_modifier_enum determine_storage_class (a_variable_ptr var, SgScopeStatement * current_scope, SgScopeStatement * variable_scope)
   {
     a_storage_class storage_class = var->storage_class;
#if DEBUG__determine_storage_class
     printf("Enter determine_storage_class()\n");
     printf("  var = %p\n", var);
     printf("    ->storage_class = %s\n", db_storage_class_names[storage_class]);
     printf("  current_scope  = %p = %s\n",   current_scope,  current_scope ? current_scope->class_name().c_str() : "");
     printf("  variable_scope = %p = %s\n", variable_scope, variable_scope ? variable_scope->class_name().c_str() : "");
#endif
     bool is_unspecified = ( storage_class == (a_storage_class)sc_unspecified );
     bool is_static = ( storage_class == (a_storage_class)sc_static );
     bool is_local_to_function = var->source_corresp.is_local_to_function;
     bool is_C_and_not_put_out = C_mode();
#if BACK_END_IS_CP_GEN_BE
     is_C_and_not_put_out &= !var->definition_has_been_put_out;
#endif
     bool is_extern = is_unspecified || (is_static && !is_local_to_function && !is_C_and_not_put_out);

     if (isSgClassDefinition(variable_scope) != NULL) {
       if (isSgClassDefinition(current_scope)) {
         storage_class = (a_storage_class)sc_static;
       }
     } else if (is_extern) {
       storage_class = (a_storage_class)sc_extern;
     }

#if GNU_EXTENSIONS_ALLOWED
     if (var_is_gnu_named_register(var)) {
       storage_class = (a_storage_class)sc_register;
     }
#endif

#if DEBUG__determine_storage_class
     printf("  storage_class = %s\n", db_storage_class_names[storage_class]);
     printf("Leave determine_storage_class()\n");
#endif

     return convert_storage_class(storage_class);
   }

SgVariableDeclaration * convert_variable_prototype(a_variable_ptr p, a_template_ptr edg_template)
   {
#ifdef DEBUG_CONVERSION
     indent_line(); debug_print("variable prototype "); print_source_corresp(p->source_corresp); debug_print("\n");
#endif

#if DEBUG__convert_variable_prototype
     printf ("In convert_variable_prototype() \n");
     printf ("  p = %p \n", p);
     printf ("    ->type          = %p \n", p->type);
     printf ("    ->declared_type = %p \n", p->declared_type);
     printf ("  edg_template = %p \n", edg_template);
#endif

  // DQ (11/29/2011): From what I have seen so far these are always the same pointer value!
  // SgType* variable_type = convert_type(p->declared_type ? p->declared_type : p->type);
  // TV (05/04/2018): two cases:
  //                       secondary decl => edg_template == NULL && p->type is valid
  //                       primary => template (templk_static_data_member) was traversed first and p->declared_type is valid
     a_type_ptr edg_type = NULL;
     if (p->declared_with_auto_type_specifier) {
       // variable declared using auto keyword
       ROSE_ASSERT(p->type != NULL);
       edg_type = p->type;
     } else if (p->is_template_variable) {
       // declaration of a template variable: template <int I> class A { static char s[I+1]; }
       edg_type = p->type;
     } else if (edg_template != NULL) {
       // definition of a template variable: template <int I> char A<I>::s[I+1] = { 0 };
       ROSE_ASSERT(p->declared_type != NULL);
       edg_type = p->declared_type;
     } else if (p->declared_type != NULL) {
       edg_type = p->declared_type;
     } else {
       edg_type = p->type;
     }
     ROSE_ASSERT(edg_type != NULL);
#if DEBUG__convert_variable_prototype
     printf ("  edg_type = %p \n", edg_type);
#endif

     SgType* variable_type = convert_type(edg_type, false, edg_template != NULL);
#if DEBUG__convert_variable_prototype
     printf ("  variable_type = %p = %s \n",variable_type,variable_type->class_name().c_str());
#endif

     SgName name = generateName(p);
#if DEBUG__convert_variable_prototype
     printf ("  name = %s \n",name.str());
#endif

  // DQ (6/19/2012): Added support for autonomous types (e.g. defining types embedded in variable declarations); see test2012_107.C.
  // NOTE: The declared_type is the type as it actually appears in the declaration of the variable at the point of its definition;
  // NULL if there is no defining declaration.  When is_parameter is TRUE, the type is what actually appeared in the parameter 
  // declaration -- e.g., before an array decays to a pointer.  (The only exception is a parameter variable of a function
  // template instantiation, where the type declared in the template may involve a template parameter.)
#if DEBUG__convert_variable_prototype
     printf ("  edg_type->autonomous_primary_tag_decl = %s \n",edg_type->autonomous_primary_tag_decl ? "true" : "false");
#endif

     SgScopeStatement* scopeOfVariable = getDeclarationScope(p->source_corresp);
     ROSE_ASSERT(scopeOfVariable != NULL);
#if DEBUG__convert_variable_prototype
     printf ("  scopeOfVariable = %p = %s = %s \n",scopeOfVariable,scopeOfVariable->class_name().c_str(),SageInterface::get_name(scopeOfVariable).c_str());
#endif

     SgScopeStatement * current_scope = topScopeStack();
     ROSE_ASSERT(current_scope != NULL);
     SgInitializer* initializer = NULL;

#if DEBUG__convert_variable_prototype
     printf ("  p->is_member_constant   = %s \n",p->is_member_constant ? "true" : "false");
     printf ("  p->initializer_in_class = %s \n",p->initializer_in_class ? "true" : "false");
#endif
     if (p->is_member_constant == true && p->initializer_in_class)
        {
          a_scope_ptr variable_scope = p->source_corresp.parent_scope;
          ROSE_ASSERT(variable_scope != NULL);
#if DEBUG__convert_variable_prototype
          printf ("  variable_scope = %p \n",variable_scope);
#endif
          std::map<a_scope_ptr, SgScopeStatement*>::iterator i = scope_cache.find(variable_scope);
          if (i != scope_cache.end())
             {
               SgScopeStatement* rose_scope = scope_cache[variable_scope];
               ROSE_ASSERT(rose_scope != NULL);
#if DEBUG__convert_variable_prototype
               printf ("  rose_scope = %p = %s \n",rose_scope,rose_scope->class_name().c_str());
#endif
#if DEBUG__convert_variable_prototype
               printf ("  current_scope = %p = %s \n",current_scope,current_scope->class_name().c_str());
#endif
               if (rose_scope == current_scope)
                  {
                    initializer = convert_var_initializer(p);
                  }
             }
        }
       else
        {
       // DQ (7/10/2020): Adding support for Cxx11_tests/test2020_69.C test code.
#if DEBUG__convert_variable_prototype
          printf ("  p->init_kind == initk_static  = %s \n",(p->init_kind == initk_static)  ? "true" : "false");
          printf ("  p->init_kind == initk_dynamic = %s \n",(p->init_kind == initk_dynamic) ? "true" : "false");
#endif
          if (p->init_kind == initk_static || p->init_kind == initk_dynamic)
             {
               ROSE_ASSERT(p->initializer.constant != NULL || p->initializer.dynamic != NULL);
             }

        }
#if DEBUG__convert_variable_prototype
     printf ("  variable initializer = %p \n",initializer);
#endif

  // More specific versions of declarations used internally.
     SgVariableDeclaration* variableDeclaration         = NULL;

#if DEBUG__convert_variable_prototype
     printf ("  Template Variable?\n");
     printf ("    edg_template                     = %x\n", edg_template);
     if (edg_template) {
       printf ("    edg_template->kind               = %x\n", edg_template->kind);
       printf ("    edg_template->prototype_template = %x\n", edg_template->prototype_template);
     }
     printf ("    p->is_template_variable          = %x\n", p->is_template_variable);
     printf ("    p->is_prototype_instantiation    = %x\n", p->is_prototype_instantiation);
     printf ("    p->is_nonreal                    = %x\n", p->is_nonreal);
     printf ("    p->is_specialized                = %x\n", p->is_specialized);
     printf ("    p->explicit_instantiation        = %x\n", p->explicit_instantiation);
     printf ("    p->class_explicitly_instantiated = %x\n", p->class_explicitly_instantiated);
     printf ("    p->template_info                 = %x\n", p->template_info);
     if (p->template_info) {
       printf ("    p->template_info->assoc_template                 = %x\n", p->template_info->assoc_template);
       printf ("    p->template_info->template_arg_list              = %x\n", p->template_info->template_arg_list);
       printf ("    p->template_info->partial_spec_template_arg_list = %x\n", p->template_info->partial_spec_template_arg_list);
     }
#endif

     SgStorageModifier::storage_modifier_enum sm = determine_storage_class(p, current_scope, scopeOfVariable);
     SageBuilder::pushScopeStack(scopeOfVariable);
     if (!edg_template && !p->template_info) {
       SgVariableSymbol* firstVariableSymbol = scopeOfVariable->lookup_variable_symbol(name);
       variableDeclaration = set_file_info_for_corresp(buildVariableDeclaration_nfi(name, variable_type, initializer, scopeOfVariable, false, sm), p->source_corresp);
       ROSE_ASSERT(variableDeclaration != NULL);
          
       SgVariableSymbol* secondVariableSymbol = getFirstVarSym(variableDeclaration);

       ROSE_ASSERT(secondVariableSymbol != NULL);
       if (firstVariableSymbol != NULL) {
         ROSE_ASSERT(firstVariableSymbol == secondVariableSymbol);
       }
     } else if (!edg_template && p->template_info) {
       ROSE_ASSERT(p->template_info->assoc_template);

       SgDeclarationStatement * tpl_decl = nullptr;
       if (p->template_info->assoc_template->prototype_template) {
         tpl_decl = getFirstNondefDeclForTemplate(p->template_info->assoc_template->prototype_template);
         ROSE_ASSERT(tpl_decl != nullptr);
       }
#if DEBUG__convert_variable_prototype
       printf ("  tpl_decl = %p = %s \n", tpl_decl, tpl_decl ? tpl_decl->class_name().c_str() : "");
#endif
       SgTemplateVariableDeclaration * tpl_var = isSgTemplateVariableDeclaration(tpl_decl);
       ROSE_ASSERT(tpl_decl == nullptr || tpl_var != nullptr);

       SgTemplateArgumentPtrList tpl_args;
       if (p->template_info->template_arg_list != nullptr) {
         tpl_args = parse_template_arguments(p->template_info->template_arg_list, scopeOfVariable);
       }
#if DEBUG__convert_variable_prototype
       printf ("  tpl_args.size() = %d\n", tpl_args.size());
#endif
       SgTemplateVariableInstantiation * tpl_var_inst = set_file_info_for_corresp(buildTemplateVariableInstantiation_nfi(name, variable_type, initializer, scopeOfVariable, tpl_var, tpl_args), p->source_corresp);
#if DEBUG__convert_variable_prototype
       printf ("  tpl_var_inst = %p\n", tpl_var_inst);
       printf ("  tpl_var_inst->get_templateArguments().size() = %d\n", tpl_var_inst->get_templateArguments().size());
#endif

       variableDeclaration = tpl_var_inst;

     } else {
       SgTemplateVariableDeclaration * tpl_var = set_file_info_for_corresp(buildTemplateVariableDeclaration_nfi(name, variable_type, initializer, scopeOfVariable), p->source_corresp);
       if (edg_template != nullptr && edg_template->template_decl != nullptr) {
         tpl_var->get_templateParameters() = parse_template_parameters(edg_template->template_decl->param_list, nullptr, tpl_var);
       }
       if (p->template_info != nullptr && p->template_info->partial_spec_template_arg_list != nullptr) {
         tpl_var->get_templateSpecializationArguments() = parse_template_arguments(p->template_info->partial_spec_template_arg_list,scopeOfVariable);
       }

       variableDeclaration = tpl_var;
     }
     SageBuilder::popScopeStack();
     ROSE_ASSERT(variableDeclaration != NULL);
#if DEBUG__convert_variable_prototype
     printf ("  variableDeclaration->get_parent() = %p = %s\n", variableDeclaration->get_parent(), variableDeclaration->get_parent() ? variableDeclaration->get_parent()->class_name().c_str() : "");
#endif
     variableDeclaration->get_declarationModifier().get_storageModifier().set_modifier(sm);
     if (isSgClassDefinition(current_scope)) {
#if DEBUG__convert_variable_prototype
       printf ("  Looking for previous declaration which storage modifier should be overwritten.\n");
#endif
       SgInitializedName * iname = variableDeclaration->get_variables()[0]->get_prev_decl_item();
       SgVariableDeclaration * prev_var = iname ? isSgVariableDeclaration(iname->get_parent()) : nullptr;
       if (prev_var != nullptr) {
#if DEBUG__convert_variable_prototype
         printf ("  Rewritting storage modifier for prev_var=%p\n", prev_var);
#endif
         prev_var->get_declarationModifier().get_storageModifier().set_modifier(sm);
       }
     }

     set_file_info_for_corresp(getFirstInitializedName(variableDeclaration), p->source_corresp);
     set_file_info_for_corresp(variableDeclaration->get_definition(), p->source_corresp);
          
     fixup_variable_or_field_declaration(variableDeclaration, edg_type);

     SgVariableSymbol* sym = getFirstVarSym(variableDeclaration);
     ROSE_ASSERT(sym != NULL);
#if DEBUG__convert_variable_prototype
     printf ("  Putting sym name = %s into variable_cache using key p = %p EDG name = %s \n",sym->get_name().str(),p,p->source_corresp.name);
#endif

  // DQ (8/29/2020): Changed this to avoid overwriting entries already in the variable_cache.
     if (variable_cache.find(p) == variable_cache.end()) {
       variable_cache[p] = sym;
     }
#if DEBUG_VARIABLE_CACHE
     else {
       printf ("NOTE: In convert_variable_prototype(): variable p = %p name = %s is already present in the variable_cache: sym name = %s \n",p,p->source_corresp.name,sym->get_name().str());
     }
     if (variable_cache.find(p) == variable_cache.end()) {
       printf ("Note: In convert_variable_prototype(): variable_cache[p] == NULL \n");
     }
#endif

#if DEBUG__convert_variable_prototype
     printf ("  variableDeclaration = %p variableDeclaration->get_declarationModifier().get_storageModifier().isStatic() = %s \n",variableDeclaration,variableDeclaration->get_declarationModifier().get_storageModifier().isStatic() ? "true" : "false");
#endif

  // DQ (4/14/2012): Modified to handle template parameter.
     if (edg_template != NULL)
        {
       // DQ (8/10/2012): Added test to make sure we are not overwriting anything in our map.
          ROSE_ASSERT(!hasFirstNondefDeclForTemplate(edg_template));

       // DQ (8/15/2013): Added warning...I would like to understand where this happens better (should we be using the first non-defining declaration?)...
#if DEBUG_NONDEFINING_TEMPLATE_DECLARATION_MAP
          printf ("WARNING: putting variableDeclaration = %p = %s into first_nondefining_declarations_for_templates[edg_template = %p] \n",variableDeclaration,variableDeclaration->class_name().c_str(),edg_template);
#endif
          setFirstNondefDeclForTemplate(edg_template, variableDeclaration);
        }
     ROSE_ASSERT(variableDeclaration->get_parent() != NULL);


#if DEBUG__convert_variable_prototype
     printf ("  skipping call to markAsVariableDeclarationContainsBaseTypeDefiningDeclaration() (using fixup_variable_or_field_declaration() instead) \n");
#endif

  // DQ (10/15/2012): Refactored this code so it can be called from here and parse_variable_declaration().
  // This function figures out if this variable declaration (also called for secondary declarations) requires 
  // the type declaration as part of (nested inside) the variable declaration.
  // markAsVariableDeclarationContainsBaseTypeDefiningDeclaration(p,variableDeclaration);

  // DQ (2/16/2013): Added support for access modifiers.
     unsigned int access_level = p->source_corresp.access;
     set_access_modifier(variableDeclaration,access_level);

  // DQ (8/18/2014): Setup any possible Microsoft declspec attributes.
     if (p->source_corresp.attributes != NULL)
        {
       // set_ms_attributes(initializedName,p->source_corresp.attributes);
          set_ms_attributes(variableDeclaration->get_declarationModifier(),p->source_corresp.attributes);
        }

  // DQ (5/24/2015): Adding support for specification of thread local storage.
     if (p->decl_modifiers & DM_THREAD)
        {
#if DEBUG__convert_variable_prototype
          printf ("  (p->decl_modifiers & DM_THREAD) == true: p->decl_modifiers = %zu \n",p->decl_modifiers);
#endif
       // Note that I am using the newer C11 data member (though this is not C11 specific).
       // The alternative is to specify the thread local storage using the storage modifier (older appraoch).
          variableDeclaration->set_is_thread_local(true);
        }
#if DEBUG__convert_variable_prototype
     printf ("Leave convert_variable_prototype()\n");
#endif

     return variableDeclaration;
   }

}
