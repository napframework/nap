include(${NAP_ROOT}/cmake/dist_shared_crossplatform.cmake)

if(NOT TARGET wiringpi)
    find_package(wiringpi REQUIRED)
endif()
set(MODULE_NAME_EXTRA_LIBS wiringpi)

add_include_to_interface_target(mod_napwiringpi ${WIRINGPI_INCLUDE_DIRS})

# Install pigpio lib into packaged app
if(UNIX)
    file(GLOB WIRINGPI_DYLIBS ${THIRDPARTY_DIR}/wiringpi/lib/librt*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
    install(FILES ${WIRINGPI_DYLIBS} DESTINATION lib)
endif()

# Install pigpio license into packaged project
install(FILES ${THIRDPARTY_DIR}/wiringpi/README.md DESTINATION licenses/wiringpi)