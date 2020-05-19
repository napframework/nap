# Exclude for Android
if(ANDROID)
    return()
endif()

project(mod_napimgui)

# add all cpp files to SOURCES
file(GLOB SOURCES src/*.cpp)
file(GLOB HEADERS src/*.h)
file(GLOB_RECURSE GUI src/imgui/*.cpp src/imgui/*.h)

source_group("headers" FILES ${HEADERS})
source_group("sources" FILES ${SOURCES})
source_group("imgui" FILES ${GUI})

# Get our NAP modules dependencies from module.json
module_json_to_cmake()

# compile shared lib as target
add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS} ${GUI})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Modules)
# Remove lib prefix on Unix libraries
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

# add include dirs
target_include_directories(${PROJECT_NAME} PUBLIC src)

# preprocessors
target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)

# link libs
target_link_libraries(${PROJECT_NAME} ${DEPENDENT_NAP_MODULES})

# Deploy module.json as MODULENAME.json alongside module post-build
copy_module_json_to_bin()

# Package into platform release
if(APPLE)
    set(MACOS_EXTRA_RPATH_RELEASE ../../../../thirdparty/glew/lib)
    list(APPEND MACOS_EXTRA_RPATH_RELEASE ../../../../thirdparty/SDL2/lib)
    # A temporary ugly fix for inter-dependent modules and their RPATHs on macOS. NAP-225.
    list(APPEND MACOS_EXTRA_RPATH_RELEASE ../../../../thirdparty/FreeImage/lib)
    set(MACOS_EXTRA_RPATH_DEBUG ${MACOS_EXTRA_RPATH_RELEASE})
elseif(UNIX)
    set(LINUX_EXTRA_RPATH ../../../../thirdparty/glew/lib)
    list(APPEND LINUX_EXTRA_RPATH ../../../../thirdparty/SDL2/lib)
endif()    
package_module()
