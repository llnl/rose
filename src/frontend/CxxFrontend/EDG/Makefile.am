include $(top_srcdir)/config/Makefile.for.ROSE.includes.and.libs

# edg_build compiles the EDG front end from the edg_compiler submodule into libedg.la;
# edgRose (the EDG IL -> SageIII translator) must come after it because it includes the
# headers generated there (cmake_defines.h, err_codes.h).
SUBDIRS = edg_build edgRose

EXTRA_DIST = CMakeLists.txt README MIGRATION.md

edg_build/libedg.la:
	$(MAKE) -C edg_build

edgRose/libnewsage.la:
	$(MAKE) -C edgRose libnewsage.la

libroseEDGlibraries = \
  edg_build/libedg.la \
  edgRose/libnewsage.la

noinst_LTLIBRARIES = libroseEDG.la

# We need a simple file to seed the generation of the library (mark the library
# as having C++ code)
libroseEDG_la_SOURCES = dummyCppFileForLibroseEDG.C
libroseEDG_la_LIBADD = $(libroseEDGlibraries)
libroseEDG_la_LDFLAGS = -no-undefined

clean-local:
	rm -rf Templates.DB ii_files ti_files cxx_templates
	rm -rf *.o
