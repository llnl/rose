add_subdirectory(ClassHierarchy)

add_library(roseBinaryAnalysisByteCode OBJECT
  Analysis.C
  Cil.C
  Jvm.C
  )

add_dependencies(roseBinaryAnalysisByteCode rosetta_generated)

install(FILES
  Analysis.h
  Cil.h
  ClassHierarchy.h
  Jvm.h

  DESTINATION ${INCLUDE_INSTALL_DIR}/Rose/BinaryAnalysis/ByteCode)
