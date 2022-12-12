find_path(TCLAP_DIR
          NAMES include/tclap/CmdLine.h
          HINTS
          ${THIRDPARTY_DIR}/tclap
          )

set(TCLAP_INCLUDE_DIRS ${TCLAP_DIR}/include)

mark_as_advanced(TCLAP_DIR)
mark_as_advanced(TCLAP_INCLUDE_DIRS)

if(TCLAP_DIR)
    set(TCLAP_FOUND TRUE)
endif()

mark_as_advanced(TCLAP_FOUND)
mark_as_advanced(TCLAP_CXX_FLAGS)

if(TCLAP_FOUND)
    if(NOT TCLAP_FIND_QUIETLY)
        message(STATUS "Found tclap header files in ${TCLAP_INCLUDE_DIRS}")
        if(DEFINED TCLAP_CXX_FLAGS)
            message(STATUS "Found tclap C++ extra compilation flags: ${TCLAP_CXX_FLAGS}")
        endif()
    endif()
elseif(TCLAP_FIND_REQUIRED)
    message(FATAL_ERROR "Could not find tclap, consider adding tclap path to CMAKE_PREFIX_PATH")
else()
    message(STATUS "Optional package tclap was not found")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(tclap REQUIRED_VARS TCLAP_INCLUDE_DIRS)
