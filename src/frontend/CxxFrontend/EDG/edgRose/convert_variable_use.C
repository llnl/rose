
#include "edgRose.h"

#define DEBUG_convert_variable_use 0

SgVariableSymbol*
EDG_ROSE_Translation::convert_variable_use(a_variable_ptr p) {
  // DQ (6/27/2019): Note that this function only builds a SgInitializedName where as the similar function 
  // convert_field_use() builds a SgVariableDeclaration.  This is inconsistant and something that we 
  // have to address in some of the other code because the parent of the SgInitializedName is not a SgVariableDeclaration.
  // This might be something that we might want to fix in the future.

  // FIXME: May need to fix this to handle forward references in classes

  ROSE_ASSERT(p != NULL);

#ifdef DEBUG_CONVERSION
  indent_line(); debug_print("variable use "); print_source_corresp(p->source_corresp); debug_print("\n");
#endif

#if DEBUG_convert_variable_use
  printf ("In convert_variable_use():\n");
  printf ("  p = %p : %s\n", p, p->source_corresp.name);
  printf ("  p->type = %p\n", p->type);
  printf ("  p->is_member_constant = %s \n", p->is_member_constant ? "true" : "false");
  printf ("  p->is_compound_literal = %s \n", p->is_compound_literal ? "true" : "false");
  printf ("  p->template_info = %p\n", p->template_info);
#endif

  SgVariableSymbol * sym = nullptr;
  if (variable_cache.find(p) != variable_cache.end()) {
    sym = variable_cache[p];
    ROSE_ASSERT(sym != nullptr);
  }
#if DEBUG_convert_variable_use
  printf ("  sym  = %p : %s\n", sym, sym ? sym->get_name().str() : "");
#endif

  if (sym == nullptr) {
    ROSE_ASSERT(!p->is_this_parameter);

    if (p->is_handler_param) {
      // happens with `ellipses_1.cxx_n2n.passed` in `tests/tools/RWSetTests`
      mprintf ("ERROR: In convert_variable_use(): p->is_handler_param == true \n");
    }

    SgName name = generateName(p);
    ROSE_ASSERT(name != "");
#if DEBUG_convert_variable_use
    printf ("  name = %s \n", name.str());
#endif

    bool is_reference_to_builtin_variables = name == string("__func__") ||
                                             name == string("__PRETTY_FUNCTION__") ||
                                             name == string("__FUNCTION__");
    sym = is_reference_to_builtin_variables ? use_of_builtin_variable(p, name) : use_of_unseen_variable(p, name);
    ROSE_ASSERT(sym != NULL);
    ROSE_ASSERT(variable_cache.find(p) != variable_cache.end());
  }
  return sym;
}
