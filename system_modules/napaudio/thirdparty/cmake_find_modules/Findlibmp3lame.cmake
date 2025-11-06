# - Try to find LIBMP3LAME
# Once done this will define
# LIBMP3LAME_FOUND - System has LIBMP3LAME
# LIBMP3LAME_INCLUDE_DIRS - The LIBMP3LAME include directories
# LIBMP3LAME_LIBRARIES - The libraries needed to use LIBMP3LAME
# LIBMP3LAME_DEFINITIONS - Compiler switches required for using LIBMP3LAME
# LIBMP3LAME_LICENSE_FILES - Files required when package is distributed (Licenses etc.)

include(${NAP_ROOT}/cmake/targetarch.cmake)
target_architecture(ARCH)

find_path(LIBMP3LAME_DIR
         NAMES
         msvc/x86_64/include/lame/lame.h
         linux/${ARCH}/include/lame/lame.h
         HINTS
          ${NAP_ROOT}/system_modules/napaudio/thirdparty/libmp3lame
        )

if (WIN32)
    set(LIBMP3LAME_LIB_DIR ${LIBMP3LAME_DIR}/msvc/x86_64/lib)
    set(LIBMP3LAME_LIBRARIES ${LIBMP3LAME_LIB_DIR}/libmp3lame.lib)
    set(LIBMP3LAME_LIBS_RELEASE_DLL ${LIBMP3LAME_LIB_DIR}/libmp3lame.dll)
    set(LIBMP3LAME_INCLUDE_DIR ${LIBMP3LAME_DIR}/msvc/x86_64/include)
    set(LIBMP3LAME_LICENSE_FILES ${LIBMP3LAME_DIR}/msvc/x86_64/copyright)
else ()
    set(LIBMP3LAME_LIB_DIR ${LIBMP3LAME_DIR}/linux/${ARCH}/lib)
    set(LIBMP3LAME_LIBS_RELEASE_DLL ${LIBMP3LAME_LIB_DIR}/libmp3lame.so)
    set(LIBMP3LAME_LIBRARIES ${LIBMP3LAME_LIBS_RELEASE_DLL})
    set(LIBMP3LAME_INCLUDE_DIR ${LIBMP3LAME_DIR}/linux/${ARCH}/include)
    set(LIBMP3LAME_LICENSE_FILES ${LIBMP3LAME_DIR}/linux/${ARCH}/copyright)
endif ()



include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LIBMP3LAME_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(libmp3lame REQUIRED_VARS LIBMP3LAME_DIR LIBMP3LAME_LIB_DIR LIBMP3LAME_INCLUDE_DIR)

add_library(libmp3lame SHARED IMPORTED)
set_target_properties(libmp3lame PROPERTIES
                      IMPORTED_CONFIGURATIONS "Debug;Release;MinSizeRel;RelWithDebInfo"
                      IMPORTED_LOCATION_RELEASE ${LIBMP3LAME_LIBS_RELEASE_DLL}
                      IMPORTED_LOCATION_DEBUG ${LIBMP3LAME_LIBS_RELEASE_DLL}
                      IMPORTED_LOCATION_MINSIZEREL ${LIBMP3LAME_LIBS_RELEASE_DLL}
                      IMPORTED_LOCATION_RELWITHDEBINFO ${LIBMP3LAME_LIBS_RELEASE_DLL}
                      )

if(WIN32)
    set_target_properties(libmp3lame PROPERTIES
                          IMPORTED_IMPLIB ${LIBMP3LAME_LIBRARIES}
                          )
endif()
