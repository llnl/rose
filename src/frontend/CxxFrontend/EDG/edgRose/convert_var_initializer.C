
#include "edgRose.h"

namespace EDG_ROSE_Translation {

#define DEBUG__convert_var_initializer 0

SgInitializer* convert_var_initializer(a_variable_ptr edg_init) {
#if DEBUG__convert_var_initializer
  printf ("Enter convert_var_initializer()\n");
  printf ("  edg_init = %p\n", edg_init);
  printf ("  kind = %s\n", summarize_init_kind_in_string(edg_init->init_kind));
#endif

  SgInitializer * initializer = convert_initializer(edg_init->init_kind, edg_init->initializer);
  if (initializer != NULL) {
    set_file_info_end(initializer, edg_init->initializer_range.end);
    initializer->set_is_braced_initialized(edg_init->has_direct_braced_initializer);
  }

#if DEBUG__convert_var_initializer
  printf ("  initializer = %p = %s\n", initializer, initializer ? initializer->class_name().c_str() : "");
  auto * soc = initializer ? initializer->get_startOfConstruct() : nullptr;
  auto * eoc = initializer ? initializer->get_endOfConstruct() : nullptr;
  printf ("    ->get_startOfConstruct() = %p\n", soc);
  printf ("      ->isCompilerGenerated() = %s\n", soc ? (soc->isCompilerGenerated() ? "True" : "False" ) : "None");
  printf ("    ->get_endOfConstruct()   = %p\n", eoc);
  printf ("      ->isCompilerGenerated() = %s\n", eoc ? (eoc->isCompilerGenerated() ? "True" : "False" ) : "None");
  printf ("Leave convert_var_initializer()\n");
#endif

  return initializer;
}

}
