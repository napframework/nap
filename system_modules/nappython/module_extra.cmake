if(NAP_BUILD_CONTEXT MATCHES "framework_release")
    # Copy Python framework libs, during post-build or install, depending on our platform
    if(WIN32)
        # Copy Python modules post-build on Windows
        win64_copy_python_modules_postbuild(FALSE)
    endif()

    if(UNIX)
        # Python modules library installation
        install(DIRECTORY ${THIRDPARTY_DIR}/python/lib/python3.6
                DESTINATION lib/
                PATTERN __pycache__ EXCLUDE
                PATTERN *.pyc EXCLUDE)
    endif()
endif()
