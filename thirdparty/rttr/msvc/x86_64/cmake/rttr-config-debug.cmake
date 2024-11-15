#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "RTTR::Core" for configuration "Debug"
set_property(TARGET RTTR::Core APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(RTTR::Core PROPERTIES
  IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/lib/rttr_core_d.lib"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/bin/rttr_core_d.dll"
  )

list(APPEND _cmake_import_check_targets RTTR::Core )
list(APPEND _cmake_import_check_files_for_RTTR::Core "${_IMPORT_PREFIX}/lib/rttr_core_d.lib" "${_IMPORT_PREFIX}/bin/rttr_core_d.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
