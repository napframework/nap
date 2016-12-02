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

# Override binary directories
set(BIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bin/${BUILD_CONF})
file(MAKE_DIRECTORY ${BIN_DIR})
set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/${BUILD_CONF})
file(MAKE_DIRECTORY ${LIB_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${LIB_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${LIB_DIR})
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})

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
file(GLOB HEADERS src/*.h)
source_group("headers" FILES ${HEADERS})
source_group("sources" FILES ${SOURCES})

# compile shared lib as target
add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})

# add to include path and expose to depending projects
target_include_directories(${PROJECT_NAME} PUBLIC src
        ${SDL2_INCLUDE_DIR}
        ${GLM_INCLUDE_DIRS}
        ${GLEW_INCLUDE_DIRS}
        ${FREEIMAGE_INCLUDE_DIRS}
        ${ASSIMP_INCLUDE_DIRS}
        )

# Link with SDL and GLEW
target_link_libraries(${PROJECT_NAME}
        ${SDL2_LIBRARY}
        ${GLEW_LIBRARIES}
        ${FREEIMAGE_LIBRARIES}
        ${ASSIMP_LIBRARIES}
        ${OPENGL_gl_LIBRARY}
        )

include(GenerateExportheader)
GENERATE_EXPORT_HEADER(${PROJECT_NAME})

