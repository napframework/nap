
            message(WARNING "Using `SPVRemapperTargets.cmake` is deprecated: use `find_package(glslang)` to find glslang CMake targets.")

            if (NOT TARGET glslang::SPVRemapper)
                include("${CMAKE_CURRENT_LIST_DIR}/../../share/glslang/glslang-targets.cmake")
            endif()

            add_library(SPVRemapper ALIAS glslang::SPVRemapper)
        