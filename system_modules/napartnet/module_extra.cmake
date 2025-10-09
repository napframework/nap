if(NOT TARGET artnet)
    find_package(artnet REQUIRED)
endif()

set(artnet_dest_dir system_modules/napartnet/thirdparty/libartnet)
if(NAP_BUILD_CONTEXT MATCHES "source")
    target_include_directories(${PROJECT_NAME} PUBLIC ${ARTNET_INCLUDE_DIRS})

    target_link_libraries(${PROJECT_NAME} artnet)
    if(WIN32)
      target_link_libraries(${PROJECT_NAME} Ws2_32.lib)
    endif()

    if(WIN32)
        # Copy artnet DLL to build directory
        if(NOT TARGET artnet)
            find_package(artnet REQUIRED)
        endif()
        copy_artnet_dll()
    elseif(UNIX AND NOT APPLE)
        set(ARTNET_LIB_FILE ${ARTNET_LIBS_DIR}/libartnet.so)

        # HACK, somehow on some systems, libartnet.so gets linked with libartnet.so.1 because it already exists
        add_custom_command(TARGET ${PROJECT_NAME}
                POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy "${ARTNET_LIB_FILE}" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/libartnet.so.1"
                COMMENT "Copy ${F} -> $<TARGET_FILE_DIR:${PROJECT_NAME}>/libartnet.so.1")
    endif()

    # Package libartnet into platform release
    # Install docs
    install(FILES ${ARTNET_DIR}/source/AUTHORS
                  ${ARTNET_DIR}/source/COPYING
                  ${ARTNET_DIR}/source/README
            DESTINATION ${artnet_dest_dir}/source
            )

    # Install includes
    install(DIRECTORY ${ARTNET_INCLUDE_DIRS} DESTINATION ${artnet_dest_dir}/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH})

    # Install libraries
    if(UNIX)
        file(GLOB ARTNET_DYLIBS ${ARTNET_LINUX_DIR}/lib/libartnet*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
        install(FILES ${ARTNET_DYLIBS} DESTINATION ${artnet_dest_dir}/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH}/lib)
    else()
        install(FILES $<TARGET_FILE:artnet> DESTINATION ${artnet_dest_dir}/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH}/bin/Release)
        install(FILES $<TARGET_FILE_DIR:artnet>/libartnet.lib DESTINATION ${artnet_dest_dir}/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH}/bin/Release)
    endif()

else()
    set(MODULE_EXTRA_LIBS artnet)

    if(WIN32)
        # Add post-build step to set copy artnet to bin
        add_custom_command(TARGET ${PROJECT_NAME}
                           POST_BUILD
                           COMMAND ${CMAKE_COMMAND}
                                   -E copy_if_different
                                   $<TARGET_FILE:artnet>
                                   $<TARGET_FILE_DIR:${PROJECT_NAME}>
                           )
    elseif(UNIX)
        # Install artnet lib into packaged project on Linux
        file(GLOB ARTNET_DYLIBS ${NAP_ROOT}/${artnet_dest_dir}/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH}/lib/libartnet*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
        install(FILES ${ARTNET_DYLIBS} DESTINATION lib)
    endif()

    # Install artnet license into packaged project
    install(FILES ${ARTNET_LICENSE_FILES} DESTINATION licenses/libartnet)
endif()
