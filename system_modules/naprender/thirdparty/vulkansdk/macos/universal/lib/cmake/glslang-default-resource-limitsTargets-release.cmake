#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "glslang-default-resource-limits" for configuration "Release"
set_property(TARGET glslang-default-resource-limits APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(glslang-default-resource-limits PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libglslang-default-resource-limits.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS glslang-default-resource-limits )
list(APPEND _IMPORT_CHECK_FILES_FOR_glslang-default-resource-limits "${_IMPORT_PREFIX}/lib/libglslang-default-resource-limits.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
