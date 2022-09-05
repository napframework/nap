# Find install directory
if(WIN32)
    find_path(SPIRVCROSS_DIR
          NAMES include/spirv_cross/spirv.h
          HINTS ${THIRDPARTY_DIR}/SPIRV-cross/msvc/x86_64
          )

    # Debug core lib
    find_library(SPIRVCROSS_LIBS_DEBUG
             NAMES spirv-cross-cored
             PATHS ${SPIRVCROSS_DIR}/lib               
             NO_DEFAULT_PATH
            )

    # Release core lib
    find_library(SPIRVCROSS_LIBS_RELEASE
             NAMES spirv-cross-core
             PATHS ${SPIRVCROSS_DIR}/lib               
             NO_DEFAULT_PATH
            )
elseif(APPLE)
    find_path(SPIRVCROSS_DIR
          NAMES include/spirv_cross/spirv.h
          HINTS ${THIRDPARTY_DIR}/SPIRV-cross/macos/x86_64
          )

    # Release core lib
    find_library(SPIRVCROSS_LIBS_RELEASE
             NAMES spirv-cross-core
             PATHS ${SPIRVCROSS_DIR}
             PATH_SUFFIXES lib               
             NO_DEFAULT_PATH
            )

    set(SPIRVCROSS_LIBS_DEBUG ${SPIRVCROSS_LIBS_RELEASE})
elseif(UNIX)
    find_path(SPIRVCROSS_DIR
          NAMES include/spirv_cross/spirv.h
          HINTS ${THIRDPARTY_DIR}/SPIRV-cross/linux/${ARCH}
          )

    # Release core lib
    find_library(SPIRVCROSS_LIBS_RELEASE
             NAMES spirv-cross-core
             PATHS ${SPIRVCROSS_DIR}
             PATH_SUFFIXES lib               
             NO_DEFAULT_PATH
            )

    set(SPIRVCROSS_LIBS_DEBUG ${SPIRVCROSS_LIBS_RELEASE})
endif()

# Include directory
find_path(SPIRVCROSS_INCLUDE_DIR
            NAMES spirv_cross/spirv.h
            HINTS ${SPIRVCROSS_DIR}/include
            )

mark_as_advanced(SPIRVCROSS_DIR)
mark_as_advanced(SPIRVCROSS_INCLUDE_DIR)
mark_as_advanced(SPIRVCROSS_LIBS_DEBUG)
mark_as_advanced(SPIRVCROSS_LIBS_RELEASE)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SPIRVCross REQUIRED_VARS SPIRVCROSS_DIR SPIRVCROSS_INCLUDE_DIR SPIRVCROSS_LIBS_DEBUG SPIRVCROSS_LIBS_RELEASE)
