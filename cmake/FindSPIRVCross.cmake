find_path(SPIRVCROSS_DIR
          NAMES spirv_cross/spirv.h
          HINTS ${THIRDPARTY_DIR}/SPIRV-cross/install/include
          )

set(SPIRVCROSS_INCLUDE_DIR ${SPIRVCROSS_DIR})

if(WIN32)
    set(SPIRVCROSS_LIBS_DEBUG ${SPIRVCROSS_DIR}/../lib/msvc/Debug/spirv-cross-core.lib)
    set(SPIRVCROSS_LIBS_RELEASE ${SPIRVCROSS_DIR}/../lib/msvc/Release/spirv-cross-core.lib)
endif()


mark_as_advanced(SPIRVCROSS_INCLUDE_DIR)
mark_as_advanced(SPIRVCROSS_LIBS_DEBUG)
mark_as_advanced(SPIRVCROSS_LIBS_RELEASE)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SPIRVCross REQUIRED_VARS SPIRVCROSS_DIR)