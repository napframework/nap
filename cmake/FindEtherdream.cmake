if (WIN32)
    find_path(
        ETHERDREAM_DIR
        NAMES msvc/include/j4cDAC.h
        HINTS ${CMAKE_CURRENT_LIST_DIR}/../../thirdparty/etherdream
    )
    set(ETHERDREAM_INCLUDE_DIRS ${ETHERDREAM_DIR}/msvc/include)
    set(ETHERDREAM_LIBS ${ETHERDREAM_DIR}/msvc/bin/Release/EtherDream.lib)
elseif(APPLE)
	find_path(
		ETHERDREAM_DIR
		NAMES osx/include/etherdream.h
		HINTS ${CMAKE_CURRENT_LIST_DIR}/../../thirdparty/etherdream
	)
	set(ETHERDREAM_INCLUDE_DIRS ${ETHERDREAM_DIR}/osx/include)
	set(ETHERDREAM_LIBS ${ETHERDREAM_DIR}/osx/bin/Release/libEtherDream.dylib)
else()
	find_path(
		ETHERDREAM_DIR
		NAMES linux/include/etherdream.h
		HINTS ${CMAKE_CURRENT_LIST_DIR}/../../thirdparty/etherdream
	set(ETHERDREAM_INCLUDE_DIRS ${ETHERDREAM_DIR}/linux/include)
	set(ETHERDREAM_LIBS ${ETHERDREAM_DIR}/linux/bin/libetherdream.so)
	)
endif()

mark_as_advanced(ETHERDREAM_INCLUDE_DIRS)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(etherdream REQUIRED_VARS ETHERDREAM_INCLUDE_DIRS ETHERDREAM_LIBS)
