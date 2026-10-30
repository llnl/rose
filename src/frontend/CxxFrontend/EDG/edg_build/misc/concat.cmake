file(READ ${file1} contents1)
file(READ ${file2} contents2)
file(WRITE "${output}.tmp" ${contents1} ${contents2})
execute_process(
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
  "${output}.tmp" "${output}")
