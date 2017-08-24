# default artnet directory
find_path(
	ARTNET_DIR
	NAMES artnet/artnet.h
	HINTS ${CMAKE_CURRENT_LIST_DIR}/../../thirdparty/libartnet
)

# artnet include directory
set(ARTNET_INCLUDE_DIRS ${ARTNET_DIR})

# only windows for now, need to compile the others
if (WIN32)
	set(ARTNET_LIBS ${ARTNET_DIR}/msvc/install/bin/Release/libartnet.lib)
	set(ARTNET_LIBS_DIR ${ARTNET_DIR}/msvc/install/bin/Release)
endif()

mark_as_advanced(ARTNET_INCLUDE_DIRS)
mark_as_advanced(ARTNET_LIBS_DIR)

# promote package for find
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(artnet REQUIRED_VARS ARTNET_INCLUDE_DIRS ARTNET_LIBS ARTNET_LIBS_DIR)
