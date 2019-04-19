set(RTTR_DIR ${THIRDPARTY_DIR}/rttr)
add_library(RTTR::Core SHARED IMPORTED)
set_target_properties(RTTR::Core PROPERTIES
        INTERFACE_COMPILE_DEFINITIONS "RTTR_DLL"
        INTERFACE_INCLUDE_DIRECTORIES "${RTTR_DIR}/include"
        )
set_property(TARGET RTTR::Core APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_property(TARGET RTTR::Core APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(RTTR::Core PROPERTIES
        IMPORTED_LOCATION_RELEASE "${RTTR_DIR}/bin/Release/${ANDROID_ABI}/librttr_core.so"
        IMPORTED_SONAME_RELEASE "librttr_core.so"
        IMPORTED_LOCATION_DEBUG "${RTTR_DIR}/bin/Debug/${ANDROID_ABI}/librttr_core_d.so"
        IMPORTED_SONAME_DEBUG "librttr_core_d.so"
        )


include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(RTTR::Core REQUIRED_VARS RTTR_DIR)