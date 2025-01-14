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

    # Package kissfft license into platform release
    set(KISS_LICENSE_TARGET_DIR system_modules/${PROJECT_NAME}/thirdparty/kissfft/licenses)
    install(FILES ${KISSFFT_LICENSE_FILES} DESTINATION ${KISS_LICENSE_TARGET_DIR})
else()
    # Install kissfft license into packaged project
    set(KISS_LICENSE_SOURCE_DIR ${NAP_ROOT}/system_modules/napfft/thirdparty/kissfft/licenses)
    file(GLOB KISSFFT_LICENSE_FILES ${KISS_LICENSE_SOURCE_DIR}/*)
    install(FILES ${KISSFFT_LICENSE_FILES} DESTINATION licenses/kissfft/)
endif()
