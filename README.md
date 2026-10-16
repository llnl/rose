# ROSE Compiler
	
ROSE is an open source compiler infrastructure to build source-to-source program transformation and analysis tools for large-scale Fortran (77/95/2003), C (up to C23), C++ (up to C++17), OpenMP, Cuda, and UPC applications. ROSE also has support for Java (7), Ada (95), and the analysis of binary files which use the ARM, AMD64 (x86_64), Intel x86, MIPS, Motorola 68k, PowerPC (32- and 64-bit), Java byte code, or CIL instruction sets. The intended users of ROSE could be either experienced compiler researchers or library and tool developers who may have minimal compiler experience. ROSE is particularly well suited for building custom tools for static analysis, program optimization, arbitrary program transformation, domain-specific optimizations, complex loop optimizations, performance analysis, and cyber-security.

[ROSE Doxygen API](http://rose-compiler.github.io/)

# Why Use ROSE

ROSE is not grep, sed, LLVM, or a Perl script. A ROSE Tool uses the ROSE compiler-based infrastructure to parse code into a complete Abstract Syntax Tree (AST). The AST contains all the syntax and semantic information in the original code, and has a rich API supporting sophisticated analysis and transformations. The ROSE Tool queries the AST and reports on and/or changes the AST, then may emit new code from the AST. All ROSE Tools can thus precisely replicate the parsing and semantic analysis behaviour of multiple compiler languages, vendors, and versions. New ROSE Tools can quickly be created by customers or by the ROSE Team. ROSE is open-source, and is portable across a large and expanding range of platforms. ROSE Tools can process large code bases, and the ROSE infrastructure and ROSE Tool collection are continuously upgraded and extended by the LLNL ROSE Team and outside contributors.

# Installation Instructions
ROSE requires CMake version >= 3.15

Rose supports multiple languages, for this example we will only enable C support.

Assuming an out-of-source build in a directory called "rosebuild"

```
mkdir rosebuild
cd rosebuild
cmake .. -DCMAKE_INSTALL_PREFIX=/path/for/ROSE/install \
         -DENABLE_C=ON
make -j${NUM_PROCESSORS} install
```


# Supported Compilers
When ROSE is configured with C/C++ support it needs the EDG front end. EDG is open source (Apache-2.0 WITH LLVM-exception); ROSE's patched copy is the git submodule `src/frontend/CxxFrontend/EDG/edg_compiler` (https://github.com/rose-compiler/edg). Either build it from source

```
git submodule update --init src/frontend/CxxFrontend/EDG/edg_compiler
```

or, with the autotools build and no submodule, let the build download a precompiled EDG library. Precompiled libraries are currently generated for GCC 7 through 12. The support for 11 and 12 is in progress, so for fully supported versions use GCC 7 through 10. See `src/frontend/CxxFrontend/EDG/README`.

When not using c/c++ support GCC 7 through 13 are supported.

# ROSE Directories 
* `src`: all source code for ROSE
* `tools`: usable feature complete tools
* `projects`: in development and incomplete tools
* `tutorial`: exmples of ROSE features
* `docs`: files for bulding the documentation

# Documentation
For more information about ROSE and how to use it visit the github wiki at https://github.com/rose-compiler/rose/wiki

The ROSE API can be found at http://rose-compiler.github.io/. The API can also be made locally by going to `cd $ROSE_BUILD/docs/Rose` and running `make doxygen_docs`. The html pages can then be found in `${ROSE_BUILD}/docs/Rose/ROSE_WebPages` and can be easily viewed by pointing your browser at `${ROSE_BUILD}/docs/Rose/ROSE_WebPages`.


# License
This project is licensed under the BSD License - see the LICENSE.md file for details
SPDX-License-Identifier: BSD-3-Clause

LLNL-CODE-2018981
