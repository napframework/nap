# default nanosvg directory
find_path(NANOSVG_DIR
          NAMES include/nanosvg.h
          HINTS ${THIRDPARTY_DIR}/nanosvg
          )

set(NANOSVG_INCLUDE_DIRS ${NANOSVG_DIR}/include)
mark_as_advanced(NANOSVG_INCLUDE_DIRS)

# promote package for find
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(nanosvg REQUIRED_VARS NANOSVG_DIR)
