# - Try to find LIBSNDFILE
# Once done this will define
# LIBSNDFILE_FOUND - System has LIBSNDFILE
# LIBSNDFILE_INCLUDE_DIRS - The LIBSNDFILE include directories
# LIBSNDFILE_LIBRARIES - The libraries needed to use LIBSNDFILE
# LIBSNDFILE_DEFINITIONS - Compiler switches required for using LIBSNDFILE
# LIBSNDFILE_LICENSE_FILES - Files required when package is distributed (Licenses etc.)

include(${NAP_ROOT}/cmake/targetarch.cmake)
target_architecture(ARCH)

find_path(LIBSNDFILE_DIR
         NAMES
         msvc/x86_64/include/sndfile.h
         macos/${ARCH}/include/sndfile.h
         linux/${ARCH}/include/sndfile.h
         HINTS
          ${NAP_ROOT}/system_modules/napaudio/thirdparty/libsndfile
        )

if (WIN32)
    set(LIBSNDFILE_LIB_DIR ${LIBSNDFILE_DIR}/msvc/x86_64/lib)
    set(LIBSNDFILE_LIBRARIES ${LIBSNDFILE_LIB_DIR}/libsndfile-1.lib)
    set(LIBSNDFILE_LIBS_RELEASE_DLL ${LIBSNDFILE_LIB_DIR}/libsndfile-1.dll)
    set(LIBSNDFILE_INCLUDE_DIR ${LIBSNDFILE_DIR}/msvc/x86_64/include)
elseif (APPLE)
    set(LIBSNDFILE_LIB_DIR ${LIBSNDFILE_DIR}/macos/${ARCH}/lib)
    set(LIBSNDFILE_LIBS_RELEASE_DLL ${LIBSNDFILE_LIB_DIR}/libsndfile.1.dylib)
    set(LIBSNDFILE_LIBRARIES ${LIBSNDFILE_LIB_DIR}/libsndfile.1.dylib)
    set(LIBSNDFILE_INCLUDE_DIR ${LIBSNDFILE_DIR}/macos/${ARCH}/include)
else ()
    set(LIBSNDFILE_LIB_DIR ${LIBSNDFILE_DIR}/linux/${ARCH}/lib)
    set(LIBSNDFILE_LIBS_RELEASE_DLL ${LIBSNDFILE_LIB_DIR}/libsndfile.so)
    set(LIBSNDFILE_LIBRARIES ${LIBSNDFILE_LIBS_RELEASE_DLL})
    set(LIBSNDFILE_INCLUDE_DIR ${LIBSNDFILE_DIR}/linux/${ARCH}/include)
endif ()

find_path(LIBSNDFILE_SOURCE_DIR
    NAMES COPYING
    HINTS
    ${LIBSNDFILE_DIR}/source
    )
set(LIBSNDFILE_LICENSE_FILES ${LIBSNDFILE_SOURCE_DIR}/COPYING)

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LIBSNDFILE_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(libsndfile REQUIRED_VARS LIBSNDFILE_DIR LIBSNDFILE_SOURCE_DIR LIBSNDFILE_LIB_DIR LIBSNDFILE_INCLUDE_DIR)

add_library(libsndfile SHARED IMPORTED)
set_target_properties(libsndfile PROPERTIES
                      IMPORTED_CONFIGURATIONS "Debug;Release;MinSizeRel;RelWithDebInfo"
                      IMPORTED_LOCATION_RELEASE ${LIBSNDFILE_LIBS_RELEASE_DLL}
                      IMPORTED_LOCATION_DEBUG ${LIBSNDFILE_LIBS_RELEASE_DLL}
                      IMPORTED_LOCATION_MINSIZEREL ${LIBSNDFILE_LIBS_RELEASE_DLL}
                      IMPORTED_LOCATION_RELWITHDEBINFO ${LIBSNDFILE_LIBS_RELEASE_DLL}
                      )

if(WIN32)
    set_target_properties(libsndfile PROPERTIES
                          IMPORTED_IMPLIB ${LIBSNDFILE_LIBRARIES}
                          )
endif()
