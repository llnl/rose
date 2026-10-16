# The C/C++ frontend: Clang, or EDG compiled from the edg_compiler submodule (see EDG/README).
# The top-level CMakeLists.txt stops with an error when C/C++ analysis is enabled without the Clang
# frontend and the EDG source is missing, so have_EDG_source is true whenever we get here with EDG.

#if ROSE_BUILD_CXX_LANGUAGE_SUPPORT
if(ENABLE_C)
  if(ROSE_USE_CLANG_FRONTEND)
    message(STATUS "Selecting Clang for C/C++ frontend")
    # Autoconf reference: libroseCxxlibraries = frontend/CxxFrontend/Clang/libroseClangFrontend.la
    add_subdirectory(Clang)
  else()
    message(STATUS "Selecting EDG for C/C++ frontend")
    if(have_EDG_source AND EDG_COMPILE)
      add_subdirectory(EDG)
    endif()
  endif()
endif() # ENABLE_C
