cmake_minimum_required(VERSION 2.8)
project(kalvertoren)

#add all cpp files to SOURCES
file(GLOB SOURCES src/*.cpp)
file(GLOB HEADERS src/*.h)
file(GLOB SHADERS shaders/*.frag shaders/*.vert)
file(GLOB DATA data/*)

# Create IDE groups
source_group("headers" FILES ${HEADERS})
source_group("sources" FILES ${SOURCES})
source_group("shaders" FILES ${SHADERS})

set(pybind11_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../thirdparty/pybind11/install/share/cmake/pybind11")
find_package(pybind11) 

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${SHADERS})
set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)")
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Apps)
add_dependencies(${PROJECT_NAME} fbxconverter mod_nappython)

target_include_directories(${PROJECT_NAME} PUBLIC ${pybind11_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} napcore mod_naprender nrender mod_napmath mod_napsdlinput mod_napsdlwindow mod_napvideo mod_napcameracontrol mod_napapp mod_napartnet mod_napimgui)
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
export_fbx(${CMAKE_CURRENT_LIST_DIR}/data/kalvertoren)
copy_dir_to_bin(${CMAKE_CURRENT_LIST_DIR}/data data)
copy_dir_to_bin(${CMAKE_CURRENT_LIST_DIR}/shaders shaders)
