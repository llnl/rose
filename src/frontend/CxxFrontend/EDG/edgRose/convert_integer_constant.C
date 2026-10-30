
#include "edgRose.h"

#define DEBUG_CONVERT_INTEGER_CONSTANT 0

SgValueExp* EDG_ROSE_Translation::convert_integer_constant(a_constant_ptr e, a_type_ptr p, an_integer_value value, const char* str, bool in_enumerator)
   {
     ROSE_ASSERT(p != NULL);
  // printf ("In convert_integer_constant(): calling summarize_type_kind_in_string() p = %s \n",summarize_type_kind_in_string(p));

#ifdef DEBUG_CONVERSION
     indent_line(); debug_print("convert_integer_constant: e = %p at:",e); print_source_corresp(e->source_corresp); debug_print("\n");
#endif

#if 0
     printf ("In convert_integer_constant(): calling summarize_constant() \n");
     summarize_constant(e);
#endif

  // DQ (12/31/2008): This function has been written assuming that the types match, but that don't always match.
     ROSE_ASSERT(p->kind == tk_integer);

     SgValueExp* returnValue = NULL;

  // DQ (2/8/2011): Added assert for what I think is true. There should be no hidden expression trees from constant folded expressions.
     if (e != NULL && e->expr != NULL)
        {
#if 0
          printf ("Note: convert_integer_constant(): Found a hidden expression tree representing a constant folded expression. \n");
#endif
       // ROSE_ASSERT(false);
        }

#if DEBUG_CONVERT_INTEGER_CONSTANT
     printf ("In convert_integer_constant():\n");
     printf ("  -- p->variant.integer.enum_type = %s \n",p->variant.integer.enum_type ? "true" : "false");
     printf ("  -- in_enumerator                = %s \n",in_enumerator ? "true" : "false");
#endif

     an_integer_kind kind = p->variant.integer.int_kind;
     if (p->variant.integer.enum_type == true && in_enumerator == false)
        {
#ifdef DEBUG_CONVERSION
          indent_line(); debug_print("enum value '%s' '%s'\n", (e->source_corresp.name == NULL ? "NULL" : e->source_corresp.name), str);
#endif
          SgType* type = convert_type(p);
          ROSE_ASSERT (type != NULL);

#if DEBUG_CONVERT_INTEGER_CONSTANT
          printf ("  -- e->source_corresp.name = %s \n",e->source_corresp.name);
          printf ("  -- type = %p (%s)\n",type,type->class_name().c_str());
#endif

          SgEnumType * etype = isSgEnumType(type);
          ROSE_ASSERT(etype != NULL);

       // TV (09/19/2018): New case for the translated type (SgTypeChar).
       //   * Refactored the code
       //   * original case: Cxx_tests/test2007_83.C but it does produce a SgEnumType now (does it happen?)
       //   * new case:      Cxx_tests/rose-1380.C   which yield a SgTypeChar

          SgEnumDeclaration * edecl = isSgEnumDeclaration(etype->get_declaration());
          ROSE_ASSERT (edecl != NULL);

#if 0
          printf ("enum declaration: edecl = %p \n",edecl);
#endif

       // DQ (8/31/2012): This is a special case where the value of an enum does not map to an enum field so there is no associated name (just a value).
       // In this case we should output a SgCastExp, but we can't from the current design of this function convert_integer_constant(), which returns a 
       // SgValueExp type instead of a more general SgExpression.  For now we build in the fix to get_name_for_enumerator().
          string name = (e->source_corresp.name == NULL ? get_name_for_enumerator(p, value) : e->source_corresp.name);

       // DQ (3/20/2009): Added cast because Cygwin g++ 3.4.3 and g++ 4.2.4 report an error in the conversion of "an_integer_value" to "int"
          returnValue = buildEnumVal_nfi((int)value, edecl, name);
        }
       else
        {
       // DQ (9/2/2013): Added new code to detect references to enum values that have been cast to integer values. These were 
       // previously translated into SgIntVal which was OK, but then the lower level reference to the integer was either compiler 
       // generated or could have come from a different file (see test2012_156.c and test2013_43.c).  This was a problem once 
       // the support for unparsing SgAggregateInitialize child expressions were fixed to record there source position and
       // the handling of array intializers from an include file fixed (e.g. code such as: int array[] = { #include "values.h" };
          if (e->is_named_constant_definition == true)
             {
            // This is the case of an enum field that is referenced, but it's type is not marked as enum because it is cast to another tyle (likely an integer).
            // In this case we have to build a SgEnumVal.

               string name = (e->source_corresp.name == NULL ? get_name_for_enumerator(p, value) : e->source_corresp.name);
#if 0
               printf ("integer: enum_type = %p \n",e->type->variant.integer.enum_type ? "true" : "false");
#endif
               ROSE_ASSERT(e->type->variant.integer.enum_type == false);
            // printf ("integer (enum_type == false): enum_info.affiliated_type = %p \n",e->type->variant.integer.enum_info.affiliated_type);
               ROSE_ASSERT(e->type->variant.integer.enum_info.affiliated_type != NULL);

               SgType* type = convert_type(e->type->variant.integer.enum_info.affiliated_type);
               ROSE_ASSERT (type != NULL);

               SgEnumType* enum_type = isSgEnumType(type);

            // DQ (1/28/2014): Wireshark demonstrates an example of this problem (where type is not an SgEnumType).
            // ROSE_ASSERT (enum_type != NULL);
               if (enum_type != NULL)
                  {
                    ROSE_ASSERT (enum_type->get_declaration() != NULL);

                    SgEnumDeclaration* edecl = isSgEnumDeclaration(enum_type->get_declaration());
                    ROSE_ASSERT (edecl != NULL);

                    returnValue = buildEnumVal_nfi((int)value, edecl, name);
                  }
                 else
                  {
                 // DQ (1/28/2014): This can be a named constant or the result of a cast from an enum (I think).
                 // This was the only bug that was required to be fixed to run the 2.5 million line "wireshark" application.
#if 0
                 // If is is not a enum type, then what is this!
                    printf ("In convert_integer_constant(): name = %s type = %p = %s \n",name.c_str(),type,type->class_name().c_str());
#endif
                 // In the case I am debugging, this is a: SgTypeUnsignedInt

                 // These are named integer constants, build a value for the moment. But I think we want 
                 // to build a SgVerRefExp to define a better fix.
                    switch (type->variantT())
                       {
                         case V_SgTypeUnsignedInt:
                            {
                              returnValue = buildUnsignedIntVal_nfi((unsigned int)value, str);
                              break;
                            }

                         default:
                            {
                              printf ("In convert_integer_constant(): default reached in switch \n");
                              ROSE_ASSERT(false);
                            }
                       }
#if 0
                    printf ("Exiting as a test! \n");
                    ROSE_ASSERT(false);
#endif
                  }
#if 0
               printf ("Case not handled yet: case of an enum field that is referenced, but it's type is not marked as enum because it is cast to another type \n");
               ROSE_ASSERT(false);
#endif
             }
            else
             {
            // DQ (9/2/2013): This is the previous code before the addition of the case above.
               if (p->variant.integer.wchar_t_type == true)
                  {
#ifdef DEBUG_CONVERSION
                    indent_line(); debug_print("wchar_t\n");
#endif
                 // DQ (3/20/2009): Added cast because Cygwin g++ 3.4.3 and g++ 4.2.4 report an error in the conversion of "an_integer_value" to "wchar"
                 // return buildWcharVal_nfi((wchar_t)value, str);
                    returnValue = buildWcharVal_nfi((wchar_t)value, str);
                  }
                 else
                  {
                    if (p->variant.integer.bool_type == true)
                       {
#ifdef DEBUG_CONVERSION
                         indent_line(); debug_print("bool\n");
#endif
                      // return buildBoolValExp_nfi((int)value);
                         returnValue = buildBoolValExp_nfi((int)value);
                       }
                      else
                       {
#ifdef DEBUG_CONVERSION
                         indent_line(); debug_print("integer (kind = %d = %s) str = %s\n",kind,summarize_integer_string(kind).c_str(),str);
#endif
                         switch (kind)
                            {
                           // DQ (3/20/2009): Added cast because Cygwin g++ 3.4.3 and g++ 4.2.4 report an error in the conversion of "an_integer_value" to all these types
                              case ik_char:               returnValue = buildCharVal_nfi((char)value, str);                                  break;
                           // PL (1/21/2025): Despite C++ not having a signed char literal (only char and unsigned char literals), signed chars can come out of EDG when
                           // performing frontend constant folding. For example, an expression like `(signed char)0` can get constant folded into a signed char literal with
                           // value 0, despite there not being a syntactic construct in C++ that can directly generate such a node. We don't want to just emit an SgCharVal
                           // when encountering this, because the C++ standard does not specify the signedness of the native `char` type. We need to emit an SgSignedCharVal
                           // to enforce signedness.
                           //
                           // This solution was discussed with and agreed by DQ.
                              case ik_signed_char:        returnValue = buildSignedCharVal_nfi((signed char)value, str);                     break;
                              case ik_unsigned_char:      returnValue = buildUnsignedCharVal_nfi((unsigned char)value, str);                 break;
                              case ik_short:              returnValue = buildShortVal_nfi((short int)value, str);                            break;
                              case ik_unsigned_short:     returnValue = buildUnsignedShortVal_nfi((short unsigned int)value, str);           break;
                              case ik_int:                returnValue = buildIntVal_nfi((int)value, str);                                    break;
                              case ik_unsigned_int:       returnValue = buildUnsignedIntVal_nfi((unsigned int)value, str);                   break;
                              case ik_long:               returnValue = buildLongIntVal_nfi((long int)value, str);                           break;
                              case ik_unsigned_long:      returnValue = buildUnsignedLongVal_nfi((long unsigned int)value, str);             break;
#if LONG_LONG_ALLOWED
                              case ik_long_long:          returnValue = buildLongLongIntVal_nfi((long long int)value, str);                  break;
                              case ik_unsigned_long_long: returnValue = buildUnsignedLongLongIntVal_nfi((long long unsigned int)value, str); break;
#endif
#if INT128_EXTENSIONS_ALLOWED
                           // DQ (3/24/2014): Added support for 128-bit integers.
                           // We don't have support for 128-bit values yet.
                              case ik_int128:             returnValue = buildLongLongIntVal_nfi((long long int)value, str);                  break;
                              case ik_unsigned_int128:    returnValue = buildUnsignedLongLongIntVal_nfi((long long unsigned int)value, str); break;
#endif
                           // DQ (12/29/2008): Added cases not handled (these are OK to default to error in default clause, I think).
                              case ik_last:
                                 {
                                   printf ("Case not handled! \n");
                                   ROSE_ASSERT(false);
                                   break;
                                 }

                              default:
                                 {
                                   ERROR_MESSAGE(stderr, "In convert_integer_constant(): Bad integer kind %d (ik_last = %d)\n", kind,(int)ik_last);
                                   ROSE_ASSERT(false);
                                 }
                            }
                       }
                  }
             }
        }

  // ROSE_ASSERT (!"Hit end of convert_integer_constant");

#if 0
     printf ("In convert_integer_constant(): returnValue = %p = %s \n",returnValue,returnValue->class_name().c_str());
     printf ("In convert_integer_constant(): returnValue->get_file_info()->get_filenameString() = %s \n",returnValue->get_file_info()->get_filenameString().c_str());
#endif

  // DQ (12/3/2012): Added test.
     ROSE_ASSERT(returnValue->get_file_info()->get_filenameString() != "");

     return returnValue;
   }
