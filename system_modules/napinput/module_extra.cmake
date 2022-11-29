if(NAP_BUILD_CONTEXT MATCHES "source")
    if (WIN32)
        # Install for fbxconverter
        install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION tools/platform
                CONFIGURATIONS Release)
    endif()
endif()
