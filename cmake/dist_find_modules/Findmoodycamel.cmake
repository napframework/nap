# default moodycamel directory
find_path(MOODYCAMEL_DIR
          NO_CMAKE_FIND_ROOT_PATH
          NAMES concurrentqueue.h
          HINTS ${THIRDPARTY_DIR}/moodycamel
          )

mark_as_advanced(MOODYCAMEL_DIR)
set(MOODYCAMEL_INCLUDE_DIRS ${MOODYCAMEL_DIR})
mark_as_advanced(MOODYCAMEL_INCLUDE_DIRS)

# promote package for find
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(moodycamel REQUIRED_VARS MOODYCAMEL_DIR)
