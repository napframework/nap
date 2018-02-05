# Copy Python framework libs, during post-build or install, depending on our platform
if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME}
                       POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy_directory ${THIRDPARTY_DIR}/python/lib $<TARGET_FILE_DIR:${PROJECT_NAME}>
                       )
endif()
if(APPLE)
    # Install our Python dylib from thirdparty
    file(GLOB python_dylibs "${THIRDPARTY_DIR}/python/*.dylib")
    list(LENGTH ${python_dylibs} list_length)
    if(${list_length} GREATER 1)
        message(FATAL_ERROR "Unexpectedly found more than one dylib in ${THIRDPARTY_DIR}/python")
    endif()
    foreach(python_dylib ${python_dylibs})
        install(FILES ${python_dylib}
                DESTINATION lib/
                RENAME Python)
    endforeach()

    # Framework library
    install(DIRECTORY ${THIRDPARTY_DIR}/python/lib
            DESTINATION lib/)
endif()