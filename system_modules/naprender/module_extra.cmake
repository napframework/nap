if(NAP_BUILD_CONTEXT MATCHES "source")
    # Find SDL2
    # Because we use the FindSDL2 from dist, we augment it with some extra platform specifc vars
    set(SDL2_DIR ${THIRDPARTY_DIR}/SDL2)
    set(SDL2_DIST_FILES ${SDL2_DIR}/COPYING.txt)
    if(WIN32)
        set(SDL2_LIBS_DIR ${SDL2_DIR}/msvc/x86_64/lib)
        set(CMAKE_LIBRARY_PATH ${SDL2_LIBS_DIR})
        set(CMAKE_PREFIX_PATH ${SDL2_DIR}/msvc/x86_64)
    elseif(APPLE)
        set(SDL2_LIBS_DIR ${SDL2_DIR}/macos/x86_64/lib)
        set(CMAKE_LIBRARY_PATH ${SDL2_LIBS_DIR})
        set(CMAKE_PREFIX_PATH ${SDL2_DIR}/macos/x86_64)
    elseif(UNIX)
        set(SDL2_LIBS_DIR ${SDL2_DIR}/linux/${ARCH}/lib)
        set(CMAKE_LIBRARY_PATH ${SDL2_LIBS_DIR})
        set(CMAKE_PREFIX_PATH ${SDL2_DIR}/linux/${ARCH})    
    endif()
    find_package(SDL2 REQUIRED)

    # Find Assimp
    set(ASSIMP_DIST_FILES ${THIRDPARTY_DIR}/assimp/source/LICENSE)
    if(WIN32)
        set(assimp_DIR ${THIRDPARTY_DIR}/assimp/msvc/x86_64/lib/cmake/assimp-3.3)
    elseif(APPLE)
        set(assimp_DIR ${THIRDPARTY_DIR}/assimp/macos/x86_64/lib/cmake/assimp-3.3)      
    elseif(UNIX)
        set(assimp_DIR ${THIRDPARTY_DIR}/assimp/linux/${ARCH}/lib/cmake/assimp-3.3)   
    endif()
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

    # Windows thirdparty DLL copying
    if(WIN32)
        # Copy freeimage DLL to build directory
        copy_freeimage_dll()

        # Copy SDL2 DLLs to build directory
        copy_files_to_bin(${THIRDPARTY_DIR}/sdl2/msvc/x86_64/lib/SDL2.dll)

        # Copy over Assimp to build directory
        file(GLOB ASSIMP_DLLS ${ASSIMP_ROOT_DIR}/bin/*.dll)
        copy_files_to_bin(${ASSIMP_DLLS})
    endif()

    if (WIN32)
        # Install for fbxconverter
        install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION tools/platform
                CONFIGURATIONS Release)
    endif()

    # Package dependent libs into platform release --

    # Assimp
    install(FILES ${ASSIMP_DIST_FILES} DESTINATION thirdparty/assimp)
    install(DIRECTORY ${ASSIMP_ROOT_DIR}/ DESTINATION thirdparty/assimp)

    # FreeImage
    install(FILES ${FREEIMAGE_DIST_FILES} DESTINATION thirdparty/FreeImage/license)
    install(FILES ${FREEIMAGE_INCLUDE_DIR}/FreeImage.h DESTINATION thirdparty/FreeImage/include)
    if (WIN32)
        install(FILES ${FREEIMAGE_LIBS_DIR}/FreeImage.lib DESTINATION thirdparty/FreeImage/lib)
        install(FILES ${FREEIMAGE_LIBS_DIR}/FreeImage.dll DESTINATION thirdparty/FreeImage/bin)
    elseif(APPLE)
        file(GLOB FREEIMAGE_DYLIBS ${FREEIMAGE_LIBS_DIR}/*.dylib*)
        install(FILES ${FREEIMAGE_DYLIBS} DESTINATION thirdparty/FreeImage/lib)
    elseif(UNIX)
        file(GLOB FREEIMAGE_DYLIBS ${FREEIMAGE_LIBS_DIR}/*.so*)
        install(FILES ${FREEIMAGE_DYLIBS} DESTINATION thirdparty/FreeImage/lib)
    endif()

    # SDL2
    install(DIRECTORY ${SDL2_INCLUDE_DIR} DESTINATION thirdparty/SDL2/include)
    install(FILES ${SDL2_DIST_FILES} DESTINATION thirdparty/SDL2)
    if (WIN32)
        install(DIRECTORY ${SDL2_LIBS_DIR}/ DESTINATION thirdparty/SDL2/lib)
    elseif(APPLE)
        file(GLOB SDL_DYLIBS ${SDL2_LIBS_DIR}/*.dylib)
        install(FILES ${SDL_DYLIBS} DESTINATION thirdparty/SDL2/lib)    
    elseif(UNIX)
        file(GLOB SDL_DYLIBS ${SDL2_LIBS_DIR}/*.so*)
        install(FILES ${SDL_DYLIBS} ${SDL2_LIBS_DIR}/libSDL2main.a 
            DESTINATION thirdparty/SDL2/lib)
    endif()

    # Vulkan SDK
    install(FILES ${VULKANSDK_ROOT_DIR}/LICENSE.txt DESTINATION thirdparty/vulkansdk)
    if (WIN32)
        install(DIRECTORY ${VULKANSDK_DIR}/include DESTINATION thirdparty/vulkansdk)
        install(DIRECTORY ${VULKANSDK_DIR}/lib DESTINATION thirdparty/vulkansdk)
        install(DIRECTORY ${VULKANSDK_DIR}/bin DESTINATION thirdparty/vulkansdk)
        install(DIRECTORY ${VULKANSDK_DIR}/share DESTINATION thirdparty/vulkansdk)
        install(DIRECTORY ${VULKANSDK_DIR}/config DESTINATION thirdparty/vulkansdk)
    elseif(APPLE)
        install(DIRECTORY ${VULKANSDK_DIR}/include DESTINATION thirdparty/vulkansdk)
        install(DIRECTORY ${VULKANSDK_DIR}/lib DESTINATION thirdparty/vulkansdk)
        install(DIRECTORY ${VULKANSDK_DIR}/share DESTINATION thirdparty/vulkansdk)
        install(DIRECTORY ${VULKANSDK_DIR}/bin DESTINATION thirdparty/vulkansdk)
        install(FILES dist/vulkan_macos/MoltenVK_icd.json DESTINATION modules/${PROJECT_NAME}/macos/)
    elseif(UNIX)
        install(DIRECTORY ${VULKANSDK_DIR}/include DESTINATION thirdparty/vulkansdk)
        install(DIRECTORY ${VULKANSDK_DIR}/lib DESTINATION thirdparty/vulkansdk)
        install(DIRECTORY ${VULKANSDK_DIR}/share DESTINATION thirdparty/vulkansdk)
        install(DIRECTORY ${VULKANSDK_DIR}/etc DESTINATION thirdparty/vulkansdk)
        install(DIRECTORY ${VULKANSDK_DIR}/bin DESTINATION thirdparty/vulkansdk)
    endif()
else()
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
endif()
