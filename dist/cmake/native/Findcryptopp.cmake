find_path(
        CRYPTOPP_DIR
        NAMES include/dll.h
        HINTS ${THIRDPARTY_DIR}/cryptopp
)
set(CRYPTOPP_LIB_DIR ${CRYPTOPP_DIR}/lib)
set(CRYPTOPP_INCLUDE_DIRS ${CRYPTOPP_DIR}/include)
if(WIN32)
    set(CRYPTOPP_LIB_DEBUG ${CRYPTOPP_LIB_DIR}/Debug/cryptopp.lib)
    set(CRYPTOPP_DLL_DEBUG ${CRYPTOPP_LIB_DIR}/Debug/cryptopp.dll)
    set(CRYPTOPP_LIB_RELEASE ${CRYPTOPP_LIB_DIR}/Release/cryptopp.lib)
    set(CRYPTOPP_DLL_RELEASE ${CRYPTOPP_LIB_DIR}/Release/cryptopp.dll)
elseif(APPLE)
    set(CRYPTOPP_LIB_RELEASE ${CRYPTOPP_LIB_DIR}/libcryptopp.a)
    set(CRYPTOPP_LIB_DEBUG ${CRYPTOPP_LIB_RELEASE})
endif()

mark_as_advanced(CRYPTOPP_INCLUDE_DIRS CRYPTOPP_LIB_DIR)
mark_as_advanced(CRYPTOPP_LIB_DEBUG CRYPTOPP_DLL_DEBUG)
mark_as_advanced(CRYPTOPP_LIB_RELEASE CRYPTOPP_DLL_RELEASE)

 
if(WIN32)
    add_library(cryptopp SHARED IMPORTED)
    set_target_properties(cryptopp PROPERTIES
                          IMPORTED_CONFIGURATIONS "Debug;Release"
                          IMPORTED_LOCATION_RELEASE ${CRYPTOPP_DLL_RELEASE}
                          IMPORTED_LOCATION_DEBUG ${CRYPTOPP_DLL_DEBUG}
                          )

    set_target_properties(cryptopp PROPERTIES
                          IMPORTED_IMPLIB_RELEASE ${CRYPTOPP_LIB_RELEASE}
                          IMPORTED_IMPLIB_DEBUG ${CRYPTOPP_LIB_DEBUG}
                          )
else()
    add_library(cryptopp STATIC IMPORTED)
    set_target_properties(cryptopp PROPERTIES
                          IMPORTED_CONFIGURATIONS "Debug;Release"
                          IMPORTED_LOCATION_RELEASE ${CRYPTOPP_LIB_RELEASE}
                          IMPORTED_LOCATION_DEBUG ${CRYPTOPP_LIB_DEBUG}
                          )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(cryptopp REQUIRED_VARS CRYPTOPP_INCLUDE_DIRS CRYPTOPP_LIB_DEBUG CRYPTOPP_LIB_RELEASE)
 
# Copy the cryptopp dynamic linked lib into the build directory
macro(copy_cryptopp_dll)
    add_custom_command(
            TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different 
                    $<TARGET_FILE:cryptopp> 
                    "$<TARGET_PROPERTY:${PROJECT_NAME},RUNTIME_OUTPUT_DIRECTORY_$<UPPER_CASE:$<CONFIG>>>"
    )
endmacro()
