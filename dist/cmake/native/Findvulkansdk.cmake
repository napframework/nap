# default vulkansdk directory
find_path(VULKANSDK_DIR
          NO_CMAKE_FIND_ROOT_PATH
          NAMES Include/vulkan/vulkan.h
          HINTS ${THIRDPARTY_DIR}/vulkansdk
          )

set(VULKANSDK_LIBS_DIR ${VULKANSDK_DIR}/Lib)
set(VULKANSDK_LIBS ${VULKANSDK_LIBS_DIR}/vulkan-1.lib)
set(VULKANSDK_INCLUDE_DIRS ${VULKANSDK_DIR}/Include)
mark_as_advanced(VULKANSDK_LIBS_DIR)

# allows the target to refer to 'vulkansdk' to import libraries
add_library(vulkansdk SHARED IMPORTED)
set_target_properties(vulkansdk PROPERTIES
                          IMPORTED_IMPLIB_RELEASE ${VULKANSDK_LIBS}
                          IMPORTED_IMPLIB_DEBUG ${VULKANSDK_LIBS}
                          )

# promote package for find
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(vulkansdk REQUIRED_VARS VULKANSDK_DIR)