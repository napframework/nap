include(${NAP_ROOT}/cmake/dist_shared_crossplatform.cmake)

# find OpenCV package
if(NOT TARGET OpenCV)
    if(MSCV)
        find_package(OpenCV PATHS ${THIRDPARTY_DIR}/opencv REQUIRED)
    else()
        find_package(OpenCV PATHS ${THIRDPARTY_DIR}/opencv/lib/cmake/opencv4)
    endif()
endif()
set(MODULE_NAME_EXTRA_LIBS OpenCV)

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
endif()

# Install OpenCV license into packaged project
install(FILES ${THIRDPARTY_DIR}/opencv/LICENSE DESTINATION licenses/opencv)
