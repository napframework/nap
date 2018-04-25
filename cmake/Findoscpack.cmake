# default oscpack directory
find_path(OSCPACK_DIR
          NAMES osc/OscTypes.h
          HINTS ${THIRDPARTY_DIR}/oscpack
          )

if(WIN32)
    set(OSCPACK_LIBS_DEBUG ${OSCPACK_DIR}/msvc64/Debug/oscpack.lib Ws2_32 winmm)
    set(OSCPACK_LIBS_RELEASE ${OSCPACK_DIR}/msvc64/Release/oscpack.lib Ws2_32 winmm)
elseif(APPLE)
    set(OSCPACK_LIBS_DEBUG ${OSCPACK_DIR}/osx/install/lib/Debug/liboscpack.1.1.0.dylib)
    set(OSCPACK_LIBS_RELEASE ${OSCPACK_DIR}/osx/install/lib/Release/liboscpack.1.1.0.dylib)
else()
    set(OSCPACK_LIBS_DEBUG ${OSCPACK_DIR}/linux/liboscpack.so.1.1.0)
    set(OSCPACK_LIBS_RELEASE ${OSCPACK_DIR}/linux/liboscpack.so.1.1.0)
endif()

# include directory is universal
set(OSCPACK_INCLUDE_DIRS ${OSCPACK_DIR})

mark_as_advanced(OSCPACK_INCLUDE_DIRS)

# promote package for find
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(oscpack REQUIRED_VARS OSCPACK_INCLUDE_DIRS OSCPACK_LIBS_RELEASE OSCPACK_LIBS_DEBUG)