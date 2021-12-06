cmake_minimum_required(VERSION 3.18.4)

# Exclude for Android
if(ANDROID)
    return()
endif()

project(mod_napasio)

# add all cpp files to SOURCES
file(GLOB_RECURSE SOURCES src/*.cpp src/*.h)

# Get our NAP modules dependencies from module.json
module_json_to_cmake()

find_package(asio REQUIRED)

# LIBRARY

# compile shared lib as target
add_library(${PROJECT_NAME} SHARED ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Modules)

# Remove lib prefix on Unix libraries
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

# add include dirs
target_include_directories(${PROJECT_NAME} PUBLIC src ${ASIO_INCLUDE_DIR})

# preprocessor
target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY PUBLIC ASIO_STANDALONE)

# additional definitions
if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PUBLIC _WIN32_WINNT=0x0501)
endif()

target_link_libraries(${PROJECT_NAME} ${DEPENDENT_NAP_MODULES} napcore)

# Deploy module.json as MODULENAME.json alongside module post-build
copy_module_json_to_bin()
package_module()

# Package asio into platform release
install(FILES ${ASIO_DIST_FILES} DESTINATION thirdparty/asio)
install(DIRECTORY ${ASIO_INCLUDE_DIR} DESTINATION thirdparty/asio)
