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
            if (PROJECT_PACKAGE_BIN_DIR)
                set(BIN_DIR ${CMAKE_SOURCE_DIR}/${PROJECT_PACKAGE_BIN_DIR}/)
            else()
                set(BIN_DIR ${CMAKE_SOURCE_DIR}/bin/${OUTPUTCONFIG}/)
            endif()
            string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
            set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${BIN_DIR})
        endforeach()
    else()
        # Single built type, for Linux
        if (PROJECT_PACKAGE_BIN_DIR)
            set(BIN_DIR ${CMAKE_SOURCE_DIR}/${PROJECT_PACKAGE_BIN_DIR}/)
        else()
            set(BIN_DIR ${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}/)
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
                                          ERROR_QUIET
                                          RESULT_VARIABLE EXIT_CODE)
                          if(NOT \${EXIT_CODE} EQUAL 0)
                              message(FATAL_ERROR \"Failed to replace install name on ${FILEPATH}\")
                          endif()
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
                    OUTPUT_VARIABLE REPLACE_INSTALL_NAME
                    RESULT_VARIABLE EXIT_CODE)
    if(NOT ${EXIT_CODE} EQUAL 0)
        message(FATAL_ERROR "Failed to replace install name on ${SRC_FILEPATH} using otool")
    endif()
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
    set(PYTHON_PREFIX ${THIRDPARTY_DIR}/python)
    if(UNIX)
        set(PYTHON_EXECUTABLE ${PYTHON_PREFIX}/bin/python3)
    else()
        set(PYTHON_EXECUTABLE ${PYTHON_PREFIX}/python.exe)
    endif()
endmacro()

# Windows: Post-build copy Python DLLs into project bin output
# FOR_NAPKIN: Whether copying for Napkin (changing output dir)
macro(win64_copy_python_dlls_postbuild FOR_NAPKIN)
    if(${FOR_NAPKIN})
        set(PYDLL_PATH_SUFFIX "../napkin/")
    else()
        set(PYDLL_PATH_SUFFIX "")
    endif()
    file(GLOB PYTHON_DLLS ${THIRDPARTY_DIR}/python/*.dll)
    foreach(PYTHON_DLL ${PYTHON_DLLS})
        add_custom_command(TARGET ${PROJECT_NAME}
                           POST_BUILD
                           COMMAND ${CMAKE_COMMAND} -E copy ${PYTHON_DLL} $<TARGET_FILE_DIR:${PROJECT_NAME}>/${PYDLL_PATH_SUFFIX}
                           )
    endforeach()
endmacro()

# Windows: Post-build copy Python modules into project bin output
# FOR_NAPKIN_BUILD_OUTPUT: Whether copying for Napkin (changing output dir)
macro(win64_copy_python_modules_postbuild FOR_NAPKIN_BUILD_OUTPUT)
    if(${FOR_NAPKIN_BUILD_OUTPUT})
        set(PYMOD_PATH_SUFFIX "../napkin/")
    else()
        set(PYMOD_PATH_SUFFIX "")
    endif()
    add_custom_command(TARGET ${PROJECT_NAME}
                       POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy ${THIRDPARTY_DIR}/python/python36.zip $<TARGET_FILE_DIR:${PROJECT_NAME}>/${PYMOD_PATH_SUFFIX}
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

    execute_process(COMMAND ${PYTHON_BIN} ${NAP_ROOT}/tools/platform/module_info_parse_to_cmake.py ${DIRECTORY} ${NAP_ROOT}
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

# Linux: At install time add an additional RPATH onto a file
# FILEPATH: The file to update
# EXTRA_RPATH: The new RPATH entry
macro(linux_append_rpath_at_install_time FILEPATH EXTRA_RPATH)
    install(CODE "if(EXISTS ${FILEPATH})
                      execute_process(COMMAND sh -c \"patchelf --print-rpath ${FILEPATH}\"
                                      OUTPUT_VARIABLE ORIG_RPATH
                                      RESULT_VARIABLE EXIT_CODE)
                      if(NOT \${EXIT_CODE} EQUAL 0)
                          message(FATAL_ERROR \"Failed to fetch RPATH from ${FILEPATH} using patchelf. Is patchelf installed?\")
                      endif()
                      set(NEW_RPATH \"\")
                      if(NOT \${ORIG_RPATH} STREQUAL \"\")
                          string(STRIP \${ORIG_RPATH} NEW_RPATH)
                          string(APPEND NEW_RPATH \":\")
                      endif()
                      string(APPEND NEW_RPATH \${EXTRA_RPATH})
                      execute_process(COMMAND patchelf
                                              --set-rpath
                                              \"\${NEW_RPATH}\"
                                              \${FILEPATH}
                                      ERROR_QUIET
                                      RESULT_VARIABLE EXIT_CODE)
                      if(NOT \${EXIT_CODE} EQUAL 0)
                          message(FATAL_ERROR \"Failed to set RPATH on ${FILEPATH} using patchelf\")
                      endif()
                  endif()
                  ")
endmacro()

# Deploy appropriate path mapping to cache location alongside binary, and install
# in packaged app
# PROJECT_DIR: The project directory
function(deploy_single_path_mapping PROJECT_DIR)
    # Deploy to build output
    find_path_mapping(${NAP_ROOT}/tools/platform/path_mappings ${PROJECT_DIR} framework_release)
    if(DEFINED PATH_MAPPING_FILE)
        message(VERBOSE "Using path mapping ${PATH_MAPPING_FILE}")
    else()
        message(FATAL_ERROR "Couldn't locate path mapping")
    endif()

    if(APPLE OR WIN32)
        # Multi build-type systems
        set(DEST_CACHE_PATH $<TARGET_PROPERTY:${PROJECT_NAME},RUNTIME_OUTPUT_DIRECTORY_$<UPPER_CASE:$<CONFIG>>>/cache/path_mapping.json)
    else()
        # Single build-type systems
        set(DEST_CACHE_PATH $<TARGET_PROPERTY:${PROJECT_NAME},RUNTIME_OUTPUT_DIRECTORY>/cache/path_mapping.json)
    endif()
    add_custom_command(TARGET ${PROJECT_NAME}
                       POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PATH_MAPPING_FILE} ${DEST_CACHE_PATH}
                       COMMENT "Deploying path mapping to bin")

    set(PROJ_DEST_CACHE_PATH ${PROJECT_DIR}/cache/path_mapping.json)
    if(NOT (WIN32 AND NAP_PACKAGED_APP_BUILD))
        add_custom_command(TARGET ${PROJECT_NAME}
                           POST_BUILD
                           COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PATH_MAPPING_FILE} ${PROJ_DEST_CACHE_PATH}
                           COMMENT "Deploying path mapping to project directory")
    endif()

    # Install into packaged app
    find_path_mapping(${NAP_ROOT}/tools/platform/path_mappings ${PROJECT_DIR} packaged_app)
    if(DEFINED PATH_MAPPING_FILE)
        message(VERBOSE "Using path mapping ${PATH_MAPPING_FILE}")
    else()
        message(FATAL_ERROR "Couldn't locate path mapping")
    endif()
    install(FILES ${PATH_MAPPING_FILE} DESTINATION cache RENAME path_mapping.json)
    if(WIN32 AND NAP_PACKAGED_APP_BUILD)
        set(PROJ_DEST_CACHE_PATH ${CMAKE_INSTALL_PREFIX}/cache/path_mapping.json)
        add_custom_command(TARGET ${PROJECT_NAME}
                           POST_BUILD
                           COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PATH_MAPPING_FILE} ${PROJ_DEST_CACHE_PATH}
                           COMMENT "Deploying Win64 path mapping to packaged app project directory")
    endif()
endfunction()
