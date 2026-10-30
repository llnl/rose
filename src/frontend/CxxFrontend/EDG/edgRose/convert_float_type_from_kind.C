
#include "edgRose.h"

SgType* EDG_ROSE_Translation::convert_float_type_from_kind(a_float_kind kind)
   {
    switch (kind) {
      case fk_float: return buildFloatType();
      case fk_double: return buildDoubleType();
      case fk_long_double: return buildLongDoubleType();
      case fk_float80: return buildFloat80Type();
      case fk_float128: return buildFloat128Type();
      case fk_float16: return buildFloat16Type();
      case fk_fp16: return buildFp16Type();
      case fk_std_float16: return buildFloat16Type();
      case fk_std_bfloat16: return buildBFloat16Type();
      case fk_float32x: return buildFloat32xType();
      case fk_float64x: return buildFloat64xType();
      case fk_std_float32: return buildFloat32Type();
      case fk_std_float64: return buildFloat64Type();
      case fk_std_float128: return buildFloat128Type();
   // DQ (12/29/2008): Added cases not handled (these are OK to default to error in default clause, I think).
      case fk_last:
        {
          printf ("Case not handled! \n");
          ROSE_ASSERT(false);
          break;
        }

      default:
           ERROR_MESSAGE(stderr, "Bad float kind %d\n", kind);
           ROSE_ASSERT(false);
    }
    ROSE_ASSERT (!"Hit end of convert_float_type_from_kind");
    return NULL;
  }
