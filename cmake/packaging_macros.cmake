# Top level entry point for creating platform release.  Mainly a gathering place for things not captured 
# in the install phase elsewhere.
macro(package_nap)
    # Populate JSON build info
    if(DEFINED NAP_PACKAGED_BUILD)
        if(EXISTS ${NAP_ROOT}/cmake/build_number.cmake)
            include(${NAP_ROOT}/cmake/build_number.cmake)
        else()
            set(NAP_BUILD_NUMBER 0)
        endif()
        math(EXPR NAP_BUILD_NUMBER "${NAP_BUILD_NUMBER}+1")
        include(${NAP_ROOT}/cmake/version.cmake)        
        configure_file(${NAP_ROOT}/cmake/build_info.json.in ${NAP_ROOT}/dist/cmake/build_info.json @ONLY)
        configure_file(${NAP_ROOT}/cmake/build_number.cmake.in ${NAP_ROOT}/cmake/build_number.cmake @ONLY)
    endif()

    # Package shared cmake files
    install(DIRECTORY ${NAP_ROOT}/dist/cmake/ 
            DESTINATION cmake
            )   

    # Install wrapper batch scripts for user tools
    if(WIN32)
        file(GLOB USER_TOOL_WRAPPERS "${NAP_ROOT}/dist/win64/user_tools_wrappers/*.*")
    else()
        file(GLOB USER_TOOL_WRAPPERS "${NAP_ROOT}/dist/unix/user_tools_wrappers/*")
    endif()
    install(PROGRAMS ${USER_TOOL_WRAPPERS} DESTINATION tools)

    # Package platform tools
    file(GLOB PLATFORM_TOOL_SCRIPTS "${NAP_ROOT}/dist/user_scripts/platform/*py")
    install(PROGRAMS ${PLATFORM_TOOL_SCRIPTS} DESTINATION tools/platform)

    # Package project directory package & regenerate shortcuts
    package_project_dir_shortcuts("tools/platform/project_dir_shortcuts")

    # Package module directory regenerate shortcut
    package_module_dir_shortcuts("tools/platform/module_dir_shortcuts")

    # Package check_build_environment scripts
    if(APPLE)
        install(PROGRAMS ${NAP_ROOT}/dist/macos/check_build_environment/check_build_environment DESTINATION tools)
    elseif(UNIX)
        install(PROGRAMS ${NAP_ROOT}/dist/linux/check_build_environment/check_build_environment DESTINATION tools)
        install(PROGRAMS ${NAP_ROOT}/dist/linux/check_build_environment/check_build_environment_worker.py DESTINATION tools/platform)
    else()
        install(FILES ${NAP_ROOT}/dist/win64/check_build_environment/check_build_environment.bat DESTINATION tools)
        install(FILES ${NAP_ROOT}/dist/win64/check_build_environment/check_build_environment_continued.py DESTINATION tools/platform)
    endif()

    # Create empty projects and usermodules directories
    install(CODE "FILE(MAKE_DIRECTORY \${ENV}\${CMAKE_INSTALL_PREFIX}/projects)")
    install(CODE "FILE(MAKE_DIRECTORY \${ENV}\${CMAKE_INSTALL_PREFIX}/user_modules)")

    # Package thirdparty Python into release
    package_python()

    # Package Qt into release
    package_qt()

    # Package documentation
    if(INCLUDE_DOCS)
        find_package(Doxygen REQUIRED)
        install(CODE "execute_process(COMMAND python ${NAP_ROOT}/docs/doxygen/generateDocumentation.py)
                      execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${NAP_ROOT}/docs/html/ ${CMAKE_INSTALL_PREFIX}/doc)")
    endif()

    # Package IDE templates
    if(WIN32)
        install(DIRECTORY ${NAP_ROOT}/ide_templates/visual_studio_templates/ DESTINATION visual_studio_templates)
    elseif(APPLE)
        install(DIRECTORY ${NAP_ROOT}/ide_templates/xcode_templates/ DESTINATION xcode_templates)
    endif()

    # Package Windows redistributable help
    if(WIN32)
        install(FILES "${NAP_ROOT}/dist/win64/redist_help/Microsoft Visual C++ Redistributable Help.txt" DESTINATION tools/platform)
    endif()
endmacro()

# Package installed Python for distribution with NAP release (for use with mod_nappython, Napkin and interpreter for Python scripts)
macro(package_python)
    if(WIN32)
        # Install main framework
        install(DIRECTORY ${THIRDPARTY_DIR}/python/msvc/python-embed-amd64/
                DESTINATION thirdparty/python/
                CONFIGURATIONS Release)

        # Install license
        install(FILES ${THIRDPARTY_DIR}/python/msvc/LICENSE.txt
                DESTINATION thirdparty/python/
                CONFIGURATIONS Release)
    elseif(UNIX)
        if(APPLE)
            set(PYTHON_PREFIX ${THIRDPARTY_DIR}/python/osx/install)
        else()
            set(PYTHON_PREFIX ${THIRDPARTY_DIR}/python/linux/install)
        endif()

        # Install dylib        
        file(GLOB PYTHON_DYLIBs ${PYTHON_PREFIX}/lib/*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
        install(FILES ${PYTHON_DYLIBs}
                DESTINATION thirdparty/python/lib
                CONFIGURATIONS Release
                )

        # Install main framework
        install(DIRECTORY ${PYTHON_PREFIX}/lib/python3.6
                DESTINATION thirdparty/python/lib/
                CONFIGURATIONS Release
                PATTERN *.pyc EXCLUDE
                PATTERN *.a EXCLUDE
                PATTERN site-packages EXCLUDE)

        # Install command line intrepreter
        file(GLOB PYTHON_INTERPRETER ${PYTHON_PREFIX}/bin/python*)
        install(PROGRAMS ${PYTHON_INTERPRETER}
                DESTINATION thirdparty/python/bin/
                CONFIGURATIONS Release)

        # Install includes
        install(DIRECTORY ${PYTHON_PREFIX}/include
                DESTINATION thirdparty/python/
                CONFIGURATIONS Release)

        # Install license
        install(FILES ${PYTHON_PREFIX}/LICENSE
                DESTINATION thirdparty/python/
                CONFIGURATIONS Release)
    endif()
endmacro()

# Package installed QT for distribution with NAP release (for use with Napkin)
macro(package_qt)
    set(QT_FRAMEWORKS Core Gui Widgets)

    # Install licenses.  This link is a little tenuous but seems to work for Win64
    # TODO Fail if we don't find sufficient licenses (which happens pre Qt 5.10)
    install(DIRECTORY ${QT_DIR}/../../Licenses/
            DESTINATION thirdparty/Qt/licenses
            CONFIGURATIONS Release
            )
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
        list(APPEND QT_FRAMEWORKS PrintSupport)

        # Install frameworks
        foreach(QT_INSTALL_FRAMEWORK ${QT_FRAMEWORKS})
            set(QT_FRAMEWORK_SRC ${QT_DIR}/lib/Qt${QT_INSTALL_FRAMEWORK}.framework/Versions/Current/Qt${QT_INSTALL_FRAMEWORK})
            set(FRAMEWORK_INSTALL_LOC ${CMAKE_INSTALL_PREFIX}/thirdparty/Qt/lib/Qt${QT_INSTALL_FRAMEWORK})

            install(FILES ${QT_FRAMEWORK_SRC}
                    DESTINATION thirdparty/Qt/lib/
                    CONFIGURATIONS Release
                    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
                    )

            # Change dylib installed id
            install(CODE "execute_process(COMMAND ${CMAKE_INSTALL_NAME_TOOL} 
                                                  -id @rpath/Qt${QT_INSTALL_FRAMEWORK}
                                                  ${FRAMEWORK_INSTALL_LOC}
                                          ERROR_QUIET)")


            macos_replace_qt_framework_links("${QT_FRAMEWORKS}" Qt${QT_INSTALL_FRAMEWORK} ${QT_FRAMEWORK_SRC} ${FRAMEWORK_INSTALL_LOC} "@loader_path")
        endforeach()

        set(PATH_FROM_QT_PLUGIN_TOLIB "@loader_path/../../../../../../thirdparty/Qt/lib")

        # Install plugins
        install(FILES ${QT_DIR}/plugins/platforms/libqcocoa.dylib
                DESTINATION thirdparty/Qt/plugins/platforms/
                CONFIGURATIONS Release
                PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
                )
        macos_replace_qt_framework_links("${QT_FRAMEWORKS}" 
                                         libqcocoa 
                                         ${QT_DIR}/plugins/platforms/libqcocoa.dylib 
                                         ${CMAKE_INSTALL_PREFIX}/thirdparty/Qt/plugins/platforms/libqcocoa.dylib
                                         ${PATH_FROM_QT_PLUGIN_TOLIB}
                                         )
    elseif(UNIX)
        list(APPEND QT_FRAMEWORKS DBus XcbQpa)

        # Install frameworks
        foreach(QT_INSTALL_FRAMEWORK ${QT_FRAMEWORKS})
            file(GLOB QT_FRAMEWORK_SRC ${QT_DIR}/lib/libQt5${QT_INSTALL_FRAMEWORK}.so*)

            install(FILES ${QT_FRAMEWORK_SRC}
                    DESTINATION thirdparty/Qt/lib
                    CONFIGURATIONS Release
                    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
                    )
        endforeach()

        # Install plugins
        install(FILES ${QT_DIR}/plugins/platforms/libqxcb.so
                DESTINATION thirdparty/Qt/plugins/platforms
                CONFIGURATIONS Release
                PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
                )

        # Install Qt dependent libs on Linux
        set(QT_DEPENDENT_LIBS_LINUX icudata icui18n icuuc)
        foreach(QT_DEPENDENT_LIB ${QT_DEPENDENT_LIBS_LINUX})
            file(GLOB QT_FRAMEWORK_SRC ${QT_DIR}/lib/lib${QT_DEPENDENT_LIB}.so*)

            install(FILES ${QT_FRAMEWORK_SRC}
                    DESTINATION thirdparty/Qt/lib
                    CONFIGURATIONS Release
                    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
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

# Package project directory package & regenerate shortcuts
macro(package_project_dir_shortcuts DESTINATION)
    if(WIN32)
        install(PROGRAMS ${NAP_ROOT}/dist/win64/project_dir_shortcuts/package.bat
                         ${NAP_ROOT}/dist/win64/project_dir_shortcuts/regenerate.bat
                DESTINATION ${DESTINATION})
    else()
        install(PROGRAMS ${NAP_ROOT}/dist/unix/project_dir_shortcuts/package
                         ${NAP_ROOT}/dist/unix/project_dir_shortcuts/regenerate
                DESTINATION ${DESTINATION})
    endif()
endmacro()

# Package module directory regenerate shortcut
macro(package_module_dir_shortcuts DESTINATION)
    if(WIN32)
        install(PROGRAMS ${NAP_ROOT}/dist/win64/module_dir_shortcuts/regenerate.bat
                DESTINATION ${DESTINATION})
    else()
        install(PROGRAMS ${NAP_ROOT}/dist/unix/module_dir_shortcuts/regenerate
                DESTINATION ${DESTINATION})
    endif()
endmacro()

macro(package_project_into_release DEST_DIR)
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
            DESTINATION ${DEST_DIR}
            PATTERN "CMakeLists.txt" EXCLUDE
            PATTERN "cached_project_json.cmake" EXCLUDE
            PATTERN "dist" EXCLUDE
            PATTERN "*.mesh" EXCLUDE)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/dist/CMakeLists.txt DESTINATION ${DEST_DIR})

    # Package any projectmodule cmake files
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/dist/module/CMakeLists.txt)
        install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/dist/module/CMakeLists.txt DESTINATION ${DEST_DIR}/module)
    endif()
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/dist/module/module_extra.cmake)
        install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/dist/module/module_extra.cmake DESTINATION ${DEST_DIR}/module)
    endif()   

    # Package any project extra cmake
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/dist/project_extra.cmake)
        install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/dist/project_extra.cmake DESTINATION ${DEST_DIR})
    endif()

    # Package our regenerate & package shortcuts into the project directory
    package_project_dir_shortcuts(${DEST_DIR})
endmacro()

# Package module into platform release
macro(package_module)
    # Package headers
    install(DIRECTORY "src/" DESTINATION "modules/${PROJECT_NAME}/include"
            FILES_MATCHING PATTERN "*.h")

    # If the module has some extra cmake logic package it
    if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/dist/cmake)
        install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dist/cmake/ DESTINATION modules/${PROJECT_NAME}/)
    endif()

    # Package library
    if (WIN32)
        install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION modules/${PROJECT_NAME}/lib/$<CONFIG>
                                        LIBRARY DESTINATION modules/${PROJECT_NAME}/lib/$<CONFIG>
                                        ARCHIVE DESTINATION modules/${PROJECT_NAME}/lib/$<CONFIG>)
    elseif(APPLE)
        install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION modules/${PROJECT_NAME}/lib/$<CONFIG>)
    else()
        install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION modules/${PROJECT_NAME}/lib/${CMAKE_BUILD_TYPE})
    endif()

    # Set packaged RPATH for *nix (for macOS I believe we need to make sure this is being done done after we 
    # install the target above due to ordering of install_name_tool calling)
    if(UNIX)
        if(DEFINED UNIX_EXTRA_RPATH)
            set(EXTRA_RPATH "${UNIX_EXTRA_RPATH}")    
        else()
            set(EXTRA_RPATH "")
        endif()
        set_installed_module_rpath_for_dependent_modules("${DEPENDENT_NAP_MODULES}" ${PROJECT_NAME} ${EXTRA_RPATH})
    endif()
endmacro()

# Set the packaged RPATH of a module for its dependent modules.
macro(set_installed_module_rpath_for_dependent_modules DEPENDENT_NAP_MODULES TARGET_NAME)
    set(NAP_ROOT_LOCATION_TO_MODULE "../../../..")
    if(APPLE)
        set_installed_rpath_on_macos_module_for_dependent_modules("${DEPENDENT_NAP_MODULES}" ${TARGET_NAME} ${NAP_ROOT_LOCATION_TO_MODULE} "${ARGN}")
    elseif(UNIX)
        set_installed_rpath_on_linux_object_for_dependent_modules("${DEPENDENT_NAP_MODULES}" ${TARGET_NAME} ${NAP_ROOT_LOCATION_TO_MODULE} "${ARGN}")
    endif()
endmacro()

# Set the packaged RPATH of a Linux binary object for its dependent modules
macro(set_installed_rpath_on_linux_object_for_dependent_modules DEPENDENT_NAP_MODULES TARGET_NAME NAP_ROOT_LOCATION_TO_ORIGIN)
    # Add our core lib path first
    set(BUILT_RPATH "$ORIGIN/${NAP_ROOT_LOCATION_TO_ORIGIN}/lib/${CMAKE_BUILD_TYPE}")
    set(BUILT_RPATH "${BUILT_RPATH}:$ORIGIN/${NAP_ROOT_LOCATION_TO_ORIGIN}/thirdparty/python/lib")
    set(BUILT_RPATH "${BUILT_RPATH}:$ORIGIN/${NAP_ROOT_LOCATION_TO_ORIGIN}/thirdparty/rttr/bin")    

    set(EXTRA_PATHS ${ARGN})
    foreach(EXTRA_PATH ${EXTRA_PATHS})
        set(BUILT_RPATH "${BUILT_RPATH}:$ORIGIN/${EXTRA_PATH}")
    endforeach()

    # Iterate over each module and append to path
    foreach(module ${DEPENDENT_NAP_MODULES})
        set(THIS_MODULE_PATH "$ORIGIN/${NAP_ROOT_LOCATION_TO_ORIGIN}/modules/${module}/lib/${CMAKE_BUILD_TYPE}")
        set(BUILT_RPATH "${BUILT_RPATH}:${THIS_MODULE_PATH}")
    endforeach(module)
    set_target_properties(${TARGET_NAME} PROPERTIES SKIP_BUILD_RPATH FALSE
                                                    INSTALL_RPATH ${BUILT_RPATH})
endmacro()

# On macOS set the packaged RPATH of a binary object for its dependent modules for a single build configuration
macro(set_single_config_installed_rpath_on_macos_object_for_dependent_modules CONFIG DEPENDENT_NAP_MODULES OBJECT_FILENAME NAP_ROOT_LOCATION_TO_OBJECT)
    ensure_macos_file_has_rpath_at_install(${OBJECT_FILENAME} "@loader_path/${NAP_ROOT_LOCATION_TO_OBJECT}/thirdparty/python/lib")
    ensure_macos_file_has_rpath_at_install(${OBJECT_FILENAME} "@loader_path/${NAP_ROOT_LOCATION_TO_OBJECT}/thirdparty/rttr/bin")
    ensure_macos_file_has_rpath_at_install(${OBJECT_FILENAME} "@loader_path/${NAP_ROOT_LOCATION_TO_OBJECT}/lib/${CONFIG}")   
    foreach(DEPENDENT_MODULE_NAME ${DEPENDENT_NAP_MODULES})
        ensure_macos_file_has_rpath_at_install(${OBJECT_FILENAME} "@loader_path/${NAP_ROOT_LOCATION_TO_OBJECT}/modules/${DEPENDENT_MODULE_NAME}/lib/${CONFIG}")
    endforeach()
    set(EXTRA_PATHS ${ARGN})
    foreach(EXTRA_PATH ${EXTRA_PATHS})
        ensure_macos_file_has_rpath_at_install(${OBJECT_FILENAME} "@loader_path/${EXTRA_PATH}")
    endforeach()       
endmacro()

# On macOS set the packaged RPATH of a module on macOS for its dependent modules
macro(set_installed_rpath_on_macos_module_for_dependent_modules DEPENDENT_NAP_MODULES MODULE_NAME NAP_ROOT_LOCATION_TO_MODULE)
    foreach(MODULECONFIG Release Debug)
        ensure_macos_module_has_rpath_at_install(${MODULE_NAME} ${MODULECONFIG} "@loader_path/${NAP_ROOT_LOCATION_TO_MODULE}/thirdparty/python/lib")
        ensure_macos_module_has_rpath_at_install(${MODULE_NAME} ${MODULECONFIG} "@loader_path/${NAP_ROOT_LOCATION_TO_MODULE}/thirdparty/rttr/bin")
        ensure_macos_module_has_rpath_at_install(${MODULE_NAME} ${MODULECONFIG} "@loader_path/${NAP_ROOT_LOCATION_TO_MODULE}/lib/${MODULECONFIG}")
        foreach(DEPENDENT_MODULE_NAME ${DEPENDENT_NAP_MODULES})
            ensure_macos_module_has_rpath_at_install(${MODULE_NAME} ${MODULECONFIG} "@loader_path/${NAP_ROOT_LOCATION_TO_MODULE}/modules/${DEPENDENT_MODULE_NAME}/lib/${MODULECONFIG}")
        endforeach()
        set(EXTRA_PATHS ${ARGN})
        foreach(EXTRA_PATH ${EXTRA_PATHS})
            ensure_macos_module_has_rpath_at_install(${MODULE_NAME} ${MODULECONFIG} "@loader_path/${EXTRA_PATH}")
        endforeach()       
    endforeach()
endmacro()

# On macOS ensure the specified module has the provided RPATH for the specified build configuration
macro(ensure_macos_module_has_rpath_at_install MODULE_NAME CONFIG PATH_TO_ADD)
    set(MODULE_FILENAME ${CMAKE_INSTALL_PREFIX}/modules/${MODULE_NAME}/lib/${CONFIG}/lib${MODULE_NAME}.dylib)
    ensure_macos_file_has_rpath_at_install(${MODULE_FILENAME} ${PATH_TO_ADD})
endmacro()

# On macOS ensure the specified binary object has the provided RPATH
macro(ensure_macos_file_has_rpath_at_install FILENAME PATH_TO_ADD)
    install(CODE "execute_process(COMMAND ${CMAKE_INSTALL_NAME_TOOL} 
                                          -add_rpath
                                          ${PATH_TO_ADD}
                                          ${FILENAME} 
                                  ERROR_QUIET)")
endmacro()