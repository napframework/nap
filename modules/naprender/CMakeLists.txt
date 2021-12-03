cmake_minimum_required(VERSION 3.18.4)
# Exclude for Android
if(ANDROID)
    return()
endif()

project(mod_naprender)

# Add all cpp files to SOURCES
file(GLOB SOURCES src/*.cpp)
file(GLOB HEADERS src/*.h)
source_group("headers" FILES ${HEADERS})
source_group("sources" FILES ${SOURCES})

# Get our NAP modules dependencies from module.json
module_json_to_cmake()

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

# Compile shared lib as target
add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Modules)
# Remove lib prefix on Unix libraries
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

# Add includes
set(INCLUDES
    ${SDL2_INCLUDE_DIR}
    ${FREEIMAGE_INCLUDE_DIR}
    ${ASSIMP_INCLUDE_DIRS}
    ${VULKANSDK_INCLUDE_DIRS}
    ${SPIRVCROSS_INCLUDE_DIR}
    ${GLSLANG_INCLUDE_DIR}
    )
target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDES} src)

# Set compile definitions
target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY _USE_MATH_DEFINES)
target_compile_definitions(${PROJECT_NAME} PRIVATE MODULE_NAME=${PROJECT_NAME})
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
target_link_libraries(${PROJECT_NAME} ${LIBRARIES} ${DEPENDENT_NAP_MODULES})
target_link_libraries(${PROJECT_NAME} debug ${SPIRVCROSS_LIBS_DEBUG} optimized ${SPIRVCROSS_LIBS_RELEASE})
target_link_libraries(${PROJECT_NAME} debug "${GLSLANG_LIBS_DEBUG}" optimized "${GLSLANG_LIBS_RELEASE}")

# Deploy module.json as MODULENAME.json alongside module post-build
copy_module_json_to_bin()

# Windows thirdparty DLL copying
if(WIN32)
    # Copy freeimage DLL to build directory
    copy_freeimage_dll()

    # Copy over Windows graphics DLLs to build directory
    copy_base_windows_graphics_dlls()

    # Copy over Assimp to build directory
    file(GLOB ASSIMP_DLLS ${ASSIMP_ROOT_DIR}/bin/*.dll)
    copy_files_to_bin(${ASSIMP_DLLS})
endif()

# Package into platform release
if(APPLE)
    set(MACOS_EXTRA_RPATH_RELEASE ../../../../thirdparty/SDL2/lib)
    list(APPEND MACOS_EXTRA_RPATH_RELEASE ../../../../thirdparty/assimp/lib)
    list(APPEND MACOS_EXTRA_RPATH_RELEASE ../../../../thirdparty/FreeImage/lib)
    list(APPEND MACOS_EXTRA_RPATH_RELEASE ../../../../thirdparty/freetype/lib)
    list(APPEND MACOS_EXTRA_RPATH_RELEASE ../../../../thirdparty/vulkansdk/lib)
    set(MACOS_EXTRA_RPATH_DEBUG ${MACOS_EXTRA_RPATH_RELEASE})
elseif(UNIX)
    set(LINUX_EXTRA_RPATH ../../../../thirdparty/SDL2/lib)
    list(APPEND LINUX_EXTRA_RPATH ../../../../thirdparty/assimp/lib)
    list(APPEND LINUX_EXTRA_RPATH ../../../../thirdparty/FreeImage/lib)
    list(APPEND LINUX_EXTRA_RPATH ../../../../thirdparty/freetype/lib)
    list(APPEND LINUX_EXTRA_RPATH ../../../../thirdparty/vulkansdk/lib)
endif()
package_module()

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
install(DIRECTORY ${SDL2_INCLUDE_DIR} DESTINATION thirdparty/SDL2)
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
    #install(FILES dist/vulkan_macos/MoltenVK_icd.json DESTINATION modules/${PROJECT_NAME}/macos/)
elseif(UNIX)
    install(DIRECTORY ${VULKANSDK_DIR}/include DESTINATION thirdparty/vulkansdk)
    install(DIRECTORY ${VULKANSDK_DIR}/lib DESTINATION thirdparty/vulkansdk)
    install(DIRECTORY ${VULKANSDK_DIR}/share DESTINATION thirdparty/vulkansdk)
    install(DIRECTORY ${VULKANSDK_DIR}/etc DESTINATION thirdparty/vulkansdk)
    install(DIRECTORY ${VULKANSDK_DIR}/bin DESTINATION thirdparty/vulkansdk)
endif()
