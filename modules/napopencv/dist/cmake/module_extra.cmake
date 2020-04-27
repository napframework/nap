include(${NAP_ROOT}/cmake/dist_shared_crossplatform.cmake)

# find OpenCV package
if(NOT TARGET OpenCV)
    if(WIN32)
        find_package(OpenCV PATHS ${THIRDPARTY_DIR}/opencv REQUIRED)
    else()
        find_package(OpenCV PATHS ${THIRDPARTY_DIR}/opencv/lib/cmake/opencv4)
    endif()
endif()

# add includes
add_include_to_interface_target(mod_napopencv ${OpenCV_INCLUDE_DIRS})

# add libraries
set(MODULE_NAME_EXTRA_LIBS ${OpenCV_LIBS})

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
    file(GLOB CV_FFMPEG_DLLS ${THIRDPARTY_DIR}/opencv/x64/vc14/bin/opencv_videoio_ffmpeg*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
    copy_files_to_bin(${CV_FFMPEG_DLLS})

    # Install OpenCV license into packaged project
    install(FILES ${THIRDPARTY_DIR}/opencv/LICENSE DESTINATION licenses/opencv)

elseif(UNIX)

    # install library
    file(GLOB OPENCV_DYLIBS ${THIRDPARTY_DIR}/opencv/lib/libopencv*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
    install(FILES ${OPENCV_DYLIBS} DESTINATION lib)

    # install licenses
    install(DIRECTORY ${THIRDPARTY_DIR}/opencv/share/licenses DESTINATION licenses/opencv)
    
endif()

# Install FFmpeg with packaged app on *nix
if(APPLE)
    # Add FFmpeg RPATH to built app
    macos_add_rpath_to_module_post_build(${PROJECT_NAME} $<TARGET_FILE:${PROJECT_NAME}> ${THIRDPARTY_DIR}/FFmpeg/lib) 

    # Install FFmpeg into packaged app
    install(DIRECTORY ${THIRDPARTY_DIR}/FFmpeg/lib/ DESTINATION lib)
elseif(UNIX)
    # Install FFmpeg into packaged app
    install(DIRECTORY ${THIRDPARTY_DIR}/FFmpeg/lib/ DESTINATION lib)
endif()

# Install FFmpeg license into packaged app
install(FILES ${THIRDPARTY_DIR}/FFmpeg/COPYING.LGPLv2.1
              ${THIRDPARTY_DIR}/FFmpeg/COPYING.LGPLv3
              ${THIRDPARTY_DIR}/FFmpeg/LICENSE.md
        DESTINATION licenses/FFmpeg
        )
# Install FFmpeg source into packaged app to comply with license
install(FILES ${THIRDPARTY_DIR}/FFmpeg/ffmpeg-3.4.2.tar.xz DESTINATION licenses/FFmpeg)
