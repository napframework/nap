# Get our NAP modules dependencies from module.json, populating into DEPENDENT_NAP_MODULES
# SYSTEM_MAPPINGS: The diretory containing the system path mappings
# PROJECT_DIR: The project directory
# CONTEXT: Our current build context, ie. one of 'source', 'framework_release' or 'packaged_app'
macro(find_path_mapping SYSTEM_MAPPINGS PROJECT_DIR CONTEXT)
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

    # Check for custom project mappings
    foreach(CHECK_PATH ${CHECK_PATH_LIST})
        set(FULL_CHECK_PATH ${PROJECT_DIR}/config/custom_path_mappings/${CHECK_PATH}/${CONTEXT}.json)
        if(EXISTS ${FULL_CHECK_PATH})
            set(PATH_MAPPING_FILE ${FULL_CHECK_PATH})
            message(STATUS "Using custom path mapping: ${FULL_CHECK_PATH}")
            break()
        endif()
    endforeach()

    # Otherwise find system mapping
    if(NOT DEFINED PATH_MAPPING_FILE)
        foreach(CHECK_PATH ${CHECK_PATH_LIST})
            if(${CONTEXT} STREQUAL source)
                set(FULL_CHECK_PATH ${SYSTEM_MAPPINGS}/${CHECK_PATH}/${CONTEXT}.json)
            else()
                set(FULL_CHECK_PATH ${SYSTEM_MAPPINGS}/${CONTEXT}.json)
            endif()
            if(EXISTS ${FULL_CHECK_PATH})
                set(PATH_MAPPING_FILE ${FULL_CHECK_PATH})
                break()
            endif()
        endforeach()
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


