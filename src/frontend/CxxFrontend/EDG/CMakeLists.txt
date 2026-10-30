remove_definitions(-DROSE_DLL_EXPORTS)

if(USE_ROSE_EDG_DEBUGGING_SUPPORT)
  set(EDG__C_FLAGS "${CMAKE_C_FLAGS} -Ddisc_union=struct" CACHE STRING "Flags to use in directory ${CMAKE_CURRENT_BINARY_DIR}")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EDG__C_FLAGS}")
endif()

add_subdirectory(EDG_${EDG_VERSION})
add_subdirectory(edgRose)

add_library(RoseSourceCxxFrontendEdg SHARED 
    dummyCppFileForLibroseEDG.C
    $<TARGET_OBJECTS:edg_internal>
    $<TARGET_OBJECTS:edg_rose>)

set_target_properties(RoseSourceCxxFrontendEdg PROPERTIES
    INSTALL_RPATH "$ORIGIN;$ORIGIN/../lib"
    BUILD_WITH_INSTALL_RPATH FALSE
    INSTALL_RPATH_USE_LINK_PATH TRUE
    OUTPUT_NAME "RoseSourceCxxFrontendEdg")

install(TARGETS RoseSourceCxxFrontendEdg
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin)

## install(EXPORT RoseSourceCxxFrontendEdgTargets
##     FILE RoseSourceCxxFrontendEdgTargets.cmake
##     NAMESPACE ROSE::
##     DESTINATION lib/cmake/RoseSourceCxxFrontendEdg)