# Set this flag in order to link libsndfile and libsamplerate and build support for reading audio files
set(NAP_AUDIOFILE_SUPPORT ON)

if(NAP_AUDIOFILE_SUPPORT)
    if(NOT TARGET libsndfile)
        find_package(libsndfile REQUIRED)
    endif()

    if(NOT TARGET libsamplerate)
        find_package(libsamplerate REQUIRED)
    endif()

    if(NOT TARGET libflac)
        find_package(libflac REQUIRED)
    endif()
    if(NOT TARGET libmp3lame)
        find_package(libmp3lame REQUIRED)
    endif()
    if(NOT TARGET libmpg123)
        find_package(libmpg123 REQUIRED)
    endif()
    if(NOT TARGET libogg)
        find_package(libogg REQUIRED)
    endif()
    if(NOT TARGET libopus)
        find_package(libopus REQUIRED)
    endif()
    if(NOT TARGET libvorbis)
        find_package(libvorbis REQUIRED)
    endif()

endif()


# Add sources to target
if(NAP_BUILD_CONTEXT MATCHES "source")
    if (NAP_AUDIOFILE_SUPPORT)
        # Add compile definition to enable audio file support
        target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_AUDIOFILE_SUPPORT)
    else()
        # Filter out sources for audio file functionality
        set(AUDIO_FILE_SUPPORT_FILTER ".*audiofileutils.*" ".*audiofileresource.*")
    endif()

    add_source_dir("core" "src/audio/core")
    add_source_dir("node" "src/audio/node")
    add_source_dir("component" "src/audio/component")
    add_source_dir("service" "src/audio/service")
    add_source_dir("resource" "src/audio/resource" ${AUDIO_FILE_SUPPORT_FILTER})
    add_source_dir("utility" "src/audio/utility" ${AUDIO_FILE_SUPPORT_FILTER})
endif()

set(SNDFILE_DEPENDENCIES libflac libmp3lame libmpg123 libogg libopus libvorbis)


# Add thirdparty libraries for audio file support
if (NAP_AUDIOFILE_SUPPORT)
    if(NAP_BUILD_CONTEXT MATCHES "source")
        set(LIBRARIES libsndfile libsamplerate libflac libmp3lame libmpg123 libogg libopus libvorbis)
        if(UNIX)
            list(APPEND LIBRARIES atomic)
        endif()

        set(INCLUDES ${LIBSNDFILE_INCLUDE_DIR} ${LIBSAMPLERATE_INCLUDE_DIR} ${LIBFLAC_INCLUDE_DIR} ${LIBMP3LAME_INCLUDE_DIR} ${LIBMPG123_INCLUDE_DIR} ${LIBOGG_INCLUDE_DIR} ${LIBOPUS_INCLUDE_DIR} ${LIBVORBIS_INCLUDE_DIR})

        target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDES})
        target_link_libraries(${PROJECT_NAME} ${LIBRARIES})
        target_compile_definitions(${PROJECT_NAME} PRIVATE _USE_MATH_DEFINES)

        # Copy over DLLs on Windows
        if(WIN32)
            if(NOT DEFINED LIBSNDFILE_LIB_DIR)
                find_package(libsndfile REQUIRED)
            endif()

            if(NOT DEFINED LIBSAMPLERATE_LIB_DIR)
                find_package(libsamplerate REQUIRED)
            endif()

            if(NOT DEFINED LIBFLAC_LIB_DIR)
                find_package(libflac REQUIRED)
            endif()
            if(NOT DEFINED LIBMP3LAME_LIB_DIR)
                find_package(libmp3lame REQUIRED)
            endif()
            if(NOT DEFINED LIBMPG123_LIB_DIR)
                find_package(libmpg123 REQUIRED)
            endif()
            if(NOT DEFINED LIBOGG_LIB_DIR)
                find_package(libogg REQUIRED)
            endif()
            if(NOT DEFINED LIBOPUS_LIB_DIR)
                find_package(libopus REQUIRED)
            endif()
            if(NOT DEFINED LIBVORBIS_LIB_DIR)
                find_package(libvorbis REQUIRED)
            endif()



            # Copy audio DLLs to project build directory
            set(FILES_TO_COPY
                ${LIBSNDFILE_LIBS_RELEASE_DLL}
                ${LIBSAMPLERATE_LIBS_RELEASE_DLL}
                ${LIBFLAC_LIBS_RELEASE_DLL}
                ${LIBMP3LAME_LIBS_RELEASE_DLL}
                ${LIBMPG123_LIBS_RELEASE_DLL}
                ${LIBOGG_LIBS_RELEASE_DLL}
                ${LIBOPUS_LIBS_RELEASE_DLL}
                ${LIBVORBIS_LIBS_RELEASE_DLL}
                )
            copy_files_to_bin(${FILES_TO_COPY})
        endif()

        set(dest_thirdparty system_modules/${PROJECT_NAME}/thirdparty)

        # Package libsndfile into platform release
        install(FILES ${LIBSNDFILE_LICENSE_FILES} DESTINATION ${dest_thirdparty}/libsndfile/source)
        install(FILES ${LIBSNDFILE_INCLUDE_DIR}/sndfile.h ${LIBSNDFILE_INCLUDE_DIR}/sndfile.hh DESTINATION ${dest_thirdparty}/libsndfile/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH}/include)
        if(WIN32)
            file(GLOB LIBSNDFILE_DYLIBS ${LIBSNDFILE_LIB_DIR}/*snd*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
            install(FILES ${LIBSNDFILE_DYLIBS} DESTINATION ${dest_thirdparty}/libsndfile/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH}/lib)
            file(GLOB LIBSNDFILE_IMPLIBS ${LIBSNDFILE_LIB_DIR}/*snd*.lib)
            install(FILES ${LIBSNDFILE_IMPLIBS} DESTINATION ${dest_thirdparty}/libsndfile/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH}/lib)
        elseif(UNIX)
            file(GLOB LIBSNDFILE_DYLIBS ${LIBSNDFILE_LIB_DIR}/lib*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
            install(FILES ${LIBSNDFILE_DYLIBS} DESTINATION ${dest_thirdparty}/libsndfile/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH}/lib)
        endif()

         # Package libsamplerate into platform release
        install(FILES ${LIBSAMPLERATE_LICENSE_FILES} DESTINATION ${dest_thirdparty}/libsamplerate/source)
        install(FILES ${LIBSAMPLERATE_INCLUDE_DIR}/samplerate.h DESTINATION ${dest_thirdparty}/libsamplerate/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH}/include)
        if(WIN32)
            file(GLOB LIBSAMPLERATE_DYLIBS ${LIBSAMPLERATE_LIB_DIR}/*samplerate*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
            install(FILES ${LIBSAMPLERATE_DYLIBS} DESTINATION ${dest_thirdparty}/libsamplerate/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH}/lib)
            file(GLOB LIBSAMPLERATE_IMPLIBS ${LIBSAMPLERATE_LIB_DIR}/*samplerate*.lib)
            install(FILES ${LIBSAMPLERATE_IMPLIBS} DESTINATION ${dest_thirdparty}/libsamplerate/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH}/lib)
        elseif(UNIX)
            file(GLOB LIBSAMPLERATE_DYLIBS ${LIBSAMPLERATE_LIB_DIR}/lib*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
            install(FILES ${LIBSAMPLERATE_DYLIBS} DESTINATION ${dest_thirdparty}/libsamplerate/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH}/lib)
        endif()


        # Install the dependencies. 
        foreach(dep IN LISTS SNDFILE_DEPENDENCIES)
            string(TOUPPER ${dep} DEP_UPPER)
            set(libdir "${DEP_UPPER}_DIR")
            install(DIRECTORY "${${libdir}}/"
                DESTINATION ${dest_thirdparty}/${dep}/
                )
        endforeach()

    else()
        set(MODULE_NAME_EXTRA_LIB "libsndfile;libsamplerate;libflac;libmp3lame;libmpg123;libogg;libopus;libvorbis")

        if(NOT TARGET moodycamel)
            find_package(moodycamel REQUIRED)
        endif()
        add_include_to_interface_target(napaudio ${MOODYCAMEL_INCLUDE_DIRS})
	    add_include_to_interface_target(napaudio ${LIBSNDFILE_INCLUDE_DIR})
        add_include_to_interface_target(napaudio ${LIBSAMPLERATE_INCLUDE_DIR})


        if(WIN32)
            set(DLLCOPY_PATH_SUFFIX "")

            # Copy libsndfile to bin post-build on Win64
            add_custom_command(TARGET ${PROJECT_NAME}
                               POST_BUILD
                               COMMAND ${CMAKE_COMMAND}
                                       -E copy_if_different
                                       ${LIBSNDFILE_LIBS_RELEASE_DLL}
                                       $<TARGET_FILE_DIR:${PROJECT_NAME}>/${DLLCOPY_PATH_SUFFIX}
                               )

            # Copy libsamplerate to bin post-build on Win64
            add_custom_command(TARGET ${PROJECT_NAME}
                               POST_BUILD
                               COMMAND ${CMAKE_COMMAND}
                                       -E copy_if_different
                                       ${LIBSAMPLERATE_LIBS_RELEASE_DLL}
                                       $<TARGET_FILE_DIR:${PROJECT_NAME}>/${DLLCOPY_PATH_SUFFIX}
                               )

            foreach(dep IN LISTS SNDFILE_DEPENDENCIES)
                string(TOUPPER ${dep} DEP_UPPER)
                set(libdll "${DEP_UPPER}_LIBS_RELEASE_DLL")
                add_custom_command(TARGET ${PROJECT_NAME}
                               POST_BUILD
                               COMMAND ${CMAKE_COMMAND}
                                       -E copy_if_different
                                       ${${libdll}}
                                       $<TARGET_FILE_DIR:${PROJECT_NAME}>/${DLLCOPY_PATH_SUFFIX}
                               )
            endforeach()

        elseif(UNIX)
            set(module_thirdparty ${NAP_ROOT}/system_modules/napaudio/thirdparty)

            # Install libsndfile into packaged app
            file(GLOB SNDFILE_DYLIBS ${module_thirdparty}/libsndfile/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH}/lib/*snd*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
            install(FILES ${SNDFILE_DYLIBS} DESTINATION lib)

            # Install libsamplerate into packaged app
            file(GLOB SAMPLERATE_DYLIBS ${module_thirdparty}/libsamplerate/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH}/lib/*samplerate*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
            install(FILES ${SAMPLERATE_DYLIBS} DESTINATION lib)

            foreach(dep IN LISTS SNDFILE_DEPENDENCIES)
                string(TOUPPER ${dep} DEP_UPPER)
                set(libdir "${DEP_UPPER}_LIB_DIR")
                install(DIRECTORY "${${libdir}}/"
                    DESTINATION lib
                    )
            endforeach()

        endif()

        # Install thirdparty licenses into packaged project
        install(FILES ${LIBSNDFILE_LICENSE_FILES} DESTINATION licenses/libsndfile)
        install(FILES ${LIBSAMPLERATE_LICENSE_FILES} DESTINATION licenses/libsamplerate)
        foreach(dep IN LISTS SNDFILE_DEPENDENCIES)
            string(TOUPPER ${dep} DEP_UPPER)
            set(licfiles "${DEP_UPPER}_LIB_DIR")
            install(FILES "${${licfiles}}" DESTINATION "licfiles/${DEP_UPPER}")
        endforeach()


    endif()
endif()