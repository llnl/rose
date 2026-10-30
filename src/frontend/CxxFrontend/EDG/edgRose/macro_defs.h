#ifdef ROSE_DEBUG_NEW_EDG_ROSE_CONNECTION
#  define DEBUG_CONVERSION
#endif

#define CIL

#if GNU_EXTENSIONS_ALLOWED
#  if !defined(__sun) && !defined(__aarch64__) && !defined(__arm__) && !defined(_MSC_VER)
#    if GNU_X86_ATTRIBUTES_ALLOWED == FALSE
#      error "Need GNU_X86_ATTRIBUTES_ALLOWED"
#    endif /* __sun */
#  endif
#  if !GNU_NAKED_ATTRIBUTE_ALLOWED
#    error "Need GNU_NAKED_ATTRIBUTE_ALLOWED"
#  endif
#  if !GNU_INIT_PRIORITY_ATTRIBUTE_ALLOWED
#    error "Need GNU_INIT_PRIORITY_ATTRIBUTE_ALLOWED"
#  endif
#  if !GNU_VISIBILITY_ATTRIBUTE_ALLOWED
#    error "Need GNU_VISIBILITY_ATTRIBUTE_ALLOWED"
#  endif
#  if !GNU_EXTENSIONS_ALLOWED
#    error "Need GNU_EXTENSIONS_ALLOWED"
#  endif
#endif

#if !RECORD_FORM_OF_NAME_REFERENCE
#  error "Need RECORD_FORM_OF_NAME_REFERENCE"
#endif

#if COMPILE_MULTIPLE_SOURCE_FILES
#  error "Need COMPILE_MULTIPLE_SOURCE_FILES == FALSE"
#endif

#if !MICROSOFT_EXTENSIONS_ALLOWED && !GNU_EXTENSIONS_ALLOWED
#  error "Test MICROSOFT_EXTENSIONS_ALLOWED if GNU_EXTENSIONS_ALLOWED is false"
#endif

#define COMPILING_NEW_EDG_ROSE_INTERFACE TRUE

#ifndef CFE
#  define CFE
#endif

#define ERROR_MESSAGE fprintf

#define USE_DEBUG_VERSION_OF_PRINTF 0
#if !USE_DEBUG_VERSION_OF_PRINTF
#  define debug_print printf
#endif

#ifdef DEBUG_CONVERSION
#  define DEBUG_ROSE_EDG_CONNECTION true
#else
#  define DEBUG_ROSE_EDG_CONNECTION false
#endif

//////////////////////////////////////////////////////////
// Below are controls for the convertion behavior

#ifndef WARNING_FOR_NONREAL_DEVEL
#  define WARNING_FOR_NONREAL_DEVEL 0
#endif

// Some simplified control over debugging output.
#define DEBUG_POP_SCOPE  0
#define DEBUG_PUSH_SCOPE 0
#define DEBUG_PRINT_SCOPE_STACK 0
#define DEBUG_NONDEFINING_TEMPLATE_DECLARATION_MAP 0
#define DEBUG_STRUCTURAL_DESIGN 0
#define DEBUG_CLASSES_FROM_TEMPLATE_PARAMETERS 0
#define DEBUG_GET_DECLARATION_SCOPE 0
#define DEBUG_VARIABLE_CACHE 0

#define OUTPUT_DEFINING_AND_NONDEFINING_DECLARATIONS 0

#define MAKE_REDUNDANT_CALLS_TO_DETECT_TRANSFORAMTIONS 0

#define NEW_DUFF_DEVICE_SWITCH_SUPPORT 1
#define FIX_FOR_RC_107 0

//////////////////////////////////////////////////////////
// Below are checks for various macros setting

#if 0

#  if RECORD_MACRO_INVOCATIONS
#    error "RECORD_MACRO_INVOCATIONS IS defined!"
#  else
#    error "RECORD_MACRO_INVOCATIONS is NOT defined!"
#  endif

#  if FULLY_RESOLVED_MACRO_POSITIONS
#    error "FULLY_RESOLVED_MACRO_POSITIONS IS defined!"
#  else
#    error "FULLY_RESOLVED_MACRO_POSITIONS is NOT defined!"
#  endif

#  if RECORD_MACROS_IN_IL
#    error "RECORD_MACROS_IN_IL IS defined!"
#  else
#    error "RECORD_MACROS_IN_IL is NOT defined!"
#  endif

#  if MACRO_INVOCATION_TREE_IN_IL
#    error "MACRO_INVOCATION_TREE_IN_IL IS defined!"
#  else
#    error "MACRO_INVOCATION_TREE_IN_IL is NOT defined!"
#  endif

#  if GNU_X86_ATTRIBUTES_ALLOWED
#    error "GNU_X86_ATTRIBUTES_ALLOWED IS defined as non-zero value!"
#  else
#    ifndef _MSC_VER
#      error "GNU_X86_ATTRIBUTES_ALLOWED is NOT defined or defined as zero value!"
#    endif
#  endif

#  if VLA_ALLOWED
#    error "Using VLA_ALLOWED == true --- This is the case we want: VLA support is ON."
#  else
#    ifndef VLA_ALLOWED
#      error "VLA_ALLOWED is not defined"
#    else
#      if !VLA_ALLOWED
#        error "Using VLA_ALLOWED == false --- Need to turn on VLA support."
#      endif
#    endif
#  endif

#  if NO_VLA_DIMENSION_TEMPORARIES_IN_FUNCTION_PROTOTYPES
#    error "Using NO_VLA_DIMENSION_TEMPORARIES_IN_FUNCTION_PROTOTYPES == true"
#  else
#    ifndef NO_VLA_DIMENSION_TEMPORARIES_IN_FUNCTION_PROTOTYPES
#      error "NO_VLA_DIMENSION_TEMPORARIES_IN_FUNCTION_PROTOTYPES is not defined"
#    else
#      if !NO_VLA_DIMENSION_TEMPORARIES_IN_FUNCTION_PROTOTYPES
#        error "Using NO_VLA_DIMENSION_TEMPORARIES_IN_FUNCTION_PROTOTYPES == false --- This is the case we want: VLA support is ON."
#      endif
#    endif
#  endif

#  if DEFAULT_VLA_ENABLED
#    error "Using DEFAULT_VLA_ENABLED == true"
#  else
#    ifndef DEFAULT_VLA_ENABLED
#      error "DEFAULT_VLA_ENABLED is not defined"
#    else
#      if !DEFAULT_VLA_ENABLED
#        error "Using DEFAULT_VLA_ENABLED == false --- Need to turn on VLA support."
#      endif
#    endif
#  endif

#  if GCC_BUILTIN_VARARGS
#    warning "GCC_BUILTIN_VARARGS is TRUE"
#  else
#    error "GCC_BUILTIN_VARARGS is FALSE"
#  endif

#  if GCC_BUILTIN_VARARGS_IN_GENERATED_CODE
#    warning "GCC_BUILTIN_VARARGS_IN_GENERATED_CODE is TRUE"
#  else
#    error "GCC_BUILTIN_VARARGS_IN_GENERATED_CODE is FALSE"
#  endif
 
#  if BUILTIN_FUNCTIONS_ENABLED
#    warning "BUILTIN_FUNCTIONS_ENABLED is TRUE"
#  else
#    warning "BUILTIN_FUNCTIONS_ENABLED is FALSE"
#  endif

#endif