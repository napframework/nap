if (NOT NAP_BUILD_CONTEXT MATCHES "source")
    # Install data directory into packaged app
    install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/data DESTINATION system_modules/naprenderadvanced)
endif()
