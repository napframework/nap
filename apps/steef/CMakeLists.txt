cmake_minimum_required(VERSION 2.8)
project(steef)

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

target_link_libraries(${PROJECT_NAME} napcore mod_naprender nrender mod_napmath mod_napsdlinput mod_napsdlwindow mod_napapp)

# Build into a project directory under bin
set_output_directories()

if (WIN32)
    # Copy freeimage DLL
    find_package(freeimage REQUIRED)
    copy_freeimage_dll()

    # Copy over some crap window dlls
    copy_base_windows_graphics_dlls()

    # Copy all of our Windows DLLs that have build in bin into project dir
    # TODO this is NOT a clean solution, but if we're mainly releasing projects built against packaged releases of the
    #      framework maybe this is OK?
    bulk_copy_windows_dlls_to_bin()
endif()

# Copy project json to project bin dir and export fbx, via post-build events
copy_files_to_bin(${CMAKE_CURRENT_LIST_DIR}/project.json)
export_fbx_in_place(${CMAKE_CURRENT_SOURCE_DIR}/data/)