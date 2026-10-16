# Upgrading ROSE to a new EDG release

ROSE's EDG lives in two places:

* `src/frontend/CxxFrontend/EDG/edg_compiler` — git submodule pointing at the public fork
  https://github.com/rose-compiler/edg. Branch `rose/<version>` = upstream tag `<version>` of
  https://github.com/edgcpp/compiler + ROSE's patches as individual commits (`ROSE-PATCHES.md` there).
  ROSE pins a tag `rose/<version>-YYYYMMDD`.
* `src/frontend/CxxFrontend/EDG/{edg_build,edgRose}` — ROSE's build of EDG and the EDG-to-SageIII
  translator, ordinary files in this repository.

An upgrade therefore has two halves: rebase the patch series in the fork, then move the pointer and
adapt ROSE.

## 1. In the fork (`rose-compiler/edg`)

```bash
git clone https://github.com/rose-compiler/edg.git && cd edg
git remote add upstream https://github.com/edgcpp/compiler.git
git fetch upstream --tags

# keep main a pure mirror of upstream
git push origin upstream/main:main

# new branch = old series rebased onto the new upstream tag
git checkout -b rose/NEW rose/OLD
git rebase --onto NEW OLD
```

While resolving:

* Drop a patch when upstream now contains the change (e.g. the `USE_CMAKE_DEFINES` hook in
  `defines.h` exists upstream from 7.0 on).
* Upstream adds files between releases (7.0 added `cfe_daemon.c`, `ifc_modules_read.c`, ...). Decide
  whether ROSE's library needs them and update **both** source lists,
  `edg_build/Makefile.am` and `edg_build/CMakeLists.txt`, in ROSE.
* Update `ROSE-PATCHES.md` (origin, files, flags) and commit it.
* Check the series still builds with upstream's own build (`cmake --preset linux-gcc-debug` from 7.0 on).

Then tag and push (the user with publish rights does this):

```bash
git tag -a rose/NEW-$(date +%Y%m%d) -m "ROSE patch series on EDG NEW"
git push origin rose/NEW rose/NEW-$(date +%Y%m%d)
```

## 2. In ROSE

```bash
cd src/frontend/CxxFrontend/EDG/edg_compiler
git fetch origin && git checkout rose/NEW-YYYYMMDD
cd - && git add src/frontend/CxxFrontend/EDG/edg_compiler
```

* configure / CMake pick the version up from `edg_compiler/src/version.h`; nothing to edit for the
  version itself.
* Reconcile `edg_build/defines.h.rose` with upstream's new `defines.h` / `defines_linux.h`
  (`git diff OLD NEW -- src/defines_linux.h` in the fork shows new or renamed configuration macros).
* Adapt `edgRose/` to IL changes. Historically this is small (6.5 -> 6.7: two lines in
  `edgRose/utilities.C`); compile errors in `edgRose` point at the changed EDG enumerators/fields.
* If the new version should have a `ROSE_USE_EDG_VERSION_<M>_<m>` macro, add it to the `AS_CASE` in
  `config/support-edg.m4` (CMake derives it from `EDG_VERSION`).
* Build and run rose-tests (`check-smoke`, `CompileTests/C_tests`, `CompileTests/Cxx_tests`) with both
  CMake and autotools; `make binary_edg_tarball` must still produce
  `roseBinaryEDG-<M>-<m>-<triplet>-<compiler>-<signature>.tar.gz`.
* The submodule pointer change needs EDG-maintainer approval in the MR (`testing-scripts/util/policy_check.sh`).

## 3. Upstreaming

Patches that are not ROSE-specific (see the "Upstream candidate" notes in `ROSE-PATCHES.md`) should be
submitted to https://github.com/edgcpp/compiler: open a GitHub Discussion first, tag commits
`[GH #nnn]`, add a `src/Changes` entry (see their `CONTRIBUTING.md`). Every accepted patch shrinks the
series for the next rebase.
