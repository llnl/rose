# Check compilers and version number
#  Liao 11/25/2009
# This is translated from config/choose-backend-compiler.m4
#
# use the results to setup rose_config.h later on
# TSG 2/3/25 Fixup -- Remove unnecessary code and add three functions 
#            for getting major, minor, and patch version from compiler 
#            Backend c/cxx compiler logic has been moved to this file              

# Three Functions for returning MAJOR.MINOR.PATCH 
function(get_major_version version output_variable)
  string(REGEX MATCH "^[0-9]+" major_version ${version})
  set(${output_variable} ${major_version} PARENT_SCOPE)
endfunction()

function(get_minor_version version output_variable)
  string(REGEX MATCH "^[0-9]+\\.([0-9]+)" _match "${version}")
  set(${output_variable} "${CMAKE_MATCH_1}" PARENT_SCOPE)
endfunction()

function(get_patch_version version output_variable)
  string(REGEX MATCH "^[0-9]+\\.[0-9]+\\.([0-9]+)" _match "${version}")
  set(${output_variable} "${CMAKE_MATCH_1}" PARENT_SCOPE)
endfunction()
# End of functions for returning MAJOR.MINOR.PATCH 

# --------check Backend C compiler -----------------------
if(ENABLE_C)
  include(CMakeDetermineCCompiler)
  # Support for Visual Studio is handled in a separate file
  if(WIN32)
    include(roseWindowsSupport)
    return()
  endif()

  # Ada Support
  if(ENABLE_ADA) 
    # Ensure GNAT_HOME is defined
    if(NOT DEFINED ENV{GNAT_HOME} OR "$ENV{GNAT_HOME}" STREQUAL "")
      message(FATAL_ERROR "GNAT_HOME environment variable must be defined for Ada support")
    endif()
    # Ensure that GCC/GNU is the compiler
    if(NOT CMAKE_C_COMPILER_ID STREQUAL "GNU")
      message(FATAL_ERROR "GCC/GNU is needed for Ada support")
    endif()
    # Hard-set the compiler path/name needed for Ada support
    set(BACKEND_C_COMPILER_NAME_WITH_PATH "$ENV{GNAT_HOME}/bin/gcc")
    set(BACKEND_C_COMPILER_NAME_WITHOUT_PATH "gcc")
  endif() # end Ada 

  get_major_version("${CMAKE_C_COMPILER_VERSION}" BACKEND_C_COMPILER_MAJOR_VERSION_NUMBER) 
  get_minor_version("${CMAKE_C_COMPILER_VERSION}" BACKEND_C_COMPILER_MINOR_VERSION_NUMBER) 
  get_patch_version("${CMAKE_C_COMPILER_VERSION}" BACKEND_C_COMPILER_PATCH_LEVEL_NUMBER)

  # Set With and Without Path
  if(CMAKE_C_COMPILER_ID STREQUAL "GNU") 
    # Set BACKEND_C_COMPILER_WITH_PATH for GNU 
    if(NOT DEFINED BACKEND_C_COMPILER_NAME_WITH_PATH)
      find_program(GCC_PATH NAMES gcc HINTS ENV PATH)
      if(GCC_PATH)
        message(STATUS "Setting BACKEND_C_COMPILER_NAME_WITH_PATH to ${GCC_PATH}")
        set(BACKEND_C_COMPILER_NAME_WITH_PATH "${GCC_PATH}")
      endif()
    endif()
    # Set BACKEND_C_COMPILER_NAME_WITHOUT_PATH for GNU 
    if(NOT DEFINED BACKEND_C_COMPILER_NAME_WITHOUT_PATH)
      set(BACKEND_C_COMPILER_NAME_WITHOUT_PATH "gcc") 
      set(BACKEND_C_COMPILER "gcc")
      set(ROSE_BACKEND_C_COMPILER "gcc") 
    endif() # end GNU for C 
  
  # Set With and Without Path for Clang
  elseif(CMAKE_C_COMPILER_ID MATCHES "Clang")   # Clang and AppleClang
    # Set BACKEND_C_COMPILER_WITH_PATH for Clang
    if(NOT DEFINED BACKEND_C_COMPILER_NAME_WITH_PATH)
      find_program(CLANG_PATH NAMES clang HINTS ENV PATH)
      if(CLANG_PATH)
        message(STATUS "Setting BACKEND_C_COMPILER_NAME_WITH_PATH to ${CLANG_PATH}")
        set(BACKEND_C_COMPILER_NAME_WITH_PATH "${CLANG_PATH}")
      endif()
    endif()
    # Set BACKEND_C_COMPILER_WITHOUT_PATH for Clang
    if(NOT DEFINED BACKEND_C_COMPILER_NAME_WITHOUT_PATH)
      message(STATUS "Setting BACKEND_C_COMPILER_NAME_WITHOUT_PATH to clang")
      set(BACKEND_C_COMPILER_NAME_WITHOUT_PATH "clang")
      set(BACKEND_C_COMPILER "clang")
      set(ROSE_BACKEND_C_COMPILER "clang") 
    endif() # end Clang for C 
    # Set Apple Case here  
  endif() # end Set With and Without Path 
  
  if(NOT BACKEND_C_COMPILER)
    message(WARNING "Setting BACKEND_C_COMPILER -- roseCMakeDetermineCCompiler.cmake may not be working or needed") 
    # Use the default C compiler if we haven't determined a backend C compiler
    set(BACKEND_C_COMPILER  ${CMAKE_C_COMPILER})
  endif() 

  if(VERBOSE)
    message(STATUS "BACKEND_C_COMPILER_MAJOR_VERSION_NUMBER: ${BACKEND_C_COMPILER_MAJOR_VERSION_NUMBER}") 
    message(STATUS "BACKEND_C_COMPILER_MINOR_VERSION_NUMBER: ${BACKEND_C_COMPILER_MINOR_VERSION_NUMBER}")
    message(STATUS "BACKEND_C_COMPILER_PATCH_VERSION_NUMBER: ${BACKEND_C_COMPILER_PATCH_LEVEL_NUMBER}")
    message(STATUS "BACKEND_C_COMPILER ${BACKEND_C_COMPILER}")
    message(STATUS "ROSE_BACKEND_C_COMPILER ${ROSE_BACKEND_C_COMPILER}")
    message(STATUS "BACKEND_C_COMPILER_NAME_WITHOUT_PATH: ${BACKEND_C_COMPILER_NAME_WITHOUT_PATH}")
    message(STATUS "BACKEND_C_COMPILER_NAME_WITH_PATH: ${BACKEND_C_COMPILER_NAME_WITH_PATH}")
  endif() 
endif()

# --------check Backend CXX compiler -----------------------
if(ENABLE_CPP)
  if(CMAKE_CROSSCOMPILING)
    include(roseCMakeDetermineCXXCompiler)
  endif()
  # Ada Support
  if(ENABLE_ADA) 
     # Ensure GNAT_HOME is defined
    if(NOT DEFINED ENV{GNAT_HOME} OR "$ENV{GNAT_HOME}" STREQUAL "")
      message(FATAL_ERROR "GNAT_HOME environment variable must be defined for Ada support")
    endif()
    # Ensure that GCC/GNU is the compiler
    if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
      message(FATAL_ERROR "GCC/GNU is needed for Ada support")
    endif()
    # Hard-set the compiler path/name needed for Ada support
    set(BACKEND_CXX_COMPILER_NAME_WITH_PATH "$ENV{GNAT_HOME}/bin/g++")
    set(BACKEND_CXX_COMPILER_NAME_WITHOUT_PATH "g++")
  endif() # end Ada 

  # Extract major, minor, and patch version from CMAKE_CXX_COMPILER_VERSION
  get_major_version("${CMAKE_CXX_COMPILER_VERSION}" BACKEND_CXX_COMPILER_MAJOR_VERSION_NUMBER)
  get_minor_version("${CMAKE_CXX_COMPILER_VERSION}" BACKEND_CXX_COMPILER_MINOR_VERSION_NUMBER)
  get_patch_version("${CMAKE_CXX_COMPILER_VERSION}" BACKEND_CXX_COMPILER_PATCH_VERSION_NUMBER)

  # Set With and Without Path For GNU 
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # Set BACKEND_CXX_COMPILER_WITH_PATH for GNU 
    if(NOT DEFINED BACKEND_CXX_COMPILER_NAME_WITH_PATH)
      find_program(GXX_PATH NAMES g++ HINTS ENV PATH)
      if(GXX_PATH)
        message(STATUS "Setting BACKEND_CXX_COMPILER_NAME_WITH_PATH to ${GXX_PATH}")
        set(BACKEND_CXX_COMPILER_NAME_WITH_PATH "${GXX_PATH}")
      endif()
    endif()
    # Set BACKEND_CXX_COMPILER_NAME_WITHOUT_PATH for GNU 
    if(NOT DEFINED BACKEND_CXX_COMPILER_NAME_WITHOUT_PATH)
      set(BACKEND_CXX_COMPILER_NAME_WITHOUT_PATH "g++")
      set(BACKEND_CXX_COMPILER "g++")
      set(ROSE_BACKEND_CXX_COMPILER "g++") 
    endif() # end GNU for CXX  

  # Set With and Without Path for Clang++ 
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")   # Clang and AppleClang
    # Set BACKEND_CXX_COMPILER_WITH_PATH for Clang++
    if(NOT DEFINED BACKEND_CXX_COMPILER_NAME_WITH_PATH)
      find_program(CLANGXX_PATH NAMES clang++ HINTS ENV PATH)
      if(CLANGXX_PATH)
        message(STATUS "Setting BACKEND_CXX_COMPILER_NAME_WITH_PATH to ${CLANGXX_PATH}")
        set(BACKEND_CXX_COMPILER_NAME_WITH_PATH "${CLANGXX_PATH}")
      endif()
    endif()
    # Set BACKEND_CXX_COMPILER_WITHOUT_PATH for Clang++ 
    if(NOT DEFINED BACKEND_CXX_COMPILER_NAME_WITHOUT_PATH)
      message(STATUS "Setting BACKEND_CXX_COMPILER_NAME_WITHOUT_PATH to clang++")
      set(BACKEND_CXX_COMPILER_NAME_WITHOUT_PATH "clang++")
      set(BACKEND_CXX_COMPILER "clang++")
      set(ROSE_BACKEND_CXX_COMPILER "clang++")
    endif() # end Clang for CXX 
  endif() # end Set With and Without 
  # Can Set Apple Case here 
  # Use the default C++ compiler if we haven't determined a backend C++ compiler
  if(NOT BACKEND_CXX_COMPILER)
    message(WARNING "Setting BACKEND_CXX_COMPILER  -- CMakeDetermineCCompiler.cmake may not be working or needed") 
    set(BACKEND_CXX_COMPILER  ${CMAKE_CXX_COMPILER})
  endif() 

  if(VERBOSE)
    message(STATUS "BACKEND_CXX_COMPILER_MAJOR_VERSION_NUMBER: ${BACKEND_CXX_COMPILER_MAJOR_VERSION_NUMBER}")
    message(STATUS "BACKEND_CXX_COMPILER_MINOR_VERSION_NUMBER: ${BACKEND_CXX_COMPILER_MINOR_VERSION_NUMBER}")
    message(STATUS "BACKEND_CXX_COMPILER_PATCH_VERSION_NUMBER: ${BACKEND_CXX_COMPILER_PATCH_VERSION_NUMBER}")
    message(STATUS "BACKEND_CXX_COMPILER: ${BACKEND_CXX_COMPILER}")
    message(STATUS "ROSE_BACKEND_CXX_COMPILER: ${ROSE_BACKEND_CXX_COMPILER}")
    message(STATUS "BACKEND_CXX_COMPILER_NAME_WITHOUT_PATH: ${BACKEND_CXX_COMPILER_NAME_WITHOUT_PATH}")
    message(STATUS "BACKEND_CXX_COMPILER_NAME_WITH_PATH: ${BACKEND_CXX_COMPILER_NAME_WITH_PATH}")
  endif() # end VERBOSE
endif() # end ENABLE_CPP

# TO DO: Remove redundancies where appropriate
if(ENABLE_FORTRAN)
# --------check Fortran compiler -----------------------
# CMakeDetermineFortranCompiler does not recognize gfortran first
# we use a slightly modified CMakeDetermineFortranCompiler.cmake to put gfortran to the highest priority
# Pei-Hung (04/08/21) allowed gfortran* for homebrew gfortran with suffix name
include(roseCMakeDetermineFortranCompiler)
if("${CMAKE_Fortran_COMPILER}"  MATCHES ".*gfortran.*$")
  if(VERBOSE)
    message("find gfortran compiler ${CMAKE_Fortran_COMPILER}")
  endif()
  if(NOT BACKEND_FORTRAN_COMPILER)
    set (BACKEND_FORTRAN_COMPILER  ${CMAKE_Fortran_COMPILER})
  endif()
  execute_process(
      COMMAND ${BACKEND_FORTRAN_COMPILER} --version
      COMMAND head -1
      COMMAND cut -f2 -d\)
      COMMAND tr -d \ # must have a space
      COMMAND cut -d. -f1
      OUTPUT_VARIABLE BACKEND_FORTRAN_COMPILER_MAJOR_VERSION_NUMBER)
  string(REGEX MATCH "[0-9]+" BACKEND_FORTRAN_COMPILER_MAJOR_VERSION_NUMBER ${BACKEND_FORTRAN_COMPILER_MAJOR_VERSION_NUMBER})
  execute_process(
    COMMAND basename ${BACKEND_FORTRAN_COMPILER}
    OUTPUT_VARIABLE BACKEND_FORTRAN_COMPILER_NAME_WITHOUT_PATH)
  string(REGEX MATCH "[a-zA-Z0-9/.+-]+" BACKEND_FORTRAN_COMPILER_NAME_WITHOUT_PATH ${BACKEND_FORTRAN_COMPILER_NAME_WITHOUT_PATH})
  execute_process(
    COMMAND ${BACKEND_FORTRAN_COMPILER} --version
    COMMAND head -1
    COMMAND cut -f2 -d\)
    COMMAND tr -d \ # must have a space
    COMMAND cut -d. -f2
    OUTPUT_VARIABLE BACKEND_FORTRAN_COMPILER_MINOR_VERSION_NUMBER)
  string(REGEX MATCH "[0-9]+" BACKEND_FORTRAN_COMPILER_MINOR_VERSION_NUMBER ${BACKEND_FORTRAN_COMPILER_MINOR_VERSION_NUMBER})
  endif()
endif()

# Values of __GNUC__, __GNUC_MINOR__ and __GNUC_PATCHLEVEL__ exactly as the backend compiler predefines them;
# rose_config.h hands them to EDG (CXX_SPEC_DEF). For GCC these equal its version; clang (and AppleClang)
# reports 4.2.1 and the system headers rely on that, so the compiler is asked rather than assumed.
if(ENABLE_CPP)
  execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dM -E -x c++ /dev/null
                  OUTPUT_VARIABLE _rose_backend_predefines ERROR_QUIET)
  foreach(_m MAJOR:__GNUC__ MINOR:__GNUC_MINOR__ PATCH:__GNUC_PATCHLEVEL__)
    string(REGEX REPLACE ":.*" "" _which "${_m}")
    string(REGEX REPLACE ".*:" "" _macro "${_m}")
    string(REGEX MATCH "#define ${_macro} ([0-9]+)" _unused "${_rose_backend_predefines}")
    set(ROSE_BACKEND_GNUC_${_which} "${CMAKE_MATCH_1}")
  endforeach()
  if(NOT ROSE_BACKEND_GNUC_MAJOR)   # fall back to the compiler's version number (previous behaviour)
    set(ROSE_BACKEND_GNUC_MAJOR "${BACKEND_C_COMPILER_MAJOR_VERSION_NUMBER}")
    set(ROSE_BACKEND_GNUC_MINOR "${BACKEND_C_COMPILER_MINOR_VERSION_NUMBER}")
    set(ROSE_BACKEND_GNUC_PATCH "${BACKEND_C_COMPILER_PATCH_LEVEL_NUMBER}")
  endif()
  message(STATUS "Backend predefines __GNUC__ ${ROSE_BACKEND_GNUC_MAJOR}.${ROSE_BACKEND_GNUC_MINOR}.${ROSE_BACKEND_GNUC_PATCH}")
endif()

# Set BACKEND_CXX_IS_<>_COMPILER macro for rose_config.h 
if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  # using Clang or AppleClang
  set(BACKEND_CXX_IS_CLANG_COMPILER 1)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  # using GCC
  set(BACKEND_CXX_IS_GNU_COMPILER 1)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
  # using Intel C++
  set(BACKEND_CXX_IS_INTEL_COMPILER 1)
endif()
