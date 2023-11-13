# Websocketpp Source Directory
find_path(WEBSOCKETPP_ROOT_DIR
    NAMES COPYING
    HINTS ${NAP_ROOT}/system_modules/napwebsocket/thirdparty/websocketpp
    )
set(WEBSOCKETPP_LICENSE_FILES ${WEBSOCKETPP_ROOT_DIR}/COPYING)

# Websocketpp dist directory, header only
find_path(WEBSOCKETPP_DIR
          NAMES include/websocketpp/version.hpp
          HINTS ${WEBSOCKETPP_ROOT_DIR}/install
          )
set(WEBSOCKETPP_INCLUDE_DIR ${WEBSOCKETPP_DIR}/include)
set(OPENSSL_INCLUDE_DIR "${WEBSOCKETPP_ROOT_DIR}/../openssl/include")

# openssl libs
if(UNIX)
    set(LIBCRYPTO_LIB "${WEBSOCKETPP_ROOT_DIR}/../openssl/linux/x86_64/libs/libcrypto.so")
    set(LIBSSL_LIB "${WEBSOCKETPP_ROOT_DIR}/../openssl/linux/x86_64/libs/libssl.so")
    file(GLOB OPENSSL_LIBS "${WEBSOCKETPP_ROOT_DIR}/../openssl/linux/x86_64/libs/libcrypto*${CMAKE_SHARED_LIBRARY_SUFFIX}*"
            "${WEBSOCKETPP_ROOT_DIR}/../openssl/linux/x86_64/libs/libssl*${CMAKE_SHARED_LIBRARY_SUFFIX}*")
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
    set(LIBCRYPTO_LIB "${WEBSOCKETPP_ROOT_DIR}/../openssl/msvc/x86_64/libs/libcrypto.lib")
    set(LIBSSL_LIB "${WEBSOCKETPP_ROOT_DIR}/../openssl/msvc/x86_64/libs/libssl.lib")
    set(LIBCRYPTO_DLL "${WEBSOCKETPP_ROOT_DIR}/../openssl/msvc/x86_64/libs/libssl-1_1-x64.dll")
    set(LIBSSL_DLL "${WEBSOCKETPP_ROOT_DIR}/../openssl/msvc/x86_64/libs/libcrypto-1_1-x64.dll")
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


mark_as_advanced(WEBSOCKETPP_DIR)
mark_as_advanced(WEBSOCKETPP_ROOT_DIR)
mark_as_advanced(WEBSOCKETPP_INCLUDE_DIR)

# promote package for find
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(websocketpp REQUIRED_VARS WEBSOCKETPP_ROOT_DIR WEBSOCKETPP_DIR WEBSOCKETPP_LICENSE_FILES)

# Copy the artnet dynamic linked lib into the build directory
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
