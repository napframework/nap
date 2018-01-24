cmake_minimum_required(VERSION 2.8)
project(videomodulation)

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
add_dependencies(${PROJECT_NAME} fbxconverter)

# TODO update, was this pre-merge
# target_link_libraries(${PROJECT_NAME} napcore mod_naprender nrender mod_napmath mod_napinput mod_napsdlinput mod_napsdlwindow mod_napvideo mod_napapp mod_napimgui)
target_link_libraries(${PROJECT_NAME} mod_videomodulation mod_napapp)

# Build into a project directory under bin
set_output_directories()

if (WIN32)
    # Copy freeimage DLL
	find_package(freeimage REQUIRED)
	copy_freeimage_dll()

    # Copy over some crap window dlls
	copy_base_windows_graphics_dlls()

	# copy all ffmpeg dlls
    copy_windows_ffmpeg_dlls()

	# Copy all of our Windows DLLs that have build in bin into project dir
	# TODO this is NOT a clean solution, but if we're mainly releasing projects build against packaged releases of the
	#      framework maybe this is OK?
	bulk_copy_windows_dlls_to_bin()
endif()

# Copy project json to project bin dir and export fbx, via post-build events
copy_files_to_bin(${CMAKE_CURRENT_LIST_DIR}/project.json)
export_fbx_in_place(${CMAKE_CURRENT_SOURCE_DIR}/data/)

# Package into release build
install(DIRECTORY . DESTINATION "examples/${PROJECT_NAME}"
		PATTERN "CMakeLists.txt" EXCLUDE
		PATTERN "dist" EXCLUDE)
install(FILES dist/CMakeLists.txt DESTINATION "examples/${PROJECT_NAME}/")