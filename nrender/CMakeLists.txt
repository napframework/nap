cmake_minimum_required(VERSION 3.2)
project(nrender)

if (WIN32)
    if (MSVC)
        set(CMAKE_LIBRARY_PATH "../../thirdparty/SDL2/msvc/lib/x64")
        set(CMAKE_PREFIX_PATH ../../thirdparty/SDL2/msvc)
    else()
        set(CMAKE_LIBRARY_PATH "../../thirdparty/SDL2/mingw/x86_64-w64-mingw32")
        set(CMAKE_PREFIX_PATH ../../thirdparty/SDL2/mingw/x86_64-w64-mingw32)
    endif()
endif()
find_package(SDL2 REQUIRED)

if (WIN32)
    if (MSVC)
        set(CMAKE_PREFIX_PATH ../../thirdparty/glew/msvc)
        set(CMAKE_LIBRARY_PATH "../../thirdparty/glew/msvc/lib/Release/x64")
    else ()
        set(CMAKE_PREFIX_PATH ../../thirdparty/glew/msvc)
        set(CMAKE_LIBRARY_PATH "../../thirdparty/glew/msvc/lib/Release/x64")
    endif()
elseif (APPLE)
    set(CMAKE_PREFIX_PATH ../../thirdparty/glew/osx)
    set(CMAKE_LIBRARY_PATH "../../thirdparty/glew/osx/lib")
endif()

if (WIN32)
    set(assimp_DIR "${CMAKE_CURRENT_LIST_DIR}/../../thirdparty/assimp/msvc64/install/lib/cmake/assimp-3.3")
elseif(APPLE)
    set(assimp_DIR "${CMAKE_CURRENT_LIST_DIR}/../../thirdparty/assimp/xcode/install/lib/cmake/assimp-3.3")
endif()

find_library(GLEW_LIBRARY NAMES GLEW glew32 glew glew32s PATH_SUFFIXES lib64)

find_package(GLEW REQUIRED)
find_package(OpenGL)
find_package(glm REQUIRED)
find_package(assimp REQUIRED)
if (NOT UNIX)
    find_package(freeimage REQUIRED)
endif ()

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

if (MSVC)
    list(APPEND LIBRARIES
            ${GLEW_LIBRARIES}
            ${FREEIMAGE_LIBRARIES}
            ${ASSIMP_LIBRARY_DIRS}/${ASSIMP_LIBRARIES}${CMAKE_STATIC_LIBRARY_SUFFIX}
            )
elseif(APPLE)
    list(APPEND LIBRARIES
            ${GLEW_LIBRARIES}
            ${FREEIMAGE_LIBRARIES}
            ${ASSIMP_LIBRARY_DIRS}/${CMAKE_SHARED_LIBRARY_PREFIX}${ASSIMP_LIBRARIES}${CMAKE_SHARED_LIBRARY_SUFFIX}
            )

elseif(UNIX)
    list(APPEND LIBRARIES
            freeimage
            GLEW
            assimp
            )
endif()

# add to include path and expose to depending projects
target_include_directories(${PROJECT_NAME} PUBLIC src ${INCLUDES})
target_link_libraries(${PROJECT_NAME} ${LIBRARIES} naputility)

if (MSVC)
    include(GenerateExportheader)
    GENERATE_EXPORT_HEADER(${PROJECT_NAME})
endif()

