if(NOT WIN32)
    target_compile_definitions(${PROJECT_NAME} PUBLIC HAVE_CONFIG_H)
endif()
