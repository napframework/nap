cmake_minimum_required(VERSION 3.5)

# Verify we have a project name
if (NOT DEFINED PROJECT_NAME_CAMELCASE)
    message(FATAL_ERROR "No project name")
endif()

# Build modules for substitution
# TODO for now this does a hacky job of plugging modules into CMake and C++.  Temporary until we
# read modules from project.json
set(MODULE_LIST_SUB_JSON "")
set(MODULE_LIST_SUB_CMAKE "")
set(MODULE_LIST_SUB_CPP "")
if(DEFINED MODULE_LIST)
    foreach(module ${MODULE_LIST})
        # Append to JSON module list
        if(NOT ${MODULE_LIST_SUB_JSON} STREQUAL "")
            set(MODULE_LIST_SUB_JSON "${MODULE_LIST_SUB_JSON},\n        ")
        endif()
        set(MODULE_LIST_SUB_JSON "${MODULE_LIST_SUB_JSON}\"${module}\"")

        # Append to CMake module list - TODO temporary until project.json, remove
        set(MODULE_LIST_SUB_CMAKE "${MODULE_LIST_SUB_CMAKE}${module} ")

        # # Append to C++ module list - TODO temporary until project.json, remove
        # if(NOT ${MODULE_LIST_SUB_CPP} STREQUAL "")
        #     set(MODULE_LIST_SUB_CPP "${MODULE_LIST_SUB_CPP},")
        # endif()
        # set(MODULE_LIST_SUB_CPP "${MODULE_LIST_SUB_CPP}\"${module}\"")
    endforeach()
    string (STRIP "${MODULE_LIST_SUB_CMAKE}" MODULE_LIST_SUB_CMAKE)
endif ()

# Set lowercase project name, used for a filenames etc
string(TOLOWER ${PROJECT_NAME_CAMELCASE} PROJECT_NAME_LOWERCASE)

# Setup our paths
set(TEMPLATE_ROOT ${CMAKE_CURRENT_LIST_DIR}/template)
set(NAP_ROOT ${CMAKE_CURRENT_LIST_DIR}/../..)
set(PROJECT_DIR ${NAP_ROOT}/projects/${PROJECT_NAME_LOWERCASE})

# Create our project files, with substitutions
# TODO potentially look into a generic recursive globbed version of this, but it's probably not worth it
configure_file(${TEMPLATE_ROOT}/project.json ${PROJECT_DIR}/project.json @ONLY)
configure_file(${TEMPLATE_ROOT}/CMakeLists.txt ${PROJECT_DIR}/CMakeLists.txt @ONLY)
configure_file(${TEMPLATE_ROOT}/data/template/appStructure.json ${PROJECT_DIR}/data/${PROJECT_NAME_LOWERCASE}/appStructure.json @ONLY)

configure_file(${TEMPLATE_ROOT}/src/main.cpp ${PROJECT_DIR}/src/main.cpp @ONLY)
configure_file(${TEMPLATE_ROOT}/src/templateapp.cpp ${PROJECT_DIR}/src/${PROJECT_NAME_LOWERCASE}app.cpp @ONLY)
configure_file(${TEMPLATE_ROOT}/src/templateapp.h ${PROJECT_DIR}/src/${PROJECT_NAME_LOWERCASE}app.h @ONLY)
file(MAKE_DIRECTORY ${PROJECT_DIR}/shaders)
