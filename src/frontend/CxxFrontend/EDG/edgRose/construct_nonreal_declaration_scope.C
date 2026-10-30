
#include "edgRose.h"

namespace EDG_ROSE_Translation {

#define DEBUG__construct_nonreal_declaration_scope 0

SgDeclarationScope * construct_nonreal_declaration_scope() {
#if DEBUG__construct_nonreal_declaration_scope
  printf("Enter construct_nonreal_declaration_scope()\n");
#endif
  SgDeclarationScope * nonreal_decl_scope = new SgDeclarationScope();
  SageInterface::setSourcePosition(nonreal_decl_scope);
  nonreal_decl_scope->get_startOfConstruct()->setCompilerGenerated();
  nonreal_decl_scope->get_endOfConstruct()->setCompilerGenerated();

  return nonreal_decl_scope;
}

}
