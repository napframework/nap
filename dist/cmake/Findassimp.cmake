if(WIN32)
    find_path(
        ASSIMP_DIR
        NAMES bin/assimp-vc140-mt.dll
        HINTS
        ${THIRDPARTY_DIR}/assimp
    )
    set(ASSIMP_LIBS_DIR ${ASSIMP_DIR}/bin)
    set(ASSIMP_LIBS ${ASSIMP_LIBS_DIR}/assimp-vc140-mt.dll)
elseif(UNIX)
    find_path(
        ASSIMP_DIR
        NAMES lib/libassimp${CMAKE_SHARED_LIBRARY_SUFFIX}
        HINTS
        ${THIRDPARTY_DIR}/assimp
    )
    set(ASSIMP_LIBS_DIR ${ASSIMP_DIR}/lib)
    set(ASSIMP_LIBS ${ASSIMP_LIBS_DIR}/libassimp${CMAKE_SHARED_LIBRARY_SUFFIX})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(assimp REQUIRED_VARS ASSIMP_LIBS_DIR ASSIMP_DIR)

add_library(assimp SHARED IMPORTED)
set_target_properties(assimp PROPERTIES
    IMPORTED_CONFIGURATIONS "Debug;Release"
    IMPORTED_LOCATION_RELEASE ${ASSIMP_LIBS}
    IMPORTED_LOCATION_DEBUG ${ASSIMP_LIBS}
)

if(WIN32)
    set_target_properties(assimp PROPERTIES
                          IMPORTED_IMPLIB ${ASSIMP_DIR}/lib/assimp-vc140-mt.lib
                          )

    # Copy over DLL post-build
    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:assimp> $<TARGET_FILE_DIR:${PROJECT_NAME}>/
    )
endif()
