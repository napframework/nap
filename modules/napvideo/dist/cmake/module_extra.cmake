find_package(FFmpeg REQUIRED)
target_link_libraries(${PROJECT_NAME} ${FFMPEG_LIBRARIES})

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
    macos_add_rpath_to_module_post_build(${PROJECT_NAME} $<TARGET_FILE:${PROJECT_NAME}> ${THIRDPARTY_DIR}/FFmpeg/lib) 

    # Install FFmpeg into packaged app
    install(DIRECTORY "${THIRDPARTY_DIR}/FFmpeg/lib/" DESTINATION "lib")
elseif(UNIX)
    # Install FFmpeg into packaged app
    install(DIRECTORY "${THIRDPARTY_DIR}/FFmpeg/lib/" DESTINATION "lib")
endif()

# TODO Install FFmpeg license into packaged app
# TODO Install FFmpeg source into packaged app to comply with license?!
