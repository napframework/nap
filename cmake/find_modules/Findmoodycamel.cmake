# default moodycamel directory
find_path(MOODYCAMEL_DIR
          NAMES concurrentqueue.h
          NO_CMAKE_FIND_ROOT_PATH
          HINTS ${THIRDPARTY_DIR}/moodycamel
          )

set(MOODYCAMEL_INCLUDE_DIRS ${MOODYCAMEL_DIR})
mark_as_advanced(MOODYCAMEL_INCLUDE_DIRS)

# promote package for find
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(moodycamel REQUIRED_VARS MOODYCAMEL_DIR)
