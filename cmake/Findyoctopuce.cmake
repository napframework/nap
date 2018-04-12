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
    set(YOCTO_DEBUG_DLL ${YOCTO_DIR}/Binaries/windows/x64/Debug/yocto.dll)
    set(YOCTO_RELEASE_DLL ${YOCTO_DIR}/Binaries/windows/x64/Release/yocto.dll)
elseif(APPLE)
    set(YOCTO_LIBS_DEBUG ${YOCTO_DIR}/Binaries/osx/libyocto.dylib)
    set(YOCTO_LIBS_RELEASE ${YOCTO_DIR}/Binaries/osx/libyocto.dylib)
    set(YOCTO_LIBS_DIR ${YOCTO_DIR}/Binaries/osx)
    set(YOCTO_DEBUG_DLL ${YOCTO_LIBS_DEBUG})
    set(YOCTO_RELEASE_DLL ${YOCTO_LIBS_RELEASE})
else()
    set(YOCTO_LIBS_DEBUG ${YOCTO_DIR}/Binaries/linux/64bits/libyocto.so.1.0.1)
    set(YOCTO_LIBS_RELEASE ${YOCTO_DIR}/Binaries/linux/64bits/libyocto.so.1.0.1)
    set(YOCTO_LIBS_DIR ${YOCTO_DIR}/Binaries/linux/64bits)
    set(YOCTO_DEBUG_DLL ${YOCTO_LIBS_DEBUG})
    set(YOCTO_RELEASE_DLL ${YOCTO_LIBS_RELEASE})
endif()

mark_as_advanced(YOCTO_INCLUDE_DIRS)
mark_as_advanced(YOCTO_LIBS_DEBUG)
mark_as_advanced(YOCTO_LIBS_RELEASE)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(yoctopuce REQUIRED_VARS YOCTO_INCLUDE_DIRS YOCTO_LIBS_DEBUG YOCTO_LIBS_RELEASE YOCTO_LIBS_DIR)

add_library(yoctopuce SHARED IMPORTED)
set_target_properties(yoctopuce PROPERTIES
                      IMPORTED_CONFIGURATIONS "Debug;Release;MinSizeRel;RelWithDebInfo"
                      IMPORTED_LOCATION_RELEASE ${YOCTO_RELEASE_DLL}
                      IMPORTED_LOCATION_DEBUG ${YOCTO_DEBUG_DLL}
                      IMPORTED_LOCATION_MINSIZEREL ${YOCTO_RELEASE_DLL}
                      IMPORTED_LOCATION_RELWITHDEBINFO ${YOCTO_RELEASE_DLL}
                      )

if(WIN32)
    set_target_properties(yoctopuce PROPERTIES
                          IMPORTED_IMPLIB_RELEASE ${YOCTO_LIBS_RELEASE}
                          IMPORTED_IMPLIB_DEBUG ${YOCTO_LIBS_DEBUG}
                          )
endif()

# Copy the etherdream dynamic linked lib into the build directory
macro(copy_yoctopuce_dll)
    add_custom_command(
            TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different 
                    $<TARGET_FILE:yoctopuce> 
                   "$<TARGET_PROPERTY:${PROJECT_NAME},RUNTIME_OUTPUT_DIRECTORY_$<UPPER_CASE:$<CONFIG>>>"

            )
endmacro()
