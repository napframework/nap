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

        # Add includes
        target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/module/src)

        # Link
        target_link_libraries(${PROJECT_NAME} mod_${PROJECT_NAME})

        # On Windows copy over module DLLs post-build
        if(WIN32)
            add_custom_command(
                TARGET ${PROJECT_NAME}
                POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:mod_${PROJECT_NAME}> $<TARGET_FILE_DIR:${PROJECT_NAME}>/
                COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/module/module.json $<TARGET_FILE_DIR:${PROJECT_NAME}>/mod_${PROJECT_NAME}.json
            )       
        endif()
    endif()
endmacro()

# Generic way to import each module for different configurations.  Included is a fairly simple mechanism for 
# extra per-module CMake logic, to be refined.
macro(find_nap_module MODULE_NAME)
    if (EXISTS ${NAP_ROOT}/user_modules/${MODULE_NAME}/)
        message(STATUS "Module is user module: ${MODULE_NAME}")
        set(MODULE_INTO_PROJ TRUE)
        add_subdirectory(${NAP_ROOT}/user_modules/${MODULE_NAME} user_modules/${MODULE_NAME})
        unset(MODULE_INTO_PROJ)

        # On Windows copy over module DLLs post-build
        if(WIN32)
            add_custom_command(
                TARGET ${PROJECT_NAME}
                POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${MODULE_NAME}> $<TARGET_FILE_DIR:${PROJECT_NAME}>/
                COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE_DIR:${MODULE_NAME}>/${MODULE_NAME}.json $<TARGET_FILE_DIR:${PROJECT_NAME}>/
                )       
        endif()
    elseif (EXISTS ${NAP_ROOT}/modules/${NAP_MODULE}/)
        if(NOT TARGET ${NAP_MODULE})
            add_library(${MODULE_NAME} INTERFACE)

            message(STATUS "Adding library path for ${MODULE_NAME}")
            if (WIN32)
                set(${MODULE_NAME}_DEBUG_LIB ${NAP_ROOT}/modules/${MODULE_NAME}/lib/Debug/${MODULE_NAME}.lib)
                set(${MODULE_NAME}_RELEASE_LIB ${NAP_ROOT}/modules/${MODULE_NAME}/lib/Release/${MODULE_NAME}.lib)
                set(${MODULE_NAME}_MODULE_JSON ${NAP_ROOT}/modules/${MODULE_NAME}/lib/Release/${MODULE_NAME}.json)
            elseif (APPLE)
                set(${MODULE_NAME}_RELEASE_LIB ${NAP_ROOT}/modules/${MODULE_NAME}/lib/Release/lib${MODULE_NAME}.dylib)
                set(${MODULE_NAME}_DEBUG_LIB ${NAP_ROOT}/modules/${MODULE_NAME}/lib/Debug/lib${MODULE_NAME}.dylib)
                set(${MODULE_NAME}_MODULE_JSON ${NAP_ROOT}/modules/${MODULE_NAME}/lib/Release/lib${MODULE_NAME}.json)
            elseif (UNIX)
                set(${MODULE_NAME}_RELEASE_LIB ${NAP_ROOT}/modules/${MODULE_NAME}/lib/Release/lib${MODULE_NAME}.so)
                set(${MODULE_NAME}_DEBUG_LIB ${NAP_ROOT}/modules/${MODULE_NAME}/lib/Debug/lib${MODULE_NAME}.so)
                set(${MODULE_NAME}_MODULE_JSON ${NAP_ROOT}/modules/${MODULE_NAME}/lib/Release/lib${MODULE_NAME}.json)
            endif()

            target_link_libraries(${MODULE_NAME} INTERFACE debug ${${MODULE_NAME}_DEBUG_LIB})
            target_link_libraries(${MODULE_NAME} INTERFACE optimized ${${MODULE_NAME}_RELEASE_LIB})
            set(MODULE_INCLUDE_ROOT ${NAP_ROOT}/modules/${NAP_MODULE}/include)
            file(GLOB_RECURSE module_headers ${MODULE_INCLUDE_ROOT}/*.h ${MODULE_INCLUDE_ROOT}/*.hpp)
            target_sources(${MODULE_NAME} INTERFACE ${module_headers})

            # Build source groups for our headers maintaining their folder structure
            create_hierarchical_source_groups_for_files("${module_headers}" ${MODULE_INCLUDE_ROOT} "Modules\\${MODULE_NAME}")
        endif(NOT TARGET ${NAP_MODULE})

        # Add module includes
        if(NOT INSTALLING_MODULE_FOR_NAPKIN)
            message(STATUS "Adding include for ${NAP_MODULE}")
            target_include_directories(${PROJECT_NAME} PUBLIC ${NAP_ROOT}/modules/${NAP_MODULE}/include/)
        endif()

        # On macOS & Linux install module into packaged project
        if (NOT WIN32)
            install(FILES ${${MODULE_NAME}_RELEASE_LIB} DESTINATION lib CONFIGURATIONS Release)
            install(FILES ${${MODULE_NAME}_MODULE_JSON} DESTINATION lib CONFIGURATIONS Release)
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
        set(MODULE_EXTRA_CMAKE_PATH ${NAP_ROOT}/modules/${MODULE_NAME}/module_extra.cmake)
        if (EXISTS ${MODULE_EXTRA_CMAKE_PATH})
            include (${MODULE_EXTRA_CMAKE_PATH})
        endif()

        if(WIN32)
            # Copy over module DLLs post-build
            add_custom_command(
                TARGET ${PROJECT_NAME}
                POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different ${NAP_ROOT}/modules/${MODULE_NAME}/lib/$<CONFIG>/${MODULE_NAME}.dll $<TARGET_FILE_DIR:${PROJECT_NAME}>/
                COMMAND ${CMAKE_COMMAND} -E copy_if_different ${NAP_ROOT}/modules/${MODULE_NAME}/lib/$<CONFIG>/${MODULE_NAME}.json $<TARGET_FILE_DIR:${PROJECT_NAME}>/                
                )
        endif()        
    elseif(NOT TARGET ${MODULE_NAME})
        message(FATAL_ERROR "Could not locate module '${MODULE_NAME}'")    
    endif()    
endmacro()

# Export all FBXs in directory to meshes
# SRCDIR: The directory to import/export in
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

# Setup our project output directories
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

# Setup our module output directories
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

# macOS: Post-build replace Qt framework install names in specified file with new paths built
# from a path prefix and a full framework lib name mapped from another file
# FRAMEWORKS: Qt framework names to replace
# SRC_FILEPATH: The file used to obtain the full framework library name
# FILEPATH: The file to update
# PATH_PREFIX: The new path prefix for the framework
macro(macos_replace_qt_framework_links FRAMEWORKS SRC_FILEPATH FILEPATH PATH_PREFIX)
    foreach(QT_LINK_FRAMEWORK ${FRAMEWORKS})
        macos_replace_single_install_name_link(${QT_LINK_FRAMEWORK}
                                               ${SRC_FILEPATH}
                                               ${FILEPATH}
                                               ${PATH_PREFIX})
    endforeach()    
endmacro()

# macOS: Post-build replace a single lib install name in the specified file with new paths built
# from a path prefix and a full lib name mapped from another file
# REPLACE_LIB_NAME: Library install name to replace
# SRC_FILEPATH: The file used to obtain the full framework library name
# FILEPATH: The file to update
# PATH_PREFIX: The new path prefix for the framework
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
# ARGN: Files to copy
macro(copy_files_to_bin)
    foreach(F ${ARGN})
        add_custom_command(TARGET ${PROJECT_NAME}
                           POST_BUILD
                           COMMAND ${CMAKE_COMMAND} -E copy "${F}" "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
                           COMMENT "Copy ${F} -> $<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endforeach()
endmacro()

# Set our environment so that find_package finds our pre-packaged Python in thirdparty
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

# Windows: Post-build copy Python DLLs into project bin output
macro(win64_copy_python_dlls_postbuild)
    file(GLOB PYTHON_DLLS ${THIRDPARTY_DIR}/python/*.dll)
    foreach(PYTHON_DLL ${PYTHON_DLLS})
        add_custom_command(TARGET ${PROJECT_NAME}
                           POST_BUILD
                           COMMAND ${CMAKE_COMMAND} -E copy ${PYTHON_DLL} $<TARGET_FILE_DIR:${PROJECT_NAME}>
                           )
    endforeach()
endmacro()

# Windows: Post-build copy Python modules into project bin output
macro(win64_copy_python_modules_postbuild)
    add_custom_command(TARGET ${PROJECT_NAME}
                       POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy ${THIRDPARTY_DIR}/python/python36.zip $<TARGET_FILE_DIR:${PROJECT_NAME}>
                       )
endmacro()

# macOS: Post-build ensure our specified file has provided RPATH
# TARGET_NAME: The target to pin the post-build custom command to
# FILENAME: File to add ensure has the RPATH
# PATH_TO_ADD: The path to add
macro(macos_add_rpath_to_module_post_build TARGET_NAME FILENAME PATH_TO_ADD)
    add_custom_command(TARGET ${TARGET_NAME}
                       POST_BUILD
                       COMMAND sh -c \"${CMAKE_INSTALL_NAME_TOOL} -add_rpath ${PATH_TO_ADD} ${FILENAME} 2>/dev/null\;exit 0\"
                       )
endmacro()

# Populate modules list from project.json into var NAP_MODULES
macro(project_json_to_cmake)
    # Use configure_file to result in changes in project.json triggering reconfigure.  Appears to be best current approach.
    configure_file(${CMAKE_SOURCE_DIR}/project.json project_json_trigger_dummy.json)
    execute_process(COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_CACHEFILE_DIR}/project_json_trigger_dummy.json
                    ERROR_QUIET)    

    # Clear any system Python path settings
    unset(ENV{PYTHONHOME})
    unset(ENV{PYTHONPATH})

    # Parse our project.json and import it
    if(WIN32)
        set(PYTHON_BIN ${THIRDPARTY_DIR}/python/python.exe)
    elseif(UNIX)
        set(PYTHON_BIN ${THIRDPARTY_DIR}/python/bin/python3)
    endif()
    if(NOT EXISTS ${PYTHON_BIN})
        message(FATAL_ERROR "Python not found at ${PYTHON_BIN}")
    endif()

    execute_process(COMMAND ${PYTHON_BIN} ${NAP_ROOT}/tools/platform/project_info_parse_to_cmake.py ${CMAKE_CURRENT_SOURCE_DIR}
                    RESULT_VARIABLE EXIT_CODE
                    )
    if(NOT ${EXIT_CODE} EQUAL 0)
        message(FATAL_ERROR "Could not parse modules from project.json (${EXIT_CODE})")
    endif()
    include(cached_project_json.cmake)
endmacro()

# Get our NAP modules dependencies from module.json, populating into DEPENDENT_NAP_MODULES
macro(module_json_to_cmake)
    module_json_in_directory_to_cmake(${CMAKE_CURRENT_SOURCE_DIR})
endmacro()

# Get our NAP modules dependencies from module.json in specified directory, populating into DEPENDENT_NAP_MODULES
macro(module_json_in_directory_to_cmake DIRECTORY)
    # Use configure_file to result in changes in module.json triggering reconfigure.  Appears to be best current approach.
    configure_file(${DIRECTORY}/module.json module_json_trigger_dummy.json)
    execute_process(COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_CACHEFILE_DIR}/module_json_trigger_dummy.json
                    ERROR_QUIET)

    # Clear any system Python path settings
    unset(ENV{PYTHONHOME})
    unset(ENV{PYTHONPATH})

    # Parse our module.json and import it
    if(WIN32)
        set(PYTHON_BIN ${THIRDPARTY_DIR}/python/python.exe)
    elseif(UNIX)
        set(PYTHON_BIN ${THIRDPARTY_DIR}/python/bin/python3)
    endif()
    if(NOT EXISTS ${PYTHON_BIN})
        message(FATAL_ERROR "Python not found at ${PYTHON_BIN}")
    endif()

    execute_process(COMMAND ${PYTHON_BIN} ${NAP_ROOT}/tools/platform/module_info_parse_to_cmake.py ${DIRECTORY}
                    RESULT_VARIABLE EXIT_CODE
                    )
    if(NOT ${EXIT_CODE} EQUAL 0)
        message(FATAL_ERROR "Could not parse modules dependencies from module.json (${EXIT_CODE})")
    endif()
    include(${DIRECTORY}/cached_module_json.cmake)
    # message("${PROJECT_NAME} DEPENDENT_NAP_MODULES from module.json: ${DEPENDENT_NAP_MODULES}")
endmacro()

# Build source groups for input files maintaining their folder structure
# INPUT_FILES: Files to be added to source groups
# RELATIVE_TO_PATH: The root path against which our relative source group paths will be built
# GROUP_NAME_PREFIX: Any prefix that should be added onto the front of the group name
function(create_hierarchical_source_groups_for_files INPUT_FILES RELATIVE_TO_PATH GROUP_NAME_PREFIX)
    foreach(input_file ${INPUT_FILES})
        # Get the file's directory
        get_filename_component(THIS_FILE_DIR ${input_file} DIRECTORY)

        # Determine the file's directory path relative to the root
        file(RELATIVE_PATH RELATIVE_FILE_DIR ${RELATIVE_TO_PATH} ${THIS_FILE_DIR})

        # If it's in a subfolder..
        if(NOT ${RELATIVE_FILE_DIR} STREQUAL "")
            # Switch path separators
            string(REPLACE "/" "\\" RELATIVE_FILE_DIR ${RELATIVE_FILE_DIR})
            # Prepend path separator
            set(RELATIVE_FILE_DIR "\\${RELATIVE_FILE_DIR}")
        endif()

        # Add to source group
        source_group(${GROUP_NAME_PREFIX}${RELATIVE_FILE_DIR} FILES ${input_file})
    endforeach()
endfunction()

# Copy calling module's module.json to sit alongside module post-build
macro(copy_module_json_to_bin)
    set(DEST_FILENAME ${PROJECT_NAME}.json)
    if(UNIX)
        set(DEST_FILENAME lib${DEST_FILENAME})
    endif()
    
    if(APPLE)
        # macOS: Multi build type outputting to LIBRARY_OUTPUT_DIRECTORY
        add_custom_command(TARGET ${PROJECT_NAME}
                           POST_BUILD
                           COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/module.json" "$<TARGET_PROPERTY:${PROJECT_NAME},LIBRARY_OUTPUT_DIRECTORY_$<UPPER_CASE:$<CONFIG>>>/${DEST_FILENAME}"
                           COMMENT "Copying module.json for ${PROJECT_NAME} to ${DEST_FILENAME} in library output post-build")        
    elseif(UNIX)
        # Linux: Single build type outputting to LIBRARY_OUTPUT_DIRECTORY
        add_custom_command(TARGET ${PROJECT_NAME}
                           POST_BUILD
                           COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/module.json" "$<TARGET_PROPERTY:${PROJECT_NAME},LIBRARY_OUTPUT_DIRECTORY>/${DEST_FILENAME}"
                           COMMENT "Copying module.json for ${PROJECT_NAME} to ${DEST_FILENAME} in library output post-build")        

    else()
        # Win64: Multi build type outputting to RUNTIME_OUTPUT_DIRECTORY
        add_custom_command(TARGET ${PROJECT_NAME}
                           POST_BUILD
                           COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/module.json" "$<TARGET_PROPERTY:${PROJECT_NAME},RUNTIME_OUTPUT_DIRECTORY_$<UPPER_CASE:$<CONFIG>>>/${DEST_FILENAME}"
                           COMMENT "Copying module.json for ${PROJECT_NAME} to ${DEST_FILENAME} in library output post-build")        
    endif()
endmacro()

# For the provided top-level modules locate all dependencies and store in NAP_MODULES
# TOPLEVEL_MODULES: A list of top level modules
macro(fetch_module_dependencies TOPLEVEL_MODULES)
    set(NEW_MODULES "${TOPLEVEL_MODULES}")
    set(NAP_MODULES "")

    # Iterate until we stop go through a loop with no new dependencies
    while(NEW_MODULES)
        # Append our new modules to our output list
        list(APPEND NAP_MODULES "${NEW_MODULES}")

        # Set our modules to search on this time
        set(SEARCH_MODULES "${NEW_MODULES}")

        # Clear the new modules list, ready for populating
        set(NEW_MODULES "")

        # Fetch dependencies for our search modules in this iteration
        fetch_module_dependencies_for_modules("${SEARCH_MODULES}" "${NAP_MODULES}")
    endwhile()

    # message(STATUS "Total dependent modules for ${PROJECT_NAME}: ${NAP_MODULES}")
endmacro()

# For the provided modules for any immediate dependencies (ie. other NAP modules) which
# haven't already been found
# SEARCH_MODULES: The modules to check for dependencies on
# TOTAL_MODULES: The modules we already have as dependencies
macro(fetch_module_dependencies_for_modules SEARCH_MODULES TOTAL_MODULES)
    # Set the location for NAP framework modules
    set(NAP_MODULES_DIR ${NAP_ROOT}/modules/)

    # Set the user modules location
    set(USER_MODULES_DIR ${NAP_ROOT}/user_modules/)

    # Workaround for CMake not treating macro argument as proper list variable
    set(TOTAL_MODULES ${TOTAL_MODULES})

    # Loop for each search module
    foreach(SEARCH_MODULE ${SEARCH_MODULES})
        # Check for a NAP framework module
        if(EXISTS ${NAP_MODULES_DIR}/${SEARCH_MODULE})
            set(FOUND_PATH ${NAP_MODULES_DIR}/${SEARCH_MODULE})
        # Check for a user module
        elseif(EXISTS ${USER_MODULES_DIR}/${SEARCH_MODULE})
            set(FOUND_PATH ${USER_MODULES_DIR}/${SEARCH_MODULE})
        # Check for a project module
        elseif(${SEARCH_MODULE} STREQUAL mod_${PROJECT_NAME} AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/module)
            set(FOUND_PATH ${CMAKE_CURRENT_SOURCE_DIR}/module)
        else()
            message(FATAL_ERROR "Could not find module ${SEARCH_MODULE}")
        endif()

        # If we found the module directory but there's no module.json, fail
        if(NOT EXISTS ${FOUND_PATH}/module.json)
            message(FATAL_ERROR "Could not find module.json in ${FOUND_PATH}")
        endif()

        # Process the found module.json, making it available to CMake as DEPENDENT_NAP_MODULES
        module_json_in_directory_to_cmake(${FOUND_PATH})

        # Loop over each found dependency
        foreach(DEPENDENT_MODULE ${DEPENDENT_NAP_MODULES})
            # If we don't already have the module in our previously found dependencies or our newly found dependencies, 
            # add the new dependency
            if(NOT ${DEPENDENT_MODULE} IN_LIST NEW_MODULES AND NOT ${DEPENDENT_MODULE} IN_LIST TOTAL_MODULES)
                list(APPEND NEW_MODULES ${DEPENDENT_MODULE})
            endif()
        endforeach(DEPENDENT_MODULE ${DEPENDENT_NAP_MODULES})        
    endforeach(SEARCH_MODULE ${SEARCH_MODULES})
endmacro()
