cmake_minimum_required(VERSION 2.8)
project(kalvertoren)

#add all cpp files to SOURCES
file(GLOB SOURCES src/*.cpp)
file(GLOB HEADERS src/*.h)
file(GLOB SHADERS data/shaders/*.frag data/shaders/*.vert)
file(GLOB DATA data/*)

# Create IDE groups
source_group("headers" FILES ${HEADERS})
source_group("sources" FILES ${SOURCES})
source_group("shaders" FILES ${SHADERS})

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${SHADERS})
set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)")
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Apps)
add_dependencies(${PROJECT_NAME} fbxconverter)

target_link_libraries(${PROJECT_NAME} mod_napkalvertoren)
target_compile_definitions(${PROJECT_NAME} PRIVATE MODULE_NAME=${PROJECT_NAME})

if (WIN32)
    # Copy freeimage DLL
	find_package(freeimage REQUIRED)
	copy_freeimage_dll()

    # Copy over some crap window dlls
	copy_base_windows_graphics_dlls()

	# copy all ffmpeg dlls
    copy_windows_ffmpeg_dlls()

    # find artnet
    find_package(artnet REQUIRED)
    copy_artnet_dll()

endif()

# create post build event
export_fbx_in_place(${CMAKE_CURRENT_SOURCE_DIR}/data/)

