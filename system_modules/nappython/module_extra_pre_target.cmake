if(NAP_BUILD_CONTEXT MATCHES "source")
    # Only include if Python enabled
    if(NOT NAP_ENABLE_PYTHON)
        set(SKIP_MODULE TRUE)
    endif()
endif()
