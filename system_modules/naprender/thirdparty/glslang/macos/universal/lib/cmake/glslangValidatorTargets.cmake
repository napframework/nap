
        message(WARNING "Using `glslangValidatorTargets.cmake` is deprecated: use `find_package(glslang)` to find glslang CMake targets.")

        if (NOT TARGET glslang::glslangValidator)
            include("${CMAKE_CURRENT_LIST_DIR}/../../share/glslang/glslang-targets.cmake")
        endif()

        add_library(glslangValidator ALIAS glslang::glslangValidator)
    