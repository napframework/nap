cmake_minimum_required(VERSION 3.5)

# Verify we have a module name
if (NOT DEFINED MODULE_NAME_CAMELCASE)
    message(FATAL_ERROR "No module name")
endif()

# Set lowercase module name, used for a filenames etc
string(TOLOWER ${MODULE_NAME_CAMELCASE} MODULE_NAME_LOWERCASE)

# Setup our paths
set(TEMPLATE_ROOT ${CMAKE_CURRENT_LIST_DIR}/template)
set(NAP_ROOT ${CMAKE_CURRENT_LIST_DIR}/../..)
set(MODULE_DIR ${NAP_ROOT}/modules/mod_${MODULE_NAME_LOWERCASE})

# Create our module files, with substitutions
# TODO potentially look into a generic recursive globbed version of this, but it's probably not worth it
configure_file(${TEMPLATE_ROOT}/CMakeLists.txt ${MODULE_DIR}/CMakeLists.txt @ONLY)
configure_file(${TEMPLATE_ROOT}/src/mod_template.cpp ${MODULE_DIR}/src/mod_${MODULE_NAME_LOWERCASE}.cpp @ONLY)
configure_file(${TEMPLATE_ROOT}/src/templateservice.cpp ${MODULE_DIR}/src/${MODULE_NAME_LOWERCASE}service.cpp @ONLY)
configure_file(${TEMPLATE_ROOT}/src/templateservice.h ${MODULE_DIR}/src/${MODULE_NAME_LOWERCASE}service.h @ONLY)