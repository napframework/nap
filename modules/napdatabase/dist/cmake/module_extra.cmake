include(${NAP_ROOT}/cmake/dist_shared_crossplatform.cmake)

# Bring in SQLite from thirdparty
find_package(sqlite REQUIRED)

add_include_to_interface_target(mod_napdatabase ${SQLITE_INCLUDE_DIR})

# Install database license into packaged project
