find_path(
        AWESOMIUM_INCLUDE_DIRS
        NAMES Awesomium/WebView.h
        HINTS
        ${CMAKE_CURRENT_LIST_DIR}/../../awesomium_v1.7.5_sdk_linux64/include
)


mark_as_advanced(AWESOMIUM_INCLUDE_DIRS)

if(AWESOMIUM_INCLUDE_DIRS)
    set(AWESOMIUM_FOUND TRUE)
endif()

mark_as_advanced(AWESOMIUM_FOUND)
mark_as_advanced(AWESOMIUM_CXX_FLAGS)

if(AWESOMIUM_FOUND)
    if(NOT AWESOMIUM_FIND_QUIETLY)
        message(STATUS "Found awesomium header files in ${AWESOMIUM_INCLUDE_DIRS}")
        if(DEFINED AWESOMIUM_CXX_FLAGS)
            message(STATUS "Found awesomium C++ extra compilation flags: ${AWESOMIUM_CXX_FLAGS}")
        endif()
    endif()
elseif(AWESOMIUM_FIND_REQUIRED)
    message(FATAL_ERROR "Could not find awesomium, consider adding awesomium path to CMAKE_PREFIX_PATH")
else()
    message(STATUS "Optional package awesomium was not found")
endif()

set(AWESOMIUM_LIBRARIES ${AWESOMIUM_INCLUDE_DIRS}/../bin/libawesomium-1-7.so.5.0)
