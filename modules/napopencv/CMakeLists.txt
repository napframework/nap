cmake_minimum_required(VERSION 3.5)

# Exclude for Android
if(ANDROID)
    return()
endif()

project(mod_napopencv)

# add all cpp files to SOURCES
file(GLOB_RECURSE SOURCES src/*.cpp src/*.h)

# Get our NAP modules dependencies from module.json
module_json_to_cmake()

# Find OpenCV
if(WIN32)
    find_package(OpenCV PATHS ${THIRDPARTY_DIR}/opencv/msvc REQUIRED) 
elseif (APPLE)
    find_package(OpenCV PATHS ${THIRDPARTY_DIR}/opencv/osx/lib/cmake/opencv4 REQUIRED) 
elseif(UNIX)
    find_package(OpenCV PATHS ${THIRDPARTY_DIR}/opencv/linux/lib/cmake/opencv4 REQUIRED) 
endif()

# compile shared lib as target
add_library(${PROJECT_NAME} SHARED ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Modules)
# Remove lib prefix on Unix libraries
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

# add include dirs
target_include_directories(${PROJECT_NAME} PUBLIC src ${OpenCV_INCLUDE_DIRS})

# preprocessor
target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)

# link libs
target_link_libraries(${PROJECT_NAME} napcore ${DEPENDENT_NAP_MODULES} ${OpenCV_LIBS})

# Deploy module.json as MODULENAME.json alongside module post-build
copy_module_json_to_bin()

# Ensure application can find library when packaged and run
if(APPLE)
    set(MACOS_EXTRA_RPATH_RELEASE ../../../../thirdparty/opencv/lib)
    list(APPEND MACOS_EXTRA_RPATH_RELEASE ../../../../thirdparty/FFmpeg/lib)
    set(MACOS_EXTRA_RPATH_DEBUG ${MACOS_EXTRA_RPATH_RELEASE})
elseif(UNIX)
    set(LINUX_EXTRA_RPATH ../../../../thirdparty/opencv/lib)
    list(APPEND LINUX_EXTRA_RPATH ../../../../thirdparty/FFmpeg/lib)
endif()

# Package oscpack into platform release
package_module()

# Install opencv directory into package
if(MSVC)
    install(DIRECTORY ${THIRDPARTY_DIR}/opencv/msvc/ DESTINATION thirdparty/opencv)
elseif(APPLE)
    install(DIRECTORY ${THIRDPARTY_DIR}/opencv/osx/ DESTINATION thirdparty/opencv)
else()
    install(DIRECTORY ${THIRDPARTY_DIR}/opencv/linux/ DESTINATION thirdparty/opencv)
endif()

# Copy over opencv dll's to build directory 
if(WIN32)
    get_target_property(__dll_dbg opencv_world IMPORTED_LOCATION_DEBUG)
    get_target_property(__dll_release opencv_world  IMPORTED_LOCATION_RELEASE)

    # copy opencv debug / release dll based on config
    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} 
                -E copy_if_different 
                "$<$<CONFIG:debug>:${__dll_dbg}>$<$<CONFIG:release>:${__dll_release}>"
                $<TARGET_FILE_DIR:${PROJECT_NAME}>)

    # copy ffmpeg for opencv
    file(GLOB CV_FFMPEG_DLLS ${THIRDPARTY_DIR}/opencv/msvc/x64/vc14/bin/opencv_videoio_ffmpeg*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
    copy_files_to_bin(${CV_FFMPEG_DLLS})
endif()

