find_path(
    YOCTOPUCE_DIR
    NAMES README.txt
    HINTS ${NAP_ROOT}/system_modules/napyoctopuce/thirdparty/yoctopuce
    )

find_path(
    YOCTOPUCE_INCLUDE_DIR
    NAMES yocto_api.h
    HINTS ${YOCTOPUCE_DIR}/source/Sources
    )
set(YOCTOPUCE_INCLUDE_DIRS ${YOCTOPUCE_INCLUDE_DIR} ${YOCTOPUCE_INCLUDE_DIR}/yapi)

if(UNIX)
    if(APPLE)
        set(YOCTOPUCE_LIBS_DIR ${YOCTOPUCE_DIR}/macos/x86_64)
    else()
        set(YOCTOPUCE_LIBS_DIR ${YOCTOPUCE_DIR}/linux/${ARCH})
    endif()
    set(YOCTOPUCE_LIBS_RELEASE ${YOCTOPUCE_LIBS_DIR}/libyocto${CMAKE_SHARED_LIBRARY_SUFFIX})
    set(YOCTOPUCE_LIBS_DEBUG ${YOCTOPUCE_LIBS_RELEASE})
    set(YOCTOPUCE_LIBS_RELEASE_DLL ${YOCTOPUCE_LIBS_RELEASE})
    set(YOCTOPUCE_LIBS_DEBUG_DLL ${YOCTOPUCE_LIBS_DEBUG})
else()
    set(YOCTOPUCE_LIBS_DIR ${YOCTOPUCE_DIR}/msvc/x86_64)
    set(YOCTOPUCE_LIBS_RELEASE ${YOCTOPUCE_LIBS_DIR}/Release/yocto.lib)
    set(YOCTOPUCE_LIBS_DEBUG ${YOCTOPUCE_LIBS_DIR}/Debug/yocto.lib)
    set(YOCTOPUCE_LIBS_RELEASE_DLL ${YOCTOPUCE_LIBS_DIR}/Release/yocto.dll)
    set(YOCTOPUCE_LIBS_DEBUG_DLL ${YOCTOPUCE_LIBS_DIR}/Debug/yocto.dll)
endif()

set(YOCTOPUCE_LICENSE_FILES ${YOCTOPUCE_DIR}/README.txt)

mark_as_advanced(YOCTOPUCE_DIR)
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
                -E copy_if_different
                $<TARGET_FILE:yoctopuce>
                $<TARGET_FILE_DIR:${PROJECT_NAME}>/$<TARGET_FILE_NAME:yoctopuce>
    )
endmacro()
