if(NAP_BUILD_CONTEXT MATCHES "source")
    # Check for appropriate platform
    if(NOT RASPBERRY)
        set(SKIP_MODULE TRUE)
    endif()
endif()
