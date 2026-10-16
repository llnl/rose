
#-----------------------------------------------------------------------------
AC_DEFUN([ROSE_SUPPORT_ROSE_PART_1],
[
# Begin macro ROSE_SUPPORT_ROSE_PART_1.

# Used by Rose::initialize. See the top-level CMakeLists.txt for more details.
ROSE_CONFIG_TOKEN="rose-$(cat $srcdir/ROSE_VERSION)"

# *********************************************************************
# This macro encapsulates the complexity of the tests required for ROSE
# to understnd the machine environment and the configure command line.
# It is represented a s single macro so that we can simplify the ROSE
# configure.in and permit other external project to call this macro as
# a way to set up there environment and define the many macros that an
# application using ROSE might require.
# *********************************************************************

AMTAR="$TAR"

if test "$am__tar" = "false"; then
   AC_MSG_FAILURE([am__tar set to false])
fi

if test "$am__untar" = "false"; then
   AC_MSG_FAILURE([am__untar set to false])
fi

AC_CANONICAL_BUILD

AC_MSG_CHECKING([machine hardware cpu])
AC_MSG_RESULT([$build_cpu])

AC_MSG_CHECKING([operating system vendor])
AC_MSG_RESULT([$build_vendor])

AC_MSG_CHECKING([operating system])
AC_MSG_RESULT([$build_os])

DETERMINE_OS

AM_CONDITIONAL(ROSE_BUILD_OS_IS_OSX,    [test "x$build_vendor" = xapple])
AM_CONDITIONAL(ROSE_BUILD_OS_IS_CYGWIN, [test "x$build_os" = xcygwin])

DETERMINE_OS_VENDOR

ROSE_SUPPORT_SSL

# Need the SSL automake conditional so that libssl can be added selectively for only those
# translators that require it (since it conflicts with use of Java, and thus Fortran support).
AM_CONDITIONAL(ROSE_USE_SSL_SUPPORT, [test "x$enable_ssl" = xyes])

configure_date=`date '+%A %B %e %H:%M:%S %Y'`
AC_SUBST(configure_date)
# echo "In ROSE/con figure: configure_date = $configure_date"

if test "$prefix" = NONE; then
   AC_MSG_NOTICE([setting prefix to default: "$PWD"])
   prefix="$PWD"
fi

# Call supporting macro for the Java path required by the Open Fortran Parser (for Fortran 2003 support)
# Use our classpath in case the user's is messed up
AS_SET_CATFILE([ABSOLUTE_SRCDIR], [`pwd`], [${srcdir}])

ROSE_CONFIGURE_SECTION([Checking GNU Fortran])
AX_WITH_PROG(GFORTRAN_PATH, [gfortran], [])
AC_SUBST(GFORTRAN_PATH)

if test "x$GFORTRAN_PATH" != "x" -a "$GFORTRAN_PATH" != "no"; then
   AC_DEFINE([USE_GFORTRAN_IN_ROSE], [1], [Mark that GFORTRAN is available])
else
   AC_DEFINE([USE_GFORTRAN_IN_ROSE], [0], [Mark that GFORTRAN is not available])
fi

AC_MSG_NOTICE([GFORTRAN_PATH = "$GFORTRAN_PATH"])

#########################################################################################
##
  ROSE_SUPPORT_LANGUAGE_CONFIG_OPTIONS

  ROSE_CONFIGURE_SECTION([])
  AC_CHECK_LIB([curl], [Curl_connect], [HAVE_CURL=yes], [HAVE_CURL=no])
  AM_CONDITIONAL([HAS_LIBRARY_CURL], [test "x$HAVE_CURL" = "xyes"])

  ROSE_SUPPORT_JAVA # This macro uses JAVA_HOME
  ROSE_SUPPORT_COMPASS2
  ROSE_SUPPORT_GMP
  ROSE_SUPPORT_ISL
  ROSE_SUPPORT_MPI
  ROSE_SUPPORT_SPOT

# UPC is in the process of being removed.
# The test files are still present and probably should be removed [Rasmussen, 2026.05.07].
# ROSE_SUPPORT_UPC

##
#########################################################################################

# Check on the modified GNU Octave parser.
ROSE_SUPPORT_OCTAVE

# *******************************************************
# ROSE/projects directory compilation & testing
# *******************************************************
ROSE_ARG_ENABLE(
  [projects-directory],
  [if we should enable the ROSE/projects directory],
  [Toggle compilation and testing of the the ROSE/projects directory (disabled by default)],
  [no])

if test "x$ROSE_ENABLE_PROJECTS_DIRECTORY" = "xyes"; then
   AC_DEFINE([ROSE_BUILD_PROJECTS_DIRECTORY_SUPPORT], [], [Build ROSE projects directory])
fi
AM_CONDITIONAL(ROSE_BUILD_PROJECTS_DIRECTORY_SUPPORT, [test "x$ROSE_ENABLE_PROJECTS_DIRECTORY" = "xyes"])

# ****************************************************
# ROSE/tests directory compilation & testing
# ****************************************************
AC_MSG_CHECKING([if we should build & test the ROSE/tests directory])
AC_ARG_ENABLE([tests-directory],AS_HELP_STRING([--disable-tests-directory],[Disable compilation and testing of the ROSE/tests directory]),[],[enableval=yes])
support_tests_directory=yes
AC_CHECK_FILE([$srcdir/tests/Readme.md],[],[enableval=no])
if test "x$enableval" = "xyes"; then
   support_tests_directory=yes
   AC_MSG_RESULT(enabled)
   AC_DEFINE([ROSE_BUILD_TESTS_DIRECTORY_SUPPORT], [yes], [Build ROSE tests directory])
else
   support_tests_directory=no
   AC_MSG_RESULT(disabled)
   AC_DEFINE([ROSE_BUILD_TESTS_DIRECTORY_SUPPORT], [no], [Build ROSE tests directory])
   enableval=yes 
fi
AM_CONDITIONAL(ROSE_BUILD_TESTS_DIRECTORY_SUPPORT, [test "x$support_tests_directory" = xyes])

# *******************************************************
# ROSE/tutorial directory compilation & testing
# *******************************************************
AC_MSG_CHECKING([if we should build & test the ROSE/tutorial directory])
AC_ARG_ENABLE([tutorial-directory],AS_HELP_STRING([--disable-tutorial-directory],[Disable compilation and testing of the ROSE/tutorial directory]),[],[enableval=yes])
support_tutorial_directory=yes
if test "x$enableval" = "xyes"; then
   support_tutorial_directory=yes
   AC_MSG_RESULT(enabled)
   AC_DEFINE([ROSE_BUILD_TUTORIAL_DIRECTORY_SUPPORT], [], [Build ROSE tutorial directory])
else
   support_tutorial_directory=no
   AC_MSG_RESULT(disabled)
fi
AM_CONDITIONAL(ROSE_BUILD_TUTORIAL_DIRECTORY_SUPPORT, [test "x$support_tutorial_directory" = xyes])

# ************************************************************
# Option to turn on a special mode of memory pools: no reuse of deleted memory.
# This is useful to track AST nodes during transformation, otherwise the same memory may be reused
# by multiple different AST nodes.
# Liao 8/13/2014
# ************************************************************

AC_ARG_ENABLE(memoryPoolNoReuse, AS_HELP_STRING([--enable-memory-pool-no-reuse], [Enable special memory pool model: no reuse of deleted memory (default is to reuse memory)]))
AM_CONDITIONAL(ROSE_USE_MEMORY_POOL_NO_REUSE, [test "x$enable_memory_pool_no_reuse" = xyes])
if test "x$enable_memory_pool_no_reuse" = "xyes"; then
  AC_MSG_WARN([turn on a special mode in memory pools: no reuse of deleted memory blocks])
  AC_DEFINE([ROSE_USE_MEMORY_POOL_NO_REUSE], [], [Whether to use a special no-reuse mode of memory pools])
fi

AC_ARG_ENABLE(internalFrontendDevelopment, AS_HELP_STRING([--enable-internalFrontendDevelopment], [Enable development mode to reduce files required to support work on language frontends]))
AM_CONDITIONAL(ROSE_USE_INTERNAL_FRONTEND_DEVELOPMENT, [test "x$enable_internalFrontendDevelopment" = xyes])
if test "x$enable_internalFrontendDevelopment" = "xyes"; then
  AC_MSG_WARN([using reduced set of files to support faster development of language frontend work; e.g. new EDG version 4.3 to translate EDG to ROSE (internal use only)!])

fi

# front-ends
ROSE_SUPPORT_EDG
ROSE_SUPPORT_CLANG
ROSE_SUPPORT_FLANG

# DQ (1/4/2009) Added support for optional GNU language extensions in new EDG/ROSE interface.
# This value will be substituted into EDG/4.0/src/rose_lang_feat.h in the future (not used at present!)
AC_ARG_ENABLE(gnu-extensions, AS_HELP_STRING([--enable-gnu-extensions], [Enable internal support in ROSE for GNU language extensions]))
if test "x$enable_gnu_extensions" = "xyes"; then
  ROSE_SUPPORT_GNU_EXTENSIONS="TRUE"
else
  ROSE_SUPPORT_GNU_EXTENSIONS="FALSE"
fi
AC_SUBST(ROSE_SUPPORT_GNU_EXTENSIONS)

# DQ (1/4/2009) Added support for optional Microsoft language extensions in new EDG/ROSE interface.
# This value will be substituted into EDG/4.0/src/rose_lang_feat.h in the future (not used at present!)
AC_ARG_ENABLE(microsoft-extensions, AS_HELP_STRING([--enable-microsoft-extensions], [Enable internal support in ROSE for Microsoft language extensions]))
if test "x$enable_microsoft_extensions" = "xyes"; then
  ROSE_SUPPORT_MICROSOFT_EXTENSIONS="TRUE"
  AC_DEFINE([ROSE_USE_MICROSOFT_EXTENSIONS], [], [Controls use of Microsoft MSVC features])
else
  ROSE_SUPPORT_MICROSOFT_EXTENSIONS="FALSE"
fi
AC_SUBST(ROSE_SUPPORT_MICROSOFT_EXTENSIONS)
AM_CONDITIONAL(ROSE_USE_MICROSOFT_EXTENSIONS, [test "x$enable_microsoft_extensions" = xyes])

# TV (12/31/2018): Defining macro to detect the support of __float128 in EDG
#   EDG approximates __float128 conversions with long double routines
#   (APPROXIMATE_QUADMATH), so only compiler support for __float128 is
#   needed (no dependency on -lquadmath).
AC_LANG(C++)
AX_COMPILER_VENDOR

AC_COMPILE_IFELSE([
            AC_LANG_PROGRAM([[]], [[__float128 x = 0; (void)x;]])],
            [rose_use_edg_quad_float=yes],
            [rose_use_edg_quad_float=no])

if test "$ROSE_SUPPORT_MICROSOFT_EXTENSIONS" == "TRUE"; then
  rose_use_edg_quad_float=no
fi

if test "x$rose_use_edg_quad_float" == "xyes"; then
  AC_DEFINE([ROSE_USE_EDG_QUAD_FLOAT], [], [Enables support for __float80 and __float128 in EDG.])
fi
AC_SUBST(ROSE_USE_EDG_QUAD_FLOAT)
AM_CONDITIONAL(ROSE_USE_EDG_QUAD_FLOAT, [ test $rose_use_edg_quad_float == yes ])
unset ax_cv_cxx_compiler_vendor

# DQ (9/16/2012): Added support for debugging output of new EDG/ROSE connection.  More specifically
# if this is not enabled then it skips the use of output spew in the new EDG/ROSE connection code.
AC_ARG_ENABLE(debug_output_for_new_edg_interface,
    AS_HELP_STRING([--enable-debug_output_for_new_edg_interface], [Enable debugging output (spew) of new EDG/ROSE connection]))
AM_CONDITIONAL(ROSE_DEBUG_NEW_EDG_ROSE_CONNECTION, [test "x$enable_debug_output_for_new_edg_interface" = xyes])
if test "x$enable_debug_output_for_new_edg_interface" = "xyes"; then
  AC_MSG_WARN([using this mode causes large volumes of output spew (internal debugging only)!])
  AC_DEFINE([ROSE_DEBUG_NEW_EDG_ROSE_CONNECTION], [], [Controls large volumes of output spew useful for debugging new EDG/ROSE connection code])
fi

# Added support for Fortran front-end development using the flang (F18) compiler [Rasmussen 8/12/2019]
AC_ARG_ENABLE(experimental_flang_frontend,
    AS_HELP_STRING([--enable-experimental_flang_frontend], [Enable experimental fortran frontend development using flang]))
AM_CONDITIONAL(ROSE_EXPERIMENTAL_FLANG_ROSE_CONNECTION, [test "x$enable_experimental_flang_frontend" = xyes])
if test "x$enable_experimental_flang_frontend" = "xyes"; then
  AC_MSG_WARN([using this mode enables the experimental fortran flang front-end (internal development only)!])
  AC_DEFINE([ROSE_EXPERIMENTAL_FLANG_ROSE_CONNECTION], [], [Enables development of experimental fortran flang frontend])
fi

AC_ARG_ENABLE(experimental_csharp_frontend,
    AS_HELP_STRING([--enable-experimental_csharp_frontend], [Enable experimental csharp frontend development]))
AM_CONDITIONAL(ROSE_EXPERIMENTAL_CSHARP_ROSE_CONNECTION, [test "x$enable_experimental_csharp_frontend" = xyes])
if test "x$enable_experimental_csharp_frontend" = "xyes"; then
  AC_MSG_WARN([using this mode enables experimental csharp front-end (internal development only)!])
  AC_DEFINE([ROSE_EXPERIMENTAL_CSHARP_ROSE_CONNECTION], [], [Enables development of experimental csharp frontend])

  mono_home=`roslyn-config mono-home`
  roslyn_home=`roslyn-config csharp-home`

  AC_MSG_NOTICE([Mono home   = "$mono_home"])
  AC_MSG_NOTICE([Roslyn home = "$rosyln_home"])

  AC_DEFINE_UNQUOTED([ROSE_EXPERIMENTAL_CSHARP_ROSE_CONNECTION_MONO_HOME], ["$mono_home"], [Path to Mono Csharp compiler used in development of experimental csharp frontend])
  AC_DEFINE_UNQUOTED([ROSE_EXPERIMENTAL_CSHARP_ROSE_CONNECTION_ROSLYN_HOME], ["$roslyn_home"], [Path to Roslyn Csharp library used in development of experimental csharp frontend])

  AC_SUBST(mono_home)
  AC_SUBST(rosyln_home)
fi

# DQ (6/7/2013): Added support for debugging new csharp front-end development.
AC_ARG_ENABLE(debug_output_for_experimental_csharp_frontend,
    AS_HELP_STRING([--enable-debug_output_for_experimental_csharp_frontend], [Enable debugging output (spew) of new CSHARP/ROSE connection]))
AM_CONDITIONAL(ROSE_DEBUG_EXPERIMENTAL_CSHARP_ROSE_CONNECTION, [test "x$enable_debug_output_for_experimental_csharp_frontend" = xyes])
if test "x$enable_debug_output_for_experimental_csharp_frontend" = "xyes"; then
  AC_MSG_WARN([using this mode causes large volumes of output spew (internal debugging only)!])
  AC_DEFINE([ROSE_DEBUG_EXPERIMENTAL_CSHARP_ROSE_CONNECTION], [], [Controls large volumes of output spew useful for debugging new CSHARP/ROSE connection code])
fi

# DQ (8/23/2017): Added support for new Ada front-end development.
AC_ARG_ENABLE(experimental_ada_frontend,
    AS_HELP_STRING([--enable-experimental_ada_frontend], [Enable experimental ada frontend development]))
AM_CONDITIONAL(ROSE_EXPERIMENTAL_ADA_ROSE_CONNECTION, [test "x$enable_experimental_ada_frontend" = xyes])
if test "x$enable_experimental_ada_frontend" = "xyes"; then
  AC_MSG_WARN([using this mode enables experimental ada front-end (internal development only)!])
  AC_DEFINE([ROSE_EXPERIMENTAL_ADA_ROSE_CONNECTION], [], [Enables development of experimental ada frontend])
fi

AC_ARG_ENABLE(experimental_libadalang_frontend,
    AS_HELP_STRING([--enable-experimental_libadalang_frontend], [Enable experimental ada frontend development using libadalang]))
AM_CONDITIONAL(ROSE_EXPERIMENTAL_LIBADALANG_ROSE_CONNECTION, [test "x$enable_experimental_libadalang_frontend" = xyes])
if test "x$enable_experimental_libadalang_frontend" = "xyes"; then
  AC_MSG_WARN([using this mode enables experimental ada front-end (internal development only)!])
  AC_DEFINE([ROSE_EXPERIMENTAL_LIBADALANG_ROSE_CONNECTION], [], [Enables development of experimental ada frontend using libadalang])
fi

# DQ (6/7/2013): Added support for debugging new ada front-end development.
AC_ARG_ENABLE(debug_output_for_experimental_ada_frontend,
    AS_HELP_STRING([--enable-debug_output_for_experimental_ada_frontend], [Enable debugging output (spew) of new ADA/ROSE connection]))
AM_CONDITIONAL(ROSE_DEBUG_EXPERIMENTAL_ADA_ROSE_CONNECTION, [test "x$enable_debug_output_for_experimental_ada_frontend" = xyes])
if test "x$enable_debug_output_for_experimental_ada_frontend" = "xyes"; then
  AC_MSG_WARN([using this mode causes large volumes of output spew (internal debugging only)!])
  AC_DEFINE([ROSE_DEBUG_EXPERIMENTAL_ADA_ROSE_CONNECTION], [], [Controls large volumes of output spew useful for debugging new ADA/ROSE connection code])
fi

# DQ (6/8/2017): Adding ADA support using GNAT ASIS tool chain.
ROSE_SUPPORT_GNAT

# Setup Automake conditional.
AM_CONDITIONAL(ROSE_USE_GNAT,test ! "$with_gnat" = no)

if test "x$enable_experimental_ada_frontend" = "xyes"; then
  gnat_home=$with_gnat
  AC_MSG_NOTICE([GNAT home = "$gnat_home"])
  AC_DEFINE_UNQUOTED([ROSE_EXPERIMENTAL_ADA_ROSE_CONNECTION_GNAT_HOME], ["$with_gnat"], [Path to GNAT Ada compiler used in development of experimental ada frontend])
  AC_SUBST(gnat_home)
fi

if test "x$enable_experimental_libadalang_frontend" = "xyes"; then
  gnat_home=$with_gnat
  AC_MSG_NOTICE([GNAT home = "$gnat_home"])
  AC_DEFINE_UNQUOTED([ROSE_EXPERIMENTAL_LIBADALANG_ROSE_CONNECTION_GNAT_HOME], ["$with_gnat"], [Path to GNAT Ada compiler used in development of experimental ada frontend])
  AC_SUBST(gnat_home)
fi

# Jovial front-end development.
AC_ARG_ENABLE(experimental_jovial_frontend,
    AS_HELP_STRING([--enable-experimental_jovial_frontend], [Enable experimental jovial frontend development]))
AM_CONDITIONAL(ROSE_EXPERIMENTAL_JOVIAL_ROSE_CONNECTION, [test "x$enable_experimental_jovial_frontend" = xyes])
if test "x$enable_experimental_jovial_frontend" = "xyes"; then
  AC_MSG_WARN([using this mode enables experimental jovial front-end (internal development only)!])
  AC_DEFINE([ROSE_EXPERIMENTAL_JOVIAL_ROSE_CONNECTION], [], [Enables development of experimental jovial frontend])
fi

# Debugging Jovial front-end development.
AC_ARG_ENABLE(debug_output_for_experimental_jovial_frontend,
    AS_HELP_STRING([--enable-debug_output_for_experimental_jovial_frontend], [Enable debugging output (spew) of new JOVIAL/ROSE connection]))
AM_CONDITIONAL(ROSE_DEBUG_EXPERIMENTAL_JOVIAL_ROSE_CONNECTION, [test "x$enable_debug_output_for_experimental_jovial_frontend" = xyes])
if test "x$enable_debug_output_for_experimental_jovial_frontend" = "xyes"; then
  AC_MSG_WARN([using this mode causes large volumes of output spew (internal debugging only)!])
  AC_DEFINE([ROSE_DEBUG_EXPERIMENTAL_JOVIAL_ROSE_CONNECTION], [], [Controls large volumes of output spew useful for debugging new JOVIAL/ROSE connection code])
fi

# Octave/Matlab front-end development.
AC_ARG_ENABLE([experimental_matlab_frontend],
    AS_HELP_STRING([--enable-experimental_matlab_frontend], [Enable experimental Octave/Matlab frontend development (default=no)]))
AM_CONDITIONAL(ROSE_EXPERIMENTAL_MATLAB_ROSE_CONNECTION, [test "x$enable_experimental_matlab_frontend" = xyes])
if test "x$enable_experimental_matlab_frontend" = "xyes"; then
  AC_MSG_WARN([using this mode enables experimental Octave/Matlab front-end (internal development only)!])
  AC_DEFINE([ROSE_EXPERIMENTAL_MATLAB_ROSE_CONNECTION], [], [Enables development of experimental Octave/Matlab frontend])
fi

# Debugging Octave/Matlab front-end development.
AC_ARG_ENABLE(debug_output_for_experimental_matlab_frontend,
    AS_HELP_STRING([--enable-debug_output_for_experimental_matlab_frontend], [Enable debugging output (spew) of new Octave/Matlab ROSE connection]))
AM_CONDITIONAL(ROSE_DEBUG_EXPERIMENTAL_MATLAB_ROSE_CONNECTION, [test "x$enable_debug_output_for_experimental_matlab_frontend" = xyes])
if test "x$enable_debug_output_for_experimental_matlab_frontend" = "xyes"; then
  AC_MSG_WARN([using this mode causes large volumes of output spew (internal debugging only)!])
  AC_DEFINE([ROSE_DEBUG_EXPERIMENTAL_MATLAB_ROSE_CONNECTION], [], [Controls large volumes of output spew useful for debugging new Octave/Matlab ROSE connection code])
fi

AC_ARG_ENABLE(use_new_graph_node_backward_compatability,
    AS_HELP_STRING([--enable-use_new_graph_node_backward_compatability], [Enable new (experimental) graph IR nodes backward compatability API]))
AM_CONDITIONAL(ROSE_USING_GRAPH_IR_NODES_FOR_BACKWARD_COMPATABILITY, [test "x$enable_use_new_graph_node_backward_compatability" = xyes])
if test "x$enable_use_new_graph_node_backward_compatability" = "xyes"; then
  AC_MSG_WARN([using the new graph IR nodes in ROSE (experimental)!])
  AC_DEFINE([ROSE_USING_GRAPH_IR_NODES_FOR_BACKWARD_COMPATABILITY], [], [Whether to use the new graph IR nodes compatability option with older API])
fi

# Set up for use of bison to build dot2gml tool in directory
# src/roseIndependentSupport/dot2gml.  This is made optional
# because it seems that many don't have the correct version of bison
# to support the compilation of this tool.  This is it is a configure
# option to build it (or have the makefile system have it be built).
AC_ARG_ENABLE(dot2gml_translator,
[  --enable-dot2gml_translator   Configure option to have DOT to GML translator built (bison version specific tool).],
[ AC_MSG_NOTICE([setting up optional DOT-to-GML translator in directory: src/roseIndependentSupport/dot2gml])
])
AM_CONDITIONAL(DOT_TO_GML_TRANSLATOR,test "$enable_dot2gml_translator" = yes)

AC_CANONICAL_HOST

AC_LANG(C++)

# Get frontend compiler vendor
AX_COMPILER_VENDOR
FRONTEND_CXX_COMPILER_VENDOR="$ax_cv_cxx_compiler_vendor"

# DQ (10/27/2020): Fixed to avoid output spew.
AC_MSG_NOTICE([_AC_LANG_ABBREV              = "$_AC_LANG_ABBREV"])
AC_MSG_NOTICE([ax_cv_c_compiler_vendor      = "$ax_cv_c_compiler_vendor"])
AC_MSG_NOTICE([ax_cv_cxx_compiler_vendor    = $ax_cv_cxx_compiler_vendor"])

AC_MSG_NOTICE([FRONTEND_CXX_COMPILER_VENDOR = "$FRONTEND_CXX_COMPILER_VENDOR"])

unset ax_cv_cxx_compiler_vendor

# DQ (11/14/2011): This is defined here since it must be seen before any processing of the rose_config.h file.
if test "x$enable_internalFrontendDevelopment" = "xyes"; then
  AC_MSG_NOTICE([adding -D to command line to support faster development of language frontend work])
  CFLAGS+=" -DROSE_USE_INTERNAL_FRONTEND_DEVELOPMENT"
  CXXFLAGS+=" -DROSE_USE_INTERNAL_FRONTEND_DEVELOPMENT"
fi

AC_MSG_NOTICE([CFLAGS   = "$CFLAGS"])
AC_MSG_NOTICE([CXXFLAGS = "$CXXFLAGS"])
AC_MSG_NOTICE([CPPFLAGS = "$CPPFLAGS"])

# *****************************************************************
#    Option to define a uniform debug level for ROSE development
# *****************************************************************

# DQ (10/17/2010): This defines an advanced level of uniform support for debugging and compiler warnings in ROSE.
AC_MSG_CHECKING([for enabled advanced warning support])

# Default is that advanced warnings is off, but this can be changed later so that advanced warnings would have to be explicitly turned off.
AC_ARG_ENABLE(advanced_warnings, AS_HELP_STRING([--enable-advanced-warnings], [Support for an advanced uniform warning level for ROSE development]),[enableval=yes],[enableval=no])
AM_CONDITIONAL(ROSE_USE_UNIFORM_ADVANCED_WARNINGS_SUPPORT, [test "x$enable_advanced_warnings" = xyes])
if test "x$enable_advanced_warnings" = "xyes"; then
  AC_MSG_WARN([using an advanced uniform warning level for ROSE development])
  AC_DEFINE([ROSE_USE_UNIFORM_ADVANCED_WARNINGS_SUPPORT], [], [Support for an advanced uniform warning level for ROSE development])

# Suggested C++ specific flags (used to be run before Hudson, but fail currently).
  CXX_ADVANCED_WARNINGS+=" -D_GLIBCXX_CONCEPT_CHECKS -D_GLIBCXX_DEBUG"

# Additional flag (suggested by George).
  CXX_ADVANCED_WARNINGS+=" -D_GLIBCXX_DEBUG_PEDANTIC"

# Incrementally add the advanced options
  if test "$CXX_ADVANCED_WARNINGS"; then CXXFLAGS="$CXXFLAGS $CXX_ADVANCED_WARNINGS"; fi
fi

AC_SUBST(ROSE_USE_UNIFORM_ADVANCED_WARNINGS_SUPPORT)

AC_MSG_NOTICE([after processing --enable-advanced-warnings: CXX_ADVANCED_WARNINGS = "${CXX_ADVANCED_WARNINGS}"])
AC_MSG_NOTICE([after processing --enable-advanced-warnings: CXX_WARNINGS = "${CXX_WARNINGS}"])
AC_MSG_NOTICE([after processing --enable-advanced-warnings: C_WARNINGS   = "${C_WARNINGS}"])

AC_MSG_NOTICE([CFLAGS   = "$CFLAGS"])
AC_MSG_NOTICE([CXXFLAGS = "$CXXFLAGS"])
AC_MSG_NOTICE([CPPFLAGS = "$CPPFLAGS"])

CXX_TEMPLATE_REPOSITORY_PATH='$(top_builddir)/src'

AC_ARG_ENABLE(assertion-behavior,
    AS_HELP_STRING([--enable-assertion-behavior[=MODE]],
        [Specifies the default behavior for failing ROSE assertions. This behavior can be changed at runtime either
         via ROSE command-line switches or the rose API. Most developers (the ROSE team and users that
         are developing transformations) will probably want "abort" since this gives the most useful post-mortem
         information. On the other hand, end users usually don't need or expect post-mortem capabilities and sometimes
         even perceive them as low code quality, in which case "exit" with non-zero status is the best behavior. Finally,
         the "throw" behavior can be used as a compromise for tool developers to more gracefully recover from a ROSE
         error that would otherwise be fatal.  When --enable-assertion-behavior is not specified then "exit" is used.
         Some assertions can be disabled altogether (e.g., when an optimized library is desired) by defining NDEBUG.
         Caveats: this switch affects the behavior of the ROSE_ASSERT macro and the Sawyer ASSERT_* macros, but not
         plain old "assert"; the NDEBUG define applies to Sawyer ASSERT_* macros and plain old "assert" but not
         ROSE_ASSERT.]))

case "$enable_assertion_behavior" in
    abort)    assertion_behavior=ROSE_ASSERTION_ABORT ;;
    exit|"")  assertion_behavior=ROSE_ASSERTION_EXIT ;;
    throw)    assertion_behavior=ROSE_ASSERTION_THROW ;;
    *)
        AC_MSG_ERROR([--enable-assertion-behavior should be "abort", "exit", or "throw"])
        ;;
esac

AC_DEFINE_UNQUOTED([ROSE_ASSERTION_BEHAVIOR], [$assertion_behavior], [Determines how failed assertions should behave.])

# ********************************************************************************
#    Option support for the Address Sanitizer and other related Sanitizer tools.
# ********************************************************************************

ROSE_SUPPORT_SANITIZER

# ********************************************************************************
#    Option support for the Linux Coverage Test tools.
# ********************************************************************************

ROSE_SUPPORT_LCOV

# *****************************************************************
# ROSE_HOME should be relative to top_srcdir or top_builddir.
ROSE_HOME=.
AC_SUBST(ROSE_HOME)

AC_LANG(C++)

# Python check must occur before boost check since -lboost_python depends on python
ROSE_SUPPORT_PYTHON

ROSE_SUPPORT_BOOST

ROSE_SUPPORT_GRAPHVIZ

AX_LIB_SQLITE3
AX_LIB_MYSQL
AM_CONDITIONAL(ROSE_USE_MYSQL,test "$found_mysql" = yes)

# Test this macro here at the start to avoid long processing times (before it fails)
CHOOSE_BACKEND_COMPILER

# Get frontend compiler vendor
AX_COMPILER_VENDOR
FRONTEND_CXX_COMPILER_VENDOR="$ax_cv_cxx_compiler_vendor"
unset ax_cv_cxx_compiler_vendor

# Get backend compiler vendor
  saved_compiler_name=$CXX
  CXX=$BACKEND_CXX_COMPILER
  AC_MSG_NOTICE([after resetting CXX to be the backend compiler: CXX = "$CXX"])

  AX_COMPILER_VENDOR

  AC_MSG_NOTICE([using back-end C++ compiler = "$BACKEND_CXX_COMPILER" compiler vendor name = $ax_cv_cxx_compiler_vendor for processing of unparsed source files from ROSE preprocessors])
  BACKEND_CXX_COMPILER_VENDOR="$ax_cv_cxx_compiler_vendor"

  CXX=$saved_compiler_name
  AC_MSG_NOTICE([after resetting CXX to be the saved name of the original compiler: CXX = "$CXX"])

AC_MSG_NOTICE([FRONTEND_CXX_COMPILER_VENDOR = "$FRONTEND_CXX_COMPILER_VENDOR"])

AC_MSG_CHECKING([whether your compiler is a GNU compiler and the version that is supported by ROSE (4.0.x - 6.3.x)])
AC_ARG_ENABLE([gcc-version-check],AS_HELP_STRING([--disable-gcc-version-check],[Disable GCC version 4.0.x - 6.3.x verification check]),,[enableval=yes])
if test "x$FRONTEND_CXX_COMPILER_VENDOR" = "xgnu" ; then
if test "x$enableval" = "xyes" ; then
      AC_LANG_PUSH([C])
      # http://www.gnu.org/s/hello/manual/autoconf/Running-the-Compiler.html
      AC_COMPILE_IFELSE([
        AC_LANG_SOURCE([[
          #if (__GNUC__ >= 4 && __GNUC_MINOR__ <= 9)
            int rose_supported_gcc;
          #else
            not gcc, or gcc version is not supported by rose
          #endif
        ]])
       ],
       [AC_MSG_RESULT([done])],
       gcc_version=`gcc -dumpversion`
       [AC_MSG_FAILURE([your GCC $gcc_version version is currently NOT supported by ROSE; GCC 4.0.x to 4.8.x is supported now])])
      AC_LANG_POP([C])
else
    AC_MSG_RESULT([skipping])
fi
else
    AC_MSG_RESULT([not a GNU compiler])
fi

# DQ (2/7/17): This is a problem reported by Robb (sometimes gcc is not installed).
# This is used in EDG (host_envir.h)  Test by building a bad version of gcc
# use shell script called gcc with "exit 1" inside.
if test "x$FRONTEND_CXX_COMPILER_VENDOR" = "xgnu" ; then
   GCC_VERSION=`gcc -dumpversion | cut -d\. -f1`
   GCC_MINOR_VERSION=`gcc -dumpversion | cut -d\. -f2`

   AC_MSG_NOTICE([initial compiler version test: GCC_VERSION = "$GCC_VERSION"])
   AC_MSG_NOTICE([initial compiler version test: GCC_MINOR_VERSION = "$GCC_MINOR_VERSION"])

   AC_SUBST(GCC_VERSION)
   AC_SUBST(GCC_MINOR_VERSION)
else
 # DQ (2/8/2017): Default configuration of EDG will behave like GNU 4.8.x (unclear if this is idea).
   GCC_VERSION=4
   GCC_MINOR_VERSION=8
fi

# DQ (2/7/2017): These macros test for C++11 and C++14 features and
# the default behavior of the CXX compiler.  Unfortunately the also
# modify the CXX value so we have to save it and reset it after the
# macros are called.  We modified the macros as well to save the
# default behavior of the CXX compiler so that we can detect C++11
# mode within the frontend compiler used to compile ROSE.  Thi is used
# mostly so far to just disable some test that are causing GNU g++
# version 4.8.x internal errors (because the C++11 support is new).

AC_MSG_NOTICE([before checking C++11 support: CXX = $CXX CXXCPP = "$CXXCPP"])

AC_MSG_NOTICE([calling AX CXX COMPILE STDCXX 11 macro])
save_CXX="$CXX"
AX_CXX_COMPILE_STDCXX_11(, optional)

AC_MSG_NOTICE([after checking C++11 support: CXX = "$CXX", CXXCPP = "$CXXCPP"])

AC_MSG_NOTICE([rose_frontend_compiler_default_is_cxx11_success = "$rose_frontend_compiler_default_is_cxx11_success"])
AC_MSG_NOTICE([gcc_version_4_8                                 = "$gcc_version_4_8"])

AM_CONDITIONAL(ROSE_USING_GCC_VERSION_4_8_CXX11, [test "x$gcc_version_4_8" = "xyes" && test "x$rose_frontend_compiler_default_is_cxx11_success" = "xyes"])

AC_MSG_NOTICE([calling AX CXX COMPILE STDCXX 14 macro])
AX_CXX_COMPILE_STDCXX_14(, optional)

AC_MSG_NOTICE([after checking C++14 support: CXX = "$CXX", CXXCPP = "$CXXCPP"])
CXX="$save_CXX"

AC_MSG_NOTICE([after restoring the saved value of CXX: CXX = "$CXX", CXXCPP = "$CXXCPP"])

ROSE_SUPPORT_FATAL_WARNINGS

# End macro ROSE_SUPPORT_ROSE_PART_1.
]
)

AC_DEFUN([ROSE_SUPPORT_ROSE_BUILD_INCLUDE_FILES],
[
# Begin macro ROSE_SUPPORT_ROSE_BUILD_INCLUDE_FILES.

AC_MSG_NOTICE([in ROSE SUPPORT ROSE BUILD INCLUDE FILES: Using back-end C++ compiler = "$BACKEND_CXX_COMPILER" compiler vendor name = "$ax_cv_cxx_compiler_vendor" for processing of unparsed source files from ROSE preprocessors])

# DQ (7/26/2020): Spelling it correctly so that we can force the directory of header files to be rebuilt.

# Note that this directory name is not spelled correctly, is this a typo?
rm -rf ./include-stagin

if test x$enable_clang_frontend = xyes; then
  INSTALL_CLANG_SPECIFIC_HEADERS
else

  # DQ (11/1/2011): I think that we need these for more complex header file
  # requirements than we have seen in testing C code to date.  Previously
  # in testing C codes with the EDG 4.x we didn't need as many header files.
    GENERATE_BACKEND_C_COMPILER_SPECIFIC_HEADERS
    GENERATE_BACKEND_CXX_COMPILER_SPECIFIC_HEADERS
fi

# End macro ROSE_SUPPORT_ROSE_BUILD_INCLUDE_FILES.
]
)

#-----------------------------------------------------------------------------

AC_DEFUN([ROSE_SUPPORT_ROSE_PART_2],
[
# Begin macro ROSE_SUPPORT_ROSE_PART_2.

# DQ (9/26/2015): Since the config/ltdl.m4 file in regenerated, we can't edit it easily.
# So make this a requirement so that it will not be expanded there.
m4_require([_LT_SYS_DYNAMIC_LINKER])

AC_PROG_CXX

AC_MSG_NOTICE([in configure.in ... CXX = "$CXX"])

# DQ (9/17/2006): These should be the same for both C and C++ (else we will need separate macros)
# Setup the -D<xxx> defines required to allow EDG to take the same path through the compiler
# specific and system specific header files as for the backend compiler.  These depend
# upon the selection of the back-end compiler.
GET_COMPILER_SPECIFIC_DEFINES
ROSE_CONFIG_TOKEN="$ROSE_CONFIG_TOKEN $FRONTEND_CXX_COMPILER_VENDOR-$FRONTEND_CXX_VERSION_MAJOR.$FRONTEND_CXX_VERSION_MINOR"

# DQ (9/20/20): Moving the setup of compiler flags to after the macros that define the compiler versions are computed.
# Setup default options for C and C++ compilers compiling ROSE source code.
ROSE_FLAG_C_OPTIONS
ROSE_FLAG_CXX_OPTIONS
ROSE_FLAG_OPTIONS

ROSE_CONFIGURE_SECTION([Checking backend C/C++ compiler specific references])
SETUP_BACKEND_C_COMPILER_SPECIFIC_REFERENCES
SETUP_BACKEND_CXX_COMPILER_SPECIFIC_REFERENCES

ROSE_SUPPORT_LONG_MAKE_CHECK_RULE

# Make the use of longer test optional where it is used in some ROSE/tests directories
AM_CONDITIONAL(ROSE_USE_LONG_MAKE_CHECK_RULE,test "$with_ROSE_LONG_MAKE_CHECK_RULE" = yes)

# JJW -- use standard version in /usr/share/aclocal, and configure XML only
# once for roseHPCT and BinaryContextLookup
with_xml="no"
AM_PATH_XML2(2.0.0, [with_xml="yes"])

# Make the use of libxml2 explicitly controlled.
AM_CONDITIONAL(ROSE_USE_XML,test "$with_xml" != no)

# Check for objdump for BinaryContextLookup since it doesn't normally exist on
# Mac
AC_CHECK_TOOL(ROSE_OBJDUMP_PATH, [objdump], [no])
AM_CONDITIONAL(ROSE_USE_OBJDUMP, [test "$ROSE_OBJDUMP_PATH" != "no"])
AM_CONDITIONAL(ROSE_USE_BINARYCONTEXTLOOKUP, [test "$with_xml" != "no" -a "$ROSE_OBJDUMP_PATH" != "no"])

AC_C_BIGENDIAN
AC_CHECK_HEADERS([byteswap.h machine/endian.h])

ROSE_SUPPORT_VALGRIND

AC_ARG_WITH(wave-default, [  --with-wave-default     Use Wave as the default preprocessor],
            [AC_DEFINE([ROSE_WAVE_DEFAULT], true, [Use Wave as default in ROSE])],
            [AC_DEFINE([ROSE_WAVE_DEFAULT], false, [Simple preprocessor as default in ROSE])]
            )

AC_ARG_WITH(alloc-memset, [  --with-alloc-memset     Memory pool protection (memory is set on memory pool operation): 0 -> none, 1 -> command-line (NIY), 2 -> zeroed new/delete, 3 -> aggressive (diff. value for each mempool operation)],
            [AC_DEFINE_UNQUOTED([ROSE_ALLOC_MEMSET], $with_alloc_memset, [With memset on mempool operations])],
            [AC_DEFINE([ROSE_ALLOC_MEMSET], 0, [Without memset on mempool operations])]
            )

AC_ARG_WITH(pedantic-alloc, [  --with-pedantic-alloc   Enables pedantic assertions in Memory Pool: 0 -> none, 1 -> enabled ],
            [AC_DEFINE_UNQUOTED([ROSE_PEDANTIC_ALLOC], $with_pedantic_alloc, [With pedantic allocation check])],
            [AC_DEFINE([ROSE_PEDANTIC_ALLOC], 0, [Without pedantic allocation check])]
            )

AC_ARG_WITH(alloc-trace, [  --with-alloc-trace     Memory pool allocation tracing (tiny reproducers only): 0 -> none, 1 -> command-line (NIY), 2 -> enabled],
            [AC_DEFINE_UNQUOTED([ROSE_ALLOC_TRACE], $with_alloc_trace, [With tracing of memory pool operation])],
            [AC_DEFINE([ROSE_ALLOC_TRACE], 0, [Without tracing of memory pool operation])]
            )

# Figure out what version of lex we have available
# flex works better than lex (this gives a preference to flex (flex is gnu))
AM_PROG_LEX([noyywrap])
AC_SUBST(LEX)
AC_PROG_YACC
AC_SUBST(YACC)

AC_PROG_MAKE_SET

AC_MSG_NOTICE([testing the value of CC: (CC = "$CC")])
AC_MSG_NOTICE([testing the value of CPPFLAGS: (CPPFLAGS = "$CPPFLAGS")])

ROSE_SUPPORT_MAPLE

# Setup Automake conditional in Projects/programModeling/Makefile.am
AM_CONDITIONAL(ROSE_USE_MAPLE,test ! "$with_maple" = no)

# Setup Automake conditional.
AM_CONDITIONAL(ROSE_USE_VXWORKS,test ! "$with_vxworks" = no)

#Call supporting macro for IDA PRO
ROSE_SUPPORT_IDA

# Setup Automake conditional in projects/AstEquivalence/Makefile.am
AM_CONDITIONAL(ROSE_USE_IDA,test ! "$with_ida" = no)

# Call supporting macro for libffi (Foreign Function Interface library)
# This library is used by Peter's work on the Interpreter in ROSE.
ROSE_SUPPORT_LIBFFI

# Setup Automake conditional in projects/interpreter/Makefile.am
AM_CONDITIONAL(ROSE_USE_LIBFFI,test ! "$with_libffi" = no)

# Aterm library is used by the Jovial frontend
ROSE_SUPPORT_ATERM

# Stratego/XT library used by the Jovial frontend
ROSE_SUPPORT_STRATEGO

if test "x$enable_experimental_fortran_frontend" = "xyes"; then
   if test "x$ATERM_LIBRARY_PATH" = "x"; then
      AC_MSG_ERROR([support for experimental_fortran_frontend requires Aterm library support, --with-aterm=PATH must be specified])
   fi
   if test "x$STRATEGO_LIBRARY_PATH" = "x"; then
      AC_MSG_ERROR([support for experimental_fortran_frontend requires Stratego library support, --with-stratego=PATH must be specified])
   fi
fi

# ATerm and Stratego/XT tools binary installation required for Jovial support.
if test "x$enable_experimental_jovial_frontend" = "xyes"; then
   if test "x$ATERM_LIBRARY_PATH" = "x"; then
      AC_MSG_ERROR([support for experimental_jovial_frontend requires Aterm library support, --with-aterm=PATH must be specified!])
   fi
   if test "x$STRATEGO_LIBRARY_PATH" = "x"; then
      AC_MSG_ERROR([support for experimental_jovial_frontend requires Stratego library support, --with-stratego=PATH must be specified!])
   fi
fi

# Octave/Matlab parser installation required for Matlab support.
if test "x$enable_experimental_matlab_frontend" = "xyes"; then
   if test "x$OCTAVE_PARSER_INSTALL_TARFILE" = "x"; then
      AC_MSG_ERROR([support for experimental_matlab_frontend requires the modified GNU Octave parser, --with-octave-parser=PATH must be specified!])
   fi
fi

ROSE_SUPPORT_MINT

ROSE_SUPPORT_VECTORIZATION

# Pei-Hung (12/17/2014): Adding support for POCC.
ROSE_SUPPORT_POCC

ROSE_SUPPORT_PHP

AM_CONDITIONAL(ROSE_USE_PHP,test ! "$with_php" = no)

#ASR
ROSE_SUPPORT_LLVM

AM_CONDITIONAL(ROSE_USE_LLVM,test ! "$with_llvm" = no)

# Call supporting macro for Windows Source Code Analysis
ROSE_SUPPORT_WINDOWS_ANALYSIS

# Setup Automake conditional in Projects/programModeling/Makefile.am
AM_CONDITIONAL(ROSE_USE_WINDOWS_ANALYSIS_SUPPORT,test ! "$with_wine" = no)

# Control use of debugging support to convert most unions in EDG to structs.
ROSE_SUPPORT_EDG_DEBUGGING

# Call supporting macro for Omni OpenMP
#
ROSE_SUPPORT_OMNI_OPENMP

# call supporting macro for GCC 4.4.x gomp OpenMP runtime library
# AM_CONDITIONAL is already included into the macro
ROSE_WITH_GOMP_OPENMP_LIBRARY

# Call supporting macro for GCC OpenMP
ROSE_SUPPORT_GCC_OMP

# Configuration commandline support for OpenMP in ROSE
AM_CONDITIONAL(ROSE_USE_GCC_OMP,test ! "$with_parallel_ast_traversal_omp" = no)

# JJW and TP (3-17-2008) -- added MPI support
AC_ARG_WITH(parallel_ast_traversal_mpi,
[  --with-parallel_ast_traversal_mpi     Enable AST traversal in parallel using MPI.],
[ AC_MSG_NOTICE([setting up optional MPI-based tools])
])
AM_CONDITIONAL(ROSE_MPI,test "$with_parallel_ast_traversal_mpi" = yes)
AC_CHECK_TOOLS(MPICXX, [mpiCC mpic++ mpicxx])

# TPS (2-11-2009) -- added PCH Support
AC_ARG_WITH(pch,
[  --with-pch                    Configure option to have pre-compiled header support enabled.],
[ AC_MSG_NOTICE([enabling precompiled header])
])
AM_CONDITIONAL(ROSE_PCH,test "$with_pch" = yes)
if test "x$with_pch" = xyes; then
  AC_MSG_NOTICE(["PCH enabled: CPPFLAGS = "$CPPFLAGS"])
  if test "x$with_parallel_ast_traversal_mpi" = xyes; then
    AC_MSG_ERROR([PCH support cannot be configured together with MPI support])
  fi
  if test "x$with_parallel_ast_traversal_omp" = xyes; then
    AC_MSG_ERROR([PCH Support cannot be configured together with GCC_OMP support])
  fi
else
  AC_MSG_NOTICE("PCH disabled: no Support for PCH")
fi

# TP SUPPORT FOR OPENGL
AC_ARG_ENABLE([rose-openGL],
  [  --enable-rose-openGL  enable openGL],
  [  rose_openGL=${enableval}
AC_PATH_X dnl We need to do this by hand for some reason

AC_FIND_OPENGL

AC_MSG_NOTICE([have_GL = "$have_GL" and have_glut = "$have_glut" and rose_openGL = "$rose_openGL"])
if test ! "x$rose_openGL" = xno; then
   AC_MSG_NOTICE([checking OpenGL dependencies..."])
  if test "x$have_GL" = xyes; then
    AC_MSG_NOTICE([OpenGL enabled: found OpenGL])
  else
    AC_MSG_ERROR([OpenGL not found])
  fi
 if test "x$have_glut" = xyes; then
    AC_MSG_NOTICE([OpenGL enabled: found GLUT])
 else
   AC_MSG_NOTICE([OpenGL GLUT not found; please use --with-glut])
 fi
fi
], [ rose_openGL=no
  AC_MSG_NOTICE([OpenGL disabled])
])
AM_CONDITIONAL(ROSE_USE_OPENGL, test ! "x$have_GL" = xno -a ! "x$rose_openGL" = xno)

AM_CONDITIONAL(USE_ROSE_GLUT_SUPPORT, false)

AC_ARG_WITH(glut,
[  --with-glut=PATH     Configure option to have GLUT enabled.],
,
if test ! "$with_glut" ; then
   with_glut=no
fi
)

AC_MSG_NOTICE([in ROSE SUPPORT MACRO: with_glut = "$with_glut"])

if test "$with_glut" = no; then
   # If dwarf is not specified, then don't use it.
   AC_MSG_NOTICE([skipping use of GLUT support])
else
   AM_CONDITIONAL(USE_ROSE_GLUT_SUPPORT, true)
   glut_path=$with_glut
   AC_MSG_NOTICE([setup GLUT support in ROSE. path = "$glut_path"])
   AC_DEFINE([USE_ROSE_GLUT_SUPPORT],1,[Controls use of ROSE support for GLUT library.])
fi


AC_SUBST(glut_path)



AC_CHECK_PROGS(PERL, [perl])

# DQ (9/4/2009): Added checking for indent command (common in Linux, but not on some platforms).
# This command is used in the tests/nonsmoke/functional/roseTests/astInterfaceTests/Makefile.am file.
AC_CHECK_PROGS(INDENT, [indent])
AM_CONDITIONAL(ROSE_USE_INDENT, [test "x$INDENT" = "xindent"])
AC_MSG_NOTICE([INDENT = "$INDENT"])

AC_CHECK_PROGS(TCLSH, [tclsh])
AM_CONDITIONAL(ROSE_USE_TCLSH, [test "x$TCLSH" = "xtclsh"])
AC_MSG_NOTICE([TCLSH = "$TCLSH"])

ROSE_SUPPORT_OFP

# DQ (3/6/2013): The major version number must match or the ac_pkg_swig.m4 will report
# we are using the wrong version of swig (likely we need a newer version of this m4 script).
# AC_PROG_SWIG(1.3.31)
AC_PROG_SWIG(2.0.0)
SWIG_ENABLE_CXX

AC_ARG_WITH([javaport],
   [  --with-javaport ... Enable generation of Java bindings for ROSE using Swig],
   [with_javaport=$withval],
   [with_javaport=no])
AM_CONDITIONAL(ENABLE_JAVAPORT,test "$with_javaport" = yes)

if test "x$with_javaport" = "xyes"; then
  if test "x$USE_JAVA" = "x0"; then
    AC_MSG_ERROR([trying to enable --with-javaport without --with-java also being set])
  fi
  if /bin/sh -c "$SWIG -version" >& /dev/null; then
    :
  else
    AC_MSG_ERROR([trying to enable --with-javaport without SWIG installed])
  fi
  AC_MSG_WARN([enabling Java binding support -- SWIG produces invalid C++ code, so -fno-strict-aliasing is being added to CXXFLAGS to work around this issue.  If you are not using GCC as a compiler, this flag will need to be changed.])
  CXXFLAGS="$CXXFLAGS -fno-strict-aliasing"
fi

ROSE_SUPPORT_HASKELL

ROSE_SUPPORT_CUDA

ROSE_SUPPORT_SWIPL

ROSE_SUPPORT_Z3

ROSE_SUPPORT_BDDBDDB

# Setup Automake conditional in Projects/DatalogAnalysis/Makefile.am
AM_CONDITIONAL(ROSE_USE_BDDBDDB,test ! "$with_bddbddb" = no)

ROSE_SUPPORT_VISUALIZATION

# Setup Automake conditional in src/roseIndependentSupport/visualization/Makefile.am
AM_CONDITIONAL(ROSE_USE_VISUALIZATION,(test ! "$with_FLTK_include" = no) || (test ! "$with_FLTK_libs" = no) || (test ! "$with_GraphViz_include" = no) || (test ! "$with_GraphViz_libs" = no))

# *********************************************************************
# Option to control internal support of PPL (Parma Polyhedron Library)
# *********************************************************************

AC_ARG_WITH(
        [ppl],
        AS_HELP_STRING([--with-ppl@<:@=DIR@:>@], [use Parma Polyhedral Library (PPL)]),
        [
        if test "$withval" = "no"; then
      AC_MSG_FAILURE([--with-ppl=PATH must be specified to use option --with-ppl (a valid Parma Polyhedral Library (PPL) intallation)])
        elif test "$withval" = "yes"; then
      AC_MSG_FAILURE([--with-ppl=PATH must be specified to use option --with-ppl (a valid Parma Polyhedral Library (PPL) intallation)])
        else
            has_ppl_path="yes"
            ppl_path="$withval"
        fi
        ],
        [has_ppl_path="no"]
)

AC_ARG_ENABLE(
        ppl,
        AS_HELP_STRING(
                [--enable-ppl],
                [Support for Parma Polyhedral Library (PPL)]
        )
)
AM_CONDITIONAL(
        ROSE_USE_PPL,
        [test "x$enable_ppl" = "xyes"])
if test "x$enable_ppl" = "xyes"; then
        if test "x$has_ppl_path" = "xyes"; then
                PPL_PATH="$ppl_path"
                AC_DEFINE([ROSE_USE_PPL], [], [Whether to use Parma Polyhedral Library (PPL) support or not within ROSE])
        fi
fi
AC_SUBST(ROSE_USE_PPL)
AC_SUBST(PPL_PATH)

# *********************************************************************************
# Option to control internal support of Cloog (Code generator for Polyhedral Model)
# *********************************************************************************

AC_ARG_WITH(
        [cloog],
        AS_HELP_STRING([--with-cloog@<:@=DIR@:>@], [use Cloog]),
        [
        if test "$withval" = "no"; then
      AC_MSG_FAILURE([--with-cloog=PATH must be specified to use option --with-cloog (a valid Cloog intallation)])
        elif test "$withval" = "yes"; then
            AC_MSG_FAILURE([--with-cloog=PATH must be specified to use option --with-cloog (a valid Cloog intallation)])
        else
            has_cloog_path="yes"
            cloog_path="$withval"
        fi
        ],
        [has_cloog_path="no"]
)

AC_ARG_ENABLE(
        cloog,
        AS_HELP_STRING(
                [--enable-cloog],
                [Support for Cloog]
        )
)
AM_CONDITIONAL(
        ROSE_USE_CLOOG,
        [test "x$enable_cloog" = "xyes"])
if test "x$enable_cloog" = "xyes"; then
        if test "x$has_cloog_path" = "xyes"; then
                CLOOG_PATH="$cloog_path"
                AC_DEFINE([ROSE_USE_CLOOG], [], [Whether to use Cloog support or not within ROSE])
        fi
fi
AC_SUBST(ROSE_USE_CLOOG)
AC_SUBST(CLOOG_PATH)

# **************************************************************************************
# Option to control internal support of ScopLib (A classic library for Polyhedral Model)
# **************************************************************************************

AC_ARG_WITH(
        [scoplib],
        AS_HELP_STRING([--with-scoplib@<:@=DIR@:>@], [use ScopLib]),
        [
        if test "$withval" = "no"; then
      AC_MSG_FAILURE([--with-scoplib=PATH must be specified to use option --with-scoplib (a valid ScopLib intallation)])
        elif test "$withval" = "yes"; then
      AC_MSG_FAILURE([--with-scoplib=PATH must be specified to use option --with-scoplib (a valid ScopLib intallation)])
        else
            has_scoplib_path="yes"
            scoplib_path="$withval"
        fi
        ],
        [has_scoplib_path="no"]
)

AC_ARG_ENABLE(
        scoplib,
        AS_HELP_STRING(
                [--enable-scoplib],
                [Support for ScopLib]
        )
)
AM_CONDITIONAL(
        ROSE_USE_SCOPLIB,
        [test "x$enable_scoplib" = "xyes"])
if test "x$enable_scoplib" = "xyes"; then
        if test "x$has_scoplib_path" = "xyes"; then
                SCOPLIB_PATH="$scoplib_path"
                AC_DEFINE([ROSE_USE_SCOPLIB], [], [Whether to use ScopLib support or not within ROSE])
        fi
fi
AC_SUBST(ROSE_USE_SCOPLIB)
AC_SUBST(SCOPLIB_PATH)

# *************************************************************************************
# Option to control internal support of Candl (Dependency analysis in Polyhedral Model)
# *************************************************************************************

AC_ARG_WITH(
        [candl],
        AS_HELP_STRING([--with-candl@<:@=DIR@:>@], [use Candl]),
        [
        if test "$withval" = "no"; then
            AC_MSG_FAILURE([--with-candl=PATH must be specified to use option --with-candl (a valid Candl intallation)])
        elif test "$withval" = "yes"; then
            AC_MSG_FAILURE([--with-candl=PATH must be specified to use option --with-candl (a valid Candl intallation)])
        else
            has_candl_path="yes"
            candl_path="$withval"
        fi
        ],
        [has_candl_path="no"]
)

AC_ARG_ENABLE(
        candl,
        AS_HELP_STRING(
                [--enable-candl],
                [Support for Candl]
        )
)
AM_CONDITIONAL(
        ROSE_USE_CANDL,
        [test "x$enable_candl" = "xyes"])
if test "x$enable_candl" = "xyes"; then
        if test "x$has_candl_path" = "xyes"; then
                CANDL_PATH="$candl_path"
                AC_DEFINE([ROSE_USE_CANDL], [], [Whether to use Candl support or not within ROSE])
        fi
fi
AC_SUBST(ROSE_USE_CANDL)
AC_SUBST(CANDL_PATH)

# *****************************************************************
#            Accelerator Support (CUDA, OpenCL)
# *****************************************************************

# Check: --with-cuda-inc, --with-cuda-lib, and  --with-cuda-bin
ROSE_CHECK_CUDA
# Check: --with-opencl-inc, --with-opencl-lib
ROSE_CHECK_OPENCL

# *****************************************************************
#            Option to define DOXYGEN SUPPORT
# *****************************************************************

# allow either user or developer level documentation using Doxygen
ROSE_SUPPORT_DOXYGEN

# Test for setup of document merge of Sage docs with Rose docs
# Causes document build process to take longer but builds better documentation
if (test "$enable_doxygen_generate_fast_docs" = yes) ; then
   AC_MSG_NOTICE([generate Doxygen documentation faster (using tag file mechanism)])
else
   AC_MSG_NOTICE([generate Doxygen documentation slower (reading all of Sage III and Rose together)])
fi

AC_PROG_CXXCPP

AC_DISABLE_STATIC

LT_AC_PROG_SED dnl This seems to not be called, even though it is needed in the other macros
m4_pattern_allow([LT_LIBEXT])dnl From http://www.mail-archive.com/libtool-commit@gnu.org/msg01369.html

LT_INIT([dlopen])
LT_CONFIG_LTDL_DIR([libltdl])
LTDL_CONVENIENCE
LTDL_INIT([recursive])

dnl AC_LT DL_SHLIBPATH dnl Get the environment variable like LD_LIBRARY_PATH for the Fortran support to use
dnl This seems to be an internal variable, set by different macros in different
dnl Libtool versions, but with the same name
AC_DEFINE_UNQUOTED(ROSE_SHLIBPATH_VAR, ["$shlibpath_var"], [Variable like LD_LIBRARY_PATH])

AC_TRY_EVAL(ac_compile);

AC_CHECK_HEADERS([dlfcn.h], [have_dladdr=yes], [have_dladdr=no])
if test "x$have_dladdr" = "xyes"; then
  AC_CHECK_LIB([dl], [dladdr], [], [have_dladdr=no])
fi
if test "x$have_dladdr" = "xyes"; then
  AC_DEFINE([HAVE_DLADDR], [], [Whether <dlfcn.h> and -ldl contain dladdr()])
  use_rose_in_build_tree_var=no
else
  AC_MSG_WARN([ROSE cannot find the locations of loaded shared libraries using your dynamic linker.  ROSE can only be used with the given build directory or prefix, and the ROSE_IN_BUILD_TREE environment variable must be used to distinguish the two cases.])
  use_rose_in_build_tree_var=yes
fi
AM_CONDITIONAL(USE_ROSE_IN_BUILD_TREE_VAR, [test "x$use_rose_in_build_tree_var" = "xyes"])

# Determine what C++ compiler is being used.
AC_MSG_CHECKING(what the C++ compiler $CXX really is)
BTNG_INFO_CXX_ID
AC_MSG_RESULT($CXX_ID-$CXX_VERSION)

# Enable turning on purify and setting its options, etc.
ROSE_SUPPORT_PURIFY

# Enable turning on Insure and setting its options, etc.
ROSE_SUPPORT_INSURE

# Determine how to create C++ libraries.
AC_MSG_CHECKING(how to create C++ libraries)
BTNG_CXX_AR
AC_MSG_RESULT($CXX_STATIC_LIB_UPDATE and $CXX_DYNAMIC_LIB_UPDATE)

# Let user specify where to find A++P++ installation.
# Specify by --with-AxxPxx= or setting AxxPxx_PREFIX.
# Note that the prefix specified should be that specified
# when installing A++P++.  The prefix appendages are also
# added here.
# BTNG.
AC_MSG_CHECKING(for A++P++)
AC_ARG_WITH(AxxPxx,
[  --with-AxxPxx=PATH   Specify the prefix where A++P++ is installed],
,
if test "$AxxPxx_PREFIX" ; then
   with_AxxPxx="$AxxPxx_PREFIX"
else
   with_AxxPxx=no
fi
)
test "$with_AxxPxx" && test "$with_AxxPxx" != no && AxxPxx_PREFIX="$with_AxxPxx"
AC_MSG_RESULT($AxxPxx_PREFIX)
if test "$AxxPxx_PREFIX" ; then
  # Note that the prefix appendages are added to AxxPxx_PREFIX to find A++ and P++.
  AC_MSG_RESULT(using $AxxPxx_PREFIX as path to A++ Library)
  Axx_INCLUDES="-I$AxxPxx_PREFIX/A++/lib/include"
  Axx_LIBS="-L$AxxPxx_PREFIX/A++/lib/lib -lApp -lApp_static -lApp"
  Pxx_INCLUDES="-I$AxxPxx_PREFIX/P++/lib/include"
  Pxx_LIBS="-L$AxxPxx_PREFIX/P++/lib/lib -lApp -lApp_static -lApp"
  # optional_AxxPxxSpecificExample_subdirs="EXAMPLES"
  # we will want to setup subdirectories in the TESTS directory later so set it up now
  # optional_AxxPxxSpecificTest_subdirs="A++Tests"
else
  AC_MSG_RESULT(No path specified for A++ Library)
fi
AC_SUBST(Axx_INCLUDES)
AC_SUBST(Axx_LIBS)
AC_SUBST(Pxx_INCLUDES)
AC_SUBST(Pxx_LIBS)

AM_CONDITIONAL(AXXPXX_SPECIFIC_TESTS,test ! "$with_AxxPxx" = no)

AC_ARG_WITH(PERFORMANCE_TESTS,
   [  --with-PERFORMANCE_TESTS ... compile and run performance tests within both A++ and P++],, with_PERFORMANCE_TESTS=no )

export with_PERFORMANCE_TESTS;

with_gcj=no ;
AM_CONDITIONAL(USE_GCJ,test "$with_gcj" = yes)

ROSE_CONFIGURE_SECTION([Checking system capabilities])

AC_SEARCH_LIBS(clock_gettime, [rt], [
  RT_LIBS="$LIBS"
  LIBS=""
],[
  RT_LIBS=""
])
AC_SUBST(RT_LIBS)

## This should be set after a complex test (turn it on as default)
AC_DEFINE([HAVE_EXPLICIT_TEMPLATE_INSTANTIATION],[],[Use explicit template instantiation.])

# Copied from the P++/configure.in
# Determine how to build a C++ library.
AC_MSG_CHECKING(how to build C++ libraries)
BTNG_CXX_AR
if test "$CXX_ID" = ibm; then
  # IBM does not have a method for supporting shared libraries
  # Here is a kludge.
  CXX_SHARED_LIB_UPDATE="`cd ${srcdir}/../config && pwd`/mklib.aix -o"
  BTNG_AC_LOG(CXX_SHARED_LIB_UPDATE changed to $CXX_SHARED_LIB_UPDATE especially for the IBM)
fi
AC_MSG_RESULT($CXX_STATIC_LIB_UPDATE and $CXX_SHARED_LIB_UPDATE)
AC_SUBST(CXX_STATIC_LIB_UPDATE)
AC_SUBST(CXX_SHARED_LIB_UPDATE)

# The STL tests use the CC command line which specifies -ptr$(CXX_TEMPLATE_REPOSITORY_PATH) but this
# is not defined in the shell so no substitution is done and a directory named
# $(CXX_TEMPLATE_REPOSITORY_PATH) is built in the top level directory.  The least we can do is
# delete it if we can't stop it from being generated.
# AC_MSG_RESULT(deleting temporary template directory built during STL tests.)
# rm -rf '$(CXX_TEMPLATE_REPOSITORY_PATH)'
rm -rf Templates.DB

# End macro ROSE_SUPPORT_ROSE_PART_2.
]
)

#-----------------------------------------------------------------------------

AC_DEFUN([ROSE_SUPPORT_ROSE_PART_3],
[
# Begin macro ROSE_SUPPORT_ROSE_PART_3.

## Setup the EDG specific stuff
SETUP_EDG

ROSE_ARG_ENABLE(
  [alternate-edg-build-cpu],
  [for alternate EDG build cpu],
  [allows you to generate EDG binaries with a different CPU type in the name string]
)

#The build_triplet_without_redhat variable is used only in src/frontend/CxxFrontend/Makefile.am to determine the binary edg name
build_triplet_without_redhat=`${srcdir}/config/cleanConfigGuessOutput "$build" "$build_cpu" "$build_vendor"`
if test "x$CONFIG_HAS_ROSE_ENABLE_ALTERNATE_EDG_BUILD_CPU" = "xyes"; then
  # Manually modify the build CPU <build_cpu>-<build_vendor>-<build>
  build_triplet_without_redhat="$(echo "$build_triplet_without_redhat" | sed 's/^[[^-]]*\(.*\)/'$ROSE_ENABLE_ALTERNATE_EDG_BUILD_CPU'\1/')"
fi
AC_SUBST(build_triplet_without_redhat) dnl This is done even with EDG source, since it is used to determine the binary to make in roseFreshTest

# End macro ROSE_SUPPORT_ROSE_PART_3.
])

#-----------------------------------------------------------------------------

AC_DEFUN([ROSE_SUPPORT_ROSE_PART_4],
[
# Begin macro ROSE_SUPPORT_ROSE_PART_4.

dnl ---------------------------------------------------------------------
dnl (8/29/2007): This was added to provide more portable times upon the
dnl suggestion of Matt Sottile at LANL.
dnl ---------------------------------------------------------------------
AC_C_INLINE
AC_CHECK_HEADERS([sys/time.h time.h c_asm.h intrinsics.h mach/mach_time.h])

AC_CHECK_TYPE([hrtime_t],[AC_DEFINE(HAVE_HRTIME_T, 1, [Define to 1 if hrtime_t is defined in <sys/time.h>])],,[#if HAVE_SYS_TIME_H
#include <sys/time.h>
#endif])

AC_CHECK_FUNCS([gethrtime read_real_time time_base_to_time clock_gettime mach_absolute_time])

dnl Cray UNICOS _rtc() (real-time clock) intrinsic
AC_MSG_CHECKING([for _rtc intrinsic])
rtc_ok=yes

dnl ---------------------------------------------------------------------

AC_LINK_IFELSE(
  [AC_LANG_PROGRAM(
    [[
#ifdef HAVE_INTRINSICS_H
#include <intrinsics.h>
#endif
/* includes */
    ]],
    [[
      /* body */
      _rtc()
    ]]
  )],
    [
      AC_DEFINE(HAVE__RTC,1,[Define if you have the UNICOS _rtc() intrinsic.])
    ],
    [
      rtc_ok=no
    ]
)

dnl ---------------------------------------------------------------------

AC_MSG_RESULT($rtc_ok)

# Record the location of the build tree (so it can be substituted into ROSE/docs/Rose/rose.cfg)
top_pwd=$PWD
AC_SUBST(top_pwd)
# echo "In ROSE/con figure: top_pwd = $top_pwd"

absolute_path_srcdir="`cd $srcdir; pwd`"
AC_SUBST(absolute_path_srcdir)

# Liao 6/20/2011, store source path without symbolic links, used to have consistent source and compile paths for ROSE
# when call graph analysis tests are used.
res_top_src=$(cd "$srcdir" && pwd -P)
AC_DEFINE_UNQUOTED([ROSE_SOURCE_TREE_PATH],"$res_top_src",[Location of ROSE Source Tree.])

# kelly64 (6/26/2013): Compass2 xml configuration files require fully-resolved
#                      absolute paths.
AC_SUBST(res_top_src)

# This is silly, but it is done to hide an include command (in
# projects/compass/Makefile.am, including compass-makefile.inc in the build
# tree) from Automake because the needed include file does not exist when
# automake is run
INCLUDE_COMPASS_MAKEFILE_INC="include compass_makefile.inc"
AC_SUBST(INCLUDE_COMPASS_MAKEFILE_INC)

# ROSE-HPCT module -- note that this needs the XML check to have already
# happened
ACROSE_ENABLE_ROSEHPCT

# PC (08/20/2009): Symbolic links need to be resolved for the callgraph analysis tests
res_top_pwd=$(cd "$top_pwd" && pwd -P)

# DQ (11/10/2007): Add paths defined by automake to the generated rose.h.in and rose.h
# header files so that this information will be available at compile time. Unclear
# which syntax is best for the specification of these paths.
AC_DEFINE_UNQUOTED([ROSE_COMPILE_TREE_PATH],"$res_top_pwd",[Location of ROSE Compile Tree.])

# This block turns off features of libharu that don't work with Java
with_png=no
export with_png
with_zlib=no
export with_zlib

# DQ (4/11/2010): This seems to have to appear before the Qt macros
# because the "AC PATH QT" are defined in config/qrose_indigo_1.m4.
# *****************************************************
#  Support for QRose Qt GUI (work at Imperial College)
# *****************************************************

# GMY (9/3/2008) QT4 & QROSE Optional Packages
AC_ARG_WITH(QRose, [  --with-QRose=PATH     prefix of QRose installation],
   [QROSE_PREFIX=$with_QRose
    if test "x$with_QRose" = xyes; then
       AC_MSG_FAILURE([--with-QRose=PATH must be specified to use option --with-QRose (a valid QRose intallation)])
    fi
    if test "x$with_QRose" = x; then
       AC_MSG_FAILURE([empty path used in --with-QRose=PATH must be specified to use option --with-QRose (a valid Qt intallation)])
    fi
   ],
        [with_QRose=no])

AC_SUBST(QROSE_PREFIX)
AM_CONDITIONAL(ROSE_USE_QROSE,test "x$with_QRose" != xno)

AC_MSG_NOTICE([with_QRose = "$with_QRose"])



# DQ (4/11/2010): Organized the Qt configure support.
# ****************************************************
#         Support for Qt (General GUI support)
# ****************************************************

# These are defined in config/qrose_indigo_1.m4, they
# are not standard AC macros.
AC_PATH_QT
AC_PATH_QT_MOC
AC_PATH_QT_RCC
AC_PATH_QT_UIC

# The code to set ROSEQT is in this macro's definition.
AC_PATH_QT_VERSION

AC_MSG_NOTICE([with_qt     = "$with_qt"])
if test "x$with_qt" = xyes; then
   AC_MSG_FAILURE([path to Qt not specified (usage: --with-qt=PATH)])
fi

# If QRose was specified then make sure that Qt was specified.
if test "x$with_QRose" != xno; then
   if test "x$with_qt" = xno; then
      AC_MSG_FAILURE([QRose requires valid specification of Qt installation (requires option: --with-qt=PATH)])
   fi
fi

# ****************************************************
#   Support for Assembly Semantics (binary analysis)
ROSE_SUPPORT_BINARY

ROSE_SUPPORT_YICES

ROSE_SUPPORT_PYTHON_API

# Added support for detection of libnuma, a NUMA aware memory allocation mechanism for many-core optimizations.
AC_CHECK_HEADERS(numa.h, [found_libnuma=yes])

if test "x$found_libnuma" = xyes; then
  AC_DEFINE([HAVE_NUMA_H],[],[Support for libnuma a NUMA memory allocation library for many-core optimizations])
fi

AM_CONDITIONAL(ROSE_USE_LIBNUMA, [test "x$found_libnuma" = xyes])

# PC (7/10/2009): The Haskell build system expects a fully numeric version number.
PACKAGE_VERSION_NUMERIC=`echo $PACKAGE_VERSION | sed -e 's/\([[a-z]]\+\)/\.\1/; y/a-i/1-9/'`
AC_SUBST(PACKAGE_VERSION_NUMERIC)

# This CPP symbol is defined so we can check whether rose_config.h is included into a public header file.  It serves
# no other purpose.  The name must not begin with "ROSE_" but must have a high probability of being globally unique (which
# is why it ends with "_ROSE").
AC_DEFINE(CONFIG_ROSE, 1, [Always defined and used for checking whether global CPP namespace is polluted])

# End macro ROSE_SUPPORT_ROSE_PART_4.
]
)

AC_DEFUN([CLASSPATH_COND_IF],
[m4_ifdef([AM_COND_IF],
  [AM_COND_IF([$1], [$3], [$4])],
  [if $2; then
     m4_default([$3], [:])
   else
     m4_default([$4], [:])
   fi
])])

#-----------------------------------------------------------------------------

AC_DEFUN([ROSE_SUPPORT_ROSE_PART_5],
[
# Begin macro ROSE_SUPPORT_ROSE_PART_5.

AC_MSG_NOTICE([CC = "$CC"])
AC_MSG_NOTICE([CPPFLAGS = "$CPPFLAGS"])

AC_MSG_NOTICE([subdirs = "$subdirs"])
AC_CONFIG_SUBDIRS([libltdl])

# The EDG build glue (src/frontend/CxxFrontend/EDG/{,edg_build,edgRose}/Makefile.am) is part of this repository,
# so these Makefiles are always generated; whether they are built depends on ROSE_HAS_EDG_SOURCE (see
# src/frontend/CxxFrontend/Makefile.am), i.e. on whether the edg_compiler submodule is checked out.
AC_CONFIG_FILES([
src/frontend/CxxFrontend/EDG/Makefile
src/frontend/CxxFrontend/EDG/edg_build/Makefile
src/frontend/CxxFrontend/EDG/edgRose/Makefile
])

# End macro ROSE_SUPPORT_ROSE_PART_5.
]
)


#-----------------------------------------------------------------------------
AC_DEFUN([ROSE_SUPPORT_ROSE_PART_6],
[
# Begin macro ROSE_SUPPORT_ROSE_PART_6.

AC_CONFIG_FILES([
Makefile
config/Makefile
docs/Makefile
docs/Rose/Makefile
docs/Rose/ROSE_DemoGuide.tex
docs/Rose/ROSE_DeveloperInstructions.tex
docs/Rose/ROSE_Exam.tex
docs/Rose/ROSE_InstallationInstructions.tex
docs/Rose/Tutorial/Makefile
docs/Rose/Tutorial/gettingStarted.tex
docs/Rose/Tutorial/tutorial.tex
docs/Rose/footer.html
docs/Rose/gettingStarted.tex
docs/Rose/leftmenu.html
docs/Rose/manual.tex
docs/Rose/rose-install-demo.cfg
docs/Rose/rose.cfg
docs/Rose/sage.cfg
docs/testDoxygen/Makefile
docs/testDoxygen/test.cfg
exampleTranslators/AstCopyReplTester/Makefile
exampleTranslators/DOTGenerator/Makefile
exampleTranslators/Makefile
exampleTranslators/PDFGenerator/Makefile
exampleTranslators/defaultTranslator/Makefile
exampleTranslators/documentedExamples/AstRewriteExamples/Makefile
exampleTranslators/documentedExamples/Makefile
exampleTranslators/documentedExamples/dataBaseExamples/Makefile
exampleTranslators/documentedExamples/simpleTranslatorExamples/Makefile
exampleTranslators/documentedExamples/simpleTranslatorExamples/exampleMakefile
LicenseInformation/Makefile
python/Makefile
python/Rose/Makefile
python/Rose/BinaryAnalysis/Makefile
python/Rose/BinaryAnalysis/Partitioner2/Makefile
python/Rose/BinaryAnalysis/Unparser/Makefile
scripts/Makefile
src/3rdPartyLibraries/json/Makefile
src/3rdPartyLibraries/json/nlohmann/Makefile
src/3rdPartyLibraries/MSTL/Makefile
src/3rdPartyLibraries/Makefile
src/3rdPartyLibraries/antlr-jars/Makefile
src/3rdPartyLibraries/flang-parser/Makefile
src/3rdPartyLibraries/fortran-parser/Makefile
src/3rdPartyLibraries/java-parser/Makefile
src/AstNodes/Makefile
src/AstNodes/BinaryAnalysis/Makefile
src/AstNodes/Expression/Makefile
src/Makefile
src/Rose/Makefile
src/Rosebud/Makefile
src/generated/Makefile
src/generated/BinaryAnalysis/Makefile
src/generated/Jovial/Makefile
src/generated/Rose/Makefile
src/generated/Rose/Sarif/Makefile
src/ROSETTA/Makefile
src/ROSETTA/src/Makefile
src/backend/Makefile
src/frontend/BinaryFormats/Makefile
src/frontend/CxxFrontend/Clang/Makefile
src/frontend/CxxFrontend/Makefile
src/frontend/Disassemblers/Makefile
src/frontend/ECJ_ROSE_Connection/Makefile
src/frontend/Experimental_General_Language_Support/Makefile
src/frontend/Experimental_General_Language_Support/ATerm/Makefile
src/frontend/Experimental_Flang_ROSE_Connection/Makefile
src/frontend/Experimental_Csharp_ROSE_Connection/Makefile
src/frontend/Experimental_Ada_ROSE_Connection/Makefile
src/frontend/Experimental_Ada_ROSE_Connection/parser/Makefile
src/frontend/Experimental_Ada_ROSE_Connection/parser/asis_adapter/Makefile
src/frontend/Experimental_Ada_ROSE_Connection/parser/ada_main/Makefile
src/frontend/Experimental_Libadalang_ROSE_Connection/Makefile
src/frontend/Experimental_Jovial_ROSE_Connection/Makefile
src/frontend/Experimental_Matlab_ROSE_Connection/Makefile
src/frontend/Makefile
src/frontend/OpenFortranParser_SAGE_Connection/Makefile
src/frontend/PHPFrontend/Makefile
src/frontend/PythonFrontend/Makefile
src/frontend/SageIII/Makefile
src/frontend/SageIII/astFileIO/Makefile
src/frontend/SageIII/astFixup/Makefile
src/frontend/SageIII/astFromString/Makefile
src/frontend/SageIII/astHiddenTypeAndDeclarationLists/Makefile
src/frontend/SageIII/astPostProcessing/Makefile
src/frontend/SageIII/astTokenStream/Makefile
src/frontend/SageIII/astVisualization/Makefile
src/frontend/SageIII/includeDirectivesProcessing/Makefile
src/frontend/SageIII/sage.docs
src/frontend/SageIII/sageInterface/Makefile
src/frontend/SageIII/virtualCFG/Makefile
src/midend/Makefile
src/midend/BinaryAnalysis/Makefile
src/midend/programAnalysis/Makefile
src/midend/programAnalysis/ssaUnfilteredCfg/Makefile
src/midend/programAnalysis/staticSingleAssignment/Makefile
src/midend/programAnalysis/systemDependenceGraph/Makefile
src/midend/programTransformation/extractFunctionArgumentsNormalization/Makefile
src/midend/programTransformation/loopProcessing/Makefile
src/midend/programTransformation/singleStatementToBlockNormalization/Makefile
src/roseExtensions/Makefile
src/roseExtensions/dataStructureTraversal/Makefile
src/roseExtensions/failSafe/Makefile
src/roseExtensions/highLevelGrammar/Makefile
src/roseExtensions/roseHPCToolkit/Makefile
src/roseExtensions/roseHPCToolkit/docs/Makefile
src/roseExtensions/roseHPCToolkit/include/Makefile
src/roseExtensions/roseHPCToolkit/include/rosehpct/Makefile
src/roseExtensions/roseHPCToolkit/include/rosehpct/gprof/Makefile
src/roseExtensions/roseHPCToolkit/include/rosehpct/profir/Makefile
src/roseExtensions/roseHPCToolkit/include/rosehpct/profir2sage/Makefile
src/roseExtensions/roseHPCToolkit/include/rosehpct/sage/Makefile
src/roseExtensions/roseHPCToolkit/include/rosehpct/util/Makefile
src/roseExtensions/roseHPCToolkit/include/rosehpct/xml-xercesc/Makefile
src/roseExtensions/roseHPCToolkit/include/rosehpct/xml/Makefile
src/roseExtensions/roseHPCToolkit/include/rosehpct/xml2profir/Makefile
src/roseExtensions/roseHPCToolkit/src/Makefile
src/roseExtensions/roseHPCToolkit/src/gprof/Makefile
src/roseExtensions/roseHPCToolkit/src/profir/Makefile
src/roseExtensions/roseHPCToolkit/src/profir2sage/Makefile
src/roseExtensions/roseHPCToolkit/src/sage/Makefile
src/roseExtensions/roseHPCToolkit/src/util/Makefile
src/roseExtensions/roseHPCToolkit/src/xml-xercesc/Makefile
src/roseExtensions/roseHPCToolkit/src/xml/Makefile
src/roseExtensions/roseHPCToolkit/src/xml2profir/Makefile
src/roseExtensions/sqlite3x/Makefile
src/roseIndependentSupport/Makefile
src/roseIndependentSupport/dot2gml/Makefile
src/roseSupport/Makefile
src/util/Makefile
src/util/commandlineProcessing/Makefile
src/util/graphs/Makefile
src/Sawyer/Makefile
src/util/stringSupport/Makefile
src/util/support/Makefile
stamp-h
tools/Makefile
tools/BinaryAnalysis/Makefile
tools/CodeThorn/Makefile
tools/CodeThorn/src/Makefile
tools/checkFortranInterfaces/Makefile
tools/classMemberVariablesInLambdas/Makefile
tools/featureVector/Makefile
tools/globalVariablesInLambdas/Makefile
tools/PortabilityTesting/Makefile
tools/PortabilityTesting/scripts/Makefile
tools/PortabilityTesting/src/Makefile
tools/ReadWriteSetGeneration/Makefile
tutorial/Makefile
tutorial/binaryAnalysis/Makefile
tutorial/exampleMakefile
tutorial/intelPin/Makefile
tutorial/outliner/Makefile
tutorial/roseHPCT/Makefile
])
]
)


#-----------------------------------------------------------------------------

AC_DEFUN([ROSE_SUPPORT_ROSE_PART_7],
[
# Begin macro ROSE_SUPPORT_ROSE_PART_7.

AC_CONFIG_COMMANDS([default],
    [
     AC_MSG_NOTICE([ensuring Grammar in the compile tree (assuming source tree is not the same as the compile tree)])
     pathToSourceDir="`cd $srcdir && pwd`"
     test -d src/ROSETTA/Grammar || ( rm -rf src/ROSETTA/Grammar && ln -s "$pathToSourceDir/src/ROSETTA/Grammar" src/ROSETTA/Grammar )
    ],
    [])

# Generate rose_paths.C
AC_CONFIG_COMMANDS([rose_paths.C], [
    AC_MSG_NOTICE([building src/util/rose_paths.C])
    make src/util/rose_paths.C
])

# Generate public config file from private config file. The public config file adds "ROSE_" to the beginning of
# certain symbols. See scripts/publicConfiguration.pl for details.
AC_CONFIG_COMMANDS([rosePublicConfig.h],[
    AC_MSG_NOTICE([building rosePublicConfig.h])
    make rosePublicConfig.h
])

# End macro ROSE_SUPPORT_ROSE_PART_7.
]
)
