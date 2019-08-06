include(${NAP_ROOT}/cmake/dist_shared_crossplatform.cmake)

if(NOT TARGET serial)
    find_package(serial REQUIRED)
endif()

set(MODULE_NAME_EXTRA_LIBS serial)
add_include_to_interface_target(mod_napserial ${SERIAL_INCLUDE_DIRS})
