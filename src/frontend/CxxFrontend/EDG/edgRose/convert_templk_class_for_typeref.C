
#include "edgRose.h"

#define DEBUG_convert_templk_class_for_typeref 0

// This function handles `templk_class` and `templk_member_class` when it is a type alias
SgTemplateTypedefDeclaration * EDG_ROSE_Translation::convert_templk_class_for_typeref(a_template_ptr p) {

#if DEBUG_convert_templk_class_for_typeref
  printf ("In convert_templk_class_for_typeref():\n");
#endif
  a_type_ptr type = p->prototype_instantiation.type;
  ROSE_ASSERT(type != NULL);

  a_type_ptr edg_base_type = type->variant.typeref.type;
  ROSE_ASSERT(edg_base_type != NULL);
#if DEBUG_convert_templk_class_for_typeref
  printf ("  edg_base_type = %p\n", edg_base_type);
#endif

  SgType* rose_base_type = convert_type(edg_base_type, false);
  ROSE_ASSERT(rose_base_type != NULL);
#if DEBUG_convert_templk_class_for_typeref
  printf ("  rose_base_type = %p = %s \n", rose_base_type, rose_base_type->class_name().c_str());
#endif

  SgName name = p->source_corresp.name;
  SgScopeStatement* scope = topScopeStack();
  ROSE_ASSERT(scope != NULL);

  ROSE_ASSERT(p->template_decl != NULL);
  ROSE_ASSERT(p->template_decl->param_list != NULL);
  SgTemplateParameterPtrList tpl_params = parse_template_parameters(p->template_decl->param_list, scope, NULL);

  SgTemplateTypedefDeclaration* templateTypedefDeclaration = SageBuilder::buildTemplateTypedefDeclaration_nfi(name, rose_base_type, scope, false);
  ROSE_ASSERT(templateTypedefDeclaration != NULL);

  templateTypedefDeclaration->get_templateParameters() = tpl_params;

  if (!hasFirstNondefDeclForTemplate(p)) {
    ROSE_ASSERT(templateTypedefDeclaration->get_firstNondefiningDeclaration() != NULL);

    templateTypedefDeclaration->set_firstNondefiningDeclaration(templateTypedefDeclaration);
    templateTypedefDeclaration->set_definingDeclaration(templateTypedefDeclaration);

    setFirstNondefDeclForTemplate(p, templateTypedefDeclaration->get_firstNondefiningDeclaration());
  } else {
    SgDeclarationStatement* prev = getFirstNondefDeclForTemplate(p);
    ROSE_ASSERT(prev != NULL);

    templateTypedefDeclaration->set_firstNondefiningDeclaration(prev);
    templateTypedefDeclaration->set_definingDeclaration(prev);
  }

  for (SgTemplateParameterPtrList::iterator it = tpl_params.begin(); it != tpl_params.end(); it++) {
    if ((*it)->get_parent() != NULL) {
      SgDeclarationStatement * declstmt_parent = isSgDeclarationStatement((*it)->get_parent());
      ROSE_ASSERT(declstmt_parent != NULL);
      ROSE_ASSERT(declstmt_parent->get_firstNondefiningDeclaration() == templateTypedefDeclaration->get_firstNondefiningDeclaration());
    } else {
      (*it)->set_parent(templateTypedefDeclaration);
    }
  }

  string edg_template_string = p->text ? p->text : "";
  templateTypedefDeclaration->set_string(edg_template_string);

  set_file_info_for_corresp(templateTypedefDeclaration, p->source_corresp);

  return templateTypedefDeclaration;
}
