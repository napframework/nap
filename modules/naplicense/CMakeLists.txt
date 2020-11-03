# Exclude for Android
if(ANDROID)
    return()
endif()

project(mod_naplicense)

# add all cpp files to SOURCES
file(GLOB_RECURSE SOURCES src/*.cpp src/*.h)

# Get our NAP modules dependencies from module.json
module_json_to_cmake()

# LIBRARY
find_package(cryptopp REQUIRED)

# compile shared lib as target
add_library(${PROJECT_NAME} SHARED ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Modules)
# Remove lib prefix on Unix libraries
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

# add include dirs
target_include_directories(${PROJECT_NAME} PUBLIC src ${CRYPTOPP_INCLUDE_DIRS})

# preprocessor
target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)

# link with external libs
if(NOT WIN32)
	target_compile_definitions(${PROJECT_NAME} PUBLIC HAVE_CONFIG_H)
endif()
target_link_libraries(${PROJECT_NAME} napcore ${DEPENDENT_NAP_MODULES} cryptopp)

# copy crypto dll to bin directory
if (WIN32)
  copy_cryptopp_dll()
endif()

# Deploy module.json as MODULENAME.json alongside module post-build
copy_module_json_to_bin()

# Package into platform release
package_module()

# Package cryptopp into platform release
if(WIN32)
    install(DIRECTORY ${THIRDPARTY_DIR}/cryptopp/msvc/lib DESTINATION thirdparty/cryptopp 
    	FILES_MATCHING PATTERN "*.dll")
endif()

# install crypto pp license
install(FILES ${THIRDPARTY_DIR}/cryptopp/License.txt DESTINATION thirdparty/cryptopp)
