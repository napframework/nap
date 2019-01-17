if(WIN32)
    find_path(
            SQLITE_DIR
            NAMES msvc/include/sqlite3.h
            HINTS ${THIRDPARTY_DIR}/sqlite
    )
    set(SQLITE_INCLUDE_DIRS ${SQLITE_DIR}/msvc/include)
    set(SQLITE_LIBS_DIR_RELEASE ${SQLITE_DIR}/msvc/bin/Release)
	set(SQLITE_LIBS_DIR_DEBUG ${SQLITE_DIR}/msvc/bin/Debug)
    set(SQLITE_LIBS_DEBUG ${SQLITE_LIBS_DIR_DEBUG}/sqlite.lib)
	set(SQLITE_LIBS_RELEASE ${SQLITE_LIBS_DIR_RELEASE}/sqlite.lib)
elseif(APPLE)
    find_path(SQLITE_DIR
              NAMES osx/include/sqlite3.h
              HINTS ${THIRDPARTY_DIR}/sqlite
              )
    set(SQLITE_INCLUDE_DIRS ${SQLITE_DIR}/osx/include)
    set(SQLITE_LIBS_DIR_RELEASE ${SQLITE_DIR}/osx/bin/Release)
	set(SQLITE_LIBS_DIR_DEBUG ${SQLITE_DIR}/osx/bin/Debug)
    set(SQLITE_LIBS_DEBUG ${SQLITE_LIBS_DIR_DEBUG}/libsqlite.dylib)
	set(SQLITE_LIBS_RELEASE ${SQLITE_LIBS_DIR_RELEASE}/libsqlite.dylib)
else()
    find_path(SQLITE_DIR
              NAMES linux/include/sqlite3.h
              HINTS ${THIRDPARTY_DIR}/sqlite
              )
    set(SQLITE_INCLUDE_DIRS ${SQLITE_DIR}/linux/include)
    set(SQLITE_LIBS_DIR_RELEASE ${SQLITE_DIR}/linux/bin)
	set(SQLITE_LIBS_DIR_DEBUG ${SQLITE_DIR}/linux/bin)
    set(SQLITE_LIBS_DEBUG ${SQLITE_LIBS_DIR_DEBUG}/libsqlite.so)
	set(SQLITE_LIBS_RELEASE ${SQLITE_LIBS_DIR_RELEASE}/libsqlite.so)
endif()

mark_as_advanced(SQLITE_INCLUDE_DIRS)
mark_as_advanced(SQLITE_LIBS_DIR_DEBUG)
mark_as_advanced(SQLITE_LIBS_DIR_RELEASE)

add_library(sqlitelib STATIC IMPORTED)
set_target_properties(sqlitelib PROPERTIES
                      IMPORTED_CONFIGURATIONS "Debug;Release"
                      )

if(WIN32)
    set_target_properties(sqlitelib PROPERTIES
                          IMPORTED_IMPLIB_RELEASE ${SQLITE_LIBS}
                          IMPORTED_IMPLIB_DEBUG ${SQLITE_LIBS}
                          )
endif()


include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(sqlite REQUIRED_VARS SQLITE_DIR SQLITE_INCLUDE_DIRS SQLITE_LIBS_DEBUG SQLITE_LIBS_RELEASE SQLITE_LIBS_DIR_DEBUG SQLITE_LIBS_DIR_RELEASE)