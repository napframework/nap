# - Try to find LIBOGG
# Once done this will define
# LIBOGG_FOUND - System has LIBOGG
# LIBOGG_INCLUDE_DIRS - The LIBOGG include directories
# LIBOGG_LIBRARIES - The libraries needed to use LIBOGG
# LIBOGG_DEFINITIONS - Compiler switches required for using LIBOGG
# LIBOGG_LICENSE_FILES - Files required when package is distributed (Licenses etc.)

include(${NAP_ROOT}/cmake/targetarch.cmake)
target_architecture(ARCH)

find_path(LIBOGG_DIR
         NAMES
         msvc/x86_64/include/ogg/ogg.h
         linux/${ARCH}/include/ogg/ogg.h
         HINTS
          ${NAP_ROOT}/system_modules/napaudio/thirdparty/libogg
        )

if (WIN32)
    set(LIBOGG_LIB_DIR ${LIBOGG_DIR}/msvc/x86_64/lib)
    set(LIBOGG_LIBRARIES ${LIBOGG_LIB_DIR}/ogg.lib)
    set(LIBOGG_LIBS_RELEASE_DLL ${LIBOGG_LIB_DIR}/ogg.dll)
    set(LIBOGG_INCLUDE_DIR ${LIBOGG_DIR}/msvc/x86_64/include)
    set(LIBOGG_LICENSE_FILES ${LIBOGG_DIR}/msvc/x86_64/copyright)
else ()
    set(LIBOGG_LIB_DIR ${LIBOGG_DIR}/linux/${ARCH}/lib)
    set(LIBOGG_LIBS_RELEASE_DLL ${LIBOGG_LIB_DIR}/libogg.so)
    set(LIBOGG_LIBRARIES ${LIBOGG_LIBS_RELEASE_DLL})
    set(LIBOGG_INCLUDE_DIR ${LIBOGG_DIR}/linux/${ARCH}/include)
    set(LIBOGG_LICENSE_FILES ${LIBOGG_DIR}/linux/${ARCH}/copyright)
endif ()


include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LIBOGG_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(libogg REQUIRED_VARS LIBOGG_DIR LIBOGG_LIB_DIR LIBOGG_INCLUDE_DIR)

add_library(libogg SHARED IMPORTED)
set_target_properties(libogg PROPERTIES
                      IMPORTED_CONFIGURATIONS "Debug;Release;MinSizeRel;RelWithDebInfo"
                      IMPORTED_LOCATION_RELEASE ${LIBOGG_LIBS_RELEASE_DLL}
                      IMPORTED_LOCATION_DEBUG ${LIBOGG_LIBS_RELEASE_DLL}
                      IMPORTED_LOCATION_MINSIZEREL ${LIBOGG_LIBS_RELEASE_DLL}
                      IMPORTED_LOCATION_RELWITHDEBINFO ${LIBOGG_LIBS_RELEASE_DLL}
                      )

if(WIN32)
    set_target_properties(libogg PROPERTIES
                          IMPORTED_IMPLIB ${LIBOGG_LIBRARIES}
                          )
endif()
