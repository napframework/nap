find_path(FREETYPE_DIR
          NAMES 
          msvc/x86_64/include/freetype2/ft2build.h
          macos/x86_64/include/freetype2/ft2build.h
          linux/${ARCH}/include/freetype2/ft2build.h
          HINTS 
          ${THIRDPARTY_DIR}/freetype
          )

if(WIN32)
    set(FREETYPE_LIBS_DIR ${FREETYPE_DIR}/msvc/x86_64/lib)
    set(FREETYPE_INCLUDE_DIR ${FREETYPE_DIR}/msvc/x86_64/include/freetype2)
    set(FREETYPE_LIBS_DEBUG ${FREETYPE_LIBS_DIR}/Debug/freetyped.lib)
    set(FREETYPE_LIBS_RELEASE ${FREETYPE_LIBS_DIR}/Release/freetype.lib)
    set(FREETYPE_DLL_DEBUG ${FREETYPE_LIBS_DIR}/Debug/freetyped.dll)
    set(FREETYPE_DLL_RELEASE ${FREETYPE_LIBS_DIR}/Release/freetype.dll)
elseif(APPLE)
    set(FREETYPE_LIBS_DIR ${FREETYPE_DIR}/macos/x86_64/lib)
    set(FREETYPE_INCLUDE_DIR ${FREETYPE_DIR}/macos/x86_64/include/freetype2)
    set(FREETYPE_LIBS_DEBUG ${FREETYPE_LIBS_DIR}/libfreetype.dylib)
    set(FREETYPE_LIBS_RELEASE ${FREETYPE_LIBS_DIR}/libfreetype.dylib)
    set(FREETYPE_DLL_DEBUG ${FREETYPE_LIBS_DEBUG})
    set(FREETYPE_DLL_RELEASE ${FREETYPE_LIBS_RELEASE})
elseif(UNIX)
    set(FREETYPE_LIBS_DIR ${FREETYPE_DIR}/linux/${ARCH}/lib)
    set(FREETYPE_INCLUDE_DIR ${FREETYPE_DIR}/linux/${ARCH}/include/freetype2)
    set(FREETYPE_LIBS_RELEASE ${FREETYPE_LIBS_DIR}/libfreetype.so)
    set(FREETYPE_LIBS_DEBUG ${FREETYPE_LIBS_RELEASE})
    set(FREETYPE_DLL_RELEASE ${FREETYPE_LIBS_RELEASE})
    set(FREETYPE_DLL_DEBUG ${FREETYPE_LIBS_DEBUG})
endif()

file(GLOB FREETYPE_DIST_FILES ${FREETYPE_DIR}/source/docs/*.txt)
list(APPEND FREETYPE_DIST_FILES ${FREETYPE_DIR}/source/README)

mark_as_advanced(FREETYPE_INCLUDE_DIR)
mark_as_advanced(FREETYPE_LIBS_DEBUG)
mark_as_advanced(FREETYPE_LIBS_RELEASE)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(freetype REQUIRED_VARS 
    FREETYPE_DIR 
    FREETYPE_INCLUDE_DIR 
    FREETYPE_LIBS_DEBUG 
    FREETYPE_LIBS_RELEASE
    FREETYPE_DIST_FILES)

add_library(freetype SHARED IMPORTED)

set_target_properties(freetype PROPERTIES
                      IMPORTED_CONFIGURATIONS "Debug;Release;MinSizeRel;RelWithDebInfo"
                      IMPORTED_LOCATION_RELEASE ${FREETYPE_DLL_RELEASE}
                      IMPORTED_LOCATION_DEBUG ${FREETYPE_DLL_DEBUG}
                      IMPORTED_LOCATION_MINSIZEREL ${FREETYPE_DLL_RELEASE}
                      IMPORTED_LOCATION_RELWITHDEBINFO ${FREETYPE_DLL_RELEASE}
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
