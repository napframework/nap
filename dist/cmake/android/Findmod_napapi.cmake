set(MODNAPAPI_LIBS_RELEASE ${NAP_ROOT}/modules/mod_napapi/lib/Release/${ANDROID_ABI}/libmod_napapi.so)
set(MODNAPAPI_LIBS_DEBUG ${NAP_ROOT}/modules/mod_napapi/lib/Debug/${ANDROID_ABI}/libmod_napapi.so)
add_library(mod_napapi INTERFACE)
target_link_libraries(mod_napapi INTERFACE optimized ${MODNAPAPI_LIBS_RELEASE})
target_link_libraries(mod_napapi INTERFACE debug ${MODNAPAPI_LIBS_DEBUG})
set_target_properties(mod_napapi PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${NAP_ROOT}/modules/mod_napapi/include)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(mod_napapi REQUIRED_VARS MODNAPAPI_LIBS_RELEASE MODNAPAPI_LIBS_DEBUG)
