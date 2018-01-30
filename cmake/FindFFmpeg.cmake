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
    # use pkg-config to get the directories and then use these values
    # in the FIND_PATH() and FIND_LIBRARY() calls
    find_package(PkgConfig)
    if(PKG_CONFIG_FOUND)
        pkg_check_modules(_FFMPEG_AVCODEC libavcodec)
        pkg_check_modules(_FFMPEG_AVFORMAT libavformat)
        pkg_check_modules(_FFMPEG_AVUTIL libavutil)
    endif(PKG_CONFIG_FOUND)

    find_path(FFMPEG_AVCODEC_INCLUDE_DIR
              NAMES libavcodec/avcodec.h
              HINTS ${THIRDPARTY_DIR}/ffmpeg/include
              )

    find_library(FFMPEG_LIBAVCODEC
                 NAMES avcodec
                 PATHS ${THIRDPARTY_DIR}/ffmpeg/lib
                 )

    find_library(FFMPEG_LIBAVFORMAT
                 NAMES avformat
                 PATHS ${THIRDPARTY_DIR}/ffmpeg/lib
                 )

    find_library(FFMPEG_LIBAVUTIL
                 NAMES avutil
                 PATHS ${THIRDPARTY_DIR}/ffmpeg/lib
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
            )

    endif(FFMPEG_FOUND)

    if(FFMPEG_FOUND)
        if(NOT FFMPEG_FIND_QUIETLY)
            message(STATUS "Found FFMPEG or Libav: ${FFMPEG_LIBRARIES}, ${FFMPEG_INCLUDE_DIR}")
        endif(NOT FFMPEG_FIND_QUIETLY)
    else(FFMPEG_FOUND)
        if(FFMPEG_FIND_REQUIRED)
            message(FATAL_ERROR "Could not find libavcodec or libavformat or libavutil")
        endif(FFMPEG_FIND_REQUIRED)
    endif(FFMPEG_FOUND)

endif(FFMPEG_LIBRARIES AND FFMPEG_INCLUDE_DIR)