# get install dir
if(APPLE)
	find_path(
		GLSLANG_DIR
		NAMES include/glslang/Public/ShaderLang.h
		HINTS ${THIRDPARTY_DIR}/glslang/osx/install
		)
elseif(UNIX)
	find_path(
		GLSLANG_DIR
		NAMES include/glslang/Public/ShaderLang.h
		HINTS ${THIRDPARTY_DIR}/glslang/linux/install
		)

	# static libs
	find_library(GLSLANG_LIBRARY_RELEASE
			 NAMES glslang
			 PATHS ${GLSLANG_DIR}/lib	   
			 NO_DEFAULT_PATH
			)
				
	find_library(OSDEPENDENT_LIBRARY_RELEASE
			 NAMES OSDependent
			 PATHS ${GLSLANG_DIR}/lib		   
			 NO_DEFAULT_PATH
			)
				
	find_library(SPIRV_LIBRARY_RELEASE
			 NAMES SPIRV
			 PATHS ${GLSLANG_DIR}/lib
			 NO_DEFAULT_PATH
			)
				
	find_library(OGLCOMPILER_LIBRARY_RELEASE
			 NAMES OGLCompiler
			 PATHS ${GLSLANG_DIR}/lib
			 NO_DEFAULT_PATH
			)

	set(GLSLANG_LIBRARY_DEBUG ${GLSLANG_LIBRARY_RELEASE})
	set(OSDEPENDENT_LIBRARY_DEBUG ${OSDEPENDENT_LIBRARY_RELEASE})
	set(SPIRV_LIBRARY_DEBUG ${SPIRV_LIBRARY_RELEASE})
	set(OGLCOMPILER_LIBRARY_DEBUG ${OGLCOMPILER_LIBRARY_RELEASE})				

elseif(WIN32)
	find_path(
		GLSLANG_DIR
		NAMES include/glslang/Public/ShaderLang.h
		HINTS ${THIRDPARTY_DIR}/glslang/msvc/install
		)

		# static libs
		find_library(GLSLANG_LIBRARY_DEBUG
			 NAMES glslangd
			 PATHS ${GLSLANG_DIR}/lib				   
			 NO_DEFAULT_PATH
			)
				
		find_library(OSDEPENDENT_LIBRARY_DEBUG
			 NAMES OSDependentd
			 PATHS ${GLSLANG_DIR}/lib				   
			 NO_DEFAULT_PATH
			)
				
		find_library(SPIRV_LIBRARY_DEBUG
			 NAMES SPIRVd
			 PATHS ${GLSLANG_DIR}/lib
			 NO_DEFAULT_PATH
			)
				
		find_library(OGLCOMPILER_LIBRARY_DEBUG
			 NAMES OGLCompilerd
			 PATHS ${GLSLANG_DIR}/lib
			 NO_DEFAULT_PATH
			)					

		find_library(GLSLANG_LIBRARY_RELEASE
			 NAMES glslang
			 PATHS ${GLSLANG_DIR}/lib
			 NO_DEFAULT_PATH
			)
				
		find_library(OSDEPENDENT_LIBRARY_RELEASE
			 NAMES OSDependent
			 PATHS ${GLSLANG_DIR}/lib
			 NO_DEFAULT_PATH
			)	
				
		find_library(SPIRV_LIBRARY_RELEASE
			 NAMES SPIRV
			 PATHS ${GLSLANG_DIR}/lib
			 NO_DEFAULT_PATH
			)
				
		find_library(OGLCOMPILER_LIBRARY_RELEASE
			 NAMES OGLCompiler
			 PATHS ${GLSLANG_DIR}/lib
			 NO_DEFAULT_PATH
			)		
endif()

# include directory
find_path(GLSLANG_INCLUDE_DIR
			NAMES glslang/Public/ShaderLang.h
			HINTS ${GLSLANG_DIR}/include
			)		

# Setup libraries when all found				
if (GLSLANG_LIBRARY_DEBUG AND GLSLANG_LIBRARY_RELEASE AND OSDEPENDENT_LIBRARY_RELEASE AND OGLCOMPILER_LIBRARY_RELEASE
		AND SPIRV_LIBRARY_DEBUG AND SPIRV_LIBRARY_RELEASE AND OSDEPENDENT_LIBRARY_DEBUG AND OGLCOMPILER_LIBRARY_DEBUG)
	
	set(GLSLANG_LIBS_DEBUG ${GLSLANG_LIBRARY_DEBUG} ${SPIRV_LIBRARY_DEBUG} ${OSDEPENDENT_LIBRARY_DEBUG} ${OGLCOMPILER_LIBRARY_DEBUG})
	set(GLSLANG_LIBS_RELEASE ${GLSLANG_LIBRARY_RELEASE} ${SPIRV_LIBRARY_RELEASE} ${OSDEPENDENT_LIBRARY_RELEASE} ${OGLCOMPILER_LIBRARY_RELEASE})
endif()					  

# not configurable
mark_as_advanced(GLSLANG_DIR)
mark_as_advanced(GLSLANG_LIBS_DEBUG)
mark_as_advanced(GLSLANG_LIBS_RELEASE)
mark_as_advanced(GLSLANG_INCLUDE_DIR)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(glslang REQUIRED_VARS GLSLANG_DIR GLSLANG_INCLUDE_DIR GLSLANG_LIBS_DEBUG GLSLANG_LIBS_RELEASE)