# Find RTTR
set(RTTR_DIR "${NAP_ROOT}/thirdparty/rttr/cmake")
find_package(RTTR CONFIG REQUIRED Core)

# Find Python
set(pybind11_DIR "${NAP_ROOT}/thirdparty/pybind11/share/cmake/pybind11")
find_package(pybind11 REQUIRED)
target_include_directories(${PROJECT_NAME} PUBLIC ${pybind11_INCLUDE_DIRS})

if (WIN32)
    find_path(
        NAPRTTI_LIBS_DIR
        NAMES Release/naprtti.lib
        HINTS
            ${CMAKE_CURRENT_LIST_DIR}/../lib/
    )
    set(NAPRTTI_LIBS_RELEASE_DLL ${NAPRTTI_LIBS_DIR}/Release/naprtti.dll)
    set(NAPRTTI_LIBS_DEBUG_DLL ${NAPRTTI_LIBS_DIR}/Debug/naprtti.dll)
    set(NAPRTTI_LIBS_IMPLIB_DEBUG ${NAPRTTI_LIBS_DIR}/Debug/naprtti.lib)
    set(NAPRTTI_LIBS_IMPLIB_RELEASE ${NAPRTTI_LIBS_DIR}/Release/naprtti.lib)
elseif (APPLE)
    find_path(
        NAPRTTI_LIBS_DIR
        NAMES Release/libnaprtti.dylib
        HINTS
            ${CMAKE_CURRENT_LIST_DIR}/../lib/
    )
    set(NAPRTTI_LIBS_RELEASE_DLL ${NAPRTTI_LIBS_DIR}/Release/libnaprtti.dylib)
    set(NAPRTTI_LIBS_DEBUG_DLL ${NAPRTTI_LIBS_DIR}/Debug/libnaprtti.dylib)
elseif (UNIX)
    find_path(
        NAPRTTI_LIBS_DIR
        NAMES Release/libnaprtti.so
        HINTS
            ${CMAKE_CURRENT_LIST_DIR}/../lib/
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

if (WIN32)
    set_target_properties(naprtti PROPERTIES
        IMPORTED_IMPLIB_RELEASE ${NAPRTTI_LIBS_IMPLIB_RELEASE}
        IMPORTED_IMPLIB_DEBUG ${NAPRTTI_LIBS_IMPLIB_DEBUG}
        IMPORTED_IMPLIB_MINSIZEREL ${NAPRTTI_LIBS_IMPLIB_RELEASE}
        IMPORTED_IMPLIB_RELWITHDEBINFO ${NAPRTTI_LIBS_IMPLIB_RELEASE}
    )
endif()

# TODO later: Fix CMake approach and use config-style package files

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(naprtti REQUIRED_VARS NAPRTTI_LIBS_RELEASE_DLL NAPRTTI_LIBS_DIR)


if (WIN32)
    # Copy over DLLs post-build
    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:naprtti> $<TARGET_FILE_DIR:${PROJECT_NAME}>/
    )

    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:RTTR::Core> $<TARGET_FILE_DIR:${PROJECT_NAME}>/
    )
endif()

