# default oscpack directory
find_path(SERIAL_DIR
          NAMES include/serial/serial.h
          HINTS ${THIRDPARTY_DIR}/serial
          )

if(WIN32)
    set(SERIAL_LIBS_DEBUG ${SERIAL_DIR}/lib/Debug/serial.lib Ws2_32 winmm)
    set(SERIAL_LIBS_RELEASE ${SERIAL_DIR}/lib/Release/serial.lib Ws2_32 winmm)
elseif(APPLE)
    set(SERIAL_LIBS_DEBUG ${SERIAL_DIR}/lib/Debug/libserial.a)
    set(SERIAL_LIBS_RELEASE ${SERIAL_DIR}/lib/Release/libserial.a)
else()
    set(SERIAL_LIBS_DEBUG ${SERIAL_DIR}/lib/Debug/libserial.a)
    set(SERIAL_LIBS_RELEASE ${SERIAL_DIR}/lib/Release/libserial.a)
endif()

# include directory is universal
set(SERIAL_INCLUDE_DIRS ${SERIAL_DIR}/include)
mark_as_advanced(SERIAL_INCLUDE_DIRS)

# promote package for find
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(serial REQUIRED_VARS SERIAL_DIR)

# serial is always linked static
add_library(serial STATIC IMPORTED)
set_target_properties(serial PROPERTIES
                      IMPORTED_CONFIGURATIONS "Debug;Release"
                      IMPORTED_LOCATION_RELEASE ${SERIAL_LIBS_RELEASE}
                      IMPORTED_LOCATION_DEBUG ${SERIAL_LIBS_DEBUG}
                      )
