AC_DEFUN([ROSE_SUPPORT_EDG], [

    ###############################################################################################################################
    # EDG, the C/C++ front end, is always compiled from source. The source is the git submodule
    # src/frontend/CxxFrontend/EDG/edg_compiler (a public fork of github.com/edgcpp/compiler carrying ROSE's patches,
    # https://github.com/rose-compiler/edg). Its version is read from src/version.h. There is no precompiled EDG
    # library any more: when C/C++ analysis is enabled and the Clang frontend is not, the submodule must be present.
    ###############################################################################################################################

    AC_ARG_ENABLE([edg-version],
                  [AS_HELP_STRING([--enable-edg-version=VERSION],
                                  [Assert the EDG version expected in the edg_compiler submodule (e.g. 6.5). Not needed
                                   normally: the version is read from the submodule; configure fails if they differ.])])

    edg_source_dir="${srcdir}/src/frontend/CxxFrontend/EDG/edg_compiler"
    edg_submodule_hint="run \"git submodule update --init src/frontend/CxxFrontend/EDG/edg_compiler\" in the ROSE source tree"

    if test -r "$edg_source_dir/src/basics.h" -a -r "$edg_source_dir/src/version.h"; then
        has_edg_source=yes
        edg_version=`sed -n 's/^#define VERSION_NUMBER *"\(.*\)".*/\1/p' "$edg_source_dir/src/version.h"`
        AC_MSG_NOTICE([EDG source present in src/frontend/CxxFrontend/EDG/edg_compiler: version $edg_version])
        if test -n "$enable_edg_version" -a "$enable_edg_version" != "$edg_version"; then
            AC_MSG_FAILURE([--enable-edg-version=$enable_edg_version does not match the EDG source present (version $edg_version)])
        fi
    else
        has_edg_source=no
        edg_version="${enable_edg_version:-0.0}"
        # support_cxx_frontend comes from ROSE_SUPPORT_LANGUAGES (run earlier); enable_clang_frontend from the command line.
        if test "x$support_cxx_frontend" = xyes -o "x$support_c_frontend" = xyes; then
            if test "x$enable_clang_frontend" != xyes; then
                AC_MSG_FAILURE([C/C++ analysis needs the EDG front end, but its source is not present in $edg_source_dir; $edg_submodule_hint, or configure without C/C++ support, or with --enable-clang-frontend])
            fi
        fi
        AC_MSG_NOTICE([EDG source not present; not needed for this configuration])
    fi

    edg_major_version_number=`echo $edg_version | cut -d. -f1`
    edg_minor_version_number=`echo $edg_version | cut -d. -f2`
    if test -z "$edg_major_version_number" -o -z "$edg_minor_version_number"; then
        AC_MSG_FAILURE([cannot parse EDG version "$edg_version" (expected MAJOR.MINOR)])
    fi

    enable_new_edg_interface=yes
    AC_DEFINE_UNQUOTED([ROSE_EDG_MAJOR_VERSION_NUMBER], $edg_major_version_number , [EDG major version number])
    AC_DEFINE_UNQUOTED([ROSE_EDG_MINOR_VERSION_NUMBER], $edg_minor_version_number , [EDG minor version number])

    ROSE_EDG_MAJOR_VERSION_NUMBER=$edg_major_version_number
    ROSE_EDG_MINOR_VERSION_NUMBER=$edg_minor_version_number

    AC_SUBST(ROSE_EDG_MAJOR_VERSION_NUMBER)
    AC_SUBST(ROSE_EDG_MINOR_VERSION_NUMBER)

    # Per-version macros and automake conditionals. New code should test ROSE_EDG_MAJOR_VERSION_NUMBER and
    # ROSE_EDG_MINOR_VERSION_NUMBER instead; these remain because rose-tests and rose-config.cfg still use them.
    AS_CASE([$edg_version],
            [5.0], [AC_DEFINE([ROSE_USE_EDG_VERSION_5_0], [], [Whether to use EDG version 5.0])],
            [6.0], [AC_DEFINE([ROSE_USE_EDG_VERSION_6_0], [], [Whether to use EDG version 6.0])],
            [6.3], [AC_DEFINE([ROSE_USE_EDG_VERSION_6_3], [], [Whether to use EDG version 6.3])],
            [6.5], [AC_DEFINE([ROSE_USE_EDG_VERSION_6_5], [], [Whether to use EDG version 6.5])],
            [7.0], [AC_DEFINE([ROSE_USE_EDG_VERSION_7_0], [], [Whether to use EDG version 7.0])],
            [AC_MSG_NOTICE([no ROSE_USE_EDG_VERSION_x_y macro for EDG $edg_version; use ROSE_EDG_MAJOR/MINOR_VERSION_NUMBER])])

    AM_CONDITIONAL(ROSE_USE_EDG_VERSION_5_0,  [test "$edg_version" = 5.0])
    AM_CONDITIONAL(ROSE_USE_EDG_VERSION_6_0,  [test "$edg_version" = 6.0])
    AM_CONDITIONAL(ROSE_USE_EDG_VERSION_6_3,  [test "$edg_version" = 6.3])
    AM_CONDITIONAL(ROSE_USE_EDG_VERSION_6_5,  [test "$edg_version" = 6.5])

    # Whether the EDG directory is built (src/frontend/CxxFrontend/Makefile.am).
    AM_CONDITIONAL(ROSE_HAS_EDG_SOURCE, [test "$has_edg_source" = "yes"])
])

