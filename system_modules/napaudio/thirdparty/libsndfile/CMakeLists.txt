include(${NAP_ROOT}/cmake/module_utilities.cmake)

project(libsndfile)

set(platform_dir ${CMAKE_CURRENT_SOURCE_DIR}/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH})
set(include_dir ${platform_dir}/include)
set(lib_dir ${platform_dir}/lib)
if (WIN32)
    set(implib ${lib_dir}/libsndfile-1.lib)
    set(dll ${lib_dir}/libsndfile-1.dll)
else ()
    set(implib ${lib_dir}/${PROJECT_NAME}${implib_suffix})
    set(dll ${lib_dir}/${PROJECT_NAME}${dll_suffix})
endif ()

add_import_library(${PROJECT_NAME} ${implib} ${dll} ${include_dir})