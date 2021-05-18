cmake_minimum_required(VERSION 3.18.4)
# Exclude for Android
project(mod_napcolor)

# add all cpp files to SOURCES
file(GLOB SOURCES src/*.cpp)
file(GLOB HEADERS src/*.h)
source_group("headers" FILES ${HEADERS})
source_group("sources" FILES ${SOURCES})

# Get our NAP modules dependencies from module.json
module_json_to_cmake()

# compile shared lib as target
add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Modules)
# Remove lib prefix on Unix libraries
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

# add include dirs
target_include_directories(${PROJECT_NAME} PUBLIC src)

target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)
target_compile_definitions(${PROJECT_NAME} PRIVATE MODULE_NAME=${PROJECT_NAME})

# link with external libs
target_link_libraries(${PROJECT_NAME} napcore ${DEPENDENT_NAP_MODULES})

# Deploy module.json as MODULENAME.json alongside module post-build
copy_module_json_to_bin()

# Package into platform release
package_module()

# Install for fbxconverter
if (WIN32)
    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION tools/platform
            CONFIGURATIONS Release)
endif()
