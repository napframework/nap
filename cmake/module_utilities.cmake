# Use this function to link import targets from module_extra.cmake and
function(target_link_import_library target library)
    target_link_libraries(${target} ${library})
    get_target_property(library_type ${library} TYPE)
    if (${library_type} STREQUAL SHARED_LIBRARY)
        get_target_property(library_path ${library} IMPORTED_LOCATION)
        add_custom_command(
                TARGET ${target} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${library_path}
                ${LIB_DIR})
        install(FILES ${library_path} TYPE LIB OPTIONAL)
    endif()
endfunction()


# Creates a target importing a single shared library
function(add_import_library target_name implib dll include_dir)
    # Resolve symbolic links
    get_filename_component(implib ${implib} REALPATH)
    get_filename_component(dll ${dll} REALPATH)
    get_filename_component(include_dir ${include_dir} REALPATH)

    if (NOT EXISTS ${dll})
        message(WARNING "Dynamic library for ${target_name} not found for this platform. Tried: ${dll}")
        return()
    endif()

    if (NOT EXISTS ${implib})
        message(WARNING "Import library for ${target_name} not found for this platform. Tried: ${implib}")
        return()
    endif()

    if (NOT EXISTS ${include_dir})
        message(WARNING "Include directory for ${target_name} not found for this platform. Tried: ${include_dir}")
        return()
    endif()

    add_library(${target_name} SHARED IMPORTED GLOBAL)
    target_include_directories(${target_name} INTERFACE ${include_dir})
    set_property(TARGET ${target_name} PROPERTY IMPORTED_LOCATION ${dll})
    set_property(TARGET ${target_name} PROPERTY IMPORTED_IMPLIB ${implib})
endfunction()
