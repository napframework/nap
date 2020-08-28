# Exclude for Android
if(ANDROID)
    return()
endif()

project(mod_naprender)

# add all cpp files to SOURCES
file(GLOB SOURCES src/*.cpp)
file(GLOB HEADERS src/*.h)
source_group("headers" FILES ${HEADERS})
source_group("sources" FILES ${SOURCES})

# Get our NAP modules dependencies from module.json
module_json_to_cmake()

# LIBRARY

# compile shared lib as target
add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Modules)
# Remove lib prefix on Unix libraries
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

# add include dirs
target_include_directories(${PROJECT_NAME} PUBLIC ${NRENDER_INCLUDE_DIRS} src)

# link with external libs
target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY _USE_MATH_DEFINES)
target_compile_definitions(${PROJECT_NAME} PRIVATE MODULE_NAME=${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} nrender ${DEPENDENT_NAP_MODULES})

# Deploy module.json as MODULENAME.json alongside module post-build
copy_module_json_to_bin()

# Add GLEW macOS RPATH for Source context
if(APPLE)
    add_macos_glew_rpath()
endif()

# Package into platform release
if(APPLE)
    set(MACOS_EXTRA_RPATH_RELEASE ../../../../thirdparty/glew/lib)
    list(APPEND MACOS_EXTRA_RPATH_RELEASE ../../../../thirdparty/SDL2/lib)
    list(APPEND MACOS_EXTRA_RPATH_RELEASE ../../../../thirdparty/assimp/lib)
    list(APPEND MACOS_EXTRA_RPATH_RELEASE ../../../../thirdparty/FreeImage/lib)
    list(APPEND MACOS_EXTRA_RPATH_RELEASE ../../../../thirdparty/freetype/lib)
    set(MACOS_EXTRA_RPATH_DEBUG ${MACOS_EXTRA_RPATH_RELEASE})
elseif(UNIX)
    set(LINUX_EXTRA_RPATH ../../../../thirdparty/glew/lib)
    list(APPEND LINUX_EXTRA_RPATH ../../../../thirdparty/SDL2/lib)
    list(APPEND LINUX_EXTRA_RPATH ../../../../thirdparty/assimp/lib)
    list(APPEND LINUX_EXTRA_RPATH ../../../../thirdparty/FreeImage/lib)
    list(APPEND LINUX_EXTRA_RPATH ../../../../thirdparty/freetype/lib)
endif()
package_module()

if (WIN32)
    # Install for fbxconverter
    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION tools/platform
            CONFIGURATIONS Release)
endif()
