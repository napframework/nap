# Find websocketpp dependency
if(NOT TARGET websocketpp)
    find_package(websocketpp REQUIRED)
endif()

target_link_libraries(${PROJECT_NAME} libcrypto)
target_link_libraries(${PROJECT_NAME} libssl)

if(NAP_BUILD_CONTEXT MATCHES "source")
    target_include_directories(${PROJECT_NAME} PUBLIC ${WEBSOCKETPP_INCLUDE_DIR})

    # preprocessor
    target_compile_definitions(${PROJECT_NAME} PUBLIC _WEBSOCKETPP_CPP11_INTERNAL_)

    # Package websocket into platform release
    set(dest_dir system_modules/${PROJECT_NAME}/thirdparty/websocketpp)
    install(DIRECTORY ${WEBSOCKETPP_INCLUDE_DIR} DESTINATION ${dest_dir}/install)
    install(FILES ${WEBSOCKETPP_LICENSE_FILES} DESTINATION ${dest_dir})
else()
    add_include_to_interface_target(napwebsocket ${WEBSOCKETPP_INCLUDE_DIR})
    add_define_to_interface_target(napwebsocket _WEBSOCKETPP_CPP11_INTERNAL_)

    install(FILES ${WEBSOCKETPP_LICENSE_FILES} DESTINATION licenses/WebSocket++)
endif()
