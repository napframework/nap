if(NOT TARGET portaudio)
    find_package(portaudio REQUIRED)
endif()

if(NAP_BUILD_CONTEXT MATCHES "source")
    source_group("service" src/audio/service/*.*)

    set(LIBRARIES portaudio)
    if(APPLE)
        list(APPEND LIBRARIES "-framework CoreAudio" "-framework CoreServices" "-framework CoreFoundation" "-framework AudioUnit" "-framework AudioToolbox")
    elseif(UNIX)
        list(APPEND LIBRARIES atomic)
    endif()

    set(INCLUDES ${PORTAUDIO_INCLUDE_DIR})
    target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDES})

    target_link_libraries(${PROJECT_NAME} ${LIBRARIES})

    target_compile_definitions(${PROJECT_NAME} PRIVATE _USE_MATH_DEFINES)

    # Copy over DLLs on Windows
    if(WIN32)
        if(NOT DEFINED PORTAUDIO_LIB_DIR)
            find_package(portaudio REQUIRED)
        endif()

        # Copy audio DLLs to project build directory
        set(FILES_TO_COPY
            ${PORTAUDIO_LIBS_RELEASE_DLL}
            )
        copy_files_to_bin(${FILES_TO_COPY})
    endif()

    set(dest_thirdparty system_modules/${PROJECT_NAME}/thirdparty)

    # Package portaudio into platform release
    install(FILES ${PORTAUDIO_LICENSE_FILES} DESTINATION DESTINATION ${dest_thirdparty}/portaudio/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH})
    install(DIRECTORY ${PORTAUDIO_INCLUDE_DIR} DESTINATION ${dest_thirdparty}/portaudio/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH})
    if(WIN32)
        install(FILES ${PORTAUDIO_LIBRARIES}
                DESTINATION ${dest_thirdparty}/portaudio/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH}/lib)
        install(FILES ${PORTAUDIO_LIBS_RELEASE_DLL}
                DESTINATION ${dest_thirdparty}/portaudio/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH}/lib)
    elseif(UNIX)
        file(GLOB PORTAUDIO_DYLIBS ${PORTAUDIO_LIB_DIR}/libport*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
        install(FILES ${PORTAUDIO_DYLIBS}
                DESTINATION ${dest_thirdparty}/portaudio/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH}/lib)
    endif()
else()
    set(MODULE_NAME_EXTRA_LIB "portaudio")

    if(NOT TARGET moodycamel)
        find_package(moodycamel REQUIRED)
    endif()
    add_include_to_interface_target(napportaudio ${MOODYCAMEL_INCLUDE_DIRS})

    if(NOT TARGET portaudio)
        find_package(portaudio REQUIRED)
    endif()
    target_include_directories(${PROJECT_NAME} PUBLIC ${PORTAUDIO_INCLUDE_DIR})

    if(WIN32)
        # Copy portaudio to bin post-build on Win64
        add_custom_command(TARGET ${PROJECT_NAME}
                           POST_BUILD
                           COMMAND ${CMAKE_COMMAND}
                                   -E copy_if_different
                                   ${PORTAUDIO_LIBS_RELEASE_DLL}
                                   $<TARGET_FILE_DIR:${PROJECT_NAME}>/${DLLCOPY_PATH_SUFFIX}
                           )
    elseif(UNIX)
        set(module_thirdparty ${NAP_ROOT}/system_modules/napportaudio/thirdparty)

        # Install portaudio lib into packaged app
        file(GLOB PORTAUDIO_DYLIBS ${module_thirdparty}/portaudio/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH}/lib/libport*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
        install(FILES ${PORTAUDIO_DYLIBS} DESTINATION lib)
    endif()

    # Install thirdparty licenses into packaged project
    install(FILES ${PORTAUDIO_LICENSE_FILES} DESTINATION licenses/PortAudio)
endif()
