if (WIN32)
    find_path(
        REALSENSE_DIR
        NAMES bin/realsense2.dll
        HINTS ${THIRDPARTY_DIR}/librealsense
    )
    set(REALSENSE_LIBS_DIR ${REALSENSE_DIR}/bin)
    set(REALSENSE_LIBS ${REALSENSE_LIBS_DIR}/realsense2.lib)
    set(REALSENSE_LIBS_RELEASE_DLL ${REALSENSE_LIBS_DIR}/realsense2.dll)
else()
    find_path(
        REALSENSE_DIR
        NAMES bin/librealsense2.so.2.51.1
        HINTS ${THIRDPARTY_DIR}/librealsense
    )   
    set(REALSENSE_LIBS_DIR ${REALSENSE_DIR}/bin)
    set(REALSENSE_LIBS ${REALSENSE_LIBS_DIR}/librealsense2.so.2.51.1)
    set(REALSENSE_LIBS_RELEASE_DLL ${REALSENSE_LIBS})
endif()

mark_as_advanced(REALSENSE_LIBS_DIR)

# promote package for find
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(realsense REQUIRED_VARS REALSENSE_LIBS_DIR)

add_library(realsense SHARED IMPORTED)
set_target_properties(realsense PROPERTIES
                      IMPORTED_CONFIGURATIONS "Debug;Release"
                      IMPORTED_LOCATION_RELEASE ${REALSENSE_LIBS_RELEASE_DLL}
                      IMPORTED_LOCATION_DEBUG ${REALSENSE_LIBS_RELEASE_DLL}
                      )

if(WIN32)
    set_target_properties(realsense PROPERTIES
                          IMPORTED_IMPLIB_RELEASE ${REALSENSE_LIBS}
                          IMPORTED_IMPLIB_DEBUG ${REALSENSE_LIBS}
                          )
endif()

# Copy the realsense dynamic linked lib into the build directory
macro(copy_realsense_dll)
    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:realsense> $<TARGET_FILE_DIR:${PROJECT_NAME}>/$<TARGET_FILE_NAME:realsense>
    )
endmacro()
