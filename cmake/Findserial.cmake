# default serial directory
find_path(SERIAL_DIR
          NO_CMAKE_FIND_ROOT_PATH
          NAMES source/include/serial/serial.h
          HINTS ${THIRDPARTY_DIR}/serial
          )

if(WIN32)
    set(SERIAL_LIBS_DIR ${SERIAL_DIR}/msvc/x86_64)
    set(SERIAL_LIBS_DEBUG ${SERIAL_LIBS_DIR}/Debug/serial.lib Ws2_32 winmm)
    set(SERIAL_LIBS_RELEASE ${SERIAL_LIBS_DIR}/Release/serial.lib Ws2_32 winmm)
elseif(APPLE)
    set(SERIAL_LIBS_DIR ${SERIAL_DIR}/macos/x86_64)
    set(SERIAL_LIBS_DEBUG ${SERIAL_LIBS_DIR}/Debug/libserial.a)
    set(SERIAL_LIBS_RELEASE ${SERIAL_LIBS_DIR}/Release/libserial.a)
else()
    set(SERIAL_LIBS_DIR ${SERIAL_DIR}/linux/${ARCH})
    set(SERIAL_LIBS_DEBUG ${SERIAL_LIBS_DIR}/Debug/libserial.a)
    set(SERIAL_LIBS_RELEASE ${SERIAL_LIBS_DIR}/Release/libserial.a)
endif()

# include directory is universal
set(SERIAL_INCLUDE_DIR ${SERIAL_DIR}/source/include)
mark_as_advanced(SERIAL_INCLUDE_DIR)

# set files to copy
set(SERIAL_DIST_FILES ${SERIAL_DIR}/source/README.md)

# promote package for find
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(serial REQUIRED_VARS SERIAL_DIR SERIAL_LIBS_DIR SERIAL_LIBS_DEBUG SERIAL_LIBS_RELEASE SERIAL_INCLUDE_DIR SERIAL_DIST_FILES)
