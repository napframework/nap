find_package(FFmpeg REQUIRED)

if(WIN32)
    # Copy over DLLs post-build on Windows
	get_filename_component(FFMPEG_LIB_DIR ${FFMPEG_LIBAVCODEC} DIRECTORY)
    file(GLOB FFMPEG_DLLS ${FFMPEG_LIB_DIR}/../bin/*.dll)

    foreach (SINGLE_DLL ${FFMPEG_DLLS})
	    add_custom_command(
		    TARGET ${PROJECT_NAME}
		    POST_BUILD
	        COMMAND ${CMAKE_COMMAND} -E copy ${SINGLE_DLL} $<TARGET_FILE_DIR:${PROJECT_NAME}>/
		)
	endforeach()
elseif(APPLE)
    # Add FFmpeg RPATH to built app
    add_custom_command(TARGET ${PROJECT_NAME}
                       POST_BUILD
                       COMMAND sh -c \"${CMAKE_INSTALL_NAME_TOOL} -add_rpath ${THIRDPARTY_DIR}/FFmpeg/lib $<TARGET_FILE:${PROJECT_NAME}> 2>/dev/null\;exit 0\"
                       )

    # Install FFmpeg into packaged app
    install(DIRECTORY "${THIRDPARTY_DIR}/FFmpeg/lib/" DESTINATION "lib")
endif()