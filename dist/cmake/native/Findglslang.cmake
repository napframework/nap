find_path(GLSLANG_INCLUDE_DIR
		  NAMES glslang/Public/ShaderLang.h
		  HINTS ${THIRDPARTY_DIR}/glslang/include
		  )

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(glslang REQUIRED_VARS GLSLANG_INCLUDE_DIR)