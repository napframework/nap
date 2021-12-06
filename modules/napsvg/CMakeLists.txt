cmake_minimum_required(VERSION 3.18.4)
# Exclude for Android
if(ANDROID)
    return()
endif()

project(mod_napsvg)

# add all cpp files to SOURCES
file(GLOB_RECURSE SOURCES src/*.cpp src/*.h)

# Get our NAP modules dependencies from module.json
module_json_to_cmake()

# Get nanosvg
find_package(nanosvg REQUIRED)

# LIBRARY

# compile shared lib as target
add_library(${PROJECT_NAME} SHARED ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Modules)
# Remove lib prefix on Unix libraries
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

# add include dirs
target_include_directories(${PROJECT_NAME} PUBLIC src ${NANOSVG_INCLUDE_DIRS})

# preprocessor
target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY PUBLIC NANOSVG_IMPLEMENTATION PUBLIC NANOSVG_ALL_COLOR_KEYWORDS)

# link with external libs
target_link_libraries(${PROJECT_NAME} ${DEPENDENT_NAP_MODULES})

# Deploy module.json as MODULENAME.json alongside module post-build
copy_module_json_to_bin()

# Package into platform release
if(APPLE)
    # A temporary ugly fix for inter-dependent modules and their RPATHs on macOS.  NAP-225.
    set(UNIX_EXTRA_RPATH ../../../../thirdparty/FreeImage/lib)
endif()
package_module()

# Package nanosvg into platform release
install(FILES ${NANOSVG_DIR}/LICENSE.txt ${NANOSVG_DIR}/README.md DESTINATION thirdparty/nanosvg)
install(DIRECTORY ${NANOSVG_INCLUDE_DIRS}/ DESTINATION thirdparty/nanosvg/include)
