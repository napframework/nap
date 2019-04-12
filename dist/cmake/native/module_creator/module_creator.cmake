cmake_minimum_required(VERSION 3.5)

# Verify we have a module name
if (NOT DEFINED MODULE_NAME_PASCALCASE)
    message(FATAL_ERROR "No module name")
endif()

# Set lowercase module name, used for a filenames etc
string(TOLOWER ${MODULE_NAME_PASCALCASE} MODULE_NAME_LOWERCASE)

# Setup our paths
set(TEMPLATE_ROOT ${CMAKE_CURRENT_LIST_DIR}/template)
set(NAP_ROOT ${CMAKE_CURRENT_LIST_DIR}/../..)
if(DEFINED PROJECT_MODULE)
    set(MODULE_DIR ${PROJECT_MODULE_PROJECT_PATH}/module)
    set(PATH_FROM_MODULE_TO_NAP_ROOT ../../..)
else()
    set(MODULE_DIR ${NAP_ROOT}/user_modules/mod_${MODULE_NAME_LOWERCASE})
    set(PATH_FROM_MODULE_TO_NAP_ROOT ../..)
endif()

# Create our module files, with substitutions
configure_file(${TEMPLATE_ROOT}/CMakeLists.txt ${MODULE_DIR}/CMakeLists.txt @ONLY)
configure_file(${TEMPLATE_ROOT}/src/mod_template.cpp ${MODULE_DIR}/src/mod_${MODULE_NAME_LOWERCASE}.cpp @ONLY)
configure_file(${TEMPLATE_ROOT}/src/templateservice.cpp ${MODULE_DIR}/src/${MODULE_NAME_LOWERCASE}service.cpp @ONLY)
configure_file(${TEMPLATE_ROOT}/src/templateservice.h ${MODULE_DIR}/src/${MODULE_NAME_LOWERCASE}service.h @ONLY)
configure_file(${TEMPLATE_ROOT}/module.json ${MODULE_DIR}/module.json @ONLY)

# Create our module directory regenerate shortcut
if(NOT DEFINED PROJECT_MODULE)
    if(UNIX)
        configure_file(${NAP_ROOT}/tools/platform/module_dir_shortcuts/regenerate ${MODULE_DIR}/regenerate @ONLY)
    elseif(WIN32)
        configure_file(${NAP_ROOT}/tools/platform/module_dir_shortcuts/regenerate.bat ${MODULE_DIR}/regenerate.bat @ONLY)    
    endif()
endif()
