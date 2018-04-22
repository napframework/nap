if(NOT TARGET mpg123)
    find_package(mpg123 REQUIRED)
endif()
target_link_libraries(${PROJECT_NAME} mpg123)

if(WIN32)
    # Add post-build step to set copy mpg123 to bin on Win64
    add_custom_command(TARGET ${PROJECT_NAME}
                       POST_BUILD
                       COMMAND ${CMAKE_COMMAND} 
                               -E copy
                               $<TARGET_FILE:mpg123>
                               $<TARGET_FILE_DIR:${PROJECT_NAME}> 
                       )

    # Copy libsndfile to bin post-build on Win64
    add_custom_command(TARGET ${PROJECT_NAME}
                       POST_BUILD
                       COMMAND ${CMAKE_COMMAND} 
                               -E copy
                               ${THIRDPARTY_DIR}/libsndfile/bin/libsndfile-1.dll
                               $<TARGET_FILE_DIR:${PROJECT_NAME}> 
                       )

    # Copy portaudio to bin post-build on Win64
    add_custom_command(TARGET ${PROJECT_NAME}
                       POST_BUILD
                       COMMAND ${CMAKE_COMMAND} 
                               -E copy
                               ${THIRDPARTY_DIR}/portaudio/bin/portaudio_x64.dll
                               $<TARGET_FILE_DIR:${PROJECT_NAME}> 
                       )
elseif(UNIX)
    # Install mpg123 lib into packaged app
    file(GLOB MPG123_DYLIBS ${THIRDPARTY_DIR}/mpg123/lib/libmpg*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
    install(FILES ${MPG123_DYLIBS} DESTINATION lib)

    if(APPLE)
        # Add mpg123 RPATH to built app
        macos_add_rpath_to_module_post_build(${PROJECT_NAME} $<TARGET_FILE:${PROJECT_NAME}> ${THIRDPARTY_DIR}/mpg123/bin) 

        # mpg123 link isn't working unless we get the symlink
        install(FILES $<TARGET_FILE_DIR:mpg123>/libmpg123.dylib DESTINATION lib)        
    endif()
endif()