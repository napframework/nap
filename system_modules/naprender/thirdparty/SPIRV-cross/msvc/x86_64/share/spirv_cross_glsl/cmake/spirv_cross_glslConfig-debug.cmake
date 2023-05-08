#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "spirv-cross-glsl" for configuration "Debug"
set_property(TARGET spirv-cross-glsl APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(spirv-cross-glsl PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/spirv-cross-glsld.lib"
  )

list(APPEND _IMPORT_CHECK_TARGETS spirv-cross-glsl )
list(APPEND _IMPORT_CHECK_FILES_FOR_spirv-cross-glsl "${_IMPORT_PREFIX}/lib/spirv-cross-glsld.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
