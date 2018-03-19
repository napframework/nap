set(YOCTO_DIR ${THIRDPARTY_DIR}/yoctopuce)
set(YOCTO_INCLUDE_DIRS ${YOCTO_DIR}/include ${YOCTO_DIR}/include/yapi)
if (WIN32)
    find_path(
        YOCTOPUCE_DIR
        NAMES bin/release/yocto.dll
        HINTS ${YOCTO_DIR}
        )
    set(YOCTOPUCE_LIBS_DIR ${YOCTOPUCE_DIR}/bin)
    set(YOCTOPUCE_LIBS ${YOCTOPUCE_LIBS_DIR}/release/yocto.lib)
    set(YOCTOPUCE_LIBS_RELEASE_DLL ${YOCTOPUCE_LIBS_DIR}/release/yocto.dll)
elseif(APPLE)
    find_path(
        YOCTOPUCE_DIR
        NAMES bin/libyocto.dylib
        HINTS ${YOCTO_DIR}
        )   
    set(YOCTOPUCE_LIBS_DIR ${YOCTOPUCE_DIR}/bin)
    set(YOCTOPUCE_LIBS ${YOCTOPUCE_LIBS_DIR}/libyocto.dylib)
    set(YOCTOPUCE_LIBS_RELEASE_DLL ${YOCTOPUCE_LIBS})
else()
    find_path(
        YOCTOPUCE_DIR
        NAMES bin/libyocto.so.1.0.1
        HINTS ${YOCTO_DIR}
        )   
    set(YOCTOPUCE_LIBS_DIR ${YOCTOPUCE_DIR}/bin)
    set(YOCTOPUCE_LIBS ${YOCTOPUCE_LIBS_DIR}/libyocto.so.1.0.1)
    set(YOCTOPUCE_LIBS_RELEASE_DLL ${YOCTOPUCE_LIBS})
endif()

mark_as_advanced(YOCTOPUCE_LIBS_DIR)

# promote package for find
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(yoctopuce REQUIRED_VARS YOCTOPUCE_DIR YOCTOPUCE_LIBS YOCTOPUCE_LIBS_DIR YOCTO_INCLUDE_DIRS)

add_library(yoctopuce SHARED IMPORTED)
set_target_properties(yoctopuce PROPERTIES
                      IMPORTED_CONFIGURATIONS "Debug;Release"
                      IMPORTED_LOCATION_RELEASE ${YOCTOPUCE_LIBS_RELEASE_DLL}
                      IMPORTED_LOCATION_DEBUG ${YOCTOPUCE_LIBS_RELEASE_DLL}
                      )

if(WIN32)
    set_target_properties(yoctopuce PROPERTIES
                          IMPORTED_IMPLIB_RELEASE ${YOCTOPUCE_LIBS}
                          IMPORTED_IMPLIB_DEBUG ${YOCTOPUCE_LIBS}
                          )
endif()

# Copy the yoctopuce dynamic linked lib into the build directory
macro(copy_yoctopuce_dll)
    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:yoctopuce> $<TARGET_FILE_DIR:${PROJECT_NAME}>/$<TARGET_FILE_NAME:yoctopuce>
    )
endmacro()
