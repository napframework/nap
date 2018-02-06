find_package(artnet REQUIRED)
target_link_libraries(${PROJECT_NAME} artnet)

if(APPLE)
    # Add post-build step to set artnet RPATH
    # add_custom_command(TARGET ${PROJECT_NAME}
    #                    POST_BUILD
    #                    COMMAND ${CMAKE_INSTALL_NAME_TOOL} 
    #                            -add_rpath
    #                            $<TARGET_FILE_DIR:artnet>
    #                            $<TARGET_FILE:${PROJECT_NAME}> 
    #                    )

    # Install artnet lib into packaged app
    install(FILES $<TARGET_FILE:artnet> DESTINATION lib)
endif()