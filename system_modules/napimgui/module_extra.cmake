if(NAP_BUILD_CONTEXT MATCHES "source")
    file(GLOB_RECURSE GUI src/imgui/*.cpp src/imgui/*.h)

    source_group("imgui" FILES ${GUI})

    target_sources(${PROJECT_NAME} PRIVATE ${GUI})
    target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY_IMGUI)
else()
    # Install data directory into packaged app
    install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/data DESTINATION system_modules/napimgui)
endif()
