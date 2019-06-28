# default artnet directory
find_path(SOEM_DIR
          NO_CMAKE_FIND_ROOT_PATH
          NAMES soem/ethercat.h
          HINTS ${THIRDPARTY_DIR}/soem
          )

if(WIN32)
    set(SOEM_LIBS_DEBUG ${SOEM_DIR}/msvc/x64/Debug/soem.lib Ws2_32 winmm)
    set(SOEM_LIBS_RELEASE ${SOEM_DIR}/msvc/x64/Release/soem.lib Ws2_32 winmm)
elseif(APPLE)
    #set(SERIAL_LIBS_DEBUG ${SERIAL_DIR}/osx/Debug/libserial.a)
    #set(SERIAL_LIBS_RELEASE ${SERIAL_DIR}/osx/Release/libserial.a)
else()
    #set(SERIAL_LIBS_DEBUG ${SERIAL_DIR}/linux/Debug/libserial.a)
    #set(SERIAL_LIBS_RELEASE ${SERIAL_DIR}/linux/Release/libserial.a)
endif()

# include directory is universal
set(SOEM_INCLUDE_DIRS ${SOEM_DIR}/soem)
mark_as_advanced(SOEM_INCLUDE_DIRS)

# promote package for find
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(soem REQUIRED_VARS SOEM_DIR)
