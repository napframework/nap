# Copy Python framework libs, during post-build or install, depending on our platform
if(WIN32)
    if(DEFINED PACKAGE_NAPKIN AND NOT PACKAGE_NAPKIN)
        add_custom_command(TARGET ${PROJECT_NAME}
                           POST_BUILD
                           COMMAND ${CMAKE_COMMAND} -E copy_directory ${THIRDPARTY_DIR}/python/ $<TARGET_FILE_DIR:${PROJECT_NAME}>
                           )
    endif()
endif()
if(APPLE)
    # Install our Python dylib from thirdparty
    install(FILES ${THIRDPARTY_DIR}/python/lib/libpython3.6m.dylib
            DESTINATION lib/)

    # Framework library
    install(DIRECTORY ${THIRDPARTY_DIR}/python/lib/python3.6
            DESTINATION lib/)
endif()