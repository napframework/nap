include(${NAP_ROOT}/cmake/nap_utilities.cmake)

project(rtmidi)

set(platform_dir ${CMAKE_CURRENT_SOURCE_DIR}/${NAP_THIRDPARTY_PLATFORM_DIR}/${ARCH})
set(include_dir ${platform_dir}/include/rtmidi)
set(lib_dir ${platform_dir}/lib)

if(WIN32)
    set(staticlib ${lib_dir}/${PROJECT_NAME}.lib)
    add_static_import_library(${PROJECT_NAME} ${staticlib} ${include_dir})
else ()
    set(implib ${lib_dir}/${implib_prefix}${PROJECT_NAME}${implib_suffix})
    set(dll ${lib_dir}/${dll_prefix}${PROJECT_NAME}${dll_suffix})
    add_import_library(${PROJECT_NAME} ${implib} ${dll} ${include_dir})
endif ()