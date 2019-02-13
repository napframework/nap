include(${NAP_ROOT}/cmake/dist_shared_crossplatform.cmake)

set(SQLITE_INCLUDES ${THIRDPARTY_DIR}/sqlite)

add_include_to_interface_target(mod_napdatabase ${SQLITE_INCLUDES})

# Install database license into packaged project
