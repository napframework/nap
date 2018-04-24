# default moodycamel directory
find_path(MOODYCAMEL_DIR
          NAMES concurrentqueue.h
          HINTS ${THIRDPARTY_DIR}/moodycamel
          )

mark_as_advanced(MOODYCAMEL_DIR)
if(MOODYCAMEL_DIR)
    set(MOODYCAMEL_FOUND true)
endif()
mark_as_advanced(MOODYCAMEL_FOUND)

set(MOODYCAMEL_INCLUDE_DIRS ${MOODYCAMEL_DIR})
mark_as_advanced(MOODYCAMEL_INCLUDE_DIRS)

if(MOODYCAMEL_FOUND)
    message(STATUS "Found moodycamel header files in ${MOODYCAMEL_INCLUDE_DIRS}")
endif()


# promote package for find
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(moodycamel REQUIRED_VARS MOODYCAMEL_INCLUDE_DIRS)
