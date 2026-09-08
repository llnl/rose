include $(top_srcdir)/config/Makefile.for.ROSE.includes.and.libs

# override ROSE settings from AM_INIT_AUTOMAKE
AUTOMAKE_OPTIONS = subdir-objects

COUNTEREXAMPLES = counterexamples/ltl2haskell.sh counterexamples/Makefile counterexamples/qc.hs counterexamples/log2csv.awk
EXTRA_DIST = Doxyfile $(TESTS) $(COUNTEREXAMPLES)

#
bin_PROGRAMS = codethorn thorn1 thorn2 thorn3 thorn4 ltlthorn woodpecker addressTakenAnalysis cldemo

# matcher_demo matcher astinfo cldemo
noinst_PROGRAMS = addressTakenAnalysis cldemo matcher_demo
#noinst_PROGRAMS matcher_demo matcher astinfo cldemo

CLEANFILES =

if ROSE_WITH_SPOT
AM_LDFLAGS = $(ROSE_LIBS) $(LIBS_WITH_RPATH) -fopenmp -L$(SPOT_LIBRARY_PATH)
AM_CPPFLAGS = $(ROSE_INCLUDES) -I$(top_srcdir)/tools/Codethorn/src -I$(SPOT_INCLUDE_PATH)/spot -I$(SPOT_INCLUDE_PATH) -g -rdynamic
else
AM_LDFLAGS = $(ROSE_LIBS) $(LIBS_WITH_RPATH) -fopenmp
AM_CPPFLAGS = $(ROSE_INCLUDES) -I$(top_srcdir)/tools/Codethorn/src -g -rdynamic
endif

MYDATE = $(shell date +%Y_%m_%d)

lib_LTLIBRARIES = libcodethorn.la libltlthorn.la

##############################################################
# LIBRARY: CODETHORN (CORE)
##############################################################

# codethorn liabrary header files without corresponding source files
CODETHORN_SOLO_HEADER_FILES=\
  AnalysisParameters.h \
  CollectionOperators.h \
  CtxAnalysis.h \
  CtxAttribute.h \
  CtxLattice.h \
  CtxPropertyStateFactory.h \
  CtxTransfer.h \
  DataDependenceVisualizer.h \
  EqualityMaintainer.h \
  Evaluator.h \
  ExecutionTrace.h \
  ExternalMangling.h \
  HSetMaintainer.h \
  Memoizer.h \
  ReadWriteData.h \
  Utility.h \
  WorkListSeq.h

# codethorn library header files (each with a corresponding source file)
CODETHORN_HEADER_FILES=\
  AbstractValue.h \
  addressTakenAnalysis.h \
  AliasAnalysis.h \
  AstUtility.h \
  AnalysisAstAnnotator.h \
  AnalysisReporting.h \
  CTAnalysis.h \
  ArrayElementAccessData.h \
  AstNodeInfo.h \
  BoolLattice.h \
  CallString.h \
  CallGraphAnalysis.h \
  CallGraphExport.h \
  CFAnalysis.h \
  ClassHierarchyAnalysis.h \
  ObjectLayoutAnalysis.h \
  ClassHierarchyExport.h \
  ClassHierarchyWriter.h \
  CodeThornLib.h \
  CodeThornPasses.h \
  CommandLineOptions.h \
  ContNodeAttribute.h \
  CounterexampleGenerator.h \
  CPAstAttribute.h \
  CppLabeler.h \
  CppStdUtilities.h \
  CTIOLabeler.h \
  CtxCallStrings.h \
  CtxPrioritySolver.h \
  CtxReachabilityLattice.h \
  CtxSolver0.h \
  CtxUnfoldedAnalysis.h \
  DataDependenceVisualizer.h \
  defUseQuery.h \
  DFAbstractSolver.h \
  DFAnalysisBaseWithoutData.h \
  DFAnalysisBase.h \
  CSDFAnalysisBase.h \
  CSDFAnalysisBaseWithoutData.h \
  ConstantConditionAnalysis.h \
  DFTransferFunctions.h \
  DotGraphCfgFrontend.h \
  EState.h \
  EStateTransferFunctions.h \
  EStateWorkList.h \
  EStatePriorityWorkList.h \
  FIConstAnalysis.h \
  Flow.h \
  FunctionCallMapping.h \
  FunctionCallMapping2.h \
  FunctionCallTarget.h \
  FunctionId.h \
  FunctionIdMapping.h \
  GeneralWorkList.h \
  GeneralWorkList.C \
  GeneralPriorityWorkList.h \
  GeneralPriorityWorkList.C \
  InputOutput.h \
  InternalChecks.h \
  IOAnalyzer.h \
  IOSequenceGenerator.h \
  IOSolver.h \
  LanguageRestrictorCollection.h \
  LanguageRestrictor.h \
  Lattice.h \
  LoopInfo.h \
  Miscellaneous2.h \
  Miscellaneous.h \
  Normalization.h \
  NormalizationCxx.h \
  NormalizationInliner.h \
  NormalizationOp.h \
  NumberIntervalLattice.h \
  Options.h \
  CodeThornOptions.h \
  ParProOptions.h \
  LTLOptions.h \
  LTLRersMapping.h \
  DFSolver1.h \
  DFSolver2.h \
  MemoryViolationAnalysis.h \
  PointerAnalysisInterface.h \
  PragmaHandler.h \
  PredefinedSemanticFunctions.h \
  ProgramAbstractionLayer.h \
  ProgramInfo.h \
  ProgramLocationsReport.h \
  ProgramLocationsAnalysis.h \
  PropertyState.h \
  PropertyStateFactory.h \
  PropertyValueTable.h \
  PState.h \
  ReachabilityAnalysis.h \
  ReadWriteListener.h \
  ReadWriteTraceAnalysis.h \
  RersSpecialization.h \
  RERS_empty_specialization.h \
  RersCounterexample.h \
  RewriteStatistics.h \
  RewriteSystem.h \
  RoseCompatibility.h \
  Solver5.h \
  Solver16.h \
  Solver17.h \
  Solver18.h \
  Solver19.h \
  Solver8.h \
  Solver.h \
  Specialization.h \
  SvcompWitness.h \
  TimeMeasurement.h \
  TimingCollector.h \
  TopologicalSort.h \
  TransitionGraph.h \
  TransitionGraphReducer.h \
  TypeSizeMapping.h \
  VariableIdUtils.h \
  VariableIdMappingExtended.h \
  VariableValueMonitor.h \
  Visualizer.h

# codethorn library source files (each with a corresponding header file)
CODETHORN_SOURCE_FILES=\
  AbstractValue.C \
  addressTakenAnalysis.C \
  AliasAnalysis.C \
  AstUtility.C \
  AnalysisAstAnnotator.C \
  AnalysisReporting.C \
  CTAnalysis.C \
  ArrayElementAccessData.C \
  AstNodeInfo.C \
  BoolLattice.C \
  CallGraphAnalysis.C \
  CallGraphExport.C \
  CallString.C \
  CFAnalysis.C \
  ClassHierarchyAnalysis.C \
  ObjectLayoutAnalysis.C \
  ClassHierarchyExport.C \
  ClassHierarchyWriter.C \
  CodeThornLib.C \
  CodeThornPasses.C \
  CommandLineOptions.C \
  ContNodeAttribute.C \
  CounterexampleGenerator.C \
  CPAstAttribute.C \
  CppLabeler.C \
  CppStdUtilities.C \
  CTIOLabeler.C \
  CtxCallStrings.C \
  CtxPrioritySolver.C \
  CtxReachabilityLattice.C \
  CtxSolver0.C \
  CtxUnfoldedAnalysis.C \
  DataDependenceVisualizer.C \
  defUseQuery.C \
  DFAnalysisBaseWithoutData.C \
  DFAnalysisBase.C \
  CSDFAnalysisBase.C \
  ConstantConditionAnalysis.C \
  DFTransferFunctions.C \
  DotGraphCfgFrontend.C \
  EState.C \
  EStateTransferFunctions.C \
  EStateWorkList.C \
  EStatePriorityWorkList.C \
  FIConstAnalysis.C \
  Flow.C \
  FunctionCallMapping.C \
  FunctionCallMapping2.C \
  FunctionCallTarget.C \
  FunctionId.C \
  FunctionIdMapping.C \
  InputOutput.C \
  InternalChecks.C \
  IOAnalyzer.C \
  IOSequenceGenerator.C \
  IOSolver.C \
  LanguageRestrictorCollection.C \
  LanguageRestrictor.C \
  Lattice.C \
  LoopInfo.C \
  CodeThornOptions.C \
  LTLOptions.C \
  ParProOptions.C \
  LTLRersMapping.C \
  Miscellaneous2.C \
  Miscellaneous.C \
  Normalization.C \
  NormalizationCxx.C \
  NormalizationInliner.C \
  NormalizationOp.C \
  NumberIntervalLattice.C \
  DFSolver1.C \
  DFSolver2.C \
  MemoryViolationAnalysis.C \
  PointerAnalysisInterface.C \
  PragmaHandler.C \
  PredefinedSemanticFunctions.C \
  ProgramAbstractionLayer.C \
  ProgramInfo.C \
  ProgramLocationsReport.C \
  ProgramLocationsAnalysis.C \
  PropertyState.C \
  PropertyStateFactory.C \
  PropertyValueTable.C \
  PState.C \
  ReachabilityAnalysis.C \
  ReadWriteListener.C \
  ReadWriteTraceAnalysis.C \
  RERS_empty_specialization.C \
  RersCounterexample.C \
  RewriteStatistics.C \
  RewriteSystem.C \
  RoseCompatibility.C \
  Solver5.C \
  Solver16.C \
  Solver17.C \
  Solver18.C \
  Solver19.C \
  Solver8.C \
  Solver.C \
  Specialization.C \
  SvcompWitness.C \
  TimeMeasurement.C \
  TimingCollector.C \
  TopologicalSort.C \
  TransitionGraph.C \
  TransitionGraphReducer.C \
  TypeSizeMapping.C \
  VariableIdUtils.C \
  VariableIdMappingExtended.C \
  VariableValueMonitor.C \
  Visualizer.C

#libcodethorn_la_LDFLAGS= -no-undefined
libcodethorn_la_LDFLAGS=
libcodethorn_la_CXXFLAGS = -fopenmp -Wall -O3

# codethorn header files are installed in <installpath>/rose/codethorn
codethornincludedir = $(pkgincludedir)/codethorn
nobase_codethorninclude_HEADERS=\
  $(CODETHORN_SOLO_HEADER_FILES) \
  $(CODETHORN_HEADER_FILES)

# codethorn (core) library sources
libcodethorn_la_SOURCES=\
  $(CODETHORN_SOURCE_FILES)

################################################################
# LIBRARY: LTLTHORN/ (for linear temporal logic verification)
################################################################

libltlthorn_la_LDFLAGS=
libltlthorn_la_CXXFLAGS = -fopenmp -Wall -O3

LTLTHORN_SOLO_HEADER_FILES=
LTLTHORN_HEADER_FILES=\
  ltlthorn-lib/CounterexampleAnalyzer.h \
  ltlthorn-lib/LtsminConnection.h \
  ltlthorn-lib/ParProAutomataGenerator.h \
  ltlthorn-lib/ParProAutomata.h \
  ltlthorn-lib/ParProAnalyzer.h \
  ltlthorn-lib/ParProEState.h \
  ltlthorn-lib/ParProExplorer.h \
  ltlthorn-lib/ParProLtlMiner.h \
  ltlthorn-lib/ParProSpotState.h \
  ltlthorn-lib/ParProSpotSuccIter.h \
  ltlthorn-lib/ParProSpotTgba.h \
  ltlthorn-lib/ParProTransitionGraph.h \
  ltlthorn-lib/PromelaCodeGenerator.h \
  ltlthorn-lib/Solver10.h \
  ltlthorn-lib/Solver11.h \
  ltlthorn-lib/Solver12.h \
  ltlthorn-lib/SpotConnection.h \
  ltlthorn-lib/SpotMiscellaneous.h \
  ltlthorn-lib/SpotRenameVisitor.h \
  ltlthorn-lib/SpotState.h \
  ltlthorn-lib/SpotSuccIter.h \
  ltlthorn-lib/SpotTgba.h

LTLTHORN_SOURCE_FILES=\
  ltlthorn-lib/CounterexampleAnalyzer.C \
  ltlthorn-lib/LtsminConnection.C \
  ltlthorn-lib/ParProAutomataGenerator.C \
  ltlthorn-lib/ParProAutomata.C \
  ltlthorn-lib/ParProAnalyzer.C \
  ltlthorn-lib/ParProEState.C \
  ltlthorn-lib/ParProExplorer.C \
  ltlthorn-lib/ParProLtlMiner.C \
  ltlthorn-lib/ParProSpotState.C \
  ltlthorn-lib/ParProSpotSuccIter.C \
  ltlthorn-lib/ParProSpotTgba.C \
  ltlthorn-lib/ParProTransitionGraph.C \
  ltlthorn-lib/PromelaCodeGenerator.C \
  ltlthorn-lib/Solver10.C \
  ltlthorn-lib/Solver11.C \
  ltlthorn-lib/Solver12.C \
  ltlthorn-lib/SpotConnection.C \
  ltlthorn-lib/SpotMiscellaneous.C \
  ltlthorn-lib/SpotRenameVisitor.C \
  ltlthorn-lib/SpotState.C \
  ltlthorn-lib/SpotSuccIter.C \
  ltlthorn-lib/SpotTgba.C

# only informative (not installed)
libltlthorn_xHEADERS = \
  $(LTLTHORN_SOLO_HEADER_FILES) \
  $(LTLTHORN_HEADER_FILES) \
  LTLThornLib.h

# library sources
libltlthorn_la_SOURCES= \
  $(LTLTHORN_SOURCE_FILES) \
  LTLThornLib.C


##############################################################
# TOOL: CODETHORN
##############################################################

# yacc flags for LTL parser
codethorn_YFLAGS = -p ltl_

# this line creates speparately named object files
codethorn_CXXFLAGS = -fopenmp -Wall -O3

# codethorn has most of its code organized in two separate libraries
codethorn_DEPENDENCIES=libcodethorn.la libltlthorn.la

if ROSE_WITH_SPOT
#spot and bdd need to be linked last with gcc 5.4
codethorn_LDADD = $(LIBS_WITH_RPATH) $(ROSE_LIBS) -lcodethorn -lltlthorn -lspot -lbdd
codethorn_LDFLAGS = -L$(SPOT_LIBRARY_PATH)
else
codethorn_LDADD = $(LIBS_WITH_RPATH) $(ROSE_LIBS) -lcodethorn -lltlthorn
endif

codethorndir=.

# only informative
codethorn_xHEADERS = \
  CommandLineOptions.h \
  CodeThornCommandLineOptions.h \
  codethorn.h \
  LTL.h

# codethorn sources in addition to libcodethorn and libltlthorn
codethorn_SOURCES = \
  LTLParser.y++ \
  CodeThornCommandLineOptions.C \
  codethorn.C

##############################################################
# TOOL: LTLTHORN
##############################################################

# yacc flags for LTL parser
ltlthorn_YFLAGS = -p ltl_

# this line creates speparately named object files
ltlthorn_CXXFLAGS = -fopenmp -Wall -O3

# ltlthorn has most of its code organized in two separate libraries
ltlthorn_DEPENDENCIES=libcodethorn.la libltlthorn.la

if ROSE_WITH_SPOT
#spot and bdd need to be linked last with gcc 5.4
ltlthorn_LDADD = $(LIBS_WITH_RPATH) $(ROSE_LIBS) -lcodethorn -lltlthorn -lspot -lbdd
ltlthorn_LDFLAGS = -L$(SPOT_LIBRARY_PATH)
else
ltlthorn_LDADD = $(LIBS_WITH_RPATH) $(ROSE_LIBS) -lcodethorn -lltlthorn
endif

ltlthorndir=.

# only informative
ltlthorn_xHEADERS = \
  CommandLineOptions.h \
  LTLThornCommandLineOptions.h \
  codethorn.h \
  LTL.h

# codethorn sources in addition to libcodethorn and libltlthorn
ltlthorn_SOURCES = \
  LTLParser.y++ \
  LTLThornCommandLineOptions.C \
  ltlthorn.C


##############################################################
# TOOL: THORN1
##############################################################

# yacc flags for LTL parser
thorn1_YFLAGS = -p ltl_

# this line creates speparately named object files
thorn1_CXXFLAGS = -fopenmp -Wall -O3

# thorn1 has most of its code organized in two separate libraries
thorn1_DEPENDENCIES=libcodethorn.la libltlthorn.la

if ROSE_WITH_SPOT
#spot and bdd need to be linked last with gcc 5.4
thorn1_LDADD = $(LIBS_WITH_RPATH) $(ROSE_LIBS) -lcodethorn -lltlthorn -lspot -lbdd
thorn1_LDFLAGS = -L$(SPOT_LIBRARY_PATH)
else
thorn1_LDADD = $(LIBS_WITH_RPATH) $(ROSE_LIBS) -lcodethorn -lltlthorn
endif

thorn1dir=.

# only informative
thorn1_xHEADERS = \
  CommandLineOptions.h \
  Thorn1CommandLineOptions.h \
  codethorn.h \
  LTL.h

# codethorn sources in addition to libcodethorn and libltlthorn
thorn1_SOURCES = \
  LTLParser.y++ \
  Thorn1CommandLineOptions.C \
  thorn1.C

###################################################################
# TOOL: THORN2 class hierarchy analysis (formerly known as Acuity)
###################################################################

# thorn2 uses libcodethorn
thorn2_DEPENDENCIES=libcodethorn.la

thorn2_LDADD = $(LIBS_WITH_RPATH) $(ROSE_LIBS) -lcodethorn

# this line creates speparately named object files, fopenmp to silence warnings about openmp directives
thorn2_CXXFLAGS = -fopenmp -Wall -O3

# codethorn sources in addition to libcodethorn
thorn2_SOURCES = \
  CodeThornCommandLineOptions.C \
  thorn2.C

###################################################################
# TOOL: THORN3 create dumps of ROSE ASTs and data structures
###################################################################

# thorn3 uses libcodethorn
thorn3_DEPENDENCIES=libcodethorn.la

thorn3_LDADD = $(LIBS_WITH_RPATH) $(ROSE_LIBS) -lcodethorn

# this line creates speparately named object files, fopenmp is required
thorn3_CXXFLAGS = -fopenmp -Wall -O3

# codethorn sources in addition to libcodethorn
thorn3_SOURCES = \
  thorn3.C


###################################################################
# TOOL: THORN4 create graph visualizations of state transition systems
###################################################################

# only informative
thorn4_xHEADERS = \
  Thorn4CommandLineParser.h

# thorn4 uses libcodethorn
thorn4_DEPENDENCIES=libcodethorn.la

thorn4_LDADD = $(LIBS_WITH_RPATH) $(ROSE_LIBS) -lcodethorn

# this line creates speparately named object files, fopenmp is required
thorn4_CXXFLAGS = -fopenmp -Wall -O3

# codethorn sources in addition to libcodethorn
thorn4_SOURCES = \
  Thorn4CommandLineParser.C \
  thorn4.C


##############################################################
# TOOL: WOODPECKER
##############################################################

woodpecker_DEPENDENCIES=libcodethorn.la
if ROSE_WITH_SPOT
woodpecker_LDADD = $(LIBS_WITH_RPATH) $(ROSE_LIBS)  -lcodethorn
woodpecker_LDFLAGS = -L$(SPOT_LIBRARY_PATH)
else
woodpecker_LDADD = $(LIBS_WITH_RPATH) $(ROSE_LIBS)  -lcodethorn
endif
woodpecker_CXXFLAGS= -fopenmp -Wall -O3
woodpecker_SOURCES = \
  shared-src/ProgramStats.h \
  shared-src/ProgramStats.C \
  woodpecker-src/Threadification.h \
  woodpecker-src/Threadification.C \
  woodpecker-src/TrivialInlining.C \
  woodpecker-src/TrivialInlining.h \
  woodpecker-src/DeadCodeElimination.C \
  woodpecker-src/DeadCodeElimination.h \
  woodpecker-src/ConversionFunctionsGenerator.h \
  woodpecker-src/ConversionFunctionsGenerator.C \
  woodpecker-src/woodpecker.C

roseonlytools: woodpecker

##############################################################
# TOOL: ADDRESSTAKENANALYSIS (for testing only)
##############################################################

if ROSE_WITH_SPOT
addressTakenAnalysis_LDADD = $(LIBS_WITH_RPATH) $(ROSE_LIBS)  -lcodethorn -lboost_program_options
addressTakenAnalysis_LDFLAGS = -L$(SPOT_LIBRARY_PATH)
else
addressTakenAnalysis_LDADD = $(LIBS_WITH_RPATH) $(ROSE_LIBS)  -lcodethorn -lboost_program_options
endif
addressTakenAnalysis_CXXFLAGS = -fopenmp -Wall -O3
addressTakenAnalysis_DEPENDENCIES=libcodethorn.la

REGRESSION_DATA_DIR=regressiondata

addressTakenAnalysis_SOURCES = \
  addressTakenAnalysisDriver.C

.PHONY: codethorn-dist vis bsps test checkdemos

# obsolete since integration into ROSE
# explicit rules because of multiple bison parser
#matcherparser.C: $(srcdir)/matcherlexer.ll $(srcdir)/matcherparser.yy
#	$(YACC) $(YFLAGS) -d -p matcherparser $(srcdir)/matcherparser.yy # generates y.tab.h/c
#	$(LEX) $(LFLAGS) $(AM_LFLAGS) -Pmatcherparser $(srcdir)/matcherlexer.ll
#	mv y.tab.h matcherparser.h
#	cat lex.yy.c y.tab.c > matcherparser.C
#	rm lex.yy.c y.tab.c

# MS: matcher_demo
matcher_demo_DEPENDENCIES=libcodethorn.la
matcher_demo_LDADD = $(LIBS_WITH_RPATH) $(ROSE_LIBS) -lcodethorn
matcher_demo__CXXFLAGS = -Wall -O3 -march=native -ftree-vectorize
matcher_demo_SOURCES = matcher/matcher_demo.C
#BUILT_SOURCES =
#AM_YFLAGS =
#AM_LFLAGS =

#matcher_DEPENDENCIES=libcodethorn.la
#matcher_LDADD = $(LIBS_WITH_RPATH) $(ROSE_LIBS) -lcodethorn
#matcher__CXXFLAGS = -Wall -O3 -march=native -ftree-vectorize
#matcher_SOURCES = matcher.C

cldemo_LDADD = $(LIBS_WITH_RPATH) $(ROSE_LIBS)
cldemo__CXXFLAGS = -Wall -O3 -march=native -ftree-vectorize
cldemo_SOURCES = cldemo.C

#MS: iterator_test not ingegrated yet
#iterator_test_SOURCES = iterator_test.C ShowSeq.h
#iterator_test_LDADD = -lrose libmatcher.la

#astinfo_DEPENDENCIES=
#astinfo_LDADD = $(LIBS_WITH_RPATH) $(ROSE_LIBS)
#astinfo_SOURCES = astinfo.C LineColInfo.C

#MS: ast_demo not integrated yet
#ast_demo_SOURCES = ast_demo.C TimeMeasurement.C TimeMeasurement.h  RoseAst.C RoseAst.h
#ast_demo_LFLAGS = -Pmatcher
#ast_demo_YFLAGS = -d -p matcher

docs:
	cd "$(srcdir)" && doxygen

clean-thorn4:
	rm -rf thorn4-test-concrete thorn4-test-abstract

clean-local: clean-thorn4
	rm -f *.dot *.dat
	rm -f *.ps
	rm -f *.jpg
	rm -f vis/*
	rm -f bsps/*
	rm -f *.consistent Problem*.[0-9].csv
	rm -f codethorn-LTLParser.c++
	rm -f tmp.nsdump
	rm -f rose_Problem1401_opt.pp.c
	rm -f Problem1401_assert_results_forcedtop_1000_n_bf.csv
	rm -f transitiongraph1.pdf
	rm -f transitiongraph2.pdf
	rm -f rose_NORM*.[Cc]
	rm -f LTLParser.c++
	rm -f *.cfg.dot # runParCfgTests
	rm -f a.out
	rm -rf ../docs/doxygen
	rm -f *.imout*
	rm -f *.exe
	rm -f *.imout.check
	rm -Rf make_check_log_*
	rm -f *-LTLParser.c++
	rm -f *.csv
	rm -f *.txt
	rm -f *.ti

distclean-local: clean
	rm -f *.tgz
	rm -f *~
	rm -f codethornref

codethorn-dist:
	tar cvzf codethorn_$(MYDATE).tgz *.C *cpp *.h *.lxx *.yxx tests/*.C tests/*.c Makefile*

demo:
	$(srcdir)/demodir/rundemo.sh $(srcdir)/demodir $(top_builddir)/tools/CodeThorn
