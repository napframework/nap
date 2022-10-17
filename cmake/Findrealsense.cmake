# - Try to find RealSense SDK

find_path(REALSENSE_DIR
          NAMES
          source/rs.h
          HINTS
          ${THIRDPARTY_DIR}/realsense
          ${CMAKE_CURRENT_LIST_DIR}/../../realsense
          )

set(REALSENSE_INCLUDE_DIR ${REALSENSE_DIR}/source)

if(WIN32)
    set(REALSENSE_LIBRARY_DIR ${REALSENSE_DIR}/msvc/${ARCH}/lib)
    set(REALSENSE_LIBRARIES_RELEASE ${REALSENSE_LIBRARY_DIR}/realsense2.lib)
    set(REALSENSE_LIBRARIES_DEBUG ${REALSENSE_LIBRARY_DIR}/realsense2.lib)
    set(REALSENSE_LIBRARIES_RELEASE_DLL ${REALSENSE_LIBRARY_DIR}/realsense2.dll)
    set(REALSENSE_LIBRARIES_DEBUG_DLL ${REALSENSE_LIBRARY_DIR}/realsense2.dll)
else()
    set(REALSENSE_LIBRARY_DIR ${REALSENSE_DIR}/linux/${ARCH}/lib)
    set(REALSENSE_LIBRARIES_RELEASE ${REALSENSE_LIBRARY_DIR}/librealsense2.so.2.51.1)
    set(REALSENSE_LIBRARIES_DEBUG ${REALSENSE_LIBRARY_DIR}/librealsense2.so.2.51.1)
endif()

include(FindPackageHandleStandardArgs)

add_library(realsense SHARED IMPORTED)

# handle the QUIETLY and REQUIRED arguments and set WIRINGPI_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(realsense REQUIRED_VARS
        REALSENSE_DIR
        REALSENSE_INCLUDE_DIR
        REALSENSE_LIBRARY_DIR
        REALSENSE_LIBRARIES_DEBUG
        REALSENSE_LIBRARIES_DEBUG)

set_target_properties(realsense PROPERTIES
        IMPORTED_CONFIGURATIONS "Debug;Release;MinSizeRel;RelWithDebInfo"
        IMPORTED_LOCATION_RELEASE ${REALSENSE_LIBRARIES_RELEASE_DLL}
        IMPORTED_LOCATION_DEBUG ${REALSENSE_LIBRARIES_DEBUG_DLL}
        IMPORTED_LOCATION_MINSIZEREL ${REALSENSE_LIBRARIES_RELEASE_DLL}
        IMPORTED_LOCATION_RELWITHDEBINFO ${REALSENSE_LIBRARIES_RELEASE_DLL}
        )

#
macro(copy_realsense_dll)
    add_custom_command(TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:realsense>
            "$<TARGET_PROPERTY:${PROJECT_NAME},RUNTIME_OUTPUT_DIRECTORY_$<UPPER_CASE:$<CONFIG>>>"
            )
endmacro()
