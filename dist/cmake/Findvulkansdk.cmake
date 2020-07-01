# default vulkansdk directory
find_path(VULKANSDK_DIR
          NO_CMAKE_FIND_ROOT_PATH
          NAMES Include/vulkan/vulkan.h
          HINTS ${THIRDPARTY_DIR}/vulkansdk
          )

set(VULKANSDK_LIBS_DIR ${VULKANSDK_DIR}/lib/msvc)
set(VULKANSDK_LIBS ${VULKANSDK_LIBS_DIR}/msvc/vulkan-1.lib)
set(VULKANSDK_INCLUDE_DIRS ${VULKANSDK_DIR}/msvc/Include)

mark_as_advanced(VULKANSDK_INCLUDE_DIRS)
mark_as_advanced(VULKANSDK_LIBS_DIR)
mark_as_advanced(VULKANSDK_LIBS)

# promote package for find
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(vulkansdk REQUIRED_VARS VULKANSDK_DIR)