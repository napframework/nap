# default artnet directory
find_path(SERIAL_DIR
          NO_CMAKE_FIND_ROOT_PATH
          NAMES include/serial/serial.h
          HINTS ${THIRDPARTY_DIR}/serial
          )

if(SERIAL_DIR)
    set(SERIAL_FOUND true)
else()
    set(SERIAL_FOUND false)
endif()

if(WIN32)
    set(SERIAL_LIBS_DEBUG ${SERIAL_DIR}/msvc/x64/Debug/serial.lib Ws2_32 winmm)
    set(SERIAL_LIBS_RELEASE ${SERIAL_DIR}/msvc/x64/Release/serial.lib Ws2_32 winmm)
elseif(APPLE)
    set(SERIAL_LIBS_DEBUG ${SERIAL_DIR}/osx/Debug/libserial.a)
    set(SERIAL_LIBS_RELEASE ${SERIAL_DIR}/osx/Release/libserial.a)
else()
    #set(SERIAL_LIBS_DEBUG ${OSCPACK_DIR}/linux/install/lib/liboscpack.so)
    #set(SERIAL_LIBS_RELEASE ${OSCPACK_DIR}/linux/install/lib/liboscpack.so)
endif()

if(SERIAL_FOUND)
    message(STATUS "Found serial: ${SERIAL_DIR}")
endif()

# include directory is universal
set(SERIAL_INCLUDE_DIRS ${SERIAL_DIR}/include)
mark_as_advanced(SERIAL_INCLUDE_DIRS)

# promote package for find
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(serial REQUIRED_VARS SERIAL_DIR SERIAL_INCLUDE_DIRS SERIAL_LIBS_DEBUG SERIAL_LIBS_RELEASE)
