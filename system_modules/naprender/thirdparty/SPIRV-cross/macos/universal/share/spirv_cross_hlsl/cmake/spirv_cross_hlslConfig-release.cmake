#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "spirv-cross-hlsl" for configuration "Release"
set_property(TARGET spirv-cross-hlsl APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(spirv-cross-hlsl PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libspirv-cross-hlsl.a"
  )

list(APPEND _cmake_import_check_targets spirv-cross-hlsl )
list(APPEND _cmake_import_check_files_for_spirv-cross-hlsl "${_IMPORT_PREFIX}/lib/libspirv-cross-hlsl.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
