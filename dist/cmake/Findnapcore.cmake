if (WIN32)
    find_path(
        NAPCORE_LIBS_DIR
        NAMES Release/napcore.lib
        HINTS ${CMAKE_CURRENT_LIST_DIR}/../lib/
    )
    set(NAPCORE_LIBS_RELEASE_DLL ${NAPCORE_LIBS_DIR}/Release/napcore.dll)
    set(NAPCORE_LIBS_DEBUG_DLL ${NAPCORE_LIBS_DIR}/Debug/napcore.dll)
    set(NAPCORE_LIBS_IMPLIB_DEBUG ${NAPCORE_LIBS_DIR}/Debug/napcore.lib)
    set(NAPCORE_LIBS_IMPLIB_RELEASE ${NAPCORE_LIBS_DIR}/Release/napcore.lib)
elseif (APPLE)
    find_path(
        NAPCORE_LIBS_DIR
        NAMES Release/libnapcore.dylib
        HINTS ${CMAKE_CURRENT_LIST_DIR}/../lib/
    )
    set(NAPCORE_LIBS_RELEASE_DLL ${NAPCORE_LIBS_DIR}/Release/libnapcore.dylib)
    set(NAPCORE_LIBS_DEBUG_DLL ${NAPCORE_LIBS_DIR}/Debug/libnapcore.dylib)
elseif (UNIX)
    find_path(
        NAPCORE_LIBS_DIR
        NAMES Release/libnapcore.so
        HINTS ${CMAKE_CURRENT_LIST_DIR}/../lib/
    )
    set(NAPCORE_LIBS_RELEASE_DLL ${NAPCORE_LIBS_DIR}/Release/libnapcore.so)
    set(NAPCORE_LIBS_DEBUG_DLL ${NAPCORE_LIBS_DIR}/Debug/libnapcore.so)
endif()

add_library(napcore SHARED IMPORTED)
set_target_properties(napcore PROPERTIES
    IMPORTED_CONFIGURATIONS "Debug;Release;MinSizeRel;RelWithDebInfo"
    IMPORTED_LOCATION_RELEASE ${NAPCORE_LIBS_RELEASE_DLL}
    IMPORTED_LOCATION_DEBUG ${NAPCORE_LIBS_DEBUG_DLL}
    IMPORTED_LOCATION_MINSIZEREL ${NAPCORE_LIBS_RELEASE_DLL}
    IMPORTED_LOCATION_RELWITHDEBINFO ${NAPCORE_LIBS_RELEASE_DLL}
)

if (WIN32)
  set_target_properties(napcore PROPERTIES
    IMPORTED_IMPLIB_RELEASE ${NAPCORE_LIBS_IMPLIB_RELEASE}
    IMPORTED_IMPLIB_DEBUG ${NAPCORE_LIBS_IMPLIB_DEBUG}
    IMPORTED_IMPLIB_MINSIZEREL ${NAPCORE_LIBS_IMPLIB_RELEASE}
    IMPORTED_IMPLIB_RELWITHDEBINFO ${NAPCORE_LIBS_IMPLIB_RELEASE}
  )
endif()

# TODO later: Fix CMake approach and use config-style package files

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(napcore REQUIRED_VARS NAPCORE_LIBS_RELEASE_DLL NAPCORE_LIBS_DIR)

if (WIN32)
    # Find our naputility import lib
    find_package(naputility REQUIRED)
    target_link_libraries(${PROJECT_NAME} naputility)

    # Copy over DLL post-build
    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:napcore> $<TARGET_FILE_DIR:${PROJECT_NAME}>/
    )
endif()