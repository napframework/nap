# Install asio license into packaged project
install(FILES ${THIRDPARTY_DIR}/asio/LICENSE_1_0.txt DESTINATION licenses/asio)

if(NOT TARGET asio)
    find_package(asio REQUIRED)
endif()

# Add asio to any target including this module
add_include_to_interface_target(mod_napudp ${ASIO_INCLUDE_DIRS})
add_define_to_interface_target(mod_napudp ASIO_STANDALONE)

# Install license into packaged project
install(FILES ${THIRDPARTY_DIR}/asio/LICENSE_1_0.txt DESTINATION licenses/asio)
