if(NOT TARGET serial)
    find_package(serial REQUIRED)
endif()

if(NAP_BUILD_CONTEXT MATCHES "source")
    target_include_directories(${PROJECT_NAME} PUBLIC ${SERIAL_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} debug ${SERIAL_LIBS_DEBUG} optimized ${SERIAL_LIBS_RELEASE})

    # Package serial into platform release
    install(FILES ${SERIAL_DIST_FILES} DESTINATION thirdparty/serial)
    install(DIRECTORY ${SERIAL_INCLUDE_DIR} DESTINATION thirdparty/serial PATTERN "*.cpp" EXCLUDE)
    install(DIRECTORY ${SERIAL_LIBS_DIR}/ DESTINATION thirdparty/serial/lib)
else()
    set(MODULE_NAME_EXTRA_LIBS serial)
    add_include_to_interface_target(mod_napserial ${SERIAL_INCLUDE_DIRS})
endif()
