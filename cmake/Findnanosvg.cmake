# default artnet directory
find_path(NANOSVG_DIR
          NAMES src/nanosvg.h
          HINTS ${THIRDPARTY_DIR}/nanosvg
          )

mark_as_advanced(NANOSVG_DIR)
set(NANOSVG_INCLUDE_DIRS ${NANOSVG_DIR}/src)
mark_as_advanced(NANOSVG_INCLUDE_DIRS)

# promote package for find
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(nanosvg REQUIRED_VARS NANOSVG_DIR)
