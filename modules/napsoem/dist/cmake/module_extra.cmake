# Install readme
install(FILES ${THIRDPARTY_DIR}/soem/LICENSE DESTINATION licenses/soem)

if(NOT TARGET soem)
    find_package(soem REQUIRED)
endif()
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