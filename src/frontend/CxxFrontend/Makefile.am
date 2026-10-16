include $(top_srcdir)/config/Makefile.for.ROSE.includes.and.libs

# The C/C++ frontend: Clang, or EDG compiled from the edg_compiler submodule (see EDG/README).
# configure requires the EDG source whenever C/C++ support is enabled and the Clang frontend is not,
# so there is no fallback here; precompiled EDG libraries no longer exist.

SUBDIRS = Clang
DIST_SUBDIRS = Clang
PHONIES =

#If we have Clang, then we don't need EDG at all.
if !ROSE_USE_CLANG_FRONTEND
if ROSE_HAS_EDG_SOURCE

SUBDIRS += EDG

# edgRose includes ROSETTA-generated headers, so those have to exist before the EDG directory is built.
PHONIES += edg-prerequisites
edg-prerequisites:
	$(MAKE) -C ../../util
	$(MAKE) -C ../../Sawyer
	$(MAKE) -C ../../ROSETTA/src

EDG/libroseEDG.la: edg-prerequisites
	$(MAKE) -C ./EDG libroseEDG.la

MOSTLYCLEANFILES = EDG/libroseEDG.la

endif
endif

clean-local:
	rm -rf EDG/.libs

.PHONY: $(PHONIES)
