if(TARGET FreeImage)
    return()
endif()

set(freeimage_search_dir ${NAP_ROOT}/system_modules/naprender/thirdparty/FreeImage)
if(WIN32)
    find_path(
            FREEIMAGE_DIR
            NAMES Dist/x64/FreeImage.h
            HINTS ${freeimage_search_dir}/msvc/x86_64
    )
    set(FREEIMAGE_INCLUDE_DIR ${FREEIMAGE_DIR}/Dist/x64)
    set(FREEIMAGE_LIBS_DIR ${FREEIMAGE_DIR}/Dist/x64)
    set(FREEIMAGE_LIBRARIES ${FREEIMAGE_LIBS_DIR}/FreeImage.lib)
    set(FREEIMAGE_LIBS_RELEASE_DLL ${FREEIMAGE_LIBS_DIR}/FreeImage.dll)
    file(GLOB FREEIMAGE_LICENSE_FILES ${FREEIMAGE_DIR}/license*.txt)
elseif(APPLE)
    set(FREEIMAGE_DIR ${NAP_ROOT}/system_modules/naprender/thirdparty/FreeImage/macos/${ARCH})
    set(FREEIMAGE_INCLUDE_DIR ${FREEIMAGE_DIR}/include)
    set(FREEIMAGE_LIBS_DIR ${FREEIMAGE_DIR}/lib)
    set(FREEIMAGE_LIBRARIES ${FREEIMAGE_LIBS_DIR}/libfreeimage-3.18.0.dylib)
    file(GLOB FREEIMAGE_LICENSE_FILES ${FREEIMAGE_DIR}/license/license*.txt)
elseif(UNIX)
    find_path(
            FREEIMAGE_DIR
            NAMES include/FreeImage.h
            HINTS ${freeimage_search_dir}/linux/${ARCH}
    )
    set(FREEIMAGE_LIBS_DIR ${FREEIMAGE_DIR}/lib)
    set(FREEIMAGE_INCLUDE_DIR ${FREEIMAGE_DIR}/include)
    set(FREEIMAGE_LIBRARIES ${FREEIMAGE_LIBS_DIR}/libfreeimage.so)
    file(GLOB FREEIMAGE_LICENSE_FILES ${FREEIMAGE_DIR}/license*.txt)
endif()

add_library(FreeImage SHARED IMPORTED)
set_target_properties(FreeImage PROPERTIES
                      IMPORTED_CONFIGURATIONS "Debug;Release;MinSizeRel;RelWithDebInfo"
                      IMPORTED_LOCATION_RELEASE ${FREEIMAGE_LIBS_RELEASE_DLL}
                      IMPORTED_LOCATION_DEBUG ${FREEIMAGE_LIBS_RELEASE_DLL}
                      IMPORTED_LOCATION_MINSIZEREL ${FREEIMAGE_LIBS_RELEASE_DLL}
                      IMPORTED_LOCATION_RELWITHDEBINFO ${FREEIMAGE_LIBS_RELEASE_DLL}
                      )

mark_as_advanced(FREEIMAGE_INCLUDE_DIR)
mark_as_advanced(freeimage_search_dir)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(freeimage REQUIRED_VARS
    FREEIMAGE_DIR
    FREEIMAGE_LIBRARIES
    FREEIMAGE_INCLUDE_DIR
    FREEIMAGE_LIBS_DIR
    FREEIMAGE_LICENSE_FILES)

# Copy the FreeImage dynamic linked lib into the build directory
macro(copy_freeimage_dll)
    if(WIN32)
        if(NOT TARGET FreeImage)
            find_package(freeimage REQUIRED)
        endif()
        add_custom_command(
            TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:FreeImage> $<TARGET_FILE_DIR:${PROJECT_NAME}>/
            )

    endif()
endmacro()
