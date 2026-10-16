# Summarizes the _important_ configuration results for the user.

########################################################################################################################
#                    DO NOT SUMMARIZE VARIABLES THAT THE __USER__ DOESN'T CARE ABOUT!
#
# For instance, if the user didn't enable C++ as a analyzable language, then don't tell him what the backend C++
# compiler is.
#
# Also, do not add things here that are just for your own debugging. Debugging output should be close to where the
# detection is happening, and should use the AC_MSG_NOTICE macro, not "echo". This file is for summaries only!
#
########################################################################################################################

AC_DEFUN([ROSE_SUMMARY_HEADING],[
    echo "  $1"
])

AC_DEFUN([ROSE_SUPPORT_SUMMARY],[
    echo "${T_BM}Configuration summary:${T_NM}"

    #--------------------------------------------------------------------------------
    # ROSE conflates the compilers used to compile ROSE with the compiler frontend
    # compiler used for C++ analysis, thus the variables printed for this section
    # of the summary are named "FRONTEND".
    ROSE_SUMMARY_HEADING([Compilers for ROSE source code])
    echo "    C++ compiler                     ${CXX}"
    echo "    C++ vendor                       ${HOST_CXX_VENDOR:-unknown}"
    echo "    C++ version                      ${HOST_CXX_VERSION:-unknown}"
    echo "    C++ language                     ${HOST_CXX_LANGUAGE:-unknown}"
    echo "    Bison version                    ${bison_version:-unknown}"

    #--------------------------------------------------------------------------------
    ROSE_SUMMARY_HEADING([Languages supported for analysis])
    echo "    Binaries                         ${support_binaries_frontend:-no}"
    echo "    C                                ${support_c_frontend:-no}"
    echo "    C++                              ${support_cxx_frontend:-no}"
    echo "    C preprocessor                   ${support_cpp_frontend:-no}"
    echo "    Cuda                             ${support_cuda_frontend:-no}"
    echo "    Fortran                          ${support_fortran_frontend:-no}"
    echo "    Java                             ${support_java_frontend:-no}"
    echo "    OpenCL                           ${support_opencl_frontend:-no}"
    echo "    PHP                              ${support_php_frontend:-no}"
    echo "    Python                           ${support_python_frontend:-no}"

    #--------------------------------------------------------------------------------
    ROSE_SUMMARY_HEADING([Boost library])
    echo "    location                         ${ac_boost_path:-unknown}"
    echo "    version constant                 ${rose_boost_version:-unknown}"
    echo "    asio library                     ${BOOST_ASIO_LIB:-none}"
    echo "    atomic library                   ${BOOST_ATOMIC_LIB:-none}"
    echo "    chrono library                   ${BOOST_CHRONO_LIB:-none}"
    echo "    date/time library                ${BOOST_DATE_TIME_LIB:-none}"
    echo "    filesystem library               ${BOOST_FILESYSTEM_LIB:-none}"
    echo "    iostreams library                ${BOOST_IOSTREAMS_LIB:-none}"
    echo "    program options library          ${BOOST_PROGRAM_OPTIONS_LIB:-none}"
    echo "    python library                   ${BOOST_PYTHON_LIB:-none}"
    echo "    random library                   ${BOOST_RANDOM_LIB:-none}"
    echo "    regex library                    ${BOOST_REGEX_LIB:-none}"
    echo "    serialization library            ${BOOST_SERIALIZATION_LIB:-none}"
    echo "    signals library                  ${BOOST_SIGNALS_LIB:-none}"
    echo "    system library                   ${BOOST_SYSTEM_LIB:-none}"
    echo "    test exec monitor library        ${BOOST_TEST_EXEC_MONITOR_LIB:-none}"
    echo "    thread library                   ${BOOST_THREAD_LIB:-none}"
    echo "    unit test framework library      ${BOOST_UNIT_TEST_FRAMEWORK_LIB:-none}"
    echo "    wave library                     ${BOOST_WAVE_LIB:-none}"
    echo "    wserialization library           ${BOOST_WSERIALIZATION_LIB:-none}"

    #--------------------------------------------------------------------------------
    if test -n "$support_binaries_frontend" -o -n "$verbose"; then
        ROSE_SUMMARY_HEADING([Binary analysis support])
	echo "    Capstone          (ARM decoding) ${ROSE_HAVE_CAPSTONE:-none}"
	echo "    Dlib          (graph algorithms) ${DLIB_PREFIX:-none}"
	echo "    DWARF     (ELF format debugging) ${ROSE_HAVE_LIBDWARF:-none}"
	echo "    ELF       (only needed by DWARF) ${ROSE_HAVE_LIBELF:-none}"
	echo "    i386 support                     ${CFLAGS32:-none}"
	echo "    magic      (file identification) ${ROSE_HAVE_LIBMAGIC:-none}"
	echo "    readline     (interactive input) ${LIBREADLINE_LIBRARY_PATH:-none}"
	echo "    SQLite3     (database in a file) ${SQLITE3_VERSION:-none}"
	echo "    YAML-cpp         (configuration) ${ROSE_YAMLCPP_PREFIX:-none}"
	echo "    z3 version          (SMT solver) ${Z3_VERSION:-unknown}"
	echo "    z3 executable                    ${Z3:-none}"
	echo "    z3 library                       ${Z3_LIBRARY_PATH:-none}"
    fi

    #--------------------------------------------------------------------------------
    if test -n "$support_cxx_frontend" -o -n "$verbose"; then
        ROSE_SUMMARY_HEADING([C/C++ analysis support])
	echo "    frontend C++ parser              EDG-${edg_major_version_number}.${edg_minor_version_number}"
	echo "    compiling EDG from source        $has_edg_source"
	echo "    backend C++ compiler             ${BACKEND_CXX_COMPILER_COMMAND:-none}"
	echo "    C++ vendor                       ${BACKEND_CXX_COMPILER_VENDOR:-unknown}"
	echo "    C++ version                      ${BACKEND_CXX_VERSION_TRIPLET:-unknown}"
	echo "    backend C compiler               ${BACKEND_C_COMPILER:-none}"
    fi

    #--------------------------------------------------------------------------------
    if test -n "$support_fortran_frontend" -o -n "$verbose"; then
        ROSE_SUMMARY_HEADING([Fortran analysis support])
	echo "    fortran compiler                 ${BACKEND_FORTRAN_COMPILER:-none}"
    fi

    #--------------------------------------------------------------------------------
    if test -n "$support_java_frontend" -o -n "$verbose"; then
        ROSE_SUMMARY_HEADING([Java analysis support])
        echo "    installation path            	   ${JAVA_PATH:-none}"
	echo "    C++ header switches          	   ${JAVA_JVM_INCLUDE:-none}"
	echo "    C++ link switches            	   ${JAVA_JVM_LINK:-none}"
	echo "    runtime command                  ${JAVA:-none}"
	echo "    compiler command                 ${JAVAC:-none}"
	echo "    archive tool                     ${JAR:-none}"
    fi

    #--------------------------------------------------------------------------------
    if test -n "$support_python_frontend" -o -n "$verbose"; then
        ROSE_SUMMARY_HEADING([Python analysis support])
        echo "    executable                       ${PYTHON:-none}"
	echo "    version                          $PYTHON_VERSION_MAJOR_VERSION.$PYTHON_VERSION_MAJOR_VERSION.$PYTHON_VERSION_MINOR_VERSION.$PYTHON_VERSION_PATCH_VERSION"
	echo "    site pkg                         ${PYTHON_SITE_PKG:-none}"
        echo "    dev exists                       $PYTHON_DEV_EXISTS"
        echo "    c preprocessor switches          ${PYTHON_CPPFLAGS:-none}"
        echo "    linker switches                  ${PYTHON_LDFLAGS:-none}"
        echo "    extra libraries                  ${PYTHON_EXTRA_LIBS:-none}"
        echo "    extra linker switches            ${PYTHON_EXTRA_LDFLAGS:-none}"
        echo "    use python                       ${PYTHON:-no}"
        echo "    use python development           ${ROSE_USE_PYTHON_DEV:-no}"
    fi

    #--------------------------------------------------------------------------------
    if test -n "$DOXYGEN" -o -n "$verbose"; then
        ROSE_SUMMARY_HEADING([Doxygen documentation generator])
	echo "    executable                       ${DOXYGEN:-none}"
	echo "    version                          ${DOXYGEN_VERSION:-unknown}"
        echo "    indexer                          ${DOXYINDEXER:-none}"
        echo "    stubs for undocumented entities  ${DOXYGEN_EXTRACT_ALL:-no}"
        echo "    document private entities        ${DOXYGEN_EXTRACT_PRIVATE:-no}"
        echo "    document static entities         ${DOXYGEN_EXTRACT_STATIC:-no}"
        echo "    document internal entities       ${DOXYGEN_INTERNAL_DOCS:-no}"
        echo "    hide undocumented members        ${DOXYGEN_HIDE_UNDOC_MEMBERS:-no}"
        echo "    hide undocumented classes        ${DOXYGEN_HIDE_UNDOC_CLASSES:-no}"
        echo "    hide friend compounds            ${DOXYGEN_HIDE_FRIEND_COMPOUNDS:-no}"
    fi
])
