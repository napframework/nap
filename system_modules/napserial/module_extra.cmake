if(NOT TARGET serial)
    find_package(serial REQUIRED)
endif()

if(NAP_BUILD_CONTEXT MATCHES "source")
    target_include_directories(${PROJECT_NAME} PUBLIC ${SERIAL_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} debug ${SERIAL_LIBS_DEBUG} optimized ${SERIAL_LIBS_RELEASE})

    set(thirdparty_module_dest system_modules/${PROJECT_NAME}/thirdparty)

    # Package serial into platform release
    install(DIRECTORY ${SERIAL_LIBS_DIR} DESTINATION ${thirdparty_module_dest}/serial/${NAP_THIRDPARTY_PLATFORM_DIR})
    install(FILES ${SERIAL_DIR}/source/README.md DESTINATION ${thirdparty_module_dest}/serial/source)
    install(DIRECTORY ${SERIAL_INCLUDE_DIR} DESTINATION ${thirdparty_module_dest}/serial/source PATTERN "*.cpp" EXCLUDE)
else()
    set(MODULE_EXTRA_LIBS_OPTIMIZED ${SERIAL_LIBS_RELEASE})
    set(MODULE_EXTRA_LIBS_DEBUG ${SERIAL_LIBS_DEBUG})
    add_include_to_interface_target(napserial ${SERIAL_INCLUDE_DIR})

    install(FILES ${SERIAL_LICENSE_FILES} DESTINATION licenses/serial/)
endif()
