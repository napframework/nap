set(RTMIDI_DIR ${THIRDPARTY_DIR}/rtmidi)
set(RTMIDI_INCLUDE_DIRS ${RTMIDI_DIR}/include)
if (WIN32)
    set(RTMIDI_LIBS_DIR ${RTMIDI_DIR}/bin)
    set(RTMIDI_LIBS_RELEASE ${RTMIDI_LIBS_DIR}/bin/rtmidi.lib winmm)
    set(RTMIDI_LIBS_DEBUG ${RTMIDI_LIBS_DIR}/bin/rtmidid.lib winmm)
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
FIND_PACKAGE_HANDLE_STANDARD_ARGS(rtmidi REQUIRED_VARS RTMIDI_DIR RTMIDI_LIBS_RELEASE RTMIDI_LIBS_DEBUG RTMIDI_LIBS_DIR RTMIDI_INCLUDE_DIRS)

add_library(rtmidi SHARED IMPORTED)
set_target_properties(rtmidi PROPERTIES
                      IMPORTED_CONFIGURATIONS "Debug;Release"
                      IMPORTED_LOCATION_RELEASE ${RTMIDI_LIBS_RELEASE}
                      IMPORTED_LOCATION_DEBUG ${RTMIDI_LIBS_DEBUG}
                      )
