
#include "edgRose.h"

#define DEBUG__convert_routine_prototype 0

namespace EDG_ROSE_Translation {

static SgScopeStatement * determine_scope_of_function(
  a_routine_ptr p
) {
  SgScopeStatement* scopeOfFunction = getDeclarationScope(p->source_corresp);
  ROSE_ASSERT(scopeOfFunction != nullptr);
  if (il_header.source_language == sl_C && scopeOfFunction == SageBuilder::topScopeStack()) {
    SgScopeStatement* declarationScope = SageBuilder::getGlobalScopeFromScopeStack();
    scopeOfFunction = declarationScope;
  }
  a_scope_ptr edg_scope = p->source_corresp.parent_scope;
  if ( ( scopeOfFunction == nullptr || isSgGlobal(scopeOfFunction) ) && edg_scope != nullptr ) {
    if (nonreal_scope_cache.find(edg_scope) != nonreal_scope_cache.end()) {
      scopeOfFunction = nonreal_scope_cache[edg_scope];
      ROSE_ASSERT(scopeOfFunction != nullptr);
    } else if (scope_cache.find(edg_scope) != scope_cache.end()) {
      scopeOfFunction = scope_cache[edg_scope];
      ROSE_ASSERT(scopeOfFunction != nullptr);
    }
  }
  if ( scopeOfFunction == nullptr || ( isSgGlobal(scopeOfFunction) && p->source_corresp.is_class_member ) ) {
    scopeOfFunction = SageBuilder::topScopeStack();
    while (scopeOfFunction != nullptr) {
      if (isSgGlobal(scopeOfFunction) || isSgClassDefinition(scopeOfFunction) || isSgFunctionDefinition(scopeOfFunction) || isSgNamespaceDefinitionStatement(scopeOfFunction) )
        break;
      scopeOfFunction = scopeOfFunction->get_scope();
    }
  }

  return scopeOfFunction;
}

static void parse_tparam_targ_fparam_rtype(
  a_routine_ptr p,
  bool buildTemplateDeclaration,
  bool buildTemplateInstantiation,
  SgScopeStatement* scopeOfFunction,
  SgTemplateParameterPtrList & templateParameterList,
  SgTemplateArgumentPtrList & templateArgumentsList,
  SgFunctionParameterList * & param_list,
  SgType * & return_type
) {
  if (buildTemplateDeclaration)
    templateParameterList = parse_template_parameters(p, scopeOfFunction, nullptr);

  if (buildTemplateInstantiation)
    if (p->template_arg_list != nullptr)
      templateArgumentsList = parse_template_arguments(p->template_arg_list, scopeOfFunction);

  SgDeclarationScope * declarationScope = nullptr;
  param_list = convert_function_parameter_list(p, false, nullptr, declarationScope);
  ROSE_ASSERT(param_list != nullptr);

  a_type_ptr p_type = p->type;
  if (p_type->kind == tk_routine) {
    return_type = convert_type(p_type->variant.routine.return_type);
  } else {
    ROSE_ASSERT(p_type->kind == tk_typeref);
    p_type = p_type->variant.typeref.type;
    ROSE_ASSERT(p_type->kind == tk_routine);

    return_type = convert_type(p_type->variant.routine.return_type);
  }
  ROSE_ASSERT(return_type != nullptr);
}

static void setup_name_nondefn_firstdefn_and_caches(
  a_routine_ptr p,
  bool forceTemplateDeclaration,
  bool buildTemplateDeclaration,
  bool buildTemplateInstantiation,
  a_template_ptr edg_template,
  SgName & nameWithOutTemplateArguments,
  SgName & nameWithTemplateArguments,
  SgFunctionDeclaration * & first_nondefining_declaration,
  SgFunctionDeclaration * & return_defining_declaration,
  SgTemplateArgumentPtrList & templateArgumentsList // FIXME const
) {

  setDefiningAndFirstNondefiningDeclarations(p, forceTemplateDeclaration, first_nondefining_declaration, return_defining_declaration, edg_template);
  if (first_nondefining_declaration != nullptr) {
    if (buildTemplateDeclaration) {
      SgTemplateFunctionDeclaration*       templateFunctionDeclaration       = isSgTemplateFunctionDeclaration(first_nondefining_declaration);
      SgTemplateMemberFunctionDeclaration* templateMemberFunctionDeclaration = isSgTemplateMemberFunctionDeclaration(first_nondefining_declaration);
      if (templateFunctionDeclaration == nullptr && templateMemberFunctionDeclaration == nullptr) {
        first_nondefining_declaration = nullptr;
      }
    }
    if (buildTemplateInstantiation) {
      SgTemplateInstantiationFunctionDecl*       templateFunctionInstantiationDeclaration       = isSgTemplateInstantiationFunctionDecl(first_nondefining_declaration);
      SgTemplateInstantiationMemberFunctionDecl* templateMemberFunctionInstantiationDeclaration = isSgTemplateInstantiationMemberFunctionDecl(first_nondefining_declaration);
      if (templateFunctionInstantiationDeclaration == nullptr && templateMemberFunctionInstantiationDeclaration == nullptr) {
        first_nondefining_declaration = nullptr;
      }
    }
  }
  testFirstNondefiningDeclarationType(first_nondefining_declaration,buildTemplateDeclaration,buildTemplateInstantiation);

  if (first_nondefining_declaration != nullptr) {
    if (function_cache.find(p) == function_cache.end()) {
      SgSymbol* symbol = first_nondefining_declaration->get_symbol_from_symbol_table();
      ROSE_ASSERT(symbol != nullptr);
      function_cache[p] = symbol;
      ROSE_ASSERT(function_cache[p] != nullptr);
    }
    ROSE_ASSERT(function_cache[p] != nullptr);
  }

  nameWithOutTemplateArguments = generateName(p);
  nameWithTemplateArguments    = nameWithOutTemplateArguments;
  if (buildTemplateInstantiation) {
    nameWithTemplateArguments = appendTemplateArgumentsToName(nameWithOutTemplateArguments, templateArgumentsList);
    auto cached_name_it = function_name_cache.find(p);
    if (cached_name_it != function_name_cache.end()) {
      auto cached_name = cached_name_it->second;
      if (cached_name != nameWithTemplateArguments.str()) {
        mprintf ("WARNING: Cached template function instantiation name is different than locally computed one\n");
        mprintf ("         cached = %s\n", cached_name.c_str());
        mprintf ("         local  = %s\n", nameWithTemplateArguments.str());
        nameWithTemplateArguments = cached_name;
      }
    } else {
      function_name_cache[p] = nameWithTemplateArguments;
    }
  }
}

SgFunctionDeclaration * convert_routine_prototype(a_routine_ptr p, bool forceTemplateDeclaration, a_template_ptr edg_template) {
  ROSE_ASSERT(p != NULL);
#if DEBUG__convert_routine_prototype
  printf ("Enter convert_routine_prototype(): p->declared_type = %p \n",p->declared_type);
  printf ("  p                  = %p\n", p);
  printf ("    ->type           = %p\n", p->type);
  printf ("    ->declared_type  = %p\n", p->declared_type);
  printf ("    ->assoc_template = %p\n", p->assoc_template);
  printf ("  forceTemplateDeclaration = %s\n",forceTemplateDeclaration ? "true" : "false");
  printf ("  edg_template = %p\n", p);
#endif
     a_type_ptr p_type = p->type;
     ROSE_ASSERT(p_type != NULL);

     bool buildTemplateDeclaration   = false;
     bool buildTemplateInstantiation = false;
     setTemplateOrTemplateInstantiationFunctionGeneration(p,forceTemplateDeclaration,buildTemplateDeclaration,buildTemplateInstantiation);
#if DEBUG__convert_routine_prototype
     printf ("  buildTemplateInstantiation = %s \n",buildTemplateInstantiation ? "true" : "false");
     printf ("  buildTemplateDeclaration   = %s \n",buildTemplateDeclaration ? "true" : "false");
#endif

     if (buildTemplateInstantiation == true) {
       ROSE_ASSERT(p->is_template_function == true);
     } else if (buildTemplateDeclaration == false) {
       ROSE_ASSERT(p->is_template_function == false);
     }

     SgScopeStatement* scopeOfFunction = determine_scope_of_function(p);
     ROSE_ASSERT(scopeOfFunction != nullptr);

     SgTemplateParameterPtrList templateParameterList;
     SgTemplateArgumentPtrList templateArgumentsList;
     SgFunctionParameterList* param_list = nullptr;
     SgType* return_type = nullptr;
     parse_tparam_targ_fparam_rtype(
         p, buildTemplateDeclaration, buildTemplateInstantiation, scopeOfFunction,
         templateParameterList, templateArgumentsList, param_list, return_type
     );
     ROSE_ASSERT(param_list != nullptr);
     ROSE_ASSERT(return_type != nullptr);
#if DEBUG__convert_routine_prototype
     printf ("  param_list  = %p \n", param_list);
     printf ("  return_type = %p = %s\n", return_type, return_type->class_name().c_str());
     // TODO display templateParameterList
     // TODO display templateArgumentsList
#endif

     SgName nameWithOutTemplateArguments;
     SgName nameWithTemplateArguments;
     SgFunctionDeclaration* first_nondefining_declaration = nullptr;
     SgFunctionDeclaration* return_defining_declaration = nullptr;
     setup_name_nondefn_firstdefn_and_caches(
         p, forceTemplateDeclaration, buildTemplateDeclaration, buildTemplateInstantiation, edg_template,
         nameWithOutTemplateArguments, nameWithTemplateArguments,
         first_nondefining_declaration, return_defining_declaration,
         templateArgumentsList
     );
#if DEBUG__convert_routine_prototype
     printf ("  nameWithOutTemplateArguments = %s \n", nameWithOutTemplateArguments.str());
     printf ("  nameWithTemplateArguments    = %s \n", nameWithTemplateArguments.str());
#endif

     a_routine_type_supplement_ptr rout_type_sup = p_type->variant.routine.extra_info;
     ROSE_ASSERT(rout_type_sup != nullptr);
#if DEBUG__convert_routine_prototype
     printf ("  rout_type_sup->this_class = %s\n", rout_type_sup->this_class ? "true" : "false");
     printf ("  p->source_corresp.is_class_member = %s\n",p->source_corresp.is_class_member ? "true" : "false");
#endif

     SgFunctionDeclaration*               function_declaration                 = nullptr;
     SgTemplateFunctionDeclaration*       template_function_declaration        = nullptr;
     SgTemplateMemberFunctionDeclaration* template_member_function_declaration = nullptr;

     SgFunctionDeclaration * decl = nullptr;

     if (p->source_corresp.is_class_member) {
       unsigned int functionConstVolatileFlags = setupFunctionTypeModifiers(rout_type_sup);

       ROSE_ASSERT(scopeOfFunction != nullptr);
       ROSE_ASSERT(isSgClassDefinition(scopeOfFunction) != nullptr || isSgDeclarationScope(scopeOfFunction) != nullptr);

       SgMemberFunctionDeclaration* mdecl = nullptr;
       if (buildTemplateDeclaration) {
         template_member_function_declaration = SageBuilder::buildNondefiningTemplateMemberFunctionDeclaration(
             nameWithOutTemplateArguments, return_type, param_list, scopeOfFunction, NULL, functionConstVolatileFlags, &templateParameterList
         );
         ROSE_ASSERT(template_member_function_declaration != NULL);

         putFunctionInTemplateMap(p, edg_template, isSgFunctionDeclaration(template_member_function_declaration->get_firstNondefiningDeclaration()), NULL);

         mdecl = template_member_function_declaration;
         decl = template_member_function_declaration;

         for (SgTemplateParameterPtrList::iterator it = templateParameterList.begin(); it != templateParameterList.end(); it++)
           if ((*it)->get_parent() == nullptr)
             (*it)->set_parent(decl);
         setupDeclarationScope(template_member_function_declaration);
       } else {
         mdecl = SageBuilder::buildNondefiningMemberFunctionDeclaration(
             nameWithOutTemplateArguments, return_type, param_list, scopeOfFunction, nullptr, functionConstVolatileFlags, buildTemplateInstantiation, &templateArgumentsList
         );
         if (buildTemplateInstantiation) {
           SgTemplateInstantiationMemberFunctionDecl* memberFunctionTemplateInstantiation = isSgTemplateInstantiationMemberFunctionDecl(mdecl);
           ROSE_ASSERT(memberFunctionTemplateInstantiation != NULL);

           memberFunctionTemplateInstantiation->set_template_argument_list_is_explicit(p->expl_template_arg_list_used);
           ROSE_ASSERT(scopeOfFunction->lookup_function_symbol(nameWithTemplateArguments) != NULL);
           setTemplateFunctionDeclaration (p,memberFunctionTemplateInstantiation);
           ROSE_ASSERT(memberFunctionTemplateInstantiation->get_templateName().is_null() == false);
           ROSE_ASSERT(memberFunctionTemplateInstantiation->get_templateArguments().size() == templateArgumentsList.size());

           // Both the template name, template declaration, and the template arguments must be set.
           if (memberFunctionTemplateInstantiation->get_templateDeclaration() == NULL) {
             templateParameterList = parse_template_parameters(p,scopeOfFunction,NULL);
             ROSE_ASSERT(p->assoc_template == NULL);
             SgDeclarationScope * declarationScope = NULL;
             SgFunctionParameterList * extra_param_list = convert_function_parameter_list(p, false, nullptr, declarationScope);
             SgTemplateMemberFunctionDeclaration * templateMemberFunctionDeclaration = SageBuilder::buildNondefiningTemplateMemberFunctionDeclaration(
                 nameWithOutTemplateArguments, return_type, extra_param_list, scopeOfFunction, NULL, functionConstVolatileFlags, &templateParameterList
             );
             ROSE_ASSERT(templateMemberFunctionDeclaration != NULL);

             memberFunctionTemplateInstantiation->set_templateDeclaration(templateMemberFunctionDeclaration);

             for (SgTemplateParameterPtrList::iterator it = templateParameterList.begin(); it != templateParameterList.end(); it++)
               if ((*it)->get_parent() == nullptr)
                 (*it)->set_parent(templateMemberFunctionDeclaration);

             putFunctionInTemplateMap(p, edg_template, templateMemberFunctionDeclaration, NULL);
           }

           ROSE_ASSERT(memberFunctionTemplateInstantiation->get_templateDeclaration() != NULL);
           ROSE_ASSERT(memberFunctionTemplateInstantiation->get_templateName().is_null() == false);
         }

         ROSE_ASSERT(isSgMemberFunctionDeclaration(mdecl) != NULL);

         if (first_nondefining_declarations_for_functions.find(p) == first_nondefining_declarations_for_functions.end()) {
           first_nondefining_declarations_for_functions[p] = isSgFunctionDeclaration(mdecl->get_firstNondefiningDeclaration());
         }

         decl = mdecl;
         setupDeclarationScope(mdecl);
       }

       ROSE_ASSERT(isSgDeclarationScope(scopeOfFunction) || mdecl->get_associatedClassDeclaration() != NULL);
       function_declaration = mdecl;
       decl = mdecl;
     } else if (buildTemplateDeclaration) {
       template_function_declaration = SageBuilder::buildNondefiningTemplateFunctionDeclaration(
           nameWithOutTemplateArguments, return_type, param_list, scopeOfFunction, nullptr, &templateParameterList
       );
       ROSE_ASSERT(template_function_declaration != NULL);

       decl = template_function_declaration;

       putFunctionInTemplateMap(p, edg_template, isSgFunctionDeclaration(template_function_declaration->get_firstNondefiningDeclaration()), NULL);

       ROSE_ASSERT(decl->get_firstNondefiningDeclaration() != NULL);
       ROSE_ASSERT(decl->get_firstNondefiningDeclaration()->get_symbol_from_symbol_table() != NULL);
               
       for (SgTemplateParameterPtrList::iterator it = templateParameterList.begin(); it != templateParameterList.end(); it++)
         if ((*it)->get_parent() == nullptr)
           (*it)->set_parent(decl);
       setupDeclarationScope(template_function_declaration);
     } else {
       function_declaration = buildNondefiningFunctionDeclaration (nameWithOutTemplateArguments, return_type, param_list, scopeOfFunction, nullptr, buildTemplateInstantiation,&templateArgumentsList);
       if (function_declaration->get_definingDeclaration() != NULL) {
         ROSE_ASSERT(function_declaration->variant() == function_declaration->get_definingDeclaration()->variant());
       }

       if (buildTemplateInstantiation) {
         SgTemplateInstantiationFunctionDecl* templateFunctionInstantiationDeclaration = isSgTemplateInstantiationFunctionDecl(function_declaration);
         ROSE_ASSERT(templateFunctionInstantiationDeclaration != NULL);

         templateFunctionInstantiationDeclaration->set_template_argument_list_is_explicit(p->expl_template_arg_list_used);
         setTemplateFunctionDeclaration (p,templateFunctionInstantiationDeclaration);

         SgDeclarationStatement* templateDeclaration = templateFunctionInstantiationDeclaration->get_templateDeclaration();
         ROSE_ASSERT(templateDeclaration != NULL);

         SgTemplateFunctionDeclaration* templateFunctionDeclaration = isSgTemplateFunctionDeclaration(templateDeclaration);
         ROSE_ASSERT(templateFunctionDeclaration != NULL);
         SgTemplateInstantiationFunctionDecl* templateFunctionInstantiation = isSgTemplateInstantiationFunctionDecl(function_declaration);
         templateFunctionInstantiation->set_templateDeclaration(templateFunctionDeclaration);

         a_template_arg_ptr templateArgs = p->template_arg_list;
         if (templateArgs != NULL) {
           ROSE_ASSERT(scopeOfFunction != NULL);
           templateFunctionInstantiation->get_templateArguments() = templateArgumentsList;
           ROSE_ASSERT(templateFunctionInstantiation->get_templateArguments().empty() == false);

           setTemplateArgumentParents (templateArgumentsList,templateFunctionInstantiation);
         }

         ROSE_ASSERT(templateFunctionInstantiation != NULL);
         ROSE_ASSERT(templateFunctionInstantiation->get_templateArguments().empty() == false);
       }

       if (first_nondefining_declarations_for_functions.find(p) == first_nondefining_declarations_for_functions.end()) {
         first_nondefining_declarations_for_functions[p] = isSgFunctionDeclaration(function_declaration->get_firstNondefiningDeclaration());
       }

       ROSE_ASSERT(first_nondefining_declarations_for_functions.find(p) != first_nondefining_declarations_for_functions.end());
       ROSE_ASSERT(first_nondefining_declarations_for_functions[p] != NULL);

       decl = function_declaration;
       ROSE_ASSERT(decl != NULL);
       setupDeclarationScope(decl);
     }

     if (function_declaration != NULL) {
       set_function_modifier(function_declaration->get_functionModifier(), p);
       set_special_function_modifier (function_declaration->get_specialFunctionModifier(), p->special_kind);
     
     } else if (template_function_declaration != NULL) {
       set_function_modifier(template_function_declaration->get_functionModifier(), p);
       set_special_function_modifier(template_function_declaration->get_specialFunctionModifier(), p->special_kind);

     } else if (template_member_function_declaration != nullptr) {
       set_function_modifier(template_member_function_declaration->get_functionModifier(), p);
       set_special_function_modifier(template_member_function_declaration->get_specialFunctionModifier(), p->special_kind);

     } else {
       ROSE_ABORT();
     }

     if (first_nondefining_declaration == NULL) {
       first_nondefining_declaration = isSgFunctionDeclaration(decl->get_firstNondefiningDeclaration());
       function_cache[p] = isSgSymbol(first_nondefining_declaration->get_symbol_from_symbol_table());
     }
     ROSE_ASSERT(function_cache.find(p) != function_cache.end());
     ROSE_ASSERT(function_cache[p] != NULL);

     if (function_declaration != NULL) {
       set_routine_attributes(p,function_declaration);
     }

#if ENABLE_CUDA_SUPPORT
     set_routine_cuda_attributes(p,decl);
#endif

     set_file_info_for_corresp(decl, p->source_corresp);
     function_declaration_map[p].push_back(decl);

     if (buildTemplateDeclaration == false && buildTemplateInstantiation == false && p->source_corresp.is_class_member == false) {
       SgStorageModifier::storage_modifier_enum sm = convert_storage_class(p->storage_class);
       decl->get_declarationModifier().get_storageModifier().set_modifier(sm);
     }

#if GNU_EXTENSIONS_ALLOWED || REDEFINE_EXTNAME_PRAGMA_ENABLED
     a_const_char* asm_name = (p->gnu_extra_info != NULL) ? p->gnu_extra_info->asm_name : NULL;
     if (asm_name != NULL) {
       if (function_declaration != NULL) {
         function_declaration->set_asm_name(asm_name);
       } else {
         printf ("SgFunctionDeclaration::set_asm_name() not supported for SgTemplateFunctionDeclaration \n");
         ROSE_ABORT();
       }
     }
#endif

     if (p->is_declared_constexpr) {
       decl->set_is_constexpr(true);
     }

     test_for_correct_form_of_declaration(buildTemplateInstantiation,buildTemplateDeclaration,decl);

     set_linkage(p, decl);

     testAndMarkAsSpecialized(p, buildTemplateInstantiation, NULL, decl);

     ROSE_ASSERT(internal_intermediate_scope->symbol_table_size() == 0);

     ROSE_ASSERT(decl->get_firstNondefiningDeclaration()->get_symbol_from_symbol_table() != NULL);

     set_access_modifier(decl, p->source_corresp.access);

     if (p->source_corresp.is_class_member && p->type->variant.routine.extra_info->this_class == NULL) {
       decl->get_declarationModifier().get_storageModifier().setStatic();
     }
    
     if (p->type->kind == tk_routine && rout_type_sup->exception_specification != NULL) {
       if (!rout_type_sup->exception_specification->is_noexcept && !rout_type_sup->exception_specification->compiler_generated) {
         decl->get_declarationModifier().setThrow();
       }
     }
     if (rout_type_sup->trailing_return_type) {
       decl->set_using_new_function_return_type_syntax(true);
     }
     if (p->override) {
       decl->get_declarationModifier().setOverride();
     }
     if (p->final) {
       decl->get_declarationModifier().setFinal();
     }

     testEdgRoseMaps();

#if DEBUG__convert_routine_prototype
  printf ("Leave convert_routine_prototype()\n");
  printf ("  static = %s\n", decl->get_declarationModifier().get_storageModifier().isStatic() ? "true" : "false");
#endif
     return decl;
   }
}
