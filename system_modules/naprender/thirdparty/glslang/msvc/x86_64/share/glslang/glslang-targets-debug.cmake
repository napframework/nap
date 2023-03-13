#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "glslang::OSDependent" for configuration "Debug"
set_property(TARGET glslang::OSDependent APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(glslang::OSDependent PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/OSDependentd.lib"
  )

list(APPEND _IMPORT_CHECK_TARGETS glslang::OSDependent )
list(APPEND _IMPORT_CHECK_FILES_FOR_glslang::OSDependent "${_IMPORT_PREFIX}/lib/OSDependentd.lib" )

# Import target "glslang::glslang" for configuration "Debug"
set_property(TARGET glslang::glslang APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(glslang::glslang PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/glslangd.lib"
  )

list(APPEND _IMPORT_CHECK_TARGETS glslang::glslang )
list(APPEND _IMPORT_CHECK_FILES_FOR_glslang::glslang "${_IMPORT_PREFIX}/lib/glslangd.lib" )

# Import target "glslang::MachineIndependent" for configuration "Debug"
set_property(TARGET glslang::MachineIndependent APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(glslang::MachineIndependent PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/MachineIndependentd.lib"
  )

list(APPEND _IMPORT_CHECK_TARGETS glslang::MachineIndependent )
list(APPEND _IMPORT_CHECK_FILES_FOR_glslang::MachineIndependent "${_IMPORT_PREFIX}/lib/MachineIndependentd.lib" )

# Import target "glslang::GenericCodeGen" for configuration "Debug"
set_property(TARGET glslang::GenericCodeGen APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(glslang::GenericCodeGen PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/GenericCodeGend.lib"
  )

list(APPEND _IMPORT_CHECK_TARGETS glslang::GenericCodeGen )
list(APPEND _IMPORT_CHECK_FILES_FOR_glslang::GenericCodeGen "${_IMPORT_PREFIX}/lib/GenericCodeGend.lib" )

# Import target "glslang::OGLCompiler" for configuration "Debug"
set_property(TARGET glslang::OGLCompiler APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(glslang::OGLCompiler PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/OGLCompilerd.lib"
  )

list(APPEND _IMPORT_CHECK_TARGETS glslang::OGLCompiler )
list(APPEND _IMPORT_CHECK_FILES_FOR_glslang::OGLCompiler "${_IMPORT_PREFIX}/lib/OGLCompilerd.lib" )

# Import target "glslang::glslangValidator" for configuration "Debug"
set_property(TARGET glslang::glslangValidator APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(glslang::glslangValidator PROPERTIES
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/bin/glslangValidator.exe"
  )

list(APPEND _IMPORT_CHECK_TARGETS glslang::glslangValidator )
list(APPEND _IMPORT_CHECK_FILES_FOR_glslang::glslangValidator "${_IMPORT_PREFIX}/bin/glslangValidator.exe" )

# Import target "glslang::spirv-remap" for configuration "Debug"
set_property(TARGET glslang::spirv-remap APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(glslang::spirv-remap PROPERTIES
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/bin/spirv-remap.exe"
  )

list(APPEND _IMPORT_CHECK_TARGETS glslang::spirv-remap )
list(APPEND _IMPORT_CHECK_FILES_FOR_glslang::spirv-remap "${_IMPORT_PREFIX}/bin/spirv-remap.exe" )

# Import target "glslang::glslang-default-resource-limits" for configuration "Debug"
set_property(TARGET glslang::glslang-default-resource-limits APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(glslang::glslang-default-resource-limits PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/glslang-default-resource-limitsd.lib"
  )

list(APPEND _IMPORT_CHECK_TARGETS glslang::glslang-default-resource-limits )
list(APPEND _IMPORT_CHECK_FILES_FOR_glslang::glslang-default-resource-limits "${_IMPORT_PREFIX}/lib/glslang-default-resource-limitsd.lib" )

# Import target "glslang::SPVRemapper" for configuration "Debug"
set_property(TARGET glslang::SPVRemapper APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(glslang::SPVRemapper PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/SPVRemapperd.lib"
  )

list(APPEND _IMPORT_CHECK_TARGETS glslang::SPVRemapper )
list(APPEND _IMPORT_CHECK_FILES_FOR_glslang::SPVRemapper "${_IMPORT_PREFIX}/lib/SPVRemapperd.lib" )

# Import target "glslang::SPIRV" for configuration "Debug"
set_property(TARGET glslang::SPIRV APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(glslang::SPIRV PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/SPIRVd.lib"
  )

list(APPEND _IMPORT_CHECK_TARGETS glslang::SPIRV )
list(APPEND _IMPORT_CHECK_FILES_FOR_glslang::SPIRV "${_IMPORT_PREFIX}/lib/SPIRVd.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
