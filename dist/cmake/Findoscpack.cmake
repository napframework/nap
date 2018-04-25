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
else()
    set(OSCPACK_LIBS_DEBUG ${OSCPACK_DIR}/lib/liboscpack.so.1.1.0)
    set(OSCPACK_LIBS_RELEASE ${OSCPACK_DIR}/lib/liboscpack.so.1.1.0)
endif()

# include directory is universal
set(OSCPACK_INCLUDE_DIRS ${OSCPACK_DIR}/include)

mark_as_advanced(OSCPACK_INCLUDE_DIRS)

# promote package for find
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(oscpack REQUIRED_VARS OSCPACK_DIR OSCPACK_INCLUDE_DIRS)