# default oscpack directory
find_path(OSCPACK_DIR
          NAMES include/osc/OscTypes.h
          HINTS ${THIRDPARTY_DIR}/oscpack
          )

if(WIN32)
    set(OSCPACK_LIBS_DEBUG ${OSCPACK_DIR}/lib/Debug/oscpack.lib Ws2_32 winmm)
    set(OSCPACK_LIBS_RELEASE ${OSCPACK_DIR}/lib/Release/oscpack.lib Ws2_32 winmm)
elseif(APPLE)
    set(OSCPACK_LIBS_DEBUG ${OSCPACK_DIR}/lib/Debug/liboscpack.1.1.0.dylib)
    set(OSCPACK_LIBS_RELEASE ${OSCPACK_DIR}/lib/Release/liboscpack.1.1.0.dylib)
elseif(ANDROID)
    set(OSCPACK_LIBS_DEBUG ${OSCPACK_DIR}/lib/Debug/${ANDROID_ABI}/liboscpack.so)
    set(OSCPACK_LIBS_RELEASE ${OSCPACK_DIR}/lib/Release/${ANDROID_ABI}/liboscpack.so)
else()
    set(OSCPACK_LIBS_DEBUG ${OSCPACK_DIR}/lib/liboscpack.so)
    set(OSCPACK_LIBS_RELEASE ${OSCPACK_DIR}/lib/liboscpack.so)
endif()

# include directory is universal
set(OSCPACK_INCLUDE_DIRS ${OSCPACK_DIR}/include)

mark_as_advanced(OSCPACK_INCLUDE_DIRS)

# promote package for find
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(oscpack REQUIRED_VARS OSCPACK_DIR OSCPACK_INCLUDE_DIRS OSCPACK_LIBS_DEBUG OSCPACK_LIBS_RELEASE)

if(WIN32)
    add_library(oscpack STATIC IMPORTED)
else()
    add_library(oscpack SHARED IMPORTED)
endif()
set_target_properties(oscpack PROPERTIES
                      IMPORTED_CONFIGURATIONS "Debug;Release"
                      IMPORTED_LOCATION_RELEASE ${OSCPACK_LIBS_RELEASE}
                      IMPORTED_LOCATION_DEBUG ${OSCPACK_LIBS_DEBUG}
                      )
