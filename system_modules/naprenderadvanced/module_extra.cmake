if (NAP_BUILD_CONTEXT MATCHES "source")
    # Skip certain features when building against Raspberry Pi 4
    if (RPI_MODEL MATCHES 4)
        target_compile_definitions(${PROJECT_NAME} PRIVATE RASPBERRY_PI_4)
    endif()
else()
    # Install data directory into packaged app
    install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/data DESTINATION system_modules/naprenderadvanced)
endif()
