if(NAP_BUILD_CONTEXT MATCHES "source")
    find_package(cryptopp REQUIRED)

    target_include_directories(${PROJECT_NAME} PUBLIC ${CRYPTOPP_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} cryptopp)

    if (WIN32)
        # Copy Crypto++ DLL to bin directory
        copy_cryptopp_dll()

        # Package Crypto++ into platform release
        set(dest_dir system_modules/${PROJECT_NAME}/thirdparty/cryptopp/msvc/x86_64)
        install(DIRECTORY ${CRYPTOPP_LIB_DIR} DESTINATION ${dest_dir}
                FILES_MATCHING PATTERN "*.dll")
        install(DIRECTORY ${CRYPTOPP_INCLUDE_DIRS} DESTINATION ${dest_dir})
    endif()

    # Install Crypto++ license
    install(FILES ${CRYPTOPP_DIR}/License.txt DESTINATION thirdparty/cryptopp)
else()
    # Install Crypto++ DLL when using Windows
    if(WIN32)
        find_package(cryptopp REQUIRED)

        # Add post-build step to copy Crypto++ to bin
        add_custom_command(TARGET ${PROJECT_NAME}
                           POST_BUILD
                           COMMAND ${CMAKE_COMMAND}
                                   -E copy_if_different
                                   $<TARGET_FILE:cryptopp>
                                   $<TARGET_FILE_DIR:${PROJECT_NAME}>
                           )

        # Install Crypto++ library for licensegenerator
        install(FILES $<TARGET_FILE:cryptopp> DESTINATION licensegenerator)
    endif()

    # Install Crypto++ license into packaged app
    install(FILES ${THIRDPARTY_DIR}/cryptopp/License.txt DESTINATION "licenses/Crypto++")
    install(FILES ${THIRDPARTY_DIR}/tclap/COPYING DESTINATION licenses/tclap/)

    # Install licensegenerator into packaged app
    file(GLOB LICENSE_GENERATOR_BIN ${NAP_ROOT}/tools/license/licensegenerator*)
    install(PROGRAMS ${LICENSE_GENERATOR_BIN} DESTINATION licensegenerator)
endif()
