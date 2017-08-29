if (WIN32)
    find_path(
        ETHERDREAM_DIR
        NAMES msvc/include/j4cDAC.h
        HINTS ${CMAKE_CURRENT_LIST_DIR}/../../thirdparty/etherdream
    )
    set(ETHERDREAM_INCLUDE_DIRS ${ETHERDREAM_DIR}/msvc/include)
    set(ETHERDREAM_LIBS ${ETHERDREAM_DIR}/msvc/bin/Release/EtherDream.lib)
    set(ETHERDREAM_LIBS_DIR ${ETHERDREAM_DIR}/msvc/bin/Release)
elseif(APPLE)
	find_path(
		ETHERDREAM_DIR
		NAMES osx/include/etherdream.h
		HINTS ${CMAKE_CURRENT_LIST_DIR}/../../thirdparty/etherdream
	)
	set(ETHERDREAM_INCLUDE_DIRS ${ETHERDREAM_DIR}/osx/include)
	set(ETHERDREAM_LIBS ${ETHERDREAM_DIR}/osx/bin/Release/libEtherDream.dylib)
	set(ETHERDREAM_LIBS_DIR ${ETHERDREAM_DIR}/osx/bin/Release)
else()
	find_path(
		ETHERDREAM_DIR
		NAMES linux/include/etherdream.h
		HINTS ${CMAKE_CURRENT_LIST_DIR}/../../thirdparty/etherdream
	)
	set(ETHERDREAM_INCLUDE_DIRS ${ETHERDREAM_DIR}/linux/include)
	set(ETHERDREAM_LIBS ${ETHERDREAM_DIR}/linux/bin/libetherdream.so)
	set(ETHERDREAM_LIBS_DIR ${ETHERDREAM_DIR}/linux/bin)
endif()

mark_as_advanced(ETHERDREAM_INCLUDE_DIRS)
mark_as_advanced(ETHERDREAM_LIBS_DIR)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(etherdream REQUIRED_VARS ETHERDREAM_INCLUDE_DIRS ETHERDREAM_LIBS ETHERDREAM_LIBS_DIR)
