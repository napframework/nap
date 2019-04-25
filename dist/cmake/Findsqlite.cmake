find_path(
    SQLITE_DIR
    NO_CMAKE_FIND_ROOT_PATH
    NAMES src/sqlite3.h
    HINTS ${THIRDPARTY_DIR}/sqlite
    )

set(SQLITE_INCLUDE_DIR ${SQLITE_DIR}/src)
if(NOT TARGET sqlite)
    file(GLOB SOURCES ${SQLITE_DIR}/src/*.c ${SQLITE_DIR}/src/*.h)
    add_library(sqlite STATIC ${SOURCES})
    set_property(TARGET sqlite PROPERTY POSITION_INDEPENDENT_CODE ON)    
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(sqlite REQUIRED_VARS SQLITE_DIR SQLITE_INCLUDE_DIR)