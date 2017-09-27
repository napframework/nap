# default artnet directory
find_path(
	ARTNET_DIR
	NAMES artnet/artnet.h
	HINTS ${CMAKE_CURRENT_LIST_DIR}/../../thirdparty/libartnet
)

if (WIN32)
	set(ARTNET_LIBS ${ARTNET_DIR}/msvc/install/bin/Release/libartnet.lib)
	set(ARTNET_LIBS_DIR ${ARTNET_DIR}/msvc/install/bin/Release)
	set(ARTNET_INCLUDE_DIRS ${ARTNET_DIR})
	set(ARTNET_LIBS_RELEASE_DLL ${ARTNET_LIBS_DIR}/libartnet.dll)

elseif(APPLE)
	set(ARTNET_LIBS ${ARTNET_DIR}/osx/bin/Release/libArtnet.dylib)
	set(ARTNET_LIBS_DIR ${ARTNET_DIR}/osx/bin/Release)
	set(ARTNET_INCLUDE_DIRS ${ARTNET_DIR} ${ARTNET_DIR}/osx)
	set(ARTNET_LIBS_RELEASE_DLL ${ARTNET_LIBS_DIR}/libArtnet.dylib)
else()
	set(ARTNET_LIBS ${ARTNET_DIR}/linux/bin/libartnet.so)
	set(ARTNET_LIBS_DIR ${ARTNET_DIR}/linux/bin)
	set(ARTNET_INCLUDE_DIRS ${ARTNET_DIR} ${ARTNET_DIR}/linux)
	set(ARTNET_LIBS_RELEASE_DLL ${ARTNET_LIBS_DIR}/libartnet.so)
endif()

mark_as_advanced(ARTNET_INCLUDE_DIRS)
mark_as_advanced(ARTNET_LIBS_DIR)

# promote package for find
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(artnet REQUIRED_VARS ARTNET_INCLUDE_DIRS ARTNET_LIBS ARTNET_LIBS_DIR)

# Copy the artnet dynamic linked lib into the build directory
macro(copy_artnet_dll)
    add_library(artnetlib SHARED IMPORTED)
    set_target_properties(artnetlib PROPERTIES
      IMPORTED_CONFIGURATIONS "Debug;Release;MinSizeRel;RelWithDebInfo"
      IMPORTED_LOCATION_RELEASE ${ARTNET_LIBS_RELEASE_DLL}
      IMPORTED_LOCATION_DEBUG ${ARTNET_LIBS_RELEASE_DLL}
      IMPORTED_LOCATION_MINSIZEREL ${ARTNET_LIBS_RELEASE_DLL}
      IMPORTED_LOCATION_RELWITHDEBINFO ${ARTNET_LIBS_RELEASE_DLL}
      )

    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:artnetlib> $<TARGET_FILE_DIR:${PROJECT_NAME}>/$<TARGET_FILE_NAME:artnetlib>
    )
endmacro()
