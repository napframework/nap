set(NATIVEFILEDIALOG_FOUND true)
mark_as_advanced(NATIVEFILEDIALOG_FOUND)

set (NATIVEFILEDIALOG_DUMMY "dummy")
mark_as_advanced(NATIVEFILEDIALOG_DUMMY)

if(NATIVEFILEDIALOG_FOUND)
    message(STATUS "Found NATIVEFILEDIALOG")
endif()

# promote package for find
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(nativefiledialog REQUIRED_VARS NATIVEFILEDIALOG_DUMMY)
