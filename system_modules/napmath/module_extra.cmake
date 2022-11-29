if(NOT TARGET glm)
    set(GLM_FIND_QUIETLY TRUE)
    find_package(glm REQUIRED)
endif()

if(NAP_BUILD_CONTEXT MATCHES "source")
    # Include GLM
    target_include_directories(${PROJECT_NAME} PUBLIC ${GLM_INCLUDE_DIRS})

    # For backwards compatibility, force identity matrix on construction
    target_compile_definitions(${PROJECT_NAME} PUBLIC GLM_FORCE_CTOR_INIT)

    # Setup definitions
    target_compile_definitions(${PROJECT_NAME} PRIVATE _USE_MATH_DEFINES MODULE_NAME=${PROJECT_NAME})
    if(WIN32)
        target_compile_definitions(${PROJECT_NAME} PUBLIC NOMINMAX)
    endif()

    # Package GLM
    install(FILES ${GLM_DIR}/copying.txt DESTINATION thirdparty/glm)
    install(DIRECTORY ${GLM_DIR}/glm/ DESTINATION thirdparty/glm)

    if (WIN32)
        # Install for fbxconverter
        install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION tools/buildsystem
                CONFIGURATIONS Release)
    endif()
else()
    # Add GLM
    add_include_to_interface_target(mod_napmath ${GLM_INCLUDE_DIRS})

    # For backwards compatibility, force identity matrix on construction
    target_compile_definitions(${PROJECT_NAME} PUBLIC GLM_FORCE_CTOR_INIT)

    if(WIN32)
        add_define_to_interface_target(mod_napmath NOMINMAX)
    endif()

    # Install thirdparty licenses into packaged app
    install(FILES ${THIRDPARTY_DIR}/glm/copying.txt DESTINATION licenses/glm)
endif()
