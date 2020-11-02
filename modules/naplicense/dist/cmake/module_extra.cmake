include(${NAP_ROOT}/cmake/dist_shared_crossplatform.cmake)

# install cryptopp dlls when using windows
if(WIN32 AND NOT TARGET cryptopp)
    find_package(cryptopp REQUIRED)

    # Add post-build step to copy cryptopp to bin
    add_custom_command(TARGET ${PROJECT_NAME}
                       POST_BUILD
                       COMMAND ${CMAKE_COMMAND} 
                               -E copy
                               $<TARGET_FILE:cryptopp>
                               $<TARGET_FILE_DIR:${PROJECT_NAME}> 
                       )

    # Install cryptopp library for licensegenerator
    # install(FILES ${THIRDPARTY_DIR}/cryptopp/lib/release/cryptopp.dll DESTINATION licensegenerator)
    install(FILES $<TARGET_FILE:cryptopp> DESTINATION licensegenerator)
endif()

# Install cryptopp license into packaged project
install(FILES ${THIRDPARTY_DIR}/cryptopp/License.txt DESTINATION licenses/cryptopp)

# Install licensegenerator
file(GLOB LICENSE_GENERATOR_BIN ${NAP_ROOT}/tools/license/licensegenerator*)
install(PROGRAMS ${LICENSE_GENERATOR_BIN} DESTINATION licensegenerator)
