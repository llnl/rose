remove_definitions(-DROSE_DLL_EXPORTS)

# EDG's "disc_union" macro selects whether the IL's discriminated unions are real unions or
# structs; structs are easier to inspect in a debugger.
if(USE_ROSE_EDG_DEBUGGING_SUPPORT)
  set(EDG_DISC_UNION "struct")
else()
  set(EDG_DISC_UNION "union")
endif()

# edg_build compiles the EDG front end from the edg_compiler submodule (object library
# edg_internal); edgRose, the EDG IL -> SageIII translator, depends on headers generated there.
add_subdirectory(edg_build)
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