# default vulkansdk directory
find_path(VULKANSDK_DIR
          NO_DEFAULT_PATH
          NAMES Include/vulkan/vulkan.h
          HINTS ${THIRDPARTY_DIR}/vulkansdk
          )

# include directory
find_path(VULKANSDK_INCLUDE_DIRS
			NO_DEFAULT_PATH
			NAMES vulkan/vulkan.h
			HINTS ${VULKANSDK_DIR}/Include
			)

# vulkan library directory
set(VULKANSDK_LIBS_DIR ${VULKANSDK_DIR}/Lib)

# vulkan core lib
find_library(VULKANSDK_LIBS
			NO_DEFAULT_PATH
			NAMES vulkan-1
			PATHS ${VULKANSDK_LIBS_DIR}		   
			)

# hide from gui
mark_as_advanced(VULKANSDK_INCLUDE_DIRS)
mark_as_advanced(VULKANSDK_LIBS_DIR)
mark_as_advanced(VULKANSDK_LIBS)

# allows the target to refer to 'vulkansdk' to import libraries
add_library(vulkansdk SHARED IMPORTED)
set_target_properties(vulkansdk PROPERTIES
                          IMPORTED_IMPLIB_RELEASE ${VULKANSDK_LIBS}
                          IMPORTED_IMPLIB_DEBUG ${VULKANSDK_LIBS}
                          )

# promote package for find
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(vulkansdk REQUIRED_VARS VULKANSDK_DIR VULKANSDK_LIBS)