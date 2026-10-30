
#include "edgRose.h"

SgType* EDG_ROSE_Translation::convert_integer_type(a_type_ptr p)
   {
     SgType* resultType = NULL;

     an_integer_kind kind = p->variant.integer.int_kind;

#if 0
     printf ("In convert_integer_type():\n");
     printf ("  -- kind = %s \n", summarize_integer_string(kind).c_str());
     printf ("  -- p->variant.integer.enum_type = %s \n", p->variant.integer.enum_type ? "true" : "false");
#endif
#if 0
     printf ("In convert_integer_type(): Calling summarize_type() for error \n");
     summarize_type(p);
#endif

#define DEBUG_CONVERT_INTEGER 0

     bool explicitly_signed           = p->variant.integer.explicitly_signed;
#if MICROSOFT_EXTENSIONS_ALLOWED
     bool is_microsoft_sized_int_type = p->variant.integer.microsoft_sized_int_type;
#endif
#if DEBUG_CONVERT_INTEGER
  // Needed for output of debugging information (below).
     bool has_explicit_enum_base      = p->variant.integer.has_explicit_enum_base;
#endif
     bool is_enum_type                = p->variant.integer.enum_type;
     bool is_scoped_enum              = p->variant.integer.is_scoped_enum;
#if GNU_EXTENSIONS_ALLOWED
#if DEBUG_CONVERT_INTEGER
  // Needed for output of debugging information (below).
     bool is_packed                   = p->variant.integer.packed;
#endif
#endif
     bool is_wchar_type               = p->variant.integer.wchar_t_type;
#if DEBUG_CONVERT_INTEGER
  // Needed for output of debugging information (below).
     bool is_char16_t_type            = p->variant.integer.char16_t_type;
     bool is_char32_t_type            = p->variant.integer.char32_t_type;
#endif
     bool is_bool_type                = p->variant.integer.bool_type;
     bool is_originally_unnamed_type  = p->variant.integer.originally_unnamed;
#if DEBUG_CONVERT_INTEGER
     bool is_nonreal_type             = p->variant.integer.is_nonreal;
#endif

#if DEBUG_CONVERT_INTEGER
     printf ("In convert_integer_type(): explicitly_signed           = %s \n",explicitly_signed           ? "true" : "false");
#if MICROSOFT_EXTENSIONS_ALLOWED
     printf ("In convert_integer_type(): is_microsoft_sized_int_type = %s \n",is_microsoft_sized_int_type ? "true" : "false");
#endif
     printf ("In convert_integer_type(): has_explicit_enum_base      = %s \n",has_explicit_enum_base      ? "true" : "false");
     printf ("In convert_integer_type(): is_enum_type                = %s \n",is_enum_type                ? "true" : "false");
     printf ("In convert_integer_type(): is_scoped_enum              = %s \n",is_scoped_enum              ? "true" : "false");
#if GNU_EXTENSIONS_ALLOWED
     printf ("In convert_integer_type(): is_packed                   = %s \n",is_packed                   ? "true" : "false");
#endif
     printf ("In convert_integer_type(): is_wchar_type               = %s \n",is_wchar_type               ? "true" : "false");
     printf ("In convert_integer_type(): is_char16_t_type            = %s \n",is_char16_t_type            ? "true" : "false");
     printf ("In convert_integer_type(): is_char32_t_type            = %s \n",is_char32_t_type            ? "true" : "false");
     printf ("In convert_integer_type(): is_bool_type                = %s \n",is_bool_type                ? "true" : "false");
     printf ("In convert_integer_type(): is_originally_unnamed_type  = %s \n",is_originally_unnamed_type  ? "true" : "false");
     printf ("In convert_integer_type(): is_nonreal_type             = %s \n",is_nonreal_type             ? "true" : "false");
#endif

  // DQ (1/26/2019): Original version before working on Cxx11_tests/test2019_29.C.
  // TV (09/19/2018): is_enum_type is not limited ik_int.
  //      Handle enum_type first then fall back on switch
     if (is_enum_type) {
       SgName name = p->source_corresp.name;
       if (is_originally_unnamed_type) {
      // DQ (4/3/2017): test2017_13.C demonstrates that it would be best to generate a name as early as possible for 
      // this enum type, so that it can be represented in the symbol table.
         name = generate_name_for_type(p);
       } else {
         ROSE_ASSERT(name.is_null() == false);
       }

       SgScopeStatement* scope = getDeclarationScope(p->source_corresp);
       ROSE_ASSERT(scope != NULL);

    // DQ (1/26/2019): I'm not clear why we recompute the rose scope, but when it fails we can at least use the 
    // initially computed rose scope as a backup.
       if (is_scoped_enum) {
         a_scope_ptr edg_scope = p->variant.integer.enum_info.assoc_scope;
         ROSE_ASSERT(edg_scope != NULL);
         edg_scope = edg_scope->parent;

      // DQ (1/26/2019): Minimal change.
      // ROSE_ASSERT(edg_scope != NULL);
         if (edg_scope != NULL)
            {
              if (scope_cache.find(edg_scope) != scope_cache.end()) {
                scope = scope_cache[edg_scope];
              } else {
                ROSE_ASSERT(false);
              }
            }
           else
            {
           // The scope has already been computed, so just use it directly.
              ROSE_ASSERT(scope != NULL);
            }
       }
       ROSE_ASSERT(scope != NULL);

//     ROSE_ASSERT(is_scoped_enum == false);

       SgEnumSymbol* enumSymbol = scope->lookup_enum_symbol(name);
       if (enumSymbol == NULL) {
         SgEnumDeclaration* enum_decl = NULL;
      // TV (09/21/2018): ROSE-1400
         if (is_scoped_enum) {
           enum_decl = set_file_info_for_corresp(buildNondefiningEnumDeclaration_nfi(name, scope), p->source_corresp);
           ROSE_ASSERT(enum_decl != NULL);
         } else {
           enum_decl = set_file_info_for_corresp(buildNondefiningEnumDeclaration_nfi(name, scope), p->source_corresp);
           ROSE_ASSERT(enum_decl != NULL);
           enum_decl->set_definingDeclaration(enum_decl);
           a_constant_ptr constant_list = p->variant.integer.enum_info.constant_list;
           while (constant_list != NULL) {
             SgName enumerator_name = constant_list->source_corresp.name;

             SgExpression * enumerator_value = convert_constant_in_enumerator(constant_list);
             ROSE_ASSERT(enumerator_value != NULL);

             SgAssignInitializer * assignInitializer = buildAssignInitializer_nfi(enumerator_value);
             SgInitializedName* initializedName = buildInitializedName_nfi(enumerator_name, enum_decl->get_type(), set_file_info_for_enumerator(assignInitializer, constant_list));
             initializedName = set_file_info_for_corresp(initializedName, constant_list->source_corresp);

             enum_decl->get_enumerators().push_back(initializedName);
             initializedName->set_parent(enum_decl);
             initializedName->set_scope(scope);
             initializedName->set_declptr(enum_decl);

             constant_list = constant_list->next;
           }
         }

         if (is_originally_unnamed_type) {
           enum_decl->set_isUnNamed(true);
         }

         enumSymbol = scope->lookup_enum_symbol(name);
         ROSE_ASSERT(enumSymbol != NULL);

         enum_decl->set_parent(scope);
       }

       resultType = enumSymbol->get_type();
       ROSE_ASSERT(resultType != NULL);

       return resultType;
     }

     switch (kind)
        {
       // DQ (6/19/2013): Note that C99 _Bool used to map to ik_char in EDG, but now maps to ik_unsigned_char.
          case ik_char:
             {
            // DQ (9/8/2012): This fixes where template specializations of "bool" where colliding with the same template's specialization using "char".
               if (is_bool_type == true)
                  {
#ifdef DEBUG_CONVERSION
                    indent_line(); debug_print("building bool type\n");
#endif
                    resultType = buildBoolType();
                  }
                 else
                  {
                    if (explicitly_signed == true)
                       {
#ifdef DEBUG_CONVERSION
                         printf ("WARNING: In convert_integer_type(): Might have to handle case of explicitly_signed == true explicitly \n");
#endif
                       }

#ifdef DEBUG_CONVERSION
                    indent_line(); debug_print("building signed type (building char)\n");
#endif
                    resultType = buildCharType();
                  }
               break;
             }

          case ik_signed_char:        resultType = buildSignedCharType();       break;

       // DQ (6/19/2013): This is new for EDG4.7 (_Bool now maps to unsigned char instead of char (in EDG 4.4).
       // case ik_unsigned_char:      resultType = buildUnsignedCharType();     break;
          case ik_unsigned_char:
             {
            // DQ (9/8/2012): This fixes where template specializations of "bool" where colliding with the same template's specialization using "char".
               if (is_bool_type == true)
                  {
#ifdef DEBUG_CONVERSION
                    indent_line(); debug_print("building bool type\n");
#endif
                    resultType = buildBoolType();
                  }
                 else
                  {
                    if (explicitly_signed == true)
                       {
#ifdef DEBUG_CONVERSION
                         printf ("WARNING: In convert_integer_type(): Might have to handle case of explicitly_signed == true explicitly \n");
#endif
                       }

#ifdef DEBUG_CONVERSION
                    indent_line(); debug_print("building unsigned type (building char)\n");
#endif
                    resultType = buildUnsignedCharType();
                  }
               break;
             }

          case ik_unsigned_short:
             {
            // DQ (2/16/2018): We need to support other integer types for C99 and C++11 (e.g. char16_t).
            // resultType = buildUnsignedShortType();    break;
               if (p->variant.integer.char16_t_type == true)
                  {
                 // DQ (2/16/2018): This fixes where template specializations of "char16_t" where colliding with the same template's specialization using "unsigned short".
                    resultType = buildChar16Type();      
#if 0
                    printf ("Case of char16_t not yet handled \n");
                    ROSE_ASSERT(false);
#endif
                  }
                 else
                  {
                    resultType = buildUnsignedShortType();
                  }

               break;
             }

          case ik_unsigned_int:
             {
            // DQ (2/16/2018): We need to support other integer types for C99 and C++11 (e.g. char32_t).
            // resultType = buildUnsignedIntType();      
               if (p->variant.integer.char32_t_type == true)
                  {
                 // DQ (2/16/2018): This fixes where template specializations of "char32_t" where colliding with the same template's specialization using "unsigned int".
                    resultType = buildChar32Type();      
#if 0
                    printf ("Case of char32_t not yet handled \n");
                    ROSE_ASSERT(false);
#endif
                  }
                 else
                  {
#if 0
                    printf ("In convert_integer_type(): Calling buildIntType() to handle case of is_enum_type == false \n");
#endif
                    resultType = buildUnsignedIntType();      
                  }

               break;
             }

          case ik_unsigned_long:      resultType = buildUnsignedLongType();     break;
          case ik_unsigned_long_long: resultType = buildUnsignedLongLongType(); break;

          case ik_short:              if (explicitly_signed) resultType = buildSignedShortType();    else resultType = buildShortType();    break;

          case ik_int:
             {
               if (explicitly_signed)
                  {
                 // I think that this can't be true.
                    ROSE_ASSERT(is_wchar_type == false);
                    resultType = buildSignedIntType();
                  }
                 else
                  {
                 // DQ (9/8/2012): This fixes where template specializations of "wchar" where colliding with the same template's specialization using "int".
                    if (is_wchar_type == true)
                       {
#ifdef DEBUG_CONVERSION
                         printf ("WARNING: In convert_integer_type(): Might have to handle case of explicitly_signed == true explicitly \n");
#endif
                         resultType = buildWcharType();
                       }
                      else
                       {
#if 0
                         printf ("In convert_integer_type(): Calling buildIntType() to handle case of is_enum_type == false \n");
#endif
                      // DQ (2/26/2018): We need to handle the cases of char16_t and char32_t (but this isn in another case).
                         resultType = buildIntType();
                       }
                  }
               break;
             }

#if LONG_LONG_ALLOWED
          case ik_long:               if (explicitly_signed) resultType = buildSignedLongType();     else resultType = buildLongType();     break;
          case ik_long_long:          if (explicitly_signed) resultType = buildSignedLongLongType(); else resultType = buildLongLongType(); break;
#endif
#if INT128_EXTENSIONS_ALLOWED
       // DQ (3/24/2014): Added support for 128-bit integers (we are only supporting signed and unsigned, unlike "long long" support).
       // case ik_int128:             if (explicitly_signed) resultType = buildSigned128bitIntegerType(); else resultType = buildUnsigned128bitIntegerType(); break;
       // case ik_unsigned_int128:    if (explicitly_signed) resultType = buildSigned128bitIntegerType(); else resultType = buildUnsigned128bitIntegerType(); break;
          case ik_int128:             resultType = buildSigned128bitIntegerType();   break;
          case ik_unsigned_int128:    resultType = buildUnsigned128bitIntegerType(); break;
#endif
       // DQ (12/29/2008): Added cases not handled (these are OK to default to error in default clause, I think).
       // (Note that each of these cases have the same value).
       // case ik_last:
          case ik_none:
             {
               printf ("Case not handled! \n");
               ROSE_ASSERT(false);
               break;
             }

          default:
             {
               ERROR_MESSAGE(stderr, "In convert_integer_type(): Bad integer kind %d\n", kind);
               ROSE_ASSERT(false);
             }
        }

#ifdef DEBUG_CONVERSION
     indent_line(); debug_print("leaving convert_integer_type()\n");
#endif

#if 0
     printf ("Leaving convert_integer_type(): resultType = %p = %s \n",resultType,resultType->class_name().c_str());
#endif

  // ROSE_ASSERT (!"Hit end of convert_integer_type");

     return resultType;
   }
