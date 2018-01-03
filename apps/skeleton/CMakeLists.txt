cmake_minimum_required(VERSION 2.8)
project(skeleton)

# add all cpp files to SOURCES
file(GLOB SOURCES src/*.cpp)
file(GLOB HEADERS src/*.h)
file(GLOB SHADERS shaders/*.frag shaders/*.vert)
file(GLOB DATA data/*)

# Create IDE groups
source_group("headers" FILES ${HEADERS})
source_group("sources" FILES ${SOURCES})
source_group("shaders" FILES ${SHADERS})

# add executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${SHADERS})

set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)")
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Apps)

add_dependencies(${PROJECT_NAME} fbxconverter)

target_link_libraries(${PROJECT_NAME}
        mod_napinput
        mod_napsdlinput
        mod_napsdlwindow
        mod_napsvg
        mod_napapp)


if (WIN32)
    find_package(freeimage REQUIRED)
    copy_freeimage_dll()

    copy_base_windows_graphics_dlls()
endif()

# should probably only contain the skeleton json
copy_dir_to_bin(data data)

