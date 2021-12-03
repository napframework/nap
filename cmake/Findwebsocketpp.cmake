# Websocketpp Source Directory
find_path(WEBSOCKETPP_ROOT_DIR
    NAMES COPYING
    HINTS ${THIRDPARTY_DIR}/websocketpp
    )

# Websocketpp dist directory
find_path(WEBSOCKETPP_DIR 
          NAMES include/websocketpp/version.hpp
          HINTS ${WEBSOCKETPP_ROOT_DIR}/install
          )

mark_as_advanced(WEBSOCKETPP_DIR)
mark_as_advanced(WEBSOCKETPP_ROOT_DIR)

set(WEBSOCKETPP_INCLUDE_DIRS ${WEBSOCKETPP_ROOT_DIR}/include)
set(WEBSOCKETPP_DIST_FILES ${WEBSOCKETPP_ROOT_DIR}/COPYING)

# promote package for find
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(websocketpp REQUIRED_VARS WEBSOCKETPP_ROOT_DIR WEBSOCKETPP_DIR)