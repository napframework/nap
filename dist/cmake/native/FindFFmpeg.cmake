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
# Modified for other libraries by Lasse Kärkkäinen <tronic>
# Modified for Hedgewars by Stepik777
#
# Redistribution and use is allowed according to the terms of the New
# BSD license.
#
if (FFMPEG_LIBRARIES AND FFMPEG_INCLUDE_DIR)
	# in cache already
	set(FFMPEG_FOUND TRUE)
else (FFMPEG_LIBRARIES AND FFMPEG_INCLUDE_DIR)
	find_path(FFMPEG_AVCODEC_INCLUDE_DIR
		NAMES libavcodec/avcodec.h
		HINTS ${THIRDPARTY_DIR}/FFmpeg/include
		NO_DEFAULT_PATH
	)

	find_library(FFMPEG_LIBAVCODEC
		NAMES avcodec
		PATHS ${THIRDPARTY_DIR}/FFmpeg/lib
			  ${THIRDPARTY_DIR}/FFmpeg/bin
		NO_DEFAULT_PATH
	)

	find_library(FFMPEG_LIBAVFORMAT
		NAMES avformat
		PATHS ${THIRDPARTY_DIR}/FFmpeg/lib
			  ${THIRDPARTY_DIR}/FFmpeg/bin
		NO_DEFAULT_PATH
	)

	find_library(FFMPEG_LIBAVUTIL
		NAMES avutil
		PATHS ${THIRDPARTY_DIR}/FFmpeg/lib
			  ${THIRDPARTY_DIR}/FFmpeg/bin
		NO_DEFAULT_PATH
	)

	if (FFMPEG_LIBAVCODEC AND FFMPEG_LIBAVFORMAT)
		set(FFMPEG_FOUND TRUE)
	endif()

	if (FFMPEG_FOUND)
		set(FFMPEG_INCLUDE_DIR ${FFMPEG_AVCODEC_INCLUDE_DIR})

		set(FFMPEG_LIBRARIES
			${FFMPEG_LIBAVCODEC}
			${FFMPEG_LIBAVFORMAT}
			${FFMPEG_LIBAVUTIL}
		)
	endif (FFMPEG_FOUND)

	if (FFMPEG_FOUND)
		if (NOT FFMPEG_FIND_QUIETLY)
			message(STATUS "Found FFMPEG or Libav: ${FFMPEG_LIBRARIES}, ${FFMPEG_INCLUDE_DIR}")
		endif (NOT FFMPEG_FIND_QUIETLY)
	else (FFMPEG_FOUND)
		if (FFMPEG_FIND_REQUIRED)
			message(FATAL_ERROR "Could not find libavcodec or libavformat or libavutil")
		endif (FFMPEG_FIND_REQUIRED)
		if (${FFMPEG_FIND_REQUIRED})
			message(FATAL_ERROR "Could not find libavcodec or libavformat or libavutil")
		endif()
	endif (FFMPEG_FOUND)

endif (FFMPEG_LIBRARIES AND FFMPEG_INCLUDE_DIR)