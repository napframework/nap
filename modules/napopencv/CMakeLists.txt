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

find_package(OpenCV PATHS ${THIRDPARTY_DIR}/opencv/build REQUIRED) 

# LIBRARY

# compile shared lib as target
add_library(${PROJECT_NAME} SHARED ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Modules)

# add include dirs
target_include_directories(${PROJECT_NAME} PUBLIC src ${OpenCV_INCLUDE_DIRS})

# preprocessor
target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)

# link libs
target_link_libraries(${PROJECT_NAME} napcore ${DEPENDENT_NAP_MODULES} ${OpenCV_LIBS})

# Deploy module.json as MODULENAME.json alongside module post-build
copy_module_json_to_bin()

# Copy over Assimp to build directory
if(WIN32)
    copy_files_to_bin(${THIRDPARTY_DIR}/opencv/build/x64/vc14/bin/opencv_videoio_ffmpeg411_64.dll)
    if(CMAKE_BUILD_TYPE MATCHES Debug)
        copy_files_to_bin(${THIRDPARTY_DIR}/opencv/build/x64/vc14/bin/opencv_world411d.dll)
    else()
        copy_files_to_bin(${THIRDPARTY_DIR}/opencv/build/x64/vc14/bin/opencv_world411.dll)
    endif()
endif()

# Package oscpack into platform release
package_module()

install(FILES ${THIRDPARTY_DIR}/opencv/LICENSE.txt DESTINATION thirdparty/opencv)
install(FILES ${THIRDPARTY_DIR}/opencv/LICENSE_FFMPEG.txt DESTINATION thirdparty/opencv)
install(FILES ${THIRDPARTY_DIR}/opencv/README.md.txt DESTINATION thirdparty/opencv)
install(DIRECTORY ${THIRDPARTY_DIR}/opencv/build/include 
        DESTINATION thirdparty/opencv/include
        PATTERN "*.cpp" EXCLUDE)

if(MSVC)
    install(FILES ${THIRDPARTY_DIR}/opencv/build/x64/vc14/lib/opencv_world411.lib DESTINATION thirdparty/opencv/lib)
    install(FILES ${THIRDPARTY_DIR}/opencv/build/x64/vc14/bin/opencv_videoio_ffmpeg411_64.dll DESTINATION thirdparty/opencv/bin)
#elseif(APPLE)
#    install(DIRECTORY ${THIRDPARTY_DIR}/oscpack/osx/install/lib/ DESTINATION thirdparty/oscpack/lib)
#elseif(ANDROID)
#    install(DIRECTORY ${THIRDPARTY_DIR}/oscpack/android/install/lib/ DESTINATION thirdparty/oscpack/lib)
#else()
#    install(DIRECTORY ${THIRDPARTY_DIR}/oscpack/linux/install/lib/ DESTINATION thirdparty/oscpack/lib)
endif()
