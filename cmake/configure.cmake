if(MSVC OR APPLE)
    foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
        set(BUILD_CONF ${CMAKE_CXX_COMPILER_ID}-${ARCH}-${OUTPUTCONFIG})

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

    add_compile_definitions(NAP_BUILD_CONF=${CMAKE_CXX_COMPILER_ID}-${ARCH}-$<CONFIG>)
else()
    if(ANDROID)
        set(BUILD_CONF Android${CMAKE_CXX_COMPILER_ID}-${CMAKE_BUILD_TYPE}-${ANDROID_ABI})
    else()
        set(BUILD_CONF ${CMAKE_CXX_COMPILER_ID}-${CMAKE_BUILD_TYPE}-${ARCH})
    endif()

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
endif()

# Export all FBXs in directory to meshes using fbxconverter. Meshes are created in the same directory.
# SRCDIR: The directory to work in
macro(export_fbx_in_place SRCDIR)
    if (MSVC OR APPLE)
        set(BUILD_CONF ${CMAKE_CXX_COMPILER_ID}-${ARCH}-$<CONFIG>)
    else()
        set(BUILD_CONF ${CMAKE_CXX_COMPILER_ID}-${CMAKE_BUILD_TYPE}-${ARCH})
    endif()

    # Should be able to use CMAKE_RUNTIME_OUTPUT_DIRECTORY here which would be cleaner but it didn't 
    # fall into place
    if(DEFINED NAP_PACKAGED_BUILD)
        set(FBXCONV_DIR ${CMAKE_SOURCE_DIR}/packaging_bin/${BUILD_CONF})
    else()
        set(FBXCONV_DIR ${CMAKE_SOURCE_DIR}/bin/${BUILD_CONF})
    endif()

    # Do the export
    add_custom_command(TARGET ${PROJECT_NAME}
                       POST_BUILD
                       COMMAND ${FBXCONV_DIR}/fbxconverter -o ${SRCDIR} ${SRCDIR}/*.fbx
                       COMMENT "Export FBX in '${SRCDIR}'"
                       )
endmacro()


# Copy directory to project bin output
# SRCDIR: The source directory
# DSTDIR: The destination directory without project bin output
macro(copy_dir_to_bin SRCDIR DSTDIR)
    add_custom_command(TARGET ${PROJECT_NAME}
                       POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy_directory "${SRCDIR}" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/${DSTDIR}"
                       COMMENT "Copy dir '${SRCDIR}' -> '${DSTDIR}'")
endmacro()

# Copy files to project bin output
# ARGN: Files to copy
macro(copy_files_to_bin)
    foreach(F ${ARGN})
        add_custom_command(TARGET ${PROJECT_NAME}
                           POST_BUILD
                           COMMAND ${CMAKE_COMMAND} -E copy_if_different "${F}" "$<TARGET_PROPERTY:${PROJECT_NAME},RUNTIME_OUTPUT_DIRECTORY_$<UPPER_CASE:$<CONFIG>>>"
                           COMMENT "Copying ${F} -> bin dir")
    endforeach()
endmacro()

# Copy Windows SD2 and GLEW DLLs to project bin output
macro(copy_base_windows_graphics_dlls)
    # Copy over some crap window dlls
    set(FILES_TO_COPY
        ${THIRDPARTY_DIR}/sdl2/msvc/lib/x64/SDL2.dll
        ${THIRDPARTY_DIR}/glew/msvc/bin/Release/x64/glew32.dll
        )
    copy_files_to_bin(${FILES_TO_COPY})
endmacro()

# Copy Windows FFmpeg DLLs to project bin output
macro(copy_windows_ffmpeg_dlls)
    file(GLOB FFMPEGDLLS ${THIRDPARTY_DIR}/ffmpeg/msvc/install/bin/*.dll)
    copy_files_to_bin(${FFMPEGDLLS})
endmacro()

# Helper function to filter out platform-specific files
# The function outputs the following new variables with the platform-specific sources:
# - WIN32_SOURCES
# - MACOS_SOURCES
# - LINUX_SOURCES
# - NATIVE_SOURCES
# - ANDROID_SOURCES
function(filter_platform_specific_files UNFILTERED_SOURCES)
    set(LOCAL_WIN32_SOURCES)
    set(LOCAL_MACOS_SOURCES)
    set(LOCAL_LINUX_SOURCES)
    set(LOCAL_NATIVE_SOURCES)
    set(LOCAL_ANDROID_SOURCES)
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
                else()
                    string(FIND ${TMP_PATH} "/native/" NATIVE_EXCLUDE_DIR_FOUND)
                    if(NOT ${NATIVE_EXCLUDE_DIR_FOUND} EQUAL -1)
                        list(APPEND LOCAL_ANDROID_SOURCES ${TMP_PATH})
                    else()
                        string(FIND ${TMP_PATH} "/android/" ANDROID_EXCLUDE_DIR_FOUND)
                        if(NOT ${LOCAL_ANDROID_SOURCES} EQUAL -1)
                            list(APPEND LOCAL_ANDROID_SOURCES ${TMP_PATH})
                        endif()                        
                    endif()
                endif()
            endif()
        endif()
    endforeach(TMP_PATH)

    set(WIN32_SOURCES ${LOCAL_WIN32_SOURCES} PARENT_SCOPE)
    set(MACOS_SOURCES ${LOCAL_MACOS_SOURCES} PARENT_SCOPE)
    set(LINUX_SOURCES ${LOCAL_LINUX_SOURCES} PARENT_SCOPE)
    set(NATIVE_SOURCES ${LOCAL_NATIVE_SOURCES} PARENT_SCOPE)
    set(ANDROID_SOURCES ${LOCAL_ANDROID_SOURCES} PARENT_SCOPE)
endfunction()

# Helper macro to add platform-specific files to the correct directory and
# to only compile the platform-specific files that match the current platform
macro(add_platform_specific_files WIN32_SOURCES MACOS_SOURCES LINUX_SOURCES NATIVE_SOURCES ANDROID_SOURCES)

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
        foreach(TMP_PATH ${NATIVE_SOURCES})
            string(FIND ${TMP_PATH} ".cpp" IS_CPP)
            if(NOT ${IS_CPP} EQUAL -1)
                source_group("Source Files\\Win32" FILES ${TMP_PATH})
            else()
                source_group("Header Files\\Win32" FILES ${TMP_PATH})
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
        foreach(TMP_PATH ${ANDROID_SOURCES})
            string(FIND ${TMP_PATH} ".cpp" IS_CPP)
            if(NOT ${IS_CPP} EQUAL -1)
                source_group("Source Files\\Android" FILES ${TMP_PATH})
            else()
                source_group("Header Files\\Android" FILES ${TMP_PATH})
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

    if(NOT ANDROID)
        set_source_files_properties(${ANDROID_SOURCES} PROPERTIES HEADER_FILE_ONLY TRUE)
    endif()

    if(ANDROID)
        set_source_files_properties(${NATIVE_SOURCES} PROPERTIES HEADER_FILE_ONLY TRUE)
    endif()

endmacro()

# Change our project output directories (when building against NAP source)
macro(set_output_directories)
    if (MSVC OR APPLE)
        # Loop over each configuration for multi-configuration systems
        foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
            string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
            set(BIN_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG}}/${PROJECT_NAME})
            set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${BIN_DIR})
        endforeach()
    else()
        # Single built type, for Linux
        set(BIN_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME})
        set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR})
    endif()
endmacro()

# Let find_python find our prepackaged Python in thirdparty.  Used before a call to find_package(pybind11)
macro(find_python_in_thirdparty)
    # Set our pre built Python location
    set(PYTHONLIBS_FOUND 1)
    if(ANDROID)
        set(PYTHON_PREFIX ${THIRDPARTY_DIR}/python/android/install)
        set(PYTHON_LIBRARIES ${PYTHON_PREFIX}/lib/${ANDROID_ABI}/libpython3.5m.so)
        set(PYTHON_INCLUDE_DIRS ${PYTHON_PREFIX}/include/${ANDROID_ABI}/python3.5m)
    elseif(UNIX)
        if(APPLE)
            set(PYTHON_PREFIX ${THIRDPARTY_DIR}/python/osx/install)
        else()
            set(PYTHON_PREFIX ${THIRDPARTY_DIR}/python/linux/install)
        endif()
        set(PYTHON_LIBRARIES ${PYTHON_PREFIX}/lib/libpython3.6m${CMAKE_SHARED_LIBRARY_SUFFIX})
        set(PYTHON_INCLUDE_DIRS ${PYTHON_PREFIX}/include/python3.6m)
    else()
        set(PYTHON_PREFIX ${THIRDPARTY_DIR}/python/msvc/python-embed-amd64)
        set(PYTHON_LIBRARIES ${PYTHON_PREFIX}/libs/python36.lib)
        set(PYTHON_INCLUDE_DIRS ${PYTHON_PREFIX}/include)
    endif()
endmacro()

# Populate modules list from project.json into var NAP_MODULES
macro(project_json_to_cmake)
    # Use configure_file to result in changes in project.json triggering reconfigure.  Appears to be best current approach.
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/project.json project_json_trigger_dummy.json)
    execute_process(COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_CACHEFILE_DIR}/project_json_trigger_dummy.json
                    ERROR_QUIET)

    # Clear any system Python path settings
    unset(ENV{PYTHONHOME})
    unset(ENV{PYTHONPATH})

    # Parse our project.json and import it
    if(CMAKE_HOST_WIN32)
        set(PYTHON_BIN ${THIRDPARTY_DIR}/python/msvc/python-embed-amd64/python.exe)
    elseif(CMAKE_HOST_APPLE)
        set(PYTHON_BIN ${THIRDPARTY_DIR}/python/osx/install/bin/python3)
    else()
        set(PYTHON_BIN ${THIRDPARTY_DIR}/python/linux/install/bin/python3)
    endif()
    if(NOT EXISTS ${PYTHON_BIN})
        message(FATAL_ERROR "Python not found at ${PYTHON_BIN}.  Have you updated thirdparty?")
    endif()

    execute_process(COMMAND ${PYTHON_BIN} ${NAP_ROOT}/dist/user_scripts/platform/project_info_parse_to_cmake.py
            ${CMAKE_CURRENT_SOURCE_DIR}
            RESULT_VARIABLE EXIT_CODE
            )
    if(NOT ${EXIT_CODE} EQUAL 0)
        message(FATAL_ERROR "Could not parse modules from project.json (${EXIT_CODE})")
    endif()
    include(cached_project_json.cmake)
endmacro()

# Get our NAP modules dependencies from module.json, populating into DEPENDENT_NAP_MODULES
macro(module_json_to_cmake)
    # Use configure_file to result in changes in module.json triggering reconfigure.  Appears to be best current approach.
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/module.json module_json_trigger_dummy.json)
    execute_process(COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_CACHEFILE_DIR}/module_json_trigger_dummy.json
                    ERROR_QUIET)

    # Clear any system Python path settings
    unset(ENV{PYTHONHOME})
    unset(ENV{PYTHONPATH})

    # Parse our module.json and import it
    if(CMAKE_HOST_WIN32)
        set(PYTHON_BIN ${THIRDPARTY_DIR}/python/msvc/python-embed-amd64/python.exe)
    elseif(CMAKE_HOST_APPLE)
        set(PYTHON_BIN ${THIRDPARTY_DIR}/python/osx/install/bin/python3)
    else()
        set(PYTHON_BIN ${THIRDPARTY_DIR}/python/linux/install/bin/python3)
    endif()
    if(NOT EXISTS ${PYTHON_BIN})
        message(FATAL_ERROR "Python not found at ${PYTHON_BIN}.  Have you updated thirdparty?")
    endif()

    execute_process(COMMAND ${PYTHON_BIN} ${NAP_ROOT}/dist/user_scripts/platform/module_info_parse_to_cmake.py ${CMAKE_CURRENT_SOURCE_DIR} ${NAP_ROOT} 
                    RESULT_VARIABLE EXIT_CODE
                    )
    if(NOT ${EXIT_CODE} EQUAL 0)
        message(FATAL_ERROR "Could not parse modules dependencies from module.json (${EXIT_CODE})")
    endif()
    include(cached_module_json.cmake)
endmacro()

# macOS: Add the runtime path for RTTR
# TODO As a lower priority this should get pulled in automatically, need to cleanup. Jira NAP-108.
macro(add_macos_rttr_rpath)
    add_custom_command(TARGET ${PROJECT_NAME}
                       POST_BUILD
                       COMMAND sh -c \"${CMAKE_INSTALL_NAME_TOOL} -add_rpath ${THIRDPARTY_DIR}/rttr/xcode/install/bin $<TARGET_FILE:${PROJECT_NAME}> 2>/dev/null\;exit 0\"
                       )    
endmacro()

# macOS: Add the runtime path for GLEW  
# TODO As a lower priority this should get pulled in automatically, need to cleanup. Jira NAP-108.
macro(add_macos_glew_rpath)
    add_custom_command(TARGET ${PROJECT_NAME}
                       POST_BUILD
                       COMMAND sh -c \"${CMAKE_INSTALL_NAME_TOOL} -add_rpath ${THIRDPARTY_DIR}/glew/osx/install/lib $<TARGET_FILE:${PROJECT_NAME}> 2>/dev/null\;exit 0\"
                       )    
endmacro()

# Copy Windows Python DLLs to output directory
function(copy_windows_python_dlls_to_bin)
    file(GLOB PYTHON_DLLS ${THIRDPARTY_DIR}/python/msvc/python-embed-amd64/*.dll)
    copy_files_to_bin(${PYTHON_DLLS})
endfunction()

# Copy Windows FFmpeg DLLs to project output directory
function(copy_windows_ffmpeg_dlls_to_project)
    file(GLOB FFMPEGDLLS ${THIRDPARTY_DIR}/ffmpeg/bin/*.dll)
    copy_files_to_bin(${FFMPEGDLLS})
endfunction()

# Find RTTR using our thirdparty paths
macro(find_rttr)
    if(NOT TARGET RTTR::Core)
        if (WIN32)
            if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
                set(RTTR_DIR "${THIRDPARTY_DIR}/rttr/msvc64/install/cmake")
            else()
                set(RTTR_DIR "${THIRDPARTY_DIR}/rttr/msvc32/install/cmake")
            endif()
            find_package(RTTR CONFIG REQUIRED Core)
        elseif(APPLE)
            find_path(
                    RTTR_DIR
                    NAMES rttr-config.cmake
                    HINTS
                    ${THIRDPARTY_DIR}/rttr/xcode/install/cmake
            )
            find_package(RTTR CONFIG REQUIRED Core)
        elseif(ANDROID)
            set(RTTR_DIR ${THIRDPARTY_DIR}/rttr/android/install)

            # Create imported target RTTR::Core
            add_library(RTTR::Core SHARED IMPORTED)
            set_target_properties(RTTR::Core PROPERTIES
                INTERFACE_COMPILE_DEFINITIONS "RTTR_DLL"
                INTERFACE_INCLUDE_DIRECTORIES "${RTTR_DIR}/include"
            )

            set_property(TARGET RTTR::Core APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
            set_property(TARGET RTTR::Core APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
            set_target_properties(RTTR::Core PROPERTIES
                IMPORTED_LOCATION_RELEASE "${RTTR_DIR}/bin/Release/${ANDROID_ABI}/librttr_core.so"
                IMPORTED_SONAME_RELEASE "librttr_core.so"
                IMPORTED_LOCATION_DEBUG "${RTTR_DIR}/bin/Debug/${ANDROID_ABI}/librttr_core_d.so"
                IMPORTED_SONAME_DEBUG "librttr_core_d.so"
                )
        else()
            find_path(
                    RTTR_DIR
                    NAMES rttr-config.cmake
                    HINTS
                    ${THIRDPARTY_DIR}/rttr/install/cmake
                    ${THIRDPARTY_DIR}/rttr/linux/install/cmake
            )
            find_package(RTTR CONFIG REQUIRED Core)
        endif()
    endif()
endmacro()

# Run any module post-build logic for set modules
# Note: Currently unused, leaving as useful draft for potential later use
# NAP_MODULES: The modules to work with
macro(include_module_postbuilds_per_project NAP_MODULES)
    foreach(NAP_MODULE ${NAP_MODULES})
        string(SUBSTRING ${NAP_MODULE} 4 -1 SHORT_MODULE_NAME)
        set(MODULE_POSTBUILD ${NAP_ROOT}/modules/${SHORT_MODULE_NAME}/module_post_build_per_project.cmake)
        if(EXISTS ${MODULE_POSTBUILD})
            include(${MODULE_POSTBUILD})
        endif()
    endforeach()
endmacro()

# Copy module.json for module to sit alongside module post-build
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

# Copy appropriate path mapping in cached location alongside binary
# PROJECT_DIR: The project directory
function(deploy_single_path_mapping PROJECT_DIR)
    find_path_mapping(${NAP_ROOT}/build_tools/path_mappings ${PROJECT_DIR} source)
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
    add_custom_command(TARGET ${PROJECT_NAME}
                       POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PATH_MAPPING_FILE} ${PROJ_DEST_CACHE_PATH}
                       COMMENT "Deploying path mapping to project directory")
endfunction()
