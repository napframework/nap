cmake_minimum_required(VERSION 3.5)

# Verify we have a project name
if (NOT DEFINED PROJECT_NAME_PASCALCASE)
    message(FATAL_ERROR "No project name")
endif()

# Build modules for substitution into project.json
set(MODULE_LIST_SUB_JSON "")
if(DEFINED MODULE_LIST)
    foreach(module ${MODULE_LIST})
        # Append to JSON module list
        if(NOT ${MODULE_LIST_SUB_JSON} STREQUAL "")
            set(MODULE_LIST_SUB_JSON "${MODULE_LIST_SUB_JSON},\n        ")
        endif()
        set(MODULE_LIST_SUB_JSON "${MODULE_LIST_SUB_JSON}\"${module}\"")
    endforeach()
endif ()

# Set lowercase project name, used for a filenames etc
string(TOLOWER ${PROJECT_NAME_PASCALCASE} PROJECT_NAME_LOWERCASE)

# Setup our paths
set(TEMPLATE_ROOT ${CMAKE_CURRENT_LIST_DIR}/template)
set(NAP_ROOT ${CMAKE_CURRENT_LIST_DIR}/../..)
set(PROJECT_DIR ${NAP_ROOT}/projects/${PROJECT_NAME_LOWERCASE})

# Create our project files, with substitutions
configure_file(${TEMPLATE_ROOT}/project.json ${PROJECT_DIR}/project.json @ONLY)
configure_file(${TEMPLATE_ROOT}/CMakeLists.txt ${PROJECT_DIR}/CMakeLists.txt @ONLY)
configure_file(${TEMPLATE_ROOT}/data/app_structure.json ${PROJECT_DIR}/data/app_structure.json @ONLY)

configure_file(${TEMPLATE_ROOT}/src/main.cpp ${PROJECT_DIR}/src/main.cpp @ONLY)
configure_file(${TEMPLATE_ROOT}/src/templateapp.cpp ${PROJECT_DIR}/src/${PROJECT_NAME_LOWERCASE}app.cpp @ONLY)
configure_file(${TEMPLATE_ROOT}/src/templateapp.h ${PROJECT_DIR}/src/${PROJECT_NAME_LOWERCASE}app.h @ONLY)

# Create our project directory package and regenerate shortcuts
if(UNIX)
    configure_file(${NAP_ROOT}/tools/platform/project_dir_shortcuts/package ${PROJECT_DIR}/package @ONLY)
    configure_file(${NAP_ROOT}/tools/platform/project_dir_shortcuts/regenerate ${PROJECT_DIR}/regenerate @ONLY)
elseif(WIN32)
    configure_file(${NAP_ROOT}/tools/platform/project_dir_shortcuts/package.bat ${PROJECT_DIR}/package.bat @ONLY)
    configure_file(${NAP_ROOT}/tools/platform/project_dir_shortcuts/regenerate.bat ${PROJECT_DIR}/regenerate.bat @ONLY)    
endif()

# Make a shaders directory
file(MAKE_DIRECTORY ${PROJECT_DIR}/data/shaders)