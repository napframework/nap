find_package(artnet REQUIRED)
target_link_libraries(${PROJECT_NAME} artnet)

# Add post-build step to set artnet RPATH
if(APPLE)
    add_custom_command(TARGET ${PROJECT_NAME}
                       POST_BUILD
                       COMMAND ${NAP_ROOT}/tools/platform/ensureHasRPath.py $<TARGET_FILE:${PROJECT_NAME}> $<TARGET_FILE_DIR:artnet>
                       )

    # Install artnet lib into packaged app
    install(FILES $<TARGET_FILE:artnet> DESTINATION lib)
endif()