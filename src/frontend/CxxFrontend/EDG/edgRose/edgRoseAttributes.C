
#include "edgRose.h"

using namespace std;
using namespace SageInterface;
using namespace SageBuilder;
using namespace OmpSupport;

// DQ (3/24/2016): Adding Robb's message logging mechanism to contrl output debug message from the EDG/ROSE connection code.
#undef DEBUG
#include <Rose/Diagnostics.h>

// DQ (3/24/2016): Adding Robb's message logging mechanism to contrl output debug message from the EDG/ROSE connection code.
using namespace Rose::Diagnostics;

// DQ (3/24/2016): We need a special version of tis macro with specific name qualification for "mlog" so
// that we can avoid ambiguity when the SageBuilder namespace is used (since there is an mlog there too).
#undef mprintf
#define mprintf Rose::Diagnostics::mfprintf(EDG_ROSE_Translation::mlog[Rose::Diagnostics::DEBUG]) 


// *********************************************************************
// *********** START GNU and MS Attribute Handling Support *************
// *********************************************************************

// DQ (8/17/2014): Control output of debuing info.
#define MS_DECLSPEC_DEBUG 0

// DQ (10/22/2016): Added compile-time test for packing support.
#ifndef USER_CONTROL_OF_STRUCT_PACKING
   #error "USER_CONTROL_OF_STRUCT_PACKING must be defined for ROSE"
#else
#if !USER_CONTROL_OF_STRUCT_PACKING
   #error "USER_CONTROL_OF_STRUCT_PACKING is defined but not defined to be TRUE (must be defined to be TRUE for ROSE)"
#endif
#endif

// *******************  Available attributes in EDG  *********************
#if 0
typedef struct an_attr_descr *an_attr_descr_ptr;
typedef struct an_attr_descr {
  /* Data structure describing the name and kind of an attribute, the form
     of its arguments if any (i.e., its "signature"), and the modes in which
     that attribute should be recognized.  This defines the structure of the
     table known_attr_table (below) of recognizable attributes. */
  char		*name;
			/* The name of the attribute.  If an attribute name
			   can include optional leading/trailing underscores,
			   those underscores are not included here.  The name
			   may not be longer than MAX_ATTRIBUTE_NAME_LENGTH. */
  char		*sig;
			/* A compact encoding of the "signature" of this
			   attribute.  If sig is "", no attribute arguments
			   are permitted.  If sig starts with "?", arguments
			   are optional.  The arguments are described by a
			   parenthesized comma-separated list of codes (no
			   spaces are permitted):
			     "t": a type-id is expected
			     "ci": an integer constant is expected
			     "ct": an integer constant or a type is expected
			           (similar to a "sizeof(...)" argument)
			     "n": an identifier is expected
			     "sn": a narrow string literal is expected
			     "sx": a string literal is expected (wide/narrow)
			     "*": an arbitrary set of tokens is expected
			          (this can only be for the last argument)
			   A code can be followed by a "+" to indicate that
			   one or more arguments of that kind are expected.
			   A "?" indicates that the argument list may
                           terminate at that point.
			   Examples:
			     "(ci)": one integer constant required
			     "(?sn+)": one or more string literals are
			         optional, but the enclosing parentheses are
			         required (i.e., attr() or attr("str", "ing")
			         are okay, but just attr is not).
			     "(n?,t,ci)": an identifier is required; it can
			         optionally be followed by a type and an
			         integer constant (both or neither).
			     "?(n?,t?,ci)": either no argument list appears at
			         all, or an identifier appears, optionally
			         followed by a type, itself optionally followed
			         by an integer constant.
			*/
  char		*cond;
			/* A compact encoding of the condition in which this
			   attribute is accepted.  The encoding consists of an
			   optional prefix (see below) followed by a condition
			   string cstr.  cstr[0] indicates the attribute
			   family: 'c' for [[...]] (standard C++11), 'g' for
			   __attribute((...)) in GNU modes, 's' for
			   __attribute((...)) in Sun mode, and 'm' for
			   __declspec(...) in Microsoft mode.  cstr[1] is
			   '+' if the attribute only applies in C++ modes,
			   'c' if it only applies in C mode, and 'x' if it
			   applies in both C and C++ modes (some combinations
			   are impossible; e.g. "sc" is meaningless since there
			   is no "Sun C" mode).  For standard attributes, the
			   first two characters can be followed by a bracketed
			   namespace name.  E.g., if name is "test" and cstr
			   is "c+[xyz]", then this is a description entry for
			   [[xyz::test ... ]].  If cstr[0] is 'g' or 'm', the
			   first two characters can be followed by a
			   parenthesized range of applicable versions.  E.g.,
			   "gx(30100-39999)" means the attribute is valid in 
			   GNU C/C++ modes with gnu_version >= 30100 and
			   gnu_version < 40000.  Either end of the range can
			   be dropped; e.g., "mc(1400-)" means the attribute
			   is valid in Microsoft C mode with microsoft_version
			   >= 1400.
			   A prefix "1" means the attribute can appear at most
			   once per attribute group.  E.g., "1c+" indicates a
			   standard C++ attribute that can appear at most once
			   in a attribute group (and with no namespace). */
  an_attribute_kind
		attr_kind;
			/* The attribute kind to record in the corresponding
			   attribute entry. */
} an_attr_descr;

static an_attr_descr known_attr_table[] = {
  /* Standard attributes. */
#if USER_CONTROL_OF_STRUCT_PACKING
  { "align", "(ct)", "c+", ak_align },
#endif /* USER_CONTROL_OF_STRUCT_PACKING */
  { "base_check", "", "1c+", ak_base_check },
  { "carries_dependency", "", "1c+", ak_carries_dependency },
  { "final", "", "1c+", ak_final },
  { "hiding", "", "1c+", ak_hiding },
  { "noreturn", "", "1c+", ak_noreturn },
  { "override", "", "1c+", ak_override },

#if GNU_EXTENSIONS_ALLOWED
  /* GNU Attributes. */
  { "alias", "(sn)", "gx", ak_alias },
  { "aligned", "?(ci)", "gx", ak_align },
  { "alloc_size", "(ci?,ci)", "gx(40200-)", ak_alloc_size },
  { "always_inline", "", "gx", ak_always_inline },
  { "artificial", "", "gx(40000-)", ak_artificial },
#if GNU_X86_ATTRIBUTES_ALLOWED && !USE_X86_64
  { "cdecl", "", "gx", ak_cdecl },
#endif /* GNU_X86_ATTRIBUTES_ALLOWED && !USE_X86_64 */
  { "cleanup", "(n)", "gc", ak_cleanup },
  { "cold", "", "gx(40300-)", ak_cold },
  { "common", "", "gx", ak_common },
  { "const", "", "gx", ak_const },
#if GNU_INIT_PRIORITY_ATTRIBUTE_ALLOWED
  { "constructor", "?(ci)", "gx", ak_constructor },
#else /* !GNU_INIT_PRIORITY_ATTRIBUTE_ALLOWED */
  { "constructor", "", "gx", ak_constructor },
#endif /* GNU_INIT_PRIORITY_ATTRIBUTE_ALLOWED */
  { "deprecated", "?(sx)", "gx(30100-)", ak_deprecated },
#if GNU_INIT_PRIORITY_ATTRIBUTE_ALLOWED
  { "destructor", "?(ci)", "gx", ak_destructor },
#else /* !GNU_INIT_PRIORITY_ATTRIBUTE_ALLOWED */
  { "destructor", "", "gx", ak_destructor },
#endif /* GNU_INIT_PRIORITY_ATTRIBUTE_ALLOWED */
  { "error", "(sn)", "gx(40000-)", ak_error },
  { "externally_visible", "", "gx(40000-)", ak_externally_visible },
  { "flatten", "", "gx(40000-)", ak_flatten },
  { "format", "(n,ci,ci)", "gx", ak_format },
  { "format_arg", "(ci)", "gx", ak_format_arg },
  { "gnu_inline", "", "gx", ak_gnu_inline },
  { "hot", "", "gx(40300-)", ak_hot },
#if GNU_INIT_PRIORITY_ATTRIBUTE_ALLOWED
  { "init_priority", "(ci)", "g+", ak_init_priority },
#endif /* GNU_INIT_PRIORITY_ATTRIBUTE_ALLOWED */
  { "malloc", "", "gx", ak_malloc },
  { "may_alias", "", "gx(30300-)", ak_may_alias },
  { "mode", "(n)", "gx", ak_mode },
  { "no_instrument_function", "", "gx", ak_no_instrument_function },
  { "no_check_memory_usage", "", "gx", ak_no_check_memory_usage },
#if GNU_NAKED_ATTRIBUTE_ALLOWED
  { "naked", "", "gx", ak_naked },
#endif /* GNU_NAKED_ATTRIBUTE_ALLOWED */
  { "nocommon", "", "gx", ak_nocommon },
  { "noinline", "", "gx", ak_noinline },
  { "nonnull", "?(?ci+)", "gx", ak_nonnull },
  { "noreturn", "", "gx", ak_noreturn },
  { "nothrow", "", "gx", ak_nothrow },
#if USER_CONTROL_OF_STRUCT_PACKING
  { "packed", "", "gx", ak_packed },
#endif /* USER_CONTROL_OF_STRUCT_PACKING */
  { "pure", "", "gx", ak_pure },
  { "section", "(sn)", "gx", ak_section },
  { "sentinel", "?(ci)", "gx", ak_sentinel },
#if GNU_X86_ATTRIBUTES_ALLOWED && !USE_X86_64
  { "stdcall", "", "gx", ak_stdcall },
#endif /* GNU_X86_ATTRIBUTES_ALLOWED && !USE_X86_64 */
  { "strong", "", "gx", ak_strong },
#if THREAD_LOCAL_STORAGE_SPECIFIER_ALLOWED
  { "tls_model", "(sn)", "gx(30300-)", ak_tls_model },
#endif /* THREAD_LOCAL_STORAGE_SPECIFIER_ALLOWED */
  { "transparent_union", "", "gc", ak_transparent_union },
  { "unused", "", "gx", ak_unused },
  { "used", "", "gx", ak_used },
#if GNU_VECTOR_TYPES_ALLOWED
  { "vector_size", "(ci)", "gx", ak_vector_size },
#endif /* GNU_VECTOR_TYPES_ALLOWED */
#if GNU_VISIBILITY_ATTRIBUTE_ALLOWED
  { "visibility", "(sn)", "gx", ak_visibility },
#endif /* GNU_VISIBILITY_ATTRIBUTE_ALLOWED */
  { "volatile", "", "gx", ak_noreturn },
  { "warn_unused_result", "", "gx", ak_warn_unused_result },
  { "warning", "(sn)", "gx(40000-)", ak_warning },
  { "weak", "", "gx", ak_weak },
  { "weakref", "?(sn)", "gx(40100-)", ak_weakref },
#endif /* GNU_EXTENSIONS_ALLOWED */

#if MICROSOFT_EXTENSIONS_ALLOWED
  /* Microsoft __declspec attributes. */
  { "align", "(ci)", "mx", ak_align },
  { "allocate", "(sn)", "mx", ak_section },
  { "assembly_info", "(ci,ci)", "mx", ak_assembly_info },
  { "deprecated", "?(sx)", "mx", ak_deprecated },
  { "dllexport", "", "mx", ak_dllexport },
  { "dllimport", "", "mx", ak_dllimport },
  { "__edg_interior_ptr_alias", "", "m+", ak_edg_interior_ptr_alias },
  { "__edg_pin_ptr_alias", "", "m+", ak_edg_pin_ptr_alias },
  { "implementation_key", "(ci)", "mx", ak_implementation_key },
  { "intrin_type", "", "mx", ak_intrin_type },
  { "naked", "", "mx", ak_naked },
  { "noalias", "", "mx(1400-)", ak_noalias },
  { "noinline", "", "mx", ak_noinline },
  { "noreturn", "", "mx", ak_noreturn },
  { "nothrow", "", "m+", ak_nothrow },
  { "novtable", "", "m+", ak_novtable },
  { "property", "(*)", "m+", ak_property },
  { "restrict", "", "mx(1400-)", ak_restrict },
  { "safebuffers", "", "mx", ak_safebuffers },
  { "selectany", "", "mx", ak_selectany },
#if THREAD_LOCAL_STORAGE_SPECIFIER_ALLOWED
  { "thread", "", "mx", ak_thread },
#endif /* THREAD_LOCAL_STORAGE_SPECIFIER_ALLOWED */
  { "uuid", "(sn)", "m+", ak_uuid },
#endif /* MICROSOFT_EXTENSIONS_ALLOWED */

#if SUN_EXTENSIONS_ALLOWED && GNU_EXTENSIONS_ALLOWED
  /* Sun-mode GNU-style attributes. */
  { "aligned", "?(ci)", "s+", ak_align },
  { "constructor", "", "s+", ak_constructor },
  { "destructor", "", "s+", ak_destructor },
#if USER_CONTROL_OF_STRUCT_PACKING
  { "packed", "", "s+", ak_packed },
#endif /* USER_CONTROL_OF_STRUCT_PACKING */
#if GNU_VISIBILITY_ATTRIBUTE_ALLOWED
  { "visibility", "(sn)", "s+", ak_visibility },
#endif /* GNU_VISIBILITY_ATTRIBUTE_ALLOWED */
  { "weak", "", "s+", ak_weak },
#endif /* SUN_EXTENSIONS_ALLOWED && GNU_EXTENSIONS_ALLOWED */

  { NULL, NULL, NULL, ak_last }
};
#endif
// ***********************************************************************

// #if GNU_EXTENSIONS_ALLOWED

#if GNU_VISIBILITY_ATTRIBUTE_ALLOWED
SgDeclarationModifier::gnu_declaration_visability_enum
get_ELF_visibility_attribute( an_ELF_visibility_kind visibility, a_source_correspondence_ptr scp )
   {
  // Output the given visibility as an attribute specification (provided it is
  // not evk_unspecified).  scp points to the source correspondence of the IL 
  // entry (and is used to avoid emitting the attribute if it is implicit in 
  // its parent class or namespace).

  // Notes on GNU attribute for visability
  // This attribute affects the linkage of the declaration to which it is attached. There are 
  // four supported visibility_type values: default, hidden, protected or internal visibility.
  //
  //      void __attribute__ ((visibility ("protected")))
  //      f () { /* Do something. */; }
  //      int i __attribute__ ((visibility ("hidden")));
  //
  // The possible values of visibility_type correspond to the visibility settings in the ELF gABI. 

     SgDeclarationModifier::gnu_declaration_visability_enum return_value = SgDeclarationModifier::e_unknown_visibility;

     ROSE_ASSERT(scp != NULL);

  // For routine and variable entries that are members of classes or
  // namespaces, do not emit the visibility attribute if it is equivalent
  // to that implied by the surrounding scope.
     an_ELF_visibility_kind  default_visibility = (an_ELF_visibility_kind)evk_unspecified;

     if (scp->is_class_member == true)
        {
          default_visibility = (an_ELF_visibility_kind)(class_type_supp(scp_parent_class(scp))->ELF_visibility);
        }

     if (visibility == default_visibility)
        {
       // The visibility is already implicitly set through an attribute on the enclosing class.  Skipp marking the individual members.
          visibility = (an_ELF_visibility_kind)evk_unspecified;
        }

     switch (visibility)
        {
          case evk_unspecified:
            // No visibility attribute in source code.
               return_value = SgDeclarationModifier::e_unspecified_visibility;
               break;
          case evk_hidden:
            // Support for __attribute__((visibility("hidden")))
               return_value = SgDeclarationModifier::e_hidden_visibility;
               break;
          case evk_protected:
            // Support for __attribute__((visibility("protected")))
               return_value = SgDeclarationModifier::e_protected_visibility;
               break;
          case evk_internal:
            // Support for __attribute__((visibility("internal")))
               return_value = SgDeclarationModifier::e_internal_visibility;
               break;
          case evk_default:
            // Support for __attribute__((visibility("default")))
               return_value = SgDeclarationModifier::e_default_visibility;
               break;

          default:
               ERROR_MESSAGE(stderr, "Bad visibility specification %d \n", visibility);
               ROSE_ASSERT(false);
        }

#if 0
     printf ("In get_ELF_visibility_attribute(visibility,scp): return_value = %d \n",return_value);
#endif

     return return_value;
   }
#endif /* GNU_VISIBILITY_ATTRIBUTE_ALLOWED */

void
set_alignment_attributes( a_type_ptr type, SgDeclarationStatement* decl )
   {
#if 0
     printf ("In set_alignment_attributes(a_type_ptr): type = %p decl = %p = %s \n",type,decl,decl->class_name().c_str());
#endif
#if 0
     printf ("In set_type_attributes(): calling summarize_type() \n");
     summarize_type(type);
#endif

  // Output GNU "aligned" and "packed" attributes as recorded in the given type.
#if USER_CONTROL_OF_STRUCT_PACKING
     if (type->alignment_set_explicitly == true)
        {
       // Output an attribute to indicate the explicit alignment, supports "__attribute__((__aligned__)(N))"
       // form_unsigned_argument_attribute("__aligned__",(a_host_large_unsigned)type->alignment);
#ifdef DEBUG_CONVERSION
          printf ("In set_alignment_attributes(a_type_ptr): Detected attribute(__aligned__): type->alignment = %" PRIuPTR " \n",(size_t)(type->alignment));
          printf ("   --- decl = %p = %s \n",decl,decl->class_name().c_str());
#endif
          decl->get_declarationModifier().get_typeModifier().set_gnu_attribute_alignment((a_host_large_unsigned)type->alignment);
        }

#if GNU_EXTENSIONS_ALLOWED
     if ((is_immediate_class_type(type) && type->variant.class_struct_union.is_packed) || (is_immediate_enum_type(type) && type->variant.integer.packed))
        {
       // Support for "__attribute__((__packed__))"
       // Note that #pragma paked(n) forms an alternative packing specification mechanism
       // form_simple_attribute("__packed__");
#ifdef DEBUG_CONVERSION
          printf ("In set_alignment_attributes(a_type_ptr): decl = %p = %s Detected attribute(__packed__) \n",decl,decl->class_name().c_str());
#endif
          decl->get_declarationModifier().get_typeModifier().setGnuAttributePacked();
#if 0
          printf ("Exiting as a test! \n");
          ROSE_ASSERT(false);
#endif
        }
#endif

#endif
   }


void
set_alignment_attributes( a_variable_ptr variable, SgInitializedName* initializedName )
   {
  // Output GNU "aligned" and "packed" attributes as recorded in the given variable.
#if USER_CONTROL_OF_STRUCT_PACKING

      ROSE_ASSERT(variable != NULL);

#if 0
     printf ("In set_alignment_attributes(a_variable_ptr): variable->alignment = %d \n",(int)variable->alignment);
#endif
#if 0
     printf ("set_alignment_attributes(a_variable_ptr): calling summarize_variable() \n");
     summarize_variable(variable);
#endif

     if (variable->alignment != 0)
        {
       // Output an attribute to indicate the explicit alignment, supports "__attribute__((__aligned__)(N))"
       // form_unsigned_argument_attribute("__aligned__",(a_host_large_unsigned)type->alignment);
       // decl->get_declarationModifier().get_typeModifier().set_gnu_attribute_alignment((a_host_large_unsigned)variable->alignment);
#ifdef DEBUG_CONVERSION
          printf ("In set_alignment_attributes(a_variable_ptr): Detected attribute(__aligned__): variable->alignment = %" PRIuPTR " \n",(size_t)(variable->alignment));
#endif
          initializedName->set_gnu_attribute_alignment((a_host_large_unsigned)variable->alignment);
#if 0
          printf ("In set_alignment_attributes(a_variable_ptr): Alignment was specified! \n");
#endif
          if (variable->source_corresp.attributes != NULL)
             {
            // Loop over all attributes (there can be several, and the noreturn need not be the first one).
               an_attribute_ptr tmp_attribute = variable->source_corresp.attributes;
               ROSE_ASSERT(tmp_attribute != NULL);

               bool c11_alignas_attribute_found = false;
               string name;
               do {
                    name = tmp_attribute->name;
                    if (name == "_Alignas")
                       {
                         c11_alignas_attribute_found = true;
                       }
                      else
                       {
                         tmp_attribute = tmp_attribute->next;
                       }
                  }
               while (c11_alignas_attribute_found == false && tmp_attribute != NULL);

            // string name = variable->source_corresp.attributes->name;
#if 0
               printf ("In set_alignment_attributes(a_variable_ptr): (attributes != NULL): name = %s \n",name.c_str());
#endif
            // if (name == "_Alignas")
               if (c11_alignas_attribute_found == true)
                  {
                    ROSE_ASSERT(string(tmp_attribute->name) == "_Alignas");

                 // This is the C11 construction.
                    initializedName->set_using_C11_Alignas_keyword(true);

                    an_attribute_arg_ptr attribute_arg = tmp_attribute->arguments;
                    if (attribute_arg != NULL)
                       {
                      // Check if the operand is a constant or type (there is only a single argument for an alignment attribute).
                         if (attribute_arg->kind == aak_constant || attribute_arg->kind == aak_type)
                            {
                              SgNode* constant_or_type = NULL;
                              if (attribute_arg->kind == aak_constant)
                                 {
                                   ROSE_ASSERT(attribute_arg->variant.constant != NULL);
                                   constant_or_type = EDG_ROSE_Translation::convert_constant(attribute_arg->variant.constant);
                                   constant_or_type->set_parent(initializedName); // Needed for expressions
                                   ROSE_ASSERT(constant_or_type != NULL);
                                 }
                                else
                                 {
                                   if (attribute_arg->kind == aak_type)
                                      {
                                        ROSE_ASSERT(attribute_arg->variant.type != NULL);
                                        constant_or_type = EDG_ROSE_Translation::convert_type(attribute_arg->variant.type);
                                        ROSE_ASSERT(constant_or_type != NULL);
                                      }
                                     else
                                      {
                                        printf ("Error: _Alignas attribute argument operand is neither a constant nor a type. \n");
                                        ROSE_ASSERT(false);
                                      }
                                 }
                              initializedName->set_constant_or_type_argument_for_Alignas_keyword(constant_or_type);
                            }
                       }
#if 0
                    printf ("Exiting as a test! \n");
                    ROSE_ASSERT(false);
#endif
                  }
                 else
                  {
                 // This is an unknown construction.
                  }
             }
        }

  // DQ (12/30/2013): Detect this unimplemented feature.
     a_type_ptr type = variable->type;
     ROSE_ASSERT(type != NULL);

#if 0
     printf ("set_alignment_attributes(a_variable_ptr): calling summarize_type() \n");
     summarize_type(type);
#endif

#if GNU_EXTENSIONS_ALLOWED
     if ((is_immediate_class_type(type) && type->variant.class_struct_union.is_packed) || (is_immediate_enum_type(type) && type->variant.integer.packed))
        {
       // DQ (1/3/2014): This case is demonstrated in compiling busy-box using the system ethernet.h header file.
       // I think this has to be handled in the type declaration instead of at the variable declaration.
       // So I think we should not be processing the type here.

          printf ("NOTE: In set_alignment_attributes(a_variable_ptr,SgInitializedName*): Packing in set_alignment_attributes( a_variable_ptr variable, SgInitializedName* initializedName ): not implemented! \n");
       // ROSE_ASSERT(false);
        }
#endif
#if 0
     if ((is_immediate_class_type(type) && type->variant.class_struct_union.is_packed) || (is_immediate_enum_type(type) && type->variant.integer.packed))
        {
       // Support for "__attribute__((__packed__))"
       // Note that #pragma paked(n) forms an alternative packing specification mechanism
       // form_simple_attribute("__packed__");
          decl->get_declarationModifier().get_typeModifier().setGnuAttributePacked();
        }
#endif
#endif

     if (variable->source_corresp.attributes != NULL)
        {
#if MS_DECLSPEC_DEBUG
          printf ("In set_alignment_attributes(): Detected valid attributes \n");
#endif
          SgVariableDeclaration* variableDeclaration = isSgVariableDeclaration(initializedName->get_parent());

       // This might fail for SgInitializedName objects defined in function parameters.  I need to review 
       // which attributes can be applied to function parameters.
          ROSE_ASSERT(variableDeclaration != NULL);
          set_ms_attributes(variableDeclaration->get_declarationModifier(),variable->source_corresp.attributes);
        }
   }


void
set_routine_type_attributes( a_type_ptr type, SgFunctionDeclaration* decl )
   {
  // Output GNU attributes that apply to the indicated type (which must be a routine type).
     a_routine_type_supplement_ptr rtsp = skip_typerefs(type)->variant.routine.extra_info;

     set_alignment_attributes(type,decl);

#if GNU_EXTENSIONS_ALLOWED
     if (rtsp->result_should_be_used == true)
        {
       // Support for "__attribute__((__warn_unused_result__))"
#ifdef DEBUG_CONVERSION
          printf ("In set_routine_type_attributes(): Detected attribute(__warn_unused_result__) \n");
#endif
          decl->get_declarationModifier().get_typeModifier().setGnuAttributeWarnUnusedResult();
        }
#endif
     if (rtsp->does_not_return == true)
        {
       // Support for "__attribute__((__noreturn__))"
#ifdef DEBUG_CONVERSION
          printf ("In set_routine_type_attributes(): Detected attribute(__noreturn__) \n");
#endif
#if 0
          printf ("In set_routine_type_attributes(): calling summarize_type() \n");
          summarize_type(type);
#endif
          decl->get_declarationModifier().get_typeModifier().setGnuAttributeNoReturn();
#if 0
          printf ("Exiting as a test of attribute(__noreturn__) \n");
          ROSE_ASSERT(false);
#endif
        }

#if GNU_EXTENSIONS_ALLOWED
     if (rtsp->is_const == true)
        {
       // Support for "__attribute__((__const__))"
#ifdef DEBUG_CONVERSION
          printf ("In set_routine_type_attributes(): Detected attribute(__const__) \n");
#endif
          decl->get_declarationModifier().get_typeModifier().setGnuAttributeConst();
        }
#endif

#if GNU_X86_ATTRIBUTES_ALLOWED && !USE_X86_64
     switch (rtsp->calling_convention)
        {
          case cc_default:
            // No attribute to in source code
               break;

          case cc_cdecl:
            // On the Intel 386, the cdecl attribute causes the compiler to assume that the calling 
            // function will pop off the stack space used to pass arguments. This is useful to 
            // override the effects of the -mrtd switch. 
               decl->get_declarationModifier().get_typeModifier().setGnuAttributeCdecl();
               break;

          case cc_fastcall:
            // A Microsoft-only calling convention.  These aren't generated for the GNU C compiler.
               break;

          case cc_stdcall:
            // form_simple_attribute("__stdcall__");
               decl->get_declarationModifier().get_typeModifier().setGnuAttributeStdcall();
               break;

          case cc_thiscall:
            // A Microsoft-only calling convention.  These aren't generated for the GNU C compiler.
               break;

          default: 
               ERROR_MESSAGE(stderr, "Bad calling convention specification %d \n", rtsp->calling_convention);
               ROSE_ASSERT(false);
        }
#endif

  // Generate any needed "nonnull" attributes
     a_param_type_ptr  ptp = rtsp->param_type_list;
     unsigned int      p = 1;
     for (; ptp != NULL; ptp = ptp->next, ++p)
        {
#if GNU_EXTENSIONS_ALLOWED
          if (ptp->nonnull == true)
             {
            // The nonnull attribute specifies that some function parameters should be non-null pointers. For instance, the declaration:
            //      extern void *my_memcpy (void *dest, const void *src, size_t len) __attribute__((nonnull (1, 2)));
            // causes the compiler to check that, in calls to my_memcpy, arguments dest and src are non-null. If the compiler determines 
            // that a null pointer is passed in an argument slot marked as non-null, and the -Wnonnull option is enabled, a warning is issued.
            // The compiler may also choose to make optimizations based on the knowledge that certain function arguments will not be null.

            // form_unsigned_argument_attribute("nonnull", (a_host_large_unsigned)p);

            // Set that the __nonnull__ attribute was used for this function.
#ifdef DEBUG_CONVERSION
          printf ("In set_routine_type_attributes(): Detected attribute(__nonnull__) \n");
#endif
               decl->get_declarationModifier().get_typeModifier().setGnuAttributeNonnull();

#ifdef DEBUG_CONVERSION
               printf ("Warning: support for gnu attribute __nonnull__ is incomplete, parameters that are nonnull are not recorded. \n");
#endif
            // ROSE_ASSERT(false);
             }
#endif
#if GNU_EXTENSIONS_ALLOWED
       // DQ (9/2/2012): Added more gnu attribute support.
          if (ptp->is_transparent == true)
             {
#ifdef DEBUG_CONVERSION
               printf ("In set_routine_type_attributes(): Detected __transparent_union__  (not yet supported) \n");
            // printf ("Warning: support for gnu attribute __transparent__ is not set. \n");
#endif
             }
#endif
        }

#if GNU_EXTENSIONS_ALLOWED
  // Generate the "sentinel" attribute if needed:
     if (rtsp->sentinel_pos != 0)
        {
       // This function attribute ensures that a parameter in a function call is an explicit NULL. 
       // The attribute is only valid on variadic functions. By default, the sentinel is located 
       // at position zero, the last parameter of the function call. If an optional integer position 
       // argument P is supplied to the attribute, the sentinel must be located at position P counting 
       // backwards from the end of the argument list.

       // Note that the EDG representation is "one off" compared to the source form:
       // I.e., "sentinel(0)" in the source is corresponds to sentinel_pos == 1
       // to reserve sentinel_pos == 0 as a representation for "no sentinel".
       // form_unsigned_argument_attribute("sentinel",(a_host_large_unsigned)(rtsp->sentinel_pos-1) /*lint --e(571)*/);
#ifdef DEBUG_CONVERSION
          printf ("In set_routine_type_attributes(): Detected attribute(__sentinel__) \n");
#endif
          decl->get_declarationModifier().get_typeModifier().set_gnu_attribute_sentinel((a_host_large_unsigned)(rtsp->sentinel_pos-1));
        }
#endif

     if (type->source_corresp.attributes != NULL)
        {
#if MS_DECLSPEC_DEBUG
          printf ("In set_routine_type_attributes(): Detected valid attributes \n");
#endif
       // This might fail for SgInitializedName objects defined in function parameters.  I need to review 
       // which attributes can be applied to function parameters.
          ROSE_ASSERT(decl != NULL);
          set_ms_attributes(decl->get_declarationModifier(),type->source_corresp.attributes);
        }
   }


void 
set_type_attributes( a_type_ptr type, SgDeclarationStatement* decl )
   {
  // Output GNU attributes that apply to the indicated type.

  // Used in enum, class declarations, and typedefs.

  // DQ (6/8/2013): I think we can specify this.
     ROSE_ASSERT(decl != NULL);

#ifdef DEBUG_CONVERSION
     printf ("In set_type_attributes(type = %p, decl = %p = %s) \n",type,decl,decl->class_name().c_str());
#endif
#if 0
     printf ("In set_type_attributes(): calling summarize_type() \n");
     summarize_type(type);
#endif

  /* First emit the attributes that when appearing on a typedef would be
     recorded in the typedef entry itself (as opposed to the underlying type). */
     if (type->kind != (a_type_kind)tk_routine)
        {
          set_alignment_attributes(type,decl);
        }
       else
        {
       /* For routine types, the alignment attributes are emitted by the call
          to form_routine_type_attributes (below). */
        }

#if GNU_VISIBILITY_ATTRIBUTE_ALLOWED
     if (is_immediate_class_type(type) == true) 
        {
       // form_ELF_visibility_attribute(class_type_supp(type)->ELF_visibility,&type->source_corresp);
#ifdef DEBUG_CONVERSION
          printf ("In set_type_attributes(): Detected attribute(__immediate_class__): set visibility attribute \n");
#endif
          decl->get_declarationModifier().set_gnu_attribute_visability(
              get_ELF_visibility_attribute( (an_ELF_visibility_kind)(class_type_supp(type)->ELF_visibility), &type->source_corresp)
          );
        }
#endif

#if GNU_EXTENSIONS_ALLOWED
     if (type->variables_are_implicitly_referenced == true)
        {
       /* Output the "unused" attribute. */
       // form_simple_attribute("__unused__");
#ifdef DEBUG_CONVERSION
          printf ("In set_type_attributes(): Detected attribute(__unused__) \n");
#endif
          decl->get_declarationModifier().get_typeModifier().setGnuAttributeUnused();
        }
#endif

  /* The following attributes would be recorded on the underlying type if the attribute appeared on a typedef. */
     type = skip_typerefs(type);
#if GNU_EXTENSIONS_ALLOWED
     if (type->kind == (a_type_kind)tk_union && type->variant.class_struct_union.is_transparent)
        {
       // form_simple_attribute("__transparent_union__");
#ifdef DEBUG_CONVERSION
          printf ("In set_type_attributes(): Detected __transparent_union__  calling setGnuAttributeTransparentUnion() \n");
#endif
          decl->get_declarationModifier().get_typeModifier().setGnuAttributeTransparentUnion();
        }
#endif
     if (is_pointer_type(type) && is_function_type(type_pointed_to(type)))
        {
       // Need to search for the type so that we can identify the declaration???
       // form_routine_type_attributes(f_skip_typerefs(type_pointed_to(type)));

#ifdef DEBUG_CONVERSION
          printf ("Sorry, GNU function type specific type attributes not implemented ... \n");
#endif

       // DQ (1/5/2009): Commented out assertion so we could process header files (called by test2001_24.C).
       // ROSE_ASSERT(false);
        }

#if GNU_EXTENSIONS_ALLOWED
  // DQ (10/22/2016): This is specific to packed attributes in EDG 4.11 and later, changed from EDG 4.9.
     if (type->kind == tk_integer && type->variant.integer.packed == true)
        {
       // DQ (3/15/2017): Adding support for message streams.
          mprintf ("In set_type_attributes(): Found packed attribute! \n");
        }
#endif

#if 0
     if (type->is_packed == true)
        {
          SgInitializedName* initializedName = SageInterface::getFirstInitializedName(decl);
          ROSE_ASSERT(initializedName != NULL);
          initializedName->setGnuAttributePacked();
        }
#endif

     if (type->source_corresp.attributes != NULL)
        {
#if MS_DECLSPEC_DEBUG
          printf ("In set_type_attributes(): Detected valid attributes \n");
#endif
       // This might fail for SgInitializedName objects defined in function parameters.  I need to review 
       // which attributes can be applied to function parameters.
          ROSE_ASSERT(decl != NULL);
          set_ms_attributes(decl->get_declarationModifier(),type->source_corresp.attributes);
        }
   }


// We might want to provide a reference to a SgDeclarationModifier instead of a SgInitializedName!
void
set_variable_attributes( a_variable_ptr var, SgInitializedName* initializedName )
   {
  // Output the GNU attributes that apply to the indicated variable.

     ROSE_ASSERT(var != NULL);
     ROSE_ASSERT(initializedName != NULL);

  // DQ (1/6/2014): Added assertion.
     ROSE_ASSERT(var->type != NULL);

#if 0
     printf ("In set_variable_attributes(): var = %p var->type = %p initializedName = %s \n",var,var->type,initializedName->get_name().str());
#endif

  // Has the associated declaration be set yet?
  // Since we can have multiple variables associated with a single SgVariableDeclaration, 
  // we have to store the GNU attribues directly with the variable (SgInitializedName) 
  // and so we can't use the SgDeclarationModifier on the SgVariableDeclaration!
     ROSE_ASSERT(initializedName->get_declaration() != NULL);

#if USER_CONTROL_OF_STRUCT_PACKING
#if 0
     printf ("In set_variable_attributes(a_variable_ptr,SgInitializedName*): Gnu attribute __aligned__ SgInitializedName alignment = %d \n",(int)((a_host_large_unsigned)var->alignment));
#endif

  // DQ (3/1/2013): In EDG this type of the alignment specification is: unsigned char, so we can only detect if the alignment is used by detecting a non-zero value.
  // Where alignment is used for types, there is an additional flag to specify if the alignment directive is explicly present.  For variables it is by default 0
  // in EDG and thus we only record non-zero values in the ROSE AST.
  // if (var->alignment != 0)
        {
       /* Output the alignment attribute. */
       // form_unsigned_argument_attribute("__aligned__",(a_host_large_unsigned)var->alignment);

       // DQ (2/27/2013): Added support for attribute handling on variables in variable declarations.
       // In ROSE this value is a signed short and by default is specified to be a value of -1 (which indicates that it was not specified).
          set_alignment_attributes(var,initializedName);
#if 0
          printf ("Sorry, not implemented! \n");
          ROSE_ASSERT(false);
#endif
        }
#endif /* USER_CONTROL_OF_STRUCT_PACKING */

#if GNU_INIT_PRIORITY_ATTRIBUTE_ALLOWED
     if (var->init_priority != 0)
        {
       /* The init_priority is a C++-only attribute; it is ignored with a
          warning by GNU C compilers.  To avoid the warning, we do not emit it
          in the C-generating back end.  (IL lowering ensures the
          initializations are performed in the right order.) */
       // form_unsigned_argument_attribute("__init_priority__", (a_host_large_unsigned)var->init_priority);
#ifdef DEBUG_CONVERSION
          printf ("In set_variable_attributes(): Detected attribute(__init_priority__) \n");
#endif
          initializedName->set_gnu_attribute_initialization_priority((a_host_large_unsigned)var->init_priority);
        }
#endif /* GNU_INIT_PRIORITY_ATTRIBUTE_ALLOWED */

#if GNU_EXTENSIONS_ALLOWED
     if (var->cleanup_routine != NULL)
        {
       // octl->output_str("__attribute__((cleanup("); form_unqualified_name(&var->cleanup_routine->source_corresp,iek_routine); octl->output_str(")))");

#ifdef DEBUG_CONVERSION
          printf ("In set_variable_attributes(): Detected attribute(__cleanup__) \n");
#endif

       // This should perhaps be a pointer to the function and not the function name!
          initializedName->set_gnu_attribute_cleanup_function("cleanup function name not stored yet");
        }
#if GNU_VISIBILITY_ATTRIBUTE_ALLOWED
     initializedName->set_gnu_attribute_visability(
         get_ELF_visibility_attribute(
             (an_ELF_visibility_kind)(var->ELF_visibility), &var->source_corresp
         )
     );
#endif

     if (var->is_weak && !var->is_weakref)
        {
       // The "weakref" attribute implies the "weak" attribute: Should we set both?
       // form_simple_attribute("__weak__");
#ifdef DEBUG_CONVERSION
          printf ("In set_variable_attributes(): Detected attribute(__weak__) \n");
#endif
          initializedName->setGnuAttributeWeak();
        }

     if (var->has_gnu_used_attribute == true)
        {
       // form_simple_attribute("__used__");
#ifdef DEBUG_CONVERSION
          printf ("In set_variable_attributes(): Detected attribute(__used__) \n");
#endif
          initializedName->setGnuAttributeUsed();
        }
#endif

#if GNU_EXTENSIONS_ALLOWED

     if (var->section != NULL)
        {
       // Support for __attribute__((__section__))
       // Support for named sections, as in: extern void foobar (void) __attribute__ ((section ("bar")));
       // form_string_argument_attribute("__section__", var->section);
#ifdef DEBUG_CONVERSION
          printf ("In set_variable_attributes(): Detected attribute(__section__) \n");
#endif
          initializedName->set_gnu_attribute_section_name(var->section);
        }

     if (var->aliased_variable != NULL)
        {
#ifdef DEBUG_CONVERSION
          printf ("In set_variable_attributes(): Detected attribute(__alias__) \n");
#endif
       // char *attr_str = var->is_weakref ? (char*)"__weakref__" : (char*)"__alias__";

       // DQ (10/15/2015): Fixed handling for stricter error checking in C++11 mode.
       // char *alias_name = NULL;
       // const char **alias_name = NULL;
       // a_const_char alias_name = "";
          char *alias_name = NULL;

          if (var->aliased_variable->asm_name_is_valid && var->aliased_variable->asm_name_or_reg.name != NULL)
             {
            /* If the aliased variable has an asm name, reference that from the alias. */
            // alias_name = var->aliased_variable->asm_name_or_reg.name;
               alias_name = const_cast<char*> (var->aliased_variable->asm_name_or_reg.name);
             }
            else
             {
            // alias_name = var->aliased_variable->source_corresp.name;
               alias_name = const_cast<char*> (var->aliased_variable->source_corresp.name);
             }

       // form_string_argument_attribute(attr_str, alias_name);
          if (var->is_weakref == true)
             {
               initializedName->setGnuAttributeWeakReference();
               initializedName->set_gnu_attribute_named_weak_reference(alias_name);
             }
            else
             {
               initializedName->set_gnu_attribute_named_alias(alias_name);
             }
        }
       else
        {
          if (var->is_weakref == true)
             {
            // form_simple_attribute("__weakref__");
#ifdef DEBUG_CONVERSION
          printf ("In set_variable_attributes(): Detected attribute(__weakref__) \n");
#endif
               initializedName->setGnuAttributeWeakReference();
             }
        }
#endif
#if 0
     printf ("is_pointer_type(var->type) = %s \n",is_pointer_type(var->type) ? "true" : "false");
     if (is_pointer_type(var->type) == true)
        {
          printf ("type_pointed_to(var->type) = %p \n",type_pointed_to(var->type));
       // printf ("is_function_type(type_pointed_to(var->type)) = %s \n",is_function_type(type_pointed_to(var->type)) ? "true" : "false");
          a_type_ptr base_type = type_pointed_to(var->type);
          if (base_type != NULL)
             {
               a_type_ptr typeref_base_type = skip_typerefs(base_type);
               printf ("typeref_base_type = %p \n",typeref_base_type);
               if (typeref_base_type != NULL)
                  {
                    bool isFunctionType = is_function_type(typeref_base_type);
                    printf ("isFunctionType = %s \n",isFunctionType ? "true" : "false");
                    if (isFunctionType == true)
                       {
                         printf ("Detected a function type! \n");

                         printf ("Calling summarize_type(typeref_base_type) \n");
                         summarize_type(typeref_base_type);
                       }
                      else
                       {
                         printf ("Calling summarize_type(typeref_base_type) \n");
                         summarize_type(typeref_base_type);
                       }
                  }
             }
        }

  // bool tp = skip_typerefs(type_pointed_to(var->type));

  // printf ("skip_typerefs(type_pointed_to(var->type)) = %p \n",skip_typerefs(type_pointed_to(var->type)));
  // printf ("is_function(skip_typerefs(var->type)) = %s \n",is_function(skip_typerefs(type_pointed_to(var->type))) ? "true" : "false");
#endif

  // DQ (1/6/2014): We have to unwrap the typedef to a base type to get to the function type.
  // if (is_pointer_type(var->type) && is_function_type(type_pointed_to(var->type)))
     if (is_pointer_type(var->type) && is_function_type(type_pointed_to(var->type)))
        {
       // Need to search for the type so that we can identify the declaration???
       // set_routine_type_attributes(f_skip_typerefs(type_pointed_to(var->type)));

       // printf ("is_function_type(type_pointed_to(var->type)) = %s \n",is_function_type(type_pointed_to(var->type)) ? "true" : "false");
          a_type_ptr base_type = type_pointed_to(var->type);
          if (base_type != NULL)
             {
               a_type_ptr typeref_base_type = skip_typerefs(base_type);
#if 0
               printf ("In set_variable_attributes(): typeref_base_type = %p \n",typeref_base_type);
#endif
               if (typeref_base_type != NULL)
                  {
                    bool isFunctionType = is_function_type(typeref_base_type);
#if 0
                    printf ("In set_variable_attributes(): isFunctionType = %s \n",isFunctionType ? "true" : "false");
#endif
                    if (isFunctionType == true)
                       {
#if 0
                         printf ("In set_variable_attributes(): Detected a function type! \n");
#endif
#if 0
                         printf ("Calling summarize_type(typeref_base_type) \n");
                         summarize_type(typeref_base_type);
#endif
                         ROSE_ASSERT(typeref_base_type->kind == tk_routine);
                         ROSE_ASSERT(typeref_base_type->variant.routine.extra_info != NULL);
                         if (typeref_base_type->variant.routine.extra_info->does_not_return == true)
                            {
                              ROSE_ASSERT(initializedName != NULL);
#if 0
                              printf ("In set_variable_attributes(): Detected noreturn attribute on function pointer setting gnu attribute \n");
#endif
                              initializedName->setGnuAttributeNoReturn();
#if 0
                              printf ("In set_variable_attributes(): Detected noreturn attribute on function pointer: Exiting as a test! \n");
                              ROSE_ASSERT(false);
#endif
                            }
                       }
                      else
                       {
#if 0
                         printf ("Calling summarize_type(typeref_base_type) \n");
                         summarize_type(typeref_base_type);
#endif
                       }
                  }
                 else
                  {
#if 0
                    printf ("Calling summarize_type(base_type): What is this? \n");
                    summarize_type(base_type);
#endif
                  }
             }

       // printf ("In set_variable_attributes(): Sorry, GNU function type specific variable attributes not implemented ... \n");
       // ROSE_ASSERT(false);
#if 0
          printf ("Exiting as a test! \n");
          ROSE_ASSERT(false);
#endif
        }

     if (var->source_corresp.attributes != NULL)
        {
#if MS_DECLSPEC_DEBUG
          printf ("In set_variable_attributes(): Detected valid attributes \n");
#endif
          SgVariableDeclaration* variableDeclaration = isSgVariableDeclaration(initializedName->get_parent());

       // This might fail for SgInitializedName objects defined in function parameters.  I need to review 
       // which attributes can be applied to function parameters.
          ROSE_ASSERT(variableDeclaration != NULL);
          set_ms_attributes(variableDeclaration->get_declarationModifier(),var->source_corresp.attributes);
        }

#if 0
     printf ("Leaving set_variable_attributes(): var = %p var->type = %p initializedName = %s \n",var,var->type,initializedName->get_name().str());
#endif
   }


// DQ (8/17/2014): Adding more support for Microsoft declspec annotations.
// To handle the case of a SgDeclarationModifier and SgInitializedName, we *MIGHT* make this a templated function.
void set_ms_attributes(SgDeclarationModifier & declarationModifier, an_attribute_ptr ptr )
   {
  // DQ (8/16/2014): Hunting for the Microsoft declSpec attributes.
     ROSE_ASSERT(ptr != NULL);

#if 0
     printf ("In set_ms_attributes(): Detected valid attributes \n");
#endif

     if (ptr->family == af_ms_declspec)
        {
       // printf ("Detected Microsoft declSpec attribute \n");

#if MS_DECLSPEC_DEBUG
          if (ptr->on_primary_declaration == false)
             {
               printf ("Note: In set_ms_attributes(): ptr->on_primary_declaration == false \n");
             }
#endif
       // ROSE_ASSERT(ptr->on_primary_declaration == true);

          string name = ptr->name;

#if 0
          printf ("Detected Microsoft declSpec attribute: name = %s \n",name.c_str());
#endif

#if 0
 // Kinds of declspec to support for Microsoft Windows.
    align( # )
    allocate(" segname ")
    appdomain
    code_seg(" segname ")
    deprecated
    dllimport
    dllexport
    jitintrinsic
    naked
    noalias
    noinline
#endif

          if (name == "align")
             {
#if MS_DECLSPEC_DEBUG
               printf ("Deteced declspec(align) \n");
#endif
               declarationModifier.set_ms_declspec_align();
             }

          if (name == "allocate")
             {
#if MS_DECLSPEC_DEBUG
               printf ("Deteced declspec(allocate) \n");
#endif
               declarationModifier.set_ms_declspec_allocate();
               declarationModifier.set_gnu_attribute_section_name("unknown_allocate_section_name");
             }

          if (name == "appdomain")
             {
#if MS_DECLSPEC_DEBUG
               printf ("Deteced declspec(appdomain) \n");
#endif
               declarationModifier.set_ms_declspec_appdomain();
             }

          if (name == "code_seg")
             {
#if MS_DECLSPEC_DEBUG
               printf ("Deteced declspec(code_seg) \n");
#endif
               declarationModifier.set_ms_declspec_code_seg();
               declarationModifier.set_gnu_attribute_section_name("unknown_code_seg_section_name");
             }

          if (name == "deprecated")
             {
#if MS_DECLSPEC_DEBUG
               printf ("Deteced declspec(deprecated) \n");
#endif
               declarationModifier.set_ms_declspec_deprecated();
             }

          if (name == "dllimport")
             {
#if MS_DECLSPEC_DEBUG
               printf ("Deteced declspec(dllimport) \n");
#endif
               declarationModifier.set_ms_declspec_dllimport();
             }

          if (name == "dllexport")
             {
#if MS_DECLSPEC_DEBUG
               printf ("Deteced declspec(dllexport) \n");
#endif
               declarationModifier.set_ms_declspec_dllexport();
             }

          if (name == "jitintrinsic")
             {
#if MS_DECLSPEC_DEBUG
               printf ("Deteced declspec(jitintrinsic) \n");
#endif
               declarationModifier.set_ms_declspec_jitintrinsic();
             }

          if (name == "naked")
             {
#if MS_DECLSPEC_DEBUG
               printf ("Deteced declspec(naked) \n");
#endif
               declarationModifier.set_ms_declspec_naked();
             }

          if (name == "noalias")
             {
#if MS_DECLSPEC_DEBUG
               printf ("Deteced declspec(noalias) \n");
#endif
               declarationModifier.set_ms_declspec_noalias();
             }

          if (name == "noinline")
             {
#if MS_DECLSPEC_DEBUG
               printf ("Deteced declspec(noinline) \n");
#endif
               declarationModifier.set_ms_declspec_noinline();
             }
#if 0
 // Kinds of declspec to support for Microsoft Windows.
    noreturn
    nothrow
    novtable
    process
    property( {get=get_func_name|,put=put_func_name})
    restrict
    safebuffers
    selectany
    thread
    uuid(" ComObjectGUID ")
#endif

          if (name == "noreturn")
             {
#if MS_DECLSPEC_DEBUG
               printf ("Deteced declspec(noreturn) \n");
#endif
               declarationModifier.set_ms_declspec_noreturn();
             }

          if (name == "nothrow")
             {
#if MS_DECLSPEC_DEBUG
               printf ("Deteced declspec(nothrow) \n");
#endif
               declarationModifier.set_ms_declspec_nothrow();
             }

          if (name == "novtable")
             {
#if MS_DECLSPEC_DEBUG
               printf ("Deteced declspec(novtable) \n");
#endif
               declarationModifier.set_ms_declspec_novtable();
             }

          if (name == "process")
             {
#if MS_DECLSPEC_DEBUG
               printf ("Deteced declspec(process) \n");
#endif
               declarationModifier.set_ms_declspec_process();
             }

          if (name == "property")
             {
#if MS_DECLSPEC_DEBUG
               printf ("Deteced declspec(property) (options not yet handled) \n");
#endif
               declarationModifier.set_ms_declspec_property();

            // Process the associated arguments to identify the get and put strings.
               an_attribute_arg_ptr attribute_arg = ptr->arguments;
            // int counter = 0;
               bool get_function_detected = false;
               bool put_function_detected = false;
               string property_get_function_name;
               string property_put_function_name;
               while (attribute_arg != NULL)
                  {
#if 0
                    printf ("     --- argument #%d (start)             = %p \n",counter,ptr->arguments);
#endif
#if 0
                    summarize_attribute_arg(attribute_arg);
#endif
                    if (attribute_arg->kind == aak_token || attribute_arg->kind == aak_raw_token)
                       {
#if 0
                         printf ("     token (pointer)                      = %p \n",attribute_arg->variant.token);
#endif
                         if (attribute_arg->variant.token != NULL)
                            {
#if 0
                              printf ("     token (string)                       = %s \n",attribute_arg->variant.token);
#endif
                              string property_name = attribute_arg->variant.token;
                              if (property_name == "get")
                                 {
                                   get_function_detected = true;

                                // Advance to the next token (which is a "=")
                                   attribute_arg = attribute_arg->next;
                                   ROSE_ASSERT(attribute_arg != NULL);

                                // Advance to the next token (which is the get function name)
                                   attribute_arg = attribute_arg->next;
                                   ROSE_ASSERT(attribute_arg != NULL);

                                   ROSE_ASSERT(attribute_arg->variant.token != NULL);
                                   property_get_function_name = attribute_arg->variant.token;
                                 }
                                else
                                 {
                                   if (property_name == "put")
                                      {
                                        put_function_detected = true;

                                     // Advance to the next token (which is a "=")
                                        attribute_arg = attribute_arg->next;
                                        ROSE_ASSERT(attribute_arg != NULL);

                                     // Advance to the next token (which is the put function name)
                                        attribute_arg = attribute_arg->next;
                                        ROSE_ASSERT(attribute_arg != NULL);

                                        ROSE_ASSERT(attribute_arg->variant.token != NULL);
                                        property_put_function_name = attribute_arg->variant.token;
                                      }
                                     else
                                      {
                                        attribute_arg = attribute_arg->next;
                                      }
                                 }
                            }
                           else
                            {
                              attribute_arg = attribute_arg->next;
                            }
                       }
                      else
                       {
                         attribute_arg = attribute_arg->next;
                       }
#if 0
                    printf ("     --- argument #%d (end)               = %p \n",counter,ptr->arguments);
                    counter++;
#endif
                  }
#if 0
               printf ("get_function_detected = %s property_get_function_name = %s \n",get_function_detected ? "true" : "false",property_get_function_name.c_str());
               printf ("put_function_detected = %s property_put_function_name = %s \n",put_function_detected ? "true" : "false",property_put_function_name.c_str());
#endif
            // Initialize the data in the ROSE AST.
               if (get_function_detected == true)
                    declarationModifier.set_microsoft_property_get_function_name(property_get_function_name);

               if (put_function_detected == true)
                    declarationModifier.set_microsoft_property_put_function_name(property_put_function_name);
             }

          if (name == "restrict")
             {
#if MS_DECLSPEC_DEBUG
               printf ("Deteced declspec(restrict) \n");
#endif
               declarationModifier.set_ms_declspec_restrict();
             }

          if (name == "safebuffers")
             {
#if MS_DECLSPEC_DEBUG
               printf ("Deteced declspec(safebuffers) \n");
#endif
               declarationModifier.set_ms_declspec_safebuffers();
             }

          if (name == "selectany")
             {
#if MS_DECLSPEC_DEBUG
               printf ("Deteced declspec(selectany) \n");
#endif
               declarationModifier.set_ms_declspec_selectany();
             }

          if (name == "thread")
             {
#if MS_DECLSPEC_DEBUG
               printf ("Deteced declspec(thread) \n");
#endif
               declarationModifier.set_ms_declspec_thread();
             }

          if (name == "uuid")
             {
#if MS_DECLSPEC_DEBUG
               printf ("Deteced declspec(uuid) \n");
#endif
               declarationModifier.set_ms_declspec_uuid();

            // Process the associated arguments to identify the get and put strings.
               an_attribute_arg_ptr attribute_arg = ptr->arguments;
               bool uuid_string_detected = false;
               string uuid_string = get_uuid_string(uuid_string_detected,attribute_arg);
#if 0
               printf ("uuid_string_detected = %s uuid_string = %s \n",uuid_string_detected ? "true" : "false",uuid_string.c_str());
#endif
            // Initialize the data in the ROSE AST.
               if (uuid_string_detected == true)
                    declarationModifier.set_microsoft_uuid_string(uuid_string);
             }
#if 0
          printf ("Exiting as a test! \n");
          ROSE_ASSERT(false);
#endif
        }
   }

void set_ms_attributes(SgInitializedName* initializedName, an_attribute_ptr ptr )
   {
  // DQ (8/16/2014): Hunting for the Microsoft declSpec attributes.
     ROSE_ASSERT(ptr != NULL);

#if 0
     printf ("In set_ms_attributes(SgInitializedName): Detected valid attributes \n");
#endif

     if (ptr->family == af_ms_declspec)
        {
       // printf ("Detected Microsoft declSpec attribute \n");

#if MS_DECLSPEC_DEBUG
          if (ptr->on_primary_declaration == false)
             {
               printf ("Note: In set_ms_attributes(): ptr->on_primary_declaration == false \n");
             }
#endif
       // ROSE_ASSERT(ptr->on_primary_declaration == true);

          string name = ptr->name;

#if 0
          printf ("Detected Microsoft declSpec attribute: name = %s \n",name.c_str());
#endif

#if 0
 // Kinds of declspec to support (for function parameters) for Microsoft Windows.
    appdomain
    deprecated
    dllimport
    dllexport
    novtable
    process
    restrict
    selectany
    thread
    uuid(" ComObjectGUID ")
#endif

          if (name == "appdomain")
             {
#if MS_DECLSPEC_DEBUG
               printf ("Deteced declspec(appdomain) \n");
#endif
               initializedName->set_ms_declspec_parameter_appdomain();
             }

          if (name == "deprecated")
             {
#if MS_DECLSPEC_DEBUG
               printf ("Deteced declspec(deprecated) \n");
#endif
               initializedName->set_ms_declspec_parameter_deprecated();
             }

          if (name == "dllimport")
             {
#if MS_DECLSPEC_DEBUG
               printf ("Deteced declspec(dllimport) \n");
#endif
               initializedName->set_ms_declspec_parameter_dllimport();
             }

          if (name == "dllexport")
             {
#if MS_DECLSPEC_DEBUG
               printf ("Deteced declspec(dllexport) \n");
#endif
               initializedName->set_ms_declspec_parameter_dllexport();
             }

          if (name == "novtable")
             {
#if MS_DECLSPEC_DEBUG
               printf ("Deteced declspec(novtable) \n");
#endif
               initializedName->set_ms_declspec_parameter_novtable();
             }

          if (name == "process")
             {
#if MS_DECLSPEC_DEBUG
               printf ("Deteced declspec(process) \n");
#endif
               initializedName->set_ms_declspec_parameter_process();
             }

          if (name == "restrict")
             {
#if MS_DECLSPEC_DEBUG
               printf ("Deteced declspec(restrict) \n");
#endif
               initializedName->set_ms_declspec_parameter_restrict();
             }

          if (name == "selectany")
             {
#if MS_DECLSPEC_DEBUG
               printf ("Deteced declspec(selectany) \n");
#endif
               initializedName->set_ms_declspec_parameter_selectany();
             }

          if (name == "thread")
             {
#if MS_DECLSPEC_DEBUG
               printf ("Deteced declspec(thread) \n");
#endif
               initializedName->set_ms_declspec_parameter_thread();
             }

          if (name == "uuid")
             {
#if MS_DECLSPEC_DEBUG
               printf ("Deteced declspec(uuid) \n");
#endif
               initializedName->set_ms_declspec_parameter_uuid();

            // Process the associated arguments to identify the get and put strings.
               an_attribute_arg_ptr attribute_arg = ptr->arguments;
               bool uuid_string_detected = false;
               string uuid_string = get_uuid_string(uuid_string_detected,attribute_arg);
#if 0
               printf ("uuid_string_detected = %s uuid_string = %s \n",uuid_string_detected ? "true" : "false",uuid_string.c_str());
#endif
            // Initialize the data in the ROSE AST.
               if (uuid_string_detected == true)
                    initializedName->set_microsoft_uuid_string(uuid_string);
             }
#if 0
          printf ("Exiting as a test! \n");
          ROSE_ASSERT(false);
#endif
        }
   }

string
get_uuid_string(bool & uuid_string_detected, an_attribute_arg_ptr attribute_arg)
   {
     string uuid_string;
     if (attribute_arg != NULL)
        {
#if 0
          summarize_attribute_arg(attribute_arg);
#endif
          uuid_string_detected = true;
          if (attribute_arg->kind == aak_constant)
             {
#if 0
            // DQ (7/24/2014): This is the more detailed output, but which is not frequently required.
               printf ("     In summarize_attribute_arg(): calling summarize_constant() \n");
               summarize_constant(attribute_arg->variant.constant);
#endif
               SgExpression* constant_expr = EDG_ROSE_Translation::convert_constant(attribute_arg->variant.constant);
               ROSE_ASSERT(constant_expr != NULL);
#if 0
               printf ("constant_expr = %p = %s \n",constant_expr,constant_expr->class_name().c_str());
#endif
               SgStringVal* stringValueExp = isSgStringVal(constant_expr);
               if (stringValueExp == NULL)
                  {
                    printf ("Error: uuid option did not evaluate to a string literal (need and example of this) \n");
                  }
               ROSE_ASSERT(stringValueExp != NULL);
               uuid_string = stringValueExp->get_value();
             }
        }

     return uuid_string;
   }



void
set_field_attributes( a_field_ptr field, SgVariableDeclaration* decl )
   {
#if 0
     printf ("In set_field_attributes( a_field_ptr field, SgVariableDeclaration* decl ) \n");
#endif

#if 0
     printf ("In set_field_attributes( a_field_ptr field, SgVariableDeclaration* decl ): calling summarize_field() \n");
     summarize_field(field);
#endif

#if 0
     printf ("In set_field_attributes( a_field_ptr field, SgVariableDeclaration* decl ): calling summarize_type() \n");
     ROSE_ASSERT(field->type != NULL);
     summarize_type(field->type);
#endif

  // DQ (12/30/2013): Set attributes for the associated type (see test2013_113.c for an example).
     set_type_attributes(field->type,decl);

#if GNU_EXTENSIONS_ALLOWED
#if USER_CONTROL_OF_STRUCT_PACKING
     if (field->is_packed == true)
        {
       // form_simple_attribute("__packed__");
#ifdef DEBUG_CONVERSION
          printf ("In set_field_attributes(): decl = %p = %s Detected attribute(__packed__) \n",decl,decl->class_name().c_str());
#endif

       // DQ (12/30/2013): This should set the packed attribute on the associated declarations SgInitializedName instead 
       // of on the type.  Note that test2013_104.c demonstrates where the field should be packed and test2013_113.c 
#if 0
          decl->get_declarationModifier().get_typeModifier().setGnuAttributePacked();
#else
          SgInitializedName* initializedName = SageInterface::getFirstInitializedName(decl);
          ROSE_ASSERT(initializedName != NULL);
          initializedName->setGnuAttributePacked();
#endif
        }
#endif
     if (field->alignment != 0)
        {
       // form_unsigned_argument_attribute("__aligned__",(a_host_large_unsigned)field->alignment);
#ifdef DEBUG_CONVERSION
          printf ("In set_field_attributes(): Detected attribute(__aligned__) \n");
#endif
          decl->get_declarationModifier().get_typeModifier().set_gnu_attribute_alignment((a_host_large_unsigned)field->alignment);
        }
#endif

     if (is_pointer_type(field->type) && is_function_type(type_pointed_to(field->type)))
        {
       // Need to search for the type so that we can identify the declaration???
       // set_routine_type_attributes(f_skip_typerefs(type_pointed_to(field->type)));

#ifdef DEBUG_CONVERSION
          printf ("In set_field_attributes(): Sorry, GNU function type specific field attributes not implemented ... \n");
#endif
       // DQ (1/5/2009): Commented out assertion so we could process header files (called by test2001_26.C).
       // ROSE_ASSERT(false);
        }

     if (field->source_corresp.attributes != NULL)
        {
#if MS_DECLSPEC_DEBUG
          printf ("In set_field_attributes(): Detected valid attributes \n");
#endif
       // This might fail for SgInitializedName objects defined in function parameters.  I need to review 
       // which attributes can be applied to function parameters.
          ROSE_ASSERT(decl != NULL);
          set_ms_attributes(decl->get_declarationModifier(),field->source_corresp.attributes);
        }
   }


void
set_routine_attributes( a_routine_ptr rout, SgFunctionDeclaration* decl )
   {
  // Output the GNU attributes that apply to functions.
  // More information on function attributres is available at: 
  // http://gcc.gnu.org/onlinedocs/gcc/Function-Attributes.html

#if 0
     printf ("In set_routine_attributes(): calling summarize_routine(rout = %p) \n",rout);
#endif
#if 0
     printf ("In set_routine_attributes(): calling summarize_routine(rout = %p) \n",rout);
     summarize_routine(rout);
#endif

#if GNU_EXTENSIONS_ALLOWED
     if (rout->is_initialization_routine == true)
        {
       // Support for "void f __attribute__((constructor));
          decl->get_functionModifier().setGnuAttributeConstructor();

#if GNU_INIT_PRIORITY_ATTRIBUTE_ALLOWED
          bool ctor_priority = (rout->gnu_extra_info != NULL && rout->gnu_extra_info->ctor_priority != 0);
       // if (rout->ctor_priority != 0)
          if (ctor_priority != 0)
             {
            // Support for "void f __attribute__((constructor (N)));"
#ifdef DEBUG_CONVERSION
               printf ("In set_routine_attributes(): Detected attribute(__constructor__) \n");
#endif
            // decl->get_functionModifier().set_gnu_attribute_constructor_destructor_priority((a_host_large_unsigned)rout->ctor_priority);
               decl->get_functionModifier().set_gnu_attribute_constructor_destructor_priority((a_host_large_unsigned)ctor_priority);
             }
#endif
        }

     if (rout->is_finalization_routine == true)
        {
       // Support for "void f __attribute__(destructor);
          decl->get_functionModifier().setGnuAttributeDestructor();
#if GNU_INIT_PRIORITY_ATTRIBUTE_ALLOWED
          bool dtor_priority = (rout->gnu_extra_info != NULL && rout->gnu_extra_info->dtor_priority != 0);
       // if (rout->dtor_priority != 0)
          if (dtor_priority != 0)
             {
            // Support for "void f __attribute__((destructor (N)));"
#ifdef DEBUG_CONVERSION
          printf ("In set_routine_attributes(): Detected attribute(__destructor__) \n");
#endif
            // decl->get_functionModifier().set_gnu_attribute_constructor_destructor_priority((a_host_large_unsigned)rout->dtor_priority);
               decl->get_functionModifier().set_gnu_attribute_constructor_destructor_priority((a_host_large_unsigned)dtor_priority);
             }
#endif
        }

     if (rout->is_pure == true)
        {
       // Support for __attribute__((__pure__))
#ifdef DEBUG_CONVERSION
          printf ("In set_routine_attributes(): Detected attribute(__pure__) \n");
#endif
          decl->get_functionModifier().setGnuAttributePure();
        }

     if (rout->is_weak && !rout->is_weakref == true)
        {
       // The "weakref" attribute implies the "weak" attribute (so set both in the IR, but only output one in the IR)
       // Note: we don't have support for __weakref__ in ROSE yet!
       // Support for __attribute__((__weak__))
#ifdef DEBUG_CONVERSION
          printf ("In set_routine_attributes(): Detected attribute(__weak__) \n");
#endif
          decl->get_functionModifier().setGnuAttributeWeak();
        }

     if (rout->has_gnu_used_attribute == true)
        {
       // Support for __attribute__((__used__))
#ifdef DEBUG_CONVERSION
          printf ("In set_routine_attributes(): Detected attribute(__used__) \n");
#endif
          decl->get_functionModifier().setGnuAttributeUsed();
        }
#endif

#if GNU_EXTENSIONS_ALLOWED
     if (rout->allocates_memory == true)
        {
       // Support for __attribute__((__malloc__))
#ifdef DEBUG_CONVERSION
          printf ("In set_routine_attributes(): Detected attribute(__malloc__) \n");
#endif
          decl->get_functionModifier().setGnuAttributeMalloc();
        }
#endif

#if GNU_NAKED_ATTRIBUTE_ALLOWED
     if (rout->is_naked)
        {
       // Support for __attribute__((__naked__))
#ifdef DEBUG_CONVERSION
          printf ("In set_routine_attributes(): Detected attribute(__naked__) \n");
#endif
          decl->get_functionModifier().setGnuAttributeNaked();
        }
#endif

#if GNU_EXTENSIONS_ALLOWED
     if (rout->no_instrument_function == true)
        {
       // Support for __attribute__((__no_instrument_function__))
#ifdef DEBUG_CONVERSION
          printf ("In set_routine_attributes(): Detected attribute(__no_instrument_function__) \n");
#endif
          decl->get_functionModifier().setGnuAttributeNoInstrumentFunction();
        }

     if (rout->no_check_memory_usage == true)
        {
       // Support for __attribute__((__no_check_memory_usage__))
#ifdef DEBUG_CONVERSION
          printf ("In set_routine_attributes(): Detected attribute(__no_check_memory_usage__) \n");
#endif
          decl->get_functionModifier().setGnuAttributeNoCheckMemoryUsage();
        }
#endif

     if (rout->never_inline == true)
        {
       // Support for __attribute__((__noinline__))
#ifdef DEBUG_CONVERSION
          printf ("In set_routine_attributes(): Detected attribute(__noinline__) \n");
#endif
          decl->get_functionModifier().setGnuAttributeNoInline();
        }

#if GNU_NAKED_ATTRIBUTE_ALLOWED
     if (rout->always_inline == true)
        {
       // Support for __attribute__((__always_inline__))
#ifdef DEBUG_CONVERSION
          printf ("In set_routine_attributes(): Detected attribute(__always_inline__) \n");
#endif
          decl->get_functionModifier().setGnuAttributeAlwaysInline();
        }
#endif

     if (rout->never_throws == true)
        {
       // Support for __attribute__((__nothrow__))
#ifdef DEBUG_CONVERSION
          printf ("In set_routine_attributes(): Detected attribute(__nothrow__) \n");
#endif
          decl->get_functionModifier().setGnuAttributeNoThrow();
        }

  // DQ (7/26/2014): Added support to distinguish between use of C11 _Noreturn keyword and __attribute__((__noreturn__)) gnu attribute.
     ROSE_ASSERT(rout->type != NULL);
  // ROSE_ASSERT(rout->type->kind == tk_routine);
     if (rout->type->kind == (a_type_kind)tk_routine)
        {
          a_routine_type_supplement_ptr rtsp = rout->type->variant.routine.extra_info;
          if (rtsp != NULL && rtsp->does_not_return == true)
             {
               if (rout->source_corresp.attributes != NULL)
                  {
                 // Loop over all attributes (there can be several, and the noreturn need not be the first one).
                    an_attribute_ptr tmp_attribute = rout->source_corresp.attributes;
                    ROSE_ASSERT(tmp_attribute != NULL);

                    bool noreturn_attribute_found = false;
                    string name;
                    do {
                         name = tmp_attribute->name;
                         if (name == "__noreturn__" || name == "noreturn")
                            {
                              noreturn_attribute_found = true;
                            }
                         tmp_attribute = tmp_attribute->next;
                       }
                    while (noreturn_attribute_found == false && tmp_attribute != NULL);

                 // Support for __attribute__((__noreturn__))
#ifdef DEBUG_CONVERSION
                    printf ("In set_routine_attributes(): Detected attribute(__noreturn__) name = %s \n",name.c_str());
#endif
                    if (noreturn_attribute_found == true)
                       {
                      // This case is using the gnu __attribute__ mechanism.
#ifdef DEBUG_CONVERSION
                         printf ("Specifing the noreturn using the gnu attribute mechanism (not the C11 _Noreturn keyword) \n");
#endif
                         decl->get_declarationModifier().get_typeModifier().setGnuAttributeNoReturn();
                       }
                      else
                       {
                         printf ("Error: noreturn is not specified using noreturn attribute \n");
                         ROSE_ASSERT(false);
                       }
#if 0
                    printf ("Exiting as a test! \n");
                    ROSE_ASSERT(false);
#endif
                  }
                 else
                  {
                    SgFunctionDeclaration* functionDeclaration = isSgFunctionDeclaration(decl);
                    if (functionDeclaration != NULL)
                       {
#ifdef DEBUG_CONVERSION
                         printf ("Specifing the noreturn using the C11 _Noreturn keyword \n");
#endif
                         functionDeclaration->set_using_C11_Noreturn_keyword(true);
                       }
                      else
                       {
                         printf ("Error: noreturn is not specified using noreturn attribute AND is not a SgFunctionDeclaration \n");
                         ROSE_ASSERT(false);
                       }

#ifdef DEBUG_CONVERSION
                    printf ("functionDeclaration->get_using_C11_Noreturn_keyword() = %s \n",functionDeclaration->get_using_C11_Noreturn_keyword() ? "true" : "false");
#endif
                  }
             }
        }

     if (rout->type->kind == (a_type_kind)tk_routine)
        {
       /* If this routine is declared using ordinary function declarator
          syntax (i.e., not using a typedef), generate the associated
          routine type attributes. */

       // Note that in ROSE the type attributes are not stored in the type but are put into the 
       // SgTypeModifier that is a member of the SgDeclarationModifier (so it stays with the 
       // declaration and is not shared (since types are shared)).
          set_routine_type_attributes(rout->type,decl);
        }

#if GNU_EXTENSIONS_ALLOWED
     a_const_char* section = (rout->gnu_extra_info != NULL) ? rout->gnu_extra_info->section : NULL;
     if (section != NULL)
        {
       // Support for __attribute__((__section__))
       // Support for named sections, as in: extern void foobar (void) __attribute__ ((section ("bar")));
#ifdef DEBUG_CONVERSION
          printf ("In set_routine_attributes(): Detected attribute(__section__) \n");
#endif
       // decl->get_declarationModifier().set_gnu_attribute_section_name(rout->section);
          decl->get_declarationModifier().set_gnu_attribute_section_name(section);
        }
#endif

  // Notes specific to weak references:
  // The weakref attribute marks a declaration as a weak reference. Without arguments, it should be accompanied 
  // by an alias attribute naming the target symbol. Optionally, the target may be given as an argument to weakref 
  // itself. In either case, weakref implicitly marks the declaration as weak. Without a target, given as an 
  // argument to weakref or to alias, weakref is equivalent to weak.
  //      static int x() __attribute__ ((weakref ("y")));
  //         is equivalent to... 
  //      static int x() __attribute__ ((weak, weakref, alias ("y")));
  //         and to... 
  //      static int x() __attribute__ ((weakref));
  //      static int x() __attribute__ ((alias ("y")));
  // A weak reference is an alias that does not by itself require a definition to be given for the target symbol. 
  // If the target symbol is only referenced through weak references, then the becomes a weak undefined symbol. 
  // If it is directly referenced, however, then such strong references prevail, and a definition will be required 
  // for the symbol, not necessarily in the same translation unit.
  // The effect is equivalent to moving all references to the alias to a separate translation unit, renaming the 
  // alias to the aliased symbol, declaring it as weak, compiling the two separate translation units and performing 
  // a reloadable link on them.
  // At present (in GNU), a declaration to which weakref is attached can only be static.
#if GNU_EXTENSIONS_ALLOWED
     a_routine_ptr aliased_routine = (rout->gnu_extra_info != NULL) ? rout->gnu_extra_info->aliased_routine : NULL;

     if (aliased_routine != NULL)
        {
#if 0
          printf ("In set_routine_attributes(): Checking for gnu aliased routine: aliased_routine = %p \n",aliased_routine);
#endif
#if 0
          printf ("In set_routine_attributes(): aliased_routine != NULL: calling summarize_routine(rout = %p) \n",rout);
          summarize_routine(rout);
#endif

       // If the aliased routine has an asm name, reference that from the alias.
          char* alias_name = (aliased_routine->gnu_extra_info != NULL) ? const_cast<char*> (aliased_routine->gnu_extra_info->asm_name) : NULL;
       // DQ (2/7/2014): Some functions like "abs" are implicitly aliased to "__builtin_abs()" and we want to 
       // ignore these (which are always marked as (implicit_alias == true)).
       // if (alias_name == NULL)
          if ( (alias_name == NULL) && (rout->implicit_alias == false) )
             {
            // alias_name = aliased_routine->source_corresp.name;
               alias_name = const_cast<char*> (aliased_routine->source_corresp.name);

#ifdef DEBUG_CONVERSION
               printf ("In set_routine_attributes(): Detected attribute(__alias__(\"%s\")) \n",alias_name);
#endif

               if (rout->is_weakref == true)
                  {
#if 0
                    printf ("In set_routine_attributes(): rout->is_weakref == true \n");
#endif
                    decl->get_functionModifier().setGnuAttributeWeakReference();
                    decl->get_functionModifier().set_gnu_attribute_named_weak_reference(alias_name);
                  }
                 else
                  {
                    decl->get_functionModifier().set_gnu_attribute_named_alias(alias_name);
                  }
             }
        }
       else
        {
          if (rout->is_weakref == true)
             {
            // Support for __attribute__((__weakref__))
#ifdef DEBUG_CONVERSION
          printf ("In set_routine_attributes(): Detected attribute(__weakref__) \n");
#endif
               decl->get_functionModifier().setGnuAttributeWeakReference();
             }
        }
#endif

#if GNU_VISIBILITY_ATTRIBUTE_ALLOWED
#if 0
     printf ("In set_routine_attributes(): Checking for gnu_attribute_visability \n");
#endif
     decl->get_declarationModifier().set_gnu_attribute_visability(get_ELF_visibility_attribute((an_ELF_visibility_kind)(rout->ELF_visibility), &rout->source_corresp));
#endif

  // DQ (1/19/2014): Adding support for to access GNU attributes that are not directly supported in EDG.
  // For example, __attribute__((regparm(3))) is required for some functions in the Valgrind application.
  // if (rout->source_corresp.attributes != NULL)

  // DQ (1/21/2014): This is a bug that I have been chasing all day! It is also not always true!!!
  // ROSE_ASSERT(rout->type->kind = tk_routine);

  // DQ (1/21/2014): Added code to strip off typedef when it is detected.
     a_type_ptr routine_type = rout->type;
     if (routine_type->kind != tk_routine)
        {
          ROSE_ASSERT(routine_type->kind == tk_typeref);

#ifdef DEBUG_CONVERSION
          printf ("UNCONVENTIONAL CODE: In set_routine_attributes(): (function declared using typedef of function type) routine_type = %p routine_type->variant.typeref.type = %p \n",routine_type,routine_type->variant.typeref.type);
#endif
#if 0
          printf ("######### REDEFINING routine_type = %p to routine_type->variant.typeref.type = %p \n",routine_type,routine_type->variant.typeref.type);
#endif
       // We are effectively stripping off the typedef since it is not clear how to represent a function with this in place.
       // We don't have a concept of a function declaration that is a name using a typedef of a function type.
       // So we have to peel away the typedef and represent the function directly (using the typedef's base type).
       // p_type = p_type->variant.typeref.type;
          routine_type = routine_type->variant.typeref.type;
          ROSE_ASSERT(routine_type->kind == tk_routine);
        }

     ROSE_ASSERT(routine_type->kind == tk_routine);

  // a_type_ptr return_type = rout->type->variant.routine.return_type;
     a_type_ptr return_type = routine_type->variant.routine.return_type;
     ROSE_ASSERT(return_type != NULL);

#if 0
     printf ("rout->source_corresp.attributes        = %p \n",rout->source_corresp.attributes);
     printf ("return_type->source_corresp.attributes = %p \n",return_type->source_corresp.attributes);
#endif

     if (rout->source_corresp.attributes != NULL || return_type->source_corresp.attributes != NULL)
        {
#if 0
          printf ("Detected use of GNU attribute mechanism \n");
#endif
          an_attribute_ptr attribute = rout->source_corresp.attributes;

          if (attribute == NULL)
             {
            // Since we are using EDG attribute support we have to hunt around for other locations where this attribute could have been placed.
            // test2014_41.c demonstrates that it can be hidden in the return type for the functions.  Joy, Joy!
            // ROSE_ASSERT(rout->type->kind == tk_routine);
               ROSE_ASSERT(routine_type->kind == tk_routine);
            // a_type_ptr return_type = rout->type->variant.routine.return_type;
               a_type_ptr return_type = routine_type->variant.routine.return_type;
               ROSE_ASSERT(return_type != NULL);

               attribute = return_type->source_corresp.attributes;
#if 0
               printf ("Exiting as a test! \n");
               ROSE_ASSERT(false);
#endif
             }

          while (attribute != NULL)
             {
            // We are only interested in the unknown attributes, since all others are processed using ther direct representation in EDG.

               string name = (attribute->name != NULL) ? attribute->name : "null";
#if 0
               printf ("Processing internal attribute list: attribute = %p name = %s \n",attribute,name.c_str());
#endif
            // Note: we could use this mechanism to posibily verify that all other attributes are already handled.
               if (attribute->kind == ak_unrecognized)
                  {
                    ROSE_ASSERT(attribute->name != NULL);
#if 0
                    printf ("Detected (attribute->kind == ak_unrecognized): attribute->name = %s \n",name.c_str());
#endif
                    if (name == "regparm")
                       {
                      // Get the associated artribute argument for this gnu attribute.
                         an_attribute_arg_ptr attribute_argument = attribute->arguments;
                         ROSE_ASSERT(attribute_argument != NULL);
                         if (attribute_argument->kind == aak_raw_token)
                            {
                              int regparm_value = 0;
                              if (attribute_argument->kind == aak_token || attribute_argument->kind == aak_raw_token)
                                 {
                                   string regparm_value_string = attribute_argument->variant.token;
                                   regparm_value = atoi(regparm_value_string.c_str());
#if 0
                                   printf ("regparm_value_string = %s regparm_value = %d \n",regparm_value_string.c_str(),regparm_value);
#endif
                                 }
                                else
                                 {
                                   printf ("Error: not clear how to read the attribute argument value \n");
                                   ROSE_ASSERT(false);
                                 }
#if 0
                              printf ("##### calling decl->set_gnu_regparm_attribute(regparm_value): regparm_value = %d \n",regparm_value);
#endif
                              decl->set_gnu_regparm_attribute(regparm_value);
                            }
                           else
                            {
                              printf ("Warning: Unrecognized attribute argument kind not understood: kind = %" PRIuPTR " \n",(size_t)(attribute_argument->kind));
                            }
                       }
                      else
                       {
#if 0
                         printf ("In set_routine_attributes(): Unrecognized attribute name = %s : calling summarize_routine(rout = %p) \n",name.c_str(),rout);
                         summarize_routine(rout);
#endif
                         if (name == "cdecl")
                            {
#if 0
                           // Get the associated artribute argument for this gnu attribute.
#error "DEAD CODE!"
                              an_attribute_arg_ptr attribute_argument = attribute->arguments;
                              ROSE_ASSERT(attribute_argument != NULL);
                              if (attribute_argument->kind == aak_raw_token)
                                 {
                                   int cdecl_value = 0;
                                   if (attribute_argument->kind == aak_token || attribute_argument->kind == aak_raw_token)
                                      {
                                        string cdecl_value_string = attribute_argument->variant.token;
                                        cdecl_value = atoi(cdecl_value_string.c_str());
#if 0
                                        printf ("cdecl_value_string = %s regparm_value = %d \n",cdecl_value_string.c_str(),cdecl_value);
#endif
                                      }
#error "DEAD CODE!"
                                     else
                                      {
                                        printf ("Error: not clear how to read the attribute argument value \n");
                                        ROSE_ASSERT(false);
                                      }

                                // decl->set_gnu_cdecl_attribute(regparm_value);
                                   decl->get_declarationModifier().get_typeModifier().setGnuAttributeCdecl();
                                 }
                                else
                                 {
                                   printf ("Warning: Unrecognized attribute argument kind not understood: kind = %" PRIuPTR " \n",(size_t)(attribute_argument->kind));
                                 }
#else
#if 0
                              printf ("##### calling decl->get_declarationModifier().get_typeModifier().setGnuAttributeCdecl() \n");
#endif
                              decl->get_declarationModifier().get_typeModifier().setGnuAttributeCdecl();
#endif
#if 0
                              printf ("Set function declaration modifier's type modifier's attribute to cdecl: decl = %p \n",decl);
                              ROSE_ASSERT(false);
#endif
                            }
                           else
                            {
                           // DQ (2/8/2016): Added another attribute to ignore (need to hunt for a data-base of these).
                           // DQ (11/4/2015): Also ignore the visibility attribute (for now).
                           // DQ (9/25/2015): Ignore the __leaf__ attribute (for now).
                           // if (name == "__leaf__")
                           // if (name == "__leaf__" || name == "visibility")
                              if (name == "__leaf__" || name == "visibility" || name == "__abi_tag__")
                                 {
                                // ROSE is currently ignoring this attribute (it appears in newer header files and is discussed more at:
                                //    https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#Common-Function-Attributes
                                 }
                                else
                                 {
                                   mprintf ("Note: unrecognized gnu attribute name = %s \n",name.c_str());
                                 }
                            }
                       }
#if 0
                    printf ("In set_routine_attributes(): calling summarize_routine(rout = %p) \n",rout);
                    summarize_routine(rout);
#endif
#if 0
                    printf ("Exiting as a test! \n");
                    ROSE_ASSERT(false);
#endif
                  }
#if 0
               printf ("attribute->next = %p \n",attribute->next);
#endif
               attribute = attribute->next;
             }

#if 0
          printf ("Exiting as a test! \n");
          ROSE_ASSERT(false);
#endif
        }

     if (rout->source_corresp.attributes != NULL)
        {
#if MS_DECLSPEC_DEBUG
          printf ("In set_routine_attributes(): Detected valid attributes \n");
#endif
       // This might fail for SgInitializedName objects defined in function parameters.  I need to review 
       // which attributes can be applied to function parameters.
          ROSE_ASSERT(decl != NULL);
          set_ms_attributes(decl->get_declarationModifier(),rout->source_corresp.attributes);
        }
   }

// DQ (1/24/2016): Adding specialized support for function parameters to be set using the attributes on the associated type.
void set_variable_attributes( an_attribute_ptr attribute_list, SgInitializedName* initializedName )
   {
#if 0
     printf ("Note: New support for CUDA specific __device__ keyword support via the GNU attributes and using a user defined attribute_list = %p \n",attribute_list);
#endif

     an_attribute_ptr attribute = attribute_list;
     ROSE_ASSERT(attribute != NULL);

     while (attribute != NULL)
        {
       // We are only interested in the unknown attributes, since all others are processed using ther direct representation in EDG.

          string name = (attribute->name != NULL) ? attribute->name : "null";
#if 0
          printf ("Processing internal attribute list: attribute = %p name = %s \n",attribute,name.c_str());
#endif
       // Note: we could use this mechanism to posibily verify that all other attributes are already handled.
          if (attribute->kind == ak_unrecognized)
             {
               ROSE_ASSERT(attribute->name != NULL);
#if 0
               printf ("Detected (attribute->kind == ak_unrecognized): attribute->name = %s \n",name.c_str());
#endif

            // User (ROSE) defined attribute using the GNU attribute mechanism.
               if (name == "device")
                  {
                    initializedName->set_using_device_keyword(true);
#if 0
                    SgModifierType* modifierType = isSgModifierType(initializedName->get_type());
                    if (modifierType != NULL)
                       {
                         printf ("In set_variable_attributes(an_attribute_ptr,SgInitializedName*): Setting this as a device specific type \n");
                      // DQ (1/24/2016): It would be better to set a field in the SgInitializedName since we share types and this could be a type shared with other IR nodes in ROSE.
                      // modifierType->get_typeModifier().setGnuAttributeDevice();
                      // initializedName->set_is_device(true);
#if 0
                         printf ("Exiting as a test! \n");
                         ROSE_ASSERT(false);
#endif
                       }
#endif
                  }
                 else
                  {
#if 0
                    printf ("Warning: Unrecognized attribute name = %s \n",name.c_str());
#endif
                  }
             }
#if 0
          printf ("attribute->next = %p \n",attribute->next);
#endif
          attribute = attribute->next;
        }

#if 0
          printf ("Exiting as a test! \n");
          ROSE_ASSERT(false);
#endif
   }


void set_label_attributes( a_label_ptr label, SgLabelStatement* labelstmt) {}

// *********************************************************************
// ************ END GNU and MS Attribute Handling Support **************
// *********************************************************************

#if ENABLE_CUDA_SUPPORT
void set_routine_cuda_attributes( a_routine_ptr rout, SgFunctionDeclaration* decl) {
     if (rout->host) {
       decl->get_functionModifier().setCudaHost();
     }
     if (rout->global) {
       decl->get_functionModifier().setCudaKernel();
     }
     if (rout->device) {
       decl->get_functionModifier().setCudaDevice();
     }
}

void set_variable_cuda_attributes(a_variable_ptr var, SgInitializedName * initializedName) {
//   printf("START: Set CUDA modifiers: SgInitializedName\n");
     set_variable_cuda_attributes(var, initializedName->get_storageModifier());
//   printf("END: Set CUDA modifiers: SgInitializedName\n");
}

void set_variable_cuda_attributes(a_variable_ptr var, SgDeclarationStatement * decl_stmt) {
//   printf("START: Set CUDA modifiers: SgDeclarationStatement\n");
     set_variable_cuda_attributes(var, decl_stmt->get_declarationModifier().get_storageModifier());
//   printf("END: Set CUDA modifiers: SgDeclarationStatement\n");
}

void set_variable_cuda_attributes( a_variable_ptr var, SgStorageModifier & storage_modifier) {
     if (var->device) {
//     printf("\tSet setCudaGlobal\n");
       storage_modifier.setCudaGlobal();
     }
     if (var->constant) {
//     printf("\tSet setCudaConstant\n");
       storage_modifier.setCudaConstant();
     }
     if (var->shared) {
//     printf("\tSet setCudaShared\n");
       storage_modifier.setCudaShared();
     }
}
#endif /* ENABLE_CUDA_SUPPORT */

