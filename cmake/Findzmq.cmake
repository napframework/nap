# - Try to find ZMQ
# Once done this will define
# ZMQ_FOUND - System has ZMQ
# ZMQ_INCLUDE_DIRS - The ZMQ include directories
# ZMQ_LIBRARIES - The libraries needed to use ZMQ
# ZMQ_DEFINITIONS - Compiler switches required for using ZMQ

include(${CMAKE_CURRENT_LIST_DIR}/targetarch.cmake)
target_architecture(ARCH)


find_path(ZMQ_DIR include/zmq.h
        HINTS
        ${CMAKE_CURRENT_LIST_DIR}/../../thirdparty/libzmq
        ${CMAKE_CURRENT_LIST_DIR}/../../libzmq
        )

if(WIN32)
    set(ZMQ_LIB_DIR ${ZMQ_DIR}/bin/${ARCH}/${CMAKE_BUILD_TYPE}/v140/dynamic)
    find_library(ZMQ_LIBRARIES NAMES libzmq.lib
            HINTS
            ${ZMQ_LIB_DIR}/v140/dynamic
            )

    find_library(LIB NAMES libzmq.dll
            HINTS
            ${ZMQ_LIB_DIR}/v140/dynamic
            )

    list(APPEND ZMQ_LIBRARIES ${LIB})

elseif(APPLE)
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


