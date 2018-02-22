find_path(
  YOCTO_DIR
  NAMES Sources/yocto_api.h
  HINTS ${THIRDPARTY_DIR}/yoctopuce
  )

set(YOCTO_INCLUDE_DIRS ${YOCTO_DIR}/Sources ${YOCTO_DIR}/Sources/yapi)

if(WIN32)
    set(YOCTO_LIBS_DEBUG ${YOCTO_DIR}/Binaries/windows/x64/Debug/yocto.lib)
    set(YOCTO_LIBS_RELEASE ${YOCTO_DIR}/Binaries/windows/x64/Release/yocto.lib)
    set(YOCTO_LIBS_DIR ${YOCTO_DIR}/Binaries/windows/x64)
elseif(APPLE)
    set(YOCTO_LIBS_DEBUG ${YOCTO_DIR}/Binaries/osx/libyocto.dylib)
    set(YOCTO_LIBS_RELEASE ${YOCTO_DIR}/Binaries/osx/libyocto.dylib)
    set(YOCTO_LIBS_DIR ${YOCTO_DIR}/Binaries/osx)
else()
    set(YOCTO_LIBS_DEBUG ${YOCTO_DIR}/Binaries/linux/64bits/libyocto.so.1.0.1)
    set(YOCTO_LIBS_RELEASE ${YOCTO_DIR}/Binaries/linux/64bits/libyocto.so.1.0.1)
    set(YOCTO_LIBS_DIR ${YOCTO_DIR}/Binaries/linux/64bits)
endif()

mark_as_advanced(YOCTO_INCLUDE_DIRS)
mark_as_advanced(YOCTO_LIBS_DEBUG)
mark_as_advanced(YOCTO_LIBS_RELEASE)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(yoctopuce REQUIRED_VARS YOCTO_INCLUDE_DIRS YOCTO_LIBS_DEBUG YOCTO_LIBS_RELEASE YOCTO_LIBS_DIR)

set(YOCTO_DEBUG_DLL ${YOCTO_DIR}/Binaries/windows/x64/Debug/yocto.dll)
set(YOCTO_RELEASE_DLL ${YOCTO_DIR}/Binaries/windows/x64/Release/yocto.dll)

# Copy the etherdream dynamic linked lib into the build directory
macro(copy_yoctopuce_dll)
    add_library(yoctopucelib SHARED IMPORTED)
    set_target_properties(yoctopucelib PROPERTIES
                          IMPORTED_CONFIGURATIONS "Debug;Release;MinSizeRel;RelWithDebInfo"
                          IMPORTED_LOCATION_RELEASE ${YOCTO_RELEASE_DLL}
                          IMPORTED_LOCATION_DEBUG ${YOCTO_DEBUG_DLL}
                          IMPORTED_LOCATION_MINSIZEREL ${YOCTO_RELEASE_DLL}
                          IMPORTED_LOCATION_RELWITHDEBINFO ${YOCTO_RELEASE_DLL}
                          )

    add_custom_command(
            TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:yoctopucelib> $<TARGET_FILE_DIR:${PROJECT_NAME}>/$<TARGET_FILE_NAME:yoctopucelib>
    )
endmacro()
