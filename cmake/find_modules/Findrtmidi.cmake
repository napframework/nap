# - Try to find RTMIDI
# Once done this will define
# RTMIDI_FOUND - System has RTMIDI
# RTMIDI_INCLUDE_DIR - The RTMIDI include directories
# RTMIDI_LIBRARIES_DEBUG - The libraries needed to use RTMIDI in debug mode
# RTMIDI_LIBRARIES_RELEASE - The libraries needed to use RTMIDI in release mode
# RTMIDID_DIST_FILES - Files required when rtmidi is distributed, for example: licenses

find_path(RTMIDI_DIR 
          NAMES
          msvc/x86_64/include/rtmidi/RtMidi.h
          macos/x86_64/include/rtmidi/RtMidi.h
          linux/${ARCH}/include/rtmidi/RtMidi.h
          HINTS
          ${THIRDPARTY_DIR}/rtmidi
          ${CMAKE_CURRENT_LIST_DIR}/../../rtmidi
          )

if(WIN32)
    set(RTMIDI_INCLUDE_DIR ${RTMIDI_DIR}/msvc/x86_64/include/rtmidi)
    set(RTMIDI_LIBRARY_DIR ${RTMIDI_DIR}/msvc/x86_64/lib)
    set(RTMIDI_LIBRARIES_RELEASE ${RTMIDI_LIBRARY_DIR}/rtmidi.lib winmm)
    set(RTMIDI_LIBRARIES_DEBUG ${RTMIDI_LIBRARY_DIR}/rtmidid.lib winmm)
elseif(APPLE)
    set(RTMIDI_INCLUDE_DIR ${RTMIDI_DIR}/macos/x86_64/include/rtmidi)
    set(RTMIDI_LIBRARY_DIR ${RTMIDI_DIR}/macos/x86_64/lib)
    set(RTMIDI_LIBRARIES_RELEASE ${RTMIDI_LIBRARY_DIR}/librtmidi.4.dylib)
    set(RTMIDI_LIBRARIES_DEBUG ${RTMIDI_LIBRARY_DIR}/librtmidi.4.dylib)
else()
    set(RTMIDI_INCLUDE_DIR ${RTMIDI_DIR}/linux/${ARCH}/include/rtmidi)
    set(RTMIDI_LIBRARY_DIR ${RTMIDI_DIR}/linux/${ARCH}/lib)
    set(RTMIDI_LIBRARIES_RELEASE ${RTMIDI_LIBRARY_DIR}/librtmidi.so)
    set(RTMIDI_LIBRARIES_DEBUG ${RTMIDI_LIBRARY_DIR}/librtmidi.so)
endif()

set(RTMIDI_DIST_FILES ${RTMIDI_DIR}/source/README.md)

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set RTMIDI_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(rtmidi REQUIRED_VARS 
    RTMIDI_DIR 
    RTMIDI_LIBRARY_DIR
    RTMIDI_INCLUDE_DIR 
    RTMIDI_LIBRARIES_RELEASE 
    RTMIDI_LIBRARIES_DEBUG 
    RTMIDI_DIST_FILES)
