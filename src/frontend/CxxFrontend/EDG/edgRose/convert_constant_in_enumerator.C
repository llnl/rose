
#include "edgRose.h"

namespace EDG_ROSE_Translation {

#define DEBUG__convert_constant_in_enumerator 0

// The call to convert_constant should have set the original expression tree
#define FIXME__convert_constant_in_enumerator 1

SgExpression * convert_constant_in_enumerator(a_constant_ptr e) {
  ROSE_ASSERT (e != nullptr);
#if DEBUG__convert_constant_in_enumerator
  printf ("Enter convert_constant_in_enumerator():\n");
#endif

  SgExpression* result = set_file_info_for_enumerator(convert_constant(e), e);
#if DEBUG__convert_constant_in_enumerator
  printf ("  result = %p = %s \n",result,result->class_name().c_str());
#endif

  if (e->expr != nullptr) {
    SgValueExp * not_orig_value = isSgValueExp(result);
    ROSE_ASSERT(not_orig_value != nullptr);
#if FIXME__convert_constant_in_enumerator
#if DEBUG__convert_constant_in_enumerator
    printf ("  building the originalExpressionTree e->expr = %p \n",e->expr);
#endif
    SgExpression * orig_tree = convert_expression(e->expr);
    ROSE_ASSERT(orig_tree != nullptr);
    not_orig_value->set_originalExpressionTree(orig_tree);
    orig_tree->set_parent(not_orig_value);
#if DEBUG__convert_constant_in_enumerator
    printf ("  result->set_originalExpressionTree(%p : %s) \n", orig_tree, orig_tree->class_name().c_str());
#endif
#else
    SgExpression * orig_tree = not_orig_value->get_originalExpressionTree();
    ROSE_ASSERT(orig_tree != nullptr);
#endif
  }
  return result;
}

}
