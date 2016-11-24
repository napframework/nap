 find_path(
        JSONRPCLEAN_INCLUDE_DIRS
        NAMES jsonrpc-lean/json.h
        HINTS
        ${CMAKE_CURRENT_LIST_DIR}/../../jsonrpc-lean/include
         ${CMAKE_CURRENT_LIST_DIR}/../../thirdparty/jsonrpc-lean/include
)

mark_as_advanced(JSONRPCLEAN_INCLUDE_DIRS)

if(JSONRPCLEAN_INCLUDE_DIRS)
    set(JSONRPCLEAN_FOUND TRUE)
endif()

mark_as_advanced(JSONRPCLEAN_FOUND)
mark_as_advanced(JSONRPCLEAN_CXX_FLAGS)

if(JSONRPCLEAN_FOUND)
    if(NOT JSONRPCLEAN_FIND_QUIETLY)
        message(STATUS "Found jsonrpc-lean header files in ${JSONRPCLEAN_INCLUDE_DIRS}")
        if(DEFINED JSONRPCLEAN_CXX_FLAGS)
            message(STATUS "Found jsonrpc-lean C++ extra compilation flags: ${JSONRPCLEAN_CXX_FLAGS}")
        endif()
    endif()
elseif(JSONRPCLEAN_FIND_REQUIRED)
    message(FATAL_ERROR "Could not find jsonrpc-lean, consider adding jsonrpc-lean path to CMAKE_PREFIX_PATH")
else()
    message(STATUS "Optional package jsonrpc-lean was not found")
endif()