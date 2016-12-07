find_path(
        FREEIMAGE_DIR
        NAMES Dist/x64/FreeImage.h
        HINTS
        ${CMAKE_CURRENT_LIST_DIR}/../FreeImage
        ${CMAKE_CURRENT_LIST_DIR}/../thirdparty/FreeImage
)

if (MSVC)
    set(FREEIMAGE_INCLUDE_DIRS ${FREEIMAGE_DIR}/msvc/Dist/x64)
    set(FREEIMAGE_LIBRARIES ${FREEIMAGE_DIR}/msvc/Dist/x64/FreeImage.lib)
endif()



mark_as_advanced(FREEIMAGE_INCLUDE_DIRS)