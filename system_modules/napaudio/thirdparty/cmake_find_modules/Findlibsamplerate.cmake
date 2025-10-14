# - Try to find LIBSAMPLERATE
# Once done this will define
# LIBSAMPLERATE_FOUND - System has LIBSAMPLERATE
# LIBSAMPLERATE_INCLUDE_DIRS - The LIBSAMPLERATE include directories
# LIBSAMPLERATE_LIBRARIES - The libraries needed to use LIBSAMPLERATE
# LIBSAMPLERATE_DEFINITIONS - Compiler switches required for using LIBSAMPLERATE
# LIBSAMPLERATE_LICENSE_FILES - Files required when package is distributed (Licenses etc.)

include(${NAP_ROOT}/cmake/targetarch.cmake)
target_architecture(ARCH)

find_path(LIBSAMPLERATE_DIR
         NAMES
         msvc/x86_64/include/samplerate.h
         macos/x86_64/include/samplerate.h
         linux/${ARCH}/include/samplerate.h
         HINTS
          ${NAP_ROOT}/system_modules/napaudio/thirdparty/libsamplerate
        )

if (WIN32)
    set(LIBSAMPLERATE_LIB_DIR ${LIBSAMPLERATE_DIR}/msvc/x86_64/lib)
    set(LIBSAMPLERATE_LIBRARIES ${LIBSAMPLERATE_LIB_DIR}/samplerate.lib)
    set(LIBSAMPLERATE_LIBS_RELEASE_DLL ${LIBSAMPLERATE_LIB_DIR}/samplerate.dll)
    set(LIBSAMPLERATE_INCLUDE_DIR ${LIBSAMPLERATE_DIR}/msvc/x86_64/include)
elseif (APPLE)
    set(LIBSAMPLERATE_LIB_DIR ${LIBSAMPLERATE_DIR}/macos/x86_64/lib)
    set(LIBSAMPLERATE_LIBS_RELEASE_DLL ${LIBSAMPLERATE_LIB_DIR}/libsamplerate.1.dylib)
    set(LIBSAMPLERATE_LIBRARIES ${LIBSAMPLERATE_LIB_DIR}/libsamplerate.1.dylib)
    set(LIBSAMPLERATE_INCLUDE_DIR ${LIBSAMPLERATE_DIR}/macos/x86_64/include)
else ()
    set(LIBSAMPLERATE_LIB_DIR ${LIBSAMPLERATE_DIR}/linux/${ARCH}/lib)
    set(LIBSAMPLERATE_LIBS_RELEASE_DLL ${LIBSAMPLERATE_LIB_DIR}/libsamplerate.so)
    set(LIBSAMPLERATE_LIBRARIES ${LIBSAMPLERATE_LIBS_RELEASE_DLL})
    set(LIBSAMPLERATE_INCLUDE_DIR ${LIBSAMPLERATE_DIR}/linux/${ARCH}/include)
endif ()

find_path(LIBSAMPLERATE_SOURCE_DIR
    NAMES COPYING
    HINTS
    ${LIBSAMPLERATE_DIR}/source
    )
set(LIBSAMPLERATE_LICENSE_FILES ${LIBSAMPLERATE_SOURCE_DIR}/COPYING)

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LIBSAMPLERATE_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(libsamplerate REQUIRED_VARS LIBSAMPLERATE_DIR LIBSAMPLERATE_SOURCE_DIR LIBSAMPLERATE_LIB_DIR LIBSAMPLERATE_INCLUDE_DIR)

add_library(libsamplerate SHARED IMPORTED)
set_target_properties(libsamplerate PROPERTIES
                      IMPORTED_CONFIGURATIONS "Debug;Release;MinSizeRel;RelWithDebInfo"
                      IMPORTED_LOCATION_RELEASE ${LIBSAMPLERATE_LIBS_RELEASE_DLL}
                      IMPORTED_LOCATION_DEBUG ${LIBSAMPLERATE_LIBS_RELEASE_DLL}
                      IMPORTED_LOCATION_MINSIZEREL ${LIBSAMPLERATE_LIBS_RELEASE_DLL}
                      IMPORTED_LOCATION_RELWITHDEBINFO ${LIBSAMPLERATE_LIBS_RELEASE_DLL}
                      )

if(WIN32)
    set_target_properties(libsamplerate PROPERTIES
                          IMPORTED_IMPLIB ${LIBSAMPLERATE_LIBRARIES}
                          )
endif()
