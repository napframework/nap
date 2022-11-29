if(NAP_BUILD_CONTEXT MATCHES "source")
    find_package(cryptopp REQUIRED)

    target_include_directories(${PROJECT_NAME} PUBLIC ${CRYPTOPP_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} cryptopp)

    if (WIN32)
        # Copy Crypto++ DLL to bin directory
        copy_cryptopp_dll()

        # Package Crypto++ into platform release
        install(DIRECTORY ${CRYPTOPP_LIB_DIR} DESTINATION thirdparty/cryptopp 
            FILES_MATCHING PATTERN "*.dll")
    endif()

    # Install Crypto++ license
    install(FILES ${CRYPTOPP_DIST_FILES} DESTINATION thirdparty/cryptopp)
else()
    # Install Crypto++ DLL when using Windows
    if(WIN32)
        find_package(cryptopp REQUIRED)

        # Add post-build step to copy Crypto++ to bin
        add_custom_command(TARGET ${PROJECT_NAME}
                           POST_BUILD
                           COMMAND ${CMAKE_COMMAND} 
                                   -E copy
                                   $<TARGET_FILE:cryptopp>
                                   $<TARGET_FILE_DIR:${PROJECT_NAME}> 
                           )

        # Install Crypto++ library for licensegenerator
        install(FILES $<TARGET_FILE:cryptopp> DESTINATION licensegenerator)
    endif()

    # Install Crypto++ license into packaged app
    install(FILES ${THIRDPARTY_DIR}/cryptopp/License.txt DESTINATION licenses/cryptopp)

    # Install licensegenerator into packaged app
    file(GLOB LICENSE_GENERATOR_BIN ${NAP_ROOT}/tools/license/licensegenerator*)
    install(PROGRAMS ${LICENSE_GENERATOR_BIN} DESTINATION licensegenerator)
endif()
