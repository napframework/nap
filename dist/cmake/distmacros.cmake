# Set a default build type if none was specified (single-configuration generators only, ie. Linux)
macro(set_default_build_type)
	if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
	    message(STATUS "Setting build type to 'Debug' as none was specified.")
	    set(CMAKE_BUILD_TYPE "Debug")
	endif()
endmacro()


# Somewhat temporary generic way to import each module for different configurations.  I think we'll need make proper package files for
# each module in the long run
# TODO let's avoid per-module cmake package files for now.. but probably need to re-address later
macro(find_nap_module MODULE_NAME)
    add_library(${MODULE_NAME} SHARED IMPORTED)

    message("Adding lib path for ${MODULE_NAME}")
    if (WIN32)
        set(MOD_RELEASE_DLL ${NAP_ROOT}/modules/${MODULE_NAME}/lib/Release/${MODULE_NAME}.dll)
        set(MOD_DEBUG_DLL ${NAP_ROOT}/modules/${MODULE_NAME}/lib/Debug/${MODULE_NAME}.dll)
        set(MOD_IMPLIB_DEBUG ${NAP_ROOT}/modules/${MODULE_NAME}/lib/Debug/${MODULE_NAME}.lib)
        set(MOD_IMPLIB_RELEASE ${NAP_ROOT}/modules/${MODULE_NAME}/lib/Release/${MODULE_NAME}.lib)
    elseif (APPLE)
        set(MOD_RELEASE_DLL ${NAP_ROOT}/modules/${MODULE_NAME}/lib/Release/lib${MODULE_NAME}.dylib)
        set(MOD_DEBUG_DLL ${NAP_ROOT}/modules/${MODULE_NAME}/lib/Debug/lib${MODULE_NAME}.dylib)
    elseif (UNIX)
        set(MOD_RELEASE_DLL ${NAP_ROOT}/modules/${MODULE_NAME}/lib/Release/lib${MODULE_NAME}.so)
        set(MOD_DEBUG_DLL ${NAP_ROOT}/modules/${MODULE_NAME}/lib/Debug/lib${MODULE_NAME}.so)
    endif()

    set_target_properties(${MODULE_NAME} PROPERTIES
        IMPORTED_CONFIGURATIONS "Debug;Release;MinSizeRel;RelWithDebInfo"
        IMPORTED_LOCATION_RELEASE ${MOD_RELEASE_DLL}
        IMPORTED_LOCATION_DEBUG ${MOD_DEBUG_DLL}
        IMPORTED_LOCATION_MINSIZEREL ${MOD_RELEASE_DLL}
        IMPORTED_LOCATION_RELWITHDEBINFO ${MOD_RELEASE_DLL}
    )

    # Add module includes
    message("Adding include for ${NAP_MODULE}")
    # TODO do this conditionally based on whether we have /include or /src
    target_include_directories(${PROJECT_NAME} PUBLIC ${NAP_ROOT}/modules/${NAP_MODULE}/include/)
    target_include_directories(${PROJECT_NAME} PUBLIC ${NAP_ROOT}/modules/${NAP_MODULE}/src/)

    if (WIN32)
        # Set Windows .lib locations
        # TODO test test test
        set_target_properties(${MODULE_NAME} PROPERTIES
            IMPORTED_IMPLIB_RELEASE ${MOD_IMPLIB_RELEASE}
            IMPORTED_IMPLIB_DEBUG ${MOD_IMPLIB_DEBUG}
            IMPORTED_IMPLIB_MINSIZEREL ${MOD_IMPLIB_RELEASE}
            IMPORTED_IMPLIB_RELWITHDEBINFO ${MOD_IMPLIB_RELEASE}
        )

        # Copy over module DLLs post-build
        add_custom_command(
            TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${MODULE_NAME}> $<TARGET_FILE_DIR:${PROJECT_NAME}>/
        )
    endif()

    # Bring in any additional module requirements
    set(MODULE_EXTRA_CMAKE_PATH ${NAP_ROOT}/modules/${MODULE_NAME}/moduleExtra.cmake)
    if (EXISTS ${MODULE_EXTRA_CMAKE_PATH})
        include (${MODULE_EXTRA_CMAKE_PATH})
    endif()
endmacro()

macro(dist_export_fbx SRCDIR)
    set(TOOLS_DIR "${NAP_ROOT}/tools/")

    # Set the binary name
    set(FBXCONVERTER_BIN "${TOOLS_DIR}/fbxconverter")

    # Set project data out path
    set(OUTDIR "$<TARGET_FILE_DIR:${PROJECT_NAME}>/data/${PROJECT_NAME}")

    # Ensure data output directory for project exists
    add_custom_command(TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTDIR}
        COMMENT "Ensure project output directory exists for fbxconverter")

    # Do the export
    add_custom_command(TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND "${FBXCONVERTER_BIN}" -o ${OUTDIR} "${SRCDIR}/*.fbx"
        COMMENT "Export FBX in '${SRCDIR}'")
endmacro()

# Change our project output directories
macro(set_output_directories)
    if (MSVC OR APPLE)
        # Loop over each configuration for multi-configuration systems
        foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
            set(BUILD_CONF ${CMAKE_CXX_COMPILER_ID}-${ARCH}-${OUTPUTCONFIG})
    		set(BIN_DIR ${CMAKE_SOURCE_DIR}/bin/${BUILD_CONF}/)
            string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
            set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${BIN_DIR})
            # set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${BIN_DIR})
            # set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${BIN_DIR})
        endforeach()
    else()
        # Single built type, for Linux
        set(BUILD_CONF ${CMAKE_CXX_COMPILER_ID}-${CMAKE_BUILD_TYPE}-${ARCH})
        set(BIN_DIR ${CMAKE_SOURCE_DIR}/bin/${BUILD_CONF}/)
        set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR})
        # set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${BIN_DIR})
        # set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${BIN_DIR})
    endif()
endmacro()

# Change our module output directories
macro(set_module_output_directories)
    if (MSVC OR APPLE)
        # Loop over each configuration for multi-configuration systems
        foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
            set(LIB_DIR ${CMAKE_SOURCE_DIR}/lib/${OUTPUTCONFIG}/)
            string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
            # TODO set the properties we actually need
            set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${LIB_DIR})
            set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${LIB_DIR})
            set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${LIB_DIR})
        endforeach()
    else()
        # Single built type, for Linux
        set(LIB_DIR ${CMAKE_SOURCE_DIR}/lib/${CMAKE_BUILD_TYPE}/)
        # TODO set the properties we actually need
        set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${LIB_DIR})
        set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${LIB_DIR})
        set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${LIB_DIR})
    endif()
endmacro()
