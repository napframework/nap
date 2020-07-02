# find vulkan install directory
if(WIN32)
	find_path(SPIRVCROSS_DIR
          NAMES include/spirv_cross/spirv.h
          HINTS ${THIRDPARTY_DIR}/SPIRV-cross/msvc/install
          )

	# debug core lib
	find_library(SPIRVCROSS_LIBS_DEBUG
			 NAMES spirv-cross-cored
			 PATHS ${SPIRVCROSS_DIR}/lib			   
			 NO_DEFAULT_PATH
			)

	# release core lib
	find_library(SPIRVCROSS_LIBS_RELEASE
			 NAMES spirv-cross-core
			 PATHS ${SPIRVCROSS_DIR}/lib			   
			 NO_DEFAULT_PATH
			)

elseif(APPLE)
	find_path(SPIRVCROSS_DIR
          NAMES include/spirv_cross/spirv.h
          HINTS ${THIRDPARTY_DIR}/SPIRV-cross/osx/install
          )
elseif(UNIX)
	find_path(SPIRVCROSS_DIR
          NAMES include/spirv_cross/spirv.h
          HINTS ${THIRDPARTY_DIR}/SPIRV-cross/linux/install
          )

	# release core lib
	find_library(SPIRVCROSS_LIBS_RELEASE
			 NAMES spirv-cross-core
			 PATHS ${SPIRVCROSS_DIR}
			 PATH_SUFFIXES lib			   
			 NO_DEFAULT_PATH
			)

	set(SPIRVCROSS_LIBS_DEBUG ${SPIRVCROSS_LIBS_RELEASE})
endif()

# include directory
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