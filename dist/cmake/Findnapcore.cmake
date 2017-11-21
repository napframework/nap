
if (WIN32)
    find_path(
        NAPCORE_LIBS_DIR
        NAMES Release/libnapcore.dll
        HINTS ${CMAKE_CURRENT_LIST_DIR}/../lib/nap
    )
    set(NAPCORE_LIBS_RELEASE_DLL ${NAPCORE_LIBS_DIR}/Release/libnapcore.dll)
    set(NAPCORE_LIBS_DEBUG_DLL ${NAPCORE_LIBS_DIR}/Debug/libnapcore.dll)
elseif (APPLE)
    find_path(
        NAPCORE_LIBS_DIR
        NAMES Release/libnapcore.dylib
        HINTS ${CMAKE_CURRENT_LIST_DIR}/../lib/nap
    )
    set(NAPCORE_LIBS_RELEASE_DLL ${NAPCORE_LIBS_DIR}/Release/libnapcore.dylib)
    set(NAPCORE_LIBS_DEBUG_DLL ${NAPCORE_LIBS_DIR}/Debug/libnapcore.dylib)
elseif (UNIX)
    find_path(
        NAPCORE_LIBS_DIR
        NAMES Release/libnapcore.so
        HINTS ${CMAKE_CURRENT_LIST_DIR}/../lib/nap
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

# TODO later: Fix CMake approach and use config-style package files

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(napcore REQUIRED_VARS NAPCORE_LIBS_RELEASE_DLL NAPCORE_LIBS_DIR)
