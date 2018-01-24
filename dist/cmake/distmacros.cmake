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
    # TODO update to use usermodules directory instead
    if (EXISTS ${NAP_ROOT}/usermodules/${NAP_MODULE}/)
        message("Module is user module: ${MODULE_NAME}")
        set(MODULE_INTO_PROJ TRUE)
        add_subdirectory(${NAP_ROOT}/usermodules/${NAP_MODULE} usermodules/${NAP_MODULE})
    elseif (EXISTS ${NAP_ROOT}/modules/${NAP_MODULE}/)

        add_library(${MODULE_NAME} INTERFACE)

        message("Adding lib path for ${MODULE_NAME}")
        if (WIN32)
            set(MOD_DEBUG_LIB ${NAP_ROOT}/modules/${MODULE_NAME}/lib/Debug/${MODULE_NAME}.lib)
            set(MOD_RELEASE_LIB ${NAP_ROOT}/modules/${MODULE_NAME}/lib/Release/${MODULE_NAME}.lib)
        elseif (APPLE)
            set(MOD_RELEASE_LIB ${NAP_ROOT}/modules/${MODULE_NAME}/lib/Release/lib${MODULE_NAME}.dylib)
            set(MOD_DEBUG_LIB ${NAP_ROOT}/modules/${MODULE_NAME}/lib/Debug/lib${MODULE_NAME}.dylib)
        elseif (UNIX)
            set(MOD_RELEASE_LIB ${NAP_ROOT}/modules/${MODULE_NAME}/lib/Release/lib${MODULE_NAME}.so)
            set(MOD_DEBUG_LIB ${NAP_ROOT}/modules/${MODULE_NAME}/lib/Debug/lib${MODULE_NAME}.so)
        endif()

        target_link_libraries(${MODULE_NAME} INTERFACE debug ${MOD_DEBUG_LIB})
        target_link_libraries(${MODULE_NAME} INTERFACE optimized ${MOD_RELEASE_LIB})
        file(GLOB module_headers ${NAP_ROOT}/modules/${NAP_MODULE}/include/*.h)
        target_sources(${MODULE_NAME} INTERFACE ${module_headers})
        source_group(Modules\\${MODULE_NAME} FILES ${module_headers})

        # Add module includes
        message("Adding include for ${NAP_MODULE}")
        target_include_directories(${PROJECT_NAME} PUBLIC ${NAP_ROOT}/modules/${NAP_MODULE}/include/)

        # Bring in any additional module requirements
        # TODO make sure we're running this for our source modules
        set(MODULE_EXTRA_CMAKE_PATH ${NAP_ROOT}/modules/${MODULE_NAME}/moduleExtra.cmake)
        if (EXISTS ${MODULE_EXTRA_CMAKE_PATH})
            include (${MODULE_EXTRA_CMAKE_PATH})
        endif()        
    else()
        message(FATAL_ERROR "Could not locate module '${MODULE_NAME}'")    
    endif()

    if (WIN32)
        # Copy over module DLLs post-build
        add_custom_command(
            TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${NAP_ROOT}/modules/${MODULE_NAME}/lib/$<CONFIG>/${MODULE_NAME}.dll $<TARGET_FILE_DIR:${PROJECT_NAME}>/
        )
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
            # TODO set the properties we actually need
            set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${LIB_DIR})
            set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${LIB_DIR})
            set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${LIB_DIR})
        endforeach()
    else()
        # Single built type, for Linux
        set(LIB_DIR ${CMAKE_SOURCE_DIR}/lib/${CMAKE_BUILD_TYPE}/)
        set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${LIB_DIR})
    endif()
endmacro()
