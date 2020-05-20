if(NOT DEFINED NAP_PACKAGED_APP_BUILD)
    return()
endif()

include(${CMAKE_CURRENT_LIST_DIR}/dist_shared_crossplatform.cmake)

# NAP modules which Napkin uses (as a minimum)
set(NAPKIN_DEPENDENT_NAP_MODULES mod_napscene mod_nappython mod_napmath mod_naprender mod_napvideo mod_napaudio mod_napfont mod_napinput)
# Qt frameworks which Napkin uses
set(NAPKIN_QT_INSTALL_FRAMEWORKS QtCore QtGui QtWidgets QtPrintSupport QtOpenGL)
message(STATUS "Preparing Napkin deployment to output directory")

# Deploy into napkin directory alongside release/debug build dirs when not packaging an app
if(UNIX)
    set(NAPDEPLOY_PATH_SUFFIX "../napkin/")
else()
    set(NAPDEPLOY_PATH_SUFFIX "")
endif()

#   if(WIN32 OR APPLE)
#       add_custom_command(TARGET ${PROJECT_NAME}
#                          POST_BUILD
#                          COMMAND ${CMAKE_COMMAND} -E copy ${NAP_ROOT}/tools/platform/napkin/napkin${CMAKE_EXECUTABLE_SUFFIX} $<TARGET_FILE_DIR:${PROJECT_NAME}>/${NAPDEPLOY_PATH_SUFFIX}
#                          )
#       # Deploy Napkin resources on Win64 & macOS for running against packaged NAP
#       add_custom_command(TARGET ${PROJECT_NAME}
#                          POST_BUILD
#                          COMMAND ${CMAKE_COMMAND} -E copy_directory ${NAP_ROOT}/tools/platform/napkin/resources $<TARGET_FILE_DIR:${PROJECT_NAME}>/${NAPDEPLOY_PATH_SUFFIX}/resources
#                          )
#   else()
#       # Deploy Napkin executable on Linux for running against packaged NAP
#       get_target_property(PROJ_BUILD_PATH ${PROJECT_NAME} RUNTIME_OUTPUT_DIRECTORY)
#       add_custom_command(TARGET ${PROJECT_NAME}
#                          POST_BUILD
#                          COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJ_BUILD_PATH}/../napkin/
#                          )
#       add_custom_command(TARGET ${PROJECT_NAME}
#                          POST_BUILD
#                          COMMAND ${CMAKE_COMMAND} -E copy ${NAP_ROOT}/tools/platform/napkin/napkin${CMAKE_EXECUTABLE_SUFFIX} ${PROJ_BUILD_PATH}/../napkin/
#                          )

#       # Deploy Napkin resources on Linux for running against packaged NAP
#       add_custom_command(TARGET ${PROJECT_NAME}
#                          POST_BUILD
#                          COMMAND ${CMAKE_COMMAND} -E copy_directory ${NAP_ROOT}/tools/platform/napkin/resources ${PROJ_BUILD_PATH}/../napkin/resources
#                          )
#   endif()

if(WIN32)
    # Deploy main Qt libs on Win64
    add_custom_command(TARGET ${PROJECT_NAME}
                       POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy_directory ${THIRDPARTY_DIR}/Qt/bin/Release $<TARGET_FILE_DIR:${PROJECT_NAME}>/${NAPDEPLOY_PATH_SUFFIX}
                       )

    # Deploy Qt plugins from thirdparty on Win64.  Unlike macOS Windows won't find the plugins under a plugins folder, the categories need to sit beside the binary.
    add_custom_command(TARGET ${PROJECT_NAME}
                       POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy_directory ${THIRDPARTY_DIR}/Qt/plugins/Release/platforms $<TARGET_FILE_DIR:${PROJECT_NAME}>/${NAPDEPLOY_PATH_SUFFIX}/platforms
                       )

    # Deploy Python modules post-build on Win64
    win64_copy_python_modules_postbuild(FALSE)

    # Deploy dependent modules on Win64
    foreach(MODULE_NAME ${NAPKIN_DEPENDENT_NAP_MODULES})
        add_custom_command(TARGET ${PROJECT_NAME}
                           POST_BUILD
                           COMMAND ${CMAKE_COMMAND} 
                                   -E copy
                                   ${NAP_ROOT}/modules/${MODULE_NAME}/lib/Release/${MODULE_NAME}.dll
                                   $<TARGET_FILE_DIR:${PROJECT_NAME}>/${NAPDEPLOY_PATH_SUFFIX}
                           )

        # Run any module_extra to install module dependent DLLs
        set(INSTALLING_MODULE_FOR_NAPKIN TRUE)
        if(EXISTS ${NAP_ROOT}/modules/${MODULE_NAME}/module_extra.cmake)
            include(${NAP_ROOT}/modules/${MODULE_NAME}/module_extra.cmake)
        endif()
        unset(INSTALLING_MODULE_FOR_NAPKIN)
    endforeach()

    # If not packaging a single application deploy DLLs for Napkin into Napkin's separate directory post-build
    if(NOT DEFINED PROJECT_PACKAGE_BIN_DIR)
        # NAP RTTI
        add_custom_command(
            TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${NAPRTTI_LIBS_DIR}/Release/naprtti.dll $<TARGET_FILE_DIR:${PROJECT_NAME}>/../napkin/
        )

        # RTTR
        add_custom_command(
            TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:RTTR::Core>/rttr_core.dll $<TARGET_FILE_DIR:${PROJECT_NAME}>/../napkin/
        )

        # Python DLLs
        win64_copy_python_dlls_postbuild(TRUE)

        # NAP core 
        add_custom_command(
            TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${NAPCORE_LIBS_DIR}/Release/napcore.dll $<TARGET_FILE_DIR:${PROJECT_NAME}>/../napkin/
        )

        # FreeImage
        add_custom_command(
            TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:FreeImage> $<TARGET_FILE_DIR:${PROJECT_NAME}>/../napkin/
        )

        # SDL2
        add_custom_command(
            TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $ENV{SDL2DIR}/lib/SDL2.dll $<TARGET_FILE_DIR:${PROJECT_NAME}>/../napkin/
       )

        # GLEW
        add_custom_command(
            TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:GLEW> $<TARGET_FILE_DIR:${PROJECT_NAME}>/../napkin/
        )

        # Assimp
        add_custom_command(
            TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:assimp> $<TARGET_FILE_DIR:${PROJECT_NAME}>/../napkin/
        )

        # FreeType
        add_custom_command(
            TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} 
                    -E copy 
                    $<TARGET_FILE_DIR:freetype>/../Release/freetype.dll 
                    $<TARGET_FILE_DIR:${PROJECT_NAME}>/../napkin/
        )
    endif()
elseif(APPLE)
    list(APPEND NAPKIN_QT_INSTALL_FRAMEWORKS QtDBus)

#   # Deploy Qt plugins from thirdparty on macOS for running against packaged NAP
#   add_custom_command(TARGET ${PROJECT_NAME}
#                      POST_BUILD
#                      COMMAND ${CMAKE_COMMAND} -E copy_directory 
#                              ${THIRDPARTY_DIR}/Qt/plugins 
#                              $<TARGET_FILE_DIR:${PROJECT_NAME}>/../napkin/plugins/
#                      )

#   # ---- Deploy napkin into bin dir when we do project builds running against released NAP ------
#   set(PATH_TO_NAP_ROOT "@executable_path/../../../..")
#   set(PATH_TO_THIRDPARTY "${PATH_TO_NAP_ROOT}/thirdparty")

#   # Add core libs path to RPATH
#   add_custom_command(TARGET ${PROJECT_NAME}
#                      POST_BUILD
#                      COMMAND ${CMAKE_INSTALL_NAME_TOOL} 
#                              -add_rpath ${PATH_TO_NAP_ROOT}/lib/Release 
#                              $<TARGET_FILE_DIR:${PROJECT_NAME}>/../napkin/napkin
#                      )

#   # Add module paths to RPATH
#   foreach(MODULE_NAME ${NAPKIN_DEPENDENT_NAP_MODULES})
#       add_custom_command(TARGET ${PROJECT_NAME}
#                          POST_BUILD
#                          COMMAND ${CMAKE_INSTALL_NAME_TOOL} 
#                                  -add_rpath ${PATH_TO_NAP_ROOT}/modules/${MODULE_NAME}/lib/Release 
#                                  $<TARGET_FILE_DIR:${PROJECT_NAME}>/../napkin/napkin
#                          )
#   endforeach()

#   # Update paths to Qt frameworks in Napkin.  Using explicit paths in an attempt to avoid loading
#   # any installed Qt library.
#   macos_replace_qt_framework_links("${NAPKIN_QT_INSTALL_FRAMEWORKS}" 
#                                    ${NAP_ROOT}/tools/napkin/napkin
#                                    $<TARGET_FILE_DIR:${PROJECT_NAME}>/../napkin/napkin
#                                    "${PATH_TO_THIRDPARTY}/Qt/lib/"
#                                    )

    # ---- Install napkin with packaged project ------

    # Install executable into packaged project
    install(PROGRAMS ${NAP_ROOT}/tools/napkin/napkin
            DESTINATION napkin)
    # Install resources into packaged project
    install(DIRECTORY ${NAP_ROOT}/tools/napkin/resources
            DESTINATION napkin)
    # Install main Qt libs from thirdparty into packaged project
    install(DIRECTORY ${THIRDPARTY_DIR}/Qt/lib/
            DESTINATION napkin/lib)
    # Install Qt plugins from thirdparty into packaged project
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
                                  ERROR_QUIET)")
    install(CODE "execute_process(COMMAND ${CMAKE_INSTALL_NAME_TOOL} 
                                          -add_rpath 
                                          @executable_path/../lib
                                          ${CMAKE_INSTALL_PREFIX}/napkin/napkin
                                  ERROR_QUIET)")


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

#   # Deploy Qt plugins from thirdparty for Napkin running against released NAP
#   add_custom_command(TARGET ${PROJECT_NAME}
#                      POST_BUILD
#                      COMMAND ${CMAKE_COMMAND} -E copy_directory ${THIRDPARTY_DIR}/Qt/plugins/platforms/ $<TARGET_FILE_DIR:${PROJECT_NAME}>/../napkin/platforms
#                      )

#   # Allow Qt platform plugin to find Qt frameworks in thirdparty (against released NAP)
#   add_custom_command(TARGET ${PROJECT_NAME}
#                      POST_BUILD
#                      COMMAND patchelf --set-rpath 
#                              [=[\$$ORIGIN/../../../../../thirdparty/Qt/lib]=]
#                              $<TARGET_FILE_DIR:${PROJECT_NAME}>/../napkin/platforms/libqxcb.so
#                      )


    # Install executable into packaged project
    install(PROGRAMS ${NAP_ROOT}/tools/napkin/napkin
            DESTINATION napkin)
    # Install resources into packaged project
    install(DIRECTORY ${NAP_ROOT}/tools/napkin/resources
            DESTINATION napkin)
    # Install main Qt libs from thirdparty into packaged project
    install(DIRECTORY ${THIRDPARTY_DIR}/Qt/lib/
            DESTINATION napkin/lib)
    # Install Qt plugins from thirdparty into packaged project
    install(DIRECTORY ${THIRDPARTY_DIR}/Qt/plugins/platforms
            DESTINATION napkin)

    # Allow Qt platform plugin to find Qt frameworks in thirdparty (packaged project)
    install(CODE "execute_process(COMMAND patchelf
                                          --set-rpath
                                          \$ORIGIN/../lib
                                          ${CMAKE_INSTALL_PREFIX}/napkin/platforms/libqxcb.so
                                  ERROR_QUIET)")   

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
