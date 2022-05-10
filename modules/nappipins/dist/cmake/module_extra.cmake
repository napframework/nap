include(${NAP_ROOT}/cmake/dist_shared_crossplatform.cmake)

if(NOT TARGET wiringpi)
    find_package(wiringpi REQUIRED)
endif()

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

