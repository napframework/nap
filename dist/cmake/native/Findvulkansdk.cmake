# default vulkansdk directory
find_path(VULKANSDK_DIR
          NO_DEFAULT_PATH
          NAMES include/vulkan/vulkan.h
          HINTS ${THIRDPARTY_DIR}/vulkansdk
          )

# include directory
find_path(VULKANSDK_INCLUDE_DIRS
			NO_DEFAULT_PATH
			NAMES vulkan/vulkan.h
			HINTS ${VULKANSDK_DIR}/include
			)

# vulkan library directory
set(VULKANSDK_LIBS_DIR ${VULKANSDK_DIR}/lib)

# find vulkan library
if(WIN32)
	# vulkan core lib -> vulkan-1.lib
	find_library(VULKANSDK_LIBS
				NO_DEFAULT_PATH
				NAMES vulkan-1
				PATHS ${VULKANSDK_LIBS_DIR}		   
				)
elseif(APPLE)
	# moltenvk -> libMoltenVK.a
	find_library(VULKAN_LIB
			NO_DEFAULT_PATH
			NAMES vulkan
			PATHS ${VULKANSDK_LIBS_DIR}		   
			)

	find_library(METAL_LIB Metal)
	find_library(FOUNDATION_LIB Foundation)
	find_library(QUARTZ_LIB QuartzCore)
	find_library(IOKIT_LIB IOKit)
	find_library(IOSURFACE_LIB IOSurface)

	if(VULKAN_LIB AND METAL_LIB AND FOUNDATION_LIB
		AND QUARTZ_LIB AND IOKIT_LIB AND IOSURFACE_LIB)
		set(VULKANSDK_LIBS ${VULKAN_LIB} ${METAL_LIB} ${FOUNDATION_LIB} ${QUARTZ_LIB} ${IOKIT_LIB} ${IOSURFACE_LIB})
	endif()

elseif(UNIX)
	# vulkan core lib -> libvulkan.so
	find_library(VULKANSDK_LIBS
			NO_DEFAULT_PATH
			NAMES vulkan
			PATHS ${VULKANSDK_LIBS_DIR}		   
			)
endif()

# hide from gui
mark_as_advanced(VULKANSDK_INCLUDE_DIRS)
mark_as_advanced(VULKANSDK_LIBS_DIR)
mark_as_advanced(VULKANSDK_LIBS)

# allows the target to refer to 'vulkansdk' as library target
add_library(vulkansdk SHARED IMPORTED)

# Setup library properties for linux
if(UNIX)
	set_target_properties(vulkansdk PROPERTIES
    		IMPORTED_CONFIGURATIONS "Debug;Release"
        	IMPORTED_LOCATION_RELEASE ${VULKANSDK_LIBS}
     		IMPORTED_LOCATION_DEBUG ${VULKANSDK_LIBS}
        	)
elseif(WIN32)
	set_target_properties(vulkansdk PROPERTIES
    		IMPORTED_IMPLIB_RELEASE ${VULKANSDK_LIBS}
      		IMPORTED_IMPLIB_DEBUG ${VULKANSDK_LIBS}
          	)
endif()

# promote package for find
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(vulkansdk REQUIRED_VARS VULKANSDK_DIR VULKANSDK_LIBS)