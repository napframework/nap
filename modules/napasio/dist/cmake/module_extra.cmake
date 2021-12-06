if(NOT TARGET asio)
    find_package(asio REQUIRED)
endif()

add_include_to_interface_target(mod_napasio ${ASIO_INCLUDE_DIRS})
add_define_to_interface_target(mod_napasio ASIO_STANDALONE)

if(WIN32)
    # Define WIN32_LEAN_AND_MEAN for ASIO
    add_define_to_interface_target(mod_napasio WIN32_LEAN_AND_MEAN)
endif()

# Install asio license into packaged project
install(FILES ${THIRDPARTY_DIR}/asio/LICENSE_1_0.txt DESTINATION licenses/asio)
