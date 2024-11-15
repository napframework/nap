include(${NAP_ROOT}/cmake/module_utilities.cmake)

get_filename_component(binary_module_name ${CMAKE_CURRENT_SOURCE_DIR} NAME)

project(${binary_module_name})

set(platform_dir ${CMAKE_CURRENT_SOURCE_DIR}/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH})
set(include_dir ${platform_dir}/include)
set(lib_dir ${platform_dir}/lib)
set(implib ${lib_dir}/${implib_prefix}${PROJECT_NAME}${implib_suffix})
set(dll ${lib_dir}/${dll_prefix}${PROJECT_NAME}${dll_suffix})

set(paths_extra_cmake ${CMAKE_CURRENT_SOURCE_DIR}/paths_module_extra.cmake)
if (EXISTS ${paths_extra_cmake})
    include(${paths_extra_cmake})
endif()

declare_import_target(${PROJECT_NAME} ${implib} ${dll} ${include_dir})

# Include module specific logic
set(module_extra_cmake ${CMAKE_CURRENT_SOURCE_DIR}/module_extra.cmake)
if (EXISTS ${module_extra_cmake})
    include(${module_extra_cmake})
endif()

