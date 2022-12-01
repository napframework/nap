# Find websocketpp dependency
if(NOT TARGET websocketpp)
    find_package(websocketpp REQUIRED)
endif()

if(NAP_BUILD_CONTEXT MATCHES "source")
    target_include_directories(${PROJECT_NAME} PUBLIC ${WEBSOCKETPP_INCLUDE_DIR})

    # preprocessor
    target_compile_definitions(${PROJECT_NAME} PUBLIC _WEBSOCKETPP_CPP11_INTERNAL_)

    # Package websocket into platform release
    install(FILES ${WEBSOCKETPP_DIST_FILES} DESTINATION thirdparty/websocketpp)
    install(DIRECTORY ${WEBSOCKETPP_INCLUDE_DIR}/ DESTINATION thirdparty/websocketpp/include)
else()
    add_include_to_interface_target(napwebsocket ${WEBSOCKETPP_INCLUDE_DIRS})
    add_define_to_interface_target(napwebsocket _WEBSOCKETPP_CPP11_INTERNAL_)

    install(FILES ${WEBSOCKETPP_DIR}/COPYING DESTINATION licenses/websocketpp)
endif()
