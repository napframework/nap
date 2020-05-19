# Exclude for Android
if(ANDROID)
    return()
endif()

project(mod_napvideo)

# add all cpp files to SOURCES
file(GLOB SOURCES src/*.cpp)
file(GLOB HEADERS src/*.h)
source_group("headers" FILES ${HEADERS})
source_group("sources" FILES ${SOURCES})

# Get our NAP modules dependencies from module.json
module_json_to_cmake()

set(FFMPEG_FIND_QUIETLY TRUE)
find_package(FFmpeg REQUIRED)

add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Modules)
# Remove lib prefix on Unix libraries
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

target_include_directories(${PROJECT_NAME} PUBLIC src ${FFMPEG_INCLUDE_DIR})

target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)
target_link_libraries(${PROJECT_NAME} napcore ${DEPENDENT_NAP_MODULES} ${FFMPEG_LIBRARIES})

if(WIN32)
    # Copy FFmpeg DLLs to build directory
    copy_windows_ffmpeg_dlls()
endif()

# Deploy module.json as MODULENAME.json alongside module post-build
copy_module_json_to_bin()

# Package into platform release
if(APPLE)
    set(MACOS_EXTRA_RPATH_RELEASE ../../../../thirdparty/FFmpeg/lib)
    # A temporary ugly fix for inter-dependent modules and their RPATHs on macOS.  NAP-225.
    list(APPEND MACOS_EXTRA_RPATH_RELEASE ../../../../thirdparty/portaudio/lib)
    list(APPEND MACOS_EXTRA_RPATH_RELEASE ../../../../thirdparty/libsndfile/lib)
    set(MACOS_EXTRA_RPATH_DEBUG ${MACOS_EXTRA_RPATH_RELEASE})
elseif(UNIX)
    set(LINUX_EXTRA_RPATH ../../../../thirdparty/FFmpeg/lib)
endif()
package_module()

# Package dependent into release --

# FFmpeg
# Licenses
install(DIRECTORY ${THIRDPARTY_DIR}/ffmpeg/licenses/ DESTINATION thirdparty/FFmpeg)
# Source, as required by their license
install(FILES ${THIRDPARTY_DIR}/ffmpeg/ffmpeg-3.4.2.tar.xz DESTINATION thirdparty/FFmpeg)
if(WIN32)
    install(DIRECTORY ${THIRDPARTY_DIR}/ffmpeg/msvc/install/ DESTINATION thirdparty/FFmpeg
            PATTERN "osx" EXCLUDE
            PATTERN "linux" EXCLUDE)
elseif(APPLE)
    # Install FFmpeg into release
    install(DIRECTORY ${THIRDPARTY_DIR}/ffmpeg/osx/install/ DESTINATION thirdparty/FFmpeg)

    # Add RPATH for FFmpeg to packaged module on macOS
    foreach(build_type Release Debug)
        install(CODE "execute_process(COMMAND ${CMAKE_INSTALL_NAME_TOOL}
                                              -add_rpath
                                              @loader_path/../../../../thirdparty/FFmpeg/lib
                                              ${CMAKE_INSTALL_PREFIX}/modules/mod_napvideo/lib/${build_type}/libmod_napvideo.dylib                                           
                                      ERROR_QUIET
                                      )")
    endforeach()
elseif(UNIX)
    # Install FFmpeg into release
    install(DIRECTORY ${THIRDPARTY_DIR}/ffmpeg/linux/install/ DESTINATION thirdparty/FFmpeg)

    # Let our installed FFmpeg libs find each other
    install(CODE "file(GLOB FFMPEG_DYLIBS ${CMAKE_INSTALL_PREFIX}/thirdparty/FFmpeg/lib/lib*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
                  foreach(FFMPEG_DYLIB \${FFMPEG_DYLIBS})
                      execute_process(COMMAND patchelf --set-rpath \$ORIGIN/. \${FFMPEG_DYLIB})
                  endforeach()
                  ")
endif()
