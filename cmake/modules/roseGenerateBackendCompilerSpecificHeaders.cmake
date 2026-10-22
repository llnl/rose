# Liao 11/25/2009
# Converted from generate-backend-compiler-specific-headers.m4
#
# This should be called after roseChooseBackendCompiler.cmake is called

if(VERBOSE)
  message("BACKEND_CXX_COMPILER_MAJOR_VERSION_NUMBER=" ${BACKEND_CXX_COMPILER_MAJOR_VERSION_NUMBER})
  message("BACKEND_CXX_COMPILER_MINOR_VERSION_NUMBER=" ${BACKEND_CXX_COMPILER_MINOR_VERSION_NUMBER})
endif()

# GENERATE_BACKEND_CXX_COMPILER_SPECIFIC_HEADERS
# ----------------------------------------------------
execute_process(
  COMMAND chmod u+x ${PROJECT_SOURCE_DIR}/config/create_system_headers
  COMMAND mkdir -p ${ROSE_TOP_BINARY_DIR}/include-staging/${BACKEND_CXX_COMPILER_NAME_WITHOUT_PATH}_HEADERS)

if(VERBOSE)
  message("Starting to run: ${PROJECT_SOURCE_DIR}/config/create_system_headers ${BACKEND_CXX_COMPILER_NAME_WITHOUT_PATH}"
    "./include-staging/${BACKEND_CXX_COMPILER_NAME_WITHOUT_PATH}_HEADERS ${PROJECT_SOURCE_DIR}")
endif()

# we split several commands into different execute_process ()
# otherwise cmake may skip executing the last ones!!!    
execute_process(
  COMMAND ${PROJECT_SOURCE_DIR}/config/create_system_headers ${BACKEND_CXX_COMPILER_NAME_WITHOUT_PATH}
    ./include-staging/${BACKEND_CXX_COMPILER_NAME_WITHOUT_PATH}_HEADERS ${PROJECT_SOURCE_DIR} c++ gnu
    ${BACKEND_CXX_COMPILER_MAJOR_VERSION_NUMBER}
  WORKING_DIRECTORY ${ROSE_TOP_BINARY_DIR}
  ERROR_VARIABLE errorVar)

# DQ (2/3/2016): Uncomment this to allow errors to be detected.
# the error code is too strict. don't use it for now  
# if(errorVar)
#   message(FATAL_ERROR "Could not run create_system_headers properly for c++ language support")
# endif()
set(ROSE_CXX_HEADERS_DIR "${CMAKE_INSTALL_PREFIX}/include/${BACKEND_CXX_COMPILER_NAME_WITHOUT_PATH}_HEADERS")

# GENERATE_BACKEND_C_COMPILER_SPECIFIC_HEADERS
# ----------------------------------------------------
execute_process(
  COMMAND chmod u+x ${PROJECT_SOURCE_DIR}/config/create_system_headers
  COMMAND mkdir -p ${ROSE_TOP_BINARY_DIR}/include-staging/${BACKEND_C_COMPILER_NAME_WITHOUT_PATH}_HEADERS)

if(VERBOSE)
  message("Starting to run: ${PROJECT_SOURCE_DIR}/config/create_system_headers ${BACKEND_C_COMPILER_NAME_WITHOUT_PATH}"
    "./include-staging/${BACKEND_C_COMPILER_NAME_WITHOUT_PATH}_HEADERS ${PROJECT_SOURCE_DIR}")
endif()

execute_process(
  COMMAND ${PROJECT_SOURCE_DIR}/config/create_system_headers ${BACKEND_C_COMPILER_NAME_WITHOUT_PATH}
    ./include-staging/${BACKEND_C_COMPILER_NAME_WITHOUT_PATH}_HEADERS ${PROJECT_SOURCE_DIR} c gnu
    ${BACKEND_C_COMPILER_MAJOR_VERSION_NUMBER}
  WORKING_DIRECTORY ${ROSE_TOP_BINARY_DIR}
  ERROR_VARIABLE errorVar)
# DQ (2/3/2016): Uncomment this to allow errors to be detected.
# the error code is too strict. don't use it for now  
#  if(errorVar)
#    message(FATAL_ERROR "Could not run create_system_headers properly for c language support")
#  endif()  
set(ROSE_C_HEADERS_DIR "${CMAKE_INSTALL_PREFIX}/include/${BACKEND_C_COMPILER_NAME_WITHOUT_PATH}_HEADERS")


# DQ (9/15/2010): Copy the upc.h header file from the config directory to our include-staging/${compilerName}_HEADERS
# directory.  It might be that these should be put into a UPC specific subdirectory (so that the C compiler can't
# accedentally find them), but this should be discussed.
set(upc_headers upc.h upc_io.h upc_relaxed.h upc_strict.h upc_collective.h bupc_extensions.h)

foreach(header ${upc_headers})
  configure_file(
    ${PROJECT_SOURCE_DIR}/config/${header}
    ${PROJECT_BINARY_DIR}/include-staging/${BACKEND_C_COMPILER_NAME_WITHOUT_PATH}_HEADERS/
    COPYONLY)
  configure_file(
    ${PROJECT_SOURCE_DIR}/config/${header}
    ${PROJECT_BINARY_DIR}/include-staging/${BACKEND_CXX_COMPILER_NAME_WITHOUT_PATH}_HEADERS/
    COPYONLY)
endforeach()

# DQ (8/22/2011): Added support for SSE.
# Copy alternative SSE and MMX headers to be seen by ROSE ahead of the originals.
set(SSE_headers emmintrin.h xmmintrin.h mmintrin.h ammintrin.h nmmintrin.h pmmintrin.h smmintrin.h tmmintrin.h)
if("${BACKEND_CXX_COMPILER_MAJOR_VERSION_NUMBER}" STREQUAL "4")
  if("${BACKEND_CXX_COMPILER_MINOR_VERSION_NUMBER}" GREATER "5")
    set(SSE_headers ${SSE_headers} avxintrin.h)
  endif()
endif()
foreach(header ${SSE_headers})
  configure_file(
    ${PROJECT_SOURCE_DIR}/config/rose_specific_${header}
    ${PROJECT_BINARY_DIR}/include-staging/${BACKEND_C_COMPILER_NAME_WITHOUT_PATH}_HEADERS/${header}
    COPYONLY)
  configure_file(
    ${PROJECT_SOURCE_DIR}/config/rose_specific_${header}
    ${PROJECT_BINARY_DIR}/include-staging/${BACKEND_CXX_COMPILER_NAME_WITHOUT_PATH}_HEADERS/${header}
    COPYONLY)
endforeach()

if(PERL_FOUND)
  if(VERBOSE)
    message(STATUS "generating header file for builtin functions...")
  endif()
  execute_process(
    OUTPUT_FILE ${CMAKE_BINARY_DIR}/include-staging/${BACKEND_CXX_COMPILER_NAME_WITHOUT_PATH}_HEADERS/rose_generated_builtin_functions.h
    COMMAND ${CMAKE_SOURCE_DIR}/scripts/builtinLlvmFunctions.pl
      --constexpr=${CMAKE_SOURCE_DIR}/config/constexpr_builtins.def ${CMAKE_SOURCE_DIR}/config/Builtins.def)

  if(VERBOSE)
    message(STATUS "adding contents of rose_generated_builtin_functions.h to rose_edg_required_macros_and_functions.h...")
  endif()
else()
  if(VERBOSE)
    message(STATUS "Perl not found. Using pre-generated builtins")
  endif()

  file(COPY ${CMAKE_SOURCE_DIR}/cmake/rose_generated_builtin_functions.h.pre
  DESTINATION ${CMAKE_BINARY_DIR}/include-staging/${BACKEND_CXX_COMPILER_NAME_WITHOUT_PATH}_HEADERS/)
  file(RENAME ${CMAKE_BINARY_DIR}/include-staging/${BACKEND_CXX_COMPILER_NAME_WITHOUT_PATH}_HEADERS/rose_generated_builtin_functions.h.pre
              ${CMAKE_BINARY_DIR}/include-staging/${BACKEND_CXX_COMPILER_NAME_WITHOUT_PATH}_HEADERS/rose_generated_builtin_functions.h)
endif()

# Insert the generated builtin prototypes after the REPLACE_ME_WITH_GENERATED_BUILTIN_FUNCTIONS marker line
# (done in CMake rather than with "sed -i", whose in-place syntax differs between GNU and BSD/macOS sed).
set(_rose_req_hdr "${CMAKE_BINARY_DIR}/include-staging/${BACKEND_CXX_COMPILER_NAME_WITHOUT_PATH}_HEADERS/rose_edg_required_macros_and_functions.h")
set(_rose_gen_hdr "${CMAKE_BINARY_DIR}/include-staging/${BACKEND_CXX_COMPILER_NAME_WITHOUT_PATH}_HEADERS/rose_generated_builtin_functions.h")
if(EXISTS "${_rose_req_hdr}" AND EXISTS "${_rose_gen_hdr}")
  file(READ "${_rose_req_hdr}" _rose_req_text)
  file(READ "${_rose_gen_hdr}" _rose_gen_text)
  string(REGEX REPLACE "(REPLACE_ME_WITH_GENERATED_BUILTIN_FUNCTIONS[^\n]*\n)" "\\1${_rose_gen_text}" _rose_req_text "${_rose_req_text}")
  file(WRITE "${_rose_req_hdr}" "${_rose_req_text}")
else()
  message(WARNING "builtin prototypes not merged: ${_rose_req_hdr} or ${_rose_gen_hdr} missing")
endif()
