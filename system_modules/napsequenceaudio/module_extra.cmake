if(NAP_BUILD_CONTEXT MATCHES "source")
    # Link with external libs
    if(NOT WIN32)
        target_compile_definitions(${PROJECT_NAME} PUBLIC HAVE_CONFIG_H)
    endif()
endif()
