if (WIN32)
    find_path(
        NAPRTTI_LIBS_DIR
        NAMES Release/libnaprtti.dll
        HINTS
            ${CMAKE_CURRENT_LIST_DIR}/../lib/nap
    )
    set(NAPRTTI_LIBS_RELEASE_DLL ${NAPRTTI_LIBS_DIR}/Release/libnaprtti.dll)
    set(NAPRTTI_LIBS_DEBUG_DLL ${NAPRTTI_LIBS_DIR}/Debug/libnaprtti.dll)
elseif (APPLE)
    find_path(
        NAPRTTI_LIBS_DIR
        NAMES Release/libnaprtti.dylib
        HINTS
            ${CMAKE_CURRENT_LIST_DIR}/../lib/nap
    )
    set(NAPRTTI_LIBS_RELEASE_DLL ${NAPRTTI_LIBS_DIR}/Release/libnaprtti.dylib)
    set(NAPRTTI_LIBS_DEBUG_DLL ${NAPRTTI_LIBS_DIR}/Debug/libnaprtti.dylib)
elseif (UNIX)
    find_path(
        NAPRTTI_LIBS_DIR
        NAMES Release/libnaprtti.so
        HINTS
            ${CMAKE_CURRENT_LIST_DIR}/../lib/nap
    )
    set(NAPRTTI_LIBS_RELEASE_DLL ${NAPRTTI_LIBS_DIR}/Release/libnaprtti.so)
    set(NAPRTTI_LIBS_DEBUG_DLL ${NAPRTTI_LIBS_DIR}/Debug/libnaprtti.so)
endif()

add_library(naprtti SHARED IMPORTED)
set_target_properties(naprtti PROPERTIES
  IMPORTED_CONFIGURATIONS "Debug;Release;MinSizeRel;RelWithDebInfo"
  IMPORTED_LOCATION_RELEASE ${NAPRTTI_LIBS_RELEASE_DLL}
  IMPORTED_LOCATION_DEBUG ${NAPRTTI_LIBS_DEBUG_DLL}
  IMPORTED_LOCATION_MINSIZEREL ${NAPRTTI_LIBS_RELEASE_DLL}
  IMPORTED_LOCATION_RELWITHDEBINFO ${NAPRTTI_LIBS_RELEASE_DLL}
)

# TODO later: Fix CMake approach and use config-style package files

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(naprtti REQUIRED_VARS NAPRTTI_LIBS_RELEASE_DLL NAPRTTI_LIBS_DIR)
