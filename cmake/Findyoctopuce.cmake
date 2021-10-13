find_path(
  YOCTO_DIR
  NAMES source/Sources/yocto_api.h
  HINTS ${THIRDPARTY_DIR}/yoctopuce/
  )

set(YOCTO_INCLUDE_DIRS ${YOCTO_DIR}/source/Sources ${YOCTO_DIR}/source/Sources/yapi)

if(WIN32)
    set(YOCTO_LIBS_DEBUG ${YOCTO_DIR}/msvc/x86_64/Debug/yocto.lib)
    set(YOCTO_LIBS_RELEASE ${YOCTO_DIR}/msvc/x86_64/Release/yocto.lib)
    set(YOCTO_LIBS_DIR ${YOCTO_DIR}/)
    set(YOCTO_DEBUG_DLL ${YOCTO_DIR}/msvc/x86_64/Debug/yocto.dll)
    set(YOCTO_RELEASE_DLL ${YOCTO_DIR}/msvc/x86_64/Release/yocto.dll)
elseif(APPLE)
    set(YOCTO_LIBS_DEBUG ${YOCTO_DIR}/macos/x86_64/libyocto.dylib)
    set(YOCTO_LIBS_RELEASE ${YOCTO_DIR}/macos/x86_64/libyocto.dylib)
    set(YOCTO_LIBS_DIR ${YOCTO_DIR}/macos)
    set(YOCTO_DEBUG_DLL ${YOCTO_LIBS_DEBUG})
    set(YOCTO_RELEASE_DLL ${YOCTO_LIBS_RELEASE})
else()
    set(YOCTO_LIBS_DIR ${YOCTO_DIR}/linux/${ARCH})
    set(YOCTO_LIBS_RELEASE ${YOCTO_DIR}/linux/${ARCH}/libyocto.so.1.0.1)
    set(YOCTO_LIBS_DEBUG ${YOCTO_LIBS_RELEASE})
    set(YOCTO_RELEASE_DLL ${YOCTO_LIBS_RELEASE})
    set(YOCTO_DEBUG_DLL ${YOCTO_LIBS_DEBUG})
endif()

mark_as_advanced(YOCTO_INCLUDE_DIRS)
mark_as_advanced(YOCTO_LIBS_DEBUG)
mark_as_advanced(YOCTO_LIBS_RELEASE)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(yoctopuce REQUIRED_VARS YOCTO_DIR)

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
