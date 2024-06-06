include(${NAP_ROOT}/cmake/nap_utilities.cmake)

project(mpg123)

set(platform_dir ${CMAKE_CURRENT_SOURCE_DIR}/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH})
set(include_dir ${platform_dir}/include)
set(lib_dir ${platform_dir}/lib)
set(implib ${lib_dir}/lib${PROJECT_NAME}${implib_suffix})
set(dll ${lib_dir}/lib${PROJECT_NAME}${dll_suffix})

add_import_library(${PROJECT_NAME} ${implib} ${dll} ${include_dir})