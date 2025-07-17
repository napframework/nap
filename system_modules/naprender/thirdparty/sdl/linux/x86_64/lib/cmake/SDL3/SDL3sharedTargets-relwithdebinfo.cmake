#----------------------------------------------------------------
# Generated CMake target import file for configuration "RelWithDebInfo".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "SDL3::SDL3-shared" for configuration "RelWithDebInfo"
set_property(TARGET SDL3::SDL3-shared APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(SDL3::SDL3-shared PROPERTIES
  IMPORTED_LOCATION_RELWITHDEBINFO "${_IMPORT_PREFIX}/lib/libSDL3.so.0.2.18"
  IMPORTED_SONAME_RELWITHDEBINFO "libSDL3.so.0"
  )

list(APPEND _cmake_import_check_targets SDL3::SDL3-shared )
list(APPEND _cmake_import_check_files_for_SDL3::SDL3-shared "${_IMPORT_PREFIX}/lib/libSDL3.so.0.2.18" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
