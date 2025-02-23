/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "videofile.h"

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/pixfmt.h>
#include "libswresample/swresample.h"
}

// External Includes
#include <utility/fileutils.h>

RTTI_BEGIN_CLASS(nap::VideoFile, "Video loaded from disk")
	RTTI_PROPERTY_FILELINK("Path",	&nap::VideoFile::mPath, nap::rtti::EPropertyMetaData::Required, nap::rtti::EPropertyFileType::Video, "Path to the video to load")
RTTI_END_CLASS

namespace nap
{
    const std::string VideoFile::avErrorToString(int err)
    {
        char error_buf[256];
        av_make_error_string(error_buf, sizeof(error_buf), err);
        return std::string(error_buf);
    }


	bool VideoFile::init(utility::ErrorState& errorState)
	{
		if (!errorState.check(utility::fileExists(mPath), "%s: file: %s does not exist", mID.c_str(), mPath.c_str()))
			return false;

        // obtain pixel format of the video file
        // Allocate audio /  video context
        auto* context = avformat_alloc_context();
        if (!errorState.check(context != nullptr, "%s: Error allocating context", mID.c_str()))
            return false;

        // Open file
        int error_code = 0;
        error_code = avformat_open_input(&context, mPath.c_str(), nullptr, nullptr);
        if (!errorState.check(error_code >= 0, "%s: Error opening file '%s': %s\n", mID.c_str(), mPath.c_str(), avErrorToString(error_code).c_str()))
            return false;

        // Gather stream info
        error_code = avformat_find_stream_info(context, nullptr);
        if (!errorState.check(error_code >= 0, "%s: Error finding stream: %s", mID.c_str(), avErrorToString(error_code).c_str()))
            return false;

        AVStream* video_stream = nullptr;
        for (int i = 0; i < context->nb_streams; ++i)
        {
            AVStream* cur_stream = context->streams[i];
            switch (cur_stream->codecpar->codec_type)
            {
                case AVMEDIA_TYPE_VIDEO:
                    video_stream = cur_stream;
                    break;
            }
        }

        // Ensure there's a video stream
        if (!errorState.check(video_stream != nullptr, "%s: No video stream found", mID.c_str()))
            return false;

        // finally find the pixel format
        mPixelFormat = video_stream->codecpar->format;

        // close stream and context
        avformat_close_input(&context);
        avformat_free_context(context);

		return true;
	}
}
