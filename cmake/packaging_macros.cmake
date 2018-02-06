# Package installed Python for distribution with NAP release (for use with Napkin, mod_nappython)
# TODO This is brittle and (very likely) temporary.  
# I believe we should include python in thirdparty so that:
# - We can control the macOS OS version/s we support
# - We don't have these brittle connections to a moving (homebrew) target
# - We can control the Python lib version we're deploying
macro(package_platform_python)
    if (APPLE)
        # Find Python
        execute_process(COMMAND brew --prefix python3
                        OUTPUT_VARIABLE PYTHON_PREFIX)
        string(STRIP ${PYTHON_PREFIX} PYTHON_PREFIX)
        message("Got Python prefix: ${PYTHON_PREFIX}")

        # Get our major/minor version
        execute_process(COMMAND python3 -c "import sys\nprint('%s.%s' % sys.version_info[:2])"
                        OUTPUT_VARIABLE PYTHON_MAJOR_MINOR_VERSION)
        string(STRIP ${PYTHON_MAJOR_MINOR_VERSION} PYTHON_MAJOR_MINOR_VERSION)
        message("Got Python major.minor version: ${PYTHON_MAJOR_MINOR_VERSION}")

        # Get our dylib install name so we can replace it later to have a working command line intrepreter
        set(PYTHON_EXECUTABLE ${PYTHON_PREFIX}/Frameworks/Python.framework/Versions/${PYTHON_MAJOR_MINOR_VERSION}/Resources/Python.app/Contents/MacOS/Python)
        execute_process(COMMAND sh -c "otool -L ${PYTHON_EXECUTABLE} | grep Python | awk -F'(' '{if(NR>1)print $1}'"
                        OUTPUT_VARIABLE PYTHON_REPLACE_INSTALL_LIBNAME)
        string(STRIP ${PYTHON_REPLACE_INSTALL_LIBNAME} PYTHON_REPLACE_INSTALL_LIBNAME)

        # Install dylib
        install(FILES ${PYTHON_PREFIX}/Frameworks/Python.framework/Versions/3.6/Python
                DESTINATION thirdparty/python/
                CONFIGURATIONS Release
                PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
        # Change dylib installed id
        install(CODE "execute_process(COMMAND ${CMAKE_INSTALL_NAME_TOOL} 
                                              -id @rpath/Python
                                              ${CMAKE_INSTALL_PREFIX}/thirdparty/python/libpython${PYTHON_MAJOR_MINOR_VERSION}.dylib
                                      ERROR_QUIET)")

        # Install main framework
        install(DIRECTORY ${PYTHON_PREFIX}/Frameworks/Python.framework/Versions/${PYTHON_MAJOR_MINOR_VERSION}/lib/python${PYTHON_MAJOR_MINOR_VERSION} 
                DESTINATION thirdparty/python/lib/
                CONFIGURATIONS Release
                PATTERN *.pyc EXCLUDE
                PATTERN *.dylib EXCLUDE
                PATTERN *.a EXCLUDE
                PATTERN site-packages EXCLUDE)

        # Install command line intrepreter
        install(PROGRAMS ${PYTHON_EXECUTABLE}
                DESTINATION thirdparty/python/
                RENAME python3.6
                CONFIGURATIONS Release)
        # Change link to dylib
        install(CODE "execute_process(COMMAND ${CMAKE_INSTALL_NAME_TOOL} 
                                              -change 
                                              ${PYTHON_REPLACE_INSTALL_LIBNAME}
                                              @executable_path/Python
                                              ${CMAKE_INSTALL_PREFIX}/thirdparty/python/python${PYTHON_MAJOR_MINOR_VERSION} 
                                      ERROR_QUIET)")

        # Install license
        install(FILES ${PYTHON_PREFIX}/LICENSE
                DESTINATION thirdparty/python/
                CONFIGURATIONS Release)
    endif()
endmacro()

# Package installed QT for distribution with NAP release (for use with Napkin)
# TODO A better solution is probably to keep our own packaged QT in thirdparty
macro(package_platform_qt)
    if (APPLE)
        # Find QT
        execute_process(COMMAND brew --prefix qt
                        OUTPUT_VARIABLE QT_PREFIX)
        string(STRIP ${QT_PREFIX} QT_PREFIX)
        message("Got QT prefix: ${QT_PREFIX}")

        # Install frameworks
        set(QT_FRAMEWORKS QtCore QtGui QtPrintSupport QtSvg QtWidgets)
        foreach(QT_INSTALL_FRAMEWORK ${QT_FRAMEWORKS})
            set(QT_FRAMEWORK_SRC ${QT_PREFIX}/lib/${QT_INSTALL_FRAMEWORK}.framework/Versions/Current/${QT_INSTALL_FRAMEWORK})
            set(FRAMEWORK_INSTALL_LOC ${CMAKE_INSTALL_PREFIX}/thirdparty/QT/lib/${QT_INSTALL_FRAMEWORK})

            install(FILES ${QT_FRAMEWORK_SRC}
                    DESTINATION thirdparty/QT/lib/
                    CONFIGURATIONS Release)

            # Change dylib installed id
            install(CODE "execute_process(COMMAND ${CMAKE_INSTALL_NAME_TOOL} 
                                                  -id @rpath/${QT_INSTALL_FRAMEWORK}
                                                  ${FRAMEWORK_INSTALL_LOC}
                                          ERROR_QUIET)")


            macos_replace_qt_framework_links("${QT_FRAMEWORKS}" ${QT_INSTALL_FRAMEWORK} ${QT_FRAMEWORK_SRC} ${FRAMEWORK_INSTALL_LOC} "@loader_path")
        endforeach()

        set(PATH_FROM_QT_PLUGIN_TOLIB "@loader_path/../../../../../../thirdparty/QT/lib")

        # Install plugins
        install(FILES ${QT_PREFIX}/plugins/platforms/libqcocoa.dylib
                DESTINATION thirdparty/QT/plugins/platforms/
                CONFIGURATIONS Release
                PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
        macos_replace_qt_framework_links("${QT_FRAMEWORKS}" 
                                         libqcocoa 
                                         ${QT_PREFIX}/plugins/platforms/libqcocoa.dylib 
                                         ${CMAKE_INSTALL_PREFIX}/thirdparty/QT/plugins/platforms/libqcocoa.dylib
                                         ${PATH_FROM_QT_PLUGIN_TOLIB}
                                         )

        # Glob each of our imageformat plugins and install them.  Globbing so we can update framework link install paths on them.
        file(GLOB imageformat_plugins RELATIVE ${QT_PREFIX}/plugins/imageformats/ "${QT_PREFIX}/plugins/imageformats/*dylib")
        foreach(imageformat_plugin ${imageformat_plugins})
            install(FILES ${QT_PREFIX}/plugins/imageformats/${imageformat_plugin}
                    DESTINATION thirdparty/QT/plugins/imageformats/
                    CONFIGURATIONS Release)

            macos_replace_qt_framework_links("${QT_FRAMEWORKS}" 
                                             ${imageformat_plugin} 
                                             ${QT_PREFIX}/plugins/imageformats/${imageformat_plugin}
                                             ${CMAKE_INSTALL_PREFIX}/thirdparty/QT/plugins/imageformats/${imageformat_plugin}
                                             ${PATH_FROM_QT_PLUGIN_TOLIB}
                                             )            
        endforeach()
    endif()
endmacro()

macro(macos_replace_qt_framework_links FRAMEWORKS LIB_NAME LIB_SRC_LOCATION LIB_INSTALL_LOCATION PATH_PREFIX)
    foreach(QT_LINK_FRAMEWORK ${FRAMEWORKS})
        if(NOT ${QT_LINK_FRAMEWORK} STREQUAL ${LIB_NAME})
            execute_process(COMMAND sh -c "otool -L ${LIB_SRC_LOCATION} | grep ${QT_LINK_FRAMEWORK} | awk -F'(' '{print $1}'"
                            OUTPUT_VARIABLE REPLACE_INSTALL_NAME)
            if(NOT ${REPLACE_INSTALL_NAME} STREQUAL "")
                # message("Adding install name change in ${QT_INSTALL_FRAMEWORK} for ${QT_LINK_FRAMEWORK}")
                string(STRIP ${REPLACE_INSTALL_NAME} REPLACE_INSTALL_NAME)

                # Change link to dylib
                install(CODE "execute_process(COMMAND ${CMAKE_INSTALL_NAME_TOOL} 
                                                      -change 
                                                      ${REPLACE_INSTALL_NAME}
                                                      ${PATH_PREFIX}/${QT_LINK_FRAMEWORK}
                                                      ${LIB_INSTALL_LOCATION}
                                              ERROR_QUIET)")
            endif()
        endif()
    endforeach()    
endmacro()


macro(macos_replace_qt_framework_links_install_time FRAMEWORKS LIB_NAME FILEPATH PATH_PREFIX)
    foreach(QT_LINK_FRAMEWORK ${FRAMEWORKS})
        if(NOT ${QT_LINK_FRAMEWORK} STREQUAL ${LIB_NAME})
            macos_replace_single_install_name_link_install_time(${QT_LINK_FRAMEWORK} ${FILEPATH} ${PATH_PREFIX})
        endif()
    endforeach()    
endmacro()

macro(macos_replace_single_install_name_link_install_time REPLACE_LIB_NAME FILEPATH PATH_PREFIX)
    # Change link to dylib
    install(CODE "if(EXISTS ${FILEPATH})
                      execute_process(COMMAND sh -c \"otool -L ${FILEPATH} | grep ${REPLACE_LIB_NAME} | awk -F'(' '{print $1}'\"
                                      OUTPUT_VARIABLE REPLACE_INSTALL_NAME)
                      if(NOT \${REPLACE_INSTALL_NAME} STREQUAL \"\")
                          message(\"Adding install name change in ${FILEPATH} for ${REPLACE_LIB_NAME}\")
                          # Strip read path
                          string(STRIP \${REPLACE_INSTALL_NAME} REPLACE_INSTALL_NAME)                               

                          # Change link to dylib
                          execute_process(COMMAND ${CMAKE_INSTALL_NAME_TOOL} 
                                                  -change 
                                                  \${REPLACE_INSTALL_NAME}
                                                  ${PATH_PREFIX}/${REPLACE_LIB_NAME}
                                                  ${FILEPATH}
                                          ERROR_QUIET)
                      endif()
                  endif()
                  ")
endmacro()