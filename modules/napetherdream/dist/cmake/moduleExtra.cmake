list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/thirdparty/etherdream/cmake)
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
    # Install etherdream lib into packaged app
    install(FILES $<TARGET_FILE:etherdreamlib> DESTINATION lib)
endif()
