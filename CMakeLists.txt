########################################################################################################################
# General warnings, instructions, and style guide.
########################################################################################################################

# 1. Please be careful about where to put your tests and variable settings. The order matters!!
#
# 2. CMake configuration files are neither completely case-sensitive nor completely case-insensitive. Therefore, to
#    avoid errors, assume everything is case sensitive. Prefer lower case for function names.
#
# 3. Do not use argument turds in "else()" and "endif()" functions. Those arguments are only comments and are entirely
#    redundant with the "if()" argument and easily become out of date and misleading since nobody actually reads them.
#
# 4. All messages should start with a capitalized word except in special circumstances where capitalization would be
#    incorrect (such as the name of a command).
#
# 5. Indentation is two spaces. Do not use ASCII horizontal tab characters for indentation or alignment.
#
# 6. As with standard mathematical notation, there should be no white space on either side of a function's
#    parentheses. This includes functions such as "if", "else", and "endif".
#
# 7. Prefer FALSE and TRUE as Boolean values since these are the names used in Mathematics and most other computer
#    languages. Avoid OFF, ON, NO, YES, 0, 1, and especially avoid mixing them.
#
# 8. If a CMakefile file needs to be conditionally enabled, do it in that CMake file rather than around the
#    add_subdirectory in the level above. This keeps all the logic for a directory in a single file rather than
#    split across two files. It is a bit unfortunate that CMake can't find the lower-level CMakeList files
#    on its own, so some of the logic is still necessarily in the level above.  There are exceptions to this rule,
#    and they're pretty obvious when they occur--as when a single if() protects a whole bunch of add_subdirectory.

########################################################################################################################
# Platform-independent settings
########################################################################################################################
cmake_minimum_required(VERSION 3.15)

#-----------------------------------------------------------------------------------------------------------------------
# CCACHE
# Enable ccache if not already enabled by symlink masquerading and if no other compiler launchers are already defined
#-----------------------------------------------------------------------------------------------------------------------
find_program(CCACHE_EXECUTABLE ccache)
mark_as_advanced(CCACHE_EXECUTABLE)
if(CCACHE_EXECUTABLE)
  foreach(LANG C CXX)
    if(NOT DEFINED CMAKE_${LANG}_COMPILER_LAUNCHER AND NOT CMAKE_${LANG}_COMPILER MATCHES ".*/ccache")
      message(STATUS "Enabling ccache for ${LANG}")
      set(CMAKE_${LANG}_COMPILER_LAUNCHER ${CCACHE_EXECUTABLE} CACHE STRING "")
    endif()
  endforeach()
endif()

# Option for custom compilers
option(SKIP_ABI "Option to use custom CC/CXX" OFF)
if(DEFINED CACHE{SKIP-ABI})
  message(DEPRECATION "SKIP-ABI is deprecated. Use SKIP_ABI instead. Example: cmake -DSKIP_ABI=${SKIP-ABI} ...")
  set(SKIP_ABI ${SKIP-ABI} CACHE BOOL "Option to use custom CC/CXX" FORCE)
endif()
if(SKIP_ABI) # turn off checks and abi
  set(CMAKE_C_ABI_COMPILED FALSE)
  set(CMAKE_C_COMPILER_WORKS ON CACHE INTERNAL "")
  set(CMAKE_C_STANDARD_LIBRARIES_INIT "")
  set(CMAKE_CXX_ABI_COMPILED FALSE)
  set(CMAKE_CXX_COMPILER_WORKS ON CACHE INTERNAL "")
  set(CMAKE_CXX_STANDARD_LIBRARIES_INIT "")
endif()

#-----------------------------------------------------------------------------------------------------------------------
# Read ROSE version from ROSE_VERSION file before calling project()
# This is needed so that the project version can be set correctly, which affects library versioning
#-----------------------------------------------------------------------------------------------------------------------
set(ROSE_SCM_VERSION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/ROSE_VERSION")
if(NOT EXISTS "${ROSE_SCM_VERSION_FILE}")
  message(FATAL_ERROR "ROSE version file not found: ${ROSE_SCM_VERSION_FILE}")
endif()
file(STRINGS ${ROSE_SCM_VERSION_FILE} ROSE_PACKAGE_VERSION LIMIT_COUNT 1)

project(ROSE
  VERSION ${ROSE_PACKAGE_VERSION}
  LANGUAGES C CXX
)

#-----------------------------------------------------------------------------------------------------------------------
# Adjust CMake's default optimization, debug, and assertion flags for different build types.
#
# "RelWithDebInfo" is intended to be optimized with debug info, but CMake adds -DNDEBUG by default which disables
# assertions. We want assertions enabled by default for this build type. If you don't want assertions then use
# "-DCMAKE_C_FLAGS=-DNDEBUG" and "-DCMAKE_CXX_FLAGS=-DNDEBUG". The optimization level defaults to "-O2", which
# is a compromise between full optimization and the ability to use most features of interactive debuggers.
#
# "Debug" is intended for developers to debug their libraries and tools, but CMake doesn't turn off optimizations. We
# add "-O0" so that all features of interactive debugging work properly, although this has a major negative effect on
# performance. We don't use "-Og" because this doesn't give the best debugging experience (too many local variables
# get optimized out and therefore one cannot inspect them in the debugger).
# -----------------------------------------------------------------------------------------------------------------------
string(REPLACE "-DNDEBUG" "" CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO}")
string(REPLACE "-DNDEBUG" "" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")

#-----------------------------------------------------------------------------------------------------------------------
# BLT
#-----------------------------------------------------------------------------------------------------------------------
 # Use internal BLT if no BLT_SOURCE_DIR is given
if(NOT DEFINED BLT_SOURCE_DIR)
  set(BLT_SOURCE_DIR "${PROJECT_SOURCE_DIR}/cmake/blt" CACHE PATH "Path to BLT")
endif()

# Support having a shared BLT outside of the repository if given a BLT_SOURCE_DIR
if(NOT EXISTS ${BLT_SOURCE_DIR}/SetupBLT.cmake)
  message(FATAL_ERROR "Given BLT_SOURCE_DIR does not contain SetupBLT.cmake or"
                      "the BLT git submodule is not present. "
                      "Either run the following two commands in your git repository: \n"
                      "    git submodule init\n"
                      "    git submodule update\n"
                      "Or add -DBLT_SOURCE_DIR=/path/to/blt to your CMake command." )
endif()

# If the user sets CMAKE_CXX_STANDARD, use that for BLT_CXX_STD as well
if(CMAKE_CXX_STANDARD)
  set(BLT_CXX_STD "c++${CMAKE_CXX_STANDARD}" CACHE STRING "")
elseif(NOT BLT_CXX_STD)
  # Default to C++14 if not set so GTest/GMock can build
  set(BLT_CXX_STD "c++14" CACHE STRING "")
  set(CMAKE_CXX_STANDARD 14)
endif()
  set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(${BLT_SOURCE_DIR}/SetupBLT.cmake)

#-----------------------------------------------------------------------------------------------------------------------
# CMAKE POLICIES  
#-----------------------------------------------------------------------------------------------------------------------
# CMake 2.8.12 and newer has support for using @rpath in a target install name. This was enabled by setting the target
# property MACOSX_RPATH. The @rpath in an install name is a more flexible and powerful mechanism than @executable_path
# or @loader_path for locating shared libraries.
#
# CMake 3.0 and later prefer CMP0042 to be ON by default. Projects wanting @rpath in a target's install name may remove
# any setting of the INSTALL_NAME_DIR and CMAKE_INSTALL_NAME_DIR variables.
#
# CMP0042 was introduced in CMake version 3.0. CMake version 3.0.2 warns when the policy is not set and uses OLD
# behavior.
if(POLICY_CMP0042)
  cmake_policy(SET CMP0042 NEW)
endif()

# CMP0054 causes CMake to only interpret "if()" arguments as variables or keywords when unquoted. CMake 3.1 and above no
# longer implicitly dereference variables or interpret keywords in an if() command argument when it is a Quoted Argument
# or a Bracket Argument. The OLD behavior for this policy is to dereference variables and interpret keywords even if
# they are quoted or bracketed. The NEW behavior is to not dereference variables or interpret keywords that have been
# quoted or bracketed.
if(POLICY CMP0054)
  cmake_policy(SET CMP0054 NEW)
endif()

# In CMake 3.12 and above the find_package(<PackageName>) command now searches prefixes specified by the
# <PackageName>_ROOT CMake variable and the <PackageName>_ROOT environment variable. Package roots are maintained as a
# stack so nested calls to all find_* commands inside find modules also search the roots as prefixes. This policy
# provides compatibility with projects that have not been updated to avoid using <PackageName>_ROOT variables for other
# purposes.  The OLD behavior for this policy is to ignore <PackageName>_ROOT variables. The NEW behavior for this
# policy is to use <PackageName>_ROOT variables.
#
# Behavior is set to "NEW" because ROSE matrix testing, Livermore's LC RZ/CZ resources, Spack, and RMC/Spock seldom
# install ROSE software dependencies in standard locations because they need to support the ability to install multiple
# versions and configurations of the dependencies.
if(POLICY_CMP0074)
  cmake_policy(SET CMP0074 NEW)
endif()

# In CMake 3.27 and above the find_package(<PackageName>) command also searches for upper-case <PACKAGENAME>_ROOT
# variables in addition to the properly-cased <PackageName>_ROOT variables. This allows find_package(Boost) to honor
# the BOOST_ROOT variable (which ROSE sets from the BOOST_HOME environment variable) and find_package(LLVM) to honor
# the LLVM_ROOT variable. The OLD behavior ignores the upper-case variants for compatibility with projects that used
# them for other purposes. The NEW behavior searches both the upper-case and properly-cased variants.
#
# Behavior is set to "NEW" because ROSE explicitly sets BOOST_ROOT and LLVM_ROOT with the intention that
# find_package() should use them to locate these dependencies.
if(POLICY CMP0144)
  cmake_policy(SET CMP0144 NEW)
endif()

# This controls whether the "cmake" command is verbose. It has nothing to do with the verbosity the resulting Makefiles.
option(VERBOSE "CMake should be verbose" FALSE)
if(VERBOSE)
  set(QUIET FALSE)
  set(CMAKE_VERBOSE_MAKEFILE TRUE)
else()
  set(QUIET TRUE)
  set(CMAKE_VERBOSE_MAKEFILE FALSE)
endif()

option(BUILD_SHARED_LIBS "Build all libraries shared" FALSE)

option(use-lib64-paths "Should ROSE be installed in lib64 paths? (Some Linux distributes expect this)" OFF)
if(use-lib64-paths)
  set(ROSE_LIB_DIR_NAME "lib64")
else()
  set(ROSE_LIB_DIR_NAME "lib")
endif()

if(WIN32)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  add_definitions(-DBOOST_ALL_NO_LIB=1)
endif()

# FIXME: Why do we have to have a copy of some standard built-in modules inside rose?
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" "${CMAKE_SOURCE_DIR}/cmake/modules"  ${CMAKE_MODULE_PATH})

# ROSE source and build (binary) directory hierarchies
set(ROSE_TOP_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(ROSE_TOP_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

########################################################################################################################
# ROSE version information
########################################################################################################################
# The ROSE version string was already read from ROSE_VERSION file before the project() command.
# Now read the numerical version from SCM_DATE file.

# All distributions of ROSE also have a config/SCM_DATE file with a numerical version that's updated during the ROSE release process.
set(ROSE_SCM_DATE_FILE "${PROJECT_SOURCE_DIR}/config/SCM_DATE")
if(NOT EXISTS "${ROSE_SCM_DATE_FILE}")
  message(FATAL_ERROR "ROSE SCM Date file not found: ${ROSE_SCM_DATE_FILE}")
endif()

file(STRINGS ${ROSE_SCM_VERSION_FILE} ROSE_SCM_VERSION_ID LIMIT_COUNT 1)
file(READ ${ROSE_SCM_DATE_FILE} ROSE_VERSION)

# Fix trailing whitespace
string(STRIP "${ROSE_VERSION}" ROSE_VERSION)

# Print Results
message(STATUS "The ROSE version integer is ${ROSE_VERSION}")
message(STATUS "The ROSE version string is ${ROSE_PACKAGE_VERSION}")

# Parse version into components for package configuration
# Note: PROJECT_VERSION_MAJOR, PROJECT_VERSION_MINOR, etc. are also set by project() command
string(REGEX MATCH "^([0-9]+)\\.([0-9]+)\\.([0-9]+)\\.([0-9]+)" _version_match "${ROSE_PACKAGE_VERSION}")
set(ROSE_VERSION_MAJOR "${CMAKE_MATCH_1}")
set(ROSE_VERSION_MINOR "${CMAKE_MATCH_2}")
set(ROSE_VERSION_PATCH "${CMAKE_MATCH_3}")
set(ROSE_VERSION_BUILD "${CMAKE_MATCH_4}")

# Generate version file for package configuration
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/cmake/RoseConfigVersion.cmake
  VERSION ${ROSE_PACKAGE_VERSION}
  COMPATIBILITY SameMajorVersion
)

########################################################################################################################
# Initialize the leading part of the Rose::initialize token
########################################################################################################################
# Configuration synopsis used by Rose::initialize.  The ROSE_CONFIG_TOKEN is #define'd as a string in rose_config.h and
# ultimately rosePublicConfig.h and the string is a synopsis of some important configuration details. This string is
# passed by user code to the Rose::initialize function which compares it against the same macro compiled into the ROSE
# library. If their contents differ it means that the ROSE header files being used by the user are not the same as the
# ROSE header files that were used to compile librose being linked by the user and bad things will probably happen.
#
# The CMake ROSE_CONFIG_TOKEN variable is the string that becomes the eventual value of the ROSE_CONFIG_TOKEN C
# preprocessor macro.  We initialize it here, but other parts of the CMake file might augment its value with additional
# information (e.g., adding the boost version number).
set(ROSE_CONFIG_TOKEN "rose-${ROSE_SCM_VERSION_ID}")

########################################################################################################################
# Boost libraries
########################################################################################################################
if(CMAKE_BUILD_TYPE STREQUAL "Debug") 
  message(STATUS
    "If you get a whole bunch of warnings saying 'New Boost version may have incorrect or missing dependencies and "
    "imported targets' it is probably because you're using a Boost version that was released after your CMake version. "
    "See [https://github.com/Kitware/CMake/commits/master/Modules/FindBoost.cmake] to see latest supported version and "
    "[https://github.com/Microsoft/vcpkg/issues/2459] for a description of the problem. Note that CMake versions "
    "3.11.0 through 3.13.2 and possibly later) cannot be compiled (syntax errors) with a GNU C++ compiler that's "
    "configured to use a non-default language standard (e.g., C++11 with GCC-5.4 whose default is GNU++03).")
endif() 

set(Boost_USE_STATIC_LIBS FALSE)
set(Boost_DEBUG ${VERBOSE})
option(BOOST_USE_MULTITHREADED "Should Boost multithreaded libraries be used?" OFF) # puts -pthread on link and compile

# Honor BOOST_HOME environment variable
if(DEFINED ENV{BOOST_HOME})
  set(BOOST_ROOT "$ENV{BOOST_HOME}")
endif()

# Boost 1.47 is no longer supported by ROSE, but is what's installed on Jenkins' CMake test machine. This should
#        be changed to the actual minimum supported version once Pei-Hung upgrades the machine. [Matzke 2019-01-21]
#
# First look for the Boost libraries that are required in order to compile ROSE, then additionally look for any optional
# libraries that are useful to ROSE but not required. From a user: "The find_package() change is required for boost
# 1.70 or above on any system when using a recent enough version of cmake.  Without it, serialization support will not
# be used.  Before boost 1.70, the findBoost() in cmake is used directly, and it will look for boost components that are
# not in the find_package clause.  Since 1.70, cmake will load the findBoost() provided by the boost package itself.
# Unfortunately, that findBoost() will only check for components in the find_package() clause.  This means that
# serialization is considered not to exist, even if it is there.  My change calls find_package again without the
# REQUIRED clause after the first one has succeeded.  That way the first clause checks for requires components, and the
# second can check again including optional components.  If the version of cmake is high enough you can use an
# OPTIONAL_COMPONENTS clause instead, but that wasn't introduced until 3.11."
# Boost.System has been header-only since Boost 1.69 and recent releases (1.90) no longer ship a libboost_system
# at all, so it cannot be a required component. Look for it first, on its own (every find_package(Boost) call
# resets Boost_LIBRARIES to the components of that call, so this must precede the call that defines the real
# library list), and link Boost::system only where the target exists (older Boost).
find_package(Boost QUIET COMPONENTS system)
if(TARGET Boost::system)
  set(ROSE_BOOST_SYSTEM_LINK Boost::system)
else()
  set(ROSE_BOOST_SYSTEM_LINK "")
endif()
find_package(Boost REQUIRED COMPONENTS chrono date_time filesystem iostreams program_options random regex wave thread serialization)
set(Boost_SYSTEM_FOUND TRUE)   # header-only or linked above: the functionality is present either way
if(WIN32) 
  set(BOOST_LIBRARYDIR ${Boost_LIBRARY_DIRS})
  set(BOOST_INCLUDEDIR ${Boost_INCLUDE_DIRS}/)
endif() 

include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

# Defines for rose_config.h  
set(ROSE_CONFIG_TOKEN "${ROSE_CONFIG_TOKEN} boost-${Boost_VERSION}")

# Boost version-specific macros
if(Boost_VERSION VERSION_GREATER_EQUAL "1.78.0") 
  # Resolves compiler warning about Boost placeholders (_1, _2, etc.) in the global namespace, which is deprecate
  # Boost now recommends including the <boost/bind/bind.hpp> header and using the boost::placeholders namespace instead
  # To resolve this warning, we can use the line of CMake below or Update the code to use the recommended namespace
  # To update the code,  Modify Sawyer codes to include <boost/bind/bind.hpp> and use boost::placeholders
  add_definitions(-DBOOST_BIND_GLOBAL_PLACEHOLDERS) 
endif()

# Boost compiler-specific flags
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang") 
  # Do not supress boost warnings in debug mode  
  if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug") 
    # Compiler specific commands, not to be used with MSVC 
    add_compile_options(-Wno-switch-default) # supress warnings coming from the boost library code itself 
    add_compile_options(-Wno-deprecated)     # supress warnings coming from the boost library code itself 
  endif() 
endif() 

# If the boost is compiled with multi-thread support then we must add "-pthread" to all compile and link commands, or
# whatever is appropriate for the system.  With GCC it is not sufficient to just add "-lpthread" to the link
# commands--the "-pthread" switch MUST be added to the compile commands as well.
# FIXME: Do we want to use our option(BOOST_USE_MULTITHREADED) or always do this?
if(Boost_THREAD_FOUND)
  set(Threads_FIND_QUIETLY ${QUIET})
  find_package(Threads REQUIRED)
  if(Threads_FOUND)
    message(STATUS "Threads found: CMAKE_THREAD_LIBS_INIT=${CMAKE_THREAD_LIBS_INIT}")
  endif()
  # FIXME: This breaks things
  # if(Threads_FOUND)
  #   if(BOOST_USE_MULTITHREADED) # Something inside this conditional gets OpenCL
  #     message(WARNING "Setting _REENTRANT")
  #     set(_REENTRANT 1)
  #  endif()
  # NOTE: -pthread is now added via Threads::Threads target in src/CMakeLists.txt (PUBLIC dependency)
  # This ensures proper propagation to external projects via RoseConfig.cmake
  add_definitions(-pthread) # Keep for internal ROSE builds (non-target-based code)
  set(ROSE_CONFIG_TOKEN "${ROSE_CONFIG_TOKEN} pthread")
endif()

# Boost misconfiguration is often the cause of many issues
message(STATUS "Boost_VERSION: ${Boost_VERSION}" )           # Report Boost version in all modes  
message(STATUS "Boost_INCLUDE_DIRS:  ${Boost_INCLUDE_DIRS}") # Report Boost -I in all modes  
message(STATUS "Boost_LIBRARY_DIRS:  ${Boost_LIBRARY_DIRS}") # Report Boost -L in all modes  

# Directories where Rose executable, rose library, and rose headers will be installed 
set(INCLUDE_INSTALL_DIR "include/rose")
set(BIN_INSTALL_DIR     "bin")
set(LIB_INSTALL_DIR     "${ROSE_LIB_DIR_NAME}")

set(INSTALL_TARGETS_DEFAULT_ARGS
    RUNTIME DESTINATION "${BIN_INSTALL_DIR}"
    LIBRARY DESTINATION "${LIB_INSTALL_DIR}"
    ARCHIVE DESTINATION "${LIB_INSTALL_DIR}"
    COMPONENT Devel)

# A new definition to tweak code for cmake
set(USE_CMAKE ON CACHE BOOL "Define if using CMake")

# ROSE configuration variables for Boost
set(HAVE_BOOST ${Boost_FOUND})
set(HAVE_BOOST_SERIALIZATION_LIB ${Boost_SERIALIZATION_FOUND})
set(HAVE_BOOST_DATE_TIME ${Boost_DATE_TIME_FOUND})
set(HAVE_BOOST_FILESYSTEM ${Boost_FILESYSTEM_FOUND})
set(HAVE_BOOST_PROGRAM_OPTIONS ${Boost_PROGRAM_OPTIONS_FOUND})
set(HAVE_BOOST_REGEX ${Boost_REGEX_FOUND})
set(HAVE_BOOST_SYSTEM ${Boost_SYSTEM_FOUND})
set(HAVE_BOOST_THREAD ${Boost_THREAD_FOUND})
set(HAVE_BOOST_WAVE ${Boost_WAVE_FOUND})
set(USE_ROSE_BOOST_WAVE_SUPPORT ${Boost_WAVE_FOUND})

########################################################################################################################
# Operating system information
########################################################################################################################

# Test whether the host OS (the OS compiling ROSE) is Red Hat Enterprise Linux, and if so, set ROSE_HOST_OS_IS_RHEL to
#the RHEL major version number.
set(ROSE_HOST_OS_IS_RHEL "")
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
  execute_process(
    COMMAND bash -c ". /etc/os-release && echo -n $ID"
    OUTPUT_VARIABLE ROSE_HOST_OS_ID)
  if ("${ROSE_HOST_OS_ID}" MATCHES "rhel")
    execute_process(
      COMMAND bash -c ". /etc/os-release && echo -n \${VERSION_ID%%.*}"
      OUTPUT_VARIABLE ROSE_HOST_OS_VERSION_MAJOR)
    set(ROSE_HOST_OS_IS_RHEL "${ROSE_HOST_OS_VERSION_MAJOR}")
  endif()
endif()

if (ROSE_HOST_OS_IS_RHEL EQUAL 8 AND NOT DEFINED ROSE_ENABLE_BOOST_SERIALIZATION)
  message(WARNING "Boost serialization takes a long time to assemble on RHEL8, disable using -DROSE_ENABLE_BOOST_SERIALIZATION=OFF (mute using =ON)")
endif()

option(ROSE_ENABLE_BOOST_SERIALIZATION "Enable Boost serialization" ON)

########################################################################################################################
# Compiler toolchain features
########################################################################################################################

if(MSVC)
  set(CMAKE_BUILD_TYPE Release)
  # Compiler flags
  # /TP to indicate files are C++
  # / CLR common intermediate language
  # /GL whole program optimization
  # /O1 optimization for small files
  # /Ob0 disable inline expansion
  # /MP multiple processors compilation
  # /0s small files
  # /wd4716 to turn no return to a warning and not an error
  # /w -- suppresses all compiler warnings
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /TP /MP /O1 /Os /GR /EHsc /wd4541 /wd4716 /bigobj /w")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}   /wd4541 ")
  # Linker flags
  # /IGNORE:4217 - suppress "symbol 'A' defined in 'B' is imported by 'C' in function 'D'"
  # /IGNORE:4286 - suppress "symbol 'A' defined in 'B' is imported by 'C'
  set(WINDOWS_EXTRA_LINKER_FLAGS "/INCREMENTAL:NO /IGNORE:4217,4286")
  set(CMAKE_SHARED_LINKER_FLAGS_DEBUG " ${CMAKE_SHARED_LINKER_FLAGS_DEBUG} ${WINDOWS_EXTRA_LINKER_FLAGS}")
  set(CMAKE_SHARED_LINKER_FLAGS_RELEASE " ${CMAKE_SHARED_LINKER_FLAGS_RELEASE} ${WINDOWS_EXTRA_LINKER_FLAGS}")
  set(CMAKE_MODULE_LINKER_FLAGS_DEBUG " ${CMAKE_MODULE_LINKER_FLAGS_DEBUG} ${WINDOWS_EXTRA_LINKER_FLAGS}")
  set(CMAKE_MODULE_LINKER_FLAGS_RELEASE " ${CMAKE_MODULE_LINKER_FLAGS_RELEASE} ${WINDOWS_EXTRA_LINKER_FLAGS}")
  set(CMAKE_LINKER_FLAGS " ${CMAKE_LINKER_FLAGS} ${WINDOWS_EXTRA_LINKER_FLAGS}")
  set(CMAKE_SHARED_LINKER_FLAGS " ${CMAKE_SHARED_LINKER_FLAGS} ${WINDOWS_EXTRA_LINKER_FLAGS}")
  set(CMAKE_MODULE_LINKER_FLAGS " ${CMAKE_MODULE_LINKER_FLAGS} ${WINDOWS_EXTRA_LINKER_FLAGS}")
  set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} ${WINDOWS_EXTRA_LINKER_FLAGS}")
  set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} ${WINDOWS_EXTRA_LINKER_FLAGS}")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${WINDOWS_EXTRA_LINKER_FLAGS}")

else()

  if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-w)
  endif()

  # Ensure C++11 ABI compatibility
  if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GLIBCXX_USE_CXX11_ABI=1")
  endif()

  if(CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_GLIBCXX_USE_CXX11_ABI=1")
  endif()
endif()

# Set ROSE_SHLIBPATH_VAR.  For Visual Studio, this is PATH.  Otherwise, it is just LD_LIBRARY_PATH.
if(MSVC)
  set(ROSE_SHLIBPATH_VAR "PATH")
  add_definitions("-D__STDC_LIMIT_MACROS")
else()
  set(ROSE_SHLIBPATH_VAR "LD_LIBRARY_PATH")
endif()

########################################################################################################################
# Analyzable languages supported by ROSE
########################################################################################################################
# NOTE: We are migrating from hyphenated option names (ENABLE-C) to underscore names (ENABLE_C)
#       to follow CMake best practices. Both forms work during the transition period, but hyphenated
#       names will trigger deprecation warnings and will be removed in a future release.

# Binary analysis
option(ENABLE_BINARY_ANALYSIS "Enable binary analysis" OFF)
if(DEFINED CACHE{ENABLE-BINARY-ANALYSIS})
  message(DEPRECATION "ENABLE-BINARY-ANALYSIS is deprecated. Use ENABLE_BINARY_ANALYSIS instead. Example: cmake -DENABLE_BINARY_ANALYSIS=${ENABLE-BINARY-ANALYSIS} ...")
  set(ENABLE_BINARY_ANALYSIS ${ENABLE-BINARY-ANALYSIS} CACHE BOOL "Enable binary analysis" FORCE)
endif()
if(ENABLE_BINARY_ANALYSIS)
  set(ROSE_BUILD_BINARY_ANALYSIS_SUPPORT 1)
endif()

# C/C++
option(ENABLE_C "Enable C/C++ analysis" OFF)
if(DEFINED CACHE{ENABLE-C})
  message(DEPRECATION "ENABLE-C is deprecated. Use ENABLE_C instead. Example: cmake -DENABLE_C=${ENABLE-C} ...")
  set(ENABLE_C ${ENABLE-C} CACHE BOOL "Enable C/C++ analysis" FORCE)
endif()
if(ENABLE_C)
  set(ROSE_BUILD_CXX_LANGUAGE_SUPPORT 1)
  set(ROSE_BUILD_C_LANGUAGE_SUPPORT 1)
endif()

# CUDA
option(ENABLE_CUDA "Enable CUDA analysis" OFF) # OFF because lack of CUDA is not handled properly
if(DEFINED CACHE{ENABLE-CUDA})
  message(DEPRECATION "ENABLE-CUDA is deprecated. Use ENABLE_CUDA instead. Example: cmake -DENABLE_CUDA=${ENABLE-CUDA} ...")
  set(ENABLE_CUDA ${ENABLE-CUDA} CACHE BOOL "Enable CUDA analysis" FORCE)
endif()
if(ENABLE_CUDA)
  if(APPLE)
    message(FATAL_ERROR "CUDA analysis (ENABLE_CUDA) is not supported on macOS")
  endif()
  find_package(CUDA REQUIRED)
  set(ROSE_BUILD_CUDA_LANGUAGE_SUPPORT 1)
endif()

# Java
option(ENABLE_JAVA "Enable Java analysis" OFF)
if(DEFINED CACHE{ENABLE-JAVA})
  message(DEPRECATION "ENABLE-JAVA is deprecated. Use ENABLE_JAVA instead. Example: cmake -DENABLE_JAVA=${ENABLE-JAVA} ...")
  set(ENABLE_JAVA ${ENABLE-JAVA} CACHE BOOL "Enable Java analysis" FORCE)
endif()
if(ENABLE_JAVA)
  message(STATUS "Looking for JAVA ...")
  if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR VERBOSE)
    message(STATUS "JAVA_ROOT is: ${JAVA_ROOT}")
  endif()
  find_package(Java COMPONENTS Development)
  # Set JAVA_RUNTIME for rose_config.h (defines JAVA_JVM_PATH and enables USE_ROSE_JAVA_SUPPORT)
  set(JAVA_RUNTIME "${Java_JAVA_EXECUTABLE}")
  find_program(GCJ gcj)
  find_program(GCJH gcjh)
  find_package(JNI)
  set(ROSE_BUILD_JAVA_LANGUAGE_SUPPORT 1)
  set(USE_ROSE_INTERNAL_JAVA_SUPPORT 1)
  get_filename_component(BACKEND_JAVA_COMPILER ${Java_JAVAC_EXECUTABLE} NAME)
endif()

# OpenCL
option(ENABLE_OPENCL "Enable OpenCL analysis" OFF)
if(DEFINED CACHE{ENABLE-OPENCL})
  message(DEPRECATION "ENABLE-OPENCL is deprecated. Use ENABLE_OPENCL instead. Example: cmake -DENABLE_OPENCL=${ENABLE-OPENCL} ...")
  set(ENABLE_OPENCL ${ENABLE-OPENCL} CACHE BOOL "Enable OpenCL analysis" FORCE)
endif()
if(ENABLE_OPENCL)
  find_package(OpenCL)
  set(ROSE_BUILD_OPENCL_LANGUAGE_SUPPORT 1)
  find_path(with-opencl-inc NAMES cl.h DOC "For OpenCL runtime library")
  find_library(with-opencl-lib OpenCL DOC "OpenCL library for runtime examples")
endif()

# Fortran
option(ENABLE_FORTRAN "Enable Fortran analysis." OFF) # OFF because lack of OFP is not handled properly
if(DEFINED CACHE{ENABLE-FORTRAN})
  message(DEPRECATION "ENABLE-FORTRAN is deprecated. Use ENABLE_FORTRAN instead. Example: cmake -DENABLE_FORTRAN=${ENABLE-FORTRAN} ...")
  set(ENABLE_FORTRAN ${ENABLE-FORTRAN} CACHE BOOL "Enable Fortran analysis." FORCE)
endif()
if(ENABLE_FORTRAN)
  if(NOT ENABLE_JAVA)
    message(FATAL_ERROR "Fortran analysis also requires Java analysis.  Either
    turn on ENABLE_JAVA, or turn off ENABLE_FORTRAN")
  endif()
  set(ROSE_BUILD_FORTRAN_LANGUAGE_SUPPORT 1)
  enable_language(Fortran)

  # check if gfortran (gnu) was found
  if(CMAKE_COMPILER_IS_GNUG77 OR CMAKE_Fortran_COMPILER MATCHES "gfortran")
    set(USE_GFORTRAN_IN_ROSE 1)
    set(BACKEND_FORTRAN_IS_GNU_COMPILER 1)
  endif() 
  
  # query fortran compiler version
  execute_process(COMMAND ${CMAKE_Fortran_COMPILER} --version
                  OUTPUT_VARIABLE Fortran_COMPILER_VERSION_OUTPUT
                  OUTPUT_STRIP_TRAILING_WHITESPACE) 
  string(REGEX MATCH "([0-9]+\\.[0-9]+\\.[0-9]+)"
         Fortran_COMPILER_VERSION "${Fortran_COMPILER_VERSION_OUTPUT}")
  if(Fortran_COMPILER_VERSION)
    message(STATUS "Fortran compiler version detected:  ${Fortran_COMPILER_VERSION}")
  else() 
    message(WARNING "Failed to detect Fortran compiler version")
  endif()
  
  # This is a cache string var assuming we sometimes may expect user to set  
  set(enable-ofp-version "20200819-JDK8" CACHE STRING "version number for OFP")
  
  # Set rose_config.h macros
  set(ROSE_OFP_VERSION_NUMBER "${enable-ofp-version}")
 
  # This is trying to set an environment variable -- is this what we want to do?  
  set($ENV{CLASSPATH} ${CMAKE_SOURCE_DIR}/src/3rdPartyLibraries/antlr-jars/antlr-3.5.2-complete.jar::${CMAKE_SOURCE_DIR}/rc/3rdPartyLibraries/fortran-parser/OpenFortranParser-${enable-ofp-version}.jar)
endif()

# PHP
option(ENABLE_PHP "Enable PHP analysis" OFF)
if(DEFINED CACHE{ENABLE-PHP})
  message(DEPRECATION "ENABLE-PHP is deprecated. Use ENABLE_PHP instead. Example: cmake -DENABLE_PHP=${ENABLE-PHP} ...")
  set(ENABLE_PHP ${ENABLE-PHP} CACHE BOOL "Enable PHP analysis" FORCE)
endif()
if(ENABLE_PHP)
  set(ROSE_BUILD_PHP_LANGUAGE_SUPPORT 1)
  find_path(with-php php DOC "Specify the prefix where PHP (and phc) is installed")
endif()

# Python
option(ENABLE_PYTHON "Enable Python analysis" OFF)
if(DEFINED CACHE{ENABLE-PYTHON})
  message(DEPRECATION "ENABLE-PYTHON is deprecated. Use ENABLE_PYTHON instead. Example: cmake -DENABLE_PYTHON=${ENABLE-PYTHON} ...")
  set(ENABLE_PYTHON ${ENABLE-PYTHON} CACHE BOOL "Enable Python analysis" FORCE)
endif()
if(ENABLE_PYTHON)
  set(ROSE_BUILD_PYTHON_LANGUAGE_SUPPORT 1)
  find_package(PythonLibs REQUIRED)
  option(WITH-PYTHON "Build code that requires a Python interpreter" ON)
  if(WITH-PYTHON)
    find_package(PythonInterp)
  endif()
endif()

# Ada
option(ENABLE_ADA "Enable Ada analysis" OFF)
if(DEFINED CACHE{ENABLE-ADA})
  message(DEPRECATION "ENABLE-ADA is deprecated. Use ENABLE_ADA instead. Example: cmake -DENABLE_ADA=${ENABLE-ADA} ...")
  set(ENABLE_ADA ${ENABLE-ADA} CACHE BOOL "Enable Ada analysis" FORCE)
endif()
if(ENABLE_ADA)
  set(ROSE_EXPERIMENTAL_ADA_ROSE_CONNECTION_GNAT_HOME "$ENV{GNAT_HOME}")
  set(ROSE_EXPERIMENTAL_ADA_ROSE_CONNECTION ON)
endif()

# Libadalang
# I think this name is terrible, considering the autotools equivalent is --enable-experimental_libadalang_frontend, but it follows the established pattern
option(ENABLE_LIBADALANG "Enable experimental libadalang frontend" OFF)
if(DEFINED CACHE{ENABLE-LIBADALANG})
  message(DEPRECATION "ENABLE-LIBADALANG is deprecated. Use ENABLE_LIBADALANG instead. Example: cmake -DENABLE_LIBADALANG=${ENABLE-LIBADALANG} ...")
  set(ENABLE_LIBADALANG ${ENABLE-LIBADALANG} CACHE BOOL "Enable experimental libadalang frontend" FORCE)
endif()
if(ENABLE_LIBADALANG)
  set(ROSE_EXPERIMENTAL_LIBADALANG_ROSE_CONNECTION_GNAT_HOME "$ENV{GNAT_HOME}")
  set(ROSE_EXPERIMENTAL_LIBADALANG_ROSE_CONNECTION ON)
endif()

if(ENABLE_ADA OR ENABLE_LIBADALANG)
  message(STATUS "Checking the environment variables GNAT_HOME and BOOST_HOME have been set.") 
  
  # Force user to define GNAT_HOME 
  if(NOT DEFINED ENV{GNAT_HOME}) 
    message(FATAL_ERROR "Ada support requires the environment variable GNAT_HOME to be set to an installed GNAT") 
  endif() 

  # Force user to define GNAT_HOME as a directory that exists on the system or within the container 
  if(NOT IS_DIRECTORY "$ENV{GNAT_HOME}") 
    message(WARNING "GNAT_HOME: $ENV{GNAT_HOME}")
    message(FATAL_ERROR "GNAT_HOME does not exist on this system") 
  endif() 

  # Force user to define BOOST_HOME
  if(NOT DEFINED ENV{BOOST_HOME}) 
    message(FATAL_ERROR "Ada support requires the environment variable BOOST_HOME to be set to an installed boost")
  endif() 

  # Force user to define BOOST_HOME as a directory that exists on the system or within the container 
  if(NOT IS_DIRECTORY "$ENV{BOOST_HOME}") 
    message(WARNING "BOOST_HOME: $ENV{BOOST_HOME}")
    message(FATAL_ERROR "BOOST_HOME does not exist on this system") 
  endif()    

  # Check PATH and LD_LIBRARY_PATH
  get_filename_component(USER_PATH $ENV{PATH} ABSOLUTE)               # retrieve USER_PATH  
  get_filename_component(USER_LD_PATH $ENV{LD_LIBRARY_PATH} ABSOLUTE) # retrieve USER_LD_PATH
  
  # Ensure user has GNAT_HOME/bin in their path 
  string(FIND "${USER_PATH}" "$ENV{GNAT_HOME}/bin" found_gnat_in_path) 
  if(found_gnat_in_path EQUAL -1) 
    message(WARNING "GNAT_HOME/bin was not found in users path: ${USER_PATH}") 
  else() 
    message(STATUS "Found $ENV{GNAT_HOME}/bin")
  endif()

  # Ensure user has GNAT_HOME/lib in their LD path 
  string(FIND "${USER_LD_PATH}" "$ENV{GNAT_HOME}/lib" found_gnat_in_ld_path) 
     
  if(found_gnat_in_ld_path EQUAL -1) 
    message(WARNING "{GNAT_HOME}/lib was not found in users LD_LIBRARY_PATH: ${USER_LD_PATH}") 
  else() 
    message(STATUS "Found $ENV{GNAT_HOME}/lib")
  endif()

  # Ensure user has GNAT_HOME/lib in their LD path 
  string(FIND "${USER_LD_PATH}" "${BOOST_HOME}/lib" found_boost_in_ld_path) 
  if(found_boost_in_ld_path EQUAL -1) 
    message(WARNING "BOOST_HOME not found in users LD_LIBRARY_PATH: ${USER_LD_PATH}") 
  else() 
    message(STATUS "Found $ENV{BOOST_HOME}/lib")
  endif()

  set(BACKEND_ADA_COMPILER "gnat" CACHE STRING "Specify backend Ada compiler, default is gnat")
  set(BACKEND_ADA_COMPILER_NAME_WITH_PATH ${BACKEND_ADA_COMPILER})
endif()

# Jovial
option(ENABLE_JOVIAL "Enable Jovial analysis" OFF)
if(DEFINED CACHE{ENABLE-JOVIAL})
  message(DEPRECATION "ENABLE-JOVIAL is deprecated. Use ENABLE_JOVIAL instead. Example: cmake -DENABLE_JOVIAL=${ENABLE-JOVIAL} ...")
  set(ENABLE_JOVIAL ${ENABLE-JOVIAL} CACHE BOOL "Enable Jovial analysis" FORCE)
endif()
if(ENABLE_JOVIAL)
  set(ROSE_BUILD_JOVIAL_LANGUAGE_SUPPORT 1)
endif()

# Null analysis (in order to configure ROSE with no actual analysis since at least one is required)
option(ENABLE_NULL "Enable null analysis (empty analysis that does nothing)" OFF)
if(ENABLE_NULL)
  set(ROSE_BUILD_NULL_ANALYSIS 1)
endif()

# Check that at least one analysis is enabled
if(NOT ENABLE_BINARY_ANALYSIS AND NOT ENABLE_C AND NOT ENABLE_CUDA AND NOT ENABLE_JAVA AND
   NOT ENABLE_OPENCL AND NOT ENABLE_FORTRAN AND NOT ENABLE_PHP AND NOT ENABLE_PYTHON AND
   NOT ENABLE_ADA AND NOT ENABLE_JOVIAL AND NOT ENABLE_LIBADALANG AND NOT ENABLE_NULL)
  message(FATAL_ERROR
    "ROSE must be configured with at least one analysis feature enabled. The following "
    "are the most common features, but see the ROSE documentation for the extensive, complete "
    "list."
    "\n"
    "  C/C++ Analysis:\n"
    "    cmake -DENABLE_C=ON ...\n"
    "    Enables analysis of C and C++ code using the EDG frontend. Requires EDG and Boost.\n"
    "    Best supported on RHEL with GCC. Not portable to all platforms.\n"
    "  Binary Analysis:\n"
    "    cmake -DENABLE_BINARY_ANALYSIS=ON ...\n"
    "    Enables analysis of binary executables and object files. Requires only Boost.\n"
    "    Portable and well-supported on RHEL and Debian-based systems.\n"
    "  Fortran Analysis:\n"
    "    cmake -DENABLE_FORTRAN=ON -DENABLE_JAVA=ON ...\n"
    "    Enables analysis of Fortran code. Requires Java analysis to be enabled.\n"
    "  Ada Analysis:\n"
    "    cmake -DENABLE_ADA=ON ...\n"
    "    Enables analysis of Ada code. Experimental feature.\n"
    "  Java Analysis:\n"
    "    cmake -DENABLE_JAVA=ON ...\n"
    "    Enables analysis of Java code.\n"
    "  Null Analysis (no actual analysis):\n"
    "    cmake -DENABLE_NULL=ON ...\n"
    "    Empty analysis in order to configure ROSE without any true analysis capabilities."
  )
endif()

# The C preprocessor is enabled automatically if C, C++, or Fortran is enabled
if(ENABLE_C OR ENABLE_FORTRAN)
  set(ENABLE_CPP ON)
  set(ROSE_BUILD_CPP_LANGUAGE_SUPPORT 1)
endif()

########################################################################################################################
# EDG (Edison Design Group) frontend C/C++ compiler
########################################################################################################################
option(EDG_COMPILE "Compile EDG source code if available" ON)

# Clang or EDG is required for frontend
option(ENABLE_CLANG_FRONTEND "Enable Clang frontend" OFF)
if(DEFINED CACHE{ENABLE-CLANG-FRONTEND})
  message(DEPRECATION "ENABLE-CLANG-FRONTEND is deprecated. Use ENABLE_CLANG_FRONTEND instead. Example: cmake -DENABLE_CLANG_FRONTEND=${ENABLE-CLANG-FRONTEND} ...")
  set(ENABLE_CLANG_FRONTEND ${ENABLE-CLANG-FRONTEND} CACHE BOOL "Enable Clang frontend" FORCE)
endif()
if(ENABLE_CLANG_FRONTEND)
  # Define macro which is used by source code
  set(ROSE_USE_CLANG_FRONTEND 1)
  set(EDG_COMPILE OFF)
  # Honor LLVM_HOME environment variable if it exists
  if(DEFINED ENV{LLVM_HOME})
    set(LLVM_ROOT "$ENV{LLVM_HOME}")
    set(Clang_ROOT "$ENV{LLVM_HOME}")
  endif()
  find_package(LLVM REQUIRED)
  find_package(Clang REQUIRED)
  # The embedded frontend creates CompilerInvocation directly and therefore
  # does not receive the Clang driver's builtin-header search path.
  execute_process(
    COMMAND "${LLVM_TOOLS_BINARY_DIR}/clang" -print-resource-dir
    OUTPUT_VARIABLE ROSE_CLANG_RESOURCE_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE ROSE_CLANG_RESOURCE_DIR_RESULT)
  if(ROSE_CLANG_RESOURCE_DIR_RESULT EQUAL 0 AND NOT ROSE_CLANG_RESOURCE_DIR STREQUAL "")
    set(ROSE_CLANG_RESOURCE_INCLUDE_DIR "${ROSE_CLANG_RESOURCE_DIR}/include")
    message(STATUS "Using Clang resource headers: ${ROSE_CLANG_RESOURCE_INCLUDE_DIR}")
  else()
    message(WARNING "Unable to determine the Clang resource directory from ${LLVM_TOOLS_BINARY_DIR}/clang")
  endif()
  # find_package may stomp on CLANG_VERSION_MAJOR, so it must be after. At the moment, the package
  # sets CLANG_VERSION_MAJOR to 0, but they may correct it in the future, which would make the
  # script unnecessary
  # also, assuming that you must use clang to compile the clang frontend,
  # we could perhaps use BACKEND_CXX_COMPILER_MAJOR_VERSION_NUMBER instead of this script
  execute_process(COMMAND "${CMAKE_SOURCE_DIR}/config/getClangMajorVersionNumber.sh" OUTPUT_VARIABLE CLANG_VERSION_MAJOR)
endif()

# EDG version, read from the git submodule src/frontend/CxxFrontend/EDG/edg_compiler (a public fork of
# github.com/edgcpp/compiler carrying ROSE's patches). EDG is always compiled from that source; there is no
# precompiled EDG library any more.
set(edg_source_dir "${PROJECT_SOURCE_DIR}/src/frontend/CxxFrontend/EDG/edg_compiler")
set(EDG_VERSION "" CACHE STRING "major.minor version of EDG; read from the edg_compiler submodule, do not set.")
if(EXISTS "${edg_source_dir}/src/version.h")
  file(STRINGS "${edg_source_dir}/src/version.h" edg_version_line REGEX "^#define VERSION_NUMBER[ \t]+\"")
  string(REGEX REPLACE ".*\"([0-9]+\\.[0-9]+)[^\"]*\".*" "\\1" edg_version_from_source "${edg_version_line}")
  if(NOT edg_version_from_source STREQUAL EDG_VERSION)
    set(EDG_VERSION "${edg_version_from_source}" CACHE STRING "major.minor version of EDG; read from the edg_compiler submodule." FORCE)
  endif()
endif()
string(REGEX REPLACE "^([0-9]+)\\..*$" "\\1" EDG_MAJOR_VERSION "${EDG_VERSION}")
string(REGEX REPLACE "^[0-9]+\\.([0-9]+).*$" "\\1" EDG_MINOR_VERSION "${EDG_VERSION}")

# Do we have the EDG source? It is required whenever C/C++ analysis is enabled and the Clang frontend is not.
if(EXISTS "${edg_source_dir}/src/basics.h" AND EDG_COMPILE)
  set(have_EDG_source TRUE)
  message(STATUS "EDG - will compile EDG-${EDG_VERSION} source code from src/frontend/CxxFrontend/EDG/edg_compiler")
else()
  set(have_EDG_source FALSE)
  if(ENABLE_C AND NOT ENABLE_CLANG_FRONTEND)
    message(FATAL_ERROR
      "C/C++ analysis needs the EDG front end, but its source is not present in\n"
      "  ${edg_source_dir}\n"
      "Check out the submodule:  git submodule update --init src/frontend/CxxFrontend/EDG/edg_compiler\n"
      "or configure without C/C++ support, or with -DENABLE_CLANG_FRONTEND=ON.")
  endif()
  message(STATUS "EDG - not needed")
endif()

########################################################################################################################
# System features
########################################################################################################################
# A set of common features including endian, stdio.h, printf, size of long int, etc.
set(CMAKE_REQUIRED_QUIET ${QUIET})
include(ConfigureChecks)
set(CMAKE_REQUIRED_QUIET FALSE)

find_package(Perl)

########################################################################################################################
# Miscellaneous user-selectable features
########################################################################################################################

option(ENABLE_EXAMPLE_TRANSLATORS_DIRECTORY "Enable compilation and testing of exampleTranslators directory" OFF)
if(DEFINED CACHE{ENABLE-EXAMPLE-TRANSLATORS-DIRECTORY})
  message(DEPRECATION "ENABLE-EXAMPLE-TRANSLATORS-DIRECTORY is deprecated. Use ENABLE_EXAMPLE_TRANSLATORS_DIRECTORY instead. Example: cmake -DENABLE_EXAMPLE_TRANSLATORS_DIRECTORY=${ENABLE-EXAMPLE-TRANSLATORS-DIRECTORY} ...")
  set(ENABLE_EXAMPLE_TRANSLATORS_DIRECTORY ${ENABLE-EXAMPLE-TRANSLATORS-DIRECTORY} CACHE BOOL "Enable compilation and testing of exampleTranslators directory" FORCE)
endif()

# 1/8/25 we will most likely remove this enable-tests-directory option
option(ENABLE_TESTS_DIRECTORY "Enable compilation and testing of the ROSE/tests directory" OFF)
if(DEFINED CACHE{ENABLE-TESTS-DIRECTORY})
  message(DEPRECATION "ENABLE-TESTS-DIRECTORY is deprecated. Use ENABLE_TESTS_DIRECTORY instead. Example: cmake -DENABLE_TESTS_DIRECTORY=${ENABLE-TESTS-DIRECTORY} ...")
  set(ENABLE_TESTS_DIRECTORY ${ENABLE-TESTS-DIRECTORY} CACHE BOOL "Enable compilation and testing of the ROSE/tests directory" FORCE)
endif()

# 1/15/24 Turn on ctests until new CMake-Build-RHEL7.sh is merged
if(NOT ENABLE_TESTS_DIRECTORY AND EXISTS "${PROJECT_SOURCE_DIR}/tests/README.md")
  set(ENABLE_TESTS_DIRECTORY ON CACHE BOOL "Enable compilation and testing of the ROSE/tests directory" FORCE)
endif()

option(ENABLE_TUTORIAL_DIRECTORY "Enable compilation and testing of the ROSE/tutorial directory" OFF)
if(DEFINED CACHE{ENABLE-TUTORIAL-DIRECTORY})
  message(DEPRECATION "ENABLE-TUTORIAL-DIRECTORY is deprecated. Use ENABLE_TUTORIAL_DIRECTORY instead. Example: cmake -DENABLE_TUTORIAL_DIRECTORY=${ENABLE-TUTORIAL-DIRECTORY} ...")
  set(ENABLE_TUTORIAL_DIRECTORY ${ENABLE-TUTORIAL-DIRECTORY} CACHE BOOL "Enable compilation and testing of the ROSE/tutorial directory" FORCE)
endif()

option(ENABLE_ADVANCED_WARNINGS "Support for an advanced uniform warning level for ROSE development" OFF)
if(DEFINED CACHE{ENABLE-ADVANCED-WARNINGS})
  message(DEPRECATION "ENABLE-ADVANCED-WARNINGS is deprecated. Use ENABLE_ADVANCED_WARNINGS instead. Example: cmake -DENABLE_ADVANCED_WARNINGS=${ENABLE-ADVANCED-WARNINGS} ...")
  set(ENABLE_ADVANCED_WARNINGS ${ENABLE-ADVANCED-WARNINGS} CACHE BOOL "Support for an advanced uniform warning level for ROSE development" FORCE)
endif()

if(ENABLE_ADVANCED_WARNINGS)
  message(WARNING "Using an advanced uniform warning level for ROSE development")
  set(ROSE_USE_UNIFORM_ADVANCED_WARNINGS_SUPPORT 1)
endif()

option(ENABLE_ASSEMBLY_SEMANTICS "Enable semantics-based analysis of assembly code" OFF)
if(DEFINED CACHE{ENABLE-ASSEMBLY-SEMANTICS})
  message(DEPRECATION "ENABLE-ASSEMBLY-SEMANTICS is deprecated. Use ENABLE_ASSEMBLY_SEMANTICS instead. Example: cmake -DENABLE_ASSEMBLY_SEMANTICS=${ENABLE-ASSEMBLY-SEMANTICS} ...")
  set(ENABLE_ASSEMBLY_SEMANTICS ${ENABLE-ASSEMBLY-SEMANTICS} CACHE BOOL "Enable semantics-based analysis of assembly code" FORCE)
endif()

option(ENABLE_CANDL "Support for ScopLib" OFF)
if(DEFINED CACHE{ENABLE-CANDL})
  message(DEPRECATION "ENABLE-CANDL is deprecated. Use ENABLE_CANDL instead. Example: cmake -DENABLE_CANDL=${ENABLE-CANDL} ...")
  set(ENABLE_CANDL ${ENABLE-CANDL} CACHE BOOL "Support for ScopLib" FORCE)
endif()
if(ENABLE_CANDL)
  find_path(with-candl "include/candl.h"
    DOC "Path to a valid Candl installation")
endif()

option(ENABLE_CLOOG "Support for Cloog" OFF)
if(DEFINED CACHE{ENABLE-CLOOG})
  message(DEPRECATION "ENABLE-CLOOG is deprecated. Use ENABLE_CLOOG instead. Example: cmake -DENABLE_CLOOG=${ENABLE-CLOOG} ...")
  set(ENABLE_CLOOG ${ENABLE-CLOOG} CACHE BOOL "Support for Cloog" FORCE)
endif()
if(ENABLE_CLOOG)
  find_path(with-cloog "include/cloog.h"
    DOC "Path to a valid Cloog installation")
endif()

option(ENABLE_COMPASS2 "build the Compass2 static analysis tool" OFF)
if(DEFINED CACHE{ENABLE-COMPASS2})
  message(DEPRECATION "ENABLE-COMPASS2 is deprecated. Use ENABLE_COMPASS2 instead. Example: cmake -DENABLE_COMPASS2=${ENABLE-COMPASS2} ...")
  set(ENABLE_COMPASS2 ${ENABLE-COMPASS2} CACHE BOOL "build the Compass2 static analysis tool" FORCE)
endif()

option(ENABLE_EDG_CUDA "Build EDG 4.0 with CUDA support" OFF)
if(DEFINED CACHE{ENABLE-EDG-CUDA})
  message(DEPRECATION "ENABLE-EDG-CUDA is deprecated. Use ENABLE_EDG_CUDA instead. Example: cmake -DENABLE_EDG_CUDA=${ENABLE-EDG-CUDA} ...")
  set(ENABLE_EDG_CUDA ${ENABLE-EDG-CUDA} CACHE BOOL "Build EDG 4.0 with CUDA support" FORCE)
endif()

option(ENABLE_EDG_OPENCL "Build EDG 4.0 with OpenCL support" OFF)
if(DEFINED CACHE{ENABLE-EDG-OPENCL})
  message(DEPRECATION "ENABLE-EDG-OPENCL is deprecated. Use ENABLE_EDG_OPENCL instead. Example: cmake -DENABLE_EDG_OPENCL=${ENABLE-EDG-OPENCL} ...")
  set(ENABLE_EDG_OPENCL ${ENABLE-EDG-OPENCL} CACHE BOOL "Build EDG 4.0 with OpenCL support" FORCE)
endif()

option(ENABLE_EDG_UNION_STRUCT_DEBUGGING "Should EDG Union/Struct debugging support be used?" OFF)
if(DEFINED CACHE{ENABLE-EDG_UNION_STRUCT_DEBUGGING})
  message(DEPRECATION "ENABLE-EDG_UNION_STRUCT_DEBUGGING is deprecated. Use ENABLE_EDG_UNION_STRUCT_DEBUGGING instead. Example: cmake -DENABLE_EDG_UNION_STRUCT_DEBUGGING=${ENABLE-EDG_UNION_STRUCT_DEBUGGING} ...")
  set(ENABLE_EDG_UNION_STRUCT_DEBUGGING ${ENABLE-EDG_UNION_STRUCT_DEBUGGING} CACHE BOOL "Should EDG Union/Struct debugging support be used?" FORCE)
endif()
if(ENABLE_EDG_UNION_STRUCT_DEBUGGING)
  set(USE_ROSE_EDG_DEBUGGING_SUPPORT 1)
endif()

option(ENABLE_FLTK "Enable FLTK")
if(DEFINED CACHE{ENABLE-FLTK})
  message(DEPRECATION "ENABLE-FLTK is deprecated. Use ENABLE_FLTK instead. Example: cmake -DENABLE_FLTK=${ENABLE-FLTK} ...")
  set(ENABLE_FLTK ${ENABLE-FLTK} CACHE BOOL "Enable FLTK" FORCE)
endif()
if(ENABLE_FLTK)
  find_package(FLTK REQUIRED)
endif()

option(ENABLE_GNU_EXTENSIONS "Enable internal support in ROSE for GNU language extensions" OFF)
if(DEFINED CACHE{ENABLE-GNU-EXTENSIONS})
  message(DEPRECATION "ENABLE-GNU-EXTENSIONS is deprecated. Use ENABLE_GNU_EXTENSIONS instead. Example: cmake -DENABLE_GNU_EXTENSIONS=${ENABLE-GNU-EXTENSIONS} ...")
  set(ENABLE_GNU_EXTENSIONS ${ENABLE-GNU-EXTENSIONS} CACHE BOOL "Enable internal support in ROSE for GNU language extensions" FORCE)
endif()
if(ENABLE_GNU_EXTENSIONS)
  set(ROSE_SUPPORT_GNU_EXTENSIONS TRUE)
endif()

option(ENABLE_INTERNALFRONTENDDEVELOPMENT "Enable development mode to reduce files required to support work on
  language frontends" OFF)
if(DEFINED CACHE{ENABLE-INTERNALFRONTENDDEVELOPMENT})
  message(DEPRECATION "ENABLE-INTERNALFRONTENDDEVELOPMENT is deprecated. Use ENABLE_INTERNALFRONTENDDEVELOPMENT instead. Example: cmake -DENABLE_INTERNALFRONTENDDEVELOPMENT=${ENABLE-INTERNALFRONTENDDEVELOPMENT} ...")
  set(ENABLE_INTERNALFRONTENDDEVELOPMENT ${ENABLE-INTERNALFRONTENDDEVELOPMENT} CACHE BOOL "Enable development mode to reduce files required to support work on language frontends" FORCE)
endif()
if(ENABLE_INTERNALFRONTENDDEVELOPMENT)
  set(ROSE_USE_INTERNAL_FRONTEND_DEVELOPMENT 1)
endif()

option(ENABLE_MICROSOFT_EXTENSIONS "Enable internal support in ROSE for GNU language extensions" OFF)
if(DEFINED CACHE{ENABLE-MICROSOFT-EXTENSIONS})
  message(DEPRECATION "ENABLE-MICROSOFT-EXTENSIONS is deprecated. Use ENABLE_MICROSOFT_EXTENSIONS instead. Example: cmake -DENABLE_MICROSOFT_EXTENSIONS=${ENABLE-MICROSOFT-EXTENSIONS} ...")
  set(ENABLE_MICROSOFT_EXTENSIONS ${ENABLE-MICROSOFT-EXTENSIONS} CACHE BOOL "Enable internal support in ROSE for GNU language extensions" FORCE)
endif()
if(ENABLE_MICROSOFT_EXTENSIONS)
  set(ROSE_SUPPORT_MICROSOFT_EXTENSIONS TRUE)
  set(ROSE_USE_MICROSOFT_EXTENSIONS TRUE)
endif()

if(ENABLE_JOVIAL) # Find dependencies Aterm and Stratego 
  if(NOT DEFINED ATERM_ROOT) # Require location of aterm to be specified by the user at config time  
    message(FATAL_ERROR "Jovial Support Requires Aterm. Please define -DATERM_ROOT with the path of an Aterm installation on your system")
  endif() 
  if(NOT EXISTS "${ATERM_ROOT}" OR NOT IS_DIRECTORY "${ATERM_ROOT}")
    message(WARNING "${ATERM_ROOT} does not exist or is not a directory.") 
    message(FATAL_ERROR "Jovial Support Requires Aterm. Please set ATERM_ROOT to indicate the path of Aterm.")
  endif() 
  include(FindAterm) # Find Aterm based on ATERM_ROOT  
  find_aterm()
  # Error if we cannot find aterm
  if(NOT USE_ROSE_ATERM_SUPPORT)
    message(FATAL_ERROR "Support for experimental_jovial_frontend requires Aterm library support.\nPlease define ATERM_ROOT with location of Aterm install.")
  else() 
    message(STATUS "Found aterm: ${ATERM_ROOT}") 
  endif()
  if(NOT DEFINED STRATEGO_ROOT) # Require location of sglri executable
    message(FATAL_ERROR "Jovial support requires the Stratego sglri executable. Please define -DSTRATEGO_ROOT with the path of a Stratego installation on your system")
  endif()
  include(FindSGLRI) # Find Stratego based on STRATEGO_ROOT
  find_sglri()
endif()

option(ENABLE_PPL "Support for Parma Polyhedral Library" OFF)
if(DEFINED CACHE{ENABLE-PPL})
  message(DEPRECATION "ENABLE-PPL is deprecated. Use ENABLE_PPL instead. Example: cmake -DENABLE_PPL=${ENABLE-PPL} ...")
  set(ENABLE_PPL ${ENABLE-PPL} CACHE BOOL "Support for Parma Polyhedral Library" FORCE)
endif()
if(ENABLE_PPL)
  find_path(with-ppl "include/ppl.h" DOC "Path to Parma Polyhedral Library installation")
  find_library(libppl ppl)
endif()

option(ENABLE_PURIFY_API "Enable purify API in code" OFF)
if(DEFINED CACHE{ENABLE-PURIFY-API})
  message(DEPRECATION "ENABLE-PURIFY-API is deprecated. Use ENABLE_PURIFY_API instead. Example: cmake -DENABLE_PURIFY_API=${ENABLE-PURIFY-API} ...")
  set(ENABLE_PURIFY_API ${ENABLE-PURIFY-API} CACHE BOOL "Enable purify API in code" FORCE)
endif()
if(ENABLE_PURIFY_API)
  set(USE_PURIFY_API 1)
endif()

option(ENABLE_PURIFY_LINKER "Augment the linker with purify" OFF)
if(DEFINED CACHE{ENABLE-PURIFY-LINKER})
  message(DEPRECATION "ENABLE-PURIFY-LINKER is deprecated. Use ENABLE_PURIFY_LINKER instead. Example: cmake -DENABLE_PURIFY_LINKER=${ENABLE-PURIFY-LINKER} ...")
  set(ENABLE_PURIFY_LINKER ${ENABLE-PURIFY-LINKER} CACHE BOOL "Augment the linker with purify" FORCE)
endif()
if(ENABLE_PURIFY_LINKER)
  set(USE_PURIFY 1)
  set(USE_PURIFY_LINKER 1)
endif()

option(ENABLE_ROSEHPCT "enable build of the ROSE-HPCT module" OFF)
if(DEFINED CACHE{ENABLE-ROSEHPCT})
  message(DEPRECATION "ENABLE-ROSEHPCT is deprecated. Use ENABLE_ROSEHPCT instead. Example: cmake -DENABLE_ROSEHPCT=${ENABLE-ROSEHPCT} ...")
  set(ENABLE_ROSEHPCT ${ENABLE-ROSEHPCT} CACHE BOOL "enable build of the ROSE-HPCT module" FORCE)
endif()
if(ENABLE_ROSEHPCT)
  set(ROSE_BUILD_ROSEHPCT 1)
endif()

option(ENABLE_ROSE_OPENGL "enable openGL" OFF)
if(DEFINED CACHE{ENABLE-ROSE-OPENGL})
  message(DEPRECATION "ENABLE-ROSE-OPENGL is deprecated. Use ENABLE_ROSE_OPENGL instead. Example: cmake -DENABLE_ROSE_OPENGL=${ENABLE-ROSE-OPENGL} ...")
  set(ENABLE_ROSE_OPENGL ${ENABLE-ROSE-OPENGL} CACHE BOOL "enable openGL" FORCE)
endif()
if(ENABLE_ROSE_OPENGL)
  find_package(OpenGL)
  find_package(GLUT)
endif()

option(ENABLE_SCOPLIB "Support for ScopLib" OFF)
if(DEFINED CACHE{ENABLE-SCOPLIB})
  message(DEPRECATION "ENABLE-SCOPLIB is deprecated. Use ENABLE_SCOPLIB instead. Example: cmake -DENABLE_SCOPLIB=${ENABLE-SCOPLIB} ...")
  set(ENABLE_SCOPLIB ${ENABLE-SCOPLIB} CACHE BOOL "Support for ScopLib" FORCE)
endif()
if(ENABLE_SCOPLIB)
  find_path(with-scoplib "include/scolib.h" DOC "Path to a valid ScopLib installation")
endif()

option(ENABLE_POET "Enable POET support" OFF)
if(DEFINED CACHE{ENABLE-POET})
  message(DEPRECATION "ENABLE-POET is deprecated. Use ENABLE_POET instead. Example: cmake -DENABLE_POET=${ENABLE-POET} ...")
  set(ENABLE_POET ${ENABLE-POET} CACHE BOOL "Enable POET support" FORCE)
endif()

option(ENABLE_LCOV "Enable creation of *.gcno and *.gcda files for coverage testing" OFF)
if(ENABLE_LCOV)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    #Add flags to produce *.gcda files when run
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fprofile-arcs -ftest-coverage")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fprofile-arcs -ftest-coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lgcov --coverage")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -lgcov --coverage")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -lgcov --coverage")
    #Create lcov-web-pages target to generate the LCOV HTML
    add_custom_target(lcov-web-pages)
    add_custom_command(
      TARGET lcov-web-pages
      POST_BUILD
      COMMAND  ${CMAKE_COMMAND}
              -DBUILD_DIR=${CMAKE_BINARY_DIR}
              -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
              -P ${CMAKE_SOURCE_DIR}/cmake/GenerateLCOVWebPages.cmake
      VERBATIM
      DEPENDS rose-compiler
      COMMENT "Create coverage analysis webpages from *.gcda files in the build tree")
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(WARNING "LCOV with Clang backend is NYI")
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
    message(FATAL_ERROR "Intel compilers don't support lcov tools")
  else()
    message(WARNING "Unrecognized backend compiler ${CMAKE_CXX_COMPILER_ID}, building without lcov.")
  endif()
endif()

# We are removing BISON 
include(FindBison)
find_bison()

# To use Z3, set Z3_ROOT to the Z3 installation prefix. To avoid Z3, use Z3_ROOT=no.
include(FindZ3)
find_z3()

# To use doxygen, set ENABLE_DOXYGEN=ON and DOXYGEN_ROOT=<doxygen/installation/path> 
if(ENABLE_DOXYGEN) 
  if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND "${DOXYGEN_ROOT}" STREQUAL "")
    message(STATUS "Set DOXYGEN_ROOT to the directory doxygen is installed if you want to explicitly control which Doxygen to use") 
  endif() 
  include(FindDoxygen)
  find_doxygen()
endif()

# To use Dlib, set DLIB_ROOT to the Dlib installation prefix. To avoid Dlib set DLIB_ROOT=no
include(FindDlib)
find_dlib()

# To use Capstone, set CAPSTONE_ROOT to the Capstone installation prefix. To avoid Capstone set CAPSTONE_ROOT=no
include(FindCapstone)
find_capstone()

# To use Cereal, set CEREAL_ROOT to the Cereal installation prefix. To avoid Cereal set CEREAL_ROOT=no
include(FindCereal)
find_cereal()

# To use GCrypt, set GCRYPT_ROOT to the GCrypt installation prefix. To avoid GCrypt set GCRYPT_ROOT=no
include(FindGcrypt)
find_gcrypt()
if(GCRYPT_FOUND)
  # To use GPG-Error, set GPGERROR_ROOT to the GPG-Error installation prefix. To avoid GPG-Error set GPGERROR=no
  include(FindGpgError)
  find_gpgerror()
endif()

# To use libmagic, set MAGIC_ROOT to the libmagic installation prefix. To avoid libmagic set MAGIC_ROOT=no
include(FindMagic)
find_magic()

# To use libpqxx, set PQXX_ROOT to the libpqxx installation prefix. To avoid libpqxx set PQXX_ROOT=no
include(FindPqxx)
find_pqxx()

# To use libspot, set SPOT_ROOT to the spot installation prefix. To avoid spot set SPOT_ROOT=no
if(ENABLE_C)
  include(FindSpot)
  find_spot()
endif()

# To use Yices, set YICES_ROOT to the Yices installation prefix. To avoid Yices set YICES_ROOT=no
include(FindYices)
find_yices()

# Zlib is required by Boost. To find it in a special place, set ZLIB_ROOT to its installation prefix.
include(FindZlib)
find_zlib()

# ELF and DWARF libraries (elf must be found before trying to find dwarf)
include(FindElf)
find_elf()
include(FindDwarf)
find_dwarf()

# Flatbuffers. Either set FLATBUFFERS_ROOT to the installation prefix, or place the installation prefix on CMAKE_PREFIX_PATH.
option(ENABLE_FLATBUFFERS_SERIALIZATION "Enable Flatbuffers serialization for binary analysis" OFF)

# ROSE_ENABLE_FLATBUFFERS indicates whether FlatBuffers serialization is enabled for this build.
set(ROSE_ENABLE_FLATBUFFERS OFF)

if(ENABLE_FLATBUFFERS_SERIALIZATION)
  message(STATUS "Looking for Flatbuffers library")
  find_package(flatbuffers CONFIG REQUIRED)
  set(ROSE_ENABLE_FLATBUFFERS ON)
  message(STATUS "Flatbuffers found")
else()
  message(STATUS "Flatbuffers serialization disabled")
endif()

# Whether <sys/personality.h> is present (used by Rose::BinaryAnalysis::Debugger::Linux)
find_path(ROSE_HAVE_SYS_PERSONALITY_H "sys/personality.h")

# Whether <pthread.h> is present
find_path(HAVE_PTHREAD_H pthread.h)

set(ASSERTION_BEHAVIOR "exit" CACHE STRING "Default behavior of failed assertions. Can be 'abort', 'exit', or 'throw'.")
if(ASSERTION_BEHAVIOR STREQUAL "abort")
  add_definitions("-DROSE_ASSERTION_BEHAVIOR=ROSE_ASSERTION_ABORT")
elseif(ASSERTION_BEHAVIOR STREQUAL "exit")
  add_definitions("-DROSE_ASSERTION_BEHAVIOR=ROSE_ASSERTION_EXIT")
elseif(ASSERTION_BEHAVIOR STREQUAL "throw")
  add_definitions("-DROSE_ASSERTION_BEHAVIOR=ROSE_ASSERTION_THROW")
else()
  message(FATAL_ERROR "ASSERTION_BEHAVIOR should be 'abort', 'exit', or 'throw'")
endif()

find_path(with-backstroke-ross ROSS DOC "Specify the path where ROSS is installed")
find_path(with-backstroke-speedes SPEEDES DOC "Specify the path where SPEEDES is installed")
find_library(with-gomp_omp_runtime_library gomp_omp DOC "Specify the prefix where GOMP Runtime System is installed")
find_path(with-GraphViz_include graphviz.h DOC "Specify the prefix where GraphViz include files are installed")
find_path(with-GraphViz_libs GraphViz_libs DOC "Specify the prefix where GraphViz libraries are installed")
find_path(with-haskell runghc DOC "Path to bin directory containing ghc and runghc.")
find_path(with-ida "ida2sql.py" DOC "Specify the prefix where IDA Pro is installed")
find_path(with-insure insure++ DOC "Specify the prefix where insure++ is installed")
find_path(with-IntelPin include/IntelPinSupport.h DOC "Specify the prefix where Intel Pin Package is installed")
find_path(with-ltdl-include ltdl.h DOC "use the ltdl headers installed in DIR")
find_library(with-ltdl-lib ltdl DOC "Path to ltdl library")
find_path(with-llvm "llvm" DOC "Specify the prefix where LLVM (and opt) is installed")
find_path(with-maple "include/maple.h" DOC "Specify the prefix where Maple is installed")
find_path(with-omni_omp_runtime_support include/omni_omp.h DOC "Specify the prefix where Omni OpenMP Runtime System is installed")
find_path(with-purify bin/purify DOC "Specify the prefix where purify is installed")
if(with-purify)
  set(USE_PURIFY 1)
endif()

find_path(with-QRose QRose DOC "prefix of QRose installation")

find_path(with-rted rted DOC "Configure option to have RTED enabled.")
find_path(with-smt-solver smt-solver DOC "Specify the path to an SMT-LIB compatible SMT solver. Used only for testing")

find_path(with-wine "bin/wine" DOC "Specify the prefix where Wine is installed")
if(with-wine)
  set(ROSE_WINE_INCLUDES -I${with-wine}/include)
  set(ROSE_USE_WINDOWS_ANALYSIS_SUPPORT ON)
endif()

if((NOT WIN32) AND (ENABLE_BINARY_ANALYSIS))
  include(FindSqlite3)
  find_sqlite3()
endif()

find_package(FLEX)

# Find libm.so for C Support on Linux and find librt.so for Linux systems
if(NOT WIN32 AND NOT APPLE)
  if(ENABLE_C)
    # libc.so
    find_library(C_LIB c REQUIRED)
    # libm.so
    find_library(M_LIB m REQUIRED)
  endif()
  # libdl.so
  find_library(DL_LIB dl REQUIRED)
  # librt.so, Linux/POSIX Timers and clocks
  find_library(RT_LIB rt QUIET)
endif()

# Only enable __float128 support in EDG if C/C++ is enabled and the compiler
# supports the type (also exclude windows). EDG approximates __float128
# conversions with long double routines (APPROXIMATE_QUADMATH), so the GNU
# quadmath library is not needed.
if(ENABLE_C AND NOT ROSE_SUPPORT_MICROSOFT_EXTENSIONS)
  set(COMPILE_TEST_SOURCE "
  int main() { __float128 x = 0; return (int)x; }")

  # Perform the compile test
  check_cxx_source_compiles("${COMPILE_TEST_SOURCE}" HAVE_FLOAT128_COMPILE)

  # Determine whether __float128 is usable
  if(HAVE_FLOAT128_COMPILE)
    set(ROSE_USE_EDG_QUAD_FLOAT 1)
    set(SUPPORT_FLOAT128 ON CACHE BOOL "Variable that will persist to EDG submodule telling it to enable __float128 support")
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
      message(STATUS "__float128 is available and usable, ROSE_USE_EDG_QUAD_FLOAT set to 1")
    endif()
  else()
    # Comment this line out for issues with clang
    set(ROSE_USE_EDG_QUAD_FLOAT 0)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
      message(STATUS "__float128 is not available or not usable, ROSE_USE_EDG_QUAD_FLOAT set to 0")
    endif()
  endif()

  set(COMPILE_TEST_SOURCE "
  int main() { __asm__(\"\"); return 0; }")

  check_cxx_source_compiles("${COMPILE_TEST_SOURCE}" BACKEND_C_COMPILER_SUPPORTS_UNDESCORE_ASM)

  if(NOT BACKEND_C_COMPILER_SUPPORTS_UNDESCORE_ASM)
    set(BACKEND_C_COMPILER_SUPPORTS_LONG_STRING_ASM 1)
  endif()
endif()

# This is essential to find the right include path from either build or installation tree for a translator
if(HAVE_DLFCN_H)
  if(HAVE_DLADDR)
    set(use_rose_in_build_tree_var TRUE)
    # this following line won't work since it only set the environment variable for cmake's session not for ctest
    # session. Still no good way to set it within cmake fortunately,
    set($ENV{ROSE_IN_BUILD_TREE} ${ROSE_TOP_BINARY_DIR})
  endif()
else()
  set(use_rose_in_build_tree_var, FALSE)
endif()

# ROSE_ALLOC_MEMSET
set(ROSE_ALLOC_MEMSET "0" CACHE STRING "Memory pool protection (memory is set on memory pool operation): 0 -> none, 1 -> command-line (NIY), 2 -> zeroed new/delete, 3 -> aggressive (diff. value for each mempool operation)")
# Make sure ROSE_ALLOC_MEMSET is valid
if(NOT (ROSE_ALLOC_MEMSET GREATER_EQUAL 0 AND ROSE_ALLOC_MEMSET LESS_EQUAL 3))
    message(FATAL_ERROR "ROSE_ALLOC_MEMSET must be between 0 and 3. Current value: ${ROSE_ALLOC_MEMSET}")
endif()

########################################################################################################################
# Alt Back-end compilers -- set alt rose_config.h variables
########################################################################################################################
set(WITH-ALT-BACKEND-CXX-COMPILER "" CACHE STRING "Specify an alternative C++ back-end compiler")
if(WITH-ALT-BACKEND-CXX-COMPILER)
  if(EXISTS ${WITH-ALT-BACKEND-CXX-COMPILER})
    set(BACKEND_CXX_COMPILER ${WITH-ALT-BACKEND-CXX-COMPILER})
  else()
    message(WARNING "${WITH-ALT-BACKEND-CXX-COMPILER} does not exist; specify an absolute path to the compiler")
  endif()
endif()

set(WITH-ALT-BACKEND-C-COMPILER "" CACHE STRING "Specify an alternative C back-end compiler")
if(WITH-ALT-BACKEND-C-COMPILER)
  if(EXISTS ${WITH-ALT-BACKEND-C-COMPILER})
    set(BACKEND_C_COMPILER ${WITH-ALT-BACKEND-C-COMPILER})
    message(STATUS "just set backend C compiler to ${BACKEND_C_COMPILER}")
  else()
    message(WARNING "${WITH-ALT-BACKEND-C-COMPILER} does not exist; specify an absolute path to the compiler")
  endif()
endif()

set(WITH-ALT-BACKEND-FORTRAN-COMPILER "" CACHE STRING "Specify an alternative fortran back-end compiler")
if(WITH-ALT-BACKEND-FORTRAN-COMPILER)
  if(EXISTS ${WITH-ALT-BACKEND-FORTRAN-COMPILER})
    set(BACKEND_FORTRAN_COMPILER ${WITH-ALT-BACKEND-FORTRAN-COMPILER})
  else()
    message(WARNING "${WITH-ALT-BACKEND-FORTRAN-COMPILER} does not exist; specify an absolute path to the compiler")
  endif()
endif()

set(WITH-ALT-BACKEND-JAVA-COMPILER "" CACHE STRING "Specify an alternative java back-end compiler")
if(WITH-ALT-BACKEND-JAVA-COMPILER)
  if(EXISTS ${WITH-ALT-BACKEND-JAVA-COMPILER})
    set(BACKEND_JAVA_COMPILER ${WITH-ALT-BACKEND-JAVA-COMPILER})
  else()
    message(WARNING "${WITH-ALT-BACKEND-JAVA-COMPILER} does not exist; specify an absolute path to the compiler")
  endif()
endif()

########################################################################################################################
# Determine libraries that rose-compiler and librose.so will link to 
########################################################################################################################
# Initialize an empty list to collect all common linked third-party libraries for rose
set(link_with_libraries "") 
# All OS platforms require boost 
list(APPEND link_with_libraries ${Boost_LIBRARIES}) 

# Linux dependencies
if(NOT WIN32 AND NOT APPLE)
  # Conditionally append libc.so to list of libraries rose will link to 
  if(C_LIB AND NOT C_LIB STREQUAL "C_LIB-NOTFOUND")
    list(APPEND link_with_libraries ${C_LIB})
  endif()
  # Conditionally append libm.so to list of libraries rose will link to 
  if(M_LIB AND NOT M_LIB STREQUAL "M_LIB-NOTFOUND")
    list(APPEND link_with_libraries ${M_LIB})
  endif()
  # Conditionally append libdl.so to list of libraries rose will link to 
  if(M_LIB AND NOT DL_LIB STREQUAL "DL_LIB-NOTFOUND")
    list(APPEND link_with_libraries ${DL_LIB})
  endif()
  # Conditionally append librt.so to list of libraries rose will link to
  if(RT_LIB AND NOT RT_LIB STREQUAL "RT_LIB-NOTFOUND")
    list(APPEND link_with_libraries ${RT_LIB})
  endif()    
  # Conditionally append Pthreads to list of libraries rose will link to  
  list(APPEND link_with_libraries ${CMAKE_THREAD_LIBS_INIT})
endif() 

# Mac OS dependencies 
if(APPLE)
  # Conditionally append MLIB
  if(M_LIB AND NOT M_LIB STREQUAL "M_LIB-NOTFOUND")
    list(APPEND link_with_libraries ${M_LIB})
  endif()
  list(APPEND link_with_libraries ${CMAKE_THREAD_LIBS_INIT})
endif()

# Windows dependencies 
if(WIN32)
  list(APPEND link_with_libraries shlwapi.lib psapi.lib)
endif()

# Special Case for rose tool
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND ENABLE_CPP)
  if(NOT ENABLE_C)
    list(APPEND link_with_libraries stdc++)
  endif()
endif() 
if(HAVE_LIBDL)
  list(APPEND link_with_libraries dl)
endif()

# Check compilers and version numbers. The module is located in src/cmake/modules.
if(ENABLE_C OR ENABLE_FORTRAN)
  include(roseChooseBackendCompiler)
  include(roseGenerateBackendCompilerSpecificHeaders)
  install(DIRECTORY ${CMAKE_BINARY_DIR}/include-staging/ DESTINATION include/edg)
else()
  # These are required to be defined even when C/C++ analysis is not enabled, but their values don't matter
  set(BACKEND_C_COMPILER_NAME_WITHOUT_PATH "no_compiler")
  set(BACKEND_C_COMPILER_MAJOR_VERSION_NUMBER "0")
  set(BACKEND_C_COMPILER_MINOR_VERSION_NUMBER "0")
  set(BACKEND_C_COMPILER_PATCH_LEVEL_NUMBER "0")
  set(BACKEND_CXX_COMPILER_NAME_WITHOUT_PATH "no_compiler")
  set(BACKEND_CXX_COMPILER_MAJOR_VERSION_NUMBER "0")
  set(BACKEND_CXX_COMPILER_MINOR_VERSION_NUMBER "0")
  set(BACKEND_CXX_COMPILER_PATCH_LEVEL_NUMBER "0")
endif()

########################################################################################################################
# Summary of results
########################################################################################################################
if(VERBOSE)
  # if you are building in-source, this is the same as CMAKE_SOURCE_DIR, otherwise
  # this is the top level directory of your build tree
  message(STATUS "CMAKE_BINARY_DIR:                   ${CMAKE_BINARY_DIR}")

  # if you are building in-source, this is the same as CMAKE_CURRENT_SOURCE_DIR, otherwise this
  # is the directory where the compiled or generated files from the current CMakeLists.txt will go to
  message(STATUS "CMAKE_CURRENT_BINARY_DIR:           ${CMAKE_CURRENT_BINARY_DIR}")

  # this is the directory, from which cmake was started, i.e. the top level source directory
  message(STATUS "CMAKE_SOURCE_DIR:                   ${CMAKE_SOURCE_DIR}")

  # this is the directory where the currently processed CMakeLists.txt is located in
  message(STATUS "CMAKE_CURRENT_SOURCE_DIR:           ${CMAKE_CURRENT_SOURCE_DIR}")

  # contains the full path to the top level directory of your build tree
  message(STATUS "PROJECT_BINARY_DIR:                 ${PROJECT_BINARY_DIR}")

  # contains the full path to the root of your project source directory,
  # i.e. to the nearest directory where CMakeLists.txt contains the PROJECT() command
  message(STATUS "PROJECT_SOURCE_DIR:                 ${PROJECT_SOURCE_DIR}")

  # set this variable to specify a common place where CMake should put all executable files
  # (instead of CMAKE_CURRENT_BINARY_DIR)
  message(STATUS "EXECUTABLE_OUTPUT_PATH:             ${EXECUTABLE_OUTPUT_PATH}")

  # set this variable to specify a common place where CMake should put all libraries
  # (instead of CMAKE_CURRENT_BINARY_DIR)
  message(STATUS "LIBRARY_OUTPUT_PATH:                ${LIBRARY_OUTPUT_PATH}")

  # tell CMake to search first in directories listed in CMAKE_MODULE_PATH
  # when you use FIND_PACKAGE() or INCLUDE()
  message(STATUS "CMAKE_MODULE_PATH:                  ${CMAKE_MODULE_PATH}")

  # this is the complete path of the cmake which runs currently (e.g. /usr/local/bin/cmake)
  message(STATUS "CMAKE_COMMAND:                      ${CMAKE_COMMAND}")

  # this is the CMake installation directory
  message(STATUS "CMAKE_ROOT:                         ${CMAKE_ROOT}")

  # this is the filename including the complete path of the file where this variable is used.
  message(STATUS "CMAKE_CURRENT_LIST_FILE:            ${CMAKE_CURRENT_LIST_FILE}")

  # this is used when searching for include files e.g. using the FIND_PATH() command.
  message(STATUS "CMAKE_INCLUDE_PATH:                 ${CMAKE_INCLUDE_PATH}")

  # this is used when searching for libraries e.g. using the FIND_LIBRARY() command.
  message(STATUS "CMAKE_LIBRARY_PATH:                 ${CMAKE_LIBRARY_PATH}")

  # the complete system name, e.g. "Linux-2.4.22", "FreeBSD-5.4-RELEASE" or "Windows 5.1"
  message(STATUS "CMAKE_SYSTEM:                       ${CMAKE_SYSTEM}")

  # the short system name, e.g. "Linux", "FreeBSD" or "Windows"
  message(STATUS "CMAKE_SYSTEM_NAME:                  ${CMAKE_SYSTEM_NAME}")

  # only the version part of CMAKE_SYSTEM
  message(STATUS "CMAKE_SYSTEM_VERSION:               ${CMAKE_SYSTEM_VERSION}")

  # the processor name (e.g. "Intel(R) Pentium(R) M processor 2.00GHz")
  message(STATUS "CMAKE_SYSTEM_PROCESSOR:             ${CMAKE_SYSTEM_PROCESSOR}")

  # is TRUE on all UNIX-like OS's, including Apple OS X and CygWin
  message(STATUS "UNIX:                               ${UNIX}")

  # is TRUE on Windows, including CygWin
  message(STATUS "WIN32:                              ${WIN32}")

  # is TRUE on Apple OS X
  message(STATUS "APPLE:                              ${APPLE}")

  # is TRUE when using the MinGW compiler in Windows
  message(STATUS "MINGW:                              ${MINGW}")

  # is TRUE on Windows when using the CygWin version of cmake
  message(STATUS "CYGWIN:                             ${CYGWIN}")

  # is TRUE on Windows when using a Borland compiler
  message(STATUS "BORLAND:                            ${BORLAND}")

  # is RHEL
  message(STATUS "Red Hat version:                    ${ROSE_HOST_OS_IS_RHEL}")

  # Microsoft compiler
  message(STATUS "MSVC:                               ${MSVC}")
  message(STATUS "MSVC_IDE:                           ${MSVC_IDE}")
  message(STATUS "MSVC60:                             ${MSVC60}")
  message(STATUS "MSVC70:                             ${MSVC70}")
  message(STATUS "MSVC71:                             ${MSVC71}")
  message(STATUS "MSVC80:                             ${MSVC80}")
  message(STATUS "CMAKE_COMPILER_2005:                ${CMAKE_COMPILER_2005}")

  # set this to true if you don't want to rebuild the object files if the rules have changed,
  # but not the actual source files or headers (e.g. if you changed the some compiler switches)
  message(STATUS "CMAKE_SKIP_RULE_DEPENDENCY:         ${CMAKE_SKIP_RULE_DEPENDENCY}")

  # since CMake 2.1 the install rule depends on all, i.e. everything will be built before installing.
  # If you don't like this, set this one to true.
  message(STATUS "CMAKE_SKIP_INSTALL_ALL_DEPENDENCY:  ${CMAKE_SKIP_INSTALL_ALL_DEPENDENCY}")

  # If set, runtime paths are not added when using shared libraries. Default it is set to OFF
  message(STATUS "CMAKE_SKIP_RPATH:                   ${CMAKE_SKIP_RPATH}")

  # set this to true if you are using makefiles and want to see the full compile and link
  # commands instead of only the shortened ones
  message(STATUS "CMAKE_VERBOSE_MAKEFILE:             ${CMAKE_VERBOSE_MAKEFILE}")

  # this will cause CMake to not put in the rules that re-run CMake. This might be useful if
  # you want to use the generated build files on another machine.
  message(STATUS "CMAKE_SUPPRESS_REGENERATION:        ${CMAKE_SUPPRESS_REGENERATION}")

  # A simple way to get switches to the compiler is to use ADD_DEFINITIONS().
  # But there are also two variables exactly for this purpose:
  #   CMAKE_C_FLAGS:   the compiler flags for compiling C sources, and
  #   CMAKE_CXX_FLAGS: the compiler flags for compiling C++ sources
  message(STATUS "CMAKE_C_FLAGS:                      ${CMAKE_C_FLAGS}")
  message(STATUS "CMAKE_CXX_FLAGS:                    ${CMAKE_CXX_FLAGS}")

  # Choose the type of build.  Example: SET(CMAKE_BUILD_TYPE Debug)
  message(STATUS "CMAKE_BUILD_TYPE:                   ${CMAKE_BUILD_TYPE}")

  # if this is set to ON, then all libraries are built as shared libraries by default.
  message(STATUS "BUILD_SHARED_LIBS:                  ${BUILD_SHARED_LIBS}")

  # the compiler used for C files
  message(STATUS "CMAKE_C_COMPILER:                   ${CMAKE_C_COMPILER}")

  # the compiler used for C++ files
  message(STATUS "CMAKE_CXX_COMPILER:                 ${CMAKE_CXX_COMPILER}")

  # if the compiler is a variant of gcc, this should be set to 1
  message(STATUS "CMAKE_COMPILER_IS_GNUCC:            ${CMAKE_COMPILER_IS_GNUCC}")

  # if the compiler is a variant of g++, this should be set to 1
  message(STATUS "CMAKE_COMPILER_IS_GNUCXX:           ${CMAKE_COMPILER_IS_GNUCXX}")

  # the tools for creating libraries
  message(STATUS "CMAKE_AR:                           ${CMAKE_AR}")
  message(STATUS "CMAKE_RANLIB:                       ${CMAKE_RANLIB}")

endif()


########################################################################################################################
# ROSE headers are scattered throughout the source tree.
########################################################################################################################
set(ROSE_INCLUDES
  ${ROSE_TOP_BINARY_DIR}
  ${ROSE_TOP_BINARY_DIR}/src/frontend/SageIII/
  ${ROSE_TOP_BINARY_DIR}/src/frontend/SageIII/astFileIO
  ${ROSE_TOP_BINARY_DIR}/src/util
  ${ROSE_TOP_SRC_DIR}
  ${ROSE_TOP_SRC_DIR}/src/frontend/SageIII
  ${ROSE_TOP_SRC_DIR}/src/frontend/SageIII/sage_support
  ${ROSE_TOP_SRC_DIR}/src/ROSETTA/src/
  ${ROSE_TOP_SRC_DIR}/src
  ${ROSE_TOP_SRC_DIR}/src/generated
  ${ROSE_TOP_SRC_DIR}/src/frontend/CxxFrontend/EDG/EDG_SAGE_Connection/
  ${ROSE_TOP_SRC_DIR}/src/frontend/CxxFrontend/EDG/EDG_3.3/src
  ${ROSE_TOP_SRC_DIR}/src/frontend/SageIII
  ${ROSE_TOP_SRC_DIR}/src/frontend/SageIII/astFixup
  ${ROSE_TOP_SRC_DIR}/src/frontend/SageIII/astPostProcessing
  ${ROSE_TOP_SRC_DIR}/src/frontend/SageIII/astMerge
  ${ROSE_TOP_SRC_DIR}/src/frontend/SageIII/astVisualization
  ${ROSE_TOP_SRC_DIR}/src/frontend/SageIII/includeDirectivesProcessing
  ${ROSE_TOP_SRC_DIR}/src/frontend/SageIII/astFileIO
  ${ROSE_TOP_SRC_DIR}/src/frontend/SageIII/sageInterface
  ${ROSE_TOP_SRC_DIR}/src/frontend/SageIII/virtualCFG
  ${ROSE_TOP_SRC_DIR}/src/frontend/SageIII/astTokenStream
  ${ROSE_TOP_SRC_DIR}/src/frontend/SageIII/astHiddenTypeAndDeclarationLists
  ${ROSE_TOP_SRC_DIR}/src/frontend/SageIII/astFileIO
  ${ROSE_TOP_SRC_DIR}/src/frontend/SageIII/astFromString
  ${ROSE_TOP_SRC_DIR}/src/frontend/OpenFortranParser_SAGE_Connection
  ${ROSE_TOP_SRC_DIR}/src/frontend/PHPFrontend
  ${ROSE_TOP_SRC_DIR}/src/frontend/PythonFrontend
  ${ROSE_TOP_SRC_DIR}/src/frontend/BinaryFormats
  ${ROSE_TOP_SRC_DIR}/src/frontend/BinaryLoader
  ${ROSE_TOP_SRC_DIR}/src/frontend/Disassemblers
  ${ROSE_TOP_SRC_DIR}/src/frontend/ExecFormats
  ${ROSE_TOP_SRC_DIR}/src/frontend
  ${ROSE_TOP_SRC_DIR}/src/backend/unparser
  ${ROSE_TOP_SRC_DIR}/src/backend/unparser/formatSupport
  ${ROSE_TOP_SRC_DIR}/src/backend/unparser/languageIndependenceSupport
  ${ROSE_TOP_SRC_DIR}/src/backend/unparser/CxxCodeGeneration
  ${ROSE_TOP_SRC_DIR}/src/backend/unparser/FortranCodeGeneration
  ${ROSE_TOP_SRC_DIR}/src/backend/unparser/JavaCodeGeneration
  ${ROSE_TOP_SRC_DIR}/src/backend/unparser/PHPCodeGeneration
  ${ROSE_TOP_SRC_DIR}/src/backend/unparser/PythonCodeGeneration
  ${ROSE_TOP_SRC_DIR}/src/backend/unparser/AdaCodeGeneration
  ${ROSE_TOP_SRC_DIR}/src/backend/unparser/JovialCodeGeneration
  ${ROSE_TOP_SRC_DIR}/src/backend/asmUnparser
  ${ROSE_TOP_SRC_DIR}/src/util
  ${ROSE_TOP_SRC_DIR}/src/util/support
  ${ROSE_TOP_SRC_DIR}/src/util/graphs
  ${ROSE_TOP_SRC_DIR}/src/util/stringSupport
  ${ROSE_TOP_SRC_DIR}/src/util/commandlineProcessing
  ${ROSE_TOP_SRC_DIR}/src/midend
  ${ROSE_TOP_SRC_DIR}/src/midend/abstractHandle
  ${ROSE_TOP_SRC_DIR}/src/midend/abstractLayer
  ${ROSE_TOP_SRC_DIR}/src/midend/abstractMemoryObject
  ${ROSE_TOP_SRC_DIR}/src/midend/astDiagnostics
  ${ROSE_TOP_SRC_DIR}/src/midend/programTransformation/astInlining
  ${ROSE_TOP_SRC_DIR}/src/midend/programTransformation/astOutlining
  ${ROSE_TOP_SRC_DIR}/src/midend/astProcessing
  ${ROSE_TOP_SRC_DIR}/src/midend/astMatching
  ${ROSE_TOP_SRC_DIR}/src/midend/astQuery
  ${ROSE_TOP_SRC_DIR}/src/midend/astRewriteMechanism
  ${ROSE_TOP_SRC_DIR}/src/midend/astUtil
  ${ROSE_TOP_SRC_DIR}/src/midend/astUtil/annotation
  ${ROSE_TOP_SRC_DIR}/src/midend/astUtil/astInterface
  ${ROSE_TOP_SRC_DIR}/src/midend/astUtil/astSupport
  ${ROSE_TOP_SRC_DIR}/src/midend/astUtil/symbolicVal
  ${ROSE_TOP_SRC_DIR}/src/midend/astUtil/dependenceTracking
  ${ROSE_TOP_SRC_DIR}/src/midend/BinaryAnalysis
  ${ROSE_TOP_SRC_DIR}/src/midend/BinaryAnalysis/dataflowanalyses
  ${ROSE_TOP_SRC_DIR}/src/midend/BinaryAnalysis/graph
  ${ROSE_TOP_SRC_DIR}/src/midend/BinaryAnalysis/instructionSemantics
  ${ROSE_TOP_SRC_DIR}/src/midend/programTransformation/loopProcessing/computation
  ${ROSE_TOP_SRC_DIR}/src/midend/programTransformation/loopProcessing/depGraph
  ${ROSE_TOP_SRC_DIR}/src/midend/programTransformation/loopProcessing/depInfo
  ${ROSE_TOP_SRC_DIR}/src/midend/programTransformation/loopProcessing/driver
  ${ROSE_TOP_SRC_DIR}/src/midend/programTransformation/loopProcessing/outsideInterface
  ${ROSE_TOP_SRC_DIR}/src/midend/programTransformation/loopProcessing/prepostTransformation
  ${ROSE_TOP_SRC_DIR}/src/midend/programTransformation/loopProcessing/slicing
  ${ROSE_TOP_SRC_DIR}/src/midend/programTransformation/ompLowering
  ${ROSE_TOP_SRC_DIR}/src/midend/programAnalysis
  ${ROSE_TOP_SRC_DIR}/src/midend/programAnalysis/CFG
  ${ROSE_TOP_SRC_DIR}/src/midend/programAnalysis/CallGraphAnalysis
  ${ROSE_TOP_SRC_DIR}/src/midend/programAnalysis/OAWrap
  ${ROSE_TOP_SRC_DIR}/src/midend/programAnalysis/bitvectorDataflow
  ${ROSE_TOP_SRC_DIR}/src/midend/programAnalysis/VirtualFunctionAnalysis
  ${ROSE_TOP_SRC_DIR}/src/midend/programAnalysis/defUseAnalysis
  ${ROSE_TOP_SRC_DIR}/src/midend/programAnalysis/distributedMemoryAnalysis
  ${ROSE_TOP_SRC_DIR}/src/midend/programAnalysis/dominanceAnalysis
  ${ROSE_TOP_SRC_DIR}/src/midend/programAnalysis/pointerAnal
  ${ROSE_TOP_SRC_DIR}/src/midend/programAnalysis/staticSingleAssignment
  ${ROSE_TOP_SRC_DIR}/src/midend/programAnalysis/staticInterproceduralSlicing
  ${ROSE_TOP_SRC_DIR}/src/midend/programAnalysis/valuePropagation
  ${ROSE_TOP_SRC_DIR}/src/midend/programTransformation/partialRedundancyElimination
  ${ROSE_TOP_SRC_DIR}/src/midend/programTransformation/finiteDifferencing
  ${ROSE_TOP_SRC_DIR}/src/midend/programTransformation/functionCallNormalization
  ${ROSE_TOP_SRC_DIR}/src/midend/programTransformation/constantFolding
  ${ROSE_TOP_SRC_DIR}/src/midend/programTransformation/implicitCodeGeneration
  ${ROSE_TOP_SRC_DIR}/src/midend/programTransformation/runtimeTransformation
  ${ROSE_TOP_SRC_DIR}/src/roseSupport
  ${ROSE_TOP_SRC_DIR}/src/3rdPartyLibraries/MSTL
  ${ROSE_TOP_SRC_DIR}/src/util/graphs
  ${ROSE_TOP_SRC_DIR}/src/midend/astUtil/astInterface
  ${ROSE_TOP_SRC_DIR}/src/3rdPartyLibraries/json)

if(SQLITE3_FOUND_LIB)
  list(APPEND ROSE_INCLUDES ${ROSE_TOP_SRC_DIR}/src/roseExtensions/sqlite3x)
endif()

if(ENABLE_POET)
  list(APPEND ROSE_INCLUDES ${ROSE_TOP_SRC_DIR}/src/3rdPartyLibraries/POET)
endif()

if(WIN32)
  list(APPEND ROSE_INCLUDES ${ROSE_TOP_SRC_DIR}/winspecific)
endif()

########################################################################################################################
# Set output directories
########################################################################################################################
# Set default output paths for libraries and directories.  This causes all the libraries to be built into a single lib
# directory, and all the executables to be built into a single bin directory.  If these variables are not set, then by
# default the targets will be littered throughout the binary directory, matching the structure of the source directory.
#
# Organizing the output of the build directory this way is important when building on Windows.  By default, Windows
# .exes do not know where to find their required .dlls.  The easiest way to overcome this problem is by placing the .exe
# and all its dependent .dlls in the same directory.  We accomplish this with CMake by setting
# CMAKE_RUNTIME_OUTPUT_DIRECTORY.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Set up RPATH so that our executables will be able to find their dependent
# libraries using relative paths.
if(APPLE)
  set(origin "@executable_path")
  set(CMAKE_INSTALL_RPATH "${origin}/../lib;${origin}/") # May rm this
endif()

if(NOT DEFINED CMAKE_INSTALL_RPATH)
  message(STATUS "Setting CMAKE_INSTALL_RPATH")
  if(APPLE)
    # macOS uses @executable_path (already set above)
    set(CMAKE_INSTALL_RPATH "${origin}/../lib;${origin}/")
  else()
    # Linux/Unix: use $ORIGIN for relative RPATH (relocatable binaries)
    set(origin "$ORIGIN")
    set(CMAKE_INSTALL_RPATH "${origin}/../lib:${origin}")
    # Also add absolute paths as fallback
    list(APPEND CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib" "${CMAKE_INSTALL_PREFIX}/bin")
    if(Boost_LIBRARY_DIRS)
      list(APPEND CMAKE_INSTALL_RPATH "${Boost_LIBRARY_DIRS}")
    endif()
  endif()
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_RPATH}")
  endif()
endif()

if(NOT DEFINED CMAKE_INSTALL_RPATH_USE_LINK_PATH) 
  message(STATUS "Setting CMAKE_INSTALL_RPATH_USE_LINK_PATH")
  set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE) 
  if(CMAKE_BUILD_TYPE STREQUAL "Debug") 
    message(STATUS "CMAKE_INSTALL_RPATH_USE_LINK_PATH ${CMAKE_INSTALL_RPATH_USE_LINK_PATH}")
  endif() 
endif() 

########################################################################################################################
# Recursion
########################################################################################################################
# Recursion in CMake is not automatic. We have to tell it where the other CMakeList.txt files are located rather than
# letting it find them automatically.
add_subdirectory(LicenseInformation)
add_subdirectory(config)
add_subdirectory(src)
add_subdirectory(exampleTranslators)
add_subdirectory(scripts)

# Copy CTestCustom.cmake into the binary directory.  This file tells CTest to ignore some output that it otherwise
# considers compilation errors.
configure_file(cmake/CTestCustom.cmake.in ${CMAKE_BINARY_DIR}/CTestCustom.cmake @ONLY)

#TODO: better support for other compilers & operating systems
if(ENABLE_C)
  set(C_INCLUDE_STRING "{\"${BACKEND_C_COMPILER_NAME_WITHOUT_PATH}_HEADERS\"")
  set(CXX_INCLUDE_STRING "{\"${BACKEND_CXX_COMPILER_NAME_WITHOUT_PATH}_HEADERS\"")
  find_program(shell sh)
  if(shell)
    # find C includes and put them in this order: directory includes, compiler header includes, then /usr/include
    execute_process(
      COMMAND ${shell} ${CMAKE_SOURCE_DIR}/config/dirincludes "./include-staging/" "${BACKEND_C_COMPILER_NAME_WITHOUT_PATH}_HEADERS"
      OUTPUT_VARIABLE C_includeString)
    string(STRIP "${C_includeString}" C_includeString)
    set(C_INCLUDE_STRING "${C_INCLUDE_STRING} ${C_includeString}")
    execute_process(
      COMMAND ${shell} ${CMAKE_SOURCE_DIR}/config/get_compiler_header_dirs ${BACKEND_C_COMPILER_NAME_WITHOUT_PATH} c ${CMAKE_C_COMPILER_ID}
      OUTPUT_VARIABLE C_backend_includes)
    string(REPLACE "\n" ";" C_backend_includes "${C_backend_includes}")
    foreach(C_include_dir ${C_backend_includes})
      string(STRIP "${C_include_dir}" C_include_dir)
      set(C_INCLUDE_STRING "${C_INCLUDE_STRING}, \"${C_include_dir}\"")
    endforeach()
    if(ROSE_CLANG_RESOURCE_INCLUDE_DIR)
      set(C_INCLUDE_STRING "${C_INCLUDE_STRING}, \"${ROSE_CLANG_RESOURCE_INCLUDE_DIR}\"")
    endif()
    set(C_INCLUDE_STRING "${C_INCLUDE_STRING}, \"/usr/include\"")

    # find CXX includes and put them in this order: directory includes, compiler header includes, then /usr/include
    execute_process(
      COMMAND ${shell} ${CMAKE_SOURCE_DIR}/config/dirincludes "./include-staging/" "${BACKEND_CXX_COMPILER_NAME_WITHOUT_PATH}_HEADERS"
      OUTPUT_VARIABLE CXX_includeString)
    string(STRIP "${CXX_includeString}" CXX_includeString)
    set(CXX_INCLUDE_STRING "${CXX_INCLUDE_STRING} ${CXX_includeString}")
    execute_process(
      COMMAND ${shell} ${CMAKE_SOURCE_DIR}/config/get_compiler_header_dirs ${BACKEND_C_COMPILER_NAME_WITHOUT_PATH} c++ ${CMAKE_C_COMPILER_ID}
      OUTPUT_VARIABLE CXX_backend_includes)
    string(REPLACE "\n" ";" CXX_backend_includes "${CXX_backend_includes}")
    foreach(CXX_include_dir ${CXX_backend_includes})
      string(STRIP "${CXX_include_dir}" CXX_include_dir)
      set(CXX_INCLUDE_STRING "${CXX_INCLUDE_STRING}, \"${CXX_include_dir}\"")
    endforeach()
    if(ROSE_CLANG_RESOURCE_INCLUDE_DIR)
      set(CXX_INCLUDE_STRING "${CXX_INCLUDE_STRING}, \"${ROSE_CLANG_RESOURCE_INCLUDE_DIR}\"")
    endif()
    set(CXX_INCLUDE_STRING "${CXX_INCLUDE_STRING}, \"/usr/include\"")

    # In the Autools build system, ROSE stores compiler specific headers in two distinct locations:
    #    In the build tree, the directory is named $ROSE_BUILD/include-staging
    #    In the installation tree, the directory is named $ROSE_INSTALL/include
    # During make-install, the content of $ROSE_BUILD/include-staging is simply copied over into $ROSE_INSTALL/include.
    #
    # If a ROSE translator is detected as being run from the build tree, then the associated header file include paths
    # are added to the translator's command line, e.g. <translator> -I$ROSE_BUILD/include-staging/<path>. Similarly, if
    # the ROSE translator is detected as being run from the install tree, the command line will contain the associated
    # paths in the installation location, e.g. -I$ROSE_INSTALL/include/<path>
    #
    # The setup of CMake for ROSE, on the other hand, does not make this distinction. Instead, the CMake system utilizes
    # a single directory $ROSE_CMAKE_BIN/include for the same purpose.
    execute_process(
      COMMAND ${CMAKE_COMMAND} -E create_symlink
      ${CMAKE_BINARY_DIR}/include-staging ${CMAKE_BINARY_DIR}/include)
  elseif(MSVC)
    set(INCLUDE_LIST $ENV{INCLUDE})
    string(REPLACE "\\" "\\\\" INCLUDE_LIST "${INCLUDE_LIST}")
    foreach(C_include_dir ${INCLUDE_LIST})
      string(STRIP "${C_include_dir}" C_include_dir)
      set(C_INCLUDE_STRING "${C_INCLUDE_STRING}, \"${C_include_dir}\"")
    endforeach()
    set(CXX_INCLUDE_STRING "${C_INCLUDE_STRING}")
  endif()
  set(C_INCLUDE_STRING "${C_INCLUDE_STRING}}")
  set(CXX_INCLUDE_STRING "${CXX_INCLUDE_STRING}}")
else()
  set(C_INCLUDE_STRING {\"none\"})
  set(CXX_INCLUDE_STRING {\"none\"})
endif()

########################################################################################################################
# Private and public build-time configuration files
########################################################################################################################
# Private configuration file.
#
# The rose_config.h file contains CPP macros describing what features were detected during the configuration phase.
# This file should not be installed and should not be #include'd into user-level code because it pollutes the global
# name space.  This needs to be here at the end in order to catch all the variables defined above such as compiler
# names, versions, etc.
configure_file(${ROSE_TOP_SRC_DIR}/cmake/rose_config.h.in.cmake ${ROSE_TOP_BINARY_DIR}/rose_config.h)

# Public configuration file.
#
# The rosePublicConfig.h header is a subset of config variables prepended with ROSE_ to avoid
# polluting a users namespace.
set(ROSE_HAVE_SQLITE3 ${HAVE_SQLITE3})
set(ROSE_HAVE_PTHREAD_H ${HAVE_PTHREAD_H})
set(ROSE_USE_CMAKE ${USE_CMAKE})
set(ROSE_HAVE_BOOST_SERIALIZATION_LIB ${HAVE_BOOST_SERIALIZATION_LIB})

configure_file(${ROSE_TOP_SRC_DIR}/cmake/rosePublicConfig.h.in.cmake ${ROSE_TOP_BINARY_DIR}/rosePublicConfig.h)

########################################################################################################################
# Ancillary ROSE components. These are things that are not part of the ROSE library.
########################################################################################################################
add_custom_target(tools COMMAND echo "tools now build with all")
add_custom_target(install-tools COMMAND echo "tools now install as part of regular install")

add_subdirectory(tools)

# 1/15/24 Turn on ctests until new CMake-Build-RHEL7.sh is merged
if(ENABLE_TESTS_DIRECTORY)
  enable_testing()
  add_subdirectory(tests)
  add_subdirectory(tutorial)
endif()

########################################################################################################################
# Configure and install CMake package config files
########################################################################################################################
# This must come after all dependencies are found so that variables like GCRYPT_FOUND are set

# Configure package config file
configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/RoseConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/cmake/RoseConfig.cmake
  INSTALL_DESTINATION ${LIB_INSTALL_DIR}/cmake/Rose
  PATH_VARS INCLUDE_INSTALL_DIR LIB_INSTALL_DIR
)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/cmake/RoseConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/cmake/RoseConfigVersion.cmake
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/RoseUse.cmake
  DESTINATION ${LIB_INSTALL_DIR}/cmake/Rose
)

# Install Find modules needed by RoseConfig.cmake
# These modules are used to locate ROSE's optional dependencies
install(FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/FindGcrypt.cmake
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/FindGpgError.cmake
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/FindZ3.cmake
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/FindDwarf.cmake
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/FindElf.cmake
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/FindCapstone.cmake
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/FindDlib.cmake
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/FindSqlite3.cmake
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/FindYices.cmake
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/FindMagic.cmake
  DESTINATION ${LIB_INSTALL_DIR}/cmake/Rose
)

########################################################################################################################
# Configure and install pkg-config file
########################################################################################################################
# Build pkg-config dependency lists and flags
set(ROSE_PC_REQUIRES "")
set(ROSE_PC_LIBS "")
set(ROSE_PC_CFLAGS "-std=c++14")

# Add Boost libraries directly (Boost typically doesn't provide pkg-config files)
# Include library directory and all required Boost components
if(Boost_LIBRARY_DIRS)
  set(ROSE_PC_LIBS "${ROSE_PC_LIBS} -L${Boost_LIBRARY_DIRS}")
endif()
if(Boost_INCLUDE_DIRS)
  set(ROSE_PC_CFLAGS "${ROSE_PC_CFLAGS} -I${Boost_INCLUDE_DIRS}")
endif()

# Add individual Boost libraries
foreach(comp chrono date_time filesystem iostreams program_options random regex system thread wave serialization)
  set(ROSE_PC_LIBS "${ROSE_PC_LIBS} -lboost_${comp}")
endforeach()

# Add pthread flag if Boost threading is used
if(Boost_THREAD_FOUND)
  set(ROSE_PC_CFLAGS "${ROSE_PC_CFLAGS} -pthread")
  set(ROSE_PC_LIBS "${ROSE_PC_LIBS} -pthread")
endif()

# Add optional PUBLIC dependencies that consumers might need
# These are dependencies whose headers appear in ROSE's public API
# Include library paths to help linker find them
if(GCRYPT_FOUND)
  if(GCRYPT_LIBRARY_PATH)
    set(ROSE_PC_LIBS "${ROSE_PC_LIBS} -L${GCRYPT_LIBRARY_PATH}")
  endif()
  if(GPGERROR_LIBRARY_PATH)
    set(ROSE_PC_LIBS "${ROSE_PC_LIBS} -L${GPGERROR_LIBRARY_PATH}")
  endif()
  set(ROSE_PC_LIBS "${ROSE_PC_LIBS} -lgcrypt -lgpg-error")
  # Add include paths for gcrypt headers
  if(GCRYPT_HEADER_PATH)
    set(ROSE_PC_CFLAGS "${ROSE_PC_CFLAGS} -I${GCRYPT_HEADER_PATH}")
  endif()
  # Add include path for gpg-error headers (needed by gcrypt.h)
  if(GPGERROR_HEADER_PATH)
    set(ROSE_PC_CFLAGS "${ROSE_PC_CFLAGS} -I${GPGERROR_HEADER_PATH}")
  endif()
endif()

if(CAPSTONE_FOUND)
  if(CAPSTONE_LIBRARY)
    get_filename_component(_capstone_lib_dir "${CAPSTONE_LIBRARY}" DIRECTORY)
    set(ROSE_PC_LIBS "${ROSE_PC_LIBS} -L${_capstone_lib_dir}")
  endif()
  set(ROSE_PC_LIBS "${ROSE_PC_LIBS} -lcapstone")
  # Add include path for capstone headers
  if(CAPSTONE_ROOT)
    set(ROSE_PC_CFLAGS "${ROSE_PC_CFLAGS} -I${CAPSTONE_ROOT}/include")
  endif()
endif()

# Note: Dlib is header-only, so include path is needed but no libs
if(DLIB_FOUND AND DLIB_ROOT)
  set(ROSE_PC_CFLAGS "${ROSE_PC_CFLAGS} -I${DLIB_ROOT}/include")
endif()

# Add system libraries
if(CMAKE_DL_LIBS)
  set(ROSE_PC_LIBS "${ROSE_PC_LIBS} ${CMAKE_DL_LIBS}")
endif()

# Configure the pkg-config file using configure_package_config_file for proper path handling
set(PACKAGE_LIB_INSTALL_DIR "\${exec_prefix}/${LIB_INSTALL_DIR}")
set(PACKAGE_INCLUDE_INSTALL_DIR "\${prefix}/${INCLUDE_INSTALL_DIR}")
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/rose.pc.in
  ${CMAKE_CURRENT_BINARY_DIR}/rose.pc
  @ONLY
)

# Install the pkg-config file
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/rose.pc
  DESTINATION ${LIB_INSTALL_DIR}/pkgconfig
)

########################################################################################################################
# Install RMC configuration file if present
########################################################################################################################
# When ROSE dependencies are managed with "rmc", a file named ".spock" is created at the top of the build tree.
# Install this file as "lib/rmc/rose" if RMC is active (indicated by SPOCK_EMPLOYED environment variable).
if(DEFINED ENV{SPOCK_EMPLOYED})
  if(EXISTS "${CMAKE_BINARY_DIR}/.spock")
    install(FILES ${CMAKE_BINARY_DIR}/.spock
      DESTINATION ${LIB_INSTALL_DIR}/rmc
      RENAME rose
    )
  endif()
endif()

########################################################################################################################
# Miscellaneous final actions
########################################################################################################################
# These actions don't work on Linux Debian-based systems during "make install" and appear to not be necessary anyway.
if(NOT UNIX)
  # This has to remain the last subdirectory so that all the targets are installed before bundle.cmake runs.
  add_subdirectory(cmake)
  # This include file defines how CPack should create our installers.
  include(cmake/ROSECPack.cmake)
endif()
