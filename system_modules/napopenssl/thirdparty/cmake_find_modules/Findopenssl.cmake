# OpenSSL Directory
find_path(OPENSSL_ROOT_DIR
    NAMES LICENSE
    HINTS ${NAP_ROOT}/system_modules/napopenssl/thirdparty/openssl)
set(OPENSSL_LICENSE_FILES ${OPENSSL_ROOT_DIR}/LICENSE)

# Include dir
set(OPENSSL_INCLUDE_DIR "${OPENSSL_ROOT_DIR}/include")

# openssl libs
if(UNIX)
    set(LIBCRYPTO_LIB "${OPENSSL_ROOT_DIR}/linux/${ARCH}/lib/libcrypto.so")
    set(LIBSSL_LIB "${OPENSSL_ROOT_DIR}/linux/${ARCH}/lib/libssl.so")
    file(GLOB OPENSSL_LIBS "${OPENSSL_ROOT_DIR}/linux/${ARCH}/lib/libcrypto*${CMAKE_SHARED_LIBRARY_SUFFIX}*"
            "${OPENSSL_ROOT_DIR}/linux/x86_64/libs/libssl*${CMAKE_SHARED_LIBRARY_SUFFIX}*")
    add_library(libcrypto SHARED IMPORTED)
    set_target_properties(libcrypto PROPERTIES
            IMPORTED_CONFIGURATIONS "Debug;Release"
            IMPORTED_LOCATION_RELEASE ${LIBCRYPTO_LIB}
            IMPORTED_LOCATION_DEBUG ${LIBCRYPTO_LIB}
    )
    add_library(libssl SHARED IMPORTED)
    set_target_properties(libssl PROPERTIES
            IMPORTED_CONFIGURATIONS "Debug;Release"
            IMPORTED_LOCATION_RELEASE ${LIBSSL_LIB}
            IMPORTED_LOCATION_DEBUG ${LIBSSL_LIB}
    )
else (WIN32)
    set(LIBCRYPTO_LIB "${OPENSSL_ROOT_DIR}/msvc/x86_64/lib/libcrypto.lib")
    set(LIBSSL_LIB "${OPENSSL_ROOT_DIR}/msvc/x86_64/lib/libssl.lib")
    set(LIBCRYPTO_DLL "${OPENSSL_ROOT_DIR}/msvc/x86_64/lib/libcrypto-3-x64.dll")
    set(LIBSSL_DLL "${OPENSSL_ROOT_DIR}/msvc/x86_64/lib/libssl-3-x64.dll")
    file(GLOB OPENSSL_LIBS "${LIBCRYPTO_LIB}" "${LIBSSL_LIB}")
    add_library(libcrypto SHARED IMPORTED)
    set_target_properties(libcrypto PROPERTIES
            IMPORTED_CONFIGURATIONS "Debug;Release"
            IMPORTED_LOCATION_RELEASE ${LIBCRYPTO_DLL}
            IMPORTED_LOCATION_DEBUG ${LIBCRYPTO_DLL}
    )
    add_library(libssl SHARED IMPORTED)
    set_target_properties(libssl PROPERTIES
            IMPORTED_CONFIGURATIONS "Debug;Release"
            IMPORTED_LOCATION_RELEASE ${LIBSSL_DLL}
            IMPORTED_LOCATION_DEBUG ${LIBSSL_DLL}
    )

    set_target_properties(libssl PROPERTIES
        IMPORTED_IMPLIB_RELEASE ${LIBSSL_LIB}
        IMPORTED_IMPLIB_DEBUG ${LIBSSL_LIB}
    )

    set_target_properties(libcrypto PROPERTIES
        IMPORTED_IMPLIB_RELEASE ${LIBCRYPTO_LIB}
        IMPORTED_IMPLIB_DEBUG ${LIBCRYPTO_LIB}
    )
endif ()


mark_as_advanced(OPENSSL_ROOT_DIR)
mark_as_advanced(OPENSSL_INCLUDE_DIR)

# promote package for find
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(openssl REQUIRED_VARS OPENSSL_ROOT_DIR OPENSSL_INCLUDE_DIR)

# Copy the openssl dynamic linked lib into the build directory
macro(copy_openssl_dll)
    add_custom_command(TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:libssl>
            "$<TARGET_PROPERTY:${PROJECT_NAME},RUNTIME_OUTPUT_DIRECTORY_$<UPPER_CASE:$<CONFIG>>>"
    )
    add_custom_command(TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:libcrypto>
            "$<TARGET_PROPERTY:${PROJECT_NAME},RUNTIME_OUTPUT_DIRECTORY_$<UPPER_CASE:$<CONFIG>>>"
    )
endmacro()
