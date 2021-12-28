cmake_minimum_required(VERSION 3.18.4)
project(mod_napsequenceaudio)

# add all cpp files to SOURCES
file(GLOB_RECURSE SOURCES src/*.cpp src/*.h)

# Get our NAP modules dependencies from module.json
module_json_to_cmake()

# LIBRARY

# compile shared lib as target
add_library(${PROJECT_NAME} SHARED ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Modules)

# Remove lib prefix on Unix libraries
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

# add include dirs
target_include_directories(${PROJECT_NAME} PUBLIC src)

# preprocessor
target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)

# link with external libs
if(NOT WIN32)
	target_compile_definitions(${PROJECT_NAME} PUBLIC HAVE_CONFIG_H)
endif()

target_link_libraries(${PROJECT_NAME} ${DEPENDENT_NAP_MODULES} napcore)

# Deploy module.json as MODULENAME.json alongside module post-build
copy_module_json_to_bin()

# Package into platform release
if(APPLE)
	# A temporary ugly fix for inter-dependent modules and their RPATHs on macOS. NAP-225.
    set(MACOS_EXTRA_RPATH_RELEASE ../../../../thirdparty/mpg123/lib)
    list(APPEND MACOS_EXTRA_RPATH_RELEASE ../../../../thirdparty/portaudio/lib)
    list(APPEND MACOS_EXTRA_RPATH_RELEASE ../../../../thirdparty/libsndfile/lib)
    set(MACOS_EXTRA_RPATH_DEBUG ${MACOS_EXTRA_RPATH_RELEASE})
elseif(UNIX)
    set(LINUX_EXTRA_RPATH ../../../../thirdparty/mpg123/lib)
    list(APPEND LINUX_EXTRA_RPATH ../../../../thirdparty/portaudio/lib)
    list(APPEND LINUX_EXTRA_RPATH ../../../../thirdparty/libsndfile/lib)
endif()

# Package into platform release
package_module()
