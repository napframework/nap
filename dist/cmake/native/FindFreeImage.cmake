if (WIN32)
    find_path(
        FREEIMAGE_DIR
        NAMES bin/FreeImage.dll
        HINTS ${THIRDPARTY_DIR}/FreeImage
    )
    set(FREEIMAGE_INCLUDE_DIRS ${FREEIMAGE_DIR}/include/)
    set(FREEIMAGE_LIBS_DLL ${FREEIMAGE_DIR}/bin/FreeImage.dll)
    set(FREEIMAGE_IMPLIB ${FREEIMAGE_DIR}/lib/FreeImage.lib)
elseif(APPLE)
    find_path(
        FREEIMAGE_DIR
        NAMES include/FreeImage.h
        HINTS ${THIRDPARTY_DIR}/FreeImage
    )
    set(FREEIMAGE_INCLUDE_DIRS ${FREEIMAGE_DIR}/include/)
    set(FREEIMAGE_LIBS_DLL ${FREEIMAGE_DIR}/lib/libfreeimage-3.17.0.dylib)
elseif(UNIX)
    find_path(
        FREEIMAGE_DIR
        NAMES lib/libfreeimage.so
        HINTS ${THIRDPARTY_DIR}/FreeImage
    )
    set(FREEIMAGE_INCLUDE_DIRS ${FREEIMAGE_DIR}/include/)
    set(FREEIMAGE_LIBS_DLL ${FREEIMAGE_DIR}/lib/libfreeimage.so)
endif()

mark_as_advanced(FREEIMAGE_INCLUDE_DIRS)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(FreeImage REQUIRED_VARS FREEIMAGE_DIR)

if (WIN32 OR (UNIX AND NOT APPLE))
    add_library(FreeImage SHARED IMPORTED)
    set_target_properties(FreeImage PROPERTIES
        IMPORTED_CONFIGURATIONS "Debug;Release;MinSizeRel;RelWithDebInfo"
        IMPORTED_LOCATION_RELEASE ${FREEIMAGE_LIBS_DLL}
        IMPORTED_LOCATION_DEBUG ${FREEIMAGE_LIBS_DLL}
        IMPORTED_LOCATION_MINSIZEREL ${FREEIMAGE_LIBS_DLL}
        IMPORTED_LOCATION_RELWITHDEBINFO ${FREEIMAGE_LIBS_DLL}
    )
endif()

if (WIN32)
    set_target_properties(FreeImage PROPERTIES
        IMPORTED_IMPLIB ${FREEIMAGE_IMPLIB}
    )

    # Copy over DLL post-build
    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:FreeImage> $<TARGET_FILE_DIR:${PROJECT_NAME}>/
    )
endif()