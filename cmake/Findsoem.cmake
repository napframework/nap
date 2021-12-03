if(WIN32)
  # default soem directory
  find_path(SOEM_DIR
          NO_CMAKE_FIND_ROOT_PATH
          NAMES include/soem/ethercat.h
          HINTS ${THIRDPARTY_DIR}/soem/msvc/x86_64
          )

    # wpcap dir
    set(WPCAP_DIR ${THIRDPARTY_DIR}/soem/source/oshw/win32/wpcap)

    # library
    set(SOEM_LIBS 
      ${SOEM_DIR}/lib/soem.lib 
      ${WPCAP_DIR}/Lib/x64/wpcap.lib
      ${WPCAP_DIR}/Lib/x64/Packet.lib
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
          HINTS ${THIRDPARTY_DIR}/soem/macos/x86_64
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
  # default soem directory
  find_path(SOEM_DIR
          NO_CMAKE_FIND_ROOT_PATH
          NAMES include/soem/ethercat.h
          HINTS ${THIRDPARTY_DIR}/soem/linux/${ARCH}
          )
    # library
    set(SOEM_LIBS 
      ${SOEM_DIR}/lib/libsoem.a
      pthread 
      rt)

    # all includes for soem windows
    set(SOEM_INCLUDE_DIRS 
        ${SOEM_DIR}/include 
        ${SOEM_DIR}/include/soem)
endif()

# SOEM Source Code Directory
find_path(SOEM_SOURCE_DIR
    NO_CMAKE_FIND_ROOT_PATH
    NAMES LICENSE
    HINTS ${THIRDPARTY_DIR}/soem/source
)

# SOEM License
set(SOEM_DIST_FILES ${SOEM_SOURCE_DIR}/LICENSE)

mark_as_advanced(SOEM_INCLUDE_DIRS)

# promote package for find
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(soem REQUIRED_VARS SOEM_DIR SOEM_SOURCE_DIR)
