set(assimp_parent ${NAP_ROOT}/system_modules/naprender/thirdparty/assimp)
set(assimp_DIR ${assimp_parent}/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH}/lib/cmake/assimp-3.3)
set(ASSIMP_LICENSE_FILES ${assimp_parent}/source/LICENSE)

function(copy_assimp_dll)
    if(NOT WIN32)
        return()
    endif()
    
    file(GLOB ASSIMP_DLL ${ASSIMP_ROOT_DIR}/bin/*.dll)
    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${ASSIMP_DLL} $<TARGET_FILE_DIR:${PROJECT_NAME}>/
        )
endfunction()

# Find SDL2
find_sdl2()

if(NAP_BUILD_CONTEXT MATCHES "source")
    # Find Assimp
    find_package(assimp REQUIRED)

    # Find other package dependencies
    find_package(freeimage REQUIRED)
    find_package(vulkansdk REQUIRED)
    find_package(SPIRVCross REQUIRED)
    find_package(glslang REQUIRED)

    # Add includes
    set(INCLUDES
        ${SDL2_INCLUDE_DIR}
        ${FREEIMAGE_INCLUDE_DIR}
        ${ASSIMP_INCLUDE_DIRS}
        ${VULKANSDK_INCLUDE_DIRS}
        ${SPIRVCROSS_INCLUDE_DIR}
        ${GLSLANG_INCLUDE_DIR}
        )
    target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDES})

    # Set compile definitions
    target_compile_definitions(${PROJECT_NAME} PRIVATE _USE_MATH_DEFINES)
    if(APPLE)
        target_compile_definitions(${PROJECT_NAME} PUBLIC VK_USE_PLATFORM_METAL_EXT=1)
    endif()

    # Add libraries
    set(LIBRARIES
        ${VULKANSDK_LIBS}
        ${SDL2_LIBRARY}
        ${FREEIMAGE_LIBRARIES}
        )

    # TODO Investigate why we're using a static lib for Win64 only
    if(MSVC)
        list(APPEND LIBRARIES
             ${ASSIMP_LIBRARY_DIRS}/${ASSIMP_LIBRARIES}${CMAKE_STATIC_LIBRARY_SUFFIX}
             )
    elseif(UNIX)
        list(APPEND LIBRARIES
             ${ASSIMP_LIBRARY_DIRS}/${CMAKE_SHARED_LIBRARY_PREFIX}${ASSIMP_LIBRARIES}${CMAKE_SHARED_LIBRARY_SUFFIX}
             )
    endif()

    if(UNIX AND NOT APPLE AND ${ARCH} STREQUAL "armhf")
        list(APPEND LIBRARIES atomic)
    endif()

    # Link libraries
    target_link_libraries(${PROJECT_NAME} ${LIBRARIES})
    target_link_libraries(${PROJECT_NAME} debug ${SPIRVCROSS_LIBS_DEBUG} optimized ${SPIRVCROSS_LIBS_RELEASE})
    target_link_libraries(${PROJECT_NAME} debug "${GLSLANG_LIBS_DEBUG}" optimized "${GLSLANG_LIBS_RELEASE}")

    set(thirdparty_module_dest system_modules/${PROJECT_NAME}/thirdparty)

    # Windows thirdparty DLL copying
    if(WIN32)
        # Copy freeimage DLL to build directory
        copy_freeimage_dll()

        # Copy SDL2 DLLs to build directory
        copy_files_to_bin(${SDL2_DIR}/msvc/x86_64/lib/SDL2.dll)

        # Copy over Assimp to build directory
        copy_assimp_dll()
    endif()

    if (WIN32)
        # Install for fbxconverter
        install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION tools/buildsystem
                CONFIGURATIONS Release)
    endif()

    # Package dependent libs into framework release --

    # Assimp
    install(DIRECTORY ${ASSIMP_ROOT_DIR} DESTINATION ${thirdparty_module_dest}/assimp/${NAP_THIRDPARTY_PLATFORM_DIR})
    install(FILES ${ASSIMP_LICENSE_FILES} DESTINATION ${thirdparty_module_dest}/assimp/source)

    # FreeImage
    install(DIRECTORY ${FREEIMAGE_DIR} DESTINATION ${thirdparty_module_dest}/FreeImage/${NAP_THIRDPARTY_PLATFORM_DIR})

    # SDL2
    install(DIRECTORY ${SDL2_DIR}/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH} DESTINATION ${thirdparty_module_dest}/SDL2/${NAP_THIRDPARTY_PLATFORM_DIR})
    install(FILES ${SDL2_LICENSE_FILES} DESTINATION ${thirdparty_module_dest}/SDL2)

    # Vulkan SDK
    install(DIRECTORY ${VULKANSDK_DIR}
            DESTINATION ${thirdparty_module_dest}/vulkansdk/${NAP_THIRDPARTY_PLATFORM_DIR}
            PATTERN "*.tar.gz" EXCLUDE)

    install(FILES ${VULKANSDK_LICENSE_FILES} DESTINATION ${thirdparty_module_dest}/vulkansdk)
    if(APPLE)
        install(FILES dist/vulkan_macos/MoltenVK_icd.json DESTINATION system_modules/${PROJECT_NAME}/macos/)
    endif()

    # glslang
    install(FILES ${GLSLANG_DIR}/../../source/LICENSE.txt DESTINATION ${thirdparty_module_dest}/glslang/source)

    # SPIRV-Cross
    install(FILES ${SPIRVCROSS_DIR}/../../source/LICENSE DESTINATION ${thirdparty_module_dest}/SPIRV-Cross/source)
else()
    if(NOT TARGET vulkansdk)
        find_package(vulkansdk REQUIRED)
    endif()
    add_include_to_interface_target(naprender ${VULKANSDK_INCLUDE_DIRS})

    if(NOT TARGET assimp)
        find_package(assimp REQUIRED)
    endif()

    # FreeImage
    if(NOT TARGET freeimage)
        find_package(freeimage REQUIRED)
    endif()
    add_include_to_interface_target(naprender ${FREEIMAGE_INCLUDE_DIR})

    # Add Vulkan library
    set(MODULE_EXTRA_LIBS ${VULKANSDK_LIBS})

    if(UNIX)
        # Package assimp into packaged project on *nix
        install(DIRECTORY ${ASSIMP_LIBRARY_DIRS}/
                DESTINATION lib
                PATTERN "cmake" EXCLUDE
                PATTERN "pkgconfig" EXCLUDE
                PATTERN "*.a" EXCLUDE)

        # Package SDL2 into packaged project on *nix
        install(DIRECTORY ${SDL2_LIBS_DIR}/
                DESTINATION lib
                PATTERN "cmake" EXCLUDE
                PATTERN "pkgconfig" EXCLUDE
                PATTERN "*.a" EXCLUDE)

        # Package FreeImage into packaged project on *nix
        install(DIRECTORY ${FREEIMAGE_LIBS_DIR}/
                DESTINATION lib
                PATTERN "cmake" EXCLUDE
                PATTERN "pkgconfig" EXCLUDE
                PATTERN "*.a" EXCLUDE)

        # Package VulkanSDK into packaged project on Linux
        # We don't include the layers, since they are disabled for release
        install(DIRECTORY ${VULKANSDK_LIBS_DIR}/
                DESTINATION lib
                PATTERN "cmake" EXCLUDE
                PATTERN "pkgconfig" EXCLUDE
                PATTERN "*.a" EXCLUDE
                PATTERN "libVkLayer*" EXCLUDE)
    else()
        copy_freeimage_dll()
        copy_assimp_dll()
    endif()

    # Package MoltenVK ICD file for packaged app on macOS
    if(APPLE)
        install(FILES ${NAP_ROOT}/system_modules/naprender/macos/MoltenVK_icd.json DESTINATION lib)
    endif()

    # Install thirdparty licenses into lib
    install(FILES ${FREEIMAGE_LICENSE_FILES} DESTINATION licenses/FreeImage)
    install(FILES ${ASSIMP_LICENSE_FILES} DESTINATION licenses/assimp)
    install(FILES ${SDL2_LICENSE_FILES} DESTINATION licenses/SDL2)
    install(FILES ${VULKANSDK_LICENSE_FILES} DESTINATION "licenses/Vulkan SDK")
    install(FILES ${NAP_ROOT}/system_modules/naprender/thirdparty/glslang/source/LICENSE.txt DESTINATION licenses/glslang/)
    install(FILES ${NAP_ROOT}/system_modules/naprender/thirdparty/SPIRV-Cross/source/LICENSE DESTINATION licenses/SPIRV-Cross/)
    install(FILES ${THIRDPARTY_DIR}/tclap/COPYING DESTINATION licenses/tclap/)

    # Install data directory
    install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/data DESTINATION system_modules/naprender)
endif()
