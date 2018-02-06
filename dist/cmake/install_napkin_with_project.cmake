# Install executable
add_custom_command(TARGET ${PROJECT_NAME}
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy ${NAP_ROOT}/tools/platform/napkin/$<CONFIG>/napkin${CMAKE_EXECUTABLE_SUFFIX} $<TARGET_FILE_DIR:${PROJECT_NAME}>
                   )

# Install resources
add_custom_command(TARGET ${PROJECT_NAME}
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory ${NAP_ROOT}/tools/platform/napkin//resources $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources
                   )


# # Install main QT libs from thirdparty
# TODO use for Win32 or remove?
# add_custom_command(TARGET ${PROJECT_NAME}
#                    POST_BUILD
#                    COMMAND ${CMAKE_COMMAND} -E copy_directory ${THIRDPARTY_DIR}/QT/lib $<TARGET_FILE_DIR:${PROJECT_NAME}>/lib/
#                    )

# Install QT plugins from thirdparty
add_custom_command(TARGET ${PROJECT_NAME}
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory ${THIRDPARTY_DIR}/QT/plugins $<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins
                   )

set(NAPKIN_DEPENDENT_NAP_MODULES mod_napscene mod_nappython mod_napmath)
set(NAPKIN_QT_INSTALL_FRAMEWORKS QtCore QtGui QtWidgets QtPrintSupport)

if(APPLE)
    # ---- Deploy napkin into bin dir when we do project builds ------
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

    # Update paths to Qt frameworks in napkin.  Using explicit paths in an attempt to avoid loading
    # any installed Qt library.
    macos_replace_qt_framework_links("${NAPKIN_QT_INSTALL_FRAMEWORKS}" 
                                     ${NAP_ROOT}/tools/platform/napkin/Release/napkin
                                     $<TARGET_FILE_DIR:${PROJECT_NAME}>/napkin
                                     "${PATH_TO_THIRDPARTY}/QT/lib/"
                                     )

    # Update path to Python
    macos_replace_single_install_name_link("Python" 
                                           ${NAP_ROOT}/tools/platform/napkin/Release/napkin
                                           $<TARGET_FILE_DIR:${PROJECT_NAME}>/napkin
                                           "@rpath")


    # ---- Install napkin with packaged project ------

    # Install executable
    install(PROGRAMS ${NAP_ROOT}/tools/platform/napkin/Release/napkin
            DESTINATION .)
    # Install resources
    install(DIRECTORY ${NAP_ROOT}/tools/platform/napkin/resources
            DESTINATION .)
    # Install main QT libs from thirdparty
    install(DIRECTORY ${THIRDPARTY_DIR}/QT/lib/
            DESTINATION lib)
    # Install QT plugins from thirdparty
    install(DIRECTORY ${THIRDPARTY_DIR}/QT/plugins
            DESTINATION .)

    # Ensure we have our dependent modules
    set(NAPKIN_MODULES mod_nappython mod_napmath mod_napscene)
    set(INSTALLING_MODULE_FOR_NAPKIN TRUE)
    foreach(NAP_MODULE ${NAPKIN_MODULES})
        find_nap_module(${NAP_MODULE})
    endforeach()
    unset(INSTALLING_MODULE_FOR_NAPKIN)

    # Add core libs path to RPATH
    install(CODE "execute_process(COMMAND ${CMAKE_INSTALL_NAME_TOOL} 
                                          -add_rpath 
                                          @executable_path/lib
                                          ${CMAKE_INSTALL_PREFIX}/napkin
                                  ERROR_QUIET)")

    # Update paths to Qt frameworks in libqcocoa plugin.  Using explicit paths in an attempt to avoid loading
    # any installed Qt library.
    file(GLOB imageformat_plugins RELATIVE ${THIRDPARTY_DIR}/QT/plugins/imageformats "${THIRDPARTY_DIR}/QT/plugins/imageformats/*dylib")
    foreach(imageformat_plugin ${imageformat_plugins})
        macos_replace_qt_framework_links_install_time("${NAPKIN_QT_INSTALL_FRAMEWORKS}"
                                                      ${imageformat_plugin} 
                                                      ${CMAKE_INSTALL_PREFIX}/plugins/imageformats/${imageformat_plugin}
                                                      "@loader_path/../../lib/"
                                                      )
    endforeach()

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

    # Update path to Python
    macos_replace_single_install_name_link_install_time("Python" ${CMAKE_INSTALL_PREFIX}/napkin "@loader_path/lib/")
endif()