# - Try to find TURBOACTIVATE
# Once done this will define
# TURBOACTIVATE_FOUND - System has TURBOACTIVATE
# TURBOACTIVATE_INCLUDE_DIRS - The TURBOACTIVATE include directories
# TURBOACTIVATE_LIBRARIES - The libraries needed to use TURBOACTIVATE

include(${CMAKE_CURRENT_LIST_DIR}/targetarch.cmake)
target_architecture(ARCH)

set(TURBOACTIVATE_DIR ${THIRDPARTY_DIR}/turboactivate)
set(TURBOACTIVATE_LIB_DIR ${TURBOACTIVATE_DIR}/lib)

if (WIN32)
    set(TURBOACTIVATE_LIBRARIES ${TURBOACTIVATE_LIB_DIR}/TurboActivate.lib)
    set(TURBOACTIVATE_LIBS_RELEASE_DLL ${TURBOACTIVATE_DIR}/bin/TurboActivate.dll)
elseif (APPLE)
    set(TURBOACTIVATE_LIBS_RELEASE_DLL ${TURBOACTIVATE_LIB_DIR}/libTurboActivate.dylib)
    set(TURBOACTIVATE_LIBRARIES ${TURBOACTIVATE_LIBS_RELEASE_DLL})
else ()
    set(TURBOACTIVATE_LIBS_RELEASE_DLL ${TURBOACTIVATE_LIB_DIR}/libTurboActivate.so)
    set(TURBOACTIVATE_LIBRARIES ${TURBOACTIVATE_LIBS_RELEASE_DLL})
endif ()


set(TURBOACTIVATE_INCLUDE_DIRS ${TURBOACTIVATE_DIR}/include)

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set TURBOACTIVATE_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(turboactivate REQUIRED_VARS TURBOACTIVATE_LIBRARIES TURBOACTIVATE_INCLUDE_DIRS TURBOACTIVATE_DIR)

add_library(turboactivate SHARED IMPORTED)
set_target_properties(turboactivate PROPERTIES
                      IMPORTED_CONFIGURATIONS "Debug;Release"
                      IMPORTED_LOCATION_RELEASE ${TURBOACTIVATE_LIBS_RELEASE_DLL}
                      IMPORTED_LOCATION_DEBUG ${TURBOACTIVATE_LIBS_RELEASE_DLL}
                      )

if(WIN32)
    set_target_properties(turboactivate PROPERTIES
                          IMPORTED_IMPLIB ${TURBOACTIVATE_LIBRARIES}
                          )
endif()
