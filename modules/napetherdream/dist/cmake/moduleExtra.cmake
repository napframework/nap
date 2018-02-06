find_package(etherdream REQUIRED)
target_link_libraries(${PROJECT_NAME} etherdreamlib)

if(APPLE)
    # Add post-build step to set etherdream RPATH
    # add_custom_command(TARGET ${PROJECT_NAME}
    #                    POST_BUILD
    #                    COMMAND ${CMAKE_INSTALL_NAME_TOOL} 
    #                            -add_rpath
    #                            $<TARGET_FILE_DIR:etherdreamlib>
    #                            $<TARGET_FILE:${PROJECT_NAME}> 
    #                    )

    # Install etherdream lib into packaged app
    install(FILES $<TARGET_FILE:etherdreamlib> DESTINATION lib)
endif()