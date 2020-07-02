# find directory
if(WIN32)
	find_path(VULKANSDK_DIR
		NO_CMAKE_FIND_ROOT_PATH
        NAMES Include/vulkan/vulkan.h
        HINTS ${THIRDPARTY_DIR}/vulkansdk/msvc
        )
elseif(APPLE)
	find_path(VULKANSDK_DIR
		NO_CMAKE_FIND_ROOT_PATH
        NAMES Include/vulkan/vulkan.h
        HINTS ${THIRDPARTY_DIR}/vulkansdk/osx
        )
elseif(UNIX)
	find_path(VULKANSDK_DIR
		NO_CMAKE_FIND_ROOT_PATH
        NAMES Include/vulkan/vulkan.h
        HINTS ${THIRDPARTY_DIR}/vulkansdk/linux
        )
endif()

# include directory
find_path(VULKANSDK_INCLUDE_DIRS
			NO_CMAKE_FIND_ROOT_PATH
			NAMES vulkan/vulkan.h
			HINTS ${VULKANSDK_DIR}/Include
			)

# vulkan library directory
set(VULKANSDK_LIBS_DIR ${VULKANSDK_DIR}/Lib)

# vulkan core lib
find_library(VULKANSDK_LIBS
			NO_CMAKE_FIND_ROOT_PATH
			NAMES vulkan-1
			PATHS ${VULKANSDK_LIBS_DIR}		   
			)

mark_as_advanced(VULKANSDK_INCLUDE_DIRS)
mark_as_advanced(VULKANSDK_LIBS_DIR)
mark_as_advanced(VULKANSDK_LIBS)

# promote package for find
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(vulkansdk REQUIRED_VARS VULKANSDK_DIR VULKANSDK_INCLUDE_DIRS VULKANSDK_LIBS)