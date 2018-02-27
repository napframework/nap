cmake_minimum_required(VERSION 3.5)
project(nrender)

if(WIN32)
    if(MSVC)
        set(CMAKE_LIBRARY_PATH "${THIRDPARTY_DIR}/SDL2/msvc/lib/x64")
        set(CMAKE_PREFIX_PATH ${THIRDPARTY_DIR}/SDL2/msvc)
    else()
        set(CMAKE_LIBRARY_PATH "${THIRDPARTY_DIR}/SDL2/mingw/x86_64-w64-mingw32")
        set(CMAKE_PREFIX_PATH ${THIRDPARTY_DIR}/SDL2/mingw/x86_64-w64-mingw32)
    endif()
elseif(APPLE)
    set(CMAKE_LIBRARY_PATH "${THIRDPARTY_DIR}/SDL2/osx/install/lib")
    set(CMAKE_PREFIX_PATH ${THIRDPARTY_DIR}/SDL2/osx/install)
elseif(UNIX)
    set(CMAKE_LIBRARY_PATH "${THIRDPARTY_DIR}/SDL2/linux/install/lib")
    set(CMAKE_PREFIX_PATH ${THIRDPARTY_DIR}/SDL2/linux/install)    
endif()
find_package(SDL2 REQUIRED)

if(WIN32)
    if(MSVC)
        set(CMAKE_PREFIX_PATH ${THIRDPARTY_DIR}/glew/msvc)
        set(CMAKE_LIBRARY_PATH "${THIRDPARTY_DIR}/glew/msvc/lib/Release/x64")
    else()
        set(CMAKE_PREFIX_PATH ${THIRDPARTY_DIR}/glew/msvc)
        set(CMAKE_LIBRARY_PATH "${THIRDPARTY_DIR}/glew/msvc/lib/Release/x64")
    endif()
elseif(APPLE)
    set(CMAKE_LIBRARY_PATH "${THIRDPARTY_DIR}/glew/osx/install/lib")
    set(CMAKE_PREFIX_PATH ${THIRDPARTY_DIR}/glew/osx/install)
elseif(UNIX)
    set(CMAKE_LIBRARY_PATH "${THIRDPARTY_DIR}/glew/linux/install/lib")
    set(CMAKE_PREFIX_PATH ${THIRDPARTY_DIR}/glew/linux/install)
endif()
find_library(GLEW_LIBRARY NAMES GLEW glew32 glew glew32s PATH_SUFFIXES lib64)
find_package(GLEW REQUIRED)

if(WIN32)
    set(assimp_DIR "${THIRDPARTY_DIR}/assimp/msvc64/install/lib/cmake/assimp-3.3")
elseif(APPLE)
    set(assimp_DIR "${THIRDPARTY_DIR}/assimp/osx/install/lib/cmake/assimp-3.3")      
elseif(UNIX)
    set(assimp_DIR "${THIRDPARTY_DIR}/assimp/linux/install/lib/cmake/assimp-3.3")   
endif()
find_package(assimp REQUIRED)

find_package(OpenGL)
find_package(glm REQUIRED)
find_package(freeimage REQUIRED)

file(GLOB_RECURSE SOURCES src/*.cpp src/*.h)

# compile shared lib as target
add_library(${PROJECT_NAME} STATIC ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Core)

set(INCLUDES
    ${SDL2_INCLUDE_DIR}
    ${GLM_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIR}
    ${FREEIMAGE_INCLUDE_DIRS}
    ${ASSIMP_INCLUDE_DIRS}
    )

set(LIBRARIES
    ${SDL2_LIBRARY}
    ${OPENGL_gl_LIBRARY}
    )

list(APPEND LIBRARIES
     ${GLEW_LIBRARIES}
     ${FREEIMAGE_LIBRARIES}
     )

if(MSVC)
    list(APPEND LIBRARIES
         ${ASSIMP_LIBRARY_DIRS}/${ASSIMP_LIBRARIES}${CMAKE_STATIC_LIBRARY_SUFFIX}
         )
elseif(UNIX)
    list(APPEND LIBRARIES
         ${ASSIMP_LIBRARY_DIRS}/${CMAKE_SHARED_LIBRARY_PREFIX}${ASSIMP_LIBRARIES}${CMAKE_SHARED_LIBRARY_SUFFIX}
         )
endif()

# add to include path and expose to depending projects
target_compile_definitions(${PROJECT_NAME} PRIVATE MODULE_NAME=${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PUBLIC src ${INCLUDES})
target_link_libraries(${PROJECT_NAME} ${LIBRARIES} naputility naprtti)

if(MSVC)
    include(GenerateExportheader)
    generate_export_header(${PROJECT_NAME})
endif()

if(WIN32)
    copy_freeimage_dll()
    copy_base_windows_graphics_dlls()
endif()

# Package into release build
if (WIN32)
    install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib/$<CONFIG>
                                    ARCHIVE DESTINATION lib/$<CONFIG>)
    # TODO test and remove our archive destination?
elseif(APPLE)
    install(TARGETS ${PROJECT_NAME} ARCHIVE DESTINATION lib/$<CONFIG>)  
else()
    install(TARGETS ${PROJECT_NAME} ARCHIVE DESTINATION lib/${CMAKE_BUILD_TYPE})   
endif()

install(DIRECTORY "src/" DESTINATION "include/nrender/"
        FILES_MATCHING PATTERN "*.h")

# Package dependent includes 
# TODO discuss moving elsewhere, the idea here was that the object using the dependencies installs them

# Assimp
install(FILES ${THIRDPARTY_DIR}/assimp/LICENSE DESTINATION thirdparty/assimp)
if (WIN32)
    install(DIRECTORY ${THIRDPARTY_DIR}/assimp/msvc64/install/ DESTINATION thirdparty/assimp)
elseif(APPLE)
    install(DIRECTORY ${THIRDPARTY_DIR}/assimp/osx/install/ DESTINATION thirdparty/assimp)
elseif(UNIX)
    install(DIRECTORY ${THIRDPARTY_DIR}/assimp/linux/install/ DESTINATION thirdparty/assimp)   
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

# GLM
install(FILES "${THIRDPARTY_DIR}/glm/copying.txt" DESTINATION "thirdparty/glm")
install(DIRECTORY "${THIRDPARTY_DIR}/glm/glm/" DESTINATION "thirdparty/glm")

# GLEW
if (WIN32)
    install(DIRECTORY "${THIRDPARTY_DIR}/glew/msvc/" DESTINATION "thirdparty/glew"
            PATTERN "lib" EXCLUDE
            PATTERN "bin" EXCLUDE)
    install(DIRECTORY "${THIRDPARTY_DIR}/glew/msvc/bin/Release/x64/" DESTINATION "thirdparty/glew/bin")
    install(FILES ${THIRDPARTY_DIR}/glew/msvc/lib/Release/x64/glew32.lib DESTINATION "thirdparty/glew/lib/")
elseif(APPLE)
    install(DIRECTORY "${THIRDPARTY_DIR}/glew/osx/install/" DESTINATION "thirdparty/glew"
            PATTERN "lib" EXCLUDE)
    install(FILES "${THIRDPARTY_DIR}/glew/osx/install/lib/libGLEW.dylib" DESTINATION "thirdparty/glew/lib")
elseif(UNIX)
    install(DIRECTORY "${THIRDPARTY_DIR}/glew/linux/install/" DESTINATION "thirdparty/glew"
            PATTERN "lib" EXCLUDE)
    file(GLOB GLEW_DYLIBS ${THIRDPARTY_DIR}/glew/linux/install/lib/*.so*)
    install(FILES ${GLEW_DYLIBS} DESTINATION thirdparty/glew/lib)
endif()

# SDL2
install(FILES ${THIRDPARTY_DIR}/SDL2/msvc/COPYING.txt
        DESTINATION thirdparty/SDL2)
if (WIN32)
    install(DIRECTORY "${THIRDPARTY_DIR}/SDL2/msvc/" DESTINATION "thirdparty/SDL2"
            PATTERN "lib" EXCLUDE)
    install(DIRECTORY "${THIRDPARTY_DIR}/SDL2/msvc/lib/x64/" DESTINATION "thirdparty/SDL2/lib")
elseif(APPLE)
    install(DIRECTORY "${THIRDPARTY_DIR}/SDL2/osx/install/include" DESTINATION "thirdparty/SDL2")
    file(GLOB SDL_DYLIBS ${THIRDPARTY_DIR}/SDL2/osx/install/lib/*.dylib)
    install(FILES ${SDL_DYLIBS} DESTINATION thirdparty/SDL2/lib)    
elseif(UNIX)
    install(DIRECTORY "${THIRDPARTY_DIR}/SDL2/linux/install/include" DESTINATION "thirdparty/SDL2")
    file(GLOB SDL_DYLIBS ${THIRDPARTY_DIR}/SDL2/linux/install/lib/*.so*)
    install(FILES ${SDL_DYLIBS} DESTINATION thirdparty/SDL2/lib)    
endif()