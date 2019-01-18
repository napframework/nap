set(MODNAPINPUT_LIBS_RELEASE ${NAP_ROOT}/modules/mod_napinput/lib/Release/${ANDROID_ABI}/libmod_napinput.so)
set(MODNAPINPUT_LIBS_DEBUG ${NAP_ROOT}/modules/mod_napinput/lib/Debug/${ANDROID_ABI}/libmod_napinput.so)
add_library(mod_napinput INTERFACE)
target_link_libraries(mod_napinput INTERFACE optimized ${MODNAPINPUT_LIBS_RELEASE})
target_link_libraries(mod_napinput INTERFACE debug ${MODNAPINPUT_LIBS_DEBUG})
set_target_properties(mod_napinput PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${NAP_ROOT}/modules/mod_napinput/include)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(mod_napinput REQUIRED_VARS MODNAPINPUT_LIBS_RELEASE MODNAPINPUT_LIBS_DEBUG)