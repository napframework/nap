macro(package_nap)
    # Package shared cmake files
    install(DIRECTORY ${NAP_ROOT}/dist/cmake/ 
            DESTINATION cmake
            )   

    # Package Windows-only cmake files (as of today this is just Findglew.cmake)
    if(WIN32)
        install(DIRECTORY ${NAP_ROOT}/dist/win64/cmake
                DESTINATION cmake)
    endif()

    # Package user tools
    file(GLOB USER_TOOL_SCRIPTS "${NAP_ROOT}/dist/projectscripts/*py")
    install(PROGRAMS ${USER_TOOL_SCRIPTS} DESTINATION tools)

    # Package platform tools
    file(GLOB PLATFORM_TOOL_SCRIPTS "${NAP_ROOT}/dist/projectscripts/platform/*py")
    install(PROGRAMS ${PLATFORM_TOOL_SCRIPTS} DESTINATION tools/platform)
    if(APPLE)
        install(PROGRAMS dist/macOS/macOSTempDylibCopyAndPathFix.py DESTINATION tools/platform)
    elseif(UNIX)
        install(PROGRAMS dist/linux/install_ubuntu_1710_dependencies.sh DESTINATION tools/platform)
    endif()

    # Package project directory package & refresh shortcuts
    package_project_dir_shortcuts("tools/platform/project_dir_shortcuts")

    # Create empty projects and usermodules directories
    install(CODE "FILE(MAKE_DIRECTORY \${ENV}\${CMAKE_INSTALL_PREFIX}/projects)")
    install(CODE "FILE(MAKE_DIRECTORY \${ENV}\${CMAKE_INSTALL_PREFIX}/usermodules)")

    # For now package platform Qt and Python for distribution, for discussion
    package_platform_python()
    package_platform_qt()
endmacro()

# Package installed Python for distribution with NAP release (for use with mod_nappython & Napkin)
# TODO This is brittle and (very likely) temporary
# I believe we should include python in thirdparty so that:
# - We can control the OS version/s we support (eg. the homebrew version we're installing for macOS won't be backwards compatible)
# - We don't have these brittle connections to a moving (eg. homebrew) target
# - We can control the Python lib version we're deploying
macro(package_platform_python)
    # TODO Install Python license

    if(WIN32)
        set(pybind11_DIR "${THIRDPARTY_DIR}/pybind11/install/share/cmake/pybind11")
        find_package(pybind11 REQUIRED) 
        message(STATUS "Got Python prefix: ${PYTHON_PREFIX}")

        # Ensure we're found Python3
        if(NOT ${PYTHON_VERSION_MAJOR} EQUAL 3)
            message(FATAL_ERROR "Python found for packaging in ${PYTHON_PREFIX} is not v3 (it's v${PYTHON_VERSION_MAJOR})")
        endif()

        # Install DLLs
        file(GLOB PYTHON_DLLs ${PYTHON_PREFIX}/*.dll)
        install(FILES ${PYTHON_DLLs}
                DESTINATION thirdparty/python/
                CONFIGURATIONS Release
                )

        # Install main framework
        install(DIRECTORY ${PYTHON_PREFIX}/Lib
                DESTINATION thirdparty/python/
                CONFIGURATIONS Release
                PATTERN *.pyc EXCLUDE
                PATTERN *.dll EXCLUDE
                PATTERN site-packages EXCLUDE)
    elseif(APPLE)
        # Find Python
        execute_process(COMMAND brew --prefix python3
                        OUTPUT_VARIABLE PYTHON_PREFIX)
        string(STRIP ${PYTHON_PREFIX} PYTHON_PREFIX)
        message(STATUS "Got Python prefix: ${PYTHON_PREFIX}")

        # Get our major/minor version
        execute_process(COMMAND python3 -c "import sys\nprint('%s.%s' % sys.version_info[:2])"
                        OUTPUT_VARIABLE PYTHON_MAJOR_MINOR_VERSION)
        string(STRIP ${PYTHON_MAJOR_MINOR_VERSION} PYTHON_MAJOR_MINOR_VERSION)
        message(STATUS "Got Python major.minor version: ${PYTHON_MAJOR_MINOR_VERSION}")

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
# TODO A better solution is probably to keep our own packaged Qt in thirdparty
macro(package_platform_qt)
    set(QT_FRAMEWORKS Core Gui Widgets)

    # TODO Install Qt license

    if(WIN32)
        # Install frameworks
        foreach(QT_INSTALL_FRAMEWORK ${QT_FRAMEWORKS})
            set(QT_FRAMEWORK_SRC ${QT_DIR}/bin/Qt5${QT_INSTALL_FRAMEWORK})

            install(FILES ${QT_FRAMEWORK_SRC}d.dll
                    DESTINATION thirdparty/Qt/bin/Debug
                    CONFIGURATIONS Release)

            install(FILES ${QT_FRAMEWORK_SRC}.dll
                    DESTINATION thirdparty/Qt/bin/Release
                    CONFIGURATIONS Release)
        endforeach()

        # Install plugins
        install(FILES  ${QT_DIR}/plugins/platforms/qwindowsd.dll
                DESTINATION thirdparty/Qt/plugins/Debug/platforms/
                CONFIGURATIONS Release)

        install(FILES ${QT_DIR}/plugins/platforms/qwindows.dll
                DESTINATION thirdparty/Qt/plugins/Release/platforms/
                CONFIGURATIONS Release)        
    elseif(APPLE)
        # macOS appears to depend on these extra Qt frameworks
        list(APPEND QT_FRAMEWORKS PrintSupport Svg)

        # Find QT
        execute_process(COMMAND brew --prefix qt
                        OUTPUT_VARIABLE QT_PREFIX)
        string(STRIP ${QT_PREFIX} QT_PREFIX)
        message(STATUS "Got QT prefix: ${QT_PREFIX}")

        # Install frameworks
        foreach(QT_INSTALL_FRAMEWORK ${QT_FRAMEWORKS})
            set(QT_FRAMEWORK_SRC ${QT_PREFIX}/lib/Qt${QT_INSTALL_FRAMEWORK}.framework/Versions/Current/Qt${QT_INSTALL_FRAMEWORK})
            set(FRAMEWORK_INSTALL_LOC ${CMAKE_INSTALL_PREFIX}/thirdparty/Qt/lib/Qt${QT_INSTALL_FRAMEWORK})

            install(FILES ${QT_FRAMEWORK_SRC}
                    DESTINATION thirdparty/Qt/lib/
                    CONFIGURATIONS Release)

            # Change dylib installed id
            install(CODE "execute_process(COMMAND ${CMAKE_INSTALL_NAME_TOOL} 
                                                  -id @rpath/Qt${QT_INSTALL_FRAMEWORK}
                                                  ${FRAMEWORK_INSTALL_LOC}
                                          ERROR_QUIET)")


            macos_replace_qt_framework_links("${QT_FRAMEWORKS}" Qt${QT_INSTALL_FRAMEWORK} ${QT_FRAMEWORK_SRC} ${FRAMEWORK_INSTALL_LOC} "@loader_path")
        endforeach()

        set(PATH_FROM_QT_PLUGIN_TOLIB "@loader_path/../../../../../../thirdparty/Qt/lib")

        # Install plugins
        install(FILES ${QT_PREFIX}/plugins/platforms/libqcocoa.dylib
                DESTINATION thirdparty/Qt/plugins/platforms/
                CONFIGURATIONS Release
                PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
        macos_replace_qt_framework_links("${QT_FRAMEWORKS}" 
                                         libqcocoa 
                                         ${QT_PREFIX}/plugins/platforms/libqcocoa.dylib 
                                         ${CMAKE_INSTALL_PREFIX}/thirdparty/Qt/plugins/platforms/libqcocoa.dylib
                                         ${PATH_FROM_QT_PLUGIN_TOLIB}
                                         )

        # Glob each of our imageformat plugins and install them.  Globbing so we can update framework link install paths on them.
        file(GLOB imageformat_plugins RELATIVE ${QT_PREFIX}/plugins/imageformats/ "${QT_PREFIX}/plugins/imageformats/*dylib")
        foreach(imageformat_plugin ${imageformat_plugins})
            install(FILES ${QT_PREFIX}/plugins/imageformats/${imageformat_plugin}
                    DESTINATION thirdparty/Qt/plugins/imageformats/
                    CONFIGURATIONS Release)

            macos_replace_qt_framework_links("${QT_FRAMEWORKS}" 
                                             ${imageformat_plugin} 
                                             ${QT_PREFIX}/plugins/imageformats/${imageformat_plugin}
                                             ${CMAKE_INSTALL_PREFIX}/thirdparty/Qt/plugins/imageformats/${imageformat_plugin}
                                             ${PATH_FROM_QT_PLUGIN_TOLIB}
                                             )            
        endforeach()
    endif()
endmacro()

macro(macos_replace_qt_framework_links FRAMEWORKS LIB_NAME LIB_SRC_LOCATION LIB_INSTALL_LOCATION PATH_PREFIX)
    foreach(QT_LINK_FRAMEWORK ${FRAMEWORKS})
        if(NOT Qt${QT_LINK_FRAMEWORK} STREQUAL ${LIB_NAME})
            execute_process(COMMAND sh -c "otool -L ${LIB_SRC_LOCATION} | grep Qt${QT_LINK_FRAMEWORK} | awk -F'(' '{print $1}'"
                            OUTPUT_VARIABLE REPLACE_INSTALL_NAME)
            if(NOT ${REPLACE_INSTALL_NAME} STREQUAL "")
                # message("Adding install name change in ${QT_INSTALL_FRAMEWORK} for Qt${QT_LINK_FRAMEWORK}")
                string(STRIP ${REPLACE_INSTALL_NAME} REPLACE_INSTALL_NAME)

                # Change link to dylib
                install(CODE "execute_process(COMMAND ${CMAKE_INSTALL_NAME_TOOL} 
                                                      -change 
                                                      ${REPLACE_INSTALL_NAME}
                                                      ${PATH_PREFIX}/Qt${QT_LINK_FRAMEWORK}
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
                          #message(\"Adding install name change in ${FILEPATH} for ${REPLACE_LIB_NAME}\")
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

macro(package_project_dir_shortcuts DESTINATION)
    # Package project directory package & refresh shortcuts
    if(WIN32)
        install(PROGRAMS ${NAP_ROOT}/dist/win64/project_dir_shortcuts/package.cmd
                         ${NAP_ROOT}/dist/win64/project_dir_shortcuts/refresh.cmd
                DESTINATION ${DESTINATION})
    else()
        install(PROGRAMS ${NAP_ROOT}/dist/unix/project_dir_shortcuts/package
                         ${NAP_ROOT}/dist/unix/project_dir_shortcuts/refresh
                DESTINATION ${DESTINATION})
    endif()
endmacro()

macro(package_project_into_release DEST_DIR)
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
            DESTINATION ${DEST_DIR}
            PATTERN "CMakeLists.txt" EXCLUDE
            PATTERN "dist" EXCLUDE)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/dist/CMakeLists.txt DESTINATION ${DEST_DIR})

    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/dist/module/CMakeLists.txt)
        install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/dist/module/CMakeLists.txt DESTINATION ${DEST_DIR}/module)
    endif()

    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/dist/projectExtra.cmake)
        install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/dist/projectExtra.cmake DESTINATION ${DEST_DIR})
    endif()

    package_project_dir_shortcuts(${DEST_DIR})
endmacro()