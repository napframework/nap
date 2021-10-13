cmake_minimum_required(VERSION 3.18.4)
# Exclude for Android
if(ANDROID)
    return()
endif()

project(mod_napsoem)

# add all cpp files to SOURCES
file(GLOB_RECURSE SOURCES src/*.cpp src/*.h)

# Get our NAP modules dependencies from module.json
module_json_to_cmake()

find_package(soem REQUIRED)
# LIBRARY

# compile shared lib as target
add_library(${PROJECT_NAME} SHARED ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Modules)
# Remove lib prefix on Unix libraries
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

# add include dirs
target_include_directories(${PROJECT_NAME} PUBLIC src ${SOEM_INCLUDE_DIRS})

# preprocessor
target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY PUBLIC __STDC_LIMIT_MACROS)

# link libs
target_link_libraries(${PROJECT_NAME} napcore ${DEPENDENT_NAP_MODULES} ${SOEM_LIBS})

# Deploy module.json as MODULENAME.json alongside module post-build
copy_module_json_to_bin()

# Package into platform release
package_module()

# Install readme
install(FILES ${THIRDPARTY_DIR}/soem/source/README.md DESTINATION thirdparty/soem)
install(FILES ${THIRDPARTY_DIR}/soem/source/LICENSE DESTINATION thirdparty/soem)

# Install include
install(DIRECTORY ${SOEM_DIR}/include DESTINATION thirdparty/soem)

# Install lib
install(DIRECTORY ${SOEM_DIR}/lib DESTINATION thirdparty/soem)

#install wpcap includes (only Windows)
if(WIN32)
	install(DIRECTORY ${WPCAP_DIR}/Include DESTINATION thirdparty/soem/wpcap)
	install(DIRECTORY ${WPCAP_DIR}/Lib DESTINATION thirdparty/soem/wpcap)
endif()


