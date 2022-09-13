# - Try to find HOLOPLAYCORE
# Once done this will define
# 
# HOLOPLAYCORE_FOUND - System has HOLOPLAYCORE
# HOLOPLAYCORE_INCLUDE_DIR - The HOLOPLAYCORE include directory
# HOLOPLAYCORE_LIB - The library needed to use HOLOPLAYCORE
# HOLOPLAYCORE_DLL - The dll needed to use HOLOPLAYCORE

find_path(HOLOPLAYCORE_DIR
        NAMES include/HoloPlayCore.h
        HINTS ${THIRDPARTY_DIR}/holoplaycore
        )
find_path(HOLOPLAYCORE_INCLUDE_DIR
        NAMES HoloPlayCore.h
        HINTS ${HOLOPLAYCORE_DIR}/include
        )

if(WIN32)
    find_path(HOLOPLAYCORE_LIB_DIR
        NAMES HoloPlayCore.lib
        HINTS ${HOLOPLAYCORE_DIR}/bin/msvc
    )         
    set(HOLOPLAYCORE_LIB ${HOLOPLAYCORE_LIB_DIR}/HoloPlayCore.lib)
    set(HOLOPLAYCORE_DLL ${HOLOPLAYCORE_LIB_DIR}/HoloPlayCore.dll)

    find_library(HOLOPLAYCORE_LIB
        PATHS ${HOLOPLAYCORE_DIR}/bin
        NO_DEFAULT_PATH
    )
elseif(APPLE)
    find_path(HOLOPLAYCORE_LIB_DIR
            NAMES libHoloPlayCore.dylib
            HINTS ${HOLOPLAYCORE_DIR}/bin/macos
            )
    set(HOLOPLAYCORE_DLL ${HOLOPLAYCORE_LIB_DIR}/libHoloPlayCore.dylib)
endif()

mark_as_advanced(HOLOPLAYCORE_INCLUDE_DIR)
mark_as_advanced(HOLOPLAYCORE_LIB_DIR)

add_library(holoplaycore SHARED IMPORTED)
set_target_properties(holoplaycore PROPERTIES
        IMPORTED_CONFIGURATIONS "Debug;Release;"
        IMPORTED_LOCATION_RELEASE ${HOLOPLAYCORE_DLL}
        IMPORTED_LOCATION_DEBUG ${HOLOPLAYCORE_DLL}
        )

if (WIN32)
    set_target_properties(holoplaycore PROPERTIES
        IMPORTED_IMPLIB_RELEASE ${HOLOPLAYCORE_LIB}
        IMPORTED_IMPLIB_DEBUG ${HOLOPLAYCORE_LIB}
        )
endif()

# handle the QUIETLY and REQUIRED arguments
# if all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(holoplaycore REQUIRED_VARS HOLOPLAYCORE_DIR)

# Copy the dynamic linked lib into the build directory
macro(copy_holoplaycore_dll)
    if(WIN32)
        add_custom_command(TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:holoplaycore>
                "$<TARGET_PROPERTY:${PROJECT_NAME},RUNTIME_OUTPUT_DIRECTORY_$<UPPER_CASE:$<CONFIG>>>")
    endif()
endmacro()
