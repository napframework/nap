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
            elseif (ANDROID)
                set(${MODULE_NAME}_RELEASE_LIB ${NAP_ROOT}/modules/${MODULE_NAME}/lib/Release/${ANDROID_ABI}/lib${MODULE_NAME}.so)
                set(${MODULE_NAME}_DEBUG_LIB ${NAP_ROOT}/modules/${MODULE_NAME}/lib/Debug/${ANDROID_ABI}/lib${MODULE_NAME}.so)
                set(${MODULE_NAME}_MODULE_JSON ${NAP_ROOT}/modules/${MODULE_NAME}/module.json)
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
            set_target_properties(${MODULE_NAME} PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${NAP_ROOT}/modules/${MODULE_NAME}/include)

            # Build source groups for our headers maintaining their folder structure
            if(NOT ANDROID)
                create_hierarchical_source_groups_for_files("${module_headers}" ${MODULE_INCLUDE_ROOT} "Modules\\${MODULE_NAME}")
            endif()
        endif(NOT TARGET ${NAP_MODULE})

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
            unset(MODULE_NAME_EXTRA_LIBS)
            include (${MODULE_EXTRA_CMAKE_PATH})
            if(MODULE_NAME_EXTRA_LIBS)
                target_link_libraries(${MODULE_NAME} INTERFACE ${MODULE_NAME_EXTRA_LIBS})
            endif()
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