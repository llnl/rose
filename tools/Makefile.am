# Build the ROSE-based tools
include $(top_srcdir)/config/Makefile.for.ROSE.includes.and.libs

SUBDIRS = globalVariablesInLambdas classMemberVariablesInLambdas checkFortranInterfaces BinaryAnalysis featureVector PortabilityTesting CodeThorn ReadWriteSetGeneration 

EXTRA_DIST = 

# install the tools in 'bin'
bin_SCRIPTS = 

AM_CPPFLAGS = $(ROSE_INCLUDES)
AM_LDFLAGS = $(ROSE_RPATHS)
LDADD = $(ROSE_LIBS)

EXTRA_DIST += tests

#-------------- only for C/C++
if ROSE_BUILD_CXX_LANGUAGE_SUPPORT

bin_PROGRAMS = moveDeclarationToInnermostScope rajaChecker

moveDeclarationToInnermostScope_SOURCES = moveDeclarationToInnermostScope.C
rajaChecker_SOURCES                       = rajaChecker.C

#----------testing part 
#  rose_inputrajaChecker.C 
# To test an individual input, type 'make insideIfStmt.cpp.output'
Checker_Test_Files =
Checker_Test_Files += \
   nodalAccPatternForAllWrapper.cpp \
   nodalAccPattern1.cpp \
   nodalAccPattern2.cpp \
   nodalAccPattern-localIndex.cpp \
   nodalAccPattern-min-max.cpp \
   constDouble.cpp \
   indirectArrayAccess.cpp \
   indirectLambdaCall.cpp \
   referenceType.cpp \
   indirecArrayInLambda.cpp \
   dataMemberAccumulation.cpp \
   declaredAsFuncParameter.cpp \
   withoutSharedBase.cpp \
   nodalAccPattern3.cpp \
   nodalAccPattern4.cpp \
   definedTwice.cpp \
   insideForLoop.cpp \
   integerPattern.cpp \
   insideIfStmt.cpp \
   nodalAccPattern5-multipleAssign.cpp

# special options for raja checker 
rose_inputrajaChecker.C: $(top_srcdir)/tools/tests/inputrajaChecker.C rajaChecker
	./rajaChecker -rose:Cxx11_only -c $<

# enable diff based correctness checking
DIFF=diff
REFERENCE_PATH=$(top_srcdir)/tools/tests/references
# generate .cpp.output,  diff them with reference files, store the diffs into .cpp.diff
Checker_Output_Files=$(Checker_Test_Files:.cpp=.cpp.output)
Checker_Output_Diff_Files=$(Checker_Test_Files:.cpp=.cpp.diff)

$(Checker_Output_Files): rajaChecker
	./rajaChecker --debug --report report-$(@:%.output=%).txt -std=c++11 -c $(top_srcdir)/tools/tests/$(@:%.output=%) &>$(@:%.output=%).screen.log

#  each diff file depends on its own output file, not all generated examples!!
$(Checker_Output_Diff_Files): %.diff:%.output
	if $(DIFF) $(@:.cpp.diff=.cpp.output) $(REFERENCE_PATH)/$(@:.cpp.diff=.cpp.output) > $@ ; then echo "Test Passed" ; else echo "Files differ; test failed"; cat $@; rm -rf $@; exit 1; fi

#short cut to test rajaChecker
rajaChecker_diff: $(Checker_Output_Diff_Files)

# C++ 11 features need GCC 4.9 and EDG 4.12 and later
# Liao 4/10/2017
if USING_GNU_COMPILER
if ROSE_USING_GCC_VERSION_LATER_4_9
check: # $(Checker_Output_Diff_Files)
endif
endif

.phony: rose_clean_local
rose_clean_local:

clean-local: rose_clean_local
	rm -rf rose_*.C rose_*.cpp rose*.cxx *.diff *.output *.txt *.o *.log
endif	
