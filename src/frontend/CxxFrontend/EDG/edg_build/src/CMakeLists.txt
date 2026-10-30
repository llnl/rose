include_directories(
  ${ROSE_SOURCE_DIR}/src/frontend/CxxFrontend/EDG/EDG_${EDG_VERSION}/src
  ${ROSE_SOURCE_DIR}/src/frontend/CxxFrontend/EDG/edgRose)

# add_subdirectory(disp)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Ddisc_union=union")

#------------------------------------------------------------------------------
# error_codes.h
#------------------------------------------------------------------------------
add_custom_command(
  OUTPUT "err_codes.h" "err_data.h"
  COMMAND mk_errinfo
          "${CMAKE_CURRENT_SOURCE_DIR}/error_msg.txt"
          "${CMAKE_CURRENT_SOURCE_DIR}/error_tag.txt"
          "err_codes.h.tmp"
          "err_data.h.tmp"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different err_codes.h.tmp err_codes.h
  COMMAND ${CMAKE_COMMAND} -E copy_if_different err_data.h.tmp err_data.h
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

# "mk_errinfo" target is in misc/
add_custom_target(generate-error-codes DEPENDS "err_codes.h" mk_errinfo)

#------------------------------------------------------------------------------
# edg_internal
#------------------------------------------------------------------------------
add_library(edg_internal OBJECT
  attribute.cc
  class_decl.cc
  cmd_line.cc
  const_ints.cc
  cp_gen_be.cc
  debug.cc
  decl_inits.cc
  decl_spec.cc
  declarator.cc
  decls.cc
  def_arg.cc
  disambig.cc
  error.cc
  expr.cc
  exprutil.cc
  extasm.cc
  fe_init.cc
  fe_wrapup.cc
  fixed_pt.cc
  float_pt.cc
  folding.cc
  func_def.cc
  host_envir.cc
  ifc_modules.cc
  ifc_modules_templ.cc
  ifc_map_functions.cc
  ifc_map_functions_acc.cc
  ifc_map_functions_val.cc
  ifc_map_functions_dbg.cc
  il.cc
  il_alloc.cc
  il_display.cc
  il_read.cc
  il_to_str.cc
  il_walk.cc
  il_write.cc
  inline.cc
  interpret.cc
  layout.cc
  lexical.cc
  literals.cc
  lookup.cc
  lower_c99.cc
  lower_eh.cc
  lower_il.cc
  lower_init.cc
  lower_name.cc
  macro.cc
  mem_manage.cc
  modules.cc
  ms_attrib.cc
  overload.cc
  pch.cc
  pragma.cc
  preproc.cc
  scope_stk.cc
  src_seq.cc
  statements.cc
  symbol_ref.cc
  symbol_tbl.cc
  sys_predef.cc
  target.cc
  templates.cc
  trans_copy.cc
  trans_corresp.cc
  trans_unit.cc
  types.cc)

add_dependencies(edg_internal generate-error-codes generate-defines-dot-h)

# 2013-08-20: TODO: Add identical dependencies and flags from Makefile.am
#
#                      -DGCC_VERSION=@GCC_VERSION@
#                      -DGCC_MINOR_VERSION=@GCC_MINOR_VERSION@
#
  # Add header search path "-I${EDG}/src" for dynamically generated
  # header file "defines.h".
  #
#
target_include_directories(edg_internal PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

set_target_properties(
  edg_internal
  PROPERTIES COMPILE_FLAGS
  "-DMAKE_FRONT_END_CALLABLE=TRUE")

