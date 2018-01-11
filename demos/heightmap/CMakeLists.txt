cmake_minimum_required(VERSION 2.8)
project(heightmap)

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
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Demos)

target_link_libraries(${PROJECT_NAME} mod_heightmap mod_napapp mod_napcameracontrol)
add_dependencies(${PROJECT_NAME} fbxconverter)

if (WIN32)
	# Copy freeimage DLL
	find_package(freeimage REQUIRED)
	copy_freeimage_dll()

    	# Copy over some crap window dlls
	copy_base_windows_graphics_dlls()
	copy_windows_ffmpeg_dlls()
endif()

# create post build event
copy_dir_to_bin(${CMAKE_CURRENT_LIST_DIR}/data data)
copy_dir_to_bin(${CMAKE_CURRENT_LIST_DIR}/shaders shaders)
export_fbx(${CMAKE_CURRENT_LIST_DIR}/data)