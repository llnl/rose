# Build the EDG C/C++ front end from the edg_compiler submodule (upstream layout) as the
# libtool convenience library libedg.la, configured for ROSE.
#
# EDG's sources are *.c files that are compiled as C++ (that is how EDG itself builds them:
# "g++ -std=c++14 *.c").  We keep automake's C rules and add "-x c++" so that the compiler
# treats every EDG source as C++; the two helper programs are linked with $(CXX).
#
# EDG's own src/defines.h is not edited.  Since EDG 7.0 (back-ported to ROSE's 6.5 branch)
# defines.h includes a generated "cmake_defines.h" when USE_CMAKE_DEFINES is defined; ROSE
# generates that file here from defines.h.rose plus the host/target description printed by
# EDG's dettarg utility.
#
# Stages (same as the former EDG_<version>/{misc,src}/Makefile.am):
#   1. lldef            does the host C compiler support "long long"?         (lldef.sh)
#   2. dettarg          EDG's target-description program                      (util/dettarg.c)
#   3. dettarg.out      its output                                            (dettarg.out.sh)
#   4. cmake_defines.h  = defines.h.rose + dettarg.out
#   5. mk_errinfo       EDG's error-table generator                           (util/mk_errinfo.c)
#   6. err_codes.h, err_data.h  from src/error_msg.txt and src/error_tag.txt
#   7. libedg.la        from src/*.c (all front-end sources except the stand-alone drivers
#                       cfe.c and c_gen_be.c, and floating.c, which is only for hosts
#                       without native floating point)
#
# The same list of EDG sources is kept in CMakeLists.txt; update both when EDG adds a file.

include $(top_srcdir)/config/Makefile.for.ROSE.includes.and.libs

AUTOMAKE_OPTIONS = subdir-objects

# Location of the EDG source (the edg_compiler submodule), relative to this directory.
EDG_SRC = ../edg_compiler

# Compile EDG's C files as C++ with the flags ROSE uses for C++.
AM_CFLAGS = -x c++ $(CXX_FLAGS) $(CXX_DEBUG) $(CXX_OPTIMIZE) $(CXX_WARNINGS) -fPIC
if USING_CLANG_COMPILER
AM_CFLAGS += -Wno-format
endif

AM_CPPFLAGS = \
   -DUSE_CMAKE_DEFINES \
   -DMAKE_FRONT_END_CALLABLE=TRUE \
   -DGCC_VERSION=@GCC_VERSION@ \
   -DGCC_MINOR_VERSION=@GCC_MINOR_VERSION@ \
   -Ddisc_union=@edg_union_struct_debugging_value@ \
   -I$(builddir) \
   -I$(srcdir)/$(EDG_SRC)/src

# Generated before anything else is compiled (see also the explicit object dependencies below,
# which matter for parallel builds and for "make libedg.la").
BUILT_SOURCES = lldef cmake_defines.h err_codes.h err_data.h

noinst_PROGRAMS = dettarg mk_errinfo
noinst_LTLIBRARIES = libedg.la

#------------------------------------------------------------------------------------------
# 1. lldef: "-DHAVE_LONG_LONG=0|1" for dettarg
#------------------------------------------------------------------------------------------
lldef: $(srcdir)/lldef.sh
	CC="$(CC)" $(SHELL) $(srcdir)/lldef.sh

#------------------------------------------------------------------------------------------
# 2./3. dettarg and dettarg.out
#------------------------------------------------------------------------------------------
nodist_dettarg_SOURCES = $(EDG_SRC)/util/dettarg.c
dettarg_CFLAGS = $(AM_CFLAGS) `cat lldef`
dettarg_LINK = $(CXX) $(AM_LDFLAGS) $(LDFLAGS) -o $@
$(dettarg_OBJECTS): lldef

dettarg.out: dettarg$(EXEEXT) lldef $(srcdir)/dettarg.out.sh
	BINDIR=. $(SHELL) $(srcdir)/dettarg.out.sh .

#------------------------------------------------------------------------------------------
# 4. cmake_defines.h: ROSE's EDG configuration plus the target description
#------------------------------------------------------------------------------------------
cmake_defines.h: $(srcdir)/defines.h.rose dettarg.out
	cat $(srcdir)/defines.h.rose dettarg.out > $@.tmp && mv $@.tmp $@

#------------------------------------------------------------------------------------------
# 5./6. mk_errinfo and the error tables
#------------------------------------------------------------------------------------------
nodist_mk_errinfo_SOURCES = $(EDG_SRC)/util/mk_errinfo.c
mk_errinfo_LINK = $(CXX) $(AM_LDFLAGS) $(LDFLAGS) -o $@
$(mk_errinfo_OBJECTS): cmake_defines.h

err_codes.h: mk_errinfo$(EXEEXT) $(srcdir)/$(EDG_SRC)/src/error_msg.txt $(srcdir)/$(EDG_SRC)/src/error_tag.txt
	./mk_errinfo$(EXEEXT) $(srcdir)/$(EDG_SRC)/src/error_msg.txt $(srcdir)/$(EDG_SRC)/src/error_tag.txt err_codes.h err_data.h

# Produced by the same mk_errinfo run.
err_data.h: err_codes.h

#------------------------------------------------------------------------------------------
# 7. libedg.la
#------------------------------------------------------------------------------------------
nodist_libedg_la_SOURCES = \
   $(EDG_SRC)/src/attribute.c \
   $(EDG_SRC)/src/cfe_daemon.c \
   $(EDG_SRC)/src/class_decl.c \
   $(EDG_SRC)/src/cmd_line.c \
   $(EDG_SRC)/src/const_ints.c \
   $(EDG_SRC)/src/cp_gen_be.c \
   $(EDG_SRC)/src/debug.c \
   $(EDG_SRC)/src/decl_inits.c \
   $(EDG_SRC)/src/decl_spec.c \
   $(EDG_SRC)/src/declarator.c \
   $(EDG_SRC)/src/decls.c \
   $(EDG_SRC)/src/def_arg.c \
   $(EDG_SRC)/src/disambig.c \
   $(EDG_SRC)/src/error.c \
   $(EDG_SRC)/src/expr.c \
   $(EDG_SRC)/src/exprutil.c \
   $(EDG_SRC)/src/extasm.c \
   $(EDG_SRC)/src/fe_init.c \
   $(EDG_SRC)/src/fe_wrapup.c \
   $(EDG_SRC)/src/fixed_pt.c \
   $(EDG_SRC)/src/float_pt.c \
   $(EDG_SRC)/src/folding.c \
   $(EDG_SRC)/src/func_def.c \
   $(EDG_SRC)/src/host_envir.c \
   $(EDG_SRC)/src/ifc_modules.c \
   $(EDG_SRC)/src/ifc_modules_templ.c \
   $(EDG_SRC)/src/ifc_map_functions.c \
   $(EDG_SRC)/src/ifc_map_functions_acc.c \
   $(EDG_SRC)/src/ifc_map_functions_val.c \
   $(EDG_SRC)/src/ifc_map_functions_dbg.c \
   $(EDG_SRC)/src/ifc_map_functions_mut.c \
   $(EDG_SRC)/src/ifc_modules_read.c \
   $(EDG_SRC)/src/ifc_modules_write.c \
   $(EDG_SRC)/src/il.c \
   $(EDG_SRC)/src/il_alloc.c \
   $(EDG_SRC)/src/il_display.c \
   $(EDG_SRC)/src/il_read.c \
   $(EDG_SRC)/src/il_to_str.c \
   $(EDG_SRC)/src/il_walk.c \
   $(EDG_SRC)/src/il_write.c \
   $(EDG_SRC)/src/inline.c \
   $(EDG_SRC)/src/interpret.c \
   $(EDG_SRC)/src/layout.c \
   $(EDG_SRC)/src/lexical.c \
   $(EDG_SRC)/src/literals.c \
   $(EDG_SRC)/src/lookup.c \
   $(EDG_SRC)/src/lower_c99.c \
   $(EDG_SRC)/src/lower_eh.c \
   $(EDG_SRC)/src/lower_il.c \
   $(EDG_SRC)/src/lower_init.c \
   $(EDG_SRC)/src/lower_name.c \
   $(EDG_SRC)/src/macro.c \
   $(EDG_SRC)/src/mem_manage.c \
   $(EDG_SRC)/src/modules.c \
   $(EDG_SRC)/src/ms_attrib.c \
   $(EDG_SRC)/src/overload.c \
   $(EDG_SRC)/src/pch.c \
   $(EDG_SRC)/src/pragma.c \
   $(EDG_SRC)/src/preproc.c \
   $(EDG_SRC)/src/scope_stk.c \
   $(EDG_SRC)/src/src_seq.c \
   $(EDG_SRC)/src/statements.c \
   $(EDG_SRC)/src/symbol_ref.c \
   $(EDG_SRC)/src/symbol_tbl.c \
   $(EDG_SRC)/src/sys_predef.c \
   $(EDG_SRC)/src/target.c \
   $(EDG_SRC)/src/templates.c \
   $(EDG_SRC)/src/trans_copy.c \
   $(EDG_SRC)/src/trans_corresp.c \
   $(EDG_SRC)/src/trans_unit.c \
   $(EDG_SRC)/src/types.c \
   $(EDG_SRC)/src/unicode_name_fsm.c

$(libedg_la_OBJECTS): cmake_defines.h err_codes.h err_data.h

#------------------------------------------------------------------------------------------
# Housekeeping
#------------------------------------------------------------------------------------------
EXTRA_DIST = CMakeLists.txt defines.h.rose defines.h.win32.rose \
             lldef.sh dettarg.out.sh concat.cmake generate_dettarg.out.cmake

CLEANFILES = lldef lltest.c a.out dettarg.out cmake_defines.h cmake_defines.h.tmp \
             err_codes.h err_data.h mk_errinfo.pcc.c
