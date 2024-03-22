# Use this function to link import targets from module_extra.cmake and
function(target_link_import_library target library)
    if (NOT TARGET ${library})
        message(FATAL_ERROR "Library dependency ${library} not found for building ${target}")
    endif()
    target_link_libraries(${target} ${library})
    get_target_property(library_type ${library} TYPE)
    if (${library_type} STREQUAL SHARED_LIBRARY)
        get_target_property(library_path ${library} IMPORTED_LOCATION)
#        set(library_path $<TARGET_FILE:${library}>)
        get_filename_component(library_file_name ${library_path} NAME)
        add_custom_command(
                TARGET ${target} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${library_path}
                ${LIB_DIR})
        install(FILES ${LIB_DIR}/${library_file_name} TYPE LIB OPTIONAL)
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


# Add a source directory to an already defined target
# NAME of the source group in the IDE
# DIR directory of the source files relative to the project directory
# ARGN additional optional arguments are regex expressions to filter from the file list
function(add_source_dir NAME DIR)
    # Collect source files in directory
    file(GLOB SOURCES ${DIR}/*.cpp ${DIR}/*.h ${DIR}/*.hpp)

    # Loop through optional arguments and exclude them from the sources list
    foreach(element ${ARGN})
        list(FILTER SOURCES EXCLUDE REGEX ${element})
    endforeach()

    # Create source group for IDE
    source_group(${NAME} FILES ${SOURCES})

    # Add sources to target
    target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})
endfunction()
