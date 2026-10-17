# Build the EDG C/C++ front end from the edg_compiler submodule (upstream layout) as the
# object library "edg_internal", configured for ROSE.  Makefile.am in this directory is the
# autotools twin and documents the stages; keep the source list below in sync with it.
#
# EDG's sources are *.c files compiled as C++ (EDG builds them with "g++ -std=c++14 *.c").
# EDG's src/defines.h includes the generated cmake_defines.h when USE_CMAKE_DEFINES is set;
# ROSE generates that file from defines.h.rose plus the output of EDG's dettarg utility.

set(EDG_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../edg_compiler")

if(NOT EXISTS "${EDG_SOURCE_DIR}/src/basics.h")
  message(FATAL_ERROR
    "EDG source not found in ${EDG_SOURCE_DIR}.\n"
    "Run: git submodule update --init src/frontend/CxxFrontend/EDG/edg_compiler")
endif()

# EDG's "disc_union" macro: IL discriminated unions as real unions, or as structs for
# easier debugging (USE_ROSE_EDG_DEBUGGING_SUPPORT, handled in ../CMakeLists.txt).
if(NOT DEFINED EDG_DISC_UNION)
  set(EDG_DISC_UNION "union")
endif()

#------------------------------------------------------------------------------------------
# Sources: every front-end file in src/ except the stand-alone drivers cfe.c and c_gen_be.c,
# and floating.c (software floating point for hosts without native FP).
#------------------------------------------------------------------------------------------
set(EDG_FRONTEND_SOURCES
  attribute.c
  cfe_daemon.c
  class_decl.c
  cmd_line.c
  const_ints.c
  cp_gen_be.c
  debug.c
  decl_inits.c
  decl_spec.c
  declarator.c
  decls.c
  def_arg.c
  disambig.c
  error.c
  expr.c
  exprutil.c
  extasm.c
  fe_init.c
  fe_wrapup.c
  fixed_pt.c
  float_pt.c
  folding.c
  func_def.c
  host_envir.c
  ifc_modules.c
  ifc_modules_templ.c
  ifc_map_functions.c
  ifc_map_functions_acc.c
  ifc_map_functions_val.c
  ifc_map_functions_dbg.c
  ifc_map_functions_mut.c
  ifc_modules_read.c
  ifc_modules_write.c
  il.c
  il_alloc.c
  il_display.c
  il_read.c
  il_to_str.c
  il_walk.c
  il_write.c
  inline.c
  interpret.c
  layout.c
  lexical.c
  literals.c
  lookup.c
  lower_c99.c
  lower_eh.c
  lower_il.c
  lower_init.c
  lower_name.c
  macro.c
  mem_manage.c
  modules.c
  ms_attrib.c
  overload.c
  pch.c
  pragma.c
  preproc.c
  scope_stk.c
  src_seq.c
  statements.c
  symbol_ref.c
  symbol_tbl.c
  sys_predef.c
  target.c
  templates.c
  trans_copy.c
  trans_corresp.c
  trans_unit.c
  types.c
  unicode_name_fsm.c)
list(TRANSFORM EDG_FRONTEND_SOURCES PREPEND "${EDG_SOURCE_DIR}/src/")

set(EDG_DETTARG_SOURCE    "${EDG_SOURCE_DIR}/util/dettarg.c")
set(EDG_MK_ERRINFO_SOURCE "${EDG_SOURCE_DIR}/util/mk_errinfo.c")

# All of EDG's .c files are C++.
set_source_files_properties(${EDG_FRONTEND_SOURCES} ${EDG_DETTARG_SOURCE} ${EDG_MK_ERRINFO_SOURCE}
  PROPERTIES LANGUAGE CXX)

#------------------------------------------------------------------------------------------
# 1. Does the host have "long long"?  (SIZEOF_LONG_LONG comes from cmake/ConfigureChecks.cmake)
#------------------------------------------------------------------------------------------
if(SIZEOF_LONG_LONG STREQUAL "")
  set(edg_have_long_long 0)
else()
  set(edg_have_long_long 1)
endif()

#------------------------------------------------------------------------------------------
# 2./3. dettarg -> dettarg.out  (target description of the host/backend compiler)
#------------------------------------------------------------------------------------------
add_executable(dettarg ${EDG_DETTARG_SOURCE})
target_compile_definitions(dettarg PRIVATE HAVE_LONG_LONG=${edg_have_long_long})
# ROSE's dettarg patch includes rose_config.h from the top of the build tree.
target_include_directories(dettarg PRIVATE ${CMAKE_BINARY_DIR})

add_custom_command(
  OUTPUT dettarg.out
  COMMAND ${CMAKE_COMMAND}
    -D dettarg=$<TARGET_FILE:dettarg>
    -D output=${CMAKE_CURRENT_BINARY_DIR}/dettarg.out
    -D SIZEOF_LONG_LONG=${SIZEOF_LONG_LONG}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/generate_dettarg.out.cmake
  DEPENDS dettarg ${CMAKE_CURRENT_SOURCE_DIR}/generate_dettarg.out.cmake)

#------------------------------------------------------------------------------------------
# 4. cmake_defines.h = defines.h.rose + dettarg.out
#------------------------------------------------------------------------------------------
add_custom_command(
  OUTPUT cmake_defines.h
  COMMAND ${CMAKE_COMMAND}
    -D file1=${CMAKE_CURRENT_SOURCE_DIR}/defines.h.rose
    -D file2=${CMAKE_CURRENT_BINARY_DIR}/dettarg.out
    -D output=${CMAKE_CURRENT_BINARY_DIR}/cmake_defines.h
    -P ${CMAKE_CURRENT_SOURCE_DIR}/concat.cmake
  DEPENDS dettarg.out ${CMAKE_CURRENT_SOURCE_DIR}/defines.h.rose ${CMAKE_CURRENT_SOURCE_DIR}/concat.cmake)

add_custom_target(generate-defines-dot-h DEPENDS cmake_defines.h)

#------------------------------------------------------------------------------------------
# 5./6. mk_errinfo -> err_codes.h, err_data.h
#------------------------------------------------------------------------------------------
add_executable(mk_errinfo ${EDG_MK_ERRINFO_SOURCE})
target_compile_definitions(mk_errinfo PRIVATE USE_CMAKE_DEFINES disc_union=${EDG_DISC_UNION})
target_include_directories(mk_errinfo PRIVATE
  ${CMAKE_CURRENT_BINARY_DIR}      # cmake_defines.h
  ${EDG_SOURCE_DIR}/src
  ${CMAKE_BINARY_DIR})             # rose_config.h (included by defines.h.rose)
add_dependencies(mk_errinfo generate-defines-dot-h)

add_custom_command(
  OUTPUT err_codes.h err_data.h
  COMMAND mk_errinfo
          ${EDG_SOURCE_DIR}/src/error_msg.txt
          ${EDG_SOURCE_DIR}/src/error_tag.txt
          err_codes.h.tmp
          err_data.h.tmp
  COMMAND ${CMAKE_COMMAND} -E copy_if_different err_codes.h.tmp err_codes.h
  COMMAND ${CMAKE_COMMAND} -E copy_if_different err_data.h.tmp err_data.h
  DEPENDS mk_errinfo ${EDG_SOURCE_DIR}/src/error_msg.txt ${EDG_SOURCE_DIR}/src/error_tag.txt
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_custom_target(generate-error-codes DEPENDS err_codes.h err_data.h)

#------------------------------------------------------------------------------------------
# 7. edg_internal
#------------------------------------------------------------------------------------------
add_library(edg_internal OBJECT ${EDG_FRONTEND_SOURCES})
add_dependencies(edg_internal generate-error-codes generate-defines-dot-h)

target_compile_definitions(edg_internal PRIVATE
  USE_CMAKE_DEFINES
  MAKE_FRONT_END_CALLABLE=TRUE
  disc_union=${EDG_DISC_UNION})

target_include_directories(edg_internal
  PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}    # cmake_defines.h, err_codes.h, err_data.h
    ${EDG_SOURCE_DIR}/src
  PRIVATE
    ${CMAKE_BINARY_DIR})           # rose_config.h

# The objects end up in the shared library RoseSourceCxxFrontendEdg.
set_target_properties(edg_internal PROPERTIES POSITION_INDEPENDENT_CODE ON)
