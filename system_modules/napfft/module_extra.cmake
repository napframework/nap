if(NOT TARGET kissfft)
    find_package(kissfft REQUIRED)
endif()

if(NAP_BUILD_CONTEXT MATCHES "source")
    target_include_directories(${PROJECT_NAME} PUBLIC src ${KISSFFT_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} ${KISSFFT_RELEASE_LIB})

    # additional definitions
    if(WIN32)
        target_compile_definitions(${PROJECT_NAME} PUBLIC WIN32_LEAN_AND_MEAN _WIN32_WINNT=0x0A00)

        add_custom_command(
            TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND}
                    -E copy_if_different
                    "$<$<CONFIG:Debug>:${KISSFFT_DEBUG_DLL}>$<$<CONFIG:Release>:${KISSFFT_RELEASE_DLL}>"
                    $<TARGET_FILE_DIR:${PROJECT_NAME}>
            COMMENT "Copying ${KISSFFT_DLL_FILENAME} -> bin dir")
    endif()

    # Package kissfft into platform release
    install(FILES ${KISSFFT_LICENSE_FILES} DESTINATION ${KISSFFT_DIR})
    install(DIRECTORY ${KISSFFT_INCLUDE_DIR} DESTINATION ${KISSFFT_DIR})
else()
    add_include_to_interface_target(napfft ${KISSFFT_INCLUDE_DIR})

    if(WIN32)
        # Define _WIN32_WINNT for KISSFFT
        add_define_to_interface_target(napfft WIN32_LEAN_AND_MEAN)
        add_define_to_interface_target(napfft _WIN32_WINNT=0x0A00)
    endif()

    # Install kissfft license into packaged project
    install(FILES ${KISSFFT_LICENSE_FILES} DESTINATION licenses/kissfft/)
endif()
