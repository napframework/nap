# - Try to find LIBVORBIS
# Once done this will define
# LIBVORBIS_FOUND - System has LIBVORBIS
# LIBVORBIS_INCLUDE_DIRS - The LIBVORBIS include directories
# LIBVORBIS_LIBRARIES - The libraries needed to use LIBVORBIS
# LIBVORBIS_DEFINITIONS - Compiler switches required for using LIBVORBIS
# LIBVORBIS_LICENSE_FILES - Files required when package is distributed (Licenses etc.)

include(${NAP_ROOT}/cmake/targetarch.cmake)
target_architecture(ARCH)

find_path(LIBVORBIS_DIR
         NAMES
         msvc/x86_64/include/vorbis/codec.h
         linux/${ARCH}/include/vorbis/codec.h
         HINTS
          ${NAP_ROOT}/system_modules/napaudio/thirdparty/libvorbis
        )

if (WIN32)
    set(LIBVORBIS_LIB_DIR ${LIBVORBIS_DIR}/msvc/x86_64/lib)
    set(LIBVORBIS_LIBRARIES ${LIBVORBIS_LIB_DIR}/vorbis.lib ${LIBVORBIS_LIB_DIR}/vorbisenc.lib ${LIBVORBIS_LIB_DIR}/vorbisfile.lib )
    set(LIBVORBIS_LIBS_RELEASE_DLL ${LIBVORBIS_LIB_DIR}/vorbis.dll ${LIBVORBIS_LIB_DIR}/vorbisenc.dll ${LIBVORBIS_LIB_DIR}/vorbisfile.dll)
    set(LIBVORBIS_INCLUDE_DIR ${LIBVORBIS_DIR}/msvc/x86_64/include)
    set(LIBVORBIS_LICENSE_FILES ${LIBVORBIS_DIR}/msvc/x86_64/copyright)

else ()
    set(LIBVORBIS_LIB_DIR ${LIBVORBIS_DIR}/linux/${ARCH}/lib)
    set(LIBVORBIS_LIBS_RELEASE_DLL ${LIBVORBIS_LIB_DIR}/libvorbis.so ${LIBVORBIS_LIB_DIR}/libvorbisenc.so ${LIBVORBIS_LIB_DIR}/libvorbisfile.so)
    set(LIBVORBIS_LIBRARIES ${LIBVORBIS_LIBS_RELEASE_DLL})
    set(LIBVORBIS_INCLUDE_DIR ${LIBVORBIS_DIR}/linux/${ARCH}/include)
    set(LIBVORBIS_LICENSE_FILES ${LIBVORBIS_DIR}/linux/${ARCH}/copyright)
endif ()


include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LIBVORBIS_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(libvorbis REQUIRED_VARS LIBVORBIS_DIR LIBVORBIS_LIB_DIR LIBVORBIS_INCLUDE_DIR)

add_library(libvorbis SHARED IMPORTED)
set_target_properties(libvorbis PROPERTIES
                      IMPORTED_CONFIGURATIONS "Debug;Release;MinSizeRel;RelWithDebInfo"
                      IMPORTED_LOCATION_RELEASE ${LIBVORBIS_LIBS_RELEASE_DLL}
                      IMPORTED_LOCATION_DEBUG ${LIBVORBIS_LIBS_RELEASE_DLL}
                      IMPORTED_LOCATION_MINSIZEREL ${LIBVORBIS_LIBS_RELEASE_DLL}
                      IMPORTED_LOCATION_RELWITHDEBINFO ${LIBVORBIS_LIBS_RELEASE_DLL}
                      )

if(WIN32)
    set_target_properties(libvorbis PROPERTIES
                          IMPORTED_IMPLIB ${LIBVORBIS_LIBRARIES}
                          )
endif()
