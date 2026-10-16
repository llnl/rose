AC_DEFUN([ROSE_SUPPORT_EDG], [

    ###############################################################################################################################
    # This part of the configuration is about what *version* of EDG to use when building ROSE. It has nothing to do with deciding
    # whether to compile EDG from source code, use a precompiled EDG binary, or do neither (that's all later).
    #
    # The EDG source lives in the git submodule src/frontend/CxxFrontend/EDG/edg_compiler (a public fork of
    # github.com/edgcpp/compiler carrying ROSE's patches). When it is checked out, the version is read from its
    # src/version.h and --enable-edg-version, if given, must agree with it. Without the source, --enable-edg-version
    # (default 6.5) selects which precompiled EDG library to download.
    ###############################################################################################################################

    AC_ARG_ENABLE([edg-version],
                  [AS_HELP_STRING([--enable-edg-version=VERSION],
                                  [Specifies the version of EDG to use as the parser for C/C++ when ROSE is configured
                                   to be able to analyze C and C++. Only needed when the EDG source (the edg_compiler
                                   submodule) is not present and a precompiled EDG library is to be downloaded; when
                                   the source is present its version is used and this value must match it.])])

    edg_source_dir="${srcdir}/src/frontend/CxxFrontend/EDG/edg_compiler"
    edg_version_from_source=
    if test -r "$edg_source_dir/src/version.h"; then
        edg_version_from_source=`sed -n 's/^#define VERSION_NUMBER *"\(.*\)".*/\1/p' "$edg_source_dir/src/version.h"`
    fi

    if test -n "$edg_version_from_source"; then
        AC_MSG_NOTICE([EDG source present in src/frontend/CxxFrontend/EDG/edg_compiler: version $edg_version_from_source])
        if test -n "$enable_edg_version" -a "$enable_edg_version" != "$edg_version_from_source"; then
            AC_MSG_FAILURE([--enable-edg-version=$enable_edg_version does not match the EDG source present (version $edg_version_from_source)])
        fi
        edg_version="$edg_version_from_source"
    elif test -n "$enable_edg_version"; then
        AC_MSG_NOTICE([no EDG source present; EDG version $enable_edg_version requested for the precompiled library])
        edg_version="$enable_edg_version"
    else
        AC_MSG_NOTICE([no EDG source present; default EDG version 6.5 used for the precompiled library])
        edg_version=6.5
    fi

    edg_major_version_number=`echo $edg_version | cut -d. -f1`
    edg_minor_version_number=`echo $edg_version | cut -d. -f2`
    if test -z "$edg_major_version_number" -o -z "$edg_minor_version_number"; then
        AC_MSG_FAILURE([cannot parse EDG version "$edg_version" (expected MAJOR.MINOR)])
    fi
    AC_MSG_NOTICE([EDG version $edg_major_version_number.$edg_minor_version_number])

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

    ###############################################################################################################################
    # This part of the configuration is about whether to compile the EDG library from source code, or to use a precompiled EDG
    # library that's downloaded, or to not link with EDG at all (e.g., when ROSE is configured without C or C++ support).
    ###############################################################################################################################

    AC_ARG_ENABLE([edg-compile],
                  [AS_HELP_STRING([--enable-edg-compile=maybe|yes|no],
				  [Whether to compile the Edison Design Group (EDG) C/C++ parsing library from source code.
				   The default, "maybe", means that the library is compiled from source code if the source code
				   is available (the edg_compiler git submodule is checked out) otherwise the build will attempt
				   to download a precompiled version of the library. The values "yes" and "no" are used by the
				   ROSE development team for testing and mean, respectively, fail if the source code is not
				   available or ignore the source code even if it is available.])],
		  [want_edg_source="$enableval"],
		  [want_edg_source=maybe])

    if test -r "$edg_source_dir/src/basics.h"; then
        has_edg_source=yes
    else
        has_edg_source=no
    fi

    if test "$want_edg_source" = yes -a "$has_edg_source" = no; then
        AC_MSG_FAILURE(["--enable-edg-compile=yes" was specified but the EDG source code is not present (run "git submodule update --init src/frontend/CxxFrontend/EDG/edg_compiler")])
    elif test "$want_edg_source" = no; then
        AC_MSG_NOTICE([EDG source code ignored even if present])
        has_edg_source=no
    fi

    AM_CONDITIONAL(ROSE_HAS_EDG_SOURCE, [test "$has_edg_source" = "yes"])

    ###############################################################################################################################
    # This part of the configuration is about whether we should build an EDG binary tarball by compiling the EDG source code.
    ###############################################################################################################################

    # Find md5 or md5sum and create a signature for ROSE binary compatibility
    AC_CHECK_PROGS(MD5, [md5 md5sum], [false])
    AC_SUBST(MD5)
    if test "$has_edg_source" = yes; then
        if test "$MD5" = "false"; then
            AC_MSG_WARN([could not find either md5 or md5sum; building binary EDG tarballs is disabled])
            binary_edg_tarball_enabled=no
        else
            binary_edg_tarball_enabled=yes
        fi
    else
        binary_edg_tarball_enabled=no
    fi

    AM_CONDITIONAL(BINARY_EDG_TARBALL_ENABLED, [test "$binary_edg_tarball_enabled" = "yes"])
])

