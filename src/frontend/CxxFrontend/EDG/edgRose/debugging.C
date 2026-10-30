#include "sage3basic.h"
#include "edgRose.h"
#include "rose_config.h"

using namespace Rose;

void 
EDG_ROSE_Translation::displayFunctionCache( const std::string & label )
   {
  // Support display of function cache for debugging.

     printf ("In displayFunctionCache(label = %s) \n",label.c_str());
  // std::map<a_routine_ptr, SgFunctionSymbol*>::iterator i = function_cache.begin();
     std::map<a_routine_ptr, SgSymbol*>::iterator i = function_cache.begin();
     while (i != function_cache.end())
        {
       // printf ("a_routine_ptr = %p SgFunctionSymbol = %p \n",i->first,i->second);
          printf ("a_routine_ptr = %p SgSymbol = %p (SgFunctionSymbol or SgTemplateSymbol) \n",i->first,i->second);
          i++;
        }
   }

#ifndef __cplusplus
// Support for compiler C++ code using C compiler.
#define false FALSE
#define true TRUE
#define bool int
#define ROSE_ASSERT(x) assert(x)

// DQ (4/23/2016): We don't want to support compiling this file using C instead of C++.
#error "Compiling this code using C instead of C++ is not longer supported!"

#endif


#ifdef __cplusplus
// DQ (12/30/2008): Added this so that "string" will be defined.
using namespace std;
#endif

#if (VERSION_NUMBER_FOR_MACRO < 310)
  // DQ (12/30/2008): These are not in EDG 3.3, but in later versions of 3.x, and EDG 4.0
  // DQ (12/30/2008): Not available in new connection to EDG/Sage/ROSE translation.
extern an_il_to_str_output_control_block
		octl;	/* Output control block for interface to il_to_str
			   routines. */
#endif

// Build a global array and a function to place strings into it
// then set the octl.output_str function pointer to our new function
// and accumulate the type name into the new function.
#ifdef __cplusplus
extern "C" {
static char displayTreeStringBuffer [1024];
void displayTreeStringFunction ( const char* inputString )
   {
     int bufferPosition = strlen(displayTreeStringBuffer);
     assert (bufferPosition < 1024);
     char* remainingBuffer = &(displayTreeStringBuffer[bufferPosition]);
     sprintf (remainingBuffer,"%s",inputString);
   }
}
#endif

const char*
summarize_entry_kind_in_string_kind (an_il_entry_kind entry_kind)
   {
  // In this case EDG provides a list of the names associated with this enum
  // return il_entry_kind_names[(int)entry_kind];

  // char* name = NULL;
  // name = il_entry_kind_names[(int)entry_kind];

  // Except for the memory issues I would like to return more information in the name.
  // For example, when the entry is a statement append ":" and then the kind of the statement.
  // This is not possible with this "char* interface.
  // return name;

     return il_entry_kind_names[(int)entry_kind];
   }

string
summarize_entry_kind_in_string_kind (const a_tagged_pointer & entry, bool includePointerValue)
   {
  // This later version of this function provides more informative output for debugging.

     string name;

     an_il_entry_kind entry_kind = (an_il_entry_kind)entry.kind;

  // DQ (1/22/2018): This kind appears in stdc pragmas.
  // ROSE_ASSERT(entry_kind != iek_none);

     name = il_entry_kind_names[entry_kind];

  // Except for the memory issues I would like to return more information in the name.
  // For example, when the entry is a statement append ":" and then the kind of the statement.
  // This is not possible with this "char* interface.

  // DQ (1/22/2018): This kind appears in stdc pragmas.
     if (entry_kind == iek_none)
        {
       // name += string("(") + StringUtility::numberToString(entry.ptr) + string("):") + summarize_statement_in_string((a_statement_ptr)entry.ptr);
          if (includePointerValue == true)
               name += string("(") + StringUtility::numberToString(entry.ptr) + string(")");
          name += string(":") + "iek_none";
        }

     if (entry_kind == iek_statement)
        {
       // name += string("(") + StringUtility::numberToString(entry.ptr) + string("):") + summarize_statement_in_string((a_statement_ptr)entry.ptr);
          if (includePointerValue == true)
               name += string("(") + StringUtility::numberToString(entry.ptr) + string(")");
          name += string(":") + summarize_statement_in_string((a_statement_ptr)entry.ptr);
        }

     if (entry_kind == iek_type)
        {
       // name += string("(") + StringUtility::numberToString(entry.ptr) + string("):") + summarize_type_kind_in_string((a_type_ptr)entry.ptr);
          if (includePointerValue == true)
               name += string("(") + StringUtility::numberToString(entry.ptr) + string(")");
          name += string(":") + summarize_type_kind_in_string((a_type_ptr)entry.ptr);
        }

     if (entry_kind == iek_expr_node)
        {
       // name += string("(") + StringUtility::numberToString(entry.ptr) + string("):") + summarize_expression_in_string((an_expr_node_ptr)entry.ptr);
          if (includePointerValue == true)
               name += string("(") + StringUtility::numberToString(entry.ptr) + string(")");
          name += string(":") + summarize_expression_in_string((an_expr_node_ptr)entry.ptr);
        }

  // DQ (10/29/2012): Added more debugging information
     if (entry_kind == iek_src_seq_end_of_construct)
        {
          a_src_seq_end_of_construct_ptr src_seq_end_of_construct = (a_src_seq_end_of_construct_ptr)(entry.ptr);
       // printf ("In SourceSequenceList::operator++(): associated entry to end_of_construct (before increment) sse = %p = %s \n",
       //      src_seq_end_of_construct->entity.ptr,summarize_entry_kind_in_string_kind(src_seq_end_of_construct->entity).c_str());
       // name += string("(") + StringUtility::numberToString(entry.ptr) + string("): asociated with entity = ") + summarize_entry_kind_in_string_kind(src_seq_end_of_construct->entity);
          if (includePointerValue == true)
               name += string("(") + StringUtility::numberToString(entry.ptr) + string(")");
          name += string(": asociated with entity = ") + summarize_entry_kind_in_string_kind(src_seq_end_of_construct->entity);
        }

     return name;
   }


char*
summarize_address_base_kind_in_string ( an_address_base_kind kind )
   {
#if 0
  abk_routine,          /* Pointer to a function. */
  abk_variable,         /* Pointer to a variable. */
  abk_constant,		/* Pointer to a constant. */
  abk_uuidof,		/* Pointer to _GUID structure for Microsoft __uuidof
			   operation. */
  abk_label             /* Pointer to a label.  This is used for the
			   GNU address-of-label extension. */
#endif

     char* s = NULL;
     switch (kind)
        {
          case abk_routine:
               s = (char*)"abk_routine";
               break;
          case abk_variable:
               s = (char*)"abk_variable";
               break;
          case abk_constant:
               s = (char*)"abk_constant";
               break;
          case abk_uuidof:
               s = (char*)"abk_uuidof";
               break;
          case abk_label:
               s = (char*)"abk_label";
               break;

          default:
               s = (char*)"**BAD ADDRESS BASE KIND**";
        }  /* switch */

     assert(s != NULL);
     return s;
   }


char*
summarize_init_kind_in_string ( an_init_kind kind )
   {
     char* s = NULL;
     switch (kind)
        {
          case initk_none:
               s = (char*)"initk_none";
               break;
          case initk_static:
               s = (char*)"initk_static";
               break;
          case initk_dynamic:
               s = (char*)"initk_dynamic";
               break;
          case initk_zero:
               s = (char*)"initk_zero";
               break;
          case initk_function_local:
               s = (char*)"initk_function_local";
               break;

          default:
               s = (char*)"**BAD INIT  KIND**";
        }  /* switch */

     assert(s != NULL);
     return s;
   }

// string summarize_qualifiers_in_string ( a_bit_field qualifiers:NUM_BITS_FOR_TYPE_QUALIFIER_SET )
string
summarize_qualifiers_in_string ( size_t qualifiers )
   {
  // a_bit_field qualifiers:NUM_BITS_FOR_TYPE_QUALIFIER_SET;
     string s;

     if (qualifiers & TQ_NONE)      s += "none ";
     if (qualifiers & TQ_CONST)     s += "const ";
     if (qualifiers & TQ_VOLATILE)  s += "volatile ";
     if (qualifiers & TQ_RESTRICT)  s += "restrict ";
#if MICROSOFT_EXTENSIONS_ALLOWED
     if (qualifiers & TQ_UNALIGNED) s += "unaligned ";
#endif
#if NEAR_AND_FAR_ALLOWED
     if (qualifiers & TQ_NEAR) s += "near ";
     if (qualifiers & TQ_FAR)  s += "far ";
#endif
#if UPC_EXTENSIONS_ALLOWED
     if (qualifiers & TQ_UPC_SHARED)  s += "upc_shared ";
     if (qualifiers & TQ_UPC_STRICT)  s += "upc_strict ";
     if (qualifiers & TQ_UPC_RELAXED) s += "upc_relaxed ";
#endif
#if NAMED_ADDRESS_SPACES_ALLOWED
  // DQ (10/28/2015): Not clear how to support these qualifiers.
  // #error "Named address spaces are not supported!"
#endif

  // if (qualifiers & TQ_) s += "xxx";

  // printf ("In summarize_qualifiers_in_string(%zu): s = %s \n",qualifiers,s.c_str());

     return s;
   }


char*
summarize_constant_kind_in_string(a_constant_ptr ptr)
   {
  // Display the indicated constant entry.
     char* s = NULL;
     assert (ptr != NULL);
     switch (ptr->kind)
        {
          case ck_error:
               s = (char*)"ck_error";
               break;
          case ck_integer:
               s = (char*)"ck_integer";
               break;
#if FIXED_POINT_ALLOWED
          case ck_fixed_point:
               s = (char*)"ck_fixed_point";
               break;
#endif /* FIXED_POINT_ALLOWED */
          case ck_string:
               s = (char*)"ck_string";
               break;
          case ck_float:
               s = (char*)"ck_float";
               break;
#if defined(FFE) || C99_IL_EXTENSIONS_SUPPORTED
          case ck_complex:
               s = (char*)"ck_complex";
               break;
#endif
#if C99_IL_EXTENSIONS_SUPPORTED
          case ck_imaginary:
               s = (char*)"ck_imaginary";
               break;
#endif
#ifdef CFE
          case ck_address:
               s = (char*)"ck_address";
               switch (ptr->variant.address.kind)
                  {
                    case abk_routine:
                         s = (char*)"ck_address:abk_routine";
                         break;
                    case abk_variable:
                         s = (char*)"ck_address:abk_variable";
                         break;
                    case abk_constant:
                         s = (char*)"ck_address:abk_constant";
                         break;
                    case abk_typeid:
                         s = (char*)"ck_address:abk_typeid";
                         break;
                    case abk_uuidof:
                         s = (char*)"ck_address:abk_uuidof";
                         break;
                    case abk_temporary:
                         s = (char*)"ck_address:abk_temporary";
                         break;
                    default:
                         s = (char*)"ck_address:**BAD ADDRESS CONSTANT KIND**";
                  }  /* switch */
               break;
          case ck_ptr_to_member:
            // s = "ck_ptr_to_member";
               if (ptr->variant.ptr_to_member.is_function_ptr)
                  {
                    s = (char*)"ck_ptr_to_member:routine";
                 // pdfOutput_ptr("ck_ptr_to_member:routine", (char *)ptr->variant.ptr_to_member.variant.routine, iek_routine);
                  }
                 else
                  {
                    s = (char*)"ck_ptr_to_member:field";
                 // pdfOutput_ptr("ck_ptr_to_member:field", (char *)ptr->variant.ptr_to_member.variant.field, iek_field);
                  }  /* if */
               break;
#if DO_IL_LOWERING && GENERATE_EH_TABLES && !DO_FULL_PORTABLE_EH_LOWERING
          case ck_stack_offset:
               s = (char*)"ck_stack_offset";
               break;
#endif /* DO_IL_LOWERING && ... */
          case ck_dynamic_init:
               s = (char*)"ck_dynamic_init";
               break;
#endif /* ifdef CFE */
          case ck_aggregate:
               s = (char*)"ck_aggregate";
               break;
          case ck_init_repeat:
               s = (char*)"ck_init_repeat";
               break;
          case ck_template_param:    /* C++ Front end only, except when prototype instantiations are passed to a back end. */
               s = (char*)"ck_template_param";
               break;
          case ck_designator:        /* Used to change the "current object" in an aggregate initializer (C99). */
               s = (char*)"ck_designator";
               break;
#if UPC_EXTENSIONS_ALLOWED
          case ck_upc_threads:       /* A multiple of the UPC pseudo-constant THREADS. */
               s = (char*)"ck_upc_threads";
               break;
          case ck_upc_mythread:      /* The UPC pseudo-constant MYTHREAD. */
               s = (char*)"ck_upc_mythread";
               break;
#endif /* UPC_EXTENSIONS_ALLOWED */
          case ck_last:		/*lint -esym(769,a_constant_repr_kind_tag::ck_last)*/
               s = (char*)"ck_last";
               break;

          case ck_void:
            // This is a C++14 specific constant.
               s = (char*)"ck_void";
               break;
          default:
               s = (char*)"**BAD CONSTANT KIND**";
        }  /* switch */

     assert(s != NULL);
     return s;
   }  /* summarize_constant_kind_in_string */


char*
summarize_scope_in_string (a_scope_ptr ptr)
   {
     char* s = NULL;
     switch (ptr->kind)
        {
          case sck_file:
               s = (char*)"sck_file";
               break;
          case sck_func_prototype:
               s = (char*)"sck_func_prototype";
               break;
          case sck_block:
               s = (char*)"sck_block";
               break;
          case sck_namespace:
               s = (char*)"sck_namespace";
               break;
          case sck_namespace_extension:
               s = (char*)"sck_namespace_extension";
               break;
          case sck_namespace_reactivation:
               s = (char*)"sck_namespace_reactivation";
               break;
          case sck_class_struct_union:
               s = (char*)"sck_class_struct_union";
               break;
          case sck_class_reactivation:
               s = (char*)"sck_class_reactivation";
               break;
          case sck_template_declaration:
               s = (char*)"sck_template_declaration";
               break;
          case sck_template_instantiation:
               s = (char*)"sck_template_instantiation";
               break;
          case sck_instantiation_context:
               s = (char*)"sck_instantiation_context";
               break;
          case sck_pragma:
               s = (char*)"sck_pragma";
               break;
          case sck_function_access:
               s = (char*)"sck_function_access";
               break;
          case sck_condition:
               s = (char*)"sck_condition";
               break;
          case sck_enum:
               s = (char*)"sck_stmt_function";
               break;
          case sck_function:
               s = (char*)"sck_function";
               break;
      /* Front end only. */
          default:
               s = (char*)"**BAD SCOPE KIND**";
               printf ("Error: default reached in switch for scope! \n");
               abort();
        }  /* switch */

     assert (s != NULL);

     return s;
   }



char*
summarize_template_param_type_kind_in_string (a_template_param_type_kind kind)
   {
     char* s = NULL;

     switch (kind)
        {
          case tpck_param:            s = (char*)"tpck_param";            break;
          case tpck_expression:       s = (char*)"tpck_expression";       break;
          case tpck_member:           s = (char*)"tpck_member";           break;
          case tpck_unknown_function: s = (char*)"tpck_unknown_function"; break;
          case tpck_address:          s = (char*)"tpck_address";          break;
          case tpck_sizeof:           s = (char*)"tpck_sizeof";           break;
          case tpck_alignof:          s = (char*)"tpck_alignof";          break;
          case tpck_uuidof:           s = (char*)"tpck_uuidof";           break;
          case tpck_typeid:           s = (char*)"tpck_typeid";           break;
          case tpck_template_ref:     s = (char*)"tpck_template_ref";     break;
          case tpck_noexcept:         s = (char*)"tpck_noexcept";         break;
          case tpck_destructor:       s = (char*)"tpck_destructor";       break;
          case tpck_integer_pack:     s = (char*)"tpck_integer_pack";     break;
          default:
             {
               s = (char*)"**BAD SCOPE KIND**";
               printf ("Error: default reached in switch for summarize_template_param_type_kind_in_string! \n");
               ROSE_ASSERT(false);
             }
        }

     ROSE_ASSERT(s != NULL);

     return s;
   }


char*
summarize_template_kind_in_string (a_template_kind kind)
   {
  // DQ (9/13/2012): Added more support for debugging templates.
     char* s = NULL;
     switch (kind)
        {
          case templk_class:                   s = (char*)"templk_class";                   break;
          case templk_member_class:            s = (char*)"templk_member_class";            break;
          case templk_function:                s = (char*)"templk_function";                break;
          case templk_variable:                s = (char*)"templk_variable";                break;
          case templk_member_function:         s = (char*)"templk_member_function";         break;
          case templk_static_data_member:      s = (char*)"templk_static_data_member";      break;
          case templk_member_enum:             s = (char*)"templk_member_enum";             break;
          case templk_template_template_param: s = (char*)"templk_template_template_param"; break;
          case templk_concept:                 s = (char*)"templk_concept";                 break;

          default:
             {
               s = (char*)"**BAD TEMPLATE KIND**";
               printf ("Error: default reached in switch for summarize_template_kind_in_string! kind = %d \n",(int)kind);
               ROSE_ASSERT(false);
             }
        }

     ROSE_ASSERT(s != NULL);

     return s;
   }

char*
summarize_symbol_kind_in_string(a_symbol_ptr ptr)
/*
Display the indicated symbol entry.
*/
   {
     char* defaultString = (char*)"<null input in summarize_symbol_kind_in_string>";
     if (ptr == NULL)
          return defaultString;

     return summarize_symbol_kind_in_string_kind(ptr->kind);
   }

char*
summarize_symbol_kind_in_string_kind(a_symbol_kind kind)
   {
  // Display the indicated type entry.
     char* s = NULL;

  // printf ("In summarize_symbol_kind_in_string(kind = %d) \n",kind);

     switch (kind)
        {
          case sk_keyword:              s = (char*)"sk_keyword";              break;
          case sk_macro:                s = (char*)"sk_macro";                break;
          case sk_constant:             s = (char*)"sk_constant";             break;
          case sk_type:                 s = (char*)"sk_type";                 break;
          case sk_class_or_struct_tag:  s = (char*)"sk_class_or_struct_tag";  break;
          case sk_union_tag:            s = (char*)"sk_union_tag";            break;
          case sk_enum_tag:             s = (char*)"sk_enum_tag";             break;
          case sk_variable:             s = (char*)"sk_variable";             break;
          case sk_field:                s = (char*)"sk_field";                break;
          case sk_static_data_member:   s = (char*)"sk_static_data_member";   break;
          case sk_member_function:      s = (char*)"sk_member_function";      break;
          case sk_routine:              s = (char*)"sk_routine";              break;
          case sk_label:                s = (char*)"sk_label";                break;
          case sk_undefined:            s = (char*)"sk_undefined";            break;
          case sk_extern_variable:      s = (char*)"sk_extern_variable";      break;
          case sk_extern_routine:       s = (char*)"sk_extern_routine";       break;
          case sk_projection:           s = (char*)"sk_projection";           break;
          case sk_overloaded_function:  s = (char*)"sk_overloaded_function";  break;
          case sk_parameter:            s = (char*)"sk_parameter";            break;
          case sk_class_template:       s = (char*)"sk_class_template";       break;
          case sk_function_template:    s = (char*)"sk_function_template";    break;
          case sk_namespace:            s = (char*)"sk_namespace";            break;
          case sk_namespace_projection: s = (char*)"sk_namespace_projection"; break;
#if NAMED_ADDRESS_SPACES_ALLOWED
          case sk_named_address_space:  s = (char*)"sk_named_address_space";  break;
#endif
#if NAMED_REGISTERS_ALLOWED
          case sk_named_register:       s = (char*)"sk_named_register";       break;
#endif
#if MICROSOFT_EXTENSIONS_ALLOWED
          case sk_property_set:         s = (char*)"sk_property_set";         break;
#endif
          case sk_last:                 s = (char*)"sk_last";                 break;

          default:
               printf ("Error: default reached kind = %d \n",kind);
               s = (char*)"**BAD SYMBOL KIND**";
        }

     assert (s != NULL);

     return s;
   }


char*
summarize_type_kind_in_string(a_type_ptr ptr)
/*
Display the indicated type entry.
*/
   {
     char* defaultString = (char*)"<null input in summarize_type_kind_in_string>";
     if (ptr == NULL)
          return defaultString;

     return summarize_type_kind_in_string_kind(ptr->kind);
   }

char*
summarize_template_param_constant_kind_in_string(a_template_param_constant_kind kind)
   {
     char* s = NULL;
     switch (kind)
        {
          case tpck_param:
               s = (char*)"tpck_param";
               break;
          case tpck_expression:
               s = (char*)"tpck_expression";
               break;
          case tpck_member:
               s = (char*)"tpck_member";
               break;
          case tpck_unknown_function:
               s = (char*)"tpck_unknown_function";
               break;
          case tpck_address:
               s = (char*)"tpck_address";
               break;
          case tpck_sizeof:
               s = (char*)"tpck_sizeof";
               break;
          case tpck_alignof:
               s = (char*)"tpck_alignof";
               break;
          case tpck_uuidof:
               s = (char*)"tpck_uuidof";
               break;
          case tpck_typeid:
               s = (char*)"tpck_typeid";
               break;
          case tpck_template_ref:
               s = (char*)"tpck_template_ref";
               break;
          case tpck_noexcept:
               s = (char*)"tpck_noexcept";
               break;
          case tpck_destructor:
               s = (char*)"tpck_destructor";
               break;
          case tpck_integer_pack:
               s = (char*)"tpck_integer_pack";
               break;
          default:
               s = (char*)"**BAD TEMPLATE PARAMETER CONSTANT KIND**";
        }  /* switch */

     assert (s != NULL);

     return s;
   }  /* summarize_template_param_constant_kind_in_string */

char*
summarize_type_kind_in_string_kind(a_type_kind kind)
/*
Display the indicated type entry.
*/
   {
     char* s = NULL;
  // char* defaultString = "<null input in summarize_type_kind_in_string>";

  // handle case of null pointer as input
  // if (ptr == NULL) return defaultString;

     switch (kind)
        {
          case tk_error:
               s = (char*)"tk_error";
               break;
          case tk_unknown:
               s = (char*)"tk_unknown";
               break;
          case tk_void:
               s = (char*)"tk_void";
               break;
          case tk_integer:
             {
               s = (char*)"tk_integer";
               break;
             }
          case tk_float:
               s = (char*)"tk_float";
               break;
#if C99_IL_EXTENSIONS_SUPPORTED
          case tk_imaginary: // C99 imaginary types.
               s = (char*)"tk_imaginary";
               break;
#endif /* C99_IL_EXTENSIONS_SUPPORTED */
#if C99_IL_EXTENSIONS_SUPPORTED
          case tk_complex:
               s = (char*)"tk_complex";
               break;
#endif
#ifdef FFE
//             goto do_float_complex;
//        case tk_complex:
//             s = "tk_complex";
// do_float_complex:
#endif /* ifdef FFE */
               break;
          case tk_pointer:
               s = (char*)"tk_pointer";
               break;
          case tk_routine:
               s = (char*)"tk_routine";
               break;
#ifdef CFE
          case tk_array:
               s = (char*)"tk_array";
               break;
          case tk_class:
               s = (char*)"tk_class";
               goto do_struct_union;
          case tk_struct:
               s = (char*)"tk_struct";
               goto do_struct_union;
          case tk_union:
               s = (char*)"tk_union";
do_struct_union:
               break;
          case tk_typeref:
               s = (char*)"tk_typeref";
               break;
          case tk_ptr_to_member:
               s = (char*)"tk_ptr_to_member";
               break;
#else
#error "CFE is not defined"
#endif /* ifdef CFE */
#ifdef FFE
          case tk_fcharacter:
               s = (char*)"tk_fcharacter";
               break;
          case tk_hollerith:
               s = (char*)"tk_hollerith";
               break;
          case tk_farray:
               s = (char*)"tk_farray";
               break;
          case tk_stmt_label:
               s = (char*)"tk_stmt_label";
               break;
          case tk_format:
               s = (char*)"tk_format";
               break;
          case tk_association:
               s = (char*)"tk_association";
               break;
          case tk_unspec_routine:
               s = (char*)"tk_unspec_routine";
               break;
          case tk_blockdata:
               s = (char*)"tk_blockdata";
               break;
#endif /* ifdef FFE */
          case tk_template_param:
               s = (char*)"tk_template_param";
               break;
#if GNU_VECTOR_TYPES_ALLOWED
          case tk_vector:
               s = (char*)"tk_vector";
               break;
#endif
          case tk_nullptr:
               s = (char*)"tk_nullptr";
               break;
          default:
               s = (char*)"**BAD TYPE KIND**";
               printf ("Default reached in summarize_type_kind_in_string_kind() kind = %d \n",(int)kind);

            // DQ (6/20/2013): Added assertion to detect this default case (and force implementation of any missing cases).
               ROSE_ASSERT(false);
        }  /* switch */

     assert (s != NULL);

     return s;
   }  /* summarize_type_kind_in_string */

char*
summarize_decl_modifier(a_decl_modifier kind)
   {
  // DQ (5/24/2015): I think this is the not the correct implementation for this function becaue the
  // modifiers are bits of the bitset represented by the kind, and not specific values.  There is
  // now a better implementation of this function in:
  //    void output_decl_modifier_enum_values(a_decl_modifier decl_modifier);
     char* s = NULL;

     if (kind > dmt_last)
        {
          printf ("Error: In summarize_decl_modifier(): kind = %u dmt_last = %u \n",kind,dmt_last);
          s = (char*)"**BAD DECL MODIFIER KIND**";
        }
     switch (kind)
        {
#if MICROSOFT_EXTENSIONS_ALLOWED
          case dmt_dllimport: s = (char*)"dmt_dllimport"; break;
          case dmt_dllexport: s = (char*)"dmt_dllexport"; break;
#endif
#if MICROSOFT_EXTENSIONS_ALLOWED || THREAD_LOCAL_STORAGE_SPECIFIER_ALLOWED
          case dmt_thread: s = (char*)"dmt_thread"; break;
#endif
#if MICROSOFT_EXTENSIONS_ALLOWED
          case dmt_microsoft_inline: s = (char*)"dmt_microsoft_inline"; break;
          case dmt_forceinline: s = (char*)"dmt_forceinline"; break;
          case dmt_selectany: s = (char*)"dmt_selectany"; break;
          case dmt_novtable: s = (char*)"dmt_novtable"; break;
          case dmt_noalias: s = (char*)"dmt_noalias"; break;
          case dmt_restrict: s = (char*)"dmt_restrict"; break;
          case dmt_safebuffers: s = (char*)"dmt_safebuffers"; break;
#endif
#if SUN_EXTENSIONS_ALLOWED
          case dmt_global_link_scope: s = (char*)"dmt_global_link_scope"; break;
          case dmt_symbolic_link_scope: s = (char*)"dmt_symbolic_link_scope"; break;
          case dmt_hidden_link_scope: s = (char*)"dmt_hidden_link_scope"; break;
#endif
#if ENABLE_CUDA_SUPPORT
          case dmt_host: s = (char*)"dmt_host"; break;
          case dmt_global: s = (char*)"dmt_global"; break;
          case dmt_device: s = (char*)"dmt_device"; break;
          case dmt_shared: s = (char*)"dmt_shared"; break;
          case dmt_local: s = (char*)"dmt_local"; break;
          case dmt_constant: s = (char*)"dmt_constant"; break;
#endif
          case dmt_last: s = (char*)"dmt_last"; break;

          default:
               s = (char*)"**BAD DECL MODIFIER KIND**";
        }  /* switch */

     assert (s != NULL);

     return s;
   }  /* summarize_decl_modifier */









char*
summarize_string_literal_kind(a_string_or_char_literal_kind literal_kind)
   {
      char* s = (char*)"";

#if 0
     int kind = literal_encoding_prefix(literal_kind);

#if 1
     printf ("In summarize_string_literal_kind(): literal_kind = %d kind = %d \n",literal_kind,kind);
#endif

     switch (kind)
        {
          case SCLK_NOT_A_LITERAL:
               s = (char*)"SCLK_NOT_A_LITERAL: The characters are not a literal prefix";
               break;

          case SCLK_ORDINARY_LITERAL:
               s = (char*)"SCLK_ORDINARY_LITERAL: No encoding prefix";
               break;

          case SCLK_UTF8_LITERAL:
               s = "(char*)SCLK_UTF8_LITERAL: u8";
               break;

          case SCLK_CHAR16_T_LITERAL:
               s = (char*)"SCLK_CHAR16_T_LITERAL: u";
               break;

          case SCLK_CHAR32_T_LITERAL:
               s = (char*)"SCLK_CHAR32_T_LITERAL: U";
               break;

          case SCLK_WIDE_LITERAL:
               s = (char*)"SCLK_WIDE_LITERAL: L";
               break;

          case SCLK_RAW_STRING_LITERAL:
               s = (char*)"SCLK_RAW_STRING_LITERAL: R";
               break;

          case SCLK_STRING_LITERAL:
               s = (char*)"SCLK_STRING_LITERAL: TRUE for string literal, FALSE for char literal";
               break;

          case SCLK_ORDINARY_STRING_LITERAL:
               s = (char*)"SCLK_ORDINARY_STRING_LITERAL: A convenient name for a narrow non-raw string literal";
               break;

          default:
               s = (char*)"**BAD LITERAL KIND**";
               printf ("Default reached in summarize_string_literal_kind() kind = %d \n",(int)kind);
        }
#else
#if 1
     int tmp_kind = literal_encoding_prefix(literal_kind);
     printf ("In summarize_string_literal_kind(): literal_kind = %d tmp_kind = %d \n",literal_kind,tmp_kind);
#endif

     bool is_raw_string = (literal_kind & SCLK_RAW_STRING_LITERAL) != 0;
     if (is_raw_string == true)
        {
          s = (char*)"SCLK_RAW_STRING_LITERAL: R";
          printf ("s = %s \n",s);
        }

#if 0
     bool is_string_literal = (literal_kind & SCLK_STRING_LITERAL) != 0;
     if (is_string_literal == true)
        {
          s = (char*)"SCLK_STRING_LITERAL: TRUE for string literal, FALSE for char literal";
          printf ("s = %s \n",s);
        }
#endif

  // DQ (8/13/2014): After checking for SCLK_RAW_STRING_LITERAL and SCLK_STRING_LITERAL reset the literal_kind.
     literal_kind = literal_encoding_prefix(literal_kind);

     if (literal_kind == SCLK_NOT_A_LITERAL)
        {
          s = (char*)"SCLK_NOT_A_LITERAL: The characters are not a literal prefix";
          printf ("SCLK_NOT_A_LITERAL = %d s = %s \n",SCLK_NOT_A_LITERAL,s);
        }
#if 0
  // bool is_ordinary_literal = (literal_kind & SCLK_ORDINARY_LITERAL) != 0;
     bool is_ordinary_literal = (literal_kind == SCLK_ORDINARY_LITERAL);
     if (is_ordinary_literal == true)
        {
          s = (char*)"SCLK_ORDINARY_LITERAL: No encoding prefix";
          printf ("SCLK_ORDINARY_LITERAL = %d s = %s \n",SCLK_ORDINARY_LITERAL,s);
        }
#endif
  // bool is_utf8_literal = (literal_kind & SCLK_UTF8_LITERAL) != 0;
     bool is_utf8_literal = (literal_kind == SCLK_UTF8_LITERAL);
     if (is_utf8_literal == true)
        {
          s = (char*)"SCLK_UTF8_LITERAL: u8";
          printf ("SCLK_UTF8_LITERAL = %d s = %s \n",SCLK_UTF8_LITERAL,s);
        }

  // bool is_char16_literal = (literal_kind & SCLK_CHAR16_T_LITERAL) != 0;
     bool is_char16_literal = (literal_kind == SCLK_CHAR16_T_LITERAL);
     if (is_char16_literal == true)
        {
          s = (char*)"SCLK_CHAR16_T_LITERAL: u";
          printf ("SCLK_CHAR16_T_LITERAL = %d s = %s \n",SCLK_CHAR16_T_LITERAL,s);
        }

  // bool is_char32_literal = (literal_kind & SCLK_CHAR32_T_LITERAL) != 0;
     bool is_char32_literal = (literal_kind == SCLK_CHAR32_T_LITERAL);
     if (is_char32_literal == true)
        {
          s = (char*)"SCLK_CHAR32_T_LITERAL U";
          printf ("SCLK_CHAR32_T_LITERAL = %d s = %s \n",SCLK_CHAR32_T_LITERAL,s);
        }

  // bool is_wide_literal = (literal_kind & SCLK_WIDE_LITERAL) != 0;
     bool is_wide_literal = (literal_kind == SCLK_WIDE_LITERAL);
     if (is_wide_literal == true)
        {
          s = (char*)"SCLK_WIDE_LITERAL: L";
          printf ("SCLK_WIDE_LITERAL = %d s = %s \n",SCLK_WIDE_LITERAL,s);
        }

#if 0
  // bool is_ordinary_string_literal = (literal_kind & SCLK_ORDINARY_STRING_LITERAL) != 0;
     bool is_ordinary_string_literal = (literal_kind == SCLK_ORDINARY_STRING_LITERAL);
     if (is_ordinary_string_literal == true)
        {
          s = (char*)"SCLK_ORDINARY_STRING_LITERAL: A convenient name for a narrow non-raw string literal";
          printf ("SCLK_ORDINARY_STRING_LITERAL = %d s = %s \n",SCLK_ORDINARY_STRING_LITERAL,s);
        }
#endif
#endif

     assert (s != NULL);

     return s;
   }


void
summarize_constant(a_constant *cp)
   {
  // Print a short version of the constant at *cp.

  // form_constant(cp, /*need_parens=*/FALSE, &octl);

  // printf ("summarize_constant() function not implemented! \n");

     ROSE_ASSERT(cp != NULL);

     printf ("In summarize_constant for constant                      = %p \n",cp);
     printf ("     Output source_correspondence information: \n");
#ifdef __cplusplus
     summarize_source_correspondence(cp->source_corresp);
#endif
     printf ("     DONE: Output source_correspondence information: \n");
#if EXTRA_SOURCE_POSITIONS_IN_IL
     printf ("     end_position: \n");
     summarize_source_position(cp->end_position);
#endif
     printf ("     cp->next                                           = %p \n",cp->next);
     printf ("     cp->type                                           = %p \n",cp->type);
     printf ("     cp->orig_type                                      = %p \n",cp->orig_type);
     printf ("     cp->expr                                           = %p \n",cp->expr);
     printf ("     cp->rescan_info                                    = %p \n",cp->rescan_info);
#if DO_IL_LOWERING
     printf ("     cp->assoc_var                                      = %p \n",cp->assoc_var);
#endif
     printf ("     cp->character_kind                                 = %" PRIuPTR " \n",(size_t)cp->character_kind);
     printf ("        --- NUM_BITS_FOR_CHARACTER_KIND                 = %u \n",(unsigned int)(NUM_BITS_FOR_CHARACTER_KIND));
     printf ("     cp->implicit_cast                                  = %s \n",cp->implicit_cast ? "true" : "false");
     printf ("     cp->explicit_cast_applied                          = %s \n",cp->explicit_cast_applied ? "true" : "false");
     printf ("     cp->is_reinterpret_cast                            = %s \n",cp->is_reinterpret_cast ? "true" : "false");
     printf ("     cp->is_reinterpret_like_cast                       = %s \n",cp->is_reinterpret_like_cast ? "true" : "false");

     printf ("     cp->non_arithmetic                                 = %s \n",cp->non_arithmetic ? "true" : "false");
     printf ("     cp->is_simple_zero                                 = %s \n",cp->is_simple_zero ? "true" : "false");
#if DO_IL_LOWERING || BACK_END_IS_C_GEN_BE
     printf ("     cp->assoc_var_assigned                             = %s \n",cp->assoc_var_assigned ? "true" : "false");
#endif
     printf ("     cp->null_pointer_constant_ruled_out                = %s \n",cp->null_pointer_constant_ruled_out ? "true" : "false");
#if GNU_EXTENSIONS_ALLOWED
     printf ("     cp->null_keyword                                   = %s \n",cp->null_keyword ? "true" : "false");
#endif
     printf ("     cp->nullptr_keyword                                = %s \n",cp->nullptr_keyword ? "true" : "false");
#if MICROSOFT_EXTENSIONS_ALLOWED
     printf ("     cp->native_nullptr_keyword                         = %s \n",cp->native_nullptr_keyword ? "true" : "false");
#endif
     printf ("     cp->explicit_braces_on_aggregate                   = %s \n",cp->explicit_braces_on_aggregate ? "true" : "false");

  // DQ (7/21/2013): Added more debugging info (likely only in later versions of EDG).
     printf ("     cp->from_undefined_preproc_id                      = %s \n",cp->from_undefined_preproc_id ? "true" : "false");
#if MICROSOFT_EXTENSIONS_ALLOWED || GNU_EXTENSIONS_ALLOWED
     printf ("     cp->flexible_array_initializer                     = %s \n",cp->flexible_array_initializer ? "true" : "false");
#endif
     printf ("     cp->uses_designated_initializers                   = %s \n",cp->uses_designated_initializers ? "true" : "false");
#if MICROSOFT_EXTENSIONS_ALLOWED
     printf ("     cp->is_literal_field                               = %s \n",cp->is_literal_field ? "true" : "false");
#endif
     printf ("     cp->is_pack_expansion                              = %s \n",cp->is_pack_expansion ? "true" : "false");
#if BACK_END_IS_C_GEN_BE
     printf ("     cp->elide_aggregate_braces                         = %s \n",cp->elide_aggregate_braces ? "true" : "false");
#endif
     printf ("     cp->is_named_constant_definition                   = %s \n",cp->is_named_constant_definition ? "true" : "false");
     printf ("     cp->partial_aggr_value                             = %s \n",cp->partial_aggr_value ? "true" : "false");
     printf ("     cp->is_partially_initialized                       = %s \n",cp->is_partially_initialized ? "true" : "false");
     printf ("     cp->implicit_aggr_element                          = %s \n",cp->implicit_aggr_element ? "true" : "false");
     printf ("     cp->is_compound_literal                            = %s \n",cp->is_compound_literal ? "true" : "false");
     printf ("     cp->is_result_of_constexpr_call                    = %s \n",cp->is_result_of_constexpr_call ? "true" : "false");
#if DO_IL_LOWERING
     printf ("     cp->has_been_prelowered                            = %s \n",cp->has_been_prelowered ? "true" : "false");
     printf ("     cp->vptr_has_been_lowered                          = %s \n",cp->vptr_has_been_lowered ? "true" : "false");
     printf ("     cp->empty_base_classes_have_been_removed           = %s \n",cp->empty_base_classes_have_been_removed ? "true" : "false");
#endif
     printf ("     cp->constant_for_base_class                        = %s \n",cp->constant_for_base_class ? "true" : "false");
     printf ("     cp->constant_for_base_class_from_constexpr_folding = %s \n",cp->constant_for_base_class_from_constexpr_folding ? "true" : "false");
     printf ("     cp->part_of_constexpr_master_expr                  = %s \n",cp->part_of_constexpr_master_expr ? "true" : "false");

#if LITERALS_AS_STRINGS_SUPPORT
  // DQ (12/30/2008): Added support for literands to be saved as strings (copied from 3.10, from Rama's work).
  // char float_value_lexeme[FLOAT_SUPPORT_MAX_SIZE_OF_FLOAT_LEXEMES];
  // char int_value_lexeme  [INT_SUPPORT_MAX_SIZE_OF_INT_LEXEMES];
#if 0
     for (int i = 0; i < FLOAT_SUPPORT_MAX_SIZE_OF_FLOAT_LEXEMES; i++)
        {
          printf ("     cp->float_value_lexeme[%d]                             = %d \n",i,cp->float_value_lexeme[i]);
        }
     for (int i = 0; i < FLOAT_SUPPORT_MAX_SIZE_OF_FLOAT_LEXEMES; i++)
        {
          printf ("     cp->float_value_lexeme[%d]                             = %c \n",i,cp->float_value_lexeme[i]);
        }
#endif
  // DQ (3/25/2017): Eliminate Clang warning: warning: comparison of array 'cp->float_value_lexeme' not equal to a null pointer is always true [-Wtautological-pointer-compare]
  // printf ("     cp->float_value_lexeme                             = %s \n",cp->float_value_lexeme != NULL ? cp->float_value_lexeme : "null");
     printf ("     cp->float_value_lexeme                             = %s \n",cp->float_value_lexeme);

     for (int i = 0; i < INT_SUPPORT_MAX_SIZE_OF_INT_LEXEMES; i++)
        {
          printf ("     cp->int_value_lexeme[%d]                          = %d \n",i,cp->int_value_lexeme[i]);
        }
     for (int i = 0; i < INT_SUPPORT_MAX_SIZE_OF_INT_LEXEMES; i++)
        {
          printf ("     cp->int_value_lexeme[%d]                          = %c \n",i,cp->int_value_lexeme[i]);
        }

  // DQ (3/25/2017): Eliminate Clang warning: warning: comparison of array 'cp->float_value_lexeme' not equal to a null pointer is always true [-Wtautological-pointer-compare]
  // printf ("     cp->int_value_lexeme                               = %s \n",cp->int_value_lexeme != NULL ? cp->int_value_lexeme : "null");
     printf ("     cp->int_value_lexeme                               = %s \n",cp->int_value_lexeme);
#endif

  // DQ (7/13/2020): Added value of kind to output.
     printf ("     cp->kind                                           = %d \n",(int)(cp->kind));

     printf ("     cp->kind                                           = %s \n",summarize_constant_kind_in_string(cp));

     switch(cp->kind)
        {
          case ck_integer:
#if 1
            // DQ (10/11/2012): Uncommented as part of debugging.
            // JJW (8/21/2008): This does not work on Cygwin
               printf ("     ck_integer: value = %d \n",(int)cp->variant.integer_value);
#endif
               break;

#if FIXED_POINT_ALLOWED
          case ck_fixed_point:
               printf ("     ck_fixed_point:  string.length = %f \n",(float)(cp->variant.fixed_point_value));
               break;
#endif

          case ck_string:
               printf ("     ck_string:  string.length = %d \n",(int)cp->variant.string.length);
               printf ("     ck_string:  string.value = %p \n",cp->variant.string.value);
            // I think this should work (should be a null terminated string!)
               if (cp->variant.string.value != NULL)
                  {
                    printf ("     ck_string:  string.value = NULL \n");
                  }
                 else
                  {
                    printf ("     ck_string:  string.value = %s \n",cp->variant.string.value);
                  }
#if DO_IL_LOWERING && ASSIGN_STRING_LITERAL_SEQUENCE_NUMBERS
               printf ("     ck_string:  string.length = %" PRIuPTR " \n",(size_t)cp->variant.string.sequence_number);
#endif
               printf ("     ck_string:  literal_kind = %d \n",(int)cp->variant.string.literal_kind);
               printf ("     ck_string:  literal_kind = %s \n",summarize_string_literal_kind(cp->variant.string.literal_kind));
               break;

       // DQ (8/27/2006): it is not documented but ck_imaginary shares a float!
          case ck_imaginary:
            // printf ("     ck_imaginary: value = %f \n",cp->variant.float_value);
               printf ("     ck_imaginary: value = %f \n",*((double*)(&(cp->variant.float_value))));
               break;
          case ck_float:
            // printf ("     ck_float: value = %f \n",cp->variant.float_value);
               printf ("     ck_float: value = %f \n",*((double*)(&(cp->variant.float_value))));
               break;

#if C99_IL_EXTENSIONS_SUPPORTED
          case ck_complex:
            // printf ("     ck_complex: real      value = %f \n",cp->variant.complex_value->real);
            // printf ("     ck_complex: imaginary value = %f \n",cp->variant.complex_value->imag);
               printf ("     ck_complex: real      value = %f \n",*((double*)(&(cp->variant.complex_value->real))));
               printf ("     ck_complex: imaginary value = %f \n",*((double*)(&(cp->variant.complex_value->imag))));
               break;
#endif /* C99_IL_EXTENSIONS_SUPPORTED */

          case ck_address:
             {
               printf ("     ck_address: address kind = %d \n",cp->variant.address.kind);
               switch(cp->variant.address.kind)
                  {
                    case abk_routine:
                         printf ("          ck_address: address of routine = %p \n",cp->variant.address.variant.routine);
#if 1
                      // DQ (6/11/2013): Added support for output of associated constant.
                         if (cp->variant.address.variant.routine != NULL)
                            {
                              printf ("In summarize_routine for constant(): case ck_address: case abk_routine: constant: Calling summarize_constant() \n");
                              summarize_routine(cp->variant.address.variant.routine);
                            }
#endif
                         break;
                    case abk_variable:
                         printf ("          ck_address: address of variable = %p \n",cp->variant.address.variant.variable);
                         break;
                    case abk_constant:
                         printf ("          ck_address: address of constant = %p \n",cp->variant.address.variant.constant);
                         break;
                    case abk_uuidof:
                         printf ("          ck_address: address of uuidof = %p \n",cp->variant.address.variant.type);
                         break;
                    case abk_label:
                         printf ("          ck_address: address of label = %p \n",cp->variant.address.variant.label);
                         break;
                    case abk_temporary:
                         printf ("          ck_address: address of temporary constant = %p \n",cp->variant.address.variant.constant);
                         break;
                    default:
                       {
                         printf ("Error: default reached in switch \n");
                         assert(false);
                       }
                  }
               printf ("     ck_address: address offset = %d \n",(int)cp->variant.address.offset);
               break;
             }

          case ck_ptr_to_member:
               printf ("     ck_ptr_to_member: casting_base_class = %p \n",cp->variant.ptr_to_member.casting_base_class);
#if RECORD_FORM_OF_NAME_REFERENCE
               printf ("     ck_ptr_to_member: name_reference     = %p \n",cp->variant.ptr_to_member.name_reference);
#endif
               printf ("     ck_ptr_to_member: cast_to_base     = %s \n",cp->variant.ptr_to_member.cast_to_base ? "true" : "false");
               printf ("     ck_ptr_to_member: is_function_ptr  = %s \n",cp->variant.ptr_to_member.is_function_ptr ? "true" : "false");
               if (cp->variant.ptr_to_member.is_function_ptr == true)
                    printf ("     ck_ptr_to_member: routine  = %p \n",cp->variant.ptr_to_member.variant.routine);
                 else
                    printf ("     ck_ptr_to_member: field    = %p \n",cp->variant.ptr_to_member.variant.field);
               break;

#if DO_IL_LOWERING && GENERATE_EH_TABLES && !DO_FULL_PORTABLE_EH_LOWERING
          case ck_stack_offset:
               printf ("     ck_stack_offset: variable = %d \n",cp->variant.stack_offset.variable);
               printf ("     ck_stack_offset: offset   = %d \n",cp->variant.stack_offset.offset);
               break;
#endif

          case ck_dynamic_init:
               printf ("     ck_dynamic_init: dynamic_init.prt = %p \n",cp->variant.dynamic_init.ptr);
               break;

          case ck_aggregate:
               printf ("     ck_aggregate: first_constant = %p \n",cp->variant.aggregate.first_constant);
               printf ("     ck_aggregate: last_constant = %p \n",cp->variant.aggregate.last_constant);

            // DQ (7/12/2020): Added more debug information.
               printf ("     ck_aggregate: has_dynamic_init_component = %s \n",cp->variant.aggregate.has_dynamic_init_component ? "true" : "false");
               break;

          case ck_init_repeat:
               printf ("     ck_init_repeat: constant = %p \n",cp->variant.init_repeat.constant);
               printf ("     ck_init_repeat: count    = %d \n",(int)cp->variant.init_repeat.count);
               break;

          case ck_template_param:
               printf ("     ck_template_param: kind (value)      = %d \n",cp->variant.template_param.kind);
               printf ("     ck_template_param: kind (string)     = %s \n",summarize_template_param_constant_kind_in_string(cp->variant.template_param.kind));
               printf ("     ck_template_param: is_qualified_name = %s \n",cp->variant.template_param.is_qualified_name ? "true" : "false");
               switch(cp->variant.template_param.kind)
                  {
                    case tpck_param:
                         printf ("     ck_template_param: coordinates.position  = %lu \n",(unsigned long)cp->variant.template_param.variant.coordinates.position);
                         printf ("     ck_template_param: coordinates.depth     = %lu \n",(unsigned long)cp->variant.template_param.variant.coordinates.depth);
                         break;

                    case tpck_expression:
                         printf ("     ck_template_param: expr                  = %p \n",cp->variant.template_param.variant.expr);
                         break;

                    case tpck_member:
                      // printf ("     ck_template_param: expr                  = %p \n",cp->variant.template_param.variant.expr);
                         printf ("     ck_template_param: member: (no variant fields) \n");
                         break;

                    case tpck_unknown_function:
                         printf ("     ck_template_param: conversion_type       = %p \n",cp->variant.template_param.variant.unknown_function.conversion_type);
                         printf ("     ck_template_param: symbol                = %p \n",cp->variant.template_param.variant.unknown_function.symbol);
                         printf ("     ck_template_param: opname_kind           = %d \n",cp->variant.template_param.variant.unknown_function.opname_kind);
                         break;

                    case tpck_address:
                         printf ("     ck_template_param: constant              = %p \n",cp->variant.template_param.variant.constant);
#if 1
                      // DQ (6/11/2013): Added support for output of associated constant.
                         if (cp->variant.template_param.variant.constant != NULL)
                            {
                              printf ("In summarize_constant for constant(): ck_template_param: constant: Calling summarize_constant() \n");
                              summarize_constant(cp->variant.template_param.variant.constant);
                            }
#endif
                         break;

                 // Added case tpck_typeid 
                    case tpck_typeid:
                    case tpck_sizeof:
                    case tpck_alignof:
                    case tpck_uuidof:
                         printf ("     ck_template_param: templ_sizeof.type     = %p \n",cp->variant.template_param.variant.templ_sizeof.type);
                         printf ("     ck_template_param: templ_sizeof.expr     = %p \n",cp->variant.template_param.variant.templ_sizeof.expr);
                         break;

                    case tpck_template_ref:
                         printf ("     ck_template_param: template_ref.con      = %p \n",cp->variant.template_param.variant.template_ref.con);
                         printf ("     ck_template_param: template_ref.arg_list = %p \n",cp->variant.template_param.variant.template_ref.arg_list);
                         break;
                    case tpck_noexcept:
                         printf ("     ck_template_param: tpck_sizeof.type     = %p \n",cp->variant.template_param.variant.templ_sizeof.type);
                         printf ("     ck_template_param: tpck_sizeof.expr     = %p \n",cp->variant.template_param.variant.templ_sizeof.expr);
                         printf ("     ck_template_param: tpck_sizeof.is_std_alignof = %s \n",cp->variant.template_param.variant.templ_sizeof.is_std_alignof ? "true" : "false");
#if PROTOTYPE_INSTANTIATIONS_IN_IL
                      // DQ (10/5/2016): This variable appears to have been moded to specific variants as part of changes to the  4.11 IR.
                         printf ("ROSE EDG 4.11 fixme: In summarize_constant(): case tpck_noexcept: tpck_sizeof.local_expr_ref\n");
#endif
                         break;
                    case tpck_destructor:
                         printf ("     ck_template_param: tpck_destructor.type     = %p \n",cp->variant.template_param.variant.destructor.type);
                         break;
                    case tpck_integer_pack:
                         printf ("     ck_template_param: tpck_integer_pack        = %p \n",cp->variant.template_param.variant.bound);
                         break;
                    default:
                       {
                         printf ("Default reached in switch in summarize_constant() cp->variant.template_param.kind = %d \n",cp->variant.template_param.kind);
                         ROSE_ASSERT(false);
                       }
                  }

            // printf ("     ck_template_param: kind  = %d \n",cp->variant.template_param.kind);
               printf ("     ck_template_param: count = %d \n",(int)cp->variant.init_repeat.count);
               break;

          case ck_designator:
            // DQ (7/13/2025): Note that the behavior is different depending on when is_field_designator and is_generic are TRUE and FALSE.
            // a_bit_field is_field_designator:1;  // TRUE if the designator is for a field.
            // a_bit_field is_generic:1;           // TRUE if the entity being designated has not been looked up (e.g., in template contexts). */

               printf ("cp->variant.designator.is_field_designator = %s \n",cp->variant.designator.is_field_designator == TRUE ? "true" : "false");
               printf ("cp->variant.designator.is_generic          = %s \n",cp->variant.designator.is_generic == TRUE ? "true" : "false");

               if (cp->variant.designator.is_generic == TRUE)
                  {
                    if (cp->variant.designator.is_field_designator == TRUE)
                       {
                      // a_const_char* field_name; // The name of the designated field.
                         printf ("     ck_designator: field_name    = %s \n",cp->variant.designator.variant.field_name);
                       }
                      else
                       {
                      // a_constant_ptr subscript; // A constant representing the subscript of the designated array element.  (Not currently used.) 
                         printf ("     ck_designator: subscript     = %p \n",cp->variant.designator.variant.subscript);
                       }
                  }
                 else
                  {
                    if (cp->variant.designator.is_field_designator == TRUE)
                       {
                         printf ("     ck_designator: field         = %p \n",cp->variant.designator.variant.field);
                       }
                      else
                       {
                         printf ("     ck_designator: array_element = %d \n",(int)cp->variant.designator.variant.array_element);
                       }
                  }

            // DQ (7/13/2025): Original code...
               printf ("     Older code: not relevant now that the EDG design has a union over: field, array_element, subscript, and field_name \n");
               printf ("     ck_designator: field         = %p \n",cp->variant.designator.variant.field);
               printf ("     ck_designator: array_element = %d \n",(int)cp->variant.designator.variant.array_element);

               break;
          case ck_void:
            // This is a C++14 specific constant.
               printf ("     ck_void: kind (void value)  \n");
               break;
          case ck_error:
            // This is a C++11 specific constant issue for test2017_01.C.
               printf ("     ck_error: kind (no fields)\n");
               break;

          default:
             {
               printf ("Error, default reached in switch within summarize_constant() \n");
               ROSE_ASSERT(false);
             }
        }

  // printf ("     cp-> = %s \n",cp-> ? "true" : "false");
  // printf ("     cp-> = %p \n",cp->);
  // printf ("************************************\n\n");
   }


#ifdef __cplusplus
char*
summarize_constant_in_string(a_constant *cp)
/*
Print a short version of the constant at *cp.
*/
   {
     char* defaultString = (char*)"<null input in summarize_constant_in_string>";

  // handle case of null pointer as input
     if (cp == NULL) return defaultString;

#if (VERSION_NUMBER_FOR_MACRO < 310)
  // DQ (12/30/2008): These are not in EDG 3.3, but in later versions of 3.x, and EDG 4.0
  // DQ (12/30/2008): Not available in new connection to EDG/Sage/ROSE translation.

     an_output_str_function_ptr previousFunction = octl.output_str;
     assert (previousFunction != NULL);
     octl.output_str = displayTreeStringFunction;

  // initialize the string to a null string
  // sprintf (displayTreeStringBuffer,"");
     displayTreeStringBuffer [0] = '\0';

  // now call the form_type function and accumulate the type name into the string
  // (note: summarize_type can't be a recursive call!)
     form_constant(cp, /*need_parens=*/FALSE, &octl);

  // restore the original pointer
     octl.output_str = previousFunction;
#endif

  // printf ("Inside of summarize_constant_in_string: displayTreeStringBuffer = %s",displayTreeStringBuffer);

  // Note that this modified the return type of this EDG function
     return displayTreeStringBuffer;
   }  /* summarize_constant_in_string */
#endif

char*
summarize_statement_in_string ( a_statement_ptr ptr )
/*
Display the indicated statement.
*/
   {
     char* s = NULL;
     char* defaultString = (char*)"<null input in summarize_statement_in_string>";

  // handle case of null pointer as input
     if (ptr == NULL)
        {
          printf ("Handling ptr == NULL in summarize_statement_in_string() \n");
          return defaultString;
        }

  // switch ( (a_statement_kind) ptr->kind )
     switch ( (a_statement_kind) (ptr->kind) )
        {
          case stmk_expr:
               s = (char*)"stmk_expr";
               break;

          case stmk_if:
               s = (char*)"stmk_if";
               break;

          case stmk_constexpr_if: /* C++17 constexpr if-then-else. */
               s = (char*)"stmk_constexpr_if";
               break;

          case stmk_while:
               s = (char*)"stmk_while";
               break;

          case stmk_goto:
               s = (char*)"stmk_goto";
               break;

          case stmk_label:
               s = (char*)"stmk_label";
               break;

          case stmk_return:
               s = (char*)"stmk_return";
               break;

          case stmk_block:
               s = (char*)"stmk_block";
               break;

          case stmk_end_test_while:
               s = (char*)"stmk_end_test_while";
               break;

          case stmk_for:
               s = (char*)"stmk_for";
               break;

       // DQ (10/18/2012): Added new case.
          case stmk_switch_case:
               s = (char*)"stmk_switch_case";
               break;

          case stmk_switch:
               s = (char*)"stmk_switch";
               break;

          case stmk_init:
               s = (char*)"stmk_init";
               break;

          case stmk_asm:
            /* Asm statement. */
               s = (char*)"stmk_asm";
               break;

#if ASM_FUNCTION_ALLOWED
          case stmk_asm_func_body:
            /* Statement representing an function body. */
               s = (char*)"stmk_asm_func_body";
               break;
#endif /* ASM_FUNCTION_ALLOWED */

          case stmk_try_block:
            /* Try block. */
               s = (char*)"stmk_try_block";
               break;

#if MICROSOFT_EXTENSIONS_ALLOWED
          case stmk_microsoft_try:
            /* Try block. */
               s = (char*)"stmk_microsoft_try";
               break;
#endif /* MICROSOFT_EXTENSIONS_ALLOWED */
// #if GENERATE_SOURCE_SEQUENCE_LISTS

          case stmk_decl:
            /* "Decl" pseudo-statement. */
            // See comments in EDG manual Appendix G (specifically G.1)
            // s = (char*)"stmk_decl (start of declaration region (no other data))";
               s = (char*)"stmk_decl";
               break;
// #endif

          case stmk_set_vla_size:
               s = (char*)"stmk_set_vla_size";
               break;

          case stmk_vla_decl:
               s = (char*)"stmk_vla_decl";
               break;
#if 0
       // DQ (12/30/2008): This case does not exist for EDG 3.10
          case stmk_vla_dealloc:
            /* Marks where a VLA object (a variable with a variable length array type) should be deallocated. */
               s = (char*)"stmk_vla_dealloc";
               break;
#endif

#if UPC_EXTENSIONS_ALLOWED
          case stmk_upc_notify:
            /* Notify statement (split barrier start) */
               s = (char*)"stmk_upc_notify";
               break;

          case stmk_upc_wait:
            /* Wait statement (split barrier end) */
               s = (char*)"stmk_upc_wait";
               break;

          case stmk_upc_barrier:
            /* Barrier statement (full barrier) */
               s = (char*)"stmk_upc_barrier";
               break;

          case stmk_upc_fence:
            /* Fence statement */
               s = (char*)"stmk_upc_fence";
               break;

          case stmk_upc_forall:
            /* Forall statement */
               s = (char*)"stmk_upc_forall";
               break;
#endif /* UPC_EXTENSIONS_ALLOWED */

#if REPRESENT_EMPTY_STATEMENTS_IN_IL
          case stmk_empty:
            /* Empty ("null") statement. (";" in C/C++) */
               s = (char*)"stmk_empty";
               break;
#endif /* REPRESENT_EMPTY_STATEMENTS_IN_IL */

#if GNU_EXTENSIONS_ALLOWED
          case stmk_stmt_expr_result:
            // A statement in a GNU statement expression producing the result 
            // value of that expression.  Always the last statement of its block.
               s = (char*)"stmk_stmt_expr_result";
               break;
#endif /* GNU_EXTENSIONS_ALLOWED */

     // DQ (1/27/2017): Added support for more C++11 constructs.
        case stmk_range_based_for:
            /* range_based_for statement */
               s = (char*)"stmk_range_based_for";
               break;

#if GNU_EXTENSIONS_ALLOWED
     // DQ (12/13/2017): Added support for another GNU constructs.
        case stmk_assigned_goto:
            /* range_based_for statement */
               s = (char*)"stmk_assigned_goto";
               break;
#endif

          case stmk_last:
            /* Last enum value */
               s = (char*)"stmk_last";
               break;



          default:
             {
               printf ("Error: default in switch \n");
               printf ("Error: ptr->kind = %d stmk_expr = %d stmk_last = %d \n",ptr->kind,stmk_expr,stmk_last);
            // s = (char*)"**BAD STATEMENT KIND**";
               ROSE_ASSERT (false);
             }
        }

     ROSE_ASSERT (s != NULL);

     return s;
   }  /* summarize_statement_in_string */

char*
summarize_base_type_kind_in_string( a_based_type_list_member_ptr ptr )
   {
     char* s = NULL;
     char* defaultString = (char*)"<null input in summarize_base_type_kind_in_string>";

  // handle case of null pointer as input
     if (ptr == NULL)
          return defaultString;

     switch (ptr->kind)
        {
          case btk_qualified:
               s = (char*)"btk_qualified";
               break;
          case btk_reference:
               s = (char*)"btk_reference";
               break;
          case btk_ptr_to_member:
               s = (char*)"btk_ptr_to_member";
               break;
          case btk_unqualified_array_type:
               s = (char*)"btk_unqualified_array_type";
               break;
          case btk_pointer:
               s = (char*)"btk_pointer";
               break;

          default:
              {
                printf ("default reached in switch summarize_base_type_kind_in_string \n");
                ROSE_ASSERT(false);
              }
        }

     ROSE_ASSERT(s != NULL);

     return s;
   }




char*
summarize_float_kind_in_string ( a_float_kind kind )
   {

     char* s = NULL;
     switch (kind)
        {
          case fk_float:
               s = (char*)"fk_float";
               break;
          case fk_double:
               s = (char*)"fk_double";
               break;
          case fk_long_double:
               s = (char*)"fk_long_double";
               break;
          case fk_last:
               s = (char*)"fk_last";
               break;

          default:
               s = (char*)"**BAD FLOAT KIND**";
        }  /* switch */

     assert(s != NULL);
     return s;
   }




void
summarize_type ( a_type *tp )
   {
     ROSE_ASSERT(tp != NULL);

     printf ("In summarize_type for type = %p \n",tp);

     printf ("     Output source_correspondence information: \n");
#ifdef __cplusplus
     summarize_source_correspondence(tp->source_corresp);
#endif
     printf ("     DONE: Output source_correspondence information: \n");
     printf ("     tp->next = %p \n",tp->next);

  // printf ("     Skipped a_based_type_list_member_ptr based_types; \n");
     a_based_type_list_member_ptr elementOfList = tp->based_types;
     printf ("     Types based on this one (a_based_type_list_member_ptr based_types): head of list = %p \n",elementOfList);
     int counter = 0;
     while (elementOfList != NULL)
        {
          printf ("          based type list element #%d \n",counter++);
       // ROSE_ASSERT (elementOfList->front_end_only == false);
       // printf ("          relationship to type = %s \n",summarize_base_type_kind_in_string(elementOfList));
          a_type *basedType = elementOfList->based_type;
       // ROSE_ASSERT(basedType != NULL);
          printf ("          basedType            = %p \n",basedType);
          elementOfList = elementOfList->next;
        }

  // printf ("     a_targ_size_t	size = %d \n",tp->size);
     printf ("     a_targ_size_t size         = %ld \n",(unsigned long) tp->size);

  // DQ (7/24/2014): Added support for output of alignment value.
  // printf ("     Skipped a_targ_alignment alignment; \n");
     printf ("     a_targ_alignment alignment = %ld \n",(unsigned long) tp->alignment);

     printf ("     tp->kind                   = %s \n",summarize_type_kind_in_string(tp));

     switch(tp->kind)
        {
       // DQ (7/30/2014): Added newer version of this case.
          case tk_array:
             {
               printf ("          array: element_type                                   = %p \n",tp->variant.array.element_type);
               printf ("          array: qualifiers                                     = %d \n",(int) tp->variant.array.qualifiers);
               printf ("          array: qualifiers (summary)                           = %s \n",summarize_qualifiers_in_string((size_t)(tp->variant.array.qualifiers)).c_str());
               printf ("          array: is_template_dependent_size_array               = %s \n",tp->variant.array.is_template_dependent_size_array ? "true" : "false");
               printf ("          array: is_variable_size_array                         = %s \n",tp->variant.array.is_variable_size_array ? "true" : "false");
               printf ("          array: is_vla                                         = %s \n",tp->variant.array.is_vla ? "true" : "false");
               printf ("          array: constant_bound_expr_in_local_expr_node_ref     = %s \n",tp->variant.array.constant_bound_expr_in_local_expr_node_ref ? "true" : "false");
               printf ("          array: dep_constant_bound_expr_in_local_expr_node_ref = %s \n",tp->variant.array.dep_constant_bound_expr_in_local_expr_node_ref ? "true" : "false");
               printf ("          array: has_assoc_vla_dimension                        = %s \n",tp->variant.array.has_assoc_vla_dimension ? "true" : "false");
               printf ("          array: bound_is_zero                                  = %s \n",tp->variant.array.bound_is_zero ? "true" : "false");
               printf ("          array: is_static                                      = %s \n",tp->variant.array.is_static ? "true" : "false");
#if UPC_EXTENSIONS_ALLOWED
               printf ("          array: is_threads_dimension                           = %s \n",tp->variant.array.is_threads_dimension ? "true" : "false");
#endif
               if (tp->variant.array.is_variable_size_array == false && tp->variant.array.is_template_dependent_size_array == false)
                  {
                    printf ("          array: number_of_elements                            = %" PRIuPTR " \n",(size_t)tp->variant.array.variant.number_of_elements);
                  }
                 else
                  {
                    if (tp->variant.array.is_variable_size_array == true)
                       {
                         printf ("          array: element_count_expr                        = %p \n",tp->variant.array.variant.element_count_expr);
                       }

                    if (tp->variant.array.is_template_dependent_size_array == true)
                       {
                         printf ("          array: element_count_constant                    = %p \n",tp->variant.array.variant.element_count_constant);
                       }
                  }

               printf ("          array: bound_constant                     = %p \n",tp->variant.array.bound_constant);

               printf ("Calling summarize_type from summarize_type: case tk_array ... \n");
               summarize_type(tp->variant.array.element_type);
               printf ("DONE: Calling summarize_type from summarize_type: case tk_array ... \n");

               break;
             }

          case tk_typeref:
             {
               printf ("          typedef: referenced type                         = %p \n",tp->variant.typeref.type);
               printf ("          typedef: extra_info                              = %p \n",tp->variant.typeref.extra_info);
               ROSE_ASSERT(tp->variant.typeref.extra_info != NULL);
               if (tp->variant.typeref.extra_info)
                  {
                 // summarize_typeref_type_supplement(tp->variant.typeref.extra_info);
                    printf ("          typedef: extra_info: template_arg_list           = %p \n",tp->variant.typeref.extra_info->template_arg_list);
                    printf ("          typedef: extra_info: assoc_template              = %p \n",tp->variant.typeref.extra_info->assoc_template);
                    printf ("          typedef: extra_info: expr                        = %p \n",tp->variant.typeref.extra_info->expr);
#if UPC_EXTENSIONS_ALLOWED
                    printf ("          typedef: extra_info: upc_block_size              = %d \n",(int)tp->variant.typeref.extra_info->upc_block_size);
#endif
                    printf ("          typedef: extra_info: proxy_class                 = %p \n",tp->variant.typeref.extra_info->proxy_class);
                    printf ("          typedef: extra_info: operator_type_arg           = %p \n",tp->variant.typeref.extra_info->operator_type_arg);
                    printf ("          typedef: extra_info: min_template_arguments      = %d \n",tp->variant.typeref.extra_info->min_template_arguments);
#if EXTRA_SOURCE_POSITIONS_IN_IL
                    printf ("          typedef: extra_info: type_id_range \n");
                    summarize_source_range(tp->variant.typeref.extra_info->type_id_range);
#endif
                  }
#if DO_IL_LOWERING
               printf ("          typedef: orig_type                               = %p \n",tp->variant.typeref.orig_type);
#endif /* DO_IL_LOWERING */
               printf ("          typedef: qualifiers                              = %d \n",(int)tp->variant.typeref.qualifiers);
               printf ("          typedef: qualifiers (summary)                    = %s \n",summarize_qualifiers_in_string((size_t)(tp->variant.typeref.qualifiers)).c_str());
               printf ("          typedef: predeclared                             = %s \n",tp->variant.typeref.predeclared ? "true" : "false");
#if NEAR_AND_FAR_ALLOWED
               bool explicit_memory_attribute_made_implicit = tp->variant.typeref.explicit_memory_attribute_made_implicit;
               printf ("          typedef: explicit_memory_attribute_made_implicit = %s \n",explicit_memory_attribute_made_implicit ? "true" : "false");
#endif
               bool has_variably_modified_type = tp->variant.typeref.has_variably_modified_type;
               printf ("          typedef: has_variably_modified_type              = %s \n",has_variably_modified_type ? "true" : "false");
#if BACK_END_IS_CP_GEN_BE
               printf ("          typedef: surrounding_name_linkage_state          = %d \n",(int)tp->variant.typeref.surrounding_name_linkage_state);
#endif /* BACK_END_IS_CP_GEN_BE */

// TODO EDG 6.5 defines tp->variant.typeref.kind in il_def.h at line 9204
//      it replaces the flags below
#if 0
               printf ("          typedef: is_decltype                             = %s \n",tp->variant.typeref.is_decltype ? "true" : "false");
               printf ("          typedef: is_deduced_decltype_auto                = %s \n",tp->variant.typeref.is_deduced_decltype_auto ? "true" : "false");
               printf ("          typedef: is_deduced_auto                         = %s \n",tp->variant.typeref.is_deduced_auto ? "true" : "false");
               printf ("          typedef: decltype_expr_not_parenthesized         = %s \n",tp->variant.typeref.decltype_expr_not_parenthesized ? "true" : "false");
               printf ("          typedef: is_underlying_type                      = %s \n",tp->variant.typeref.is_underlying_type ? "true" : "false");
#if GNU_EXTENSIONS_ALLOWED
               bool is_typeof = tp->variant.typeref.is_typeof;
               printf ("          typedef: is_typeof                               = %s \n",is_typeof ? "true" : "false");
               printf ("          typedef: is_typeof_with_type_operand             = %s \n",tp->variant.typeref.is_typeof_with_type_operand ? "true" : "false");
#endif
               printf ("          typedef: is_dependent_type_operator              = %s \n",tp->variant.typeref.is_dependent_type_operator ? "true" : "false");
               printf ("          typedef: for_type_attributes                     = %s \n",tp->variant.typeref.for_type_attributes ? "true" : "false");
               printf ("          typedef: is_alias                                = %s \n",tp->variant.typeref.is_alias ? "true" : "false");
               printf ("          typedef: is_template_alias                       = %s \n",tp->variant.typeref.is_template_alias ? "true" : "false");
               printf ("          typedef: is_nonreal                              = %s \n",tp->variant.typeref.is_nonreal ? "true" : "false");
               printf ("          typedef: is_prototype_instantiation              = %s \n",tp->variant.typeref.is_prototype_instantiation ? "true" : "false");
               printf ("          typedef: is_bases                                = %s \n",tp->variant.typeref.is_bases ? "true" : "false");
               printf ("          typedef: direct_bases                            = %s \n",tp->variant.typeref.direct_bases ? "true" : "false");
#endif
               printf ("Calling summarize_type from summarize_type: case tk_typeref ... \n");
               summarize_type(tp->variant.typeref.type);
               printf ("DONE: Calling summarize_type from summarize_type: case tk_typeref ... \n");
               break;
             }

       // DQ (10/4/2012): Added support for unions.
          case tk_union:

       // DQ (6/12/2011): Added suppport for output of debugging information on structs.
          case tk_struct:
          case tk_class:
             {
               printf ("          class: field_list                                            = %p \n",tp->variant.class_struct_union.field_list);
               printf ("          class: extra_info                                            = %p \n",tp->variant.class_struct_union.extra_info);

               a_class_type_supplement_ptr extra_info = tp->variant.class_struct_union.extra_info;
               if (extra_info != NULL)
                  {
                    printf ("             extra_info: base_classes                           = %p \n",extra_info->base_classes);
#if IA64_ABI
                    printf ("             extra_info: preorder_base_classes                  = %p \n",extra_info->preorder_base_classes);
                    printf ("             extra_info: primary_base_class                     = %p \n",extra_info->primary_base_class);
#endif /* IA64_ABI */
                    printf ("             extra_info: size_without_virtual_base_classes      = %ld \n",(unsigned long) extra_info->size_without_virtual_base_classes);
                    printf ("             extra_info: alignment_without_virtual_base_classes = %d \n",extra_info->alignment_without_virtual_base_classes);
                    printf ("             extra_info: highest_virtual_function_number        = %d \n",extra_info->highest_virtual_function_number);
#if DO_IL_LOWERING && IA64_ABI
                    printf ("             extra_info: next_negative_virtual_table_index      = %d \n",extra_info->next_negative_virtual_table_index);
                    printf ("             extra_info: first_vcall_offset_index               = %d \n",extra_info->first_vcall_offset_index);
                    printf ("             extra_info: vcall_offsets                          = %p \n",extra_info->vcall_offsets);
#endif /* DO_IL_LOWERING && IA64_ABI */
                    printf ("             extra_info: virtual_function_info_offset           = %ld \n",(unsigned long) extra_info->virtual_function_info_offset);
                    printf ("             extra_info: virtual_function_info_base_class       = %p \n",extra_info->virtual_function_info_base_class);
#if MICROSOFT_EXTENSIONS_ALLOWED
                    printf ("             extra_info: uuid_string                            = %p \n",extra_info->uuid_string);
#endif /* MICROSOFT_EXTENSIONS_ALLOWED */
#if DECL_MODIFIERS_IN_USE
                    printf ("             extra_info: decl_modifiers (enum value)            = %u \n",extra_info->decl_modifiers);
                    output_decl_modifier_enum_values(extra_info->decl_modifiers);
#endif /* DECL_MODIFIERS_IN_USE */
#if MICROSOFT_EXTENSIONS_ALLOWED
                    printf ("             extra_info: orig_type_kind                         = %d \n",extra_info->orig_type_kind);
                    printf ("             extra_info: inheritance_kind                       = %d \n",extra_info->inheritance_kind);
                    printf ("             extra_info: inheritance_kind_is_explicit           = %s \n",extra_info->inheritance_kind_is_explicit ? "true" : "false");

                 // DQ (7/21/2012): Added more field values (some specific to lambda expressions).
                    printf ("             extra_info: has_direct_property_or_event           = %s \n",extra_info->has_direct_property_or_event ? "true" : "false");
                    printf ("             extra_info: declared_assembly_visibility           = %" PRIuPTR " \n",(size_t)extra_info->declared_assembly_visibility);
                    printf ("             extra_info: assembly_visibility                    = %" PRIuPTR " \n",(size_t)extra_info->assembly_visibility);
                    printf ("             extra_info: cli_class_type_kind                    = %" PRIuPTR " \n",(size_t)extra_info->cli_class_type_kind);
                    printf ("             extra_info: is_hide_by_sig                         = %s \n",extra_info->is_hide_by_sig ? "true" : "false");
                    printf ("             extra_info: is_cli_array                           = %s \n",extra_info->is_cli_array ? "true" : "false");
                    printf ("             extra_info: is_cli_attribute                       = %s \n",extra_info->is_cli_attribute ? "true" : "false");
                    printf ("             extra_info: is_cppcx_write_only_array              = %s \n",extra_info->is_cppcx_write_only_array ? "true" : "false");
                    printf ("             extra_info: is_cppcx_box                           = %s \n",extra_info->is_cppcx_box ? "true" : "false");
                    printf ("             extra_info: is_partial                             = %s \n",extra_info->is_partial ? "true" : "false");
#endif /* MICROSOFT_EXTENSIONS_ALLOWED */
#if GNU_EXTENSIONS_ALLOWED
#if GNU_VISIBILITY_ATTRIBUTE_ALLOWED
                    printf ("             extra_info: ELF_visibility                         = %" PRIuPTR " \n",(size_t)extra_info->ELF_visibility);
#endif /* GNU_VISIBILITY_ATTRIBUTE_ALLOWED */
#endif /* GNU_EXTENSIONS_ALLOWED */


#if NEAR_AND_FAR_ALLOWED
                    printf ("             extra_info: qualifiers                             = %d \n",extra_info->qualifiers);
                    printf ("             extra_info: qualifiers (summary)                   = %s \n",summarize_qualifiers_in_string((size_t)(extra_info->qualifiers)).c_str());
#endif /* NEAR_AND_FAR_ALLOWED */
                  }

#if BACK_END_IS_CP_GEN_BE
                    printf ("             extra_info: surrounding_name_linkage_state         = %d \n",extra_info->surrounding_name_linkage_state);
#endif
#if DO_IL_LOWERING
                    printf ("             extra_info: compiler_generated                     = %s \n",extra_info->compiler_generated ? "true" : "false");
#endif /* DO_IL_LOWERING */


#if RECORD_HIDDEN_NAMES_IN_IL
                 // DQ (7/21/2012): Added more field values (some specific to lambda expressions).
                    printf ("             extra_info: hidden_names_processed                 = %s \n",extra_info->hidden_names_processed ? "true" : "false");
                    printf ("             extra_info: base_class_hiding_in_progress          = %s \n",extra_info->base_class_hiding_in_progress ? "true" : "false");
#endif /* RECORD_HIDDEN_NAMES_IN_IL */

                 // DQ (7/21/2012): Added more field values (some specific to lambda expressions).
                    printf ("             extra_info: is_lambda_closure_class                = %s \n",extra_info->is_lambda_closure_class ? "true" : "false");
                    printf ("             extra_info: is_initializer_list                    = %s \n",extra_info->is_initializer_list ? "true" : "false");
                    printf ("             extra_info: has_initializer_list_ctor              = %s \n",extra_info->has_initializer_list_ctor ? "true" : "false");
                    printf ("             extra_info: has_anonymous_union_member             = %s \n",extra_info->has_anonymous_union_member ? "true" : "false");

#if NEED_NAME_MANGLING
                 // DQ (7/21/2012): Added more field values (some specific to lambda expressions).
                 // DQ (10/5/2016): This variable appears to have been moved to specific variants as part of changes to the 4.11 IR.

                 // printf ("ROSE EDG 4.11 fixme: Unclear how this code is modified over the EDG 4.9 version of the code below! \n");
                 // ROSE_ASSERT(false);

                    printf ("             extra_info: defined_in_variable_initializer        = %s \n",extra_info->defined_in_variable_initializer ? "true" : "false");
                    printf ("             extra_info: defined_in_field_initializer           = %s \n",extra_info->defined_in_field_initializer ? "true" : "false");
#endif /* NEED_NAME_MANGLING */
                 // DQ (7/21/2012): Added more field values (some specific to lambda expressions).
                    printf ("             extra_info: named_in_inline_template_directive     = %s \n",extra_info->named_in_inline_template_directive ? "true" : "false");
#if USE_X86_64
                    printf ("             extra_info: is_va_list_tag                         = %s \n",extra_info->is_va_list_tag ? "true" : "false");
#endif /* USE_X86_64 */
                    printf ("             extra_info: defined_in_parent_class                = %s \n",extra_info->defined_in_parent_class ? "true" : "false");
                    printf ("             extra_info: anonymous_union_kind                   = %d \n",extra_info->anonymous_union_kind);
                    printf ("             extra_info: anonymous_union_field                  = %p \n",extra_info->anonymous_union_field);
                    printf ("             extra_info: befriending_classes                    = %p \n",extra_info->befriending_classes);
                    printf ("             extra_info: friend_routines                        = %p \n",extra_info->friend_routines);
                    printf ("             extra_info: friend_classes                         = %p \n",extra_info->friend_classes);
                    printf ("             extra_info: assoc_scope                            = %p \n",extra_info->assoc_scope);
#if 1
                 // DQ (3/17/2015): Adding support to optionally output the scope to find hidden types.
                    if (extra_info->assoc_scope != NULL)
                       {
                         printf ("                  START: calling summarize_scope(extra_info->assoc_scope): \n");
                         summarize_scope(extra_info->assoc_scope);
                         printf ("                  DONE: calling summarize_scope(extra_info->assoc_scope): \n");
                       }
#endif
                    printf ("             extra_info: assoc_template                         = %p \n",extra_info->assoc_template);
                    printf ("             extra_info: template_arg_list                      = %p \n",extra_info->template_arg_list);
                    if (extra_info->template_arg_list)
                       {
                         printf ("                  Ouput the template_arg_list \n");
                         summarize_template_arg_list(extra_info->template_arg_list);
                       }
                    printf ("             extra_info: partial_spec_template_arg_list         = %p \n",extra_info->partial_spec_template_arg_list);
                    if (extra_info->partial_spec_template_arg_list != NULL)
                       {
                         printf ("                  PARTIAL SPECIALIZATION: Output the partial_spec_template_arg_list \n");
                         summarize_template_arg_list(extra_info->partial_spec_template_arg_list);
                       }
#if MICROSOFT_EXTENSIONS_ALLOWED
                    printf ("             extra_info: partial_class_bodies       = %p \n",extra_info->partial_class_bodies);
#endif /* MICROSOFT_EXTENSIONS_ALLOWED */

#if NEW_CAN_BE_FOLDED_INTO_CTOR
                    printf ("             extra_info: assoc_operator_new_routine             = %p \n",extra_info->assoc_operator_new_routine);
#endif /* NEW_CAN_BE_FOLDED_INTO_CTOR */
#if DELETE_CAN_BE_FOLDED_INTO_DTOR
                    printf ("             extra_info: assoc_operator_delete_routine          = %p \n",extra_info->assoc_operator_delete_routine);
#endif /* DELETE_CAN_BE_FOLDED_INTO_DTOR */
#if DO_IL_LOWERING
                    printf ("             extra_info: virtual_function_table_var             = %p \n",extra_info->virtual_function_table_var);
#if IA64_ABI
                    printf ("             extra_info: virtual_table_table_var                = %p \n",extra_info->virtual_table_table_var);
#endif /* IA64_ABI */
                    printf ("             extra_info: type_as_subobject                      = %p \n",extra_info->type_as_subobject);
#if MICROSOFT_EXTENSIONS_ALLOWED
                    printf ("             extra_info: uuid_variable                          = %p \n",extra_info->uuid_variable);
#endif /* MICROSOFT_EXTENSIONS_ALLOWED */
#if PROMOTE_LOCAL_ENTITIES_TO_FILE_SCOPE
                    printf ("             extra_info: promoted_local_types                   = %p \n",extra_info->promoted_local_types);
#endif /* PROMOTE_LOCAL_ENTITIES_TO_FILE_SCOPE */
#if ABI_CHANGES_FOR_CONSTRUCTION_VTBLS
                    printf ("             extra_info: construction_vtbls                     = %p \n",extra_info->construction_vtbls);
#endif /* ABI_CHANGES_FOR_CONSTRUCTION_VTBLS */
#endif /* DO_IL_LOWERING */

                    printf ("             extra_info: min_template_arguments                 = %d \n",(int)extra_info->min_template_arguments);

#if NEED_NAME_MANGLING
                    if (extra_info->defined_in_variable_initializer == false && extra_info->defined_in_field_initializer == false)
                       {
                         printf ("             extra_info: lambda_parent.routine                  = %p \n",extra_info->lambda_parent.routine);
                       }

                    if (extra_info->defined_in_variable_initializer == true)
                       {
                         printf ("             extra_info: lambda_parent.variable                 = %p \n",extra_info->lambda_parent.variable);
                       }
                    if (extra_info->defined_in_field_initializer == true)
                       {
                         printf ("             extra_info: lambda_parent.field                    = %p \n",extra_info->lambda_parent.field);
                       }
#endif
                    printf ("             extra_info: hash_value                             = %" PRIuPTR " \n",(size_t)extra_info->hash_value);
#if MICROSOFT_EXTENSIONS_ALLOWED
                    printf ("             extra_info: corresponding_basic_type               = %p \n",extra_info->corresponding_basic_type);
                    printf ("             extra_info: assembly_scope_index                   = %" PRIuPTR " \n",(size_t)extra_info->assembly_scope_index);
                    printf ("             extra_info: metadata_type_def_token                = %" PRIuPTR " \n",(size_t)extra_info->metadata_type_def_token);
                    printf ("             extra_info: base_dispose_bool_routine              = %p \n",extra_info->base_dispose_bool_routine);
                    printf ("             extra_info: base_idisposable_dispose_routine       = %p \n",extra_info->base_idisposable_dispose_routine);
                    printf ("             extra_info: base_object_finalize_routine           = %p \n",extra_info->base_object_finalize_routine);
                    printf ("             extra_info: invocation_type                        = %p \n",extra_info->invocation_type);
                 // printf ("             extra_info: attribute_usage                        = %" PRIuPTR " \n",(size_t)extra_info->attribute_usage);
                    printf ("             extra_info: attribute_usage.valid_on               = %" PRIuPTR " \n",(size_t)extra_info->attribute_usage.valid_on);
                    printf ("             extra_info: attribute_usage.allow_multiple         = %s \n",(size_t)extra_info->attribute_usage.allow_multiple ? "true" : "false");
                    printf ("             extra_info: attribute_usage.inherited              = %s \n",(size_t)extra_info->attribute_usage.inherited ? "true" : "false");
#endif
#if PROTOTYPE_INSTANTIATIONS_IN_IL
                    printf ("             extra_info: proxy_of_type                          = %p \n",extra_info->proxy_of_type);
#endif

#if MICROSOFT_EXTENSIONS_ALLOWED
               bool is_interface = tp->variant.class_struct_union.is_interface;
               printf ("          class: is_interface                                          = %s \n",is_interface ? "true" : "false");
               printf ("          class: is_interface_like                                     = %s \n",tp->variant.class_struct_union.is_interface_like ? "true" : "false");
               printf ("          class: is_delegate_class                                     = %s \n",tp->variant.class_struct_union.is_delegate_class ? "true" : "false");
               printf ("          class: is_generic_definition                                 = %s \n",tp->variant.class_struct_union.is_generic_definition ? "true" : "false");
               printf ("          class: is_generic_instance                                   = %s \n",tp->variant.class_struct_union.is_generic_instance ? "true" : "false");
               printf ("          class: is_open_constructed_type                              = %s \n",tp->variant.class_struct_union.is_open_constructed_type ? "true" : "false");
               printf ("          class: is_generic_constraint                                 = %s \n",tp->variant.class_struct_union.is_generic_constraint ? "true" : "false");
               printf ("          class: is_hybrid_constraint                                  = %s \n",tp->variant.class_struct_union.is_hybrid_constraint ? "true" : "false");
               printf ("          class: any_interface_constraints                             = %s \n",tp->variant.class_struct_union.any_interface_constraints ? "true" : "false");
               printf ("          class: unconstrained                                         = %s \n",tp->variant.class_struct_union.unconstrained ? "true" : "false");
               printf ("          class: sealed                                                = %s \n",tp->variant.class_struct_union.sealed ? "true" : "false");
#if BACK_END_IS_CP_GEN_BE
               printf ("          class: defined_with_abstract_class_modifier                  = %s \n",tp->variant.class_struct_union.defined_with_abstract_class_modifier ? "true" : "false");
#endif
#endif
               printf ("          class: final                                                 = %s \n",tp->variant.class_struct_union.final ? "true" : "false");
               bool any_const_member = tp->variant.class_struct_union.any_const_member;
               printf ("          class: any_const_member                                      = %s \n",any_const_member ? "true" : "false");
               printf ("          class: any_volatile_member                                   = %s \n",tp->variant.class_struct_union.any_volatile_member ? "true" : "false");
               bool any_mutable_member = tp->variant.class_struct_union.any_mutable_member;
               printf ("          class: any_mutable_member                                    = %s \n",any_mutable_member ? "true" : "false");
               bool any_virtual_base_classes = tp->variant.class_struct_union.any_virtual_base_classes;
               printf ("          class: any_virtual_base_classes                              = %s \n",any_virtual_base_classes ? "true" : "false");
               bool abstract = tp->variant.class_struct_union.abstract;
               printf ("          class: abstract                                              = %s \n",abstract ? "true" : "false");
               bool any_virtual_functions = tp->variant.class_struct_union.any_virtual_functions;
               printf ("          class: any_virtual_functions                                 = %s \n",any_virtual_functions ? "true" : "false");
               bool any_pure_virtual_functions = tp->variant.class_struct_union.any_pure_virtual_functions;
               printf ("          class: any_pure_virtual_functions                            = %s \n",any_pure_virtual_functions ? "true" : "false");
               bool any_virtual_functions_including_in_base_classes = tp->variant.class_struct_union.any_virtual_functions_including_in_base_classes;
               printf ("          class: any_virtual_functions_including_in_base_classes       = %s \n",any_virtual_functions_including_in_base_classes ? "true" : "false");
               bool nested_class_defined_outside_of_parent = tp->variant.class_struct_union.nested_class_defined_outside_of_parent;
               printf ("          class: nested_class_defined_outside_of_parent                = %s \n",nested_class_defined_outside_of_parent ? "true" : "false");
               bool originally_unnamed = tp->variant.class_struct_union.originally_unnamed;
               printf ("          class: originally_unnamed                                    = %s \n",originally_unnamed ? "true" : "false");
// #if ALLOW_NONSTANDARD_ANONYMOUS_UNIONS
               bool is_nonstd_anonymous_union_type = tp->variant.class_struct_union.is_nonstd_anonymous_union_type;
               printf ("          class: is_nonstd_anonymous_union_type                        = %s \n",is_nonstd_anonymous_union_type ? "true" : "false");
// #endif
               bool is_template_class = tp->variant.class_struct_union.is_template_class;
               printf ("          class: is_template_class                                     = %s \n",is_template_class ? "true" : "false");
               bool is_nonreal_class = tp->variant.class_struct_union.is_nonreal_class;
               printf ("          class: is_nonreal_class                                      = %s \n",is_nonreal_class ? "true" : "false");
               printf ("          class: is_ms_instantiated_nonreal_class                      = %s \n",tp->variant.class_struct_union.is_ms_instantiated_nonreal_class ? "true" : "false");
               bool is_prototype_instantiation = tp->variant.class_struct_union.is_prototype_instantiation;
               printf ("          class: is_prototype_instantiation                            = %s \n",is_prototype_instantiation ? "true" : "false");
               bool is_specialized = tp->variant.class_struct_union.is_specialized;
               printf ("          class: is_specialized                                        = %s \n",is_specialized ? "true" : "false");
               bool specialized_with_old_syntax = tp->variant.class_struct_union.specialized_with_old_syntax;
               printf ("          class: specialized_with_old_syntax                           = %s \n",specialized_with_old_syntax ? "true" : "false");
               printf ("          class: is_in_class_specialization                            = %s \n",tp->variant.class_struct_union.is_in_class_specialization ? "true" : "false");
               printf ("          class: explicitly_instantiated                               = %s \n",tp->variant.class_struct_union.explicitly_instantiated ? "true" : "false");
               printf ("          class: do_not_instantiate                                    = %s \n",tp->variant.class_struct_union.do_not_instantiate ? "true" : "false");
               printf ("          class: proxy_class                                           = %s \n",tp->variant.class_struct_union.proxy_class ? "true" : "false");
#if MAINTAIN_NEEDED_FLAGS
               bool definition_needed = tp->variant.class_struct_union.definition_needed;
               printf ("          class: definition_needed                                     = %s \n",definition_needed ? "true" : "false");
               bool keep_definition_in_il = tp->variant.class_struct_union.keep_definition_in_il;
               printf ("          class: keep_definition_in_il                                 = %s \n",keep_definition_in_il ? "true" : "false");
#endif /* MAINTAIN_NEEDED_FLAGS */
               bool is_empty_class = tp->variant.class_struct_union.is_empty_class;
               printf ("          class: is_empty_class                                        = %s \n",is_empty_class ? "true" : "false");
               bool has_zero_init_component = tp->variant.class_struct_union.has_zero_init_component;
               printf ("          class: has_zero_init_component                               = %s \n",has_zero_init_component ? "true" : "false");
               bool contains_flexible_array_member = tp->variant.class_struct_union.contains_flexible_array_member;
               printf ("          class: contains_flexible_array_member                        = %s \n",contains_flexible_array_member ? "true" : "false");
#if GNU_EXTENSIONS_ALLOWED
               bool is_transparent = tp->variant.class_struct_union.is_transparent;
               printf ("          class: is_transparent                                        = %s \n",is_transparent ? "true" : "false");
#if USER_CONTROL_OF_STRUCT_PACKING
               bool is_packed = tp->variant.class_struct_union.is_packed;
               printf ("          class: is_packed                                             = %s \n",is_packed ? "true" : "false");
#endif /* USER_CONTROL_OF_STRUCT_PACKING */
#endif /* GNU_EXTENSIONS_ALLOWED */
               printf ("          class: has_operator_ampersand                                = %s \n",tp->variant.class_struct_union.has_operator_ampersand ? "true" : "false");
               printf ("          class: virtual_functions_marked_as_required                  = %s \n",tp->variant.class_struct_union.virtual_functions_marked_as_required ? "true" : "false");
               printf ("          class: copy_assignment_decl_suppressed                       = %s \n",tp->variant.class_struct_union.copy_assignment_decl_suppressed ? "true" : "false");
               printf ("          class: copy_ctor_decl_suppressed                             = %s \n",tp->variant.class_struct_union.copy_ctor_decl_suppressed ? "true" : "false");
               printf ("          class: default_ctor_decl_suppressed                          = %s \n",tp->variant.class_struct_union.default_ctor_decl_suppressed ? "true" : "false");
               printf ("          class: dtor_decl_suppressed                                  = %s \n",tp->variant.class_struct_union.dtor_decl_suppressed ? "true" : "false");
               printf ("          class: inc_class_used_in_array_type                          = %s \n",tp->variant.class_struct_union.inc_class_used_in_array_type ? "true" : "false");
#if USER_CONTROL_OF_STRUCT_PACKING
               int max_member_alignment = tp->variant.class_struct_union.max_member_alignment;
               printf ("          class: max_member_alignment                                  = %d \n",max_member_alignment);
#endif /* USER_CONTROL_OF_STRUCT_PACKING */

               break;
             }

          case tk_integer:
             {
               printf ("          integer: int_kind                                       = %d \n",tp->variant.integer.int_kind);
            // DQ (10/28/2015): Added summary of kind.
               printf ("          integer: int_kind (summary)                             = %s \n",summarize_integer_string(tp->variant.integer.int_kind).c_str());

               bool isExplicitlySigned = tp->variant.integer.explicitly_signed;
               printf ("          integer: explicitly_signed                              = %s \n",isExplicitlySigned ? "true" : "false");
#if MICROSOFT_EXTENSIONS_ALLOWED
               bool isMicrosoftSizedIntType = tp->variant.integer.microsoft_sized_int_type;
               printf ("          integer: microsoft_sized_int_type                       = %s \n",isMicrosoftSizedIntType ? "true" : "false");
#endif
               bool has_explicit_enum_base = tp->variant.integer.has_explicit_enum_base;
               printf ("          integer: has_explicit_enum_base                         = %s \n",has_explicit_enum_base ? "true" : "false");
               bool isEnumType = tp->variant.integer.enum_type;
               printf ("          integer: enum_type                                      = %s \n",isEnumType ? "true" : "false");
               bool isScopedEnum = tp->variant.integer.is_scoped_enum;
               printf ("          integer: is_scoped_enum                                 = %s \n",isScopedEnum ? "true" : "false");
#if DO_IL_LOWERING
               printf ("          integer: originally_a_scoped_enum                       = %s \n",tp->variant.integer.originally_a_scoped_enum ? "true" : "false");
#endif
#if GNU_EXTENSIONS_ALLOWED
               bool isPacked = tp->variant.integer.packed;
               printf ("          integer: packed                                         = %s \n",isPacked ? "true" : "false");
#endif
               bool isWchartType = tp->variant.integer.wchar_t_type;
               printf ("          integer: wchar_t_type                                   = %s \n",isWchartType ? "true" : "false");
               bool isChar16_tType = tp->variant.integer.char16_t_type;
               printf ("          integer: char16_t_type                                  = %s \n",isChar16_tType ? "true" : "false");
               bool isChar32_tType = tp->variant.integer.char32_t_type;
               printf ("          integer: char32_t_type                                  = %s \n",isChar32_tType ? "true" : "false");
               bool isBoolType = tp->variant.integer.bool_type;
               printf ("          integer: bool_type                                      = %s \n",isBoolType ? "true" : "false");
               bool isOriginallyUnnamed = tp->variant.integer.originally_unnamed;
               printf ("          integer: originally_unnamed                             = %s \n",isOriginallyUnnamed ? "true" : "false");

               printf ("          integer: is_template_enum                               = %s \n",tp->variant.integer.is_template_enum ? "true" : "false");
               printf ("          integer: is_prototype_instantiation                     = %s \n",tp->variant.integer.is_prototype_instantiation ? "true" : "false");
               printf ("          integer: is_nonreal                                     = %s \n",tp->variant.integer.is_nonreal ? "true" : "false");
               printf ("          integer: is_specialized                                 = %s \n",tp->variant.integer.is_specialized ? "true" : "false");
               if (tp->variant.integer.enum_type == true)
                  {
                    if (tp->variant.integer.is_scoped_enum == false)
                       {
                         printf ("          integer (enum_type == true && is_scoped_enum == false): enum_info.constant_list = %p \n",tp->variant.integer.enum_info.constant_list);
                       }
                      else
                       {
                         printf ("          integer (enum_type == true && is_scoped_enum == true): enum_info.assoc_scope    = %p \n",tp->variant.integer.enum_info.assoc_scope);
                       }
                  }
                 else
                  {
                    printf ("          integer (enum_type == false): enum_info.affiliated_type = %p \n",tp->variant.integer.enum_info.affiliated_type);
                  }
#if MICROSOFT_EXTENSIONS_ALLOWED
               if (tp->variant.integer.extra_info != NULL)
                  {
                    printf ("          integer: uuid_string = %p \n",tp->variant.integer.extra_info->uuid_string);
                  }
#endif
               printf ("          integer: integer.extra_info    = %p \n",tp->variant.integer.extra_info);
               if (tp->variant.integer.extra_info)
                  {
                    summarize_integer_type_supplement(tp->variant.integer.extra_info);
                  }
               break;
             }

          case tk_pointer:
             {
               printf ("          pointer: type                               = %p \n",tp->variant.pointer.type);
#if MICROSOFT_EXTENSIONS_ALLOWED
               printf ("          pointer: base_variable                      = %p \n",tp->variant.pointer.base_variable);
#endif
               bool isReference = tp->variant.pointer.is_reference;
               printf ("          pointer: is_reference                       = %s \n",isReference ? "true" : "false");
               printf ("          pointer: is_rvalue_reference                = %s \n",tp->variant.pointer.is_rvalue_reference ? "true" : "false");

#if MICROSOFT_EXTENSIONS_ALLOWED
               printf ("          pointer: is_handle         = %s \n",tp->variant.pointer.is_handle ? "true" : "false");
               printf ("          pointer: is_interior_ptr         = %s \n",tp->variant.pointer.is_interior_ptr ? "true" : "false");
               printf ("          pointer: is_pin_ptr         = %s \n",tp->variant.pointer.is_pin_ptr ? "true" : "false");
               printf ("          pointer: a_pointer_modifier_set modifiers (not output) \n");
#endif
               printf ("Calling summarize_type from summarize_type: case tk_pointer ... \n");
               summarize_type(tp->variant.pointer.type);
               printf ("DONE: Calling summarize_type from summarize_type: case tk_pointer ... \n");
               break;
             }

          case tk_routine:
             {
               printf ("          routine.return_type                         = %p \n",tp->variant.routine.return_type);
               printf ("          routine.extra_info                          = %p \n",tp->variant.routine.extra_info);

               printf ("Calling summarize_type from summarize_type: case tk_routine.return_type ... \n");
            // DQ (9/15/2015): Commented out the output of the return type info (to simplify the output in debugging).
#if 0
               summarize_type(tp->variant.routine.return_type);
#else
               printf ("Commented out the output of the return type info \n");
#endif
               printf ("DONE: Calling summarize_type from summarize_type: case tk_routine.return_type ... \n");

               ROSE_ASSERT(tp->variant.routine.extra_info != NULL);

            // DQ (9/11/2015): Output the type perameters:
               a_param_type_ptr parameter_type = tp->variant.routine.extra_info->param_type_list;
               printf ("Calling summarize_type from summarize_type: case tk_routine parameter_type list ... \n");
               while (parameter_type != NULL)
                  {
                    printf ("                    parameter_type               = %p \n",parameter_type);
                    summarize_param_type(parameter_type);
                    parameter_type = parameter_type->next;
                  }
               printf ("DONE: Calling summarize_type from summarize_type: case tk_routine parameter_type list ... \n");

               ROSE_ASSERT(tp->variant.routine.extra_info != NULL);
               summarize_routine_type_supplement (tp->variant.routine.extra_info);
               break;
             }

          case tk_ptr_to_member:
             {
               printf ("          ptr_to_member: class_of_which_a_member      = %p \n",tp->variant.ptr_to_member.class_of_which_a_member);
               printf ("          ptr_to_member: type                         = %p \n",tp->variant.ptr_to_member.type);
#if 1
               if (tp->variant.ptr_to_member.type != NULL)
                  {
                    printf ("Calling summarize_type(%p) \n",tp->variant.ptr_to_member.type);
                    printf ("**************** start of summarize_type() ******************* \n");
                    summarize_type(tp->variant.ptr_to_member.type);
                    printf ("***************** end of summarize_type() ******************** \n");
                  }
#endif
#if MICROSOFT_EXTENSIONS_ALLOWED
               printf ("          ptr_to_member: modifiers     = %" PRIuPTR " \n",(size_t)tp->variant.ptr_to_member.modifiers);
#endif
#if 0
               printf ("          ptr_to_member: name_reference               = %p \n",tp->variant.ptr_to_member.name_reference);
               printf ("          ptr_to_member: cast_to_base                 = %s \n",tp->variant.ptr_to_member.cast_to_base ? "true" : "false");
               printf ("          ptr_to_member: is_function_ptr              = %s \n",tp->variant.ptr_to_member.is_function_ptr ? "true: member function pointer" : "false: pointer to member data");

               bool isFunctionPtr = tp->variant.ptr_to_member.ptr_to_member.is_function_ptr;
               if (isFunctionPtr == true)
                  {
                    printf ("          ptr_to_member: routine  = %p \n",tp->variant.ptr_to_member.routine);
                  }
                 else
                  {
                    printf ("          ptr_to_member: field  = %p \n",tp->variant.ptr_to_member.ptr_to_member.field);
                  }

               if (tp->variant.pointer.ptr_to_member.casting_base_class != NULL)
                  {
                    printf ("Calling summarize_type from summarize_type: case tk_pointer ... \n");
                    summarize_type(tp->variant.ptr_to_member.casting_base_class);
                 // printf ("DONE: Calling summarize_type from summarize_type: case tk_pointer ... \n");
                  }
#endif
               break;
             }

       // DQ (8/25/2012): Added case to debug template details.
          case tk_template_param:
             {
               printf ("          template_param: kind                       = %s \n",summarize_template_param_type_kind_in_string(tp->variant.template_param.kind));
               printf ("          template_param: is_pack                    = %s \n",(tp->variant.template_param.is_pack                   == true) ? "true" : "false");
               printf ("          template_param: is_generic_param           = %s \n",(tp->variant.template_param.is_generic_param          == true) ? "true" : "false");
#if MICROSOFT_EXTENSIONS_ALLOWED
               printf ("          template_param: being_checked              = %s \n",(tp->variant.template_param.being_checked             == true) ? "true" : "false");
               printf ("          template_param: is_generic_function_param  = %s \n",(tp->variant.template_param.is_generic_function_param == true) ? "true" : "false");
#endif
               printf ("          template_param: extra_info                 = %p \n",tp->variant.template_param.extra_info);

            // extra data.
               if (tp->variant.template_param.extra_info != NULL)
                  {
                    printf ("               extra_info: class_type                = %p \n",tp->variant.template_param.extra_info->class_type);
                    printf ("               extra_info: orig_nested_type          = %p \n",tp->variant.template_param.extra_info->orig_nested_type);
                    printf ("               extra_info: template_symbol           = %p \n",tp->variant.template_param.extra_info->template_symbol);
#if MICROSOFT_EXTENSIONS_ALLOWED
                    printf ("               extra_info: generic_constraints       = %p \n",tp->variant.template_param.extra_info->generic_constraints);
                    printf ("               extra_info: generic_param_seq_number  = %d \n",tp->variant.template_param.extra_info->generic_param_seq_number);
#endif
                    printf ("               extra_info: coordinates.position      = %d \n",tp->variant.template_param.extra_info->coordinates.position);
                    printf ("               extra_info: coordinates.depth         = %d \n",tp->variant.template_param.extra_info->coordinates.depth);
                  }
               break;
             }
#if GNU_EXTENSIONS_ALLOWED && GNU_VECTOR_TYPES_ALLOWED
          case tk_vector:
             {
            // DQ (9/12/2016): We have now set GNU_VECTOR_TYPES_ALLOWED to true, and so we are seeing an error in this code for the first time.
            // printf ("          vector: element_type                       = %p \n",summarize_template_param_type_kind_in_string(tp->variant.vector.element_type));
               printf ("          vector: element_type                       = %p \n",tp->variant.vector.element_type);
            // printf ("          vector: size_constant                      = %p \n",summarize_template_param_type_kind_in_string(tp->variant.vector.size_constant));
               printf ("          vector: size_constant                      = %p \n",tp->variant.vector.size_constant);
               break;
             }
#endif

       // DQ (7/30/2014): Added support for nullptr type.
          case tk_nullptr:
             {
               break;
             }

       // DQ (1/22/2018): Added support for complex type.
          case tk_complex:
             {
               printf ("          complex: float_kind = %s \n",summarize_float_kind_in_string(tp->variant.float_kind));
               break;
             }

          default:
             {
               printf ("case not handled ... \n");
            // printf ("default reached in switch summarize_base_type_kind_in_string \n");
            // ROSE_ASSERT(false);
             }
        }


#ifdef __cplusplus

#if BACK_END_IS_SAGE_GEN_BE
     printf ("     SgType *sg_type_ptr = %p = %s \n",tp->sg_type_ptr,(tp->sg_type_ptr != NULL) ? tp->sg_type_ptr->sage_class_name() : "NULL");
#endif /* BACK_END_IS_SAGE_GEN_BE */
#endif
     printf ("     incomplete                                  = %s \n",(tp->incomplete == true) ? "true" : "false");
     printf ("     used_in_exception_or_rtti                   = %s \n",(tp->used_in_exception_or_rtti == true) ? "true" : "false");
     printf ("     declared_in_function_prototype              = %s \n",(tp->declared_in_function_prototype == true) ? "true" : "false");
     printf ("     is_instantiation_dependent                  = %s \n",(tp->is_instantiation_dependent == true) ? "true" : "false");
     printf ("     is_instantiation_dependent_cached           = %s \n",(tp->is_instantiation_dependent_cached == true) ? "true" : "false");

#if CFRONT_2_1_OBJECT_CODE_COMPATIBILITY
     printf ("     use_cfront_transitional_nested_type_name_mangling = %s \n",(tp->use_cfront_transitional_nested_type_name_mangling == true) ? "true" : "false");
#endif /* CFRONT_2_1_OBJECT_CODE_COMPATIBILITY */
#if BACK_END_IS_C_GEN_BE
#endif /* BACK_END_IS_C_GEN_BE */
// #if BACK_END_IS_CP_GEN_BE || BACK_END_IS_SAGE_GEN_BE
#if BACK_END_IS_C_GEN_BE
     printf ("     prototype_scope_types_if_any_promoted       = %s \n",(tp->prototype_scope_types_if_any_promoted == true) ? "true" : "false");
     printf ("     has_been_defined                            = %s \n",(tp->has_been_defined == true) ? "true" : "false");
     printf ("     typedef_pending                             = %s \n",(tp->typedef_pending == true) ? "true" : "false");
     printf ("     generated_as_empty_struct                   = %s \n",(tp->generated_as_empty_struct == true) ? "true" : "false");
#endif
     printf ("     typedef_definition_has_been_put_out         = %s \n",(tp->typedef_definition_has_been_put_out == true) ? "true" : "false");
#if BACK_END_IS_CP_GEN_BE
     printf ("     has_been_declared                           = %s \n",(tp->has_been_declared == true) ? "true" : "false");
     printf ("     definition_delayed                          = %s \n",(tp->definition_delayed == true) ? "true" : "false");
     printf ("     elaborated_type_specifier_needed            = %s \n",(tp->elaborated_type_specifier_needed == true) ? "true" : "false");
     printf ("     replace_by_generated_typedef                = %s \n",(tp->replace_by_generated_typedef == true) ? "true" : "false");
     printf ("     typedef_for_vacuous_dtor_call_put_out       = %s \n",(tp->typedef_for_vacuous_dtor_call_put_out == true) ? "true" : "false");
#endif
#if MICROSOFT_EXTENSIONS_ALLOWED
     printf ("     emit_microsoft_class_decl_modifiers         = %s \n",(tp->emit_microsoft_class_decl_modifiers == true) ? "true" : "false");
#endif
#if USER_CONTROL_OF_STRUCT_PACKING
     printf ("     alignment_set_explicitly                    = %s \n",(tp->alignment_set_explicitly == true) ? "true" : "false");
#endif
// #endif /* BACK_END_IS_CP_GEN_BE */

// DQ (5/22/2013): This is redundant.
// #if GNU_EXTENSIONS_ALLOWED || MICROSOFT_EXTENSIONS_ALLOWED
//      printf ("     alignment_set_explicitly                    = %s \n",(tp->alignment_set_explicitly == true) ? "true" : "false");
// #endif /* GNU_EXTENSIONS_ALLOWED || MICROSOFT_EXTENSIONS_ALLOWED */

#if GNU_EXTENSIONS_ALLOWED
     printf ("     variables_are_implicitly_referenced         = %s \n",(tp->variables_are_implicitly_referenced == true) ? "true" : "false");
     printf ("     may_alias                                   = %s \n",(tp->may_alias == true) ? "true" : "false");
#endif /* GNU_EXTENSIONS_ALLOWED */
#if MICROSOFT_EXTENSIONS_ALLOWED
     printf ("     has_microsoft_w64_specifier                 = %s \n",(tp->has_microsoft_w64_specifier == true) ? "true" : "false");
     printf ("     is_microsoft_intrinsic                      = %s \n",(tp->is_microsoft_intrinsic == true) ? "true" : "false");
#endif /* MICROSOFT_EXTENSIONS_ALLOWED */
#if GENERATE_SOURCE_SEQUENCE_LISTS
     printf ("     autonomous_primary_tag_decl                 = %s \n",(tp->autonomous_primary_tag_decl == true) ? "true" : "false");
#endif /* GENERATE_SOURCE_SEQUENCE_LISTS */
#if BACK_END_IS_SAGE_GEN_BE
     printf ("     substituted_for_template_param              = %s \n",(tp->substituted_for_template_param == true) ? "true" : "false");
#endif /* BACK_END_IS_SAGE_GEN_BE */
     printf ("     is_builtin_va_list                          = %s \n",(tp->is_builtin_va_list == true) ? "true" : "false");
     printf ("     is_builtin_va_list_from_cstdarg             = %s \n",(tp->is_builtin_va_list_from_cstdarg == true) ? "true" : "false");
#ifdef GUARD_MACRO_FOR_VA_LIST
     printf ("     va_list_guard_macro_was_defined             = %s \n",(tp->va_list_guard_macro_was_defined == true) ? "true" : "false");
#endif /* ifdef GUARD_MACRO_FOR_VA_LIST */
#ifdef GUARD_MACRO2_FOR_VA_LIST
     printf ("     va_list_guard_macro2_was_defined            = %s \n",(tp->va_list_guard_macro2_was_defined == true) ? "true" : "false");
#endif /* ifdef GUARD_MACRO2_FOR_VA_LIST */
#if DO_IL_LOWERING
#if ENSURE_LOWERED_TYPE_LIST_ORDERING
     printf ("     type_processed_for_ordering                 = %s \n",(tp->type_processed_for_ordering == true) ? "true" : "false");
     printf ("     type_processed_as_complete_for_ordering     = %s \n",(tp->type_processed_as_complete_for_ordering == true) ? "true" : "false");
#endif /* ENSURE_LOWERED_TYPE_LIST_ORDERING */
#if LOWER_VARIABLE_LENGTH_ARRAYS
     printf ("     visited_for_vla_lowering  g                 = %s \n",(tp->visited_for_vla_lowering == true) ? "true" : "false");
#endif /* LOWER_VARIABLE_LENGTH_ARRAYS */
     printf ("     a_variable_ptr typeinfo_var                 = %p \n",tp->typeinfo_var);
#endif /* DO_IL_LOWERING */

  // DQ (2/10/2018): Mark the end of the output from this function.
     printf ("Leaving summarize_type for type = %p \n",tp);
   }


// DQ (8/12/2014): Adding more C++11 support.
void
summarize_integer_type_supplement(an_integer_type_supplement_ptr ptr)
   {
     ROSE_ASSERT(ptr != NULL);

     printf ("          In summarize_integer_type_supplement for extra_info = %p \n",ptr);
     printf ("          extra_info->enumerator_list_seen                = %s \n",ptr->enumerator_list_seen ? "true" : "false");
#if GNU_EXTENSIONS_ALLOWED
     printf ("          extra_info->underlying_type_should_use_unsigned = %s \n",ptr->underlying_type_should_use_unsigned ? "true" : "false");
#endif
#if MICROSOFT_EXTENSIONS_ALLOWED
     printf ("          extra_info->declared_assembly_visibility        = %d \n",(int)ptr->declared_assembly_visibility);
     printf ("          extra_info->assembly_visibility                 = %s \n",(int)ptr->assembly_visibility ? "true" : "false");
     printf ("          extra_info->assembly_scope_index                = %d \n",(int)ptr->assembly_scope_index);
     printf ("          extra_info->metadata_type_def_token             = %d \n",(int)ptr->metadata_type_def_token);
     printf ("          extra_info->uuid_string                         = %s \n",ptr->uuid_string);
     printf ("          extra_info->boxed_type                          = %p \n",ptr->boxed_type);
#if DO_IL_LOWERING
     printf ("          extra_info->uuid_variable                       = %p \n",ptr->uuid_variable);
#endif
#endif
     printf ("          extra_info->base_type                           = %p \n",ptr->base_type);
     printf ("          extra_info->base_type_position: \n");
     summarize_source_position (ptr->base_type_position);
     printf ("          extra_info->assoc_template                      = %p \n",ptr->assoc_template);
   }


void
summarize_class ( a_type *type )
   {
     ROSE_ASSERT(type != NULL);
     ROSE_ASSERT ( type->kind == tk_class || type->kind == tk_struct || type->kind == tk_union );

     printf ("In summarize_class for type                                              = %p \n",type);

#if MICROSOFT_EXTENSIONS_ALLOWED
     printf ("   type->variant.class_struct_union.is_interface                         = %s \n",
          (type->variant.class_struct_union.is_interface == true) ? "true" : "false");
#endif
     printf ("   type->variant.class_struct_union.any_const_member                     = %s \n",
          (type->variant.class_struct_union.any_const_member == true) ? "true" : "false");
     printf ("   type->variant.class_struct_union.any_mutable_member                   = %s \n",
          (type->variant.class_struct_union.any_mutable_member == true) ? "true" : "false");
     printf ("   type->variant.class_struct_union.any_virtual_base_classes             = %s \n",
          (type->variant.class_struct_union.any_virtual_base_classes == true) ? "true" : "false");
     printf ("   type->variant.class_struct_union.abstract                             = %s \n",
          (type->variant.class_struct_union.abstract == true) ? "true" : "false");
     printf ("   type->variant.class_struct_union.any_virtual_functions                = %s \n",
          (type->variant.class_struct_union.any_virtual_functions == true) ? "true" : "false");
     printf ("   type->variant.class_struct_union.any_pure_virtual_functions           = %s \n",
          (type->variant.class_struct_union.any_pure_virtual_functions == true) ? "true" : "false");
     printf ("   type->variant.class_struct_union.any_virtual_functions_including_in_base_classes = %s \n",
          (type->variant.class_struct_union.any_virtual_functions_including_in_base_classes == true) ? "true" : "false");
     printf ("   type->variant.class_struct_union.nested_class_defined_outside_of_parent = %s \n",
          (type->variant.class_struct_union.nested_class_defined_outside_of_parent == true) ? "true" : "false");
     printf ("   type->variant.class_struct_union.originally_unnamed                   = %s \n",
          (type->variant.class_struct_union.originally_unnamed == true) ? "true" : "false");
#if ALLOW_NONSTANDARD_ANONYMOUS_UNIONS
     printf ("   type->variant.class_struct_union.is_nonstd_anonymous_union_type       = %s \n",
          (type->variant.class_struct_union.is_nonstd_anonymous_union_type == true) ? "true" : "false");
#endif
     printf ("   type->variant.class_struct_union.is_template_class                    = %s \n",
          (type->variant.class_struct_union.is_template_class == true) ? "true" : "false");
     printf ("   type->variant.class_struct_union.is_nonreal_class                     = %s \n",
          (type->variant.class_struct_union.is_nonreal_class == true) ? "true" : "false");
     printf ("   type->variant.class_struct_union.is_prototype_instantiation           = %s \n",
          (type->variant.class_struct_union.is_prototype_instantiation == true) ? "true" : "false");
     printf ("   type->variant.class_struct_union.is_specialized                       = %s \n",
          (type->variant.class_struct_union.is_specialized == true) ? "true" : "false");
     printf ("   type->variant.class_struct_union.specialized_with_old_syntax          = %s \n",
          (type->variant.class_struct_union.specialized_with_old_syntax == true) ? "true" : "false");
#if MAINTAIN_NEEDED_FLAGS
     printf ("   type->variant.class_struct_union.definition_needed                    = %s \n",
          (type->variant.class_struct_union.definition_needed == true) ? "true" : "false");
     printf ("   type->variant.class_struct_union.keep_definition_in_il                = %s \n",
          (type->variant.class_struct_union.keep_definition_in_il == true) ? "true" : "false");
#endif
     printf ("   type->variant.class_struct_union.is_empty_class                       = %s \n",
          (type->variant.class_struct_union.is_empty_class == true) ? "true" : "false");
     printf ("   type->variant.class_struct_union.has_zero_init_component              = %s \n",
          (type->variant.class_struct_union.has_zero_init_component == true) ? "true" : "false");
     printf ("   type->variant.class_struct_union.contains_flexible_array_member       = %s \n",
          (type->variant.class_struct_union.contains_flexible_array_member == true) ? "true" : "false");
#if GNU_EXTENSIONS_ALLOWED
     printf ("   type->variant.class_struct_union.is_transparent                       = %s \n",
          (type->variant.class_struct_union.is_transparent == true) ? "true" : "false");
#if USER_CONTROL_OF_STRUCT_PACKING
     printf ("   type->variant.class_struct_union.is_packed                            = %s \n",
          (type->variant.class_struct_union.is_packed == true) ? "true" : "false");
#endif
#endif
   }


void
output_decl_modifier_enum_values(a_decl_modifier_set decl_modifiers)
   {
#if DECL_MODIFIERS_IN_USE
  // DQ (5/24/2015): This is incorrect since the enum values represent individual bits of the decl_modifiers flag set that would be set.
  // printf ("             extra_info: decl_modifiers (enum name)             = %s \n",summarize_decl_modifier(extra_info->decl_modifiers));
     if (decl_modifiers & DM_NONE)
        {
          printf ("        decl_modifiers (enum name) DM_NONE \n");
        }
#if MICROSOFT_EXTENSIONS_ALLOWED
     if (decl_modifiers & DM_DLLIMPORT)
        {
          printf ("        decl_modifiers (enum name) DM_DLLIMPORT \n");
        }
     if (decl_modifiers & DM_DLLEXPORT)
        {
          printf ("        decl_modifiers (enum name) DM_DLLEXPORT \n");
        }
     if (decl_modifiers & DM_DLLFLAGS)
        {
          printf ("        decl_modifiers (enum name) DM_DLLFLAGS \n");
        }
#endif
#if MICROSOFT_EXTENSIONS_ALLOWED || THREAD_LOCAL_STORAGE_SPECIFIER_ALLOWED
     if (decl_modifiers & DM_THREAD)
        {
          printf ("        decl_modifiers (enum name) DM_THREAD \n");
        }
#endif
#if MICROSOFT_EXTENSIONS_ALLOWED
     if (decl_modifiers & DM_MICROSOFT_INLINE)
        {
          printf ("        decl_modifiers (enum name) DM_MICROSOFT_INLINE \n");
        }
     if (decl_modifiers & DM_FORCEINLINE)
        {
          printf ("        decl_modifiers (enum name) DM_FORCEINLINE \n");
        }
     if (decl_modifiers & DM_SELECTANY)
        {
          printf ("        decl_modifiers (enum name) DM_SELECTANY \n");
        }
     if (decl_modifiers & DM_NOVTABLE)
        {
          printf ("        decl_modifiers (enum name) DM_NOVTABLE \n");
        }
     if (decl_modifiers & DM_NOALIAS)
        {
          printf ("        decl_modifiers (enum name) DM_NOALIAS \n");
        }
     if (decl_modifiers & DM_RESTRICT)
        {
          printf ("        decl_modifiers (enum name) DM_RESTRICT \n");
        }
     if (decl_modifiers & DM_SAFEBUFFERS)
        {
          printf ("        decl_modifiers (enum name) DM_SAFEBUFFERS \n");
        }
#endif
#if SUN_EXTENSIONS_ALLOWED
     if (decl_modifiers & DM_GLOBAL_LINK_SCOPE)
        {
          printf ("        decl_modifiers (enum name) DM_GLOBAL_LINK_SCOPE \n");
        }
     if (decl_modifiers & DM_SYMBOLIC_LINK_SCOPE)
        {
          printf ("        decl_modifiers (enum name) DM_SYMBOLIC_LINK_SCOPE \n");
        }
     if (decl_modifiers & DM_HIDDEN_LINK_SCOPE)
        {
          printf ("        decl_modifiers (enum name) DM_HIDDEN_LINK_SCOPE \n");
        }
     if (decl_modifiers & DM_ANY_SUN_LINK_SCOPE)
        {
          printf ("        decl_modifiers (enum name) DM_ANY_SUN_LINK_SCOPE \n");
        }
#endif
#endif
   }


void
summarize_variable ( a_variable_ptr ptr )
   {
     ROSE_ASSERT(ptr != NULL);
     if (ptr->source_corresp.name != NULL)
          printf ("   Inside of summarize_variable (ptr = %p name = %s) \n",ptr,ptr->source_corresp.name);
       else
          printf ("   Inside of summarize_variable (ptr = %p name = %p) \n",ptr,ptr->source_corresp.name);

#if 0
  // DQ (12/30/2008): Removed to support new EDG/Sage/ROSE connection.
     Sg_File_Info* fileInfo_1 = generateFileInfo(ptr->source_corresp);
     fileInfo_1->display("summarize_variable");
     delete fileInfo_1;
#endif

#ifdef __cplusplus
     summarize_source_correspondence(ptr->source_corresp);
#endif
     printf ("   next                                = %p \n",ptr->next);
     printf ("   type                                = %p \n",ptr->type);
     printf ("   storage_class                       = %s \n",db_storage_class_names[ptr->storage_class]);
     printf ("   declared_storage_class              = %s \n",db_storage_class_names[ptr->declared_storage_class]);
#if DECL_MODIFIERS_IN_USE
     printf ("   decl_modifiers (enum value)         = %u \n",ptr->decl_modifiers);
  // printf ("   decl_modifiers (enum name)          = %s \n",summarize_decl_modifier(ptr->decl_modifiers));
     output_decl_modifier_enum_values(ptr->decl_modifiers);
#endif

#if GNU_EXTENSIONS_ALLOWED || REDEFINE_EXTNAME_PRAGMA_ENABLED
     if (ptr->asm_name_is_valid)
        {
          if (ptr->asm_name_or_reg.name != NULL)
             {
               printf ("   asm_name_or_reg.name      = %s \n",ptr->asm_name_or_reg.name);
             }
        }
       else
        {
#if NAMED_REGISTERS_ALLOWED
          if (ptr->has_named_register_storage_class)
             {
               printf ("   asm_name_or_reg.id        = %d \n",ptr->asm_name_or_reg.id);
             }
            else
             {
#if GNU_EXTENSIONS_ALLOWED
               printf ("   asm_name_or_reg.reg       = %d \n",ptr->asm_name_or_reg.reg);
#endif
             }
#endif
        }
#endif /* GNU_EXTENSIONS_ALLOWED || REDEFINE_EXTNAME_PRAGMA_ENABLED || NAMED_REGISTERS_ALLOWED */

#if GNU_EXTENSIONS_ALLOWED || MICROSOFT_EXTENSIONS_ALLOWED
     printf ("   alignment                           = %d \n",ptr->alignment);
#endif
#if GNU_EXTENSIONS_ALLOWED
#if GNU_INIT_PRIORITY_ATTRIBUTE_ALLOWED
     printf ("   init_priority                       = %hu \n",ptr->init_priority);
#endif
     printf ("   cleanup_routine                     = %p \n",ptr->cleanup_routine);
#if GNU_VISIBILITY_ATTRIBUTE_ALLOWED
     printf ("   ELF_visibility                      = %d \n",ptr->ELF_visibility);
#endif
     printf ("   is_weak                             = %s \n",ptr->is_weak ? "true" : "false");
     printf ("   is_weakref                          = %s \n",ptr->is_weakref ? "true" : "false");
     printf ("   is_gnu_alias                        = %s \n",ptr->is_gnu_alias ? "true" : "false");
     printf ("   is_not_common                       = %s \n",ptr->is_not_common ? "true" : "false");
     printf ("   is_common                           = %s \n",ptr->is_common ? "true" : "false");
#endif
#if GNU_EXTENSIONS_ALLOWED || REDEFINE_EXTNAME_PRAGMA_ENABLED
     printf ("   asm_name_is_valid                   = %s \n",ptr->asm_name_is_valid ? "true" : "false");
#endif
#if NAMED_REGISTERS_ALLOWED
     printf ("   has_named_register_storage_class    = %s \n",ptr->has_named_register_storage_class ? "true" : "false");
#endif
     printf ("   address_taken                       = %s \n",ptr->address_taken ? "true" : "false");
     printf ("   is_parameter                        = %s \n",ptr->is_parameter ? "true" : "false");
     printf ("   referenced_non_locally              = %s \n",ptr->referenced_non_locally ? "true" : "false");
     printf ("   modified_within_try_block           = %s \n",ptr->modified_within_try_block ? "true" : "false");
     printf ("   is_template_variable                = %s \n",ptr->is_template_variable ? "true" : "false");
     printf ("   is_prototype_instantiation          = %s \n",ptr->is_prototype_instantiation ? "true" : "false");
     printf ("   is_specialized                      = %s \n",ptr->is_specialized ? "true" : "false");
     printf ("   specialized_with_old_syntax         = %s \n",ptr->specialized_with_old_syntax ? "true" : "false");
     printf ("   explicit_instantiation              = %s \n",ptr->explicit_instantiation ? "true" : "false");
     printf ("   class_explicitly_instantiated       = %s \n",ptr->class_explicitly_instantiated ? "true" : "false");
     printf ("   explicit_do_not_instantiate         = %s \n",ptr->explicit_do_not_instantiate ? "true" : "false");
#if AUTOMATIC_TEMPLATE_INSTANTIATION
     printf ("   can_be_instantiated                 = %s \n",ptr->can_be_instantiated ? "true" : "false");
     printf ("   do_not_instantiate                  = %s \n",ptr->do_not_instantiate ? "true" : "false");
     printf ("   instance_required                   = %s \n",ptr->instance_required ? "true" : "false");
#endif
     printf ("   param_value_has_been_changed        = %s \n",ptr->param_value_has_been_changed ? "true" : "false");
#if MINIMAL_INLINING
     printf ("   param_used_as_lvalue                = %s \n",ptr->param_used_as_lvalue ? "true" : "false");
#endif
#if ENABLE_CUDA_SUPPORT
     printf ("   host                                = %s \n",ptr->host ? "true" : "false");
     printf ("   global                              = %s \n",ptr->global ? "true" : "false");
     printf ("   device                              = %s \n",ptr->device ? "true" : "false");
     printf ("   shared                              = %s \n",ptr->shared ? "true" : "false");
     printf ("   local                               = %s \n",ptr->local ? "true" : "false");
     printf ("   constant                            = %s \n",ptr->constant ? "true" : "false");
#endif /* ENABLE_CUDA_SUPPORT */
     printf ("   param_used_more_than_once           = %s \n",ptr->param_used_more_than_once ? "true" : "false");
     printf ("   is_handler_param                    = %s \n",ptr->is_handler_param ? "true" : "false");
     printf ("   is_this_parameter                   = %s \n",ptr->is_this_parameter ? "true" : "false");

  // DQ (6/10/2013): Modified to support EDG 4.7 frontend.
  // printf ("   is_partially_initialized            = %s \n",ptr->is_partially_initialized ? "true" : "false");

     printf ("   is_anonymous_parent_object          = %s \n",ptr->is_anonymous_parent_object ? "true" : "false");
     printf ("   is_member_constant                  = %s \n",ptr->is_member_constant ? "true" : "false");
     printf ("   is_constexpr                        = %s \n",ptr->is_constexpr ? "true" : "false");
     printf ("   superseded_external                 = %s \n",ptr->superseded_external ? "true" : "false");
     printf ("   has_variably_modified_type          = %s \n",ptr->has_variably_modified_type ? "true" : "false");
     printf ("   is_vla                              = %s \n",ptr->is_vla ? "true" : "false");
#if DO_IL_LOWERING
     printf ("   initialization_rewritten_as_assignment     = %s \n",ptr->initialization_rewritten_as_assignment ? "true" : "false");
#if MINIMAL_INLINING
     printf ("   is_temp_for_unmodified_inlined_param       = %s \n",ptr->is_temp_for_unmodified_inlined_param ? "true" : "false");
     printf ("   is_temp_for_constructor_this_inlined_param = %s \n",ptr->is_temp_for_constructor_this_inlined_param ? "true" : "false");
#endif /* MINIMAL_INLINING */
     printf ("   promoted_local_static_init          = %s \n",ptr->promoted_local_static_init ? "true" : "false");
     printf ("   promoted_local_static               = %s \n",ptr->promoted_local_static ? "true" : "false");
     printf ("   is_optional_vtable                  = %s \n",ptr->is_optional_vtable ? "true" : "false");
     printf ("   lowering_generated                  = %s \n",ptr->lowering_generated ? "true" : "false");
#endif /* DO_IL_LOWERING */
     printf ("   is_compound_literal                 = %s \n",ptr->is_compound_literal ? "true" : "false");
     printf ("   has_parenthesized_initializer       = %s \n",ptr->has_parenthesized_initializer ? "true" : "false");
     printf ("   has_direct_braced_initializer       = %s \n",ptr->has_direct_braced_initializer ? "true" : "false");
#if MICROSOFT_EXTENSIONS_ALLOWED || GNU_EXTENSIONS_ALLOWED
     printf ("   has_flexible_array_initializer      = %s \n",ptr->has_flexible_array_initializer ? "true" : "false");
#endif /* MICROSOFT_EXTENSIONS_ALLOWED || GNU_EXTENSIONS_ALLOWED */
     printf ("   declared_with_auto_type_specifier   = %s \n",ptr->declared_with_auto_type_specifier ? "true" : "false");
#if BACK_END_IS_CP_GEN_BE
     printf ("   definition_has_been_put_out         = %s \n",ptr->definition_has_been_put_out ? "true" : "false");
#endif /* BACK_END_IS_CP_GEN_BE */
     printf ("   declared_with_decltype_auto         = %s \n",ptr->declared_with_decltype_auto ? "true" : "false");

#if GENERATE_SOURCE_SEQUENCE_LISTS
     printf ("   embedded_source_sequence_entries    = %s \n",ptr->embedded_source_sequence_entries ? "true" : "false");
#endif /* GENERATE_SOURCE_SEQUENCE_LISTS */
     printf ("   declared_using_type_without_linkage = %s \n",ptr->declared_using_type_without_linkage ? "true" : "false");
     printf ("   is_parameter_pack                   = %s \n",ptr->is_parameter_pack ? "true" : "false");
     printf ("   is_pack_element                     = %s \n",ptr->is_pack_element ? "true" : "false");
#if MICROSOFT_EXTENSIONS_ALLOWED
     printf ("   is_initonly                         = %s \n",ptr->is_initonly ? "true" : "false");
#endif /* MICROSOFT_EXTENSIONS_ALLOWED */
     printf ("   is_enhanced_for_iterator            = %s \n",ptr->is_enhanced_for_iterator ? "true" : "false");
     printf ("   initializer_in_class                = %s \n",ptr->initializer_in_class ? "true" : "false");
     printf ("   constant_valued                     = %s \n",ptr->constant_valued ? "true" : "false");
     printf ("   is_thread_local                     = %s \n",ptr->is_thread_local ? "true" : "false");
     printf ("   init_kind                           = %d \n",ptr->init_kind);
     printf ("   initializer.constant                = %p \n",ptr->initializer.constant);
     printf ("   initializer.dynamic                 = %p \n",ptr->initializer.dynamic);
     printf ("   initializer.bound_expr              = %p \n",ptr->initializer.bound_expr);
#if EXTRA_SOURCE_POSITIONS_IN_IL
     printf ("   initializer_range: \n");
     summarize_source_range(ptr->initializer_range);
#endif

  // A list of entities defined in the initializer associated with this variable, if this is a static
  // data member.  Currently, this list only has C++11 closure types.
     printf ("   entities_defined_in_initializer     = %p \n",ptr->entities_defined_in_initializer);

#if 0
  // Refactor as a function instead.
#else
  // A list of tagged pointers to the entities declared by this statement.
     an_il_entity_list_entry_ptr entities = ptr->entities_defined_in_initializer;

  // DQ (10/16/2012): List the different kinds of entities declared in this statement.
     if (entities != NULL)
        {
          while (entities != NULL)
             {
               a_tagged_pointer entity = entities->entity;
               printf ("        --- entities = %p \n",entities);
               printf ("           --- entity.kind = %d = %s \n",entity.kind,summarize_entry_kind_in_string_kind((an_il_entry_kind)(entity.kind)));
               printf ("           --- entity.ptr  = %p \n",entity.ptr);
               entities = entities->next;
             }
        }
#endif

#if MICROSOFT_EXTENSIONS_ALLOWED
     printf ("   property_or_event_descr         = %p \n",ptr->property_or_event_descr);
#endif
     printf ("   template_info                   = %p \n",ptr->template_info);
#if GNU_EXTENSIONS_ALLOWED
     printf ("   section                         = %p \n",ptr->section);
     if (ptr->section != NULL)
          printf ("   section                    = %s \n",ptr->section);
     printf ("   aliased_variable                = %p \n",ptr->aliased_variable);
#endif
#if DO_IL_LOWERING && IA64_ABI
     printf ("   comdat_group        = %s \n",ptr->comdat_group != NULL ? ptr->comdat_group : "NULL");
#endif /* DO_IL_LOWERING && IA64_ABI */
#if DO_IL_LOWERING
     printf ("   vla_element_count_variable       = %p \n",ptr->vla_element_count_variable);
#endif
     printf ("   declared_type                   = %p \n",ptr->declared_type);
#if ONE_INSTANTIATION_PER_OBJECT
     printf ("   instantiation_needed_bit_number = %d \n",ptr->instantiation_needed_bit_number);
#endif /* ONE_INSTANTIATION_PER_OBJECT */
#if MINIMAL_INLINING
     printf ("   remapping_for_inlining          = %p \n",ptr->remapping_for_inlining);
#endif
#if SEPARATE_ROUTINES_FOR_FILE_SCOPE_DYNAMIC_INITS
     printf ("   dynamic_init_routine            = %p \n",ptr->dynamic_init_routine);
#endif
   }


void
summarize_routine ( a_routine_ptr ptr )
   {
     ROSE_ASSERT(ptr != NULL);
     printf ("   Inside of summarize_routine (ptr = %p) \n",ptr);

  // DQ (4/2/2013): This appears in the compilation of doxygen...
  // ROSE_ASSERT(ptr->source_corresp.name != NULL);
     if (ptr->source_corresp.name != NULL)
        {
          printf ("   Inside of summarize_routine (name = %s) \n",ptr->source_corresp.name);
        }
       else
        {
       // DQ (4/2/2013): This appears for C++ constructs such as: static struct { int foobar; } foo;
          printf ("   Inside of summarize_routine (name = NULL pointer) \n");
        }

#ifdef __cplusplus
     summarize_source_correspondence ( ptr->source_corresp );
#endif

#if 0
  // DQ (12/30/2008): Removed to support new EDG/Sage/ROSE connection.
     Sg_File_Info* fileInfo_1 = generateFileInfo(ptr->source_corresp);
     fileInfo_1->display("summarize_routine");
     delete fileInfo_1;
#endif

     printf ("   next                                    = %p \n",ptr->next);
     printf ("   type                                    = %p \n",(char*)(ptr->type));

     printf ("   NULL_region_number (assoc_scope)        = %d \n",NULL_region_number);
     printf ("   function_def_number                     = %d \n",ptr->function_def_number);
     printf ("   memory_region                           = %zu \n",(size_t)(ptr->memory_region));

     printf ("   storage_class                           = %s \n",db_storage_class_names[ptr->storage_class]);
#if GENERATE_SOURCE_SEQUENCE_LISTS
     printf ("   declared_storage_class                  = %s \n",db_storage_class_names[ptr->declared_storage_class]);
#endif
     printf ("   special_kind                            = %s \n",db_special_function_kinds[ptr->special_kind]);

     printf ("   storage_class (value)             = %u \n",(unsigned int)(ptr->storage_class));
     printf ("   storage_class (enum name)         = %s \n",db_storage_class_names[ptr->storage_class]);
     printf ("   special_kind                      = %s \n",db_special_function_kinds[ptr->special_kind]);

     a_special_function_kind kind = ptr->special_kind;
     switch(kind)
        {
          case sfk_none:
             {
            // Not a special function.
#if GNU_EXTENSIONS_ALLOWED
            // An enumerator indicating the kind of builtin function; bfk_none for an ordinary function.
#if BUILTIN_FUNCTIONS_ENABLED
               a_builtin_function_kind builtin_function_kind = ptr->variant.builtin_function_kind;
               printf ("   sfk_none: builtin_function_kind         = %d \n",builtin_function_kind);

            // The first entry is NULL.
               if (builtin_function_kind > 0)
                  {
                    printf ("   sfk_none: (table of builtin function names is not available after EDG 4.11) builtin_function_kind = %d \n",builtin_function_kind);
                  }
#endif
#endif
            // ROSE_ASSERT(!"sfk_none case not implemented");
               break;
             }

          case sfk_constructor:
          case sfk_destructor:
             {
            // A constructor: ctor_dtor
               string s = (kind == sfk_constructor) ? "sfk_constructor" : "sfk_destructor";
#if IA64_ABI && DO_IL_LOWERING
            // DQ (7/31/2014): I am not clear on what is wrong with this code.
               printf ("   %s: ptr->variant.ctor_dtor.alternate_entry_points = %p \n",s.c_str(),ptr->variant.ctor_dtor.alternate_entry_points);
            // printf ("   %s: ptr->variant.ctor_dtor.base_name_offset       = %d \n",s.c_str(),(int)ptr->variant.ctor_dtor.base_name_offset);
               printf ("   %s: ptr->variant.ctor_dtor.base_name_offset       = %zu \n",s.c_str(),ptr->variant.ctor_dtor.base_name_offset);
#else
               printf ("   case %s: (error: data members on available in EDG: output not handled) \n",s.c_str());
#endif
               break;
             }

          case sfk_conversion:
             {
            // A conversion operator function

            // DQ (12/5/2011): There is nothing to for for this case.

            // ROSE_ASSERT(!"sfk_conversion case not implemented");
               break;
             }

          case sfk_operator:
             {
            // Any other operator function

            // An enumerator indicating the kind of operator when the special function kind is sfk_operator; onk_none otherwise.
            // an_opname_kind opname_kind = ptr->variant.opname_kind;

               printf ("sfk_operator case not implemented \n");
               printf ("   opname_kind                       = %d \n",ptr->variant.opname_kind);

            // ROSE_ASSERT(!"sfk_operator case not implemented");
               break;
             }

          case sfk_udl_operator:
             {
            // Any other operator function

            // An enumerator indicating the kind of operator when the special function kind is sfk_operator; onk_none otherwise.
            // an_opname_kind opname_kind = ptr->variant.opname_kind;

               printf ("sfk_udl_operator case not implemented \n");
               printf ("   opname_kind                       = %d \n",ptr->variant.opname_kind);

            // ROSE_ASSERT(!"sfk_operator case not implemented");
               break;
             }
#if MICROSOFT_EXTENSIONS_ALLOWED

#if 0
       // sfk_property_get, sfk_property_set, sfk_event_add, sfk_event_remove, or sfk_event_raise
       // case sfk_property_get:
       // case sfk_property_set:
       // case sfk_event_add:
       // case sfk_event_remove:
       // case sfk_event_raise:
       // case sfk_static_constructor:
             {
            // A C++/CLI static constructor

            // When special_kind == sfk_property_get, sfk_property_set, sfk_event_add, sfk_event_remove, or sfk_event_raise.
            // Pointer to the description of the associated property or event.
            // a_property_or_event_descr_ptr property_or_event_descr;

               printf ("case (sfk_property_get, sfk_property_set, sfk_event_add, sfk_event_remove, or sfk_event_raise): ptr->variant.property_or_event_descr = %p \n",ptr->variant.property_or_event_descr);

               ROSE_ASSERT(!"sfk_static_constructor case not implemented");
               break;
             }
#endif
          case sfk_finalizer:
             {
            // A C++/CLI finalizer

               ROSE_ASSERT(!"sfk_finalizer case not implemented");
               break;
             }

       // DQ (8/14/2014): This is set equal to sfk_property_get, so we can't include both.
       // case sfk_first_accessor:
// #if !MICROSOFT_EXTENSIONS_ALLOWED
          case sfk_property_get:
// #endif
             {
            // A "get" accessor function of a C++/CLI property

               ROSE_ASSERT(!"sfk_property_get == sfk_first_accessor case not implemented");
               break;
             }

          case sfk_property_set:
             {
            // A "set" accessor function of a C++/CLI property

               ROSE_ASSERT(!"sfk_property_set case not implemented");
               break;
             }

          case sfk_event_add:
             {
            // An "add" accessor function of a C++/CLI event

               ROSE_ASSERT(!"sfk_event_add case not implemented");
               break;
             }

          case sfk_event_remove:
             {
            // A "remove" accessor function of a C++/CLI event

               ROSE_ASSERT(!"sfk_event_remove case not implemented");
               break;
             }

          case sfk_event_raise:
#if !MICROSOFT_EXTENSIONS_ALLOWED
          case sfk_last_accessor:
#endif
             {
            // A "raise" accessor function of a C++/CLI event

               ROSE_ASSERT(!"sfk_event_raise == sfk_last_accessor case not implemented");
               break;
             }
#endif

       // When special_kind == sfk_lambda_entry_point.
          case sfk_lambda_entry_point:
             {
               printf ("ptr->variant.lambda_call_operator = %p \n",ptr->variant.lambda_call_operator);
             }

          case sfk_last:
             {
            // Must be last
               ROSE_ASSERT(!"sfk_last case not implemented");
               break;
             }

          default:
             {
               printf ("kind not found for summarize_routine(ptr = %p) kind = %d = %s \n",ptr,kind,db_special_function_kinds[ptr->special_kind]);
               ROSE_ASSERT(false);
             }
        }

     printf ("   address_taken                           = %s \n",ptr->address_taken ? "true" : "false");
     printf ("   is_virtual                              = %s \n",ptr->is_virtual ? "true" : "false");

     printf ("   overrides_base_member                   = %s \n",ptr->overrides_base_member ? "true" : "false");
     printf ("   pure_virtual                            = %s \n",ptr->pure_virtual ? "true" : "false");
     printf ("   final                                   = %s \n",ptr->final ? "true" : "false");
     printf ("   override                                = %s \n",ptr->override ? "true" : "false");
#if MICROSOFT_EXTENSIONS_ALLOWED
     printf ("   abstract                                = %s \n",ptr->abstract ? "true" : "false");
     printf ("   sealed                                  = %s \n",ptr->sealed ? "true" : "false");
     printf ("   new_member                              = %s \n",ptr->new_member ? "true" : "false");
     printf ("   interface_slot                          = %s \n",ptr->interface_slot ? "true" : "false");
     printf ("   definition_cannot_be_generated          = %s \n",ptr->definition_cannot_be_generated ? "true" : "false");
#endif
     printf ("   covariant_return_virtual_override       = %s \n",ptr->covariant_return_virtual_override ? "true" : "false");
     printf ("   is_inline                               = %s \n",ptr->is_inline ? "true" : "false");
     printf ("   is_declared_constexpr                   = %s \n",ptr->is_declared_constexpr ? "true" : "false");
     printf ("   is_constexpr                            = %s \n",ptr->is_constexpr ? "true" : "false");
     printf ("   compiler_generated                      = %s \n",ptr->compiler_generated ? "true" : "false");
     printf ("   defined                                 = %s \n",ptr->defined ? "true" : "false");
     printf ("   called                                  = %s \n",ptr->called ? "true" : "false");
     printf ("   is_explicit_constructor                 = %s \n",ptr->is_explicit_constructor ? "true" : "false");
     printf ("   is_explicit_conversion_function         = %s \n",ptr->is_explicit_conversion_function ? "true" : "false");
     printf ("   is_trivial_default_constructor          = %s \n",ptr->is_trivial_default_constructor ? "true" : "false");
     printf ("   is_trivial_copy_function                = %s \n",ptr->is_trivial_copy_function ? "true" : "false");

     printf ("   is_trivial_destructor                   = %s \n",ptr->is_trivial_destructor ? "true" : "false");
     printf ("   is_initializer_list_ctor                = %s \n",ptr->is_initializer_list_ctor ? "true" : "false");
     printf ("   is_delegating_ctor                      = %s \n",ptr->is_delegating_ctor ? "true" : "false");
     printf ("   is_inheriting_ctor                      = %s \n",ptr->is_inheriting_ctor ? "true" : "false");

#if ASSIGNMENT_TO_THIS_ALLOWED
     printf ("   assignment_to_this_done                 = %s \n",ptr->assignment_to_this_done ? "true" : "false");
#endif
     printf ("   is_template_function                    = %s \n",ptr->is_template_function ? "true" : "false");
     printf ("   is_specialized                          = %s \n",ptr->is_specialized ? "true" : "false");
     printf ("   specialized_with_old_syntax             = %s \n",ptr->specialized_with_old_syntax ? "true" : "false");
     printf ("   is_prototype_instantiation              = %s \n",ptr->is_prototype_instantiation ? "true" : "false");
     printf ("   never_throws                            = %s \n",ptr->never_throws ? "true" : "false");
     printf ("   is_in_class_specialization              = %s \n",ptr->is_in_class_specialization ? "true" : "false");
     printf ("   explicit_instantiation                  = %s \n",ptr->explicit_instantiation ? "true" : "false");
     printf ("   class_explicitly_instantiated           = %s \n",ptr->class_explicitly_instantiated ? "true" : "false");
     printf ("   explicit_do_not_instantiate             = %s \n",ptr->explicit_do_not_instantiate ? "true" : "false");

#if GNU_EXTENSIONS_ALLOWED || MICROSOFT_EXTENSIONS_ALLOWED
     printf ("   never_inline                            = %s \n",ptr->never_inline ? "true" : "false");
#endif
#if GNU_NAKED_ATTRIBUTE_ALLOWED || MICROSOFT_EXTENSIONS_ALLOWED
     printf ("   is_naked                                = %s \n",ptr->is_naked ? "true" : "false");
#endif
#if MICROSOFT_EXTENSIONS_ALLOWED
     printf ("   declared_only_as_friend                 = %s \n",ptr->declared_only_as_friend ? "true" : "false");
     printf ("   explicit_extern_inline                  = %s \n",ptr->explicit_extern_inline ? "true" : "false");
     printf ("   direct_linkage_specifier_on_nondef_decl = %s \n",ptr->direct_linkage_specifier_on_nondef_decl ? "true" : "false");
     printf ("   is_reverse_conversion_function          = %s \n",ptr->is_reverse_conversion_function ? "true" : "false");
     printf ("   is_generic_definition                   = %s \n",ptr->is_generic_definition ? "true" : "false");
     printf ("   is_generic_instance                     = %s \n",ptr->is_generic_instance ? "true" : "false");
#endif
#if GNU_EXTENSIONS_ALLOWED
#if GNU_VISIBILITY_ATTRIBUTE_ALLOWED
     printf ("   ELF_visibility                          = %s \n",ptr->ELF_visibility ? "true" : "false");
#endif
     printf ("   is_initialization_routine               = %s \n",ptr->is_initialization_routine ? "true" : "false");
     printf ("   is_finalization_routine                 = %s \n",ptr->is_finalization_routine ? "true" : "false");

     printf ("   is_pure                                 = %s \n",ptr->is_pure ? "true" : "false");
     printf ("   is_weak                                 = %s \n",ptr->is_weak ? "true" : "false");
     printf ("   is_weakref                              = %s \n",ptr->is_weakref ? "true" : "false");
     printf ("   is_gnu_alias                            = %s \n",ptr->is_gnu_alias ? "true" : "false");
     printf ("   is_ifunc                                = %s \n",ptr->is_ifunc ? "true" : "false");
     printf ("   has_gnu_used_attribute                  = %s \n",ptr->has_gnu_used_attribute ? "true" : "false");

     printf ("   allocates_memory                        = %s \n",ptr->allocates_memory ? "true" : "false");
     printf ("   no_instrument_function                  = %s \n",ptr->no_instrument_function ? "true" : "false");
     printf ("   no_check_memory_usage                   = %s \n",ptr->no_check_memory_usage ? "true" : "false");

     printf ("   always_inline                           = %s \n",ptr->always_inline ? "true" : "false");
     printf ("   gnu_c89_inline                          = %s \n",ptr->gnu_c89_inline ? "true" : "false");
     printf ("   implicit_alias                          = %s \n",ptr->implicit_alias ? "true" : "false");
#if GNU_COMPLEX_EXTENSIONS_ALLOWED && LOWER_COMPLEX && BACK_END_IS_C_GEN_BE
     printf ("   builtin_using_complex_type              = %s \n",ptr->builtin_using_complex_type ? "true" : "false");
#endif
#endif
#if ENABLE_CUDA_SUPPORT
     printf ("   host                                    = %s \n",ptr->host ? "true" : "false");
     printf ("   device                                  = %s \n",ptr->device ? "true" : "false");
     printf ("   global                                  = %s \n",ptr->global ? "true" : "false");
#endif

#if AUTOMATIC_TEMPLATE_INSTANTIATION
     printf ("   can_be_instantiated                     = %s \n",ptr->can_be_instantiated ? "true" : "false");
     printf ("   do_not_instantiate                      = %s \n",ptr->do_not_instantiate ? "true" : "false");
     printf ("   instance_required                       = %s \n",ptr->instance_required ? "true" : "false");
#endif

     printf ("   contains_try_block                      = %s \n",ptr->contains_try_block ? "true" : "false");
     printf ("   contains_local_class_type               = %s \n",ptr->contains_local_class_type ? "true" : "false");
     printf ("   superseded_external                     = %s \n",ptr->superseded_external ? "true" : "false");
     printf ("   defined_in_friend_decl                  = %s \n",ptr->defined_in_friend_decl ? "true" : "false");
     printf ("   defined_outside_of_parent               = %s \n",ptr->defined_outside_of_parent ? "true" : "false");

#if DO_IL_LOWERING && MINIMAL_INLINING
     printf ("   inlinable                               = %s \n",ptr->inlinable ? "true" : "false");
#endif
#if MAINTAIN_NEEDED_FLAGS
  // printf ("   need_out_of_line_copy             = %s \n",ptr->need_out_of_line_copy ? "true" : "false");
  // DQ (12/30/2008): This case does not exist for EDG 3.10
     printf ("   definition_needed                       = %s \n",ptr->definition_needed ? "true" : "false");
     printf ("   keep_definition_in_il                   = %s \n",ptr->keep_definition_in_il ? "true" : "false");
#endif
     printf ("   expl_template_arg_list_used             = %s \n",ptr->expl_template_arg_list_used ? "true" : "false");
#if BACK_END_IS_CP_GEN_BE
  // This is serveal bits long!
     printf ("   surrounding_name_linkage_state          = %d \n",(int)ptr->surrounding_name_linkage_state);
     printf ("   definition_C_name_linkage_specified     = %s \n",ptr->definition_C_name_linkage_specified ? "true" : "false");
     printf ("   definition_has_direct_linkage_specifier = %s \n",ptr->definition_has_direct_linkage_specifier ? "true" : "false");
#if NONCLASS_TEMPLATE_INSTANTIATIONS_IN_SOURCE_SEQUENCE_LISTS
     printf ("   has_been_defined                        = %s \n",ptr->has_been_defined ? "true" : "false");
#endif /* NONCLASS_TEMPLATE_INSTANTIATIONS_IN_SOURCE_SEQUENCE_LISTS */
#endif
     printf ("   definition_for_inlining_only            = %s \n",ptr->definition_for_inlining_only ? "true" : "false");
#if INSTANTIATE_EXTERN_INLINE
     printf ("   inline_instance_required                = %s \n",ptr->inline_instance_required ? "true" : "false");
#endif
     printf ("   suppress_inline_body                    = %s \n",ptr->suppress_inline_body ? "true" : "false");
     printf ("   on_inline_function_list                 = %s \n",ptr->on_inline_function_list ? "true" : "false");

     printf ("   need_out_of_line_copy                   = %s \n",ptr->need_out_of_line_copy ? "true" : "false");

  // This is serveal bits long!
     printf ("   fp_contract                             = %d \n",(int)ptr->fp_contract);
     printf ("   fenv_access                             = %d \n",(int)ptr->fenv_access);
     printf ("   cx_limited_range                        = %d \n",(int)ptr->cx_limited_range);
#if FIXED_POINT_ALLOWED
     printf ("   fx_full_precision                       = %d \n",(int)ptr->fx_full_precision);
     printf ("   fx_fract_overflow                       = %d \n",(int)ptr->fx_fract_overflow);
     printf ("   fx_accum_overflow                       = %d \n",(int)ptr->fx_accum_overflow);
#endif /* FIXED_POINT_ALLOWED */

#if UPC_EXTENSIONS_ALLOWED
     printf ("   upc_access_method                       = %d \n",(int)ptr->upc_access_method);
#endif /* UPC_EXTENSIONS_ALLOWED */
#if GNU_EXTENSIONS_ALLOWED
     printf ("   contains_statement_expression           = %s \n",ptr->contains_statement_expression ? "true" : "false");
#endif /* GNU_EXTENSIONS_ALLOWED */
#if IA64_ABI
     printf ("   inline_in_class_definition              = %s \n",ptr->inline_in_class_definition ? "true" : "false");
#endif /* IA64_ABI */
#if DO_IL_LOWERING && IA64_ABI
     printf ("   use_comdat                              = %s \n",ptr->use_comdat ? "true" : "false");
     printf ("   ctor_dtor_kind                          = %d \n",(int)ptr->ctor_dtor_kind);
     printf ("   is_alias_entry                          = %s \n",ptr->is_alias_entry ? "true" : "false");
#endif /* DO_IL_LOWERING && IA64_ABI */
#if DO_IL_LOWERING
     printf ("   lowering_delayed_on_nested_function     = %s \n",ptr->lowering_delayed_on_nested_function ? "true" : "false");
     printf ("   has_no_effect                           = %s \n",ptr->has_no_effect ? "true" : "false");
#endif /* DO_IL_LOWERING */
#if PROMOTE_LOCAL_ENTITIES_TO_FILE_SCOPE
     printf ("   statics_have_been_promoted              = %s \n",ptr->statics_have_been_promoted ? "true" : "false");
#endif /* PROMOTE_LOCAL_ENTITIES_TO_FILE_SCOPE */
     printf ("   is_lambda_body                          = %s \n",ptr->is_lambda_body ? "true" : "false");
     printf ("   declared_using_type_without_linkage     = %s \n",ptr->declared_using_type_without_linkage ? "true" : "false");
     printf ("   is_defaulted                            = %s \n",ptr->is_defaulted ? "true" : "false");
     printf ("   is_deleted                              = %s \n",ptr->is_deleted ? "true" : "false");
     printf ("   contains_local_static_variable          = %s \n",ptr->contains_local_static_variable ? "true" : "false");
#if GENERATE_SOURCE_SEQUENCE_LISTS
     printf ("   embedded_source_sequence_entries        = %s \n",ptr->embedded_source_sequence_entries ? "true" : "false");
#endif /* GENERATE_SOURCE_SEQUENCE_LISTS */
     printf ("   considered_decider_function_at_some_point = %s \n",ptr->considered_decider_function_at_some_point ? "true" : "false");
     printf ("   is_raw_literal_operator                 = %s \n",ptr->is_raw_literal_operator ? "true" : "false");
#if USE_LAZY_INITIALIZATION_FOR_THREAD_LOCAL_VARIABLES
     printf ("   is_tls_init_alias                       = %s \n",ptr->is_tls_init_alias ? "true" : "false");
#endif /* USE_LAZY_INITIALIZATION_FOR_THREAD_LOCAL_VARIABLES */
     printf ("   is_tls_init_routine                     = %s \n",ptr->is_tls_init_routine ? "true" : "false");
#if GNU_INIT_PRIORITY_ATTRIBUTE_ALLOWED
     printf ("   has_ctor_priority                       = %s \n",ptr->has_ctor_priority ? "true" : "false");
     printf ("   has_dtor_priority                       = %s \n",ptr->has_dtor_priority ? "true" : "false");
#endif /* GNU_INIT_PRIORITY_ATTRIBUTE_ALLOWED */
     printf ("   has_deducible_return_type               = %s \n",ptr->has_deducible_return_type ? "true" : "false");
     printf ("   has_deduced_return_type                 = %s \n",ptr->has_deduced_return_type ? "true" : "false");

#if DECL_MODIFIERS_IN_USE
     printf ("   decl_modifiers (enum value)             = %u \n",ptr->decl_modifiers);
  // printf ("   decl_modifiers (enum name)              = %s \n",summarize_decl_modifier(ptr->decl_modifiers));
     output_decl_modifier_enum_values(ptr->decl_modifiers);
  // a_decl_modifier decl_modifiers;
#endif /* DECL_MODIFIERS_IN_USE */
     printf ("   number.virtual_function                 = %u \n",ptr->number.virtual_function);
  // a_virtual_function_number virtual_function_number;
#if MICROSOFT_EXTENSIONS_ALLOWED
     printf ("   overridden_functions                    = %s \n",ptr->overridden_functions ? "true" : "false");
  // an_il_entity_list_entry_ptr overridden_functions;
#endif /* MICROSOFT_EXTENSIONS_ALLOWED */
     printf ("   friends_or_originator.befriending_classes = %p \n",ptr->friends_or_originator.befriending_classes);
     if (ptr->friends_or_originator.befriending_classes != NULL)
        {
          a_class_list_entry_ptr tmp = ptr->friends_or_originator.befriending_classes;
          while(tmp != NULL)
             {
               printf ("      befriending class_type = %p \n",tmp->class_type);
               tmp = tmp->next;
             }
        }

     printf ("   template_arg_list                       = %p \n",ptr->template_arg_list);
     if (ptr->template_arg_list)
        {
          printf ("          Ouput the template_arg_list \n");
          summarize_template_arg_list(ptr->template_arg_list);
        }

     printf ("   assoc_template                          = %p \n",ptr->assoc_template);

#if GNU_EXTENSIONS_ALLOWED || REDEFINE_EXTNAME_PRAGMA_ENABLED
     if (ptr->gnu_extra_info != NULL)
        {
          printf ("   gnu_extra_info->section            = %p \n",ptr->gnu_extra_info->section);
          printf ("   gnu_extra_info->aliased_routine    = %p \n",ptr->gnu_extra_info->aliased_routine);
#if GNU_INIT_PRIORITY_ATTRIBUTE_ALLOWED
          printf ("   gnu_extra_info->ctor_priority      = %s \n",ptr->gnu_extra_info->ctor_priority ? "true" : "false");
          printf ("   gnu_extra_info->dtor_priority      = %s \n",ptr->gnu_extra_info->dtor_priority ? "true" : "false");
#endif /* GNU_INIT_PRIORITY_ATTRIBUTE_ALLOWED */
          printf ("   gnu_extra_info->asm_name           = %s \n",ptr->gnu_extra_info->asm_name);
        }
#endif /* GNU_EXTENSIONS_ALLOWED || REDEFINE_EXTNAME_PRAGMA_ENABLED */
     
#if GENERATE_SOURCE_SEQUENCE_LISTS
     printf ("   declared_type                           = %p \n",ptr->declared_type);
#endif /* GENERATE_SOURCE_SEQUENCE_LISTS */
#if DO_IL_LOWERING && ABI_CHANGES_FOR_COVARIANT_VIRTUAL_FUNC_RETURN
     printf ("   overriding_function_for_wrapper         = %p \n",ptr->overriding_function_for_wrapper);
     printf ("   overridden_function_for_wrapper         = %p \n",ptr->overridden_function_for_wrapper);
#if IA64_ABI
     printf ("   delta                                   = %" PRIuPTR " \n",(size_t)ptr->delta);
     printf ("   vcall_index                             = %" PRIuPTR " \n",(size_t)ptr->vcall_index);
     printf ("   return_delta                            = %" PRIuPTR " \n",(size_t)ptr->return_delta);
     printf ("   vbase_index                             = %" PRIuPTR " \n",(size_t)ptr->vbase_index);
#endif /* IA64_ABI */
#endif /* DO_IL_LOWERING && ABI_CHANGES_FOR_COVARIANT_VIRTUAL_FUNC_RETURN */
#if DO_IL_LOWERING && IA64_ABI
     printf ("   primary_ctor_or_dtor                    = %p \n",ptr->primary_ctor_or_dtor);
#endif /* DO_IL_LOWERING && IA64_ABI */
#if ONE_INSTANTIATION_PER_OBJECT
     printf ("   instantiation_needed_bit_number         = %lu \n",ptr->instantiation_needed_bit_number);
#endif /* ONE_INSTANTIATION_PER_OBJECT */
     printf ("   routine_fixup                           = %p \n",ptr->routine_fixup);
#if GNU_INIT_PRIORITY_ATTRIBUTE_ALLOWED && DO_IL_LOWERING
     printf ("   init_priority                           = %d \n",(int)ptr->init_priority);
#endif /* GNU_INIT_PRIORITY_ATTRIBUTE_ALLOWED && DO_IL_LOWERING */
     printf ("   generating_using_decl                   = %p \n",ptr->generating_using_decl);

#if 0
  // printf ("   inline_instance_required          = %s \n",ptr->inline_instance_required ? "true" : "false");
     printf ("   fenv_access                       = %s \n",ptr->fenv_access ? "true" : "false");
     printf ("   cx_limited_range                  = %s \n",ptr->cx_limited_range ? "true" : "false");
  // printf ("   upc_access_method                 = %s \n",ptr->upc_access_method ? "true" : "false");
  // printf ("   contains_statement_expression     = %s \n",ptr->contains_statement_expression ? "true" : "false");
  // printf ("   inline_in_class_definition        = %s \n",ptr->inline_in_class_definition ? "true" : "false");
  // printf ("   use_comdat                        = %s \n",ptr->use_comdat ? "true" : "false");
  // printf ("   ctor_dtor_kind                    = %s \n",ptr->ctor_dtor_kind ? "true" : "false");
  // printf ("   decl_modifiers                    = %d \n",ptr->decl_modifiers);
     printf ("   virtual_function_number           = %d \n",ptr->virtual_function_number);
  // printf ("   overridden_function               = %p \n",ptr->overridden_function);
     printf ("   befriending_classes               = %p \n",ptr->befriending_classes);
     printf ("   template_arg_list                 = %p \n",ptr->template_arg_list);
     printf ("   assoc_template                    = %p \n",ptr->assoc_template);
  // printf ("   section                           = %p \n",ptr->section);
  // printf ("   aliased_routine                   = %p \n",ptr->aliased_routine);
  // printf ("   asm_name                          = %p \n",ptr->asm_name);
     printf ("   declared_type                     = %p \n",ptr->declared_type);
  // printf ("   overriding_function_for_covariant_return_type = %p \n",ptr->overriding_function_for_covariant_return_type);
  // printf ("   overridden_function_for_covariant_return_type = %p \n",ptr->overridden_function_for_covariant_return_type);
  // printf ("   delta                             = %p \n",ptr->delta);
  // printf ("   vcall_index                       = %d \n",ptr->vcall_index);
  // printf ("   return_delta                      = %d \n",ptr->return_delta);
  // printf ("   vbase_index                       = %d \n",ptr->vbase_index);
  // printf ("   instantiation_needed_bit_number    = %d \n",ptr->instantiation_needed_bit_number);
     printf ("   routine_fixup                     = %p \n",ptr->routine_fixup);
  // printf ("   init_priority                     = %d \n",ptr->init_priority);
  // printf ("   is_fortran_entry                  = %s \n",ptr->is_fortran_entry ? "true" : "false");
  // printf ("   local_routine_scope               = %p \n",ptr->local_routine_scope);
  // printf ("   intrinsic_func_code               = %d \n",ptr->intrinsic_func_code);
#endif

     ROSE_ASSERT(ptr->type != NULL);
     printf ("At base of summarize_routine: testing type pointer (%p) \n",ptr->type);

  // DQ (11/26/2011): Make sure this is a valid pointer (sometimes set to values such as "0x3").
     if (ptr->type > (a_type_ptr)100)
        {
          printf ("Inside of summarize_routine: calling summarize_type(%p) \n",ptr->type);
          summarize_type(ptr->type);
        }

     printf ("Leaving summarize_routine \n");
   }


void
summarize_template ( a_template_ptr ptr )
   {
     ROSE_ASSERT (ptr != NULL);

     printf ("In summarize_template(ptr = %p) \n",ptr);

  // DQ (8/28/2012): Added output of source_corresp info.
     summarize_source_correspondence ( ptr->source_corresp );

#if 0
  // DQ (12/30/2008): Removed to support new EDG/Sage/ROSE connection.
  // Sg_File_Info *fileInfo = generateFileInfo(ptr->source_corresp);
     Sg_File_Info *fileInfo = generateFileInfo(ptr->source_corresp);
     ROSE_ASSERT (fileInfo != NULL);

     printf ("     File Info: file = %s line = %d col = %d \n",
          fileInfo->get_filename(),fileInfo->get_line(),fileInfo->get_col());

     delete fileInfo;
#endif

     printf ("     template kind = %s \n",summarize_template_kind_in_string(ptr->kind));

     switch (ptr->kind)
        {
          case templk_class:                                  /* Class template. */
               printf ("     case templk_class \n");
               break;
          case templk_member_class:   /* A class nested within a class template. */
               printf ("     case templk_member_class \n");
               break;
          case templk_function:               /* (Non-member) function template. */
               printf ("     case templk_function \n");
               break;
          case templk_member_function:              /* Member function template. */
               printf ("     case templk_member_function \n");
               break;
          case templk_static_data_member:        /* Static data member template. */
               printf ("     case templk_static_data_member \n");
               break;

       // DQ (7/19/2012): Added remaining case.
          case templk_template_template_param:        /* template_template_param template. */
               printf ("     case templk_template_template_param \n");
               break;

          default:
               printf ("     default reached in summarize_template() \n");
               ROSE_ASSERT (false);
        }

     printf ("     is_exported        = %s \n",ptr->is_exported ? "true" : "false");

  // DQ (2/28/2016): Added to output.
     printf ("     ignore_export      = %s \n",ptr->ignore_export      ? "true" : "false");
     printf ("     is_pack            = %s \n",ptr->is_pack            ? "true" : "false");
     printf ("     is_friend_template = %s \n",ptr->is_friend_template ? "true" : "false");
     printf ("     text               = %s \n",ptr->text);
#if EXTRA_SOURCE_POSITIONS_IN_IL
  // a_source_position export_position;
#if 0
  // DQ (12/30/2008): Removed to support new EDG/Sage/ROSE connection.
     printf ("export_position not output \n");
     Sg_File_Info* fileInfo_1 = generateFileInfo(ptr->export_position);
     fileInfo_1->display("export_position");
     delete fileInfo_1;

  // DQ (2/24/2007): Note that if the definition_range is a null range then this is a forward declaration (non-defining declaration)
  // a_source_range definition_range;
     printf ("definition_range not output \n");
     Sg_File_Info* fileInfo_2 = generateFileInfo(ptr->definition_range.start);
     fileInfo_2->display("definition_range: start");
     delete fileInfo_2;
     Sg_File_Info* fileInfo_3 = generateFileInfo(ptr->definition_range.end);
     fileInfo_3->display("definition_range: end");
     delete fileInfo_3;
#endif
#endif /* EXTRA_SOURCE_POSITIONS_IN_IL */

     printf ("     template_info = %p \n",ptr->template_info);

#if 1
  // DQ (4/24/2012): This appears to be a problem...failing in name...
     if (ptr->template_info != NULL)
        {
       // printf ("          template_info.pending_instantiations    = %lu \n",ptr->template_info->pending_instantiations);
       // DQ (12/29/2011): This is a uint32_t type for EDG 4.3, but provides a warning with EDG 3.3.
          printf ("          template_info.pending_instantiations    = %u \n",ptr->template_info->pending_instantiations);
          printf ("          template_info.pragmas_bound_to_template = %p \n",ptr->template_info->pragmas_bound_to_template);
          printf ("          template_info.befriending_classes       = %p \n",ptr->template_info->befriending_classes);

       // DQ (4/24/2012): This appears to be a problem...failing in name...
       // ROSE_ASSERT(ptr->template_info->name != NULL);
          if (ptr->template_info->name != NULL)
             {
            // DQ (2/28/2016): This is failing for test2016_17.C (boost example code, I think that this might be a stale pointer in EDG in some cases).
            // printf ("          template_info.name                      = %s \n",ptr->template_info->name);
            // string s = ptr->template_info->name;
            // printf ("          template_info.name                      = %s \n",s.c_str());
             }
            else
             {
               printf ("          template_info.name == NULL \n");
             }

          printf ("          Much more information not printed out here ... \n");
        }
#endif
  // DQ (2/28/2016): Added to output.
     printf ("     skipping a_template_param_coordinate coordinates \n");

  // DQ (7/19/2012): Skip a_template_param_coordinate (only relevant to the (internal) frontend, I think).

     printf ("     template_decl = %p \n",ptr->template_decl);
     if (ptr->template_decl != NULL)
        {
          printf ("          template_decl->parent     = %p \n",ptr->template_decl->parent);
          printf ("          template_decl->param_list = %p \n",ptr->template_decl->param_list);
          a_template_parameter_ptr temp_ptr = ptr->template_decl->param_list;
          if (temp_ptr != NULL)
             {
               printf ("          --- START: a_template_parameter_ptr temp_ptr = %p \n",temp_ptr);
               summarize_template_parameter(temp_ptr);
               printf ("          --- END: a_template_parameter_ptr temp_ptr = %p \n",temp_ptr);
               temp_ptr = temp_ptr->next;
             }

          printf ("          template_decl->scope      = %p \n",ptr->template_decl->scope);
       // printf ("          Source position of keyword template is available \n");
          printf ("          Template keyword source position: \n");
          summarize_source_position (ptr->template_decl->template_pos);

#if 0
  // DQ (12/30/2008): Removed to support new EDG/Sage/ROSE connection.
          Sg_File_Info *fileInfo = generateFileInfo(ptr->template_decl->template_pos);
          printf ("          File Info from Template Declaration: file = %s line = %d col = %d \n",
               fileInfo->get_filename(),fileInfo->get_line(),fileInfo->get_col());
          delete fileInfo;
#endif
        }

  // output prototype_instantiation
     switch (ptr->kind)
        {
          case templk_class:                                  /* Class template. */
               printf ("     prototype_instantiation.type = %p \n",ptr->prototype_instantiation.type);
               break;
          case templk_member_class:   /* A class nested within a class template. */
               printf ("     prototype_instantiation.type = %p \n",ptr->prototype_instantiation.type);
               break;
          case templk_function:               /* (Non-member) function template. */
               printf ("     prototype_instantiation.routine = %p \n",ptr->prototype_instantiation.routine);
               break;
          case templk_member_function:              /* Member function template. */
               printf ("     prototype_instantiation.routine = %p \n",ptr->prototype_instantiation.routine);
               break;
          case templk_static_data_member:        /* Static data member template. */
               printf ("     prototype_instantiation.variable = %p \n",ptr->prototype_instantiation.variable);
               break;

       // DQ (7/19/2012): Added remaining case (I think the variable is the relevant field, but I'm not certain).
          case templk_template_template_param:        /* template_template_param template. */
            // printf ("     prototype_instantiation.variable = %p \n",ptr->prototype_instantiation.variable);
               printf ("     Nothing relevant for templk_template_template_param in union prototype_instantiation \n");
               break;

          default:
               printf ("     default reached in summarize_template() \n");
               ROSE_ASSERT (false);
        }

     printf ("     canonical_template  = %p \n",ptr->canonical_template);
     printf ("     definition_template = %p \n",ptr->definition_template);

     if ( (ptr != ptr->definition_template) && (ptr->definition_template != NULL) ) {
          summarize_template(ptr->definition_template);
     }

     printf ("     prototype_template  = %p \n",ptr->prototype_template);
     printf ("     next = %p \n",ptr->next);

  // DQ (2/28/2016): Added to output.
#if BACK_END_IS_CP_GEN_BE
#if USER_CONTROL_OF_STRUCT_PACKING
     printf ("     skipping output of: a_targ_alignmen final_alignment \n");
#endif
     printf ("     min_template_arguments = %d \n",(int)ptr->min_template_arguments);
#endif
   }

char*
summarize_template_parameter_kind_in_string ( a_template_parameter_ptr ptr )
   {
     char* s = NULL;
     ROSE_ASSERT(ptr != NULL);

     switch(ptr->kind)
        {
          case tpk_error:
             {
               s = (char*)"tpk_error";
               break;
             }

          case tpk_type:
             {
               s = (char*)"tpk_type";
               break;
             }

          case tpk_nontype:
             {
               s = (char*)"tpk_nontype";
               break;
             }

          case tpk_template:
             {
               s = (char*)"tpk_template";
               break;
             }

          default:
             {
               printf ("default reached ... \n");
               ROSE_ASSERT(false);
             }
        }

     return s;
   }

void
summarize_template_parameter ( a_template_parameter_ptr ptr )
   {
     ROSE_ASSERT(ptr != NULL);

     summarize_source_correspondence(ptr->source_corresp);

     printf ("     next                                                 = %p \n",ptr->next);
     printf ("     is_pack                                              = %s \n",ptr->is_pack ? "true" : "false");
     printf ("     kind                                                 = %s \n",summarize_template_parameter_kind_in_string(ptr));

     switch(ptr->kind)
        {
          case tpk_error:
             {
               printf ("case tpk_error \n");
               break;
             }

          case tpk_type:
             {
               printf ("     kind == tpk_type: ptr                                = %p \n",ptr->variant.type.ptr);
               printf ("     kind == tpk_type: default_arg_type                   = %p \n",ptr->variant.type.default_arg_type);
               break;
             }

          case tpk_nontype:
             {
               printf ("     kind == tpk_nontype: constant                        = %p \n",ptr->variant.nontype.constant);
               printf ("     kind == tpk_nontype: default_arg_constant            = %p \n",ptr->variant.nontype.default_arg_constant);
               break;
             }

          case tpk_template:
             {
               printf ("     kind == tpk_template: class_template                 = %p \n",ptr->variant.templ.class_template);
               printf ("     kind == tpk_template: default_arg_template           = %p \n",ptr->variant.templ.default_arg_template);
               break;
             }

          default:
             {
               printf ("default reached ... \n");
               ROSE_ASSERT(false);
             }
        }
   }


void
summarize_src_seq_secondary_decl(a_src_seq_secondary_decl_ptr sec_decl)
   {
     if (sec_decl != NULL)
        {
       // DQ (6/20/2013): Added output of source_position.
          summarize_source_position (sec_decl->decl_position);

#if EXTRA_SOURCE_POSITIONS_IN_IL
          printf ("decl_pos_info                       = %p \n",sec_decl->decl_pos_info);
          if (sec_decl->decl_pos_info != NULL)
             {
            // DQ (6/20/2013): Added output of source_position.
               summarize_decl_pos_info (sec_decl->decl_pos_info);
             }
#endif
          printf ("entity.kind                         = %d \n",sec_decl->entity.kind);
          printf ("entity.ptr                          = %p \n",sec_decl->entity.ptr);
          printf ("declared_type                       = %p \n",sec_decl->declared_type);
#if RECORD_FORM_OF_NAME_REFERENCE
          printf ("name_reference                      = %p \n",sec_decl->name_reference);
#endif
          printf ("attributes  - - - - - - - - - - - - = %p \n",sec_decl->attributes);
          an_attribute_ptr attribute = sec_decl->attributes;
          while (attribute != NULL)
             {
               summarize_attribute(attribute);
               attribute = attribute->next;
             }

       // DQ (6/20/2013): Need to output the a_storage_class declared_storage_class;

          printf ("autonomous_tag_decl                 = %s \n",(sec_decl->autonomous_tag_decl == true) ? "true" : "false");
          printf ("embedded_source_sequence_entries    = %s \n",(sec_decl->embedded_source_sequence_entries == true) ? "true" : "false");
          printf ("friend_decl                         = %s \n",(sec_decl->friend_decl == true) ? "true" : "false");
          printf ("declared_in_func_prototype          = %s \n",(sec_decl->declared_in_func_prototype == true) ? "true" : "false");
          printf ("specialized_with_new_syntax         = %s \n",(sec_decl->specialized_with_new_syntax == true) ? "true" : "false");
          printf ("first_declaration                   = %s \n",(sec_decl->first_declaration == true) ? "true" : "false");
#if TEMPLATE_INSTANTIATIONS_IN_SOURCE_SEQUENCE_LISTS
          printf ("is_partial_instantiation            = %s \n",(sec_decl->is_partial_instantiation == true) ? "true" : "false");
          printf ("compiler_generated_forward_decl     = %s \n",(sec_decl->compiler_generated_forward_decl == true) ? "true" : "false");
          printf ("originally_nonautonomous_definition = %s \n",(sec_decl->originally_nonautonomous_definition == true) ? "true" : "false");
#endif
#if GNU_EXTENSIONS_ALLOWED
          printf ("marked_as_gnu_extension             = %s \n",(sec_decl->marked_as_gnu_extension == true) ? "true" : "false");
#endif
          printf ("is_decl_after_first_in_comma_list   = %s \n",(sec_decl->is_decl_after_first_in_comma_list == true) ? "true" : "false");
          printf ("explicit_storage_class              = %s \n",(sec_decl->explicit_storage_class == true) ? "true" : "false");
          printf ("is_alias                            = %s \n",(sec_decl->is_alias == true) ? "true" : "false");
        }
       else
        {
          printf ("sec_decl == NULL \n");
        }
   }


#ifdef __cplusplus
string summarize_expression_in_string (an_expr_node_ptr ptr)
   {
     string s = summarize_expr_kind ((an_expr_node_kind)ptr->kind);
     if (ptr->kind == enk_operation)
        {
          s += ":" + summarize_expr_operator(ptr->variant.operation.kind);
        }

     if (ptr->kind == enk_constant)
        {
          string temp_s = summarize_constant_kind_in_string(ptr->variant.constant.ptr);
          s += ":" + temp_s;
        }

     return s;
   }
#endif

#if 1
char* summarize_expr_kind (an_expr_node_kind x)
   {
     char* s;

  // DQ (12/29/2011): Fixed compiler warnings using a cast to "char*"
     switch (x)
        {
          case enk_error:                         s = (char*)"enk_error";                         break;
          case enk_operation:                     s = (char*)"enk_operation";                     break;
          case enk_constant:                      s = (char*)"enk_constant";                      break;
          case enk_variable:                      s = (char*)"enk_variable";                      break;
          case enk_field:                         s = (char*)"enk_field";                         break;
          case enk_temp_init:                     s = (char*)"enk_temp_init";                     break;
          case enk_new_delete:                    s = (char*)"enk_new_delete";                    break;
          case enk_lambda:                        s = (char*)"enk_lambda";                        break;
#if MICROSOFT_EXTENSIONS_ALLOWED
          case enk_gcnew:                         s = (char*)"enk_gcnew";                         break;
#endif /* MICROSOFT_EXTENSIONS_ALLOWED */
          case enk_throw:                         s = (char*)"enk_throw";                         break;
          case enk_condition:                     s = (char*)"enk_condition";                     break;
          case enk_object_lifetime:               s = (char*)"enk_object_lifetime";               break;
          case enk_typeid:                        s = (char*)"enk_typeid";                        break;

       // DQ (6/20/2013): Note that enk_runtime_sizeof is set to the value of enk_sizeof.
       // case enk_sizeof:                        s = (char*)"enk_sizeof";                        break;
          case enk_runtime_sizeof:                s = (char*)"enk_sizeof/enk_runtime_sizeof";     break;

          case enk_sizeof_pack:                   s = (char*)"enk_sizeof_pack";                   break;
          case enk_alignof:                       s = (char*)"enk_alignof";                       break;
#if GNU_EXTENSIONS_ALLOWED
          case enk_statement:                     s = (char*)"enk_statement";                     break;
#endif /* GNU_EXTENSIONS_ALLOWED */
          case enk_reuse_value:                   s = (char*)"enk_reuse_value";                   break;
#if DO_IL_LOWERING && !DO_FULL_PORTABLE_EH_LOWERING
          case enk_lowered_eh_construct:          s = (char*)"enk_lowered_eh_construct";          break;
#endif
#if DO_IL_LOWERING && ABI_CHANGES_FOR_COVARIANT_VIRTUAL_FUNC_RETURN
          case enk_result_of_overriding_function: s = (char*)"enk_result_of_overriding_function"; break;
#endif
          case enk_routine:                       s = (char*)"enk_routine";                       break;
#if VLA_DEALLOCATIONS_IN_IL
          case enk_vla_dealloc:                   s = (char*)"enk_vla_dealloc";                   break;   
#endif
          case enk_type_operand:                  s = (char*)"enk_type_operand";                  break;
          case enk_builtin_operation:             s = (char*)"enk_builtin_operation";             break;
          case enk_param_ref:                     s = (char*)"enk_param_ref";                     break;
          case enk_braced_init_list:              s = (char*)"enk_braced_init_list";              break;
          case enk_c11_generic:                   s = (char*)"enk_c11_generic";                   break;
#if BUILTIN_FUNCTIONS_ENABLED
          case enk_builtin_choose_expr:           s = (char*)"enk_builtin_choose_expr";           break;
#endif
          case enk_yield:                         s = (char*)"enk_yield";                         break;
          case enk_await:                         s = (char*)"enk_await";                         break;
          case enk_fold:                          s = (char*)"enk_fold";                          break;
          case enk_initializer:                   s = (char*)"enk_initializer";                   break;
          case enk_concept_id:                    s = (char*)"enk_concept_id";                    break;
          case enk_requires:                      s = (char*)"enk_requires";                      break;
          case enk_compound_req:                  s = (char*)"enk_compound_req";                  break;
          case enk_nested_req:                    s = (char*)"enk_nested_req";                    break;
          case enk_const_eval_deferred:           s = (char*)"enk_const_eval_deferred";           break;
          case enk_template_name:                 s = (char*)"enk_template_name";                 break;
          case enk_last:                          s = (char*)"enk_last";                          break;
          default:
             {
               printf ("Default reached in summarize_expr_kind() x = %d \n",(int)x);
               s = (char*)"error: unknown expression";

            // DQ (6/20/2013): Added assertion to detect this default case (and force implementation of any missing cases).
               ROSE_ASSERT(false);
               break;
             }
        }

     return s;
   }
#endif

#ifdef __cplusplus
// string summarize_expr_operator (an_expr_operator_kind x)
string summarize_expr_operator (an_expr_operator_kind x)
   {
     string s;
     switch (x)
        {
          case eok_address_of: s = "eok_address_of"; break;
          case eok_reference_to: s = "eok_reference_to"; break;
          case eok_handle_to: s = "eok_handle_to"; break;
          case eok_indirect: s = "eok_indirect"; break;
          case eok_ref_indirect: s = "eok_ref_indirect"; break;
          case eok_cast: s = "eok_cast"; break;
          case eok_lvalue_cast: s = "eok_lvalue_cast"; break;
          case eok_ref_cast: s = "eok_ref_cast"; break;
          case eok_lvalue_adjust: s = "eok_lvalue_adjust"; break;
          case eok_class_rvalue_adjust: s = "eok_class_rvalue_adjust"; break;
          case eok_box: s = "eok_box"; break;
          case eok_handle_to_box: s = "eok_handle_to_box"; break;
          case eok_unbox: s = "eok_unbox"; break;
          case eok_unbox_lvalue: s = "eok_unbox_lvalue"; break;
          case eok_base_class_cast: s = "eok_base_class_cast"; break;
          case eok_derived_class_cast: s = "eok_derived_class_cast"; break;
          case eok_pm_base_class_cast: s = "eok_pm_base_class_cast"; break;
          case eok_pm_derived_class_cast: s = "eok_pm_derived_class_cast"; break;
          case eok_dynamic_cast: s = "eok_dynamic_cast"; break;
          case eok_ref_dynamic_cast: s = "eok_ref_dynamic_cast"; break;
          case eok_bool_cast: s = "eok_bool_cast"; break;
          case eok_array_to_pointer: s = "eok_array_to_pointer"; break;
          case eok_dot_vacuous_destructor_call: s = "eok_dot_vacuous_destructor_call"; break;
          case eok_points_to_vacuous_destructor_call: s = "eok_points_to_vacuous_destructor_call"; break;
#if MICROSOFT_EXTENSIONS_ALLOWED
          case eok_assume: s = "eok_assume"; break;
#endif /* MICROSOFT_EXTENSIONS_ALLOWED */
          case eok_noexcept: s = "eok_noexcept"; break;
          case eok_parens: s = "eok_parens"; break;
          case eok_negate: s = "eok_negate"; break;
          case eok_unary_plus: s = "eok_unary_plus"; break;
          case eok_complement: s = "eok_complement"; break;
          case eok_not: s = "eok_not"; break;
          case eok_vector_not: s = "eok_vector_not"; break;
          case eok_vector_fill: s = "eok_vector_fill"; break;
#if C99_IL_EXTENSIONS_SUPPORTED
          case eok_xconj: s = "eok_xconj"; break;
          case eok_real_part: s = "eok_real_part"; break;
          case eok_imag_part: s = "eok_imag_part"; break;
#endif /* C99_IL_EXTENSIONS_SUPPORTED */
          case eok_post_incr: s = "eok_post_incr"; break;
          case eok_post_decr: s = "eok__post_decr"; break;
          case eok_pre_incr: s = "eok_pre_incr"; break;
          case eok_pre_decr: s = "eok_pre_decr"; break;
          case eok_add: s = "eok_add"; break;
          case eok_subtract: s = "eok_subtract"; break;
          case eok_multiply: s = "eok_multiply"; break;
          case eok_divide: s = "eok_divide"; break;
          case eok_remainder: s = "eok_remainder"; break;
#if C99_IL_EXTENSIONS_SUPPORTED
          case eok_jmultiply: s = "eok_jmultiply"; break;
          case eok_jdivide: s = "eok_jdivide"; break;
          case eok_fjadd: s = "eok_fjadd"; break;
          case eok_jfadd: s = "eok_jfadd"; break;
          case eok_fjsubtract: s = "eok_"; break;
          case eok_jfsubtract: s = "eok_jfsubtract"; break;
#endif /* C99_IL_EXTENSIONS_SUPPORTED */
          case eok_padd: s = "eok_padd"; break;
          case eok_psubtract: s = "eok_psubtract"; break;
          case eok_pdiff: s = "eok_pdiff"; break;
          case eok_shiftl: s = "eok_shiftl"; break;
          case eok_shiftr: s = "eok_shiftr"; break;
          case eok_and: s = "eok_and"; break;
          case eok_or: s = "eok_or"; break;
          case eok_xor: s = "eok_xor"; break;
          case eok_eq: s = "eok_eq"; break;
          case eok_ne: s = "eok_ne"; break;
          case eok_gt: s = "eok_gt"; break;
          case eok_lt: s = "eok_lt"; break;
          case eok_ge: s = "eok_ge"; break;
          case eok_le: s = "eok_le"; break;
          case eok_vector_eq: s = "eok_vector_eq"; break;
          case eok_vector_ne: s = "eok_vector_ne"; break;
          case eok_vector_gt: s = "eok_vector_gt"; break;
          case eok_vector_lt: s = "eok_vector_lt"; break;
          case eok_vector_ge: s = "eok_vector_ge"; break;
          case eok_vector_le: s = "eok_vector_le"; break;
          case eok_gnu_min: s = "eok_gnu_min"; break;
          case eok_gnu_max: s = "eok_gnu_max"; break;
          case eok_assign: s = "eok_assign"; break;
          case eok_add_assign: s = "eok_add_assign"; break;
          case eok_subtract_assign: s = "eok_subtract_assign"; break;
          case eok_multiply_assign: s = "eok_multiply_assign"; break;
          case eok_divide_assign: s = "eok_divide_assign"; break;
          case eok_remainder_assign: s = "eok_remainder_assign"; break;
          case eok_shiftl_assign: s = "eok_shiftl_assign"; break;
          case eok_shiftr_assign: s = "eok_shiftr_assign"; break;
          case eok_and_assign: s = "eok_and_assign"; break;
          case eok_or_assign: s = "eok_or_assign"; break;
          case eok_xor_assign: s = "eok_xor_assign"; break;
          case eok_padd_assign: s = "eok_padd_assign"; break;
          case eok_psubtract_assign: s = "eok_psubtract_assign"; break;
          case eok_bassign: s = "eok_bassign"; break;
          case eok_land: s = "eok_land"; break;
          case eok_lor: s = "eok_lor"; break;
          case eok_vector_land: s = "eok_vector_land"; break;
          case eok_vector_lor: s = "eok_vector_lor"; break;
          case eok_comma: s = "eok_comma"; break;
          case eok_subscript: s = "eok_subscript"; break;
          case eok_vector_subscript: s = "eok_vector_subscript"; break;
          case eok_dot_field: s = "eok_dot_field"; break;
          case eok_points_to_field: s = "eok_points_to_field"; break;
          case eok_pm_field: s = "eok_pm_field"; break;
          case eok_pm_points_to_field: s = "eok_pm_points_to_field"; break;
          case eok_dot_pm_func_ptr: s = "eok_dot_pm_func_ptr"; break;
          case eok_points_to_pm_func_ptr: s = "eok_points_to_pm_func_ptr"; break;
          case eok_dot_static: s = "eok_dot_static"; break;
          case eok_points_to_static: s = "eok_points_to_static"; break;
          case eok_virtual_function_ptr: s = "eok_virtual_function_ptr"; break;
          case eok_question: s = "eok_question"; break;
          case eok_vector_question: s = "eok_vector_question"; break;
          case eok_call: s = "eok_call"; break;
          case eok_dot_member_call: s = "eok_dot_member_call"; break;
          case eok_points_to_member_call: s = "eok_points_to_member_call"; break;
          case eok_dot_pm_call: s = "eok_dot_pm_call"; break;
          case eok_points_to_pm_call: s = "eok_points_to_pm_call"; break;
          case eok_cli_subscript: s = "eok_cli_subscript"; break;
          case eok_va_start: s = "eok_va_start"; break;
          case eok_va_arg: s = "eok_va_arg"; break;
          case eok_va_end: s = "eok_va_end"; break;
          case eok_va_copy: s = "eok_va_copy"; break;
          case eok_va_start_single_operand: s = "eok_va_start_single_operand"; break;
          case eok_lvalue: s = "eok_lvalue"; break;
          case eok_error: s = "eok_error"; break;
          case eok_last: s = "eok_last"; break;
          case eok_spaceship: s = "eok_spaceship"; break;
          case eok_await:  s = "eok_await";  break;
          case eok_yield:  s = "eok_yield";  break;
          default:
               printf ("Default reached in summarize_expr_operator(an_expr_operator_kind = %d) \n",(int)x);
        }

     return s;
   }

#endif

#ifdef __cplusplus
string summarizeTypeModifier ( const a_type_qualifier_set    & modifiers )
   {
      string returnString;

      if (modifiers == TQ_NONE)       returnString += "none ";
      if (modifiers & TQ_CONST)       returnString += "const ";
      if (modifiers & TQ_VOLATILE)    returnString += "volatile ";
      if (modifiers & TQ_RESTRICT)    returnString += "restrict ";
#if MICROSOFT_EXTENSIONS_ALLOWED
      if (modifiers & TQ_UNALIGNED)   returnString += "unaligned ";
#endif
#if NEAR_AND_FAR_ALLOWED
      if (modifiers & TQ_NEAR)        returnString += "near ";
      if (modifiers & TQ_FAR)         returnString += "far ";
#endif
#if UPC_EXTENSIONS_ALLOWED
      if (modifiers & TQ_UPC_SHARED)  returnString += "upc_shared ";
      if (modifiers & TQ_UPC_STRICT)  returnString += "upc_strict ";
      if (modifiers & TQ_UPC_RELAXED) returnString += "upc_relaxed ";
#endif

      return returnString;
   }
#endif

#ifdef __cplusplus
string summarizeStorageModifier ( const a_storage_class & modifiers )
   {
     return string(db_storage_class_names[modifiers]);
   }
#endif

#ifdef __cplusplus
string summarizeAccessModifier ( const an_access_specifier & modifiers )
   {
     string returnString;

     switch (modifiers)
        {
          case as_public:
               returnString = "public ";
               break;
          case as_protected:
               returnString = "protected ";
               break;
          case as_private:
               returnString = "private ";
               break;
          case as_inaccessible:
               returnString = "inaccessible ";
               break;
          default:
               printf ("Error: default reached \n");
               ROSE_ASSERT (false);
               break;
        };

      return returnString;
   }
#endif

#ifdef __cplusplus
string summarizeLinkageModifier ( const a_name_linkage_kind & modifiers )
   {
     string returnString;

     switch (modifiers)
        {
          case nlk_none:
               returnString = "nlk_none ";
               break;
          case nlk_internal:
               returnString = "nlk_internal ";
               break;
          case nlk_cplusplus_external:
               returnString = "nlk_cplusplus_external ";
               break;
          case nlk_external:
               returnString = "nlk_external ";
               break;
       // nlk_last_standard was set explicitly to nlk_external
       // case nlk_last_standard:
       //      returnString = "nlk_last_standard ";
       //      break;
          case nlk_last:
               returnString = "nlk_last ";
               break;
          default:
               printf ("Error: default reached \n");
               ROSE_ASSERT (false);
               break;
        };

      return returnString;
   }
#endif

#ifdef __cplusplus
string
summarizeSpecialFunctionModifier ( const a_special_function_kind & modifiers )
   {
     return db_special_function_kinds[modifiers];
   }
#endif

#if UPC_EXTENSIONS_ALLOWED
#ifdef __cplusplus
string
summarizeUPC_Modifier ( const a_upc_access_method & modifiers )
   {
     string returnString;

     switch (modifiers)
        {
          case upc_access_unspecified:
               returnString = "upc_access_unspecified ";
               break;
          case upc_access_strict:
               returnString = "upc_access_strict ";
               break;
          case upc_access_relaxed:
               returnString = "upc_access_relaxed ";
               break;
          default:
               printf ("Error: default reached \n");
               ROSE_ASSERT (false);
               break;
        };

      return returnString;
   }
#endif
#endif

#ifdef __cplusplus
string
summarizeDynamicInitialization ( const a_dynamic_init_kind & x )
   {
     string returnString;

     switch (x)
        {
          case dik_none:
               returnString = "dik_none";
               break;
          case dik_zero:
               returnString = "dik_zero";
               break;
          case dik_constant:
               returnString = "dik_constant";
               break;
          case dik_expression:
               returnString = "dik_expression";
               break;
          case dik_call_returning_class_via_cctor:
               returnString = "dik_call_returning_class_via_cctor";
               break;
          case dik_constructor:
               returnString = "dik_constructor";
               break;
          case dik_nonconstant_aggregate:
               returnString = "dik_nonconstant_aggregate";
               break;
          case dik_bitwise_copy:
               returnString = "dik_bitwise_copy";
               break;
          case dik_lambda:
               returnString = "dik_lambda";
               break;
          default:
               printf ("Error: default reached: x = %d \n",x);
               ROSE_ASSERT (false);
               break;
        };

     return returnString;
   }
#endif


#ifdef __cplusplus
string
summarize_integer_string(an_integer_kind kind)
   {
     string returnString;

     switch (kind)
        {
          case ik_char:               returnString = "dik_char";              break;
          case ik_signed_char:        returnString = "dik_zero";              break;
          case ik_unsigned_char:      returnString = "ik_unsigned_char";      break;
          case ik_short:              returnString = "ik_short";              break;
          case ik_unsigned_short:     returnString = "ik_unsigned_short";     break;
          case ik_int:                returnString = "ik_int";                break;
          case ik_unsigned_int:       returnString = "ik_unsigned_int";       break;
          case ik_long:               returnString = "ik_long";               break;
          case ik_unsigned_long:      returnString = "ik_unsigned_long";      break;
#if LONG_LONG_ALLOWED
          case ik_long_long:          returnString = "ik_long_long";          break;
          case ik_unsigned_long_long: returnString = "ik_unsigned_long_long"; break;
#endif
#if INT128_EXTENSIONS_ALLOWED
          case ik_int128:             returnString = "ik_int128";             break;
          case ik_unsigned_int128:    returnString = "ik_unsigned_int128";    break;
#endif
          case ik_none:               returnString = "ik_none";               break;
       // Note that ik_last == ik_none (duplicate case).
       // case ik_last:               returnString = "ik_last";               break;

          default:
               printf ("Error: default reached \n");
               ROSE_ASSERT (false);
               break;
        };

     return returnString;
   }
#endif

string
summarize_local_expr_node_ref_string(a_local_expr_node_ref_kind kind)
   {
     string returnString;

     switch (kind)
        {
          case lerk_none:            returnString = "lerk_none";            break;
          case lerk_typeof:          returnString = "lerk_typeof";          break;
#if PROTOTYPE_INSTANTIATIONS_IN_IL
          case lerk_generic_sizeof:  returnString = "lerk_generic_sizeof";  break;
#endif
          case lerk_array_bound:     returnString = "lerk_array_bound";     break;
          case lerk_dep_array_bound: returnString = "lerk_dep_array_bound"; break;
          case lerk_decltype:        returnString = "lerk_decltype";        break;

          default:
               printf ("Error: default reached \n");
               ROSE_ASSERT (false);
               break;
        };

     return returnString;
   }

void summarize_source_position ( const a_source_position & sourcePosition )
   {
#if 1
     printf ("In summarize_source_position(): sourcePosition.seq = %u \n",sourcePosition.seq);
#endif
  // DQ (6/13/2013): Added support to output the source position information.
#if 0
     char* file_name = NULL;
     char* full_name = NULL;

     a_line_number linenumber = 0;
     a_boolean at_end = FALSE;

  // a_source_position pos = ptr->decl_position;
  // print_source_pos(pos);
     conv_seq_to_file_and_line(sourcePosition.seq, &file_name, &full_name, &linenumber, &at_end);
     printf ("          ---- file_name     = %s \n",file_name);
     printf ("          ---- full_name     = %s \n",full_name);
     printf ("          ---- line number   = %" PRIuPTR " \n",linenumber);
     printf ("          ---- column number = %u \n",sourcePosition.column);
#else
     summarize_source_sequence (sourcePosition.seq);
     printf ("          ---- column number = %u \n",sourcePosition.column);
#endif

  // DQ (1/2/2016): Verify that we have this defines so that we can have access to more macro information (later).
#if MACRO_INVOCATION_TREE_IN_IL
extern void copy_macro_invocation_tree_to_il(void);
#else
   #error "I think we want this defined!"
#endif /* MACRO_INVOCATION_TREE_IN_IL */

  // DQ (1/2/2016): Verify that we have this defines so that we can have access to more macro information (later).
#if FULLY_RESOLVED_MACRO_POSITIONS
extern void init_macro_text_map(sizeof_t num_entries, a_macro_text_map_ptr mtmp, a_boolean resizable);
#else
   #error "I think we want this defined!"
#endif /* MACRO_INVOCATION_TREE_IN_IL */

#if RECORD_MACRO_INVOCATIONS
  // DQ (1/2/2016): Added documentation specific to macro expansions 
  // (note the ROSE compiles EDG with RECORD_MACRO_INVOCATIONS defined to be 1).
     if (sourcePosition.macro_context != NO_PARENT_MACRO_INVOCATION)
        {
          printf ("          ---- macro context != NO_PARENT_MACRO_INVOCATION \n");

          a_macro_invocation_record_ptr mirp = NULL;

       // This is how the internal data structure for macro handling is accessed 
       // (but it results in an undefined function at link time presently). So I
       // need to look into this further at a later point.
       // mirp = macro_invocation_record_at_index(mirp->parent_macro_index);

          int stack_depth = 0;
          for (mirp = macro_invocation_record_at_index(sourcePosition.macro_context); 
               mirp != NULL && mirp->parent_macro_index != NO_PARENT_MACRO_INVOCATION; 
               mirp = macro_invocation_record_at_index(mirp->parent_macro_index)) 
             {
               ++stack_depth;
             }

          printf ("### stack_depth = %d \n",stack_depth);

          mirp = macro_invocation_record_at_index(sourcePosition.macro_context);

       // DQ (2/10/2019): This is a problem for some larger tests when debuging the template argument translation.
       // ROSE_ASSERT(mirp != NULL);
          if (mirp != NULL)
             {

          summarize_macro_invocation_record(mirp);

          for (int i = 0; i < stack_depth; ++i)
             {
               string name_string = mirp->assoc_macro->source_corresp.name;
               printf ("### (in loop): name_string = %s \n",name_string.c_str());
#if 0
               if (i < 5 || i >= stack_depth - 4) 
                  {
                 // init_error_params();
                    if (mirp->assoc_macro != NULL) 
                       {
                         name_string = mirp->assoc_macro->source_corresp.name;
                       } 
                      else
                       {
                         name_string = error_text(ec_name_of_unknown_macro);
                       }  /* if */

                    printf ("### name_string = %s \n",name_string.c_str());

                 // copy_simple_position_to_full_position(mirp->start, full_pos);
                 // error_msg_positions[1] = &full_pos;
                 // diag_message(ec_in_expansion_of_macro, &null_source_position, severity, dck_macro_context);
                  }
                 else
                  {
#if 0
                    if (!frames_omitted_msg_printed) 
                       {
                         static char buffer[20];
                      // init_error_params();
                      // (void)sprintf(buffer, "%d", stack_depth - 9);
                      // error_msg_strings[1] = buffer;
                      // diag_message(ec_macro_context_lines_skipped, error_pos, severity, dck_macro_context);
                      // frames_omitted_msg_printed = TRUE;
                       }  /* if */
#endif
                  }
#endif
               mirp = macro_invocation_record_at_index(mirp->parent_macro_index);

               ROSE_ASSERT(mirp != NULL);

               summarize_macro_invocation_record(mirp);
             }
#if 0
          printf ("Exiting as a test! \n");
          ROSE_ASSERT(false);
#endif
             }
        }
       else
        {
          printf ("          ---- macro context == NO_PARENT_MACRO_INVOCATION \n");
        }
#endif
   }

void summarize_source_sequence ( const a_seq_number sourceSequenceNumber )
   {
     static char file_name_buffer[10000];
     static char full_name_buffer[10000];
     a_const_char** file_name = (a_const_char**)file_name_buffer;
     a_const_char** full_name = (a_const_char**)full_name_buffer;

     a_line_number linenumber = 0;
     a_boolean at_end = FALSE;

     if (sourceSequenceNumber > 0)
        {
          if (sourceSequenceNumber > 1000000)
             {
               printf ("ERROR: In summarize_source_sequence(): sourceSequenceNumber > 1000000 (and considered to be out of range, EDG 4.12 bug) \n");
               return;
             }

       // DQ (10/27/2015): This function is an error to call when sourceSequenceNumber == 0.
       // conv_seq_to_file_and_line(sourceSequenceNumber, &file_name, &full_name, &linenumber, &at_end);
          conv_seq_to_file_and_line(sourceSequenceNumber, file_name, full_name, &linenumber, &at_end);

          printf ("          ---- file_name     = %s \n",*file_name);
          printf ("          ---- full_name     = %s \n",*full_name);
          printf ("          ---- line number   = %u \n",linenumber);
        }
       else
        {
          printf ("          ---- sourceSequenceNumber == 0 \n");
        }
   }

void summarize_source_range ( const a_source_range & sourceRange )
   {
  // DQ (6/13/2013): Added support to output the source position information.
     printf ("     range start: \n");
     summarize_source_position(sourceRange.start);
     printf ("     range end: \n");
     summarize_source_position(sourceRange.end);
   }

void summarize_decl_pos_info ( const a_decl_position_supplement_ptr & ptr )
   {
  // DQ (6/13/2013): Added support to output the source position information.
     printf ("     identifier range: \n");
     summarize_source_range(ptr->identifier_range);
     printf ("     specifiers range: \n");
     summarize_source_range(ptr->specifiers_range);

  // This is a union of declarator_range/enum_value_range/namespace_definition_range (context dependent).
     printf ("     declarator_range (union): \n");
     summarize_source_range(ptr->variant.declarator_range);

  // DQ (7/25/2014): Added EDG 4.9 specific feature to report on source positions of different parts of constrcuts (declarations, I think).
     printf ("     extra_positions = %p \n",ptr->extra_positions);

  // DQ (8/14/2014): This is non-null for MS Windows (in processing the iostream header file).
  // I would like to find a location where this is NON-NULL.
  // ROSE_ASSERT(ptr->extra_positions == NULL);

     if (ptr->extra_positions != NULL)
        {
          printf ("In summarize_decl_pos_info(): This is a rare case when this is non-NULL (only happends for MS Windows (in processing the iostream header file). \n");

       // These are the positions of specific parts of declarations and other language constructs (new for EDG4.9, I think).
          an_element_position_ptr temp_position = ptr->extra_positions;
          do {
               summarize_source_position(temp_position->position);
               printf ("     extra_positions: kind = %s \n",summarize_element_position_kind(temp_position->kind).c_str());

               temp_position = temp_position->next;
             }
          while (temp_position != NULL);           
        }
   }

string summarize_element_position_kind ( an_element_position_kind kind )
   {
     string s;
     switch(kind)
        {
          case epk_error:                 s = "epk_error";                 break;
#if EXTRA_SOURCE_POSITIONS_IN_IL
          case epk_specialization_header: s = "epk_specialization_header"; break;
#endif
          case epk_noreturn:              s = "epk_noreturn";              break;
          case epk_last:                  s = "epk_last";                  break;

          default:
             {
               printf ("Error: default reached in summarize_element_position_kind(): kind = %d \n",(int)kind);
               ROSE_ASSERT(false);
             }
        }

     return s;
   }

#ifdef __cplusplus
// DQ (9/13/2004): added to support debugging!
// void summarize_source_correspondence ( a_source_correspondence & X )
void summarize_source_correspondence ( const a_source_correspondence & X )
   {
     printf ("     In summarize_source_correspondence(%p) \n",&X);
     printf ("     X.assoc_info  - - - - - - - - - - - - - - - - - - = %p \n",X.assoc_info);

     if (X.name != NULL)
        {
          printf ("     X.name (redundant)                                = %p \n",X.name);
          printf ("     X.name                                            = %s \n",X.name);
        }
       else
        {
       // printf ("     X.name                                            = %p \n",X.name);
          printf ("     X.name                                            = NULL \n");
        }

#if NEED_NAME_MANGLING
     printf ("     X.unmangled_name_or_mangled_encoding              = %s \n",X.unmangled_name_or_mangled_encoding);
#endif

#if !COMPILING_NEW_EDG_ROSE_INTERFACE
  // DQ (12/30/2008): sg_symbol_ptr does not exist for EDG 3.10
     printf ("     X.sg_symbol_ptr         = %p = %s \n",X.sg_symbol_ptr,(X.sg_symbol_ptr != NULL) ? X.sg_symbol_ptr->class_name().c_str() : "NULL");
#endif

     printf ("     trans_unit_corresp  - - - - - - - - - - - - - -   = %p \n",X.trans_unit_corresp);
#if COMPILING_NEW_EDG_ROSE_INTERFACE
     printf ("     parent_scope                                      = %p \n",X.parent_scope);
     printf ("     enclosing_routine                                 = %p \n",X.enclosing_routine);
#endif

  // a_source_position decl_position;
     printf ("     X.decl_position.seq  - - - - - - - - - - - - - -  = %ld \n",(unsigned long)X.decl_position.seq);
     if (X.decl_position.seq == 0)
        {
          printf ("     --- This might be compiler generated, or at least not in the source code! \n");
        }
       else
        {
#if 0
       // DQ (6/13/2013): Added support to output the source position information.
          char* file_name = NULL;
          char* full_name = NULL;

          a_line_number linenumber = 0;
          a_boolean at_end = FALSE;

          a_source_position pos = X.decl_position;
       // print_source_pos(pos);
          conv_seq_to_file_and_line(pos.seq, &file_name, &full_name, &linenumber, &at_end);
          printf ("          file_name     = %s \n",file_name);
          printf ("          full_name     = %s \n",full_name);
          printf ("          line number   = %u \n",linenumber);
          printf ("          column number = %u \n",pos.column);
#else
       // DQ (10/27/2015): Need to fixup the AST, but this is failing after AST construction.
#if 0
          printf ("Commenting this out for AST type fixup after AST construction \n");
#else
          summarize_source_position(X.decl_position);
#endif
#endif
        }

// #if 0
// DQ (12/30/2008): Removed to support new EDG/Sage/ROSE connection.
#if !COMPILING_NEW_EDG_ROSE_INTERFACE
     Sg_File_Info* fileInfo_1 = generateFileInfo(X.decl_position);
     fileInfo_1->display("X.decl_position");
     delete fileInfo_1;
#endif

#if EXTRA_SOURCE_POSITIONS_IN_IL
  // a_decl_position_supplement_ptr decl_pos_info;
  // printf ("     Skipped output of: X.decl_pos_info \n");
     printf ("     decl_pos_info                                     = %p \n",X.decl_pos_info);
     if (X.decl_pos_info != NULL)
        {
          printf ("     Output of decl_pos_info: \n");
#if 0
       // DQ (10/27/2015): Need to fixup the AST, but this is failing after AST construction.
          printf ("Commenting this out for AST type fixup after AST construction \n");
#else
          summarize_decl_pos_info(X.decl_pos_info);
#endif
#if 0
  // DQ (12/30/2008): Removed to support new EDG/Sage/ROSE connection.
          Sg_File_Info* fileInfo_2 = generateFileInfo(X.decl_pos_info->identifier_range,startOfConstruct);
          fileInfo_2->display("X.decl_pos_info->identifier_range (startOfConstruct)");
          Sg_File_Info* fileInfo_3 = generateFileInfo(X.decl_pos_info->identifier_range,endOfConstruct);
          fileInfo_3->display("X.decl_pos_info->identifier_range (endOfConstruct)");
          Sg_File_Info* fileInfo_4 = generateFileInfo(X.decl_pos_info->specifiers_range,startOfConstruct);
          fileInfo_4->display("X.decl_pos_info->specifiers_range (startOfConstruct)");
          Sg_File_Info* fileInfo_5 = generateFileInfo(X.decl_pos_info->specifiers_range,endOfConstruct);
          fileInfo_5->display("X.decl_pos_info->specifiers_range (endOfConstruct)");
          Sg_File_Info* fileInfo_6 = generateFileInfo(X.decl_pos_info->variant.declarator_range,startOfConstruct);
          fileInfo_6->display("X.decl_pos_info->variant.declarator_range or enum_value_range (startOfConstruct)");
          Sg_File_Info* fileInfo_7 = generateFileInfo(X.decl_pos_info->variant.declarator_range,endOfConstruct);
          fileInfo_7->display("X.decl_pos_info->variant.declarator_range or enum_value_range (endOfConstruct)");
          delete fileInfo_2;
          delete fileInfo_3;
          delete fileInfo_4;
          delete fileInfo_5;
          delete fileInfo_6;
          delete fileInfo_7;
#endif
        }
#endif /* EXTRA_SOURCE_POSITIONS_IN_IL */

#if RECORD_FORM_OF_NAME_REFERENCE
     printf ("     name_references                                      = %p \n",X.name_references);
     if (X.name_references != NULL)
        {
          summarize_name_reference(X.name_references);
          printf ("DONE: summarize_name_reference() \n");
        }     
#endif /* RECORD_FORM_OF_NAME_REFERENCE */

  // a_bit_field /* an_access_specifier */ access:2;
     printf ("     access   - - - - - - - - - - - - - - - - - - - - -   = %d \n",X.access);
#if MICROSOFT_EXTENSIONS_ALLOWED
     printf ("     assembly_access   - - - - - - - - - - - - - - - -    = %d \n",X.assembly_access);
#endif /* MICROSOFT_EXTENSIONS_ALLOWED */
  // a_bit_field referenced:1;
     printf ("     referenced                                           = %s \n",X.referenced ? "true" : "false");
#if MAINTAIN_NEEDED_FLAGS
  // a_bit_field needed:1;
     printf ("     needed                                               = %s \n",X.needed ? "true" : "false");
#endif /* MAINTAIN_NEEDED_FLAGS */
  // a_bit_field /* a_name_linkage_kind */ name_linkage:NUM_BITS_FOR_NAME_LINKAGE;
     printf ("     name_linkage  - - - - - - - - - - - - - - - - - - -  = %d \n",X.name_linkage);
     printf ("     name_linkage (kind)                                  = %s \n",summarizeLinkageModifier((a_name_linkage_kind)X.name_linkage).c_str());

  // a_bit_field has_associated_pragma:1;
     printf ("     has_associated_pragma                                = %s \n",X.has_associated_pragma ? "true" : "false");
  // a_bit_field is_local_to_function:1;
     printf ("     is_local_to_function   - - - - - - - - - - - - - -   = %s \n",X.is_local_to_function ? "true" : "false");
  // a_bit_field parent_via_local_scope_ref:1;
     printf ("     parent_via_local_scope_ref                           = %s \n",X.parent_via_local_scope_ref ? "true" : "false");
  // a_bit_field is_class_member:1;
     printf ("     is_class_member                                      = %s \n",X.is_class_member ? "true" : "false");
#if MICROSOFT_EXTENSIONS_ALLOWED
  // a_bit_field has_associated_attribute:1;
     printf ("     has_associated_attribute                             = %s \n",X.has_associated_attribute ? "true" : "false");
#endif /* MICROSOFT_EXTENSIONS_ALLOWED */


#if NEED_NAME_MANGLING
  // a_bit_field name_has_been_mangled:1;
     printf ("     name_has_been_mangled   - - - - - - - - - - - - - -  = %s \n",X.name_has_been_mangled ? "true" : "false");
  // a_bit_field mangled_name_cannot_be_included_in_other_name:1;
     printf ("     mangled_name_cannot_be_included_in_other_name        = %s \n",X.mangled_name_cannot_be_included_in_other_name ? "true" : "false");
  // a_bit_field final_name_mangling_pending:1;
     printf ("     final_name_mangling_pending                          = %s \n",X.final_name_mangling_pending ? "true" : "false");
  // a_bit_field unnamed_entity_given_fabricated_name:1;
     printf ("     unnamed_entity_given_fabricated_name                 = %s \n",X.unnamed_entity_given_fabricated_name ? "true" : "false");
#endif /* NEED_NAME_MANGLING */


#if BACK_END_IS_CP_GEN_BE
  // a_bit_field qualification_needed:1;
     printf ("     qualification_needed                                 = %s \n",X.qualification_needed ? "true" : "false");
  // a_bit_field partially_hidden_by_microsoft_injected_class_name:1;
     printf ("     partially_hidden_by_microsoft_injected_class_name    = %s \n",X.partially_hidden_by_microsoft_injected_class_name ? "true" : "false");
  // a_bit_field visible_as_unqualified_name:1;
     printf ("     visible_as_unqualified_name                          = %s \n",X.visible_as_unqualified_name ? "true" : "false");
#endif /* BACK_END_IS_CP_GEN_BE */
#if GENERATE_SOURCE_SEQUENCE_LISTS
  // a_bit_field is_decl_after_first_in_comma_list:1;
     printf ("     is_decl_after_first_in_comma_list                    = %s \n",X.is_decl_after_first_in_comma_list ? "true" : "false");
#endif /* GENERATE_SOURCE_SEQUENCE_LISTS */


#if ONE_INSTANTIATION_PER_OBJECT
  // a_bit_field static_used_by_instantiation:1;
     printf ("     static_used_by_instantiation   - - - - - - - - - -   = %s \n",X.static_used_by_instantiation ? "true" : "false");
#if DUPLICATE_SPECIAL_STATICS_IN_INSTANTIATION_SLICES
  // a_bit_field duplicate_static_in_instantiation_slices:1;
     printf ("     duplicate_static_in_instantiation_slices             = %s \n",X.duplicate_static_in_instantiation_slices ? "true" : "false");
#endif /* DUPLICATE_SPECIAL_STATICS_IN_INSTANTIATION_SLICES */
#endif /* ONE_INSTANTIATION_PER_OBJECT */

#if MAINTAIN_NEEDED_FLAGS
  // a_bit_field okay_to_walk_subtree_of_local_entity:1;
     printf ("     okay_to_walk_subtree_of_local_entity                 = %s \n",X.okay_to_walk_subtree_of_local_entity ? "true" : "false");
#endif /* MAINTAIN_NEEDED_FLAGS */
  // a_bit_field copied_from_secondary_trans_unit:1;
     printf ("     copied_from_secondary_trans_unit  - - - - - - - - -  = %s \n",X.copied_from_secondary_trans_unit ? "true" : "false");
  // a_bit_field same_name_as_external_entity_in_secondary_trans_unit:1;
     printf ("     same_name_as_external_entity_in_secondary_trans_unit = %s \n",X.same_name_as_external_entity_in_secondary_trans_unit ? "true" : "false");
  // a_bit_field member_of_unknown_base:1;
     printf ("     member_of_unknown_base                               = %s \n",X.member_of_unknown_base ? "true" : "false");
  // a_bit_field qualified_unknown_base_member:1;
     printf ("     qualified_unknown_base_member                        = %s \n",X.qualified_unknown_base_member ? "true" : "false");

#if MICROSOFT_EXTENSIONS_ALLOWED
  // a_bit_field member_of_unknown_super:1;
     printf ("     member_of_unknown_super  - - - - - - - - - - - - -   = %s \n",X.member_of_unknown_super ? "true" : "false");
  // a_bit_field microsoft_identifier_used:1;
     printf ("     microsoft_identifier_used  - - - - - - - - - - - -   = %s \n",X.microsoft_identifier_used ? "true" : "false");
#endif /* MICROSOFT_EXTENSIONS_ALLOWED */

#if GNU_EXTENSIONS_ALLOWED && GENERATE_SOURCE_SEQUENCE_LISTS
  // a_bit_field marked_as_gnu_extension:1;
     printf ("     marked_as_gnu_extension                              = %s \n",X.marked_as_gnu_extension ? "true" : "false");
#endif /* GNU_EXTENSIONS_ALLOWED && GENERATE_SOURCE_SEQUENCE_LISTS */
  // a_bit_field externalized:1;
     printf ("     externalized  - - - - - - - - - - - - - - - - - - -  = %s \n",X.externalized ? "true" : "false");
#if RECORD_SCOPE_DEPTH_IN_IL
  // a_scope_depth scope_depth;
     printf ("     scope_depth                                          = %d \n",X.scope_depth);
#endif /* RECORD_SCOPE_DEPTH_IN_IL */
#if GENERATE_SOURCE_SEQUENCE_LISTS
  // a_source_sequence_entry_ptr source_sequence_entry;
     printf ("     source_sequence_entry                                = %p \n",X.source_sequence_entry);
#endif /* GENERATE_SOURCE_SEQUENCE_LISTS */
#if ONE_INSTANTIATION_PER_OBJECT
  // a_per_instantiation_needed_flags_entry_ptr per_instantiation_needed_flags;
     printf ("     per_instantiation_needed_flags  - - - - - - - - - -  = %p \n",X.per_instantiation_needed_flags);
#endif /* ONE_INSTANTIATION_PER_OBJECT */

     printf ("     attributes  - - - - - - - - - - - - - - - - - - - -  = %p \n",X.attributes);
     an_attribute_ptr attribute = X.attributes;
     while (attribute != NULL)
        {
          summarize_attribute(attribute);
          attribute = attribute->next;
        }
   }
#endif


// DQ (9/26/2004): This is a specialized function written to debug the declaration within the typedef
void
printTypeInfo(a_type_ptr type)
   {
#if 0
  // DQ (12/30/2008): sg_symbol_ptr does not exist for EDG 3.10
     SgSymbol* symbol = get_symbol_ptr(type->source_corresp);

     printf ("Debugging typedef: under_type = %p symbol = %p \n",type,symbol);
     if (symbol != NULL)
        {
          SgClassSymbol* classSymbol = isSgClassSymbol(symbol);
          printf ("Debugging typedef: classSymbol = %p \n",classSymbol);
          if (classSymbol != NULL)
             {
               SgClassDeclaration* classDeclaration = classSymbol->get_declaration();
               ROSE_ASSERT(classDeclaration != NULL);
               printf ("In printTypeInfo(%p): classDeclaration = %p = %s \n",type,classDeclaration,classDeclaration->sage_class_name());
             }
        }
#else
  // DQ (12/30/2008): FIXME
     printf ("printTypeInfo() body commented out access to SgSymbol from type has changed! \n");
#endif
   }


#ifdef __cplusplus
void
printDeclarationInType ( SgType* type )
   {
     printf ("Debugging typedef with wrong (defining) declaration in SgTypePtr = %p = %s \n",type,type->sage_class_name());
     SgClassType* classType = isSgClassType(type);
     printf ("classType = %p \n",classType);
     if (classType != NULL)
        {
          ROSE_ASSERT(classType->get_declaration() != NULL);
          printf ("classType->get_declaration() = %p = %s isForward = %s \n",
               classType->get_declaration(),classType->get_declaration()->sage_class_name(),
               (classType->get_declaration()->isForward() == true) ? "true" : "false");
        }
   }
#endif

#ifdef __cplusplus
string
sourcePositionInformation ( SgNode* node )
   {
     string returnString;

  // Sg_File_Info* fileInfo = node->get_file_info();
     Sg_File_Info* fileInfo = node->get_startOfConstruct();

     if (fileInfo != NULL)
        {
          returnString += " : line ";
          returnString += StringUtility::numberToString(fileInfo->get_line());
          returnString += " : file ";
          returnString += fileInfo->get_filename();
        }

     return returnString;
   }
#endif

#ifdef __cplusplus
#if 1
// DQ (3/7/2010): Used in some code uncommented via TRACE macros in sage_gen_be.C.
// DQ (1/4/2009): This does not appear to be used!

string
getLocationName ( SgScopeStatement* scope )
   {
  // DQ (11/15/2004): this function builds a string to report the location of the scope 
  // used in debugging within the handling of the scope stack.  Might be more useful
  // than just for this purpose.

     string returnString;

     ROSE_ASSERT(scope != NULL);
     switch (scope->variantT())
        {
          case V_SgNamespaceDefinitionStatement:
             {
               SgNamespaceDefinitionStatement *nameSpace = isSgNamespaceDefinitionStatement(scope);
               ROSE_ASSERT(nameSpace != NULL);
               returnString = nameSpace->get_qualified_name().str();
               returnString += sourcePositionInformation(scope);
               break;
             }

          case V_SgFunctionDefinition:
             {
               SgFunctionDefinition *functionDefinition = isSgFunctionDefinition(scope);
               ROSE_ASSERT(functionDefinition != NULL);
               SgFunctionDeclaration *functionDeclaration = isSgFunctionDeclaration(functionDefinition->get_declaration());
               ROSE_ASSERT(functionDeclaration != NULL);
               returnString = functionDeclaration->get_name().str();
               returnString += sourcePositionInformation(scope);
               break;
             }

          case V_SgTemplateInstantiationDefn:
          case V_SgClassDefinition:
             {
               SgClassDefinition *classDefinition = isSgClassDefinition(scope);
               ROSE_ASSERT(classDefinition != NULL);
               SgClassDeclaration *classDeclaration = isSgClassDeclaration(classDefinition->get_declaration());
               ROSE_ASSERT(classDeclaration != NULL);

               if (classDeclaration->get_name().is_null() == false)
                    returnString = classDeclaration->get_name().str();
                 else
                    returnString = "(Empty name for class or instantiated template declaration)";

               returnString += sourcePositionInformation(scope);
               break;
             }

       // These are anonymous cases which don't have user-defined names within the AST
          case V_SgBasicBlock:
             {
               SgBasicBlock *block = isSgBasicBlock(scope);
               ROSE_ASSERT(block != NULL);
               returnString = "anonymous SgBasicBlock scope";
               returnString += sourcePositionInformation(scope);
#if 0
               SgScopeStatement *parentScope = isSgScopeStatement(block->get_parent());
            // ROSE_ASSERT(parentScope != NULL);
               if (parentScope != NULL)
                    returnString += getLocationName(parentScope);
#endif
               break;
             }

          case V_SgIfStmt:
             {
               returnString = "anonymous SgIfStmt scope";
               break;
             }

          case V_SgDoWhileStmt:
             {
               returnString = "anonymous SgDoWhileStmt scope";
               break;
             }

          case V_SgCatchOptionStmt:
             {
               returnString = "anonymous SgCatchOptionStmt scope";
               break;
             }

          case V_SgForStatement:
             {
               returnString = "anonymous SgForStatement scope";
               break;
             }

          case V_SgSwitchStatement:
             {
               returnString = "anonymous SgSwitchStatement scope";
               break;
             }

          case V_SgWhileStmt:
             {
               returnString = "anonymous SgWhileStmt scope";
               break;
             }

          case V_SgGlobal:
             {
               returnString = "anonymous SgGlobal scope";
               break;
             }

          default:
             {
               printf ("Error: default reached in getLocationName scope = %s \n",scope->class_name().c_str());
               ROSE_ASSERT(false);
               break;
             }
        }

     return returnString;
   }
#endif
#endif

#ifdef __cplusplus
string
generateScopeName( const a_source_correspondence & source_corresp )
   {
  // printf ("source_corresp.is_class_member = %s \n",(source_corresp.is_class_member == true) ? "true" : "false");

     printf ("In generateScopeName(): calling summarize_source_correspondence \n");
     summarize_source_correspondence(const_cast<a_source_correspondence&>(source_corresp));

     string returnString = "name of parent scope: ";
     if (source_corresp.is_class_member == true)
        {
          printf ("Not supported in EDG 4.0 \n");
          ROSE_ASSERT(false);
        }
       else
        {
          printf ("Not supported in EDG 4.0 \n");
          ROSE_ASSERT(false);
        }

     return returnString;
   }
#endif

#ifdef __cplusplus
string
generateScopeName( a_type_ptr edgType )
   {
  // printf ("edgType = %p \n",edgType);
     return generateScopeName(edgType->source_corresp);
   }

string
generateScopeName( a_routine_ptr edgRoutine )
   {
  // printf ("edgRoutine = %p \n",edgRoutine);
     return generateScopeName(edgRoutine->source_corresp);
   }

string
generateScopeName ( a_source_sequence_entry_ptr ssp )
   {
     ROSE_ASSERT(ssp != NULL);
  // printf ("EDG Entity Kind = %s \n",summarize_entry_kind_in_string((an_il_entry_kind)ssp->entity.kind));

  // Note that the design of sage sets the a_source_correspondence structure at 
  // the start of each entity and thus we may make the following cast!
     a_source_correspondence_ptr scp = (a_source_correspondence_ptr) ssp->entity.ptr;
     string returnString = generateScopeName(*scp);
     return returnString;
   }


void
testForBadEdgTemplateName( const string & name )
   {
  // DQ (2/21/2005): Error checking on existance of substrings that can be common
  // in building template name and which are a source of errors in generated code.
  // When these names are used as a basis for code generation the generated code 
  // can have errors.
     if ( (name.find("<<") != string::npos) || (name.find(">>") != string::npos) )
        {
       // This could just be the input or output operators so make sure this is not that case!
          if ( (name.find("operator<<") == string::npos) && (name.find("operator>>") == string::npos) )
             {
            // OK, we have some sort of error to report!
               printf ("\n\n");
               printf ("Error: \"<<\" or \">>\" found in generated name, this could lead to an error in compiling the generated code later (in edg47Rose/degugging.C) \n");
               printf ("..... generated name = %s \n",name.c_str());
               printf ("\n\n");
               ROSE_ASSERT (false);
            // ROSE_ASSERT (returnName.find("<<") == string::npos);
            // ROSE_ASSERT (returnName.find(">>") == string::npos);
             }
        }
   }
#endif

char*
summarize_storage_class_in_string(a_storage_class X)
/*
Display the indicated storage_class.
*/
   {
     char* s = NULL;
     switch (X)
        {
          case sc_extern:
               s = (char*)"sc_extern";
               break;
          case sc_static:
               s = (char*)"sc_static";
               break;
          case sc_auto:
               s = (char*)"sc_auto";
               break;
          case sc_unspecified:
               s = (char*)"sc_unspecified";
               break;
          case sc_typedef:
               s = (char*)"sc_typedef";
               break;
          case sc_register:
               s = (char*)"sc_register";
               break;
          case sc_asm:
               s = (char*)"sc_asm";
               break;
          case sc_last:
               s = (char*)"sc_last";
               break;
          default:
               s = (char*)"**BAD STORAGE CLASS**";
        }  /* switch */

     assert (s != NULL);
     return s;
   }

string
summarize_builtin_operation_kind_in_string(a_builtin_operation_kind kind)
   {
     char* s = NULL;
     switch (kind)
        {
          case bok_offsetof:
               s = (char*)"bok_offsetof";
               break;
          case bok_has_assign:
               s = (char*)"bok_has_assign";
               break;
          case bok_has_copy:
               s = (char*)"bok_has_copy";
               break;
          case bok_has_nothrow_assign:
               s = (char*)"bok_has_nothrow_assign";
               break;
          case bok_has_nothrow_constructor:
               s = (char*)"bok_has_nothrow_constructor";
               break;
          case bok_has_nothrow_copy:
               s = (char*)"bok_has_nothrow_copy";
               break;
          case bok_has_trivial_assign:
               s = (char*)"bok_has_trivial_assign";
               break;
          case bok_has_trivial_constructor:
               s = (char*)"bok_has_trivial_constructor";
               break;
          case bok_has_trivial_copy:
               s = (char*)"bok_has_trivial_copy";
               break;
          case bok_has_trivial_destructor:
               s = (char*)"bok_has_trivial_destructor";
               break;
          case bok_has_user_destructor:
               s = (char*)"bok_has_user_destructor";
               break;
          case bok_has_virtual_destructor:
               s = (char*)"bok_has_virtual_destructor";
               break;
          case bok_is_abstract:
               s = (char*)"bok_is_abstract";
               break;
          case bok_is_base_of:
               s = (char*)"bok_is_base_of";
               break;
          case bok_is_class:
               s = (char*)"bok_is_class";
               break;
          case bok_is_convertible_to:
               s = (char*)"bok_is_convertible_to";
               break;
          case bok_is_empty:
               s = (char*)"bok_is_empty";
               break;
          case bok_is_enum:
               s = (char*)"bok_is_enum";
               break;
          case bok_is_pod:
               s = (char*)"bok_is_pod";
               break;
          case bok_is_polymorphic:
               s = (char*)"bok_is_polymorphic";
               break;
          case bok_is_union:
               s = (char*)"bok_is_union";
               break;
          case bok_types_compatible:
               s = (char*)"bok_types_compatible";
               break;
          case bok_intaddr:
               s = (char*)"bok_intaddr";
               break;
          case bok_is_trivial:
               s = (char*)"bok_is_trivial";
               break;
          case bok_is_standard_layout:
               s = (char*)"bok_is_standard_layout";
               break;
          case bok_is_trivially_copyable:
               s = (char*)"bok_is_trivially_copyable";
               break;
          case bok_is_literal_type:
               s = (char*)"bok_is_literal_type";
               break;
          case bok_has_trivial_move_constructor:
               s = (char*)"bok_has_trivial_move_constructor";
               break;
          case bok_has_trivial_move_assign:
               s = (char*)"bok_has_trivial_move_assign";
               break;
          case bok_has_nothrow_move_assign:
               s = (char*)"bok_has_nothrow_move_assign";
               break;
          case bok_is_constructible:
               s = (char*)"bok_is_constructible";
               break;
          case bok_is_nothrow_constructible:
               s = (char*)"bok_is_nothrow_constructible";
               break;
          case bok_has_finalizer:
               s = (char*)"bok_has_finalizer";
               break;
          case bok_is_delegate:
               s = (char*)"bok_is_delegate";
               break;
          case bok_is_interface_class:
               s = (char*)"bok_is_interface_class";
               break;
          case bok_is_ref_array:
               s = (char*)"bok_is_ref_array";
               break;
          case bok_is_ref_class:
               s = (char*)"bok_is_ref_class";
               break;
          case bok_is_sealed:
               s = (char*)"bok_is_sealed";
               break;
          case bok_is_simple_value_class:
               s = (char*)"bok_is_simple_value_class";
               break;
          case bok_is_final:
               s = (char*)"bok_is_final";
               break;
          case bok_is_trivially_constructible:
               s = (char*)"bok_is_trivially_constructible";
               break;
          case bok_is_destructible:
               s = (char*)"bok_is_destructible";
               break;
          case bok_is_nothrow_destructible:
               s = (char*)"bok_is_nothrow_destructible";
               break;
          case bok_is_trivially_destructible:
               s = (char*)"bok_is_trivially_destructible";
               break;
          case bok_is_nothrow_assignable:
               s = (char*)"bok_is_nothrow_assignable";
               break;
          case bok_is_trivially_assignable:
               s = (char*)"bok_is_trivially_assignable";
               break;

       // This case should not be seen.
          case bok_last:
               s = (char*)"bok_last";
               ROSE_ASSERT(false);
               break;

       // This case should not be seen.
          default:
               s = (char*)"**BAD operation kind**";
               ROSE_ASSERT(false);
        }

     return s;
   }



void
summarize_expr ( an_expr_node_ptr ptr )
   {
     ROSE_ASSERT (ptr != NULL);

  // generateFileInfo(ptr->source_corresp)->display("location of EDG expression");
  // printf ("     Output source_correspondence information: \n");
  // summarize_source_correspondence(ptr->source_corresp);

     printf ("In summarize_expr()                             = %p \n",ptr);

     printf ("     type                                       = %p \n",ptr->type);
     if (ptr->type != NULL)
        {
          printf ("     type->kind                                 = %s \n",summarize_type_kind_in_string(ptr->type));
        }
     printf ("     orig_lvalue_type                           = %p \n",ptr->orig_lvalue_type);
     if (ptr->orig_lvalue_type != NULL)
        {
          printf ("     orig_lvalue_type->kind                     = %s \n",summarize_type_kind_in_string(ptr->orig_lvalue_type));
        }
     printf ("     next                                       = %p \n",ptr->next);
     printf ("     kind                                       = %s \n",summarize_expr_kind((an_expr_node_kind)ptr->kind));
     printf ("     is_lvalue                                  = %s \n",ptr->is_lvalue ? "true" : "false");
     printf ("     is_xvalue                                  = %s \n",ptr->is_xvalue ? "true" : "false");
     printf ("     result_is_not_used                         = %s \n",ptr->result_is_not_used ? "true" : "false");
     printf ("     is_initialization_guard                    = %s \n",ptr->is_initialization_guard ? "true" : "false");
     printf ("     generated_default_arg                      = %s \n",ptr->generated_default_arg ? "true" : "false");
#if GNU_EXTENSIONS_ALLOWED
     printf ("     marked_as_gnu_extension                    = %s \n",ptr->marked_as_gnu_extension ? "true" : "false");
#endif
     printf ("     is_static_cast                             = %s \n",ptr->is_static_cast ? "true" : "false");
     printf ("     is_functional_notation_cast                = %s \n",ptr->is_functional_notation_cast ? "true" : "false");
     printf ("     is_brace_notation_cast                     = %s \n",ptr->is_brace_notation_cast ? "true" : "false");
     printf ("     is_objectless_nonstatic_data_mem_ref       = %s \n",ptr->is_objectless_nonstatic_data_mem_ref ? "true" : "false");
     printf ("     is_pack_expansion                          = %s \n",ptr->is_pack_expansion ? "true" : "false");
#if MICROSOFT_EXTENSIONS_ALLOWED
     printf ("     is_safe_cast                               = %s \n",ptr->is_safe_cast ? "true" : "false");
     printf ("     element_of_cli_param_array_arg             = %s \n",ptr->element_of_cli_param_array_arg ? "true" : "false");
#endif
#if DO_IL_LOWERING
     printf ("     is_non_normalized_boolean_controlling_expr = %s \n",ptr->is_non_normalized_boolean_controlling_expr ? "true" : "false");
#endif
#if BACK_END_IS_CP_GEN_BE
     printf ("     keep_as_cast_for_cp_gen_be                 = %s \n",ptr->keep_as_cast_for_cp_gen_be ? "true" : "false");
#endif /* BACK_END_IS_CP_GEN_BE */

#if MICROSOFT_EXTENSIONS_ALLOWED
     printf ("     is_cli_typeid                              = %s \n",ptr->is_cli_typeid ? "true" : "false");
#endif
     printf ("     compiler_generated                         = %s \n",ptr->compiler_generated ? "true" : "false");

     switch(ptr->kind)
        {
          case enk_operation:
             {
#ifdef __cplusplus
               printf ("     (enk_operation): operator_kind                           = %s \n",summarize_expr_operator(ptr->variant.operation.kind).c_str());
#endif
               if (ptr->variant.operation.kind == eok_call)
                  {
                 // This is a function call of some sort (what is the name)
                  }
               printf ("     (enk_operation): returns_lvalue_instead_of_usual_rvalue  = %s \n",ptr->variant.operation.returns_lvalue_instead_of_usual_rvalue ? "true" : "false");
               printf ("     (enk_operation): is_reinterpret_cast                     = %s \n",ptr->variant.operation.is_reinterpret_cast ? "true" : "false");
               printf ("     (enk_operation): is_const_cast                           = %s \n",ptr->variant.operation.is_const_cast ? "true" : "false");
               printf ("     (enk_operation): is_reference_cast                       = %s \n",ptr->variant.operation.is_reference_cast ? "true" : "false");

               printf ("     (enk_operation): is_rvalue_reference_cast                = %s \n",ptr->variant.operation.is_rvalue_reference_cast ? "true" : "false");
#if MICROSOFT_EXTENSIONS_ALLOWED
               printf ("     is_tracking_reference_cast                               = %s \n",ptr->variant.operation.is_tracking_reference_cast ? "true" : "false");
#endif
               printf ("     (enk_operation): implicit_in_member_naming               = %s \n",ptr->variant.operation.implicit_in_member_naming ? "true" : "false");
               printf ("     (enk_operation): implicit_step_of_explicit_cast          = %s \n",ptr->variant.operation.implicit_step_of_explicit_cast ? "true" : "false");
               printf ("     (enk_operation): is_conversion_call                      = %s \n",ptr->variant.operation.is_conversion_call ? "true" : "false");
               printf ("     (enk_operation): arg_dependent_lookup_suppressed_on_call = %s \n",ptr->variant.operation.arg_dependent_lookup_suppressed_on_call ? "true" : "false");
               printf ("     (enk_operation): call_with_qualified_function_name       = %s \n",ptr->variant.operation.call_with_qualified_function_name ? "true" : "false");

#if BACK_END_IS_CP_GEN_BE
               printf ("     (enk_operation): only_found_through_arg_dependent_lookup = %s \n",ptr->variant.operation.is_const_cast ? "true" : "false");
#endif
               printf ("     (enk_operation): call_uses_operator_syntax               = %s \n",ptr->variant.operation.call_uses_operator_syntax ? "true" : "false");
#if GNU_EXTENSIONS_ALLOWED
               printf ("     (enk_operation): is_gnu_two_operand_question_mark        = %s \n",ptr->variant.operation.is_gnu_two_operand_question_mark ? "true" : "false");
#endif
               printf ("     (enk_operation): pointer_operand_is_second               = %s \n",ptr->variant.operation.pointer_operand_is_second ? "true" : "false");
               printf ("     (enk_operation): is_virtual_call                         = %s \n",ptr->variant.operation.is_virtual_call ? "true" : "false");
#if MICROSOFT_EXTENSIONS_ALLOWED
               printf ("     rewritten_property_reference_kind                        = %" PRIuPTR " \n",(size_t)ptr->variant.operation.rewritten_property_reference_kind);
               printf ("     requires_runtime_cast_check                              = %s \n",ptr->variant.operation.requires_runtime_cast_check ? "true" : "false");
#endif
#if MICROSOFT_EXTENSIONS_ALLOWED
               printf ("     (enk_operation): is_const_cast                           = %s \n",ptr->variant.operation.is_const_cast ? "true" : "false");
#endif
            // printf ("     (enk_operation): implicit_in_member_naming               = %s \n",ptr->variant.operation.implicit_in_member_naming ? "true" : "false");
            // printf ("     (enk_operation): implicit_step_of_explicit_cast          = %s \n",ptr->variant.operation.implicit_step_of_explicit_cast ? "true" : "false");
            // printf ("     (enk_operation): is_reference_cast                       = %s \n",ptr->variant.operation.is_reference_cast ? "true" : "false");
            // printf ("     (enk_operation): is_conversion_call                      = %s \n",ptr->variant.operation.is_conversion_call ? "true" : "false");

               printf ("     (enk_operation): operands                                = %p \n",ptr->variant.operation.operands);

            // Loop over the operand pointers only.
               int counter = 0;
               an_expr_node_ptr ops = ptr->variant.operation.operands;
               while (ops != NULL)
                  {
                    printf ("     (enk_operation): --- operand #%d = %p \n",counter,ops);
                    ops = ops->next;
                    counter++;
                  }

#if ENABLE_CUDA_SUPPORT
               printf ("     is_kernel_call                                  = %s \n",ptr->variant.operation.is_kernel_call ? "true" : "false");
               printf ("     config_operands                                 = %p \n",ptr->variant.operation.config_operands);
#endif
               break;
             }

          case enk_constant:
             {
            // DQ (10/5/2016): This is an IR change in the EDG 4.11 IR.
               printf ("     (enk_constant): constant = %p \n",ptr->variant.constant.ptr);
               printf ("     constant kind            = %s \n",summarize_constant_kind_in_string(ptr->variant.constant.ptr));
               printf ("     In summarize_expr(): (enk_constant): calling summarize_constant(%p) \n",ptr->variant.constant.ptr);
               summarize_constant(ptr->variant.constant.ptr);

               printf ("     name_reference        = %p \n",ptr->variant.constant.name_reference);
               if (ptr->variant.constant.name_reference != NULL)
                  {
                    summarize_name_reference(ptr->variant.constant.name_reference);
                  }
               break;
             }

          case enk_variable:
             {
               printf ("     (enk_variable): variable = %p \n",ptr->variant.variable.ptr);
               printf ("     name_reference        = %p \n",ptr->variant.variable.name_reference);
               if (ptr->variant.variable.name_reference != NULL)
                  {
                    summarize_name_reference(ptr->variant.variable.name_reference);
                  }
               break;
             }

#if GNU_EXTENSIONS_ALLOWED
          case enk_statement:
             {
               printf ("     (enk_statement): statement                 = %p \n",ptr->variant.statement);
#if 1
            // DQ (3/19/2015): Added more debugging code.
               if (ptr->variant.statement != NULL)
                  {
                 // DQ (7/6/2019): Added formatting and more debug information: calling from inside of summarize_expr ( an_expr_node_ptr ptr )
                    printf ("     In summarize_expr(): GNU STATEMENT EXPRESSION: Calling summarize_statement() \n");
                    summarize_statement(ptr->variant.statement);
                    printf ("     In summarize_expr(): DONE: GNU STATEMENT EXPRESSION: Calling summarize_statement() \n");
                  }
#endif
               break;
             }
#endif

          case enk_reuse_value:
             {
               printf ("     (enk_reuse_value): reused_value_init = %p \n",ptr->variant.reused_value_init);
               break;
             }

#if DO_IL_LOWERING && !DO_FULL_PORTABLE_EH_LOWERING
          case enk_lowered_eh_construct:
             {
               printf ("     (enk_x): x = %p \n",ptr->variant.x);
               break;
             }
#endif
          case enk_routine:
             {
               printf ("     (enk_routine): constant     = %p \n",ptr->variant.routine.ptr);

#if MICROSOFT_EXTENSIONS_ALLOWED && !DO_IL_LOWERING
// undeclared identifiers
//               printf ("     --- property_or_event_descr = %p \n",property_or_event_descr);
//               printf ("     --- special_kind            = %d \n",(int)special_kind);
#endif /* MICROSOFT_EXTENSIONS_ALLOWED && !DO_IL_LOWERING */

               ROSE_ASSERT(ptr->variant.routine.ptr != NULL);
               summarize_routine(ptr->variant.routine.ptr);

               printf ("     name_reference        = %p \n",ptr->variant.routine.name_reference);
               if (ptr->variant.routine.name_reference != NULL)
                  {
                    summarize_name_reference(ptr->variant.routine.name_reference);
                  }
               break;
             }
          case enk_field:
             {
               printf ("     (enk_field): field = %p \n",ptr->variant.field.ptr);
               printf ("     name_reference        = %p \n",ptr->variant.field.name_reference);
               if (ptr->variant.field.name_reference != NULL)
                  {
                    summarize_name_reference(ptr->variant.field.name_reference);
                  }
               break;
             }

          case enk_temp_init:
             {
               printf ("     (enk_temp_init): dynamic_init = %p \n",ptr->variant.init.dynamic_init);
               break;
             }

          case enk_new_delete:
             {
               printf ("     (enk_new_delete): new_delete = %p \n",ptr->variant.new_delete);
               break;
             }

       // DQ (7/21/2012): Added support for lambda expressions.
          case enk_lambda:
             {
            // Note: We could call the relatively short summarize_lambda() function...
               printf ("     (enk_lambda): init.source.lambda    = %p \n",ptr->variant.init.source.lambda);
               printf ("     (enk_lambda): init.dynamic_init     = %p \n",ptr->variant.init.dynamic_init);
               break;
             }

#if MICROSOFT_EXTENSIONS_ALLOWED
          case enk_gcnew:
             {
               printf ("     (enk_gcnew): gcnew_info = %p \n",ptr->variant.gcnew_info);
               break;
             }
#endif
          case enk_throw:
             {
               printf ("     (enk_throw): throw_info = %p \n",ptr->variant.throw_info);
               break;
             }

          case enk_condition:
             {
               printf ("     (enk_condition): condition    = %p \n",ptr->variant.condition);
               if (ptr->variant.condition != NULL)
                  {
                    printf ("     (enk_condition): scope        = %p \n",ptr->variant.condition->scope);
                    printf ("     (enk_condition): dynamic_init = %p \n",ptr->variant.condition->dynamic_init);
                    printf ("     (enk_condition): expr         = %p \n",ptr->variant.condition->expr);
                  }

               break;
             }

          case enk_object_lifetime:
             {
               printf ("     (enk_object_lifetime): expr = %p \n",ptr->variant.object_lifetime.expr);
               printf ("     (enk_object_lifetime): ptr  = %p \n",ptr->variant.object_lifetime.ptr);
               break;
             }
          case enk_typeid:
             {
               printf ("     (enk_typeid): type_with_opt_expr = %p \n",ptr->variant.typeid_info.type_with_opt_expr);
               break;
             }
          case enk_alignof:
             {
               printf ("     (enk_alignof): is_type      = %s \n",ptr->variant.sizeof_info.is_type ? "true" : "false");
               printf ("     (enk_alignof): std_alignof  = %s \n",ptr->variant.sizeof_info.is_std_alignof ? "true" : "false");
               if (ptr->variant.sizeof_info.is_type == true)
                    printf ("     (enk_alignof): type    = %p \n",ptr->variant.sizeof_info.variant.type);
                 else
                    printf ("     (enk_alignof): expr    = %p \n",ptr->variant.sizeof_info.variant.expr);
               break;
             }
          case enk_runtime_sizeof:
             {
               printf ("     (enk_runtime_sizeof): is_type   = %s \n",ptr->variant.sizeof_info.is_type ? "true" : "false");
               if (ptr->variant.sizeof_info.is_type == true)
                    printf ("     (enk_runtime_sizeof): type      = %p \n",ptr->variant.sizeof_info.variant.type);
                 else
                    printf ("     (enk_runtime_sizeof): expr      = %p \n",ptr->variant.sizeof_info.variant.expr);
               break;
             }

          case enk_sizeof_pack:
             {
               printf ("     (enk_sizeof_pack): is_type               = %s \n",ptr->variant.sizeof_pack.is_type ? "true" : "false");
               printf ("     (enk_sizeof_pack): is_template_template  = %s \n",ptr->variant.sizeof_pack.is_template_template ? "true" : "false");
               if (ptr->variant.sizeof_pack.is_type == true && ptr->variant.sizeof_pack.is_template_template == false)
                  {
                    printf ("     (enk_sizeof_pack): type                  = %p \n",ptr->variant.sizeof_pack.variant.type);
                  }
                 else
                  {
                    if (ptr->variant.sizeof_pack.is_type == false && ptr->variant.sizeof_pack.is_template_template == false)
                       {
                         printf ("     (enk_sizeof_pack): type                  = %p \n",ptr->variant.sizeof_pack.variant.type);
                       }
                      else
                       {
                         printf ("     (enk_sizeof_pack): templ                 = %p \n",ptr->variant.sizeof_pack.variant.templ);
                       }
                  }
               break;
             }

#if VLA_DEALLOCATIONS_IN_IL
          case enk_vla_dealloc:
             {
               printf ("     (enk_vla_dealloc): vla_variable                  = %p \n",ptr->variant.vla_variable);
               break;
             }
#endif
          case enk_type_operand:
             {
               printf ("     (enk_type_operand): type              = %p \n",ptr->variant.type_operand.type);
               printf ("     (enk_type_operand): field = %p \n",ptr->variant.type_operand.type);
               printf ("     name_reference        = %p \n",ptr->variant.type_operand.name_reference);
               if (ptr->variant.type_operand.name_reference != NULL)
                  {
                    summarize_name_reference(ptr->variant.type_operand.name_reference);
                  }
               break;
             }

          case enk_builtin_operation:
             {
               printf ("     (enk_builtin_operation): kind = %s \n",summarize_builtin_operation_kind_in_string(ptr->variant.builtin_operation.kind).c_str());
               printf ("     operand list                  = %p \n",ptr->variant.builtin_operation.operands);
               an_expr_node_ptr temp_operand = ptr->variant.builtin_operation.operands;
               while (temp_operand != NULL)
                  {
                    printf ("     --- operand                  = %p \n",temp_operand);
                    temp_operand = temp_operand->next;
                  }
               break;
             }

          case enk_param_ref:
             {
               printf ("     (enk_param_ref): param_num = %u \n",ptr->variant.param_ref.param_num);
               printf ("     (enk_param_ref): levels_up = %u \n",ptr->variant.param_ref.levels_up);
               break;
             }

       // DQ (7/24/2014): Added C++11 specific support (incomplete).
          case enk_braced_init_list:
             {
               printf ("     (enk_braced_init_list): braced_init_list = %p \n",ptr->variant.braced_init_list);
               an_expr_node_ptr temp_operand = ptr->variant.braced_init_list;
               while (temp_operand != NULL)
                  {
                    printf ("     (enk_braced_init_list): operands = %p \n",temp_operand);
                    temp_operand = temp_operand->next;
                  }
               break;
             }
          case enk_c11_generic:
             {
               printf ("     (enk_c11_generic): operands = %p \n",ptr->variant.c11_generic.operands);
               printf ("     (enk_c11_generic): result = %p \n",ptr->variant.c11_generic.result);
               break;
             }

          default:
             {
               printf ("     (case not handled ...) kind = %s \n",summarize_expr_kind((an_expr_node_kind)ptr->kind));
             }
        }
#if EXTRA_SOURCE_POSITIONS_IN_IL
     printf ("   expr_range: \n");
     summarize_source_range (ptr->expr_range);
     printf ("   operator_position: \n");
     summarize_source_position (ptr->position);
#endif
     printf ("     extra.rescan_info           = %p \n",ptr->extra.rescan_info);
     printf ("     extra.next_avail            = %p \n",ptr->extra.next_avail);
   }



void
summarize_name_reference ( a_name_reference_ptr ptr )
   {
     ROSE_ASSERT (ptr != NULL);

     printf ("In summarize_name_reference = %p \n",ptr);

     printf ("     next                         = %p \n",ptr->next);
     printf ("     qualifier                    = %p \n",ptr->qualifier);
     printf ("     destructor_type              = %p \n",ptr->variant.destructor_type);\
     printf ("     num_template_arguments       = %ld\n",ptr->num_template_arguments);
     printf ("     is_global_qualified_name     = %s \n",ptr->is_global_qualified_name ? "true" : "false");
     printf ("     is_template_id               = %s \n",ptr->is_template_id ? "true" : "false");
     printf ("     is_super_qualified           = %s \n",ptr->is_super_qualified ? "true" : "false");
#if GENERATE_SOURCE_SEQUENCE_LISTS
     printf ("     used_in_primary_declarator   = %s \n",ptr->used_in_primary_declarator ? "true" : "false");
#endif
     printf ("     from_prototype_instantiation = %s \n",ptr->from_prototype_instantiation ? "true" : "false");

#if 0
  // Avoid the list of all possible references
     if (ptr->next != NULL)
          summarize_name_reference(ptr->next);
#endif
   }


void summarize_statement ( a_statement_ptr ptr )
   {
     ROSE_ASSERT (ptr != NULL);

     printf ("In summarize_statement (%p) \n",ptr);

     printf ("     start_position: \n");
     summarize_source_position (ptr->position);

#if EXTRA_SOURCE_POSITIONS_IN_IL
     printf ("     end_position: \n");
     summarize_source_position (ptr->end_position);
#endif

     printf ("     next                                   = %p \n",ptr->next);
     printf ("     kind                                   = %s \n",summarize_statement_in_string(ptr));

  // DQ (1/3/2014): Added more info to support debugging.
     printf ("     attributes                             = %p \n",ptr->attributes);
     if (ptr->attributes != NULL)
        {
          summarize_attribute(ptr->attributes);
        }

     printf ("     has_associated_pragma                  = %s \n",ptr->has_associated_pragma ? "true" : "false");
     printf ("     is_initialization_guard                = %s \n",ptr->is_initialization_guard ? "true" : "false");
#if !REPRESENT_EMPTY_STATEMENTS_IN_IL
     printf ("     has_associated_pragma                  = %s \n",ptr->has_associated_pragma ? "true" : "false");
#endif

     printf ("     primary test (for this statement) expr = %p \n",ptr->expr);

#if GENERATE_SOURCE_SEQUENCE_LISTS
  // summarize_source_correspondence ( (ptr->source_sequence_entry) )
     printf ("     source_sequence_entry                  = %p \n",ptr->source_sequence_entry);
#endif

     switch(ptr->kind)
        {
          case stmk_if:
             {
               printf ("     true block    = %p \n",ptr->variant.if_stmt.then_statement);
               printf ("     false block   = %p \n",ptr->variant.if_stmt.else_statement);
               printf ("     else_position = \n");
               if (ptr->variant.if_stmt.else_position.seq != 0)
                  {
#if 0
  // DQ (12/30/2008): Removed to support new EDG/Sage/ROSE connection.
                    Sg_File_Info* fileInfo_1 = generateFileInfo(ptr->variant.if_stmt.else_position);
                    fileInfo_1->display("location of else keyword");
                    delete fileInfo_1;
#endif
                  }
                 else
                  {
                    printf ("      --- No direct correspondence in source code for else keyword (ptr->end_position.seq == 0) \n");
                  }
               break;
             }

          case stmk_while:
          case stmk_end_test_while:
             {
               printf ("     loop_statement = %p \n",ptr->variant.loop_statement);
               break;
             }

          case stmk_for:
#if UPC_EXTENSIONS_ALLOWED
          case stmk_upc_forall:
#endif
             {
               printf ("     statement  = %p \n",ptr->variant.for_loop.statement);
               printf ("     extra_info = %p \n",ptr->variant.for_loop.extra_info);
               break;
             }

       // DQ (1/28/2017): Added support for range based for loop.
          case stmk_range_based_for:
             {
               printf ("     range_based_for body       = %p \n",ptr->variant.range_based_for_loop.statement);
               printf ("     range_based_for extra_info = %p \n",ptr->variant.range_based_for_loop.extra_info);
               summarize_range_based_for_loop(ptr->variant.range_based_for_loop.extra_info);
               break;
             }

          case stmk_switch:
             {
               printf ("     switch body block = %p \n",ptr->variant.switch_stmt.body_statement);
               printf ("     switch extra_info = %p \n",ptr->variant.switch_stmt.extra_info);
               break;
             }

       // DQ (10/23/2012): Added case to support debugging of switch statements.
          case stmk_switch_case:
             {
               printf ("     switch case parent statement (associated with this case) = %p \n",ptr->variant.switch_case.switch_statement);
               printf ("     switch case extra_info (about this case)                 = %p \n",ptr->variant.switch_case.extra_info);

               printf ("     calling summarize on switch case extra_info              = %p \n",ptr->variant.switch_case.extra_info);
               summarize_switch_case(ptr->variant.switch_case.extra_info);
               break;
             }

          case stmk_goto:
          case stmk_label:
             {
               printf ("     ptr       = %p \n",ptr->variant.label.ptr);
               printf ("     lifetime  = %p \n",ptr->variant.label.lifetime);

               if (ptr->variant.label.ptr != NULL)
                  {
                    printf ("     calling summarize_label(%p) \n",ptr->variant.label.ptr);
                    summarize_label(ptr->variant.label.ptr);
                  }
               break;
             }

          case stmk_return:
             {
               printf ("     (returns value by calling copy constructor, C++ only) return_dynamic_ini = %p \n",ptr->variant.return_dynamic_init);
               break;
             }

          case stmk_block:
             {
               printf ("     block statements = %p \n",ptr->variant.block.statements);
               printf ("     block extra_info = %p \n",ptr->variant.block.extra_info);

            // DQ (12/23/2013): Added support to output more info on the stmk_block.
               ROSE_ASSERT(ptr->variant.block.extra_info != NULL);
               summarize_block(ptr->variant.block.extra_info);

               break;
             }

          case stmk_init:
             {
               printf ("     dynamic_init = %p \n",ptr->variant.dynamic_init);
               break;
             }

          case stmk_asm:
             {
               printf ("     asm_entry = %p \n",ptr->variant.asm_entry);
               break;
             }

#if ASM_FUNCTION_ALLOWED
          case stmk_asm_func_body:
             {
               printf ("     asm_func_body = %s \n",ptr->variant.asm_func_body);
               break;
             }
#endif

          case stmk_try_block:
             {
               printf ("     try_block = %p \n",ptr->variant.try_block);
               break;
             }

#if MICROSOFT_EXTENSIONS_ALLOWED
          case stmk_microsoft_try:
             {
               printf ("     microsoft_try = %p \n",ptr->variant.microsoft_try);
               break;
             }
#endif

          case stmk_set_vla_size:
             {
               printf ("     vla_dimension = %p \n",ptr->variant.vla_dimension);

            // DQ (2/10/2018): Adding output of vla_dimension information.
               if (ptr->variant.vla_dimension)
                  {
                    summarize_vla_dimension(ptr->variant.vla_dimension);
                  }
               break;
             }

          case stmk_vla_decl:
             {
               printf ("     vla.is_typedef_decl = %s \n",ptr->variant.vla.is_typedef_decl ? "true" : "false");
               if (ptr->variant.vla.is_typedef_decl == true)
                    printf ("     vla.variant.typedef_type = %p \n",ptr->variant.vla.variant.typedef_type);
                 else
                    printf ("     vla.variant.variable = %p \n",ptr->variant.vla.variant.variable);
               break;
             }
          case stmk_expr:
          case stmk_empty:
             {
            // no data members for these cases
               break;
             }

          case stmk_decl:
             {
            // When kind is stmk_decl then the source_sequence_entry points to the first of a series of entries representing declarations.
               printf ("     case stmk_decl: source_sequence_entry = %p \n",ptr->source_sequence_entry);
               a_source_sequence_entry_ptr listElement = ptr->source_sequence_entry;
               while (listElement != NULL)
                  {
                 // printf ("          listElement->entity = %p (kind = %s) \n",listElement,summarize_entry_kind_in_string_kind(listElement->entity.kind));
                    printf ("          listElement->entity = %p (kind = %s) \n",listElement,summarize_entry_kind_in_string_kind(listElement->entity).c_str());
                    listElement = listElement->next;
                  }
               break;
             }

          default:
             {
               printf ("Union not handled in debug code: summarize_statement(%p) \n",ptr);
               ROSE_ASSERT(false);
             }
        }
   }

void
summarize_range_based_for_loop (a_range_based_for_loop_ptr ptr)
   {
     ROSE_ASSERT (ptr != NULL);

     printf ("     In summarize_range_based_for_loop (%p) \n",ptr);

  // printf ("          iterator                  = %p = %s \n",ptr->iterator,(ptr->iterator != NULL) ? summarize_statement_in_string(ptr->iterator) : "NULL");
     printf ("          iterator                  = %p \n",ptr->iterator);
     printf ("          range                     = %p \n",ptr->range);
     printf ("          range_based_for_scope     = %p \n",ptr->range_based_for_scope);\
     printf ("          iterator_scope            = %p \n",ptr->iterator_scope);
     printf ("          begin                     = %p \n",ptr->begin);
     printf ("          end                       = %p \n",ptr->end);
     printf ("          ne_call_expr              = %p \n",ptr->ne_call_expr);
     printf ("          incr_call_expr            = %p \n",ptr->incr_call_expr);
#if COROUTINES_ALLOWED
     printf ("          use_await                 = %p \n",ptr->use_await);
#endif
   }


void
summarize_switch_case (a_switch_case_entry_ptr ptr)
   {
     ROSE_ASSERT (ptr != NULL);

     printf ("     In summarize_switch_case (%p) \n",ptr);

     printf ("          stmt                      = %p = %s \n",ptr->stmt,(ptr->stmt != NULL) ? summarize_statement_in_string(ptr->stmt) : "NULL");
     printf ("          case_value                = %p \n",ptr->case_value);
#if GNU_EXTENSIONS_ALLOWED
     printf ("          range_end                 = %p \n",ptr->range_end);
#endif
     printf ("          next                      = %p \n",ptr->next);
     printf ("          next_on_sorted_list       = %p \n",ptr->next_on_sorted_list);
  // printf ("          position                  = %p \n",ptr->position);
#if EXTRA_SOURCE_POSITIONS_IN_IL
  // printf ("          end_position              = %p \n",ptr->end_position);
  // printf ("          colon_position            = %p \n",ptr->colon_position);
#endif

  // DQ (12/26/2014): Added more source position info to debugging output.
     printf ("     start_position: \n");
     summarize_source_position (ptr->position);
     printf ("     colon_position: \n");
     summarize_source_position (ptr->colon_position);

#if EXTRA_SOURCE_POSITIONS_IN_IL
  // DQ (12/26/2014): Added more source position info to debugging output.
     printf ("     end_position: \n");
     summarize_source_position (ptr->end_position);
#endif

     printf ("          reachable_by_fall_through = %s \n",ptr->reachable_by_fall_through ? "true" : "false");
   }



void
summarize_switch_descriptor (a_switch_stmt_descr_ptr ptr)
   {
     ROSE_ASSERT (ptr != NULL);

     a_switch_case_entry_ptr case_stmt = ptr->cases;

     printf ("In summarize_switch_descriptor (%p) \n",ptr);

#if 0
  // DQ (12/26/2014): Added more source position info to debugging output.
     printf ("     start_position: \n");
     summarize_source_position (ptr->position);

#if EXTRA_SOURCE_POSITIONS_IN_IL
  // DQ (12/26/2014): Added more source position info to debugging output.
     printf ("     end_position: \n");
     summarize_source_position (ptr->end_position);
#endif
#endif

  // Cases in source code order
     printf ("     cases (includes default case) = %p \n",ptr->cases);
     case_stmt = ptr->cases;
     while (case_stmt != NULL)
        {
          summarize_switch_case(case_stmt);
          case_stmt = case_stmt->next;
        }

  // The default case
     printf ("     default_case                  = %p \n",ptr->default_case);
     case_stmt = ptr->default_case;
     while (case_stmt != NULL)
        {
          summarize_switch_case(case_stmt);
          case_stmt = case_stmt->next;
        }
#if 0
  // DQ (10/31/2012): I think this can cause an infinite loop.
  // Cases in sorted order
     printf ("     sorted_cases                  = %p \n",ptr->sorted_cases);
     case_stmt = ptr->sorted_cases;
     while (case_stmt != NULL)
        {
          summarize_switch_case(case_stmt);
       // case_stmt = case_stmt->next_on_sorted_list;
          case_stmt = case_stmt->next_on_sorted_list != NULL ? case_stmt->next_on_sorted_list : case_stmt->next;
        }
#endif
   }




void
summarize_dynamic_init ( a_dynamic_init_ptr ptr )
   {
     ROSE_ASSERT (ptr != NULL);

     printf ("In summarize_dynamic_init (%p) \n",ptr);

     printf ("     next                                               = %p \n",ptr->next);
     printf ("     variable                                           = %p \n",ptr->variable);
     printf ("     destructor                                         = %p \n",ptr->destructor);
     printf ("     lifetime                                           = %p \n",ptr->lifetime);
     printf ("     next_in_destruction_list                           = %p \n",ptr->next_in_destruction_list);
     printf ("     init_expr_lifetime                                 = %p \n",ptr->init_expr_lifetime);
     printf ("     kind (value)                                       = %d \n",ptr->kind);
     printf ("     kind (name)                                        = %s \n",summarizeDynamicInitialization (ptr->kind).c_str());

     printf ("     static_temp                                        = %s \n",ptr->static_temp ? "true" : "false");

     printf ("     follows_an_exec_statement                          = %s \n",ptr->follows_an_exec_statement ? "true" : "false");
     printf ("     inside_conditional_expression                      = %s \n",ptr->inside_conditional_expression ? "true" : "false");
     printf ("     unordered                                          = %s \n",ptr->unordered ? "true" : "false");
     printf ("     has_temporary_lifetime                             = %s \n",ptr->has_temporary_lifetime ? "true" : "false");
     printf ("     is_constructor_init                                = %s \n",ptr->is_constructor_init ? "true" : "false");
     printf ("     is_freeing_of_storage_on_exception                 = %s \n",ptr->is_freeing_of_storage_on_exception ? "true" : "false");
     printf ("     is_array_freeing                                   = %s \n",ptr->is_array_freeing ? "true" : "false");
     printf ("     destruction_is_for_partially_constructed_aggregate = %s \n",ptr->destruction_is_for_partially_constructed_aggregate ? "true" : "false");
#if DO_IL_LOWERING
     printf ("     is_guard_var_for_local_static_var_init             = %s \n",ptr->is_guard_var_for_local_static_var_init ? "true" : "false");
#endif /* DO_IL_LOWERING */
     printf ("     overlaps_temps_in_inner_lifetime                   = %s \n",ptr->overlaps_temps_in_inner_lifetime ? "true" : "false");
#if DO_IL_LOWERING
     printf ("     included_in_slice                                  = %s \n",ptr->included_in_slice ? "true" : "false");
#endif /* DO_IL_LOWERING */
     printf ("     is_explicit_cast                                   = %s \n",ptr->is_explicit_cast ? "true" : "false");

     printf ("     is_compound_literal                                = %s \n",ptr->is_compound_literal ? "true" : "false");

  // DQ (6/10/2013): Modified to support EDG 4.7 frontend.
  // printf ("     is_list_initializer                                = %s \n",ptr->is_list_initializer ? "true" : "false");
     printf ("     is_braced_initializer                              = %s \n",ptr->is_braced_initializer ? "true" : "false");

  // DQ (6/10/2013): Modified to support EDG 4.7 frontend.
  // printf ("     is_partially_initialized_compound_literal          = %s \n",ptr->is_partially_initialized_compound_literal ? "true" : "false");
     printf ("     is_partially_initialized                           = %s \n",ptr->is_partially_initialized ? "true" : "false");

     printf ("     is_result_for_class_rvalue_question_mark           = %s \n",ptr->is_result_for_class_rvalue_question_mark ? "true" : "false");
     printf ("     is_reused_value                                    = %s \n",ptr->is_reused_value ? "true" : "false");
#if DO_IL_LOWERING
     printf ("     is_vla_deallocation                                = %s \n",ptr->is_vla_deallocation ? "true" : "false");
#if GENERATE_EH_TABLES
     printf ("     is_freeing_of_exception_object                     = %s \n",ptr->is_freeing_of_exception_object ? "true" : "false");
#endif
#endif
     printf ("     is_creation_of_initializer_list_object             = %s \n",ptr->is_creation_of_initializer_list_object ? "true" : "false");
     printf ("     is_array_for_initializer_list_object               = %s \n",ptr->is_array_for_initializer_list_object ? "true" : "false");
     printf ("     is_top_temporary_for_constexpr_reference_param     = %s \n",ptr->is_top_temporary_for_constexpr_reference_param ? "true" : "false");

     switch (ptr->kind)
        {
          case dik_none:
          case dik_zero:
          case dik_bitwise_copy:
#if 0
          case dik_constant:
             {
               printf ("          ptr->variant.constant               = %p \n",ptr->variant.constant);
               break;
             }
#endif
          case dik_constant:
          case dik_nonconstant_aggregate:
             {
               printf ("          ptr->variant.constant.ptr           = %p \n",ptr->variant.constant.ptr);
               if (ptr->variant.constant.ptr != NULL)
                  {
                    printf ("          summarize_dynamic_init(): calling summarize_constant(%p) \n",ptr->variant.constant.ptr);
                    summarize_constant(ptr->variant.constant.ptr);
                  }
               break;
             }

          case dik_expression:
          case dik_call_returning_class_via_cctor:
             {
               printf ("          ptr->variant.expression             = %p \n",ptr->variant.expression);
               if (ptr->variant.expression != NULL)
                  {
                    printf ("          In summarize_dynamic_init(): calling summarize_expression () \n");
                    summarize_expr(ptr->variant.expression);
                  }
               break;
             }

          case dik_constructor:
             {
               printf ("          ptr->variant.constructor.ptr                                      = %p \n",ptr->variant.constructor.ptr);
               printf ("          ptr->variant.constructor.args                                     = %p \n",ptr->variant.constructor.args);
               printf ("          ptr->variant.constructor.is_copy_constructor_with_implied_source  = %s \n",ptr->variant.constructor.is_copy_constructor_with_implied_source ? "true" : "false");
               printf ("          ptr->variant.constructor.is_implicit_copy_for_copy_initialization = %s \n",ptr->variant.constructor.is_implicit_copy_for_copy_initialization ? "true" : "false");
               printf ("          ptr->variant.constructor.value_initialization                     = %s \n",ptr->variant.constructor.value_initialization ? "true" : "false");
               printf ("          ptr->variant.constructor.has_sequenced_arguments                  = %s \n",ptr->variant.constructor.has_sequenced_arguments ? "true" : "false");
               break;
             }

          default:
             {
               printf ("Error: default reached in switch kind = %d \n",ptr->kind);
               ROSE_ASSERT(false);
               break;
             }
        }

     printf ("     lifetime_of_overlapping_temps                      = %p \n",ptr->lifetime_of_overlapping_temps);

     printf ("     master_entry                                       = %p \n",ptr->master_entry);
     printf ("     rescan_info                                        = %p \n",ptr->rescan_info);
   }


void summarize_namespace ( a_namespace_ptr ptr )
   {
  /* Description of an EDG a_namespace_ptr */

     ROSE_ASSERT (ptr != NULL);

     printf ("In summarize_namespace (%p) \n",ptr);
     printf ("     Output source_correspondence information: \n");
#ifdef __cplusplus
     summarize_source_correspondence(ptr->source_corresp);
#endif
     printf ("     DONE: Output source_correspondence information: \n");

     printf ("     next                                                 = %p \n",ptr->next);
#if MICROSOFT_EXTENSIONS_ALLOWED
     printf ("     proxy_class                                          = %p \n",ptr->proxy_class);
#endif
     printf ("     is_namespace_alias                                   = %s \n",ptr->is_namespace_alias ? "true" : "false");
     if (ptr->is_namespace_alias == true)
        {
          printf ("     (is_namespace_alias == true) assoc_namespace         = %p \n",ptr->variant.assoc_namespace);
        }
       else
        {
          printf ("     (is_namespace_alias == false) assoc_scope            = %p \n",ptr->variant.assoc_scope);
        }
     printf ("     is_inline                                            = %s \n",ptr->is_inline ? "true" : "false");
     printf ("     named_in_strong_using                                = %s \n",ptr->named_in_strong_using ? "true" : "false");
#if BACK_END_IS_CP_GEN_BE
     printf ("     shadowed_by_class                                    = %s \n",ptr->shadowed_by_class ? "true" : "false");
#endif
   }


void summarize_field ( a_field_ptr ptr )
   {

  /* Description of an EDG a_filed_ptr */

     ROSE_ASSERT (ptr != NULL);

     printf ("In summarize_field (%p) \n",ptr);

#if 0
  // DQ (12/30/2008): Removed to support new EDG/Sage/ROSE connection.
     Sg_File_Info* fileInfo_1 = generateFileInfo(ptr->source_corresp);
     fileInfo_1->display("location of EDG field");
     delete fileInfo_1;
#endif

     printf ("     Output source_correspondence information: \n");
#ifdef __cplusplus
     summarize_source_correspondence(ptr->source_corresp);
#endif

     printf ("     next                                   = %p \n",ptr->next);
     printf ("     type                                   = %p \n",ptr->type);
     printf ("     offset                                 = %d \n",(int)ptr->offset);
     printf ("     offset_bit_remainder                   = %d \n",ptr->offset_bit_remainder);
     printf ("     bit_size                               = %d \n",ptr->bit_size);
#if USER_CONTROL_OF_STRUCT_PACKING
#if GNU_EXTENSIONS_ALLOWED || MICROSOFT_EXTENSIONS_ALLOWED
     printf ("     alignment                              = %d \n",ptr->alignment);
#endif /* GNU_EXTENSIONS_ALLOWED || MICROSOFT_EXTENSIONS_ALLOWED */
#if GNU_EXTENSIONS_ALLOWED
     printf ("     is_packed                              = %s \n",ptr->is_packed ? "true" : "false");
#endif /* GNU_EXTENSIONS_ALLOWED */
#endif /* USER_CONTROL_OF_STRUCT_PACKING */
#if IA64_ABI
     printf ("     offset_is_set                          = %s \n",ptr->offset_is_set ? "true" : "false");
#endif /* IA64_ABI */
     printf ("     is_bit_field                           = %s \n",ptr->is_bit_field ? "true" : "false");
     printf ("     bit_field_is_signed                    = %s \n",ptr->bit_field_is_signed ? "true" : "false");
     printf ("     is_anonymous_parent_object             = %s \n",ptr->is_anonymous_parent_object ? "true" : "false");
     printf ("     is_mutable                             = %s \n",ptr->is_mutable ? "true" : "false");
     printf ("     compiler_generated                     = %s \n",ptr->compiler_generated ? "true" : "false");

     printf ("     is_captured_this                       = %s \n",ptr->is_captured_this ? "true" : "false");
     printf ("     is_captured_pack_element               = %s \n",ptr->is_captured_pack_element ? "true" : "false");
#if MICROSOFT_EXTENSIONS_ALLOWED
     printf ("     is_initonly                            = %s \n",ptr->is_initonly ? "true" : "false");
#endif
#if GNU_EXTENSIONS_ALLOWED
     printf ("     vla_treated_as_zero_length_array       = %s \n",ptr->vla_treated_as_zero_length_array ? "true" : "false");
#endif
#if DO_IL_LOWERING
     printf ("     base_class_subobject_with_tail_padding = %s \n",ptr->base_class_subobject_with_tail_padding ? "true" : "false");
#endif
     printf ("     has_initializer                        = %s \n",ptr->has_initializer ? "true" : "false");
     printf ("     has_direct_braced_initializer          = %s \n",ptr->has_direct_braced_initializer ? "true" : "false");
     printf ("     has_nonconstant_initializer            = %s \n",ptr->has_nonconstant_initializer ? "true" : "false");
     printf ("     initializer                            = %p \n",ptr->initializer);
     printf ("     entities_defined_in_initializer        = %p \n",ptr->entities_defined_in_initializer);
// #if RECORD_CONSTANT_EXPRESSIONS_IN_IL
     printf ("     bit_size_constant                      = %p \n",ptr->bit_size_constant);
// #endif /* RECORD_CONSTANT_EXPRESSIONS_IN_IL */
#if MICROSOFT_EXTENSIONS_ALLOWED
  // printf ("     get_property_name          = %s \n",ptr->get_property_name != NULL ? ptr->get_property_name : "NULL");
  // printf ("     put_property_name          = %s \n",ptr->put_property_name != NULL ? ptr->put_property_name : "NULL");
     printf ("     property_or_event_descr                = %p \n",ptr->property_or_event_descr);
#endif /* MICROSOFT_EXTENSIONS_ALLOWED */
     printf ("     declared_bit_size                      = %d \n",(int)ptr->declared_bit_size);
#if BACK_END_IS_C_GEN_BE
     printf ("     bit_field_alignment_type               = %p \n",ptr->bit_field_alignment_type);
#endif /* BACK_END_IS_C_GEN_BE */
   }



void summarize_asm_entry ( an_asm_entry_ptr ptr )
   {
  /* Description of an EDG an_asm_entry_ptr */

     ROSE_ASSERT (ptr != NULL);

     printf ("In summarize_asm_entry (%p) \n",ptr);
     printf ("     Output source_correspondence information: \n");
#ifdef __cplusplus
     summarize_source_correspondence(ptr->source_corresp);
#endif

     printf ("     next                       = %p \n",ptr->next);
     printf ("     asm_string                 = %p \n",ptr->asm_string);
#if GNU_EXTENSIONS_ALLOWED
#if (VERSION_NUMBER_FOR_MACRO >= 310)
     printf ("     gnu_asm_form               = %s \n",ptr->gnu_asm_form ? "true" : "false");
#endif
     printf ("     is_volatile                = %s \n",ptr->is_volatile ? "true" : "false");
     printf ("     operands                   = %p \n",ptr->operands);
     printf ("     clobbers                   = %p \n",ptr->clobbers);
#endif
   }



char*
summarize_operand_kind ( an_operand_kind kind )
   {
     char* s = NULL;
     switch (kind)
        {
          case ok_error:
               s = (char*)"ok_error";
               break;
          case ok_expression:
               s = (char*)"ok_expression";
               break;
          case ok_constant:
               s = (char*)"ok_constant";
               break;
          case ok_indefinite_function:
               s = (char*)"ok_indefinite_function";
               break;
          case ok_sym_for_member:
               s = (char*)"ok_sym_for_member";
               break;
#if MICROSOFT_EXTENSIONS_ALLOWED
          case ok_property_ref:
               s = (char*)"ok_property_ref";
               break;
#endif
          case ok_undefined_symbol:
               s = (char*)"ok_undefined_symbol";
               break;

          default:
               s = (char*)"**BAD OPERAND KIND**";
        }  /* switch */

     assert(s != NULL);
     return s;
   }


void summarize_operand ( an_operand* ptr )
   {
  /* Description of an EDG an_operand_ptr */

     ROSE_ASSERT (ptr != NULL);

     printf ("In summarize_operand (%p) \n",ptr);

     printf ("     type              = %p \n",ptr->type);
     printf ("     kind              = %d \n",ptr->kind);
     printf ("     kind (string)     = %s \n",summarize_operand_kind (ptr->kind));

     printf ("     state             = %d \n",ptr->state);

     printf ("     bound_function                  = %s \n",ptr->bound_function ? "true" : "false");
     printf ("     virtual_function                = %s \n",ptr->virtual_function ? "true" : "false");
     printf ("     is_qualified_name               = %s \n",ptr->is_qualified_name ? "true" : "false");
     printf ("     is_qualified_name               = %s \n",ptr->is_qualified_name ? "true" : "false");
     printf ("     access_control_error_reported   = %s \n",ptr->access_control_error_reported ? "true" : "false");
     printf ("     is_operand_of_address_of        = %s \n",ptr->is_operand_of_address_of ? "true" : "false");
     printf ("     is_template_id                  = %s \n",ptr->is_template_id ? "true" : "false");
     printf ("     is_simple_string_literal        = %s \n",ptr->is_simple_string_literal ? "true" : "false");
     printf ("     is_cfront_null_pointer_constant = %s \n",ptr->is_cfront_null_pointer_constant ? "true" : "false");
#if MICROSOFT_EXTENSIONS_ALLOWED
     printf ("     is_microsoft_noop               = %s \n",ptr->is_microsoft_noop ? "true" : "false");
#endif
#if RECORD_FORM_OF_NAME_REFERENCE
     printf ("     name_reference_set              = %s \n",ptr->name_reference_set ? "true" : "false");
  // printf ("     name_reference            = %p \n",ptr->name_reference);
#endif

#if 0
     printf ("     position                  = %p \n",ptr->position);
#if EXTRA_SOURCE_POSITIONS_IN_IL
     printf ("     end_position              = %p \n",ptr->end_position);
#endif
#endif

     printf ("     ref_entries_list          = %p \n",ptr->ref_entries_list);
     printf ("     template_arg_list         = %p \n",ptr->template_arg_list);
  // printf ("     id_position               = %p \n",ptr->id_position);

     if (ptr->kind == ok_expression)
          printf ("     variant.expression   = %p \n",ptr->variant.expression);

     if (ptr->kind == ok_constant)
        {
       // printf ("     variant.constant  = %p \n",ptr->variant.constant);
          printf ("     (ptr->kind == ok_constant): Calling summarize_constant() \n");
          summarize_constant(&(ptr->variant.constant));
        }

#if MICROSOFT_EXTENSIONS_ALLOWED
     if (ptr->kind == ok_property_ref)
        {
          printf ("     variant.property_ref.object     = %p \n",ptr->variant.property_ref.object);
       // This field is not available in later versions of EDG.
       // printf ("     variant.property_ref.field      = %p \n",ptr->variant.property_ref.field);
          printf ("     variant.property_ref.subscripts = %p \n",ptr->variant.property_ref.subscripts);
        }
#endif
   }

void
summarize_using_declaration ( a_using_decl_ptr ptr )
   {
  /* Description of an EDG a_using_decl_ptr */

     ROSE_ASSERT (ptr != NULL);

     printf ("In summarize_using_declaration (%p) \n",ptr);

     printf ("     next                    = %p \n",ptr->next);
  // printf ("     --- kind                = %s \n",summarize_entry_kind_in_string_kind(ptr->entity.kind));
     printf ("     --- kind                = %s \n",summarize_entry_kind_in_string_kind(ptr->entity).c_str());
     printf ("     --- ptr                 = %p \n",ptr->entity.ptr);

     printf ("     attributes  - - - - - - = %p \n",ptr->attributes);
     an_attribute_ptr attribute = ptr->attributes;
     while (attribute != NULL)
        {
          summarize_attribute(attribute);
          attribute = attribute->next;
        }

     printf ("     is_using_directive      = %s \n",ptr->is_using_directive ? "true" : "false");
     printf ("     is_class_member         = %s \n",ptr->is_class_member ? "true" : "false");
     printf ("     is_inheriting_ctor      = %s \n",ptr->is_inheriting_ctor ? "true" : "false");
     printf ("     hidden                  = %s \n",ptr->hidden ? "true" : "false");
     printf ("     compiler_generated      = %s \n",ptr->compiler_generated ? "true" : "false");
     printf ("     inline_namespace        = %s \n",ptr->inline_namespace ? "true" : "false");
     printf ("     strong                  = %s \n",ptr->strong ? "true" : "false");
     printf ("     is_pack_expansion       = %s \n",ptr->is_pack_expansion ? "true" : "false");
     printf ("     is_representative       = %s \n",ptr->is_representative ? "true" : "false");
     printf ("     is_using_enum           = %s \n",ptr->is_using_enum ? "true" : "false");
     printf ("     is_enumerator           = %s \n",ptr->is_enumerator ? "true" : "false");

     printf ("     access                  = %d \n",ptr->access);

  // printf ("     qualifier               = %s \n",ptr->qualifier ? "true" : "false");
     if (ptr->is_class_member)
        {
          printf ("     qualifier.class         = %p \n",ptr->qualifier.class_type);
        }
       else
        {
          printf ("     qualifier.namespace_ptr = %p \n",ptr->qualifier.namespace_ptr);
        }

     printf ("     decl_sequence_number    = %lu \n",ptr->decl_sequence_number);
#if GENERATE_SOURCE_SEQUENCE_LISTS
     printf ("     source_sequence_entry   = %p \n",ptr->source_sequence_entry);
     printf ("     --- next_in_set         = %p \n",ptr->next_in_set);
#endif

  // DQ (9/20/2022): Added output of source position information.
     summarize_source_position (ptr->position);
   }

void summarize_local_scope_ref ( a_local_scope_ref_ptr input_ptr )
   {
  // DQ (1/30/2018): Added support for finding scopes of declarations when the parent_scope is NULL.

     ROSE_ASSERT (input_ptr != NULL);

     printf ("In summarize_local_scope_ref (%p) \n",input_ptr);

     a_local_scope_ref_ptr ptr = input_ptr;
     while (ptr != NULL)
        {
          printf ("     next (local scope ref)         = %p \n",ptr->next);
          printf ("     scope                          = %p \n",ptr->scope);
          printf ("     referrer.ptr                   = %p \n",ptr->referrer.ptr);

          if (ptr->referrer.ptr != NULL)
             {
               printf ("     ---- ---- ---- local_scope_ref: referrer.kind (value)  = %d \n",ptr->referrer.kind);
               printf ("     ---- ---- ---- local_scope_ref: referrer.kind (string) = %s \n",summarize_entry_kind_in_string_kind(ptr->referrer.kind));
               printf ("     ---- ---- ---- local_scope_ref: referrer.ptr = %p \n",ptr->referrer.ptr);

            // std::string summarize_entry_kind_in_string_kind (const a_tagged_pointer & entry, bool includePointerValue = true );
               printf ("     ---- ---- ---- local_scope_ref: referrer (alt) = %s \n",summarize_entry_kind_in_string_kind(ptr->referrer,true).c_str());
             }

          ptr = ptr->next;
        }
   }



void summarize_scope ( a_scope_ptr ptr )
   {
     ROSE_ASSERT (ptr != NULL);

     printf ("In summarize_scope (%p) \n",ptr);

     printf ("     next (scope)                          = %p \n",ptr->next);
     printf ("     parent (scope)                        = %p \n",ptr->parent);
     printf ("     number (scope)                        = %d \n",ptr->number);
     printf ("     kind                                  = %s \n",summarize_scope_in_string(ptr));
     printf ("     function_body_processing_finished     = %s \n",ptr->function_body_processing_finished ? "true" : "false");
#if SCOPE_ORPHANED_LIST_PROCESSING_NEEDED
     printf ("     scope_orphaned_list_header_generated  = %s \n",ptr->scope_orphaned_list_header_generated ? "true" : "false");
#endif
     printf ("     is_constexpr_routine                  = %s \n",ptr->is_constexpr_routine ? "true" : "false");

     switch (ptr->kind)
        {
          case sck_file:
             {
            // No variant fields
               break;
             }

          case sck_template_declaration:
             {
            // No variant fields
               break;
             }

          case sck_block:
             {
               printf ("     assoc_handler             = %p \n",ptr->variant.assoc_handler);
               break;
             }

          case sck_func_prototype:
          case sck_class_struct_union:
          case sck_class_reactivation:
          case sck_enum:
             {
               printf ("     assoc_type                = %p \n",ptr->variant.assoc_type);
               break;
             }

          case sck_condition:
             {
               printf ("     assoc_statement           = %p \n",ptr->variant.assoc_statement);
               break;
             }

          case sck_namespace:
             {
               printf ("     assoc_namespace           = %p \n",ptr->variant.assoc_namespace);
               break;
             }

          case sck_function:
             {
               printf ("     variant.routine.ptr                           = %p \n",ptr->variant.routine.ptr);
               printf ("     variant.routine.parameters                    = %p \n",ptr->variant.routine.parameters);
               printf ("     variant.routine.constructor_inits             = %p \n",ptr->variant.routine.constructor_inits);
               printf ("     variant.routine.lifetime_of_local_static_vars = %p \n",ptr->variant.routine.lifetime_of_local_static_vars);
               printf ("     variant.routine.this_param_variable           = %p \n",ptr->variant.routine.this_param_variable);
               printf ("     variant.routine.return_value_variable         = %p \n",ptr->variant.routine.return_value_variable);
               if (ptr->is_constexpr_routine == true)
                  {
                    printf ("     is_constexpr_routine = true");
                  }
               break;
             }

          default:
             {
               printf ("Default reached in switch kind unknown \n");
               ROSE_ASSERT(false);
             }
        }
     printf ("     assoc_block                 = %p \n",ptr->assoc_block);
     printf ("     lifetime                    = %p \n",ptr->lifetime);
     printf ("     constants                   = %p \n",ptr->constants);
     printf ("     types                       = %p \n",ptr->types);

#if 1
  // DQ (2/14/2016): Extra debugging code.
     a_type_ptr temp_type = ptr->types;
     while(temp_type != NULL )
        {
          string s = summarize_type_kind_in_string(temp_type);
          printf ("        --- type kind is: %s \n",s.c_str());
          temp_type = temp_type->next;
        }
#endif

     printf ("     variables                   = %p \n",ptr->variables);
#if 0
  // DQ (2/14/2016): Extra debugging code.
     a_variable_ptr temp_variable = ptr->variables;
     while(temp_variable != NULL )
        {
       // string s = summarize_type_kind_in_string(temp_variable);
          string s = temp_variable->source_corresp.name;
          printf ("        --- variable name is: %s \n",s.c_str());
          temp_variable = temp_variable->next;
        }
#endif
     printf ("     nonstatic_variables         = %p \n",ptr->nonstatic_variables);
     printf ("     labels                      = %p \n",ptr->labels);
     printf ("     routines                    = %p \n",ptr->routines);

#if 0
  // DQ (1/26/2019): This seems to be an issue with Cxx11_tests/test2019_29.C.
  // DQ (2/14/2016): Extra debugging code.
     a_routine_ptr temp_routine = ptr->routines;
     while(temp_routine != NULL )
        {
          ROSE_ASSERT(temp_routine != NULL);
       // ROSE_ASSERT(temp_routine->source_corresp.name != NULL);
#if 1
          if (temp_routine->source_corresp.name != NULL)
             {
               printf ("In loop over routines: get the name of the routine: temp_routine->source_corresp.name = %p \n",temp_routine->source_corresp.name);
             }
#endif
       // DQ (2/12/2018): In some cases this can be NULL (lambda functions, I think).
          if (temp_routine->source_corresp.name != NULL)
             {
               string s = temp_routine->source_corresp.name;
               printf ("        --- routine name is: %s \n",s.c_str());
             }
            else
             {
               printf ("Note: In loop over routines: temp_routine->source_corresp.name == NULL \n");
             }
#if 1
          printf ("In loop over routines: temp_routine = %p \n",temp_routine);
#endif
          temp_routine = temp_routine->next;
#if 1
          printf ("In loop over routines: temp_routine = %p \n",temp_routine);
#endif
        }
#endif

#if 0
     printf ("After loop over routines: temp_routine = %p \n",temp_routine);
#endif

     printf ("     asm_entries                 = %p \n",ptr->asm_entries);
     printf ("     scopes                      = %p \n",ptr->scopes);
     printf ("     namespaces                  = %p \n",ptr->namespaces);
     printf ("     using_declarations          = %p \n",ptr->using_declarations);
     printf ("     dynamic_inits               = %p \n",ptr->dynamic_inits);
     printf ("     local_static_variable_inits = %p \n",ptr->local_static_variable_inits);
     printf ("     vla_dimensions              = %p \n",ptr->vla_dimensions);
#if DO_IL_LOWERING
  #error "We don't want to use IL lowering in EDG."
#endif
     if (ptr->vla_dimensions != NULL)
        {
          printf ("     iterate over available vla_dimensions entries: \n");
          a_vla_dimension_ptr vla_entry = ptr->vla_dimensions;
          while (vla_entry != NULL)
             {
               printf ("     ---- vla_entry               = %p \n",vla_entry);
               printf ("     ---- vla: type               = %p \n",vla_entry->type);
               printf ("     ---- vla: dimension_expr     = %p \n",vla_entry->dimension_expr);
               printf ("     ---- vla: original_dimension = %p \n",vla_entry->original_dimension);
               printf ("     ---- vla: in_prototype_scope = %s \n",vla_entry->in_prototype_scope ? "true" : "false");

               vla_entry = vla_entry->next;
             }
        }

     printf ("     expr_node_refs              = %p \n",ptr->expr_node_refs);

  // DQ (4/5/2015): Added support to find expressions used in typeof() operators.
     if (ptr->expr_node_refs != NULL)
        {
          a_local_expr_node_ref_ptr local_expr_node_ref_ptr = ptr->expr_node_refs;
          printf ("     ---- local_expr_node_refs = %p \n",local_expr_node_ref_ptr);
          while (local_expr_node_ref_ptr != NULL)
             {
               printf ("     ---- ---- local_expr_node_ref: expr          = %p \n",local_expr_node_ref_ptr->expr);
               printf ("     ---- ---- local_expr_node_ref: kind (value)  = %d \n",(int)local_expr_node_ref_ptr->kind);
               printf ("     ---- ---- local_expr_node_ref: kind (string) = %s \n",summarize_local_expr_node_ref_string(local_expr_node_ref_ptr->kind).c_str());
               printf ("     ---- ---- local_expr_node_ref: referrer.ptr  = %p \n",local_expr_node_ref_ptr->referrer.ptr);
               if (local_expr_node_ref_ptr->referrer.ptr != NULL)
                  {
                    printf ("     ---- ---- ---- local_expr_node_ref: referrer.kind (value)  = %d \n",local_expr_node_ref_ptr->referrer.kind);
                    printf ("     ---- ---- ---- local_expr_node_ref: referrer.kind (string) = %s \n",summarize_entry_kind_in_string_kind(local_expr_node_ref_ptr->referrer.kind));
                    printf ("     ---- ---- ---- local_expr_node_ref: referrer.ptr = %p \n",local_expr_node_ref_ptr->referrer.ptr);

                 // std::string summarize_entry_kind_in_string_kind (const a_tagged_pointer & entry, bool includePointerValue = true );
                    printf ("     ---- ---- ---- local_expr_node_ref: referrer (alt) = %s \n",summarize_entry_kind_in_string_kind(local_expr_node_ref_ptr->referrer,true).c_str());

                  }

               local_expr_node_ref_ptr = local_expr_node_ref_ptr->next;
             }
        }

     printf ("     scope_refs                  = %p \n",ptr->scope_refs);
     if (ptr->scope_refs != NULL)
        {
          summarize_local_scope_ref(ptr->scope_refs);
        }

     printf ("     pragmas                     = %p \n",ptr->pragmas);
     printf ("     depth_in_scope_stack        = %d \n",ptr->depth_in_scope_stack);
#if GENERATE_SOURCE_SEQUENCE_LISTS
     printf ("     source_sequence_list        = %p \n",ptr->source_sequence_list);
     printf ("     src_seq_sublist_list        = %p \n",ptr->src_seq_sublist_list);
#endif
#if RECORD_HIDDEN_NAMES_IN_IL
     printf ("     hidden_names                = %p \n",ptr->hidden_names);
#endif
     printf ("     templates                   = %p \n",ptr->templates);
#if MICROSOFT_EXTENSIONS_ALLOWED
     printf ("     ms_attributes               = %p \n",ptr->ms_attributes);
#endif
#if GENERATE_MICROSOFT_IF_EXISTS_ENTRIES
     printf ("     ms_if_exists                = %p \n",ptr->ms_if_exists);
#endif
   }

void
summarize_param_type ( a_param_type_ptr ptr )
   {
  /* Description of an EDG a_param_type_ptr */

     ROSE_ASSERT (ptr != NULL);

     printf ("In summarize_param_type (%p) \n",ptr);

     printf ("     next (a_param_type_ptr)                          = %p \n",ptr->next);

     printf ("     name                                             = %s \n",ptr->name);
     printf ("     type                                             = %p \n",ptr->type);

#if 0
  // DQ (2/10/2018): Looking for the VLA information.
     if (ptr->type != NULL)
        {
          printf ("In summarize_param_type (%p): calling summarize_type(ptr->type) \n",ptr);
          summarize_type(ptr->type);
          printf ("DONE: In summarize_param_type (%p): calling summarize_type(ptr->type) \n",ptr);
        }
#endif

     printf ("     declared_type                                    = %p \n",ptr->declared_type);

#if 0
  // DQ (2/10/2018): Looking for the VLA information.
     if (ptr->declared_type != NULL)
        {
          if (ptr->declared_type == ptr->type)
             {
               printf ("In summarize_param_type (%p): ptr->declared_type == ptr->type \n",ptr);
             }
            else
             {
               printf ("In summarize_param_type (%p): ptr->declared_type != ptr->type: calling summarize_type(ptr->declared_type) \n",ptr);
               summarize_type(ptr->declared_type);
               printf ("DONE: In summarize_param_type (%p): ptr->declared_type != ptr->type: calling summarize_type(ptr->declared_type) \n",ptr);
             }
        }
#endif

     printf ("     passed_via_copy_constructor                      = %s \n",ptr->passed_via_copy_constructor ? "true" : "false");
     printf ("     has_default_arg                                  = %s \n",ptr->has_default_arg ? "true" : "false");
     printf ("     default_arg_appeared_in_class_definition         = %s \n",ptr->default_arg_appeared_in_class_definition ? "true" : "false");
     printf ("     has_unevaluated_template_default                 = %s \n",ptr->has_unevaluated_template_default ? "true" : "false");
     printf ("     default_being_instantiated                       = %s \n",ptr->default_being_instantiated ? "true" : "false");
     printf ("     type_involves_deduced_template_param             = %s \n",ptr->type_involves_deduced_template_param ? "true" : "false");
     printf ("     type_involves_template_param                     = %s \n",ptr->type_involves_template_param ? "true" : "false");
     printf ("     is_parameter_pack                                = %s \n",ptr->is_parameter_pack ? "true" : "false");
     printf ("     is_pack_element                                  = %s \n",ptr->is_pack_element ? "true" : "false");
     printf ("     qualifiers                                       = %" PRIuPTR " \n",(size_t)ptr->qualifiers);
     printf ("     qualifiers (summary)                             = %s \n",summarize_qualifiers_in_string((size_t)(ptr->qualifiers)).c_str());
#if GNU_EXTENSIONS_ALLOWED
     printf ("     is_transparent                                   = %s \n",ptr->is_transparent ? "true" : "false");
     printf ("     nonnull                                          = %s \n",ptr->nonnull ? "true" : "false");
#endif
     printf ("     duplicate_name                                   = %s \n",ptr->duplicate_name ? "true" : "false");
#if MICROSOFT_EXTENSIONS_ALLOWED
     printf ("     is_cli_param_array                               = %s \n",ptr->is_cli_param_array ? "true" : "false");
#endif
     printf ("     move_ctor_or_assign_parameter                    = %s \n",ptr->move_ctor_or_assign_parameter ? "true" : "false");
     printf ("     param_num                                        = %" PRIuPTR " \n",(size_t)ptr->param_num);
     printf ("     default_arg_expr                                 = %p \n",ptr->default_arg_expr);
     printf ("     orig_param_type_for_unevaluated_default_arg_expr = %p \n",ptr->orig_param_type_for_unevaluated_default_arg_expr);
     printf ("     entities_defined_in_default_arg                  = %p \n",ptr->entities_defined_in_default_arg);

#if 0
  // Refactor as a function instead.
#else
  // A list of tagged pointers to the entities declared by this statement.
     an_il_entity_list_entry_ptr entities = ptr->entities_defined_in_default_arg;

  // DQ (10/16/2012): List the different kinds of entities declared in this statement.
     if (entities != NULL)
        {
          while (entities != NULL)
             {
               a_tagged_pointer entity = entities->entity;
               printf ("        --- entities = %p \n",entities);
            // printf ("           --- entity.kind = %d = %s \n",entity.kind,summarize_entry_kind_in_string_kind((an_il_entry_kind)(entity.kind)));
               printf ("           --- entity.kind = %d = %s \n",entity.kind,summarize_entry_kind_in_string_kind(entity).c_str());
               printf ("           --- entity.ptr  = %p \n",entity.ptr);
               entities = entities->next;
             }
        }
#endif

     printf ("     attributes  - - - - - - - - - - - - - - - - - -  = %p \n",ptr->attributes);
     an_attribute_ptr attribute = ptr->attributes;
     while (attribute != NULL)
        {
          summarize_attribute(attribute);
          attribute = attribute->next;
        }

#if MICROSOFT_EXTENSIONS_ALLOWED
     printf ("     ms_attributes                                    = %p \n",ptr->ms_attributes);
#endif
#if EXTRA_SOURCE_POSITIONS_IN_IL
     printf ("     decl_pos_info                                    = %p \n",ptr->decl_pos_info);
#endif
     printf ("     pack_expansion_descr                             = %p \n",ptr->pack_expansion_descr);

  // DQ (2/10/2018): Help identify the end fo the output from this function.
     printf ("Leaving summarize_param_type (%p) \n",ptr);
   }


void
summarize_lambda_expr (a_lambda_ptr ptr)
   {
     ROSE_ASSERT (ptr != NULL);

     printf ("In summarize_lambda_expr (%p) \n",ptr);

     printf ("     capture_list                             = %p \n",ptr->capture_list);
     a_lambda_capture_ptr capture_list_ptr = ptr->capture_list;
     while(capture_list_ptr != NULL)
        {
           summarize_lambda_capture(capture_list_ptr);
           capture_list_ptr = capture_list_ptr->next;
        }

     printf ("     closure_class                            = %p \n",ptr->closure_class);
     printf ("     lambda_routine                           = %p \n",ptr->lambda_routine);

     printf ("     is_mutable                               = %s \n",ptr->is_mutable ? "true" : "false");
     printf ("     has_capture_default                      = %s \n",ptr->has_capture_default ? "true" : "false");
     printf ("     default_is_by_reference                  = %s \n",ptr->default_is_by_reference ? "true" : "false");
     printf ("     explicit_return_type                     = %s \n",ptr->explicit_return_type ? "true" : "false");
     printf ("     has_parameter_decl                       = %s \n",ptr->has_parameter_decl ? "true" : "false");

     printf ("     start_position: \n");
     summarize_source_position (ptr->start_position);
#if EXTRA_SOURCE_POSITIONS_IN_IL
     printf ("     capture_end_position: \n");
     summarize_source_position (ptr->capture_end_position);
     printf ("     mutable_position: \n");
     summarize_source_position (ptr->mutable_position);
#endif
   }

void
summarize_lambda_capture (a_lambda_capture_ptr ptr)
   {
     ROSE_ASSERT (ptr != NULL);

     printf ("     In summarize_lambda_capture (%p) \n",ptr);
     printf ("          variable                     = %p \n",ptr->captured.variable);
     printf ("          source_closure_field         = %p \n",ptr->capture_info.source_closure_field);
     printf ("          closure_field                = %p \n",ptr->closure_field);
     printf ("          capture_by_reference         = %s \n",ptr->capture_by_reference ? "true" : "false");
     printf ("          is_implicit                  = %s \n",ptr->is_implicit ? "true" : "false");
     printf ("          is_pack_expansion            = %s \n",ptr->is_pack_expansion ? "true" : "false");

     printf ("     position: \n");
     summarize_source_position (ptr->position);
#if EXTRA_SOURCE_POSITIONS_IN_IL
     printf ("     end_position: \n");
     summarize_source_position (ptr->end_position);
#endif
   }



void
summarize_template_arg_list (a_template_arg_ptr arg_list)
   {
     ROSE_ASSERT (arg_list != NULL);

     printf ("                  In summarize_template_arg_list (%p) \n",arg_list);

     a_template_arg_ptr arg = arg_list;

     while (arg != NULL)
        {
          summarize_template_arg(arg);

          arg = arg->next;
        }
   }


char*
summarize_templ_arg_kind_in_string(a_templ_arg_kind kind)
   {
     char* s = NULL;

  // printf ("In summarize_templ_arg_kind_in_string(kind = %d) \n",kind);

     switch (kind)
        {
          case tak_type:                    s = (char*)"tak_type";                    break;
          case tak_nontype:                 s = (char*)"tak_nontype";                 break;
          case tak_template:                s = (char*)"tak_template";                break;
          case tak_start_of_pack_expansion: s = (char*)"tak_start_of_pack_expansion"; break;

          default: s = (char*)"**BAD templ_arg_kind**";
        }

     assert (s != NULL);

     return s;
   }


void
summarize_template_arg (a_template_arg_ptr ptr)
   {
     ROSE_ASSERT (ptr != NULL);
     printf ("                  In summarize_template_arg (%p) \n",ptr);

     printf ("                       kind                            = %s \n",summarize_templ_arg_kind_in_string(ptr->kind));
     printf ("                       pack_expansion_descr            = %p \n",ptr->pack_expansion_descr);
     if (ptr->pack_expansion_descr != NULL)
        {
          summarize_pack_expansion_descr (ptr->pack_expansion_descr);
        }
     printf ("                       is_array_bound_of_unknown_type  = %s \n",ptr->is_array_bound_of_unknown_type ? "true" : "false");
     printf ("                       explicitly_specified            = %s \n",ptr->explicitly_specified ? "true" : "false");
     printf ("                       template_template_param_checked = %s \n",ptr->template_template_param_checked ? "true" : "false");
     printf ("                       is_pack_element                 = %s \n",ptr->is_pack_element ? "true" : "false");
     printf ("                       is_pack                         = %s \n",ptr->is_pack ? "true" : "false");
     if (ptr->kind == tak_type)
        {
          printf ("                       type                            = %p \n",ptr->variant.type);
          printf ("                       type kind                       = %s \n",summarize_type_kind_in_string_kind(ptr->variant.type->kind));
#if 0
       // DQ (9/7/2014): Investigate this to see if we can distinguish between template parameters specified using "typename" and "class".
       // Note that we normally don't want to output this level of detail.
          if (ptr->variant.type != NULL)
             {
               printf ("@@@@@@@@@@@@@@ START: Investigate this to see if we can distinguish between template parameters specified using typename and class \n");
               summarize_type(ptr->variant.type);
               printf ("@@@@@@@@@@@@@@ DONE: Investigate this to see if we can distinguish between template parameters specified using typename and class \n");
             }
#endif
        }
     if (ptr->kind == tak_nontype && ptr->is_array_bound_of_unknown_type == false)
        {
          printf ("                       constant                        = %p \n",ptr->variant.constant);
#if 1
          printf ("calling summarize_constant(%p) \n",ptr->variant.constant);
          printf ("**************** start of summarize_constant() ******************* \n");
          summarize_constant(ptr->variant.constant);
          printf ("***************** end of summarize_constant() ******************** \n");
#endif
        }
     if (ptr->kind == tak_nontype && ptr->is_array_bound_of_unknown_type == true)
        {
          printf ("                       integer_value                   = %d \n",(int)ptr->variant.integer_value);
        }
     if (ptr->kind == tak_template)
        {
          printf ("                       ptr                        = %p \n",ptr->variant.templ.ptr);
          printf ("                       substituted_param_template = %p \n",ptr->variant.templ.substituted_param_template);
        }
     printf ("                       arg_operand                = %p \n",ptr->arg_operand);
   }



void
summarize_pack_expansion_descr (a_pack_expansion_descr_ptr ptr)
   {
     printf ("                  In summarize_pack_expansion_descr (%p) \n",ptr);

     printf ("                       next                      = %p \n",ptr->next);
     printf ("                       previous                  = %p \n",ptr->previous);
     printf ("                       first_token               = %d \n",ptr->first_token);
     printf ("                       last_token                = %d \n",ptr->last_token);
     printf ("                       packs_referenced          = %p \n",ptr->packs_referenced);

  // a_source_position ellipsis_position;
     summarize_source_position (ptr->ellipsis_position);

     printf ("                       ellipsis_seen             = %s \n",ptr->ellipsis_seen ? "true" : "false");
     printf ("                       is_function_declarator    = %s \n",ptr->is_function_declarator ? "true" : "false");
     printf ("                       uses_only_enclosing_packs = %s \n",ptr->uses_only_enclosing_packs ? "true" : "false");
   }

extern std::list<SgScopeStatement*> SageBuilder::ScopeStack;

void
printScopeStack( std::string label )
   {
     std::list<SgScopeStatement*> tempScopeStack = SageBuilder::ScopeStack;

     printf ("printScopeStack(): label = %s \n",label.c_str());

     std::list<SgScopeStatement*>::iterator i = tempScopeStack.begin();
     while (i != tempScopeStack.end())
        {
          printf ("--- scope = %p = %s \n",*i,(*i)->class_name().c_str());
          i++;
        }
   }



void
summarize_label (a_label_ptr ptr)
   {
     ROSE_ASSERT (ptr != NULL);

  // ROSE_ASSERT(ptr->source_corresp.name != NULL);
     printf ("   Inside of summarize_label (name = %s) \n",ptr->source_corresp.name != NULL ? ptr->source_corresp.name : "null");

#ifdef __cplusplus
     summarize_source_correspondence ( ptr->source_corresp );
#endif

     printf ("     next                          = %p \n",ptr->next);
     printf ("     reachable_by_fall_through     = %s \n",ptr->reachable_by_fall_through ? "true" : "false");
     printf ("     break_label                   = %s \n",ptr->break_label ? "true" : "false");
     printf ("     switch_break_label            = %s \n",ptr->switch_break_label ? "true" : "false");
     printf ("     continue_label                = %s \n",ptr->continue_label ? "true" : "false");
#if MICROSOFT_EXTENSIONS_ALLOWED
     printf ("     leave_label                   = %s \n",ptr->leave_label ? "true" : "false");
#endif
#if GNU_EXTENSIONS_ALLOWED
     printf ("     address_taken                 = %s \n",ptr->address_taken ? "true" : "false");
     printf ("     locally_declared              = %s \n",ptr->locally_declared ? "true" : "false");
#endif
     printf ("     exec_stmt                     = %p \n",ptr->exec_stmt);
#if MICROSOFT_EXTENSIONS_ALLOWED
     printf ("     num_microsoft_trys_inside_of  = %lu \n",ptr->num_microsoft_trys_inside_of);
#endif
   }




void
summarize_block (a_block_ptr ptr)
   {
     ROSE_ASSERT (ptr != NULL);

     printf ("   Inside of summarize_block (ptr = %p) \n",ptr);

     printf ("     assoc_scope                  = %p \n",ptr->assoc_scope);
     printf ("     lifetime                     = %p \n",ptr->lifetime);
     printf ("     end_of_block_reachable       = %s \n",ptr->end_of_block_reachable ? "true" : "false");
#if GNU_EXTENSIONS_ALLOWED
     printf ("     is_statement_expression      = %s \n",ptr->is_statement_expression ? "true" : "false");
#endif
     printf ("     implicit_scope_not_allowed   = %s \n",ptr->implicit_scope_not_allowed ? "true" : "false");
#if UPC_EXTENSIONS_ALLOWED
     printf ("     upc_access_method            = %d \n",ptr->upc_access_method);
#endif
   }



char*
summarize_pragma_kind_in_string (a_pragma_kind kind)
   {
     char* s = NULL;
     switch (kind)
        {
          case pk_none:
               s = (char*)"pk_none";
               break;

          case pk_printf_args:
               s = (char*)"pk_printf_args";
               break;

          case pk_scanf_args:
               s = (char*)"pk_scanf_args";
               break;

          case pk_lint_argsused:
               s = (char*)"pk_lint_argsused";
               break;

          case pk_lint_varargs_count:
               s = (char*)"pk_lint_varargs_count";
               break;

          case pk_lint_notreached:
               s = (char*)"pk_lint_notreached";
               break;

          case pk_instantiate:
               s = (char*)"pk_instantiate";
               break;

          case pk_do_not_instantiate:
               s = (char*)"pk_do_not_instantiate";
               break;

          case pk_can_instantiate:
               s = (char*)"pk_can_instantiate";
               break;

          case pk_inline_template:
               s = (char*)"pk_inline_template";
               break;

#if USER_CONTROL_OF_STRUCT_PACKING
          case pk_pack:
               s = (char*)"pk_pack";
               break;
#endif
#if IDENT_DIRECTIVE_AND_PRAGMA
          case pk_ident:
               s = (char*)"pk_ident";
               break;
#endif
#if PRAGMA_WEAK_ALLOWED
          case pk_weak:
               s = (char*)"pk_weak";
               break;
#endif
          case pk_once:
               s = (char*)"pk_once";
               break;

          case pk_hdrstop:
               s = (char*)"pk_hdrstop";
               break;

          case pk_no_pch:
               s = (char*)"pk_no_pch";
               break;

          case pk_define_type_info:
               s = (char*)"pk_define_type_info";
               break;

          case pk_stdc:
               s = (char*)"pk_stdc";
               break;

#if UPC_EXTENSIONS_ALLOWED
          case pk_upc:
               s = (char*)"pk_upc";
               break;
#endif
#if REDEFINE_EXTNAME_PRAGMA_ENABLED
          case pk_redefine_extname:
               s = (char*)"pk_redefine_extname";
               break;
#endif
#if SUN_EXTENSIONS_ALLOWED
          case pk_enable_ldscope:
               s = (char*)"pk_enable_ldscope";
               break;

          case pk_disable_ldscope:
               s = (char*)"pk_disable_ldscope";
               break;
#endif
#if GNU_EXTENSIONS_ALLOWED
          case pk_gcc_immediate:
               s = (char*)"pk_gcc_immediate";
               break;
          case pk_gcc_next_token:
               s = (char*)"pk_gcc_next_token";
               break;
#endif
          case pk_diag_suppress:
               s = (char*)"pk_diag_suppress";
               break;

          case pk_diag_remark:
               s = (char*)"pk_diag_remark";
               break;

          case pk_diag_warning:
               s = (char*)"pk_diag_warning";
               break;

          case pk_diag_error:
               s = (char*)"pk_diag_error";
               break;

          case pk_diag_once:
               s = (char*)"pk_diag_once";
               break;

          case pk_diag_default:
               s = (char*)"pk_diag_default";
               break;

#if INCLUDE_EDG_TEST_PRAGMAS
          case pk_test_next_statement:
               s = (char*)"pk_test_next_statement";
               break;

          case pk_test_next_decl:
               s = (char*)"pk_test_next_decl";
               break;

          case pk_test_immediate:
               s = (char*)"pk_test_immediate";
               break;

          case pk_test_immediate_text:
               s = (char*)"pk_test_immediate_text";
               break;

          case pk_test_immediate_pp_text:
               s = (char*)"pk_test_immediate_pp_text";
               break;

          case pk_test_other:
               s = (char*)"pk_test_other";
               break;

          case pk_test_bind_next_pass:
               s = (char*)"pk_test_bind_next_pass";
               break;
#endif
#if ADD_CHECKING_PRAGMAS_FOR_INTERNAL_TESTING
          case pk_checking_pragma:
               s = (char*)"pk_checking_pragma";
               break;
#endif
#if DEBUG
          case pk_db_opt:
               s = (char*)"pk_db_opt";
               break;

          case pk_db_name:
               s = (char*)"pk_db_name";
               break;
#endif
#if GENERATE_MICROSOFT_IF_EXISTS_ENTRIES
          case pk_if_exists:
               s = (char*)"pk_if_exists";
               break;
#endif
#if MICROSOFT_EXTENSIONS_ALLOWED
          case pk_push_macro:
               s = (char*)"pk_push_macro";
               break;

          case pk_pop_macro:
               s = (char*)"pk_pop_macro";
               break;

          case pk_start_map_region:
               s = (char*)"pk_start_map_region";
               break;

          case pk_stop_map_region:
               s = (char*)"pk_stop_map_region";
               break;

#if NATIVE_MULTIBYTE_CHARS_SUPPORTED_WITH_UNICODE
          case pk_setlocale:
               s = (char*)"pk_setlocale";
               break;
#endif
          case pk_comment:
               s = (char*)"pk_comment";
               break;

          case pk_conform:
               s = (char*)"pk_conform";
               break;

          case pk_include_alias:
               s = (char*)"pk_include_alias";
               break;
#endif
#if INCLUDE_UNRECOGNIZED_PRAGMAS_IN_IL
          case pk_unrecognized:
               s = (char*)"pk_unrecognized";
               break;
#endif
          case pk_last:
               s = (char*)"pk_last";
               break;

          default:
               s = (char*)"**BAD PRAGMA BASE KIND**";
               printf ("In summarize_template_kind_in_string(kind = %d) \n",kind);
               ROSE_ASSERT(false);
        }  /* switch */

     assert(s != NULL);
     return s;
   }



void
summarize_pragma ( a_pragma_ptr ptr )
   {
     ROSE_ASSERT (ptr != NULL);

     printf ("In summarize_pragma (%p) \n",ptr);

     printf ("     next                    = %p \n",ptr->next);
     printf ("     kind                    = %s \n",summarize_pragma_kind_in_string(ptr->kind));

     printf ("     ignore_in_back_end      = %s \n",ptr->ignore_in_back_end ? "true" : "false");
#if MICROSOFT_EXTENSIONS_ALLOWED
     printf ("     is_microsoft_pragma_operator      = %s \n",ptr->is_microsoft_pragma_operator ? "true" : "false");
#endif
     printf ("     entity                  = %s \n",summarize_entry_kind_in_string_kind(ptr->entity).c_str());
  // a_source_position decl_position;
     printf ("     ptr->position.seq  - -  = %ld \n",(unsigned long)ptr->position.seq);
     if (ptr->position.seq == 0)
        {
          printf ("     --- This might be compiler generated, or at least not in the source code! \n");
        }
       else
        {
          summarize_source_position(ptr->position);
        }
#if GENERATE_SOURCE_SEQUENCE_LISTS
     printf ("     source_sequence_entry  = %p \n",ptr->source_sequence_entry);
#endif
     printf ("     pragma_text  = %p \n",ptr->pragma_text);
     if (ptr->pragma_text != NULL)
        {
          printf ("     --- pragma_text  = %s \n",ptr->pragma_text);
        }

     switch(ptr->kind)
        {

          case pk_stdc:
               printf ("     ptr->variant.stdc.kind   = %d \n",ptr->variant.stdc.kind);
               printf ("     ptr->variant.stdc.value   = %d \n",ptr->variant.stdc.value);
               break;
#if UPC_EXTENSIONS_ALLOWED
          case pk_upc:
            // This is either: upc_pk_access or upc_pk_coherence
               printf ("     ptr->variant.upc.kind   = %d \n",ptr->variant.upc.kind);
               if (ptr->variant.upc.kind == upc_pk_access)
                  {
                    printf ("     ptr->variant.upc.kind  = upc_pk_access \n");
                    printf ("     ptr->variant.upc.value.access_method  = %d \n",ptr->variant.upc.value.access_method);
                  }
                 else
                  {
                    ROSE_ASSERT(ptr->variant.upc.kind == upc_pk_coherence);
                    printf ("     ptr->variant.upc.kind = upc_pk_coherence \n");
                    printf ("     ptr->variant.upc.value.operation  = %d \n",ptr->variant.upc.value.operation);
                  }
               break;
#endif
#if IDENT_DIRECTIVE_AND_PRAGMA
          case pk_ident:
               printf ("     ident_string  = %p \n",ptr->variant.ident_string);
               break;
#endif
#if USER_CONTROL_OF_STRUCT_PACKING && BACK_END_IS_CP_GEN_BE
          case pk_pack:
               printf ("     alignment  = %d \n",ptr->variant.alignment);
               break;
#endif
#if MICROSOFT_EXTENSIONS_ALLOWED
          case pk_comment:
               printf ("     ptr->variant.comment.kind   = %d \n",ptr->variant.comment.kind);
               printf ("     ptr->variant.comment.str    = %p \n",ptr->variant.comment.str);
               break;
          case pk_conform:
               printf ("     ptr->variant.conform.kind       = %d \n",ptr->variant.conform.kind);
               printf ("     ptr->variant.conform.on         = %s \n",ptr->variant.conform.on ? "true" : "false");
               printf ("     ptr->variant.conform.off        = %s \n",ptr->variant.conform.off ? "true" : "false");
               printf ("     ptr->variant.conform.show       = %s \n",ptr->variant.conform.show ? "true" : "false");
               printf ("     ptr->variant.conform.push       = %s \n",ptr->variant.conform.push ? "true" : "false");
               printf ("     ptr->variant.conform.pop        = %s \n",ptr->variant.conform.pop ? "true" : "false");
               printf ("     ptr->variant.conform.identifier = %p \n",ptr->variant.conform.identifier);
               break;
          case pk_include_alias:
               printf ("     ptr->variant.include_alias.long_file_name  = %p \n",ptr->variant.include_alias.long_file_name);
               if (ptr->variant.include_alias.long_file_name != NULL)
                  {
                    printf ("     --- ptr->variant.include_alias.long_file_name  = %s \n",ptr->variant.include_alias.long_file_name);
                  }
               printf ("     ptr->variant.include_alias.short_file_name = %p \n",ptr->variant.include_alias.short_file_name);
               if (ptr->variant.include_alias.short_file_name != NULL)
                  {
                    printf ("     --- ptr->variant.include_alias.short_file_name  = %s \n",ptr->variant.include_alias.short_file_name);
                  }
               break;
#endif
          case pk_unrecognized:
               printf ("     Found kind == pk_unrecognized (no specific information available) \n");
               break;

       // DQ (11/12/2012): Added EDG 4.x cases not handled
          case pk_none:
          case pk_printf_args:
          case pk_scanf_args:
          case pk_lint_argsused:
          case pk_lint_varargs_count:
          case pk_lint_notreached:
          case pk_instantiate:
          case pk_do_not_instantiate:
          case pk_can_instantiate:
          case pk_once:
          case pk_hdrstop:
          case pk_no_pch:
          case pk_define_type_info:
#if GNU_EXTENSIONS_ALLOWED
          case pk_gcc_immediate:
          case pk_gcc_next_token:
#endif
          case pk_diag_suppress:
          case pk_diag_remark:
          case pk_diag_warning:
          case pk_diag_error:
          case pk_diag_once:
          case pk_diag_default:
          case pk_db_opt:
          case pk_db_name:
#if MICROSOFT_EXTENSIONS_ALLOWED
          case pk_if_exists:
          case pk_push_macro:
          case pk_pop_macro:
          case pk_start_map_region:
          case pk_stop_map_region:
#endif
          case pk_last:
             {
               printf ("pragma case not handled! kind = %d = %s \n",ptr->kind,summarize_pragma_kind_in_string(ptr->kind));
               ROSE_ASSERT(false);
               break;
             }

          default:
             {
               printf ("Error: default reached in summarize_pragma() \n");
               ROSE_ASSERT(false);
               break;
             }
        }
   }


void
summarize_new_delete_supplement ( a_new_delete_supplement_ptr ptr )
    {
     ROSE_ASSERT (ptr != NULL);

     printf ("     is_new                          = %s \n",ptr->is_new ? "true" : "false");
     printf ("     placement_new                   = %s \n",ptr->placement_new ? "true" : "false");
     printf ("     array_delete                    = %s \n",ptr->array_delete ? "true" : "false");
     printf ("     global_new_or_delete            = %s \n",ptr->global_new_or_delete ? "true" : "false");
     printf ("     has_new_initializer             = %s \n",ptr->has_new_initializer ? "true" : "false");
     printf ("     new_initializer_is_brace_enclosed = %s \n",ptr->new_initializer_is_brace_enclosed ? "true" : "false");
     printf ("     type                            = %p \n",ptr->type);
     printf ("     routine                         = %p \n",ptr->routine);
     printf ("     arg                             = %p \n",ptr->arg);
     printf ("     dynamic_init                    = %p \n",ptr->dynamic_init);
     printf ("     freeing_of_storage_on_exception = %p \n",ptr->freeing_of_storage_on_exception);
     printf ("     number_of_elements              = %p \n",ptr->number_of_elements);
   }




void
summarize_symbol ( a_symbol_ptr ptr )
   {
  /* Description of an EDG a_symbol_ptr */

     ROSE_ASSERT (ptr != NULL);

     printf ("In summarize_symbol (%p) \n",ptr);

     printf ("     header                            = %p \n",ptr->header);
     printf ("     next                              = %p \n",ptr->next);
     printf ("     next_in_scope                     = %p \n",ptr->next_in_scope);

     printf ("     prev_in_scope                     = %p \n",ptr->prev_in_scope);
     printf ("     next_in_lookup_table              = %p \n",ptr->next_in_lookup_table);

  // TV (12/20/2023): Looks like this function has not been used since EDG 4.11.... 
  // DQ (10/5/2016): This is a name change in the EDG 4.11 IR.
     printf ("ROSE EDG 4.11 fixme: Unclear how this code is modified over the EDG 4.9 version of the code below! \n");
     ROSE_ASSERT(false);

     printf ("     decl_seq                          = %u \n",(a_decl_sequence_number)ptr->decl_seq);
  // printf ("     decl_position                     = %u \n",ptr->decl_position.seq);
     printf ("     decl_position                     = %" PRIuPTR " \n",(size_t)ptr->decl_position.seq);
  // printf ("     parent                            = %p \n",ptr->parent);
     printf ("     corresp_nonreal_or_nested_type    = %p \n",ptr->corresp_nonreal_or_nested_type);

     printf ("     kind                              = %d = %s \n",ptr->kind,summarize_symbol_kind_in_string_kind(ptr->kind));

     printf ("     referenced                        = %s \n",ptr->referenced ? "true" : "false");
     printf ("     defined                           = %s \n",ptr->defined ? "true" : "false");
     printf ("     explicit_linkage_specifier        = %s \n",ptr->explicit_linkage_specifier ? "true" : "false");
     printf ("     reentered_from_prototype_scope    = %s \n",ptr->reentered_from_prototype_scope ? "true" : "false");
     printf ("     is_class_member                   = %s \n",ptr->is_class_member ? "true" : "false");
     printf ("     is_error                          = %s \n",ptr->is_error ? "true" : "false");
     printf ("     is_template_param                 = %s \n",ptr->is_template_param ? "true" : "false");
     printf ("     is_nonreal_nested_type            = %s \n",ptr->is_nonreal_nested_type ? "true" : "false");
     printf ("     template_param_not_visible        = %s \n",ptr->template_param_not_visible ? "true" : "false");
     printf ("     force_external_linkage            = %s \n",ptr->force_external_linkage ? "true" : "false");
     printf ("     ambiguous                         = %s \n",ptr->ambiguous ? "true" : "false");

     printf ("     synthesized_namespace_projection  = %s \n",ptr->synthesized_namespace_projection ? "true" : "false");
     printf ("     qualified_lookup                  = %s \n",ptr->qualified_lookup ? "true" : "false");
     printf ("     must_be_class_or_namespace_lookup = %s \n",ptr->must_be_class_or_namespace_lookup ? "true" : "false");
     printf ("     must_be_tag_lookup                = %s \n",ptr->must_be_tag_lookup ? "true" : "false");
     printf ("     tentative_type_lookup             = %s \n",ptr->tentative_type_lookup ? "true" : "false");
     printf ("     do_not_reuse                      = %s \n",ptr->do_not_reuse ? "true" : "false");
     printf ("     instantiation_context_lookup      = %s \n",ptr->instantiation_context_lookup ? "true" : "false");
     printf ("     must_be_class_lookup              = %s \n",ptr->must_be_class_lookup ? "true" : "false");
     printf ("     hidden_by_old_for_init            = %s \n",ptr->hidden_by_old_for_init ? "true" : "false");
     printf ("     overload_set_member               = %s \n",ptr->overload_set_member ? "true" : "false");
     printf ("     is_invisible                      = %s \n",ptr->is_invisible ? "true" : "false");
     printf ("     is_unknown_function               = %s \n",ptr->is_unknown_function ? "true" : "false");
     printf ("     is_nonreal_member                 = %s \n",ptr->is_nonreal_member ? "true" : "false");
     printf ("     potentially_overloaded            = %s \n",ptr->potentially_overloaded ? "true" : "false");
#if MICROSOFT_EXTENSIONS_ALLOWED
     printf ("     is_super_reference                = %s \n",ptr->is_super_reference ? "true" : "false");
     printf ("     is_microsoft_invisible_operator   = %s \n",ptr->is_microsoft_invisible_operator ? "true" : "false");
     printf ("     hide_by_sig_lookup_done           = %s \n",ptr->hide_by_sig_lookup_done ? "true" : "false");
     printf ("     suppress_hide_by_sig_lookup       = %s \n",ptr->suppress_hide_by_sig_lookup ? "true" : "false");
#endif
#if GNU_EXTENSIONS_ALLOWED
     printf ("     is_alias                          = %s \n",ptr->is_alias ? "true" : "false");
#endif
     printf ("     is_pack_element                   = %s \n",ptr->is_pack_element ? "true" : "false");

#if 0
     printf ("                = %p \n",ptr-> ? "true" : "false");
     printf ("                = %p \n",ptr-> ? "true" : "false");
     printf ("                = %p \n",ptr-> ? "true" : "false");
     printf ("                = %p \n",ptr-> ? "true" : "false");
     printf ("                = %p \n",ptr-> ? "true" : "false");
     printf ("                = %p \n",ptr-> ? "true" : "false");
     printf ("                = %p \n",ptr-> ? "true" : "false");
     printf ("                = %p \n",ptr-> ? "true" : "false");
     printf ("                = %p \n",ptr-> ? "true" : "false");
     printf ("                = %p \n",ptr-> ? "true" : "false");

     printf ("                = %p \n",ptr->);
     printf ("                = %p \n",ptr->);
#endif

   }



void
summarize_exception_specification_type ( an_exception_specification_type_ptr ptr )
   {
     ROSE_ASSERT (ptr != NULL);

  // printf ("                   --- Calling summarize_exception_specification_type(p = %p) \n",p);
     printf ("                   --- In summarize_exception_specification_type (%p) \n",ptr);

     printf ("                       --- next              = %p \n",ptr->next);
     printf ("                       --- type              = %p \n",ptr->type);
     printf ("                       --- redundant         = %s \n",ptr->redundant ? "true" : "false");
     printf ("                       --- is_pack_expansion = %s \n",ptr->is_pack_expansion ? "true" : "false");
   }


void
summarize_exception_specification ( an_exception_specification_ptr ptr )
   {
     ROSE_ASSERT (ptr != NULL);

  // printf ("               routine.extra_info.exception_specification  = %p \n",tp->variant.routine.extra_info->exception_specification);
     printf ("               --- In summarize_exception_specification (%p) \n",ptr);

     printf ("               --- is_noexcept                                                  = %s \n",ptr->is_noexcept ? "true" : "false");
     printf ("               --- indeterminate                                                = %s \n",ptr->indeterminate ? "true" : "false");
     printf ("               --- throw_any                                                    = %s \n",ptr->throw_any ? "true" : "false");
     printf ("               --- compiler_generated                                           = %s \n",ptr->compiler_generated ? "true" : "false");
     printf ("               --- arg_cached                                                   = %s \n",ptr->arg_cached ? "true" : "false");

     if (ptr->arg_cached == true)
        {
          printf ("                   --- ptr->variant.token_cache = %p (frontend use only) \n",ptr->variant.token_cache);
        }
       else
        {
       // ptr->arg_cached == false
          if (ptr->is_noexcept == true)
             {
               printf ("                   --- ptr->variant.noexcept_arg = %p (handle constant) \n",ptr->variant.noexcept_arg);

            // DQ (10/4/2015): This can be NULL for test2015_85.C, for example.
            // ROSE_ASSERT(ptr->variant.noexcept_arg != NULL);
               if (ptr->variant.noexcept_arg != NULL)
                  {
                    summarize_constant(ptr->variant.noexcept_arg);
                  }
             }
            else
             {
               printf ("                   --- exception_specification_type_list = %p \n",ptr->variant.exception_specification_type_list);
               an_exception_specification_type_ptr p = ptr->variant.exception_specification_type_list;
               while (p != NULL)
                  {
                 // printf ("                   --- exception_specification_type_list = %p \n",ptr->exception_specification_type_list);
                    printf ("                   --- Calling summarize_exception_specification_type(p = %p) \n",p);
                    summarize_exception_specification_type(p);
                    p = p->next;
                  }
             }
        }

#if MICROSOFT_EXTENSIONS_ALLOWED
  // DQ (10/4/2015): This is no longer MS specific.
  // printf ("                   --- throw_any                         = %s \n",ptr->throw_any ? "true" : "false");
#endif

#if EXTRA_SOURCE_POSITIONS_IN_IL
  // DQ (10/4/2015): Added source range output to debugging info.
     printf ("                   --- source_range: \n");
     summarize_source_range(ptr->source_range);
#endif

     printf ("               --- Leaving summarize_exception_specification (%p) \n",ptr);
   }

// DQ (4/24/2013): Added support for a_routine_type_supplement.
void summarize_routine_type_supplement ( a_routine_type_supplement_ptr ptr )
   {
     ROSE_ASSERT (ptr != NULL);

     printf ("               routine.extra_info.param_type_list                               = %p \n",ptr->param_type_list);

#if 0
  // DQ (2/10/2018): this was added to support searching for vla info, but it is redundant information to be output in general.
     if (ptr->param_type_list != NULL)
        {
          a_param_type_ptr parameter_type = ptr->param_type_list;
          while (parameter_type != NULL)
             {
               printf ("In summarize_routine_type_supplement(): calling summarize_param_type(): parameter_type = %p \n",parameter_type);
               summarize_param_type(parameter_type);
               printf ("DONE: In summarize_routine_type_supplement(): calling summarize_param_type(): parameter_type = %p \n",parameter_type);

               parameter_type = parameter_type->next;
             }
        }
#endif

     printf ("               routine.extra_info.assoc_routine                                 = %p \n",ptr->assoc_routine);
     printf ("               routine.extra_info.has_ellipsis                                  = %s \n",ptr->has_ellipsis ? "true" : "false");

     printf ("               routine.extra_info.prototyped                                    = %s \n",ptr->prototyped ? "true" : "false");
     printf ("               routine.extra_info.old_style_params_scanned                      = %s \n",ptr->old_style_params_scanned ? "true" : "false");
     printf ("               routine.extra_info.trailing_return_type                          = %s \n",ptr->trailing_return_type ? "true" : "false");
     printf ("               routine.extra_info.lint_argsused_flag                            = %s \n",ptr->lint_argsused_flag ? "true" : "false");
     printf ("               routine.extra_info.value_returned_by_cctor                       = %s \n",ptr->value_returned_by_cctor ? "true" : "false");
#if DO_IL_LOWERING
     printf ("               routine.extra_info.value_returned_as_parameter                   = %s \n",ptr->value_returned_as_parameter ? "true" : "false");
     printf ("               routine.extra_info.return_value_parameter_follows_this           = %s \n",ptr->return_value_parameter_follows_this ? "true" : "false");
#endif
     printf ("               routine.extra_info.assoc_routine_is_ctor                         = %s \n",ptr->assoc_routine_is_ctor ? "true" : "false");
     printf ("               routine.extra_info.assoc_routine_is_dtor                         = %s \n",ptr->assoc_routine_is_dtor ? "true" : "false");
     printf ("               routine.extra_info.suppress_diagnostic_on_incomplete_return_type = %s \n",ptr->suppress_diagnostic_on_incomplete_return_type ? "true" : "false");
     printf ("               routine.extra_info.routine_name_linkage                          = %u \n",(unsigned int)ptr->routine_name_linkage);
     printf ("               routine.extra_info.routine_name_linkage_is_explicit              = %s \n",ptr->routine_name_linkage_is_explicit ? "true" : "false");
     printf ("               routine.extra_info.qualifiers                                    = %u \n",(unsigned int)ptr->qualifiers);
     printf ("               routine.extra_info.qualifiers (summary)                          = %s \n",summarize_qualifiers_in_string((size_t)(ptr->qualifiers)).c_str());
     printf ("               routine.extra_info.this_qualifiers                               = %u \n",(unsigned int)ptr->this_qualifiers);
     printf ("               routine.extra_info.this_qualifiers (summary)                     = %s \n",summarize_qualifiers_in_string((size_t)(ptr->this_qualifiers)).c_str());
     printf ("               routine.extra_info.ref_qualifiers                                = %u \n",(unsigned int)ptr->ref_qualifiers);
     printf ("               routine.extra_info.ref_qualifiers (summary)                      = %s \n",summarize_qualifiers_in_string((size_t)(ptr->ref_qualifiers)).c_str());
     printf ("               routine.extra_info.does_not_return                               = %s \n",ptr->does_not_return ? "true" : "false");

#if GNU_EXTENSIONS_ALLOWED
#if COMPILING_NEW_EDG_ROSE_INTERFACE
     printf ("               routine.extra_info.result_should_be_used                         = %s \n",ptr->result_should_be_used ? "true" : "false");
#endif
     printf ("               routine.extra_info.is_const                                      = %s \n",ptr->is_const ? "true" : "false");
#endif
     printf ("               routine.extra_info.is_variadic_instance                          = %s \n",ptr->is_variadic_instance ? "true" : "false");
#if MICROSOFT_EXTENSIONS_ALLOWED || GNU_X86_ATTRIBUTES_ALLOWED
     printf ("               routine.extra_info.explicit_calling_convention                   = %s \n",ptr->explicit_calling_convention ? "true" : "false");
#endif
     printf ("               routine.extra_info.lint_varargs_count                            = %u \n",(unsigned int)ptr->lint_varargs_count);
     printf ("               routine.extra_info.arg_pragma                                    = %u \n",(unsigned int)ptr->arg_pragma);
#if GNU_EXTENSIONS_ALLOWED
     printf ("               routine.extra_info.fmt_arg                                       = %u \n",(unsigned int)ptr->fmt_arg);
#if COMPILING_NEW_EDG_ROSE_INTERFACE
     printf ("               routine.extra_info.sentinel_pos                                  = %u \n",(unsigned int)ptr->sentinel_pos);
#endif
#endif
#if MICROSOFT_EXTENSIONS_ALLOWED || GNU_X86_ATTRIBUTES_ALLOWED
     printf ("               routine.extra_info.calling_convention                            = %u \n",(unsigned int)ptr->calling_convention);
#endif
     printf ("               routine.extra_info.this_class                                    = %p \n",ptr->this_class);
     printf ("               routine.extra_info.prototype_scope                               = %p \n",ptr->prototype_scope);
     printf ("               routine.extra_info.exception_specification                       = %p \n",ptr->exception_specification);
     if (ptr->exception_specification != NULL)
        {
          summarize_exception_specification (ptr->exception_specification);
        }
   }


// DQ (1/2/2014): Adding debugging support for macros.
void summarize_macro ( a_macro_ptr ptr )
   {
     ROSE_ASSERT (ptr != NULL);

#ifdef __cplusplus
     summarize_source_correspondence ( ptr->source_corresp );
#endif

     printf ("     next                       = %p \n",ptr->next);
     printf ("     is_undef                   = %s \n",ptr->is_undef ? "true" : "false");
     printf ("     is_command_line_definition = %s \n",ptr->is_command_line_definition ? "true" : "false");
     printf ("     is_predefined              = %s \n",ptr->is_predefined ? "true" : "false");
     printf ("     object_like                = %s \n",ptr->object_like ? "true" : "false");

#if EXTRA_SOURCE_POSITIONS_IN_IL
     printf ("   replacement_text_range: \n");
     summarize_source_range(ptr->replacement_text_range);
#endif

     printf ("     text     = %s \n",ptr->text != NULL ? ptr->text : "null");
   }


// DQ (1/2/2014): Adding debugging support for macro (contained in the il_header).
void summarize_il_header ( an_il_header & p )
   {
     printf ("     primary_source_file                           = %p \n",p.primary_source_file);
     printf ("     primary_scop                                  = %p \n",p.primary_scope);
     printf ("     main_routine                                  = %p \n",p.main_routine);
     printf ("     compiler_version (EDG version)                = %s \n",p.compiler_version != NULL ? p.compiler_version : "null");

  // The string for the time includes a CR.
     printf ("     time_of_compilation                           = %s",p.time_of_compilation != NULL ? p.time_of_compilation : "null");

     printf ("     plain_chars_are_signed                        = %s \n",p.plain_chars_are_signed ? "true" : "false");
     printf ("     region_scope_entry                            = %p \n",p.region_scope_entry);

#if SCOPE_ORPHANED_LIST_PROCESSING_NEEDED
     printf ("     scope_orphaned_list_headers                   = %p \n",p.scope_orphaned_list_headers);

  // DQ (3/18/2015): Added support for hunting down the lost type in test2015_12.c (C test code from Xen).
     a_scope_orphaned_list_header_ptr scope_orphaned_list_header = p.scope_orphaned_list_headers;
     while (scope_orphaned_list_header != NULL)
        {
          summarize_scope_orphaned_list_header(scope_orphaned_list_header);

          scope_orphaned_list_header = scope_orphaned_list_header->next;
        }
#endif

     printf ("     source_language                               = %d \n",p.source_language);

#if MICROSOFT_EXTENSIONS_ALLOWED
     printf ("     cli_metadata_files                            = %p \n",p.cli_metadata_files);
#endif

     printf ("     pcc_compatibility_mode                        = %s \n",p.pcc_compatibility_mode ? "true" : "false");
     printf ("     enum_type_is_integral                         = %s \n",p.enum_type_is_integral ? "true" : "false");
#if USER_CONTROL_OF_STRUCT_PACKING
     printf ("     default_max_member_alignment                  = %d \n",p.default_max_member_alignment);
#endif
#if RECORD_MACROS_IN_IL
     printf ("     macros                                        = %p \n",p.macros);
     if (p.macros != NULL)
        {
          size_t counter = 0;
          for (a_macro_ptr i = p.macros; i != NULL; i = i->next)
             {
               printf ("In summarize_il_header(): counter = %" PRIuPTR " calling summarize_macro() \n",counter);
#if 0
            // The output of the macro details is overly verbose (lots of predefined macros internal to EDG).
               summarize_macro(i);
#else
               printf ("   --- macro->source_corresp.name = %s \n",i->source_corresp.name);
               printf ("   --- macro->text                = %s \n",i->text);
#endif
               counter++;
             }
        }
#endif

#if MICROSOFT_EXTENSIONS_ALLOWED
     printf ("     microsoft_mode                                = %s \n",p.microsoft_mode ? "true" : "false");
     printf ("     cppcli_enabled                                = %s \n",p.cppcli_enabled ? "true" : "false");
     printf ("     microsoft_version                             = %" PRIuPTR " \n",p.microsoft_version);
#endif

#if UPC_EXTENSIONS_ALLOWED
     printf ("     default_upc_strict_access                     = %s \n",p.default_upc_strict_access ? "true" : "false");
#endif

#if GNU_EXTENSIONS_ALLOWED
     printf ("     gcc_mode                                      = %s \n",p.gcc_mode ? "true" : "false");
     printf ("     gpp_mode                                      = %s \n",p.gpp_mode ? "true" : "false");
     printf ("     gnu_version                                   = %" PRIuPTR " \n",p.gnu_version);
     printf ("     short_enums                                   = %s \n",p.short_enums ? "true" : "false");
     printf ("     default_nocommon                              = %s \n",p.default_nocommon ? "true" : "false");
#endif

#if NEAR_AND_FAR_ALLOWED
     printf ("     near_and_far_are_enabled                      = %s \n",p.near_and_far_are_enabled ? "true" : "false");
     printf ("     far_data_pointers                             = %s \n",p.far_data_pointers ? "true" : "false");
     printf ("     far_code_pointers                             = %s \n",p.far_code_pointers ? "true" : "false");
#endif

     printf ("     UCN_identifiers_used                          = %s \n",p.UCN_identifiers_used ? "true" : "false");
     printf ("     vla_used                                      = %s \n",p.vla_used ? "true" : "false");
     printf ("     any_templates_seen                            = %s \n",p.any_templates_seen ? "true" : "false");
     printf ("     prototype_instantiations_in_il                = %s \n",p.prototype_instantiations_in_il ? "true" : "false");
     printf ("     il_has_all_prototype_instantiations           = %s \n",p.il_has_all_prototype_instantiations ? "true" : "false");
     printf ("     il_has_C_semantics                            = %s \n",p.il_has_C_semantics ? "true" : "false");

#if ONE_INSTANTIATION_PER_OBJECT
     printf ("     instantiation_dir_name                        = %s \n",p.instantiation_dir_name != NULL ? p.instantiation_dir_name : "null");
     printf ("     number_of_external_nonclass_template_entities = %" PRIuPTR " \n",p.number_of_external_nonclass_template_entities);
#endif

     printf ("     nontag_types_used_in_exception_or_rtti        = %p \n",p.nontag_types_used_in_exception_or_rtti);
     printf ("     seq_number_lookup_entries                     = %p \n",p.seq_number_lookup_entries);
     printf ("     num_seq_number_lookup_entries                 = %" PRIuPTR " \n",p.num_seq_number_lookup_entries);

#if MACRO_INVOCATION_TREE_IN_IL
  // printf ("     num_macro_invocation_records                  = %d \n",p.num_macro_invocation_records);
     printf ("     num_macro_invocation_records                  = %" PRIuPTR " \n",(size_t)(p.num_macro_invocation_records));
     printf ("     max_macro_invocation_depth                    = %" PRIuPTR " \n",p.max_macro_invocation_depth);

     printf ("     root_macro_invocation_record_block            = %p \n",p.root_macro_invocation_record_block);
     if (p.root_macro_invocation_record_block != NULL)
        {
          printf ("In summarize_il_header(): calling summarize_macro_invocation_record_block() \n");
          summarize_macro_invocation_record_block(p.root_macro_invocation_record_block);
        }
#endif

#if SEPARATE_ROUTINES_FOR_FILE_SCOPE_DYNAMIC_INITS
     printf ("     file_scope_dynamic_init_routines              = %p \n",p.file_scope_dynamic_init_routines);
#endif
   }


// DQ (9/17/2018): Adding debugging support for source file details.
void summarize_source_file ( const a_source_file_ptr & ptr )
   {
     ROSE_ASSERT(ptr != NULL);

     printf ("In summarize_source_file(): ptr = %p \n",ptr);

     printf ("     file_name                          = %s \n",ptr->file_name);
     printf ("     full_name                          = %s \n",ptr->full_name);
     printf ("     name_as_written                    = %s \n",ptr->name_as_written);
     printf ("     first_seq_number                   = %u \n",ptr->first_seq_number);
     printf ("     last_seq_number                    = %u \n",ptr->last_seq_number);

     printf ("     first_line_number                  = %u \n",ptr->first_line_number);

     printf ("     first_child_file                   = %p  \n",ptr->first_child_file);
     printf ("     last_child_file                    = %p  \n",ptr->last_child_file);
     printf ("     next                               = %p  \n",ptr->next);

#if INSTANTIATION_BY_IMPLICIT_INCLUSION
     printf ("     related_file_implicit_include_done = %s \n",ptr->related_file_implicit_include_done ? "true" : "false");
     printf ("     is_implicit_include                = %s \n",ptr->is_implicit_include                ? "true" : "false");
#endif /* INSTANTIATION_BY_IMPLICIT_INCLUSION */

     printf ("     is_include_file                    = %s \n",ptr->is_include_file                    ? "true" : "false");
     printf ("     included_by_system_include         = %s \n",ptr->included_by_system_include         ? "true" : "false");
     printf ("     included_by_preinclude             = %s \n",ptr->included_by_preinclude             ? "true" : "false");
     printf ("     preinclude_macros_only             = %s \n",ptr->preinclude_macros_only             ? "true" : "false");
     printf ("     from_system_include_dir            = %s \n",ptr->from_system_include_dir            ? "true" : "false");
     printf ("     top_level_file                     = %s \n",ptr->top_level_file                     ? "true" : "false");
     printf ("     top_level_file_from_pch            = %s \n",ptr->top_level_file_from_pch            ? "true" : "false");

#if MICROSOFT_EXTENSIONS_ALLOWED
     printf ("     is_assembly_file                   = %s \n",ptr->is_assembly_file                   ? "true" : "false");
#endif

     a_source_file_ptr temp_file = ptr->first_child_file;
  // while (temp_file != ptr->last_child_file)
     while (temp_file != NULL)
        {
       // printf ("temp_file = %p \n",temp_file);

          summarize_source_file(temp_file);

          temp_file = temp_file->next;
        }
   }


// DQ (1/2/2014): Adding debugging support for macros.
void summarize_macro_invocation_record ( a_macro_invocation_record_ptr ptr )
   {
     ROSE_ASSERT(ptr != NULL);

  // printf ("     parent_macro_index                               = %d \n",ptr->parent_macro_index);
     printf ("     parent_macro_index                               = %" PRIuPTR " \n",(size_t)ptr->parent_macro_index);
     printf ("     parent_macro_index == NO_PARENT_MACRO_INVOCATION = %s \n",(ptr->parent_macro_index == NO_PARENT_MACRO_INVOCATION) ? "true" : "false");

     if ( (ptr->parent_macro_index != NO_PARENT_MACRO_INVOCATION) && (ptr->parent_macro_index < 0) )
        {
           printf ("     parent_macro_index indicates pop level = %d \n", -((int)ptr->parent_macro_index));
        }

     printf ("     assoc_macro                                      = %p \n",ptr->assoc_macro);
     if (ptr->assoc_macro != NULL)
        {
          printf ("In summarize_macro(): calling summarize_macro() \n");
          summarize_macro(ptr->assoc_macro);
        }

     printf ("     start position: \n");
     summarize_simple_source_position(&(ptr->start));

#if EXTRA_SOURCE_POSITIONS_IN_IL
     printf ("     end position: \n");
     summarize_simple_source_position(&(ptr->end));
#endif
   }


#if FULLY_RESOLVED_MACRO_POSITIONS || RECORD_MACRO_INVOCATIONS
// DQ (1/2/2014): Adding debugging support for macros.
void summarize_simple_source_position ( a_simple_source_position_ptr ptr )
   {
#if 0
#if FULLY_RESOLVED_MACRO_POSITIONS || RECORD_MACRO_INVOCATIONS
typedef struct a_simple_source_position *a_simple_source_position_ptr;
typedef struct a_simple_source_position {
  /* Indicates a source position without the dual-resolution (macro)
     features of a_source_position. */
  a_seq_number	seq;
  a_column_number
		column;
} a_simple_source_position;
#endif /* FULLY_RESOLVED_MACRO_POSITIONS || RECORD_MACRO_INVOCATIONS */
#endif

     ROSE_ASSERT(ptr != NULL);

  // printf ("     ---  seq    = %u \n",ptr->seq);
     printf ("     ---  seq    = %" PRIuPTR " \n",(size_t)ptr->seq);

     printf ("     ---  In summarize_simple_source_position(): calling summarize_source_sequence() \n");
     summarize_source_sequence(ptr->seq);

     printf ("     ---  column = %d \n",ptr->column);
   }
#endif

// DQ (1/2/2014): Adding debugging support for macros.
void summarize_macro_invocation_record_block ( a_macro_invocation_record_block_ptr ptr )
   {
     ROSE_ASSERT(ptr != NULL);

     printf ("     first_record_in_block = %d \n",(int)ptr->first_record_in_block);
     printf ("     left_subtree          = %p \n",ptr->left_subtree);
     if (ptr->left_subtree != NULL)
        {
          printf ("In summarize_macro_invocation_record_block(): recursive call to summarize_macro_invocation_record_block() \n");
          summarize_macro_invocation_record_block(ptr->left_subtree);
        }

     printf ("     right_subtree         = %p \n",ptr->right_subtree);
     if (ptr->right_subtree != NULL)
        {
          printf ("In summarize_macro_invocation_record_block(): recursive call to summarize_macro_invocation_record_block() \n");
          summarize_macro_invocation_record_block(ptr->right_subtree);
        }

     printf ("     prev                  = %p \n",ptr->prev);
     if (ptr->prev != NULL)
        {
          printf ("In summarize_macro_invocation_record_block(): recursive call to summarize_macro_invocation_record_block() \n");
          summarize_macro_invocation_record_block(ptr->prev);
        }

     printf ("     next                  = %p \n",ptr->next);
     if (ptr->next != NULL)
        {
          printf ("In summarize_macro_invocation_record_block(): recursive call to summarize_macro_invocation_record_block() \n");
          summarize_macro_invocation_record_block(ptr->next);
        }

     printf ("     records[*]: \n");
     for (size_t i = 0; i < MACRO_INVOCATION_RECORDS_PER_BLOCK; i++)
        {
          if (ptr->records[i].assoc_macro != NULL)
             {
               printf ("     ---  records[i = %" PRIuPTR "]: \n",i);
               summarize_macro_invocation_record(&(ptr->records[i]));
             }
        }
   }


string
attribute_kind_to_string (an_attribute_kind kind)
   {
     string s = "unknown";

     switch (kind)
        {
          case ak_unrecognized:       s = "ak_unrecognized"; break;
          case ak_empty_attr:         s = "ak_empty_attr"; break;
          case ak_align:              s = "ak_align"; break;
          case ak_base_check:         s = "ak_base_check"; break;
          case ak_carries_dependency: s = "ak_carries_dependency"; break;
          case ak_final:              s = "ak_final"; break;
          case ak_hiding:             s = "ak_hiding"; break;
          case ak_noreturn:           s = "ak_noreturn"; break;
          case ak_override:           s = "ak_override"; break;
#if GNU_EXTENSIONS_ALLOWED || MICROSOFT_EXTENSIONS_ALLOWED
          case ak_deprecated:         s = "ak_deprecated"; break;
#if GNU_NAKED_ATTRIBUTE_ALLOWED || MICROSOFT_EXTENSIONS_ALLOWED
          case ak_naked:              s = "ak_naked"; break;
#endif
          case ak_noinline:           s = "ak_noinline"; break;
          case ak_nothrow:            s = "ak_nothrow"; break;
          case ak_section:            s = "ak_section"; break;
#endif

#if GNU_EXTENSIONS_ALLOWED
          case ak_alias:                  s = "ak_alias"; break;
          case ak_alloc_size:             s = "ak_alloc_size"; break;
          case ak_always_inline:          s = "ak_always_inline"; break;
          case ak_artificial:             s = "ak_artificial"; break;
#if GNU_X86_ATTRIBUTES_ALLOWED && !USE_X86_64
          case ak_cdecl:                  s = "ak_cdecl"; break;
#endif
          case ak_cleanup:                s = "ak_cleanup"; break;
          case ak_cold:                   s = "ak_cold"; break;
          case ak_common:                 s = "ak_common"; break;
          case ak_const:                  s = "ak_const"; break;
          case ak_constructor:            s = "ak_constructor"; break;
          case ak_destructor:             s = "ak_destructor"; break;
          case ak_error:                  s = "ak_error"; break;
          case ak_externally_visible:     s = "ak_externally_visible"; break;
#if GNU_X86_ATTRIBUTES_ALLOWED && !USE_X86_64
          case ak_fastcall:               s = "ak_fastcall"; break;
#endif
          case ak_flatten:                s = "ak_flatten"; break;
          case ak_format:                 s = "ak_format"; break;
          case ak_format_arg:             s = "ak_format_arg"; break;
          case ak_gnu_inline:             s = "ak_gnu_inline"; break;
          case ak_hot:                    s = "ak_hot"; break;
#if GNU_INIT_PRIORITY_ATTRIBUTE_ALLOWED
          case ak_init_priority:          s = "ak_init_priority"; break;
#endif
          case ak_malloc:                 s = "ak_malloc"; break;
          case ak_may_alias:              s = "ak_may_alias"; break;
          case ak_mode:                   s = "ak_mode"; break;
          case ak_no_instrument_function: s = "ak_no_instrument_function"; break;
          case ak_no_check_memory_usage:  s = "ak_no_check_memory_usage"; break;
          case ak_nocommon:               s = "ak_nocommon"; break;
          case ak_nonnull:                s = "ak_nonnull"; break;
          case ak_packed:                 s = "ak_packed"; break;
          case ak_pure:                   s = "ak_pure"; break;
          case ak_sentinel:               s = "ak_sentinel"; break;
#if GNU_X86_ATTRIBUTES_ALLOWED && !USE_X86_64
          case ak_stdcall:                s = "ak_stdcall"; break;
#endif
          case ak_strong:                 s = "ak_strong"; break;
#if THREAD_LOCAL_STORAGE_SPECIFIER_ALLOWED
          case ak_tls_model:              s = "ak_tls_model"; break;
#endif
          case ak_transparent_union:      s = "ak_transparent_union"; break;
          case ak_unused:                 s = "ak_unused"; break;
          case ak_used:                   s = "ak_used"; break;
#if GNU_VECTOR_TYPES_ALLOWED
          case ak_vector_size:            s = "ak_vector_size"; break;
#endif
#if GNU_VISIBILITY_ATTRIBUTE_ALLOWED
          case ak_visibility:             s = "ak_visibility"; break;
#endif
          case ak_warn_unused_result:     s = "ak_warn_unused_result"; break;
          case ak_warning:                s = "ak_warning"; break;
          case ak_weak:                   s = "ak_weak"; break;
          case ak_weakref:                s = "ak_weakref"; break;
          case ak_abi_tag:                s = "ak_abi_tag"; break;
#endif /* GNU_EXTENSIONS_ALLOWED */

#if MICROSOFT_EXTENSIONS_ALLOWED
          case ak_appdomain:              s = "ak_appdomain"; break;
          case ak_assembly_info:          s = "ak_assembly_info"; break;
          case ak_dllexport:              s = "ak_dllexport"; break;
          case ak_dllimport:              s = "ak_dllimport"; break;
          case ak_edg_interior_ptr_alias: s = "ak_edg_interior_ptr_alias"; break;
          case ak_edg_pin_ptr_alias:      s = "ak_edg_pin_ptr_alias"; break;
          case ak_implementation_key:     s = "ak_implementation_key"; break;
          case ak_intrin_type:            s = "ak_intrin_type"; break;
          case ak_jitintrinsic:           s = "ak_jitintrinsic"; break;
          case ak_noalias:                s = "ak_noalias"; break;
          case ak_non_user_code:          s = "ak_non_user_code"; break;
          case ak_novtable:               s = "ak_novtable"; break;
          case ak_process:                s = "ak_process"; break;
          case ak_property:               s = "ak_property"; break;
          case ak_restrict:               s = "ak_restrict"; break;
          case ak_safebuffers:            s = "ak_safebuffers"; break;
          case ak_selectany:              s = "ak_selectany"; break;
#if THREAD_LOCAL_STORAGE_SPECIFIER_ALLOWED
          case ak_thread:                 s = "ak_thread"; break;
#endif
          case ak_uuid: s = "ak_uuid"; break;
#endif /* MICROSOFT_EXTENSIONS_ALLOWED */

#if ENABLE_CUDA_SUPPORT
          case ak_host:     s = "ak_host"; break;
          case ak_global:   s = "ak_global"; break;
          case ak_device:   s = "ak_device"; break;
          case ak_shared:   s = "ak_shared"; break;
          case ak_local:    s = "ak_local"; break;
          case ak_constant: s = "ak_constant"; break;
#endif /* ENABLE_CUDA_SUPPORT */

#if INCLUDE_EDG_TEST_ATTRIBUTES
          case ak_edg_e1: s = "ak_edg_e1"; break;
          case ak_edg_n1: s = "ak_edg_n1"; break;
#endif
          case ak_pragma_pack_state: s = "ak_pragma_pack_state"; break;

          case ak_last: s = "ak_last"; break;

          default: 
             {
               printf ("In attribute_kind_to_string(%" PRIuPTR "): default reached \n",(size_t)kind);
               ROSE_ASSERT(false);
             }
        }

     return s;
   }

string
attribute_family_to_string (an_attribute_family family)
   {
     string s = "unknown";

     switch (family)
        {
          case af_internal:    s = "af_internal";    break;
          case af_std:         s = "af_std";         break;
          case af_gnu:         s = "af_gnu";         break;
          case af_ms_declspec: s = "af_ms_declspec"; break;
          case af_alignas:     s = "af_alignas";     break;
          case af_last:        s = "af_last";        break;

          default: 
             {
               printf ("In attribute_family_to_string(%" PRIuPTR "): default reached \n",(size_t)family);
               ROSE_ASSERT(false);
             }
        }

     return s;
   }

string
attribute_syntactic_location_to_string (an_attribute_location location)
   {
     string s = "unknown";

     switch (location)
        {
          case al_implicit:                 s = "al_implicit"; break;
          case al_prefix:                   s = "al_prefix"; break;
          case al_tag_name:                 s = "al_tag_name"; break;
          case al_post_tag_definition:      s = "al_post_tag_definition"; break;
          case al_base_specifier:           s = "al_base_specifier"; break;
          case al_specifier:                s = "al_specifier"; break;
          case al_declarator_id:            s = "al_declarator_id"; break;
          case al_post_ptr_or_ref:          s = "al_post_ptr_or_ref"; break;
          case al_post_array:               s = "al_post_array"; break;
          case al_post_func:                s = "al_post_func"; break;
          case al_postfix:                  s = "al_postfix"; break;
          case al_predeclarator:            s = "al_predeclarator"; break;
          case al_id_equivalent:            s = "al_id_equivalent"; break;
          case al_trailing_return:          s = "al_trailing_return"; break;
          case al_post_initializer:         s = "al_post_initializer"; break;
          case al_namespace:                s = "al_namespace"; break;
          case al_label:                    s = "al_label"; break;
          case al_explicit:                 s = "al_explicit"; break;
          case al_id_equivalent_as_postfix: s = "al_id_equivalent_as_postfix"; break;

          case al_last: s = "al_last"; break;

          default: 
             {
               printf ("In attribute_family_to_string(%" PRIuPTR "): default reached \n",(size_t)location);
               ROSE_ASSERT(false);
             }
        }

     return s;
   }

string
attribute_arg_kind_to_string (an_attribute_arg_kind kind)
   {
     string s = "unknown";

     switch (kind)
        {
          case aak_empty:     s = "aak_empty"; break;
          case aak_raw_token: s = "aak_raw_token"; break;
          case aak_token:     s = "aak_token"; break;
          case aak_constant:  s = "aak_constant"; break;
          case aak_type:      s = "aak_type"; break;

          case aak_last:      s = "aak_last"; break;

          default: 
             {
               printf ("In attribute_family_to_string(%" PRIuPTR "): default reached \n",(size_t)kind);
               ROSE_ASSERT(false);
             }
        }

     return s;
   }

       // DQ (1/2/2014): Adding debugging support for macros.
void summarize_attribute_arg ( an_attribute_arg_ptr ptr )
   {
     ROSE_ASSERT(ptr != NULL);

     printf ("     next                                 = %p \n",ptr->next);
     printf ("     kind (enum value)                    = %" PRIuPTR " \n",(size_t)ptr->kind);
     printf ("     kind (name)                          = %s \n",attribute_arg_kind_to_string(ptr->kind).c_str());

     printf ("     position: \n");
     summarize_source_position(ptr->position);

#if EXTRA_SOURCE_POSITIONS_IN_IL
     printf ("     end_position: \n");
     summarize_source_position(ptr->end_position);
#endif

     printf ("     token_kind                           = %" PRIuPTR " \n",(size_t)ptr->token_kind);
  // printf ("     token_kind                           = %s \n",attribute_arg_token_kind_to_string(ptr->token_kind).c_str());
  // printf ("     token_kind                           = %" PRIuPTR " \n",(size_t)ptr->token_kind);

     if (ptr->kind != aak_empty)
        {
          if (ptr->kind == aak_token || ptr->kind == aak_raw_token)
             {
               printf ("     token (pointer)                      = %p \n",ptr->variant.token);
               if (ptr->variant.token != NULL)
                  {
                    printf ("     token (string)                       = %s \n",ptr->variant.token);
                  }
             }

          if (ptr->kind == aak_constant)
             {
               printf ("     constant                            = %p \n",ptr->variant.constant);
               if (ptr->variant.constant != NULL)
                  {
#if 0
                 // DQ (7/24/2014): This is the more detailed output, but which is not frequently required.
                    printf ("     In summarize_attribute_arg(): calling summarize_constant() \n");
                    summarize_constant(ptr->variant.constant);
#else
                    printf ("     In summarize_attribute_arg(): skip calling summarize_constant() \n");
#endif
                  }
             }

          if (ptr->kind == aak_type)
             {
               printf ("     type                                = %p \n",ptr->variant.type);
               if (ptr->variant.type != NULL)
                  {
                    summarize_type(ptr->variant.type);
                  }
             }
        }
       else
        {
       // No variant fields.
        }
     
   }

// DQ (1/2/2014): Adding debugging support for macros.
void summarize_attribute_group ( an_attribute_group_ptr ptr )
   {
     ROSE_ASSERT(ptr != NULL);

     printf ("     position: \n");
     summarize_source_position(ptr->position);

#if EXTRA_SOURCE_POSITIONS_IN_IL
     printf ("     end_position: \n");
     summarize_source_position(ptr->end_position);
#endif
   }


// DQ (1/2/2014): Adding debugging support for macros.
void summarize_attribute ( an_attribute_ptr ptr )
   {
     ROSE_ASSERT(ptr != NULL);

     printf ("     next                                 = %p \n",ptr->next);
     printf ("     kind (enum value)                    = %" PRIuPTR " \n",(size_t)ptr->kind);
     printf ("     kind (name)                          = %s \n",attribute_kind_to_string(ptr->kind).c_str());
     printf ("     family (enum value)                  = %" PRIuPTR " \n",(size_t)ptr->family);
     printf ("     family (name)                        = %s \n",attribute_family_to_string(ptr->family).c_str());

     printf ("     syntactic location                   = %" PRIuPTR " \n",(size_t)ptr->syntactic_location);
     printf ("     syntactic location                   = %s \n",attribute_syntactic_location_to_string(ptr->syntactic_location).c_str());

     printf ("     on_primary_declaration               = %s \n",ptr->on_primary_declaration    ? "true" : "false");
     printf ("     transforms_type_specifier            = %s \n",ptr->transforms_type_specifier ? "true" : "false");
     printf ("     must_be_preserved_in_trans_unit_copy = %s \n",ptr->must_be_preserved_in_trans_unit_copy ? "true" : "false");
     printf ("     is_pack_expansion                    = %s \n",ptr->is_pack_expansion ? "true" : "false");

     printf ("     name                                 = %s \n",ptr->name != NULL ? ptr->name : "null");
     printf ("     namespace_name                       = %s \n",ptr->namespace_name != NULL ? ptr->namespace_name : "null");

     printf ("     arguments                            = %p \n",ptr->arguments);
     an_attribute_arg_ptr attribute_arg = ptr->arguments;
     int counter = 0;
     while (attribute_arg != NULL)
        {
          printf ("     --- argument #%d (start)             = %p \n",counter,ptr->arguments);
          summarize_attribute_arg(attribute_arg);
          attribute_arg = attribute_arg->next;

          printf ("     --- argument #%d (end)               = %p \n",counter,ptr->arguments);
          counter++;
        }

     printf ("     group                                = %p \n",ptr->group);
     if (ptr->group != NULL)
        {
          summarize_attribute_group(ptr->group);
        }

     printf ("     assoc_info                           = %p \n",ptr->assoc_info);

     printf ("     position: \n");
     summarize_source_position(ptr->position);

#if EXTRA_SOURCE_POSITIONS_IN_IL
     printf ("     end_position: \n");
     summarize_source_position(ptr->end_position);
#endif
   }


void summarize_static_assertion ( a_static_assertion_ptr ptr )
   {
     ROSE_ASSERT(ptr != NULL);

     printf ("     condition               = %p \n",ptr->condition);
     printf ("     string_literal          = %p \n",ptr->string_literal);
     printf ("     position: \n");
     summarize_source_position(ptr->position);
   }


// DQ (8/11/2014): Adding support for base_class_ptr.
void summarize_base_class ( a_base_class_ptr ptr )
   {
     ROSE_ASSERT(ptr != NULL);

     printf ("     next                                           = %p \n",ptr->next);
#if IA64_ABI
     printf ("     next_preorder                                  = %p \n",ptr->next_preorder);
     printf ("     primary_base_class                             = %p \n",ptr->primary_base_class);
#endif
     printf ("     attributes                                     = %p \n",ptr->attributes);
#if MICROSOFT_EXTENSIONS_ALLOWED
     printf ("     ms_attributes                                  = %p \n",ptr->ms_attributes);
#endif
     printf ("     type                                           = %p \n",ptr->type);
     printf ("     orig_type                                      = %p \n",ptr->orig_type);
     printf ("     derived_class                                  = %p \n",ptr->derived_class);
     printf ("     trans_unit_corresp                             = %p \n",ptr->trans_unit_corresp);
     printf ("     decl_position: \n");
     summarize_source_position(ptr->decl_position);
#if EXTRA_SOURCE_POSITIONS_IN_IL
     printf ("     base_specifier_range: \n");
     summarize_source_range(ptr->base_specifier_range);
#endif
     printf ("     direct                                         = %s \n",ptr->direct ? "true" : "false");
     printf ("     is_virtual                                     = %s \n",ptr->is_virtual ? "true" : "false");
     printf ("     ambiguous                                      = %s \n",ptr->ambiguous ? "true" : "false");
     printf ("     shares_virtual_function_info                   = %s \n",ptr->shares_virtual_function_info ? "true" : "false");
     printf ("     ignore_during_dependent_lookup                 = %s \n",ptr->ignore_during_dependent_lookup ? "true" : "false");
#if CFRONT_OBJECT_CODE_COMPATIBILITY
     printf ("     complete_subobject                             = %s \n",ptr->complete_subobject ? "true" : "false");
     printf ("     pointer_offset_is_set                          = %s \n",ptr->pointer_offset_is_set ? "true" : "false");
#endif /* CFRONT_OBJECT_CODE_COMPATIBILITY */
     printf ("     is_optimized_empty_base                        = %s \n",ptr->is_optimized_empty_base ? "true" : "false");
#if IA64_ABI
     printf ("     offset_is_set                                  = %s \n",ptr->offset_is_set ? "true" : "false");
#endif /* IA64_ABI */
#if MICROSOFT_EXTENSIONS_ALLOWED
     printf ("     is_implicit_direct_base                        = %s \n",ptr->is_implicit_direct_base ? "true" : "false");
#endif /* MICROSOFT_EXTENSIONS_ALLOWED */
     printf ("     direct_base_number                             = %d \n",ptr->direct_base_number);
     printf ("     offset                                         = %" PRIuPTR " \n",(size_t)ptr->offset);
#if CFRONT_OBJECT_CODE_COMPATIBILITY
     printf ("     data_section_base_class                        = %p \n",ptr->data_section_base_class);
#endif
#if !IA64_ABI
     printf ("     pointer_offset                                 = %d \n",ptr->pointer_offset);
     printf ("     pointer_base_class                             = %p \n",ptr->pointer_base_class);
#endif /* !IA64_ABI */
     printf ("     derivation                                     = %p \n",ptr->derivation);

     printf ("     is_pack_expansion                              = %s \n",ptr->is_pack_expansion ? "true" : "false");
     if (ptr->is_pack_expansion) {
       printf ("     pack_expansion_descr                           = %p \n",ptr->variant.pack_expansion_descr);
     } else {
       printf ("     overriding_virtual_functions                   = %p \n",ptr->variant.overriding_virtual_functions);
     }
#if DO_IL_LOWERING
#if !IA64_ABI
     printf ("     virtual_function_table_var                     = %p \n",ptr->virtual_function_table_var);
#else /* IA64_ABI */
     printf ("     virtual_function_table_offset                  = %d \n",ptr->virtual_function_table_offset);
#endif /* IA64_ABI */
#if ABI_CHANGES_FOR_CONSTRUCTION_VTBLS
     printf ("     index_in_construction_vtbl_array               = %d \n",ptr->index_in_construction_vtbl_array);
     printf ("     base_subarray_index_in_construction_vtbl_array = %d \n",ptr->base_subarray_index_in_construction_vtbl_array);
#if !IA64_ABI
  /* When is_virtual is TRUE: */
     printf ("     base_construction_vtbls                        = %p \n",ptr->base_construction_vtbls);
#endif /* !IA64_ABI */
#endif /* ABI_CHANGES_FOR_CONSTRUCTION_VTBLS */
#if IA64_ABI
     printf ("     vbase_offset_index                             = %d \n",ptr->vbase_offset_index);
#endif /* IA64_ABI */
#endif /* DO_IL_LOWERING */
   }


#if MICROSOFT_EXTENSIONS_ALLOWED
// DQ (8/16/2014): Adding support for MS attributes.
void summarize_ms_attribute ( an_ms_attribute_ptr ptr )
   {
     ROSE_ASSERT(ptr != NULL);

     printf ("     next                                           = %p \n",ptr->next);

     printf ("     next_in_block                                           = %p \n",ptr->next_in_block);
  // printf ("     entity                                           = %p \n",ptr->entity);
     printf ("          --- entity.kind = %d = %s \n",ptr->entity.kind,summarize_entry_kind_in_string_kind((an_il_entry_kind)(ptr->entity.kind)));
     printf ("          --- entity.ptr  = %p \n",ptr->entity.ptr);
// EDG 4.9 (and greater) version specific code.

     printf ("     is_attribute_attribute                            = %s \n",ptr->is_attribute_attribute ? "true" : "false");

     if (ptr->kind != msak_custom)
        {
          printf ("     info.kind_descr                              = %p \n",ptr->variant.info.kind_descr);
          printf ("     info.name                                    = %s \n",ptr->variant.info.name);
          printf ("     info.string                                  = %s \n",ptr->variant.info.string);
          printf ("     info.arg_list                                = %p \n",ptr->variant.info.arg_list);
//          printf ("          --- info.arg_list.kind = %d = %s \n",ptr->variant.info.arg_list.kind,summarize_entry_kind_in_string_kind((an_il_entry_kind)(ptr->variant.info.arg_list.kind)));
//          printf ("          --- info.arg_list.ptr  = %p \n",ptr->variant.info.arg_list.ptr);

          int count = 0;
          an_ms_attribute_arg_ptr temp = ptr->variant.info.arg_list;
          if (temp != NULL)
             {
               printf ("   --- ms_attribute_arg #%d \n",count);
               summarize_ms_attribute_arg(temp);

               count++;
               temp = temp->next;
             }
        }
       else
        {
          if (ptr->kind == msak_custom)
             {
               printf ("     custom_info.type                      = %p \n",ptr->variant.custom_info.type);
               printf ("     custom_info.constructor               = %p \n",ptr->variant.custom_info.constructor);
               printf ("     custom_info.args                      = %p \n",ptr->variant.custom_info.args);
               printf ("     custom_info.named_args                = %p \n",ptr->variant.custom_info.named_args);

               int count = 0;
               a_custom_ms_attribute_arg_ptr temp = ptr->variant.custom_info.named_args;
               if (temp != NULL)
                  {
                    printf ("   --- ms_custom_attribute_arg #%d \n",count);
                    printf ("   --- field         = %p \n",temp->field);
                    printf ("   --- expression    = %p \n",temp->expression);

                    count++;
                    temp = temp->next;
                  }
             }
        }

     printf ("     source position: \n");
     summarize_source_position (ptr->position);

#if GENERATE_SOURCE_SEQUENCE_LISTS
     printf ("     source_sequence_entry                       = %p \n",ptr->source_sequence_entry);
#endif
     printf ("     kind                                        = %u \n",ptr->kind);
     printf ("     target                                      = %u \n",ptr->target);
   }
#endif


#if MICROSOFT_EXTENSIONS_ALLOWED
// DQ (8/16/2014): Adding support for MS attributes.
void summarize_ms_attribute_arg ( an_ms_attribute_arg_ptr ptr )
   {
     ROSE_ASSERT(ptr != NULL);

     printf ("     next                                           = %p \n",ptr->next);
     printf ("     param_name                                     = %s \n",ptr->param_name);

     if (ptr->kind == msaak_integer)
        {
          printf ("     integer_value = %lu \n",ptr->variant.integer_value);
        }

     if (ptr->kind == msaak_boolean)
        {
          printf ("     bool_value = %s \n",ptr->variant.bool_value ? "true" : "false");
        }

     if (ptr->kind == msaak_other)
        {
          printf ("     other_string = %s \n",ptr->variant.other_string);
        }

     if (ptr->kind == msaak_string)
        {
          printf ("     string_constant = %p \n",ptr->variant.string_constant);
        }

     if (ptr->kind == msaak_uuid)
        {
          printf ("     uuid_string = %s \n",ptr->variant.uuid_string);
        }

     if (ptr->kind == msaak_enumeration)
        {
          printf ("     enum_value = %u \n",ptr->variant.enum_value);
        }

     printf ("     kind                                           = %u \n",ptr->kind);
   }
#endif

// DQ (11/6/2014): Adding support for a_constructor_init_ptr+.
void summarize_constructor_init ( a_constructor_init_ptr ptr )
   {
     ROSE_ASSERT(ptr != NULL);

     printf ("     next                                           = %p \n",ptr->next);

     printf ("     kind                                           = %d \n",ptr->kind);

     printf ("     compiler_generated                             = %s \n",ptr->compiler_generated    ? "true" : "false");
     printf ("     is_pack_expansion                              = %s \n",ptr->is_pack_expansion     ? "true" : "false");
     printf ("     is_braced                                      = %s \n",ptr->is_braced             ? "true" : "false");
     printf ("     use_field_initializer                          = %s \n",ptr->use_field_initializer ? "true" : "false");

     if (ptr->kind == cik_virtual_base_class || ptr->kind == cik_direct_base_class)
        {
          a_base_class_ptr base_class = ptr->variant.base_class;
          printf ("     base_class                                           = %p \n",base_class);
        }
       else
        {
          if (ptr->kind == cik_field)
             {
               a_field_ptr field = ptr->variant.field;
               printf ("     field                                           = %p \n",field);
             }
            else
             {
               ROSE_ASSERT(ptr->kind == cik_delegation);
             }
        }

     printf ("     initializer                                    = %p \n",ptr->initializer);

  // TV (12/20/2023): Looks like this function has not been used since EDG 4.11.... 
  // DQ (10/5/2016): This is a name change in the EDG 4.11 IR.
     printf ("ROSE EDG 4.11 fixme: Unclear how this code is modified over the EDG 4.9 version of the code below! \n");
     ROSE_ASSERT(false);

     printf ("     orig_type                                      = %p \n",ptr->orig_type);

#if EXTRA_SOURCE_POSITIONS_IN_IL
     printf ("   ctor_init_range: \n");
     summarize_source_range(ptr->ctor_init_range);
#endif
   }


// DQ (3/18/2015): Adding support for a_scope_orphaned_list_header_ptr.
void summarize_scope_orphaned_list_header ( a_scope_orphaned_list_header_ptr ptr )
   {
     ROSE_ASSERT(ptr != NULL);

     printf ("In summarize_scope_orphaned_list_header(): ptr = %p \n",ptr);

     printf ("     next                         = %p \n",ptr->next);

     printf ("     assoc_routine                = %p \n",ptr->assoc_routine);
     printf ("     scope_number                 = %d \n",(int)ptr->scope_number);

     printf ("     orphaned_types               = %p \n",ptr->orphaned_types);
     printf ("     orphaned_variables           = %p \n",ptr->orphaned_variables);
     printf ("     orphaned_namespaces          = %p \n",ptr->orphaned_namespaces);

#if GENERATE_SOURCE_SEQUENCE_LISTS
     printf ("     orphaned_src_seq_sublists    = %p \n",ptr->orphaned_src_seq_sublists);
#endif
   }


// DQ (3/18/2015): Adding support for an_instantiation_directive_ptr.
void summarize_instantiation_directive ( an_instantiation_directive_ptr ptr )
   {
     ROSE_ASSERT(ptr != NULL);

     printf ("In summarize_instantiation_directive(): ptr = %p \n",ptr);

     printf ("     position: \n");
     summarize_source_position (ptr->position);

  // DQ (10/16/2012): List the different kinds of entities declared in this statement.
  // A list of tagged pointers to the entities declared by this statement.
  // a_tagged_pointer entities = ptr->entity;
     bool includePointerValue = true;
     string s = summarize_entry_kind_in_string_kind (ptr->entity,includePointerValue);
     printf ("entity = %s \n",s.c_str());

#if 0
  // DQ (10/16/2012): List the different kinds of entities declared in this statement.
     an_il_entity_list_entry_ptr entities = ptr->entity;
     if (entities != NULL)
        {
          while (entities != NULL)
             {
               a_tagged_pointer entity = entities->entity;
               printf ("        --- entities = %p \n",entities);
               printf ("           --- entity.kind = %d = %s \n",entity.kind,summarize_entry_kind_in_string_kind((an_il_entry_kind)(entity.kind)));
               printf ("           --- entity.ptr  = %p \n",entity.ptr);
               entities = entities->next;
             }
        }
#endif
#if 0
     if (ptr->entities != NULL)
        {
          printf ("associated entities: \n");
          a_tagged_pointer entity = ptr->entity;
          while (entity != NULL)
             {
               printf ("     --- entity = %p \n",entity);
               printf ("     --- entity.kind = %d = %s \n",entity.kind,summarize_entry_kind_in_string_kind((an_il_entry_kind)(entity.kind)));
               printf ("     --- entity.ptr  = %p \n",entity.ptr);
               entity = entity.next;
             }
        }
#endif

     printf ("     do_not_instantiate                     = %s \n",ptr->do_not_instantiate ? "true" : "false");
     printf ("     attributes  - - - - - - - - - - - - -  = %p \n",ptr->attributes);
     an_attribute_ptr attribute = ptr->attributes;
     while (attribute != NULL)
        {
          summarize_attribute(attribute);
          attribute = attribute->next;
        }

#if EXTRA_SOURCE_POSITIONS_IN_IL
     printf ("     decl_pos_info                          = %p \n",ptr->decl_pos_info);
     summarize_decl_pos_info (ptr->decl_pos_info);
#endif
   }



void
print_source_pos(a_source_position pos)
   {
  // DQ (10/20/2015): Note that EDG 4.9 changes the function argument types to conv_seq_to_file_and_line()
  // And makes the typedef represented by a_const_char be "const char" for C++11 and "char" for C++98.
     a_const_char* file_name_ptr_memory_location = NULL;
     a_const_char* full_name_ptr_memory_location = NULL;
     a_const_char** file_name = &file_name_ptr_memory_location;
     a_const_char** full_name = &full_name_ptr_memory_location;

     a_line_number ln = 0;
     a_boolean at_end = FALSE;

#if 0
  // DQ (7/13/2025): Now that we are using EDG 6.5, we should not need this.
  // DQ (10/11/2016): I think that some of these values are uninitialized (and thus out of range).
     if (pos.seq > 1000000)
        {
          printf ("ERROR: In print_source_pos(): pos.seq > 1000000 (and considered to be out of range, EDG 4.12 bug) \n");
          return;
        }
#endif

#if 0
     printf ("Calling conv_seq_to_file_and_line(): pos.seq = %lu \n",pos.seq);
#endif

  // conv_seq_to_file_and_line(pos.seq, &file_name, &full_name, &ln, &at_end);
     conv_seq_to_file_and_line(pos.seq, file_name, full_name, &ln, &at_end);

#if 0
     printf ("DONE: Calling conv_seq_to_file_and_line() \n");
#endif

     debug_print("%s:%u:%d", *file_name, ln, pos.column);

  // DQ (12/17/2012): Added assertion.
     ROSE_ASSERT(Sg_File_Info::get_nametofileid_map().find("") == Sg_File_Info::get_nametofileid_map().end());

#if 0
     printf ("Exiting as a test in print_source_pos() \n");
     ROSE_ASSERT(false);
#endif
   }


// DQ (5/13/2012): Added function to access parst of the source code position from EDG.
string
source_position_filename_with_path(a_source_position pos)
   {
  // DQ (10/20/2015): Note that EDG 4.9 changes the function argument types to conv_seq_to_file_and_line()
  // And makes the typedef represented by a_const_char be "const char" for C++11 and "char" for C++98.
     a_const_char* file_name_ptr_memory_location = NULL;
     a_const_char* full_name_ptr_memory_location = NULL;
     a_const_char** file_name = &file_name_ptr_memory_location;
     a_const_char** full_name = &full_name_ptr_memory_location;

     a_line_number ln = 0;
     a_boolean at_end = FALSE;

  // conv_seq_to_file_and_line(pos.seq, &file_name, &full_name, &ln, &at_end);
     conv_seq_to_file_and_line(pos.seq, file_name, full_name, &ln, &at_end);

  // DQ (12/17/2012): Added assertion.
     ROSE_ASSERT(Sg_File_Info::get_nametofileid_map().find("") == Sg_File_Info::get_nametofileid_map().end());

     return *full_name;
   }

void
print_source_corresp(a_source_correspondence sc)
   {
     a_source_position pos = sc.decl_position;
     print_source_pos(pos);
     debug_print(" \"%s\"", (sc.name ? sc.name : "(no name)"));

#if EXTRA_SOURCE_POSITIONS_IN_IL
  // DQ (11/4/2012): their is also additional information that is sometime available.
  // This points to a block containing additional source position information about the declaration, however it may be NULL.
     a_decl_position_supplement_ptr	sup = sc.decl_pos_info;

  // indent_line();
     debug_print("\n");
     debug_print("%2d %*s", EDG_ROSE_Translation::indent, 2 * EDG_ROSE_Translation::indent, "");
     debug_print("--- decl_position_supplement = %p : %s ", (sup != NULL) ? sup : NULL, (sup != NULL) ? "extra source position info" : "(no additional source position info available)");

  // DQ (12/12/2012): This appears to add a few too many extra lines.
  // debug_print("\n");

     if (sup != NULL)
        {
          bool output_value = (!source_range_null(sup->identifier_range)) || 
                              (!source_range_null(sup->specifiers_range)) ||
                              (!source_range_null(sup->variant.declarator_range));
#if 0
       // Debugging, it appears all the values are the same if ever they are non-zero.
          printf ("\nsup->identifier_range.start.seq       = %lu sup->identifier_range.end.seq         = %lu \n",sup->identifier_range.start.seq        ,sup->identifier_range.end.seq);
          printf ("sup->specifiers_range.start.seq         = %lu sup->specifiers_range.end.seq         = %lu \n",sup->specifiers_range.start.seq        ,sup->specifiers_range.end.seq);
          printf ("sup->variant.declarator_range.start.seq = %lu sup->variant.declarator_range.end.seq = %lu \n",sup->variant.declarator_range.start.seq,sup->variant.declarator_range.end.seq);
#endif
          if (output_value == true)
             {
               debug_print("\n");
               debug_print("%2d %*s", EDG_ROSE_Translation::indent, 2 * EDG_ROSE_Translation::indent, "");
               debug_print("--- additional source position info: ");

            // We only need to output one valid range.
               if (!source_range_null(sup->identifier_range))
                  {
#if 0
                    printf ("Output the identifier_range \n");
#endif
                    print_source_range(sup->identifier_range);
                  }
                 else
                  {
#if 0
                    printf ("Test the specifiers_range \n");
#endif
                    if (!source_range_null(sup->specifiers_range))
                       {
                      // debug_print("\n");
#if 0
                         printf ("Output the specifiers_range \n");
#endif
                         print_source_range(sup->specifiers_range);
                       }
                      else
                       {
#if 0
                         printf ("Test the declarator_range \n");
#endif
                         if (!source_range_null(sup->variant.declarator_range))
                            {
                           // debug_print("\n");
#if 0
                              printf ("Output the declarator_range \n");
#endif
                              print_source_range(sup->variant.declarator_range);
                            }
                       }
                  }
             }
        }
#endif

#if 0
     printf ("Leaving print_source_corresp(a_source_correspondence sc) \n");
#endif
   }


void
print_statement_position ( a_statement_ptr stmt )
   {
  // DQ (10/12/2016): This change is required fo the newer version of EDG 4.11, but still works with the older EDG 4.9 version as well.
  // a_stmt_source_position pos = stmt->position;
     a_source_position pos = stmt->position;

     print_source_pos(pos);
  // debug_print(" \"%s\"", (sc.name ? sc.name : "(no name)"));

#if EXTRA_SOURCE_POSITIONS_IN_IL
  // DQ (10/12/2016): This change is required fo the newer version of EDG 4.11, but still works with the older EDG 4.9 version as well.
  // a_stmt_source_position end_pos = stmt->end_position;
     a_source_position end_pos = stmt->end_position;
     print_source_pos(end_pos);
#endif
   }


void
print_source_range(a_source_range r)
   {
#if 1
  // DQ (1/23/2013): Adding source sequence number output to support debugging of default arguments in test2013_29.C.
     extern a_seq_number & global_source_sequence_number_reference;

     a_seq_number source_sequence_number = r.start.seq;
  // printf ("ssn = %" PRIuPTR " global_ssn = %" PRIuPTR " : ",source_sequence_number,global_source_sequence_number_reference);
     printf ("ssn = %lu global_ssn = %lu : ",(unsigned long)source_sequence_number,(unsigned long)global_source_sequence_number_reference);
#endif

#if 0
     printf ("In print_source_range(): Calling print_source_pos(r.start) \n");
#endif

     print_source_pos(r.start);

#if 0
     printf ("In print_source_range(): Calling debug_print() \n");
#endif

     debug_print(" -- ");

#if 0
     printf ("In print_source_range(): Calling print_source_pos(r.end) \n");
#endif

  // DQ (10/10/2016): This is a problem for EDG 4.11 (test2004_77.C) and 4.12 (test2004_33.C).
     print_source_pos(r.end);

#if 0
     printf ("Leaving print_source_range() \n");
#endif
   }


// DQ (3/21/2017): This function was moved from SourceSequenceList.C because it is related to debugging.
void
unexpected_end_of_construct(SourceSequenceList& sse)
   {
  // This is ONLY called from parse_statement(): case iek_src_seq_end_of_construct: but the point is that we should never reach this case.
     ROSE_ASSERT (sse->entity.kind == iek_src_seq_end_of_construct);
     ERROR_MESSAGE(stderr, "Bad end of construct at "); 
     print_source_pos(((a_src_seq_end_of_construct_ptr)(sse->entity.ptr))->position); 
     ERROR_MESSAGE(stderr, "\n");

  // abort();
     ROSE_ASSERT(false);
   }


// DQ (3/21/2017): This function was moved from SourceSequenceList.C because it is related to debugging.
string
output_expr_enum( an_expr_node_kind kind )
   {
     string s;
     s = summarize_expr_kind(kind);
     return s;
   }


// DQ (3/21/2017): This function was moved from SourceSequenceList.C because it is related to debugging.
string
output_type_enum( a_type_kind kind )
   {
     string s;
     s = summarize_type_kind_in_string_kind(kind);
     return s;
   }


// DQ (3/21/2017): This is debugging support code moved to cleanup code coverage results.
string
EDG_ROSE_Translation::outputSpecializationString(SgDeclarationStatement::template_specialization_enum spec)
   {
     string returnString;

     switch(spec)
        {
          case SgDeclarationStatement::e_unknown:                returnString = "e_unknown";                break;
          case SgDeclarationStatement::e_no_specialization:      returnString = "e_no_specialization";      break;
          case SgDeclarationStatement::e_partial_specialization: returnString = "e_partial_specialization"; break;
          case SgDeclarationStatement::e_specialization:         returnString = "e_specialization";         break;

          case SgDeclarationStatement::e_template_specialization_last:
             {
               returnString = "e_template_specialization_last";
#ifdef DEBUG_CONVERSION
               printf ("Error: this value should not be used \n");
#endif
               break;
             }

          default:
             {
               printf ("Default reached in switch: spec = %" PRIuPTR " \n",(size_t)spec);
               ROSE_ASSERT(false);
             }
        }

     return returnString;
   }


// DQ (3/21/2017): This is debugging support code moved to cleanup code coverage results.
void
EDG_ROSE_Translation::output_typeof_maps( const std::string & label )
   {
  // EXTERN std::map<a_type_ptr,      SgTypeOfType*> edg_type_to_rose_typeof_type_map;
  // EXTERN std::map<an_expr_node_ptr,SgTypeOfType*> edg_expression_to_rose_typeof_type_map;
     size_t counter = 0;
     std::map<a_type_ptr,      SgTypeOfType*>::iterator it_type_map = edg_type_to_rose_typeof_type_map.begin();
     std::map<an_expr_node_ptr,SgTypeOfType*>::iterator it_exp_map  = edg_expression_to_rose_typeof_type_map.begin();

     printf ("Output for typeof maps: label = %s \n",label.c_str());

     while (it_type_map != edg_type_to_rose_typeof_type_map.end())
        {
       // Output each element of the type typeof map
          printf ("   type map element [counter=%zu]: first = %p second = %p \n",counter,it_type_map->first,it_type_map->second);

          it_type_map++;
          counter++;
        }

     counter = 0;
     while (it_exp_map != edg_expression_to_rose_typeof_type_map.end())
        {
       // Output each element of the exp typeof map
          printf ("   exp map element [counter=%zu]: first = %p second = %p \n",counter,it_exp_map->first,it_exp_map->second);

          it_exp_map++;
          counter++;
        }
   }


// DQ (3/21/2017): This is debugging support code moved to cleanup code coverage results.
void
EDG_ROSE_Translation::outputTypesSetDisplay( const string & label )
   {
  // The problem is that not enough types (e.g. pointers to types) are not added to this list.
  // it is appears that the type must have been defined in the current location (since it is not in the list).

     printf ("In outputTypesSetDisplay(): label = %s \n",label.c_str());

     printf ("Elements in the outputTypesSet (edg types): \n");
     std::set<a_type_ptr>::iterator i = outputTypesSet.begin();
     while (i != outputTypesSet.end())
        {
          printf ("   --- i = %p = %s \n",*i,(*i)->source_corresp.name != NULL ? (*i)->source_corresp.name : "null");
          i++;
        }
   }



// DQ (2/10/2018): Adding more support for VLA types.
void summarize_vla_dimension(a_vla_dimension_ptr ptr)
   {
     ROSE_ASSERT(ptr != NULL);

     printf ("In summarize_vla_dimension()                = %p \n",ptr);

     printf ("     next                                   = %p \n",ptr->next);
     printf ("     type                                   = %p \n",ptr->type);
     printf ("     dimension_expr                         = %p \n",ptr->dimension_expr);

     printf ("     original_dimension                     = %p \n",ptr->original_dimension);
     if (ptr->original_dimension)
        {
          printf ("In summarize_vla_dimension(): nested call to summarize_vla_dimension(original_dimension) \n");
          summarize_vla_dimension(ptr->original_dimension);
        }

     printf ("     in_prototype_scope                     = %s \n",ptr->in_prototype_scope ? "true" : "false");

     printf ("     position: \n");
     summarize_source_position (ptr->position);

#if DO_IL_LOWERING
#if LOWER_VARIABLE_LENGTH_ARRAYS
     printf ("     total_number_of_elements               = %p \n",ptr->total_number_of_elements);
#else
     printf ("     dimension_variable                     = %p \n",ptr->dimension_variable);
#endif
#endif

   }


std::string summarize_token_kind(a_token_kind kind)
   {
     string s;
     switch (kind)
        {
          case tok_lbracket:              s = "["; break;
          case tok_rbracket:              s = "]"; break;
          case tok_lparen:                s = "("; break;
          case tok_rparen:                s = ")" ; break;
          case tok_period:                s = "."; break;
          case tok_arrow:                 s = "->"; break;
          case tok_plus_plus:             s = "++"; break;
          case tok_minus_minus:           s = "--"; break;
          case tok_ampersand:             s = "&"; break;
          case tok_star:                  s = "*"; break;
          case tok_plus:                  s = "+"; break;
          case tok_minus:                 s = "-"; break;
          case tok_compl:                 s = "~"; break;
          case tok_not:                   s = "!"; break;
          case tok_divide:                s = "/"; break;
          case tok_remainder:             s = "%"; break;
          case tok_shift_left:            s = "<<"; break;
          case tok_shift_right:           s = ">>"; break;
          case tok_lt:                    s = "<"; break;
          case tok_gt:                    s = ">"; break;
          case tok_le:                    s = "<="; break;
          case tok_ge:                    s = ">="; break;
          case tok_eq:                    s = "=="; break;
          case tok_ne:                    s = "!="; break;
          case tok_spaceship:             s = "<=>"; break;
          case tok_excl_or:               s = "^"; break;
          case tok_or:                    s = "|"; break;
          case tok_and_and:               s = "&&"; break;
          case tok_or_or:                 s = "||"; break;
          case tok_quest_mark:            s = "?"; break;
          case tok_colon:                 s = ":"; break;
          case tok_assign:                s = "="; break;
          case tok_times_assign:          s = "*="; break;
          case tok_divide_assign:         s = "/="; break;
          case tok_remainder_assign:      s = "%="; break;
          case tok_plus_assign:           s = "+="; break;
          case tok_minus_assign:          s = "-="; break;
          case tok_shift_left_assign:     s = "<<="; break;
          case tok_shift_right_assign:    s = ">>="; break;
          case tok_and_assign:            s = "&="; break;
          case tok_excl_or_assign:        s = "^="; break;
          case tok_or_assign:             s = "|="; break;
          case tok_comma:                 s = ","; break;
          case tok_sharp:                 s = "#"; break;
          case tok_paste:                 s = "##"; break;
       // The min and max operators are only recognized in GNU C++ mode.
          case tok_gnu_min:               s = "<?"; break;
          case tok_gnu_max:               s = ">?"; break;
#if ENABLE_CUDA_SUPPORT
          case tok_config_left:           s = "<<<"; break;
          case tok_config_right:          s = ">>>"; break;
          case tok_host:                  s = "__host__"; break;
          case tok_global:                s = "__global__"; break;
          case tok_device:                s = "__device__"; break;
          case tok_shared:                s = "__shared__"; break;
          case tok_local:                 s = "__local__"; break;
          case tok_constant:              s = "__constant__"; break;
#endif
       // Punctuators (standard, 3.1.6) that are not also operators:
          case tok_lbrace:                s = "{"; break;
          case tok_rbrace:                s = "}"; break;
          case tok_semicolon:             s = ";"; break;
          case tok_ellipsis:              s = "..."; break;

          default:
               printf ("Default reached in summarize_token_kind(a_token_kind kind) = %d) \n",(int)kind);
        }

     return s;
   }

