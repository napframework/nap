# Websocketpp Source Directory
find_path(WEBSOCKETPP_ROOT_DIR
    NAMES COPYING
    HINTS ${NAP_ROOT}/system_modules/napwebsocket/thirdparty/websocketpp
    )
set(WEBSOCKETPP_LICENSE_FILES ${WEBSOCKETPP_ROOT_DIR}/COPYING)

# Websocketpp dist directory, header only
find_path(WEBSOCKETPP_DIR
          NAMES include/websocketpp/version.hpp
          HINTS ${WEBSOCKETPP_ROOT_DIR}/install
          )
set(WEBSOCKETPP_INCLUDE_DIR ${WEBSOCKETPP_DIR}/include)

set(LIBCRYPTO_LIBS "${WEBSOCKETPP_ROOT_DIR}/../libcrypto/libcrypto.so")
add_library(libcrypto SHARED IMPORTED)
set_target_properties(libcrypto PROPERTIES
        IMPORTED_CONFIGURATIONS "Debug;Release"
        IMPORTED_LOCATION_RELEASE ${LIBCRYPTO_LIBS}
        IMPORTED_LOCATION_DEBUG ${LIBCRYPTO_LIBS}
)

set(LIBSSL_LIBS "${WEBSOCKETPP_ROOT_DIR}/../libssl/libssl.so")
add_library(libssl SHARED IMPORTED)
set_target_properties(libssl PROPERTIES
        IMPORTED_CONFIGURATIONS "Debug;Release"
        IMPORTED_LOCATION_RELEASE ${LIBSSL_LIBS}
        IMPORTED_LOCATION_DEBUG ${LIBSSL_LIBS}
)


mark_as_advanced(WEBSOCKETPP_DIR)
mark_as_advanced(WEBSOCKETPP_ROOT_DIR)
mark_as_advanced(WEBSOCKETPP_INCLUDE_DIR)

# promote package for find
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(websocketpp REQUIRED_VARS WEBSOCKETPP_ROOT_DIR WEBSOCKETPP_DIR WEBSOCKETPP_LICENSE_FILES)
