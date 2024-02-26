# Set this flag in order to link libsndfile and mpg123 and build support for reading audio files
set(NAP_AUDIOFILE_SUPPORT ON)

if(NAP_AUDIOFILE_SUPPORT)
    if(NOT TARGET libsndfile)
        find_package(libsndfile REQUIRED)
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

# Add thirdparty libraries for audio file support
if (NAP_AUDIOFILE_SUPPORT)
    if(NAP_BUILD_CONTEXT MATCHES "source")
        set(LIBRARIES libsndfile)
        if (APPLE)
            list(APPEND LIBRARIES "-framework CoreFoundation")
        elseif(UNIX)
            list(APPEND LIBRARIES atomic)
        endif()

        set(INCLUDES ${LIBSNDFILE_INCLUDE_DIR} ${LIBMPG123_INCLUDE_DIR})
        target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDES})

        target_link_libraries(${PROJECT_NAME} ${LIBRARIES})

        target_compile_definitions(${PROJECT_NAME} PRIVATE _USE_MATH_DEFINES)

        # Copy over DLLs on Windows
        if(WIN32)
            if(NOT DEFINED LIBSNDFILE_LIB_DIR)
                find_package(libsndfile REQUIRED)
            endif()

            # Copy audio DLLs to project build directory
            set(FILES_TO_COPY
                ${LIBSNDFILE_LIBS_RELEASE_DLL}
                )
            copy_files_to_bin(${FILES_TO_COPY})
        endif()

        set(dest_thirdparty system_modules/${PROJECT_NAME}/thirdparty)

        # Package libsndfile into platform release
        install(FILES ${LIBSNDFILE_LICENSE_FILES} DESTINATION ${dest_thirdparty}/libsndfile/source)
        message(${ARCH})
        install(FILES ${LIBSNDFILE_INCLUDE_DIR}/sndfile.h ${LIBSNDFILE_INCLUDE_DIR}/sndfile.hh
                DESTINATION ${dest_thirdparty}/libsndfile/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH}/include)
        if(WIN32)
            file(GLOB LIBSNDFILE_DYLIBS ${LIBSNDFILE_LIB_DIR}/libsnd*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
            install(FILES ${LIBSNDFILE_DYLIBS}
                    DESTINATION ${dest_thirdparty}/libsndfile/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH}/lib)
            file(GLOB LIBSNDFILE_IMPLIBS ${LIBSNDFILE_LIB_DIR}/libsnd*.lib)
            install(FILES ${LIBSNDFILE_IMPLIBS}
                    DESTINATION ${dest_thirdparty}/libsndfile/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH}/lib)
        elseif(UNIX)
            file(GLOB LIBSNDFILE_DYLIBS ${LIBSNDFILE_LIB_DIR}/libsnd*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
            install(FILES ${LIBSNDFILE_DYLIBS}
                    DESTINATION ${dest_thirdparty}/libsndfile/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH}/lib)
        endif()
    else()
        set(MODULE_NAME_EXTRA_LIB "libsndfile")

        if(NOT TARGET moodycamel)
            find_package(moodycamel REQUIRED)
        endif()
        add_include_to_interface_target(napaudio ${MOODYCAMEL_INCLUDE_DIRS})

        if(WIN32)
            # Copy libsndfile to bin post-build on Win64
            add_custom_command(TARGET ${PROJECT_NAME}
                               POST_BUILD
                               COMMAND ${CMAKE_COMMAND}
                                       -E copy_if_different
                                       ${LIBSNDFILE_LIBS_RELEASE_DLL}
                                       $<TARGET_FILE_DIR:${PROJECT_NAME}>/${DLLCOPY_PATH_SUFFIX}
                               )

        elseif(UNIX)
            set(module_thirdparty ${NAP_ROOT}/system_modules/napaudio/thirdparty)

            # Install libsndfile into packaged app
            file(GLOB SNDFILE_DYLIBS ${module_thirdparty}/libsndfile/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH}/lib/libsnd*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
            install(FILES ${SNDFILE_DYLIBS} DESTINATION lib)
        endif()

        # Install thirdparty licenses into packaged project
        install(FILES ${LIBSNDFILE_LICENSE_FILES} DESTINATION licenses/libsndfile)
    endif()
endif()