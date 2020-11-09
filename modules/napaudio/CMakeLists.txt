cmake_minimum_required(VERSION 3.18.4)
# Exclude for Android
if(ANDROID)
    return()
endif()

project(mod_napaudio)

find_package(portaudio REQUIRED)
find_package(libsndfile REQUIRED)
find_package(libmpg123 REQUIRED)

file(GLOB_RECURSE sources src/*.cpp src/*.h)

source_group("core" src/audio/core/*.*)
source_group("node" src/audio/node/*.*)
source_group("component" src/audio/component/*.*)
source_group("resource" src/audio/resource/*.*)
source_group("service" src/audio/service/*.*)
source_group("utility" src/audio/utility/*.*)

# Get our NAP modules dependencies from module.json
module_json_to_cmake()

set(LIBRARIES napcore portaudio libsndfile libmpg123)
if(APPLE)
    list(APPEND LIBRARIES "-framework CoreAudio" "-framework CoreServices" "-framework CoreFoundation" "-framework AudioUnit" "-framework AudioToolbox")
elseif(UNIX)
    list(APPEND LIBRARIES atomic)
endif()

set(INCLUDES src ${PORTAUDIO_INCLUDE_DIR} ${LIBSNDFILE_INCLUDE_DIR} ${LIBMPG123_INCLUDE_DIR})

add_library(${PROJECT_NAME} SHARED ${sources})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Modules)
# Remove lib prefix on Unix libraries
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDES})
target_link_libraries(${PROJECT_NAME} ${LIBRARIES} ${DEPENDENT_NAP_MODULES})

target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)
target_compile_definitions(${PROJECT_NAME} PRIVATE _USE_MATH_DEFINES)

# Copy over DLLs on Windows
if(WIN32)
    if(NOT DEFINED LIBSNDFILE_LIB_DIR)
        find_package(libsndfile REQUIRED)
    endif()

    if(NOT DEFINED PORTAUDIO_LIB_DIR)
        find_package(portaudio REQUIRED)
    endif()

    if(NOT DEFINED LIBMPG123_LIB_DIR)
        find_package(libmpg123 REQUIRED)
    endif()

    # Copy audio DLLs to project build directory
    set(FILES_TO_COPY
        ${LIBSNDFILE_LIB_DIR}/libsndfile-1.dll
        ${PORTAUDIO_LIB_DIR}/portaudio_x64.dll
        ${LIBMPG123_LIB_DIR}/libmpg123.dll
        )

    copy_files_to_bin(${FILES_TO_COPY})
endif()

# Deploy module.json as MODULENAME.json alongside module post-build
copy_module_json_to_bin()

# Package into platform release
if(APPLE)
    set(MACOS_EXTRA_RPATH_RELEASE ../../../../thirdparty/mpg123/lib)
    list(APPEND MACOS_EXTRA_RPATH_RELEASE ../../../../thirdparty/portaudio/lib)
    list(APPEND MACOS_EXTRA_RPATH_RELEASE ../../../../thirdparty/libsndfile/lib)
    set(MACOS_EXTRA_RPATH_DEBUG ${MACOS_EXTRA_RPATH_RELEASE})
elseif(UNIX)
    set(LINUX_EXTRA_RPATH ../../../../thirdparty/mpg123/lib)
    list(APPEND LINUX_EXTRA_RPATH ../../../../thirdparty/portaudio/lib)
    list(APPEND LINUX_EXTRA_RPATH ../../../../thirdparty/libsndfile/lib)
endif()
package_module()

# Package mpg123 into platform release
install(FILES ${THIRDPARTY_DIR}/mpg123/COPYING DESTINATION thirdparty/mpg123)
install(DIRECTORY ${LIBMPG123_INCLUDE_DIR}/ DESTINATION thirdparty/mpg123/include)
if(WIN32)
    file(GLOB MPG123_DYLIBS ${THIRDPARTY_DIR}/mpg123/install/msvc/*.dll)
    install(FILES ${MPG123_DYLIBS}
            DESTINATION thirdparty/mpg123/bin)
    file(GLOB MPG123_HEADERS ${THIRDPARTY_DIR}/mpg123/install/msvc/*.h)
    install(FILES ${MPG123_HEADERS}
            DESTINATION thirdparty/mpg123/include)
    file(GLOB MPG123_LIBFILES ${THIRDPARTY_DIR}/mpg123/install/msvc/*.lib)
    install(FILES ${MPG123_LIBFILES}
            DESTINATION thirdparty/mpg123/lib)
elseif(APPLE)
    # Ensure our mpg123 install name id is set properly, this is really for intall into packaging
    add_custom_command(TARGET ${PROJECT_NAME}
                       PRE_BUILD
                       COMMAND ${CMAKE_INSTALL_NAME_TOOL} -id @rpath/libmpg123.dylib ${LIBMPG123_LIB_DIR}/libmpg123.0.dylib
                       )

    file(GLOB MPG123_DYLIBS ${LIBMPG123_LIB_DIR}/libmpg*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
    install(FILES ${MPG123_DYLIBS}
            DESTINATION thirdparty/mpg123/lib)

    foreach(build_type Release Debug)
        ensure_macos_file_has_rpath_at_install(${CMAKE_INSTALL_PREFIX}/modules/mod_napaudio/lib/${build_type}/libmod_napaudio.dylib
                                               "@loader_path/../../../../thirdparty/mpg123/lib")
    endforeach()
elseif(ANDROID)
    install(DIRECTORY ${LIBMPG123_DIR}/install/android/lib/
            DESTINATION thirdparty/mpg123/lib
            )
elseif(UNIX)
    file(GLOB MPG123_DYLIBS ${LIBMPG123_LIB_DIR}/libmpg123*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
    install(FILES ${MPG123_DYLIBS}
            DESTINATION thirdparty/mpg123/lib)
endif()

# Package libsndfile into platform release
install(FILES ${THIRDPARTY_DIR}/libsndfile/linux/install/COPYING DESTINATION thirdparty/libsndfile)
install(FILES ${LIBSNDFILE_INCLUDE_DIR}/sndfile.h
              ${LIBSNDFILE_INCLUDE_DIR}/sndfile.hh
        DESTINATION thirdparty/libsndfile/include)
if(WIN32)
    file(GLOB LIBSNDFILE_DYLIBS ${LIBSNDFILE_LIB_DIR}/libsnd*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
    install(FILES ${LIBSNDFILE_DYLIBS}
            DESTINATION thirdparty/libsndfile/bin)
    file(GLOB LIBSNDFILE_IMPLIBS ${LIBSNDFILE_LIB_DIR}/libsnd*.lib)
    install(FILES ${LIBSNDFILE_IMPLIBS}
            DESTINATION thirdparty/libsndfile/lib)
elseif(ANDROID)
    install(DIRECTORY ${LIBSNDFILE_DIR}/android/install/lib/
            DESTINATION thirdparty/libsndfile/lib
            )
elseif(UNIX)
    file(GLOB LIBSNDFILE_DYLIBS ${LIBSNDFILE_LIB_DIR}/libsnd*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
    install(FILES ${LIBSNDFILE_DYLIBS}
            DESTINATION thirdparty/libsndfile/lib)
endif()

# Package portaudio into platform release
install(FILES ${THIRDPARTY_DIR}/portaudio/linux/LICENSE.txt DESTINATION thirdparty/portaudio)
install(DIRECTORY ${PORTAUDIO_INCLUDE_DIR}/ DESTINATION thirdparty/portaudio/include)
if(WIN32)
    install(FILES ${PORTAUDIO_LIBRARIES}
            DESTINATION thirdparty/portaudio/lib)
    install(FILES ${PORTAUDIO_LIBS_RELEASE_DLL}
            DESTINATION thirdparty/portaudio/bin)
elseif(ANDROID)
    install(DIRECTORY ${PORTAUDIO_DIR}/android/lib/
            DESTINATION thirdparty/portaudio/lib
            )
elseif(UNIX)
    file(GLOB PORTAUDIO_DYLIBS ${PORTAUDIO_LIB_DIR}/libport*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
    install(FILES ${PORTAUDIO_DYLIBS}
            DESTINATION thirdparty/portaudio/lib)
endif()
