
if (WIN32)
    find_path(
        RTTR_LIBS_DIR
        NAMES Release/rttr_core.dll
        HINTS ${CMAKE_CURRENT_LIST_DIR}/../lib/thirdparty
    )
    set(RTTR_LIBS_RELEASE_DLL ${RTTR_LIBS_DIR}/Release/rttr_core.dll)
    set(RTTR_LIBS_DEBUG_DLL ${RTTR_LIBS_DIR}/Debug/rttr_core_d.dll)
elseif (APPLE)
    find_path(
        RTTR_LIBS_DIR
        NAMES Release/librttr_core.0.9.6.dylib
        HINTS ${CMAKE_CURRENT_LIST_DIR}/../lib/thirdparty
    )
    set(RTTR_LIBS_RELEASE_DLL ${RTTR_LIBS_DIR}/Release/librttr_core.0.9.6.dylib)
    set(RTTR_LIBS_DEBUG_DLL ${RTTR_LIBS_DIR}/Debug/librttr_core_d.0.9.6.dylib)
elseif (UNIX)
    find_path(
        RTTR_LIBS_DIR
        NAMES Release/librttr_core.so.0.9.6
        HINTS ${CMAKE_CURRENT_LIST_DIR}/../lib/thirdparty
    )
    set(RTTR_LIBS_RELEASE_DLL ${RTTR_LIBS_DIR}/Release/librttr_core.so.0.9.6)
    set(RTTR_LIBS_DEBUG_DLL ${RTTR_LIBS_DIR}/Debug/librttr_core.so.0.9.6)
endif()

add_library(rttr SHARED IMPORTED)
set_target_properties(rttr PROPERTIES
  IMPORTED_CONFIGURATIONS "Debug;Release;MinSizeRel;RelWithDebInfo"
  IMPORTED_LOCATION_RELEASE ${RTTR_LIBS_RELEASE_DLL}
  IMPORTED_LOCATION_DEBUG ${RTTR_LIBS_DEBUG_DLL}
  IMPORTED_LOCATION_MINSIZEREL ${RTTR_LIBS_RELEASE_DLL}
  IMPORTED_LOCATION_RELWITHDEBINFO ${RTTR_LIBS_RELEASE_DLL}
)

# TODO later: Fix CMake approach and use config-style package files

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(rttr REQUIRED_VARS RTTR_LIBS_RELEASE_DLL RTTR_LIBS_DIR)
