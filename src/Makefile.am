include $(top_srcdir)/config/Makefile.for.ROSE.includes.and.libs
bin_PROGRAMS =
bin_SCRIPTS =
pkgdata_DATA =
MOSTLYCLEANFILES =

if ROSE_USE_INTERNAL_FRONTEND_DEVELOPMENT
SUBDIRS = util Sawyer Rosebud ROSETTA roseSupport Rose frontend midend backend \
          3rdPartyLibraries roseIndependentSupport
else
SUBDIRS = util Sawyer Rosebud ROSETTA roseSupport Rose frontend midend backend \
          3rdPartyLibraries roseIndependentSupport roseExtensions
endif

SUBDIRS += generated AstNodes

# This lists a large number of directories.  If "make install"
# is run then we only need a single directory
AM_CPPFLAGS = -Winvalid-pch $(ROSE_INCLUDES)

if ROSE_BUILD_CXX_LANGUAGE_SUPPORT
if ROSE_USE_CLANG_FRONTEND
   libroseCxxlibraries = frontend/CxxFrontend/Clang/libroseClangFrontend.la
else
   libroseCxxlibraries = frontend/CxxFrontend/EDG/libroseEDG.la
endif
else
   libroseCxxlibraries =
endif

if ROSE_BUILD_FORTRAN_LANGUAGE_SUPPORT
   libroseOpenFortranParserlibraries = frontend/OpenFortranParser_SAGE_Connection/libroseFortran.la
   pkgdata_DATA += $(top_srcdir)/src/3rdPartyLibraries/fortran-parser/iso_c_binding.rmod
   pkgdata_DATA += $(top_srcdir)/src/3rdPartyLibraries/fortran-parser/openacc.rmod
else
   libroseOpenFortranParserlibraries =
endif

# Added support for Fortran front-end development using the flang (F18) compiler [Rasmussen 8/12/2019]
if ROSE_EXPERIMENTAL_FLANG_ROSE_CONNECTION
   libroseExperimentalFlangParserlibraries = \
        frontend/Experimental_General_Language_Support/libexperimentalRoseGeneralLanguage.la \
        frontend/Experimental_Flang_ROSE_Connection/libexperimentalRoseFlang.la \
        3rdPartyLibraries/flang-parser/libFlangParser.la
else
   libroseExperimentalFlangParserlibraries =
endif

if ROSE_BUILD_ATERM_SUPPORT
   libroseExperimentalGeneralLanguagelibraries = \
        frontend/Experimental_General_Language_Support/libexperimentalRoseGeneralLanguage.la \
        frontend/Experimental_General_Language_Support/ATerm/libexperimentalRoseATerm.la
   pkgdata_DATA += $(top_srcdir)/src/3rdPartyLibraries/jovial-parser/share/rose/Jovial.tbl
else
   libroseExperimentalGeneralLanguagelibraries =
endif

if ROSE_BUILD_JAVA_LANGUAGE_SUPPORT
libroseJavaParserlibraries = frontend/ECJ_ROSE_Connection/libroseJava.la
else
libroseJavaParserlibraries =
endif

if ROSE_BUILD_PHP_LANGUAGE_SUPPORT
   librosePHPlibraries = frontend/PHPFrontend/librosePHPFrontend.la
else
   librosePHPlibraries =
endif

if ROSE_BUILD_PYTHON_LANGUAGE_SUPPORT
   librosePythonlibraries = frontend/PythonFrontend/librosePythonFrontend.la
else
   librosePythonlibraries =
endif

if ROSE_EXPERIMENTAL_CSHARP_ROSE_CONNECTION
   libroseCsharplibraries = frontend/Experimental_Csharp_ROSE_Connection/libexperimentalRoseCsharp.la
else
   libroseCsharplibraries =
endif

if ROSE_EXPERIMENTAL_ADA_ROSE_CONNECTION
   libroseAdalibraries = frontend/Experimental_Ada_ROSE_Connection/libexperimentalRoseAda.la
else
# Ada is not enabled
   libroseAdalibraries =
endif

if ROSE_EXPERIMENTAL_LIBADALANG_ROSE_CONNECTION
   libroseLibadalanglibraries = frontend/Experimental_Libadalang_ROSE_Connection/libexperimentalRoseAda.la
else
   libroseLibadalanglibraries =
endif

if ROSE_EXPERIMENTAL_JOVIAL_ROSE_CONNECTION
   libroseJoviallibraries = frontend/Experimental_Jovial_ROSE_Connection/libexperimentalRoseJovial.la
else
   libroseJoviallibraries =
endif

if ROSE_EXPERIMENTAL_MATLAB_ROSE_CONNECTION
   libroseMatlablibraries = frontend/Experimental_Matlab_ROSE_Connection/libexperimentalRoseMatlab.la
else
   libroseMatlablibraries =
endif

if ROSE_BUILD_BINARY_ANALYSIS_SUPPORT
   libroseBinaryLibraries =							\
	$(top_builddir)/src/frontend/BinaryFormats/libroseBinaryFormats.la	\
	$(top_builddir)/src/frontend/Disassemblers/libroseDisassemblers.la	\
	$(top_builddir)/src/midend/BinaryAnalysis/libbinaryMidend.la
else
   libroseBinaryLibraries =							\
	$(top_builddir)/src/frontend/BinaryFormats/libroseBinaryFormats.la	\
	$(top_builddir)/src/midend/BinaryAnalysis/libbinaryMidend.la		\
	$(top_builddir)/src/frontend/Disassemblers/libroseDisassemblers.la
endif

if ROSE_USE_MYSQL
  OPT_MYSQL_LDFLAGS = $(MYSQL_LDFLAGS)
else
  OPT_MYSQL_LDFLAGS =
endif

if ROSE_USE_SQLITE_DATABASE
   libroseSqlite3xConnection = \
		$(top_builddir)/src/roseExtensions/sqlite3x/libRoseSQLite3xDatabase.la
   OPT_SQLITE3_LDFLAGS       = $(SQLITE3_LDFLAGS)

else
   libroseSqlite3xConnection =
   OPT_SQLITE3_LDFLAGS =
endif

if ROSE_USE_INTERNAL_FRONTEND_DEVELOPMENT
libroseLibraries = \
   $(top_builddir)/src/3rdPartyLibraries/MSTL/libmstl.la \
   $(top_builddir)/src/backend/libbackend.la \
   $(top_builddir)/src/frontend/SageIII/libsage3.la \
   $(libroseCxxlibraries) \
   $(top_builddir)/src/midend/libmidend.la \
   $(top_builddir)/src/roseSupport/libroseSupport.la \
   $(top_builddir)/src/util/support/libsupport.la \
   $(top_builddir)/src/util/graphs/libgraphs.la \
   $(top_builddir)/src/util/libroseutil.la \
   $(top_builddir)/src/util/libroseSaywer.la \
   $(libroseBinaryLibraries)

else
libroseLibraries = \
   $(top_builddir)/src/3rdPartyLibraries/MSTL/libmstl.la \
   $(top_builddir)/src/backend/libbackend.la \
   $(top_builddir)/src/frontend/SageIII/libsage3.la \
   $(top_builddir)/src/generated/BinaryAnalysis/libroseGeneratedBinaryAnalysis.la \
   $(top_builddir)/src/generated/Jovial/libroseGeneratedJovial.la \
   $(top_builddir)/src/generated/Rose/Sarif/libSrcGeneratedRoseSarif.la \
   $(libroseCxxlibraries) \
   $(libroseOpenFortranParserlibraries) \
   $(libroseExperimentalFlangParserlibraries) \
   $(libroseExperimentalGeneralLanguagelibraries) \
   $(libroseJavaParserlibraries) \
   $(librosePHPlibraries) \
   $(librosePythonlibraries) \
   $(libroseBinaryLibraries) \
   $(libroseCsharplibraries) \
   $(libroseAdalibraries) \
   $(libroseLibadalanglibraries) \
   $(libroseJoviallibraries) \
   $(top_builddir)/src/midend/programAnalysis/libprogramAnalysis.la \
   $(top_builddir)/src/midend/programTransformation/loopProcessing/libLoopProcessing.la \
   $(top_builddir)/src/midend/programTransformation/extractFunctionArgumentsNormalization/libExtractFunctionArgumentsNormalization.la \
   $(top_builddir)/src/midend/programTransformation/singleStatementToBlockNormalization/libSingleStatementToBlockNormalization.la \
   $(top_builddir)/src/midend/libmidend.la \
   $(top_builddir)/src/roseSupport/libroseSupport.la \
   $(top_builddir)/src/util/support/libsupport.la \
   $(top_builddir)/src/util/graphs/libgraphs.la \
   $(top_builddir)/src/util/libroseutil.la \
   $(top_builddir)/src/Sawyer/libroseSawyer.la \
   $(top_builddir)/src/roseExtensions/failSafe/libfailsafe.la \
   $(top_builddir)/src/Rose/librose_partial.la \
   $(top_builddir)/src/AstNodes/BinaryAnalysis/libroseAstNodesBinaryAnalysis.la \
   $(top_builddir)/src/AstNodes/Expression/libroseAstNodesExpression.la \
   $(libroseSqlite3xConnection)

endif

EXTRA_DIST = CMakeLists.txt extra.docs

# Header files that need to be included in the distribution
pkginclude_HEADERS = \
     featureTests.h \
     RoseFirst.h \
     rose.h \
     roseInternal.h \
     rose_msvc.h \
     msvc_stdint.h \
     roseSupport/utility_functionsImpl.C \
     midend/astProcessing/AstDOTGeneration.h \
     midend/astProcessing/AstDOTGenerationImpl.C \
     frontend/SageIII/sage3basic.h \
     frontend/SageIII/sage3basic.hhh \
     midend/astDiagnostics/AstConsistencyTests.h \
     backend/asmUnparser/AsmUnparser_compat.h

# Header files organized by namespace. If the directory part of the name doesn't match
# a C++ namespace, then DO NOT put the header file in this list.
nobase_pkginclude_HEADERS=								\
	Rose/Affirm.h									\
	Rose/As.h									\
	Rose/AST.h								\
	Rose/AST/NodeId.h								\
	Rose/AST/Traversal.h								\
	Rose/AST/Utility.h								\
	Rose/BasicTypes.h								\
	Rose/BinaryAnalysis.h								\
	Rose/BinaryAnalysis/AbstractLocation.h						\
	Rose/BinaryAnalysis/Address.h							\
	Rose/BinaryAnalysis/AddressInterval.h						\
	Rose/BinaryAnalysis/AddressIntervalSet.h					\
	Rose/BinaryAnalysis/AddressSet.h						\
	Rose/BinaryAnalysis/Alignment.h							\
	Rose/BinaryAnalysis/Architecture.h						\
	Rose/BinaryAnalysis/Architecture/Amd64.h					\
	Rose/BinaryAnalysis/Architecture/ArmAarch32.h					\
	Rose/BinaryAnalysis/Architecture/ArmAarch64.h					\
	Rose/BinaryAnalysis/Architecture/Base.h						\
	Rose/BinaryAnalysis/Architecture/BasicTypes.h					\
	Rose/BinaryAnalysis/Architecture/Cil.h						\
	Rose/BinaryAnalysis/Architecture/Exception.h					\
	Rose/BinaryAnalysis/Architecture/Intel80286.h					\
	Rose/BinaryAnalysis/Architecture/Intel8086.h					\
	Rose/BinaryAnalysis/Architecture/Intel8088.h					\
	Rose/BinaryAnalysis/Architecture/IntelI386.h					\
	Rose/BinaryAnalysis/Architecture/IntelI486.h					\
	Rose/BinaryAnalysis/Architecture/IntelPentium.h					\
	Rose/BinaryAnalysis/Architecture/IntelPentiumii.h				\
	Rose/BinaryAnalysis/Architecture/IntelPentiumiii.h				\
	Rose/BinaryAnalysis/Architecture/IntelPentium4.h				\
	Rose/BinaryAnalysis/Architecture/Jvm.h						\
	Rose/BinaryAnalysis/Architecture/Mips32.h					\
	Rose/BinaryAnalysis/Architecture/Motorola.h					\
	Rose/BinaryAnalysis/Architecture/Motorola68040.h				\
	Rose/BinaryAnalysis/Architecture/NxpColdfire.h					\
	Rose/BinaryAnalysis/Architecture/Powerpc.h					\
	Rose/BinaryAnalysis/Architecture/Powerpc32.h					\
	Rose/BinaryAnalysis/Architecture/Powerpc64.h					\
	Rose/BinaryAnalysis/Architecture/X86.h						\
	Rose/BinaryAnalysis/AsmFunctionIndex.h						\
	Rose/BinaryAnalysis/AstHasher.h							\
	Rose/BinaryAnalysis/BasicTypes.h						\
	Rose/BinaryAnalysis/BestMapAddress.h						\
	Rose/BinaryAnalysis/BinaryLoader.h						\
	Rose/BinaryAnalysis/BinaryLoaderElf.h						\
	Rose/BinaryAnalysis/BinaryLoaderElfObj.h					\
	Rose/BinaryAnalysis/BinaryLoaderPe.h						\
	Rose/BinaryAnalysis/ByteCode.h							\
	Rose/BinaryAnalysis/ByteCode/Analysis.h						\
	Rose/BinaryAnalysis/ByteCode/Cil.h						\
	Rose/BinaryAnalysis/ByteCode/ClassHierarchy.h					\
	Rose/BinaryAnalysis/ByteCode/ClassHierarchy/CilCompatibility.h		\
	Rose/BinaryAnalysis/ByteCode/ClassHierarchy/CilHierarchyAnalysis.h		\
	Rose/BinaryAnalysis/ByteCode/ClassHierarchy/JvmCompatibility.h		\
	Rose/BinaryAnalysis/ByteCode/ClassHierarchy/JvmHierarchyAnalysis.h		\
	Rose/BinaryAnalysis/ByteCode/Jvm.h						\
	Rose/BinaryAnalysis/ByteOrder.h							\
	Rose/BinaryAnalysis/CallingConvention.h						\
	Rose/BinaryAnalysis/CallingConvention/Analysis.h				\
	Rose/BinaryAnalysis/CallingConvention/BasicTypes.h				\
	Rose/BinaryAnalysis/CallingConvention/Declaration.h				\
	Rose/BinaryAnalysis/CallingConvention/Definition.h				\
	Rose/BinaryAnalysis/CallingConvention/Exception.h				\
	Rose/BinaryAnalysis/CallingConvention/StoragePool.h				\
	Rose/BinaryAnalysis/CilStringFinder.h						\
	Rose/BinaryAnalysis/CodeInserter.h						\
	Rose/BinaryAnalysis/Concolic.h							\
	Rose/BinaryAnalysis/ConcreteLocation.h						\
	Rose/BinaryAnalysis/Concolic/Architecture.h					\
	Rose/BinaryAnalysis/Concolic/BasicTypes.h					\
	Rose/BinaryAnalysis/Concolic/Callback.h						\
	Rose/BinaryAnalysis/Concolic/Callback/BasicTypes.h				\
	Rose/BinaryAnalysis/Concolic/Callback/MemoryExit.h				\
	Rose/BinaryAnalysis/Concolic/Callback/MemoryInput.h				\
	Rose/BinaryAnalysis/Concolic/Callback/MemoryTime.h				\
	Rose/BinaryAnalysis/Concolic/ConcolicExecutor.h					\
	Rose/BinaryAnalysis/Concolic/ConcreteExecutor.h					\
	Rose/BinaryAnalysis/Concolic/ConcreteResult.h					\
	Rose/BinaryAnalysis/Concolic/Database.h						\
	Rose/BinaryAnalysis/Concolic/Emulation.h					\
	Rose/BinaryAnalysis/Concolic/ExecutionEvent.h					\
	Rose/BinaryAnalysis/Concolic/ExecutionLocation.h				\
	Rose/BinaryAnalysis/Concolic/ExecutionManager.h					\
	Rose/BinaryAnalysis/Concolic/I386Linux.h					\
	Rose/BinaryAnalysis/Concolic/I386Linux/Architecture.h				\
	Rose/BinaryAnalysis/Concolic/I386Linux/BasicTypes.h				\
	Rose/BinaryAnalysis/Concolic/I386Linux/ExitStatusExecutor.h			\
	Rose/BinaryAnalysis/Concolic/I386Linux/ExitStatusResult.h			\
	Rose/BinaryAnalysis/Concolic/I386Linux/TracingExecutor.h			\
	Rose/BinaryAnalysis/Concolic/I386Linux/TracingResult.h				\
	Rose/BinaryAnalysis/Concolic/InputVariables.h					\
	Rose/BinaryAnalysis/Concolic/M68kSystem.h					\
	Rose/BinaryAnalysis/Concolic/M68kSystem/Architecture.h				\
	Rose/BinaryAnalysis/Concolic/M68kSystem/BasicTypes.h				\
	Rose/BinaryAnalysis/Concolic/M68kSystem/TracingExecutor.h			\
	Rose/BinaryAnalysis/Concolic/M68kSystem/TracingResult.h				\
	Rose/BinaryAnalysis/Concolic/Settings.h						\
	Rose/BinaryAnalysis/Concolic/SharedMemory.h					\
	Rose/BinaryAnalysis/Concolic/Specimen.h						\
	Rose/BinaryAnalysis/Concolic/SystemCall.h					\
	Rose/BinaryAnalysis/Concolic/TestCase.h						\
	Rose/BinaryAnalysis/Concolic/TestSuite.h					\
	Rose/BinaryAnalysis/ControlFlow.h						\
	Rose/BinaryAnalysis/DataDependence.h						\
	Rose/BinaryAnalysis/DataFlow.h							\
	Rose/BinaryAnalysis/Debugger.h							\
	Rose/BinaryAnalysis/Debugger/Base.h						\
	Rose/BinaryAnalysis/Debugger/BasicTypes.h					\
	Rose/BinaryAnalysis/Debugger/Exception.h					\
	Rose/BinaryAnalysis/Debugger/Gdb.h						\
	Rose/BinaryAnalysis/Debugger/GdbResponse.h					\
	Rose/BinaryAnalysis/Debugger/Linux.h						\
	Rose/BinaryAnalysis/Debugger/ThreadId.h						\
	Rose/BinaryAnalysis/Demangler.h							\
	Rose/BinaryAnalysis/Disassembler.h						\
	Rose/BinaryAnalysis/Disassembler/Aarch32.h					\
	Rose/BinaryAnalysis/Disassembler/Aarch64.h					\
	Rose/BinaryAnalysis/Disassembler/Base.h						\
	Rose/BinaryAnalysis/Disassembler/BasicTypes.h					\
	Rose/BinaryAnalysis/Disassembler/Exception.h					\
	Rose/BinaryAnalysis/Disassembler/Cil.h						\
	Rose/BinaryAnalysis/Disassembler/Jvm.h						\
	Rose/BinaryAnalysis/Disassembler/M68k.h						\
	Rose/BinaryAnalysis/Disassembler/Mips.h						\
	Rose/BinaryAnalysis/Disassembler/Null.h						\
	Rose/BinaryAnalysis/Disassembler/Powerpc.h					\
	Rose/BinaryAnalysis/Disassembler/X86.h						\
	Rose/BinaryAnalysis/Dwarf.h                                                     \
	Rose/BinaryAnalysis/Dwarf/BasicTypes.h                                          \
	Rose/BinaryAnalysis/Dwarf/Constants.h                                           \
	Rose/BinaryAnalysis/Dwarf/Exception.h                                           \
	Rose/BinaryAnalysis/FeasiblePath.h						\
	Rose/BinaryAnalysis/FunctionCall.h						\
	Rose/BinaryAnalysis/FunctionSimilarity.h					\
	Rose/BinaryAnalysis/Hexdump.h							\
	Rose/BinaryAnalysis/HotPatch.h							\
	Rose/BinaryAnalysis/InstructionEnumsAarch32.h					\
	Rose/BinaryAnalysis/InstructionEnumsAarch64.h					\
	Rose/BinaryAnalysis/InstructionEnumsCil.h					\
	Rose/BinaryAnalysis/InstructionEnumsJvm.h					\
	Rose/BinaryAnalysis/InstructionEnumsM68k.h					\
	Rose/BinaryAnalysis/InstructionEnumsMips.h					\
	Rose/BinaryAnalysis/InstructionEnumsPowerpc.h					\
	Rose/BinaryAnalysis/InstructionEnumsX86.h					\
	Rose/BinaryAnalysis/InstructionMap.h						\
	Rose/BinaryAnalysis/InstructionProvider.h					\
	Rose/BinaryAnalysis/InstructionSemantics.h					\
	Rose/BinaryAnalysis/InstructionSemantics/BaseSemantics/AddressSpace.h		\
	Rose/BinaryAnalysis/InstructionSemantics/BaseSemantics/BasicTypes.h		\
	Rose/BinaryAnalysis/InstructionSemantics/BaseSemantics/Dispatcher.h		\
	Rose/BinaryAnalysis/InstructionSemantics/BaseSemantics/Exception.h		\
	Rose/BinaryAnalysis/InstructionSemantics/BaseSemantics/Formatter.h		\
	Rose/BinaryAnalysis/InstructionSemantics/BaseSemantics/FrameState.h		\
	Rose/BinaryAnalysis/InstructionSemantics/BaseSemantics/MemoryCell.h		\
	Rose/BinaryAnalysis/InstructionSemantics/BaseSemantics/MemoryCellList.h		\
	Rose/BinaryAnalysis/InstructionSemantics/BaseSemantics/MemoryCellMap.h		\
	Rose/BinaryAnalysis/InstructionSemantics/BaseSemantics/MemoryCellState.h	\
	Rose/BinaryAnalysis/InstructionSemantics/BaseSemantics/MemoryState.h		\
	Rose/BinaryAnalysis/InstructionSemantics/BaseSemantics/Merger.h			\
	Rose/BinaryAnalysis/InstructionSemantics/BaseSemantics/RegisterState.h		\
	Rose/BinaryAnalysis/InstructionSemantics/BaseSemantics/RegisterStateGeneric.h	\
	Rose/BinaryAnalysis/InstructionSemantics/BaseSemantics/RiscOperators.h		\
	Rose/BinaryAnalysis/InstructionSemantics/BaseSemantics/State.h			\
	Rose/BinaryAnalysis/InstructionSemantics/BaseSemantics/SValue.h			\
	Rose/BinaryAnalysis/InstructionSemantics/BaseSemantics/SymbolicMemory.h		\
	Rose/BinaryAnalysis/InstructionSemantics/BaseSemantics.h			\
	Rose/BinaryAnalysis/InstructionSemantics/ConcreteSemantics.h			\
	Rose/BinaryAnalysis/InstructionSemantics/ConcreteSemantics.tpp			\
	Rose/BinaryAnalysis/InstructionSemantics/DataFlowSemantics.h			\
	Rose/BinaryAnalysis/InstructionSemantics/DescriptorParser.h			\
	Rose/BinaryAnalysis/InstructionSemantics/DispatcherAarch32.h			\
	Rose/BinaryAnalysis/InstructionSemantics/DispatcherAarch64.h			\
	Rose/BinaryAnalysis/InstructionSemantics/DispatcherCil.h                        \
	Rose/BinaryAnalysis/InstructionSemantics/DispatcherJvm.h			\
	Rose/BinaryAnalysis/InstructionSemantics/DispatcherM68k.h			\
	Rose/BinaryAnalysis/InstructionSemantics/DispatcherMips.h			\
	Rose/BinaryAnalysis/InstructionSemantics/DispatcherPowerpc.h			\
	Rose/BinaryAnalysis/InstructionSemantics/DispatcherX86.h			\
	Rose/BinaryAnalysis/InstructionSemantics/IntervalSemantics.h			\
	Rose/BinaryAnalysis/InstructionSemantics/LlvmSemantics.h			\
	Rose/BinaryAnalysis/InstructionSemantics/MultiSemantics.h			\
	Rose/BinaryAnalysis/InstructionSemantics/NativeSemantics.h			\
	Rose/BinaryAnalysis/InstructionSemantics/NullSemantics.h			\
	Rose/BinaryAnalysis/InstructionSemantics/PartialSymbolicSemantics.h		\
	Rose/BinaryAnalysis/InstructionSemantics/SourceAstSemantics.h			\
	Rose/BinaryAnalysis/InstructionSemantics/StaticSemantics.h			\
	Rose/BinaryAnalysis/InstructionSemantics/SymbolicSemantics.h			\
	Rose/BinaryAnalysis/InstructionSemantics/SymbolicSemantics.tpp			\
	Rose/BinaryAnalysis/InstructionSemantics/TestSemantics.h			\
	Rose/BinaryAnalysis/InstructionSemantics/TaintSemantics.h			\
	Rose/BinaryAnalysis/InstructionSemantics/TraceSemantics.h			\
	Rose/BinaryAnalysis/InstructionSemantics/Utility.h				\
	Rose/BinaryAnalysis/LibraryIdentification.h					\
	Rose/BinaryAnalysis/MagicNumber.h						\
	Rose/BinaryAnalysis/Matrix.h							\
	Rose/BinaryAnalysis/MemoryMap.h							\
	Rose/BinaryAnalysis/ModelChecker.h						\
	Rose/BinaryAnalysis/ModelChecker/BasicBlockUnit.h				\
	Rose/BinaryAnalysis/ModelChecker/BasicTypes.h					\
	Rose/BinaryAnalysis/ModelChecker/Engine.h					\
	Rose/BinaryAnalysis/ModelChecker/ErrorTag.h					\
	Rose/BinaryAnalysis/ModelChecker/Exception.h					\
	Rose/BinaryAnalysis/ModelChecker/ExecutionUnit.h				\
	Rose/BinaryAnalysis/ModelChecker/ExternalFunctionUnit.h				\
	Rose/BinaryAnalysis/ModelChecker/FailureUnit.h					\
	Rose/BinaryAnalysis/ModelChecker/InstructionUnit.h				\
	Rose/BinaryAnalysis/ModelChecker/NullDereferenceTag.h				\
	Rose/BinaryAnalysis/ModelChecker/OutOfBoundsTag.h				\
	Rose/BinaryAnalysis/ModelChecker/PartitionerModel.h				\
	Rose/BinaryAnalysis/ModelChecker/Path.h						\
	Rose/BinaryAnalysis/ModelChecker/PathNode.h					\
	Rose/BinaryAnalysis/ModelChecker/PathPredicate.h				\
	Rose/BinaryAnalysis/ModelChecker/PathPrioritizer.h				\
	Rose/BinaryAnalysis/ModelChecker/PathQueue.h					\
	Rose/BinaryAnalysis/ModelChecker/Periodic.h					\
	Rose/BinaryAnalysis/ModelChecker/SemanticCallbacks.h				\
	Rose/BinaryAnalysis/ModelChecker/Settings.h					\
	Rose/BinaryAnalysis/ModelChecker/SourceLister.h					\
	Rose/BinaryAnalysis/ModelChecker/Tag.h						\
	Rose/BinaryAnalysis/ModelChecker/UninitializedVariableTag.h			\
	Rose/BinaryAnalysis/ModelChecker/Variables.h					\
	Rose/BinaryAnalysis/ModelChecker/WorkerStatus.h					\
	Rose/BinaryAnalysis/NoOperation.h						\
	Rose/BinaryAnalysis/Partitioner2.h						\
	Rose/BinaryAnalysis/Partitioner2/AddressUsageMap.h				\
	Rose/BinaryAnalysis/Partitioner2/BasicBlock.h					\
	Rose/BinaryAnalysis/Partitioner2/BasicTypes.h					\
	Rose/BinaryAnalysis/Partitioner2/CfgPath.h					\
	Rose/BinaryAnalysis/Partitioner2/Configuration.h				\
	Rose/BinaryAnalysis/Partitioner2/ControlFlowGraph.h				\
	Rose/BinaryAnalysis/Partitioner2/DataBlock.h					\
	Rose/BinaryAnalysis/Partitioner2/DataFlow.h					\
	Rose/BinaryAnalysis/Partitioner2/Engine.h					\
	Rose/BinaryAnalysis/Partitioner2/EngineBinary.h					\
	Rose/BinaryAnalysis/Partitioner2/EngineJvm.h					\
	Rose/BinaryAnalysis/Partitioner2/Exception.h					\
	Rose/BinaryAnalysis/Partitioner2/Function.h					\
	Rose/BinaryAnalysis/Partitioner2/FunctionCallGraph.h				\
	Rose/BinaryAnalysis/Partitioner2/GraphViz.h					\
	Rose/BinaryAnalysis/Partitioner2/IndirectControlFlow.h				\
	Rose/BinaryAnalysis/Partitioner2/JumpTable.h					\
	Rose/BinaryAnalysis/Partitioner2/MemoryTable.h					\
	Rose/BinaryAnalysis/Partitioner2/Modules.h					\
	Rose/BinaryAnalysis/Partitioner2/ModulesElf.h					\
	Rose/BinaryAnalysis/Partitioner2/ModulesJvm.h					\
	Rose/BinaryAnalysis/Partitioner2/ModulesLinux.h					\
	Rose/BinaryAnalysis/Partitioner2/ModulesM68k.h					\
	Rose/BinaryAnalysis/Partitioner2/ModulesMips.h					\
	Rose/BinaryAnalysis/Partitioner2/ModulesPe.h					\
	Rose/BinaryAnalysis/Partitioner2/ModulesPowerpc.h				\
	Rose/BinaryAnalysis/Partitioner2/ModulesX86.h					\
	Rose/BinaryAnalysis/Partitioner2/Partitioner.h					\
	Rose/BinaryAnalysis/Partitioner2/Reference.h					\
	Rose/BinaryAnalysis/Partitioner2/Semantics.h					\
	Rose/BinaryAnalysis/Partitioner2/Thunk.h					\
	Rose/BinaryAnalysis/Partitioner2/Utility.h					\
	Rose/BinaryAnalysis/PointerDetection.h						\
	Rose/BinaryAnalysis/Reachability.h						\
	Rose/BinaryAnalysis/ReadWriteSets.h						\
	Rose/BinaryAnalysis/RegisterDescriptor.h					\
	Rose/BinaryAnalysis/RegisterDictionary.h					\
	Rose/BinaryAnalysis/RegisterNames.h						\
	Rose/BinaryAnalysis/RegisterParts.h						\
	Rose/BinaryAnalysis/RelativeVirtualAddress.h					\
	Rose/BinaryAnalysis/ReturnValueUsed.h						\
	Rose/BinaryAnalysis/Serialization.h							\
	Rose/BinaryAnalysis/Serialization/SerialFrame.h				\
	Rose/BinaryAnalysis/SerialIo.h							\
	Rose/BinaryAnalysis/SmtCommandLine.h						\
	Rose/BinaryAnalysis/SmtlibSolver.h						\
	Rose/BinaryAnalysis/SmtSolver.h							\
	Rose/BinaryAnalysis/SourceLocations.h						\
	Rose/BinaryAnalysis/SRecord.h							\
	Rose/BinaryAnalysis/StackDelta.h						\
	Rose/BinaryAnalysis/String.h							\
	Rose/BinaryAnalysis/SymbolicExpr.h						\
	Rose/BinaryAnalysis/SymbolicExpression.h					\
	Rose/BinaryAnalysis/SymbolicExprParser.h					\
	Rose/BinaryAnalysis/SymbolicExpressionParser.h					\
	Rose/BinaryAnalysis/SystemCall.h						\
	Rose/BinaryAnalysis/TaintedFlow.h						\
	Rose/BinaryAnalysis/ToSource.h							\
	Rose/BinaryAnalysis/Unparser.h							\
	Rose/BinaryAnalysis/Unparser/Aarch32.h						\
	Rose/BinaryAnalysis/Unparser/Aarch64.h						\
	Rose/BinaryAnalysis/Unparser/Base.h						\
	Rose/BinaryAnalysis/Unparser/Cil.h						\
	Rose/BinaryAnalysis/Unparser/EdgeArrows.h					\
	Rose/BinaryAnalysis/Unparser/Jvm.h						\
	Rose/BinaryAnalysis/Unparser/M68k.h						\
	Rose/BinaryAnalysis/Unparser/Mips.h						\
	Rose/BinaryAnalysis/Unparser/Null.h						\
	Rose/BinaryAnalysis/Unparser/Powerpc.h						\
	Rose/BinaryAnalysis/Unparser/Settings.h						\
	Rose/BinaryAnalysis/Unparser/X86.h						\
	Rose/BinaryAnalysis/Utility.h							\
	Rose/BinaryAnalysis/Variables.h							\
	Rose/BinaryAnalysis/Variables/BaseVariable.h					\
	Rose/BinaryAnalysis/Variables/BasicTypes.h					\
	Rose/BinaryAnalysis/Variables/GlobalVariable.h					\
	Rose/BinaryAnalysis/Variables/InstructionAccess.h				\
	Rose/BinaryAnalysis/Variables/StackFrame.h					\
	Rose/BinaryAnalysis/Variables/StackVariable.h					\
	Rose/BinaryAnalysis/Variables/VariableFinder.h					\
	Rose/BinaryAnalysis/VxworksTerminal.h						\
	Rose/BinaryAnalysis/Z3Solver.h							\
	Rose/BitFlags.h									\
	Rose/BitOps.h									\
	Rose/Color.h									\
	Rose/Common.h									\
	Rose/Common/Analysis.h								\
	Rose/Common/Analysis/AbstractCompatibilityBridge.h			\
	Rose/Common/Analysis/AnalysisTypes.h				\
	Rose/Common/Analysis/ClassHierarchyAnalysis.h			\
	Rose/Common/Analysis/ClassHierarchyExport.h			\
	Rose/Common/Analysis/Utility.h					\
	Rose/CommandLine.h								\
	Rose/CommandLine/DurationParser.h						\
	Rose/CommandLine/IntervalParser.h						\
	Rose/CommandLine/License.h							\
	Rose/CommandLine/Parser.h							\
	Rose/CommandLine/SelfTest.h							\
	Rose/CommandLine/SuffixMultiplierParser.h					\
	Rose/CommandLine/Version.h							\
	Rose/Constants.h								\
	Rose/Diagnostics.h								\
	Rose/Exception.h								\
	Rose/FileSystem.h								\
	Rose/FormatRestorer.h								\
	Rose/FormattedTable.h								\
	Rose/GraphUtility.h								\
	Rose/Initialize.h								\
	Rose/Location.h									\
	Rose/Logger.h									\
	Rose/metaprog.h									\
	Rose/ParallelSort.h								\
	Rose/Progress.h									\
	Rose/RecursionCounter.h								\
	Rose/Sarif.h									\
	Rose/Sarif/BasicTypes.h								\
	Rose/Sarif/Exception.h								\
	Rose/Source/Analysis.h									\
	Rose/Source/Analysis/Analyzer.h									\
	Rose/Source/Analysis/CppClassLayoutExport.h				\
	Rose/Source/Analysis/CppCompatibility.h					\
	Rose/Source/Analysis/CppHierarchyAnalysis.h				\
	Rose/Source/Analysis/CppLayoutAnalysis.h				\
	Rose/Source/Analysis/CppSpecialMemberAnalysis.h			\
	Rose/Source/Analysis/ExternalMangling.h					\
	Rose/Source/Analysis/Visitor.h									\
	Rose/Source/AST.h									\
	Rose/Source/AST/Checker.h								\
	Rose/Source/AST/cmdline.h								\
	Rose/Source/AST/Defect.h								\
	Rose/Source/AST/IO.h									\
	Rose/Source/Generation.h									\
	Rose/Source/Generation/SymbolScanner.h							\
	Rose/Source/Generation/SymbolScanner.txx							\
	Rose/Source/Generation/API.h								\
	Rose/Source/Generation/API.txx								\
	Rose/Source/Generation/Driver.h								\
	Rose/Source/Generation/Factory.h								\
	Rose/Source/Generation/factory/factory.txx						\
	Rose/Source/Generation/factory/classes.txx						\
	Rose/Source/Generation/factory/functions.txx						\
	Rose/Source/Generation/factory/namespaces.txx						\
	Rose/Source/Generation/factory/typedefs.txx						\
	Rose/Source/Generation/factory/variables.txx						\
	Rose/Source/Generation/Object.h								\
	Rose/Source/Sarif.h								\
	Rose/SourceLocation.h								\
	Rose/StringUtility.h								\
	Rose/StringUtility/Convert.h							\
	Rose/StringUtility/Diagnostics.h						\
	Rose/StringUtility/Escape.h							\
	Rose/StringUtility/NumberToString.h						\
	Rose/StringUtility/Predicate.h							\
	Rose/StringUtility/SplitJoin.h							\
	Rose/StringUtility/StringToNumber.h						\
	Rose/Traits.h									\
	Rose/FlowGraphInterface.h								\
	Rose/FlowAnalysisInterface.h								\
	Rose/SawyerGraphConnection.h                                                    \
	Rose/Traits/Describe.h								\
	Rose/Traits/describe/declarations.h						\
	Rose/Traits/describe/fields.h							\
	Rose/Traits/describe/nodes.h							\
	Rose/Traits/describe/predicates.h						\
	Rose/Traits/grammar/lists.h							\
	Rose/Traits/grammar/predicates.h						\
	Rose/Traits/grammar/traversal.h							\
	Rose/Traits/Traversal.h								\
	Rose/Yaml.h


########################################################################################################################

# DQ (9/22/2006): Because we now use libtool, the shared library is a reference to the separate
# dynamic libraries in each directory and so testMultiRoseLib is redundant with testSharedRoseLib.
# This is true only if we have configured --with-development-libraries, else we have static
# in each directory and we will link with the collection of all static libraries.

all-local:

LINK = $(CXXLINK)

lib_LTLIBRARIES = librose.la

if ROSE_USE_INTERNAL_JAVA_SUPPORT
   JAVA_JVM_FLAGS = $(JAVA_JVM_LINK)
endif

librose_la_SOURCES = dummyCppFileForLibrose.C

if ROSE_USE_INTERNAL_FRONTEND_DEVELOPMENT
librose_la_LIBADD = $(BOOST_LDFLAGS) $(libroseLibraries) $(JAVA_JVM_FLAGS) $(LIB_QT) -lm $(RT_LIBS) \
          $(BOOST_DATE_TIME_LIB) \
          $(BOOST_THREAD_LIB) $(BOOST_FILESYSTEM_LIB) \
          $(BOOST_PROGRAM_OPTIONS_LIB) $(BOOST_REGEX_LIB) \
          $(BOOST_SYSTEM_LIB) $(BOOST_SERIALIZATION_LIB) \
          $(BOOST_WAVE_LIB) $(BOOST_IOSTREAMS_LIB)
else
librose_la_LIBADD = $(BOOST_LDFLAGS) $(libroseLibraries) $(JAVA_JVM_FLAGS) $(LIB_QT) -lm $(RT_LIBS) \
          $(BOOST_DATE_TIME_LIB) \
          $(BOOST_THREAD_LIB) $(BOOST_FILESYSTEM_LIB) \
          $(BOOST_PROGRAM_OPTIONS_LIB) $(BOOST_REGEX_LIB) \
          $(BOOST_SYSTEM_LIB) $(BOOST_SERIALIZATION_LIB) \
          $(BOOST_WAVE_LIB) $(BOOST_IOSTREAMS_LIB) \
          $(OPT_SQLITE3_LDFLAGS) \
          $(OPT_MYSQL_LDFLAGS) $(ROSE_ATERM_LIBS) $(ROSE_FLANG_LIBS)
endif

# libelf is a prerequisite for using libdwarf
if ROSE_HAVE_LIBELF
   librose_la_LIBADD += $(ROSE_ELF_LIBS_WITH_PATH)
endif

if ROSE_HAVE_LIBDWARF
   librose_la_LIBADD += $(ROSE_DWARF_LIBS_WITH_PATH)
endif

if ROSE_USE_GCC_OMP
   librose_la_LIBADD += lgomp
endif

if ROSE_EXPERIMENTAL_ADA_ROSE_CONNECTION
  librose_la_LIBADD += -L${DOT_ASIS_LIB_DIR_} -lasis_adapter
endif

if ROSE_EXPERIMENTAL_LIBADALANG_ROSE_CONNECTION
  librose_la_LIBADD += -L${GNAT_HOME}/lib -ladalang
endif

librose_la_LDFLAGS = -no-undefined $(ROSE_RPATHS)

if USING_CLANG_COMPILER
librose_la_LIBADD += -lstdc++
endif
librose_la_LDFLAGS += -fno-rtti

librose_la_DEPENDENCIES = $(libroseLibraries)

if OS_MACOSX
  librose_la_LDFLAGS += $(ROSE_LINK_RPATHS)
endif

# Build list of all required libraries so that they can be built in parallel
REQUIRED_LIBS = librose.la

LDFLAGS =
if OS_MACOSX
if ROSE_HAVE_LIBGCRYPT
PORT_PATH = $(shell which port)
PORT_HOME = $(shell dirname $(PORT_PATH))/..
LDFLAGS += -L$(PORT_HOME)/lib -lgcrypt
else
LDFLAGS +=
endif
endif


########################################################################################################################
# This tool spits out information that tells users how to compile and link programs that use ROSE. Therefore, we have to
# build a configuration file and compile its installed name into the rose-config tool. It's also especially important
# that this tool uses rpaths to find its dynamically-linked libraries, otherwise the user will be tempted to use
# LD_LIBRARY_PATH and complain when the tool segfaults, not realizing that the fault is because they told the tool to
# use a library that is incompatible with the one it was compiled to use.
bin_PROGRAMS += rose-config
rose_config_SOURCES = rose-config.C
rose_config_LDFLAGS = $(ROSE_RPATHS)
rose_config_DEPENDENCIES = $(REQUIRED_LIBS)
rose_config_CPPFLAGS = -DLIBDIR='"$(libdir)"'
rose_config_LDADD = $(ROSE_LIBS)

# This tool outputs some information about featureTests.h, which
# controls which features are to be enabled in ROSE.
bin_PROGRAMS += rose-feature-tests
rose_feature_tests_SOURCES = featureTests.C
rose_feature_tests_LDFLAGS =
rose_feature_tests_DEPENDENCIES =
rose_feature_tests_CPPFLAGS =
rose_feature_tests_LDADD =

if OS_MACOSX
  rose_config_LDFLAGS += $(ROSE_LINK_RPATHS)
endif

if USING_CLANG_COMPILER
rose_config_LDADD += -lstdc++
rose_config_LDFLAGS += -fno-rtti
endif
rose_config_LDFLAGS += -fno-rtti

rose_config_installed_cppflags =		\
	-I@includedir@/rose			\
	$(ROSE_QT_INCLUDE)			\
	$(ROSE_SQLITE_DATABASE_INCLUDE)		\
	$(ROSE_PHP_INCLUDES)			\
	$(ROSE_YICES_INCLUDES)			\
	$(ROSE_Z3_INCLUDES)			\
	$(ROSE_YAMLCPP_INCLUDES)		\
	$(ROSE_LIBMAGIC_INCLUDES)		\
	$(ROSE_ELF_INCLUDES)			\
	$(ROSE_DWARF_INCLUDES)			\
	$(ROSE_WINE_INCLUDES)			\
	$(ROSE_PCH_INCLUDE)			\
	$(INTEL_PIN_INCLUDE_PATHS)		\
	$(ROSE_ETHER_INCLUDE)			\
	$(ROSE_ATERM_INCLUDE)			\
	$(ROSE_DLIB_INCLUDES)			\
	$(ROSE_GCRYPT_INCLUDES)			\
	$(ROSE_CAPSTONE_INCLUDES)		\
	$(BOOST_CPPFLAGS)

rose_config_installed_ldflags =			\
	-L@libdir@ -lrose			\
	$(BOOST_LDFLAGS)			\
	$(BOOST_DATE_TIME_LIB)			\
	$(BOOST_THREAD_LIB)			\
	$(BOOST_FILESYSTEM_LIB)			\
	$(BOOST_PROGRAM_OPTIONS_LIB)		\
	$(BOOST_REGEX_LIB)			\
	$(BOOST_SYSTEM_LIB)			\
	$(BOOST_SERIALIZATION_LIB)		\
	$(BOOST_WAVE_LIB)			\
	$(BOOST_IOSTREAMS_LIB)			\
	$(BOOST_CHRONO_LIB)			\
	$(JAVA_JVM_LINK)			\
	$(SQLITE_DATABASE_LIBS)			\
	$(RT_LIBS)				\
	$(ROSE_YICES_LIBS_WITH_PATH)		\
	$(ROSE_Z3_LIBS_WITH_PATH)		\
	$(ROSE_PHP_LIBS_WITH_PATH)		\
	$(ROSE_ELF_LIBS_WITH_PATH)		\
	$(ROSE_DWARF_LIBS_WITH_PATH)		\
	$(ROSE_QT_LIBS_WITH_PATH)		\
	$(ROSE_SSL_LIBS)			\
	$(ROSE_ETHER_LIBS)			\
	$(ROSE_INTEL_COMPILER_MATH_LIBS)	\
	$(ROSE_ATERM_LIBS)			\
	$(ROSE_FLANG_LIBS)			\
	$(ROSE_YAMLCPP_LIBS_WITH_PATH)		\
	$(ROSE_LIBMAGIC_LIBS_WITH_PATH)		\
	$(ROSE_DLIB_LIBS_WITH_PATH)		\
	$(ROSE_GCRYPT_LIBS_WITH_PATH)		\
	$(ROSE_CAPSTONE_LIBS_WITH_PATH)		\
	-lm
if ROSE_EXPERIMENTAL_ADA_ROSE_CONNECTION
  rose_config_installed_ldflags += -L${DOT_ASIS_LIB_DIR_} -lasis_adapter
endif
rose_config_cfgdir = $(libdir)
rose_config_cfg_DATA = rose-config.cfg

# ROSE_RPATHS and ROSE_LINK_RPATHS are used to enable linking on Mac OSX from a ROSE installation.
rose-config.cfg:
	$(AM_V_GEN) (										\
		echo "# DEPRECATED 2025-10-10. Use pkg-config or RoseConfig.cmake instead.";	\
		echo;										\
		echo;										\
		echo "# Things that the 'rose-config' tool can report";				\
		echo "ROSE_CC	    = $(CC)";							\
		echo "ROSE_CXX	    = $(CXX)";							\
		echo "ROSE_CPPFLAGS = $(rose_config_installed_cppflags)";			\
		echo "ROSE_CFLAGS   = $(CFLAGS)";						\
		echo "ROSE_CXXFLAGS = $(CXXFLAGS)";						\
		echo "ROSE_LDFLAGS  = $(rose_config_installed_ldflags)";			\
		echo "ROSE_LIBDIRS  = $(filter-out -R, $(ROSE_RPATHS))";			\
		echo "ROSE_PREFIX   = $(prefix)";						\
		echo "# Additional items useful to include in Makefiles";			\
		echo "ROSE_RPATHS	= $(ROSE_RPATHS)";					\
		echo "ROSE_LINK_RPATHS	= $(ROSE_LINK_RPATHS)";					\
		echo "ROSE_BACKEND_C_COMPILER = $(BACKEND_C_COMPILER)";				\
		echo "ROSE_BACKEND_CXX_COMPILER = $(BACKEND_CXX_COMPILER)";			\
	) >$@
	@echo "# Automake configuration variables used within ROSE Tests" >> $@
	@echo "## Enabled languages" >> $@
if ROSE_BUILD_BINARY_ANALYSIS_SUPPORT
	@echo "AM_ROSE_BUILD_BINARY_ANALYSIS_SUPPORT = true" >> $@
else
	@echo "AM_ROSE_BUILD_BINARY_ANALYSIS_SUPPORT = false" >> $@
endif
if ROSE_BUILD_CXX_LANGUAGE_SUPPORT
	@echo "AM_ROSE_BUILD_CXX_LANGUAGE_SUPPORT = true" >> $@
else
	@echo "AM_ROSE_BUILD_CXX_LANGUAGE_SUPPORT = false" >> $@
endif
if ROSE_BUILD_CUDA_LANGUAGE_SUPPORT
	@echo "AM_ROSE_BUILD_CUDA_LANGUAGE_SUPPORT = true" >> $@
else
	@echo "AM_ROSE_BUILD_CUDA_LANGUAGE_SUPPORT = false" >> $@
endif
if ROSE_BUILD_FORTRAN_LANGUAGE_SUPPORT
	@echo "AM_ROSE_BUILD_FORTRAN_LANGUAGE_SUPPORT = true" >> $@
else
	@echo "AM_ROSE_BUILD_FORTRAN_LANGUAGE_SUPPORT = false" >> $@
endif
if ROSE_BUILD_JAVA_LANGUAGE_SUPPORT
	@echo "AM_ROSE_BUILD_JAVA_LANGUAGE_SUPPORT = true" >> $@
else
	@echo "AM_ROSE_BUILD_JAVA_LANGUAGE_SUPPORT = false" >> $@
endif
if ROSE_BUILD_PYTHON_LANGUAGE_SUPPORT
	@echo "AM_ROSE_BUILD_PYTHON_LANGUAGE_SUPPORT = true" >> $@
else
	@echo "AM_ROSE_BUILD_PYTHON_LANGUAGE_SUPPORT = false" >> $@
endif
if ROSE_BUILD_OPENCL_LANGUAGE_SUPPORT
	@echo "AM_ROSE_BUILD_OPENCL_LANGUAGE_SUPPORT = true" >> $@
else
	@echo "AM_ROSE_BUILD_OPENCL_LANGUAGE_SUPPORT = false" >> $@
endif
if ROSE_BUILD_C_LANGUAGE_SUPPORT
	@echo "AM_ROSE_BUILD_C_LANGUAGE_SUPPORT = true" >> $@
else
	@echo "AM_ROSE_BUILD_C_LANGUAGE_SUPPORT = false" >> $@
endif
if ROSE_BUILD_PHP_LANGUAGE_SUPPORT
	@echo "AM_ROSE_BUILD_PHP_LANGUAGE_SUPPORT = true" >> $@
else
	@echo "AM_ROSE_BUILD_PHP_LANGUAGE_SUPPORT = false" >> $@
endif
if ROSE_EXPERIMENTAL_CSHARP_ROSE_CONNECTION
	@echo "AM_ROSE_EXPERIMENTAL_CSHARP_ROSE_CONNECTION = true" >> $@
else
	@echo "AM_ROSE_EXPERIMENTAL_CSHARP_ROSE_CONNECTION = false" >> $@
endif
if ROSE_EXPERIMENTAL_ADA_ROSE_CONNECTION
	@echo "AM_ROSE_EXPERIMENTAL_ADA_ROSE_CONNECTION = true" >> $@
else
	@echo "AM_ROSE_EXPERIMENTAL_ADA_ROSE_CONNECTION = false" >> $@
endif
if ROSE_EXPERIMENTAL_LIBADALANG_ROSE_CONNECTION
	@echo "AM_ROSE_EXPERIMENTAL_LIBADALANG_ROSE_CONNECTION = true" >> $@
else
	@echo "AM_ROSE_EXPERIMENTAL_LIBADALANG_ROSE_CONNECTION = false" >> $@
endif
if ROSE_EXPERIMENTAL_JOVIAL_ROSE_CONNECTION
	@echo "AM_ROSE_EXPERIMENTAL_JOVIAL_ROSE_CONNECTION = true" >> $@
else
	@echo "AM_ROSE_EXPERIMENTAL_JOVIAL_ROSE_CONNECTION = false" >> $@
endif
if ROSE_EXPERIMENTAL_MATLAB_ROSE_CONNECTION
	@echo "AM_ROSE_EXPERIMENTAL_MATLAB_ROSE_CONNECTION = true" >> $@
else
	@echo "AM_ROSE_EXPERIMENTAL_MATLAB_ROSE_CONNECTION = false" >> $@
endif
if ROSE_EXPERIMENTAL_FLANG_ROSE_CONNECTION
	@echo "AM_ROSE_EXPERIMENTAL_FLANG_ROSE_CONNECTION = true" >> $@
else
	@echo "AM_ROSE_EXPERIMENTAL_FLANG_ROSE_CONNECTION = false" >> $@
endif
if OS_MACOSX
	@echo "AM_OS_MACOSX = true" >> $@
else
	@echo "AM_OS_MACOSX = false" >> $@
endif
if OS_VENDOR_REDHAT_REL5
	@echo "AM_OS_VENDOR_REDHAT_REL5 = true" >> $@
else
	@echo "AM_OS_VENDOR_REDHAT_REL5 = false" >> $@
endif
if OS_VENDOR_REDHAT_REL6
	@echo "AM_OS_VENDOR_REDHAT_REL6 = true" >> $@
else
	@echo "AM_OS_VENDOR_REDHAT_REL6 = false" >> $@
endif
if OS_VENDOR_DEBIAN
	@echo "AM_OS_VENDOR_DEBIAN = true" >> $@
else
	@echo "AM_OS_VENDOR_DEBIAN = false" >> $@
endif
if ROSE_USING_GCC_VERSION_LATER_4_4
	@echo "AM_ROSE_USING_GCC_VERSION_LATER_4_4 = true" >> $@
else
	@echo "AM_ROSE_USING_GCC_VERSION_LATER_4_4 = false" >> $@
endif
if ROSE_USING_GCC_VERSION_LATER_4_8
	@echo "AM_ROSE_USING_GCC_VERSION_LATER_4_8 = true" >> $@
else
	@echo "AM_ROSE_USING_GCC_VERSION_LATER_4_8 = false" >> $@
endif
if ROSE_USING_GCC_VERSION_LATER_4_9
	@echo "AM_ROSE_USING_GCC_VERSION_LATER_4_9 = true" >> $@
else
	@echo "AM_ROSE_USING_GCC_VERSION_LATER_4_9 = false" >> $@
endif
if ROSE_USING_GCC_VERSION_LATER_5_1
	@echo "AM_ROSE_USING_GCC_VERSION_LATER_5_1 = true" >> $@
else
	@echo "AM_ROSE_USING_GCC_VERSION_LATER_5_1 = false" >> $@
endif
if ROSE_USING_GCC_VERSION_LATER_6_0
	@echo "AM_ROSE_USING_GCC_VERSION_LATER_6_0 = true" >> $@
else
	@echo "AM_ROSE_USING_GCC_VERSION_LATER_6_0 = false" >> $@
endif
if ROSE_USING_GCC_VERSION_LATER_6_1
	@echo "AM_ROSE_USING_GCC_VERSION_LATER_6_1 = true" >> $@
else
	@echo "AM_ROSE_USING_GCC_VERSION_LATER_6_1 = false" >> $@
endif
if ROSE_USING_GCC_VERSION_LATER_10_1
	@echo "AM_ROSE_USING_GCC_VERSION_LATER_10_1 = true" >> $@
else
	@echo "AM_ROSE_USING_GCC_VERSION_LATER_10_1 = false" >> $@
endif
if ROSE_USING_GCC_VERSION_LATER_10_5
	@echo "AM_ROSE_USING_GCC_VERSION_LATER_10_5 = true" >> $@
else
	@echo "AM_ROSE_USING_GCC_VERSION_LATER_10_5 = false" >> $@
endif
if ROSE_USING_GFORTRAN_VERSION_4_0
	@echo "AM_ROSE_USING_GFORTRAN_VERSION_4_0 = true" >> $@
else
	@echo "AM_ROSE_USING_GFORTRAN_VERSION_4_0 = false" >> $@
endif
if ROSE_USING_GFORTRAN_VERSION_4_1
	@echo "AM_ROSE_USING_GFORTRAN_VERSION_4_1 = true" >> $@
else
	@echo "AM_ROSE_USING_GFORTRAN_VERSION_4_1 = false" >> $@
endif
if ROSE_USING_GFORTRAN_VERSION_4_4
	@echo "AM_ROSE_USING_GFORTRAN_VERSION_4_4 = true" >> $@
else
	@echo "AM_ROSE_USING_GFORTRAN_VERSION_4_4 = false" >> $@
endif
if ROSE_USING_GFORTRAN_VERSION_4_5
	@echo "AM_ROSE_USING_GFORTRAN_VERSION_4_5 = true" >> $@
else
	@echo "AM_ROSE_USING_GFORTRAN_VERSION_4_5 = false" >> $@
endif
if ROSE_USING_GFORTRAN_VERSION_LATER_4_4
	@echo "AM_ROSE_USING_GFORTRAN_VERSION_LATER_4_4 = true" >> $@
else
	@echo "AM_ROSE_USING_GFORTRAN_VERSION_LATER_4_4 = false" >> $@
endif
if ROSE_USING_GFORTRAN_VERSION_LATER_4_5
	@echo "AM_ROSE_USING_GFORTRAN_VERSION_LATER_4_5 = true" >> $@
else
	@echo "AM_ROSE_USING_GFORTRAN_VERSION_LATER_4_5 = false" >> $@
endif
if ROSE_USING_BOOST_VERSION_1_35
	@echo "AM_ROSE_USING_BOOST_VERSION_1_35 = true" >> $@
else
	@echo "AM_ROSE_USING_BOOST_VERSION_1_35 = false" >> $@
endif
if ROSE_USING_BOOST_VERSION_1_57
	@echo "AM_ROSE_USING_BOOST_VERSION_1_57 = true" >> $@
else
	@echo "AM_ROSE_USING_BOOST_VERSION_1_57 = false" >> $@
endif
if ROSE_USING_BOOST_VERSION_1_62
	@echo "AM_ROSE_USING_BOOST_VERSION_1_62 = true" >> $@
else
	@echo "AM_ROSE_USING_BOOST_VERSION_1_62 = false" >> $@
endif
if ROSE_USE_EDG_VERSION_5_0
	@echo "AM_ROSE_USE_EDG_VERSION_5_0 = true" >> $@
else
	@echo "AM_ROSE_USE_EDG_VERSION_5_0 = false" >> $@
endif
if ROSE_USE_EDG_VERSION_6_0
	@echo "AM_ROSE_USE_EDG_VERSION_6_0 = true" >> $@
else
	@echo "AM_ROSE_USE_EDG_VERSION_6_0 = false" >> $@
endif
if ROSE_USE_LONG_MAKE_CHECK_RULE
	@echo "AM_ROSE_USE_LONG_MAKE_CHECK_RULE = true" >> $@
else
	@echo "AM_ROSE_USE_LONG_MAKE_CHECK_RULE = false" >> $@
endif
if ROSE_USE_SANITIZER
	@echo "AM_ROSE_USE_SANITIZER = true" >> $@
else
	@echo "AM_ROSE_USE_SANITIZER = false" >> $@
endif
if USING_INTEL_COMPILER
	@echo "AM_USING_INTEL_COMPILER = true" >> $@
else
	@echo "AM_USING_INTEL_COMPILER = false" >> $@
endif
if ROSE_HAVE_Z3
	@echo "AM_ROSE_HAVE_Z3 = true" >> $@
else
	@echo "AM_ROSE_HAVE_Z3 = false" >> $@
endif
if ROSE_HAVE_LIBZ3
	@echo "AM_ROSE_HAVE_LIBZ3 = true" >> $@
else
	@echo "AM_ROSE_HAVE_LIBZ3 = false" >> $@
endif
if ROSE_BUILD_OS_IS_CYGWIN
	@echo "AM_ROSE_BUILD_OS_IS_CYGWIN = true" >> $@
else
	@echo "AM_ROSE_BUILD_OS_IS_CYGWIN = false" >> $@
endif
if ROSE_USE_INTEL_PIN
	@echo "AM_ROSE_USE_INTEL_PIN = true" >> $@
else
	@echo "AM_ROSE_USE_INTEL_PIN = false" >> $@
endif
if ROSE_USE_SQLITE_DATABASE
	@echo "AM_ROSE_USE_SQLITE_DATABASE = true" >> $@
else
	@echo "AM_ROSE_USE_SQLITE_DATABASE = false" >> $@
endif
if ROSE_BUILD_OS_IS_OSX
	@echo "AM_ROSE_BUILD_OS_IS_OSX = true" >> $@
else
	@echo "AM_ROSE_BUILD_OS_IS_OSX = false" >> $@
endif
if ROSE_USE_SSL_SUPPORT
	@echo "AM_ROSE_USE_SSL_SUPPORT = true" >> $@
else
	@echo "AM_ROSE_USE_SSL_SUPPORT = false" >> $@
endif
if ROSE_HAVE_LIBDWARF
	@echo "AM_ROSE_HAVE_LIBDWARF = true" >> $@
else
	@echo "AM_ROSE_HAVE_LIBDWARF = false" >> $@
endif
if USING_GNU_COMPILER
	@echo "AM_USING_GNU_COMPILER = true" >> $@
else
	@echo "AM_USING_GNU_COMPILER = false" >> $@
endif
if ROSE_USE_CLANG_FRONTEND
	@echo "AM_ROSE_USE_CLANG_FRONTEND = true" >> $@
else
	@echo "AM_ROSE_USE_CLANG_FRONTEND = false" >> $@
endif
if ROSE_USE_INSURE
	@echo "AM_ROSE_USE_INSURE = true" >> $@
else
	@echo "AM_ROSE_USE_INSURE = false" >> $@
endif
if USING_CLANG_COMPILER
	@echo "AM_USING_CLANG_COMPILER = true" >> $@
else
	@echo "AM_USING_CLANG_COMPILER = false" >> $@
endif
if ROSE_USING_ALTERNATE_BACKEND_C_COMPILER
	@echo "AM_ROSE_USING_ALTERNATE_BACKEND_C_COMPILER = true" >> $@
else
	@echo "AM_ROSE_USING_ALTERNATE_BACKEND_C_COMPILER = false" >> $@
endif
if USING_XTENSA_BACKEND_COMPILER
	@echo "AM_USING_XTENSA_BACKEND_COMPILER = true" >> $@
else
	@echo "AM_USING_XTENSA_BACKEND_COMPILER = false" >> $@
endif
if ROSE_FRONTEND_COMPILER_DEFAULT_IS_CXX11
	@echo "AM_ROSE_FRONTEND_COMPILER_DEFAULT_IS_CXX11 = true" >> $@
else
	@echo "AM_ROSE_FRONTEND_COMPILER_DEFAULT_IS_CXX11 = false" >> $@
endif
if ROSE_USE_VXWORKS
	@echo "AM_ROSE_USE_VXWORKS = true" >> $@
else
	@echo "AM_ROSE_USE_VXWORKS = false" >> $@
endif
if ROSE_USE_GNAT
	@echo "AM_ROSE_USE_GNAT = true" >> $@
else
	@echo "AM_ROSE_USE_GNAT = false" >> $@
endif
if ROSE_USE_MICROSOFT_EXTENSIONS
	@echo "AM_ROSE_USE_MICROSOFT_EXTENSIONS = true" >> $@
else
	@echo "AM_ROSE_USE_MICROSOFT_EXTENSIONS = false" >> $@
endif
if ROSE_USE_WINDOWS_ANALYSIS_SUPPORT
	@echo "AM_ROSE_USE_WINDOWS_ANALYSIS_SUPPORT = true" >> $@
else
	@echo "AM_ROSE_USE_WINDOWS_ANALYSIS_SUPPORT = false" >> $@
endif
if ROSE_USE_LCOV
	@echo "AM_ROSE_USE_LCOV = true" >> $@
else
	@echo "AM_ROSE_USE_LCOV = false" >> $@
endif
if USE_VALGRIND
	@echo "AM_USE_VALGRIND = true" >> $@
else
	@echo "AM_USE_VALGRIND = false" >> $@
endif
if USING_GCC_3_4_4_BACKEND_COMPILER
	@echo "AM_USING_GCC_3_4_4_BACKEND_COMPILER = true" >> $@
else
	@echo "AM_USING_GCC_3_4_4_BACKEND_COMPILER = false" >> $@
endif
if ROSE_WITH_ATERM
	@echo "AM_ROSE_WITH_ATERM = true" >> $@
else
	@echo "AM_ROSE_WITH_ATERM = false" >> $@
endif
if ROSE_DEBUG_NEW_EDG_ROSE_CONNECTION
	@echo "AM_ROSE_DEBUG_NEW_EDG_ROSE_CONNECTION = true" >> $@
else
	@echo "AM_ROSE_DEBUG_NEW_EDG_ROSE_CONNECTION = false" >> $@
endif
if ROSE_USE_INTERNAL_FRONTEND_DEVELOPMENT
	@echo "AM_ROSE_USE_INTERNAL_FRONTEND_DEVELOPMENT = true" >> $@
else
	@echo "AM_ROSE_USE_INTERNAL_FRONTEND_DEVELOPMENT = false" >> $@
endif
if AXXPXX_SPECIFIC_TESTS
	@echo "AM_AXXPXX_SPECIFIC_TESTS = true" >> $@
else
	@echo "AM_AXXPXX_SPECIFIC_TESTS = false" >> $@
endif
if HAS_LIBRARY_CURL
	@echo "AM_HAS_LIBRARY_CURL = true" >> $@
else
	@echo "AM_HAS_LIBRARY_CURL = false" >> $@
endif
if ROSE_USE_INDENT
	@echo "AM_ROSE_USE_INDENT = true" >> $@
else
	@echo "AM_ROSE_USE_INDENT = false" >> $@
endif
if ROSE_HAVE_YICES
	@echo "AM_ROSE_HAVE_YICES = true" >> $@
else
	@echo "AM_ROSE_HAVE_YICES = false" >> $@
endif
if USE_CUDA
	@echo "AM_USE_CUDA = true" >> $@
else
	@echo "AM_USE_CUDA = false" >> $@
endif
if WITH_GOMP_OPENMP_LIB
	@echo "AM_WITH_GOMP_OPENMP_LIB = true" >> $@
else
	@echo "AM_WITH_GOMP_OPENMP_LIB = false" >> $@
endif
if ROSE_USE_OMNI_OPENMP
	@echo "AM_ROSE_USE_OMNI_OPENMP = true" >> $@
else
	@echo "AM_ROSE_USE_OMNI_OPENMP = false" >> $@
endif
if ROSE_USING_ROSE_CODE_GENERATION
	@echo "AM_ROSE_USING_ROSE_CODE_GENERATION = true" >> $@
else
	@echo "AM_ROSE_USING_ROSE_CODE_GENERATION = false" >> $@
endif
if ROSE_BUILD_ROSEHPCT
	@echo "AM_ROSE_BUILD_ROSEHPCT = true" >> $@
else
	@echo "AM_ROSE_BUILD_ROSEHPCT = false" >> $@
endif
if ROSE_HAVE_BOOST_SERIALIZATION_LIB
	@echo "AM_ROSE_HAVE_BOOST_SERIALIZATION_LIB = true" >> $@
else
	@echo "AM_ROSE_HAVE_BOOST_SERIALIZATION_LIB = false" >> $@
endif
if ROSE_USE_PYTHON3
	@echo "AM_ROSE_USE_PYTHON3 = true" >> $@
else
	@echo "AM_ROSE_USE_PYTHON3 = false" >> $@
endif
if ROSE_WITH_SPOT
	@echo "AM_ROSE_WITH_SPOT = true" >> $@
else
	@echo "AM_ROSE_WITH_SPOT = false" >> $@
endif

MOSTLYCLEANFILES += rose-config.cfg

bin_PROGRAMS += rose-compiler
if ROSE_BUILD_CXX_LANGUAGE_SUPPORT
  bin_PROGRAMS += rose-cc rose-c++
endif
if ROSE_BUILD_FORTRAN_LANGUAGE_SUPPORT
  bin_PROGRAMS += rose-fortran
endif
if ROSE_EXPERIMENTAL_ADA_ROSE_CONNECTION
  bin_PROGRAMS += rose-ada
endif
# The -std= option in ROSE only support c, c++, fortran, ada
#if ROSE_BUILD_BINARY_ANALYSIS_SUPPORT
#if ROSE_BUILD_JAVA_LANGUAGE_SUPPORT
#if ROSE_BUILD_PYTHON_LANGUAGE_SUPPORT
#if ROSE_BUILD_PHP_LANGUAGE_SUPPORT
#if ROSE_EXPERIMENTAL_CSHARP_ROSE_CONNECTION
#if ROSE_EXPERIMENTAL_ADA_ROSE_CONNECTION
#if ROSE_EXPERIMENTAL_JOVIAL_ROSE_CONNECTION
#if ROSE_EXPERIMENTAL_MATLAB_ROSE_CONNECTION

ROSE_BIN_LD_ADD   = $(ROSE_LIBS)
ROSE_BIN_LD_FLAGS = $(ROSE_RPATHS)
ROSE_BIN_DEPEND   = $(REQUIRED_LIBS)

if USING_CLANG_COMPILER
  ROSE_BIN_LD_ADD += -lstdc++
endif
ROSE_BIN_LD_FLAGS += -fno-rtti

if ROSE_USE_INTERNAL_JAVA_SUPPORT
if OS_MACOSX
  ROSE_BIN_LD_FLAGS += $(ROSE_LINK_RPATHS)
endif
endif

rose_compiler_SOURCES      = rose-compiler.C
rose_compiler_LDFLAGS      = $(ROSE_BIN_LD_FLAGS)
rose_compiler_DEPENDENCIES = $(ROSE_BIN_DEPEND)
rose_compiler_CPPFLAGS     =
rose_compiler_LDADD        = $(ROSE_BIN_LD_ADD)

rose_cc_SOURCES      = rose-compiler.C
rose_cc_LDFLAGS      = $(ROSE_BIN_LD_FLAGS)
rose_cc_DEPENDENCIES = $(ROSE_BIN_DEPEND)
rose_cc_CPPFLAGS     = -DROSE_COMPILER_FOR_LANGUAGE="\"c\""
rose_cc_LDADD        = $(ROSE_BIN_LD_ADD)

rose_c___SOURCES      = rose-compiler.C
rose_c___LDFLAGS      = $(ROSE_BIN_LD_FLAGS)
rose_c___DEPENDENCIES = $(ROSE_BIN_DEPEND)
rose_c___CPPFLAGS     = -DROSE_COMPILER_FOR_LANGUAGE="\"c++\""
rose_c___LDADD        = $(ROSE_BIN_LD_ADD)

rose_fortran_SOURCES      = rose-compiler.C
rose_fortran_LDFLAGS      = $(ROSE_BIN_LD_FLAGS)
rose_fortran_DEPENDENCIES = $(ROSE_BIN_DEPEND)
rose_fortran_CPPFLAGS     = -DROSE_COMPILER_FOR_LANGUAGE="\"fortran\""
rose_fortran_LDADD        = $(ROSE_BIN_LD_ADD)

rose_ada_SOURCES      = rose-compiler.C
rose_ada_LDFLAGS      = $(ROSE_BIN_LD_FLAGS)
rose_ada_DEPENDENCIES = $(ROSE_BIN_DEPEND)
rose_ada_CPPFLAGS     = -DROSE_COMPILER_FOR_LANGUAGE="\"ada\""
rose_ada_LDADD        = $(ROSE_BIN_LD_ADD)

if OS_MACOSX
  rose_compiler_LDFLAGS += $(ROSE_LINK_RPATHS)
  rose_cc_LDFLAGS += $(ROSE_LINK_RPATHS)
  rose_c___LDFLAGS += $(ROSE_LINK_RPATHS)
  rose_fortran_LDFLAGS += $(ROSE_LINK_RPATHS)
  rose_ada_LDFLAGS += $(ROSE_LINK_RPATHS)
endif
########################################################################################################################
# Test that a simple ROSE program can run, and as a side effect, print some version numbers.

check-boost:
	@echo "  TEST     boost shared linkage [rose-compiler]"
	${AM_V_at}$(top_srcdir)/scripts/check_boost_linkage.sh .libs/rose-compiler >&2

check-dynload:
	@echo "  TEST     whether a trivial ROSE program runs [rose-compiler]"
	${AM_V_at}if ! ./rose-compiler --version; then                                           \
	  echo "error: You cannot run a program linked with ROSE. Please be sure that your" >&2; \
	  echo "error: LD_LIBRARY_PATH (Linux) or DYLD_LIBRARY_PATH (Mac OSX) contains the" >&2; \
	  echo "error: correct libraries (JVM, Boost, librose.so, etc.)" >&2;                    \
	  exit 1;                                                                                \
	fi

check-config:
	@echo "  TEST     ROSE configuration tool [rose-config]"
	./rose-config --version

if ROSE_BUILD_C_LANGUAGE_SUPPORT
if OS_MACOSX
check-hello-c: $(eval SHELL:=/bin/bash)
	@echo "  TEST     ROSE C compiler [rose-cc]"
	${AM_V_at}echo "#include <stdio.h>"                     >  hello-world.c
	${AM_V_at}echo "int main() {"                           >> hello-world.c
	${AM_V_at}echo "   printf(\"Hello World from C\\n\");" >> hello-world.c
	${AM_V_at}echo "   return 0;"                           >> hello-world.c
	${AM_V_at}echo "}"                                      >> hello-world.c
	${AM_V_at}./rose-cc -rose:ast:write hello-world-c.ast hello-world.c -o hello-world-c
	${AM_V_at}./hello-world-c
	${AM_V_at}./rose-compiler -rose:ast:read hello-world-c.ast
else
check-hello-c:
	@echo "  TEST     ROSE C compiler [rose-cc]"
	${AM_V_at}echo "#include <stdio.h>"                     >  hello-world.c
	${AM_V_at}echo "int main() {"                           >> hello-world.c
	${AM_V_at}echo "   printf(\"Hello World from C\\n\");" >> hello-world.c
	${AM_V_at}echo "   return 0;"                           >> hello-world.c
	${AM_V_at}echo "}"                                      >> hello-world.c
	${AM_V_at}./rose-cc -rose:ast:write hello-world-c.ast hello-world.c -o hello-world-c
	${AM_V_at}./hello-world-c
	${AM_V_at}./rose-compiler -rose:ast:read hello-world-c.ast
endif
endif
if ROSE_BUILD_CXX_LANGUAGE_SUPPORT
check-hello-cxx:
	@echo "  TEST     ROSE C++ compiler [rose-c++]"
	${AM_V_at}echo "#include <iostream>"                                      >  hello-world.cpp
	${AM_V_at}echo "int main() {"                                             >> hello-world.cpp
	${AM_V_at}echo "    std::cout << \"Hello World from C++!\" << std::endl;" >> hello-world.cpp
	${AM_V_at}echo "    return 0;"                                            >> hello-world.cpp
	${AM_V_at}echo "}"                                                        >> hello-world.cpp
	${AM_V_at}./rose-c++ -rose:ast:write hello-world-cxx.ast hello-world.cpp -o hello-world-cpp
	${AM_V_at}./hello-world-cpp
	${AM_V_at}./rose-compiler -rose:ast:read hello-world-cxx.ast
endif
if ROSE_BUILD_FORTRAN_LANGUAGE_SUPPORT
check-hello-fortran:
	@echo "  TEST     ROSE Fortran compiler [rose-fortran]"
	${AM_V_at}echo "program hello"                            >  hello-world.f90
	${AM_V_at}echo "  print *, \"Hello World from Fortran!\"" >> hello-world.f90
	${AM_V_at}echo "end program hello"                        >> hello-world.f90
	${AM_V_at}./rose-fortran -rose:ast:write hello-world-f90.ast hello-world.f90 -o hello-world-f90
	${AM_V_at}./hello-world-f90
endif
if ROSE_EXPERIMENTAL_ADA_ROSE_CONNECTION
check-hello-ada:
	@echo "  TEST     ROSE Ada compiler [rose-ada]"
	${AM_V_at}echo "with Ada.Text_IO;"                        >  hello-world.adb
	${AM_V_at}echo "procedure Hello_World is"                 >> hello-world.adb
	${AM_V_at}echo "begin"                                    >> hello-world.adb
	${AM_V_at}echo "  Ada.Text_IO.Put_Line(\"Hello World\");" >> hello-world.adb
	${AM_V_at}echo "end Hello_World;"                         >> hello-world.adb
	${AM_V_at}./rose-ada hello-world.adb
endif


check-local: $(bin_PROGRAMS)
	${AM_V_at}$(MAKE) check-boost
	${AM_V_at}$(MAKE) check-dynload
	${AM_V_at}$(MAKE) check-config
if ROSE_BUILD_C_LANGUAGE_SUPPORT
	${AM_V_at}$(MAKE) check-hello-c
endif
if ROSE_BUILD_CXX_LANGUAGE_SUPPORT
	${AM_V_at}$(MAKE) check-hello-cxx
endif
if ROSE_BUILD_FORTRAN_LANGUAGE_SUPPORT
	${AM_V_at}$(MAKE) check-hello-fortran
endif
if ROSE_EXPERIMENTAL_ADA_ROSE_CONNECTION
	${AM_V_at}$(MAKE) check-hello-ada
endif

########################################################################################################################

# libtool installed to the bin directory to support linking an application
# using ROSE from a ROSE installation.
bin_SCRIPTS += ../libtool

# Do not remove libraries indiscriminately with wildcards [Matzke, 2017-06-19]
clean-local:
	rm -rf librose.a librose.so testSharedRoseLib testMultiRoseLib $(CXX_TEMPLATE_OBJECTS) Templates.DB ii_files ti_files
	rm -rf $(MOSTLYCLEANFILES)

distclean-local:
	rm -rf librose.a librose.so testSharedRoseLib testMultiRoseLib Templates.DB

cleanTest:
	rm -rf *.lo testSharedRoseLib testMultiRoseLib

# Note that uninstall-hook does not work, but uninstall-local does.
uninstall-local:
	rm -f $(DESTDIR)$(libdir)/libedg.so
	rm -f $(DESTDIR)$(libdir)/librose.so
	@echo "************************************************************"
	@echo "**** make uninstall rule complete (terminated normally) ****"
	@echo "************************************************************"


rebuildPrelinker:
if ROSE_BUILD_CXX_LANGUAGE_SUPPORT
	rm -f frontend/CxxFrontend/EDG/EDG_3.3/util/edg_prelink.lo
endif
	rm -f frontend/SageIII/Cxx_Grammar.lo
	rm -f roseSupport/templateSupport.lo
	$(MAKE)
