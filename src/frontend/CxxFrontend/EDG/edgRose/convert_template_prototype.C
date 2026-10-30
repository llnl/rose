
#include "edgRose.h"

#define DEBUG_EDG_ROSE_Translation__convert_template_prototype 0

SgDeclarationStatement*
EDG_ROSE_Translation::convert_template_prototype(a_template_ptr p) {

#ifdef DEBUG_CONVERSION
  indent_line(); debug_print("template prototype p = %p ",p);
  if (p != NULL) { print_source_corresp(p->source_corresp); debug_print("\n"); }
#endif

#if DEBUG_EDG_ROSE_Translation__convert_template_prototype
  printf("In convert_template_prototype:\n");
  printf("  p = %p\n", p);
  printf("  name = %s\n", p->source_corresp.name);
#endif
  ROSE_ASSERT (p != NULL);

  a_template_kind kind = p->kind;
#if DEBUG_EDG_ROSE_Translation__convert_template_prototype
  printf ("  kind = %d : %s \n", kind, summarize_template_kind_in_string((a_template_kind)kind));
#endif

  SgScopeStatement* scopeOfTpl = getDeclarationScope(p->source_corresp);
  ROSE_ASSERT(scopeOfTpl != NULL);
#if DEBUG_EDG_ROSE_Translation__convert_template_prototype
  printf ("  scopeOfTpl = %p : %s \n", scopeOfTpl, scopeOfTpl->class_name().c_str());
#endif

  templateDeclarationStack.push_back(p);
  SgDeclarationScope * nonreal_decl_scope = get_nonreal_declaration_scope_for_template(p, "convert_template_prototype");

  SgDeclarationStatement* result = NULL;
  switch (kind) {
    case templk_function:
    case templk_member_function: {
      result = convert_templk_function(p);
      ROSE_ASSERT(hasFirstNondefDeclForTemplate(p));
      break;
    }
    case templk_variable:
    case templk_static_data_member: {
      result = convert_templk_variable(p);
      ROSE_ASSERT( hasFirstNondefDeclForTemplate(p) );
      break;
    }
    case templk_member_class:
    case templk_class: {
      result = convert_templk_class(p);
      ROSE_ASSERT(hasFirstNondefDeclForTemplate(p));
      break;
    }
    case templk_template_template_param: {
      SgName templateName = p->source_corresp.name;
      a_scope_ptr edg_parent_scope = p->source_corresp.parent_scope;
      ROSE_ASSERT(edg_parent_scope->kind == sck_template_declaration);
      ROSE_ASSERT(nonreal_scope_cache.find(edg_parent_scope) != nonreal_scope_cache.end());

      SgDeclarationScope * nrscope = nonreal_scope_cache[edg_parent_scope];
      ROSE_ASSERT(nrscope != NULL);

      SgNonrealDecl * nrdecl = SageBuilder::buildNonrealDecl(templateName, nrscope, nonreal_decl_scope);
      ROSE_ASSERT(nrdecl != NULL);
      set_generated_file_info_to_unparse(nrdecl);
      nrdecl->set_parent(nrscope);

      nrdecl->set_is_template_template_param(true);

      ROSE_ASSERT(nrdecl->get_nonreal_decl_scope() == nonreal_decl_scope);

      SgTemplateParameterPtrList & tpl_params = nrdecl->get_tpl_params();
      tpl_params = parse_template_parameters(p->template_decl->param_list, nonreal_decl_scope, nrdecl);

      result = nrdecl;
      setFirstNondefDeclForTemplate(p, nrdecl);

      break;
    }
    case templk_member_enum: {
      printf ("templk_member_enum: Case not handled! \n");
      ROSE_ABORT();
    }
    default: {
      printf ("Bad type kind: %d\n", kind);
      ROSE_ABORT();
    }
  }
  ROSE_ASSERT(result != NULL);
#if DEBUG_EDG_ROSE_Translation__convert_template_prototype
  printf ("In convert_template_prototype(p = %p) return result = %p = %s \n",p,result,result->class_name().c_str());
#endif

  unsigned int access_level = p->source_corresp.access;
  set_access_modifier(result,access_level);
#if DEBUG_EDG_ROSE_Translation__convert_template_prototype
  printf ("In convert_template_prototype(): access_level(edg) = %d result = %p = %s: access = %s \n",access_level,result,result->class_name().c_str(),result->get_declarationModifier().get_accessModifier().displayString().c_str());
#endif

  templateDeclarationStack.pop_back();
  attach_nonreal_declaration_scope_to_template(p, result, nonreal_decl_scope, "convert_template_prototype");

  return result;
}


