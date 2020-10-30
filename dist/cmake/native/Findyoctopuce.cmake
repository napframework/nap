find_path(
  YOCTOPUCE_DIR
  NAMES include/yocto_api.h
  HINTS ${THIRDPARTY_DIR}/yoctopuce
  )

set(YOCTOPUCE_INCLUDE_DIRS ${YOCTOPUCE_DIR}/include ${YOCTOPUCE_DIR}/include/yapi)
set(YOCTOPUCE_LIBS_DIR ${YOCTOPUCE_DIR}/bin)

if (WIN32)
    set(YOCTOPUCE_LIBS_RELEASE ${YOCTOPUCE_LIBS_DIR}/release/yocto.lib)
    set(YOCTOPUCE_LIBS_DEBUG ${YOCTOPUCE_LIBS_DIR}/debug/yocto.lib)
    set(YOCTOPUCE_LIBS_RELEASE_DLL ${YOCTOPUCE_LIBS_DIR}/release/yocto.dll)
    set(YOCTOPUCE_LIBS_DEBUG_DLL ${YOCTOPUCE_LIBS_DIR}/debug/yocto.dll)
elseif(APPLE)
    set(YOCTOPUCE_LIBS_RELEASE ${YOCTOPUCE_LIBS_DIR}/libyocto.dylib)
    set(YOCTOPUCE_LIBS_DEBUG ${YOCTOPUCE_LIBS_RELEASE})
    set(YOCTOPUCE_LIBS_RELEASE_DLL ${YOCTOPUCE_LIBS_RELEASE})
    set(YOCTOPUCE_LIBS_DEBUG_DLL ${YOCTOPUCE_LIBS_DEBUG})
else()
    set(YOCTOPUCE_LIBS_RELEASE ${YOCTOPUCE_LIBS_DIR}/libyocto.so.1.0.1)
    set(YOCTOPUCE_LIBS_DEBUG ${YOCTOPUCE_LIBS_RELEASE})
    set(YOCTOPUCE_LIBS_RELEASE_DLL ${YOCTOPUCE_LIBS_RELEASE})
    set(YOCTOPUCE_LIBS_DEBUG_DLL ${YOCTOPUCE_LIBS_DEBUG})
endif()

mark_as_advanced(YOCTOPUCE_LIBS_DIR)
mark_as_advanced(YOCTOPUCE_INCLUDE_DIRS)
mark_as_advanced(YOCTOPUCE_LIBS_DEBUG)
mark_as_advanced(YOCTOPUCE_LIBS_RELEASE)

# Promote package for find
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(yoctopuce REQUIRED_VARS YOCTOPUCE_DIR)


add_library(yoctopuce SHARED IMPORTED)
set_target_properties(yoctopuce PROPERTIES
                      IMPORTED_CONFIGURATIONS "Debug;Release"
                      IMPORTED_LOCATION_RELEASE ${YOCTOPUCE_LIBS_RELEASE_DLL}
                      IMPORTED_LOCATION_DEBUG ${YOCTOPUCE_LIBS_DEBUG_DLL}
                      )

if(WIN32)
    set_target_properties(yoctopuce PROPERTIES
                          IMPORTED_IMPLIB_RELEASE ${YOCTOPUCE_LIBS_RELEASE}
                          IMPORTED_IMPLIB_DEBUG ${YOCTOPUCE_LIBS_DEBUG}
                          )
endif()

# Copy the yoctopuce dynamic linked lib into the build directory
macro(copy_yoctopuce_dll)
    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} 
                -E copy 
                $<TARGET_FILE:yoctopuce> 
                $<TARGET_FILE_DIR:${PROJECT_NAME}>/$<TARGET_FILE_NAME:yoctopuce>
    )
endmacro()
