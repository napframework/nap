# - Try to find LIBMPG123
# Once done this will define
# LIBMPG123_FOUND - System has LIBMPG123
# LIBMPG123_INCLUDE_DIRS - The LIBMPG123 include directories
# LIBMPG123_LIBRARIES - The libraries needed to use LIBMPG123
# LIBMPG123_DEFINITIONS - Compiler switches required for using LIBMPG123

include(${CMAKE_CURRENT_LIST_DIR}/targetarch.cmake)
target_architecture(ARCH)

find_path(LIBMPG123_DIR src/libmpg123/mpg123.h
HINTS
${CMAKE_CURRENT_LIST_DIR}/../../thirdparty/mpg123
${CMAKE_CURRENT_LIST_DIR}/../../mpg123
)

if(WIN32)

elseif(APPLE)
    set(LIBMPG123_LIB_DIR /usr/local/lib)
    set(LIBMPG123_LIBRARIES ${LIBMPG123_LIB_DIR}/libmpg123.dylib)
    set(LIBMPG123_INCLUDE_DIR ${LIBMPG123_DIR}/src/libmpg123/)

else()
endif()


include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LIBMPG123_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(LIBMPG123 DEFAULT_MSG LIBMPG123_LIBRARIES LIBMPG123_INCLUDE_DIR)



