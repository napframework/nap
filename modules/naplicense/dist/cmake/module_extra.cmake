include(${NAP_ROOT}/cmake/dist_shared_crossplatform.cmake)

if(NOT TARGET cryptopp)
    find_package(cryptopp REQUIRED)
endif()
set(MODULE_NAME_EXTRA_LIBS cryptopp)

# add_include_to_interface_target(mod_napyoctopuce ${YOCTOPUCE_INCLUDE_DIRS})

if(WIN32)
    # Add post-build step to copy cryptopp to bin
    add_custom_command(TARGET ${PROJECT_NAME}
                       POST_BUILD
                       COMMAND ${CMAKE_COMMAND} 
                               -E copy
                               $<TARGET_FILE:cryptopp>
                               $<TARGET_FILE_DIR:${PROJECT_NAME}> 
                       )

    # TODO: Only do this on install, don't include
    add_custom_command(TARGET ${PROJECT_NAME}
                       POST_BUILD
                       COMMAND ${CMAKE_COMMAND} 
                               -E copy_directory
                               ${NAP_ROOT}/tools/license
                               $<TARGET_FILE_DIR:${PROJECT_NAME}>/licensegenerator
                       )

elseif(UNIX)
    # Install yoctopuce lib into packaged app
    install(FILES $<TARGET_FILE:cryptopp> DESTINATION lib)
endif()

# Install cryptopp license into packaged project
install(FILES ${THIRDPARTY_DIR}/cryptopp/License.txt DESTINATION licenses/cryptopp)

# Install licensegenerator
install(DIRECTORY ${NAP_ROOT}/tools/license DESTINATION licensegenerator)