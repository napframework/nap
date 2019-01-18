set(NAPRTTI_LIBS_RELEASE ${NAP_ROOT}/lib/Release/${ANDROID_ABI}/libnaprtti.so)
set(NAPRTTI_LIBS_DEBUG ${NAP_ROOT}/lib/Debug/${ANDROID_ABI}/libnaprtti.so)
add_library(naprtti INTERFACE)
target_link_libraries(naprtti INTERFACE optimized ${NAPRTTI_LIBS_RELEASE})
target_link_libraries(naprtti INTERFACE debug ${NAPRTTI_LIBS_DEBUG})
set_target_properties(naprtti PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${NAP_ROOT}/include)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(naprtti REQUIRED_VARS NAPRTTI_LIBS_RELEASE NAPRTTI_LIBS_DEBUG)