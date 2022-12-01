cmake_minimum_required(VERSION 3.18.4)

# Verify we have a app name
if (NOT DEFINED APP_NAME_INPUTCASE AND NOT DEFINED CMAKE_ONLY)
    message(FATAL_ERROR "No app name")
endif()

# Build modules for substitution into app.json
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

# Set lowercase app name, used for a filenames etc
if(DEFINED APP_NAME_INPUTCASE)
    string(TOLOWER ${APP_NAME_INPUTCASE} APP_NAME_LOWERCASE)
endif()

# Setup our paths
set(TEMPLATE_ROOT ${CMAKE_CURRENT_LIST_DIR}/template)
set(NAP_ROOT ${CMAKE_CURRENT_LIST_DIR}/../..)
if(NOT DEFINED PROJECT_DIR)
    set(PROJECT_DIR ${NAP_ROOT}/apps/${APP_NAME_LOWERCASE})
endif()
if(EXISTS ${NAP_ROOT}/CMakeLists.txt)
    set(NAP_BUILD_CONTEXT source)
else()
    set(NAP_BUILD_CONTEXT framework_release)
endif()

configure_file(${TEMPLATE_ROOT}/CMakeLists.txt ${PROJECT_DIR}/CMakeLists.txt @ONLY)

# Allow for use of app creator to upgrade CMakeLists.txt
if(DEFINED CMAKE_ONLY)
    return()
endif()

# Create our app files, with substitutions
configure_file(${TEMPLATE_ROOT}/app.json ${PROJECT_DIR}/app.json @ONLY)
configure_file(${TEMPLATE_ROOT}/data/objects.json ${PROJECT_DIR}/data/objects.json @ONLY)

configure_file(${TEMPLATE_ROOT}/src/main.cpp ${PROJECT_DIR}/src/main.cpp @ONLY)
configure_file(${TEMPLATE_ROOT}/src/templateapp.cpp ${PROJECT_DIR}/src/${APP_NAME_LOWERCASE}app.cpp @ONLY)
configure_file(${TEMPLATE_ROOT}/src/templateapp.h ${PROJECT_DIR}/src/${APP_NAME_LOWERCASE}app.h @ONLY)

# Create our app directory package and regenerate shortcuts
if(UNIX)
    if(NAP_BUILD_CONTEXT MATCHES "framework_release")
        configure_file(${NAP_ROOT}/tools/buildsystem/app_dir_shortcuts/unix/package.sh ${PROJECT_DIR}/package.sh @ONLY)
    endif()
    configure_file(${NAP_ROOT}/tools/buildsystem/app_dir_shortcuts/unix/regenerate.sh ${PROJECT_DIR}/regenerate.sh @ONLY)
    configure_file(${NAP_ROOT}/tools/buildsystem/app_dir_shortcuts/unix/build.sh ${PROJECT_DIR}/build.sh @ONLY)
elseif(WIN32)
    if(NAP_BUILD_CONTEXT MATCHES "framework_release")
        configure_file(${NAP_ROOT}/tools/buildsystem/app_dir_shortcuts/win64/package.bat ${PROJECT_DIR}/package.bat @ONLY)
    endif()
    configure_file(${NAP_ROOT}/tools/buildsystem/app_dir_shortcuts/win64/regenerate.bat ${PROJECT_DIR}/regenerate.bat @ONLY)
    configure_file(${NAP_ROOT}/tools/buildsystem/app_dir_shortcuts/win64/build.bat ${PROJECT_DIR}/build.bat @ONLY)
endif()

# Make a shaders directory
file(MAKE_DIRECTORY ${PROJECT_DIR}/data/shaders)

# Setup property list file on macOS
if(APPLE)
    configure_file(${TEMPLATE_ROOT}/macos/Info.plist ${PROJECT_DIR}/macos/Info.plist @ONLY)
endif()
