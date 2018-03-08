cmake_minimum_required(VERSION 3.5)
project(skeleton)

# add all cpp files to SOURCES
file(GLOB SOURCES src/*.cpp)
file(GLOB HEADERS src/*.h)
file(GLOB SHADERS data/shaders/*.frag data/shaders/*.vert)
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
        mod_napapp
        mod_napvideo)


# Build into a project directory under bin
set_output_directories()

if (WIN32)
    find_package(freeimage REQUIRED)
    copy_freeimage_dll()

    copy_base_windows_graphics_dlls()

    # copy all ffmpeg dlls
    copy_windows_ffmpeg_dlls()

    # Copy all of our Windows DLLs that have build in bin into project dir.  Temporary approach until we're
    # reading cleaner approach ala what we do against NAP release makes it into builds against NAP source.
    bulk_copy_windows_dlls_to_bin()
elseif(APPLE)
    # Add the runtime path for RTTR on macOS
    add_macos_rttr_rpath()
endif()

# Copy project json to project bin dir via post-build events
# should probably only contain the skeleton json
copy_files_to_bin(${CMAKE_CURRENT_LIST_DIR}/project.json)