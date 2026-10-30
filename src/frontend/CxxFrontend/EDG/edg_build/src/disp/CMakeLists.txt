
#------------------------------------------------------------------------------
# edgcpdisp
#------------------------------------------------------------------------------
add_executable(edgcpdisp
  const_ints.c
  debug.c
  error.c
  fe_init.c
  fixed_pt.c
  float_pt.c
  host_envir.c
  il.c
  il_display.c
  il_read.c
  il_to_str.c
  il_walk.c
  mem_manage.c
  target.c
  types.c)
add_dependencies(edgcpdisp generate-error-codes)

# DQ (12/24/2008): I don't see how using -DSTANDALONE_IL_DISPLAY would work
#                  (disallows definition of is_template_dependent_type()
#                  function)

  # Add header search path "-I${EDG}/src" for dynamically generated
  # header file "defines.h".
  #
target_include_directories(edgcpdisp PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/..)

set_target_properties(
  edgcpdisp
  PROPERTIES COMPILE_FLAGS
  "-DSTANDALONE_IL_DISPLAY -DIL_SHOULD_BE_WRITTEN_TO_FILE=TRUE")

install(TARGETS edgcpdisp DESTINATION bin)
