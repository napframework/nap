cmake_minimum_required(VERSION 2.8)
project(tommy)

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
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Test)
add_dependencies(${PROJECT_NAME} fbxconverter)
target_compile_definitions(${PROJECT_NAME} PRIVATE MODULE_NAME=${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
        napcore
        mod_naprender
        nrender
        mod_napmath
        mod_napinput
        mod_napsdlinput
        mod_napsdlwindow
        mod_napapp
        mod_napsandbox
        )

if (WIN32)
    # Copy freeimage DLL
	find_package(freeimage REQUIRED)
	copy_freeimage_dll()

    # Copy over some crap window dlls
	copy_base_windows_graphics_dlls()
endif()