# Find websocketpp dependency
if(NOT TARGET websocketpp)
    find_package(websocketpp REQUIRED)
endif()

set(libcrypto_dest_dir system_modules/napwebsocket/thirdparty/libcrypto/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH}/lib)
set(libssl_dest_dir system_modules/napwebsocket/thirdparty/libssl/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH}/lib)
if(NAP_BUILD_CONTEXT MATCHES "source")
    target_include_directories(${PROJECT_NAME} PUBLIC ${WEBSOCKETPP_INCLUDE_DIR})

    target_link_libraries(${PROJECT_NAME} debug ${LIBCRYPTO_LIB} optimized ${LIBCRYPTO_LIB})
    target_link_libraries(${PROJECT_NAME} debug ${LIBSSL_LIB} optimized ${LIBSSL_LIB})

    # preprocessor
    target_compile_definitions(${PROJECT_NAME} PUBLIC _WEBSOCKETPP_CPP11_INTERNAL_)

    # install libcrytp and libssl
    if(UNIX)
        install(FILES ${LIBCRYPTO_LIBS} DESTINATION ${libcrypto_dest_dir})
        install(FILES ${LIBSSL_LIBS} DESTINATION ${libssl_dest_dir})
    endif()

    # Package websocket into platform release
    set(websocket_dest_dir system_modules/${PROJECT_NAME}/thirdparty/websocketpp)
    install(DIRECTORY ${WEBSOCKETPP_INCLUDE_DIR} DESTINATION ${websocket_dest_dir}/install)
    install(FILES ${WEBSOCKETPP_LICENSE_FILES} DESTINATION ${websocket_dest_dir})
else()
    set(MODULE_EXTRA_LIBS ${LIBCRYPTO_LIB} ${LIBSSL_LIB})
    add_include_to_interface_target(napwebsocket ${WEBSOCKETPP_INCLUDE_DIR})
    add_define_to_interface_target(napwebsocket _WEBSOCKETPP_CPP11_INTERNAL_)

    if(UNIX)
        file(GLOB libscrypto ${NAP_ROOT}/${libcrypto_dest_dir}/libcrypto*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
        install(FILES ${libcrypto} DESTINATION lib)
        file(GLOB libsssl ${NAP_ROOT}/${libssl_dest_dir}/libssl*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
        install(FILES ${libsssl} DESTINATION lib)
    endif()

    install(FILES ${WEBSOCKETPP_LICENSE_FILES} DESTINATION licenses/WebSocket++)
endif()
