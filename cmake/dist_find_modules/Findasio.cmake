find_path(ASIO_DIR 
          NAMES include/asio.hpp
          HINTS ${THIRDPARTY_DIR}/asio
          )

mark_as_advanced(ASIO_DIR)
set(ASIO_INCLUDE_DIRS ${ASIO_DIR}/include)

# promote package for find
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(asio REQUIRED_VARS ASIO_DIR)