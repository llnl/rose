
#include "edgRose.h"

SgValueExp* EDG_ROSE_Translation::convert_float_constant(a_type_ptr p, a_host_fp_value value, const char* str)
   {
  // DQ (1/22/2018): Adding assertions.
     ROSE_ASSERT(p != NULL);

#if 0
     printf ("In convert_float_constant(): summarize_type() %p \n",p);
     summarize_type(p);
#endif

     a_float_kind kind = p->variant.float_kind;

#if 0
     printf ("In convert_float_constant(): kind = %s \n",summarize_float_kind_in_string(kind));
#endif

     switch (kind) 
        {
          case fk_float:        return buildFloatVal_nfi(value, str);
          case fk_double:       return buildDoubleVal_nfi(value, str);
          case fk_long_double:  return buildLongDoubleVal_nfi(value, str);
          case fk_float80:      return buildFloat80Val_nfi(value, str);
          case fk_std_float128:
          case fk_float128:     return buildFloat128Val_nfi(value, str);
          case fk_std_bfloat16: return buildBFloat16Val_nfi(value, str);
          case fk_fp16:
          case fk_std_float16:  return buildFloat16Val_nfi(value, str);
          case fk_float32x:
          case fk_std_float32:  return buildFloat32Val_nfi(value, str);
          case fk_float64x:
          case fk_std_float64:  return buildFloat64Val_nfi(value, str);
       // DQ (12/29/2008): Added cases not handled (these are OK to default to error in default clause, I think).
          case fk_last:
             {
               printf ("Case not handled! \n");
               ROSE_ASSERT(false);
               break;
             }

          default: 
               ERROR_MESSAGE(stderr, "Bad float kind = %d = %s \n", kind,summarize_float_kind_in_string(kind));
               ROSE_ASSERT(false);
        }

     ROSE_ASSERT (!"Hit end of convert_float_constant");
     return NULL;
   }
