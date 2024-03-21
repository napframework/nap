cmake_minimum_required(VERSION 3.19)

# Get the app name from the directory name
get_filename_component(app_name ${CMAKE_CURRENT_SOURCE_DIR} NAME)

project(${app_name})

# Bring in any additional app logic (pre-target definition)
set(APP_EXTRA_PRE_TARGET_CMAKE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/app_extra_pre_target.cmake)
if(EXISTS ${APP_EXTRA_PRE_TARGET_CMAKE_PATH})
    unset(SKIP_APP)
    include(${APP_EXTRA_PRE_TARGET_CMAKE_PATH})
    if(SKIP_APP)
        return()
    endif()
endif()

# Add all cpp files to SOURCES
file(GLOB_RECURSE SOURCES src/*.cpp)
file(GLOB_RECURSE HEADERS src/*.h src/*.hpp)
file(GLOB_RECURSE SHADERS data/shaders/*.frag data/shaders/*.vert data/shaders/*.comp)

# Declare target
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${SHADERS})

# Pull in a app module if it exists
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/module/)
    add_subdirectory(module)
    target_link_libraries(${PROJECT_NAME} nap${PROJECT_NAME})
endif()

# Read required modules from the app json file
set(app_json_path ${CMAKE_CURRENT_SOURCE_DIR}/app.json)
file(READ ${app_json_path} app_json)
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

# Check if path mapping exists, otherwise use default
string(JSON path_mapping_path GET ${app_json} PathMapping)
set(path_mapping_abs_path ${CMAKE_CURRENT_SOURCE_DIR}/${path_mapping_path})
if(("${path_mapping_path}" STREQUAL "") OR (NOT EXISTS ${path_mapping_abs_path}))
    set(path_mapping_path "default_path_mapping.json")
    set(path_mapping_abs_path ${NAP_ROOT}/cmake/${path_mapping_path})
    string(JSON patched_path_mapping_app_json SET ${app_json} "PathMapping" \"${path_mapping_path}\")
else()
    set(patched_path_mapping_app_json ${app_json})
endif()

set(app_install_data_dir ${BIN_DIR}/app_install_data/${PROJECT_NAME})
set(cache_dir ${CMAKE_CURRENT_SOURCE_DIR}/cache)
file(MAKE_DIRECTORY ${cache_dir})

# Write app json with patched path mapping to bin for installation
file(WRITE ${cache_dir}/patched_path_mapping_app.json ${patched_path_mapping_app_json})
add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${cache_dir}/patched_path_mapping_app.json
        ${app_install_data_dir}/app.json)

# Copy path mapping to bin
add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${path_mapping_abs_path} ${BIN_DIR}/${path_mapping_path})

# Patch data path in app json
string(JSON data_file_path GET ${app_json} Data)
set(absolute_data_file_path ${CMAKE_CURRENT_SOURCE_DIR}/${data_file_path})
set(bin_app_json ${app_json})
string(JSON patched_data_app_json SET ${patched_path_mapping_app_json} "Data" \"${absolute_data_file_path}\")

# Write app json with patched data path to bin for running apps from source
file(WRITE ${cache_dir}/patched_data_app.json ${patched_data_app_json})
add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${cache_dir}/patched_data_app.json
        ${BIN_DIR}/${PROJECT_NAME}.json)

# Update executable rpath
if(UNIX)
    file(RELATIVE_PATH rpath ${BIN_DIR} ${LIB_DIR})
    add_custom_command(TARGET ${PROJECT_NAME}
            POST_BUILD COMMAND
            ${CMAKE_INSTALL_NAME_TOOL} -add_rpath "@executable_path/${rpath}/."
            $<TARGET_FILE:${PROJECT_NAME}>)
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

# Install to packaged app
install(FILES $<TARGET_FILE:${PROJECT_NAME}> TYPE BIN OPTIONAL)
install(FILES ${app_install_data_dir}/app.json TYPE BIN OPTIONAL)
install(FILES ${BIN_DIR}/${path_mapping_path} TYPE BIN OPTIONAL)
install(DIRECTORY ${bin_data_dir} TYPE DATA OPTIONAL)
