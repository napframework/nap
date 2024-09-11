include(${NAP_ROOT}/cmake/nap_utilities.cmake)

project(rttr_core)

set(platform_dir ${CMAKE_CURRENT_SOURCE_DIR}/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH})
set(include_dir ${platform_dir}/include)
if(WIN32)
    set(implib_dir ${platform_dir}/lib)
else()
    set(implib_dir ${platform_dir}/bin)
endif()
set(dll_dir ${platform_dir}/bin)
set(implib ${implib_dir}/${implib_prefix}${PROJECT_NAME}${implib_suffix})
set(dll ${dll_dir}/${dll_prefix}${PROJECT_NAME}${dll_suffix})

add_import_library(${PROJECT_NAME} ${implib} ${dll} ${include_dir})