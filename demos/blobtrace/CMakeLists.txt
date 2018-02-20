cmake_minimum_required(VERSION 2.8)
project(blobtrace)

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
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Demos)

target_link_libraries(${PROJECT_NAME} mod_naprender mod_napinput mod_napsdlinput mod_napsdlwindow mod_napapp mod_napcameracontrol)
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
export_fbx_in_place(${CMAKE_CURRENT_SOURCE_DIR}/data/)

# Install virtual env config file so our Python exec lib can find the modules in thirdparty
install_python_virtualenv_config()