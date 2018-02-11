find_package(etherdream REQUIRED)
target_link_libraries(${PROJECT_NAME} etherdreamlib)

if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME}
                       POST_BUILD
                       COMMAND ${CMAKE_COMMAND} 
                               -E copy
                               $<TARGET_FILE:etherdreamlib>
                               $<TARGET_FILE_DIR:${PROJECT_NAME}> 
                       )
elseif(UNIX)
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
