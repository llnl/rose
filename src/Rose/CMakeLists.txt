# This header file is only for things in the ::Rose namespace. If
# you're adding something that's not in ::Rose you're doing it wrong!

# Namespaces within ::Rose (see top comment). Every namespace will
# have a directory listed here, and every directory here is a
# namespace.
add_subdirectory(AST)
add_subdirectory(BinaryAnalysis)
add_subdirectory(Common)
add_subdirectory(Color)
add_subdirectory(CommandLine)
add_subdirectory(Diagnostics)
add_subdirectory(FileSystem)
add_subdirectory(Sarif)
add_subdirectory(Source)
add_subdirectory(StringUtility)
add_subdirectory(Traits)

# needed to properly define ROSE_UTIL_API
add_definitions(-DROSE_UTIL_EXPORTS)

# .C files for the ::Rose namespace (see top comment)
add_library(roseNamespace OBJECT
  FormattedTable.C
  Initialize.C
  Logger.C
  Progress.C
  SourceLocation.C
  Yaml.C)

add_dependencies(roseNamespace rosetta_generated)

# Public header files for the ::Rose namespace (see top comment)
install(FILES
  Affirm.h
  As.h
  AST.h
  BasicTypes.h
  BinaryAnalysis.h
  BitFlags.h
  BitOps.h
  Color.h
  CommandLine.h
  Common.h
  Constants.h
  Diagnostics.h
  Exception.h
  FileSystem.h
  FormatRestorer.h
  FormattedTable.h
  GraphUtility.h
  Initialize.h
  Location.h
  Logger.h
  metaprog.h 
  ParallelSort.h
  Progress.h
  RecursionCounter.h
  Sarif.h
  Source.h
  SourceLocation.h
  StringUtility.h
  Yaml.h
  FlowGraphInterface.h
  FlowAnalysisInterface.h
  SawyerGraphConnection.h

  DESTINATION ${INCLUDE_INSTALL_DIR}/Rose)
