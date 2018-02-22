find_path(
  YOCTO_DIR
  NAMES Sources/yocto_api.h
  HINTS ${THIRDPARTY_DIR}/yoctopuce
  )

set(YOCTO_INCLUDE_DIRS ${YOCTO_DIR}/Sources ${YOCTO_DIR}/Sources/yapi)

if(WIN32)
    set(YOCTO_LIBS ${YOCTO_DIR}/Binaries/windows/yocto.lib ${YOCTO_DIR}/Binaries/windows/yapi/yapi.lib)
    set(YOCTO_LIBS_DIR ${YOCTO_DIR}/Binaries/windows ${YOCTO_DIR}/Binaries/windows/yapi)
    set(YOCTO_DLLS ${YOCTO_DIR}/Binaries/windows/yocto.dll ${YOCTO_DIR}/Binaries/windows/yapi/yapi.dll)
endif()

mark_as_advanced(YOCTO_INCLUDE_DIRS)
mark_as_advanced(YOCTO_LIBS_DIR)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(yoctopuce REQUIRED_VARS YOCTO_INCLUDE_DIRS YOCTO_LIBS YOCTO_LIBS_DIR)

# Copy the etherdream dynamic linked lib into the build directory
macro(copy_yoctopuce_dll)
    add_library(yoctopucelib SHARED IMPORTED)
    set_target_properties(yoctopucelib PROPERTIES
                          IMPORTED_CONFIGURATIONS "Debug;Release;MinSizeRel;RelWithDebInfo"
                          IMPORTED_LOCATION_RELEASE ${YOCTO_DLLS}
                          IMPORTED_LOCATION_DEBUG ${YOCTO_DLLS}
                          IMPORTED_LOCATION_MINSIZEREL ${YOCTO_DLLS}
                          IMPORTED_LOCATION_RELWITHDEBINFO ${YOCTO_DLLS}
                          )

    add_custom_command(
            TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:yoctopucelib> $<TARGET_FILE_DIR:${PROJECT_NAME}>/$<TARGET_FILE_NAME:yoctopucelib>
    )
endmacro()
