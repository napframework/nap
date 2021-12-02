cmake_minimum_required(VERSION 3.18.4)
# Exclude for Android
if(ANDROID)
    return()
endif()

project(mod_napfont)

# add all cpp files to SOURCES
file(GLOB_RECURSE SOURCES src/*.cpp src/*.h)

# Get our NAP modules dependencies from module.json
module_json_to_cmake()

find_package(freetype REQUIRED)

# LIBRARY

# compile shared lib as target
add_library(${PROJECT_NAME} SHARED ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Modules)
# Remove lib prefix on Unix libraries
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

# add include dirs
target_include_directories(${PROJECT_NAME} PUBLIC src ${FREETYPE_INCLUDE_DIR})

# preprocessor
target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)

# link with external libs
target_link_libraries(${PROJECT_NAME} ${DEPENDENT_NAP_MODULES} freetype)

# Copy freetype DLL
if (WIN32)
    copy_freetype_dll()
endif()

# Deploy module.json as MODULENAME.json alongside module post-build
copy_module_json_to_bin()

# Package into platform release
if(APPLE)
    set(MACOS_EXTRA_RPATH_RELEASE ../../../../thirdparty/freetype/lib)
    set(MACOS_EXTRA_RPATH_DEBUG ${MACOS_EXTRA_RPATH_RELEASE})    
elseif(UNIX)
    set(LINUX_EXTRA_RPATH ../../../../thirdparty/freetype/lib)
endif()
package_module()

# Package freetype into platform release
install(FILES ${FREETYPE_DIST_FILES} DESTINATION thirdparty/freetype)

install(DIRECTORY ${FREETYPE_INCLUDE_DIR}/ DESTINATION thirdparty/freetype/include
        FILES_MATCHING PATTERN "*.h")

if(MSVC)
    install(FILES ${FREETYPE_LIBS_RELEASE} ${FREETYPE_DLL_RELEASE} DESTINATION thirdparty/freetype/bin/release)
    install(FILES ${FREETYPE_LIBS_DEBUG} ${FREETYPE_DLL_DEBUG} DESTINATION thirdparty/freetype/bin/debug)
elseif(APPLE)
    file(GLOB RTFREETYPE_LIBS ${FREETYPE_LIBS_DIR}/libfreetype*${CMAKE_SHARED_LIBRARY_SUFFIX})
    install(FILES ${RTFREETYPE_LIBS}
            DESTINATION thirdparty/freetype/lib)
elseif(UNIX)
    file(GLOB RTFREETYPE_LIBS ${FREETYPE_LIBS_DIR}/libfreetype${CMAKE_SHARED_LIBRARY_SUFFIX}*)
    install(FILES ${RTFREETYPE_LIBS}
            DESTINATION thirdparty/freetype/lib)
endif()

if (WIN32)
    # Install for fbxconverter
    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION tools/platform
        CONFIGURATIONS Release)
endif()
