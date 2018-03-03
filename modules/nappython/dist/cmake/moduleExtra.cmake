# Copy Python framework libs, during post-build or install, depending on our platform
if(WIN32)
    if(DEFINED PACKAGE_NAPKIN AND NOT PACKAGE_NAPKIN)
        # TODO Share with install_napkin_with_project
        file(GLOB PYTHON_DLLS ${THIRDPARTY_DIR}/python/*.dll)
        foreach(PYTHON_DLL ${PYTHON_DLLS})
            add_custom_command(TARGET ${PROJECT_NAME}
                               POST_BUILD
                               COMMAND ${CMAKE_COMMAND} -E copy ${PYTHON_DLL} $<TARGET_FILE_DIR:${PROJECT_NAME}>
                               )
        endforeach()
        add_custom_command(TARGET ${PROJECT_NAME}
                           POST_BUILD
                           COMMAND ${CMAKE_COMMAND} -E copy ${THIRDPARTY_DIR}/python/python36.zip $<TARGET_FILE_DIR:${PROJECT_NAME}>
                           )
    endif()
endif()

if(UNIX)
    # Python modules library installation
    install(DIRECTORY ${THIRDPARTY_DIR}/python/lib/python3.6
            DESTINATION lib/
            PATTERN __pycache__ EXCLUDE
            PATTERN *.pyc EXCLUDE)
endif()
