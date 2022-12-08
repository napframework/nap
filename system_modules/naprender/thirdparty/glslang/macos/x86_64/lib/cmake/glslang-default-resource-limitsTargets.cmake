
        message(WARNING "Using `glslang-default-resource-limitsTargets.cmake` is deprecated: use `find_package(glslang)` to find glslang CMake targets.")

        if (NOT TARGET glslang::glslang-default-resource-limits)
            include("${CMAKE_CURRENT_LIST_DIR}/../../share/glslang/glslang-targets.cmake")
        endif()

        add_library(glslang-default-resource-limits ALIAS glslang::glslang-default-resource-limits)
    