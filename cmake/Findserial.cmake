# default serial directory
find_path(SERIAL_DIR
          NO_CMAKE_FIND_ROOT_PATH
          NAMES include/serial/serial.h
          HINTS ${THIRDPARTY_DIR}/serial
          )

if(WIN32)
    set(SERIAL_LIBS_DEBUG ${SERIAL_DIR}/msvc/x64/Debug/serial.lib Ws2_32 winmm)
    set(SERIAL_LIBS_RELEASE ${SERIAL_DIR}/msvc/x64/Release/serial.lib Ws2_32 winmm)
elseif(APPLE)
    set(SERIAL_LIBS_DEBUG ${SERIAL_DIR}/osx/Debug/libserial.a)
    set(SERIAL_LIBS_RELEASE ${SERIAL_DIR}/osx/Release/libserial.a)
else()
    set(SERIAL_LIBS_DEBUG ${SERIAL_DIR}/linux/Debug/libserial.a)
    set(SERIAL_LIBS_RELEASE ${SERIAL_DIR}/linux/Release/libserial.a)
endif()

# include directory is universal
set(SERIAL_INCLUDE_DIRS ${SERIAL_DIR}/include)
mark_as_advanced(SERIAL_INCLUDE_DIRS)

# promote package for find
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(serial REQUIRED_VARS SERIAL_DIR)
