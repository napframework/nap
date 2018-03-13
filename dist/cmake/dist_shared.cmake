# Set a default build type if none was specified (single-configuration generators only, ie. Linux)
macro(set_default_build_type)
    if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
        message(STATUS "Setting build type to 'Debug' as none was specified.")
        set(CMAKE_BUILD_TYPE "Debug")
    endif()
endmacro()

# Add the project module (if it exists) into the project
macro(add_project_module)
    if(EXISTS ${CMAKE_SOURCE_DIR}/module/)
        message("Found project module in ${CMAKE_SOURCE_DIR}/module/")
        set(MODULE_INTO_PROJ TRUE)
        set(IMPORTING_PROJECT_MODULE TRUE)
        add_subdirectory(${CMAKE_SOURCE_DIR}/module/ project_module_build/)
        unset(MODULE_INTO_PROJ)
        unset(IMPORTING_PROJECT_MODULE)
        target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/module/src)
        target_link_libraries(${PROJECT_NAME} mod_${PROJECT_NAME})

        # On Windows copy over module DLLs post-build
        if(WIN32)
            add_custom_command(
                TARGET ${PROJECT_NAME}
                POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:mod_${PROJECT_NAME}> $<TARGET_FILE_DIR:${PROJECT_NAME}>/
            )       
        endif()
    endif()
endmacro()

# Generic way to import each module for different configurations.  Included is a primitive mechanism for 
# extra per-module cmake logic.
macro(find_nap_module MODULE_NAME)
    if (EXISTS ${NAP_ROOT}/usermodules/${MODULE_NAME}/)
        message("Module is user module: ${MODULE_NAME}")
        set(MODULE_INTO_PROJ TRUE)
        add_subdirectory(${NAP_ROOT}/usermodules/${MODULE_NAME} usermodules/${MODULE_NAME})
        unset(MODULE_INTO_PROJ)

        # On Windows copy over module DLLs post-build
        if(WIN32)
            add_custom_command(
                TARGET ${PROJECT_NAME}
                POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${MODULE_NAME}> $<TARGET_FILE_DIR:${PROJECT_NAME}>/
            )       
        endif()
    elseif (EXISTS ${NAP_ROOT}/modules/${NAP_MODULE}/)
        if(NOT TARGET ${NAP_MODULE})
            add_library(${MODULE_NAME} INTERFACE)

            message(STATUS "Adding library path for ${MODULE_NAME}")
            if (WIN32)
                set(${MODULE_NAME}_DEBUG_LIB ${NAP_ROOT}/modules/${MODULE_NAME}/lib/Debug/${MODULE_NAME}.lib)
                set(${MODULE_NAME}_RELEASE_LIB ${NAP_ROOT}/modules/${MODULE_NAME}/lib/Release/${MODULE_NAME}.lib)
            elseif (APPLE)
                set(${MODULE_NAME}_RELEASE_LIB ${NAP_ROOT}/modules/${MODULE_NAME}/lib/Release/lib${MODULE_NAME}.dylib)
                set(${MODULE_NAME}_DEBUG_LIB ${NAP_ROOT}/modules/${MODULE_NAME}/lib/Debug/lib${MODULE_NAME}.dylib)
            elseif (UNIX)
                set(${MODULE_NAME}_RELEASE_LIB ${NAP_ROOT}/modules/${MODULE_NAME}/lib/Release/lib${MODULE_NAME}.so)
                set(${MODULE_NAME}_DEBUG_LIB ${NAP_ROOT}/modules/${MODULE_NAME}/lib/Debug/lib${MODULE_NAME}.so)
            endif()

            target_link_libraries(${MODULE_NAME} INTERFACE debug ${${MODULE_NAME}_DEBUG_LIB})
            target_link_libraries(${MODULE_NAME} INTERFACE optimized ${${MODULE_NAME}_RELEASE_LIB})
            file(GLOB_RECURSE module_headers ${NAP_ROOT}/modules/${NAP_MODULE}/include/*.h)
            target_sources(${MODULE_NAME} INTERFACE ${module_headers})
            source_group(Modules\\${MODULE_NAME} FILES ${module_headers})
        endif(NOT TARGET ${NAP_MODULE})

        # Add module includes
        if(NOT INSTALLING_MODULE_FOR_NAPKIN)
            message(STATUS "Adding include for ${NAP_MODULE}")
            target_include_directories(${PROJECT_NAME} PUBLIC ${NAP_ROOT}/modules/${NAP_MODULE}/include/)
        endif()

        # On macOS & Linux install module into packaged project
        if (NOT WIN32)
            install(FILES ${${MODULE_NAME}_RELEASE_LIB} DESTINATION lib CONFIGURATIONS Release)    
            # On Linux set our modules use their directory for RPATH
            if(NOT APPLE)
                install(CODE "message(\"Setting RPATH on ${CMAKE_INSTALL_PREFIX}/lib/lib${MODULE_NAME}.so\")
                              execute_process(COMMAND patchelf 
                                                      --set-rpath 
                                                      $ORIGIN/.
                                                      ${CMAKE_INSTALL_PREFIX}/lib/lib${MODULE_NAME}.so)")
            endif()
        endif()

        # Bring in any additional module requirements
        # TODO make sure we're running this for our source modules
        set(MODULE_EXTRA_CMAKE_PATH ${NAP_ROOT}/modules/${MODULE_NAME}/moduleExtra.cmake)
        if (EXISTS ${MODULE_EXTRA_CMAKE_PATH})
            include (${MODULE_EXTRA_CMAKE_PATH})
        endif()

        if(WIN32)
            # Copy over module DLLs post-build
            add_custom_command(
                TARGET ${PROJECT_NAME}
                POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy ${NAP_ROOT}/modules/${MODULE_NAME}/lib/$<CONFIG>/${MODULE_NAME}.dll $<TARGET_FILE_DIR:${PROJECT_NAME}>/
            )
        endif()        
    elseif(NOT TARGET ${MODULE_NAME})
        message(FATAL_ERROR "Could not locate module '${MODULE_NAME}'")    
    endif()    
endmacro()

# Export FBX to mesh
macro(export_fbx SRCDIR)
    # Set the binary name
    set(TOOLS_DIR ${NAP_ROOT}/tools)
    set(FBXCONVERTER_BIN ${TOOLS_DIR}/platform/fbxconverter)

    # Do the export
    add_custom_command(TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND "${FBXCONVERTER_BIN}" -o ${SRCDIR} "${SRCDIR}/*.fbx"
        COMMENT "Exporting FBX in '${SRCDIR}'")
endmacro()

# Change our project output directories
macro(set_output_directories)
    if (MSVC OR APPLE)
        # Loop over each configuration for multi-configuration systems
        foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
            set(BUILD_CONF ${CMAKE_CXX_COMPILER_ID}-${ARCH}-${OUTPUTCONFIG})
            if (PROJECT_PACKAGE_BIN_DIR)
                set(BIN_DIR ${CMAKE_SOURCE_DIR}/${PROJECT_PACKAGE_BIN_DIR}/)
            else()
                set(BIN_DIR ${CMAKE_SOURCE_DIR}/bin/${BUILD_CONF}/)
            endif()
            string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
            set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${BIN_DIR})
        endforeach()
    else()
        # Single built type, for Linux
        set(BUILD_CONF ${CMAKE_CXX_COMPILER_ID}-${CMAKE_BUILD_TYPE}-${ARCH})
        if (PROJECT_PACKAGE_BIN_DIR)
            set(BIN_DIR ${CMAKE_SOURCE_DIR}/${PROJECT_PACKAGE_BIN_DIR}/)
        else()
            set(BIN_DIR ${CMAKE_SOURCE_DIR}/bin/${BUILD_CONF}/)
        endif()
        set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR})
    endif()
endmacro()

# Change our module output directories
macro(set_module_output_directories)
    if (MSVC OR APPLE)
        # Loop over each configuration for multi-configuration systems
        foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
            set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/${OUTPUTCONFIG}/)
            string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
            set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${LIB_DIR})
            set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${LIB_DIR})
        endforeach()
    else()
        # Single built type, for Linux
        set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/${CMAKE_BUILD_TYPE}/)
        set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${LIB_DIR})
    endif()
endmacro()

# macOS: At install time replace a Qt lib install names in specified file
macro(macos_replace_qt_framework_links_install_time FRAMEWORKS LIB_NAME FILEPATH PATH_PREFIX)
    foreach(QT_LINK_FRAMEWORK ${FRAMEWORKS})
        if(NOT ${QT_LINK_FRAMEWORK} STREQUAL ${LIB_NAME})
            macos_replace_single_install_name_link_install_time(${QT_LINK_FRAMEWORK} ${FILEPATH} ${PATH_PREFIX})
        endif()
    endforeach()    
endmacro()

# macOS: At install time replace a single lib install name in the provided file
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

# macOS: At post-build replace a Qt lib install names in specified file
macro(macos_replace_qt_framework_links FRAMEWORKS SRC_FILEPATH FILEPATH PATH_PREFIX)
    foreach(QT_LINK_FRAMEWORK ${FRAMEWORKS})
        macos_replace_single_install_name_link(${QT_LINK_FRAMEWORK}
                                               ${SRC_FILEPATH}
                                               ${FILEPATH}
                                               ${PATH_PREFIX})
    endforeach()    
endmacro()

# macOS: At post-build replace a single lib install name in the provided file
macro(macos_replace_single_install_name_link REPLACE_LIB_NAME SRC_FILEPATH FILEPATH PATH_PREFIX)
    execute_process(COMMAND sh -c "otool -L ${SRC_FILEPATH} | grep ${REPLACE_LIB_NAME} | awk -F'(' '{print $1}'"
                    OUTPUT_VARIABLE REPLACE_INSTALL_NAME)
    if(NOT ${REPLACE_INSTALL_NAME} STREQUAL "")
        string(STRIP ${REPLACE_INSTALL_NAME} REPLACE_INSTALL_NAME)
        add_custom_command(TARGET ${PROJECT_NAME}
                           POST_BUILD
                           COMMAND ${CMAKE_INSTALL_NAME_TOOL} 
                                   -change 
                                   ${REPLACE_INSTALL_NAME}
                                   ${PATH_PREFIX}/${REPLACE_LIB_NAME}
                                   ${FILEPATH}
                           )
    endif()
endmacro()

# Copy files to project binary dir
macro(copy_files_to_bin)
    foreach(F ${ARGN})
        add_custom_command(TARGET ${PROJECT_NAME}
                           POST_BUILD
                           COMMAND ${CMAKE_COMMAND} -E copy "${F}" "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
                           COMMENT "Copy ${F} -> $<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endforeach()
endmacro()

# Let find_python find our prepackaged Python in thirdparty
macro(find_python_in_thirdparty)   
    set(PYTHONLIBS_FOUND 1)
    set(PYTHON_PREFIX ${THIRDPARTY_DIR}/python)
    if(UNIX)
        set(PYTHON_LIBRARIES ${PYTHON_PREFIX}/lib/libpython3.6m${CMAKE_SHARED_LIBRARY_SUFFIX})
        set(PYTHON_INCLUDE_DIRS ${PYTHON_PREFIX}/include/python3.6m)
    else()
        set(PYTHON_LIBRARIES ${PYTHON_PREFIX}/libs/python36.lib)
        set(PYTHON_INCLUDE_DIRS ${PYTHON_PREFIX}/include)
    endif()
endmacro()

# Windows: Copy Python DLLs post-build
macro(win64_copy_python_dlls_postbuild)
    file(GLOB PYTHON_DLLS ${THIRDPARTY_DIR}/python/*.dll)
    foreach(PYTHON_DLL ${PYTHON_DLLS})
        add_custom_command(TARGET ${PROJECT_NAME}
                           POST_BUILD
                           COMMAND ${CMAKE_COMMAND} -E copy ${PYTHON_DLL} $<TARGET_FILE_DIR:${PROJECT_NAME}>
                           )
    endforeach()
endmacro()

# Windows: Copy modules post-build
macro(win64_copy_python_modules_postbuild)
    add_custom_command(TARGET ${PROJECT_NAME}
                       POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy ${THIRDPARTY_DIR}/python/python36.zip $<TARGET_FILE_DIR:${PROJECT_NAME}>
                       )
endmacro()

# macOS: Ensure our specified file has provided RPATH in post-build
macro(macos_add_rpath_to_module_post_build TARGET_NAME FILENAME PATH_TO_ADD)
    add_custom_command(TARGET ${TARGET_NAME}
                       POST_BUILD
                       COMMAND sh -c \"${CMAKE_INSTALL_NAME_TOOL} -add_rpath ${PATH_TO_ADD} ${FILENAME} 2>/dev/null\;exit 0\"
                       )
endmacro()

# Populate modules list from project.json into var NAP_MODULES
macro(project_json_to_cmake)
    # Use configure_file to result in changes in project.json triggering reconfigure.  Appears to be best current approach.
    configure_file(${CMAKE_SOURCE_DIR}/project.json ProjectJsonTriggerDummy.json)
    execute_process(COMMAND ${CMAKE_COMMAND} -E remove ProjectJsonTriggerDummy.json
                    ERROR_QUIET)

    # Clear any system Python path settings
    unset(ENV{PYTHONHOME})
    unset(ENV{PYTHONPATH})

    # Parse our project.json and import it
    if(WIN32)
        set(PYTHON_BIN ${THIRDPARTY_DIR}/python/python)
    elseif(UNIX)
        set(PYTHON_BIN ${THIRDPARTY_DIR}/python/bin/python3)
    endif()
    execute_process(COMMAND ${PYTHON_BIN} ${NAP_ROOT}/tools/platform/projectInfoParseToCMake.py ${CMAKE_CURRENT_SOURCE_DIR})
    include(cached_project_json.cmake)    
endmacro()