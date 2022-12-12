# Bootstrap our build environment, setting up architecture, flags, policies, etc used across both NAP
# build contexts
macro(bootstrap_environment)
    # Enforce GCC on Linux for now (when doing packaging build at least)
    if(UNIX AND NOT APPLE)
        if(NOT NAP_BUILD_CONTEXT MATCHES "source" OR DEFINED NAP_PACKAGED_BUILD)
            if(NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
                message(FATAL_ERROR "NAP only currently supports GCC on Linux")
            endif()
        endif()
    endif()

    include(${NAP_ROOT}/cmake/targetarch.cmake)
    target_architecture(ARCH)

    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)

    if(WIN32)
        if(MSVC)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4244 /wd4305 /wd4996 /wd4267 /wd4018 /wd4251 /MP /bigobj")
            set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
            set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
            set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
            if(DEFINED INCLUDE_DEBUG_SYMBOLS AND INCLUDE_DEBUG_SYMBOLS)
                set(PACKAGE_PDBS ON)
            endif()
        else()
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse -Wa,-mbig-obj")
        endif()
    elseif(UNIX)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -Wno-format-security -Wno-switch -fvisibility=hidden")
        if(DEFINED INCLUDE_DEBUG_SYMBOLS AND NOT INCLUDE_DEBUG_SYMBOLS)
            if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
                # Verified for AppleClang, expected to also potentially work (at a later date) for Clang on Linux
                string(REPLACE "-g" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
            else()
                # Verified for GCC on Linux
                set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s")
            endif()
        endif()
    endif()

    # We don't actively support and work on macOS anymore.
    # This is not because we don't like their hardware, but because of the continuous tightening of restrictions of macOS,
    # Including aggressive gate-keeping, required app singing, forcing specific data structures, vague licence policies,
    # proprietary APIs, etc. It's simply not in line with our policies and what we stand for.
    # Feel free to continue support for macOS on your end.
    if(APPLE)
        message(DEPRECATION "macOS (as a target) is no longer in active development or supported")
        set(CMAKE_OSX_DEPLOYMENT_TARGET 10.15)
    endif()

    # We don't actively support and work on Python bindings anymore.
    if(NAP_ENABLE_PYTHON)
        message(DEPRECATION "Python bindings are no longer in active development or supported")
        add_definitions(-DNAP_ENABLE_PYTHON)
    endif()

    # Automatically link Qt executables to qtmain target on Windows.
    cmake_policy(SET CMP0020 NEW)

    # Ignore COMPILE_DEFINITIONS
    cmake_policy(SET CMP0043 NEW)

    # Allow modifying link targets created in other directories (needed for the way extra libraries are brought in from
    # module_extra.cmake
    cmake_policy(SET CMP0079 NEW)

    # Restrict to debug and release build types
    set(CMAKE_CONFIGURATION_TYPES "Debug;Release")

    # Add extra CMake find module path
    list(APPEND CMAKE_MODULE_PATH "${NAP_ROOT}/cmake/find_modules")

    if(APPLE)
        set(NAP_THIRDPARTY_PLATFORM_DIR "macos")
    elseif(MSVC)
        set(NAP_THIRDPARTY_PLATFORM_DIR "msvc")
    else()
        set(NAP_THIRDPARTY_PLATFORM_DIR "linux")
    endif()

    if(UNIX AND NOT APPLE)
        # Ensure we have patchelf on Linux, preventing silent failures
        ensure_patchelf_installed()

        # Check if we're building on raspbian
        check_raspbian_os(RASPBIAN)
    endif()
endmacro()

# Get our NAP modules dependencies from module.json, populating into DEPENDENT_NAP_MODULES
# APP_DIR: The app directory
# DEST_CONTEXT: Our desired build context, ie. one of 'source', 'framework_release' or 'packaged_app'
macro(find_path_mapping APP_DIR DEST_CONTEXT)
    unset(PATH_MAPPING_FILE)
    set(CHECK_PATH_LIST "")

    # Provide for greater specificity
    if(APPLE)
        list(APPEND CHECK_PATH_LIST macos)
    elseif(UNIX)
        list(APPEND CHECK_PATH_LIST linux)
    endif()

    # Fallback to Windows/Unix split
    if(WIN32)
        list(APPEND CHECK_PATH_LIST win64)
    else()
        list(APPEND CHECK_PATH_LIST unix)
    endif()

    # Check for custom app mappings
    foreach(CHECK_PATH ${CHECK_PATH_LIST})
        set(FULL_CHECK_PATH ${APP_DIR}/config/custom_path_mappings/${CHECK_PATH}/${DEST_CONTEXT}.json)
        if(EXISTS ${FULL_CHECK_PATH})
            set(PATH_MAPPING_FILE ${FULL_CHECK_PATH})
            message(STATUS "Using custom path mapping: ${FULL_CHECK_PATH}")
            break()
        endif()
    endforeach()

    # Otherwise find system mapping
    if(NOT DEFINED PATH_MAPPING_FILE)
        set(system_mappings ${NAP_ROOT}/tools/buildsystem/path_mappings)
        foreach(CHECK_PATH ${CHECK_PATH_LIST})
            if(NAP_BUILD_CONTEXT MATCHES "source")
                set(FULL_CHECK_PATH ${system_mappings}/${CHECK_PATH}/${DEST_CONTEXT}.json)
            else()
                set(FULL_CHECK_PATH ${system_mappings}/${DEST_CONTEXT}.json)
            endif()
            if(EXISTS ${FULL_CHECK_PATH})
                set(PATH_MAPPING_FILE ${FULL_CHECK_PATH})
                break()
            endif()
        endforeach()
    endif()

    if(DEFINED PATH_MAPPING_FILE)
        message(VERBOSE "Using path mapping ${PATH_MAPPING_FILE}")
    else()
        message(FATAL_ERROR "Couldn't locate path mapping in ${APP_DIR} (for context ${DEST_CONTEXT})")
    endif()
endmacro()

# Ensure that patchelf is installed on a Linux system. Configuration will fail if it's missing.
macro(ensure_patchelf_installed)
    execute_process(COMMAND sh -c "which patchelf"
                    OUTPUT_QUIET
                    RESULT_VARIABLE EXIT_CODE)
    if(NOT ${EXIT_CODE} EQUAL 0)
        message(FATAL_ERROR "Could not locate patchelf. Please run check_build_environment.")
    endif()
endmacro()

# Check existence of bcm_host.h header file to see if we're building on Raspberry
macro(check_raspbian_os RASPBERRY)
    if(${ARCH} MATCHES "armhf")
        MESSAGE(VERBOSE "Looking for bcm_host.h")
        INCLUDE(CheckIncludeFiles)

        # Raspbian bullseye bcm_host.h location
        CHECK_INCLUDE_FILES("/usr/include/bcm_host.h" RASPBERRY)

        # otherwise, check previous location of bcm_host.h on older Raspbian OS's
        if(NOT RASPBERRY)
            CHECK_INCLUDE_FILES("/opt/vc/include/bcm_host.h" RASPBERRY)
        endif()
    endif()
endmacro()

# Initialise our Python environment
# _LIB AND _EXECUTABLE are used to help find_package(pybind11)
function(configure_python)
    if(DEFINED PYTHON_BIN)
        return()
    endif()

    # Clear any system Python path settings
    unset(ENV{PYTHONHOME})
    unset(ENV{PYTHONPATH})

    set(PYTHON_PREFIX ${THIRDPARTY_DIR}/python)
    if(CMAKE_HOST_WIN32)
        set(PYTHON_BIN ${PYTHON_PREFIX}/msvc/x86_64/python.exe)
        set(PYTHON_LIB_DIR ${PYTHON_PREFIX}/msvc/x86_64/libs PARENT_SCOPE)
    elseif(CMAKE_HOST_APPLE)
        set(PYTHON_BIN ${PYTHON_PREFIX}/macos/x86_64/bin/python3)
        set(PYTHON_LIB_DIR ${PYTHON_PREFIX}/macos/x86_64/lib PARENT_SCOPE)
    else()
        set(PYTHON_BIN ${PYTHON_PREFIX}/linux/${ARCH}/bin/python3)
        set(PYTHON_LIB_DIR ${PYTHON_PREFIX}/linux/${ARCH}/lib PARENT_SCOPE)
    endif()
    if(NOT EXISTS ${PYTHON_BIN})
        message(FATAL_ERROR "Python not found at ${PYTHON_BIN}. Have you updated thirdparty?")
    endif()
    set(PYTHON_BIN ${PYTHON_BIN} PARENT_SCOPE)
    set(PYTHON_EXECUTABLE ${PYTHON_BIN} PARENT_SCOPE)
endfunction()

# Bring solution_info.json into Source root CMakeLists.txt
function(solution_info_to_cmake)
    set(input_path ${NAP_ROOT}/solution_info.json)
    set(output_path ${NAP_ROOT}/additional_targets.cmake)

    if(NOT EXISTS ${input_path})
        if(EXISTS ${output_path})
            file(REMOVE ${output_path})
        endif()
        return()
    endif()
    # Use configure_file to result in changes in the JSON file triggering reconfigure. Appears to be best current approach.
    configure_file(${input_path} solution_info_trigger_dummy.json)
    execute_process(COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_CACHEFILE_DIR}/solution_info_trigger_dummy.json
                    ERROR_QUIET)

    # Parse our JSON and import it
    configure_python()
    set(python_tools_dir ${NAP_ROOT}/tools/buildsystem/common)
    execute_process(COMMAND ${PYTHON_BIN}
                            ${python_tools_dir}/list_in_json_to_cmake.py
                            ${input_path} AdditionalTargets
                            EXTRA_TARGETS
                            ${output_path}
                    RESULT_VARIABLE EXIT_CODE
                    )
    if(NOT ${EXIT_CODE} EQUAL 0)
        message(FATAL_ERROR "Could not parse solution_info.json (${EXIT_CODE})")
    endif()
    include(${output_path})

    foreach(EXTRA_TARGET ${EXTRA_TARGETS})
        add_subdirectory(${EXTRA_TARGET})
    endforeach()
endfunction()

# Get our NAP modules dependencies from module.json in specified directory, populating into DEPENDENT_NAP_MODULES,
# DEEP_DEPENDENT_NAP_MODULES and DEEP_DEPENDENT_RPATHS
# Directory: Module directory
macro(module_json_in_directory_to_cmake DIRECTORY)
    # Use configure_file to result in changes in module.json triggering reconfigure. Appears to be best current approach.
    configure_file(${DIRECTORY}/module.json module_json_trigger_dummy.json)
    execute_process(COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_CACHEFILE_DIR}/module_json_trigger_dummy.json
                    ERROR_QUIET)

    # Find path mapping to build RPATHs
    find_path_mapping(${DIRECTORY} framework_release)

    # TODO this is slow, ideally only run it if module.json has changed (when calling regenerate explicitly)

    # Parse our module.json and import it
    configure_python()
    set(python_tools_dir ${NAP_ROOT}/tools/buildsystem/common)
    execute_process(COMMAND ${PYTHON_BIN}
                            ${python_tools_dir}/module_info_to_cmake.py
                            ${NAP_ROOT}
                            ${PATH_MAPPING_FILE}
                            ${ARCH}
                            parse_json
                            ${DIRECTORY}
                    RESULT_VARIABLE EXIT_CODE
                    )
    if(NOT ${EXIT_CODE} EQUAL 0)
        message(FATAL_ERROR "Could not parse modules dependencies from module.json (${EXIT_CODE})")
    endif()
    include(${DIRECTORY}/cached_module_json.cmake)
    # message("${PROJECT_NAME} DEPENDENT_NAP_MODULES from module.json: ${DEPENDENT_NAP_MODULES}")
endmacro()

# Build deep module dependencies from module list, populating DEPENDENT_NAP_MODULES,
# DEEP_DEPENDENT_NAP_MODULES and DEEP_DEPENDENT_RPATHS
# MODULE_LIST: List of modules to work from
macro(module_list_to_deep_dependencies_cmake MODULE_LIST)
    # Find path mapping to build RPATHs
    find_path_mapping(${CMAKE_CURRENT_SOURCE_DIR} framework_release)

    # Calculate deep dependencies and RPATHs
    configure_python()
    set(python_tools_dir ${NAP_ROOT}/tools/buildsystem/common)
    execute_process(COMMAND ${PYTHON_BIN}
                            ${python_tools_dir}/module_info_to_cmake.py
                            --path-mapping-element=NapkinExeToRoot
                            ${NAP_ROOT}
                            ${PATH_MAPPING_FILE}
                            ${ARCH}
                            process_module_list
                            "${MODULE_LIST}"
                            ${CMAKE_CURRENT_SOURCE_DIR}
                    RESULT_VARIABLE EXIT_CODE
                    )
    if(NOT ${EXIT_CODE} EQUAL 0)
        message(FATAL_ERROR "Could not parse modules dependencies from module list (${EXIT_CODE})")
    endif()
    include(${CMAKE_CURRENT_SOURCE_DIR}/cached_module_json.cmake)
    file(REMOVE ${CMAKE_CURRENT_SOURCE_DIR}/cached_module_json.cmake)
endmacro()

# Find RTTR using our thirdparty paths
macro(find_rttr)
    if(NOT TARGET RTTR::Core)
        if(WIN32)
            set(RTTR_DIR "${THIRDPARTY_DIR}/rttr/msvc/x86_64/cmake")
        elseif(APPLE)
            set(RTTR_DIR "${THIRDPARTY_DIR}/rttr/macos/x86_64/cmake")
            find_path(
                    RTTR_DIR
                    NAMES rttr-config.cmake
                    HINTS
                    ${THIRDPARTY_DIR}/rttr/macos/x86_64/cmake
            )
        else()
            set(RTTR_DIR "${THIRDPARTY_DIR}/rttr/linux/${ARCH}/cmake")
        endif()
        find_package(RTTR CONFIG REQUIRED Core)
        set(RTTR_LICENSE_FILES ${RTTR_DIR}/)
    endif()
endmacro()

# Find SDL2
macro(find_sdl2)
    # Because we use the upstream SDL2 find module, we augment it with some extra platform specifc vars
    set(SDL2_DIR ${NAP_ROOT}/system_modules/naprender/thirdparty/SDL2)
    set(SDL2_LICENSE_FILES ${SDL2_DIR}/COPYING.txt)
    if(NOT TARGET SDL2)
        if(WIN32)
            set(SDL2_LIBS_DIR ${SDL2_DIR}/msvc/x86_64/lib)
            set(CMAKE_LIBRARY_PATH ${SDL2_LIBS_DIR})
            set(CMAKE_PREFIX_PATH ${SDL2_DIR}/msvc/x86_64)
        elseif(APPLE)
            set(SDL2_LIBS_DIR ${SDL2_DIR}/macos/x86_64/lib)
            set(CMAKE_LIBRARY_PATH ${SDL2_LIBS_DIR})
            set(CMAKE_PREFIX_PATH ${SDL2_DIR}/macos/x86_64)
        elseif(UNIX)
            set(SDL2_LIBS_DIR ${SDL2_DIR}/linux/${ARCH}/lib)
            set(CMAKE_LIBRARY_PATH ${SDL2_LIBS_DIR})
            set(CMAKE_PREFIX_PATH ${SDL2_DIR}/linux/${ARCH})
        endif()
        list(APPEND CMAKE_MODULE_PATH ${NAP_ROOT}/system_modules/naprender/thirdparty/cmake_find_modules)
        find_package(SDL2 REQUIRED)
    endif()
endmacro()

# Helper function to filter out platform-specific files
# The function outputs the following new variables with the platform-specific sources:
# - WIN32_SOURCES
# - MACOS_SOURCES
# - LINUX_SOURCES
function(filter_platform_specific_files UNFILTERED_SOURCES)
    set(LOCAL_WIN32_SOURCES)
    set(LOCAL_MACOS_SOURCES)
    set(LOCAL_LINUX_SOURCES)
    foreach(TMP_PATH ${${UNFILTERED_SOURCES}})
        string(FIND ${TMP_PATH} "/win32/" WIN32_EXCLUDE_DIR_FOUND)
        if(NOT ${WIN32_EXCLUDE_DIR_FOUND} EQUAL -1)
            list(APPEND LOCAL_WIN32_SOURCES ${TMP_PATH})
        else()
            string(FIND ${TMP_PATH} "/osx/" MACOS_EXCLUDE_DIR_FOUND)
            if(NOT ${MACOS_EXCLUDE_DIR_FOUND} EQUAL -1)
                list(APPEND LOCAL_MACOS_SOURCES ${TMP_PATH})
            else()
                string(FIND ${TMP_PATH} "/linux/" LINUX_EXCLUDE_DIR_FOUND)
                if(NOT ${LINUX_EXCLUDE_DIR_FOUND} EQUAL -1)
                    list(APPEND LOCAL_LINUX_SOURCES ${TMP_PATH})
                endif()
            endif()
        endif()
    endforeach(TMP_PATH)

    set(WIN32_SOURCES ${LOCAL_WIN32_SOURCES} PARENT_SCOPE)
    set(MACOS_SOURCES ${LOCAL_MACOS_SOURCES} PARENT_SCOPE)
    set(LINUX_SOURCES ${LOCAL_LINUX_SOURCES} PARENT_SCOPE)
endfunction()

# Helper macro to add platform-specific files to the correct directory and
# to only compile the platform-specific files that match the current platform
macro(add_platform_specific_files WIN32_SOURCES MACOS_SOURCES LINUX_SOURCES)
    # Add to solution folders
    if(MSVC)
        # Sort header and cpps into solution folders for Win32
        foreach(TMP_PATH ${WIN32_SOURCES})
            string(FIND ${TMP_PATH} ".cpp" IS_CPP)
            if(NOT ${IS_CPP} EQUAL -1)
                source_group("Source Files\\Win32" FILES ${TMP_PATH})
            else()
                source_group("Header Files\\Win32" FILES ${TMP_PATH})
            endif()
        endforeach()
        foreach(TMP_PATH ${LINUX_SOURCES})
            string(FIND ${TMP_PATH} ".cpp" IS_CPP)
            if(NOT ${IS_CPP} EQUAL -1)
                source_group("Source Files\\Linux" FILES ${TMP_PATH})
            else()
                source_group("Header Files\\Linux" FILES ${TMP_PATH})
            endif()
        endforeach()
        foreach(TMP_PATH ${MACOS_SOURCES})
            string(FIND ${TMP_PATH} ".cpp" IS_CPP)
            if(NOT ${IS_CPP} EQUAL -1)
                source_group("Source Files\\macOS" FILES ${TMP_PATH})
            else()
                source_group("Header Files\\macOS" FILES ${TMP_PATH})
            endif()
        endforeach()
    endif()

    # Unfortunately, there's no clean way to add a file to the solution (for browsing purposes, etc) but
    # exclude it from the build. The hacky way to do it is to treat the file as a 'header' (even though it's not)
    if(NOT WIN32)
        set_source_files_properties(${WIN32_SOURCES} PROPERTIES HEADER_FILE_ONLY TRUE)
    endif()

    if(NOT APPLE)
        set_source_files_properties(${MACOS_SOURCES} PROPERTIES HEADER_FILE_ONLY TRUE)
    endif()

    if(APPLE OR NOT UNIX)
        set_source_files_properties(${LINUX_SOURCES} PROPERTIES HEADER_FILE_ONLY TRUE)
    endif()
endmacro()

# Set build output configuration for Source context
macro(set_source_build_configuration)
    if(MSVC OR APPLE)
        # Loop over each configuration for multi-configuration systems
        foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
            set(BUILD_CONF ${OUTPUTCONFIG}-${ARCH})

            # Separate our outputs for packaging and non packaging (due to differing behaviour in core, plus speeds up
            # builds when working in packaging and non-packaging at the same time)
            if(DEFINED NAP_PACKAGED_BUILD)
                set(BIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/packaging_bin/${BUILD_CONF})
                set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/packaging_lib/${BUILD_CONF})
            else()
                set(BIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bin/${BUILD_CONF})
                set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/${BUILD_CONF})
            endif()

            string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
            set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${BIN_DIR})
            set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${LIB_DIR})
            set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${LIB_DIR})
        endforeach(OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES)

        add_compile_definitions(NAP_BUILD_CONF=$<CONFIG>-${ARCH})
        add_compile_definitions(NAP_BUILD_TYPE=$<CONFIG>)
    else()
        set(BUILD_CONF ${CMAKE_BUILD_TYPE}-${ARCH})

        # Separate our outputs for packaging and non packaging (due to differing behaviour in core, plus speeds up
        # builds when working in packaging and non-packaging at the same time)
        if(DEFINED NAP_PACKAGED_BUILD)
            set(BIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/packaging_bin/${BUILD_CONF})
            set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/packaging_lib/${BUILD_CONF})
        else()
            set(BIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bin/${BUILD_CONF})
            set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/${BUILD_CONF})
        endif()
        file(MAKE_DIRECTORY ${BIN_DIR})
        file(MAKE_DIRECTORY ${LIB_DIR})
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${LIB_DIR})
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR})
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${LIB_DIR})
        set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})

        add_compile_definitions(NAP_BUILD_CONF=${BUILD_CONF})
        add_compile_definitions(NAP_BUILD_TYPE=${CMAKE_BUILD_TYPE})
    endif()
    add_compile_definitions(NAP_BUILD_ARCH=${ARCH})
    add_compile_definitions(NAP_BUILD_COMPILER=${CMAKE_CXX_COMPILER_ID})
endmacro()

# Set a default build type if none was specified (single-configuration generators only, ie. Linux)
macro(set_default_build_type)
    if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
        message(STATUS "Setting build type to 'Debug' as none was specified.")
        set(CMAKE_BUILD_TYPE "Debug")
    endif()
endmacro()

# Add the app module (if it exists) into the app
macro(add_app_module)
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/module/)
        message(DEBUG "Found app module in ${CMAKE_CURRENT_SOURCE_DIR}/module/")
        set(MODULE_INTO_PARENT TRUE)
        set(IMPORTING_APP_MODULE TRUE)
        add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/module/ app_module_build/)
        unset(MODULE_INTO_PARENT)
        unset(IMPORTING_APP_MODULE)

        # Add includes
        target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/module/src)

        # Link
        target_link_libraries(${PROJECT_NAME} nap${PROJECT_NAME})

        target_compile_definitions(${PROJECT_NAME} PRIVATE MODULE_NAME=${PROJECT_NAME})

        # On Windows copy over module DLLs post-build
        if(WIN32)
            add_custom_command(
                TARGET ${PROJECT_NAME}
                POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:nap${PROJECT_NAME}> $<TARGET_FILE_DIR:${PROJECT_NAME}>/
                COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/module/module.json $<TARGET_FILE_DIR:${PROJECT_NAME}>/nap${PROJECT_NAME}.json
            )
        endif()
    endif()
endmacro()

# Export all FBXs in directory to meshes
function(export_fbx)
    if(NAP_PACKAGED_BUILD AND NOT BUILD_APPS)
        return()
    endif()

    set(workdir ${CMAKE_CURRENT_SOURCE_DIR}/data/)

    # If we don't have any FBXs bail
    file(GLOB fbx_files ${workdir}/*.fbx)
    list(LENGTH fbx_files fbx_count)
    if(NOT fbx_files)
        return()
    endif()

    if(NAP_BUILD_CONTEXT MATCHES "source")
        if(MSVC OR APPLE)
            set(BUILD_CONF $<CONFIG>-${ARCH})
        endif()
        if(DEFINED NAP_PACKAGED_BUILD)
            set(fbxconv_dir ${NAP_ROOT}/packaging_bin/${BUILD_CONF})
        else()
            set(fbxconv_dir ${NAP_ROOT}/bin/${BUILD_CONF})
        endif()
    else()
        set(tools_dir ${NAP_ROOT}/tools)
        set(fbxconv_dir ${tools_dir}/buildsystem)
    endif()

    # Do the export
    add_custom_command(TARGET ${PROJECT_NAME}
                       POST_BUILD
                       COMMAND ${fbxconv_dir}/fbxconverter -o ${workdir} ${workdir}/*.fbx
                       COMMENT "Exporting FBX in '${workdir}'"
                       )
endfunction()

# Setup our output directories in Framework Release context
macro(set_framework_release_output_directories)
    if(MSVC OR APPLE)
        # Loop over each configuration for multi-configuration systems
        foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
            if(APP_PACKAGE_BIN_DIR)
                set(BIN_DIR ${CMAKE_SOURCE_DIR}/${APP_PACKAGE_BIN_DIR}/)
            else()
                set(BIN_DIR ${CMAKE_SOURCE_DIR}/bin/${OUTPUTCONFIG}-${ARCH}/)
            endif()
            string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
            set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${BIN_DIR})
        endforeach()
    else()
        # Single built type, for Linux
        if(APP_PACKAGE_BIN_DIR)
            set(BIN_DIR ${CMAKE_SOURCE_DIR}/${APP_PACKAGE_BIN_DIR}/)
        else()
            set(BIN_DIR ${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}-${ARCH}/)
        endif()
        set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR})
    endif()
endmacro()

# Setup our module output directories
macro(set_module_output_directories)
    if(MSVC OR APPLE)
        # Loop over each configuration for multi-configuration systems
        foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
            set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/${OUTPUTCONFIG}-${ARCH}/)
            string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
            set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${LIB_DIR})
            set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${LIB_DIR})
        endforeach()
    else()
        # Single built type, for Linux
        set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/${CMAKE_BUILD_TYPE}-${ARCH}/)
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
function(copy_files_to_bin)
    # TODO compare and retain the one that works across both contexts (I believe/hope this will be the top one)
    if(NAP_BUILD_CONTEXT MATCHES "framework_release")
        foreach(F ${ARGN})
            add_custom_command(TARGET ${PROJECT_NAME}
                               POST_BUILD
                               COMMAND ${CMAKE_COMMAND} -E copy_if_different "${F}" "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
                               COMMENT "Copying ${F} -> $<TARGET_FILE_DIR:${PROJECT_NAME}>")
        endforeach()
    else()
        foreach(F ${ARGN})
            add_custom_command(TARGET ${PROJECT_NAME}
                               POST_BUILD
                               COMMAND ${CMAKE_COMMAND} -E copy_if_different "${F}" "$<TARGET_PROPERTY:${PROJECT_NAME},RUNTIME_OUTPUT_DIRECTORY_$<UPPER_CASE:$<CONFIG>>>"
                               COMMENT "Copying ${F} -> bin dir")
        endforeach()
    endif()
endfunction()

# Copy files to project target file dir
# ARGN: Files to copy
function(copy_files_to_target_file_dir)
    foreach(F ${ARGN})
        add_custom_command(TARGET ${PROJECT_NAME}
                           POST_BUILD
                           COMMAND ${CMAKE_COMMAND} -E copy_if_different "${F}" "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
                           COMMENT "Copying ${F} -> $<TARGET_FILE_DIR:${PROJECT_NAME}>"
                           )
    endforeach()
endfunction()

# Copy directory to project bin output
# SRCDIR: The source directory
# DSTDIR: The destination directory without project bin output
function(copy_dir_to_bin SRCDIR DSTDIR)
    add_custom_command(TARGET ${PROJECT_NAME}
                       POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy_directory "${SRCDIR}" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/${DSTDIR}"
                       COMMENT "Copying dir '${SRCDIR}' -> '${DSTDIR}'")
endfunction()

# Windows: Post-build copy Python DLLs into project bin output
# FOR_NAPKIN: Whether copying for Napkin (changing output dir)
macro(win64_copy_python_dlls_postbuild FOR_NAPKIN)
    if(${FOR_NAPKIN})
        set(PYDLL_PATH_SUFFIX "../napkin/")
    else()
        set(PYDLL_PATH_SUFFIX "")
    endif()
    file(GLOB PYTHON_DLLS ${PYTHON_LIB_DIR}/../*.dll)
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
                       COMMAND ${CMAKE_COMMAND} -E copy ${PYTHON_LIB_DIR}/../python36.zip $<TARGET_FILE_DIR:${PROJECT_NAME}>/${PYMOD_PATH_SUFFIX}
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

# macOS: Add the runtime path for RTTR
# TODO As a lower priority this should get pulled in automatically, need to cleanup. Jira NAP-108.
function(macos_add_rttr_rpath)
    add_custom_command(TARGET ${PROJECT_NAME}
                       POST_BUILD
                       COMMAND sh -c \"${CMAKE_INSTALL_NAME_TOOL} -add_rpath ${THIRDPARTY_DIR}/rttr/macos/${ARCH}/bin $<TARGET_FILE:${PROJECT_NAME}> 2>/dev/null\;exit 0\"
                       )
endfunction()

# Populate modules list from app.json into var NAP_MODULES
macro(app_json_to_cmake)
    # Use configure_file to result in changes in app.json triggering reconfigure. Appears to be best current approach.
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/app.json app_json_trigger_dummy.json)
    execute_process(COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_CACHEFILE_DIR}/app_json_trigger_dummy.json
                    ERROR_QUIET)

    # TODO this is slow, ideally only run it if app.json has changed (when calling regenerate explicitly)

    # Parse our app.json and import it
    configure_python()
    set(python_tools_dir ${NAP_ROOT}/tools/buildsystem/common)
    execute_process(COMMAND ${PYTHON_BIN} ${python_tools_dir}/app_info_parse_to_cmake.py ${CMAKE_CURRENT_SOURCE_DIR}
                    RESULT_VARIABLE EXIT_CODE
                    )
    if(NOT ${EXIT_CODE} EQUAL 0)
        message(FATAL_ERROR "Could not parse modules from app.json (${EXIT_CODE})")
    endif()
    include(cached_app_json.cmake)
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
function(copy_module_json_to_bin)
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
endfunction()

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
    set(NAP_MODULES_DIR ${NAP_ROOT}/system_modules/)

    # Set the user modules location
    set(USER_MODULES_DIR ${NAP_ROOT}/modules/)

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
        # Check for an app module
        elseif(${SEARCH_MODULE} STREQUAL nap${PROJECT_NAME} AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/module)
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
# APP_DIR: The app directory
# DEST_CONTEXT: The run time context
function(deploy_single_path_mapping APP_DIR DEST_CONTEXT)
    # Deploy to build output
    find_path_mapping(${APP_DIR} ${DEST_CONTEXT})

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

    set(APP_DEST_CACHE_PATH ${APP_DIR}/cache/path_mapping.json)
    if(NOT (WIN32 AND NAP_PACKAGED_APP_BUILD))
        add_custom_command(TARGET ${PROJECT_NAME}
                           POST_BUILD
                           COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PATH_MAPPING_FILE} ${APP_DEST_CACHE_PATH}
                           COMMENT "Deploying path mapping to app directory")
    endif()

    if(NAP_BUILD_CONTEXT MATCHES "framework_release")
        # Install into packaged app
        find_path_mapping(${APP_DIR} packaged_app)
        install(FILES ${PATH_MAPPING_FILE} DESTINATION cache RENAME path_mapping.json)
        if(WIN32 AND NAP_PACKAGED_APP_BUILD)
            set(APP_DEST_CACHE_PATH ${CMAKE_INSTALL_PREFIX}/cache/path_mapping.json)
            add_custom_command(TARGET ${PROJECT_NAME}
                               POST_BUILD
                               COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PATH_MAPPING_FILE} ${APP_DEST_CACHE_PATH}
                               COMMENT "Deploying Win64 path mapping to packaged app app directory")
        endif()
    endif()
endfunction()

# Generic way to import each module for different configurations. Included is a fairly simple mechanism for
# extra per-module CMake logic, to be refined.
macro(find_nap_module MODULE_NAME)
    if(EXISTS ${NAP_ROOT}/modules/${MODULE_NAME}/)
        message(STATUS "Module is user module: ${MODULE_NAME}")
        set(MODULE_INTO_PARENT TRUE)
        add_subdirectory(${NAP_ROOT}/modules/${MODULE_NAME} modules/${MODULE_NAME})
        unset(MODULE_INTO_PARENT)

        # On Windows copy over module DLLs and JSON post-build
        if(WIN32)
            add_custom_command(
                TARGET ${PROJECT_NAME}
                POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory $<TARGET_FILE_DIR:${MODULE_NAME}>/ $<TARGET_FILE_DIR:${PROJECT_NAME}>/
                )
        endif()
    elseif(EXISTS ${NAP_ROOT}/system_modules/${NAP_MODULE}/)
        set(sys_module_dir ${NAP_ROOT}/system_modules/${NAP_MODULE}/)
        if(NOT TARGET ${NAP_MODULE})
            add_library(${MODULE_NAME} INTERFACE)

            message(STATUS "Adding library path for ${MODULE_NAME}")
            set(${MODULE_NAME}_MODULE_JSON ${sys_module_dir}/lib/Release-${ARCH}/${MODULE_NAME}.json)
            if(UNIX)
                set(${MODULE_NAME}_RELEASE_LIB ${sys_module_dir}/lib/Release-${ARCH}/${MODULE_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX})
                set(${MODULE_NAME}_DEBUG_LIB ${sys_module_dir}/lib/Debug-${ARCH}/${MODULE_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX})
            else()
                set(${MODULE_NAME}_DEBUG_LIB ${sys_module_dir}/lib/Debug-${ARCH}/${MODULE_NAME}.lib)
                set(${MODULE_NAME}_RELEASE_LIB ${sys_module_dir}/lib/Release-${ARCH}/${MODULE_NAME}.lib)
            endif()

            target_link_libraries(${MODULE_NAME} INTERFACE debug ${${MODULE_NAME}_DEBUG_LIB})
            target_link_libraries(${MODULE_NAME} INTERFACE optimized ${${MODULE_NAME}_RELEASE_LIB})
            set(MODULE_INCLUDE_ROOT ${sys_module_dir}/include)
            file(GLOB_RECURSE module_headers ${MODULE_INCLUDE_ROOT}/*.h ${MODULE_INCLUDE_ROOT}/*.hpp)
            target_sources(${MODULE_NAME} INTERFACE ${module_headers})
            set_target_properties(${MODULE_NAME} PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${sys_module_dir}/include)

            # Build source groups for our headers maintaining their folder structure
            create_hierarchical_source_groups_for_files("${module_headers}" ${MODULE_INCLUDE_ROOT} "Modules\\${MODULE_NAME}")
        endif()

        # Add any CMake find modules path
        if(EXISTS ${sys_module_dir}/thirdparty)
            list(APPEND CMAKE_MODULE_PATH ${sys_module_dir}/thirdparty/cmake_find_modules)
        endif()

        # On macOS & Linux install module into packaged app
        if(NOT WIN32)
            install(FILES ${${MODULE_NAME}_RELEASE_LIB} DESTINATION lib CONFIGURATIONS Release)
            install(FILES ${${MODULE_NAME}_MODULE_JSON} DESTINATION lib CONFIGURATIONS Release)
            # On Linux set our modules use their directory for RPATH
            if(NOT APPLE)
                install(CODE "message(\"Setting RPATH on ${CMAKE_INSTALL_PREFIX}/lib/${MODULE_NAME}.so\")
                              execute_process(COMMAND patchelf
                                                      --set-rpath
                                                      $ORIGIN/.
                                                      ${CMAKE_INSTALL_PREFIX}/lib/${MODULE_NAME}.so
                                              RESULT_VARIABLE EXIT_CODE)
                              if(NOT \${EXIT_CODE} EQUAL 0)
                                  message(FATAL_ERROR \"Failed to fetch RPATH on ${MODULE_NAME} using patchelf. Is patchelf installed?\")
                              endif()")
            endif()
        endif()

        # Bring in any additional module logic
        set(MODULE_EXTRA_CMAKE_PATH ${sys_module_dir}/module_extra.cmake)
        if(EXISTS ${MODULE_EXTRA_CMAKE_PATH})
            unset(MODULE_EXTRA_LIBS)
            unset(MODULE_EXTRA_LIBS_OPTIMIZED)
            unset(MODULE_EXTRA_LIBS_DEBUG)
            include(${MODULE_EXTRA_CMAKE_PATH})
            if(MODULE_EXTRA_LIBS)
                foreach(extra_lib ${MODULE_EXTRA_LIBS})
                    target_link_libraries(${MODULE_NAME} INTERFACE ${extra_lib})
                endforeach()
            endif()
            if(MODULE_EXTRA_LIBS_OPTIMIZED)
                foreach(extra_lib ${MODULE_EXTRA_LIBS_OPTIMIZED})
                    target_link_libraries(${MODULE_NAME} INTERFACE optimized ${extra_lib})
                endforeach()
            endif()
            if(MODULE_EXTRA_LIBS_DEBUG)
                foreach(extra_lib ${MODULE_EXTRA_LIBS_DEBUG})
                    target_link_libraries(${MODULE_NAME} INTERFACE debug ${extra_lib})
                endforeach()
            endif()
        endif()

        if(WIN32)
            # Copy over module DLLs post-build
            add_custom_command(
                TARGET ${PROJECT_NAME}
                POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different ${sys_module_dir}/lib/$<CONFIG>-${ARCH}/${MODULE_NAME}.dll $<TARGET_FILE_DIR:${PROJECT_NAME}>/
                COMMAND ${CMAKE_COMMAND} -E copy_if_different ${sys_module_dir}/lib/$<CONFIG>-${ARCH}/${MODULE_NAME}.json $<TARGET_FILE_DIR:${PROJECT_NAME}>/
                )

            # Copy PDB post-build, if we have them
            if(EXISTS ${sys_module_dir}/lib/Debug/${MODULE_NAME}.pdb)
                add_custom_command(
                    TARGET ${PROJECT_NAME}
                    POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${sys_module_dir}/lib/$<CONFIG>-${ARCH}/${MODULE_NAME}.pdb $<TARGET_FILE_DIR:${PROJECT_NAME}>/
                    )
            endif()
        endif()
    elseif(NOT TARGET ${MODULE_NAME})
        message(FATAL_ERROR "Could not locate module '${MODULE_NAME}'")
    endif()
endmacro()

# Add an include to the list of includes on an interface target
function(add_include_to_interface_target TARGET_NAME INCLUDE_PATH)
    # Deal with cases using module_extra.cmake for DLL installation when targets aren't defined
    if(INSTALLING_MODULE_FOR_NAPKIN AND NOT TARGET ${TARGET_NAME})
        return()
    endif()

    # Get existing list of includes
    get_target_property(module_includes ${TARGET_NAME} INTERFACE_INCLUDE_DIRECTORIES)

    # Handle no existing includes
    if(NOT module_includes)
        set(module_includes "")
    endif()

    # Append new path and set on target
    list(APPEND module_includes ${INCLUDE_PATH})
    set_target_properties(${TARGET_NAME} PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${module_includes}")
endfunction()

# Add a define to the list of defines on an interface target
function(add_define_to_interface_target TARGET_NAME DEFINE)
    # Deal with cases using module_extra.cmake for DLL installation when targets aren't defined
    if(INSTALLING_MODULE_FOR_NAPKIN AND NOT TARGET ${TARGET_NAME})
        return()
    endif()

    # Get existing list of includes
    get_target_property(module_defines ${TARGET_NAME} INTERFACE_COMPILE_DEFINITIONS)

    # Handle no existing includes
    if(NOT module_defines)
        set(module_defines "")
    endif()

    # Append new path and set on target
    list(APPEND module_defines ${DEFINE})
    set_target_properties(${TARGET_NAME} PROPERTIES INTERFACE_COMPILE_DEFINITIONS "${module_defines}")
endfunction()

# Determine module name from directory
function(directory_name_to_module_name)
    get_filename_component(module_name ${CMAKE_CURRENT_SOURCE_DIR} NAME)
    if(module_name MATCHES "^module$")
        # Handle app module
        get_filename_component(parent_dir ${CMAKE_CURRENT_SOURCE_DIR}/.. REALPATH)
        get_filename_component(module_name ${parent_dir} NAME)
        set(module_name "nap${module_name}")
    endif()
    set(MODULE_NAME ${module_name} PARENT_SCOPE)
endfunction()
