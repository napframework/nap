find_sdl()

if(NAP_BUILD_CONTEXT MATCHES "source")
    target_include_directories(${PROJECT_NAME} PUBLIC ${SDL_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} SDL3::SDL3)
else()
    add_include_to_interface_target(napsdlinput ${SDL_INCLUDE_DIR})
    add_include_to_interface_target(napsdlinput ${SDL_INCLUDE_DIR}/..)

    if (WIN32)
        # Copy over DLL post-build
        add_custom_command(
            TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SDL_BIN_DIR}/SDL3.dll $<TARGET_FILE_DIR:${PROJECT_NAME}>/
       )
    endif()
endif()
