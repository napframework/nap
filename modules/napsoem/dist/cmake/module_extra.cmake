# Install readme
install(FILES ${THIRDPARTY_DIR}/soem/LICENSE DESTINATION licenses/soem)

include(${NAP_ROOT}/cmake/dist_shared_crossplatform.cmake)

if(NOT TARGET soem)
    find_package(soem REQUIRED)
endif()
set(MODULE_NAME_EXTRA_LIBS soem)

add_include_to_interface_target(mod_napsoem ${SOEM_INCLUDE_DIRS})
add_define_to_interface_target(mod_napsoem __STDC_LIMIT_MACROS)