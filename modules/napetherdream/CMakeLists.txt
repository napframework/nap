cmake_minimum_required(VERSION 3.18.4)
project(mod_napetherdream)

# add all cpp files to SOURCES
file(GLOB_RECURSE SOURCES src/*.cpp src/*.h)
filter_platform_specific_files(SOURCES)
add_platform_specific_files("${WIN32_SOURCES}" "${MACOS_SOURCES}" "${LINUX_SOURCES}")

# set_source_files_properties(${LINUX_SOURCES} PROPERTIES HEADER_FILE_ONLY FALSE)

# Get our NAP modules dependencies from module.json
module_json_to_cmake()

find_package(etherdream REQUIRED)

# LIBRARY

# compile shared lib as target
add_library(${PROJECT_NAME} SHARED ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Modules)
# Remove lib prefix on Unix libraries
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

# add include dirs
target_include_directories(${PROJECT_NAME} PUBLIC src ${ETHERDREAM_INCLUDE_DIR})

# link with external libs
target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)
target_compile_definitions(${PROJECT_NAME} PRIVATE MODULE_NAME=${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} ${DEPENDENT_NAP_MODULES} etherdreamlib)

if(WIN32)
    # Copy etherdream DLL to build directory on Windows
    if(NOT TARGET etherdreamlib)
        find_package(etherdream REQUIRED)
    endif()
    copy_etherdream_dll()
endif()

# Deploy module.json as MODULENAME.json alongside module post-build
copy_module_json_to_bin()

# Package into platform release
if(APPLE)
    set(MACOS_EXTRA_RPATH_RELEASE ../../../../thirdparty/etherdream/bin)
    # A temporary ugly fix for inter-dependent modules and their RPATHs on macOS.  NAP-225.
    list(APPEND MACOS_EXTRA_RPATH_RELEASE ../../../../thirdparty/FreeImage/lib)
    set(MACOS_EXTRA_RPATH_DEBUG ${MACOS_EXTRA_RPATH_RELEASE})
elseif(UNIX)
    set(LINUX_EXTRA_RPATH ../../../../thirdparty/etherdream/bin)
endif()
package_module()

# Package etherdreamlib into platform release
install(FILES $<TARGET_FILE:etherdreamlib> DESTINATION thirdparty/etherdream/bin)

# Package etherdreamlib headers
install(DIRECTORY ${ETHERDREAM_INCLUDE_DIR}/ DESTINATION thirdparty/etherdream/include)    
if(WIN32)
    # Package etherdream IMPLIB on Windows
    get_target_property(ETHERDREAM_IMPLIB etherdreamlib IMPORTED_IMPLIB_RELEASE)
    install(FILES ${ETHERDREAM_IMPLIB} DESTINATION thirdparty/etherdream/bin)    
endif()
