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

        add_custom_command(
            TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:mod_${PROJECT_NAME}> $<TARGET_FILE_DIR:${PROJECT_NAME}>/
        )       
    endif()
endmacro()

# Somewhat temporary generic way to import each module for different configurations.  I think we'll need make proper package files for
# each module in the long run
# TODO let's avoid per-module cmake package files for now.. but probably need to re-address later
macro(find_nap_module MODULE_NAME)
    if (EXISTS ${NAP_ROOT}/usermodules/${MODULE_NAME}/)
        message("Module is user module: ${MODULE_NAME}")
        set(MODULE_INTO_PROJ TRUE)
        add_subdirectory(${NAP_ROOT}/usermodules/${MODULE_NAME} usermodules/${MODULE_NAME})
        unset(MODULE_INTO_PROJ)

        # On Windows copy over module DLLs post-build
        add_custom_command(
            TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${MODULE_NAME}> $<TARGET_FILE_DIR:${PROJECT_NAME}>/
        )       
    elseif (EXISTS ${NAP_ROOT}/modules/${NAP_MODULE}/)
        if(NOT TARGET ${NAP_MODULE})
            add_library(${MODULE_NAME} INTERFACE)

            message("Adding lib path for ${MODULE_NAME}")
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
            file(GLOB module_headers ${NAP_ROOT}/modules/${NAP_MODULE}/include/*.h)
            target_sources(${MODULE_NAME} INTERFACE ${module_headers})
            source_group(Modules\\${MODULE_NAME} FILES ${module_headers})
        endif(NOT TARGET ${NAP_MODULE})

        # Add module includes
        message("Adding include for ${NAP_MODULE}")
        target_include_directories(${PROJECT_NAME} PUBLIC ${NAP_ROOT}/modules/${NAP_MODULE}/include/)

        # On macOS & Linux install module into packaged project
        if (NOT WIN32)
            install(FILES ${${MODULE_NAME}_RELEASE_LIB} DESTINATION lib CONFIGURATIONS Release)    
        endif()

        # Bring in any additional module requirements
        # TODO make sure we're running this for our source modules
        set(MODULE_EXTRA_CMAKE_PATH ${NAP_ROOT}/modules/${MODULE_NAME}/moduleExtra.cmake)
        if (EXISTS ${MODULE_EXTRA_CMAKE_PATH})
            include (${MODULE_EXTRA_CMAKE_PATH})
        endif()

        if (WIN32)
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

macro(dist_export_fbx SRCDIR)
    set(TOOLS_DIR "${NAP_ROOT}/tools/")

    # Set the binary name
    set(FBXCONVERTER_BIN "${TOOLS_DIR}/fbxconverter")

    # Do the export
    add_custom_command(TARGET ${PROJECT_NAME}
        POST_BUILD
        # COMMAND set "PATH=${TOOLS_DIR}/..;%PATH%" # TODO confirm if needed for Win64
        COMMAND "${FBXCONVERTER_BIN}" -o ${SRCDIR} "${SRCDIR}/*.fbx"
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
