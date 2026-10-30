
#include "edgRose.h"

#define DEBUG__convert_field_use 0

namespace EDG_ROSE_Translation {

SgVariableSymbol* convert_field_use(a_field_ptr fld_ptr) {
  ROSE_ASSERT(fld_ptr != NULL);
#if DEBUG__convert_field_use
  printf ("Enter convert_field_use()\n");
  printf ("  fld_ptr = %p\n", fld_ptr);
#endif

  SgVariableSymbol * sym = field_cache[fld_ptr];
#if DEBUG__convert_field_use
  printf ("  sym = %p = %s\n", sym, sym->class_name());
#endif

  a_scope_ptr edg_parent_scope = fld_ptr->source_corresp.parent_scope;
  ROSE_ASSERT(edg_parent_scope != nullptr);
  ROSE_ASSERT(edg_parent_scope->kind == sck_class_struct_union);
  a_type_ptr edg_parent_type = edg_parent_scope->variant.assoc_type;
  ROSE_ASSERT(edg_parent_type != nullptr);
#if DEBUG__convert_field_use
  printf ("  edg_parent_scope = %p\n", edg_parent_scope);
  printf ("  edg_parent_type = %p\n", edg_parent_type);
#endif
  SgClassType * rose_parent_type = nullptr;
  if (type_translation_cache.find(edg_parent_type) == type_translation_cache.end()) {
    rose_parent_type = isSgClassType(convert_type(edg_parent_type, false));
  } else {
    rose_parent_type = isSgClassType(type_translation_cache[edg_parent_type]);
  }
  ROSE_ASSERT(rose_parent_type != NULL);
#if DEBUG__convert_field_use
  printf ("  rose_parent_type = %p = %s\n", rose_parent_type, rose_parent_type->class_name().c_str());
  printf ("    ->get_name() = %s\n", rose_parent_type->get_name().str());
#endif

  if (sym == NULL) {
    SgName name = generateName(fld_ptr);
    SgType* type = convert_type(fld_ptr->type);
    a_byte bit_size = fld_ptr->bit_size;
    bool is_bit_field = fld_ptr->is_bit_field;
    bool is_mutable = fld_ptr->is_mutable;

    SgScopeStatement* properScopeForFieldMembers = findAssociatedClassDeclaration(fld_ptr);
    ROSE_ASSERT(properScopeForFieldMembers != NULL);
    bool builtFromUseOnly = true;
    SgVariableDeclaration* decl = set_file_info_for_corresp(buildVariableDeclaration_nfi(name, type, NULL, properScopeForFieldMembers,builtFromUseOnly), fld_ptr->source_corresp);
    fixup_variable_or_field_declaration(decl, fld_ptr->type);

    SgInitializedName* in = set_file_info_for_corresp(getFirstInitializedName(decl), fld_ptr->source_corresp);
    set_file_info_for_corresp(decl->get_definition(), fld_ptr->source_corresp);
    ROSE_ASSERT (in);

    if (is_mutable) {
      in->get_storageModifier().setMutable();
      decl->get_declarationModifier().get_storageModifier().setMutable();
    }

    if (is_bit_field) {
      decl->set_bitfield(set_generated_file_info_to_unparse(buildUnsignedLongVal_nfi(bit_size, "")));
    }

    if (decl->get_bitfield() != NULL) {
      decl->get_bitfield()->set_parent(decl);
      ROSE_ASSERT(decl->get_bitfield()->get_parent() != NULL);
    }

    sym = getFirstVarSym(decl);
    ROSE_ASSERT (sym != NULL);
    field_cache[fld_ptr] = sym;
  }
#if DEBUG__convert_field_use
  printf ("  sym = %p = %s\n", sym, sym->class_name());
  printf ("Leave convert_field_use()\n");
#endif
  return sym;
}

}
