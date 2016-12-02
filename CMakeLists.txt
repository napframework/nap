cmake_minimum_required(VERSION 3.4)
project(nrender)


# set some compiler flags
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse")
endif()

if(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

# Find libraries
set(CMAKE_LIBRARY_PATH "../sdl2/lib/x64")
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})
set(CMAKE_PREFIX_PATH ../sdl2)
find_package(SDL2 REQUIRED)

set(CMAKE_PREFIX_PATH ../glew)
set(CMAKE_LIBRARY_PATH "../glew/lib/Release/x64")
find_library(GLEW_LIBRARY NAMES GLEW glew32 glew glew32s PATH_SUFFIXES lib64)
find_package(glew REQUIRED)

find_package(OpenGL)
find_package(glm REQUIRED)
find_package(freeimage REQUIRED)
find_package(assimp REQUIRED)



# add all cpp files to SOURCES
file(GLOB SOURCES src/*.cpp)

# compile shared lib as target
add_library(${PROJECT_NAME} SHARED ${SOURCES})

# add to include path and expose to depending projects
target_include_directories(${PROJECT_NAME} PUBLIC src
        ${SDL2_INCLUDE_DIR}
        ${GLM_INCLUDE_DIRS}
        ${GLEW_INCLUDE_DIRS}
        ${FREEIMAGE_INCLUDE_DIRS}
        ${ASSIMP_INCLUDE_DIRS}
        )

message(WARNING ${GLEW_LIBRARIES})

# Link with SDL and GLEW
target_link_libraries(${PROJECT_NAME}
        ${SDL2_LIBRARY}
        ${GLEW_LIBRARIES}
        ${FREEIMAGE_LIBRARIES}
        ${ASSIMP_LIBRARIES}
        ${OPENGL_gl_LIBRARY}
        )

