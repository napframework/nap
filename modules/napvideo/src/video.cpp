#include "video.h"

#include "texture2d.h"
#include "image.h"

// external includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <limits>

extern "C"
{
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
}

RTTI_BEGIN_CLASS(nap::Video)
	RTTI_PROPERTY("Path", &nap::Video::mPath, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Loop", &nap::Video::mLoop, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{

	// Max video value, used as state check
	const double Video::sVideoMax = std::numeric_limits<double>::max();


	const std::string error_to_string(int err)
	{
		char error_buf[256];
		av_make_error_string(error_buf, sizeof(error_buf), err);
		return std::string(error_buf);
	}


	Video::~Video()
	{
		stop();

		avcodec_close(mCodecContext);
		avcodec_free_context(&mCodecContext);
		avformat_close_input(&mFormatContext);
		avformat_free_context(mFormatContext);
	}


	bool Video::init(nap::utility::ErrorState& errorState)
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

		// Find the index and codec context of the video stream. The codec is stored in a local 
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
		if (!errorState.check(mCodec != nullptr, "Unable to find codec for video stream"))
			return false;

		// Copy context
		mCodecContext = avcodec_alloc_context3(mCodec);
		error = avcodec_copy_context(mCodecContext, codec_context);
		if (!errorState.check(error == 0, "Unable to copy codec context: %s", error_to_string(error).c_str()))
			return false;

		// This option causes the codec context to spawn threads internally for decoding, speeding up the decoding process
		AVDictionary* opts = nullptr;
		av_dict_set(&opts, "threads", "auto", 0);

		// We need to set this option to make sure that the decoder transfers ownership from decode buffers to us
		// when we decode frames. Otherwise, the decoder will reuse buffers, which will then overwrite data already in our queue.
		av_dict_set(&opts, "refcounted_frames", "1", 0);

		error = avcodec_open2(mCodecContext, mCodec, &opts);
		if (!errorState.check(error == 0, "Unable to open codec: %s", error_to_string(error).c_str()))
			return false;

		mWidth = mCodecContext->width;
		mHeight = mCodecContext->height;
		mDuration = static_cast<double>(mFormatContext->duration / AV_TIME_BASE);

		float yWidth = mCodecContext->width;
		float yHeight = mCodecContext->height;
		float uvWidth = mCodecContext->width * 0.5f;
		float uvHeight = mCodecContext->height * 0.5f;

		// YUV420p to RGB conversion uses an 'offset' value of (-0.0625, -0.5, -0.5) in the shader. 
		// This means that initializing the YUV planes to zero does not actually result in black output.
		// To fix this, we initialize the YUV planes to the negative of the offset
		std::vector<uint8_t> y_default_data;
		y_default_data.resize(yWidth * yHeight);
		std::memset(y_default_data.data(), 16, y_default_data.size());

		// Initialize UV planes
		std::vector<uint8_t> uv_default_data;
		uv_default_data.resize(uvWidth * uvHeight);
		std::memset(uv_default_data.data(), 127, uv_default_data.size());

		// Disable mipmapping for video
		nap::TextureParameters parameters;
		parameters.mMinFilter = EFilterMode::Linear;
		parameters.mMaxFilter = EFilterMode::Linear;

		mYTexture = std::make_unique<Texture2D>();
		mYTexture->mWidth = yWidth;
		mYTexture->mHeight = yHeight;
		mYTexture->mFormat = Texture2D::EFormat::R8;
		mYTexture->mParameters = parameters;
		if (!mYTexture->init(errorState))
			return false;

		mYTexture->getTexture().setData(y_default_data.data());

		mUTexture = std::make_unique<Texture2D>();
		mUTexture->mWidth = uvWidth;
		mUTexture->mHeight = uvHeight;
		mUTexture->mFormat = Texture2D::EFormat::R8;
		mUTexture->mParameters = parameters;
		if (!mUTexture->init(errorState))
			return false;
				
		mUTexture->getTexture().setData(uv_default_data.data());

		mVTexture = std::make_unique<Texture2D>();
		mVTexture->mWidth = uvWidth;
		mVTexture->mHeight = uvHeight;
		mVTexture->mFormat = Texture2D::EFormat::R8;
		mVTexture->mParameters = parameters;
		if (!mVTexture->init(errorState))
			return false;

		mVTexture->getTexture().setData(uv_default_data.data());

		return true;
	}


	void Video::play(double startTimeSecs)
	{
		assert(!mPlaying);

		// Reset all state to make sure threads don't immediately exit from a previous 'stop' call
		mExitIOThreadSignalled = false;
		mExitDecodeThreadSignalled = false;
		mPacketsFinished = false;
		mFramesFinished = false;
		mPlaying = true;
		mVideoClockSecs = sVideoMax;

		seek(startTimeSecs);

		mDecodeThread = std::thread(std::bind(&Video::decodeThread, this));
		mIOThread = std::thread(std::bind(&Video::ioThread, this));
	}


	void Video::stop()
	{
		if (!mPlaying)
			return;

		exitIOThread();
		exitDecodeThread();
		mPlaying = false;

		// Join thread
		mIOThread.join();
		mDecodeThread.join();

		clearPacketQueue();
		clearFrameQueue();
	}


	void Video::seek(double seconds)
	{
		// All timing information is relative to the stream start. If the stream start has no PTS value, we assume zero to be the start
		double stream_start_time = 0.0;
		if (mFormatContext->streams[mVideoStream]->start_time != AV_NOPTS_VALUE)
			stream_start_time = mFormatContext->streams[mVideoStream]->start_time *av_q2d(mFormatContext->streams[mVideoStream]->time_base);

		mSeekTarget = std::round((seconds - stream_start_time) / av_q2d(mFormatContext->streams[mVideoStream]->time_base));
	}


	void Video::clearPacketQueue()
	{
		std::unique_lock<std::mutex> lock(mPacketQueueMutex);
		while (!mPacketQueue.empty())
		{
			AVPacket* packet = mPacketQueue.front();
			mPacketQueue.pop();
			av_free_packet(packet);
		}
	}


	void Video::clearFrameQueue()
	{
		std::unique_lock<std::mutex> lock(mFrameQueueMutex);
		while (!mFrameQueue.empty())
		{
			Frame frame = mFrameQueue.front();
			mFrameQueue.pop();
			av_frame_unref(frame.mFrame);
			av_frame_free(&frame.mFrame);
		}

		// After clearing the frame queue, we also need to reset the video clock in order for playback to re-sync to possible new frames
		mVideoClockSecs = sVideoMax;
	}


	void Video::ioThread()
	{
		// Helper to free packet when it goes out of scope
		struct PacketWrapper
		{
			~PacketWrapper() { av_free_packet(mPacket); }
			AVPacket* mPacket = av_packet_alloc();
		};

		while (!mExitIOThreadSignalled)
		{
			// If a seek target is set, seek to that position
			if (mSeekTarget != -1)
			{
				int result = av_seek_frame(mFormatContext, mVideoStream, mSeekTarget, 0);
				if (result < 0)
				{
					mPacketsFinished = true;
					mErrorString = error_to_string(result);
					return;
				}

				// After seeking we should clear any queues. We will clear the packet queue here, the frame queue can only be cleared by the decode thread.
				clearPacketQueue();

				{
					// Add a 'null' packet to the queue, to signal the decoder thread that it should flush its queues and decoder buffer
					std::unique_lock<std::mutex> lock(mPacketQueueMutex);
					mPacketQueue.push(nullptr);
					mPacketAvailableCondition.notify_one();
				}
				mSeekTarget = -1;
			}

			// Read packets from the stream and push them onto the packet queue
			PacketWrapper packet;
			int result = av_read_frame(mFormatContext, packet.mPacket);

			// If we ended playback we either exit the loop or start from the beginning
			if (result == AVERROR_EOF)
			{
				if (mLoop)
				{
					// Signal a reset of the video stream
					seek(0.0f);
					continue;
				}
				else
				{
					// Exit loop
					mPacketsFinished = true;
					return;
				}
			}
			else if (result < 0)
			{
				mPacketsFinished = true;
				mErrorString = error_to_string(result);
				return;
			}

			// Is this a packet from the video stream?
			if (packet.mPacket->stream_index != mVideoStream)
				continue;

			{
				std::unique_lock<std::mutex> lock(mPacketQueueMutex);
				mPacketQueueRoomAvailableCondition.wait(lock, [this]() { return mPacketQueue.size() < 3 || mExitIOThreadSignalled; });
				if (mExitIOThreadSignalled)
					break;

				mPacketQueue.push(packet.mPacket);
				packet.mPacket = nullptr;

				mPacketAvailableCondition.notify_one();
			}
		}
	}


	void Video::exitIOThread()
	{
		mExitIOThreadSignalled = true;
		mPacketQueueRoomAvailableCondition.notify_one();
	}


	bool Video::decodeFrame(AVFrame& frame)
	{
		// Here we pull packets from the queue and decode them until all data for this frame is processed
		int frame_finished = 0;
		while (!frame_finished)
		{
			AVPacket* packet;
			{
				std::unique_lock<std::mutex> lock(mPacketQueueMutex);
				mPacketAvailableCondition.wait(lock, [this]() 
				{ 
					return !mPacketQueue.empty() || mExitDecodeThreadSignalled || mPacketsFinished; }
				);
				
				if (mExitDecodeThreadSignalled)
					return false;

				if (mPacketQueue.empty() && mPacketsFinished)
				{
					mFramesFinished = true;
					return false;
				}

				packet = mPacketQueue.front();
				mPacketQueue.pop();
				mPacketQueueRoomAvailableCondition.notify_one();
			}

			// If we dequeued a flush packet, flush all buffers for the video codec and clear the frame queue
			if (packet == nullptr)
			{
				avcodec_flush_buffers(mCodecContext);
				clearFrameQueue();
			}
			else
			{
				// Decode the frame
				int result = avcodec_decode_video2(mCodecContext, &frame, &frame_finished, packet);

				// Free the packet that was allocated by av_read_frame in the IO thread
				av_free_packet(packet);

				if (result < 0)
				{
					mFramesFinished = true;
					exitIOThread();
					mErrorString = error_to_string(result);
					return false;
				}
			}
		}

		return true;
	}


	void Video::decodeThread()
	{
		// Here we find the duration of a frame. This is used as a predication when the next frame needs to be show
		// in case the PTS in the video stream was not available
		AVRational frame_rate = av_guess_frame_rate(mFormatContext, mFormatContext->streams[mVideoStream], nullptr);
		double frame_duration = (frame_rate.num && frame_rate.den ? av_q2d(AVRational{ frame_rate.den, frame_rate.num }) : 0);

		// All timing information is relative to the stream start. If the stream start has no PTS value, we assume zero to be the start
		double stream_start_time = 0.0;
		if (mFormatContext->streams[mVideoStream]->start_time != AV_NOPTS_VALUE)
			stream_start_time = mFormatContext->streams[mVideoStream]->start_time *av_q2d(mFormatContext->streams[mVideoStream]->time_base);

		AVFrame* frame = av_frame_alloc();

		while (!mExitDecodeThreadSignalled)
		{
			if (!decodeFrame(*frame))
				break;

			// We calculate when the next frame needs to be displayed. The videostream contains PTS information, but there are cases
			// when there is no PTS available, we need to account for these cases.
			Frame new_frame;
			new_frame.mPTSSecs = sVideoMax;
			if (frame->pts == AV_NOPTS_VALUE)
			{
				// In case there is no PTS we use the previous PTS time plus the frame time as a best prediction. However, if there is 
				// no previous frame, we assume zero.
				new_frame.mPTSSecs = mPrevPTSSecs == sVideoMax ? 0.0 : mPrevPTSSecs + frame_duration;
			}
			else
			{
				// Use the PTS in the timespace of the video stream, starting from the videostream start
				new_frame.mPTSSecs = (frame->pts * av_q2d(mFormatContext->streams[mVideoStream]->time_base)) - stream_start_time;
			}

			mPrevPTSSecs = new_frame.mPTSSecs;

			// We MOVE the decoded frame to this new frame. It is freed when processed
			new_frame.mFrame = av_frame_alloc();
			av_frame_move_ref(new_frame.mFrame, frame);
			av_frame_unref(frame);

			// Push the frame onto the frame queue
			{
				std::unique_lock<std::mutex> lock(mFrameQueueMutex);
				mFrameQueueRoomAvailableCondition.wait(lock, [this]() { return mFrameQueue.size() < 3 || mExitDecodeThreadSignalled; });
				if (mExitDecodeThreadSignalled)
					break;

				mFrameQueue.push(new_frame);
				mFrameDataAvailableCondition.notify_one();
			}
		}

		av_frame_free(&frame);
	}


	void Video::exitDecodeThread()
	{
		mExitDecodeThreadSignalled = true;
		mPacketAvailableCondition.notify_one();
		mFrameQueueRoomAvailableCondition.notify_one();
	}


	bool Video::update(double deltaTime, utility::ErrorState& errorState)
	{
		if (!mPlaying)
			return true;

		// If the frametime spikes, make sure we re-sync to the first frame again, otherwise it may be possible that
		// the main thread is trying to catch up, but it never really can catch up
 		if (deltaTime > 1.0)
 			mVideoClockSecs = sVideoMax;

		// Update clock if it has been initialized
		if (mVideoClockSecs != sVideoMax)
			mVideoClockSecs += deltaTime;

		// Peek into the frame queue. If we have a frame and the PTS value of the first frame on
		// the FIFO queue has expired, we pop it. If there is no frame or the frame has not expired,
		// we return, effectively keeping the same contents in the existing textures.
		Frame cur_frame;
		{
			// This can be enabled to block waiting until data is available
			// mFrameDataAvailableCondition.wait(lock, [this]() { return !mFrameQueue.empty(); });

			if (mFrameQueue.empty() && mFramesFinished)
			{
				mPlaying = false;
				return errorState.check(mErrorString.empty(), mErrorString.c_str());
			}

			std::unique_lock<std::mutex> lock(mFrameQueueMutex);

			// Initialize the video clock to the first frame we see (if it has not been initialized yet)
			if (!mFrameQueue.empty() && mVideoClockSecs == sVideoMax)
			{
				mVideoClockSecs = mFrameQueue.front().mPTSSecs;
			}

			// UNCOMMENT FOR DEBUGGING LOOPING
			//std::cout << mVideoClockSecs << "\n";

			if (mFrameQueue.empty() || mVideoClockSecs < mFrameQueue.front().mPTSSecs)
				return true;

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

		return true;
	}
}