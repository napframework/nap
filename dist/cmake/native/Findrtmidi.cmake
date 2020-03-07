find_path(RTMIDI_DIR
          NO_CMAKE_FIND_ROOT_PATH
          NAMES include/RtMidi.h
          HINTS ${THIRDPARTY_DIR}/rtmidi
          )

set(RTMIDI_INCLUDE_DIRS ${RTMIDI_DIR}/include)
if (WIN32)
    set(RTMIDI_LIBS_DIR ${RTMIDI_DIR}/bin)
    set(RTMIDI_LIBS_RELEASE ${RTMIDI_LIBS_DIR}/rtmidi.lib)
    set(RTMIDI_LIBS_DEBUG ${RTMIDI_LIBS_DIR}/rtmidid.lib)
elseif(APPLE)
    set(RTMIDI_LIBS_DIR ${RTMIDI_DIR}/lib)
    set(RTMIDI_LIBS_RELEASE ${RTMIDI_LIBS_DIR}/librtmidi.4.dylib)
    set(RTMIDI_LIBS_DEBUG ${RTMIDI_LIBS_RELEASE})
else()
    set(RTMIDI_LIBS_DIR ${RTMIDI_DIR}/lib)
    set(RTMIDI_LIBS_RELEASE ${RTMIDI_LIBS_DIR}/librtmidi.so.4.0.0)
    set(RTMIDI_LIBS_DEBUG ${RTMIDI_LIBS_RELEASE})
endif()

mark_as_advanced(RTMIDI_LIBS_DIR)

# promote package for find
INCLUDE(FindPackageHandleStandardArgs)
find_package_handle_standard_args(rtmidi REQUIRED_VARS RTMIDI_DIR)

add_library(rtmidi SHARED IMPORTED)
set_target_properties(rtmidi PROPERTIES
                      IMPORTED_CONFIGURATIONS "Debug;Release"
                      IMPORTED_LOCATION_RELEASE ${RTMIDI_LIBS_RELEASE}
                      IMPORTED_LOCATION_DEBUG ${RTMIDI_LIBS_DEBUG}
                      )

if(WIN32)
    set_target_properties(rtmidi PROPERTIES
                          IMPORTED_IMPLIB_RELEASE ${RTMIDI_LIBS_RELEASE}
                          IMPORTED_IMPLIB_DEBUG ${RTMIDI_LIBS_DEBUG}
                          )
endif()