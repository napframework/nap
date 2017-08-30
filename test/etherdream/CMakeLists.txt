cmake_minimum_required(VERSION 2.8)
project(etherdream)

#add all cpp files to SOURCES
file(GLOB SOURCES src/*.cpp)
file(GLOB HEADERS src/*.h)
file(GLOB SHADERS shaders/*.frag shaders/*.vert)
file(GLOB DATA data/*)

# Create IDE groups
source_group("headers" FILES ${HEADERS})
source_group("sources" FILES ${SOURCES})
source_group("shaders" FILES ${SHADERS})

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${SHADERS})
set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)")
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Test)
add_dependencies(${PROJECT_NAME} fbxconverter)

target_link_libraries(${PROJECT_NAME} mod_napetherdream mod_napinput mod_napsdlinput mod_napsdlwindow mod_naposc)

if (WIN32)
	find_package(etherdream REQUIRED)

    # Copy over some crap window dlls
    set(FILES_TO_COPY
            ${FREEIMAGE_DIR}/msvc/Dist/x64/FreeImage.dll
            ../../../../thirdparty/sdl2/msvc/lib/x64/SDL2.dll
            ../../../../thirdparty/glew/msvc/bin/Release/x64/glew32.dll
            ${ETHERDREAM_LIBS_DIR}/EtherDream.dll
            )
    # copy nrender (hack)
    copy_files_to_bin(${FILES_TO_COPY})
endif()

# create post build event
export_fbx(${CMAKE_CURRENT_LIST_DIR}/data)
copy_dir_to_bin(${CMAKE_CURRENT_LIST_DIR}/data data)
copy_dir_to_bin(${CMAKE_CURRENT_LIST_DIR}/shaders shaders)
