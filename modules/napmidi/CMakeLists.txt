cmake_minimum_required(VERSION 3.18.4)
# Exclude for Android
if(ANDROID)
    return()
endif()

project(mod_napmidi)

find_package(rtmidi REQUIRED)

file(GLOB_RECURSE sources src/*.cpp src/*.h)

set(INCLUDES src ${RTMIDI_INCLUDE_DIR})

# Get our NAP modules dependencies from module.json
module_json_to_cmake()

add_library(${PROJECT_NAME} SHARED ${sources})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Modules)
# Remove lib prefix on Unix libraries
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDES})

target_link_libraries(${PROJECT_NAME} napcore ${DEPENDENT_NAP_MODULES} debug ${RTMIDI_LIBRARIES_DEBUG} optimized ${RTMIDI_LIBRARIES_RELEASE} )

target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)

# Deploy module.json as MODULENAME.json alongside module post-build
copy_module_json_to_bin()

# Package into platform release
if(APPLE)
    set(MACOS_EXTRA_RPATH_RELEASE ../../../../thirdparty/rtmidi/lib)
    set(MACOS_EXTRA_RPATH_DEBUG ${MACOS_EXTRA_RPATH_RELEASE})
elseif(UNIX)
    set(LINUX_EXTRA_RPATH ../../../../thirdparty/rtmidi/lib)
endif()
package_module()

# Package rtmidi into platform release
install(FILES ${THIRDPARTY_DIR}/rtmidi/linux/x86_64/README.md DESTINATION thirdparty/rtmidi)
install(DIRECTORY ${THIRDPARTY_DIR}/rtmidi/linux/x86_64/include/rtmidi/ DESTINATION thirdparty/rtmidi/include)
if(MSVC)
    file(GLOB RTMIDI_STATIC_LIBS ${THIRDPARTY_DIR}/rtmidi/msvc/x86_64/rtmidi*${CMAKE_STATIC_LIBRARY_SUFFIX})
    install(FILES ${RTMIDI_STATIC_LIBS}
            DESTINATION thirdparty/rtmidi/bin)
elseif(APPLE)
    install(FILES ${THIRDPARTY_DIR}/rtmidi/macos/x86_64/librtmidi.4.dylib
            DESTINATION thirdparty/rtmidi/lib)
elseif(UNIX)
    file(GLOB RTMIDI_DYLIBS ${RTMIDI_DIR}/linux/${ARCH}/lib/librtmidi${CMAKE_SHARED_LIBRARY_SUFFIX}*)
    install(FILES ${RTMIDI_DYLIBS}
            DESTINATION thirdparty/rtmidi/lib)
endif()
