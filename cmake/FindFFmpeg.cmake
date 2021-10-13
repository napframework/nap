# - Try to find ffmpeg libraries (libavcodec, libavformat and libavutil)
# Once done this will define
#
# FFMPEG_FOUND - system has ffmpeg or libav
# FFMPEG_INCLUDE_DIR - the ffmpeg include directory
# FFMPEG_LIBRARIES - Link these to use ffmpeg
# FFMPEG_LIBAVCODEC
# FFMPEG_LIBAVFORMAT
# FFMPEG_LIBAVUTIL
#
# Copyright (c) 2008 Andreas Schneider <mail@cynapses.org>
# Modified for other libraries by Lasse K�rkk�inen <tronic>
# Modified for Hedgewars by Stepik777
#
# Redistribution and use is allowed according to the terms of the New
# BSD license.
#

if(FFMPEG_LIBRARIES AND FFMPEG_INCLUDE_DIR)
    # in cache already
    set(FFMPEG_FOUND TRUE)
else(FFMPEG_LIBRARIES AND FFMPEG_INCLUDE_DIR)
    if(APPLE)
        find_path(FFMPEG_AVCODEC_INCLUDE_DIR
                  NAMES libavcodec/avcodec.h
                  HINTS ${THIRDPARTY_DIR}/ffmpeg/macos/x86_64/include
                  )
    elseif(UNIX)
        find_path(FFMPEG_AVCODEC_INCLUDE_DIR
                  NAMES libavcodec/avcodec.h
                  HINTS ${THIRDPARTY_DIR}/ffmpeg/linux/${ARCH}/include
                  )
    else()
        find_path(FFMPEG_AVCODEC_INCLUDE_DIR
                  NAMES libavcodec/avcodec.h
                  HINTS ${THIRDPARTY_DIR}/ffmpeg/msvc/x86_64/include
                  )
    endif()

    find_library(FFMPEG_LIBAVCODEC
                 NAMES avcodec
                 PATHS ${THIRDPARTY_DIR}/ffmpeg/msvc/x86_64/bin
                       ${THIRDPARTY_DIR}/ffmpeg/macos/x86_64/lib
                       ${THIRDPARTY_DIR}/ffmpeg/linux/${ARCH}/lib
                 NO_DEFAULT_PATH
                 )

    find_library(FFMPEG_LIBAVFORMAT
                 NAMES avformat
                 PATHS ${THIRDPARTY_DIR}/ffmpeg/msvc/x86_64/bin
                       ${THIRDPARTY_DIR}/ffmpeg/macos/x86_64/lib
                       ${THIRDPARTY_DIR}/ffmpeg/linux/${ARCH}/lib
                 NO_DEFAULT_PATH
                 )

    find_library(FFMPEG_LIBAVUTIL
                 NAMES avutil
                 PATHS ${THIRDPARTY_DIR}/ffmpeg/msvc/x86_64/bin
                       ${THIRDPARTY_DIR}/ffmpeg/macos/x86_64/lib
                       ${THIRDPARTY_DIR}/ffmpeg/linux/${ARCH}/lib
                 NO_DEFAULT_PATH
                 )
				 
    find_library(FFMPEG_SWRESAMPLE
                 NAMES swresample
                 PATHS ${THIRDPARTY_DIR}/ffmpeg/msvc/x86_64/bin
                       ${THIRDPARTY_DIR}/ffmpeg/macos/x86_64/lib
                       ${THIRDPARTY_DIR}/ffmpeg/linux/${ARCH}/lib                 
                 NO_DEFAULT_PATH
                 )				 

    if(FFMPEG_LIBAVCODEC AND FFMPEG_LIBAVFORMAT)
        set(FFMPEG_FOUND TRUE)
    endif()

    if(FFMPEG_FOUND)
        set(FFMPEG_INCLUDE_DIR ${FFMPEG_AVCODEC_INCLUDE_DIR})

        set(FFMPEG_LIBRARIES
            ${FFMPEG_LIBAVCODEC}
            ${FFMPEG_LIBAVFORMAT}
            ${FFMPEG_LIBAVUTIL}
            ${FFMPEG_SWRESAMPLE}
            )
    endif(FFMPEG_FOUND)

    if(FFMPEG_FOUND)
        if(NOT FFMPEG_FIND_QUIETLY)
            message(STATUS "Found FFmpeg or Libav: ${FFMPEG_LIBRARIES}, ${FFMPEG_INCLUDE_DIR}")
        endif(NOT FFMPEG_FIND_QUIETLY)
    else(FFMPEG_FOUND)
        if(FFMPEG_FIND_REQUIRED)
            message(FATAL_ERROR "Could not find libavcodec or libavformat or libavutil")
        endif(FFMPEG_FIND_REQUIRED)
    endif(FFMPEG_FOUND)

endif(FFMPEG_LIBRARIES AND FFMPEG_INCLUDE_DIR)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FFmpeg REQUIRED_VARS FFMPEG_LIBRARIES)
