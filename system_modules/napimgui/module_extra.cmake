if(NAP_BUILD_CONTEXT MATCHES "source")
    file(GLOB_RECURSE GUI src/imgui/*.cpp src/imgui/misc/cpp/*.cpp src/imgui/*.h src/imgui/misc/cpp/*.h)

    source_group("imgui" FILES ${GUI})

    target_include_directories(${PROJECT_NAME} PUBLIC src/imgui)
    target_sources(${PROJECT_NAME} PRIVATE ${GUI})
    target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY_IMGUI)
else()
    # Install data directory into packaged app
    install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/data DESTINATION system_modules/napimgui)
endif()
