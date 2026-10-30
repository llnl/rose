# Edison Design Group C++ Front End.
# Makefile for front end proper (src directory).
# This makefile makes a front end that writes/reads an IL file
# and a standalone program that will display the IL
# in human-readable form.  The latter is made by a makefile in
# the "disp" subdirectory.
#

# Note that if you set your own value for CFLAGS you're not required to
# set IL_SHOULD_BE_WRITTEN_TO_FILE.  If you don't, the IL display program
# is built anyway, but you won't be able to use it because you won't be
# able to make the front end write out an IL file.
# JJW: this is in src/disp/Makefile.am now

# DQ (3/26/2017): Skip some EDG format warnings when using Clang
if USING_CLANG_COMPILER
CXXFLAGS += -Wno-format
endif

AM_CXXFLAGS = $(CXX_FLAGS) $(CXX_DEBUG) $(CXX_OPTIMIZE) $(CXX_WARNINGS) -Ddisc_union=@edg_union_struct_debugging_value@

# DQ (10/20/2015): This causes a warning (as "redefined") with where it is set to 0 in defines.h.
# This is also inconsistatn with how this is handled within EDG 4.7 (so this is commented out).
# AM_CXXFLAGS += -DENABLE_CUDA_SUPPORT=1

# noinst_PROGRAMS = cp_gen_be edgcpfe
# noinst_PROGRAMS = cp_gen_be
noinst_LTLIBRARIES = libedg.la

# DQ (7/11/2013): We don't need to compile the src/disp directory (EDG IL display program).
# SUBDIRS = disp
SUBDIRS = disp

cp_gen_be_CXXFLAGS = \
  $(AM_CXXFLAGS) \
  -DIL_SHOULD_BE_WRITTEN_TO_FILE=FALSE \
  -DMAKE_FRONT_END_CALLABLE=FALSE \
  -DNEED_NAME_MANGLING \
  -DGCC_VERSION=@GCC_VERSION@ \
  -DGCC_MINOR_VERSION=@GCC_MINOR_VERSION@

# DQ (3/1/2017): Note that Clang on ubuntu 16.04 systems appears to require -fPIC (at least sometimes).
# See ROSE Matrix testing for 2/28/2017 for more about this error.
libedg_la_CXXFLAGS = \
  $(AM_CXXFLAGS) \
  -DMAKE_FRONT_END_CALLABLE=TRUE \
  -DGCC_VERSION=@GCC_VERSION@ \
  -DGCC_MINOR_VERSION=@GCC_MINOR_VERSION@ \
  -fPIC

edgcpfe_CXXFLAGS = \
  $(AM_CXXFLAGS) \
  -DIL_SHOULD_BE_WRITTEN_TO_FILE=TRUE \
  -DMAKE_FRONT_END_CALLABLE=FALSE \
  -DNEED_NAME_MANGLING \
  -DGCC_VERSION=@GCC_VERSION@ \
  -DGCC_MINOR_VERSION=@GCC_MINOR_VERSION@ \
  -DBACK_END_SHOULD_BE_CALLED=FALSE \
  -DBACK_END_IS_C_GEN_BE=FALSE \
  -DBACK_END_IS_CP_GEN_BE=TRUE

# DQ (7/11/2013): Removed these from cp_gen_be_CFXXLAGS
#  -DBACK_END_SHOULD_BE_CALLED=TRUE 
#  -DBACK_END_IS_C_GEN_BE=FALSE 
#  -DBACK_END_IS_CP_GEN_BE=TRUE 

# DQ (7/3/2014): Added these flags back to edgcpfe_CXXFLAGS.
# DQ (7/11/2013): Removed these from edgcpfe_CXXFLAGS
#  -DBACK_END_SHOULD_BE_CALLED=FALSE 
#  -DBACK_END_IS_C_GEN_BE=FALSE 
#  -DBACK_END_IS_CP_GEN_BE=TRUE 

# LDFLAGS = -s # Strip the programs to remove internal EDG symbols

# DQ (9/26/2015): Commented out to avoid warning in automake.
# DQ (7/3/2014): Added these to generate usable eccp script in source dir bin directory.
# edgcpfe_SOURCES = $(EDG_ALL_SOURCES) cfe.cc
cp_gen_be_SOURCES = $(EDG_ALL_SOURCES) cp_gen_be.cc cfe.cc

libedg_la_SOURCES = $(EDG_ALL_SOURCES)

# DQ (2/2/2010): Need to force this to be built first else in parallel 
# the compiler compiles might require it before it is finished being built.
$(EDG_ALL_SOURCES): defines.h


# DQ (12/24/2008): Added separate file to hold ROSE specific settings (read at top of host_envir.h).
ROSE_SPECIFIC_SOURCES = rose_host_envir.h rose_lang_feat.h rose_targ_def.h

EDG_COMMON_SOURCES = $(ROSE_SPECIFIC_SOURCES) \
	interpret.cc interpret.h \
	const_ints.cc debug.cc error.cc fe_init.cc float_pt.cc fixed_pt.cc \
	host_envir.cc il.cc il_read.cc il_to_str.cc il_walk.cc mem_manage.cc \
	src_seq.cc target.cc types.cc \
	attribute.h basic_hdrs.h basics.h c_gen_be.h class_decl.h \
	cmd_line.h const_ints.h cp_gen_be.h \
	debug.h decl_hdrs.h decl_inits.h decl_spec.h \
	declarator.h decls.h def_arg.h \
	disambig.h error.h \
	expr.h expr_hdrs.h exprutil.h extasm.h \
	fe_common.h fe_init.h \
	fe_wrapup.h float_pt.h fixed_pt.h folding.h \
	func_def.h host_envir.h host_util.h il.h il_alloc.h \
	il_def.h il_display.h il_file.h \
	il_read.h il_to_str.h il_walk.h \
	il_write.h inline.h lang_feat.h layout.h \
	lexical.h lint.h literals.h lookup.h lower_c99.h \
	lower_eh.h lower_hdrs.h \
	lower_il.h lower_il.cc \
	lower_init.h lower_name.h lower_name.cc \
	macro.h mem_manage.h mem_tables.h ms_attrib.h \
	overload.h pch.h pragma.h preproc.h scope_stk.h \
	src_seq.h statements.h symbol_ref.h symbol_tbl.h \
	sys_predef.h targ_def.h target.h \
	templates.h trans_copy.h trans_corresp.h trans_unit.h \
	types.h version.h walk_entry.h \
	err_codes.h err_data.h cp_gen_be.cc \
	ifc_modules.cc ifc_modules_templ.cc \
	ifc_map_functions.cc ifc_map_functions_acc.cc ifc_map_functions_val.cc ifc_map_functions_dbg.cc

# This causes a circular dependency in make
# defines.h 

EDG_ALL_SOURCES = $(EDG_COMMON_SOURCES) attribute.cc class_decl.cc \
                  cmd_line.cc decl_inits.cc decl_spec.cc \
		  declarator.cc decls.cc def_arg.cc disambig.cc \
		  expr.cc exprutil.cc extasm.cc fe_wrapup.cc \
		  folding.cc func_def.cc il_alloc.cc modules.cc \
		  il_display.cc il_write.cc inline.cc layout.cc lexical.cc \
		  literals.cc lookup.cc macro.cc ms_attrib.cc overload.cc \
		  pch.cc pragma.cc preproc.cc scope_stk.cc statements.cc \
		  symbol_ref.cc symbol_tbl.cc sys_predef.cc templates.cc \
		  trans_copy.cc trans_corresp.cc trans_unit.cc

BUILT_SOURCES = err_codes.h err_data.h

# err_codes.h: ../misc/mk_errinfo error_msg.txt error_tag.txt
err_codes.h: defines.h ../misc/mk_errinfo error_msg.txt error_tag.txt
	../misc/mk_errinfo $(srcdir)/error_msg.txt $(srcdir)/error_tag.txt err_codes.h err_data.h

# err_data.h: err_codes.h
err_data.h: defines.h err_codes.h

defines.h:
	@echo "Building the defines.h header file."
	cd ../misc && $(MAKE) config
	@echo "DONE: Building the defines.h header file."

EXTRA_DIST = CMakeLists.txt error_msg.txt error_tag.txt defines.h.proto

clean-local:
	rm -f defines.h

DISTCLEANFILES = err_codes.h err_data.h defines.h
