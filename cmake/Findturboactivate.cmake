# - Try to find TURBOACTIVATE
# Once done this will define
# TURBOACTIVATE_FOUND - System has TURBOACTIVATE
# TURBOACTIVATE_INCLUDE_DIRS - The TURBOACTIVATE include directories
# TURBOACTIVATE_LIBRARIES - The libraries needed to use TURBOACTIVATE

find_path(TURBOACTIVATE_DIR osx/include/TurboActivate.h
HINTS
${THIRDPARTY_DIR}/turboactivate
${CMAKE_CURRENT_LIST_DIR}/../../thirdparty/turboactivate
)

if(WIN32)
    set(TURBOACTIVATE_LIB_DIR ${TURBOACTIVATE_DIR}/msvc/lib)
    set(TURBOACTIVATE_LIB_DIR_DEBUG ${TURBOACTIVATE_DIR}/msvc/lib)
    set(TURBOACTIVATE_INCLUDE_DIRS ${TURBOACTIVATE_DIR}/msvc/include)
    set(TURBOACTIVATE_LIBRARIES ${TURBOACTIVATE_LIB_DIR}/turboactivate.dll)
    set(TURBOACTIVATE_LIBRARIES_DEBUG ${TURBOACTIVATE_LIB_DIR_DEBUG}/turboactivate.dll)
    set(TURBOACTIVATE_IMPLIB_RELEASE ${TURBOACTIVATE_LIB_DIR}/turboactivate.lib)
    set(TURBOACTIVATE_IMPLIB_DEBUG ${TURBOACTIVATE_LIB_DIR_DEBUG}/turboactivate.lib)
elseif(APPLE)
    set(TURBOACTIVATE_LIB_DIR ${TURBOACTIVATE_DIR}/osx/lib)
    set(TURBOACTIVATE_INCLUDE_DIRS ${TURBOACTIVATE_DIR}/osx/include)
    set(TURBOACTIVATE_LIBRARIES ${TURBOACTIVATE_LIB_DIR}/libTurboActivate.dylib)
    set(TURBOACTIVATE_LIBRARIES_DEBUG ${TURBOACTIVATE_LIB_DIR}/libTurboActivate.dylib)
    set(TURBOACTIVATE_IMPLIB_RELEASE ${TURBOACTIVATE_LIB_DIR}/libTurboActivate.dylib) # have to add this here for the script to succeed..
else()
    # set(TURBOACTIVATE_LIB_DIR ${TURBOACTIVATE_DIR}/linux/lib)
    # set(TURBOACTIVATE_INCLUDE_DIRS ${TURBOACTIVATE_DIR}/linux/include)
    # set(TURBOACTIVATE_LIBRARIES ${TURBOACTIVATE_LIB_DIR}/libturboactivate.so)
    # set(TURBOACTIVATE_LIBRARIES_DEBUG ${TURBOACTIVATE_LIB_DIR}/libturboactivate.so)
    # set(TURBOACTIVATE_IMPLIB_RELEASE ${TURBOACTIVATE_LIB_DIR}/libturboactivate.so) # have to add this here for the script to succeed..
endif()


include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(turboactivate DEFAULT_MSG TURBOACTIVATE_LIBRARIES TURBOACTIVATE_IMPLIB_RELEASE TURBOACTIVATE_INCLUDE_DIRS)

add_library(turboactivate SHARED IMPORTED)
set_target_properties(turboactivate PROPERTIES
                      IMPORTED_CONFIGURATIONS "Debug;Release;MinSizeRel;RelWithDebInfo"
                      IMPORTED_LOCATION_RELEASE ${TURBOACTIVATE_LIBRARIES}
                      IMPORTED_LOCATION_DEBUG ${TURBOACTIVATE_LIBRARIES_DEBUG}
                      IMPORTED_LOCATION_MINSIZEREL ${TURBOACTIVATE_LIBRARIES}
                      IMPORTED_LOCATION_RELWITHDEBINFO ${TURBOACTIVATE_LIBRARIES}
                      )

if (WIN32)
	set_target_properties(turboactivate PROPERTIES
    IMPORTED_IMPLIB_RELEASE ${TURBOACTIVATE_IMPLIB_RELEASE}
    IMPORTED_IMPLIB_DEBUG ${TURBOACTIVATE_IMPLIB_DEBUG}
  )
endif()
