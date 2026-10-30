
#include "edgRose.h"

#define DEBUG_convert_templk_variable 0

// This function handles `templk_variable` and `templk_static_data_member` for `parse_template` and `convert_template_prototype` (in `EDG_ROSE_Translation`)
SgVariableDeclaration * EDG_ROSE_Translation::convert_templk_variable(a_template_ptr p) {
#if DEBUG_convert_templk_variable
  printf ("In convert_templk_variable():\n");
#endif

  a_variable_ptr variable = p->prototype_instantiation.variable;
  ROSE_ASSERT(variable != NULL);

  SgVariableDeclaration * vdecl = convert_variable_prototype(variable, p);
  ROSE_ASSERT(vdecl != NULL);
#if DEBUG_convert_templk_variable
  printf ("  vdecl = %p = %s \n", vdecl, vdecl->class_name().c_str());
#endif
  SgTemplateVariableDeclaration * tpl_vdecl = isSgTemplateVariableDeclaration(vdecl);
  if (tpl_vdecl != NULL) {
    string edg_template_string = p->text ? p->text : "";
    tpl_vdecl->set_string(edg_template_string);
  }
  return vdecl;
}
