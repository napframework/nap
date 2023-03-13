
        message(WARNING "Using `OGLCompilerTargets.cmake` is deprecated: use `find_package(glslang)` to find glslang CMake targets.")

        if (NOT TARGET glslang::OGLCompiler)
            include("${CMAKE_CURRENT_LIST_DIR}/../../share/glslang/glslang-targets.cmake")
        endif()

        add_library(OGLCompiler ALIAS glslang::OGLCompiler)
    