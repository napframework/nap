cmake_minimum_required(VERSION 3.5)
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

# Get our modules list from project.json
project_json_to_cmake()

target_link_libraries(${PROJECT_NAME} napcore ${NAP_MODULES} nrender)

# Build into a project directory under bin
set_output_directories()

if (WIN32)
    # Copy freeimage DLL
    find_package(freeimage REQUIRED)
    copy_freeimage_dll()

    # Copy over some crap window dlls
    copy_base_windows_graphics_dlls()

    # Copy all of our Windows DLLs that have build in bin into project dir.  Temporary approach until we're
    # reading cleaner approach ala what we do against NAP release makes it into builds against NAP source.
    bulk_copy_windows_dlls_to_bin()
endif()

# Copy project json to project bin dir and export fbx, via post-build events
copy_files_to_bin(${CMAKE_CURRENT_LIST_DIR}/project.json)

# Package into release build.. for now
package_project_into_release("examples/${PROJECT_NAME}")
