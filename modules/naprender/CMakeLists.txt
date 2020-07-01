# Exclude for Android
if(ANDROID)
    return()
endif()

project(mod_naprender)

# add all cpp files to SOURCES
file(GLOB SOURCES src/*.cpp)
file(GLOB HEADERS src/*.h)
source_group("headers" FILES ${HEADERS})
source_group("sources" FILES ${SOURCES})

# Get our NAP modules dependencies from module.json
module_json_to_cmake()

if(WIN32)
    if(MSVC)
        set(CMAKE_LIBRARY_PATH ${THIRDPARTY_DIR}/SDL2/msvc/lib/x64)
        set(CMAKE_PREFIX_PATH ${THIRDPARTY_DIR}/SDL2/msvc)
    else()
        set(CMAKE_LIBRARY_PATH ${THIRDPARTY_DIR}/SDL2/mingw/x86_64-w64-mingw32)
        set(CMAKE_PREFIX_PATH ${THIRDPARTY_DIR}/SDL2/mingw/x86_64-w64-mingw32)
    endif()
elseif(APPLE)
    set(CMAKE_LIBRARY_PATH ${THIRDPARTY_DIR}/SDL2/osx/install/lib)
    set(CMAKE_PREFIX_PATH ${THIRDPARTY_DIR}/SDL2/osx/install)
elseif(UNIX)
    set(CMAKE_LIBRARY_PATH ${THIRDPARTY_DIR}/SDL2/linux/install/lib)
    set(CMAKE_PREFIX_PATH ${THIRDPARTY_DIR}/SDL2/linux/install)    
endif()
find_package(SDL2 REQUIRED)

if(WIN32)
    set(assimp_DIR ${THIRDPARTY_DIR}/assimp/msvc64/install/lib/cmake/assimp-3.3)
elseif(APPLE)
    set(assimp_DIR ${THIRDPARTY_DIR}/assimp/osx/install/lib/cmake/assimp-3.3)      
elseif(UNIX)
    set(assimp_DIR ${THIRDPARTY_DIR}/assimp/linux/install/lib/cmake/assimp-3.3)   
endif()
find_package(assimp REQUIRED)

find_package(glm REQUIRED)
find_package(freeimage REQUIRED)
find_package(vulkansdk REQUIRED)
find_package(SPIRVCross REQUIRED)
find_package(glslang REQUIRED)

# LIBRARY

set(INCLUDES
    ${SDL2_INCLUDE_DIR}
    ${GLM_INCLUDE_DIRS}
    ${FREEIMAGE_INCLUDE_DIRS}
    ${ASSIMP_INCLUDE_DIRS}
    ${VULKANSDK_INCLUDE_DIRS}
	${SPIRVCROSS_INCLUDE_DIR}
	${GLSLANG_INCLUDE_DIR}
    )

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

# compile shared lib as target
add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Modules)

# add include dirs
target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDES} src)

# link with external libs
target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY _USE_MATH_DEFINES)
target_compile_definitions(${PROJECT_NAME} PRIVATE MODULE_NAME=${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} ${LIBRARIES} ${DEPENDENT_NAP_MODULES})
target_link_libraries(${PROJECT_NAME} debug ${SPIRVCROSS_LIBS_DEBUG} optimized ${SPIRVCROSS_LIBS_RELEASE})
target_link_libraries(${PROJECT_NAME} debug "${GLSLANG_LIBS_DEBUG}" optimized "${GLSLANG_LIBS_RELEASE}")

# Deploy module.json as MODULENAME.json alongside module post-build
copy_module_json_to_bin()

# Windows thirdparty DLL copying
if(WIN32)
    # Copy freeimage DLL to build directory
    find_package(freeimage REQUIRED)
    copy_freeimage_dll()

    # Copy over Windows graphics DLLs to build directory
    copy_base_windows_graphics_dlls()

    # Copy over Assimp to build directory
    copy_files_to_bin(${THIRDPARTY_DIR}/assimp/msvc64/install/bin/assimp-vc140-mt.dll)
endif()

# Package into platform release
if(APPLE)
    set(MACOS_EXTRA_RPATH_RELEASE ../../../../thirdparty/SDL2/lib)
    list(APPEND MACOS_EXTRA_RPATH_RELEASE ../../../../thirdparty/assimp/lib)
    list(APPEND MACOS_EXTRA_RPATH_RELEASE ../../../../thirdparty/FreeImage/lib)
    list(APPEND MACOS_EXTRA_RPATH_RELEASE ../../../../thirdparty/freetype/lib)
    set(MACOS_EXTRA_RPATH_DEBUG ${MACOS_EXTRA_RPATH_RELEASE})
elseif(UNIX)
    set(LINUX_EXTRA_RPATH ../../../../thirdparty/SDL2/lib)
    list(APPEND LINUX_EXTRA_RPATH ../../../../thirdparty/assimp/lib)
    list(APPEND LINUX_EXTRA_RPATH ../../../../thirdparty/FreeImage/lib)
    list(APPEND LINUX_EXTRA_RPATH ../../../../thirdparty/freetype/lib)
endif()
package_module()

if (WIN32)
    # Install for fbxconverter
    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION tools/platform
            CONFIGURATIONS Release)
endif()

# Package dependent libs into platform release --

# Assimp
install(FILES ${THIRDPARTY_DIR}/assimp/LICENSE DESTINATION thirdparty/assimp)
if (WIN32)
    install(DIRECTORY ${THIRDPARTY_DIR}/assimp/msvc64/install/ DESTINATION thirdparty/assimp)
elseif(APPLE)
    install(DIRECTORY ${THIRDPARTY_DIR}/assimp/osx/install/ DESTINATION thirdparty/assimp)
elseif(UNIX)
    install(DIRECTORY ${THIRDPARTY_DIR}/assimp/linux/install/ 
            DESTINATION thirdparty/assimp
            PATTERN "*.a" EXCLUDE)   
endif()

# FreeImage
if (WIN32)
    file(GLOB FREEIMAGE_LICENSE_FILES ${THIRDPARTY_DIR}/FreeImage/msvc/license*.txt)
    install(FILES ${FREEIMAGE_LICENSE_FILES} DESTINATION thirdparty/FreeImage/license)
    install(FILES ${THIRDPARTY_DIR}/FreeImage/msvc/Dist/x64/FreeImage.h DESTINATION thirdparty/FreeImage/include)
    install(FILES ${THIRDPARTY_DIR}/FreeImage/msvc/Dist/x64/FreeImage.lib DESTINATION thirdparty/FreeImage/lib)
    install(FILES ${THIRDPARTY_DIR}/FreeImage/msvc/Dist/x64/FreeImage.dll DESTINATION thirdparty/FreeImage/bin)
elseif(APPLE)
    install(FILES ${THIRDPARTY_DIR}/FreeImage/osx/include/FreeImage.h DESTINATION thirdparty/FreeImage/include)
    install(FILES ${THIRDPARTY_DIR}/FreeImage/osx/lib/libfreeimage-3.17.0.dylib DESTINATION thirdparty/FreeImage/lib)
    install(DIRECTORY ${THIRDPARTY_DIR}/FreeImage/osx/license/
            DESTINATION thirdparty/FreeImage/license
            )
elseif(UNIX)
    file(GLOB FREEIMAGE_LICENSE_FILES ${THIRDPARTY_DIR}/FreeImage/linux/install/license*.txt)
    install(FILES ${FREEIMAGE_LICENSE_FILES} DESTINATION thirdparty/FreeImage/license)
    install(FILES ${THIRDPARTY_DIR}/FreeImage/linux/install/include/FreeImage.h DESTINATION thirdparty/FreeImage/include)
    file(GLOB FREEIMAGE_DYLIBS ${THIRDPARTY_DIR}/FreeImage/linux/install/lib/*.so*)
    install(FILES ${FREEIMAGE_DYLIBS} DESTINATION thirdparty/FreeImage/lib)
endif()

# SDL2
install(FILES ${THIRDPARTY_DIR}/SDL2/msvc/COPYING.txt
        DESTINATION thirdparty/SDL2)
if (WIN32)
    install(DIRECTORY ${THIRDPARTY_DIR}/SDL2/msvc/ DESTINATION thirdparty/SDL2
            PATTERN "lib" EXCLUDE)
    install(DIRECTORY ${THIRDPARTY_DIR}/SDL2/msvc/lib/x64/ DESTINATION thirdparty/SDL2/lib)
elseif(APPLE)
    install(DIRECTORY ${THIRDPARTY_DIR}/SDL2/osx/install/include DESTINATION thirdparty/SDL2)
    file(GLOB SDL_DYLIBS ${THIRDPARTY_DIR}/SDL2/osx/install/lib/*.dylib)
    install(FILES ${SDL_DYLIBS} DESTINATION thirdparty/SDL2/lib)    
elseif(UNIX)
    install(DIRECTORY ${THIRDPARTY_DIR}/SDL2/linux/install/include DESTINATION thirdparty/SDL2)
    file(GLOB SDL_DYLIBS ${THIRDPARTY_DIR}/SDL2/linux/install/lib/*.so*)
    install(FILES ${SDL_DYLIBS} ${THIRDPARTY_DIR}/SDL2/linux/install/lib/libSDL2main.a
            DESTINATION thirdparty/SDL2/lib)
endif()

#vulkansdk
install(FILES ${THIRDPARTY_DIR}/vulkansdk/LICENSE.txt DESTINATION thirdparty/vulkansdk/LICENSE.txt)
if (WIN32)
    install(DIRECTORY ${THIRDPARTY_DIR}/vulkansdk/msvc/INCLUDE DESTINATION thirdparty/vulkansdk)
    install(DIRECTORY ${THIRDPARTY_DIR}/vulkansdk/msvc/LIB DESTINATION thirdparty/vulkansdk)
endif()