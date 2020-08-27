find_package(FFmpeg REQUIRED)
set(MODULE_NAME_EXTRA_LIBS ${FFMPEG_LIBRARIES})

if(WIN32)
    # Copy over DLLs post-build on Windows
	get_filename_component(FFMPEG_LIB_DIR ${FFMPEG_LIBAVCODEC} DIRECTORY)
    file(GLOB FFMPEG_DLLS ${FFMPEG_LIB_DIR}/../bin/*.dll)

    set(DLLCOPY_PATH_SUFFIX "")
    foreach (SINGLE_DLL ${FFMPEG_DLLS})
	    add_custom_command(
		    TARGET ${PROJECT_NAME}
		    POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${SINGLE_DLL} $<TARGET_FILE_DIR:${PROJECT_NAME}>/${DLLCOPY_PATH_SUFFIX}
		)
	endforeach()
elseif(APPLE)
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
