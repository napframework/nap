if(NOT TARGET soem)
    find_package(soem REQUIRED)
endif()

if(NAP_BUILD_CONTEXT MATCHES "source")
    target_include_directories(${PROJECT_NAME} PUBLIC ${SOEM_INCLUDE_DIRS})
    target_compile_definitions(${PROJECT_NAME} PUBLIC __STDC_LIMIT_MACROS)
    target_link_libraries(${PROJECT_NAME} ${SOEM_LIBS})

    # Install license
    install(FILES ${SOEM_DIST_FILES} DESTINATION thirdparty/soem)

    # Install include
    install(DIRECTORY ${SOEM_DIR}/include DESTINATION thirdparty/soem)

    # Install lib
    install(DIRECTORY ${SOEM_DIR}/lib DESTINATION thirdparty/soem)

    # Install wpcap includes (only Windows)
    if(WIN32)
        install(DIRECTORY ${WPCAP_DIR}/Include DESTINATION thirdparty/soem/wpcap)
        install(DIRECTORY ${WPCAP_DIR}/Lib DESTINATION thirdparty/soem/wpcap)
    endif()
else()
    set(MODULE_NAME_EXTRA_LIBS soem)

    add_include_to_interface_target(mod_napsoem ${SOEM_DIR}/include)
    add_include_to_interface_target(mod_napsoem ${SOEM_DIR}/include/soem)

    if(WIN32)
        add_include_to_interface_target(mod_napsoem ${WPCAP_DIR}/Include)
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            target_link_libraries(mod_napsoem INTERFACE ${WPCAP_DIR}/Lib/x64/wpcap.lib)
            target_link_libraries(mod_napsoem INTERFACE ${WPCAP_DIR}/Lib/x64/Packet.lib)
        elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
            target_link_libraries(mod_napsoem INTERFACE ${WPCAP_DIR}/Lib/wpcap.lib)
            target_link_libraries(mod_napsoem INTERFACE ${WPCAP_DIR}/Lib/Packet.lib)
        endif()
    endif()

    add_define_to_interface_target(mod_napsoem __STDC_LIMIT_MACROS)

    # Install license into packaged app
    install(FILES ${THIRDPARTY_DIR}/soem/LICENSE DESTINATION licenses/soem)
endif()
