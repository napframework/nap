#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "SndFile::sndfile" for configuration "Release"
set_property(TARGET SndFile::sndfile APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(SndFile::sndfile PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/sndfile.lib"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/sndfile.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS SndFile::sndfile )
list(APPEND _IMPORT_CHECK_FILES_FOR_SndFile::sndfile "${_IMPORT_PREFIX}/lib/sndfile.lib" "${_IMPORT_PREFIX}/bin/sndfile.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
