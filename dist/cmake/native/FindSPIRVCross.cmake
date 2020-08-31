find_path(SPIRVCROSS_DIR
          NAMES spirv_cross/spirv.h
          HINTS ${THIRDPARTY_DIR}/SPIRV-cross/include
          )

set(SPIRVCROSS_INCLUDE_DIR ${SPIRVCROSS_DIR})
mark_as_advanced(SPIRVCROSS_INCLUDE_DIR)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SPIRVCross REQUIRED_VARS SPIRVCROSS_DIR)