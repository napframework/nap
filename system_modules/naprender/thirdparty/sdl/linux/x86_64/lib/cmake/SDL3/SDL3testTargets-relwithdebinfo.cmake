#----------------------------------------------------------------
# Generated CMake target import file for configuration "RelWithDebInfo".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "SDL3::SDL3_test" for configuration "RelWithDebInfo"
set_property(TARGET SDL3::SDL3_test APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(SDL3::SDL3_test PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELWITHDEBINFO "C"
  IMPORTED_LOCATION_RELWITHDEBINFO "${_IMPORT_PREFIX}/lib/libSDL3_test.a"
  )

list(APPEND _cmake_import_check_targets SDL3::SDL3_test )
list(APPEND _cmake_import_check_files_for_SDL3::SDL3_test "${_IMPORT_PREFIX}/lib/libSDL3_test.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
