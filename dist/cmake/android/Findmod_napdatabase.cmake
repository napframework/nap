set(MODNAPDATABASE_LIBS_RELEASE ${NAP_ROOT}/modules/mod_napdatabase/lib/Release/${ANDROID_ABI}/libmod_napdatabase.so)
set(MODNAPDATABASE_LIBS_DEBUG ${NAP_ROOT}/modules/mod_napdatabase/lib/Debug/${ANDROID_ABI}/libmod_napdatabase.so)
add_library(mod_napdatabase INTERFACE)
target_link_libraries(mod_napdatabase INTERFACE optimized ${MODNAPDATABASE_LIBS_RELEASE})
target_link_libraries(mod_napdatabase INTERFACE debug ${MODNAPDATABASE_LIBS_DEBUG})
set_target_properties(mod_napdatabase PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${NAP_ROOT}/modules/mod_napdatabase/include)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(mod_napdatabase REQUIRED_VARS MODNAPDATABASE_LIBS_RELEASE MODNAPDATABASE_LIBS_DEBUG)