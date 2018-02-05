# Copy Python framework libs, during post-build or install, depending on our platform
if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME}
                       POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy_directory ${THIRDPARTY_DIR}/python/lib $<TARGET_FILE_DIR:${PROJECT_NAME}>
                       )
endif()
if(APPLE)
    # Install our Python dylib from thirdparty
    install(FILES ${THIRDPARTY_DIR}/python/Python
            DESTINATION lib/)

    # Framework library
    install(DIRECTORY ${THIRDPARTY_DIR}/python/lib
            DESTINATION lib/)
endif()