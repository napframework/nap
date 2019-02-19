# - Try to find LIBSNDFILE
# Once done this will define
# LIBSNDFILE_FOUND - System has LIBSNDFILE
# LIBSNDFILE_INCLUDE_DIRS - The LIBSNDFILE include directories
# LIBSNDFILE_LIBRARIES - The libraries needed to use LIBSNDFILE
# LIBSNDFILE_DEFINITIONS - Compiler switches required for using LIBSNDFILE

if(NOT ANDROID)
    include(${CMAKE_CURRENT_LIST_DIR}/targetarch.cmake)
    target_architecture(ARCH)
endif()

set(LIBSNDFILE_DIR ${THIRDPARTY_DIR}/libsndfile)
set(LIBSNDFILE_LIB_DIR ${LIBSNDFILE_DIR}/lib)

if (WIN32)
    set(LIBSNDFILE_LIBRARIES ${LIBSNDFILE_LIB_DIR}/libsndfile-1.lib)
    set(LIBSNDFILE_LIBS_RELEASE_DLL ${LIBSNDFILE_DIR}/bin/libsndfile-1.dll)
elseif (APPLE)
    set(LIBSNDFILE_LIBS_RELEASE_DLL ${LIBSNDFILE_LIB_DIR}/libsndfile.1.dylib)
    set(LIBSNDFILE_LIBRARIES ${LIBSNDFILE_LIBS_RELEASE_DLL})
elseif (ANDROID)
    set(LIBSNDFILE_LIBS_RELEASE_DLL ${LIBSNDFILE_LIB_DIR}/${ANDROID_ABI}/libsndfile.so)
    set(LIBSNDFILE_LIBRARIES ${LIBSNDFILE_LIBS_RELEASE_DLL})
else ()
    set(LIBSNDFILE_LIBS_RELEASE_DLL ${LIBSNDFILE_LIB_DIR}/libsndfile.so)
    set(LIBSNDFILE_LIBRARIES ${LIBSNDFILE_LIBS_RELEASE_DLL})
endif ()

set(LIBSNDFILE_INCLUDE_DIR ${LIBSNDFILE_DIR}/include)

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LIBSNDFILE_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(libsndfile REQUIRED_VARS LIBSNDFILE_LIBRARIES LIBSNDFILE_INCLUDE_DIR LIBSNDFILE_DIR)

add_library(libsndfile SHARED IMPORTED)
set_target_properties(libsndfile PROPERTIES
                      IMPORTED_CONFIGURATIONS "Debug;Release"
                      IMPORTED_LOCATION_RELEASE ${LIBSNDFILE_LIBS_RELEASE_DLL}
                      IMPORTED_LOCATION_DEBUG ${LIBSNDFILE_LIBS_RELEASE_DLL}
                      )

if(WIN32)
    set_target_properties(libsndfile PROPERTIES
                          IMPORTED_IMPLIB ${LIBSNDFILE_LIBRARIES}
                          )
endif()