if (WIN32)
    find_path(
        ETHERDREAM_DIR
        NAMES msvc/include/j4cDAC.h
        HINTS ${CMAKE_CURRENT_LIST_DIR}/../../thirdparty/etherdream
    )
    set(ETHERDREAM_INCLUDE_DIRS ${ETHERDREAM_DIR}/msvc/include)
    set(ETHERDREAM_DLL ${ETHERDREAM_DIR}/msvc/bin/Release/EtherDream.dll)
endif()

mark_as_advanced(ETHERDREAM_INCLUDE_DIRS)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(etherdream REQUIRED_VARS ETHERDREAM_INCLUDE_DIRS ETHERDREAM_DLL)