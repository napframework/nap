# Install rtmidi license into packaged project
install(FILES ${THIRDPARTY_DIR}/asio/LICENSE_1_0.txt DESTINATION licenses/asio)

if(NOT TARGET websocketpp)
    find_package(websocketpp REQUIRED)
endif()

if(NOT TARGET asio)
    find_package(asio REQUIRED)
endif()

add_include_to_interface_target(mod_napwebsocket ${WEBSOCKETPP_INCLUDE_DIRS})
add_include_to_interface_target(mod_napwebsocket ${ASIO_INCLUDE_DIRS})
add_define_to_interface_target(mod_napwebsocket ASIO_STANDALONE)
add_define_to_interface_target(mod_napwebsocket _WEBSOCKETPP_CPP11_INTERNAL_)