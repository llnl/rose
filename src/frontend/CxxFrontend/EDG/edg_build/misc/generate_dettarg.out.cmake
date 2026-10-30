execute_process(
  COMMAND ${dettarg}
  OUTPUT_FILE ${output})

if (SIZEOF_LONG_LONG STREQUAL "")
  file(APPEND "${output}" "#define INTEGER_VALUE_REPR_IS_A_HOST_INTEGER 0")
endif()
