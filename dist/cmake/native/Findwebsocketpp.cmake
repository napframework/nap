find_path(WEBSOCKETPP_DIR 
          NAMES include/websocketpp/version.hpp
          HINTS ${THIRDPARTY_DIR}/websocketpp
          )

mark_as_advanced(WEBSOCKETPP_DIR)
set(WEBSOCKETPP_INCLUDE_DIRS ${WEBSOCKETPP_DIR}/include)

# promote package for find
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(websocketpp REQUIRED_VARS WEBSOCKETPP_DIR)
