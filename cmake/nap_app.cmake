cmake_minimum_required(VERSION 3.19)

# Get the app name from the directory name
get_filename_component(app_name ${CMAKE_CURRENT_SOURCE_DIR} NAME)

project(${app_name})

# Bring in any additional app logic (pre-target definition)
set(app_extra_pre_target_cmake_path ${CMAKE_CURRENT_SOURCE_DIR}/app_extra_pre_target.cmake)
if(EXISTS ${app_extra_pre_target_cmake_path})
    unset(SKIP_APP)
    include(${app_extra_pre_target_cmake_path})
    if(SKIP_APP)
        return()
    endif()
endif()

#set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# Add all cpp files to SOURCES
file(GLOB_RECURSE SOURCES src/*.cpp)
file(GLOB_RECURSE HEADERS src/*.h src/*.hpp)
file(GLOB_RECURSE SHADERS data/shaders/*.frag data/shaders/*.vert data/shaders/*.comp)

# Declare target
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${SHADERS})

set_target_properties(${PROJECT_NAME} PROPERTIES INSTALL_RPATH "$ORIGIN/lib")

# Pull in the app module if it exists
if (TARGET nap${PROJECT_NAME})
    target_link_libraries(${PROJECT_NAME} nap${PROJECT_NAME})
endif()

# Create the cache directory in source
set(cache_dir ${CMAKE_CURRENT_SOURCE_DIR}/cache)
file(MAKE_DIRECTORY ${cache_dir})

# Read app.json from file
set(app_json_path ${CMAKE_CURRENT_SOURCE_DIR}/app.json)
file(READ ${app_json_path} app_json)

# Read required modules from the app json file
string(JSON required_modules_json GET ${app_json} RequiredModules)
string(JSON required_module_count LENGTH ${app_json} RequiredModules)
set(module_index 0)
while(NOT ${module_index} EQUAL ${required_module_count})
    string(JSON module GET ${required_modules_json} ${module_index})
    list(APPEND required_modules ${module})
    math(EXPR module_index "${module_index}+1")
endwhile()

if (NOT required_modules)
    set(required_modules napcore)
endif()
target_link_libraries(${PROJECT_NAME} ${required_modules})

if(NAP_ENABLE_PYTHON)
    target_link_libraries(${PROJECT_NAME} ${PYTHON_LIBRARIES})
endif()

# Include any extra app CMake logic
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/app_extra.cmake)
    include(${CMAKE_CURRENT_SOURCE_DIR}/app_extra.cmake)
endif()

# Check if path mapping is set in app.json
string(JSON path_mapping_path GET ${app_json} PathMapping)
set(path_mapping_abs_path ${CMAKE_CURRENT_SOURCE_DIR}/${path_mapping_path})
if ("${path_mapping_path}" STREQUAL "")
    message(FATAL_ERROR "No path mapping found for App target ${PROJECT_NAME}. Set the path mapping in App.json")
endif()

# Check if pathmapping file from app.json exists, otherwise copy from default
if((NOT EXISTS ${path_mapping_abs_path}))
    file(COPY_FILE ${NAP_ROOT}/cmake/default_path_mapping.json ${path_mapping_abs_path})
endif()

# Set the path mapping variable in patched app.json to copy of path mapping in project root.
cmake_path(GET path_mapping_path FILENAME path_mapping_filename)
string(JSON patched_path_mapping_app_json SET ${app_json} "PathMapping" \"${path_mapping_filename}\")

# Copy path mapping to app specific install data directory in bin
set(app_install_data_dir ${BIN_DIR}/app_install_data/${PROJECT_NAME})
file(MAKE_DIRECTORY ${app_install_data_dir})
add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${path_mapping_abs_path} ${app_install_data_dir}/${path_mapping_filename})

# Write app json with patched path mapping to source cache
file(WRITE ${cache_dir}/patched_path_mapping_app.json ${patched_path_mapping_app_json})
# Write app json with patched path mapping to bin for installation
add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${cache_dir}/patched_path_mapping_app.json
        ${app_install_data_dir}/app.json)

# Patch data path in app json
string(JSON data_file_path GET ${app_json} Data)
set(absolute_data_file_path ${CMAKE_CURRENT_SOURCE_DIR}/${data_file_path})
string(JSON patched_data_app_json SET ${app_json} "Data" \"${absolute_data_file_path}\")
string(JSON patched_data_app_json SET ${patched_data_app_json} "PathMapping" \"app_install_data/${PROJECT_NAME}/${path_mapping_filename}\")

# Write app json with patched data path and path mapping to bin for running apps from source
file(WRITE ${cache_dir}/patched_data_app.json ${patched_data_app_json})
add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${cache_dir}/patched_data_app.json
        ${BIN_DIR}/${PROJECT_NAME}.json)

# Update executable rpath
if(APPLE)
    add_custom_command(TARGET ${PROJECT_NAME}
            POST_BUILD COMMAND
            ${CMAKE_INSTALL_NAME_TOOL} -add_rpath "@executable_path/${LIB_RPATH}/." $<TARGET_FILE:${PROJECT_NAME}>)
endif()

# Copy data directory to app specific bin
set(bin_data_dir ${app_install_data_dir}/data)
add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/data ${bin_data_dir})

# Run FBX converter post-build within bin data dir
add_dependencies(${PROJECT_NAME} fbxconverter)
add_custom_command(TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${BIN_DIR}/fbxconverter -o ${bin_data_dir} ${bin_data_dir}/*.fbx
        COMMENT "Exporting FBX in '${bin_data_dir}'")

# Copy NAP license files
set(bin_license_dir ${BIN_DIR}/license)
add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${NAP_ROOT}/docs/license ${bin_license_dir}/NAP)

# Install to packaged app
install(FILES $<TARGET_FILE:${PROJECT_NAME}> PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE TYPE BIN OPTIONAL)
install(FILES ${app_install_data_dir}/app.json TYPE BIN OPTIONAL)
install(FILES ${app_install_data_dir}/${path_mapping_filename} TYPE BIN OPTIONAL)
install(DIRECTORY ${bin_data_dir} TYPE DATA OPTIONAL)
install(DIRECTORY ${bin_license_dir} TYPE DOC OPTIONAL)

# Configure Info.plist, copy to bin and install to packaged app
if (APPLE)
    set(plist ${CMAKE_CURRENT_SOURCE_DIR}/cache/Info.plist)
    if(NOT EXISTS ${plist})
        string(JSON APP_TITLE GET ${app_json} Title)
        configure_file(${NAP_ROOT}/cmake/Info.plist.in ${plist})
    endif ()
    add_custom_command(
            TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${plist} ${BIN_DIR})
    install(FILES ${BIN_DIR}/Info.plist TYPE INFO OPTIONAL)
endif()

