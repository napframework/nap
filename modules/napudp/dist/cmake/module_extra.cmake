# Install asio license into packaged project
install(FILES ${THIRDPARTY_DIR}/asio/LICENSE_1_0.txt DESTINATION licenses/asio)

if(NOT TARGET asio)
    find_package(asio REQUIRED)
endif()

# Add asio to any target including this module
add_include_to_interface_target(mod_napudp ${ASIO_INCLUDE_DIRS})
add_define_to_interface_target(mod_napudp ASIO_STANDALONE)

if(WIN32)
    # Define _WIN32_WINNT for UDP
    add_define_to_interface_target(mod_napudp _WIN32_WINNT=0x0501)
endif()

# Install license into packaged project
install(FILES ${THIRDPARTY_DIR}/asio/LICENSE_1_0.txt DESTINATION licenses/asio)
