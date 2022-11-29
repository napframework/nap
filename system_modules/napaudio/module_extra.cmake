if(NOT TARGET libmpg123)
    find_package(libmpg123 REQUIRED)
endif()
if(NOT TARGET libsndfile)
    find_package(libsndfile REQUIRED)
endif()
if(NOT TARGET portaudio)
    find_package(portaudio REQUIRED)
endif()

if(NAP_BUILD_CONTEXT MATCHES "source")
    source_group("core" src/audio/core/*.*)
    source_group("node" src/audio/node/*.*)
    source_group("component" src/audio/component/*.*)
    source_group("resource" src/audio/resource/*.*)
    source_group("service" src/audio/service/*.*)
    source_group("utility" src/audio/utility/*.*)

    set(LIBRARIES portaudio libsndfile libmpg123)
    if(APPLE)
        list(APPEND LIBRARIES "-framework CoreAudio" "-framework CoreServices" "-framework CoreFoundation" "-framework AudioUnit" "-framework AudioToolbox")
    elseif(UNIX)
        list(APPEND LIBRARIES atomic)
    endif()

    set(INCLUDES ${PORTAUDIO_INCLUDE_DIR} ${LIBSNDFILE_INCLUDE_DIR} ${LIBMPG123_INCLUDE_DIR})
    target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDES})

    target_link_libraries(${PROJECT_NAME} ${LIBRARIES})

    target_compile_definitions(${PROJECT_NAME} PRIVATE _USE_MATH_DEFINES)

    # Copy over DLLs on Windows
    if(WIN32)
        if(NOT DEFINED LIBSNDFILE_LIB_DIR)
            find_package(libsndfile REQUIRED)
        endif()

        if(NOT DEFINED PORTAUDIO_LIB_DIR)
            find_package(portaudio REQUIRED)
        endif()

        if(NOT DEFINED LIBMPG123_LIB_DIR)
            find_package(libmpg123 REQUIRED)
        endif()

        # Copy audio DLLs to project build directory
        set(FILES_TO_COPY
            ${LIBSNDFILE_LIBS_RELEASE_DLL}
            ${PORTAUDIO_LIBS_RELEASE_DLL}
            ${LIBMPG123_LIBS_RELEASE_DLL}
            )
        copy_files_to_bin(${FILES_TO_COPY})
    endif()

    # Package mpg123 into platform release
    install(FILES ${LIBMPG123_DIST_FILES} DESTINATION thirdparty/mpg123)
    install(DIRECTORY ${LIBMPG123_INCLUDE_DIR}/ DESTINATION thirdparty/mpg123/include)
    if(WIN32)
        file(GLOB MPG123_DYLIBS ${LIBMPG123_LIB_DIR}/*.dll)
        install(FILES ${MPG123_DYLIBS} DESTINATION thirdparty/mpg123/bin)
        file(GLOB MPG123_LIBFILES ${LIBMPG123_LIB_DIR}/*.lib) 
        install(FILES ${MPG123_LIBFILES} DESTINATION thirdparty/mpg123/lib)
    elseif(APPLE)
        # Ensure our mpg123 install name id is set properly, this is really for intall into packaging
        add_custom_command(TARGET ${PROJECT_NAME}
                           PRE_BUILD
                           COMMAND ${CMAKE_INSTALL_NAME_TOOL} -id @rpath/libmpg123.dylib ${LIBMPG123_LIB_DIR}/libmpg123.0.dylib
                           )

        file(GLOB MPG123_DYLIBS ${LIBMPG123_LIB_DIR}/libmpg*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
        install(FILES ${MPG123_DYLIBS}
                DESTINATION thirdparty/mpg123/lib)

        foreach(build_type Release Debug)
            ensure_macos_file_has_rpath_at_install(${CMAKE_INSTALL_PREFIX}/modules/mod_napaudio/lib/${build_type}/libmod_napaudio.dylib
                                                   "@loader_path/../../../../thirdparty/mpg123/lib")
        endforeach()
    elseif(UNIX)
        file(GLOB MPG123_DYLIBS ${LIBMPG123_LIB_DIR}/libmpg123*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
        install(FILES ${MPG123_DYLIBS}
                DESTINATION thirdparty/mpg123/lib)
    endif()

    # Package libsndfile into platform release
    install(FILES ${LIBSNDFILE_DIST_FILES} DESTINATION thirdparty/libsndfile)
    install(FILES ${LIBSNDFILE_INCLUDE_DIR}/sndfile.h ${LIBSNDFILE_INCLUDE_DIR}/sndfile.hh
            DESTINATION thirdparty/libsndfile/include)
    if(WIN32)
        file(GLOB LIBSNDFILE_DYLIBS ${LIBSNDFILE_LIB_DIR}/libsnd*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
        install(FILES ${LIBSNDFILE_DYLIBS}
                DESTINATION thirdparty/libsndfile/bin)
        file(GLOB LIBSNDFILE_IMPLIBS ${LIBSNDFILE_LIB_DIR}/libsnd*.lib)
        install(FILES ${LIBSNDFILE_IMPLIBS}
                DESTINATION thirdparty/libsndfile/lib)
    elseif(UNIX)
        file(GLOB LIBSNDFILE_DYLIBS ${LIBSNDFILE_LIB_DIR}/libsnd*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
        install(FILES ${LIBSNDFILE_DYLIBS}
                DESTINATION thirdparty/libsndfile/lib)
    endif()

    # Package portaudio into platform release
    install(FILES ${PORTAUDIO_DIST_FILES} DESTINATION thirdparty/portaudio)
    install(DIRECTORY ${PORTAUDIO_INCLUDE_DIR}/ DESTINATION thirdparty/portaudio/include)
    if(WIN32)
        install(FILES ${PORTAUDIO_LIBRARIES}
                DESTINATION thirdparty/portaudio/lib)
        install(FILES ${PORTAUDIO_LIBS_RELEASE_DLL}
                DESTINATION thirdparty/portaudio/bin)
    elseif(UNIX)
        file(GLOB PORTAUDIO_DYLIBS ${PORTAUDIO_LIB_DIR}/libport*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
        install(FILES ${PORTAUDIO_DYLIBS}
                DESTINATION thirdparty/portaudio/lib)
    endif()
else()
    set(MODULE_NAME_EXTRA_LIB "libmpg123;libsndfile;portaudio")

    if(NOT TARGET moodycamel)
        find_package(moodycamel REQUIRED)
    endif()
    add_include_to_interface_target(mod_napaudio ${MOODYCAMEL_INCLUDE_DIRS})

    if(NOT TARGET portaudio)
        find_package(portaudio REQUIRED)
    endif()
    target_include_directories(${PROJECT_NAME} PUBLIC ${PORTAUDIO_INCLUDE_DIR})

    if(WIN32)
        # Add post-build step to set copy mpg123 to bin on Win64
        set(DLLCOPY_PATH_SUFFIX "")
        add_custom_command(TARGET ${PROJECT_NAME}
                           POST_BUILD
                           COMMAND ${CMAKE_COMMAND}
                                   -E copy
                                   $<TARGET_FILE:libmpg123>
                                   $<TARGET_FILE_DIR:${PROJECT_NAME}>/${DLLCOPY_PATH_SUFFIX}
                           )

        # Copy libsndfile to bin post-build on Win64
        add_custom_command(TARGET ${PROJECT_NAME}
                           POST_BUILD
                           COMMAND ${CMAKE_COMMAND}
                                   -E copy
                                   ${THIRDPARTY_DIR}/libsndfile/bin/libsndfile-1.dll
                                   $<TARGET_FILE_DIR:${PROJECT_NAME}>/${DLLCOPY_PATH_SUFFIX}
                           )

        # Copy portaudio to bin post-build on Win64
        add_custom_command(TARGET ${PROJECT_NAME}
                           POST_BUILD
                           COMMAND ${CMAKE_COMMAND}
                                   -E copy
                                   ${THIRDPARTY_DIR}/portaudio/bin/portaudio_x64.dll
                                   $<TARGET_FILE_DIR:${PROJECT_NAME}>/${DLLCOPY_PATH_SUFFIX}
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

    if(APPLE)
        # Add mpg123 RPATH to built app
        macos_add_rpath_to_module_post_build(${PROJECT_NAME} $<TARGET_FILE:${PROJECT_NAME}> ${THIRDPARTY_DIR}/mpg123/lib)

        # Add portaudio RPATH to built app
        macos_add_rpath_to_module_post_build(${PROJECT_NAME} $<TARGET_FILE:${PROJECT_NAME}> ${THIRDPARTY_DIR}/portaudio/lib)

        # Add libsndfile RPATH to built app
        macos_add_rpath_to_module_post_build(${PROJECT_NAME} $<TARGET_FILE:${PROJECT_NAME}> ${THIRDPARTY_DIR}/libsndfile/lib)
    endif()

    # Install thirdparty licenses into packaged project
    install(FILES ${THIRDPARTY_DIR}/portaudio/LICENSE.txt DESTINATION licenses/portaudio)
    install(FILES ${THIRDPARTY_DIR}/libsndfile/COPYING DESTINATION licenses/libsndfile)
    install(FILES ${THIRDPARTY_DIR}/mpg123/COPYING DESTINATION licenses/mpg123)
endif()
