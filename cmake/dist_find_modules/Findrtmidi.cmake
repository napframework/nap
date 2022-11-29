# - Try to find RTMIDI
# Once done this will define
# RTMIDI_FOUND - System has RTMIDI
# RTMIDI_INCLUDE_DIR - The RTMIDI include directories
# RTMIDI_LIBRARIES_DEBUG - The libraries needed to use RTMIDI in debug mode
# RTMIDI_LIBRARIES_RELEASE - The libraries needed to use RTMIDI in release mode
# RTMIDID_DIST_FILES - Files required when rtmidi is distributed, for example: licenses

find_path(RTMIDI_DIR
          NO_CMAKE_FIND_ROOT_PATH
          NAMES include/RtMidi.h
          HINTS ${THIRDPARTY_DIR}/rtmidi
          )

set(RTMIDI_INCLUDE_DIR ${RTMIDI_DIR}/include)
if (WIN32)
    set(RTMIDI_LIBRARY_DIR ${RTMIDI_DIR}/bin)
    set(RTMIDI_LIBRARIES_RELEASE ${RTMIDI_LIBRARY_DIR}/rtmidi.lib)
    set(RTMIDI_LIBRARIES_DEBUG ${RTMIDI_LIBRARY_DIR}/rtmidid.lib)
elseif(APPLE)
    set(RTMIDI_LIBRARY_DIR ${RTMIDI_DIR}/lib)
    set(RTMIDI_LIBRARIES_RELEASE ${RTMIDI_LIBRARY_DIR}/librtmidi.4.dylib)
    set(RTMIDI_LIBRARIES_DEBUG ${RTMIDI_LIBRARIES_RELEASE})
else()
    set(RTMIDI_LIBRARY_DIR ${RTMIDI_DIR}/lib)
    set(RTMIDI_LIBRARIES_RELEASE ${RTMIDI_LIBRARY_DIR}/librtmidi.so.4.0.0)
    set(RTMIDI_LIBRARIES_DEBUG ${RTMIDI_LIBRARIES_RELEASE})
endif()

mark_as_advanced(RTMIDI_LIBRARY_DIR)

# promote package for find
INCLUDE(FindPackageHandleStandardArgs)
find_package_handle_standard_args(rtmidi REQUIRED_VARS RTMIDI_DIR)

add_library(rtmidi SHARED IMPORTED)
set_target_properties(rtmidi PROPERTIES
                      IMPORTED_CONFIGURATIONS "Debug;Release"
                      IMPORTED_LOCATION_RELEASE ${RTMIDI_LIBRARIES_RELEASE}
                      IMPORTED_LOCATION_DEBUG ${RTMIDI_LIBRARIES_DEBUG}
                      )

if(WIN32)
    set_target_properties(rtmidi PROPERTIES
                          IMPORTED_IMPLIB_RELEASE ${RTMIDI_LIBRARIES_RELEASE}
                          IMPORTED_IMPLIB_DEBUG ${RTMIDI_LIBRARIES_DEBUG}
                          )
endif()
