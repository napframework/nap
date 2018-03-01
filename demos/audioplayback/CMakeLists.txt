cmake_minimum_required(VERSION 3.5)
project(audioplayback)

#add all cpp files to SOURCES
file(GLOB SOURCES src/*.cpp)
file(GLOB HEADERS src/*.h)
file(GLOB DATA data/*)

# Create IDE groups
source_group("headers" FILES ${HEADERS})
source_group("sources" FILES ${SOURCES})

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)")
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Demos)

target_link_libraries(${PROJECT_NAME} mod_naprender mod_napinput mod_napsdlinput mod_napsdlwindow mod_napapp mod_napcameracontrol mod_napaudio)

# Build into a project directory under bin
set_output_directories()

if (WIN32)
    # Copy freeimage DLL
    find_package(freeimage REQUIRED)
    copy_freeimage_dll()

    # Copy over some crap window dlls
    copy_base_windows_graphics_dlls()

    # Copy all of our Windows DLLs that have build in bin into project dir
    # TODO this is NOT a clean solution, but if we're mainly releasing projects build against packaged releases of the
    #      framework maybe this is OK?
    bulk_copy_windows_dlls_to_bin()    
endif()

# Copy project json to project bin dir and export fbx, via post-build events
copy_files_to_bin(${CMAKE_CURRENT_LIST_DIR}/project.json)

# Install virtual env config file so our Python exec lib can find the modules in thirdparty
install_python_virtualenv_config()

# Package into release build
package_project_into_release("demos/${PROJECT_NAME}")
