cmake_minimum_required(VERSION 3.18.4)

# Exclude for Android
if(ANDROID)
    return()
endif()

project(mod_naplookingglass)

find_package(holoplaycore REQUIRED)

file(GLOB_RECURSE sources src/*.cpp src/*.h)

set(INCLUDES src ${HOLOPLAYCORE_INCLUDE_DIR})

# Get our NAP modules dependencies from module.json
module_json_to_cmake()

add_library(${PROJECT_NAME} SHARED ${sources})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Modules)
# Remove lib prefix on Unix libraries
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDES})

target_link_libraries(${PROJECT_NAME} napcore ${DEPENDENT_NAP_MODULES} optimized holoplaycore)
target_link_libraries(${PROJECT_NAME} napcore ${DEPENDENT_NAP_MODULES})

target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)
target_compile_definitions(${PROJECT_NAME} PRIVATE MODULE_NAME=${PROJECT_NAME})

# link with external libs
if(WIN32)
  target_link_libraries(${PROJECT_NAME} napcore ${DEPENDENT_NAP_MODULES} holoplaycore Ws2_32.lib)
else()
  target_link_libraries(${PROJECT_NAME} napcore ${DEPENDENT_NAP_MODULES} holoplaycore)
endif()

if(WIN32)
    # Copy DLL to build directory
    if(NOT TARGET holoplaycore)
        find_package(holoplaycore REQUIRED)
    endif()
    copy_holoplaycore_dll()
endif()

# Deploy module.json as MODULENAME.json alongside module post-build
copy_module_json_to_bin()

# Package into platform release
if(APPLE)
    set(MACOS_EXTRA_RPATH_RELEASE ../../../../thirdparty/holoplaycore/lib)
    set(MACOS_EXTRA_RPATH_DEBUG ${MACOS_EXTRA_RPATH_RELEASE})
elseif(UNIX)
    set(LINUX_EXTRA_RPATH ../../../../thirdparty/holoplaycore/lib)
endif()
package_module()

# Package holoplaycore into platform release
install(FILES ${HOLOPLAYCORE_DIR}/LICENSE.txt DESTINATION thirdparty/holoplaycore)
install(FILES ${HOLOPLAYCORE_DIR}/README.md DESTINATION thirdparty/holoplaycore)
install(FILES ${HOLOPLAYCORE_DIR}/VERSION DESTINATION thirdparty/holoplaycore)

install(DIRECTORY ${HOLOPLAYCORE_INCLUDE_DIR} DESTINATION thirdparty/holoplaycore FILES_MATCHING PATTERN "*.h")

if(WIN32)
    install(FILES ${HOLOPLAYCORE_LIB} DESTINATION thirdparty/holoplaycore/lib)
    install(FILES ${HOLOPLAYCORE_DLL} DESTINATION thirdparty/holoplaycore/bin)
elseif(UNIX)
    install(FILES ${HOLOPLAYCORE_DLL} DESTINATION thirdparty/holoplaycore/lib)
endif()
