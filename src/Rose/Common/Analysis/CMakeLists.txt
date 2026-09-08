add_library(roseCommonAnalysis OBJECT
  AnalysisTypes.C
  ClassHierarchyAnalysis.C
  ClassHierarchyExport.C)

add_dependencies(roseCommonAnalysis rosetta_generated)

install(FILES
  AbstractCompatibilityBridge.h
  AnalysisTypes.h
  ClassHierarchyAnalysis.h
  ClassHierarchyExport.h
  Utility.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/Rose/Common/Analysis)
