find_sdl2()

if(NAP_BUILD_CONTEXT MATCHES "source")
    target_include_directories(${PROJECT_NAME} PUBLIC ${SDL2_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARY})
else()
    add_include_to_interface_target(napsdlinput ${SDL2_INCLUDE_DIR})

    if (WIN32)
        # Copy over DLL post-build
        add_custom_command(
            TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SDL2_DIR}/msvc/x86_64/lib/SDL2.dll $<TARGET_FILE_DIR:${PROJECT_NAME}>/
       )
    endif()
endif()
