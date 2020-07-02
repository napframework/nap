if(APPLE)
	find_library(GLSLANG_LIBRARY
				 NAMES glslang
				 PATHS ${THIRDPARTY_DIR}/glslang/install/lib/osx
				 NO_DEFAULT_PATH
				 )
	
	if (GLSLANG_LIBRARY)	
		set(GLSLANG_LIBS_DEBUG ${GLSLANG_LIBRARY})
		set(GLSLANG_LIBS_RELEASE ${GLSLANG_LIBRARY})	
	endif()
elseif(UNIX)
	find_library(GLSLANG_LIBRARY
				 NAMES glslang
				 PATHS ${THIRDPARTY_DIR}/glslang/install/lib/linux
				 NO_DEFAULT_PATH
				 )

	if (GLSLANG_LIBRARY)
		set(GLSLANG_LIBS_DEBUG ${GLSLANG_LIBRARY})
		set(GLSLANG_LIBS_RELEASE ${GLSLANG_LIBRARY})
	endif()
else()
	find_path(GLSLANG_INCLUDE_DIR
		  NAMES glslang/Public/ShaderLang.h
		  HINTS ${THIRDPARTY_DIR}/glslang/msvc/install/include
		  )

	find_library(GLSLANG_LIBRARY_DEBUG
				 NAMES glslangd
				 PATHS ${THIRDPARTY_DIR}/glslang/msvc/install/lib				   
				 NO_DEFAULT_PATH
				)
				
	find_library(OSDEPENDENT_LIBRARY_DEBUG
				 NAMES OSDependentd
				 PATHS ${THIRDPARTY_DIR}/glslang/msvc/install/lib				   
				 NO_DEFAULT_PATH
				)
				
	find_library(SPIRV_LIBRARY_DEBUG
				 NAMES SPIRVd
				 PATHS ${THIRDPARTY_DIR}/glslang/msvc/install/lib
				 NO_DEFAULT_PATH
				)
				
	find_library(OGLCOMPILER_LIBRARY_DEBUG
				 NAMES OGLCompilerd
				 PATHS ${THIRDPARTY_DIR}/glslang/msvc/install/lib
				 NO_DEFAULT_PATH
				)					

	find_library(GLSLANG_LIBRARY_RELEASE
				 NAMES glslang
				 PATHS ${THIRDPARTY_DIR}/glslang/msvc/install/lib
				 NO_DEFAULT_PATH
				)
				
	find_library(OSDEPENDENT_LIBRARY_RELEASE
				 NAMES OSDependent
				 PATHS ${THIRDPARTY_DIR}/glslang/msvc/install/lib
				 NO_DEFAULT_PATH
				)	
				
	find_library(SPIRV_LIBRARY_RELEASE
				 NAMES SPIRV
				 PATHS ${THIRDPARTY_DIR}/glslang/msvc/install/lib
				 NO_DEFAULT_PATH
				)
				
	find_library(OGLCOMPILER_LIBRARY_RELEASE
				 NAMES OGLCompiler
				 PATHS ${THIRDPARTY_DIR}/glslang/msvc/install/lib
				 NO_DEFAULT_PATH
				)					
				
	if (GLSLANG_LIBRARY_DEBUG AND GLSLANG_LIBRARY_RELEASE AND OSDEPENDENT_LIBRARY_RELEASE AND OGLCOMPILER_LIBRARY_RELEASE
			AND SPIRV_LIBRARY_DEBUG AND SPIRV_LIBRARY_RELEASE AND OSDEPENDENT_LIBRARY_DEBUG AND OGLCOMPILER_LIBRARY_DEBUG)
		
		set(GLSLANG_LIBS_DEBUG ${GLSLANG_LIBRARY_DEBUG} ${SPIRV_LIBRARY_DEBUG} ${OSDEPENDENT_LIBRARY_DEBUG} ${OGLCOMPILER_LIBRARY_DEBUG})
		set(GLSLANG_LIBS_RELEASE ${GLSLANG_LIBRARY_RELEASE} ${SPIRV_LIBRARY_RELEASE} ${OSDEPENDENT_LIBRARY_RELEASE} ${OGLCOMPILER_LIBRARY_RELEASE})
	endif()				
endif()		  

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(glslang REQUIRED_VARS GLSLANG_INCLUDE_DIR GLSLANG_LIBS_DEBUG GLSLANG_LIBS_RELEASE)