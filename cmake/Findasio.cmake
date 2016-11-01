find_path(
        ASIO_INCLUDE_DIRS
        NAMES asio.hpp
        HINTS
        ${CMAKE_CURRENT_LIST_DIR}/../../asio
)

mark_as_advanced(ASIO_INCLUDE_DIRS)

if(ASIO_INCLUDE_DIRS)
    set(ASIO_FOUND TRUE)
endif()

mark_as_advanced(ASIO_FOUND)
mark_as_advanced(ASIO_CXX_FLAGS)

if(ASIO_FOUND)
    if(NOT ASIO_FIND_QUIETLY)
        message(STATUS "Found ASIO header files in ${ASIO_INCLUDE_DIRS}")
        if(DEFINED ASIO_CXX_FLAGS)
            message(STATUS "Found ASIO C++ extra compilation flags: ${ASIO_CXX_FLAGS}")
        endif()
    endif()
elseif(ASIO_FIND_REQUIRED)
    message(FATAL_ERROR "Could not find ASIO, consider adding ASIO path to CMAKE_PREFIX_PATH")
else()
    message(STATUS "Optional package ASIO was not found")
endif()