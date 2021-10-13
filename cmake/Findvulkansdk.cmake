# Find Vulkan directory
if(WIN32)
    find_path(VULKANSDK_DIR
        NO_DEFAULT_PATH
        NAMES include/vulkan/vulkan.h
        HINTS ${THIRDPARTY_DIR}/vulkansdk/msvc/x86_64
        )
elseif(APPLE)
    find_path(VULKANSDK_DIR
        NO_DEFAULT_PATH
        NAMES include/vulkan/vulkan.h
        HINTS ${THIRDPARTY_DIR}/vulkansdk/macos/x86_64
        )
elseif(UNIX)
    find_path(VULKANSDK_DIR
        NO_DEFAULT_PATH
        NAMES include/vulkan/vulkan.h
        HINTS ${THIRDPARTY_DIR}/vulkansdk/linux/${ARCH}
        )
endif()

# Include directory
find_path(VULKANSDK_INCLUDE_DIRS
            NO_DEFAULT_PATH
            NAMES vulkan/vulkan.h
            HINTS ${VULKANSDK_DIR}/include
            )

# Vulkan library directory
set(VULKANSDK_LIBS_DIR ${VULKANSDK_DIR}/lib)

# Find Vulkan library
if(WIN32)
    # Vulkan core lib -> vulkan-1.lib
    find_library(VULKANSDK_LIBS
                NO_DEFAULT_PATH
                NAMES vulkan-1
                PATHS ${VULKANSDK_LIBS_DIR}           
                )
elseif(APPLE)
    # Moltenvk -> libvulkan.so
    find_library(VULKAN_LIB
            NO_DEFAULT_PATH
            NAMES vulkan
            PATHS ${VULKANSDK_LIBS_DIR}           
            )

    # Additional library dependencies on apple
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
    # Vulkan core lib -> libvulkan.so
    find_library(VULKANSDK_LIBS
            NO_DEFAULT_PATH
            NAMES vulkan
            PATHS ${VULKANSDK_LIBS_DIR}           
            )
endif()

mark_as_advanced(VULKANSDK_INCLUDE_DIRS)
mark_as_advanced(VULKANSDK_LIBS_DIR)
mark_as_advanced(VULKANSDK_LIBS)

# Promote package for find
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(vulkansdk REQUIRED_VARS VULKANSDK_DIR VULKANSDK_INCLUDE_DIRS VULKANSDK_LIBS)
