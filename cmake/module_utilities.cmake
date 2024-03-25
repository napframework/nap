# Use this function to link import libraries from module_extra.cmake and other included cmake files
# target: The target to link the library to
# library: An import target that links to a DLL
function(target_link_import_library target library)
    if (NOT TARGET ${library})
        message(FATAL_ERROR "Library dependency ${library} not found for building ${target}")
    endif()
    get_target_property(library_path ${library} IMPORTED_LOCATION)
    get_filename_component(library_file_name ${library_path} NAME)
    get_target_property(include_dir ${library} INCLUDE_DIRECTORIES)
    get_target_property(library_type ${library} TYPE)

    if (EXISTS ${include_dir})
        target_include_directories(${target} PUBLIC ${include_dir})
    endif ()

    if (${library_type} STREQUAL SHARED_LIBRARY)
        if(LINUX)
            target_link_libraries(${target} ${library_path})
        else ()
            target_link_libraries(${target} ${library})
        endif()
        add_custom_command(
                TARGET ${target} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${library_path}
                ${LIB_DIR})
        install(FILES ${LIB_DIR}/${library_file_name} TYPE LIB OPTIONAL)
    else ()
        # Static libs are linked with absolute path
        target_link_libraries(${target} ${library_path})
    endif()
endfunction()


# Creates a target importing a single shared library
# target_name: Name of the new target
# implib: Path to the implib of the shared library: on Windows this is the .lib file, on linux and MacOS it's the same file as the dll.
# dll: Path to the DLL part of the shared library: on Windows the .dll file, on linux the .so file, on MacOS the .dylib.
# include_dir: Path to the include directories for the library.
function(add_import_library target_name implib dll include_dir)
    # Resolve symbolic links
    get_filename_component(implib ${implib} REALPATH)
    get_filename_component(dll ${dll} REALPATH)
    get_filename_component(include_dir ${include_dir} REALPATH)

    message(STATUS ${dll})

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
    #    target_include_directories(${target_name} INTERFACE ${include_dir})
    set_property(TARGET ${target_name} PROPERTY IMPORTED_LOCATION ${dll})
    set_property(TARGET ${target_name} PROPERTY IMPORTED_IMPLIB ${implib})
    set_property(TARGET ${target_name} PROPERTY INCLUDE_DIRECTORIES ${include_dir})

    if (UNIX)
        get_filename_component(library_name ${dll} NAME)
        if (APPLE)
            execute_process(COMMAND install_name_tool -id
                    @rpath/${library_name}
                    ${dll}
                    RESULT_VARIABLE EXIT_CODE)
            if(NOT ${EXIT_CODE} EQUAL 0)
                message(FATAL_ERROR "Failed to set RPATH on ${library_name} using install_name_tool -id.")
            endif()
        else ()
            execute_process(COMMAND patchelf --set-soname
                    ${library_name}
                    ${dll}
                    RESULT_VARIABLE EXIT_CODE)
            if(NOT ${EXIT_CODE} EQUAL 0)
                message(FATAL_ERROR "Failed to set RPATH on ${library_name} using patchelf. Is patchelf installed?")
            endif()
        endif()

    endif()
endfunction()


# Creates a target importing one static library
# target_name: Name of the new target
# implib: Path to the to the shared library: on Windows this is a .lib file, on linux and MacOS it's a .a file
# include_dir: Path to the include directories for the library.
function(add_static_import_library target_name implib include_dir)
    # Resolve symbolic links
    get_filename_component(implib ${implib} REALPATH)
    get_filename_component(include_dir ${include_dir} REALPATH)

    if (NOT EXISTS ${implib})
        message(WARNING "Import library for ${target_name} not found for this platform. Tried: ${implib}")
        return()
    endif()

    if (NOT EXISTS ${include_dir})
        message(WARNING "Include directory for ${target_name} not found for this platform. Tried: ${include_dir}")
        return()
    endif()

    add_library(${target_name} STATIC IMPORTED GLOBAL)
    target_include_directories(${target_name} INTERFACE ${include_dir})
    set_property(TARGET ${target_name} PROPERTY IMPORTED_LOCATION ${implib})
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


function(add_subdirectory_apps_and_modules subdirectory)
    set(directory ${CMAKE_CURRENT_SOURCE_DIR}/${subdirectory})
    file(GLOB children ${directory}/*)
    foreach(child ${children})
        if (IS_DIRECTORY ${child})
            add_module_from_dir(${child})
            add_app_from_dir(${child})
            file(RELATIVE_PATH child_subdirectory ${CMAKE_CURRENT_SOURCE_DIR} ${child})
            add_subdirectory_apps_and_modules(${child_subdirectory})
        endif ()
    endforeach ()
endfunction()


function(add_module_from_dir module_dir)
    if (EXISTS ${module_dir}/module.json)
        # We have a module!
        # Create CMakeLists.txt
        set(cmake_list ${module_dir}/CMakeLists.txt)
        if (NOT EXISTS ${cmake_list})
            file(WRITE ${cmake_list}
                    [=[include(${NAP_ROOT}/cmake/nap_module.cmake)]=]
            )
        endif()
        file(RELATIVE_PATH module_subdirectory ${CMAKE_CURRENT_SOURCE_DIR} ${module_dir})
        add_subdirectory(${module_subdirectory})
    endif()
endfunction()


function(add_app_from_dir app_dir)
    if (EXISTS ${app_dir}/app.json)
        # We have an app!
        # Create CMakeLists.txt
        set(cmake_list ${app_dir}/CMakeLists.txt)
        if (NOT EXISTS ${cmake_list})
            file(WRITE ${cmake_list}
                    [=[include(${NAP_ROOT}/cmake/nap_app.cmake)]=]
            )
        endif()
        file(RELATIVE_PATH app_subdirectory ${CMAKE_CURRENT_SOURCE_DIR} ${app_dir})
        add_subdirectory(${app_subdirectory})
    endif()
endfunction()