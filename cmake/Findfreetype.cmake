if(WIN32)
    find_path(FREETYPE_DIR
              NAMES include/freetype2/ft2build.h
              HINTS ${THIRDPARTY_DIR}/freetype/msvc/x86_64/
              )
    set(FREETYPE_LIBS_DEBUG ${FREETYPE_DIR}/Debug/freetyped.lib)
    set(FREETYPE_LIBS_RELEASE ${FREETYPE_DIR}/Release/freetype.lib)
    set(FREETYPE_LIBS_DIR ${FREETYPE_DIR})
    set(FREETYPE_DEBUG_DLL ${FREETYPE_DIR}/Debug/freetyped.dll)
    set(FREETYPE_RELEASE_DLL ${FREETYPE_DIR}/Release/freetype.dll)
    set(FREETYPE_INCLUDE_DIRS ${FREETYPE_DIR}/include/freetype2)
elseif(APPLE)
    find_path(FREETYPE_DIR
              NAMES include/freetype2/ft2build.h
              HINTS ${THIRDPARTY_DIR}/freetype/macos/x86_64/
              )
    set(FREETYPE_LIBS_DIR ${FREETYPE_DIR}/lib)
    set(FREETYPE_LIBS_DEBUG ${FREETYPE_LIBS_DIR}/libfreetype.dylib)
    set(FREETYPE_LIBS_RELEASE ${FREETYPE_LIBS_DIR}/libfreetype.dylib)
    set(FREETYPE_DEBUG_DLL ${FREETYPE_LIBS_DEBUG})
    set(FREETYPE_RELEASE_DLL ${FREETYPE_LIBS_RELEASE})
    set(FREETYPE_INCLUDE_DIRS ${FREETYPE_DIR}/include/freetype2)
elseif(UNIX)
    find_path(FREETYPE_DIR
              NAMES include/freetype2/ft2build.h
              HINTS ${THIRDPARTY_DIR}/freetype/linux/${ARCH}/
              )
    set(FREETYPE_LIBS_DIR ${FREETYPE_DIR}/lib)
    set(FREETYPE_LIBS_RELEASE ${FREETYPE_LIBS_DIR}/libfreetype.so)
    set(FREETYPE_LIBS_DEBUG ${FREETYPE_LIBS_RELEASE})
    set(FREETYPE_RELEASE_DLL ${FREETYPE_LIBS_RELEASE})
    set(FREETYPE_DEBUG_DLL ${FREETYPE_LIBS_DEBUG})
    set(FREETYPE_INCLUDE_DIRS ${FREETYPE_DIR}/include/freetype2)
endif()

mark_as_advanced(FREETYPE_INCLUDE_DIRS)
mark_as_advanced(FREETYPE_LIBS_DEBUG)
mark_as_advanced(FREETYPE_LIBS_RELEASE)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(freetype REQUIRED_VARS FREETYPE_DIR FREETYPE_INCLUDE_DIRS FREETYPE_LIBS_DEBUG FREETYPE_LIBS_RELEASE)

add_library(freetype SHARED IMPORTED)

set_target_properties(freetype PROPERTIES
                      IMPORTED_CONFIGURATIONS "Debug;Release;MinSizeRel;RelWithDebInfo"
                      IMPORTED_LOCATION_RELEASE ${FREETYPE_RELEASE_DLL}
                      IMPORTED_LOCATION_DEBUG ${FREETYPE_DEBUG_DLL}
                      IMPORTED_LOCATION_MINSIZEREL ${FREETYPE_RELEASE_DLL}
                      IMPORTED_LOCATION_RELWITHDEBINFO ${FREETYPE_RELEASE_DLL}
                      )

if(WIN32)
    set_target_properties(freetype PROPERTIES
                          IMPORTED_IMPLIB_RELEASE ${FREETYPE_LIBS_RELEASE}
                          IMPORTED_IMPLIB_DEBUG ${FREETYPE_LIBS_DEBUG}
                          )
endif()

# Copy the freetype dynamic linked lib into the build directory
macro(copy_freetype_dll)
    add_custom_command(
            TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different 
                    $<TARGET_FILE:freetype> 
                    "$<TARGET_PROPERTY:${PROJECT_NAME},RUNTIME_OUTPUT_DIRECTORY_$<UPPER_CASE:$<CONFIG>>>"
            )
endmacro()
