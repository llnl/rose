# Migration notes: from the private EDG submodule to open-source EDG

Date: 2026-10-16. ROSE 2.19.3 (develop) was the last layout with the private submodule.

## What changed

| Before (private submodule `rose-compiler/edg` at this path) | After |
|---|---|
| `src/frontend/CxxFrontend/EDG` was a git submodule (LLNL GitLab, private) | ordinary directory of the ROSE repository |
| `EDG/EDG_5.0`, `EDG_6.0`, `EDG_6.3`, `EDG_6.5` — full EDG release trees, patched in place, `*.cc` | `EDG/edg_compiler` — git submodule, public fork `github.com/rose-compiler/edg`, branch `rose/6.5`; upstream layout, `src/*.c` compiled as C++ |
| `EDG/EDG_6.5/{Makefile.am,src/Makefile.am,misc/Makefile.am,lib/Makefile.am,src/disp/*}` and the `CMakeLists.txt` files next to them | `EDG/edg_build/{Makefile.am,CMakeLists.txt}` (one directory) |
| `EDG/EDG_6.5/src/defines.h.rose`, `defines.h.win32.rose` | `EDG/edg_build/defines.h.rose`, `defines.h.win32.rose` |
| `EDG/EDG_6.5/misc/{lldef.sh,dettarg.out.sh,concat.cmake,generate_dettarg.out.cmake}` | `EDG/edg_build/` (same names) |
| generated `EDG_6.5/src/defines.h` (= `defines.h.rose` + `dettarg.out`) | generated `edg_build/cmake_defines.h`, picked up by EDG's own `defines.h` through `-DUSE_CMAKE_DEFINES` |
| `EDG/edgRose/` | unchanged path, history kept |
| `EDG/edg-migration.sh` (copy a new EDG release tarball in, re-apply patches) | `scripts/edg-upgrade.md` (rebase the `rose/<ver>` branch of the fork onto the new upstream tag) |
| `--enable-edg-version=5.0|6.0|6.3|6.5` chooses a directory | version read from `edg_compiler/src/version.h`; the switch is only an assertion |
| `--enable-edg-compile=maybe|yes|no`, precompiled `roseBinaryEDG` tarballs (generation, upload, download) | removed; the submodule is required for C/C++ support without Clang |
| `EDG_6.5/lib/predefined_macros.txt`, `EDG_6.5/src/disp` (IL display program) | dropped: not used by ROSE |

## History

* `edgRose/`, the build glue and this directory's root files were imported with
  `git filter-repo` from the private repository (645 commits, 2010-02-12 .. 2026-09-02). Each
  imported commit carries an `Imported-From: rose-compiler/edg@<sha>` trailer; `git log --follow`
  and `git blame` work across the move.
* ROSE's changes to EDG itself (24 source files, about 900 lines) are commits on the `rose/6.5`
  branch of the fork, above upstream tag `6.5`; see `ROSE-PATCHES.md` there. They were replayed
  with `git am` from the private commits and also carry `Imported-From:` trailers.
* The private repository remains the record for EDG versions before 6.5 and for the proprietary
  material (manuals, drivers) that was never ROSE's to publish. It is archived read-only.

## Licensing notes for review

* EDG source: Apache-2.0 WITH LLVM-exception (github.com/edgcpp/compiler). The license file and
  SPDX headers were added upstream at tag 7.0; the `rose/6.5` branch is based on tag 6.5, whose
  files still carry the older EDG banner. ROSE proceeds on the assumption, to be confirmed in
  writing with Edison Design Group, that all tags published in that repository are covered.
* `edg_build/defines.h.rose` and `defines.h.win32.rose` are ROSE-edited copies of EDG's
  configuration templates (`defines_linux.h`, `defines_win32.h`). Their EDG banners were replaced
  by a provenance header with the upstream SPDX identifier.
* `edgRose/`, `edg_build/*.{am,txt,sh,cmake}` are ROSE code under ROSE's license.

## Building

* `git submodule update --init src/frontend/CxxFrontend/EDG/edg_compiler`, then configure as usual
  (autotools or CMake). EDG is compiled into `libroseEDG` / `RoseSourceCxxFrontendEdg` exactly as
  before.
* The precompiled EDG library (`roseBinaryEDG-*.tar.gz`: generation with `make binary_edg_tarball`,
  upload to S3, download by the autotools build when the source was absent, the
  `scripts/edg-generate-sig` compatibility signature, the `EDG_VERSION` file and the `EDG-Binary*`
  CI jobs) was removed together with this change. Configure and CMake now fail with the submodule
  instruction when C/C++ support is requested without the EDG source and without the Clang frontend.
