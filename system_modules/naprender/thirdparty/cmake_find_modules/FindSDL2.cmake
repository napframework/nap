# Locate SDL2 library
# This module defines
# SDL2_LIBRARY, the name of the library to link against
# SDL2_FOUND, if false, do not try to link to SDL2
# SDL2_INCLUDE_DIR, where to find SDL.h

if (APPLE)
    set(SDL2_INCLUDE_DIR ${SDL2_DIR}/macos/${ARCH}/include/SDL2)
    set(SDL2_LIBRARY ${SDL2_DIR}/macos/${ARCH}/lib/libSDL2.dylib)
elseif(UNIX)
    set(SDL2_INCLUDE_DIR ${SDL2_DIR}/linux/${ARCH}/include/SDL2)
    set(SDL2_LIBRARY ${SDL2_DIR}/linux/${ARCH}/lib/libSDL2.so)
elseif(WIN32)
    set(SDL2_INCLUDE_DIR ${SDL2_DIR}/msvc/x86_64/include/SDL2)
    set(SDL2_LIBRARY ${SDL2_DIR}/msvc/x86_64/lib/SDL2.lib)
endif()


INCLUDE(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(SDL2 REQUIRED_VARS SDL2_INCLUDE_DIR SDL2_LIBRARY)