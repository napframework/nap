# Copy Python framework libs, during post-build or install, depending on our platform
if(WIN32)
    if(DEFINED PACKAGE_NAPKIN AND NOT PACKAGE_NAPKIN)
        # Copy Python DLLs and modules post-build on Windows
        win64_copy_python_dlls_and_modules_postbuild()
    endif()
endif()

if(UNIX)
    # Python modules library installation
    install(DIRECTORY ${THIRDPARTY_DIR}/python/lib/python3.6
            DESTINATION lib/
            PATTERN __pycache__ EXCLUDE
            PATTERN *.pyc EXCLUDE)
endif()
