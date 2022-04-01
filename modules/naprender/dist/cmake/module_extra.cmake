include(${NAP_ROOT}/cmake/dist_shared_crossplatform.cmake)

if(NOT TARGET vulkansdk)
    find_package(vulkansdk REQUIRED)
endif()
add_include_to_interface_target(mod_naprender ${VULKANSDK_INCLUDE_DIRS})

if(NOT TARGET assimp)
    find_package(assimp REQUIRED)
endif()

# FreeImage
if(NOT TARGET FreeImage)
    find_package(FreeImage REQUIRED)
endif()
add_include_to_interface_target(mod_naprender ${FREEIMAGE_INCLUDE_DIRS})

# Add Vulkan library
set(MODULE_NAME_EXTRA_LIBS vulkansdk)

if(UNIX)
    # Package assimp into packaged project on *nix
    install(DIRECTORY ${THIRDPARTY_DIR}/assimp/lib/
            DESTINATION lib
            PATTERN "cmake" EXCLUDE
            PATTERN "pkgconfig" EXCLUDE)    

    # Package SDL2 into packaged project on *nix
    install(DIRECTORY ${THIRDPARTY_DIR}/SDL2/lib/
            DESTINATION lib
            PATTERN "cmake" EXCLUDE
            PATTERN "pkgconfig" EXCLUDE
            PATTERN "*.a" EXCLUDE)    

    # Package FreeImage into packaged project on *nix
    install(DIRECTORY ${THIRDPARTY_DIR}/FreeImage/lib/
            DESTINATION lib
            PATTERN "cmake" EXCLUDE
            PATTERN "pkgconfig" EXCLUDE
            PATTERN "*.a" EXCLUDE)

    # Package VulkanSDK into packaged project on Linux
    # We don't include the layers, since they are disabled for release
    install(DIRECTORY ${THIRDPARTY_DIR}/vulkansdk/lib/
            DESTINATION lib
            PATTERN "cmake" EXCLUDE
            PATTERN "pkgconfig" EXCLUDE
            PATTERN "*.a" EXCLUDE
            PATTERN "libVkLayer*" EXCLUDE)
endif()

# Package MoltenVK ICD file for packaged app on macOS
if(APPLE)
    install(FILES ${NAP_ROOT}/modules/mod_naprender/macos/MoltenVK_icd.json DESTINATION lib)
endif()

# Install thirdparty licenses into lib
install(DIRECTORY ${THIRDPARTY_DIR}/FreeImage/license/ DESTINATION licenses/FreeImage)
install(FILES ${THIRDPARTY_DIR}/assimp/LICENSE DESTINATION licenses/assimp)
install(FILES ${THIRDPARTY_DIR}/SDL2/COPYING.txt DESTINATION licenses/SDL2)
install(FILES ${THIRDPARTY_DIR}/vulkansdk/LICENSE.txt DESTINATION licenses/vulkansdk)

# Install data directory
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/data DESTINATION modules/mod_naprender)
