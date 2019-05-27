find_path(ASIO_DIR 
          NAMES include/asio.hpp
          HINTS ${THIRDPARTY_DIR}/asio
          )

mark_as_advanced(ASIO_DIR)
if(ASIO_DIR)
    set(ASIO_FOUND true)
endif()
mark_as_advanced(ASIO_FOUND)

set(ASIO_INCLUDE_DIRS ${ASIO_DIR}/include)
mark_as_advanced(ASIO_INCLUDE_DIRS)

if(ASIO_FOUND)
    message(STATUS "Found asio header files: ${ASIO_INCLUDE_DIRS}")
endif()

# promote package for find
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(asio REQUIRED_VARS ASIO_INCLUDE_DIRS)
