# default oscpack directory
find_path(SERIAL_DIR
          NAMES include/serial/serial.h
          HINTS ${THIRDPARTY_DIR}/serial
          )

if(SERIAL_DIR)
    set(SERIAL_FOUND true)
else()
    set(SERIAL_FOUND false)
endif()

if(WIN32)
    set(SERIAL_LIBS_DEBUG ${SERIAL_DIR}/lib/Debug/serial.lib Ws2_32 winmm)
    set(SERIAL_LIBS_RELEASE ${SERIAL_DIR}/lib/Release/serial.lib Ws2_32 winmm)
elseif(APPLE)
    set(SERIAL_LIBS_DEBUG ${SERIAL_DIR}/lib/Debug/libserial.a)
    set(SERIAL_LIBS_RELEASE ${SERIAL_DIR}/lib/Release/libserial.a)
else()
    #set(OSCPACK_LIBS_DEBUG ${OSCPACK_DIR}/lib/liboscpack.so)
    #set(OSCPACK_LIBS_RELEASE ${OSCPACK_DIR}/lib/liboscpack.so)
endif()

# include directory is universal
set(SERIAL_INCLUDE_DIRS ${SERIAL_DIR}/include)

mark_as_advanced(SERIAL_INCLUDE_DIRS)

if(SERIAL_FOUND)
    message(STATUS "Found serial in: ${SERIAL_DIR}")
endif()

# promote package for find
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(serial REQUIRED_VARS SERIAL_DIR SERIAL_INCLUDE_DIRS SERIAL_LIBS_DEBUG SERIAL_LIBS_RELEASE)

if(WIN32)
    add_library(serial STATIC IMPORTED)
elseif(APPLE)
    add_library(serial STATIC IMPORTED)
else()
    add_library(serial SHARED IMPORTED)
endif()
set_target_properties(serial PROPERTIES
                      IMPORTED_CONFIGURATIONS "Debug;Release"
                      IMPORTED_LOCATION_RELEASE ${SERIAL_LIBS_RELEASE}
                      IMPORTED_LOCATION_DEBUG ${SERIAL_LIBS_DEBUG}
                      )
