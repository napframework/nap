if(NOT TARGET glm)
    set(GLM_FIND_QUIETLY TRUE)
    find_package(glm REQUIRED)
endif()

set(thirdparty_module_dir system_modules/napmath/thirdparty)
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
    install(DIRECTORY ${GLM_DIR}/glm DESTINATION ${thirdparty_module_dir}/glm/)
    install(FILES ${GLM_DIR}/copying.txt DESTINATION ${thirdparty_module_dir}/glm)

    if (WIN32)
        # Install for fbxconverter
        install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION tools/buildsystem
                CONFIGURATIONS Release)
    endif()
else()
    # Add GLM
    add_include_to_interface_target(napmath ${GLM_INCLUDE_DIRS})

    # For backwards compatibility, force identity matrix on construction
    target_compile_definitions(${PROJECT_NAME} PUBLIC GLM_FORCE_CTOR_INIT)

    if(WIN32)
        add_define_to_interface_target(napmath NOMINMAX)
    endif()

    # Install thirdparty licenses into packaged app
    install(FILES ${NAP_ROOT}/${thirdparty_module_dir}/glm/copying.txt DESTINATION licenses/GLM)
endif()
