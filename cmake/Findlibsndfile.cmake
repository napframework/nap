# - Try to find LIBSNDFILE
# Once done this will define
# LIBSNDFILE_FOUND - System has LIBSNDFILE
# LIBSNDFILE_INCLUDE_DIRS - The LIBSNDFILE include directories
# LIBSNDFILE_LIBRARIES - The libraries needed to use LIBSNDFILE
# LIBSNDFILE_DEFINITIONS - Compiler switches required for using LIBSNDFILE

include(${CMAKE_CURRENT_LIST_DIR}/targetarch.cmake)
target_architecture(ARCH)

find_path(LIBSNDFILE_DIR src/sndfile.h
        HINTS
        ${THIRDPARTY_DIR}/libsndfile
        ${CMAKE_CURRENT_LIST_DIR}/../../libsndfile
        )

if (WIN32)
    set(LIBSNDFILE_LIB_DIR ${LIBSNDFILE_DIR}/msvc64)
    set(LIBSNDFILE_LIBRARIES ${LIBSNDFILE_LIB_DIR}/libsndfile-1.lib)
    set(LIBSNDFILE_LIBS_RELEASE_DLL ${LIBSNDFILE_LIB_DIR}/libsndfile-1.dll)
elseif (APPLE)
    # TODO why is libsndfile static on OSX and shared on others?
    set(LIBSNDFILE_LIB_DIR ${LIBSNDFILE_DIR}/xcode)
    set(LIBSNDFILE_LIBS_RELEASE_DLL ${LIBSNDFILE_LIB_DIR}/libsndfile.a)
    set(LIBSNDFILE_LIBRARIES ${LIBSNDFILE_LIB_DIR}/libsndfile.a)
else ()
    if (${ARCH} STREQUAL "armv6")
        set(LIBSNDFILE_LIB_DIR ${LIBSNDFILE_DIR}/linux/arm)
    else ()
        set(LIBSNDFILE_LIB_DIR ${LIBSNDFILE_DIR}/linux/install/lib)
    endif ()
    set(LIBSNDFILE_LIBS_RELEASE_DLL ${LIBSNDFILE_LIB_DIR}/libsndfile.so)
    set(LIBSNDFILE_LIBRARIES ${LIBSNDFILE_LIBS_RELEASE_DLL})
endif ()

set(LIBSNDFILE_INCLUDE_DIR ${LIBSNDFILE_DIR}/src)

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LIBSNDFILE_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(libsndfile REQUIRED_VARS LIBSNDFILE_LIBRARIES LIBSNDFILE_INCLUDE_DIR LIBSNDFILE_DIR)

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

