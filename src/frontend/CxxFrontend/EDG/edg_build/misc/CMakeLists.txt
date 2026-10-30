include_directories(
  ${ROSE_SOURCE_DIR}/src/frontend/CxxFrontend/EDG/EDG_${EDG_VERSION}/misc
  ${ROSE_SOURCE_DIR}/src/frontend/CxxFrontend/EDG/EDG_${EDG_VERSION}/src)

# Overview of this CMakeLists.txt:
#
#     1. Check if the type "long long" exists on this system.
#     2. Generate executable "dettarg", compiled with an appropriate
#        definition from step 1.
#     3. Generate "dettarg.out" using "dettarg".
#     4. Generate "defines.h" as the concatenation of "defines.h.rose"
#        and "dettarg.out".
#
#     5. Generate executable "mk_errinfo".

#------------------------------------------------------------------------------
# Check if "long long" exists.
#
# Defines "-DHAVE_LONG_LONG=[0,1]" for the compile-line of dettarg.
#------------------------------------------------------------------------------
include(CheckTypeSize)

set(detarg_compile_flags "")
if (SIZEOF_LONG_LONG STREQUAL "")
  set(detarg_compile_flags "-DHAVE_LONG_LONG=0")
else()
  set(detarg_compile_flags "-DHAVE_LONG_LONG=1")
endif()

#------------------------------------------------------------------------------
# dettarg
#------------------------------------------------------------------------------
add_executable(dettarg dettarg.cc)
set_target_properties(
  # Dynamically add the CFLAG definition determined above.
  dettarg
  PROPERTIES COMPILE_FLAGS "${detarg_compile_flags}")

#------------------------------------------------------------------------------
# dettarg.out
#------------------------------------------------------------------------------
# get_target_property(dettarg_location dettarg LOCATION)

add_custom_command(
  OUTPUT dettarg.out
  COMMAND ${CMAKE_COMMAND}
    -D dettarg=$<TARGET_FILE:dettarg>
    -D output=${CMAKE_CURRENT_BINARY_DIR}/dettarg.out
    -D SIZEOF_LONG_LONG=${SIZEOF_LONG_LONG}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/generate_dettarg.out.cmake
  DEPENDS dettarg)

#------------------------------------------------------------------------------
# generate-defines-dot-h -> defines.h
#
# Generates the file "defines.h" from the concatenation of "defines.h.rose"
# and "dettarg.out".
#------------------------------------------------------------------------------
set(defines_h1 "${CMAKE_CURRENT_SOURCE_DIR}/../src/defines.h.rose")

add_custom_command(
  OUTPUT "defines.h"
  COMMAND ${CMAKE_COMMAND}
    -D file1=${defines_h1}
    -D file2=${CMAKE_CURRENT_BINARY_DIR}/dettarg.out
    -D output=${CMAKE_CURRENT_BINARY_DIR}/../src/defines.h
    -P ${CMAKE_CURRENT_SOURCE_DIR}/concat.cmake
  DEPENDS dettarg.out)

add_custom_target(generate-defines-dot-h DEPENDS "defines.h" "dettarg.out")

#------------------------------------------------------------------------------
# mk_errinfo
#------------------------------------------------------------------------------
add_executable(mk_errinfo mk_errinfo.cc)
add_dependencies(mk_errinfo generate-defines-dot-h)
  # Add header search path "-I${EDG}/src" for dynamically generated
  # header file "defines.h".
  #
target_include_directories(mk_errinfo PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/../src)
#set_target_properties(
#  mk_errinfo
#  PROPERTIES COMPILE_FLAGS
#  "-I${CMAKE_CURRENT_BINARY_DIR}/../src")

