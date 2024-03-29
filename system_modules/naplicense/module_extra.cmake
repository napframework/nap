if(NAP_BUILD_CONTEXT MATCHES "source")
    if (WIN32)
        # Link against iphlpapi
        target_link_libraries(${PROJECT_NAME} iphlpapi.lib)
    endif ()
endif ()