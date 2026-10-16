include $(top_srcdir)/config/Makefile.for.ROSE.includes.and.libs

# copy the tools (scripts only) from the source tree to the source destination in the distribution
distributedScripts =				 \
	autoconf-filter.pl quiet-mode		 \
	Error.pm				 \
	Git.pm					 \
	buildExampleRoseWorkspaceScript.sh	 \
	check-dist-file dotgenLetter		 \
	configure_with_SQL			 \
	check_boost_linkage.sh			 \
	dotgen					 \
	dotgen2					 \
	dotgenLetterPDF				 \
	dotgenLetterPNG				 \
	generateDirectoryGraph			 \
	libtoolcp				 \
	lsdot					 \
	nmiBuildAndTestFarm			 \
	policies-checker.sh			 \
	policies/CaseInsensitiveFileNames.pl	 \
	policies/EightColumnTabstops.pl.disabled \
	policies/FileLister.pm			 \
	policies/NoExceedinglyLongLines.pl	 \
	policies/NoRoseIncludes.pl		 \
	policies/NoTabCharacters.pl		 \
	policies/NoUsingInHeaders.pl		 \
	policies/Policies.pm			 \
	policies/UniqueFortranModules.pl	 \
	policies/UniqueHeaderNames.pl		 \
	policies/UnixLineTermination.pl		 \
	policies/UnusedHeaders.pl		 \
	policies/UnusedSources.pl		 \
	publicConfiguration.pl			 \
	rth_run.pl				 \
	rth_stats.pl				 \
	runscript				 \
	shtool

# scripts put into the distribution
EXTRA_DIST = CMakeLists.txt README $(distributedScripts) buildExampleRoseWorkspaceDirectory hudson \
	test_exit_status test_with_answer

# install the tools in 'bin' (a subsect of EXTRA_DIST)
#bin_SCRIPTS =  $(distributedScripts)


# DQ (12/7/2009): Test for the ROSE example workspace generator.
# Build the directory in the compile tree.
ExampleRoseWorkspaceTestDirectory = tmpRoseWorkspace
if ROSE_USE_INTERNAL_JAVA_SUPPORT
        ExampleRoseWorkspaceConfigure=./configure --with-boost=$(ac_boost_path) --prefix=@prefix@
else
        ExampleRoseWorkspaceConfigure=./configure --with-boost=$(ac_boost_path) --prefix=@prefix@ --without-java
endif
# This tests the new ROSE workspace
all:


testExampleRoseWorkspace:
	echo "absolute_path_srcdir = $(absolute_path_srcdir)"
	echo "ac_boost_path = $(ac_boost_path)"
	echo "srcdir = $(srcdir)"
	echo "PWD = $(PDW)"
	rm -rf $(ExampleRoseWorkspaceTestDirectory)
	$(srcdir)/buildExampleRoseWorkspaceScript.sh $(absolute_path_srcdir) $(ExampleRoseWorkspaceTestDirectory) || exit 1
	echo "Exiting after calling script: $(srcdir)/buildExampleRoseWorkspaceScript.sh"
	cd $(ExampleRoseWorkspaceTestDirectory); \
	./build --srcpath $(absolute_path_srcdir) --verbose; \
	echo "Building the compile tree in the source tree for the example ROSE workspace."; \
	$(ExampleRoseWorkspaceConfigure); \
	make;

# This may be the cause of recursion in the make "check rule"
#	make check


# DQ (12/7/2009): Run a test of the ROSE example workspace generator.
# This requires an installed version of ROSE so it should be an 
# installcheck rule.
# check-local:
installcheck:
	@echo "Running test of new ROSE workspace support."
	$(MAKE) testExampleRoseWorkspace

clean-local:
	rm -rf *.a *.so *.o $(ExampleRoseWorkspaceTestDirectory)
