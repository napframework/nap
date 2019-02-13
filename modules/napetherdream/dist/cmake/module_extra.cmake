include(${NAP_ROOT}/cmake/dist_shared_crossplatform.cmake)

if(NOT TARGET etherdream)
    find_package(etherdream REQUIRED)
endif()

set(MODULE_NAME_EXTRA_LIBS etherdreamlib)

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
