set(glslang_search_dir ${NAP_ROOT}/system_modules/naprender/thirdparty/glslang)
# Get install dir
if(WIN32)
    # Root dir
    find_path(
            GLSLANG_DIR
            NAMES include/glslang/Public/ShaderLang.h
            HINTS ${glslang_search_dir}/msvc/x86_64)

    # Static libs
    find_library(GENERIC_CODE_GEN_LIBRARY_RELEASE
            NAMES GenericCodeGen
            PATHS ${GLSLANG_DIR}/lib
            NO_DEFAULT_PATH REQUIRED
            )

    find_library(GLSLANG_LIBRARY_RELEASE
            NAMES glslang
            PATHS ${GLSLANG_DIR}/lib
            NO_DEFAULT_PATH REQUIRED
            )

    find_library(MACHINE_INDEPENDENT_LIBRARY_RELEASE
            NAMES MachineIndependent
            PATHS ${GLSLANG_DIR}/lib
            NO_DEFAULT_PATH REQUIRED
            )

    find_library(OGLCOMPILER_LIBRARY_RELEASE
            NAMES OGLCompiler
            PATHS ${GLSLANG_DIR}/lib
            NO_DEFAULT_PATH REQUIRED
            )

    find_library(OSDEPENDENT_LIBRARY_RELEASE
            NAMES OSDependent
            PATHS ${GLSLANG_DIR}/lib
            NO_DEFAULT_PATH REQUIRED
            )

    find_library(SPIRV_LIBRARY_RELEASE
            NAMES SPIRV
            PATHS ${GLSLANG_DIR}/lib
            NO_DEFAULT_PATH REQUIRED
            )

    find_library(SPV_REMAPPER_LIBRARY_RELEASE
            NAMES SPVRemapper
            PATHS ${GLSLANG_DIR}/lib
            NO_DEFAULT_PATH REQUIRED
            )

    find_library(GENERIC_CODE_GEN_LIBRARY_DEBUG
            NAMES GenericCodeGend
            PATHS ${GLSLANG_DIR}/lib
            NO_DEFAULT_PATH REQUIRED
            )

    find_library(GLSLANG_LIBRARY_DEBUG
            NAMES glslangd
            PATHS ${GLSLANG_DIR}/lib
            NO_DEFAULT_PATH REQUIRED
            )

    find_library(MACHINE_INDEPENDENT_LIBRARY_DEBUG
            NAMES MachineIndependentd
            PATHS ${GLSLANG_DIR}/lib
            NO_DEFAULT_PATH REQUIRED
            )

    find_library(OGLCOMPILER_LIBRARY_DEBUG
            NAMES OGLCompilerd
            PATHS ${GLSLANG_DIR}/lib
            NO_DEFAULT_PATH REQUIRED
            )

    find_library(OSDEPENDENT_LIBRARY_DEBUG
            NAMES OSDependentd
            PATHS ${GLSLANG_DIR}/lib
            NO_DEFAULT_PATH REQUIRED
            )

    find_library(SPIRV_LIBRARY_DEBUG
            NAMES SPIRVd
            PATHS ${GLSLANG_DIR}/lib
            NO_DEFAULT_PATH REQUIRED
            )

    find_library(SPV_REMAPPER_LIBRARY_DEBUG
            NAMES SPVRemapperd
            PATHS ${GLSLANG_DIR}/lib
            NO_DEFAULT_PATH REQUIRED
            )
else()
    # Includes
    if(APPLE)
        # Root dir
        find_path(
                GLSLANG_DIR
                NAMES include/glslang/Public/ShaderLang.h
                HINTS ${glslang_search_dir}/macos/x86_64
        )
    elseif(UNIX)
        # Root dir
        find_path(
                GLSLANG_DIR
                NAMES include/glslang/Public/ShaderLang.h
                HINTS ${glslang_search_dir}/linux/${ARCH}
        )
    endif()

    # Static libs
    find_library(GENERIC_CODE_GEN_LIBRARY_RELEASE
            NAMES GenericCodeGen
            PATHS ${GLSLANG_DIR}/lib
            NO_DEFAULT_PATH REQUIRED
            )

    find_library(GLSLANG_LIBRARY_RELEASE
            NAMES glslang
            PATHS ${GLSLANG_DIR}/lib
            NO_DEFAULT_PATH REQUIRED
            )

    find_library(MACHINE_INDEPENDENT_LIBRARY_RELEASE
            NAMES MachineIndependent
            PATHS ${GLSLANG_DIR}/lib
            NO_DEFAULT_PATH REQUIRED
            )

    find_library(OGLCOMPILER_LIBRARY_RELEASE
            NAMES OGLCompiler
            PATHS ${GLSLANG_DIR}/lib
            NO_DEFAULT_PATH REQUIRED
            )

    find_library(OSDEPENDENT_LIBRARY_RELEASE
            NAMES OSDependent
            PATHS ${GLSLANG_DIR}/lib
            NO_DEFAULT_PATH REQUIRED
            )

    find_library(SPIRV_LIBRARY_RELEASE
            NAMES SPIRV
            PATHS ${GLSLANG_DIR}/lib
            NO_DEFAULT_PATH REQUIRED
            )

    find_library(SPV_REMAPPER_LIBRARY_RELEASE
            NAMES SPVRemapper
            PATHS ${GLSLANG_DIR}/lib
            NO_DEFAULT_PATH REQUIRED
            )

    set(GLSLANG_LIBRARY_DEBUG ${GLSLANG_LIBRARY_RELEASE})
    set(OSDEPENDENT_LIBRARY_DEBUG ${OSDEPENDENT_LIBRARY_RELEASE})
    set(SPIRV_LIBRARY_DEBUG ${SPIRV_LIBRARY_RELEASE})
    set(OGLCOMPILER_LIBRARY_DEBUG ${OGLCOMPILER_LIBRARY_RELEASE})
    set(MACHINE_INDEPENDENT_LIBRARY_DEBUG ${MACHINE_INDEPENDENT_LIBRARY_RELEASE})
    set(SPV_REMAPPER_LIBRARY_DEBUG ${SPV_REMAPPER_LIBRARY_RELEASE})
    set(GENERIC_CODE_GEN_LIBRARY_DEBUG ${GENERIC_CODE_GEN_LIBRARY_RELEASE})

endif()

# Include directory
find_path(GLSLANG_INCLUDE_DIR
            NAMES glslang/Public/ShaderLang.h
            HINTS ${GLSLANG_DIR}/include
            )

# Setup libraries when all found
set(GLSLANG_LIBS_DEBUG
        ${GLSLANG_LIBRARY_DEBUG}
        ${SPIRV_LIBRARY_DEBUG}
        ${OSDEPENDENT_LIBRARY_DEBUG}
        ${OGLCOMPILER_LIBRARY_DEBUG}
        ${GENERIC_CODE_GEN_LIBRARY_DEBUG}
        ${MACHINE_INDEPENDENT_LIBRARY_DEBUG}
        ${SPV_REMAPPER_LIBRARY_DEBUG})

set(GLSLANG_LIBS_RELEASE
        ${GLSLANG_LIBRARY_RELEASE}
        ${SPIRV_LIBRARY_RELEASE}
        ${OSDEPENDENT_LIBRARY_RELEASE}
        ${OGLCOMPILER_LIBRARY_RELEASE}
        ${GENERIC_CODE_GEN_LIBRARY_RELEASE}
        ${MACHINE_INDEPENDENT_LIBRARY_RELEASE}
        ${SPV_REMAPPER_LIBRARY_RELEASE})

# Not configurable
mark_as_advanced(GLSLANG_DIR)
mark_as_advanced(GLSLANG_LIBS_DEBUG)
mark_as_advanced(GLSLANG_LIBS_RELEASE)
mark_as_advanced(GLSLANG_INCLUDE_DIR)
mark_as_advanced(glslang_search_Dir)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(glslang REQUIRED_VARS GLSLANG_DIR GLSLANG_INCLUDE_DIR GLSLANG_LIBS_DEBUG GLSLANG_LIBS_RELEASE)
