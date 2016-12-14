# - Try to find ZMQ
# Once done this will define
# ZMQ_FOUND - System has ZMQ
# ZMQ_INCLUDE_DIRS - The ZMQ include directories
# ZMQ_LIBRARIES - The libraries needed to use ZMQ
# ZMQ_DEFINITIONS - Compiler switches required for using ZMQ

if ("${ARCH}" STREQUAL "i386")
    set(ARCHITECTURE Win32)
elseif("${ARCH}" STREQUAL "x86_64")
    set(ARCHITECTURE Win64)
endif()


find_path(ZMQ_DIR include/zmq.h
        HINTS
        ${CMAKE_CURRENT_LIST_DIR}/../../thirdparty/libzmq
        ${CMAKE_CURRENT_LIST_DIR}/../../libzmq
        )
if (UNIX)
elseif(WIN32)
    find_library(ZMQ_LIBRARIES NAMES libzmq.lib HINTS ${ZMQ_DIR}/bin/${ARCHITECTURE}/Release/v140/dynamic)
#    else()
#        find_library(ZMQ_LIBRARIES NAMES libzmq.dll HINTS
#                ${ZMQ_DIR}/bin/${CMAKE_BUILD_TYPE}
#                ${ZMQ_DIR}/bin/Debug
#                ${ZMQ_DIR}/cmake-build-debug/lib
#                )
#    endif()
endif()

if (APPLE)
    find_library(ZMQ_LIBRARIES NAMES libzmq.dylib
            HINTS
            /usr/local/opt/zeromq/lib
            )
    message(WARNING ${ZMQ_LIBRARIES})
endif()

set(ZMQ_INCLUDE_DIR ${ZMQ_DIR}/include)

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set ZMQ_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(ZMQ DEFAULT_MSG ZMQ_LIBRARIES ZMQ_INCLUDE_DIR)


