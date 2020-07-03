# find vulkan directory
if(WIN32)
	find_path(VULKANSDK_DIR
		NO_DEFAULT_PATH
        NAMES Include/vulkan/vulkan.h
        HINTS ${THIRDPARTY_DIR}/vulkansdk/msvc
        )
elseif(APPLE)
	find_path(VULKANSDK_DIR
		NO_DEFAULT_PATH
        NAMES Include/vulkan/vulkan.h
        HINTS ${THIRDPARTY_DIR}/vulkansdk/osx
        )
elseif(UNIX)
	find_path(VULKANSDK_DIR
		NO_DEFAULT_PATH
        NAMES Include/vulkan/vulkan.h
        HINTS ${THIRDPARTY_DIR}/vulkansdk/linux
        )
endif()

# include directory
find_path(VULKANSDK_INCLUDE_DIRS
			NO_DEFAULT_PATH
			NAMES vulkan/vulkan.h
			HINTS ${VULKANSDK_DIR}/Include
			)

# vulkan library directory
set(VULKANSDK_LIBS_DIR ${VULKANSDK_DIR}/Lib)

# find vulkan library
if(WIN32)
	# vulkan core lib -> vulkan-1.lib
	find_library(VULKANSDK_LIBS
				NO_DEFAULT_PATH
				NAMES vulkan-1
				PATHS ${VULKANSDK_LIBS_DIR}		   
				)
elseif(UNIX)
	# vulkan core lib -> libvulkan.so
	find_library(VULKANSDK_LIBS
			NO_DEFAULT_PATH
			NAMES vulkan
			PATHS ${VULKANSDK_LIBS_DIR}		   
			)
endif()

mark_as_advanced(VULKANSDK_INCLUDE_DIRS)
mark_as_advanced(VULKANSDK_LIBS_DIR)
mark_as_advanced(VULKANSDK_LIBS)

# promote package for find
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(vulkansdk REQUIRED_VARS VULKANSDK_DIR VULKANSDK_INCLUDE_DIRS VULKANSDK_LIBS)