set(MODNAPAPP_LIBS_RELEASE ${NAP_ROOT}/modules/mod_napapp/lib/Release/${ANDROID_ABI}/libmod_napapp.so)
set(MODNAPAPP_LIBS_DEBUG ${NAP_ROOT}/modules/mod_napapp/lib/Debug/${ANDROID_ABI}/libmod_napapp.so)
add_library(mod_napapp INTERFACE)
target_link_libraries(mod_napapp INTERFACE optimized ${MODNAPAPP_LIBS_RELEASE})
target_link_libraries(mod_napapp INTERFACE debug ${MODNAPAPP_LIBS_DEBUG})
set_target_properties(mod_napapp PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${NAP_ROOT}/modules/mod_napapp/include)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(mod_napapp REQUIRED_VARS MODNAPAPP_LIBS_RELEASE MODNAPAPP_LIBS_DEBUG)