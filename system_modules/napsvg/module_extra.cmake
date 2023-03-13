if(NAP_BUILD_CONTEXT MATCHES "source")
    # Get NanoSVG
    find_package(nanosvg REQUIRED)

    target_include_directories(${PROJECT_NAME} PUBLIC ${NANOSVG_INCLUDE_DIRS})

    target_compile_definitions(${PROJECT_NAME} PUBLIC NANOSVG_IMPLEMENTATION PUBLIC NANOSVG_ALL_COLOR_KEYWORDS)

    # Package NanoSVG into platform release
    install(FILES ${NANOSVG_DIR}/LICENSE.txt ${NANOSVG_DIR}/README.md DESTINATION thirdparty/nanosvg)
    install(DIRECTORY ${NANOSVG_INCLUDE_DIRS}/ DESTINATION thirdparty/nanosvg/include)
else()
    # Install NanoSVG license into packaged project
    install(FILES ${THIRDPARTY_DIR}/nanosvg/LICENSE.txt DESTINATION licenses/NanoSVG)
endif()
