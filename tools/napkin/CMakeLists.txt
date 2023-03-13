project(napkin)
nap_qt_pre()

file(GLOB_RECURSE SOURCES src/*.cpp)
file(GLOB_RECURSE HEADERS src/*.h)
file(GLOB_RECURSE RESOURCES resources/*.qrc)

if (MSVC)
    set(RESOURCES ${RESOURCES} resources/resources.rc)
endif()

add_executable(${PROJECT_NAME} ${SOURCES} ${UI_HEADERS} ${HEADERS} ${RESOURCES})
if(MSVC OR APPLE)
    if(NOT DEFINED NAP_PACKAGED_BUILD)
        set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${NAP_ROOT}/bin/$<CONFIG>-${ARCH}/napkin)
        set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${NAP_ROOT}/bin/$<CONFIG>-${ARCH}/napkin)
    else()
        set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${NAP_ROOT}/packaging_bin/$<CONFIG>-${ARCH}/napkin)
        set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${NAP_ROOT}/packaging_bin/$<CONFIG>-${ARCH}/napkin)
    endif()
else()
    if(NOT DEFINED NAP_PACKAGED_BUILD)
        set(NAPKIN_OUTPUT_PATH ${NAP_ROOT}/bin/${BUILD_CONF}/napkin)
        set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${NAPKIN_OUTPUT_PATH})
        file(MAKE_DIRECTORY ${NAPKIN_OUTPUT_PATH})
    endif()
endif()
set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)")
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Tools)
target_include_directories(${PROJECT_NAME} PUBLIC src)

if(WIN32)
    set_property(TARGET ${PROJECT_NAME} PROPERTY WIN32_EXECUTABLE true)
endif()

if(${ARCH} STREQUAL "armhf")
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -z muldefs")
endif()

# The NAP modules Napkin needs
set(NAP_MODULES_DEPENDENCIES napscene napcolor napmath)

# Add python if enabled
if(NAP_ENABLE_PYTHON)
    list(APPEND NAP_MODULES_DEPENDENCIES nappython)
endif()

# Add module lib dependencies
add_dependencies(${PROJECT_NAME} ${NAP_MODULES_DEPENDENCIES})

# Set libraries
set(LIBS
    napcore
    napqt
    ${NAP_MODULES_DEPENDENCIES}
    Qt5::Widgets Qt5::Core Qt5::Gui
    )


target_link_libraries(${PROJECT_NAME} ${LIBS} ${QT_LIBS})
nap_qt_post(${PROJECT_NAME})

# Add the runtime paths for RTTR
if(APPLE)
    macos_add_rttr_rpath()
endif()

# Copy resources
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   ${CMAKE_CURRENT_LIST_DIR}/resources
                   $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources
                   COMMENT "Copy Resources")

# MSVC copy steps
if(MSVC)
    if(DEFINED NAP_PACKAGED_BUILD)
        set(NAPKIN_COPY_SRC ${NAP_ROOT}/packaging_bin)
    else()
        set(NAPKIN_COPY_SRC ${NAP_ROOT}/bin)
    endif()

    # Deploy dependent modules on Win64
    foreach(MODULE_NAME ${NAP_MODULES_DEPENDENCIES})
        add_custom_command(TARGET ${PROJECT_NAME}
                           POST_BUILD
                           COMMAND ${CMAKE_COMMAND} 
                                   -E copy
                                   $<TARGET_FILE:${MODULE_NAME}>
                                   $<TARGET_FILE_DIR:${PROJECT_NAME}>
                           )
    endforeach()

    # Deploy Qt plugins from thirdparty on Win64. Unlike macOS, Windows won't find 
    # the plugins under a plugins folder, the categories need to sit beside the binary.
    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${QTDIR}/plugins/platforms $<TARGET_FILE_DIR:${PROJECT_NAME}>/platforms
        )

    # NAP RTTI
    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:naprtti> $<TARGET_FILE_DIR:${PROJECT_NAME}>
        )

    # NAP core 
    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:napcore> $<TARGET_FILE_DIR:${PROJECT_NAME}>
        )

    # RTTR
    find_rttr()
    copy_files_to_target_file_dir($<TARGET_FILE:RTTR::Core>)

    # Python
    if(NAP_ENABLE_PYTHON)
        file(GLOB PYTHON_DLLS ${THIRDPARTY_DIR}/python/msvc/x86_64/*.dll)
        copy_files_to_target_file_dir(${PYTHON_DLLS})
    endif()

    ########################################################################

endif()               

# Package into release build

# Package Qt into release
package_qt()

set(NAPKIN_PACKAGED_BUILD_TYPE Release)

if(MSVC)
    # On Windows dump the whole napkin directory from packaging_bin
    # into the release as it contains the required DLLs (as we loaded them in 
    # there for running Napkin against source from its own directory)
    install(DIRECTORY ${NAP_ROOT}/packaging_bin/Release-${ARCH}/napkin
            DESTINATION tools/
            PATTERN "*.pdb" EXCLUDE)

    if(PACKAGE_PDBS)
        install(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> DESTINATION tools/napkin)
    endif()

else()
    # Deploy Napkin binary into release on *nix
    install(TARGETS ${PROJECT_NAME}
            DESTINATION tools/napkin
            CONFIGURATIONS Release)
endif()

if(APPLE)
    # Remove thirdparty from framework release at install time
    macos_remove_rpaths_from_object_at_install_time(${CMAKE_INSTALL_PREFIX}/tools/napkin/napkin
                                                    ${THIRDPARTY_DIR}  
                                                    Release)
endif()   

install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/resources DESTINATION tools/napkin/)
module_list_to_deep_dependencies_cmake("${NAP_MODULES_DEPENDENCIES}")

set(NAPKIN_QT_INSTALL_FRAMEWORKS QtCore QtGui QtWidgets QtOpenGL)

set(PATH_TO_THIRDPARTY "../../thirdparty")
if(APPLE)
    list(APPEND NAPKIN_QT_INSTALL_FRAMEWORKS QtDBus)

    list(APPEND DEEP_DEPENDENT_RPATHS ${PATH_TO_THIRDPARTY}/Qt/lib)
    list(APPEND DEEP_DEPENDENT_RPATHS lib)

    foreach(MODULECONFIG Release)
        set_single_config_installed_rpath_on_macos_object_for_dependent_modules(${MODULECONFIG} 
                                                                                "${NAP_MODULES_DEPENDENCIES}" 
                                                                                ${CMAKE_INSTALL_PREFIX}/tools/napkin/napkin
                                                                                "../.."
                                                                                "${DEEP_DEPENDENT_RPATHS}")
    endforeach()

    # Install platform plugin alongside Napkin as it doesn't appear to be able to sourced
    # remotely (from thirdparty)
    install(DIRECTORY ${CMAKE_INSTALL_PREFIX}/thirdparty/Qt/plugins
            DESTINATION tools/napkin/
            CONFIGURATIONS Release
            )

    # Update paths to Qt frameworks in Napkin. Using explicit paths in an attempt to avoid loading
    # any installed Qt library.
    macos_replace_qt_framework_links_install_time("${NAPKIN_QT_INSTALL_FRAMEWORKS}" 
                                                  UNUSED
                                                  ${CMAKE_INSTALL_PREFIX}/tools/napkin/napkin
                                                  "@rpath"
                                                  )

elseif(UNIX)
    if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
        list(APPEND DEEP_DEPENDENT_RPATHS ${PATH_TO_THIRDPARTY}/Qt/lib)
        list(APPEND DEEP_DEPENDENT_RPATHS lib)

        set_installed_rpath_on_linux_object_for_dependent_modules("${NAP_MODULES_DEPENDENCIES}" 
                                                                  ${PROJECT_NAME} 
                                                                  "../.." 
                                                                  "${DEEP_DEPENDENT_RPATHS}"
                                                                  )

        # Install platform plugin alongside Napkin as it doesn't appear to be able to sourced
        # remotely (from thirdparty)
        install(FILES ${CMAKE_INSTALL_PREFIX}/thirdparty/Qt/plugins/platforms/libqxcb.so
                DESTINATION tools/napkin/platforms/
                PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
                )
    endif()
endif()
