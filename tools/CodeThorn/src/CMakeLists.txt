#
# Codethorn has never been tested on Windows, RWSetGeneration has linking errors
# on Windows Codethorn/src/ has some .h and .C files that require ifdef macros
# If we want it to work on Windows in the future
#
option(ENABLE_CODETHORN "Enable CodeThorn program analysis and verification" TRUE)
if(NOT WIN32 AND ENABLE_CODETHORN AND NOT ENABLE_CLANG_FRONTEND AND ENABLE_C)
  # Codethorn tools require OpenMP
  set(_CODETHORN_HAS_OPENMP FALSE)
  if(ENABLE_OPENMP)
    set(_CODETHORN_HAS_OPENMP TRUE)
  else()
    # Try to find OpenMP even if not globally enabled
    find_package(OpenMP QUIET)
    if(OpenMP_FOUND)
      set(_CODETHORN_HAS_OPENMP TRUE)
    else()
      message(WARNING "CodeThorn tools (codethorn, thorn2, thorn4) require OpenMP which was not found. Only building libraries.")
    endif()
  endif()

  # all targets include these directories
  include_directories(. ${ROSE_INCLUDES} ${CMAKE_PREFIX_INSTALL}/include/rose
    ${CMAKE_BINARY_DIR})

  # all targets apply these flags TODO: Make this target-only flags
  if(_CODETHORN_HAS_OPENMP)
    add_compile_options(-fopenmp $<$<BOOL:${ENABLE_ALL_WARNINGS}>:-Wall>
      $<$<NOT:$<BOOL:${ENABLE_ALL_WARNINGS}>>:-Wno-misleading-indentation> -O3)
  else()
    add_compile_options($<$<BOOL:${ENABLE_ALL_WARNINGS}>:-Wall>
      $<$<NOT:$<BOOL:${ENABLE_ALL_WARNINGS}>>:-Wno-misleading-indentation> -O3)
  endif()

  # ##########################################################################
  # LIBRARY: CODETHORN (CORE)
  # ##########################################################################
  include(LibCodeThornSources.cmake) # source/header variables defined here

  # Create the target codethorn (libcodethorn.so) 
  add_library(codethorn SHARED ${CODETHORN_SOURCE_FILES})

  # Link libcodethorn.so to dependencies
  target_link_libraries(codethorn ROSE_DLL)

  # Install libcodethorn.so into lib
  install(TARGETS codethorn LIBRARY DESTINATION lib)

  # Header files for libcodethorn.so to be installed
  list(APPEND CODETHORN_ALL_HEADER_FILES ${CODETHORN_SOLO_HEADER_FILES}
    ${CODETHORN_HEADER_FILES})

  install(FILES ${CODETHORN_ALL_HEADER_FILES}
    DESTINATION include/rose/codethorn)

  # ##########################################################################
  # LIBRARY: LTLTHORN/ (for linear temporal logic verification)
  # ##########################################################################
  include(LibLTLThornSources.cmake) # source/header variables defined here

  # Create the target ltlthorn (libltlthorn.so)
  add_library(ltlthorn SHARED ${LTLTHORN_SOURCE_FILES})

  # Link libltlthorn.so to dependencies
  target_link_libraries(ltlthorn ROSE_DLL)

  # Install libltlthorn.so, don't install headers for this target
  install(TARGETS ltlthorn LIBRARY DESTINATION lib)

  # ##########################################################################
  # TOOLS: Build only if OpenMP is available
  # ##########################################################################
  if(_CODETHORN_HAS_OPENMP)
    # ########################################################################
    # TOOL: CODETHORN
    # ########################################################################
    add_executable(codethorn1 CodeThornCommandLineOptions.C codethorn.C)
    set_target_properties(codethorn1 PROPERTIES OUTPUT_NAME "codethorn")
    target_link_libraries(
      codethorn1 PRIVATE codethorn ltlthorn ROSE_DLL OpenMP::OpenMP_C)

    install(TARGETS codethorn1 RUNTIME DESTINATION bin)

    # ########################################################################
    # TOOL: THORN2
    # ########################################################################

    add_executable(thorn2 CodeThornCommandLineOptions.C thorn2.C)
    target_link_libraries(thorn2 PRIVATE codethorn OpenMP::OpenMP_C)
    install(TARGETS thorn2 RUNTIME DESTINATION bin)

    # ########################################################################
    # TOOL: THORN4
    # ########################################################################

    add_executable(thorn4 Thorn4CommandLineParser.C thorn4.C)
    target_link_libraries(thorn4 PRIVATE codethorn OpenMP::OpenMP_C)
    install(TARGETS thorn4 RUNTIME DESTINATION bin)

    # ########################################################################
    # TOOL: LTLTHORN
    # ########################################################################
    # Now add these targets to the tools build target.
    add_dependencies(tools codethorn ltlthorn codethorn1)
  else()
    # Only add library targets to tools if OpenMP is not available
    add_dependencies(tools codethorn ltlthorn)
  endif()

endif()
