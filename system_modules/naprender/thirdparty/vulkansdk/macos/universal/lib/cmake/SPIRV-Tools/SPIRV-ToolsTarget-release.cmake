#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "SPIRV-Tools-static" for configuration "Release"
set_property(TARGET SPIRV-Tools-static APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(SPIRV-Tools-static PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libSPIRV-Tools.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS SPIRV-Tools-static )
list(APPEND _IMPORT_CHECK_FILES_FOR_SPIRV-Tools-static "${_IMPORT_PREFIX}/lib/libSPIRV-Tools.a" )

# Import target "SPIRV-Tools-shared" for configuration "Release"
set_property(TARGET SPIRV-Tools-shared APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(SPIRV-Tools-shared PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libSPIRV-Tools-shared.dylib"
  IMPORTED_SONAME_RELEASE "libSPIRV-Tools-shared.dylib"
  )

list(APPEND _IMPORT_CHECK_TARGETS SPIRV-Tools-shared )
list(APPEND _IMPORT_CHECK_FILES_FOR_SPIRV-Tools-shared "${_IMPORT_PREFIX}/lib/libSPIRV-Tools-shared.dylib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
