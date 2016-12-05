find_path(
        FREEIMAGE_DIR
        NAMES Dist/x32/FreeImage.h
        HINTS
        ${CMAKE_CURRENT_LIST_DIR}/../FreeImage
)

set(FREEIMAGE_INCLUDE_DIRS ${FREEIMAGE_DIR}/Dist/x64)
set(FREEIMAGE_LIBRARIES ${FREEIMAGE_DIR}/Dist/x64/FreeImage.lib)



mark_as_advanced(FREEIMAGE_INCLUDE_DIRS)