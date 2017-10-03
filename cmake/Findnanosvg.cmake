# default artnet directory
find_path(
	NANOSVG_DIR
	NAMES src/nanosvg.h
	HINTS ${CMAKE_CURRENT_LIST_DIR}/../../thirdparty/nanosvg
)

mark_as_advanced(NANOSVG_DIR)
if(NANOSVG_DIR)
	set(NANOSVG_FOUND true)
endif()
mark_as_advanced(NANOSVG_FOUND)

set(NANOSVG_INCLUDE_DIRS ${NANOSVG_DIR}/src)
mark_as_advanced(NANOSVG_INCLUDE_DIRS)

if(NANOSVG_FOUND)
	message(STATUS "Found nanosvg header files in ${NANOSVG_INCLUDE_DIRS}")
endif()


# promote package for find
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(nanosvg REQUIRED_VARS NANOSVG_INCLUDE_DIRS)
