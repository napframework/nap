# Install rtmidi license into packaged project
install(FILES ${THIRDPARTY_DIR}/asio/LICENSE_1_0.txt DESTINATION licenses/asio)

if(NOT TARGET websocketpp)
    find_package(websocketpp REQUIRED)
endif()

add_include_to_interface_target(mod_napwebsocket ${WEBSOCKETPP_INCLUDE_DIRS})
