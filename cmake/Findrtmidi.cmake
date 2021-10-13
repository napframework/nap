# - Try to find RTMIDI
# Once done this will define
# RTMIDI_FOUND - System has RTMIDI
# RTMIDI_INCLUDE_DIRS - The RTMIDI include directories
# RTMIDI_LIBRARIES_DEBUG - The libraries needed to use RTMIDI in debug mode
# RTMIDI_LIBRARIES_RELEASE - The libraries needed to use RTMIDI in release mode

find_path(RTMIDI_DIR source/RtMidi.h
          HINTS
          ${THIRDPARTY_DIR}/rtmidi
          ${CMAKE_CURRENT_LIST_DIR}/../../rtmidi
          )

set(RTMIDI_INCLUDE_DIR ${RTMIDI_DIR}/source)

if(WIN32)
    set(RTMIDI_LIBRARIES_RELEASE ${RTMIDI_DIR}/msvc/x86_64/rtmidi.lib winmm)
    set(RTMIDI_LIBRARIES_DEBUG ${RTMIDI_DIR}/msvc/x86_64/rtmidid.lib winmm)
elseif(APPLE)
    set(RTMIDI_LIBRARIES_RELEASE ${RTMIDI_DIR}/macos/x86_64/librtmidi.4.dylib)
    set(RTMIDI_LIBRARIES_DEBUG ${RTMIDI_DIR}/macos/x86_64/librtmidi.4.dylib)
else()
    set(RTMIDI_LIBRARIES_RELEASE ${RTMIDI_DIR}/linux/${ARCH}/lib/librtmidi.so)
    set(RTMIDI_LIBRARIES_DEBUG ${RTMIDI_DIR}/linux/${ARCH}/lib/librtmidi.so)
endif()

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set RTMIDI_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(rtmidi REQUIRED_VARS RTMIDI_INCLUDE_DIR)
