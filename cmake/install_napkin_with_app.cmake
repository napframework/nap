if(NOT DEFINED NAP_PACKAGED_APP_BUILD)
    return()
endif()

include(${CMAKE_CURRENT_LIST_DIR}/macros_and_functions.cmake)

# NAP modules which Napkin uses (as a minimum)
set(NAPKIN_DEPENDENT_NAP_MODULES napscene
        napcolor
        napfont
        naprender
        napcameracontrol
        napapp
        napinput
        napsdlinput
        napimgui
        napapi
        naprenderadvanced)

# Qt frameworks which Napkin uses
set(NAPKIN_QT_INSTALL_FRAMEWORKS QtCore QtGui QtWidgets QtOpenGL)
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
    # Copy napkin directory -> exclude path mapping
    install(DIRECTORY ${NAP_ROOT}/tools/napkin/
        DESTINATION napkin
        PATTERN "*/path_mapping.json" EXCLUDE
        PATTERN "*/napkin.ini" EXCLUDE)

    # Napkin has dependencies the application might not have -> we must include the (shared) data for those.
    # For example: renderadvanced is required by Napkin but not by most demos -> it requires those shaders to run
    foreach(NAP_MODULE ${NAPKIN_DEPENDENT_NAP_MODULES})
        set(MODULE_EXTRA_CMAKE_PATH ${NAP_ROOT}/system_modules/${NAP_MODULE}/module_extra.cmake)
        if(EXISTS ${MODULE_EXTRA_CMAKE_PATH})
            include(${MODULE_EXTRA_CMAKE_PATH})
        endif()
    endforeach()
else()
    # Linux

    # Install executable into packaged app
    install(PROGRAMS ${NAP_ROOT}/tools/napkin/napkin
            DESTINATION napkin)
    # Install resources into packaged app
    install(DIRECTORY ${NAP_ROOT}/tools/napkin/resources
            DESTINATION napkin
	        PATTERN "*/path_mapping.json" EXCLUDE)
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

# Install path mapping for applets running in packaged app context
install(FILES ${NAP_ROOT}/tools/buildsystem/path_mappings/applet/packaged_app.json
    DESTINATION napkin/resources/applets
    RENAME path_mapping.json)

# Install Qt licenses into packaged app
install(DIRECTORY ${THIRDPARTY_DIR}/Qt/licenses/ DESTINATION licenses/Qt)
