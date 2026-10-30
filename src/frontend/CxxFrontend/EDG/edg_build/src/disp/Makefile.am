# Edison Design Group C++ Front End.
# Makefile for IL display program (src/disp directory).
# This version uses an ANSI/ISO C compiler.
# (See Makefile.pcc for the version for old-style C compilers.)
#

# DQ (3/26/2017): Skip some EDG format warnings when using Clang
if USING_CLANG_COMPILER
CXXFLAGS += -Wno-format
endif

CXXFLAGS += -Ddisc_union=@edg_union_struct_debugging_value@

# bin_PROGRAMS = edgcpdisp

# DQ (12/24/2008): I don't see how using -DSTANDALONE_IL_DISPLAY would work (disallows definition of is_template_dependent_type() function)
# edgcpdisp_CXXFLAGS = -DSTANDALONE_IL_DISPLAY -DIL_SHOULD_BE_WRITTEN_TO_FILE -I$(srcdir)/.. -I.. $(CXXFLAGS)
# edgcpdisp_CXXFLAGS = -DIL_SHOULD_BE_WRITTEN_TO_FILE -I$(srcdir)/.. -I.. $(CXXFLAGS)
# edgcpdisp_CXXFLAGS = -I$(srcdir)/.. -I.. $(CXXFLAGS)
edgcpdisp_CXXFLAGS = -DSTANDALONE_IL_DISPLAY -DIL_SHOULD_BE_WRITTEN_TO_FILE -I$(srcdir)/.. -I.. $(CXXFLAGS)

sourceFiles = \
	const_ints.cc debug.cc error.cc fe_init.cc \
	fixed_pt.cc float_pt.cc host_envir.cc \
	il.cc il_display.cc il_read.cc il_to_str.cc il_walk.cc \
	mem_manage.cc target.cc types.cc

nodist_edgcpdisp_SOURCES = $(sourceFiles)

# Mark the edgcpdisp_SOURCES as nodist and distribute the source files using EXTRA_DIST
# This allows "make dist" to work on a binary distribution.
EXTRA_DIST = CMakeLists.txt $(sourceFiles)
