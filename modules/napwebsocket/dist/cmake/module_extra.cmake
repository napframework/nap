if(NOT TARGET websocketpp)
    find_package(websocketpp REQUIRED)
endif()

add_include_to_interface_target(mod_napwebsocket ${WEBSOCKETPP_INCLUDE_DIRS})
add_define_to_interface_target(mod_napwebsocket _WEBSOCKETPP_CPP11_INTERNAL_)

install(FILES ${WEBSOCKETPP_DIR}/COPYING DESTINATION licenses/websocketpp)