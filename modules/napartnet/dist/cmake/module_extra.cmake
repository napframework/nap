include(${NAP_ROOT}/cmake/dist_shared_crossplatform.cmake)

if(NOT TARGET artnet)
    find_package(artnet REQUIRED)
endif()
set(MODULE_NAME_EXTRA_LIBS artnet)

if(WIN32)
    # Add post-build step to set copy artnet to bin
    add_custom_command(TARGET ${PROJECT_NAME}
                       POST_BUILD
                       COMMAND ${CMAKE_COMMAND} 
                               -E copy
                               $<TARGET_FILE:artnet>
                               $<TARGET_FILE_DIR:${PROJECT_NAME}> 
                       )
elseif(UNIX)
    # Install artnet lib into packaged project
    install(FILES $<TARGET_FILE:artnet> DESTINATION lib)
endif()

# Install artnet license into packaged project
install(FILES ${THIRDPARTY_DIR}/libartnet/COPYING DESTINATION licenses/libartnet)