# ROSE + EDG 7.0 on macOS / Apple Silicon — status

Branch `edg-oss-70-Mac` (ROSE) with submodule `edg_compiler` at fork branch `rose/7.0-Mac`
(= `rose/7.0` + two commits, see `edg_compiler/ROSE-PATCHES.md`). Written 2026-10-22.
The Linux-verified state stays on `edg-oss-70` / `rose/7.0` (tag `rose/7.0-20261016`); the Mac
branches have **not** been through the Linux regression run yet.

## Environment

| | |
|---|---|
| Machine | macOS 26.7.1, arm64 (Apple M-series) |
| Backend compiler | Apple clang 21 (predefines `__GNUC__` 4.2.1), libc++ 22, SDK headers under `<SDK>/usr/include` |
| Build | CMake 4.4.3, Release, `-DENABLE-C=ON`, Fortran/Java/OpenCL/PHP/binary analysis OFF, `-DDWARF_ROOT=no`, `-DBOOST_ROOT=<boost 1.90>` |
| Running | `DYLD_LIBRARY_PATH=<build>/lib:<boost>/lib <build>/bin/rose-compiler ...` (no `make install` yet, see below) |

## What works

- `rose-compiler` round-trips hello-world C and the libc++ hello-world C++
  (`<iostream>`, `<vector>`, `<string>`, range-for) and the backend produces the object file.
- The rose-tests compile suites, run with `rose-compiler` and the suites' own flags on the exact
  test lists that passed on Linux (round 8, 2026-10-21):

| Suite | Tests | Pass | Fail |
|---|---|---|---|
| `CompileTests/C_tests` | 612 | 589 | 23 |
| `CompileTests/Cxx_tests` | 1982 | 1959 | 23 |

The runner, lists and failure lists are kept outside the repo
(`Claude_test/handoff/mac-run-suite.sh`, `mac-*_tests.list`, `mac-*_tests-failed.txt`).

### Failure classes

1. **Backend / platform rejections of valid unparsed code** (C 19, C++ 2): x86 inline asm and
   `=a`/`=d` constraints, `regparm`, Mach-O `section` format, ELF `alias` / `weakref`, glibc
   `__assert_fail`. Not fixable on macOS; the frontend handled the files.
2. **Headers that do not exist on macOS** (C++ 9): libstdc++ `bits/c++config.h`,
   `bits/stl_algobase.h`, `tr1/memory`, `fpos::_M_off`; glibc `/usr/include/math.h`; x86-only
   `mmintrin.h`.
3. **Fixable in ROSE** (C++ 7): the unparser writes `restrict` in C++ (`test2004_30`,
   `test2006_38`, `test2012_262/263/264`, `test2014_122`, `test2016_59`); GCC accepts it, Apple clang
   does not. Emit `__restrict__` for a clang backend.
4. **To investigate** (C 3, C++ 2): `test2013_304.C` (EDG in clang mode finds no matching
   function template for `test<y>()`), `test2025_01.C` (explicit specialization of a static data
   member unparsed after its instantiation), `test2015_104.c` / `test2015_105.c` ("expected ';'
   after top level declarator"), `test2012_145.c` (`STRING_MACRO` from a Linux header).

## What was changed for macOS (all guarded; Linux code paths untouched)

- CMake (`CMakeLists.txt`, `cmake/*.cmake`, `src/CMakeLists.txt`): Boost 1.90 without a
  `boost_system` library, AppleClang compiler id, portable merge of the generated builtin
  prototypes, stale `FindFlex` module removed, `-undefined dynamic_lookup` for the EDG frontend dylib.
- EDG command line (`src/frontend/CxxFrontend/EDG/edgRose/cmdline.C`): a clang backend gets
  `--clang --clang_version N` only; the GNU version macros come from the backend's real
  `__GNUC__` (4.2.1 for Apple clang).
- System headers (`config/create_system_headers`, `config/get_compiler_header_dirs`): the SDK
  `usr/include` is kept on the include path and neither it nor clang's resource directory is
  mirrored into the staged header copies (the copies broke `#include_next` chains).
- `config/rose_edg_required_macros_and_functions.h.in` (clang backend only): `__attribute__` is
  no longer erased in strict modes (libc++ needs `enable_if`); `_Float128` typedef; C11 atomics
  (`__c11_atomic_*`), `__builtin_operator_new/delete`, `__builtin_assume_aligned`,
  `__builtin_clzg/ctzg` are defined as macros / templates because ROSE builds EDG with
  `BUILTIN_FUNCTIONS_ENABLED 0`. The arm64 `__int128_t` typedefs were removed.
- `edg_build/defines.h.rose`: `__int128` is EDG's host integer type for every clang backend
  (arm64 included); `HOST_ALIGNMENT_REQUIRED 16` on Apple arm64 (matching `dettarg`).
- Fork (`rose/7.0-Mac`): keyword-aware `__has_builtin` under `ROSE_MISC`; `dettarg` alignment.
- edgRose: C++17 init-statements (`if (init; cond)`, `if constexpr`, `switch`) are converted as
  `{ init; <statement> }` from EDG 7.0's `a_condition_supplement::initialization`
  (`parse_condition_init_statement.C`); clang's type-transform typerefs
  (`__remove_reference_t(T)` and friends) map to a modifier type reference (lossy);
  builtin-operation names fall back to EDG's table (`__is_reference`); `#pragma` inside an enum
  body is skipped; condition-scope declarations may live in the generated block.
- SageIII: `SgDeleteExp` mangling (libc++ uses `decltype(delete p)`); the constant-folding
  cleanup no longer deletes subtrees that are still owned by another part of the AST.

## Pending issues

1. **Dangling AST nodes with `std::cout`**: `std::cout << "a" << std::endl;` alone makes
   `fixupSelfReferentialMacrosInAST` (AstPostProcessing) report ~774 "called tree traversal
   mechanism for illegal object: SgNode" messages: nodes deleted earlier are still referenced.
   Output is still correct. Not caused by the constant-folding cleanup (count unchanged with the
   ownership check). Memory-pool addresses differ per run, so address-conditioned `operator delete`
   breakpoints do not work; next steps: `-rose:detect_dangling_pointers 2`, or bisect the
   post-processing passes that run before it (`fixupTemplateInstantiations`,
   `markTemplateSpecializationsForOutput`, `markTemplateInstantiationsForOutput`,
   `fixupTemplateArguments`, `resetConstantFoldedValues`).
2. clang type-transform typerefs are mapped lossily (no SageIII representation yet).
3. C11 atomics and `__builtin_operator_new/delete` are macro-erased for the frontend: fine for
   compiling, wrong for analyses that care about them.
4. No rose-tests harness run yet (`testTranslator` adds AST consistency checks and the `.passed`
   bookkeeping). It needs `make install`, which needs the tool builds that fail on this Mac to be
   disabled or fixed: PortabilityTesting (Boost.Process v1 API gone in Boost 1.90), CodeThorn
   (`hash_combine` with `uint64_t`, no OpenMP).
5. Autotools build untested on macOS (autotools is being deprecated in ROSE).
6. Linux regression on these branches not yet run.

## Suggested next tasks (in order)

1. Linux regression round on `edg-oss-70-Mac` + `rose/7.0-Mac` (must stay at C 618 / C++ 1986);
   then fast-forward `edg-oss-70` / `rose/7.0` to the Mac branches.
2. Unparse `__restrict__` instead of `restrict` for a clang backend (7 C++ tests).
3. Diagnose and fix the dangling nodes behind `std::cout` (pending issue 1).
4. `test2013_304.C`, `test2025_01.C`, `test2015_104/105.c`.
5. `make install` on the Mac (tool builds disabled or fixed) and the rose-tests harness:
   `check-smoke`, then the two suites with `make -k`, to confirm the counts above with
   `testTranslator`.
6. Upstream pull request for the keyword-aware `__has_builtin` (edgcpp/compiler).
