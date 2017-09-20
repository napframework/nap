
if (WIN32)
    find_path(
        FREEIMAGE_DIR
        NAMES msvc/Dist/x64/FreeImage.h
        HINTS ${CMAKE_CURRENT_LIST_DIR}/../../thirdparty/FreeImage
    )
    set(FREEIMAGE_INCLUDE_DIRS ${FREEIMAGE_DIR}/msvc/Dist/x64)
    set(FREEIMAGE_LIBS_DIRS ${FREEIMAGE_DIR}/msvc/Dist/x64)
    set(FREEIMAGE_LIBRARIES ${FREEIMAGE_LIBS_DIRS}/FreeImage.lib)
    set(FREEIMAGE_LIBS_RELEASE_DLL ${FREEIMAGE_LIBS_DIRS}/FreeImage.dll)
elseif (APPLE)
    find_path(
        FREEIMAGE_DIR
        NAMES include/FreeImage.h
        HINTS
            ${CMAKE_CURRENT_LIST_DIR}/../../thirdparty/FreeImage
            ${CMAKE_CURRENT_LIST_DIR}/../../thirdparty/FreeImage/osx
    )
    set(FREEIMAGE_INCLUDE_DIRS ${FREEIMAGE_DIR}/include)
    set(FREEIMAGE_LIBRARIES ${FREEIMAGE_DIR}/lib/osx/freeimage.a)
endif()



mark_as_advanced(FREEIMAGE_INCLUDE_DIRS)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(freeimage REQUIRED_VARS FREEIMAGE_INCLUDE_DIRS FREEIMAGE_LIBRARIES)

# Copy the freeimage dynamic linked lib into the build directory
macro(copy_freeimage_dll)
    if (WIN32)
        add_library(freeimagelib SHARED IMPORTED)
        set_target_properties(freeimagelib PROPERTIES
          IMPORTED_CONFIGURATIONS "Debug;Release;MinSizeRel;RelWithDebInfo"
          IMPORTED_LOCATION_RELEASE ${FREEIMAGE_LIBS_RELEASE_DLL}
          IMPORTED_LOCATION_DEBUG ${FREEIMAGE_LIBS_RELEASE_DLL}
          IMPORTED_LOCATION_MINSIZEREL ${FREEIMAGE_LIBS_RELEASE_DLL}
          IMPORTED_LOCATION_RELWITHDEBINFO ${FREEIMAGE_LIBS_RELEASE_DLL}
          )

        add_custom_command(
            TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:freeimagelib> $<TARGET_FILE_DIR:${PROJECT_NAME}>/$<TARGET_FILE_NAME:freeimagelib>
        )
    endif()
endmacro()
