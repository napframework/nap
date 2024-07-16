if(NAP_BUILD_CONTEXT MATCHES "source")

    if(NOT TARGET kissfft)
        find_package(kissfft REQUIRED)
    endif()

    file(GLOB_RECURSE KISS ${KISSFFT_INCLUDE_DIR}/*.c ${KISSFFT_INCLUDE_DIR}/*.h)
    source_group("kissfft" FILES ${KISS})
    target_sources(${PROJECT_NAME} PRIVATE ${KISS})

    target_include_directories(${PROJECT_NAME} PUBLIC ${KISSFFT_INCLUDE_DIR})
    target_compile_definitions(${PROJECT_NAME} PUBLIC KISSFFT_DATATYPE=float KISSFFT_STATIC=OFF)

    # additional definitions
    if(WIN32)
        target_compile_definitions(${PROJECT_NAME} PUBLIC WIN32_LEAN_AND_MEAN _WIN32_WINNT=0x0A00)
    endif()

    # Package kissfft into platform release
    set(KISSDEST system_modules/${PROJECT_NAME}/thirdparty/kissfft)
    install(FILES ${KISSFFT_LICENSE_FILES} DESTINATION ${KISSDEST}/licenses)
else()
    # Install kissfft license into packaged project
    set(module_thirdparty ${NAP_ROOT}/system_modules/${PROJECT_NAME}/thirdparty/kissfft)
    file(GLOB KISSFFT_LICENSE_FILES ${module_thirdparty}/licenses/*)
    install(FILES ${KISSFFT_LICENSE_FILES} DESTINATION licenses/kissfft/)
endif()
