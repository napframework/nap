find_package(FFmpeg REQUIRED)

if (WIN32)
    # Copy over DLLs post-build
	get_filename_component(FFMPEG_LIB_DIR ${FFMPEG_LIBAVCODEC} DIRECTORY)
    file(GLOB FFMPEG_DLLS ${FFMPEG_LIB_DIR}/../bin/*.dll)

    foreach (SINGLE_DLL ${FFMPEG_DLLS})
	    add_custom_command(
		    TARGET ${PROJECT_NAME}
		    POST_BUILD
	        COMMAND ${CMAKE_COMMAND} -E copy ${SINGLE_DLL} $<TARGET_FILE_DIR:${PROJECT_NAME}>/
		)
	endforeach()
endif()