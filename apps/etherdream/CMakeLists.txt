cmake_minimum_required(VERSION 2.8)
project(etherdream)

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
target_link_libraries(${PROJECT_NAME} mod_napetherdream mod_napinput mod_napsdlinput mod_napsdlwindow mod_naposc mod_napsvg mod_napapp)
target_compile_definitions(${PROJECT_NAME} PRIVATE _USE_MATH_DEFINES)
add_dependencies(${PROJECT_NAME} fbxconverter)

# Copy etherdream DLL
find_package(etherdream REQUIRED)
copy_etherdream_dll()

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

# Package into release build if requested
if(PACKAGE_NAIVI_APPS)   
    install(DIRECTORY . DESTINATION "projects/${PROJECT_NAME}"
            PATTERN "CMakeLists.txt" EXCLUDE
            PATTERN "dist" EXCLUDE)
    install(FILES dist/CMakeLists.txt DESTINATION "projects/${PROJECT_NAME}/")
endif()