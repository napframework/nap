cmake_minimum_required(VERSION 2.8)
project(rendertest)

#get nrender
add_subdirectory(../../../nrender build)

#add all cpp files to SOURCES
file(GLOB SOURCES src/*.cpp)
file(GLOB HEADERS src/*.h)
file(GLOB SHADERS shaders/*.frag shaders/*.vert)
file(GLOB DATA data/*)

find_package(rapidjson REQUIRED)

add_definitions(-DRAPIDJSON_HAS_STDSTRING=1)

include_directories(${RAPIDJSON_INCLUDE_DIRS})

# Create IDE groups
source_group("headers" FILES ${HEADERS})
source_group("sources" FILES ${SOURCES})
source_group("shaders" FILES ${SHADERS})


add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${SHADERS})

target_link_libraries(${PROJECT_NAME} napcore mod_naprender nrender mod_napmath)


if (WIN32)
    # Copy over some crap window dlls
    set(FILES_TO_COPY
            ${ASSIMP_DIR}/bin64/assimp.dll
            ${FREEIMAGE_DIR}/msvc/Dist/x64/FreeImage.dll
            ../../../../thirdparty/sdl2/msvc/lib/x64/SDL2.dll
            ../../../../thirdparty/glew/msvc/bin/Release/x64/glew32.dll
            )
    # copy nrender (hack)

    copy_files_to_bin(${FILES_TO_COPY})
endif()

# create post build event
copy_dir_to_bin(${CMAKE_CURRENT_LIST_DIR}/data data)
copy_dir_to_bin(${CMAKE_CURRENT_LIST_DIR}/shaders shaders)
