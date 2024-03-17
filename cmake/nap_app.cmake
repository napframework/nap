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

# Create apps and demos IDE folders
cmake_path(GET CMAKE_CURRENT_SOURCE_DIR PARENT_PATH parent_path)
cmake_path(GET parent_path STEM LAST_ONLY parent_dir)
if(NAP_BUILD_CONTEXT MATCHES "source")
    if(DEFINED APP_CUSTOM_IDE_FOLDER)
        set(app_folder_name ${APP_CUSTOM_IDE_FOLDER})
    elseif(parent_dir MATCHES "^apps$")
        set(app_folder_name Apps)
    else()
        set(app_folder_name Demos)
    endif()
    set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER ${app_folder_name})
endif()

# Pull in a app module if it exists
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/module/)
    add_subdirectory(module)
    target_link_libraries(${PROJECT_NAME} nap${PROJECT_NAME})
endif()

# Read required modules from the app json file
file(READ ${CMAKE_CURRENT_SOURCE_DIR}/app.json app_json)
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

# Create patched version of app.json to copy to bin

# Check if path mapping exists, otherwise use default
string(JSON path_mapping GET ${app_json} PathMapping)
set(bin_app_json ${app_json})
set(path_mapping_file ${CMAKE_CURRENT_SOURCE_DIR}/${path_mapping})
if(NOT EXISTS ${path_mapping_file})
    set(path_mapping "default_path_mapping.json")
    string(JSON bin_app_json SET ${bin_app_json} "PathMapping" \"${path_mapping}\")
    set(path_mapping_file ${NAP_ROOT}/cmake/${path_mapping})
endif()

# Patch data path in app json
string(JSON data_file_path GET ${app_json} Data)
set(absolute_data_file_path ${CMAKE_CURRENT_SOURCE_DIR}/${data_file_path})
string(JSON bin_app_json SET ${app_json} "Data" \"${absolute_data_file_path}\")

# Write app json to bin
file(WRITE ${BIN_DIR}/${PROJECT_NAME}.json ${bin_app_json})

# Copy path mapping to bin
add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${path_mapping_file} ${BIN_DIR}/${path_mapping})

# Run FBX converter post-build
add_dependencies(${PROJECT_NAME} fbxconverter)
set(data_dir ${CMAKE_CURRENT_SOURCE_DIR}/data/)
add_custom_command(TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${BIN_DIR}/fbxconverter -o ${data_dir} ${data_dir}/*.fbx
        COMMENT "Exporting FBX in '${data_dir}'"
)
