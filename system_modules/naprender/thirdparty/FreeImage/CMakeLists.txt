include(${NAP_ROOT}/cmake/nap_utilities.cmake)

project(FreeImage)

if (WIN32)
    set(platform_dir ${CMAKE_CURRENT_SOURCE_DIR}/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH}/Dist/x64)
    set(include_dir ${platform_dir})
    set(implib ${platform_dir}/FreeImage.lib)
    set(dll ${platform_dir}/FreeImage.dll)
else()
    set(platform_dir ${CMAKE_CURRENT_SOURCE_DIR}/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH})
    set(include_dir ${platform_dir}/include)
    set(lib_dir ${platform_dir}/lib)
    set(implib ${lib_dir}/${implib_prefix}freeimage${implib_suffix})
    set(dll ${lib_dir}/${dll_prefix}freeimage${dll_suffix})
endif()

add_import_library(${PROJECT_NAME} ${implib} ${dll} ${include_dir})