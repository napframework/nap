set(MODNAPMATH_LIBS_RELEASE ${NAP_ROOT}/modules/mod_napmath/lib/Release/${ANDROID_ABI}/libmod_napmath.so)
set(MODNAPMATH_LIBS_DEBUG ${NAP_ROOT}/modules/mod_napmath/lib/Debug/${ANDROID_ABI}/libmod_napmath.so)
add_library(mod_napmath INTERFACE)
target_link_libraries(mod_napmath INTERFACE optimized ${MODNAPMATH_LIBS_RELEASE})
target_link_libraries(mod_napmath INTERFACE debug ${MODNAPMATH_LIBS_DEBUG})
set_target_properties(mod_napmath PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${NAP_ROOT}/modules/mod_napmath/include)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(mod_napmath REQUIRED_VARS MODNAPMATH_LIBS_RELEASE MODNAPMATH_LIBS_DEBUG)