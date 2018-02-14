find_package(artnet REQUIRED)
target_link_libraries(${PROJECT_NAME} artnet)

if(WIN32)
    # Add post-build step to set artnet RPATH
    add_custom_command(TARGET ${PROJECT_NAME}
                       POST_BUILD
                       COMMAND ${CMAKE_COMMAND} 
                               -E copy
                               $<TARGET_FILE:artnet>
                               $<TARGET_FILE_DIR:${PROJECT_NAME}> 
                       )
elseif(UNIX)
    # Install artnet lib into packaged app
    install(FILES $<TARGET_FILE:artnet> DESTINATION lib)
endif()
