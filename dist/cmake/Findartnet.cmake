# default artnet directory
if (WIN32)
    find_path(
        ARTNET_DIR
        NAMES msvc/install/bin/Release/libartnet.dll
        HINTS ${THIRDPARTY_DIR}/libartnet
    )
    set(ARTNET_LIBS_DIR ${ARTNET_DIR}/msvc/install/bin/Release)
	set(ARTNET_LIBS ${ARTNET_LIBS_DIR}/libartnet.lib)
	set(ARTNET_LIBS_RELEASE_DLL ${ARTNET_LIBS_DIR}/libartnet.dll)
elseif(APPLE)
    find_path(
        ARTNET_DIR
        NAMES osx/bin/libArtnet.dylib
        HINTS ${THIRDPARTY_DIR}/libartnet
    )   
    set(ARTNET_LIBS_DIR ${ARTNET_DIR}/osx/bin)
	set(ARTNET_LIBS ${ARTNET_LIBS_DIR}/libArtnet.dylib)
	set(ARTNET_LIBS_RELEASE_DLL ${ARTNET_LIBS_DIR}/libArtnet.dylib)
else()
    find_path(
        ARTNET_DIR
        NAMES linux/bin/libartnet.so
        HINTS ${THIRDPARTY_DIR}/libartnet
    )   
    set(ARTNET_LIBS_DIR ${ARTNET_DIR}/linux/bin)
	set(ARTNET_LIBS ${ARTNET_LIBS_DIR}/libartnet.so)
	set(ARTNET_LIBS_RELEASE_DLL ${ARTNET_LIBS_DIR}/libartnet.so)
endif()

mark_as_advanced(ARTNET_LIBS_DIR)

# promote package for find
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(artnet REQUIRED_VARS ARTNET_DIR ARTNET_LIBS ARTNET_LIBS_DIR)

add_library(artnet SHARED IMPORTED)
set_target_properties(artnet PROPERTIES
                      IMPORTED_CONFIGURATIONS "Debug;Release"
                      IMPORTED_LOCATION_RELEASE ${ARTNET_LIBS_RELEASE_DLL}
                      IMPORTED_LOCATION_DEBUG ${ARTNET_LIBS_RELEASE_DLL}
                      )

# Copy the artnet dynamic linked lib into the build directory
macro(copy_artnet_dll)
    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:artnet> $<TARGET_FILE_DIR:${PROJECT_NAME}>/$<TARGET_FILE_NAME:artnet>
    )
endmacro()
