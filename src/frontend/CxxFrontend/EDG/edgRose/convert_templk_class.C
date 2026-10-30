
#include "edgRose.h"

#define DEBUG_convert_templk_class 0

// This function handles `templk_class` and `templk_member_class` for `parse_template` and `convert_template_prototype` (in `EDG_ROSE_Translation`)
SgDeclarationStatement * EDG_ROSE_Translation::convert_templk_class(a_template_ptr p) {
  a_type_ptr type = p->prototype_instantiation.type;
  ROSE_ASSERT(type != NULL);

  SgDeclarationStatement * result = (type->kind == tk_typeref) ?
                                        convert_templk_class_for_typeref(p) :
                                        convert_templk_class_for_class(p);
#if DEBUG_convert_templk_class
  printf ("In convert_template_prototype(): p = %p result = %p = %s \n",p,result,result->class_name().c_str());
#endif

  if (result->get_firstNondefiningDeclaration() != getFirstNondefDeclForTemplate(p)) {
    printf ("WARNING: template_template_parm example fails this test for result == first_nondefining_declarations_for_templates[p] \n");
    printf ("RESET: first_nondefining_declarations_for_templates[p = %p] = result = %p = %s \n",p,result,result->class_name().c_str());
    setFirstNondefDeclForTemplate(p, result->get_firstNondefiningDeclaration());
  }
  ROSE_ASSERT(result->get_firstNondefiningDeclaration() == getFirstNondefDeclForTemplate(p));

  return result;
}
