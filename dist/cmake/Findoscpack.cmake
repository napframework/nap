# default oscpack directory
find_path(OSCPACK_DIR
          NAMES osc/OscTypes.h
          HINTS ${THIRDPARTY_DIR}/oscpack/include
          )

# include directory is universal
set(OSCPACK_INCLUDE_DIRS ${OSCPACK_DIR})

mark_as_advanced(OSCPACK_INCLUDE_DIRS)

# promote package for find
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(oscpack REQUIRED_VARS OSCPACK_DIR OSCPACK_INCLUDE_DIRS)