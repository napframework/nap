# - Try to find LIBMPG123
# Once done this will define
# LIBMPG123_FOUND - System has LIBMPG123
# LIBMPG123_INCLUDE_DIRS - The LIBMPG123 include directories
# LIBMPG123_LIBRARIES - The libraries needed to use LIBMPG123
# LIBMPG123_DEFINITIONS - Compiler switches required for using LIBMPG123
# LIBMPG123_DIST_FILES - Files required when distributed (License etc.)

include(${CMAKE_CURRENT_LIST_DIR}/targetarch.cmake)
target_architecture(ARCH)

find_path(LIBMPG123_DIR
          NAMES 
          msvc/x86_64/include/mpg123.h
          macos/x86_64/include/mpg123.h
          linux/${ARCH}/include/mpg123.h
          HINTS
          ${THIRDPARTY_DIR}/mpg123
          ${CMAKE_CURRENT_LIST_DIR}/../../mpg123
          )

if(WIN32)
    set(LIBMPG123_LIB_DIR ${LIBMPG123_DIR}/msvc/x86_64/lib)
    set(LIBMPG123_LIBRARIES ${LIBMPG123_LIB_DIR}/libmpg123.lib)
    set(LIBMPG123_LIBS_RELEASE_DLL ${LIBMPG123_LIB_DIR}/libmpg123.dll)
    set(LIBMPG123_INCLUDE_DIR ${LIBMPG123_DIR}/msvc/x86_64/include)
elseif(APPLE)
    set(LIBMPG123_LIB_DIR ${LIBMPG123_DIR}/macos/x86_64/lib)
    set(LIBMPG123_LIBS_RELEASE_DLL ${LIBMPG123_LIB_DIR}/libmpg123.dylib)
    set(LIBMPG123_LIBRARIES ${LIBMPG123_LIBS_RELEASE_DLL})
    set(LIBMPG123_INCLUDE_DIR ${LIBMPG123_DIR}/macos/x86_64/include)
else()
    set(LIBMPG123_DIR ${LIBMPG123_DIR}/linux/${ARCH})
    set(LIBMPG123_LIB_DIR ${LIBMPG123_DIR}/lib)
    set(LIBMPG123_LIBS_RELEASE_DLL ${LIBMPG123_LIB_DIR}/libmpg123.so)
    set(LIBMPG123_LIBRARIES ${LIBMPG123_LIBS_RELEASE_DLL})
    set(LIBMPG123_INCLUDE_DIR ${LIBMPG123_DIR}/include)
endif()

set(LIBMPG123_DIST_FILES ${THIRDPARTY_DIR}/mpg123/source/COPYING)

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LIBMPG123_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(libmpg123 REQUIRED_VARS LIBMPG123_DIR LIBMPG123_LIBRARIES LIBMPG123_INCLUDE_DIR LIBMPG123_DIST_FILES)

add_library(libmpg123 SHARED IMPORTED)
set_target_properties(libmpg123 PROPERTIES
                      IMPORTED_CONFIGURATIONS "Debug;Release;MinSizeRel;RelWithDebInfo"
                      IMPORTED_LOCATION_RELEASE ${LIBMPG123_LIBS_RELEASE_DLL}
                      IMPORTED_LOCATION_DEBUG ${LIBMPG123_LIBS_RELEASE_DLL}
                      IMPORTED_LOCATION_MINSIZEREL ${LIBMPG123_LIBS_RELEASE_DLL}
                      IMPORTED_LOCATION_RELWITHDEBINFO ${LIBMPG123_LIBS_RELEASE_DLL}
                      )

if(WIN32)
    set_target_properties(libmpg123 PROPERTIES
                          IMPORTED_IMPLIB ${LIBMPG123_LIBRARIES}
                          )
endif()

# Copy the libmpg123 dynamic linked lib into the build directory
macro(copy_mpg123_lib)
    if(WIN32)
        add_custom_command(TARGET ${PROJECT_NAME}
                           POST_BUILD
                           COMMAND ${CMAKE_COMMAND} -E copy_if_different 
                            $<TARGET_FILE:libmpg123>
                           "$<TARGET_PROPERTY:${PROJECT_NAME},RUNTIME_OUTPUT_DIRECTORY_$<UPPER_CASE:$<CONFIG>>>"
                           )
    endif()
endmacro()


