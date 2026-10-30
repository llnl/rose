
#include "edgRose.h"

#define DEBUG__convert_initializer 0

namespace EDG_ROSE_Translation {

SgInitializer * convert_initializer(an_init_kind kind, an_initializer edg_init) {
#if DEBUG__convert_initializer
     printf ("Enter convert_initializer():\n");
     printf ("  kind = %s\n", summarize_init_kind_in_string(kind));
     printf ("  edg_init = %p\n", &edg_init);
#endif

  SgInitializer * initializer = nullptr;
  switch (kind) {
    case initk_none:
    case initk_function_local:
        break;

    case initk_static: {
      SgExpression* exp = convert_constant(edg_init.constant);
      initializer = wrap_expr_as_initializer(exp);
      break;
    }

    case initk_dynamic: {
      ROSE_ASSERT(edg_init.dynamic != NULL);
      a_type_ptr edg_type = NULL;
      if (edg_init.dynamic->variable != NULL) {
        edg_type = edg_init.dynamic->variable->type;
      } else {
        ROSE_ASSERT(edg_init.dynamic->kind == dik_constant);
        ROSE_ASSERT(edg_init.dynamic->variant.constant.ptr != NULL);
        ROSE_ASSERT(edg_init.dynamic->variant.constant.ptr->type != NULL);
        edg_type = edg_init.dynamic->variant.constant.ptr->type;
      }
      ROSE_ASSERT(edg_type != NULL);
      SgExpression* expr = convert_dynamic_initializer(edg_init.dynamic, edg_type);
      initializer = wrap_expr_as_initializer(expr);
      break;
    }

    default:
      ROSE_ABORT();
  }

#if DEBUG__convert_initializer
  printf ("  initializer = %p = %s\n", initializer, initializer ? initializer->class_name().c_str() : "");
  auto * soc = initializer ? initializer->get_startOfConstruct() : nullptr;
  auto * eoc = initializer ? initializer->get_endOfConstruct() : nullptr;
  printf ("    ->get_startOfConstruct() = %p\n", soc);
  printf ("      ->isCompilerGenerated() = %s\n", soc ? (soc->isCompilerGenerated() ? "True" : "False" ) : "None");
  printf ("    ->get_endOfConstruct()   = %p\n", eoc);
  printf ("      ->isCompilerGenerated() = %s\n", eoc ? (eoc->isCompilerGenerated() ? "True" : "False" ) : "None");
  printf ("Leave convert_initializer()\n");
#endif
  return initializer;
}

}

