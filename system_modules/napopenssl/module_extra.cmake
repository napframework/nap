# Find openssl dependency
if(NOT TARGET openssl)
    find_package(openssl REQUIRED)
endif()

set(openssl_dest_dir system_modules/napopenssl/thirdparty/openssl/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH}/libs)
if(NAP_BUILD_CONTEXT MATCHES "source")
    target_include_directories(${PROJECT_NAME} PUBLIC ${OPENSSL_INCLUDE_DIR})

    target_link_libraries(${PROJECT_NAME} debug ${LIBCRYPTO_LIB} optimized ${LIBCRYPTO_LIB})
    target_link_libraries(${PROJECT_NAME} debug ${LIBSSL_LIB} optimized ${LIBSSL_LIB})

    # Install includes
    install(DIRECTORY ${OPENSSL_INCLUDE_DIR} DESTINATION system_modules/napopenssl/thirdparty/openssl)

    # install libcrypto and libssl
    install(FILES ${OPENSSL_LIBS} DESTINATION ${openssl_dest_dir})

    # Install and copy dll's for windows
    if(WIN32)
        install(FILES ${LIBSSL_DLL} DESTINATION ${openssl_dest_dir})
        install(FILES ${LIBCRYPTO_DLL} DESTINATION ${openssl_dest_dir})
        copy_openssl_dll()
    endif ()

    # install license
    install(FILES ${OPENSSL_LICENSE_FILES} DESTINATION system_modules/napopenssl/thirdparty/openssl)
else()
    set(MODULE_EXTRA_LIBS ${OPENSSL_LIBS})
    add_include_to_interface_target(napopenssl ${OPENSSL_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} debug ${LIBCRYPTO_LIB} optimized ${LIBCRYPTO_LIB})
    target_link_libraries(${PROJECT_NAME} debug ${LIBSSL_LIB} optimized ${LIBSSL_LIB})

    if(WIN32)
        install(FILES ${OPENSSL_LIBS} DESTINATION lib)
        copy_openssl_dll()
    elseif(UNIX)
        file(GLOB libsopenssl ${NAP_ROOT}/${openssl_dest_dir}/*)
        install(FILES ${libsopenssl} DESTINATION lib)
    endif()

    # install license
    install(FILES ${OPENSSL_LICENSE_FILES} DESTINATION licenses/openssl)
endif()
