
#include "edgRose.h"

SgType* EDG_ROSE_Translation::convert_float_type(a_type_ptr p)
   {
     a_float_kind kind = p->variant.float_kind;
     SgType* base_type = convert_float_type_from_kind(kind);
     switch (p->kind)
        {
          case tk_float: return base_type;
          case tk_imaginary: return buildComplexType(base_type);
          case tk_complex: return buildComplexType(base_type);

  // DQ (12/29/2008): Added cases not handled (these are OK to default to error in default clause, I think).
          case tk_error:
          case tk_void:

  // DQ (12/29/2008): Note that each of these cases have the same value.
       // case tk_enum:
          case tk_integer:

#if FIXED_POINT_ALLOWED
          case tk_fixed_point:
#endif
          case tk_pointer:
          case tk_routine:
          case tk_array:
          case tk_class:
          case tk_struct:
          case tk_union:
          case tk_typeref:
          case tk_ptr_to_member:
          case tk_template_param:
          case tk_unknown:
             {
               printf ("Case not handled! \n");
               ROSE_ASSERT(false);
               break;
             }

          default:
             {
               ROSE_ASSERT (!"convert_float_type");
               abort();
             }
        }

     ROSE_ASSERT(!"Should not be reached");
     return NULL;
   }
