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

if(WIN32)
    find_package(OpenCV PATHS ${THIRDPARTY_DIR}/opencv/msvc/install REQUIRED) 
endif()

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
    copy_files_to_bin(${THIRDPARTY_DIR}/opencv/msvc/install/x64/vc14/bin/opencv_videoio_ffmpeg412_64.dll)
endif()

# Package oscpack into platform release
package_module()

# Install opencv into package
if(MSVC)
    install(DIRECTORY ${THIRDPARTY_DIR}/opencv/msvc/install/ DESTINATION thirdparty/opencv)
elseif(APPLE)
    install(DIRECTORY ${THIRDPARTY_DIR}/opencv/osx/ DESTINATION thirdparty/opencv)
else()
    install(DIRECTORY ${THIRDPARTY_DIR}/opencv/linux/ DESTINATION thirdparty/opencv)
endif()

