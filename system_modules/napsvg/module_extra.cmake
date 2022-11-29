if(NAP_BUILD_CONTEXT MATCHES "source")
    # Get nanosvg
    find_package(nanosvg REQUIRED)

    target_include_directories(${PROJECT_NAME} PUBLIC ${NANOSVG_INCLUDE_DIRS})

    target_compile_definitions(${PROJECT_NAME} PUBLIC NANOSVG_IMPLEMENTATION PUBLIC NANOSVG_ALL_COLOR_KEYWORDS)

    # Package nanosvg into platform release
    install(FILES ${NANOSVG_DIR}/LICENSE.txt ${NANOSVG_DIR}/README.md DESTINATION thirdparty/nanosvg)
    install(DIRECTORY ${NANOSVG_INCLUDE_DIRS}/ DESTINATION thirdparty/nanosvg/include)
else()
    # Install nanosvg license into packaged project
    install(FILES ${THIRDPARTY_DIR}/nanosvg/LICENSE.txt DESTINATION licenses/nanosvg)
endif()
