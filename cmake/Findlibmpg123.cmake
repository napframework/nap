# - Try to find LIBMPG123
# Once done this will define
# LIBMPG123_FOUND - System has LIBMPG123
# LIBMPG123_INCLUDE_DIRS - The LIBMPG123 include directories
# LIBMPG123_LIBRARIES - The libraries needed to use LIBMPG123
# LIBMPG123_DEFINITIONS - Compiler switches required for using LIBMPG123

include(${CMAKE_CURRENT_LIST_DIR}/targetarch.cmake)
target_architecture(ARCH)

find_path(LIBMPG123_DIR src/libmpg123/mpg123.h.in
          HINTS
          ${THIRDPARTY_DIR}/mpg123
          ${CMAKE_CURRENT_LIST_DIR}/../../mpg123
          )

if(WIN32)
    set(LIBMPG123_LIB_DIR ${LIBMPG123_DIR}/install/msvc)
    set(LIBMPG123_LIBRARIES ${LIBMPG123_LIB_DIR}/libmpg123.lib)
    set(LIBMPG123_LIBS_RELEASE_DLL ${LIBMPG123_LIB_DIR}/libmpg123.dll)
    set(LIBMPG123_INCLUDE_DIR ${LIBMPG123_DIR}/install/msvc)

elseif(APPLE)
    set(LIBMPG123_LIB_DIR /usr/local/lib)
    set(LIBMPG123_LIBRARIES ${LIBMPG123_LIB_DIR}/libmpg123.dylib)
    set(LIBMPG123_INCLUDE_DIR /usr/local/include/)

else()
    if(${ARCH} STREQUAL "armv6")
        set(LIBMPG123_LIB_DIR ${LIBMPG123_DIR}/install/linux/bin/arm)
    else()
        set(LIBMPG123_LIB_DIR ${LIBMPG123_DIR}/install/linux/bin)
    endif()
    set(LIBMPG123_LIBRARIES ${LIBMPG123_LIB_DIR}/libmpg123.so)
    set(LIBMPG123_INCLUDE_DIR ${LIBMPG123_DIR}/install/linux/include)
endif()


include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LIBMPG123_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(LIBMPG123 DEFAULT_MSG LIBMPG123_LIBRARIES LIBMPG123_INCLUDE_DIR)

# Copy the portaudio dynamic linked lib into the build directory
macro(copy_mpg123_lib)
    if(WIN32)
        add_library(mpg123lib SHARED IMPORTED)
        set_target_properties(mpg123lib PROPERTIES
                              IMPORTED_CONFIGURATIONS "Debug;Release;MinSizeRel;RelWithDebInfo"
                              IMPORTED_LOCATION_RELEASE ${LIBMPG123_LIBS_RELEASE_DLL}
                              IMPORTED_LOCATION_DEBUG ${LIBMPG123_LIBS_RELEASE_DLL}
                              IMPORTED_LOCATION_MINSIZEREL ${LIBMPG123_LIBS_RELEASE_DLL}
                              IMPORTED_LOCATION_RELWITHDEBINFO ${LIBMPG123_LIBS_RELEASE_DLL}
                              )

        add_custom_command(TARGET ${PROJECT_NAME}
                           POST_BUILD
                           COMMAND ${CMAKE_COMMAND} -E
                           copy $<TARGET_FILE:mpg123lib>
                           $<TARGET_FILE_DIR:${PROJECT_NAME}>/$<TARGET_FILE_NAME:mpg123lib>
                           )
    endif()
endmacro()


