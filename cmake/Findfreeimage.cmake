if(WIN32)
    find_path(
            FREEIMAGE_DIR
            NAMES x86_64/Dist/x64/FreeImage.h
            HINTS ${THIRDPARTY_DIR}/FreeImage/msvc
    )
    set(FREEIMAGE_INCLUDE_DIRS ${FREEIMAGE_DIR}/x86_64/Dist/x64)
    set(FREEIMAGE_LIBS_DIRS ${FREEIMAGE_DIR}/x86_64/Dist/x64)
    set(FREEIMAGE_LIBRARIES ${FREEIMAGE_LIBS_DIRS}/FreeImage.lib)
    set(FREEIMAGE_LIBS_RELEASE_DLL ${FREEIMAGE_LIBS_DIRS}/FreeImage.dll)
elseif(APPLE)
    find_path(
            FREEIMAGE_DIR
            NAMES include/FreeImage.h
            HINTS
            ${THIRDPARTY_DIR}/FreeImage/macos/x86_64
    )
    set(FREEIMAGE_INCLUDE_DIRS ${FREEIMAGE_DIR}/include)
    set(FREEIMAGE_LIBRARIES ${FREEIMAGE_DIR}/lib/libfreeimage-3.18.0.dylib)
elseif(UNIX)
    find_path(
            FREEIMAGE_DIR
            NAMES include/FreeImage.h
            HINTS
            ${THIRDPARTY_DIR}/FreeImage/linux/${ARCH}
    )
    set(FREEIMAGE_INCLUDE_DIRS ${FREEIMAGE_DIR}/include)
    set(FREEIMAGE_LIBRARIES ${FREEIMAGE_DIR}/lib/libfreeimage.so)
endif()

add_library(FreeImage SHARED IMPORTED)
set_target_properties(FreeImage PROPERTIES
                      IMPORTED_CONFIGURATIONS "Debug;Release;MinSizeRel;RelWithDebInfo"
                      IMPORTED_LOCATION_RELEASE ${FREEIMAGE_LIBS_RELEASE_DLL}
                      IMPORTED_LOCATION_DEBUG ${FREEIMAGE_LIBS_RELEASE_DLL}
                      IMPORTED_LOCATION_MINSIZEREL ${FREEIMAGE_LIBS_RELEASE_DLL}
                      IMPORTED_LOCATION_RELWITHDEBINFO ${FREEIMAGE_LIBS_RELEASE_DLL}
                      )

mark_as_advanced(FREEIMAGE_INCLUDE_DIRS)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(freeimage REQUIRED_VARS FREEIMAGE_DIR FREEIMAGE_LIBRARIES FREEIMAGE_INCLUDE_DIRS)

# Copy the freeimage dynamic linked lib into the build directory
macro(copy_freeimage_dll)
    if(WIN32)
        if(NOT TARGET FreeImage)
            find_package(freeimage REQUIRED)
        endif()
        add_custom_command(
                TARGET ${PROJECT_NAME}
                POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                         $<TARGET_FILE:FreeImage> 
                        "$<TARGET_PROPERTY:${PROJECT_NAME},RUNTIME_OUTPUT_DIRECTORY_$<UPPER_CASE:$<CONFIG>>>"
                )
    endif()
endmacro()
