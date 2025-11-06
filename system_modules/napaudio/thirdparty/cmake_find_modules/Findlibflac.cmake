# - Try to find LIBFLAC
# Once done this will define
# LIBFLAC_FOUND - System has LIBFLAC
# LIBFLAC_INCLUDE_DIRS - The LIBFLAC include directories
# LIBFLAC_LIBRARIES - The libraries needed to use LIBFLAC
# LIBFLAC_DEFINITIONS - Compiler switches required for using LIBFLAC
# LIBFLAC_LICENSE_FILES - Files required when package is distributed (Licenses etc.)

include(${NAP_ROOT}/cmake/targetarch.cmake)
target_architecture(ARCH)

find_path(LIBFLAC_DIR
         NAMES
         msvc/x86_64/include/FLAC/all.h
         linux/${ARCH}/include/FLAC/all.h
         HINTS
          ${NAP_ROOT}/system_modules/napaudio/thirdparty/libflac
        )

if (WIN32)
    set(LIBFLAC_LIB_DIR ${LIBFLAC_DIR}/msvc/x86_64/lib)
    set(LIBFLAC_LIBRARIES ${LIBFLAC_LIB_DIR}/FLAC.lib)
    set(LIBFLAC_LIBS_RELEASE_DLL ${LIBFLAC_LIB_DIR}/FLAC.dll)
    set(LIBFLAC_INCLUDE_DIR ${LIBFLAC_DIR}/msvc/x86_64/include)
    set(LIBFLAC_LICENSE_FILES ${LIBFLAC_DIR}/msvc/x86_64/copyright)
else ()
    set(LIBFLAC_LIB_DIR ${LIBFLAC_DIR}/linux/${ARCH}/lib)
    set(LIBFLAC_LIBS_RELEASE_DLL ${LIBFLAC_LIB_DIR}/libFLAC.so)
    set(LIBFLAC_LIBRARIES ${LIBFLAC_LIBS_RELEASE_DLL})
    set(LIBFLAC_INCLUDE_DIR ${LIBFLAC_DIR}/linux/${ARCH}/include)
    set(LIBFLAC_LICENSE_FILES ${LIBFLAC_DIR}/linux/${ARCH}/copyright)
endif ()



include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LIBFLAC_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(libflac REQUIRED_VARS LIBFLAC_DIR LIBFLAC_LIB_DIR LIBFLAC_INCLUDE_DIR)

add_library(libflac SHARED IMPORTED)
set_target_properties(libflac PROPERTIES
                      IMPORTED_CONFIGURATIONS "Debug;Release;MinSizeRel;RelWithDebInfo"
                      IMPORTED_LOCATION_RELEASE ${LIBFLAC_LIBS_RELEASE_DLL}
                      IMPORTED_LOCATION_DEBUG ${LIBFLAC_LIBS_RELEASE_DLL}
                      IMPORTED_LOCATION_MINSIZEREL ${LIBFLAC_LIBS_RELEASE_DLL}
                      IMPORTED_LOCATION_RELWITHDEBINFO ${LIBFLAC_LIBS_RELEASE_DLL}
                      )

if(WIN32)
    set_target_properties(libflac PROPERTIES
                          IMPORTED_IMPLIB ${LIBFLAC_LIBRARIES}
                          )
endif()
