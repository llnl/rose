
#include "edgRose.h"

std::string EDG_ROSE_Translation::convert_string_constant(a_constant_ptr e)
   {
     ROSE_ASSERT (e->kind == ck_string);

  // For wchar_t elements this will be a string that is 2 or 4 times too long (with nulls).
     string value(e->variant.string.value, e->variant.string.length);

#if 0
     printf ("In convert_string_constant(): value of string from edg: value = %s length() = %" PRIuPTR " \n",value.c_str(),value.length());
#endif

#if 0
     printf ("In convert_string_constant(): Calling summarize_constant(e) \n");
     summarize_constant(e);
#endif

#if 0
     string tmp_value = escapeString(value);
     printf ("In convert_string_constant(): view the escaped string: tmp_value = %s length() = %" PRIuPTR " \n",tmp_value.c_str(),value.length());
#endif

     ROSE_ASSERT(e->type->kind == tk_array);
     a_type_ptr referenced_element_type = e->type->variant.array.element_type;
     ROSE_ASSERT(referenced_element_type != NULL);

  // value = escapeString(value);
     bool is_wide_character = isWideChar(referenced_element_type);

#if 0
     printf ("In convert_string_constant(): is_wide_character = %s \n",is_wide_character ? "true" : "false");
#endif

     if (is_wide_character == true)
        {
          size_t target_char_size = (unsigned long) referenced_element_type->size;
#if 0
          printf ("target_char_size = %" PRIuPTR " \n",target_char_size);
#endif
          size_t array_size = e->variant.string.length;
          size_t n = array_size/target_char_size;
       // basic_string<wchar_t> wstr ( (wchar_t*) e->variant.string.value, n);
#if 0
          printf ("array_size = %" PRIuPTR " target_char_size = %" PRIuPTR " n = %" PRIuPTR " \n",array_size,target_char_size,n);
#endif
          wstring wstr ( (wchar_t*) e->variant.string.value, n);
#if 0
       // This is supposed to do the conversion, but it does not work for me.
          wstring wstr(value.begin(), value.end());
          string str(wstr.begin(), wstr.end());
#endif
          string str;
       // wstring wstr(value.begin(), value.end());
       // wstring wstr(e->variant.string.value, e->variant.string.length);
          wstring::iterator i = wstr.begin();
          while (i != wstr.end())
             {
#if 0
               printf ("In loop over elements of wide character string \n");
#endif
               str += char(*i);
               i++;
             }

          value = str;
        }

#if 0
     printf ("In convert_string_constant(): before removing the  trailing null character: e->type = %p is_wide_character = %s value = %s \n",
          e->type,is_wide_character ? "true" : "false",value.c_str());
#endif

  // EDG strings can include the trailing NULL when copied to the C++ std::string object, 
  // so we have to check for this and remove it, but this must be done for the string 
  // value after translation from wide character arrays (wchar_t arrays).
  // This is the original code that was here before (this can't be used for wide character arrays.
     if (!value.empty() && value[value.size() - 1] == '\0')
        {
#if 0
          printf ("Removing the  trailing null character in the string copied from EDG (is_wide_character == false) \n");
#endif
          value.resize(value.size() - 1); // Remove null byte
        }

#if 0
     printf ("In convert_string_constant(): before escapeString(): e->type = %p is_wide_character = %s value = %s \n",e->type,is_wide_character ? "true" : "false",value.c_str());
#endif
#if 0
     printf ("In convert_string_constant(): before escapeString(): e->variant.string.length = %" PRIuPTR " \n",(size_t)(e->variant.string.length));
     printf ("In convert_string_constant(): before escapeString(): value = %s length() = %" PRIuPTR " \n",value.c_str(),value.length());
#endif

  // We have to escape anything that would be expanded to be an error in the generated code (e.g. \n, \t, \0, etc.).
     value = escapeString(value);

#if 0
     printf ("In convert_string_constant(): after escapeString(): e->type = %p is_wide_character = %s value = %s \n",e->type,is_wide_character ? "true" : "false",value.c_str());
#endif


#if 0
     printf ("In convert_string_constant(): after escapeString(): e->variant.string.length = %" PRIuPTR " \n",(size_t)(e->variant.string.length));
     printf ("In convert_string_constant(): after escapeString(): value = %s length() = %" PRIuPTR " \n",value.c_str(),value.length());
#endif

#if 0
     printf ("In convert_string_constant(): case ck_string: calling summarize_constant() \n");
     summarize_constant(e);
#endif
#if 0
     printf ("In convert_string_constant(): case ck_string: calling summarize_type(e->type) \n");
     summarize_type(e->type);
#endif

#ifdef DEBUG_CONVERSION
     indent_line(); debug_print("generate string "); debug_print("= \"%s\" ",value.c_str()); print_source_corresp(e->source_corresp); debug_print("\n");
#endif

     return value;
   }
