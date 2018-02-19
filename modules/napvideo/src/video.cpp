#include "video.h"
#include "videoservice.h"

// external includes
#include <rendertexture2d.h>
#include <imagefromfile.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <limits>

#include <SDL_audio.h>

extern "C"
{
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
	#include "libswresample/swresample.h"
}

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Video)
	RTTI_CONSTRUCTOR(nap::VideoService&)
	RTTI_PROPERTY_FILELINK("Path",	&nap::Video::mPath,		nap::rtti::EPropertyMetaData::Required, nap::rtti::EPropertyFileType::Video)
	RTTI_PROPERTY("Loop",	        &nap::Video::mLoop,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Speed",	        &nap::Video::mSpeed,	nap::rtti::EPropertyMetaData::Default)
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

	//////////////////////////////////////////////////////////////////////////

	AVState::~AVState()
	{
		close();
	}


	void AVState::close()
	{
		if (mCodec != nullptr)
		{
			mCodec = nullptr;
			avcodec_free_context(&mCodecContext);
			mCodecContext = nullptr;
		}
	}

	void AVState::clearPacketQueue()
	{
		std::unique_lock<std::mutex> lock(mPacketQueueMutex);
		while (!mPacketQueue.empty())
		{
			AVPacket* packet = mPacketQueue.front();
			mPacketQueue.pop();
			av_free_packet(packet);
		}
	}

	void AVState::clearFrameQueue()
	{
		std::unique_lock<std::mutex> lock(mFrameQueueMutex);
		while (!mFrameQueue.empty())
		{
			Frame frame = mFrameQueue.front();
			mFrameQueue.pop();
			av_frame_unref(frame.mFrame);
			av_frame_free(&frame.mFrame);
		}
	}


	bool AVState::matchesStream(const AVPacket& packet) const
	{
		return packet.stream_index == mStream;
	}


	bool AVState::addPacket(AVPacket* packet, const bool& exitIOThreadSignalled)
	{
		assert(packet == nullptr || matchesStream(*packet));

		std::unique_lock<std::mutex> lock(mPacketQueueMutex);
		mPacketQueueRoomAvailableCondition.wait(lock, [this, &exitIOThreadSignalled]() { return mPacketQueue.size() < 3 || exitIOThreadSignalled; });
		if (exitIOThreadSignalled)
			return false;

		mPacketQueue.push(packet);
		mPacketAvailableCondition.notify_one();

		return true;
	}

	
	void AVState::notifyPacketQueueRoomAvailable()
	{
		mPacketQueueRoomAvailableCondition.notify_one();
	}
	

	void AVState::setCodec(AVCodec* codec, AVCodecContext* codecContext)
	{
		mCodec = codec;
		mCodecContext = codecContext;
	}

	void AVState::startDecodeThread(const DecodeFunction& decodeFunction)
	{
		mExitDecodeThreadSignalled = false;
		mFinishedProducingFrames = false;
		mIOFinishedProducingPackets = false;

		mDecodeFunction = decodeFunction;
		mDecodeThread = std::thread(std::bind(&AVState::decodeThread, this));
	}


	void AVState::exitDecodeThread(bool join)
	{
		mExitDecodeThreadSignalled = true; 
		mPacketAvailableCondition.notify_one();
		mFrameQueueRoomAvailableCondition.notify_one();
		mFrameDataAvailableCondition.notify_one();
		if (join)
			mDecodeThread.join();
	}


	bool AVState::isFinishedConsuming() const
	{
		std::unique_lock<std::mutex> lock(mFrameQueueMutex);
		return mFrameQueue.empty();
	}


	void AVState::decodeThread()
	{
		// Here we find the duration of a frame. This is used as a predication when the next frame needs to be show
		// in case the PTS in the video stream was not available
		AVRational frame_rate = av_guess_frame_rate(mVideo->mFormatContext, mVideo->mFormatContext->streams[mStream], nullptr);
		double frame_duration = (frame_rate.num && frame_rate.den ? av_q2d(AVRational{ frame_rate.den, frame_rate.num }) : 0);

		// All timing information is relative to the stream start. If the stream start has no PTS value, we assume zero to be the start
		double stream_start_time = 0.0;
		if (mVideo->mFormatContext->streams[mStream]->start_time != AV_NOPTS_VALUE)
			stream_start_time = mVideo->mFormatContext->streams[mStream]->start_time *av_q2d(mVideo->mFormatContext->streams[mStream]->time_base);

		AVFrame* frame = av_frame_alloc();

		while (!mExitDecodeThreadSignalled)
		{
			if (!decodeFrame(*frame))
				break;

			// We calculate when the next frame needs to be displayed. The videostream contains PTS information, but there are cases
			// when there is no PTS available, we need to account for these cases.
			Frame new_frame;
			new_frame.mPTSSecs = Video::sVideoMax;
			if (frame->pts == AV_NOPTS_VALUE)
			{
				// In case there is no PTS we use the previous PTS time plus the frame time as a best prediction. However, if there is 
				// no previous frame, we assume zero.
				new_frame.mPTSSecs = mPrevPTSSecs == Video::sVideoMax ? 0.0 : mPrevPTSSecs + frame_duration;
			}
			else
			{
				// Use the PTS in the timespace of the video stream, starting from the videostream start
				new_frame.mPTSSecs = (frame->pts * av_q2d(mVideo->mFormatContext->streams[mStream]->time_base)) - stream_start_time;
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


	void AVState::notifyFinishedProducingPackets()
	{
		mIOFinishedProducingPackets = true;
		mPacketAvailableCondition.notify_one();
	}


	bool AVState::decodeFrame(AVFrame& frame)
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
					return !mPacketQueue.empty() || mExitDecodeThreadSignalled || mIOFinishedProducingPackets;
				});

				if (mExitDecodeThreadSignalled)
					return false;

				if (mPacketQueue.empty() && mIOFinishedProducingPackets)
				{
					mFinishedProducingFrames = true;
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
				int result = mDecodeFunction(mCodecContext, &frame, &frame_finished, packet);

				// Free the packet that was allocated by av_read_frame in the IO thread
				av_free_packet(packet);

				if (result < 0)
				{
					mVideo->setErrorOccurred(error_to_string(result));
					return false;
				}
			}
		}

		return true;
	}

	Frame AVState::popFrame()
	{
		std::unique_lock<std::mutex> lock(mFrameQueueMutex);
		mFrameDataAvailableCondition.wait(lock, [this]() {

			return mExitDecodeThreadSignalled || !mFrameQueue.empty();
		});

		if (mExitDecodeThreadSignalled)
			return Frame();

		Frame frame = mFrameQueue.front();
		mFrameQueue.pop();

		mFrameQueueRoomAvailableCondition.notify_one();

		return frame;
	}

	Frame AVState::tryPopFrame(double pts)
	{
		std::unique_lock<std::mutex> lock(mFrameQueueMutex);
		if (mFrameQueue.empty() || pts < mFrameQueue.front().mPTSSecs)
			return Frame();

		Frame cur_frame = mFrameQueue.front();
		mFrameQueue.pop();

		mFrameQueueRoomAvailableCondition.notify_one();

		return cur_frame;
	}

	Frame AVState::peekFrame()
	{
		std::unique_lock<std::mutex> lock(mFrameQueueMutex);
		if (mFrameQueue.empty())
			return Frame();

		return mFrameQueue.front();
	}


	//////////////////////////////////////////////////////////////////////////


	Video::Video(VideoService& service) : 
		mService(service),
		mAudioState(*this),
		mVideoState(*this)
	{
	}


	Video::~Video()
	{
		if (mFormatContext != nullptr)
			mService.removeVideoPlayer(*this);
		stop();
		
		mAudioState.close();
		mVideoState.close();

		avformat_close_input(&mFormatContext);
		avformat_free_context(mFormatContext);
	}


	bool Video::sInitCodec(AVState& destState, const AVCodecContext& sourceCodecContext, AVDictionary*& options, utility::ErrorState& errorState)
	{
		// Find the decoder for the video stream
		AVCodec* codec = avcodec_find_decoder(sourceCodecContext.codec_id);
		if (!errorState.check(codec != nullptr, "Unable to find codec for video stream"))
			return false;

		// Copy context
		AVCodecContext* codec_context = avcodec_alloc_context3(codec);
		int error = avcodec_copy_context(codec_context, &sourceCodecContext);
		if (!errorState.check(error == 0, "Unable to copy codec context: %s", error_to_string(error).c_str()))
			return false;

		error = avcodec_open2(codec_context, codec, &options);
		if (!errorState.check(error == 0, "Unable to open codec: %s", error_to_string(error).c_str()))
			return false;

		destState.setCodec(codec, codec_context);

		return true;
	}


	bool Video::init(nap::utility::ErrorState& errorState)
	{
		mFormatContext = avformat_alloc_context();
		if (!errorState.check(mFormatContext != nullptr, "Error allocating context"))
			return false;

		int error = avformat_open_input(&mFormatContext, mPath.c_str(), nullptr, nullptr);
		if (!errorState.check(error >= 0, "Error opening file '%s': %s\n", mPath.c_str(), error_to_string(error).c_str()))
			return false;

		error = avformat_find_stream_info(mFormatContext, nullptr);
		if (!errorState.check(error >= 0, "Error finding stream: %s", error_to_string(error).c_str()))
			return false;

		// Enable this to dump information about file onto standard error
		//av_dump_format(mFormatContext, 0, mPath.c_str(), 0);

		// Find the index and codec context of the video stream. The codec is stored in a local 
		// as it needs to be copied later
		AVCodecContext* source_video_codec_context = nullptr;
		AVCodecContext* source_audio_codec_context = nullptr;
		for (int i = 0; i < mFormatContext->nb_streams; ++i)
		{
			if (mFormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
			{
				source_video_codec_context = mFormatContext->streams[i]->codec;
				mVideoState.setStream(i);
			}
			else if (mFormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
			{
				source_audio_codec_context = mFormatContext->streams[i]->codec;
				mAudioState.setStream(i);
			}
		}

		if (!errorState.check(source_video_codec_context != nullptr, "No video stream found"))
			return false;

		// This option causes the codec context to spawn threads internally for decoding, speeding up the decoding process
		AVDictionary* options = nullptr;
		av_dict_set(&options, "threads", "auto", 0);

		// We need to set this option to make sure that the decoder transfers ownership from decode buffers to us
		// when we decode frames. Otherwise, the decoder will reuse buffers, which will then overwrite data already in our queue.
		av_dict_set(&options, "refcounted_frames", "1", 0);

		if (!sInitCodec(mVideoState, *source_video_codec_context, options, errorState))
			return false;

		if (source_audio_codec_context != nullptr)
			if (!sInitCodec(mAudioState, *source_audio_codec_context, options, errorState))
				return false;

		AVCodecContext& video_codec_context = mVideoState.getCodecContext();
		mWidth = video_codec_context.width;
		mHeight = video_codec_context.height;
		mDuration = static_cast<double>(mFormatContext->duration / AV_TIME_BASE);

		float yWidth	= video_codec_context.width;
		float yHeight	= video_codec_context.height;
		float uvWidth	= video_codec_context.width * 0.5f;
		float uvHeight	= video_codec_context.height * 0.5f;

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

		mYTexture = std::make_unique<RenderTexture2D>();
		mYTexture->mWidth = yWidth;
		mYTexture->mHeight = yHeight;
		mYTexture->mFormat = RenderTexture2D::EFormat::R8;
		mYTexture->mParameters = parameters;
		if (!mYTexture->init(errorState))
			return false;

		mYTexture->update(y_default_data.data());

		mUTexture = std::make_unique<RenderTexture2D>();
		mUTexture->mWidth = uvWidth;
		mUTexture->mHeight = uvHeight;
		mUTexture->mFormat = RenderTexture2D::EFormat::R8;
		mUTexture->mParameters = parameters;
		if (!mUTexture->init(errorState))
			return false;
				
		mUTexture->update(uv_default_data.data());

		mVTexture = std::make_unique<RenderTexture2D>();
		mVTexture->mWidth = uvWidth;
		mVTexture->mHeight = uvHeight;
		mVTexture->mFormat = RenderTexture2D::EFormat::R8;
		mVTexture->mParameters = parameters;
		if (!mVTexture->init(errorState))
			return false;

		mVTexture->update(uv_default_data.data());

		// Register with service
		mService.registerVideoPlayer(*this);

		return true;
	}


	void Video::play(double startTimeSecs)
	{
		assert(!mPlaying);

		// Reset all state to make sure threads don't immediately exit from a previous 'stop' call
		mPlaying = true;
		mVideoClockSecs = sVideoMax;

		seek(startTimeSecs);

		mVideoState.startDecodeThread(std::bind(&avcodec_decode_video2, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
		
		if (mAudioState.isValid())
			mAudioState.startDecodeThread(std::bind(&avcodec_decode_audio4, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));

		startIOThread();
	}


	void Video::stop()
	{
		if (!mPlaying)
			return;

		mVideoState.exitDecodeThread(true);

		if (mAudioState.isValid())
			mAudioState.exitDecodeThread(true);

		exitIOThread(true);

		mPlaying = false;

		clearPacketQueue();
		clearVideoFrameQueue();
		clearAudioFrameQueue();
	}


	void Video::seek(double seconds)
	{
		// All timing information is relative to the stream start. If the stream start has no PTS value, we assume zero to be the start
		double stream_start_time = 0.0;
		if (mFormatContext->streams[mVideoState.getStream()]->start_time != AV_NOPTS_VALUE)
			stream_start_time = mFormatContext->streams[mVideoState.getStream()]->start_time *av_q2d(mFormatContext->streams[mVideoState.getStream()]->time_base);

		mSeekTarget = std::round((seconds - stream_start_time) / av_q2d(mFormatContext->streams[mVideoState.getStream()]->time_base));
	}


	void Video::clearPacketQueue()
	{
		mVideoState.clearPacketQueue();
		mAudioState.clearPacketQueue();
	}

	void Video::clearVideoFrameQueue()
	{
		mVideoState.clearFrameQueue();

		// After clearing the frame queue, we also need to reset the video clock in order for playback to re-sync to possible new frames
		mVideoClockSecs = sVideoMax;
	}


	void Video::clearAudioFrameQueue()
	{
		mAudioState.clearFrameQueue();
	}


	bool Video::hasErrorOccurred() const
	{
		return mErrorOccurred;
	}


	void Video::setErrorOccurred(const std::string& errorMessage)
	{
		mErrorOccurred = true;
		mErrorMessage = errorMessage;
		exitIOThread(false);

		if (mAudioState.isValid())
			mAudioState.exitDecodeThread(false);

		mVideoState.exitDecodeThread(false);
	}


	void Video::onFinishedProducingPackets()
	{
		mVideoState.notifyFinishedProducingPackets();
		mAudioState.notifyFinishedProducingPackets();
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
				int result = av_seek_frame(mFormatContext, mVideoState.getStream(), mSeekTarget, 0);
				if (result < 0)
				{
					setErrorOccurred(error_to_string(result));
					return;
				}

				// After seeking we should clear any queues. We will clear the packet queue here, the frame queue can only be cleared by the decode thread.
				clearPacketQueue();

				// Add a 'null' packet to the queue, to signal the decoder thread that it should flush its queues and decoder buffer
				mVideoState.addPacket(nullptr, mExitIOThreadSignalled);

				if (mAudioState.isValid())
					mAudioState.addPacket(nullptr, mExitIOThreadSignalled);

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
					onFinishedProducingPackets();
					return;
				}
			}
			else if (result < 0)
			{
				setErrorOccurred(error_to_string(result));
				return;
			}

			if (mVideoState.matchesStream(*packet.mPacket))
			{
				if (mVideoState.addPacket(packet.mPacket, mExitIOThreadSignalled))
					packet.mPacket = nullptr;
			}
			else if (mAudioState.matchesStream(*packet.mPacket))
			{
				if (mAudioState.addPacket(packet.mPacket, mExitIOThreadSignalled))
					packet.mPacket = nullptr;
			}
		}
	}

	void Video::startIOThread()
	{
		mExitIOThreadSignalled = false;
		mIOThread = std::thread(std::bind(&Video::ioThread, this));
	}


	void Video::exitIOThread(bool join)
	{
		mExitIOThreadSignalled = true;
		
		// Unlock I/O thread, it might be waiting for room to become available in the packet queue through AVState::addPacket
		mVideoState.notifyPacketQueueRoomAvailable();
		mAudioState.notifyPacketQueueRoomAvailable();

		if (join)
			mIOThread.join();
	}


	void Video::getNextAudioFrame(const AudioParams& audioHwParams)
	{
		if (mCurrentAudioFrame.mFrame != nullptr)
		{
			av_frame_unref(mCurrentAudioFrame.mFrame);
			av_frame_free(&mCurrentAudioFrame.mFrame);
		}

		mCurrentAudioFrame = mAudioState.popFrame();

		int buffer_size = av_samples_get_buffer_size(NULL, mCurrentAudioFrame.mFrame->channels, mCurrentAudioFrame.mFrame->nb_samples, (AVSampleFormat)mCurrentAudioFrame.mFrame->format, 1);

		AVFrame* frame = mCurrentAudioFrame.mFrame;
		int64_t dec_channel_layout = (frame->channel_layout && frame->channels == av_get_channel_layout_nb_channels(frame->channel_layout)) ?
			frame->channel_layout : av_get_default_channel_layout(frame->channels);


		if ((frame->format != (AVSampleFormat)audioHwParams.fmt ||
			dec_channel_layout != audioHwParams.channel_layout ||
			frame->sample_rate != audioHwParams.freq) &&
			mAudioResampleContext == nullptr) 
		{
			mAudioResampleContext = swr_alloc_set_opts(NULL, audioHwParams.channel_layout, (AVSampleFormat)audioHwParams.fmt, audioHwParams.freq, dec_channel_layout, (AVSampleFormat)frame->format, frame->sample_rate, 0, NULL);
			int result = swr_init(mAudioResampleContext);
			assert(result >= 0);
		}

		if (mAudioResampleContext != nullptr)
		{
			const uint8_t **in = (const uint8_t **)frame->extended_data;
			int out_count = (int64_t)frame->nb_samples * audioHwParams.freq / frame->sample_rate + 256;
			int out_size = av_samples_get_buffer_size(NULL, audioHwParams.channels, out_count, (AVSampleFormat)audioHwParams.fmt, 0);
			assert(out_size >= 0);
			int len2;

			mAudioResampleBuffer.resize(out_size);
			uint8_t* resample_buffer = mAudioResampleBuffer.data();
			uint8_t** out = &resample_buffer;
			len2 = swr_convert(mAudioResampleContext, out, out_count, in, frame->nb_samples);
			assert(len2 >= 0);
			assert(len2 != out_count);

			mCurrentAudioBuffer = mAudioResampleBuffer.data();
			buffer_size = len2 * audioHwParams.channels * av_get_bytes_per_sample((AVSampleFormat)audioHwParams.fmt);
		}
		else 
		{
			mCurrentAudioBuffer = frame->data[0];
		}

		mAudioFrameSize = buffer_size;
		mAudioFrameReadPtr = 0;
	}
	
	void Video::OnAudioCallback(uint8_t* stream, int len, const AudioParams& audioHwParams)
	{
		uint8_t* dest = stream;
		int data_remaining = len;
		while (data_remaining > 0)
		{
			if (mAudioFrameReadPtr >= mAudioFrameSize)
				getNextAudioFrame(audioHwParams);

			int num_bytes_to_read = mAudioFrameSize - mAudioFrameReadPtr;
			num_bytes_to_read = std::min(num_bytes_to_read, data_remaining);

			uint8_t* source = mCurrentAudioBuffer + mAudioFrameReadPtr;
			memcpy(dest, source, num_bytes_to_read);

			data_remaining -= num_bytes_to_read;
			dest += num_bytes_to_read;
			mAudioFrameReadPtr += num_bytes_to_read;
		}
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
			mVideoClockSecs += (deltaTime * mSpeed);

		// Peek into the frame queue. If we have a frame and the PTS value of the first frame on
		// the FIFO queue has expired, we pop it. If there is no frame or the frame has not expired,
		// we return, effectively keeping the same contents in the existing textures.
		Frame cur_frame;
		{
			// This can be enabled to block waiting until data is available
			// mFrameDataAvailableCondition.wait(lock, [this]() { return !mFrameQueue.empty(); });

			if (mVideoState.isFinished())
			{
				mPlaying = false;
				return errorState.check(mErrorMessage.empty(), mErrorMessage.c_str());
			}

			// Initialize the video clock to the first frame we see (if it has not been initialized yet)
			if (mVideoClockSecs == sVideoMax)
			{
				Frame frame = mVideoState.peekFrame();
				if (frame.isValid())
					mVideoClockSecs = frame.mPTSSecs;
			}

			// Try to get next frame to display (based on video clock)
			cur_frame = mVideoState.tryPopFrame(mVideoClockSecs);
			if (!cur_frame.isValid())
				return true;
		}

		// Copy data into texture
		mYTexture->update(cur_frame.mFrame->data[0], cur_frame.mFrame->linesize[0]);
		mUTexture->update(cur_frame.mFrame->data[1], cur_frame.mFrame->linesize[1]);
		mVTexture->update(cur_frame.mFrame->data[2], cur_frame.mFrame->linesize[2]);

		// Destroy frame that was allocated in the decode thread, after it has been processed
		av_frame_unref(cur_frame.mFrame);
		av_frame_free(&cur_frame.mFrame);

		return true;
	}
}