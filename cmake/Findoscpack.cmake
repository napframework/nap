# default oscpack directory
find_path(
	OSCPACK_DIR
	NAMES osc/OscTypes.h
	HINTS ${CMAKE_CURRENT_LIST_DIR}/../../thirdparty/oscpack
)

if (WIN32)
	set(OSCPACK_LIBS_DEBUG ${OSCPACK_DIR}/msvc64/Debug/oscpack.lib Ws2_32 winmm)
	set(OSCPACK_LIBS_RELEASE ${OSCPACK_DIR}/msvc64/Release/oscpack.lib Ws2_32 winmm)
endif()

# include directory is universal
set(OSCPACK_INCLUDE_DIRS ${OSCPACK_DIR})

mark_as_advanced(OSCPACK_INCLUDE_DIRS)

# promote package for find
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(oscpack REQUIRED_VARS OSCPACK_INCLUDE_DIRS OSCPACK_LIBS_RELEASE OSCPACK_LIBS_DEBUG)