include(${NAP_ROOT}/cmake/dist_shared_crossplatform.cmake)

if(NOT TARGET cryptopp)
    find_package(cryptopp REQUIRED)
endif()
set(MODULE_NAME_EXTRA_LIBS cryptopp)

# add_include_to_interface_target(mod_napyoctopuce ${YOCTOPUCE_INCLUDE_DIRS})

if(WIN32)
    # Add post-build step to set copy yoctopuce to bin
    add_custom_command(TARGET ${PROJECT_NAME}
                       POST_BUILD
                       COMMAND ${CMAKE_COMMAND} 
                               -E copy
                               $<TARGET_FILE:cryptopp>
                               $<TARGET_FILE_DIR:${PROJECT_NAME}> 
                       )
elseif(UNIX)
    # Install yoctopuce lib into packaged app
    install(FILES $<TARGET_FILE:cryptopp> DESTINATION lib)
endif()

# TODO Install cryptopp license with packaged app if we find one