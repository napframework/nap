find_path(
        SPOUT_INCLUDE_DIRS
        NAMES Spout.h
        HINTS
        ${CMAKE_CURRENT_LIST_DIR}/../../spout2/SpoutSDK/Source
)

mark_as_advanced(SPOUT_INCLUDE_DIRS)

if(SPOUT_INCLUDE_DIRS)
    set(SPOUT_FOUND TRUE)
endif()

mark_as_advanced(SPOUT_FOUND)
mark_as_advanced(SPOUT_CXX_FLAGS)

if(SPOUT_FOUND)
    if(NOT SPOUT_FIND_QUIETLY)
        message(STATUS "Found SPOUT header files in ${SPOUT_INCLUDE_DIRS}")
        if(DEFINED SPOUT_CXX_FLAGS)
            message(STATUS "Found SPOUT C++ extra compilation flags: ${SPOUT_CXX_FLAGS}")
        endif()
    endif()
elseif(SPOUT_FIND_REQUIRED)
    message(FATAL_ERROR "Could not find spout, consider adding SPOUT path to CMAKE_PREFIX_PATH")
else()
    message(STATUS "Optional package spout was not found")
endif()