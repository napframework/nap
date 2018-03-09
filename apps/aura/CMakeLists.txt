cmake_minimum_required(VERSION 2.8)
project(aura)

#add all cpp files to SOURCES
file(GLOB SOURCES src/*.cpp)
file(GLOB HEADERS src/*.h)
file(GLOB SHADERS data/shaders/*.frag data/shaders/*.vert)
file(GLOB DATA data/*)

# Create IDE groups
source_group("headers" FILES ${HEADERS})
source_group("sources" FILES ${SOURCES})
source_group("shaders" FILES ${SHADERS})

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${NANOHEADERS} ${SHADERS})
set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)")
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Apps)
add_dependencies(${PROJECT_NAME} fbxconverter)

# preprocessor
target_link_libraries(${PROJECT_NAME} mod_napaura mod_napapp)
target_compile_definitions(${PROJECT_NAME} PRIVATE _USE_MATH_DEFINES)
add_dependencies(${PROJECT_NAME} fbxconverter)

# Copy etherdream DLL
find_package(etherdream REQUIRED)
copy_etherdream_dll()

if (WIN32)
	# Copy freeimage DLL
	find_package(freeimage REQUIRED)
	copy_freeimage_dll()

    # Copy over some crap window dlls
	copy_base_windows_graphics_dlls()
endif()

# create post build event
export_fbx_in_place(${CMAKE_CURRENT_SOURCE_DIR}/data/)
