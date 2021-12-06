find_path(ASIO_DIR 
          NAMES include/asio.hpp
          HINTS ${THIRDPARTY_DIR}/asio
          )

mark_as_advanced(ASIO_DIR)
set(ASIO_INCLUDE_DIR ${ASIO_DIR}/include)
set(ASIO_DIST_FILES ${ASIO_DIR}/LICENSE_1_0.txt)

# promote package for find
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(asio REQUIRED_VARS ASIO_DIR ASIO_INCLUDE_DIR ASIO_DIST_FILES)