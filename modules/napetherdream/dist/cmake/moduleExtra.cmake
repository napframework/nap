find_package(etherdream REQUIRED)
target_link_libraries(${PROJECT_NAME} etherdreamlib)

# Add post-build step to set etherdream RPATH
if(APPLE)
    add_custom_command(TARGET ${PROJECT_NAME}
                       POST_BUILD
                       COMMAND ${NAP_ROOT}/tools/platform/ensureHasRPath.py $<TARGET_FILE:${PROJECT_NAME}> $<TARGET_FILE_DIR:etherdreamlib>
                       )

    # Install etherdream lib into packaged app
    install(FILES $<TARGET_FILE:etherdreamlib> DESTINATION lib)
endif()