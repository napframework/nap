#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "glslang::OSDependent" for configuration "Release"
set_property(TARGET glslang::OSDependent APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(glslang::OSDependent PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/OSDependent.lib"
  )

list(APPEND _IMPORT_CHECK_TARGETS glslang::OSDependent )
list(APPEND _IMPORT_CHECK_FILES_FOR_glslang::OSDependent "${_IMPORT_PREFIX}/lib/OSDependent.lib" )

# Import target "glslang::glslang" for configuration "Release"
set_property(TARGET glslang::glslang APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(glslang::glslang PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/glslang.lib"
  )

list(APPEND _IMPORT_CHECK_TARGETS glslang::glslang )
list(APPEND _IMPORT_CHECK_FILES_FOR_glslang::glslang "${_IMPORT_PREFIX}/lib/glslang.lib" )

# Import target "glslang::MachineIndependent" for configuration "Release"
set_property(TARGET glslang::MachineIndependent APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(glslang::MachineIndependent PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/MachineIndependent.lib"
  )

list(APPEND _IMPORT_CHECK_TARGETS glslang::MachineIndependent )
list(APPEND _IMPORT_CHECK_FILES_FOR_glslang::MachineIndependent "${_IMPORT_PREFIX}/lib/MachineIndependent.lib" )

# Import target "glslang::GenericCodeGen" for configuration "Release"
set_property(TARGET glslang::GenericCodeGen APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(glslang::GenericCodeGen PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/GenericCodeGen.lib"
  )

list(APPEND _IMPORT_CHECK_TARGETS glslang::GenericCodeGen )
list(APPEND _IMPORT_CHECK_FILES_FOR_glslang::GenericCodeGen "${_IMPORT_PREFIX}/lib/GenericCodeGen.lib" )

# Import target "glslang::OGLCompiler" for configuration "Release"
set_property(TARGET glslang::OGLCompiler APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(glslang::OGLCompiler PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/OGLCompiler.lib"
  )

list(APPEND _IMPORT_CHECK_TARGETS glslang::OGLCompiler )
list(APPEND _IMPORT_CHECK_FILES_FOR_glslang::OGLCompiler "${_IMPORT_PREFIX}/lib/OGLCompiler.lib" )

# Import target "glslang::glslangValidator" for configuration "Release"
set_property(TARGET glslang::glslangValidator APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(glslang::glslangValidator PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/glslangValidator.exe"
  )

list(APPEND _IMPORT_CHECK_TARGETS glslang::glslangValidator )
list(APPEND _IMPORT_CHECK_FILES_FOR_glslang::glslangValidator "${_IMPORT_PREFIX}/bin/glslangValidator.exe" )

# Import target "glslang::spirv-remap" for configuration "Release"
set_property(TARGET glslang::spirv-remap APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(glslang::spirv-remap PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/spirv-remap.exe"
  )

list(APPEND _IMPORT_CHECK_TARGETS glslang::spirv-remap )
list(APPEND _IMPORT_CHECK_FILES_FOR_glslang::spirv-remap "${_IMPORT_PREFIX}/bin/spirv-remap.exe" )

# Import target "glslang::glslang-default-resource-limits" for configuration "Release"
set_property(TARGET glslang::glslang-default-resource-limits APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(glslang::glslang-default-resource-limits PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/glslang-default-resource-limits.lib"
  )

list(APPEND _IMPORT_CHECK_TARGETS glslang::glslang-default-resource-limits )
list(APPEND _IMPORT_CHECK_FILES_FOR_glslang::glslang-default-resource-limits "${_IMPORT_PREFIX}/lib/glslang-default-resource-limits.lib" )

# Import target "glslang::SPVRemapper" for configuration "Release"
set_property(TARGET glslang::SPVRemapper APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(glslang::SPVRemapper PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/SPVRemapper.lib"
  )

list(APPEND _IMPORT_CHECK_TARGETS glslang::SPVRemapper )
list(APPEND _IMPORT_CHECK_FILES_FOR_glslang::SPVRemapper "${_IMPORT_PREFIX}/lib/SPVRemapper.lib" )

# Import target "glslang::SPIRV" for configuration "Release"
set_property(TARGET glslang::SPIRV APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(glslang::SPIRV PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/SPIRV.lib"
  )

list(APPEND _IMPORT_CHECK_TARGETS glslang::SPIRV )
list(APPEND _IMPORT_CHECK_FILES_FOR_glslang::SPIRV "${_IMPORT_PREFIX}/lib/SPIRV.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
