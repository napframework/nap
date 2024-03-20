cmake_minimum_required(VERSION 3.19)

# Get the module name from the directory name
get_filename_component(parent_dir ${CMAKE_CURRENT_SOURCE_DIR}/.. REALPATH)
get_filename_component(parent_name ${parent_dir} NAME)
if(module_name MATCHES "^module$")
    # Handle app module
    set(module_name "nap${parent_name}")
else()
    get_filename_component(module_name ${CMAKE_CURRENT_SOURCE_DIR} NAME)
endif()

project(${module_name})

# Add source
file(GLOB SOURCES src/*.cpp)
file(GLOB HEADERS src/*.h src/*.hpp)

# Compile target as shared lib
add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})

# Remove lib prefix on Unix libraries
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

# Add include dirs
target_include_directories(${PROJECT_NAME} PUBLIC src)

# Preprocessor
target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)
target_compile_definitions(${PROJECT_NAME} PRIVATE MODULE_NAME=${PROJECT_NAME})

# Read required modules from the module json file
file(READ ${CMAKE_CURRENT_SOURCE_DIR}/module.json module_json)
string(JSON required_modules_json GET ${module_json} RequiredModules)
string(JSON required_module_count LENGTH ${module_json} RequiredModules)
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

# Bring in any additional module logic
set(MODULE_EXTRA_CMAKE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/module_extra.cmake)
if (EXISTS ${MODULE_EXTRA_CMAKE_PATH})
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty)
        list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/cmake_find_modules)
    endif()

    include(${MODULE_EXTRA_CMAKE_PATH})
endif()

# Copy module.json to bin
add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/module.json
        ${LIB_DIR}/${PROJECT_NAME}.json)

# Copy module data folder
get_filename_component(parent_dir ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)
if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/data)
    set(dest ${BIN_DIR}/${parent_name}/${PROJECT_NAME})
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/data
        ${dest}/data)
endif()

# Install library and module json
install(TARGETS ${PROJECT_NAME} LIBRARY OPTIONAL)
install(FILES ${LIB_DIR}/${PROJECT_NAME}.json TYPE LIB OPTIONAL)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data DESTINATION ${CMAKE_INSTALL_DATADIR}/${parent_name}/${PROJECT_NAME} OPTIONAL)



