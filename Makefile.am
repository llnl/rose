include $(top_srcdir)/config/Makefile.for.ROSE.includes.and.libs
EXTRA_DIST =

# DQ "config" and "src" must preceed any other directories, and
# "tutorial" and "exampleTranslators" must preceed "docs" since
# they generate data that is subsequently used in the generated
# ROSE documentation (examples, input codes, and generated output).

# DQ (2/6/2010): changed the order of tests, the projects directory now follows the tests directory.
# This was something that was discussed in email previously with the group.  Thei point is to have the
# first tests run by make check be the most relevant tests (e.g. can we compile C++ code).

# DQ (4/17/2010): Support for minimal configuration of ROSE (e.g. "--enable-only-fortran").

# We don't want "make [all]", "make check", or "make install", etc. to recurse into the tests/nonsmoke directory (unless
# ROSE was configured using "--with-ROSE_LONG_MAKE_CHECK_RULE"). If you want to build something in the non-smoke area
# you should "cd" do that directory first, as in "make -C tests/nonsmoke ...".
SUBDIRS = LicenseInformation libltdl config src python

if !ROSE_USE_CLANG_FRONTEND
if !ROSE_USE_INSURE
# DQ (4/23/2011): Ignoring all of these whole directories might be over kill.  But I have had problems with
# Insure++ compilation specific to Hudson and want to start small and work to grow the number of parts of ROSE
# used by Insure.  Except on Hudson (e.g. on my machine) Insure++ can be used to compiler nearly all of ROSE.
if ROSE_BUILD_PROJECTS_DIRECTORY_SUPPORT
   SUBDIRS += projects
endif

# The Portability Test Project is not compiled as part of ROSE but serves as a simple example of an external project.
# Portability testing will need to access this project and compile it after installing ROSE and deleting the ROSE source
# and build trees. Therefore, this project's source code should be installed as part of the ROSE installation. These
# rules are at this high level of the source code because otherwise they would interfere with the Makefile that's
# distributed as part of the test project, and because it needs to be part of the binary release even if the rest
# of the ROSE tools are excluded.
testprojdir = ${datarootdir}/test-project
testproj_DATA =								\
	tools/PortabilityTesting/lib/matrix/test-project/Makefile	\
	tools/PortabilityTesting/lib/matrix/test-project/README.md	\
	tools/PortabilityTesting/lib/matrix/test-project/test-rose.C	\
	tools/PortabilityTesting/lib/matrix/test-project/VERSION
EXTRA_DIST += $(testproj_DATA)

# DQ (2/6/2017): Added test to detect if the default mode of the C++ frontend compiler for rOSE is C++11 or not.
# Ultimately we want this test to only apply to the GNU 4.8 compiler in C++11 mode (which can have internal errors
# in compiling the C++ code represented by some more complex C++ test codes using ROSE).  Interestingly, the
# GNU 4.8 C++11 mode works fine for the C++ code that implements ROSE itself.
# if ROSE_USING_GCC_VERSION_4_8_CXX11
#check-local:
# all: remove_sage3basic_h_pch
# all:
#	@echo "**********************************************************************************************"
#	@echo "**********************************************************************************************"
#	@echo "Skipping tests in tutorial, exampleTranslators, docs, tools, scripts, and directories for"
#	@echo "GNU 4.8.x using C++11 mode (GNU 4.8 compiler has internal errors when using the C++11 mode)."
#	@echo "**********************************************************************************************"
#	@echo "**********************************************************************************************"
if !ROSE_USING_GCC_VERSION_4_8_CXX11

if ROSE_BUILD_TUTORIAL_DIRECTORY_SUPPORT
 # If we can't process the tutorial directory then we can't generate example translators or the documentation
#   SUBDIRS += tutorial exampleTranslators docs
endif
SUBDIRS += tools scripts

# endif for excluding tests using GNU 4.8 and C++11 mode together.
endif

# endif for controling Insure++ usage.
endif
endif

ACLOCAL_AMFLAGS = -I ./config -I ./acmacros -I ./libltdl

all: remove_sage3basic_h_pch

core:
	$(MAKE) -C $(top_builddir)/src

tools: core
	$(MAKE) -C $(top_builddir)/tools
	$(MAKE) -C $(top_builddir)/exampleTranslators
if ROSE_BUILD_PROJECTS_DIRECTORY_SUPPORT
	$(MAKE) -C $(top_builddir)/tutorial
endif

install-core: core rosePublicConfig.h
	$(MAKE) install-data-local
	$(MAKE) install -C $(top_builddir)/src
	cp rosePublicConfig.h $(DESTDIR)/$(pkgincludedir)
if ROSE_EXPERIMENTAL_ADA_ROSE_CONNECTION
# Currently libasis_adapter.so is built without libtool support.
#   Thus an rpath is used to link programs within the build environment
#   For installed programs, the rpath to the build environment may no
#   longer exist. Thus we remove it from the rose-config.cfg file.
#   TODO: seek better ways of integrating libasis_adapter.so
#         with libtool.
	sed -i 's|-L$(abs_builddir)/src/frontend/Experimental_Ada_ROSE_Connection/parser/asis_adapter/lib||' $(prefix)/lib/rose-config.cfg
	sed -i 's|-R $(abs_builddir)/src/frontend/Experimental_Ada_ROSE_Connection/parser/asis_adapter/lib||' $(prefix)/lib/rose-config.cfg
	sed -i 's|-Wl,-rpath $(abs_builddir)/src/frontend/Experimental_Ada_ROSE_Connection/parser/asis_adapter/lib||' $(prefix)/lib/rose-config.cfg
	sed -i 's|$(abs_builddir)/src/frontend/Experimental_Ada_ROSE_Connection/parser/asis_adapter/lib||' $(prefix)/lib/rose-config.cfg
endif

install-tools: install-core tools
	$(MAKE) install -C $(top_builddir)/tools
	$(MAKE) install -C $(top_builddir)/exampleTranslators
if ROSE_BUILD_PROJECTS_DIRECTORY_SUPPORT
	$(MAKE) install -C $(top_builddir)/tutorial
endif

check-recursive:
	@true

check-core:
	$(MAKE) check -C $(top_builddir)/src

.PHONY: tools
check-tools: tools
	$(MAKE) check -C $(top_builddir)/tools
	$(MAKE) check -C $(top_builddir)/exampleTranslators

# Install ONLY the library components (no examples, no tests, no projects). This is even more "core" than "core" but I'm
# not changing "install-core" because users use that and expect certain behavior. The purpose of this target is mainly for
# ROSE project developers that have a need to install the library and other specific tools (via other install targets) but
# don't want to install anything else (perhaps because it doesn't compile). [Robb Matzke, 2016-03-07]
install-rose-library: install-data-local install-testprojDATA
	$(MAKE) -C $(top_builddir)/src install
	$(MAKE) -C $(top_builddir)/LicenseInformation install
	cp rosePublicConfig.h $(DESTDIR)/$(pkgincludedir)
	$(MAKE) -C $(top_builddir)/python install

# Build a binary release of the ROSE library.  This assumes that you're running in the RMC-2 environment because it uses
# some "make" variables that are initialized from shell exported variables. Other build environments will need other targets.
#
# It installs whatever is in the ROSE installation prefix, but at least the ROSE library and supporting header files.
#
# We cannot distribute Oracle Java because it requires the user to agree to a license. The user must download it themselves.
#
# We cannot distribute CXX_ROOT or C_ROOT because it might be pointing to an Intel compiler, which needs to be purchased
# by the user.
#
# We cannot distribute Yices because it requires the user to agree to a license. The user must download it themselves.
#
# We should not distribute /usr or /usr/local as these are general locations with lots of other software.
rose-installer-rmc2: install-rose-library
	rm -rf $@.tmp
	mkdir  $@.tmp
	$(top_srcdir)/scripts/mkinstaller --version
	TMPDIR=$@.tmp $(top_srcdir)/scripts/mkinstaller									\
		--create="$@"												\
		--CC="$(CC)"												\
		--CXX="$(CXX)"												\
		--contact="http://rosecompiler.org"									\
		--prjnam=ROSE												\
		--prjver=$(PACKAGE_VERSION)										\
		--postinstall-bin=$(top_srcdir)/tools/PortabilityTesting/lib/matrix/container-scripts/post-install-script	\
		$(prefix)/*												\
		$(addsuffix /*, $(filter-out / /usr /usr/local,								\
		    $(BOOST_ROOT)											\
		    $(CMAKE_ROOT)											\
		    $(DLIB_ROOT)											\
		    $(GNU_COMPILERS_ROOT)										\
		    $(LIBBZ2_ROOT)											\
		    $(LIBGCRYPT_ROOT)											\
		    $(LIBGPG_ERROR_ROOT)										\
		    $(LIBGRAPHICSMAGICK_ROOT)										\
		    $(LIBJPEG_ROOT)											\
		    $(LIBLCMS_ROOT)											\
		    $(LIBLZMA_ROOT)											\
		    $(LIBPNG_ROOT)											\
		    $(LIBTIFF_ROOT)											\
		    $(PYTHON_ROOT)											\
		    $(SPOT_ROOT)											\
		    $(WT_ROOT)												\
		    $(YAMLCPP_ROOT)											\
		    $(ZLIB_ROOT)											\
		))
	rm -rf $@.tmp

check-rose-installer-rmc2: rose-installer-rmc2
	rm -rf $@.tmp installed installed-binary
	mkdir  $@.tmp
	TMPDIR=$@.tmp ./rose-installer-rmc2 --prefix=installed-binary
	LD_LIBRARY_PATH=installed-binary/lib ./installed-binary/bin/rose-config --version
	rm -rf $@.tmp installed-binary

# DQ (10/2/10): Of course this will over-rise the "make install" rule, so we can't
# allow exactly this (what was I thinking?). Commented out until I get a better fix.
# DQ (9/29/2010): Next three lines are a modification suggested by Scott Warren to
# support Eclipse.  Supports Eclipse feature called "makefile project with existing code"
# which wants to have a single target to force the build.
#.PHONY: all-install

#all-install: all install

#install: all

remove_sage3basic_h_pch:
	touch $(top_builddir)/src/frontend/SageIII/sage3basic.h.pch
	rm    $(top_builddir)/src/frontend/SageIII/sage3basic.h.pch

# These are policies that we'd like to enforce for ROSE developers.  Things like:
#    1. source files should not use CR+LF line termination common on Windows platforms
#    2. header files should not include certain ROSE headers like "rose.h"
#    3. file names must be unique on a case-insensitive file system
#    4. header files must not have names that conflict with headers in other directories
#    5. etc.
# DO NOT DISABLE without first checking with a ROSE core developer
export prefix	# for use by policy scripts
enforce_policies:
	(cd $(top_srcdir) && ./scripts/policies-checker.sh)

# Most checks in ROSE are named "check-whatever", so do the same for policy checking
.PHONY: check-policies
check-policies: enforce_policies


# DQ (6/29/2004): I don't think we need this!
# LIBS = @LIBS@ ${SAGE_LIBS}

# It seems that it is better to handle the config directory with its own Makefile.am
# This allows us to hide autoconf accessory files (config.guess, etc.)
# Copy the config directory to the distribution: why?
# (because it has the file: config/Makefile.for.ROSE.includes.and.libs)
# EXTRA_DIST = stamp-h.in stamp-h1.in GNU_HEADERS config
# EXTRA_DIST = stamp-h.in stamp-h1.in GNU_HEADERS COPYWRITE ROSE_ResearchPapers
# EXTRA_DIST = cmake CMakeLists.txt rose_config.h.in.cmake stamp-h.in stamp-h1.in COPYRIGHT \
#              LicenseInformation ROSE_ResearchPapers README.OSX README.Cygwin build ChangeLog2 bincompat-sig
EXTRA_DIST += cmake CMakeLists.txt rose_config.h.in.cmake stamp-h.in stamp-h1.in COPYRIGHT \
             LicenseInformation build

# At some point we want to put tals into the distribution (likely just PDF files, but not yet).
# ROSE_Talks

# clean-local explanation:
# Remove template repositories.  No source code is compiled, but configuration
#   tests may have created template repositories here.
clean-local:
	rm -rf Templates.DB ii_files ti_files *.csv

# Not sure if this should be part of the clean-local rule.
# chmod -R +rwx rose-0.9.5a; rm -rf rose-0.9.5a

# DQ (9/8/2006): Modified to remove backend specific header files
uninstall-local:
#	rm -rf $(DESTDIR)$(includedir)/*_HEADERS
	rm -rf `find $(DESTDIR)$(includedir)/ -iname "*_HEADERS"`

distclean-local:
	-rm -rf $(top_builddir)/include-staging/

# DQ (10/14/2010): We want to make sure that this does not go out to the install tree
# it also should not be in the distribution (since we want rose_config.h.in to be put
# into the distribution instead; from which then rose_config.h is generated).
# However, the public version should be installed and distributed.
noinst_HEADERS = rose_config.h
pkginclude_HEADERS = rosePublicConfig.h

# DQ (4/5/2009): This rule is not called when compiling this directory, it is only
# called when compiling the /src/util directory (perhaps it should be located in
# that directory's Makefile.am.
# DQ (12/3/2007): Added rose_paths.h so that it would be copied to the install tree.
# JJW (8/25/2008): Changed to a .c file
include $(top_srcdir)/config/build_rose_paths.Makefile

install-exec-local:
	@echo '*****************************************************************'
	@echo '***** make install-exec rule complete (terminated normally) *****'
	@echo '*****************************************************************'

install-data-local: uninstall-local
	-mkdir -p $(DESTDIR)$(includedir)
if ROSE_USE_CLANG_FRONTEND
	mkdir -p "$(DESTDIR)$(includedir)"
	cp -R $(top_builddir)/include-staging/* "$(DESTDIR)$(includedir)"
else
#	DQ (11/1/2011): The EDG 4.x support does not require a separate include-staging directory.
	mkdir -p "$(DESTDIR)$(includedir)/edg"
	cp -R $(top_builddir)/include-staging/* "$(DESTDIR)$(includedir)/edg"
endif
#	endif
	@echo '*****************************************************************'
	@echo '***** make install-data rule complete (terminated normally) *****'
	@echo '*****************************************************************'

# DQ (4/22/2005): Set this so that make distcheck will use the same
# --with-edg_source_code=true/false option as were used at configure.
# JJW (5/14/2008): Add the Boost flag the same way, using an internal
# variable from AX_BOOST_* to find the argument to --with-boost when
# this copy of ROSE was originally configured.
DISTCHECK_CONFIGURE_FLAGS = --with-boost=$(ac_boost_path)

# DQ (7/25/2008): If ROSE was originallly configured with Fortran (by tuning
# on the java support) then make sure it is tested as part of the distcheck rule.
# if ROSE_USE_OPEN_FORTRAN_PARSER
if ROSE_USE_INTERNAL_JAVA_SUPPORT
# DISTCHECK_CONFIGURE_FLAGS += --with-java
  DISTCHECK_CONFIGURE_FLAGS += --with-java=$(JAVA_PATH)
else
# DQ (10/22/2010): Added specification of --without-java to distcheck rule.
# If we have first built without java then we have had to build some of the
# Fortran language supporting files as empty files and thus we don't want to
# run distcheck in a way that would use those files. However the problem
# when this is not used has more to do with the Java langauge support.
# A better reason why this is required might be so that we can run the NMI tests.
  DISTCHECK_CONFIGURE_FLAGS += --without-java
# TOO (3/24/2011): TODO: the new language-support configuration that I've
# implemented does not automatically disable fortran, whereas the previous
# implementation did. Since we now have Java-language support, the
# --without-java flag is confusing. I thought it meant "without java-language
# support". Speaking with Dan, we concluded that we need to look at renaming
# --without-java to something else like --without-jvm or --without-java-components
# This flag is suppose to turn off all parts of ROSE that use Java:
# projects/javaports, java- and fortran-language support, etc.
  DISTCHECK_CONFIGURE_FLAGS += --disable-fortran
endif

# DQ (7/25/2008): If ROSE was originallly configured to use the HPC Toolkit
# then make sure it is tested as part of the distcheck rule.
if ROSE_BUILD_ROSEHPCT
  DISTCHECK_CONFIGURE_FLAGS += --enable-rosehpct
endif

# DQ (7/25/2008): If ROSE was originallly configured to use the javaport
# work (generation of Java interface functions for ROSE using SWIG) then
# make sure it is tested as part of the distcheck rule.
if ENABLE_JAVAPORT
  DISTCHECK_CONFIGURE_FLAGS += --with-javaport
endif

# DQ (4/5/2010): Use the same version of OFP as specified on the original configure command line.
# CER (10/10/2011): No longer needed. A specific OFP version is configured by default and distributed with ROSE.
#DISTCHECK_CONFIGURE_FLAGS += --enable-ofp-version=@ROSE_OFP_MAJOR_VERSION_NUMBER@.@ROSE_OFP_MINOR_VERSION_NUMBER@.@ROSE_OFP_PATCH_VERSION_NUMBER@

# SRIRAM (8/2/2010): changing distcheck configuration for LLVM translator
if ROSE_USE_LLVM
   DISTCHECK_CONFIGURE_FLAGS += --with-llvm=$(llvm_path)
endif

# DQ (4/22/2005): To make clear when binaries are in the distribution as
# opposed to EDG source code we rename the final generated distribution.
# JJW (8/3/2008): This is only used for source distributions now --
# source_with_binary_edg_dist does its own renaming
rename_distribution:
	new_name="$(distdir)-source-with-EDG-source.tar.gz"; \
	cp $(distdir).tar.gz $$new_name

# We reset distdir as shown on
# http://jcalcote.wordpress.com/2008/02/23/autoconf-macros-exposed-at3/ to add the SVN
# revision number
# The final distribution package should be
# rose-0.9.5a-8286.tar.gz
#
# Liao 12/14/2009
# We don't use subversion anymore
# We convert the number of git commits to a pseudo revision number instead
distdir = $(PACKAGE)-$(VERSION)$(DOT_SVNREV)

dist-hook:
	builddir="$$(pwd)" &&                                                      \
  (                                                                          \
  [ -e "$(srcdir)"/.git ] &&                                                 \
  cd $(srcdir) &&                                                            \
      git_head="$$(git rev-parse HEAD)" &&                                   \
      git_head_date="$$(git log -1 --format=%at $$(git rev-parse HEAD))" &&  \
          echo "$$git_head $$git_head_date" > $${builddir}/VERSION &&        \
          cp $${builddir}/VERSION $${builddir}/$(distdir)/VERSION            \
  ) ||                                                                       \
  (                                                                          \
  VERSION_FILE="$(srcdir)/VERSION" &&                                        \
  [ -e "$$VERSION_FILE" ] &&                                                 \
      cp "$$VERSION_FILE" $${builddir}/$(distdir)/VERSION                    \
  ) ||                                                                       \
  (                                                                          \
      echo "Error: could not determine SCM version information." &&          \
      exit 1                                                                 \
  ) ||                                                                       \
  exit 1

# Automake hook-ish rule to be run after dist rule (but hook runs before gzip so we can't use hook)
# We could build our own ROSE specific wrapper for dist and distcheck.
dist-after:
	@echo "Running dist hook rule ..."
	@$(MAKE) rename_distribution

# hook rule to be run after distcheck rule
distcheck-after:
	@echo "Running distcheck hook rule ..."
	@$(MAKE) rename_distribution

# CLEANFILES = rose_binary_compatibility_signature rose_binary_compatibility_signature_src
CLEANFILES = rose_binary_compatibility_signature

find_rose_lock_files:
#	du -a | grep rose_performance_report_lockfile
	find . -name rose_performance_report_lockfile -print

# New rule to simplify generation of documentation (we want to avoid using the
# automake generated "docs" rule because the generation of documentation is
# dependent upon separate tools which the user might not have available
# (true for bothe the LaTeX and html documentation).
# DQ (7/23/2004): Since there is a "docs" subdirectory we need to force the build!
FORCE_DOCS_TO_BE_MADE:

# DQ (7/25/2008): Running "make docs" introduces dependences that are inappropriate
# for a distribution.
docs: FORCE_DOCS_TO_BE_MADE
	@if [ -f $(top_srcdir)/src/midend/midend.docs ]; then \
	   echo "    Generate all possible documentation cd docs; make documentation;"; \
	   $(MAKE) -Ctutorial docs && \
	   $(MAKE) -Cdocs docs; \
	else \
	   echo "Please look at the ROSE web site for documentation (www.roseCompiler.org)"; \
	fi


# Makefile rule to test the Hudson runTest script (used for all Hudson tests).
# Note that empty ROSE_TEST_BUILD_STYLE maps to a minimal build.
testHudsonRunScript:
	export ROSE_TEST_BUILD_SKIP_BUILD_SCRIPT=yes; \
   export ROSE_TEST_GCC_VERSION=4.2.4; \
   export ROSE_TEST_BUILD_STYLE=; \
   export ROSE_TEST_BOOST_PATH=${MY_BOOST_ROOT}; \
   export ROSE_TEST_JAVA_PATH=${JAVA_PATH}; \
   cd $(top_srcdir); \
   $(top_srcdir)/scripts/hudson/runTest

# This will cause the Hudson environment to be dumped to a file so that
# it can be sourced to reproduce the environment required to run tests
# in the hudson tmp directories as part of debugging.
dumpHudsonEnv:
	export ROSE_DUMP_HUDSON_ENV=yes; $(MAKE) testHudsonRunScript


# $(top_srcdir)/scripts/hudson/testHudsonRunScript.sh;


# DQ (12/16/2009): Added interesting "git" feature by Liao.
# It is unclear if it is very meaningful since it represents
# individual work style more than anything else (but it's fun).
CheckinCounts:
	@echo "*****************************************************"
	@echo "Generated list of checkins to ROSE sumarized by user."
	@echo "*****************************************************"
	cd $(top_srcdir); git shortlog -s -n
	@echo "*****************************************************"


# This rule is useful for identifing source files where "rose_config.h" should be included.
FindFileRequiringRoseConfigHeaderShort:
#	grep "#define" rose_config.h | cut --delimiter=" " -f 2
#	@list=`grep \"\#define\" rose_config.h | cut --delimiter=" " -f 2`; for f in $$list; do
#	list=`ls -l | cut --delimiter=" " -f 2`; for f in $$list; do
	list=`egrep "#define|\/* #undef" rose_config.h | sed 's/\/\* //g' | egrep -v "const|error_t|inline|off_t|size_t" | cut --delimiter=" " -f 2`; \
   for f in $$list; do \
        echo "Searching for -->$$f<-- macro"; \
        grep -r $$f $(top_srcdir)/src/* | grep "\.h:" | cut --delimiter=":" -f 1; \
   done

# This rule is useful for identifing header files where "rose_config.h" is required and
# thus should be fixed to not require this file. In the case of #undef it is represented
# as "/* #undef" so we have to detect and remove the "/* " using sed.
# And ignore the macro definitions for "const|error_t|inline|off_t|size_t".
FindFileRequiringRoseConfigHeaderLong:
	list=`egrep "#define|\/* #undef" rose_config.h | sed 's/\/\* //g' | egrep -v "const|error_t|inline|off_t|size_t" | cut --delimiter=" " -f 2`; \
   for f in $$list; do \
        echo "Searching for -->$$f<-- macro"; \
        grep -r $$f $(top_srcdir)/* | grep "\.h:"; \
   done

FindFileRequiringRoseConfigSourceLong:
	list=`egrep "#define|\/* #undef" rose_config.h | sed 's/\/\* //g' | egrep -v "const|error_t|inline|off_t|size_t" | cut --delimiter=" " -f 2`; \
   for f in $$list; do \
        echo "Searching for -->$$f<-- macro"; \
        grep -r $$f $(top_srcdir)/* | grep "\.C:"; \
   done

FindFileRequiringRoseConfigSourceAndHeadersLong:
	list=`egrep "#define|\/* #undef" rose_config.h | sed 's/\/\* //g' | egrep -v "const|error_t|inline|off_t|size_t" | cut --delimiter=" " -f 2`; \
   for f in $$list; do \
        echo "Searching for -->$$f<-- macro"; \
        grep -r $$f $(top_srcdir)/src/* | egrep "\.C:|\.h:"; \
   done

FILTER_MAKEFILES = grep "Makefile\.am" | grep -v "Makefile\.am~"
NumberOfMakefiles:
	@echo "Number of Makefile.am files in root directory: "
	@du -a $(srcdir) | $(FILTER_MAKEFILES) | wc -l
	@echo "Number of Makefile.am files in src directory: "
	@du -a $(srcdir)/src | $(FILTER_MAKEFILES) | wc -l
	@echo "Number of Makefile.am files in projects directory: "
	@du -a $(srcdir)/projects | $(FILTER_MAKEFILES) | wc -l
	@echo "Number of Makefile.am files in exampleTranslators directory: "
	@du -a $(srcdir)/exampleTranslators | $(FILTER_MAKEFILES) | wc -l
	@echo "Number of Makefile.am files in tutorial directory: "
	@du -a $(srcdir)/tutorial | $(FILTER_MAKEFILES)| wc -l
	@echo "Number of Makefile.am files in docs directory: "
	@du -a $(srcdir)/docs | $(FILTER_MAKEFILES) | wc -l
	@echo "Number of Makefile.am files in tools directory: "
	@du -a $(srcdir)/tools | $(FILTER_MAKEFILES) | wc -l
	@echo "Number of Makefile.am files in config directory: "
	@du -a $(srcdir)/config | $(FILTER_MAKEFILES) | wc -l

# ***********************************************************
#      Display Which Automake Conditionals Are Defined
# ***********************************************************
# DQ (10/18/2010): Added test for names used in automake conditional macros
# (eventually we want tests for consistancy).  This rule generates a makefile
# containing a makefile rule. The makefile rule can be run to see what automake
# conditionals were defined in the last run of configure. It might be that
# this mechanism should be in a script generated by the build script, but we
# need an initial version of the generated script to be checked into git so that
# the build script can find it.  Note that we grep the Makefile.in instead of
# the Makefile.am (which would be more difficult, I think). So we need the
# Makefile.in from runing the build script first.  Thus having the generated
# file checked into git avoids a dependence on a file that we would not have.
FILTER_AUTOMAKE_CONDITONALS = egrep -v "autom4te" | grep -v "~" | grep AM_CONDITIONAL
TEST_FILE = $(top_srcdir)/config/automake_conditional_display_makefile
NameOfAutomakeConditionals:
#	grep -r "^AM_CONDITIONAL" $(top_srcdir) | $(FILTER_AUTOMAKE_CONDITONALS)
#	grep "_TRUE@" $(top_srcdir)/Makefile.in | grep "@" | cut --delimiter="@" -f 2 | sed s/_TRUE//g | sort --unique | grep -v "top_srcdir"
	list=`grep "_TRUE@" $(top_srcdir)/Makefile.in | grep "@" | cut --delimiter="@" -f 2 | sed 's/_TRUE//g' | sort --unique | grep -v "top_srcdir"`; \
   rm -f $(TEST_FILE); \
   touch $(TEST_FILE); \
   echo "automake_conditional_display:" >> $(TEST_FILE); \
   for f in $$list; do \
        echo "Processing -->$$f<-- macro"; \
        echo "if $$f" >> $(TEST_FILE); \
        echo -e "\t@echo \"Automake conditional $$f: DEFINED\"" >> $(TEST_FILE); \
        echo "else" >> $(TEST_FILE); \
        echo -e "\t@echo \"Automake conditional $$f: NOT defined\"" >> $(TEST_FILE); \
        echo "endif" >> $(TEST_FILE); \
   done
#	This can't be run since it can't be processed through automake and autoconf.
#	$(MAKE) -f $(TEST_FILE) automake_conditional_display

# This is a test of the new make rule added in the file $(TEST_FILE).
testNameOfAutomakeConditionals:
	@echo "*************************************************************************"
	@echo "Testing makefile rule automake_conditional_display in file: $(TEST_FILE)."
	@echo "*************************************************************************"
	$(MAKE) automake_conditional_display

# Include this file (generated first by the rule down below and checked into our git repository).
# This could be generated by our build script.
include $(top_srcdir)/config/automake_conditional_display_makefile
# ***********************************************************



# DQ (10/18/2010): These are used all over within a line (so don't use start of line anchor).
# (eventually we want tests for consistancy)
FILTER_AUTOCONF_DEFINE = egrep -v "autom4te" | grep -v "~" | grep -v "configure:" | grep AC_DEFINE
NameOfAutoconfDefines:
	grep -r "AC_DEFINE" $(top_srcdir) | $(FILTER_AUTOCONF_DEFINE)




# DQ (2/12/2010): Report the areas of ROSE where we have skipped some tests of ROSE
# either doing analysis, code generation, or translation of the ROSE source code.
ProblemAreas:
	echo "Directories where ROSE does not analyze code for some ROSE source files."
	grep -r ROSE_USING_ROSE_ANALYSIS $(top_srcdir) | grep Makefile.am
	echo "Directories where ROSE does not generate code for some ROSE source files."
	grep -r ROSE_USING_ROSE_CODE_GENERATION $(top_srcdir) | grep Makefile.am
	echo "Directories where ROSE does not translate (compile) code for some ROSE source files."
	grep -r ROSE_USING_ROSE_TRANSLATE $(top_srcdir) | grep Makefile.am
	echo "Directories where ROSE does not properly handled AST File I/O for at least some ROSE source files."
	grep -r ROSE_USING_ROSE_AST_FILE_IO $(top_srcdir) | grep Makefile.am
	echo "Files where ROSE does not analyze some amount of code (test of USE_ROSE in each ROSE source file)."
	grep -rl USE_ROSE $(top_srcdir) | grep "\.C"
	echo "Files where ROSE does not analyze some amount of code (test of CXX_IS_ROSE_ANALYSIS in each ROSE source file)."
	grep -rl CXX_IS_ROSE_ANALYSIS $(top_srcdir) | grep "\.C"
	echo "Files where ROSE does not analyze some amount of code (test of CXX_IS_ROSE_CODE_GENERATION"
	echo "in each ROSE source file)."
	grep -rl CXX_IS_ROSE_CODE_GENERATION $(top_srcdir) | grep "\.C"
#	echo "Files where ROSE does not analyze some amount of code (test of CXX_IS_ROSE_TRANSLATOR in each ROSE source file)."
#	grep -rl CXX_IS_ROSE_TRANSLATOR $(top_srcdir) | grep "\.C"

# Generate public version of rose_config.h, which is triggered by the AC_CONFIG_COMMANDS macro near the end
# of config/support-rose.m4 whenever config.status runs.
rosePublicConfig.h: rose_config.h
	@echo "  GEN     $@"
	@$(top_srcdir)/scripts/publicConfiguration.pl <$^ >$@
DISTCLEANFILES = rosePublicConfig.h

# This file does not exist until we run the compiler to build it.
# include $(top_builddir)/src/frontend/SageIII/astPostProcessing/.libs/astPostProcessing.d


################################################################################
# Makefile specific to Intel codecov process generated *.dyn files
# generated by Intel's codecov option to the Intel icc and icpc compilers.
################################################################################

# For Intel's code coverage tool (codecov) we need to avoid the *.dyn files from being
# built across the whole directory structure. So use the -prof-dir option to force them
# into a single directory to simplify processing.  Then use the profmerge utility to
# merge them into a single pgopti.dpi file.  Then process the pgopti.dpi file with the
# codecov utility to generate the HTML file and directory of HTML files).

# configure using:
# --with-C_DEBUG="-g -prof-dir /home/dquinlan/ROSE/ROSE_CompileTree/git-LINUX-64bit-intel-v14--dq-development-rc-code_coverage -prof-gen=srcpos"
# --with-CXX_DEBUG="-g -prof-dir /home/dquinlan/ROSE/ROSE_CompileTree/git-LINUX-64bit-intel-v14--dq-development-rc-code_coverage -prof-gen=srcpos"
#
# Then run "make -ki -j24 check" to generate all of the *.dyn files.
# Note that "-ki" is required because currently a few test files will demonstrate Intel specific errors.
# Also, the number of *.dyn files will be large (many thousand files) and the diskspace requirements are also large (~200Gig?).
#
# The code coverage results can be see from the CODE_COVERAGE.HTML file in the top level of the build tree.
#

process_codecov_files_part_1:
	profmerge *.dyn

process_codecov_files_part_2:
	codecov -comp ${top_srcdir}/intel_code_coverage_list

process_codecov_files: process_codecov_files_part_1 process_codecov_files_part_2

# All *.dyn files should be in the top level directory.
remove_codecov_files:
	find . -name '*.dyn' -delete

################################################################################
# Makefile specific to lcov comment to process generated *.gcda and *.gcno files
# generated by GNU's gcov option to the GNU gcc adn g++ compilers.
################################################################################

# lcov command to generate the html files.
lcov:
	lcov --capture \
-d src/util/.libs \
-d src/util/stringSupport/.libs \
-d src/util/commandlineProcessing/.libs \
-d src/util/support/.libs \
-d src/util/graphs/.libs \
-d src/3rdPartyLibraries/MSTL/.libs \
-d src/ROSETTA/src \
-d src/roseExtensions/roseHPCToolkit/src/util/.libs \
-d src/roseExtensions/roseHPCToolkit/src/xml/.libs \
-d src/roseExtensions/roseHPCToolkit/src/profir/.libs \
-d src/roseExtensions/roseHPCToolkit/src/gprof/.libs \
-d src/roseExtensions/roseHPCToolkit/src/xml2profir/.libs \
-d src/roseExtensions/roseHPCToolkit/src/sage/.libs \
-d src/roseExtensions/roseHPCToolkit/src/profir2sage/.libs \
-d src/roseExtensions/roseHPCToolkit/src/.libs \
-d src/roseExtensions/failSafe/.libs \
-d src/frontend/SageIII/.libs \
-d src/frontend/SageIII/astFixup \
-d src/frontend/SageIII/astPostProcessing \
-d src/frontend/SageIII/astFileIO \
-d src/frontend/SageIII/astMerge \
-d src/frontend/SageIII/sageInterface \
-d src/frontend/SageIII/virtualCFG \
-d src/frontend/SageIII/astTokenStream \
-d src/frontend/SageIII/astHiddenTypeAndDeclarationLists \
-d src/frontend/SageIII/astVisualization \
-d src/frontend/SageIII/astFromString \
-d src/frontend/SageIII/includeDirectivesProcessing \
-d src/frontend/SageIII/astFixup/.libs \
-d src/frontend/SageIII/astPostProcessing/.libs \
-d src/frontend/SageIII/astMerge/.libs \
-d src/frontend/SageIII/sageInterface/.libs \
-d src/frontend/SageIII/virtualCFG/.libs \
-d src/frontend/SageIII/astTokenStream/.libs \
-d src/frontend/SageIII/astVisualization/.libs \
-d src/frontend/SageIII/astFromString/.libs \
-d src/frontend/SageIII/includeDirectivesProcessing/.libs \
-d src/frontend/SageIII/sage_support/.libs \
-d src/frontend/MatlabFrontend \
-d src/frontend/CxxFrontend/EDG/edg_build/.libs \
-d src/frontend/CxxFrontend/EDG/edgRose/.libs \
-d src/frontend/OpenFortranParser_SAGE_Connection/.libs \
-d src/frontend/ECJ_ROSE_Connection/.libs \
-d src/frontend/X10_ROSE_Connection/.libs \
-d src/frontend/PHPFrontend/.libs \
-d src/frontend/BinaryLoader/.libs \
-d src/frontend/BinaryFormats/.libs \
-d src/frontend/Disassemblers/.libs \
-d src/Rose/BinaryAnalysis/Partitioner2/.libs \
-d src/midend/BinaryAnalysis/.libs \
-d src/midend/programAnalysis/.libs \
-d src/midend/programAnalysis/staticSingleAssignment/.libs \
-d src/midend/programAnalysis/ssaUnfilteredCfg/.libs \
-d src/midend/programTransformation/extractFunctionArgumentsNormalization/.libs \
-d src/midend/programTransformation/singleStatementToBlockNormalization/.libs \
-d src/midend/programTransformation/loopProcessing/depInfo/.libs \
-d src/midend/programTransformation/loopProcessing/depGraph/.libs \
-d src/midend/programTransformation/loopProcessing/computation/.libs \
-d src/midend/programTransformation/loopProcessing/outsideInterface/.libs \
-d src/midend/programTransformation/loopProcessing/slicing/.libs \
-d src/midend/programTransformation/loopProcessing/driver/.libs \
-d src/midend/programTransformation/loopProcessing/prepostTransformation/.libs \
-d src/midend/programTransformation/ompLowering/.libs \
-d src/midend/programTransformation/partialRedundancyElimination/.libs \
-d src/midend/programTransformation/finiteDifferencing/.libs \
-d src/midend/programTransformation/functionCallNormalization/.libs \
-d src/midend/programTransformation/constantFolding/.libs \
-d src/midend/programTransformation/implicitCodeGeneration/.libs \
-d src/midend/programTransformation/astInlining/.libs \
-d src/midend/programTransformation/astOutlining/.libs \
-d src/midend/programTransformation/transformationTracking/.libs \
-d src/midend/abstractHandle/.libs \
-d src/midend/abstractLayer/.libs \
-d src/midend/astDiagnostics/.libs \
-d src/midend/abstractMemoryObject/.libs \
-d src/midend/astProcessing/.libs \
-d src/midend/astQuery/.libs \
-d src/midend/astSnippet/.libs \
-d src/midend/astRewriteMechanism/.libs \
-d src/midend/astUtil/annotation/.libs \
-d src/midend/astUtil/astInterface/.libs \
-d src/midend/astUtil/astSupport/.libs \
-d src/midend/astUtil/symbolicVal/.libs \
-d src/midend/astUtil/dependenceTracking/.libs \
-d src/backend/.libs \
--output-file coverage.info

# lcov command to generate the html files.
lcov_genhtml: coverage.info
	genhtml --title ROSE --legend --demangle-cpp --prefix $(top_srcdir) --prefix $(top_pwd) --output-directory lcov_output coverage.info

# DQ (2/22/2016): We can't include directories that ere not configured.
# -d src/frontend/CxxFrontend/EDG/EDG_4.9/misc
# -d src/frontend/CxxFrontend/EDG/EDG_4.9/src/.libs

# DQ (2/22/2016): The file genhtml has an error.
#   ERROR: cannot read /export/tmp.schordan1/development/rosework/mainbranch/rose/src/midend/astMatching/matcherlexer.ll
# So we can't process this directory.
# -d src/midend/astMatching/.libs

# DQ (2/22/2016): The file genhtml has an error.
# genhtml: ERROR: cannot read /home/dquinlan/ROSE/ROSE_CompileTree/git-LINUX-64bit-4.8.3--dq-development-rc-lcov/src/3rdPartyLibraries/POET/poet_yacc.y
# So we can't process this directory.
# -d src/3rdPartyLibraries/POET/.libs

# Initial result (2/22/2016):
# Overall coverage rate:
#   lines......: 42.7% (377904 of 885133 lines)
#   functions..: 53.0% (100721 of 189865 functions)

########################################################################################################################
# Robb's version of lcov commands, although they should work anywhere.
########################################################################################################################

# We want to snarf up all the *.gcno and *.gcda files in the build tree, but lcov will refuse to recurse into
# subdirectories when the parent directory has no such files, so list the directories explicitly. The sort is
# only so we have some idea of the progress of the long-running command to create lcov.info.
lcov_data_dirs = $(shell find $(top_builddir) -type d |sort)

# Capture *.gcda files from the build tree for whatever programs we've happened to run so far. This could be
# the ROSETTA-generator plus any tests, tutorials, and projects.  This can take a long time to run!
all-lcov.info:
	lcov --capture --no-recursion $(patsubst %,-d %,$(lcov_data_dirs)) -o $@

# Capture only coverage for the ROSE library source code (no tests, tutorials, projects, system headers, boost, etc).
# DQ (3/13/2017): Modified to remove specific directories of old code to be removed from ROSE and 3rd party code that we do not maintain.
# (04/26/2016) Pei-Hung lcov has issue to output extracted result to librose-lcov.info with -o option. Using piping instead.
librose-lcov.info: all-lcov.info
	lcov --extract $< "$(top_srcdir)/src/*" > $@
	lcov --remove librose-lcov.info \
        "$(top_srcdir)/src/frontend/CxxFrontend/EDG/edgRose/debugging.*" \
        "$(top_srcdir)/src/frontend/CxxFrontend/EDG/edgRose/edgGraph.*" \
        "$(top_srcdir)/src/backend/unparser/PythonCodeGeneration/*" \
        "$(top_srcdir)/src/backend/unparser/PHPCodeGeneration/*" \
        "$(top_srcdir)/src/midend/programAnalysis/OAWrap/*" \
        "$(top_srcdir)/src/midend/programAnalysis/OpenAnalysis/*" \
        "$(top_srcdir)/src/midend/programAnalysis/genericDataflow/*" \
        "$(top_srcdir)/src/frontend/CxxFrontend/EDG/edg_compiler/*" \
        "$(top_srcdir)/src/midend/astRewriteMechanism/*" \
        "$(top_srcdir)/src/midend/programAnalysis/dominanceAnalysis/*" \
        "$(top_srcdir)/src/midend/programAnalysis/pointerAnal/*" \
        "$(top_srcdir)/src/midend/programAnalysis/ssaUnfilteredCfg/*" \
        "$(top_srcdir)/src/src/midend/programAnalysis/staticInterproceduralSlicing/*" \
        "$(top_srcdir)/src/midend/programAnalysis/staticSingleAssignment/*" \
        "$(top_srcdir)/src/midend/programAnalysis/variableRenaming/*" \
        "$(top_srcdir)/src/midend/programTransformation/extractFunctionArgumentsNormalization/*" \
        "$(top_srcdir)/src/midend/programTransformation/finiteDifferencing/*" \
        "$(top_srcdir)/src/midend/programTransformation/functionCallNormalization/*" \
        "$(top_srcdir)/src/midend/programTransformation/implicitCodeGeneration/*" \
        "$(top_srcdir)/src/midend/programTransformation/singleStatementToBlockNormalization/*" \
        "$(top_srcdir)/src/roseSupport/stringify.C" \
        "$(top_srcdir)/src/roseExtensions/roseHPCToolkit/*" \
        "$(top_srcdir)/src/frontend/SageIII/nodeBuildFunctionsForAterms.C" \
        "$(top_srcdir)/src/frontend/SageIII/astVisualization/*" \
        "$(top_srcdir)/src/midend/astProcessing/AstProcessing.h" \
        "$(top_srcdir)/src/frontend/SageIII/sageInterface/sageGeneric.h" \
        "$(top_srcdir)/src/3rdPartyLibraries/json*" \
        "$(top_srcdir)/src/roseSupport/*" \
         -o librose-lcov.info

# Note: astVisualization is only approriate for small ASTs and for debugging so excluded from code coverage analysis.
#       sageGeneric.h causes 1274 template instantiations, and we only have a test for one of them (so this file is a problem due to how lcov handles template instantaitions.
#       DQ (3/30/2017): Removed src/midend/astProcessing/AstProcessing.h file because it's template use is not handled accurately by lcov (wildly incorrect).

# DQ (4/9/2017): After getting better coverage, the ultimate limitation proved to be the poor template handling within LCOV to support more significant coverage results.
# DQ (4/5/2017): This fails with GNU 6.1 and Boost 1.51 and Boost 1.52, but it is now called and should contribute to code coverage so let it back in as a test.
#       "$(top_srcdir)/src/frontend/SageIII/sageInterface/sageGeneric.h"


librose_lcov_web_pages: librose-lcov.info
	genhtml                                 \
                --title librose                 \
                --legend                        \
                --demangle-cpp                  \
                --rc genhtml_hi_limit=85        \
                --rc genhtml_med_limit=60       \
                --prefix $(top_srcdir)          \
                --ignore-errors=source          \
                --output-directory $@ $<

# Directories to remove from LCOV testing of ROSE:
# src/frontend/CxxFrontend/EDG/edgRose/debugging.*
# src/frontend/CxxFrontend/EDG/edgRose/edgGraph.*
# src/backend/unparser/PythonCodeGeneration/*
# src/backend/unparser/PHPCodeGeneration/*
# src/midend/programAnalysis/OAWrap/*
# src/midend/programAnalysis/OpenAnalysis/*
# src/midend/programAnalysis/genericDataflowflow/*
# src/frontend/CxxFrontend/EDG/EDG_4.9/*
# src/midend/astRewriteMechanism/*
# src/midend/programAnalysis/dominanceAnalysis/*
# src/midend/programAnalysis/pointerAnal/*
# src/midend/programAnalysis/ssaUnfilteredCfg/*
# src/src/midend/programAnalysis/staticInterproceduralSlicing/*
# src/midend/programAnalysis/staticSingleAssignment/*
# src/midend/programAnalysis/staticSingleAssignment/*
# src/midend/programAnalysis/variableRenaming/*
# src/midend/programTransformation/extractFunctionArgumentsNormalization/*
# src/midend/programTransformation/finiteDifferencing/*
# src/midend/programTransformation/functionCallNormalization/*
# src/midend/programTransformation/implicitCodeGeneration/*
# src/midend/programTransformation/singleStatementToBlockNormalization/*
# src/roseSupport/stringify.C
#
#
# Directories to remove from ROSE:
# src/midend/programAnalysis/dominanceAnalysis
# src/midend/programAnalysis/pointerAnal
# src/midend/programAnalysis/ssaUnfilteredCfg
# src/midend/programAnalysis/staticInterproceduralSlicing
# src/midend/programAnalysis/staticSingleAssignment
# src/midend/programAnalysis/variableRenaming
# src/midend/programTransformation/extractFunctionArgumentsNormalization
# src/midend/programTransformation/finiteDifferencing
# src/midend/programTransformation/functionCallNormalization
# src/midend/programTransformation/implicitCodeGeneration
# src/midend/programTransformation/singleStatementToBlockNormalization
#
# May be removed in the future (to be discussed) DQ (3/13/2017):
# src/midend/programTransformation/constantFolding
# src/midend/programTransformation/ompLowering
# src/midend/programTransformation/partialRedundancyElimination
#
# These should not be evaluated using lcov (to be discussed) DQ (3/13/2017):
# src/roseSupport/stringify.C
#      Generated file, placed in source tree because it took a long time to generate when it was put into the build tree).
#      A few functions from this file are tested (14 of 864), but not all.
#      This file could only be tested using an automatically generated test and that might no be productive.
#
#
# What is this:
# src/midend/abstractMemoryObject  (DTEC work by Liao)
#
#
# Why are these so poorly tested:
# src/midend/programTransformation/astInlining
# src/midend/programTransformation/astOutlining
#
#
#


# Generate a directory containing the HTML representation of the coverage information for the ROSE library proper.
# THIS INCLUDES EDG SOURCES; DO NOT DISTRIBUTE THIS DIRECTORY
librose_lcov_web_pages: librose-lcov.info
	genhtml                                 \
                --title librose                 \
                --legend                        \
                --demangle-cpp                  \
                --rc genhtml_hi_limit=85        \
                --rc genhtml_med_limit=60       \
                --prefix $(top_srcdir)          \
                --ignore-errors=source          \
                --output-directory $@ $<
