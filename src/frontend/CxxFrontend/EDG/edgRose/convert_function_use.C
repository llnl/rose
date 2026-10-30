
#include "edgRose.h"

#define DEBUG__convert_function_use 0

namespace EDG_ROSE_Translation {

SgSymbol* convert_function_use(a_routine_ptr p) {
#if DEBUG__convert_function_use
  printf ("ENTER convert_function_use()\n");
  printf ("  p = %p = %s \n", p, p->source_corresp.name != nullptr ? p->source_corresp.name : "null");
#endif
  SgSymbol* sym = nullptr;
  if (function_cache.find(p) != function_cache.end()) {
    sym = function_cache[p];
  }
#if DEBUG__convert_function_use
  printf ("  sym = %p\n", sym);
#endif
  if (sym == nullptr) {
    convert_routine_prototype(p);
    sym = function_cache[p];
  }
  ROSE_ASSERT(sym != nullptr);
#if DEBUG__convert_function_use
  printf ("  sym = %p\n", sym);
  printf ("LEAVE convert_function_use()\n");
#endif
  return sym;
}

}