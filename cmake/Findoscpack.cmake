# default oscpack directory
find_path(OSCPACK_DIR
          NO_CMAKE_FIND_ROOT_PATH
          NAMES osc/OscTypes.h
          HINTS ${THIRDPARTY_DIR}/oscpack/msvc/x86_64/include/oscpack 
                ${THIRDPARTY_DIR}/oscpack/macos/x86_64/include/oscpack
          )

if(WIN32)
    set(OSCPACK_LIBS_DIR ${OSCPACK_DIR}/msvc/x86_64/lib)
    set(OSCPACK_LIBS_DEBUG ${OSCPACK_LIBS_DIR}/Debug/oscpack.lib Ws2_32 winmm)
    set(OSCPACK_LIBS_RELEASE ${OSCPACK_LIBS_DIR}/Release/oscpack.lib Ws2_32 winmm)
    set(OSCPACK_INCLUDE_DIRS ${OSCPACK_DIR}/msvc/x86_64/include/oscpack)
elseif(APPLE)
    set(OSCPACK_LIBS_DIR ${OSCPACK_DIR}/macos/x86_64/lib)
    set(OSCPACK_LIBS_DEBUG ${OSCPACK_LIBS_DIR}/Debug/liboscpack.1.1.0.dylib)
    set(OSCPACK_LIBS_RELEASE ${OSCPACK_LIBS_DIR}/Release/liboscpack.1.1.0.dylib)
    set(OSCPACK_INCLUDE_DIRS ${OSCPACK_DIR}/macos/x86_64/include/oscpack)
elseif(ANDROID)
    set(OSCPACK_LIBS_DEBUG ${OSCPACK_DIR}/android/install/lib/Debug/${ANDROID_ABI}/liboscpack.so)
    set(OSCPACK_LIBS_RELEASE ${OSCPACK_DIR}/android/install/lib/Release/${ANDROID_ABI}/liboscpack.so)
else()
    set(OSCPACK_LIBS_DIR ${OSCPACK_DIR}/linux/${ARCH}/lib)
    set(OSCPACK_LIBS_DEBUG ${OSCPACK_LIBS_DIR}/liboscpack.so)
    set(OSCPACK_LIBS_RELEASE ${OSCPACK_LIBS_DIR}/liboscpack.so)
    set(OSCPACK_INCLUDE_DIRS ${OSCPACK_DIR}/linux/${ARCH}/include/oscpack)
endif()

mark_as_advanced(OSCPACK_INCLUDE_DIRS)

# promote package for find
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(oscpack REQUIRED_VARS OSCPACK_DIR OSCPACK_LIBS_DIR OSCPACK_INCLUDE_DIRS)
