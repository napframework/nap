# default oscpack directory
find_path(OSCPACK_DIR
          NO_CMAKE_FIND_ROOT_PATH
          NAMES osc/OscTypes.h
          HINTS ${THIRDPARTY_DIR}/oscpack
          )

if(WIN32)
    set(OSCPACK_LIBS_DEBUG ${OSCPACK_DIR}/msvc64/Debug/oscpack.lib Ws2_32 winmm)
    set(OSCPACK_LIBS_RELEASE ${OSCPACK_DIR}/msvc64/Release/oscpack.lib Ws2_32 winmm)
elseif(APPLE)
    set(OSCPACK_LIBS_DEBUG ${OSCPACK_DIR}/osx/install/lib/Debug/liboscpack.1.1.0.dylib)
    set(OSCPACK_LIBS_RELEASE ${OSCPACK_DIR}/osx/install/lib/Release/liboscpack.1.1.0.dylib)
elseif(ANDROID)
    set(OSCPACK_LIBS_DEBUG ${OSCPACK_DIR}/android/install/lib/Debug/${ANDROID_ABI}/liboscpack.so)
    set(OSCPACK_LIBS_RELEASE ${OSCPACK_DIR}/android/install/lib/Release/${ANDROID_ABI}/liboscpack.so)
else()
    set(OSCPACK_LIBS_DEBUG ${OSCPACK_DIR}/linux/install/lib/liboscpack.so)
    set(OSCPACK_LIBS_RELEASE ${OSCPACK_DIR}/linux/install/lib/liboscpack.so)
endif()

# include directory is universal
set(OSCPACK_INCLUDE_DIRS ${OSCPACK_DIR})

mark_as_advanced(OSCPACK_INCLUDE_DIRS)

# promote package for find
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(oscpack REQUIRED_VARS OSCPACK_DIR)
