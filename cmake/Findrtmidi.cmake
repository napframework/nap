# - Try to find RTMIDI
# Once done this will define
# RTMIDI_FOUND - System has RTMIDI
# RTMIDI_INCLUDE_DIRS - The RTMIDI include directories
# RTMIDI_LIBRARIES - The libraries needed to use RTMIDI

find_path(RTMIDI_DIR RtMidi.h
HINTS
${CMAKE_CURRENT_LIST_DIR}/../../thirdparty/rtmidi
${CMAKE_CURRENT_LIST_DIR}/../../rtmidi
)

set(RTMIDI_INCLUDE_DIR ${RTMIDI_DIR})

if(WIN32)
    set(RTMIDI_LIBRARIES ${RTMIDI_DIR}/bin/msvc/librtmidi.lib)
elseif(APPLE)
    set(RTMIDI_LIBRARIES ${RTMIDI_DIR}/bin/osx/librtmidi.a)
else()
    set(RTMIDI_LIBRARIES ${RTMIDI_DIR}/bin/linux/librtmidi.so)
endif()


include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set RTMIDI_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(RTMIDI DEFAULT_MSG RTMIDI_INCLUDE_DIR RTMIDI_LIBRARIES)



