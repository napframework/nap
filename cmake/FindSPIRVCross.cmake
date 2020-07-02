if(WIN32)

	find_path(SPIRVCROSS_DIR
          NAMES include/spirv_cross/spirv.h
          HINTS ${THIRDPARTY_DIR}/SPIRV-cross/msvc/install
          )

	# include dir
	set(SPIRVCROSS_INCLUDE_DIR ${SPIRVCROSS_DIR}/include)

	# required libs, only core!
    set(SPIRVCROSS_LIBS_DEBUG ${SPIRVCROSS_DIR}/lib/spirv-cross-cored.lib)
    set(SPIRVCROSS_LIBS_RELEASE ${SPIRVCROSS_DIR}/lib/spirv-cross-core.lib)
endif()


mark_as_advanced(SPIRVCROSS_INCLUDE_DIR)
mark_as_advanced(SPIRVCROSS_LIBS_DEBUG)
mark_as_advanced(SPIRVCROSS_LIBS_RELEASE)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SPIRVCross REQUIRED_VARS SPIRVCROSS_DIR SPIRVCROSS_INCLUDE_DIR SPIRVCROSS_LIBS_DEBUG SPIRVCROSS_LIBS_RELEASE)