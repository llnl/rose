include $(top_srcdir)/config/Makefile.for.ROSE.includes.and.libs

SUBDIRS = 
EXTRA_DIST = CMakeLists.txt Makefile.am.forBinaryEDG

if ROSE_USE_EDG_VERSION_6_5
  SUBDIRS += EDG_6.5
  EXTRA_DIST += EDG_4.12 EDG_5.0 EDG_6.0 EDG_6.3
else
if ROSE_USE_EDG_VERSION_6_3
  SUBDIRS += EDG_6.3
  EXTRA_DIST += EDG_4.12 EDG_5.0 EDG_6.0 EDG_6.5
else
if ROSE_USE_EDG_VERSION_6_0
  SUBDIRS += EDG_6.0
  EXTRA_DIST += EDG_4.12 EDG_5.0 EDG_6.3 EDG_6.5
else
if ROSE_USE_EDG_VERSION_5_0
  SUBDIRS += EDG_5.0
  EXTRA_DIST += EDG_4.12 EDG_6.0 EDG_6.3 EDG_6.5
endif
endif
endif
endif

SUBDIRS += edgRose # need to appear after EDG source (depends on BUID_DIR/EDG_X.Y/src/define.h)

EDG_5.0/src/libedg.la:
	cd EDG_5.0; $(MAKE)

EDG_6.0/src/libedg.la:
	cd EDG_6.0; $(MAKE)

EDG_6.3/src/libedg.la:
	cd EDG_6.3; $(MAKE)

EDG_6.5/src/libedg.la:
	cd EDG_6.5; $(MAKE)

edgRose/libnewsage.la:
	$(MAKE) -CedgRose libnewsage.la

libroseEDGlibraries = \
  EDG_@ROSE_EDG_MAJOR_VERSION_NUMBER@.@ROSE_EDG_MINOR_VERSION_NUMBER@/src/libedg.la \
  edgRose/libnewsage.la

noinst_LTLIBRARIES = libroseEDG.la

# We need a simple file to seed the generation of the library (mark the library
# as having C++ code)
libroseEDG_la_SOURCES = dummyCppFileForLibroseEDG.C
libroseEDG_la_LIBADD = $(libroseEDGlibraries) 
libroseEDG_la_LDFLAGS = -no-undefined
#libroseEDG_la_DEPENDENCIES = $(libroseEDGlibraries)

BINARY_EDG_DIRNAME=xxx_trash_xxx
testEDGBinaryMakefileLogic:
	if test "$(am__untar)" != "false"; then \
	  if test -f $(srcdir)/$(BINARY_EDG_DIRNAME).tar.gz; then \
	    echo "Found $(srcdir)/$(BINARY_EDG_DIRNAME).tar.gz: proceeding to untar file $(BINARY_EDG_DIRNAME).tar.gz" >&2 && \
	    GZIP=$(GZIP_ENV) gunzip -c $(srcdir)/$(BINARY_EDG_DIRNAME).tar.gz | $(am__untar) && \
	    mv $(BINARY_EDG_DIRNAME)/libroseEDG.* . && \
	    mv $(BINARY_EDG_DIRNAME)/.libs . && \
	    rmdir $(BINARY_EDG_DIRNAME)/; \
	  else \
	    echo "There is no binary version of EDG (which should be named $(BINARY_EDG_DIRNAME).tar.gz) in your copy of ROSE -- this should have been caught by configure" >&2 && \
	    exit 1; \
	  fi \
	else \
	  echo "ERROR: am__untar set to false! (am__untar == $(am__untar)) Can't untar $(BINARY_EDG_DIRNAME).tar.gz) -- this should have been caught by configure" >&2 && \
	  exit 1; \
	fi


clean-local:
	rm -rf Templates.DB ii_files ti_files cxx_templates
	rm -rf *.o
