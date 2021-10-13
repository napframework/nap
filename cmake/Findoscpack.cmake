# default oscpack directory
find_path(OSCPACK_DIR
          NO_CMAKE_FIND_ROOT_PATH
          NAMES source/osc/OscTypes.h
          HINTS ${THIRDPARTY_DIR}/oscpack
          )

if(WIN32)
    set(OSCPACK_LIBS_DEBUG ${OSCPACK_DIR}/msvc/x86_64/Debug/oscpack.lib Ws2_32 winmm)
    set(OSCPACK_LIBS_RELEASE ${OSCPACK_DIR}/msvc/x86_64/Release/oscpack.lib Ws2_32 winmm)
elseif(APPLE)
    set(OSCPACK_LIBS_DEBUG ${OSCPACK_DIR}/macos/x86_64/lib/Debug/liboscpack.1.1.0.dylib)
    set(OSCPACK_LIBS_RELEASE ${OSCPACK_DIR}/macos/x86_64/lib/Release/liboscpack.1.1.0.dylib)
elseif(ANDROID)
    set(OSCPACK_LIBS_DEBUG ${OSCPACK_DIR}/android/install/lib/Debug/${ANDROID_ABI}/liboscpack.so)
    set(OSCPACK_LIBS_RELEASE ${OSCPACK_DIR}/android/install/lib/Release/${ANDROID_ABI}/liboscpack.so)
else()
    set(OSCPACK_LIBS_DEBUG ${OSCPACK_DIR}/linux/${ARCH}/lib/liboscpack.so)
    set(OSCPACK_LIBS_RELEASE ${OSCPACK_DIR}/linux/${ARCH}/lib/liboscpack.so)
endif()

# include directory is universal
set(OSCPACK_INCLUDE_DIRS ${OSCPACK_DIR}/source)

mark_as_advanced(OSCPACK_INCLUDE_DIRS)

# promote package for find
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(oscpack REQUIRED_VARS OSCPACK_DIR)
