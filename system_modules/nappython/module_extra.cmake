if(NAP_BUILD_CONTEXT MATCHES "framework_release")
    if(NOT DEFINED pybind11::embed)
        configure_python()
        set(pybind11_DIR "${NAP_ROOT}/system_modules/nappython/thirdparty/pybind11/install/share/cmake/pybind11")
        find_package(pybind11 REQUIRED)
    endif()

    target_link_libraries(${PROJECT_NAME} pybind11::embed)

    # Copy Python framework libs, during post-build or install, depending on our platform
    if(WIN32)
        # Copy Python modules post-build on Windows
        win64_copy_python_modules_postbuild(FALSE)
    endif()

    if(UNIX)
        # Python modules library installation
        install(DIRECTORY ${THIRDPARTY_DIR}/python/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH}/lib/python3.6
                DESTINATION lib/
                PATTERN __pycache__ EXCLUDE
                PATTERN *.pyc EXCLUDE)
    endif()
endif()
