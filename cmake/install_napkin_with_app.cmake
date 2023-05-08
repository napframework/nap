if(NOT DEFINED NAP_PACKAGED_APP_BUILD)
    return()
endif()

include(${CMAKE_CURRENT_LIST_DIR}/macros_and_functions.cmake)

# NAP modules which Napkin uses (as a minimum)
set(NAPKIN_DEPENDENT_NAP_MODULES napscene napmath)

# Qt frameworks which Napkin uses
set(NAPKIN_QT_INSTALL_FRAMEWORKS QtCore QtGui QtWidgets QtPrintSupport QtOpenGL)
message(STATUS "Preparing Napkin deployment to output directory")

# Let find_python find our prepackaged Python in thirdparty,
# Note that this operation is allowed to fail because, by default, Python support is disabled.
if(NOT pybind11_DIR)
    configure_python()
    set(pybind11_DIR "${NAP_ROOT}/system_modules/nappython/thirdparty/pybind11/install/share/cmake/pybind11")
    find_package(pybind11 QUIET)
endif()
if(pybind11_FOUND)
    win64_copy_python_dlls_postbuild(FALSE)
    list(APPEND NAPKIN_DEPENDENT_NAP_MODULES nappython)
endif()

if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME}
                       POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy_directory "${NAP_ROOT}/tools/napkin" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/napkin"
                       COMMENT "Copy Napkin")
elseif(APPLE)
    list(APPEND NAPKIN_QT_INSTALL_FRAMEWORKS QtDBus)

    # ---- Install napkin with packaged app ------

    # Install executable into packaged app
    install(PROGRAMS ${NAP_ROOT}/tools/napkin/napkin
            DESTINATION napkin)
    # Install resources into packaged app
    install(DIRECTORY ${NAP_ROOT}/tools/napkin/resources
            DESTINATION napkin)
    # Install main Qt libs from thirdparty into packaged app
    install(DIRECTORY ${THIRDPARTY_DIR}/Qt/lib/
            DESTINATION napkin/lib)
    # Install Qt plugins from thirdparty into packaged app
    install(DIRECTORY ${THIRDPARTY_DIR}/Qt/plugins
            DESTINATION napkin)

    # Ensure we have our dependent modules
    set(INSTALLING_MODULE_FOR_NAPKIN TRUE)
    foreach(NAP_MODULE ${NAPKIN_DEPENDENT_NAP_MODULES})
        find_nap_module(${NAP_MODULE})
    endforeach()
    unset(INSTALLING_MODULE_FOR_NAPKIN)

    # Add core libs path to RPATH
    install(CODE "execute_process(COMMAND ${CMAKE_INSTALL_NAME_TOOL} 
                                          -add_rpath 
                                          @executable_path/lib
                                          ${CMAKE_INSTALL_PREFIX}/napkin/napkin
                                  ERROR_QUIET
                                  RESULT_VARIABLE EXIT_CODE)
                  if(NOT \${EXIT_CODE} EQUAL 0)
                      message(FATAL_ERROR \"Failed to add RPATH for napkin\")
                  endif()")
    install(CODE "execute_process(COMMAND ${CMAKE_INSTALL_NAME_TOOL} 
                                          -add_rpath 
                                          @executable_path/../lib
                                          ${CMAKE_INSTALL_PREFIX}/napkin/napkin
                                  ERROR_QUIET
                                  RESULT_VARIABLE EXIT_CODE)
                  if(NOT \${EXIT_CODE} EQUAL 0)
                      message(FATAL_ERROR \"Failed to add parent lib RPATH for napkin\")
                  endif()")

    # Update paths to Qt frameworks in libqcocoa plugin
    macos_replace_qt_framework_links_install_time("${NAPKIN_QT_INSTALL_FRAMEWORKS}" 
                                                  "libqcocoa"
                                                  ${CMAKE_INSTALL_PREFIX}/napkin/plugins/platforms/libqcocoa.dylib
                                                  "@loader_path/../../lib/"
                                                  )

    # Update paths to Qt frameworks in napkin
    macos_replace_qt_framework_links_install_time("${NAPKIN_QT_INSTALL_FRAMEWORKS}" 
                                                  "napkin"
                                                  ${CMAKE_INSTALL_PREFIX}/napkin/napkin
                                                  "@loader_path/lib/"
                                                  )
else()
    # Linux

    # Install executable into packaged app
    install(PROGRAMS ${NAP_ROOT}/tools/napkin/napkin
            DESTINATION napkin)
    # Install resources into packaged app
    install(DIRECTORY ${NAP_ROOT}/tools/napkin/resources
            DESTINATION napkin)
    # Install main Qt libs from thirdparty into packaged app
    install(DIRECTORY ${THIRDPARTY_DIR}/Qt/lib/
            DESTINATION napkin/lib)
    # Install Qt plugins from thirdparty into packaged app
    install(DIRECTORY ${THIRDPARTY_DIR}/Qt/plugins/platforms
            DESTINATION napkin)

    # Allow Qt platform plugin to find Qt frameworks in thirdparty (packaged app)
    install(CODE "execute_process(COMMAND patchelf
                                          --set-rpath
                                          \$ORIGIN/../lib
                                          ${CMAKE_INSTALL_PREFIX}/napkin/platforms/libqxcb.so
                                  ERROR_QUIET
                                  RESULT_VARIABLE EXIT_CODE)
                  if(NOT \${EXIT_CODE} EQUAL 0)
                      message(FATAL_ERROR \"Failed to fetch RPATH on libqxcb.so using patchelf\")
                  endif()")

    linux_append_rpath_at_install_time(${CMAKE_INSTALL_PREFIX}/napkin/napkin "\$ORIGIN/../lib/")

    # Ensure we have our dependent modules
    set(INSTALLING_MODULE_FOR_NAPKIN TRUE)
    foreach(NAP_MODULE ${NAPKIN_DEPENDENT_NAP_MODULES})
        find_nap_module(${NAP_MODULE})
    endforeach()
    unset(INSTALLING_MODULE_FOR_NAPKIN)
endif()

# Install Qt licenses into packaged app
install(DIRECTORY ${THIRDPARTY_DIR}/Qt/licenses/ DESTINATION licenses/Qt)
