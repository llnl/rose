
#include "edgRose.h"

#define DEBUG_convert_templk_class_for_class 0

// This function handles `templk_class` and `templk_member_class` when it is a type alias
SgDeclarationStatement * EDG_ROSE_Translation::convert_templk_class_for_class(a_template_ptr p) {

  a_type_ptr type = p->prototype_instantiation.type;
  ROSE_ASSERT(type != NULL);

  SgDeclarationStatement* declstmt = convert_template_struct_secondary(type, SgClassDeclaration::e_class, p);
  SgTemplateClassDeclaration * templateClassDeclaration = isSgTemplateClassDeclaration(declstmt);
  ROSE_ASSERT(templateClassDeclaration != NULL);

  ROSE_ASSERT(templateClassDeclaration->get_definition() == NULL);
  ROSE_ASSERT(templateClassDeclaration->get_firstNondefiningDeclaration() != NULL);

  string edg_template_string = p->text ? p->text : "";
  templateClassDeclaration->set_string(edg_template_string);

  ROSE_ASSERT(matching_defining_and_nondefining_variants(templateClassDeclaration));

  if (templateClassDeclaration->get_parent() == NULL) {
    templateClassDeclaration->set_parent(topScopeStack());
  }

  return templateClassDeclaration;
}
