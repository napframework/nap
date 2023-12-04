if(NOT TARGET kissfft)
    find_package(kissfft REQUIRED)
endif()

if(NAP_BUILD_CONTEXT MATCHES "source")
    file(GLOB_RECURSE KISS ${KISSFFT_INCLUDE_DIR}/*.c ${KISSFFT_INCLUDE_DIR}/*.h)
    source_group("kissfft" FILES ${KISS})
    target_sources(${PROJECT_NAME} PUBLIC ${KISS})

    target_include_directories(${PROJECT_NAME} PUBLIC ${KISSFFT_INCLUDE_DIR})
    target_compile_definitions(${PROJECT_NAME} PUBLIC KISSFFT_DATATYPE=float KISSFFT_STATIC=OFF)

    # additional definitions
    if(WIN32)
        target_compile_definitions(${PROJECT_NAME} PUBLIC WIN32_LEAN_AND_MEAN _WIN32_WINNT=0x0A00)
    endif()

    # Package kissfft into platform release
    install(FILES ${KISSFFT_LICENSE_FILES} DESTINATION ${KISSFFT_DIR})
    install(DIRECTORY ${KISSFFT_INCLUDE_DIR} DESTINATION ${KISSFFT_DIR})
else()
    add_include_to_interface_target(napfft ${KISSFFT_INCLUDE_DIR})
    add_define_to_interface_target(napfft KISSFFT_DATATYPE=float)
    add_define_to_interface_target(napfft KISSFFT_STATIC=OFF)

    if(WIN32)
        add_define_to_interface_target(napfft WIN32_LEAN_AND_MEAN)
        add_define_to_interface_target(napfft _WIN32_WINNT=0x0A00)
    endif()

    # Install kissfft license into packaged project
    install(FILES ${KISSFFT_LICENSE_FILES} DESTINATION licenses/kissfft/)
endif()
