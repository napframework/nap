find_path(WEBSOCKETPP_DIR 
          NAMES include/websocketpp/version.hpp
          HINTS ${THIRDPARTY_DIR}/websocketpp/install
          )

mark_as_advanced(WEBSOCKETPP_DIR)
if(WEBSOCKETPP_DIR)
    set(WEBSOCKETPP_FOUND true)
endif()
mark_as_advanced(WEBSOCKETPP_FOUND)

set(WEBSOCKETPP_INCLUDE_DIRS ${WEBSOCKETPP_DIR}/include)
mark_as_advanced(WEBSOCKETPP_INCLUDE_DIRS)

if(WEBSOCKETPP_FOUND)
    message(STATUS "Found websocket header files: ${WEBSOCKETPP_INCLUDE_DIRS}")
endif()

# promote package for find
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(websocketpp REQUIRED_VARS WEBSOCKETPP_INCLUDE_DIRS)
