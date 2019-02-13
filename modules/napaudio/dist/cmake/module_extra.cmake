include(${NAP_ROOT}/cmake/dist_shared_crossplatform.cmake)

if(NOT TARGET mpg123)
    find_package(mpg123 REQUIRED)
endif()
target_link_libraries(mod_napaudio INTERFACE mpg123)

if(NOT TARGET moodycamel)
    find_package(moodycamel REQUIRED)
endif()
add_include_to_interface_target(mod_napaudio ${MOODYCAMEL_INCLUDE_DIRS})

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

    # Install portaudio lib into packaged app
    file(GLOB PORTAUDIO_DYLIBS ${THIRDPARTY_DIR}/portaudio/lib/libport*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
    install(FILES ${PORTAUDIO_DYLIBS} DESTINATION lib)    

    # Install libsndfile into packaged app
    file(GLOB SNDFILE_DYLIBS ${THIRDPARTY_DIR}/libsndfile/lib/libsnd*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
    install(FILES ${SNDFILE_DYLIBS} DESTINATION lib)
endif()

# Install thirdparty licenses into packaged project
install(FILES ${THIRDPARTY_DIR}/portaudio/LICENSE.txt DESTINATION licenses/portaudio)
install(FILES ${THIRDPARTY_DIR}/libsndfile/COPYING DESTINATION licenses/libsndfile)
install(FILES ${THIRDPARTY_DIR}/mpg123/COPYING DESTINATION licenses/mpg123)
