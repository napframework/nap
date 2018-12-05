find_path(FREETYPE_DIR
          NAMES include/ft2build.h
          HINTS ${THIRDPARTY_DIR}/freetype
          )

if(FREETYPE_DIR)
    set(FREETYPE_FOUND true)
else()
    set(FREETYPE_FOUND false)
endif()
mark_as_advanced(FREETYPE_FOUND)

set(FREETYPE_INCLUDE_DIRS ${FREETYPE_DIR}/include)

if(WIN32)
    set(FREETYPE_LIBS_DEBUG ${FREETYPE_DIR}/bin/debug/freetyped.lib)
    set(FREETYPE_LIBS_RELEASE ${FREETYPE_DIR}/bin/release/freetype.lib)
    set(FREETYPE_LIBS_DIR ${FREETYPE_DIR}/bin)
    set(FREETYPE_DEBUG_DLL ${FREETYPE_DIR}/bin/debug/freetyped.dll)
    set(FREETYPE_RELEASE_DLL ${FREETYPE_DIR}/bin/release/freetype.dll)
elseif(APPLE)
    set(FREETYPE_LIBS_DEBUG ${FREETYPE_DIR}/lib/libfreetype.dylib)
    set(FREETYPE_LIBS_RELEASE ${FREETYPE_DIR}/lib/libfreetype.dylib)
    set(FREETYPE_LIBS_DIR ${FREETYPE_DIR}/lib)
    set(FREETYPE_DEBUG_DLL ${FREETYPE_LIBS_DEBUG})
    set(FREETYPE_RELEASE_DLL ${FREETYPE_LIBS_RELEASE})
elseif(UNIX)
    set(FREETYPE_LIBS_DEBUG ${FREETYPE_DIR}/lib/libfreetype.so)
    set(FREETYPE_LIBS_RELEASE ${FREETYPE_DIR}/lib/libfreetype.so)
    set(FREETYPE_LIBS_DIR ${FREETYPE_DIR}/bin)
    set(FREETYPE_DEBUG_DLL ${FREETYPE_LIBS_DEBUG})
    set(FREETYPE_RELEASE_DLL ${FREETYPE_LIBS_RELEASE})
endif()


mark_as_advanced(FREETYPE_INCLUDE_DIRS)
mark_as_advanced(FREETYPE_LIBS_DEBUG)
mark_as_advanced(FREETYPE_LIBS_RELEASE)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(freetype REQUIRED_VARS FREETYPE_INCLUDE_DIRS FREETYPE_LIBS_DEBUG FREETYPE_LIBS_RELEASE)

add_library(freetype SHARED IMPORTED)

set_target_properties(freetype PROPERTIES
                      IMPORTED_CONFIGURATIONS "Debug;Release;"
                      IMPORTED_LOCATION_RELEASE ${FREETYPE_RELEASE_DLL}
                      IMPORTED_LOCATION_DEBUG ${FREETYPE_DEBUG_DLL}
                      )

if(WIN32)
    set_target_properties(freetype PROPERTIES
                          IMPORTED_IMPLIB_RELEASE ${FREETYPE_LIBS_RELEASE}
                          IMPORTED_IMPLIB_DEBUG ${FREETYPE_LIBS_DEBUG}
                          )
endif()

if(FREETYPE_FOUND)
    message(STATUS "Found freetype in ${FREETYPE_DIR}")
else()
    message(STATUS "Unable to find freetype!")
endif()


# Copy the freetype dynamic linked lib into the build directory
macro(copy_freetype_dll)
    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} 
                -E copy 
                $<TARGET_FILE:freetype> 
                $<TARGET_FILE_DIR:${PROJECT_NAME}>/$<TARGET_FILE_NAME:freetype>
    )
endmacro()
