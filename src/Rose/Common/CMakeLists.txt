# Common infrastructure shared by source and binary analyses.
add_subdirectory(Analysis)

install(FILES
  Analysis.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/Rose/Common)
