# - Try to find LIBOPUS
# Once done this will define
# LIBOPUS_FOUND - System has LIBOPUS
# LIBOPUS_INCLUDE_DIRS - The LIBOPUS include directories
# LIBOPUS_LIBRARIES - The libraries needed to use LIBOPUS
# LIBOPUS_DEFINITIONS - Compiler switches required for using LIBOPUS
# LIBOPUS_LICENSE_FILES - Files required when package is distributed (Licenses etc.)

include(${NAP_ROOT}/cmake/targetarch.cmake)
target_architecture(ARCH)

find_path(LIBOPUS_DIR
         NAMES
         msvc/x86_64/include/opus/opus.h
         linux/${ARCH}/include/opus/opus.h
         HINTS
          ${NAP_ROOT}/system_modules/napaudio/thirdparty/libopus
        )

if (WIN32)
    set(LIBOPUS_LIB_DIR ${LIBOPUS_DIR}/msvc/x86_64/lib)
    set(LIBOPUS_LIBRARIES ${LIBOPUS_LIB_DIR}/opus.lib)
    set(LIBOPUS_LIBS_RELEASE_DLL ${LIBOPUS_LIB_DIR}/opus.dll)
    set(LIBOPUS_INCLUDE_DIR ${LIBOPUS_DIR}/msvc/x86_64/include)
    set(LIBOPUS_LICENSE_FILES ${LIBOPUS_DIR}/msvc/x86_64/copyright)

else ()
    set(LIBOPUS_LIB_DIR ${LIBOPUS_DIR}/linux/${ARCH}/lib)
    set(LIBOPUS_LIBS_RELEASE_DLL ${LIBOPUS_LIB_DIR}/libopus.so)
    set(LIBOPUS_LIBRARIES ${LIBOPUS_LIBS_RELEASE_DLL})
    set(LIBOPUS_INCLUDE_DIR ${LIBOPUS_DIR}/linux/${ARCH}/include)
    set(LIBOPUS_LICENSE_FILES ${LIBOPUS_DIR}/linux/${ARCH}/copyright)

endif ()


include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LIBOPUS_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(libopus REQUIRED_VARS LIBOPUS_DIR LIBOPUS_LIB_DIR LIBOPUS_INCLUDE_DIR)

add_library(libopus SHARED IMPORTED)
set_target_properties(libopus PROPERTIES
                      IMPORTED_CONFIGURATIONS "Debug;Release;MinSizeRel;RelWithDebInfo"
                      IMPORTED_LOCATION_RELEASE ${LIBOPUS_LIBS_RELEASE_DLL}
                      IMPORTED_LOCATION_DEBUG ${LIBOPUS_LIBS_RELEASE_DLL}
                      IMPORTED_LOCATION_MINSIZEREL ${LIBOPUS_LIBS_RELEASE_DLL}
                      IMPORTED_LOCATION_RELWITHDEBINFO ${LIBOPUS_LIBS_RELEASE_DLL}
                      )

if(WIN32)
    set_target_properties(libopus PROPERTIES
                          IMPORTED_IMPLIB ${LIBOPUS_LIBRARIES}
                          )
endif()
