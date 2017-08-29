# - Try to find LIBSNDFILE
# Once done this will define
# LIBSNDFILE_FOUND - System has LIBSNDFILE
# LIBSNDFILE_INCLUDE_DIRS - The LIBSNDFILE include directories
# LIBSNDFILE_LIBRARIES - The libraries needed to use LIBSNDFILE
# LIBSNDFILE_DEFINITIONS - Compiler switches required for using LIBSNDFILE

if(WIN32)
    include(${CMAKE_CURRENT_LIST_DIR}/targetarch.cmake)
    target_architecture(ARCH)

    find_path(LIBSNDFILE_DIR src/sndfile.h
            HINTS
            ${CMAKE_CURRENT_LIST_DIR}/../../thirdparty/libsndfile
            ${CMAKE_CURRENT_LIST_DIR}/../../libsndfile
            )

    set(LIBSNDFILE_LIB_DIR ${LIBSNDFILE_DIR}/msvc64)
    set(LIBSNDFILE_LIBRARIES ${LIBSNDFILE_LIB_DIR}/libsndfile-1.lib)

    include(FindPackageHandleStandardArgs)
    # handle the QUIETLY and REQUIRED arguments and set LIBSNDFILE_FOUND to TRUE
    # if all listed variables are TRUE
    find_package_handle_standard_args(LIBSNDFILE DEFAULT_MSG LIBSNDFILE_LIBRARIES LIBSNDFILE_INCLUDE_DIR)

elseif(APPLE)
    include(${CMAKE_CURRENT_LIST_DIR}/targetarch.cmake)
    target_architecture(ARCH)

    find_path(LIBSNDFILE_DIR src/sndfile.h
    HINTS
    ${CMAKE_CURRENT_LIST_DIR}/../../thirdparty/libsndfile
    ${CMAKE_CURRENT_LIST_DIR}/../../libsndfile
    )

    set(LIBSNDFILE_LIB_DIR ${LIBSNDFILE_DIR}/usr/local/lib)
    set(LIBSNDFILE_LIBRARIES ${LIBSNDFILE_LIB_DIR}/libsndfile.a)

    set(LIBSNDFILE_INCLUDE_DIR ${LIBSNDFILE_DIR}/src)

    include(FindPackageHandleStandardArgs)
    # handle the QUIETLY and REQUIRED arguments and set LIBSNDFILE_FOUND to TRUE
    # if all listed variables are TRUE
    find_package_handle_standard_args(LIBSNDFILE DEFAULT_MSG LIBSNDFILE_LIBRARIES LIBSNDFILE_INCLUDE_DIR)

endif()


