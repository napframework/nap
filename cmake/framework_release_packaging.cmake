# Top level entry point for creating framework release.  Mainly a gathering place for things not captured
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
        configure_file(${NAP_ROOT}/cmake/build_info.json.in ${NAP_ROOT}/cmake/build_info.json @ONLY)
        configure_file(${NAP_ROOT}/cmake/build_number.cmake.in ${NAP_ROOT}/cmake/build_number.cmake @ONLY)
    endif()

    # Package CMake logic
    install(DIRECTORY ${NAP_ROOT}/cmake/
            DESTINATION cmake/
            PATTERN "*.in" EXCLUDE
            PATTERN "framework_release_packaging.cmake" EXCLUDE
            PATTERN "source_archive_populate_info.cmake" EXCLUDE
            )

    # Install wrapper batch scripts for user tools
    set(package_app_dir ${NAP_ROOT}/tools/buildsystem/package_app_wrapper)
    if(WIN32)
        file(GLOB USER_TOOL_WRAPPERS ${NAP_ROOT}/tools/*.bat)
        list(APPEND USER_TOOL_WRAPPERS ${package_app_dir}/win64/package_app.bat)
    else()
        file(GLOB USER_TOOL_WRAPPERS ${NAP_ROOT}/tools/*.sh)
        list(APPEND USER_TOOL_WRAPPERS ${package_app_dir}/unix/package_app.sh)
    endif()
    install(PROGRAMS ${USER_TOOL_WRAPPERS} DESTINATION tools)

    # Package buildsystem tools
    file(GLOB PLATFORM_TOOL_SCRIPTS "${NAP_ROOT}/tools/buildsystem/common/*.*")
    install(PROGRAMS ${PLATFORM_TOOL_SCRIPTS} DESTINATION tools/buildsystem/common)

    # Path mappings
    package_path_mappings()

    # Package app directory package & regenerate shortcuts
    if(UNIX)
        package_app_dir_shortcuts("tools/buildsystem/app_dir_shortcuts/unix")
    else()
        package_app_dir_shortcuts("tools/buildsystem/app_dir_shortcuts/win64")
    endif()

    # Package module directory shortcuts
    if(UNIX)
        package_module_dir_shortcuts("tools/buildsystem/module_dir_shortcuts/unix")
    else()
        package_module_dir_shortcuts("tools/buildsystem/module_dir_shortcuts/win64")
    endif()

    set(build_tools_dir ${NAP_ROOT}/tools/buildsystem)

    # Package check_build_environment scripts
    set(cbe_tool_dir ${build_tools_dir}/check_build_environment)
    if(APPLE)
        install(PROGRAMS ${cbe_tool_dir}/macos/check_build_environment.py DESTINATION tools RENAME check_build_environment)
    elseif(UNIX)
        install(PROGRAMS ${cbe_tool_dir}/linux/check_build_environment.sh DESTINATION tools)
        install(PROGRAMS ${cbe_tool_dir}/linux/check_build_environment_worker.py DESTINATION tools/buildsystem/common)
    else()
        install(FILES ${cbe_tool_dir}/win64/check_build_environment.bat DESTINATION tools)
        install(FILES ${cbe_tool_dir}/win64/check_build_environment_continued.py DESTINATION tools/buildsystem/check_build_environment/win64)
    endif()

    # TODO update to just deploy tools/buildsystem

    # Package single app CLI build script
    install(FILES ${build_tools_dir}/cli_single_app_build/cli_single_app_build.py DESTINATION tools/buildsystem/cli_single_app_build)

    # Package app/module upgrade script
    install(FILES ${build_tools_dir}/app_and_module_updater/app_and_module_updater.py DESTINATION tools/buildsystem/app_and_module_updater)

    # Package module sharing prep and setup scripts
    install(FILES ${build_tools_dir}/prepare_module_to_share/prepare_module_to_share_by_dir.py
                  ${build_tools_dir}/prepare_module_to_share/prepare_module_to_share_by_name.py
            DESTINATION tools/buildsystem/prepare_module_to_share)
    install(FILES ${build_tools_dir}/setup_module/setup_module.py DESTINATION tools/buildsystem/setup_module)
    install(FILES ${build_tools_dir}/setup_module/install_module.py DESTINATION tools/buildsystem/setup_module)

    # Create empty apps and modules directories
    install(CODE "FILE(MAKE_DIRECTORY \${ENV}\${CMAKE_INSTALL_PREFIX}/apps)")
    install(CODE "FILE(MAKE_DIRECTORY \${ENV}\${CMAKE_INSTALL_PREFIX}/modules)")

    # Package thirdparty Python into release
    package_python()

    # Package documentation
    if(INCLUDE_DOCS)
        find_package(Doxygen REQUIRED)
        install(CODE "execute_process(COMMAND python ${NAP_ROOT}/docs/doxygen/generateDocumentation.py
                                      RESULT_VARIABLE EXIT_CODE)
                      if(NOT \${EXIT_CODE} EQUAL 0)
                          message(FATAL_ERROR \"Failed to generate documentation\")
                      endif()
                      execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${NAP_ROOT}/docs/html/ ${CMAKE_INSTALL_PREFIX}/doc
                                      RESULT_VARIABLE EXIT_CODE)
                      if(NOT \${EXIT_CODE} EQUAL 0)
                          message(FATAL_ERROR \"Failed to copy HTML documentation\")
                      endif()")
    endif()

    # Package IDE templates
    if(WIN32)
        install(DIRECTORY ${NAP_ROOT}/ide_templates/visual_studio_templates/ DESTINATION visual_studio_templates)
    elseif(APPLE)
        install(DIRECTORY ${NAP_ROOT}/ide_templates/xcode_templates/ DESTINATION xcode_templates)
    endif()

    # Package CMake
    package_cmake()

    # Package Windows redistributable help
    if(WIN32)
        install(FILES "${build_tools_dir}/win64_redist_help/Microsoft Visual C++ Redistributable Help.txt" DESTINATION tools/buildsystem)
    endif()

    # Package Gatekeeper unquarantine scripts for macOS
    if(APPLE)
        install(PROGRAMS ${build_tools_dir}/macos_gatekeeper_unquarantine/unquarantine_framework.command DESTINATION tools)
        install(PROGRAMS "${build_tools_dir}/macos_gatekeeper_unquarantine/Unquarantine App.command" DESTINATION cmake/app_creator/template)
        install(FILES "${build_tools_dir}/macos_gatekeeper_unquarantine/Help launching on macOS.txt" DESTINATION cmake/app_creator/template)
    endif()

    # Install NAP source code license
    install(FILES ${NAP_ROOT}/LICENSE.txt DESTINATION .)

    # Install NAP readme
    install(FILES ${NAP_ROOT}/docs/license/README.txt DESTINATION .)
    if(APPLE)
        install(CODE "execute_process(COMMAND sh -c \"cat ${build_tools_dir}/macos_gatekeeper_unquarantine/framework_readme_extra.txt >> ${CMAKE_INSTALL_PREFIX}/README.txt\"
                                      ERROR_QUIET
                                      RESULT_VARIABLE EXIT_CODE)
                      if(NOT \${EXIT_CODE} EQUAL 0)
                          message(FATAL_ERROR \"Failed to add macOS gatekeeper note\")
                      endif()")
    endif()

    # Install NAP Packaged App license
    install(FILES ${NAP_ROOT}/docs/license/NAP.txt DESTINATION cmake/app_creator)
endmacro()

# Package installed Python for distribution with NAP release (for use with nappython, Napkin and interpreter for Python scripts)
macro(package_python)
    if(WIN32)
        # Install main framework
        install(DIRECTORY ${THIRDPARTY_DIR}/python/${NAP_THIRDPARTY_PLATFORM_DIR}/x86_64
                DESTINATION thirdparty/python/${NAP_THIRDPARTY_PLATFORM_DIR}
                CONFIGURATIONS Release)

        # Install framework for Napkin
        if(NAP_ENABLE_PYTHON)
                install(FILES ${THIRDPARTY_DIR}/python/${NAP_THIRDPARTY_PLATFORM_DIR}/x86_64/python36.zip
                        DESTINATION tools/napkin/
                        CONFIGURATIONS Release)
        endif()

        # Install license
        install(FILES ${THIRDPARTY_DIR}/python/${NAP_THIRDPARTY_PLATFORM_DIR}/LICENSE.txt
                DESTINATION thirdparty/python/${NAP_THIRDPARTY_PLATFORM_DIR}
                CONFIGURATIONS Release)
    elseif(UNIX)
        set(PYTHON_PREFIX ${THIRDPARTY_DIR}/python/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH})

        # Install dylib
        file(GLOB PYTHON_DYLIBs ${PYTHON_PREFIX}/lib/*${CMAKE_SHARED_LIBRARY_SUFFIX}*)
        install(FILES ${PYTHON_DYLIBs}
                DESTINATION thirdparty/python/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH}/lib
                CONFIGURATIONS Release
                )

        # Install main framework
        install(DIRECTORY ${PYTHON_PREFIX}/lib/python3.6
                DESTINATION thirdparty/python/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH}/lib/
                CONFIGURATIONS Release
                PATTERN *.pyc EXCLUDE
                PATTERN *.a EXCLUDE
                PATTERN site-packages EXCLUDE)

        # Install command line intrepreter
        file(GLOB PYTHON_INTERPRETER ${PYTHON_PREFIX}/bin/python*)
        install(PROGRAMS ${PYTHON_INTERPRETER}
                DESTINATION thirdparty/python/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH}/bin/
                CONFIGURATIONS Release)

        # Install includes
        install(DIRECTORY ${PYTHON_PREFIX}/include
                DESTINATION thirdparty/python/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH}/
                CONFIGURATIONS Release)

        # Install license
        install(FILES ${PYTHON_PREFIX}/LICENSE
                DESTINATION thirdparty/python/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH}
                CONFIGURATIONS Release)
    endif()
endmacro()

# Package installed Qt for distribution with NAP release (for use with Napkin)
macro(package_qt)
    set(QT_FRAMEWORKS Core Gui Widgets OpenGL)

    # Install licenses. Prefer license from Qt official download if available,
    # falling back to licenses in thirdparty for eg. Linux ARM.
    if(EXISTS ${QT_DIR}/../../Licenses/)
        install(DIRECTORY ${QT_DIR}/../../Licenses/
                DESTINATION thirdparty/Qt/licenses
                CONFIGURATIONS Release
                )
    else()
        install(DIRECTORY ${THIRDPARTY_DIR}/qt/licenses/
                DESTINATION thirdparty/Qt/licenses
                CONFIGURATIONS Release
                )
    endif()
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
        install(FILES ${QT_DIR}/plugins/platforms/qwindowsd.dll
                DESTINATION thirdparty/Qt/plugins/Debug/platforms/
                CONFIGURATIONS Release)

        install(FILES ${QT_DIR}/plugins/platforms/qwindows.dll
                DESTINATION thirdparty/Qt/plugins/Release/platforms/
                CONFIGURATIONS Release)

    elseif(APPLE)
        # macOS appears to depend on these extra Qt frameworks
        list(APPEND QT_FRAMEWORKS PrintSupport DBus)

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
                                          ERROR_QUIET
                                          RESULT_VARIABLE EXIT_CODE)
                          if(NOT \${EXIT_CODE} EQUAL 0)
                              message(FATAL_ERROR \"Failed to change Qt framework installed id\")
                          endif()")

            macos_replace_qt_framework_links("${QT_FRAMEWORKS}" Qt${QT_INSTALL_FRAMEWORK} ${QT_FRAMEWORK_SRC} ${FRAMEWORK_INSTALL_LOC} "@loader_path")
        endforeach()

        set(PATH_FROM_QT_PLUGIN_TOLIB "@loader_path/../../../../thirdparty/Qt/lib")

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

        # Allow Qt platform plugin to find Qt frameworks in thirdparty
        install(CODE "execute_process(COMMAND patchelf
                                              --set-rpath
                                              \$ORIGIN/../../../thirdparty/Qt/lib:\$ORIGIN/../../lib
                                              ${CMAKE_INSTALL_PREFIX}/thirdparty/Qt/plugins/platforms/libqxcb.so
                                      ERROR_QUIET
                                      RESULT_VARIABLE EXIT_CODE)
                      if(NOT \${EXIT_CODE} EQUAL 0)
                          message(FATAL_ERROR \"Failed to run patchelf on Qt's libqxcb.so\")
                      endif()")
    endif()
endmacro()

# Package CMake into release
macro(package_cmake)
    if(UNIX)
        install(DIRECTORY ${THIRDPARTY_DIR}/cmake/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH}
                DESTINATION thirdparty/cmake/${NAP_THIRDPARTY_PLATFORM_DIR}
                CONFIGURATIONS Release
                USE_SOURCE_PERMISSIONS
                )
    else()
        # TODO test using USE_SOURCE_PERMISSIONS on Windows and combining
        install(DIRECTORY ${THIRDPARTY_DIR}/cmake/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH}
                DESTINATION thirdparty/cmake/${NAP_THIRDPARTY_PLATFORM_DIR}
                CONFIGURATIONS Release
                )
    endif()
endmacro()

# Package app directory package & regenerate shortcuts
# DESTINATION: Directory to package into
macro(package_app_dir_shortcuts DESTINATION)
    if(WIN32)
        set(src_dir ${NAP_ROOT}/tools/buildsystem/app_dir_shortcuts/win64)
        install(PROGRAMS ${src_dir}/package.bat
                         ${src_dir}/regenerate.bat
                         ${src_dir}/build.bat
                DESTINATION ${DESTINATION})
    else()
        set(src_dir ${NAP_ROOT}/tools/buildsystem/app_dir_shortcuts/unix)
        install(PROGRAMS ${src_dir}/package.sh
                         ${src_dir}/regenerate.sh
                         ${src_dir}/build.sh
                DESTINATION ${DESTINATION})
    endif()
endmacro()

# Package module directory shortcuts
# DESTINATION: Destination directory
macro(package_module_dir_shortcuts DESTINATION)
    set(src_dir ${NAP_ROOT}/tools/buildsystem/module_dir_shortcuts)
    if(WIN32)
        set(src_dir ${src_dir}/win64)
        install(PROGRAMS ${src_dir}/regenerate.bat
                         ${src_dir}/prepare_to_share.bat
                DESTINATION ${DESTINATION})
    else()
        set(src_dir ${src_dir}/unix)
        install(PROGRAMS ${src_dir}/regenerate.sh
                         ${src_dir}/prepare_to_share.sh
                DESTINATION ${DESTINATION})
    endif()
endmacro()

# Package app in current CMake source dir into framework release
# DEST_DIR: Destination directory
macro(package_app_into_framework_release DEST_DIR)
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
            DESTINATION ${DEST_DIR}
            PATTERN "CMakeLists.txt" EXCLUDE
            PATTERN "cached_app_json.cmake" EXCLUDE
            PATTERN "cached_project_json.cmake" EXCLUDE # TODO transitional/temporary
            PATTERN "dist" EXCLUDE
            PATTERN "*.mesh" EXCLUDE
            PATTERN "cached_module_json.cmake" EXCLUDE
            PATTERN "*.plist" EXCLUDE
            PATTERN "*.ini" EXCLUDE
            PATTERN "regenerate.*" EXCLUDE
            PATTERN "build.*" EXCLUDE
            PATTERN "package.*" EXCLUDE
            )
    install(FILES ${NAP_ROOT}/cmake/app_creator/template/CMakeLists.txt DESTINATION ${DEST_DIR})

    # Package any app extra CMake
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/app_extra.cmake)
        install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/app_extra.cmake DESTINATION ${DEST_DIR})
    endif()
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/app_extra_pre_target.cmake)
        install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/app_extra_pre_target.cmake DESTINATION ${DEST_DIR})
    endif()

    # Package any app module CMake
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/module/)
        # Generate fresh module CMake
        install(CODE "execute_process(COMMAND ${CMAKE_COMMAND}
                                              -DMODULE_CMAKE_OUTPATH=${CMAKE_INSTALL_PREFIX}/${DEST_DIR}/module/CMakeLists.txt
                                              -DAPP_MODULE=1
                                              -DCMAKE_ONLY=1
                                              -DUNPREFIXED_MODULE_NAME_INPUTCASE=Unused
                                              -P ${NAP_ROOT}/cmake/module_creator/module_creator.cmake
                                      RESULT_VARIABLE EXIT_CODE)
                      if(NOT \${EXIT_CODE} EQUAL 0)
                          message(FATAL_ERROR \"Failed to package app module CMake\")
                      endif()")
        # Package module extra
        if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/module_extra.cmake)
            install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/module_extra.cmake DESTINATION ${DEST_DIR}/module)
        endif()
        if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/module_extra_pre_target.cmake)
            install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/module_extra_pre_target.cmake DESTINATION ${DEST_DIR}/module)
        endif()
    endif()

    # Package our regenerate & package shortcuts into the app directory
    package_app_dir_shortcuts(${DEST_DIR})

    # On macOS install Apple property list file
    if(APPLE AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/macos)
        install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/macos/ DESTINATION ${DEST_DIR}/macos)
    endif()
endmacro()

# Package (installed) module in current CMake source dir into framework release
macro(package_module_into_framework_release)
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
            DESTINATION modules/${PROJECT_NAME}
            PATTERN "CMakeLists.txt" EXCLUDE
            PATTERN "cached_module_json.cmake" EXCLUDE
            PATTERN "prepare_to_share.*" EXCLUDE
            PATTERN "regenerate.*" EXCLUDE
            PATTERN "${CMAKE_CURRENT_SOURCE_DIR}/lib/" EXCLUDE
            PATTERN "${CMAKE_CURRENT_SOURCE_DIR}/bin/" EXCLUDE
            PATTERN "${CMAKE_CURRENT_SOURCE_DIR}/build/" EXCLUDE
            PATTERN
            )
endmacro()

# Package system module in current CMake source dir into framework release
macro(package_system_module_into_framework_release)
    # Package headers
    install(DIRECTORY "src/" DESTINATION "system_modules/${PROJECT_NAME}/include"
            FILES_MATCHING PATTERN "*.h")

    # If the module has some extra CMake package it
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/module_extra.cmake)
        install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/module_extra.cmake DESTINATION system_modules/${PROJECT_NAME}/)
    endif()
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/module_extra_pre_target.cmake)
        install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/module_extra_pre_target.cmake DESTINATION system_modules/${PROJECT_NAME}/)
    endif()
    # If the module has some extra content package it
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/dist/cmake)
        install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dist/cmake/ DESTINATION system_modules/${PROJECT_NAME}/)
    endif()

    # If the module has an extra data directory package it
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/data)
        install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data DESTINATION system_modules/${PROJECT_NAME})
    endif()

    # If the module has find modules package them
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/cmake_find_modules)
        install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/cmake_find_modules DESTINATION system_modules/${PROJECT_NAME}/thirdparty)
    endif()

    # Package library
    if(WIN32)
        install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION system_modules/${PROJECT_NAME}/lib/$<CONFIG>-${ARCH}
                                        LIBRARY DESTINATION system_modules/${PROJECT_NAME}/lib/$<CONFIG>-${ARCH}
                                        ARCHIVE DESTINATION system_modules/${PROJECT_NAME}/lib/$<CONFIG>-${ARCH})
        if(PACKAGE_PDBS)
            install(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> DESTINATION system_modules/${PROJECT_NAME}/lib/$<CONFIG>-${ARCH})
        endif()
    elseif(APPLE)
        install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION system_modules/${PROJECT_NAME}/lib/$<CONFIG>-${ARCH})
    else()
        install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION system_modules/${PROJECT_NAME}/lib/${CMAKE_BUILD_TYPE}-${ARCH})
    endif()

    # Package module.json
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/module.json DESTINATION system_modules/${PROJECT_NAME}/)
    if(WIN32 OR APPLE)
        install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/module.json DESTINATION system_modules/${PROJECT_NAME}/lib/$<CONFIG>-${ARCH}/ RENAME ${PROJECT_NAME}.json)
    else()
        install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/module.json DESTINATION system_modules/${PROJECT_NAME}/lib/${CMAKE_BUILD_TYPE}-${ARCH}/ RENAME ${PROJECT_NAME}.json)
    endif()

    # Set packaged RPATH for *nix (for macOS I believe we need to make sure this is being done after we
    # install the target above due to ordering of install_name_tool calling)
    set(NAP_ROOT_LOCATION_TO_MODULE "../../../..")
    if(APPLE)
        if(DEFINED MACOS_EXTRA_RPATH_RELEASE)
            set(MACOS_EXTRA_RPATH_RELEASE "${MACOS_EXTRA_RPATH_RELEASE}")
        else()
            set(MACOS_EXTRA_RPATH_RELEASE "")
        endif()
        if(DEFINED MACOS_EXTRA_RPATH_DEBUG)
            set(MACOS_EXTRA_RPATH_DEBUG "${MACOS_EXTRA_RPATH_DEBUG}")
        else()
            set(MACOS_EXTRA_RPATH_DEBUG "")
        endif()
        set_installed_rpath_on_macos_module_for_dependent_modules("${DEEP_DEPENDENT_NAP_MODULES}" ${PROJECT_NAME} ${NAP_ROOT_LOCATION_TO_MODULE} "${MACOS_EXTRA_RPATH_RELEASE}" "${MACOS_EXTRA_RPATH_DEBUG}")
    elseif(UNIX)
        if(DEFINED LINUX_EXTRA_RPATH)
            set(EXTRA_RPATH "${LINUX_EXTRA_RPATH}")
        else()
            set(EXTRA_RPATH "")
        endif()
        set_installed_rpath_on_linux_object_for_dependent_modules("${DEEP_DEPENDENT_NAP_MODULES}" ${PROJECT_NAME} ${NAP_ROOT_LOCATION_TO_MODULE} ${EXTRA_RPATH})
    endif()
endmacro()

# macOS: Post-build replace Qt framework install names in specified file with new paths built
# from a path prefix and a full framework lib name mapped from another file
# FRAMEWORKS: Qt framework names to replace
# SKIP_FRAMEWORK_NAME: Don't process for this framework in the provided list
# LIB_SRC_LOCATION: The file used to obtain the full framework library name
# LIB_INSTALL_LOCATION: The file to update
# PATH_PREFIX: The new path prefix for the framework
macro(macos_replace_qt_framework_links FRAMEWORKS SKIP_FRAMEWORK_NAME LIB_SRC_LOCATION LIB_INSTALL_LOCATION PATH_PREFIX)
    foreach(QT_LINK_FRAMEWORK ${FRAMEWORKS})
        if(NOT Qt${QT_LINK_FRAMEWORK} STREQUAL ${SKIP_FRAMEWORK_NAME})
            execute_process(COMMAND sh -c "otool -L ${LIB_SRC_LOCATION} | grep Qt${QT_LINK_FRAMEWORK} | awk -F'(' '{print $1}'"
                            OUTPUT_VARIABLE REPLACE_INSTALL_NAME
                            RESULT_VARIABLE EXIT_CODE
                            )
            if(NOT ${EXIT_CODE} EQUAL 0)
                message(FATAL_ERROR "Could not extract Qt library names from ${LIB_SRC_LOCATION}")
            endif()
            if(NOT ${REPLACE_INSTALL_NAME} STREQUAL "")
                # message("Adding install name change in ${QT_INSTALL_FRAMEWORK} for Qt${QT_LINK_FRAMEWORK}")
                string(STRIP ${REPLACE_INSTALL_NAME} REPLACE_INSTALL_NAME)

                # Change link to dylib
                install(CODE "execute_process(COMMAND ${CMAKE_INSTALL_NAME_TOOL}
                                                      -change
                                                      ${REPLACE_INSTALL_NAME}
                                                      ${PATH_PREFIX}/Qt${QT_LINK_FRAMEWORK}
                                                      ${LIB_INSTALL_LOCATION}
                                              ERROR_QUIET
                                              RESULT_VARIABLE EXIT_CODE)
                              if(NOT \${EXIT_CODE} EQUAL 0)
                                  message(FATAL_ERROR \"Failed to replace install name in ${PATH_PREFIX}/Qt${QT_LINK_FRAMEWORK}\")
                              endif()")
            endif()
        endif()
    endforeach()
endmacro()

# macOS: At install time replace Qt framework install names in specified file with new paths built
# from a path prefix and a full framework lib name sourced from another file
# FRAMEWORKS: Qt framework names to replace
# SKIP_FRAMEWORK_NAME: Don't process for this framework in the provided list
# FILEPATH: The file to update
# PATH_PREFIX: The new path prefix for the framework
macro(macos_replace_qt_framework_links_install_time FRAMEWORKS SKIP_FRAMEWORK_NAME FILEPATH PATH_PREFIX)
    foreach(QT_LINK_FRAMEWORK ${FRAMEWORKS})
        if(NOT ${QT_LINK_FRAMEWORK} STREQUAL ${SKIP_FRAMEWORK_NAME})
            macos_replace_single_install_name_link_install_time(${QT_LINK_FRAMEWORK} ${FILEPATH} ${PATH_PREFIX})
        endif()
    endforeach()
endmacro()

# macOS: At install time replace a path prefix of a single lib in the specified file
# REPLACE_LIB_NAME: Library install name to replace
# FILEPATH: The file to update
# PATH_PREFIX: The new path prefix for the framework
macro(macos_replace_single_install_name_link_install_time REPLACE_LIB_NAME FILEPATH PATH_PREFIX)
    # Change link to dylib
    install(CODE "if(EXISTS ${FILEPATH})
                      execute_process(COMMAND sh -c \"otool -L ${FILEPATH} | grep ${REPLACE_LIB_NAME} | awk -F'(' '{print $1}'\"
                                      OUTPUT_VARIABLE REPLACE_INSTALL_NAME
                                      RESULT_VARIABLE EXIT_CODE)
                      if(NOT \${EXIT_CODE} EQUAL 0)
                          message(FATAL_ERROR \"Failed to search library names in ${FILEPATH}\")
                      endif()
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
                                          ERROR_QUIET
                                          RESULT_VARIABLE EXIT_CODE)
                          if(NOT \${EXIT_CODE} EQUAL 0)
                              message(FATAL_ERROR \"Failed to replace library install name in ${FILEPATH}\")
                          endif()
                      endif()
                  endif()
                  ")
endmacro()

# macOS: Remove specified path and subpaths from a single specified object at install time
# FILEPATH: The file to update
# PATH_PREFIX: The path (and sub paths) to remove
macro(macos_remove_rpaths_from_object_at_install_time FILEPATH PATH_PREFIX CONFIGURATION)
    if(CMAKE_HOST_WIN32)
        set(PYTHON_BIN ${THIRDPARTY_DIR}/python/msvc/x86_64/python.exe)
    elseif(CMAKE_HOST_APPLE)
        set(PYTHON_BIN ${THIRDPARTY_DIR}/python/macos/x86_64/bin/python3)
    else()
        set(PYTHON_BIN ${THIRDPARTY_DIR}/python/linux/${ARCH}/bin/python3)
    endif()
    if(NOT EXISTS ${PYTHON_BIN})
        message(FATAL_ERROR \"Python not found at ${PYTHON_BIN}.  Have you updated thirdparty?\")
    endif()

    # Change link to dylib
    install(CODE "if(EXISTS ${FILEPATH})
                      # Clear any system Python path settings
                      unset(ENV{PYTHONHOME})
                      unset(ENV{PYTHONPATH})

                      # Change link to dylib
                      execute_process(COMMAND ${PYTHON_BIN} ${NAP_ROOT}/tools/buildsystem/macos_rpath_stripper/strip_rpaths.py
                                              ${FILEPATH}
                                              ${PATH_PREFIX}
                                      RESULT_VARIABLE EXIT_CODE)
                      if(NOT \${EXIT_CODE} EQUAL 0)
                          message(FATAL_ERROR \"Failed to strip RPATHs on ${FILEPATH}\")
                      endif()
                  endif()
                  "
            CONFIGURATIONS ${CONFIGURATION})
endmacro()

# Linux: Set the packaged RPATH of binary object for its dependent modules
# DEPENDENT_NAP_MODULES: The modules to setup as dependencies
# TARGET_NAME: The module name
# NAP_ROOT_LOCATION_TO_ORIGIN: The relative path from the module to NAP root
# ARGN: Any extra non-NAP-module paths to add
macro(set_installed_rpath_on_linux_object_for_dependent_modules DEPENDENT_NAP_MODULES TARGET_NAME NAP_ROOT_LOCATION_TO_ORIGIN)
    # Add our core paths first
    set(BUILT_RPATH "$ORIGIN/${NAP_ROOT_LOCATION_TO_ORIGIN}/lib/${CMAKE_BUILD_TYPE}-${ARCH}")
    set(BUILT_RPATH "${BUILT_RPATH}:$ORIGIN/${NAP_ROOT_LOCATION_TO_ORIGIN}/thirdparty/python/linux/${ARCH}/lib")
    set(BUILT_RPATH "${BUILT_RPATH}:$ORIGIN/${NAP_ROOT_LOCATION_TO_ORIGIN}/thirdparty/rttr/linux/${ARCH}/bin")

    # Process any extra paths
    set(EXTRA_PATHS ${ARGN})
    foreach(EXTRA_PATH ${EXTRA_PATHS})
        set(BUILT_RPATH "${BUILT_RPATH}:$ORIGIN/${EXTRA_PATH}")
    endforeach()

    # Iterate over each module and append to path
    foreach(module ${DEPENDENT_NAP_MODULES})
        set(THIS_MODULE_PATH "$ORIGIN/${NAP_ROOT_LOCATION_TO_ORIGIN}/system_modules/${module}/lib/${CMAKE_BUILD_TYPE}-${ARCH}")
        set(BUILT_RPATH "${BUILT_RPATH}:${THIS_MODULE_PATH}")
    endforeach(module)
    set_target_properties(${TARGET_NAME} PROPERTIES SKIP_BUILD_RPATH FALSE
                                                    INSTALL_RPATH ${BUILT_RPATH})
endmacro()

# Linux: Set the RPATH for Python on a of binary object on Linux
# TARGET_NAME: The target name
# NAP_ROOT_LOCATION_TO_ORIGIN: The relative path from the object to NAP root
macro(set_python_installed_rpath_on_linux_object TARGET_NAME NAP_ROOT_LOCATION_TO_ORIGIN)
    # Add our core paths
    set(BUILT_RPATH "$ORIGIN/${NAP_ROOT_LOCATION_TO_ORIGIN}/lib/${CMAKE_BUILD_TYPE}")
    set(BUILT_RPATH "${BUILT_RPATH}:$ORIGIN/${NAP_ROOT_LOCATION_TO_ORIGIN}/thirdparty/python/linux/${ARCH}/lib")

    set_target_properties(${TARGET_NAME} PROPERTIES SKIP_BUILD_RPATH FALSE
                                                    INSTALL_RPATH ${BUILT_RPATH})
endmacro()

# macOS: Set the packaged RPATH of a binary object for its dependent modules for a single build configuration
# CONFIG: The build configuration
# DEPENDENT_NAP_MODULES: The modules to setup as dependencies
# OBJECT_FILENAME: The module filename to work on
# NAP_ROOT_LOCATION_TO_OBJECT: The relative path from the module to NAP root
# ARGN: Any extra non-NAP-module paths to add
macro(set_single_config_installed_rpath_on_macos_object_for_dependent_modules CONFIG DEPENDENT_NAP_MODULES OBJECT_FILENAME NAP_ROOT_LOCATION_TO_OBJECT)
    # Set basic paths
    ensure_macos_file_has_rpath_at_install(${OBJECT_FILENAME} "@loader_path/${NAP_ROOT_LOCATION_TO_OBJECT}/thirdparty/python/macos/${ARCH}/lib")
    ensure_macos_file_has_rpath_at_install(${OBJECT_FILENAME} "@loader_path/${NAP_ROOT_LOCATION_TO_OBJECT}/thirdparty/rttr/macos/${ARCH}/bin")
    ensure_macos_file_has_rpath_at_install(${OBJECT_FILENAME} "@loader_path/${NAP_ROOT_LOCATION_TO_OBJECT}/lib/${CONFIG}-${ARCH}")

    # Set module paths
    foreach(DEPENDENT_MODULE_NAME ${DEPENDENT_NAP_MODULES})
        ensure_macos_file_has_rpath_at_install(${OBJECT_FILENAME} "@loader_path/${NAP_ROOT_LOCATION_TO_OBJECT}/system_modules/${DEPENDENT_MODULE_NAME}/lib/${CONFIG}-${ARCH}")
    endforeach()

    # Process any extra paths
    set(EXTRA_PATHS ${ARGN})
    foreach(EXTRA_PATH ${EXTRA_PATHS})
        ensure_macos_file_has_rpath_at_install(${OBJECT_FILENAME} "@loader_path/${EXTRA_PATH}")
    endforeach()
endmacro()

# macOS: Set the packaged RPATH of a module for its dependent modules
# DEPENDENT_NAP_MODULES: The modules to setup as dependencies
# MODULE_NAME: The module to work on
# NAP_ROOT_LOCATION_TO_MODULE: The relative path from the module to NAP root
# EXTRA_RPATH_RELEASE: Any extra non-NAP-module paths to add for release build
# EXTRA_RPATH_DEBUG: Any extra non-NAP-module paths to add for debug build
macro(set_installed_rpath_on_macos_module_for_dependent_modules DEPENDENT_NAP_MODULES MODULE_NAME NAP_ROOT_LOCATION_TO_MODULE EXTRA_RPATH_RELEASE EXTRA_RPATH_DEBUG)
    foreach(MODULECONFIG Release Debug)
        # Set basic paths
        ensure_macos_module_has_rpath_at_install(${MODULE_NAME} ${MODULECONFIG} "@loader_path/${NAP_ROOT_LOCATION_TO_MODULE}/thirdparty/python/macos/${ARCH}/lib")
        ensure_macos_module_has_rpath_at_install(${MODULE_NAME} ${MODULECONFIG} "@loader_path/${NAP_ROOT_LOCATION_TO_MODULE}/thirdparty/rttr/macos/${ARCH}/bin")
        ensure_macos_module_has_rpath_at_install(${MODULE_NAME} ${MODULECONFIG} "@loader_path/${NAP_ROOT_LOCATION_TO_MODULE}/lib/${MODULECONFIG}")

        # Set module paths
        foreach(DEPENDENT_MODULE_NAME ${DEPENDENT_NAP_MODULES})
            ensure_macos_module_has_rpath_at_install(${MODULE_NAME} ${MODULECONFIG} "@loader_path/${NAP_ROOT_LOCATION_TO_MODULE}/system_modules/${DEPENDENT_MODULE_NAME}/lib/${MODULECONFIG}")
        endforeach()

        # Process any extra paths
        if(${MODULECONFIG} STREQUAL Release)
            set(EXTRA_PATHS "${EXTRA_RPATH_RELEASE}")
        else()
            set(EXTRA_PATHS "${EXTRA_RPATH_DEBUG}")
        endif()
        foreach(EXTRA_PATH ${EXTRA_PATHS})
            ensure_macos_module_has_rpath_at_install(${MODULE_NAME} ${MODULECONFIG} "@loader_path/${EXTRA_PATH}")
        endforeach()
    endforeach()
endmacro()

# macOS: Ensure the specified module has the provided RPATH for the specified build configuration
# MODULE_NAME: The module to work with
# CONFIG: The build configuration
# PATH_TO_ADD: The path to check/add
macro(ensure_macos_module_has_rpath_at_install MODULE_NAME CONFIG PATH_TO_ADD)
    set(MODULE_FILENAME ${CMAKE_INSTALL_PREFIX}/system_modules/${MODULE_NAME}/lib/${CONFIG}-${ARCH}/${MODULE_NAME}.dylib)
    ensure_macos_file_has_rpath_at_install(${MODULE_FILENAME} ${PATH_TO_ADD})
endmacro()

# macOS: Ensure the specified binary object has the provided RPATH
# FILENAME: The binary object
# PATH_TO_ADD: The path to check/add
macro(ensure_macos_file_has_rpath_at_install FILENAME PATH_TO_ADD)
    install(CODE "execute_process(COMMAND ${CMAKE_INSTALL_NAME_TOOL}
                                          -add_rpath
                                          ${PATH_TO_ADD}
                                          ${FILENAME}
                                  ERROR_QUIET)")
endmacro()

# Don't build the app module while doing a framework build unless explicitly requested
# INCLUDE_ONLY_WITH_NAIVI_APPS: whether the module is for a app that should only be
#   packaged if packaging Naivi apps
function(exclude_from_build_when_packaging INCLUDE_ONLY_WITH_NAIVI_APPS)
    if(NAP_PACKAGED_BUILD)
        if(NOT BUILD_APPS)
            set_target_properties(${PROJECT_NAME} PROPERTIES EXCLUDE_FROM_ALL TRUE)
        elseif(${INCLUDE_ONLY_WITH_NAIVI_APPS} AND NOT PACKAGE_NAIVI_APPS)
            set_target_properties(${PROJECT_NAME} PROPERTIES EXCLUDE_FROM_ALL TRUE)
        endif()
    endif()
endfunction()

# Package path mappings, for appropriate platform
function(package_path_mappings)
    if(WIN32)
        set(MAPPINGS_PREFIX win64)
    else()
        set(MAPPINGS_PREFIX unix)
    endif()
    install(DIRECTORY ${NAP_ROOT}/tools/buildsystem/path_mappings/${MAPPINGS_PREFIX}/
            DESTINATION tools/buildsystem/path_mappings
            PATTERN "source.json" EXCLUDE)
endfunction()

