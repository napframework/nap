# Set this flag in order to link libsndfile and mpg123 and build support for reading audio files
set(NAP_AUDIOFILE_SUPPORT ON)

if(NAP_AUDIOFILE_SUPPORT)
    if(NOT TARGET libmpg123)
        find_package(libmpg123 REQUIRED)
    endif()
    if(NOT TARGET libsndfile)
        find_package(libsndfile REQUIRED)
    endif()
endif()

# Add sources to target
if(NAP_BUILD_CONTEXT MATCHES "source")
    # Add compile definition if we are building for raspberry pi
    check_raspbian_os(RASPBERRY)
    if (RASPBERRY)
        target_compile_definitions(${PROJECT_NAME} PRIVATE RASPBERRY)
    endif()

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
        set(LIBRARIES libsndfile libmpg123)
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

            if(NOT DEFINED LIBMPG123_LIB_DIR)
                find_package(libmpg123 REQUIRED)
            endif()

            # Copy audio DLLs to project build directory
            set(FILES_TO_COPY
                ${LIBSNDFILE_LIBS_RELEASE_DLL}
                ${LIBMPG123_LIBS_RELEASE_DLL}
                )
            copy_files_to_bin(${FILES_TO_COPY})
        endif()

        set(dest_thirdparty system_modules/${PROJECT_NAME}/thirdparty)

        # Package mpg123 into platform release
        install(FILES ${LIBMPG123_LICENSE_FILES} DESTINATION ${dest_thirdparty}/mpg123/source)
        install(DIRECTORY ${LIBMPG123_INCLUDE_DIR} DESTINATION ${dest_thirdparty}/mpg123/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH})
        if(WIN32)
            file(GLOB MPG123_DYLIBS ${LIBMPG123_LIB_DIR}/*.dll)
            install(FILES ${MPG123_DYLIBS} DESTINATION ${dest_thirdparty}/mpg123/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH}/lib)
            file(GLOB MPG123_LIBFILES ${LIBMPG123_LIB_DIR}/*.lib)
            install(FILES ${MPG123_LIBFILES} DESTINATION ${dest_thirdparty}/mpg123/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH}/lib)
        elseif(APPLE)
            # Ensure our mpg123 install name id is set properly, this is really for intall into packaging
            add_custom_command(TARGET ${PROJECT_NAME}
                               PRE_BUILD
                               COMMAND ${CMAKE_INSTALL_NAME_TOOL} -id @rpath/libmpg123.dylib ${LIBMPG123_LIB_DIR}/libmpg123.0.dylib
                               )

            file(GLOB MPG123_DYLIBS ${LIBMPG123_LIB_DIR}/libmpg*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
            install(FILES ${MPG123_DYLIBS}
                    DESTINATION ${dest_thirdparty}/mpg123/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH}/lib)

            foreach(build_type Release Debug)
                ensure_macos_file_has_rpath_at_install(${CMAKE_INSTALL_PREFIX}/modules/napaudio/lib/${build_type}/libnapaudio.dylib
                                                       "@loader_path/../../thirdparty/mpg123/macos/${ARCH}lib")
            endforeach()
        elseif(UNIX)
            file(GLOB MPG123_DYLIBS ${LIBMPG123_LIB_DIR}/libmpg123*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
            install(FILES ${MPG123_DYLIBS} DESTINATION ${dest_thirdparty}/mpg123/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH}/lib)



        endif()

        # Package libsndfile into platform release
        install(FILES ${LIBSNDFILE_LICENSE_FILES} DESTINATION ${dest_thirdparty}/libsndfile/source)
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
            file(GLOB LIBSNDFILE_DYLIBS ${LIBSNDFILE_LIB_DIR}/lib*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
            install(FILES ${LIBSNDFILE_DYLIBS}
                    DESTINATION ${dest_thirdparty}/libsndfile/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH}/lib)
        endif()
    else()
        set(MODULE_NAME_EXTRA_LIB "libmpg123;libsndfile")

        if(NOT TARGET moodycamel)
            find_package(moodycamel REQUIRED)
        endif()
        add_include_to_interface_target(napaudio ${MOODYCAMEL_INCLUDE_DIRS})

        if(WIN32)
            # Add post-build step to set copy mpg123 to bin on Win64
            set(DLLCOPY_PATH_SUFFIX "")
            add_custom_command(TARGET ${PROJECT_NAME}
                               POST_BUILD
                               COMMAND ${CMAKE_COMMAND}
                                       -E copy_if_different
                                       $<TARGET_FILE:libmpg123>
                                       $<TARGET_FILE_DIR:${PROJECT_NAME}>/${DLLCOPY_PATH_SUFFIX}
                               )

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

            # Install mpg123 lib into packaged app
            file(GLOB MPG123_DYLIBS ${module_thirdparty}/mpg123/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH}/lib/libmpg*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
            install(FILES ${MPG123_DYLIBS} DESTINATION lib)

            # Install libsndfile into packaged app
            file(GLOB SNDFILE_DYLIBS ${module_thirdparty}/libsndfile/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH}/lib/snd*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
            install(FILES ${SNDFILE_DYLIBS} DESTINATION lib)
        endif()

        # Install thirdparty licenses into packaged project
        install(FILES ${LIBMPG123_LICENSE_FILES} DESTINATION licenses/mpg123)
        install(FILES ${LIBSNDFILE_LICENSE_FILES} DESTINATION licenses/libsndfile)
    endif()
endif()