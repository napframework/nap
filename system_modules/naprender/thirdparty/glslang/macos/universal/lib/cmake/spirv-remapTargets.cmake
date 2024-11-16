
            message(WARNING "Using `spirv-remapTargets.cmake` is deprecated: use `find_package(glslang)` to find glslang CMake targets.")

            if (NOT TARGET glslang::spirv-remap)
                include("${CMAKE_CURRENT_LIST_DIR}/../../share/glslang/glslang-targets.cmake")
            endif()

            add_library(spirv-remap ALIAS glslang::spirv-remap)
        