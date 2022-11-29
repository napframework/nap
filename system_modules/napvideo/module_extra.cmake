set(FFMPEG_FIND_QUIETLY TRUE)
find_package(FFmpeg REQUIRED)

if(NAP_BUILD_CONTEXT MATCHES "source")
    target_include_directories(${PROJECT_NAME} PUBLIC ${FFMPEG_INCLUDE_DIR})

    target_link_libraries(${PROJECT_NAME} ${FFMPEG_LIBRARIES})

    if(WIN32)
        # Copy FFmpeg DLLs to build directory
        file(GLOB FFMPEGDLLS ${THIRDPARTY_DIR}/ffmpeg/msvc/x86_64/bin/*.dll)
        copy_files_to_bin(${FFMPEGDLLS})
    endif()

    ## Package into platform release

    # Package dependencies into release --
    install(DIRECTORY ${FFMPEG_LICENSE_DIR}/ DESTINATION thirdparty/FFmpeg)
    install(FILES ${FFMPEG_DIST_FILES} DESTINATION thirdparty/FFmpeg)
    set(FFMPEG_DIR ${FFMPEG_INCLUDE_DIR}/..)
    install(DIRECTORY ${FFMPEG_DIR}/ DESTINATION thirdparty/FFmpeg)

    if(APPLE)
        # Add RPATH for FFmpeg to packaged module on macOS
        foreach(build_type Release Debug)
            # Errors are ignored as duplicate RPATHs are possible
            install(CODE "execute_process(COMMAND ${CMAKE_INSTALL_NAME_TOOL}
                                                  -add_rpath
                                                  @loader_path/../../../../thirdparty/FFmpeg/lib
                                                  ${CMAKE_INSTALL_PREFIX}/modules/mod_napvideo/lib/${build_type}/mod_napvideo.dylib                                           
                                          ERROR_QUIET
                                          )")
        endforeach()
    elseif(UNIX)
        # Let our installed FFmpeg libs find each other
        install(CODE "file(GLOB FFMPEG_DYLIBS ${CMAKE_INSTALL_PREFIX}/thirdparty/FFmpeg/lib/lib*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
                      foreach(FFMPEG_DYLIB \${FFMPEG_DYLIBS})
                          execute_process(COMMAND patchelf --set-rpath \$ORIGIN/. \${FFMPEG_DYLIB}
                                          RESULT_VARIABLE EXIT_CODE
                                          )
                          if(NOT \${EXIT_CODE} EQUAL 0)
                              message(FATAL_ERROR \"Failed to add inter-FFmpeg RPATHs on ${FFMPEG_DYLIB}\")
                          endif()
                      endforeach()
                      ")
    endif()
else()
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
    elseif(UNIX)
        if(APPLE)
            # Add FFmpeg RPATH to built app
            macos_add_rpath_to_module_post_build(${PROJECT_NAME} $<TARGET_FILE:${PROJECT_NAME}> ${THIRDPARTY_DIR}/FFmpeg/lib) 
        endif()

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
endif()
