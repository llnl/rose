/* -*- c -*-
 *
 * If new macros are defined they should be defined using either define or cmakedefine.  Define macros will always be defined
 * however they are set in this file, while cmakedefine will be set or left unset based on the cmake configurations step
 * (running cmake before running make within the build process). Values should not be hardcoded and should be based on
 * the cmake configuration. Do not include conditional logic in this file.
 *
 * Since this file is contains name-space pollution (symbols not beginning with "ROSE_") there is a separate file
 * (rosePublicConfig.h.in.cmake) to create a non-polluting file (rosePublicConfig.h) for inclusion into user code (either directly 
 * or via other ROSE public header files). Defines in rosePublicConfig.h MUST have "ROSE_" at the start.
 * 
 */

#include <rosePublicConfig.h>                           // CPP macros that start with "ROSE_" so we don't have to repeat them here

// REQUIRED LIBS that should be found in CMakeLists.txt -- find_package() */ 
/* Define to 1 if you have the `dl' library (-ldl). */
#cmakedefine HAVE_LIBDL @HAVE_LIBDL@

// TPL, also located with find_package() *
/* Define if the Boost library is available, convert the cmake output Boost_FOUND to our custom HAVE_BOOST */
#cmakedefine HAVE_BOOST @Boost_FOUND@

/* Define if the Boost::Date_Time library is available */
#cmakedefine HAVE_BOOST_DATE_TIME @Boost_DATE_TIME_FOUND@

/* define if multithread support */ 
#cmakedefine _REENTRANT @_REENTRANT@ 

/* Define if the Boost::Filesystem library is available */
#cmakedefine HAVE_BOOST_FILESYSTEM @Boost_FILESYSTEM_FOUND@

/* Define if the Boost::Program_Options library is available */
#cmakedefine HAVE_BOOST_PROGRAM_OPTIONS @Boost_PROGRAM_OPTIONS_FOUND@

/* Define if the Boost::Regex library is available */
#cmakedefine HAVE_BOOST_REGEX @Boost_REGEX_FOUND@

/* Define if the Boost::System library is available */
#cmakedefine HAVE_BOOST_SYSTEM @Boost_SYSTEM_FOUND@

/* Define if the Boost::Thread library is available */
#cmakedefine HAVE_BOOST_THREAD @Boost_THREAD_FOUND@

/* Define if the Boost::Wave library is available */
#cmakedefine HAVE_BOOST_WAVE @Boost_WAVE_FOUND@

/* Define if the ROSE Boost Wave support is enabled */
#cmakedefine USE_ROSE_BOOST_WAVE_SUPPORT @USE_ROSE_BOOST_WAVE_SUPPORT@

/* Define if the boost::serialization library is available */
#cmakedefine HAVE_BOOST_SERIALIZATION_LIB @Boost_SERIALIZATION_FOUND@

/* HEADERS_TO_CHECK 
 * These variables are defined to 1 if the respective header file is found
 * See headers_to_check in cmake/ConfigureChecks.cmake
 */

/* Define to 1 if you have the <argz.h> header file. */
#cmakedefine HAVE_ARGZ_H @HAVE_ARGZ_H@ 
/* These will be defined to 1 if we have the header above */ 
#cmakedefine HAVE_ARGZ_APPEND @HAVE_ARGZ_APPEND@
#cmakedefine HAVE_ARGZ_CREATE_SEP @HAVE_ARGZ_CREATE_SEP@
#cmakedefine HAVE_ARGZ_INSERT @HAVE_ARGZ_INSERT@
#cmakedefine HAVE_ARGZ_NEXT @HAVE_ARGZ_NEXT@
#cmakedefine HAVE_ARGZ_STRINGIFY @HAVE_ARGZ_STRINGIFY@
#cmakedefine HAVE_WORKING_ARGZ @HAVE_WORKING_ARGZ@

/* Define to 1 if you have the <c_asm.h> header file. */
#cmakedefine HAVE_C_ASM_H @HAVE_C_ASM@

/* HAVE_C_TYPE_H gets defined to 1 if <ctype.h> found  */
#cmakedefine HAVE_CTYPE_H @HAVE_CTYPE_H@

/* Define to 1 if you have the <dirent.h> header file, and it defines `DIR'. */
#cmakedefine HAVE_DIRENT_H @HAVE_DIRENT_H@

/* Define to 1 if you have the <dlfcn.h> header file. */
#cmakedefine HAVE_DLFCN_H @HAVE_DLFCN_H@

/* Define to 1 if you have the <dl.h> header file. */
#cmakedefine HAVE_DL_H @HAVE_DL_H@

/* Define to 1 if you have the <intrinsics.h> header file. */
#cmakedefine HAVE_INTRINSICS_H @HAVE_INTRINSICS_H@

/* Define to 1 if you have the <stdlib.h> header file. */
#cmakedefine HAVE_STDLIB_H @HAVE_STDLIB_H@

/* Define to 1 if you have the <sys/time.h> header file. */
#cmakedefine HAVE_SYS_TIME_H @HAVE_SYS_TIME_H@

/* Define to 1 if you have the <sys/utime.h> header file. */
#cmakedefine HAVE_SYS_UTIME_H @HAVE_SYS_UTIME_H@

/* Define to 1 if you have the `time_base_to_time' function. */
#cmakedefine HAVE_TIME_BASE_TO_TIME @HAVE_TIME_BASE_TO_TIME@

// The remaining headers below are not currently part of 
// cmake/ConfigureChecks.cmake. Some are processed in subdirectory cmake
// files, some may eventually be removed if they were intended for Autoconf  

/* Define to 1 if you have the <memory.h> header file. */
#cmakedefine HAVE_MEMORY_H @HAVE_MEMORY_H@
/* End of HEADERS_TO_CHECK */


/* FUNCTIONS_TO_CHECK   
 * These variables are defined to 1 if the respective function is found
 * See functions_to_check in cmake/ConfigureChecks.cmake
 */
/* Define to 1 if you have the `clock_gettime' function. */
#cmakedefine HAVE_CLOCK_GETTIME @HAVE_CLOCK_GETTIME@

/* Whether <dlfcn.h> and -ldl contain dladdr() */
#cmakedefine HAVE_DLADDR @HAVE_DLADDR@

/* Use explicit template instantiation for MSTL, ASTProcessing, and ROSETTA */
#cmakedefine HAVE_EXPLICIT_TEMPLATE_INSTANTIATION @HAVE_EXPLICIT_TEMPLATE_INSTANTIATION@

#cmakedefine HAVE_GETHRTIME 

/* Define to 1 if you have the `mach_absolute_time' function. */
#cmakedefine HAVE_MACH_ABSOLUTE_TIME @HAVE_MACH_ABSOLUTE_TIME@

/* End of FUNCTIONS_TO_CHECK */

// BACKEND SECTION
/* Define if building with Jovial support */ 
#cmakedefine ATERM_BIN_PATH "@ATERM_BIN_PATH@"
#cmakedefine ATERM_INSTALL_PATH "@ATERM_INSTALL_PATH@"
#cmakedefine ATERM_LIBRARY_PATH "@ATERM_LIBRARY_PATH@"

// defines for jovial_support.C 
#cmakedefine STRATEGO_BIN_PATH "@STRATEGO_BIN_PATH@"

// defines for cmdline.C
#cmakedefine BACKEND_JOVIAL_COMPILER_NAME_WITH_PATH "@BACKEND_JOVIAL_COMPILER_NAME_WITH_PATH@"

/* Define if building with Ada support */ 
#cmakedefine BACKEND_ADA_COMPILER_NAME_WITH_PATH "@BACKEND_ADA_COMPILER_NAME_WITH_PATH@"
#cmakedefine ROSE_EXPERIMENTAL_ADA_ROSE_CONNECTION 

/* Define this to "no" typically */ 
#cmakedefine BACKEND_FORTRAN_IS_GNU_COMPILER @BACKEND_FORTRAN_IS_GNU_COMPILER@

/* Backend C and C++ (CXX) Macros */
/* BACKEND CXX -- No quotes around major, minor version numbers */
#cmakedefine BACKEND_CXX_IS_GNU_COMPILER @BACKEND_CXX_IS_GNU_COMPILER@ 
#cmakedefine BACKEND_CXX_IS_CLANG_COMPILER @BACKEND_CXX_IS_CLANG_COMPILER@
#cmakedefine BACKEND_CXX_IS_INTEL_COMPILER @BACKEND_CXX_IS_INTEL_COMPILER@

#define BACKEND_CXX_COMPILER_NAME_WITH_PATH "@BACKEND_CXX_COMPILER_NAME_WITH_PATH@"
#define BACKEND_CXX_COMPILER_NAME_WITHOUT_PATH "@BACKEND_CXX_COMPILER_NAME_WITHOUT_PATH@"
#define BACKEND_CXX_COMPILER_MAJOR_VERSION_NUMBER @BACKEND_CXX_COMPILER_MAJOR_VERSION_NUMBER@
#define BACKEND_CXX_COMPILER_MINOR_VERSION_NUMBER @BACKEND_CXX_COMPILER_MINOR_VERSION_NUMBER@
#define BACKEND_CXX_COMPILER_PATCH_VERSION_NUMBER @BACKEND_C_COMPILER_PATCH_VERSION_NUMBER@ 
#define ROSE_BACKEND_CXX_COMPILER "@ROSE_BACKEND_CXX_COMPILER@"
/* BACKEND C */ 
#define BACKEND_C_COMPILER_NAME_WITH_PATH "@BACKEND_C_COMPILER_NAME_WITH_PATH@" 
#define BACKEND_C_COMPILER_NAME_WITHOUT_PATH "@BACKEND_C_COMPILER_NAME_WITHOUT_PATH@"
#define BACKEND_C_COMPILER_MAJOR_VERSION_NUMBER @BACKEND_C_COMPILER_MAJOR_VERSION_NUMBER@
#define BACKEND_C_COMPILER_MINOR_VERSION_NUMBER @BACKEND_C_COMPILER_MINOR_VERSION_NUMBER@ 
#define BACKEND_C_COMPILER_PATCH_VERSION_NUMBER @BACKEND_C_COMPILER_PATCH_VERSION_NUMBER@ 
#define ROSE_BACKEND_C_COMPILER "@ROSE_BACKEND_C_COMPILER@"

/* (May or may not explicit include path; used to call backend). */
#define BACKEND_FORTRAN_COMPILER_NAME_WITH_PATH "${CMAKE_Fortran_COMPILER}"

#define BACKEND_FORTRAN_COMPILER_NAME_WITHOUT_PATH "${BACKEND_FORTRAN_COMPILER_NAME_WITHOUT_PATH}"

/* Major version number of backend Fortran compiler. */
#define BACKEND_FORTRAN_COMPILER_MAJOR_VERSION_NUMBER ${BACKEND_FORTRAN_COMPILER_MAJOR_VERSION_NUMBER}

/* Minor version number of backend Fortran compiler. */
#define BACKEND_FORTRAN_COMPILER_MINOR_VERSION_NUMBER ${BACKEND_FORTRAN_COMPILER_MINOR_VERSION_NUMBER}

/* Name of backend Java compiler including path */
#define BACKEND_JAVA_COMPILER_NAME_WITH_PATH "${BACKEND_JAVA_COMPILER}"

/* Name of backend Python interpreter including path */
#define BACKEND_PYTHON_INTERPRETER_NAME_WITH_PATH "${BACKEND_PYTHON_INTERPRETER_NAME_WITH_PATH}"
/* END OF BACKEND SECTION */

/* Include path for backend C++ compiler. */
#cmakedefine CXX_INCLUDE_STRING @CXX_INCLUDE_STRING@

/* Include path for backend C compiler. */
#cmakedefine C_INCLUDE_STRING @C_INCLUDE_STRING@

/* Define to 1 if MySQL libraries are available */
#cmakedefine MYSQL_FOUND
#ifdef MYSQL_FOUND
  #define HAVE_MYSQL
#endif

/* Have the SQLITE3 library */
#cmakedefine HAVE_SQLITE3

/* Define to 1 if you have the POSIX.1003 header file, <pthread.h> */
#cmakedefine HAVE_PTHREAD_H 

/* Prefix path for use of IDA. */
#define IDA_PRO_PATH "${with_ida}"

/* Major version number of the Java JDK. */
#define JAVA_VERSION_MAJOR "@Java_VERSION_MAJOR@"

/* Path to JVM executable */
#define JAVA_JVM_PATH "${JAVA_RUNTIME}"

// Using define always defines as opposed to cmakedefine (conditional)
/* The size of `char', as computed by sizeof. */
#define SIZEOF_CHAR ${SIZEOF_CHAR}

/* The size of `double', as computed by sizeof. */
#define SIZEOF_DOUBLE ${SIZEOF_DOUBLE}

/* The size of `float', as computed by sizeof. */
#define SIZEOF_FLOAT ${SIZEOF_FLOAT}

/* The size of `int', as computed by sizeof. */
#define SIZEOF_INT ${SIZEOF_INT}

/* The size of `long', as computed by sizeof. */
#define SIZEOF_LONG ${SIZEOF_LONG}

/* The size of `long double', as computed by sizeof. */
#define SIZEOF_LONG_DOUBLE ${SIZEOF_LONG_DOUBLE}

/* The size of `long long', as computed by sizeof. */
#define SIZEOF_LONG_LONG ${SIZEOF_LONG_LONG}

/* The size of `short', as computed by sizeof. */
#define SIZEOF_SHORT ${SIZEOF_SHORT}

/* The size of `void *', as computed by sizeof. */
#define SIZEOF_VOID_P ${SIZEOF_VOID_P}

/* Define to 1 if you have the ANSI C header files. */
#cmakedefine STDC_HEADERS @STDC_HEADERS@ 

/* Define to 1 if you can safely include both <sys/time.h> and <time.h>. */
#cmakedefine TIME_WITH_SYS_TIME @TIME_WITH_SYS_TIME@ 

/* Define to 1 if your <sys/time.h> declares `struct tm'. */
#cmakedefine TM_IN_SYS_TIME @TM_IN_SYS_TIME@

/* Mark that GFORTRAN is available */
#cmakedefine USE_GFORTRAN_IN_ROSE @USE_GFORTRAN_IN_ROSE@ 

/* Support for Purify */
#cmakedefine USE_PURIFY

/* Support for Purify API (Compiling with the Purify library). */
#cmakedefine USE_PURIFY_API

/* Link-time support for Purify. */
#cmakedefine USE_PURIFY_LINKER

/* Use of GCC OpenMP in ROSE. */
//AS Don't know what to do with this
#cmakedefine USE_ROSE_GCC_OMP

/* Controls use of IDA support for disassembling. */
//AS Don't know what to do with this
#cmakedefine USE_ROSE_IDA_SUPPORT

/* Controls use of ROSE support for Java. */
#cmakedefine JAVA_RUNTIME
#ifdef JAVA_RUNTIME
  #define USE_ROSE_JAVA_SUPPORT
#endif

/* Version number of package */
//AS Don't know what to do with this
#define VERSION "0.9.10"

/* Define WORDS_BIGENDIAN to 1 if your processor stores words with the most
   significant byte first (like Motorola and SPARC, unlike Intel). */
#if defined AC_APPLE_UNIVERSAL_BUILD
# if defined __BIG_ENDIAN__
#  define WORDS_BIGENDIAN 1
# endif
#else
# ifndef WORDS_BIGENDIAN
#  undef WORDS_BIGENDIAN
# endif
#endif

/* Controls if EDG Union/Struct debugging support is to be used. */
#cmakedefine01 USE_ROSE_EDG_DEBUGGING_SUPPORT

/* Defined if we're using a prebuilt EDG library */
#cmakedefine BINARY_EDG

/* Internal EDG specific consistency tests. */
//AS FIXME
#define CHECKING 1

/* -D options to hand to EDG C++ front-end. */
#define CXX_SPEC_DEF {"-D__GNUG__=${BACKEND_CXX_COMPILER_MAJOR_VERSION_NUMBER}", "-D__GNUC__=${BACKEND_C_COMPILER_MAJOR_VERSION_NUMBER}", "-D__GNUC_MINOR__=${BACKEND_C_COMPILER_MINOR_VERSION_NUMBER}", "-D__GNUC_PATCHLEVEL__=${BACKEND_C_COMPILER_PATCH_LEVEL_NUMBER}", "--preinclude", "rose_edg_required_macros_and_functions.h"}

/* Defined if we're building using CMake */
#cmakedefine USE_CMAKE @USE_CMAKE@

/* Build ROSE to support the Java langauge */
#cmakedefine USE_ROSE_INTERNAL_JAVA_SUPPORT

/* Detect whether our compilers are GNU or not */
#cmakedefine CMAKE_COMPILER_IS_GNUCC
#cmakedefine CMAKE_COMPILER_IS_GNUCXX
#cmakedefine CMAKE_COMPILER_IS_GNUG77

#define SAGE_TRANSFORM 1

/* Define to 1 if you have the `read_real_time' function. */
#cmakedefine HAVE_READ_REAL_TIME @HAVE_READ_REAL_TIME@ 
 
/* Define to 1 if you have the <machine/endian.h> header file. */
#cmakedefine HAVE_MACHINE_ENDIAN_H @HAVE_MACHINE_ENDIAN_H@ 

/* Define to 1 if you have the <mach/mach_time.h> header file. */
#cmakedefine HAVE_MACH_MACH_TIME_H @HAVE_MACH_MACH_TIME_H@ 

/* Define to indicate that backend compiler uses "asm" syntax */
#cmakedefine BACKEND_C_COMPILER_SUPPORTS_LONG_STRING_ASM

/* Define to indicate that backend compiler uses "__asm__" syntax */
#cmakedefine BACKEND_C_COMPILER_SUPPORTS_UNDESCORE_ASM

// Define to indicate that "rose_config.h" has been included and is polluting the global name space.
#define CONFIG_ROSE 1
