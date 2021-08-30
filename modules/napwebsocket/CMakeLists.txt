cmake_minimum_required(VERSION 3.18.4)
# Exclude for Android
if(ANDROID)
    return()
endif()

project(mod_napwebsocket)

# add all cpp files to SOURCES
file(GLOB_RECURSE SOURCES src/*.cpp src/*.h)

# Get our NAP modules dependencies from module.json
module_json_to_cmake()

# Find websocketpp dependency
find_package(websocketpp REQUIRED)

# LIBRARY

# compile shared lib as target
add_library(${PROJECT_NAME} SHARED ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Modules)
# Remove lib prefix on Unix libraries
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

# add include dirs
target_include_directories(${PROJECT_NAME} PUBLIC src ${WEBSOCKETPP_INCLUDE_DIRS})

# preprocessor
target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY PUBLIC _WEBSOCKETPP_CPP11_INTERNAL_)

# link with external libs
if(NOT WIN32)
    target_compile_definitions(${PROJECT_NAME} PUBLIC HAVE_CONFIG_H)
endif()

target_link_libraries(${PROJECT_NAME} ${DEPENDENT_NAP_MODULES} napcore)

# Deploy module.json as MODULENAME.json alongside module post-build
copy_module_json_to_bin()
package_module()

# Package websocket into platform release
install(FILES ${THIRDPARTY_DIR}/websocketpp/readme.md
              ${THIRDPARTY_DIR}/websocketpp/changelog.md

        DESTINATION thirdparty/websocketpp
        )
install(DIRECTORY ${WEBSOCKETPP_INCLUDE_DIRS}/ DESTINATION thirdparty/websocketpp/include)
