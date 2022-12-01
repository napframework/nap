cmake_minimum_required(VERSION 3.18.4)

# Verify we have a module name
if (NOT DEFINED UNPREFIXED_MODULE_NAME_INPUTCASE AND NOT DEFINED CMAKE_ONLY)
    message(FATAL_ERROR "No module name")
endif()

# Set lowercase module name, used for a filenames etc
if(DEFINED UNPREFIXED_MODULE_NAME_INPUTCASE)
    string(TOLOWER ${UNPREFIXED_MODULE_NAME_INPUTCASE} UNPREFIXED_MODULE_NAME_LOWERCASE)
endif()

# Build modules for substitution into module.json
set(MODULE_LIST_SUB_JSON "")
if(DEFINED APP_MODULE_MODULE_LIST)
    foreach(module ${APP_MODULE_MODULE_LIST})
        # Append to JSON module list
        if(NOT ${MODULE_LIST_SUB_JSON} STREQUAL "")
            set(MODULE_LIST_SUB_JSON "${MODULE_LIST_SUB_JSON},\n        ")
        endif()
        set(MODULE_LIST_SUB_JSON "${MODULE_LIST_SUB_JSON}\"${module}\"")
    endforeach()
endif ()

# Setup our paths
set(TEMPLATE_ROOT ${CMAKE_CURRENT_LIST_DIR}/template)
set(NAP_ROOT ${CMAKE_CURRENT_LIST_DIR}/../..)
if(DEFINED APP_MODULE)
    if(NOT DEFINED MODULE_DIR)
        set(MODULE_DIR ${APP_MODULE_APP_PATH}/module)
    endif()
    set(PATH_FROM_MODULE_TO_NAP_ROOT ../../..)
else()
    if(NOT DEFINED MODULE_DIR)
        set(MODULE_DIR ${NAP_ROOT}/modules/nap${UNPREFIXED_MODULE_NAME_LOWERCASE})
    endif()
    set(PATH_FROM_MODULE_TO_NAP_ROOT ../..)
endif()

# Create our module files, with substitutions
if(NOT DEFINED MODULE_CMAKE_OUTPATH)
    set(MODULE_CMAKE_OUTPATH ${MODULE_DIR}/CMakeLists.txt)
endif()
configure_file(${TEMPLATE_ROOT}/CMakeLists.txt ${MODULE_CMAKE_OUTPATH} @ONLY)

# Allow for use of module creator to upgrade CMakeLists.txt
if(DEFINED CMAKE_ONLY)
    return()
endif()

configure_file(${TEMPLATE_ROOT}/src/naptemplate.cpp ${MODULE_DIR}/src/nap${UNPREFIXED_MODULE_NAME_LOWERCASE}.cpp @ONLY)
configure_file(${TEMPLATE_ROOT}/src/templateservice.cpp ${MODULE_DIR}/src/${UNPREFIXED_MODULE_NAME_LOWERCASE}service.cpp @ONLY)
configure_file(${TEMPLATE_ROOT}/src/templateservice.h ${MODULE_DIR}/src/${UNPREFIXED_MODULE_NAME_LOWERCASE}service.h @ONLY)
configure_file(${TEMPLATE_ROOT}/module.json ${MODULE_DIR}/module.json @ONLY)

# Create our module directory shortcuts
if(NOT DEFINED APP_MODULE)
    set(src_dir ${NAP_ROOT}/tools/buildsystem/module_dir_shortcuts)
    if(UNIX)
        set(src_dir ${src_dir}/unix)
        configure_file(${src_dir}/regenerate.sh ${MODULE_DIR}/regenerate.sh @ONLY)
        configure_file(${src_dir}/prepare_module_to_share.sh ${MODULE_DIR}/prepare_module_to_share.sh @ONLY)
    elseif(WIN32)
        set(src_dir ${src_dir}/win64)
        configure_file(${src_dir}/regenerate.bat ${MODULE_DIR}/regenerate.bat @ONLY)
        configure_file(${src_dir}/prepare_module_to_share.bat ${MODULE_DIR}/prepare_module_to_share.bat @ONLY)
    endif()
endif()
