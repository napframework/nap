#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "RTTR::Core" for configuration "Debug"
set_property(TARGET RTTR::Core APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(RTTR::Core PROPERTIES
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/bin/librttr_core_d.0.9.6.dylib"
  IMPORTED_SONAME_DEBUG "librttr_core_d.0.9.6.dylib"
  )

list(APPEND _IMPORT_CHECK_TARGETS RTTR::Core )
list(APPEND _IMPORT_CHECK_FILES_FOR_RTTR::Core "${_IMPORT_PREFIX}/bin/librttr_core_d.0.9.6.dylib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
