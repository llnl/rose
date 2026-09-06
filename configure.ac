dnl DQ (1/11/2010): Consider handing configure options as specified at:
dnl    https://projects.coin-or.org/BuildTools/wiki/user-configure

dnl The version number in the AC_INIT([ROSE], [2.15.0])
dnl do the replacement here in the configure file because AC_INIT([ROSE], [2.15.0])
dnl shell variable.
AC_INIT([ROSE], [2.15.0])
AC_PREREQ([2.59])

# A new definition to tweak code for Autoconf 
AC_DEFINE([USE_AUTOCONF], [1], [Define if using Autoconf])

# This one should perhaps be named ROSE_VERSION_STR instead of ROSE_SCM_VERSION_ID since it is not the SCM system's version
# and it is not an identifier but rather a string literal.
AC_DEFINE_UNQUOTED([ROSE_SCM_VERSION_ID], ["$(cat $srcdir/ROSE_VERSION)"],      [Set SCM version information to ROSE version.])

# A CPP constant that can be used in "#if" directives to conditionally compile source code based on the ROSE version number. The
# ROSE version "A.B.C.D" is encoded as an unsigned long "ABBCCCDDDD". For example, version 0.9.13.17 is encoded as 90130017.
# Although the value is read from a file named SCM_DATE it doesn't have anything to do with a date and it's not the SCM system's
# version but rather ROSE's version. The choice of the name "ROSE_VERSION" to hold an integer encoded version number follows
# precedent set by many other packages including Boost, SQLite, Z3, etc.
AC_DEFINE_UNQUOTED([ROSE_VERSION], [$(cat $srcdir/config/SCM_DATE)ul], [Integer encoded ROSE version A.B.C.D as ABBCCCDDDD.])

test "$verbose" = "" && echo "Run configure with --verbose to see more output"

dnl +--------------------------------------------------------------------------+
dnl | Utilities
dnl +--------------------------------------------------------------------------+
rose_shtool="$srcdir/scripts/shtool"
T_BM="$("$rose_shtool" echo -n -e %B)" dnl "terminal bold mode"
T_NM="$("$rose_shtool" echo -n -e %b)" dnl "terminal normal mode"

dnl +--------------------------------------------------------------------------+
dnl | Source tree build - disabled
dnl +--------------------------------------------------------------------------+
AC_CONFIG_SRCDIR([README.md])
if test -e "README.md"; then
  AC_MSG_FAILURE(
    [
      ROSE cannot be built in the source tree '$(dirname "$0")' yet.
      Please run $0 in a different directory.
    ]
  )
fi

dnl +--------------------------------------------------------------------------+
dnl | ROSE versioning
dnl +--------------------------------------------------------------------------+
ROSE_CONFIGURE_SECTION([Configuring ${T_BM}ROSE${T_NM}])

dnl +--------------------------------------------------------------------------+
dnl | Autotools setup
dnl +--------------------------------------------------------------------------+
AC_CONFIG_AUX_DIR([config])

# DQ (9/26/2015): Added option to eliminate large number of automake warnings (subdir-objects)
# RPM (9/29/2015): "subdir-objects" is causing $ROSE_SRC to be modified by "make"
# DQ (10/2/2015): Note that not using "foreign" option causes a LOT of warnings.
# AM_INIT_AUTOMAKE([foreign tar-pax 1.9.5])
# AM_INIT_AUTOMAKE([foreign tar-pax 1.9.5 subdir-objects])
# AM_INIT_AUTOMAKE([tar-pax 1.9.5])
# AM_INIT_AUTOMAKE([foreign tar-pax 1.9.5])
AM_INIT_AUTOMAKE([foreign tar-pax 1.11])

AC_CONFIG_HEADERS([rose_config.h])

# TOO1 (9/23/2013): Use the builtin verbose mechanism of Automake:
#
#                   ```bash
#                   $ make V=0
#                   $ make V=1
#                   ```
#
# We put this in an ifdef for backward compatibility since this
# macro (AM_SILENT_RULES) is only supported for Automake v1.11+.
#
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

dnl +--------------------------------------------------------------------------+
dnl | Environment Variables - Compilers (CC, CXX, FC)
dnl +--------------------------------------------------------------------------+
AC_MSG_CHECKING([if \$CC environment variable is set])
if test -z "${CC}"; then
    AC_MSG_RESULT([no - will attempt to auto-detect the C compiler])
else
    AC_MSG_RESULT([$CC])
fi

AC_MSG_CHECKING([if \$CXX environment variable is set])
if test -z "${CXX}"; then
    AC_MSG_RESULT([no - will attempt to auto-detect the C++ compiler])
else
    AC_MSG_RESULT([$CXX])
fi

AC_MSG_CHECKING([if \$FC environment variable is set])
if test -z "${FC}"; then
    AC_MSG_RESULT([no - will attempt to auto-detect the Fortran compiler])
else
    AC_MSG_RESULT([$FC])
fi

AM_PROG_CC_C_O

# JJW (10/8/2008): Make default CFLAGS, CXXFLAGS, and FFLAGS empty (from
# <URL:http://osdir.com/ml/sysutils.autoconf.general/2003-11/msg00124.html>)
: ${CFLAGS:=}
: ${CXXFLAGS:=}
: ${FFLAGS:=}

dnl Liao 8/17/2010. A work around a undefined SED on NERSC hopper.
dnl I had to promote it here instead of put it into ROSE_SUPPORT_ROSE_PART_2
dnl since the expansion order there is wrong.
test -z "$SED" && SED=sed

dnl +--------------------------------------------------------------------------+
dnl | ROSE Support
dnl +--------------------------------------------------------------------------+

dnl This macro calls all of the other macros that used to be in the body of
dnl this configure.in file.  This permits other projects to call this same
dnl macro and setup the variables and macros that an external project may
dnl also require.
ROSE_SUPPORT_ROSE_PART_1

ROSE_SUPPORT_ROSE_BUILD_INCLUDE_FILES
CONFIGURE_POET

# Rasmussen (11/19//2019): This is a strange bug. I can't figure out where
# the bug actually occurs in the m4 macros. However the symptom is that
# ECHO may not be set after this point but it is used. Thus setting ECHO
# may be needed. This error was noticed after a new release of XCode (11.2.1)
# was installed but probably is not related. It is more likely related to
# libtools.m4. After talking with Robb we realized that the Mac version is
# different from linux. I believe that somehow a portion of the libtool
# code is copied when ./build is run (at least this is consistent with
# the behavior.
if test "x$ECHO" = x; then
    ECHO=echo
fi

ROSE_SUPPORT_ROSE_PART_2
ROSE_SUPPORT_ROSE_PART_3
ROSE_SUPPORT_ROSE_PART_4
ROSE_SUPPORT_ROSE_PART_5
ROSE_OPTIONAL_PROJECTS
ROSE_SUPPORT_ROSE_PART_6
ROSE_SUPPORT_ROSE_PART_7

# Last minute changes to the ROSE_CONFIG_TOKEN shell variable, and then make sure it's
# output as a C preprocessor macro.
if echo "$BOOST_CPPFLAGS" |grep -- -pthread >/dev/null; then
   ROSE_CONFIG_TOKEN="$ROSE_CONFIG_TOKEN pthread"
fi
AC_DEFINE_UNQUOTED(ROSE_CONFIG_TOKEN, ["$ROSE_CONFIG_TOKEN"],
    [Encapsulates some important configuration details for Rose::initialize])

dnl Fail gracefully for blacklisted dependencies and other situations
ROSE_SUPPORT_BLACKLIST

dnl Now generate all the output files (specified above)
ROSE_CONFIGURE_SECTION([Generating files])
AC_OUTPUT

dnl +--------------------------------------------------------------------------+

cat <<X
${T_BM}
+--------------------------------+
| Configure Terminated Normally! |
+--------------------------------+
${T_NM}
Now type 'make core -jN' and then 'make install-core -jN' to only build essential libraries and tools (Recommended).
Type 'make -jN' and 'make install -jN' to build and install everything (takes a very long time).
N is the number of concurrent processes you want to use for your build.
Thank you for using ROSE.
X
