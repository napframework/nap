set(NAPCORE_LIBS_RELEASE ${NAP_ROOT}/lib/Release/${ANDROID_ABI}/libnapcore.so)
set(NAPCORE_LIBS_DEBUG ${NAP_ROOT}/lib/Debug/${ANDROID_ABI}/libnapcore.so)
add_library(napcore INTERFACE)
target_link_libraries(napcore INTERFACE optimized ${NAPCORE_LIBS_RELEASE})
target_link_libraries(napcore INTERFACE debug ${NAPCORE_LIBS_DEBUG})
set_target_properties(napcore PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${NAP_ROOT}/include)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(napcore REQUIRED_VARS NAPCORE_LIBS_RELEASE NAPCORE_LIBS_DEBUG)