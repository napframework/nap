include(${NAP_ROOT}/cmake/dist_shared_crossplatform.cmake)

if(NOT TARGET yoctopuce)
    find_package(yoctopuce REQUIRED)
endif()
set(MODULE_NAME_EXTRA_LIBS yoctopuce)

add_include_to_interface_target(mod_napyoctopuce ${YOCTOPUCE_INCLUDE_DIRS})

if(WIN32)
    # Add post-build step to set copy yoctopuce to bin
    add_custom_command(TARGET ${PROJECT_NAME}
                       POST_BUILD
                       COMMAND ${CMAKE_COMMAND} 
                               -E copy
                               $<TARGET_FILE:yoctopuce>
                               $<TARGET_FILE_DIR:${PROJECT_NAME}> 
                       )
elseif(UNIX)
    # Install yoctopuce lib into packaged app
    install(FILES $<TARGET_FILE:yoctopuce> DESTINATION lib)
endif()

# TODO Install yoctopuce license with packaged app if we find one
