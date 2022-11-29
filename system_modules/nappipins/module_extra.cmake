if(NOT TARGET wiringpi)
    find_package(wiringpi REQUIRED)
endif()

if(NAP_BUILD_CONTEXT MATCHES "source")
    target_include_directories(${PROJECT_NAME} PUBLIC ${WIRINGPI_INCLUDE_DIR})

    target_link_libraries(${PROJECT_NAME} debug ${WIRINGPI_LIBRARIES_DEBUG} optimized ${WIRINGPI_LIBRARIES_RELEASE})

    # Package wiringpi into platform release
    install(FILES ${WIRINGPI_DIST_FILES} DESTINATION thirdparty/wiringpi)
    install(DIRECTORY ${WIRINGPI_INCLUDE_DIR}/ DESTINATION thirdparty/wiringpi/include)
    install(DIRECTORY ${WIRINGPI_LIBRARY_DIR}/ DESTINATION thirdparty/wiringpi/lib)
else()
    if(NOT TARGET moodycamel)
        find_package(moodycamel REQUIRED)
    endif()
    add_include_to_interface_target(mod_nappipins ${MOODYCAMEL_INCLUDE_DIRS})

    set(MODULE_NAME_EXTRA_LIBS wiringpi)

    add_include_to_interface_target(mod_nappipins ${WIRINGPI_INCLUDE_DIRS})

    # Install wiringpi lib into packaged app
    file(GLOB WIRINGPI_DYLIBS ${THIRDPARTY_DIR}/wiringpi/lib/libwiringPi.so)
    install(FILES ${WIRINGPI_DYLIBS} DESTINATION lib)

    # Install wiringpi license into packaged project
    install(FILES ${THIRDPARTY_DIR}/wiringpi/README.md DESTINATION licenses/wiringpi)
endif()
