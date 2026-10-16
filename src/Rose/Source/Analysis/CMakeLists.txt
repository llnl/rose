
add_library(roseSourceAnalysis OBJECT
  Analyzer.C
  Visitor.C
  CppClassLayoutExport.C
  CppCompatibility.C
  CppHierarchyAnalysis.C
  CppLayoutAnalysis.C
  CppSpecialMemberAnalysis.C
  ExternalMangling.C
  ItaniumMangling.C)

add_dependencies(roseSourceAnalysis rosetta_generated)

install(FILES
  Analyzer.h Visitor.h
  CppClassLayoutExport.h
  CppCompatibility.h
  CppHierarchyAnalysis.h
  CppLayoutAnalysis.h
  CppSpecialMemberAnalysis.h
  ExternalMangling.h
  ItaniumMangling.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/Rose/Source/Analysis)
