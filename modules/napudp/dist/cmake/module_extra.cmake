# Install rtmidi license into packaged project
install(FILES ${THIRDPARTY_DIR}/asio/LICENSE_1_0.txt DESTINATION licenses/asio)

if(NOT TARGET asio)
    find_package(asio REQUIRED)
endif()

add_include_to_interface_target(mod_napudp ${ASIO_INCLUDE_DIRS})
add_define_to_interface_target(mod_napudp ASIO_STANDALONE)
