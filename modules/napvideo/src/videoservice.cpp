/// local includes
#include <videoservice.h>

// external includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ntexture.h"
#include "imageresource.h"

extern "C"
{
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
}

#define INBUF_SIZE 4096

RTTI_BEGIN_CLASS(nap::VideoService)
RTTI_END_CLASS

namespace nap
{
	const std::string error_to_string(int err)
	{
		char error_buf[256];
		av_make_error_string(error_buf, sizeof(error_buf), err);

		return std::string(error_buf);
	}

	bool VideoService::init(nap::utility::ErrorState& errorState)
	{
		av_register_all();
		avcodec_register_all();
		return true;
	}

	bool VideoResource::init(nap::utility::ErrorState& errorState)
	{
		mFormatContext = avformat_alloc_context();
		if (!errorState.check(mFormatContext != nullptr, "Error allocating context"))
			return false;

		int error = avformat_open_input(&mFormatContext, mPath.c_str(), nullptr, nullptr);
		if (!errorState.check(error >= 0, "Error opening file: %s\n", error_to_string(error).c_str()))
			return false;

		error = avformat_find_stream_info(mFormatContext, nullptr);
		if (!errorState.check(error >= 0, "Error finding stream: %s", error_to_string(error).c_str()))
			return false;

		// Dump information about file onto standard error
		av_dump_format(mFormatContext, 0, mPath.c_str(), 0);

		AVCodecContext* codecContext = nullptr;
		for (int i = 0; i < mFormatContext->nb_streams; ++i)
		{
			if (mFormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
			{
				codecContext = mFormatContext->streams[i]->codec;
				mVideoStream = i;
				break;
			}
		}

		if (!errorState.check(codecContext != nullptr, "No video stream found"))
			return false;

		// Find the decoder for the video stream
		mCodec = avcodec_find_decoder(codecContext->codec_id);
		if (!errorState.check(mCodec != nullptr, "Unable to find codec"))
			return false;

		// Copy context
		mContext = avcodec_alloc_context3(mCodec);
		error = avcodec_copy_context(mContext, codecContext);
		if (!errorState.check(error == 0, "Unable to copy codec context: %s", error_to_string(error).c_str()))
			return false;

		/* open it */
		error = avcodec_open2(mContext, mCodec, nullptr);
		if (!errorState.check(error == 0, "Unable to open codec: %s", error_to_string(error).c_str()))
			return false;

		mFrame = av_frame_alloc();
		if (!errorState.check(mFrame != nullptr, "Unable to allocate frame"))
			return false;

		mWidth = mContext->width;
		mHeight = mContext->height;

		opengl::Texture2DSettings settings;
		settings.format = GL_RED;
		settings.internalFormat = GL_RED;
		settings.width = mContext->width;
		settings.height = mContext->height;

		mYTexture = std::make_unique<MemoryTextureResource2D>();
		mYTexture->mSettings = settings;
		if (!mYTexture->init(errorState))
			return false;

		opengl::TextureParameters parameters;
		parameters.minFilter = GL_LINEAR;
		parameters.maxFilter = GL_LINEAR;
		mYTexture->getTexture().updateParameters(parameters);

		settings.width *= 0.5f;
		settings.height *= 0.5f;

		mUTexture = std::make_unique<MemoryTextureResource2D>();
		mUTexture->mSettings = settings;
		if (!mUTexture->init(errorState))
			return false;
		mUTexture->getTexture().updateParameters(parameters);

		mVTexture = std::make_unique<MemoryTextureResource2D>();
		mVTexture->mSettings = settings;
		if (!mVTexture->init(errorState))
			return false;
		mVTexture->getTexture().updateParameters(parameters);

		return true;
	}

	void VideoResource::update(double deltaTime)
	{
		if (!mPlaying)
			return;

		AVPacket packet;
		int frame_finished = 0;

		while (!frame_finished)
		{
			int result = av_read_frame(mFormatContext, &packet);
			if (result == AVERROR_EOF)
			{
				// stop?
				mPlaying = false;
				return;
			}
			else if (result < 0)
			{
				mPlaying = false;
				return;
			}

			// Is this a packet from the video stream?
			if (packet.stream_index != mVideoStream)
				continue;

			// Decode video frame
			result = avcodec_decode_video2(mContext, mFrame, &frame_finished, &packet);
			if (result < 0)
			{
				mPlaying = false;
				return;
			}

			// Free the packet that was allocated by av_read_frame
			av_free_packet(&packet);
		}

		assert(mFrame->linesize[0] == mFrame->width);
		assert(mFrame->linesize[1] == mFrame->width / 2);
		assert(mFrame->linesize[2] == mFrame->width / 2);

		rtti_cast<TextureResource>(mYTexture.get())->getTexture().setData(mFrame->data[0]);
		rtti_cast<TextureResource>(mUTexture.get())->getTexture().setData(mFrame->data[1]);
		rtti_cast<TextureResource>(mVTexture.get())->getTexture().setData(mFrame->data[2]);
	}
}


