#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "RTTR::Core" for configuration "Release"
set_property(TARGET RTTR::Core APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(RTTR::Core PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/rttr_core.lib"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/rttr_core.dll"
  )

list(APPEND _cmake_import_check_targets RTTR::Core )
list(APPEND _cmake_import_check_files_for_RTTR::Core "${_IMPORT_PREFIX}/lib/rttr_core.lib" "${_IMPORT_PREFIX}/bin/rttr_core.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
