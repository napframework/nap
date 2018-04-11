# NAP modules which Napkin uses (as a minimum)
set(NAPKIN_DEPENDENT_NAP_MODULES mod_napscene mod_nappython mod_napmath mod_naprender mod_napvideo)
# Qt frameworks which Napkin uses
set(NAPKIN_QT_INSTALL_FRAMEWORKS QtCore QtGui QtWidgets QtPrintSupport)

if(WIN32 OR APPLE)
    # Deploy Napkin executable on Win64 & macOS for running against packaged NAP
    add_custom_command(TARGET ${PROJECT_NAME}
                       POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy ${NAP_ROOT}/tools/platform/napkin/$<CONFIG>/napkin${CMAKE_EXECUTABLE_SUFFIX} $<TARGET_FILE_DIR:${PROJECT_NAME}>
                       )
    # Deploy Napkin resources on Win64 & macOS for running against packaged NAP
    add_custom_command(TARGET ${PROJECT_NAME}
                       POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy_directory ${NAP_ROOT}/tools/platform/napkin/resources $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources
                       )
else()
    # Deploy Napkin executable on Linux for running against packaged NAP
    get_target_property(PROJ_BUILD_PATH ${PROJECT_NAME} RUNTIME_OUTPUT_DIRECTORY)
    add_custom_command(TARGET ${PROJECT_NAME}
                       POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy ${NAP_ROOT}/tools/platform/napkin/${CMAKE_BUILD_TYPE}/napkin${CMAKE_EXECUTABLE_SUFFIX} ${PROJ_BUILD_PATH}
                       )

    # Deploy Napkin resources on Linux for running against packaged NAP
    add_custom_command(TARGET ${PROJECT_NAME}
                       POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy_directory ${NAP_ROOT}/tools/platform/napkin/resources ${PROJ_BUILD_PATH}/resources
                       )
endif()

if(WIN32)
    # Deploy main Qt libs on Win64
    add_custom_command(TARGET ${PROJECT_NAME}
                       POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy_directory ${THIRDPARTY_DIR}/Qt/bin/$<CONFIG> $<TARGET_FILE_DIR:${PROJECT_NAME}>
                       )

    # Deploy Qt plugins from thirdparty on Win64.  Unlike macOS Windows won't find the plugins under a plugins folder, the categories need to sit beside the binary.
    add_custom_command(TARGET ${PROJECT_NAME}
                       POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy_directory ${THIRDPARTY_DIR}/Qt/plugins/$<CONFIG>/platforms $<TARGET_FILE_DIR:${PROJECT_NAME}>/platforms
                       )

    # Deploy Python modules post-build on Win64
    win64_copy_python_modules_postbuild()

    # Deploy dependent modules on Win64
    foreach(MODULE_NAME ${NAPKIN_DEPENDENT_NAP_MODULES})
        add_custom_command(TARGET ${PROJECT_NAME}
                           POST_BUILD
                           COMMAND ${CMAKE_COMMAND} 
                                   -E copy
                                   ${NAP_ROOT}/modules/${MODULE_NAME}/lib/$<CONFIG>/${MODULE_NAME}.dll
                                   $<TARGET_FILE_DIR:${PROJECT_NAME}>
                           )

        # Run any module_extra to install module dependent DLLs
        if(EXISTS ${NAP_ROOT}/modules/${MODULE_NAME}/module_extra.cmake)
            include(${NAP_ROOT}/modules/${MODULE_NAME}/module_extra.cmake)
        endif()
    endforeach()
elseif(APPLE)
    # Deploy Qt plugins from thirdparty on macOS for running against packaged NAP
    add_custom_command(TARGET ${PROJECT_NAME}
                       POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy_directory 
                               ${THIRDPARTY_DIR}/Qt/plugins 
                               $<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins
                       )

    # ---- Deploy napkin into bin dir when we do project builds running against released NAP ------
    set(PATH_TO_NAP_ROOT "@executable_path/../../../..")
    set(PATH_TO_THIRDPARTY "${PATH_TO_NAP_ROOT}/thirdparty")

    # Add core libs path to RPATH
    add_custom_command(TARGET ${PROJECT_NAME}
                       POST_BUILD
                       COMMAND ${CMAKE_INSTALL_NAME_TOOL} 
                               -add_rpath ${PATH_TO_NAP_ROOT}/lib/$<CONFIG> 
                               $<TARGET_FILE_DIR:${PROJECT_NAME}>/napkin
                       )

    # Add module paths to RPATH
    foreach(MODULE_NAME ${NAPKIN_DEPENDENT_NAP_MODULES})
        add_custom_command(TARGET ${PROJECT_NAME}
                           POST_BUILD
                           COMMAND ${CMAKE_INSTALL_NAME_TOOL} 
                                   -add_rpath ${PATH_TO_NAP_ROOT}/modules/${MODULE_NAME}/lib/$<CONFIG> 
                                   $<TARGET_FILE_DIR:${PROJECT_NAME}>/napkin
                           )
    endforeach()

    # Update paths to Qt frameworks in Napkin.  Using explicit paths in an attempt to avoid loading
    # any installed Qt library.
    macos_replace_qt_framework_links("${NAPKIN_QT_INSTALL_FRAMEWORKS}" 
                                     ${NAP_ROOT}/tools/platform/napkin/Release/napkin
                                     $<TARGET_FILE_DIR:${PROJECT_NAME}>/napkin
                                     "${PATH_TO_THIRDPARTY}/Qt/lib/"
                                     )

    # ---- Install napkin with packaged project ------

    # Install executable into packaged project
    install(PROGRAMS ${NAP_ROOT}/tools/platform/napkin/Release/napkin
            DESTINATION .)
    # Install resources into packaged project
    install(DIRECTORY ${NAP_ROOT}/tools/platform/napkin/resources
            DESTINATION .)
    # Install main Qt libs from thirdparty into packaged project
    install(DIRECTORY ${THIRDPARTY_DIR}/Qt/lib/
            DESTINATION lib)
    # Install Qt plugins from thirdparty into packaged project
    install(DIRECTORY ${THIRDPARTY_DIR}/Qt/plugins
            DESTINATION .)

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
                                          ${CMAKE_INSTALL_PREFIX}/napkin
                                  ERROR_QUIET)")

    # Update paths to Qt frameworks in libqcocoa plugin
    macos_replace_qt_framework_links_install_time("${NAPKIN_QT_INSTALL_FRAMEWORKS}" 
                                                  "libqcocoa"
                                                  ${CMAKE_INSTALL_PREFIX}/plugins/platforms/libqcocoa.dylib
                                                  "@loader_path/../../lib/"
                                                  )

    # Update paths to Qt frameworks in napkin
    macos_replace_qt_framework_links_install_time("${NAPKIN_QT_INSTALL_FRAMEWORKS}" 
                                                  "napkin"
                                                  ${CMAKE_INSTALL_PREFIX}/napkin
                                                  "@loader_path/lib/"
                                                  )
else()
    # Linux

    # Deploy Qt plugins from thirdparty for Napkin running against released NAP
    add_custom_command(TARGET ${PROJECT_NAME}
                       POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy_directory ${THIRDPARTY_DIR}/Qt/plugins/platforms/ $<TARGET_FILE_DIR:${PROJECT_NAME}>/platforms
                       )

    # Allow Qt platform plugin to find Qt frameworks in thirdparty (against released NAP)
    add_custom_command(TARGET ${PROJECT_NAME}
                       POST_BUILD
                       COMMAND patchelf --set-rpath 
                               [=[\$$ORIGIN/../../../../../thirdparty/Qt/lib]=]
                               $<TARGET_FILE_DIR:${PROJECT_NAME}>/platforms/libqxcb.so
                       )


    # Install executable into packaged project
    install(PROGRAMS ${NAP_ROOT}/tools/platform/napkin/Release/napkin
            DESTINATION .)
    # Install resources into packaged project
    install(DIRECTORY ${NAP_ROOT}/tools/platform/napkin/resources
            DESTINATION .)
    # Install main Qt libs from thirdparty into packaged project
    install(DIRECTORY ${THIRDPARTY_DIR}/Qt/lib/
            DESTINATION lib)
    # Install Qt plugins from thirdparty into packaged project
    install(DIRECTORY ${THIRDPARTY_DIR}/Qt/plugins/platforms
            DESTINATION .)

    # Allow Qt platform plugin to find Qt frameworks in thirdparty (packaged project)
    install(CODE "execute_process(COMMAND patchelf
                                          --set-rpath
                                          \$ORIGIN/../lib
                                          ${CMAKE_INSTALL_PREFIX}/platforms/libqxcb.so
                                  ERROR_QUIET)")   

    # Ensure we have our dependent modules
    set(INSTALLING_MODULE_FOR_NAPKIN TRUE)
    foreach(NAP_MODULE ${NAPKIN_DEPENDENT_NAP_MODULES})
        find_nap_module(${NAP_MODULE})
    endforeach()
    unset(INSTALLING_MODULE_FOR_NAPKIN)
endif()

# Install Qt licenses into packaged app
install(DIRECTORY ${THIRDPARTY_DIR}/Qt/licenses/ DESTINATION licenses/Qt)
