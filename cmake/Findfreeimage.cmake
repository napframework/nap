
if (WIN32)
    find_path(
        FREEIMAGE_DIR
        NAMES msvc/Dist/x64/FreeImage.h
        HINTS ${CMAKE_CURRENT_LIST_DIR}/../../thirdparty/FreeImage
    )
    set(FREEIMAGE_INCLUDE_DIRS ${FREEIMAGE_DIR}/msvc/Dist/x64)
    set(FREEIMAGE_LIBRARIES ${FREEIMAGE_DIR}/msvc/Dist/x64/FreeImage.lib)
elseif (APPLE)
    find_path(
        FREEIMAGE_DIR
        NAMES osx/include/FreeImage.h
        HINTS
            ${CMAKE_CURRENT_LIST_DIR}/../../thirdparty/FreeImage
            ${CMAKE_CURRENT_LIST_DIR}/../thirdparty/FreeImage
    )
    set(FREEIMAGE_INCLUDE_DIRS ${FREEIMAGE_DIR}/osx/include)
    set(FREEIMAGE_LIBRARIES ${FREEIMAGE_DIR}/osx/lib/osx/freeimage.a)
endif()



mark_as_advanced(FREEIMAGE_INCLUDE_DIRS)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(freeimage REQUIRED_VARS FREEIMAGE_INCLUDE_DIRS FREEIMAGE_LIBRARIES)
