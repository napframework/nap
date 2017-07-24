/// local includes
#include <videoservice.h>

// external includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ntexture.h"
#include "image.h"

extern "C"
{
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
}

RTTI_BEGIN_CLASS(nap::VideoResource)
	RTTI_PROPERTY("Path", &nap::VideoResource::mPath, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

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

	//////////////////////////////////////////////////////////////////////////


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

		// Enable this to dump information about file onto standard error
		//av_dump_format(mFormatContext, 0, mPath.c_str(), 0);

		// Find the index and codec context of the videostream. The codec is stored in a local 
		// as it needs to be copied later
		AVCodecContext* codec_context = nullptr;
		for (int i = 0; i < mFormatContext->nb_streams; ++i)
		{
			if (mFormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
			{
				codec_context = mFormatContext->streams[i]->codec;
				mVideoStream = i;
				break;
			}
		}

		if (!errorState.check(codec_context != nullptr, "No video stream found"))
			return false;

		// Find the decoder for the video stream
		mCodec = avcodec_find_decoder(codec_context->codec_id);
		if (!errorState.check(mCodec != nullptr, "Unable to find codec for videostream"))
			return false;

		// Copy context
		mCodecContext = avcodec_alloc_context3(mCodec);
		error = avcodec_copy_context(mCodecContext, codec_context);
		if (!errorState.check(error == 0, "Unable to copy codec context: %s", error_to_string(error).c_str()))
			return false;

		/* open it */
		AVDictionary* opts = nullptr;
// 		if (!av_dict_get(opts, "threads", NULL, 0))
// 			av_dict_set(&opts, "threads", "0", 0);

		error = avcodec_open2(mCodecContext, mCodec, &opts);
		if (!errorState.check(error == 0, "Unable to open codec: %s", error_to_string(error).c_str()))
			return false;

		mFrame = av_frame_alloc();
		if (!errorState.check(mFrame != nullptr, "Unable to allocate frame"))
			return false;

		mWidth = mCodecContext->width;
		mHeight = mCodecContext->height;

		opengl::Texture2DSettings settings;
		settings.format = GL_RED;
		settings.internalFormat = GL_RED;
		settings.width = mCodecContext->width;
		settings.height = mCodecContext->height;

		// Disable mipmapping for video
		opengl::TextureParameters parameters;
		parameters.minFilter = GL_LINEAR;
		parameters.maxFilter = GL_LINEAR;

		mYTexture = std::make_unique<MemoryTexture2D>();
		mYTexture->mSettings = settings;
		if (!mYTexture->init(errorState))
			return false;
		mYTexture->getTexture().updateParameters(parameters);

		// The video is encoded in YUV format. The U and V texture are half the resolution of the Y texture
		settings.width *= 0.5f;
		settings.height *= 0.5f;

		mUTexture = std::make_unique<MemoryTexture2D>();
		mUTexture->mSettings = settings;
		if (!mUTexture->init(errorState))
			return false;
		mUTexture->getTexture().updateParameters(parameters);

		mVTexture = std::make_unique<MemoryTexture2D>();
		mVTexture->mSettings = settings;
		if (!mVTexture->init(errorState))
			return false;
		mVTexture->getTexture().updateParameters(parameters);

		return true;
	}


	void VideoResource::play()
	{
		mDecodeThread = std::thread(std::bind(&VideoResource::decodeThread, this));
		mIOThread = std::thread(std::bind(&VideoResource::ioThread, this));
		mPlaying = true;
	}


	void VideoResource::ioThread()
	{
		while (true)
		{
			// Read packets from the stream and push them onto the packet queue
			AVPacket* packet = av_packet_alloc();
			int result = av_read_frame(mFormatContext, packet);
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
			if (packet->stream_index != mVideoStream)
				continue;

			{
				std::unique_lock<std::mutex> lock(mPacketQueueMutex);
				mPacketQueueRoomAvailableCondition.wait(lock, [this]() { return mPacketQueue.size() < 3; });
				mPacketQueue.push(packet);
				mPacketAvailableCondition.notify_one();
			}
		}
	}


	void VideoResource::decodeThread()
	{
		// Here we find the duration of a frame. This is used as a predication when the next frame needs to be show
		// in case the PTS in the video stream was not available
		AVRational frame_rate = av_guess_frame_rate(mFormatContext, mFormatContext->streams[mVideoStream], nullptr);
		double frame_duration = (frame_rate.num && frame_rate.den ? av_q2d(AVRational { frame_rate.den, frame_rate.num }) : 0);

		// All timing information is relative to the stream start. If the stream start has no PTS value, we assume zero to be the start
		double stream_start_time = 0.0;
		if (mFormatContext->streams[mVideoStream]->start_time != AV_NOPTS_VALUE)
			stream_start_time = mFormatContext->streams[mVideoStream]->start_time *av_q2d(mFormatContext->streams[mVideoStream]->time_base);

		while (true)
		{
			// Here we pull packets from the queue and decode them until all data for this frame is processed
			int frame_finished = 0;
			while (!frame_finished)
			{
				AVPacket* packet;
				{
					std::unique_lock<std::mutex> lock(mPacketQueueMutex);
					mPacketAvailableCondition.wait(lock, [this]() { return !mPacketQueue.empty(); });
					packet = mPacketQueue.front();
					mPacketQueue.pop();
					mPacketQueueRoomAvailableCondition.notify_one();
				}

				int result = avcodec_decode_video2(mCodecContext, mFrame, &frame_finished, packet);
				if (result < 0)
				{
					mPlaying = false;
					return;
				}

				// Free the packet that was allocated by av_read_frame in the IO thread
				av_free_packet(packet);
			}

			// We calculate when the next frame needs to be displayed. The videostream contains PTS information, but there are cases
			// when there is no PTS available, we need to account for these cases.
			Frame new_frame;
			new_frame.mPTSSecs = DBL_MAX;
			if (mFrame->pts == AV_NOPTS_VALUE)
			{
				// In case there is no PTS we use the previous PTS time plus the frame time as a best prediction. However, if there is 
				// no previous frame, we assume zero.
				if (mPrevPTSSecs == DBL_MAX)
					new_frame.mPTSSecs = 0.0;
				else
					new_frame.mPTSSecs = mPrevPTSSecs + frame_duration;
			}
			else
			{
				// Use the PTS in the timespace of the video stream, starting from the videostream start
				new_frame.mPTSSecs = (mFrame->pts * av_q2d(mFormatContext->streams[mVideoStream]->time_base)) - stream_start_time;
			}
			mPrevPTSSecs = new_frame.mPTSSecs;

			// We MOVE the decoded frame to this new frame. It is freed when processed
			new_frame.mFrame = av_frame_alloc();
			av_frame_move_ref(new_frame.mFrame, mFrame);

			// Push the frame onto the frame queue
			{
				std::unique_lock<std::mutex> lock(mFrameQueueMutex);
				mFrameQueueRoomAvailableCondition.wait(lock, [this]() { return mFrameQueue.size() < 3; });
				mFrameQueue.push(new_frame);
				mFrameDataAvailableCondition.notify_one();
			}
		}
	}


	void VideoResource::update(double deltaTime)
	{
		if (!mPlaying)
			return;

		// Lazy init of video clock
		if (mVideoClockSecs == DBL_MAX)
			mVideoClockSecs = 0.0;
		else
			mVideoClockSecs += deltaTime;

		// Peek into the frame queue. If we have a frame and the PTS value of the first frame on
		// the FIFO queue has expired, we pop it. If there is no frame or the frame has not expired,
		// we return, effectively keeping the same contents in the existing textures.
		Frame cur_frame;
		{
			std::unique_lock<std::mutex> lock(mFrameQueueMutex);

			// This can be enabled to block waiting until data is available
			//mFrameDataAvailableCondition.wait(lock, [this]() { return !mFrameQueue.empty(); });

			if (mFrameQueue.empty() || mVideoClockSecs < mFrameQueue.front().mPTSSecs)
				return;

			cur_frame = mFrameQueue.front();
			mFrameQueue.pop();

			mFrameQueueRoomAvailableCondition.notify_one();
		}

		assert(cur_frame.mFrame->linesize[0] == cur_frame.mFrame->width);
		assert(cur_frame.mFrame->linesize[1] == cur_frame.mFrame->width / 2);
		assert(cur_frame.mFrame->linesize[2] == cur_frame.mFrame->width / 2);

		// Copy data into texture
		mYTexture->getTexture().setData(cur_frame.mFrame->data[0]);
		mUTexture->getTexture().setData(cur_frame.mFrame->data[1]);
		mVTexture->getTexture().setData(cur_frame.mFrame->data[2]);

		// Destroy frame that was allocated in the decode thread, after it has been processed
		av_frame_unref(cur_frame.mFrame);
		av_frame_free(&cur_frame.mFrame);
	}
}


