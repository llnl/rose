if(ENABLE_BINARY_ANALYSIS)
  add_library(roseBinaryAnalysisByteCodeClassHierarchy OBJECT
    CilCompatibility.C
    CilHierarchyAnalysis.C
    JvmCompatibility.C
    JvmHierarchyAnalysis.C)

  add_dependencies(roseBinaryAnalysisByteCodeClassHierarchy rosetta_generated)
endif()

install(FILES
  CilCompatibility.h
  CilHierarchyAnalysis.h
  JvmCompatibility.h
  JvmHierarchyAnalysis.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/Rose/BinaryAnalysis/ByteCode/ClassHierarchy)
