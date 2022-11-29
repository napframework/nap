# Bring in SQLite from thirdparty
find_package(sqlite REQUIRED)

if(NAP_BUILD_CONTEXT MATCHES "source")
    target_include_directories(${PROJECT_NAME} PUBLIC ${SQLITE_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} sqlite)

    # SQLite packaging into release
    install(DIRECTORY ${THIRDPARTY_DIR}/sqlite/
            DESTINATION thirdparty/sqlite
            PATTERN NAP_NOTES.txt EXCLUDE
            )
else()
    add_include_to_interface_target(mod_napdatabase ${SQLITE_INCLUDE_DIR})

    # TODO install database license into packaged project
endif()
