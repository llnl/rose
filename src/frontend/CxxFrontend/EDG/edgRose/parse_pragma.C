
#include "edgRose.h"

SgPragmaDeclaration*
EDG_ROSE_Translation::parse_pragma(SourceSequenceList& sse)
   {
     a_pragma_ptr p = (a_pragma_ptr)(sse->entity.ptr);
     ++sse;

#if 0
#ifdef DEBUG_CONVERSION
  // DQ (9/1/2013): Added output of print_source_corresp() to check on generation of additional source position information (but source_corresp is not available).
  // indent_line(); debug_print("parse_pragma: p = %p at:",p); print_source_corresp(p->source_corresp); debug_print("\n");
#endif
#ifdef DEBUG_CONVERSION
     indent_line(); debug_print("pragma %s \"%s\" ", pragma_name, p->pragma_text); print_source_pos(p->position); debug_print(" associated source_sequence_entry = %p \n",p->source_sequence_entry);
     ScopedIndent si(indent);
#endif
#endif

#if 0
     printf ("In parse_pragma(): calling summarize_pragma(p = %p) \n",p);
     summarize_pragma(p);
#endif

  // DQ (7/14/2012): Define a return variable and modify function to have a single return.
     SgPragmaDeclaration* result = NULL;

#if 1
  // DQ (9/23/2020): This is a refactoring of the code to generate_pragma_name() so that we can support 
  // the generation of SgPragma IR nodes independently of SgPragmaDeclaration nodes (which can not be 
  // added to the AST in some locations where #pragma declarations can be used, see Cxx11_tests_102.C 
  // for an example).
     string pragma_kind_name;
     pragma_kind_name = generate_pragma_name(p);
#if 0
     printf ("In parse_pragma(): pragma_kind_name = %s\n",pragma_kind_name.c_str());
#endif
     result = buildPragmaDeclaration(pragma_kind_name, topScopeStack());
#else

// #error "DEAD CODE"

     if (p->pragma_text != NULL)
        {
       // This is the most common case where UPC and OpenMP are not used.
       // return buildPragmaDeclaration(p->pragma_text, topScopeStack()); // FIXME file info
          result = buildPragmaDeclaration(p->pragma_text, topScopeStack()); // FIXME file info
        }
       else
        {
       // Switch #1
          switch (kind)
             {
#if UPC_EXTENSIONS_ALLOWED             
               case pk_upc:
                  {
#ifdef DEBUG_CONVERSION
                    indent_line(); debug_print("UPC pragma\n");
                    ScopedIndent si(indent);
#endif
                    a_upc_pragma_kind inner_kind = p->variant.upc.kind;
                    switch (inner_kind)
                       {
                         case upc_pk_access:
                            {
                              a_upc_access_method tag = p->variant.upc.value.access_method;

                           // Switch #3
                              switch (tag)
                                 {
                                   case upc_access_unspecified: 
                                        ROSE_ASSERT (!"upc_access_unspecified not supported"); 
                                        break;

                                   case upc_access_strict:
                                        result = buildPragmaDeclaration("upc strict"); 
                                        break;

                                   case upc_access_relaxed:
                                        result = buildPragmaDeclaration("upc relaxed");
                                        break;

                                   default:
                                        ERROR_MESSAGE(stderr, "ERROR: Bad UPC access tag %d\n", tag); 
                                        ROSE_ASSERT(false);
                                 }
                              break;
                            }

                         case upc_pk_coherence:
                            {
                              ROSE_ASSERT (!"UPC coherence");
                              break;
                            }

                         default:
                              ERROR_MESSAGE(stderr, "Bad UPC pragma kind %d\n", inner_kind);
                              ROSE_ASSERT(false);
                       }
                    break;
                  }
#endif
#if IDENT_DIRECTIVE_AND_PRAGMA
            // DQ (1/6/2009): Added support for #ident (tests using GNU 3.x compiler in test2006_69.C).
               case pk_ident:
                  {
                    ROSE_ASSERT(p->variant.ident_string != NULL);
                    string ident_string = convert_string_constant(p->variant.ident_string);

                 // return buildPragmaDeclaration("#ident " + ident_string); // FIXME file info
                    result = buildPragmaDeclaration("#ident " + ident_string); // FIXME file info

                    break;
                  }
#endif /* IDENT_DIRECTIVE_AND_PRAGMA */

#if MICROSOFT_EXTENSIONS_ALLOWED
               case pk_comment:
                  {
                    ROSE_ASSERT(p->variant.comment.str != NULL);
                    string comment_string = convert_string_constant(p->variant.comment.str);
                    result = buildPragmaDeclaration("#comment " + comment_string);
#if 0
                    printf ("In parse_pragma(): Defining case pk_comment to return #comment pragma: kind = %d comment_string = %s \n",p->variant.comment.kind,comment_string.c_str());
#endif
                    break;
                  }
#endif

            // DQ (1/22/2018): Added C99 specific pragma support.
               case pk_stdc:
                  {
                 // a_stdc_pragma_kind  kind;  // For the C99 STDC predefined pragmas, indicates the specific pragma being used.
                 // a_stdc_pragma_value value; // Specifies whether the attribute is being turned on, off, or reset to the default value.
#if 0
                    printf ("In parse_pragma(): case pk_stdc: calling summarize_pragma(p = %p) \n",p);
                    summarize_pragma(p);
#endif
                    string kind_name = "STDC ";
                    switch(p->variant.stdc.kind)
                       {
                         case stdc_pk_none:
                            {
                              printf ("In parse_pragma(): case stdc_pk_none: (not yet implemented) \n");
                              ROSE_ASSERT(false);
                              break;
                            }

                         case stdc_pk_fp_contract:
                            {
                           // printf ("In parse_pragma(): case stdc_pk_cx_limited_range: (not yet implemented) \n");
                              kind_name += "FP_CONTRACT";
                              break;
                            }

                         case stdc_pk_fenv_access:
                            {
                           // printf ("In parse_pragma(): case stdc_pk_cx_limited_range: (not yet implemented) \n");
                              kind_name += "FENV_ACCESS";
                              break;
                            }

                         case stdc_pk_cx_limited_range:
                            {
                           // printf ("In parse_pragma(): case stdc_pk_cx_limited_range: (not yet implemented) \n");
                              kind_name += "FP_LIMITED_RANGE";
                              break;
                            }
#if FIXED_POINT_ALLOWED
                         case stdc_pk_fx_full_precision:
                         case stdc_pk_fx_fract_overflow:
                         case stdc_pk_fx_accum_overflow:
                            {
                              printf ("Error: In parse_pragma(): case pk_stdc: cases of fixed point support \n");
                              ROSE_ASSERT(false);
                            }
#endif /* FIXED_POINT_ALLOWED */

                         case stdc_pk_last:
                            {
                              printf ("Error: case stdc_pk_last: \n");
                              ROSE_ASSERT(false);
                            }

                         default: 
                            {
                              printf ("Error: In parse_pragma(): case pk_stdc: kind default reached \n");
                              ROSE_ASSERT(false);
                            }
                       }

                  switch (p->variant.stdc.value)
                     {
                       case stdc_pv_none:
                            {
                              printf ("In parse_pragma(): case stdc_pv_none: (not yet implemented) \n");
                              ROSE_ASSERT(false);
                              break;
                            }
                       case stdc_pv_off:
                            {
                           // printf ("In parse_pragma(): case stdc_pv_off: \n");
                              kind_name += " OFF";
                              break;
                            }
                       case stdc_pv_on:
                            {
                           // printf ("In parse_pragma(): case stdc_pv_on: \n");
                              kind_name += " ON";
                              break;
                            }
#if FIXED_POINT_ALLOWED
                       case stdc_pv_sat:
                            {
                           // Represents the "SAT" option for the "Embedded C" FX_FRACT_OVERFLOW and FX_ACCUM_OVERFLOW variants of "#pragma STDC ...".
                              printf ("Error: In parse_pragma(): case pk_stdc: value case of fixed point support \n");
                              ROSE_ASSERT(false);
                            }
#endif /* FIXED_POINT_ALLOWED */
                       case stdc_pv_default:
                            {
                           // printf ("Error: In parse_pragma(): case pk_stdc: value default reached \n");
                              kind_name += " DEFAULT";
                              break;
                            }

                       default:
                            {
                              printf ("Error: In parse_pragma(): default reached \n");
                              ROSE_ASSERT(false);
                            }
                       }
#if 0
                    printf ("In parse_pragma(): kind_name = %s\n",kind_name.c_str());
#endif
                    result = buildPragmaDeclaration(kind_name, topScopeStack());
#if 0
                    printf ("In parse_pragma(): Defining case pk_stdc pragma: kind = %d \n",p->variant.stdc.kind);
#endif
#if 0
                    printf ("Exiting as a test! \n");
                    ROSE_ASSERT(false);
#endif
                    break;
                  }
               case pk_none:
               case pk_printf_args:
               case pk_scanf_args:
               case pk_lint_argsused:
               case pk_lint_varargs_count:
               case pk_lint_notreached:
               case pk_instantiate:
               case pk_do_not_instantiate:
               case pk_can_instantiate:
               case pk_inline_template:
#if USER_CONTROL_OF_STRUCT_PACKING
               case pk_pack:
#endif
#if PRAGMA_WEAK_ALLOWED
               case pk_weak:
#endif /* PRAGMA_WEAK_ALLOWED */
               case pk_once:
               case pk_hdrstop:
               case pk_no_pch:
               case pk_define_type_info:
            // case pk_stdc:
#if UPC_EXTENSIONS_ALLOWED
            // case pk_upc:
#endif /* UPC_EXTENSIONS_ALLOWED */
#if REDEFINE_EXTNAME_PRAGMA_ENABLED
               case pk_redefine_extname:
#endif /* REDEFINE_EXTNAME_PRAGMA_ENABLED */
#if SUN_EXTENSIONS_ALLOWED
               case pk_enable_ldscope:
               case pk_disable_ldscope:
#endif /* SUN_EXTENSIONS_ALLOWED */
#if GNU_EXTENSIONS_ALLOWED
               case pk_gcc:
#endif
               case pk_diag_suppress:
               case pk_diag_remark:
               case pk_diag_warning:
               case pk_diag_error:
               case pk_diag_once:
               case pk_diag_default:
               case pk_db_opt:
               case pk_db_name:
#if GENERATE_MICROSOFT_IF_EXISTS_ENTRIES
               case pk_if_exists:
#endif
#if MICROSOFT_EXTENSIONS_ALLOWED
               case pk_push_macro:
               case pk_pop_macro:
               case pk_start_map_region:
               case pk_stop_map_region:
#if NATIVE_MULTIBYTE_CHARS_SUPPORTED_WITH_UNICODE
               case pk_setlocale:
#endif /* NATIVE_MULTIBYTE_CHARS_SUPPORTED_WITH_UNICODE */
            // case pk_comment:
               case pk_conform:
               case pk_include_alias:
#endif
#if INCLUDE_UNRECOGNIZED_PRAGMAS_IN_IL
               case pk_unrecognized:
#endif
               case pk_last:
                  {
                    printf ("pragma case not handled! kind = %d pragma_name = %s \n",kind,pragma_name);
                    ROSE_ASSERT(false);
                    break;
                  }

               default:
                    ERROR_MESSAGE(stderr, "Bad pragma kind %s\n", pragma_name);
                    ROSE_ASSERT(false);
             }
        }
#endif

  // DQ (7/14/2012): Added tests on returned variable (in new function design).
     ROSE_ASSERT(result != NULL);
     ROSE_ASSERT(result->get_file_info() != NULL);
     ROSE_ASSERT(result->get_parent() != NULL);

  // DQ (11/12/2012): Setting the source position information.
     set_file_info_for_where_corresp_is_unavailable(result,p->position);

  // DQ (10/15/2013): Set the position of the end of the pragma statement (using the start position and the lenght of the pragma string).
     int ending_column = result->get_startOfConstruct()->get_col() + string("#pragma ").length() + result->get_pragma()->get_pragma().length();

#if 0
     printf ("In parse_pragma(): ending_column = %d \n",ending_column);
     printf ("   --- result->get_startOfConstruct()->get_col()   = %d \n",result->get_startOfConstruct()->get_col());
     printf ("   --- length of #pragma prefix substring          = %zu \n",string("#pragma ").length());
     printf ("   --- result->get_pragma()->get_pragma()          = %s \n",result->get_pragma()->get_pragma().c_str());
     printf ("   --- result->get_pragma()->get_pragma().length() = %zu \n",result->get_pragma()->get_pragma().length());
#endif

     ROSE_ASSERT(ending_column > 0);

     result->get_endOfConstruct()->set_col(ending_column);

#if 0
     printf ("Leaving parse_pragma(): result = %p = %s \n",result,result->class_name().c_str());
     result->get_startOfConstruct()->display("Leaving parse_pragma():  START: debug");
     result->get_endOfConstruct()  ->display("Leaving parse_pragma():  END: debug");
#endif

     return result;

  // ROSE_ASSERT (!"Should not reach end of parse_pragma"); abort();
   }
