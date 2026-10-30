# Edison Design Group C++ Front End
# Makefile for miscellaneous tools used in the build process.

CFLAGS += -Ddisc_union=@edg_union_struct_debugging_value@

# noinst_PROGRAMS = c_deproto mk_errinfo dettarg
noinst_PROGRAMS = mk_errinfo dettarg

# dist_noinst_SCRIPTS = make_predef_macro_table
nodist_noinst_SCRIPTS = make_predef_macro_table

# DQ (2/2/2010): Not clear how this got commented out.
# BUILT_SOURCES = config # ../src/defines.h
BUILT_SOURCES = config # ../src/defines.h

# Filter program that removes C prototypes so code can be compiled with an
# old-style C compiler.
# c_deproto_SOURCES = c_deproto.c

# DQ (12/23/2008): Commented out references to c_deproto.c (seems to not be available in EDG 4.0)
# nodist_c_deproto_SOURCES = c_deproto.c

# Tool program that builds the error include files from the .txt files.
# mk_errinfo_SOURCES = mk_errinfo.cc
nodist_mk_errinfo_SOURCES = mk_errinfo.cc
mk_errinfo_DEPENDENCIES = mk_errinfo.cc config
mk_errinfo_CXXFLAGS = -I$(srcdir)/../src -I../src

# See whether the host compiler supports long long
lldef:
	CC="$(CC)" $(srcdir)/lldef.sh

# Program that determines some configuration attributes of the host system.
# dettarg_SOURCES = dettarg.c
nodist_dettarg_SOURCES = dettarg.cc
dettarg_DEPENDENCIES = dettarg.cc lldef
dettarg_CFLAGS = `cat lldef`

# DQ (2/2/2010): Added more explicit dependence.
dettarg.cc: lldef

dettarg.out: dettarg lldef
	$(srcdir)/dettarg.out.sh .

# Run dettarg and append its output to the defines.h.proto file in the src
# directory to create a defines.h file.
#
# If the host C compiler does not support "long long", force
# use of simulated integers.
config: dettarg.out
	cat $(srcdir)/../src/defines.h.rose dettarg.out > ../src/defines.h; \
	echo "Default defines.h file created.";

# DQ (7/10/2013): I modified the default to use defines.h.rose which is a copy of defines.h.linux
#	  cat $(srcdir)/../src/defines.h.proto dettarg.out > ../src/defines.h; 

CLEANFILES = dettarg.out mk_errinfo.pcc.c lltest.c lldef a.out
EXTRA_DIST = CMakeLists.txt make_predef_macro_table mk_errinfo.cc dettarg.cc
