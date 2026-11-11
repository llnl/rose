message(STATUS "Building libroseClangFrontend") 
message(STATUS "Boost_INCLUDE_DIR ${Boost_INCLUDE_DIR}") 
message(STATUS "Building with Clang frontend")
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
message(STATUS "LLVM -I ${LLVM_INCLUDE_DIRS}") # ${CLANG_INCLUDE_DIRS} is the same path as ${LLVM_INCLUDE_DIRS

# TODO use target_include_directories()
include_directories(.  ${LLVM_INCLUDE_DIRS} ${ROSE_INCLUDES}) 
add_definitions(${LLVM_DEFINITIONS})

# Extra libraries for the Clang frontend tools
set(CLANG_LIBRARIES
  clangFrontendTool clangStaticAnalyzerFrontend clangStaticAnalyzerCheckers
  clangStaticAnalyzerCore clangIndex clangFrontend clangCodeGen
  clangRewrite clangSerialization clangDriver
  clangParse clangSema clangAnalysis clangAST clangBasic clangEdit clangLex)

# (10/30/2026) lin32: Clang 21 removed ARC migration; older Clang builds can also disable CLANG_ENABLE_ARCMT.
# (10/30/2026) lin32: Link the library only when the installed pre-21 Clang exports its CMake target.
if(CLANG_VERSION_MAJOR LESS 21 AND TARGET clangARCMigrate)
  list(APPEND CLANG_LIBRARIES clangARCMigrate)
endif()

if(${CLANG_VERSION_MAJOR} GREATER_EQUAL 15)
  list(APPEND CLANG_LIBRARIES clangSupport)
endif()

if(${CLANG_VERSION_MAJOR} GREATER_EQUAL 16)
  list(APPEND CLANG_LIBRARIES clangCrossTU clangASTMatchers)
endif()

if(${CLANG_VERSION_MAJOR} GREATER_EQUAL 18)
  list(APPEND CLANG_LIBRARIES clangAPINotes clangTooling)
endif()

# Source files of roseClangFrontend 
set(roseClangFrontend_SOURCE_FILES clang-frontend.cpp 
                                   clang-frontend-decl.cpp 
                                   clang-frontend-stmt.cpp
                                   clang-frontend-type.cpp
                                   clang-to-dot.cpp 
                                   clang-to-dot-decl.cpp 
                                   clang-to-dot-stmt.cpp 
                                   clang-to-dot-type.cpp  
                                   clang-to-rose-support.cpp) 

# Flags to use when compiling  
add_compile_options(
    $<$<BOOL:${ENABLE_ALL_WARNINGS}>:-Wall>
    $<$<NOT:$<BOOL:${ENABLE_ALL_WARNINGS}>>:-Wno-misleading-indentation>
) 

add_library(roseClangFrontend OBJECT ${roseClangFrontend_SOURCE_FILES}) 

#target_include_directories(roseClangFrontend PUBLIC ${CMAKE_BINARY_DIR}/src/frontend/SageIII) 
add_dependencies(roseClangFrontend rosetta_generated)

target_link_libraries(roseClangFrontend PRIVATE ${CLANG_LIBRARIES})

install(FILES clang-builtin-c.h clang-builtin-cpp.hpp clang-builtin-cuda.hpp clang-builtin-opencl.h DESTINATION ${INCLUDE_INSTALL_DIR}/../clang/)
