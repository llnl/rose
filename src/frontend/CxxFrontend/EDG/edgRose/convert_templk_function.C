
#include "edgRose.h"

// This function handles `templk_function` and `templk_member_function` for `parse_template` and `convert_template_prototype` (in `EDG_ROSE_Translation`)
SgFunctionDeclaration * EDG_ROSE_Translation::convert_templk_function(a_template_ptr p) {
  a_routine_ptr routine = p->prototype_instantiation.routine;
  ROSE_ASSERT(routine != NULL);

  bool forceTemplateFunctionDeclaration = true;
  SgFunctionDeclaration * result = convert_routine_prototype(routine,forceTemplateFunctionDeclaration,p);
  ROSE_ASSERT(result != NULL);

#if 1
  // TV: this code was only present in `EDG_ROSE_Translation::parse_template` for the `templk_function` case
  // DQ (3/16/2017): At this point the source position information has been set based on the template's inner 
  // function declaration, and does not include the template header.  If we want to report the start of the
  // template function as the start of the template header for the template function declaration, then we need
  // to reset the source position at the start_of_construct.
  set_starting_file_info_for_template(result,p);
#endif

  string edg_template_string = p->text ? p->text : "";
  SgTemplateFunctionDeclaration*       templateFunctionDeclaration       = isSgTemplateFunctionDeclaration(result);
  SgTemplateMemberFunctionDeclaration* templateMemberFunctionDeclaration = isSgTemplateMemberFunctionDeclaration(result);
  if (templateFunctionDeclaration != NULL) {
    templateFunctionDeclaration->set_string(edg_template_string);
  } else if (templateMemberFunctionDeclaration != NULL) {
    templateMemberFunctionDeclaration->set_string(edg_template_string);
  } else {
    std::cerr << "Error: result = " << std::hex << result << " : " << result->class_name() << std::endl;
    ROSE_ABORT();
  }
  return result;
}
