find_package(freetype REQUIRED)
target_link_libraries(${PROJECT_NAME} freetype)
target_include_directories(${PROJECT_NAME} PUBLIC ${FREETYPE_INCLUDE_DIRS})

if(WIN32)
    # Add post-build step to set copy yoctopuce to bin
    add_custom_command(TARGET ${PROJECT_NAME}
                       POST_BUILD
                       COMMAND ${CMAKE_COMMAND} 
                               -E copy
                               $<TARGET_FILE:freetype>
                               $<TARGET_FILE_DIR:${PROJECT_NAME}> 
                       )
elseif(UNIX)
    # Install yoctopuce lib into packaged app
    install(FILES $<TARGET_FILE:freetype> DESTINATION lib)
endif()