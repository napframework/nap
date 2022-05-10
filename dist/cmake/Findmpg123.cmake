if (WIN32)
    find_path(
        LIBMPG123_DIR
        NAMES bin/libmpg123.dll
        HINTS ${THIRDPARTY_DIR}/mpg123
        )
    set(MPG123_LIBS_DIR ${LIBMPG123_DIR}/bin)
    set(MPG123_LIBS_RELEASE_DLL ${MPG123_LIBS_DIR}/libmpg123.dll)
  	set(MPG123_LIBS ${LIBMPG123_DIR}/lib/libmpg123.lib)
elseif(UNIX)
    find_path(
        LIBMPG123_DIR
        NAMES lib/libmpg123${CMAKE_SHARED_LIBRARY_SUFFIX}
        HINTS ${THIRDPARTY_DIR}/mpg123
        )   
    set(MPG123_LIBS_DIR ${LIBMPG123_DIR}/lib)
    set(MPG123_LIBS_RELEASE_DLL ${MPG123_LIBS_DIR}/libmpg123${CMAKE_SHARED_LIBRARY_SUFFIX})
  	set(MPG123_LIBS ${MPG123_LIBS_RELEASE_DLL})
endif()

mark_as_advanced(MPG123_LIBS_DIR)

# promote package for find
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(mpg123 REQUIRED_VARS LIBMPG123_DIR MPG123_LIBS MPG123_LIBS_DIR)

add_library(mpg123 SHARED IMPORTED)
set_target_properties(mpg123 PROPERTIES
                      IMPORTED_CONFIGURATIONS "Debug;Release"
                      IMPORTED_LOCATION_RELEASE ${MPG123_LIBS_RELEASE_DLL}
                      IMPORTED_LOCATION_DEBUG ${MPG123_LIBS_RELEASE_DLL}
                      )

if(WIN32)
    set_target_properties(mpg123 PROPERTIES
                          IMPORTED_IMPLIB ${MPG123_LIBS}
                          )
endif()