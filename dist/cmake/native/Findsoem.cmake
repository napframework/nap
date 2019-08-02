if(WIN32)
  # default soem directory
  find_path(SOEM_DIR
          NO_CMAKE_FIND_ROOT_PATH
          NAMES include/soem/ethercat.h
          HINTS ${THIRDPARTY_DIR}/soem
          )

    # wpcap dir
    set(WPCAP_DIR ${THIRDPARTY_DIR}/soem/wpcap)

    # library
    set(SOEM_LIBS 
      ${SOEM_DIR}/lib/soem.lib 
      ${WPCAP_DIR}/Lib/x64/Packet.lib
      ${WPCAP_DIR}/Lib/x64/wpcap.lib
      Ws2_32 winmm)

    # all includes for soem windows
    set(SOEM_INCLUDE_DIRS 
        ${SOEM_DIR}/include 
        ${SOEM_DIR}/include/soem 
        ${WPCAP_DIR}/Include)

elseif(APPLE)
  # default soem directory
  find_path(SOEM_DIR
          NO_CMAKE_FIND_ROOT_PATH
          NAMES include/soem/ethercat.h
          HINTS ${THIRDPARTY_DIR}/soem
          )
    # library
    set(SOEM_LIBS 
      ${SOEM_DIR}/lib/libsoem.a
      pcap
      pthread)

    # all includes for soem windows
    set(SOEM_INCLUDE_DIRS 
        ${SOEM_DIR}/include 
        ${SOEM_DIR}/include/soem)
else()
    #set(SERIAL_LIBS_DEBUG ${SERIAL_DIR}/linux/Debug/libserial.a)
    #set(SERIAL_LIBS_RELEASE ${SERIAL_DIR}/linux/Release/libserial.a)
endif()

mark_as_advanced(SOEM_INCLUDE_DIRS)

# promote package for find
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(soem REQUIRED_VARS SOEM_DIR)

# soem is always linked static
add_library(soem STATIC IMPORTED)
set_target_properties(soem PROPERTIES
                      IMPORTED_CONFIGURATIONS "Debug;Release"
                      IMPORTED_LOCATION_RELEASE ${SOEM_LIBS}
                      IMPORTED_LOCATION_DEBUG ${SOEM_LIBS}
                      )