/*

----------------------------------------------------
Basic Threading structure and data flow
----------------------------------------------------
In the videoplayer, there are up to 5 threads in play:
- I/O thread				Reads 'packets' from file and pushes them onto the packet queue until it is full
- Decode thread (2)			There is a thread for decoding video packets and one for decoding audio packets. Packets are popped from the packet queue, 
							decoded into frames and then pushed onto the frame queue until it is full.
- Audio consume thread		This thread is not managed by the videoplayer, but it is the thread that will emit a callback at a certain fixed frequency to fill in the audio data. 
							This thread will pop frames from the queue, perform any resampling to the target format, and eventually copy the data to the target buffer.
- Video consume thread		This thread is not managed by the videoplayer, it is the thread that needs to call Video::update(). This call will pop frames from the frame queue
							and update the internal textures.

If there is no audio in the stream, the audio decode thread is not started. To summarize the regular playing data flow: 
 =>	Push packets into packet queue (I/O thread) => Pop packets from queue (a/v decode thread) => Push frames into frame queue (a/v decode thread) => pop frames (a/v consume threads)

 Internally, the decoders spawned multiple threads to decode frames as efficiently as possible. This is enabled by using av_dict_set(..., "threads", ...).

----------------------------------------------------
Threading communication
----------------------------------------------------
The threads communicate mainly using queues. Sometimes the threads need to give 'commands' to the other threads. To simplify the inter-thread communication, some of this
communication is performed by pushing specific packets onto the queue (seek start/seek end/end of file/io finished). When a thread wants to wait until a certain packet 
is consumed, AutoResetEvents are used. The threads act like traditional producer/consumers, so the queues are protected with condition variables to accomplish this: both
producers and consumers can wait on each other: producers can wait for room to be available, consumers can wait until data is available.

----------------------------------------------------
Fundamentals of the video format
----------------------------------------------------
A video file has multiple streams in it. We are using the audio and video streams, but there may also be subtitle or other streams in it.
The video stream contains frames of the following compression types:
- I-frame (intra-coded picture). These are keyframes that contain a full image.
- P-frame (predicated picture). These contain only the difference from the previous picture.
- B-frame (bi-directional frames). These contain information that not only relies on the previous picture, but also from the next picture.
See also: https://en.wikipedia.org/wiki/Video_compression_picture_types

Although it seems like we need to deal with these different types of frames manually, we actually don't. We feed one or more packets into avcodec_receive_frame,
and internally the codec will produce a frame for us. It is important to understand that I-frames (keyframes) are stored only once in a number of frames. This is 
important when we look at seeking (see description on seeking below).

Furthermore, for the structure of the code it is important to understand the following things:
	1) A single packet can result in multiple frames.
	2) Multiple packets can result in a single frame.
Specifically when looking at decodeFrame, you can see how the loop is setup to support both these cases.

----------------------------------------------------
Timing
----------------------------------------------------
Packets contain both a Decode TimeStamp (DTS) and a Presentation TimeStamp (PTS). Again, we don't really need to deal with the decoding timestamp, as av_receive_frame
deals with this for us. The presentation timestamp is what we need to see if the next frame should be displayed. Often the PTS is missing, so some logic is required
to guess when the next frame should be displayed. We have seen code that tries to calculate this manually, but these days we can use 'best_effort_timestamp' to give
the most accurate prediction of when the frame should be displayed, so we use this instead. 
Although we don't need to deal with the DTS, it is still relevant for us, as seeking uses the DTS instead of the PTS (see below).

Streams operate in a certain 'time base'. This is just the frequency in hz, and internally it is stored as a fraction (nominator/denominator) and it can be converted to
a proper faction by using av_q2d. Each stream also has a start_time. We use the start time and frequency to convert from seconds to the streams' timebase.

----------------------------------------------------
Synchronization
----------------------------------------------------
Each stream in a video file (audio, video) has its own timebase and PTS/DTS values. When displaying frames/playing back audio, we need to make sure that these two streams are in 'sync'.
In order to be able to sync these streams, we maintain a few different 'clocks':
	- Audio decode clock:	This clock is updated whenever a new audio frame is consumed from the audio frame queue. It updates the clock to point to the *end* of the decoded frame (i.e. *start* of the next frame)
	- Audio master clock:	This clock is updated whenever the audio callback is called. This clock is based on the audio decode clock: in order to compensate for the delay in audio submission and actual playback,
							we subtract the 'length' of the callback buffer (in time) from the decode clock. This new subtracted value serves as the 'actual' time. For more details, see the audio callback.
	- System clock:			This clock is updated with the deltatime on every update of the Video.

Syncing is achieved by simply only presenting new video frames if the clock has advanced far enough. Depending on whether the video has an audio stream or not, either the audio clock or the system clock
is used to determine whether the frame should be presented. If the audio is too far ahead of the video, video frames will be dropped. If the audio is behind the video, the frames will simply be displayed for longer periods of time.

Note that in the case where the video has an audio stream, this implies a dependency of the video stream on the audio stream. No frames will be presented if the audio clock doesn't advance,
which could lead to the frame queue being full, which in turn can lead to the decode & IO threads waiting for the frame queue to be emptied.

----------------------------------------------------
Seeking
----------------------------------------------------
Seeking is challenging for the following reasons:
- We can only seek by DTS, not PTS. So when seeking, you need to seek to some unknown number that is smaller or equal to the PTS value.
- We can only seek to keyframes. If we'd seek to direct frames, we will see corrupted data as the data relies on previous frames.
- As we need to read packets and produce frames and still have the same proper error handling for all these things like we have in the regular playing pipeline, 
  we actually need all the logic from the regular playing pipeline that is spread across threads to find our target frame. The entire playing pipeline is used 
  for this reason, but there are some important things to notice:
	* The output frames that are produced by the decode thread are temporary and should not be put into the regular target frame queue. Instead, the I/O should
	  somehow consume the 'temporary' frames.
	* We should only read packets up until the target frame and not read more packets 'ahead', otherwise, any excess packets will not end up in the target frame queue.

Because of these reasons, we need to perform the following steps:
1) When starting the seeking operation, the I/O thread will tell the decode thread that a seek operation started. The decode thread will flip the regular 
   frame queue to a 'seeking' frame queue. The I/O thread will wait until the decode thread has issued this command.
2) The I/O thread will first search iteratively for the correct starting keyframe that is smaller or equal to our target PTS. This operation is iteratively
   because we can only guess the target DTS. We cache DTS of the first packet that was used to produce the frame, so for any subsequent searches, we
   subtract one for the frames' first DTS. We do this until we find a keyframe that is smaller or equal to the target PTS. We can detect if we are at the 
   beginning of the stream, as the stream contains the very first DTS that is present in the stream. If we hit the first DTS, we do not continue the iteration.
3) From the correct starting keyframe, continue decoding until we find a frame that is equal to or greater than our target PTS.
4) When the target frame is found, the I/O thread will tell the decode thread the seek operation ended. The decode thread will flip back to the regular frame queue.

Steps 2 and 3 are performed in a lock-step fashion between the I/O thread and the video decode thread, to make sure that we do not seek any packets ahead. This lock-step
behavior is accomplished by pushing a single packet on the queue, and then waiting until the decode thread calls avcodec_receive_frame. When avcoded_receive requires 
another packet, we push another packet on the queue. This way we never push more packets onto the queue than necessary to produce the next frame that we can inspect.

Finally, if there is an audio stream available, we prefer seeking in the audio stream as we're synchronizing video to audio.
This is important in situations where the audio packets are physically ahead of the video packets. Consider the following packet layout:

Audio PTS: 0	   1 		 2
           AAAAAAAAAAAAAAAAA AAAAAAAAAA
Video PTS:                  0 		   1 2
                            V          V V

In this example, when seeking to video packet with PTS 0, we will start processing audio packets with PTS 2. As we're syncing to the audio clock, 
this means that all video frames up until time 2 will be dropped.
*/

#include "video.h"
#include "videoservice.h"
#include "rendertarget.h"

// external includes
#include <rendertexture2d.h>
#include <imagefromfile.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <limits>
#include "nap/logger.h"

extern "C"
{
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
	#include <libavutil/pixfmt.h>
	#include "libswresample/swresample.h"
}

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Video)
	RTTI_CONSTRUCTOR(nap::VideoService&)
	RTTI_PROPERTY_FILELINK("Path",	&nap::Video::mPath,		nap::rtti::EPropertyMetaData::Required, nap::rtti::EPropertyFileType::Video)
	RTTI_PROPERTY("Loop",	        &nap::Video::mLoop,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Speed",	        &nap::Video::mSpeed,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

#define VIDEO_DEBUG 0
#if VIDEO_DEBUG
	#define VIDEO_DEBUG_LOG(...) nap::Logger::info(__VA_ARGS__)
#else
	#define VIDEO_DEBUG_LOG(...)
#endif

namespace nap
{
	// Max video value, used as state check
	const double Video::sClockMax = std::numeric_limits<double>::max();


	const std::string sErrorToString(int err)
	{
		char error_buf[256];
		av_make_error_string(error_buf, sizeof(error_buf), err);
		return std::string(error_buf);
	}

	/*
	// Debug function to write a frame to jpeg
	void sWriteJPEG(AVCodecContext* videoCodecContext, AVFrame* videoFrame, int frameIndex, double framePTSSecs)
	{
		// Set up jpeg codec context
		AVCodecContext* jpeg_codec_context = avcodec_alloc_context3(NULL);
		assert(jpeg_codec_context != nullptr);
		jpeg_codec_context->bit_rate = videoCodecContext->bit_rate;
		jpeg_codec_context->width = videoCodecContext->width;
		jpeg_codec_context->height = videoCodecContext->height;
		jpeg_codec_context->pix_fmt = AV_PIX_FMT_YUVJ420P;
		jpeg_codec_context->codec_id = AV_CODEC_ID_MJPEG;
		jpeg_codec_context->codec_type = AVMEDIA_TYPE_VIDEO;
		jpeg_codec_context->time_base.num = videoCodecContext->time_base.num;
		jpeg_codec_context->time_base.den = videoCodecContext->time_base.den;

		// Find jpeg encoder
		AVCodec* jpeg_codec = avcodec_find_encoder(jpeg_codec_context->codec_id);
		if (!jpeg_codec)
			return;

		// Open the encoder
		if (avcodec_open2(jpeg_codec_context, jpeg_codec, nullptr) < 0)
			return;

		jpeg_codec_context->mb_lmin = jpeg_codec_context->lmin = jpeg_codec_context->qmin * FF_QP2LAMBDA;
		jpeg_codec_context->mb_lmax = jpeg_codec_context->lmax = jpeg_codec_context->qmax * FF_QP2LAMBDA;
		jpeg_codec_context->flags = CODEC_FLAG_QSCALE;
		jpeg_codec_context->global_quality = jpeg_codec_context->qmin * FF_QP2LAMBDA;

		videoFrame->pts = 1;
		videoFrame->quality = jpeg_codec_context->global_quality;

		// Determine frame size and allocate appropriate buffer
		int jpeg_size = avpicture_get_size(AV_PIX_FMT_YUVJ420P, videoCodecContext->width, videoCodecContext->height);
		std::vector<uint8_t> jpeg_buffer(jpeg_size, 0);

		// Construct packet pointing to jpeg buffer
		AVPacket packet;
		packet.data = jpeg_buffer.data();
		packet.size = jpeg_buffer.size();

		// Encode video frame to jpeg buffer (through packet)
		int got_packet = 0;
		if (avcodec_encode_video2(jpeg_codec_context, &packet, videoFrame, &got_packet) < 0)
		{
			avcodec_close (jpeg_codec_context);
			return;
		}

		// Determine filename & write to disk
		char jpeg_name[512] = { 0 };
		sprintf (jpeg_name, "screenshots\\%06d_%f.jpg", frameIndex, framePTSSecs);

		FILE* jpeg_file = fopen(jpeg_name, "wb");
		if (jpeg_file != nullptr)
		{
			fwrite(jpeg_buffer.data(), sizeof(uint8_t), jpeg_buffer.size(), jpeg_file);
			fclose(jpeg_file);
		}

		avcodec_close(jpeg_codec_context);
	}
	*/

	//////////////////////////////////////////////////////////////////////////

	// Helper to free packet when it goes out of scope
	struct PacketWrapper
	{
		~PacketWrapper() { av_packet_free(&mPacket); }
		AVPacket* mPacket = av_packet_alloc();
	};

	//////////////////////////////////////////////////////////////////////////

	static int64_t toFFMpegChannelLayout(AudioFormat::EChannelLayout channelLayout)
	{
		int64_t result;

		switch (channelLayout)
		{
		case AudioFormat::EChannelLayout::Mono:
			result = AV_CH_LAYOUT_MONO;
			break;
		case AudioFormat::EChannelLayout::Stereo:
			result = AV_CH_LAYOUT_STEREO;
			break;
		case AudioFormat::EChannelLayout::_2Point1:
			result = AV_CH_LAYOUT_2POINT1;
			break;
		case AudioFormat::EChannelLayout::_2_1:
			result = AV_CH_LAYOUT_2_1;
			break;
		case AudioFormat::EChannelLayout::Surround:
			result = AV_CH_LAYOUT_SURROUND;
			break;
		case AudioFormat::EChannelLayout::_3Point1:
			result = AV_CH_LAYOUT_3POINT1;
			break;
		case AudioFormat::EChannelLayout::_4Point0:
			result = AV_CH_LAYOUT_4POINT0;
			break;
		case AudioFormat::EChannelLayout::_4Point1:
			result = AV_CH_LAYOUT_4POINT1;
			break;
		case AudioFormat::EChannelLayout::_2_2:
			result = AV_CH_LAYOUT_2_2;
			break;
		case AudioFormat::EChannelLayout::Quad:
			result = AV_CH_LAYOUT_QUAD;
			break;
		case AudioFormat::EChannelLayout::_5Point0:
			result = AV_CH_LAYOUT_5POINT0;
			break;
		case AudioFormat::EChannelLayout::_5Point1:
			result = AV_CH_LAYOUT_5POINT1;
			break;
		case AudioFormat::EChannelLayout::_5Point0_Back:
			result = AV_CH_LAYOUT_5POINT0_BACK;
			break;
		case AudioFormat::EChannelLayout::_5Point1_Back:
			result = AV_CH_LAYOUT_5POINT1_BACK;
			break;
		case AudioFormat::EChannelLayout::_6Point0:
			result = AV_CH_LAYOUT_6POINT0;
			break;
		case AudioFormat::EChannelLayout::_6Point0_Front:
			result = AV_CH_LAYOUT_6POINT0_FRONT;
			break;
		case AudioFormat::EChannelLayout::Hexagonal:
			result = AV_CH_LAYOUT_HEXAGONAL;
			break;
		case AudioFormat::EChannelLayout::_6Point1:
			result = AV_CH_LAYOUT_6POINT1;
			break;
		case AudioFormat::EChannelLayout::_6Point1_Back:
			result = AV_CH_LAYOUT_6POINT1_BACK;
			break;
		case AudioFormat::EChannelLayout::_6Point1_Front:
			result = AV_CH_LAYOUT_6POINT1_FRONT;
			break;
		case AudioFormat::EChannelLayout::_7Point0:
			result = AV_CH_LAYOUT_7POINT0;
			break;
		case AudioFormat::EChannelLayout::_7Point0_Front:
			result = AV_CH_LAYOUT_7POINT0_FRONT;
			break;
		case AudioFormat::EChannelLayout::_7Point1:
			result = AV_CH_LAYOUT_7POINT1;
			break;
		case AudioFormat::EChannelLayout::_7Point1_Wide:
			result = AV_CH_LAYOUT_7POINT1_WIDE;
			break;
		case AudioFormat::EChannelLayout::_7Point1_Wide_Back:
			result = AV_CH_LAYOUT_7POINT1_WIDE_BACK;
			break;
		case AudioFormat::EChannelLayout::Octagonal:
			result = AV_CH_LAYOUT_OCTAGONAL;
			break;
		case AudioFormat::EChannelLayout::Hexadecagonal:
			result = AV_CH_LAYOUT_HEXAGONAL;
			break;
		case AudioFormat::EChannelLayout::Stereo_Downmix:
			result = AV_CH_LAYOUT_STEREO_DOWNMIX;
			break;
		}
		return result;
	}


	static int toFFMpegSampleFormat(AudioFormat::ESampleFormat sampleFormat)
	{
		int result;

		switch (sampleFormat)
		{
		case AudioFormat::ESampleFormat::U8:
			result = AV_SAMPLE_FMT_U8;
			break;
		case AudioFormat::ESampleFormat::S16:
			result = AV_SAMPLE_FMT_S16;
			break;
		case AudioFormat::ESampleFormat::S32:
			result = AV_SAMPLE_FMT_S32;
			break;
		case AudioFormat::ESampleFormat::FLT:
			result = AV_SAMPLE_FMT_FLT;
			break;
		case AudioFormat::ESampleFormat::DBL:
			result = AV_SAMPLE_FMT_DBL;
			break;
		case AudioFormat::ESampleFormat::S64:
			result = AV_SAMPLE_FMT_S64;
			break;
		}

		return result;
	}


	AudioFormat::AudioFormat(EChannelLayout channelLayout, ESampleFormat sampleFormat, int sampleRate) :
		mSampleRate(sampleRate),
		mChannelLayout(toFFMpegChannelLayout(channelLayout)),
		mSampleFormat(toFFMpegSampleFormat(sampleFormat))
	{
	}


	AudioFormat::AudioFormat(int numChannels, ESampleFormat sampleFormat, int sampleRate) :
		mSampleRate(sampleRate),
		mChannelLayout(av_get_default_channel_layout(numChannels)),
		mSampleFormat(toFFMpegSampleFormat(sampleFormat))
	{
	}

	//////////////////////////////////////////////////////////////////////////

	void Frame::free()
	{
		if (isValid())
		{
			av_frame_unref(mFrame);
			av_frame_free(&mFrame);
			mFrame = nullptr;
		}
	}

	//////////////////////////////////////////////////////////////////////////

	AVState::AVState(Video& video) :
		mVideo(&video)
	{
	}


	AVState::~AVState()
	{
		assert(mFrameQueue.empty());
		close();
	}


	void AVState::init(int stream, AVCodec* codec, AVCodecContext* codecContext)
	{
		mSeekStartPacket = std::make_unique<AVPacket>();
		av_init_packet(mSeekStartPacket.get());
		mSeekStartPacket->data = (uint8_t*)mSeekStartPacket.get();
		mSeekStartPacket->size = 0;
		mSeekStartPacket->stream_index = stream;

		mSeekEndPacket = std::make_unique<AVPacket>();
		av_init_packet(mSeekEndPacket.get());
		mSeekEndPacket->data = (uint8_t*)mSeekEndPacket.get();
		mSeekEndPacket->size = 0;
		mSeekEndPacket->stream_index = stream;

		mIOFinishedPacket = std::make_unique<AVPacket>();
		av_init_packet(mIOFinishedPacket.get());
		mIOFinishedPacket->data = (uint8_t*)mIOFinishedPacket.get();
		mIOFinishedPacket->size = 0;
		mIOFinishedPacket->stream_index = stream;

		// The End Of File packet is a special packet with a null data pointer. Unlike the other command packet we do 
		// not explicitly check for mEndOfFilePacket, but avcodec_receive_frame will return EOF if this packet is passed onto it.
		mEndOfFilePacket = std::make_unique<AVPacket>();
		av_init_packet(mEndOfFilePacket.get());
		mEndOfFilePacket->data = nullptr;
		mEndOfFilePacket->size = 0;
		mEndOfFilePacket->stream_index = stream;

		mStream = stream;
		mCodec = codec;
		mCodecContext = codecContext;
	}


	void AVState::close()
	{
		if (mCodec != nullptr)
		{
			mCodec = nullptr;
			avcodec_send_packet(mCodecContext, nullptr);
			avcodec_flush_buffers(mCodecContext);
			avcodec_free_context(&mCodecContext);
			mCodecContext = nullptr;
		}
	}


	bool AVState::waitForFrameQueueEmpty(bool& exitIOThreadSignalled)
	{
		std::unique_lock<std::mutex> lock(mFrameQueueMutex);
		mFrameQueueRoomAvailableCondition.wait(lock, [this, &exitIOThreadSignalled]()
		{
			return mCancelWaitFrameQueueEmpty || exitIOThreadSignalled || mFrameQueue.empty();
		});

		return !mCancelWaitFrameQueueEmpty && !exitIOThreadSignalled;
	}


	void AVState::cancelWaitForFrameQueueEmpty()
	{
		std::unique_lock<std::mutex> lock(mFrameQueueMutex);
		mCancelWaitFrameQueueEmpty = true;
		mFrameQueueRoomAvailableCondition.notify_all();
	}


	void AVState::resetWaitForFrameQueueEmpty()
	{
		mCancelWaitFrameQueueEmpty = false;
	}


	bool AVState::waitForEndOfFileProcessed()
	{
		return mEndOfFileProcessedEvent.wait() == utility::AutoResetEvent::EWaitResult::Completed;
	}


	void AVState::cancelWaitForEndOfFileProcessed()
	{
		mEndOfFileProcessedEvent.cancelWait();
	}


	void AVState::resetEndOfFileProcessed()
	{
		mEndOfFileProcessedEvent.reset();
	}


	void AVState::waitSeekStartPacketProcessed()
	{
		mSeekStartProcessedEvent.wait();
	}


	int AVState::waitForReceiveFrame()
	{
		VIDEO_DEBUG_LOG("waitForPacketProcessed - wait");

		int result;
		mReceiveFrameEvent.wait([this, &result](utility::AutoResetEvent::EWaitResult) { result = mReceiveFrameResult; });

		VIDEO_DEBUG_LOG("waitForPacketProcessed - wait done");

		return result;
	}

	void AVState::drainSeekFrameQueue()
	{
		// While frames are produced we need to keep on clearing the queue, otherwise the decode thread will block waiting
		// for room to be available. When waitForReceiveFrame returns true, it requires another package, so no new 
		// frames will be produced
		while (!waitForReceiveFrame())
			clearFrameQueue(mSeekFrameQueue, false);

		// It may be possible that frames were produced between the last test and the last clear, so do a final clear here
		clearFrameQueue(mSeekFrameQueue, false);

		// After waitForReceiveFrame, the event is reset. We set the event manually here, otherwise the next waitForReceiveFrame
		// will block indefinitely because at this stage, a new packet is required first before waitForReceiveFrame is unblocked.
		mReceiveFrameEvent.set();
	}

	void AVState::clearPacketQueue()
	{
		std::unique_lock<std::mutex> lock(mPacketQueueMutex);
		while (!mPacketQueue.empty())
		{
			// Get packet in front and de-allocate our side
			AVPacket* packet = mPacketQueue.front();
			mPacketQueue.pop();
			mVideo->deallocatePacket(packet->size);

			// Skip end of file packet, we want to re-use that
			if (packet == mEndOfFilePacket.get())
				continue;
			if (packet == mSeekStartPacket.get())
				continue;
			if (packet == mSeekEndPacket.get())
				continue;

			av_packet_free(&packet);
		}
	}


	void AVState::clearFrameQueue()
	{
		clearFrameQueue(mFrameQueue, true);
	}


	void AVState::clearFrameQueue(std::queue<Frame>& frameQueue, bool emitCallback)
	{
		std::unique_lock<std::mutex> lock(mFrameQueueMutex);
		while (!frameQueue.empty())
		{
			Frame frame = frameQueue.front();
			frameQueue.pop();
			av_frame_unref(frame.mFrame);
			av_frame_free(&frame.mFrame);
		}

		mFrameQueueRoomAvailableCondition.notify_all();

		if (emitCallback && mOnClearFrameQueueFunction)
			mOnClearFrameQueueFunction();
	}


	bool AVState::matchesStream(const AVPacket& packet) const
	{
		return packet.stream_index == mStream;
	}


	bool AVState::addSeekStartPacket(const bool& exitIOThreadSignalled)
	{
		return addPacket(*mSeekStartPacket, exitIOThreadSignalled);
	}


	bool AVState::addSeekEndPacket(const bool& exitIOThreadSignalled, double seekTargetSecs)
	{
		mSeekTargetSecs = seekTargetSecs;
		return addPacket(*mSeekEndPacket, exitIOThreadSignalled);
	}


	bool AVState::addEndOfFilePacket(const bool& exitIOThreadSignalled)
	{
		return addPacket(*mEndOfFilePacket, exitIOThreadSignalled);
	}


	bool AVState::addIOFinishedPacket(const bool& exitIOThreadSignalled)
	{
		return addPacket(*mIOFinishedPacket, exitIOThreadSignalled);
	}

	
	bool AVState::addPacket(AVPacket& packet, const bool& exitIOThreadSignalled)
	{
		// Allocate packet our side
		assert(matchesStream(packet));
		if (!mVideo->allocatePacket(packet.size))
			return false;

		// Add to packet queue
		std::unique_lock<std::mutex> lock(mPacketQueueMutex);
		mPacketQueue.emplace(&packet);
		mPacketAvailableCondition.notify_all();
		return true;
	}

	void AVState::notifyStartIOThread()
	{
		mEndOfFileProcessedEvent.reset();
		mSeekStartProcessedEvent.reset();
		mReceiveFrameEvent.reset();
	}

	
	void AVState::notifyExitIOThread()
	{
		mFrameQueueRoomAvailableCondition.notify_all();
		mEndOfFileProcessedEvent.cancelWait();
		mSeekStartProcessedEvent.cancelWait();
		mReceiveFrameEvent.cancelWait();
	}
	

	void AVState::startDecodeThread(const OnClearFrameQueueFunction& onClearFrameQueueFunction)
	{
		exitDecodeThread(true);

		mExitDecodeThreadSignalled = false;
		mFinishedProducingFrames = false;

		mOnClearFrameQueueFunction = onClearFrameQueueFunction;
		mDecodeThread = std::thread(std::bind(&AVState::decodeThread, this));
	}


	void AVState::exitDecodeThread(bool join)
	{
		mExitDecodeThreadSignalled = true; 
		mPacketAvailableCondition.notify_all();
		mFrameQueueRoomAvailableCondition.notify_all();
		mFrameDataAvailableCondition.notify_all();
		if (mDecodeThread.joinable() && join)
			mDecodeThread.join();
	}


	bool AVState::isFinished() const
	{
		if (!mFinishedProducingFrames)
			return false;

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
			stream_start_time = mVideo->mFormatContext->streams[mStream]->start_time * av_q2d(mVideo->mFormatContext->streams[mStream]->time_base);

		AVFrame* frame = av_frame_alloc();

		while (!mExitDecodeThreadSignalled)
		{
			int frameFirstPacketDTS;
			AVState::EDecodeFrameResult decode_result = decodeFrame(*frame, frameFirstPacketDTS);
			if (decode_result == AVState::EDecodeFrameResult::Exit)
				break;

			if (decode_result == AVState::EDecodeFrameResult::EndOfFile)
			{
				VIDEO_DEBUG_LOG("EOF for stream %d", mStream);
				continue;
			}

			// We calculate when the next frame needs to be displayed. The stream contains PTS information, but there are cases
			// when there is no PTS available, we need to account for these cases.
			Frame new_frame;
			new_frame.mPTSSecs = Video::sClockMax;
			new_frame.mFirstPacketDTS = frameFirstPacketDTS;

			// Note: we use the best_effort_timestamp (which is provided/calculated by the decoder), because it seems to deal with some video files/formats
			// where the pts of the frame is not actually correct.
			int64_t pts = frame->best_effort_timestamp;

			// It is still possible that the PTS is not set for best_effort_timestamp. This happened in an EOF scenario where two frames were produced
			// after an EOF was encountered. The first frame contained a good PTS, the second frame did not have a PTS. For such scenarios we are
			// guessing the PTS the best way we can by remembering the PTS of the last frame, and adding a duration.
			if (pts == AV_NOPTS_VALUE)
			{
				new_frame.mPTSSecs = mLastFramePTSSecs + frame_duration;
			}
			else
			{
				// Use the PTS in the timespace of the video stream, starting from the videostream start
				new_frame.mPTSSecs = (pts * av_q2d(mVideo->mFormatContext->streams[mStream]->time_base)) - stream_start_time;
			}

			mLastFramePTSSecs = new_frame.mPTSSecs;

			// We MOVE the decoded frame to this new frame. It is freed when processed
			new_frame.mFrame = av_frame_alloc();
			av_frame_move_ref(new_frame.mFrame, frame);
			av_frame_unref(frame);

			// Push the frame onto the frame queue
			{
				std::unique_lock<std::mutex> lock(mFrameQueueMutex);
				mFrameQueueRoomAvailableCondition.wait(lock, [this]() { return mActiveFrameQueue->size() < 16 || mExitDecodeThreadSignalled; });
				if (mExitDecodeThreadSignalled)
				{
					new_frame.free();
					break;
				}

				mActiveFrameQueue->push(new_frame);
				mFrameDataAvailableCondition.notify_all();
				VIDEO_DEBUG_LOG("push frame (stream %d): pkt_pos: %d, dts: %d, pts: %d", mStream, new_frame.mFrame->pkt_pos, new_frame.mFrame->pkt_dts, new_frame.mFrame->pkt_pts);
			}
		}

		av_frame_free(&frame);
	}


	AVPacket* AVState::popPacket()
	{
		while (true)
		{
			// Pop a packet from the queue
			AVPacket* packet;
			{
				std::unique_lock<std::mutex> lock(mPacketQueueMutex);
				mPacketAvailableCondition.wait(lock, [this]()
				{
					return !mPacketQueue.empty() || mExitDecodeThreadSignalled;
				});

				if (mExitDecodeThreadSignalled)
					return nullptr;

				packet = mPacketQueue.front();
				mPacketQueue.pop();

				mVideo->deallocatePacket(packet->size);

				VIDEO_DEBUG_LOG("pop packet: pkt_pos: %d, dts: %d, pts: %d, %s", packet->pos, packet->dts, packet->pts, getStream() == 0 ? "video" : "audio");
			}

			// If we dequeued a seek packet, flush all buffers for the video codec and clear the frame queue
			if (packet == mSeekStartPacket.get())
			{
				VIDEO_DEBUG_LOG("seek start received");

				avcodec_flush_buffers(mCodecContext);

				clearFrameQueue(mFrameQueue, true);
				clearFrameQueue(mSeekFrameQueue, false);

				{
					// We started seeking, we flip the target queue to a temporary 'seek' frame queue
					// so that the I/O thread can consume temporary seek frames
					std::unique_lock<std::mutex> frame_lock(mFrameQueueMutex);
					mActiveFrameQueue = &mSeekFrameQueue;
				}

				mSeekStartProcessedEvent.set();
			}
			else if (packet == mSeekEndPacket.get())
			{
				VIDEO_DEBUG_LOG("seek end received");

				clearFrameQueue(mSeekFrameQueue, false);

				// Seeking is done, switch back to the regular frame queue so that the user can continue
				// processing frames
				std::unique_lock<std::mutex> lock(mFrameQueueMutex);
				mActiveFrameQueue = &mFrameQueue;

				// We are guessing the best 'last pts'. The seek target PTS is smaller or equal to the seeked PTS, (unless we 
				// could not seek any further in the stream), so it serves as a good last pts after seeking is completed.
				mLastFramePTSSecs = mSeekTargetSecs;
			}
			else if (packet == mIOFinishedPacket.get())
			{
				// I/O thread has no more packets to produce. It is the last packet in the queue, so 
				// we are also done producing frames.
				mFinishedProducingFrames = true;
				return nullptr;
			}
			else
			{
				return packet;
			}
		}

		return nullptr;
	}


	AVState::EDecodeFrameResult AVState::decodeFrame(AVFrame& frame, int& frameFirstPacketDTS)
	{
		// Here we pull packets from the queue and decode them until all data for this frame is processed
		while (true)
		{
			int result = AVERROR(EAGAIN);
			do 
			{
				VIDEO_DEBUG_LOG("receive_frame: stream %s", getStream() == 0 ? "video" : "audio");

				// NOTE: it is important to understand that a single send_packet can cause multiple frames to be 'received'. So we
				// continue calling avcoded_receive_frame until it returns AVERROR(EAGAIN), meaning it needs a new packet.
				// Also, we may need to send multiple packets to the decoder for a single frame.
				result = avcodec_receive_frame(mCodecContext, &frame);

				VIDEO_DEBUG_LOG("receive_frame - notify %s", getStream() == 0 ? "video" : "audio");
				mReceiveFrameEvent.set([this, result]() { mReceiveFrameResult = result; });
				if (result >= 0)
				{
					// We have a frame, copy the first DTS that was used to produce this packet (and reset it for the next frame)
					frameFirstPacketDTS = mFrameFirstPacketDTS;
					mFrameFirstPacketDTS = -INT_MAX;
					VIDEO_DEBUG_LOG("receive_frame: pkt_pos: %d, dts: %d, pts: %d", frame.pkt_pos, frame.pkt_dts, frame.pkt_pts);					
					return EDecodeFrameResult::GotFrame;
				}

				// An EOF is returned from avcoded_receive_frame if an EOF packet (a specific packet with a nullptr for data) is sent
				// from the I/O thread. 
				if (result == AVERROR_EOF)
				{
					avcodec_flush_buffers(mCodecContext);
					mEndOfFileProcessedEvent.set();
					return EDecodeFrameResult::EndOfFile;
				}

				// Note: we keep spinning as long as EAGAIN is not returned, even in the case of errors. 
				// We're not 100% sure why this is (this logic came from ffplay), but perhaps there are certain 
				// kinds of decoding errors that are non-fatal and will 'resolve' themselves?
			} while (result != AVERROR(EAGAIN));

			AVPacket* packet = popPacket();
			if (packet == nullptr)
				return EDecodeFrameResult::Exit;

			VIDEO_DEBUG_LOG("avcodec_send_packet: pkt_pos: %d, dts: %d, pts: %d", packet->pos, packet->dts, packet->pts);

			// If this is the first packet for this frame, set this packets' DTS as the first DTS
			if (mFrameFirstPacketDTS == -INT_MAX)
				mFrameFirstPacketDTS = packet->dts;

			// Send packet to decoder, this is used by avcoded_receive frame later
			result = avcodec_send_packet(mCodecContext, packet);
			assert(result != AVERROR(EAGAIN));

			// If the packet is the EOF packet, we shouldn't delte it, it will be destroyed when the AVState is destroyed.
			if (packet != mEndOfFilePacket.get())
				av_packet_free(&packet);
		}

		assert(false);
		return EDecodeFrameResult::GotFrame;
	}


	Frame AVState::popFrame()
	{
		std::unique_lock<std::mutex> lock(mFrameQueueMutex);
		mFrameDataAvailableCondition.wait(lock, [this]()
		{
			return mExitDecodeThreadSignalled || !mFrameQueue.empty();
		});

		if (mExitDecodeThreadSignalled)
			return Frame();

		Frame frame = mFrameQueue.front();
		mFrameQueue.pop();

		mFrameQueueRoomAvailableCondition.notify_all();

		return frame;
	}
	

	Frame AVState::tryPopFrame(double pts)
	{
		std::unique_lock<std::mutex> lock(mFrameQueueMutex);

		Frame cur_frame;
		while (!mFrameQueue.empty() && pts >= mFrameQueue.front().mPTSSecs)
		{
			if (cur_frame.isValid())
			{
				//nap::Logger::info("Dropping frame at time %f (clock is at %f)", mFrameQueue.front().mPTSSecs, pts);
				cur_frame.free();
			}

			cur_frame = mFrameQueue.front();
			mFrameQueue.pop();
		}

		if (cur_frame.isValid())
			mFrameQueueRoomAvailableCondition.notify_all();

		return cur_frame;
	}


	Frame AVState::popSeekFrame()
	{
		std::unique_lock<std::mutex> lock(mFrameQueueMutex);

		VIDEO_DEBUG_LOG("wait for seek frame (stream %d)", mStream);
		mFrameDataAvailableCondition.wait(lock, [this]()
		{
			bool result = mExitDecodeThreadSignalled || !mSeekFrameQueue.empty();
			VIDEO_DEBUG_LOG("check seek frame condition: %s", result ? "true" : "false");
			return result;
		});

		VIDEO_DEBUG_LOG("finished wait for seek frame");

		if (mExitDecodeThreadSignalled)
			return Frame();

		Frame frame = mSeekFrameQueue.front();
		mSeekFrameQueue.pop();

		mFrameQueueRoomAvailableCondition.notify_all();

		return frame;
	}


	Frame AVState::peekFrame()
	{
		std::unique_lock<std::mutex> lock(mFrameQueueMutex);
		if (mFrameQueue.empty())
			return Frame();

		return mFrameQueue.front();
	}


	//////////////////////////////////////////////////////////////////////////

	static int sMaxPacketQueueSizeInBytes = 16 * 1024 * 1024;

	Video::Video(VideoService& service) : 
		mService(service),
		mAudioState(*this),
		mVideoState(*this)
	{
	}


	Video::~Video()
	{
        mDestructedSignal(*this);
        
		if (mFormatContext != nullptr)
			mService.removeVideoPlayer(*this);
		
		stop(true);
		
		mAudioState.close();
		mVideoState.close();

		avformat_close_input(&mFormatContext);
		avformat_free_context(mFormatContext);
	}


	bool Video::sInitAVState(AVState& destState, const AVStream& stream, AVDictionary*& options, utility::ErrorState& errorState)
	{
		// Find the decoder for the video stream
		AVCodec* codec = avcodec_find_decoder(stream.codecpar->codec_id);
		if (!errorState.check(codec != nullptr, "Unable to find codec for video stream"))
			return false;

		// Allocate a codec context for the decoder
		AVCodecContext* codec_context = avcodec_alloc_context3(codec);
		stream.codecpar->codec_id;
		if (!errorState.check(avcodec_parameters_to_context(codec_context, stream.codecpar) >= 0,
			"Failed to copy codec parameters to decoder context"))
			return false;

		// Initialize the decoders
		int error = avcodec_open2(codec_context, codec, &options);
		if (!errorState.check(error == 0, "Unable to open codec: %s", sErrorToString(error).c_str()))
			return false;

		destState.init(stream.index, codec, codec_context);
		return true;
	}


	bool Video::init(nap::utility::ErrorState& errorState)
	{
		mFormatContext = avformat_alloc_context();
		if (!errorState.check(mFormatContext != nullptr, "Error allocating context"))
			return false;

		int error = avformat_open_input(&mFormatContext, mPath.c_str(), nullptr, nullptr);
		if (!errorState.check(error >= 0, "Error opening file '%s': %s\n", mPath.c_str(), sErrorToString(error).c_str()))
			return false;

		error = avformat_find_stream_info(mFormatContext, nullptr);
		if (!errorState.check(error >= 0, "Error finding stream: %s", sErrorToString(error).c_str()))
			return false;

		// Enable this to dump information about file onto standard error
		//av_dump_format(mFormatContext, 0, mPath.c_str(), 0);

		// Find the index and codec context of the video and audio stream.
		AVStream* video_stream = nullptr;
		AVStream* audio_stream = nullptr;
		for (int i = 0; i < mFormatContext->nb_streams; ++i)
		{
			AVStream* cur_stream = mFormatContext->streams[i];
			switch (cur_stream->codec->codec_type)
			{
			case AVMEDIA_TYPE_VIDEO:
				video_stream = cur_stream;
				break;
			case AVMEDIA_TYPE_AUDIO:
				audio_stream = cur_stream;
				break;
			}
		}

		// Ensure there's a video stream
		if (!errorState.check(video_stream != nullptr, "No video stream found"))
			return false;

		// This option causes the codec context to spawn threads internally for decoding, speeding up the decoding process
		AVDictionary* options = nullptr;
		av_dict_set(&options, "threads", "auto", 0);

		// We need to set this option to make sure that the decoder transfers ownership from decode buffers to us
		// when we decode frames. Otherwise, the decoder will reuse buffers, which will then overwrite data already in our queue.
		av_dict_set(&options, "refcounted_frames", "1", 0);

		// Initialize video stream
		if (!sInitAVState(mVideoState, *video_stream, options, errorState))
			return false;

		// Initialize audio stream if available
		if (audio_stream != nullptr && !sInitAVState(mAudioState, *audio_stream, options, errorState))
				return false;

		AVCodecContext& video_codec_context = mVideoState.getCodecContext();
		mWidth = video_codec_context.width;
		mHeight = video_codec_context.height;
		mDuration = static_cast<double>((double)mFormatContext->duration / AV_TIME_BASE);

		float yWidth	= video_codec_context.width;
		float yHeight	= video_codec_context.height;
		float uvWidth	= video_codec_context.width * 0.5f;
		float uvHeight	= video_codec_context.height * 0.5f;

		// Disable mip-mapping for video
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

		mUTexture = std::make_unique<RenderTexture2D>();
		mUTexture->mWidth = uvWidth;
		mUTexture->mHeight = uvHeight;
		mUTexture->mFormat = RenderTexture2D::EFormat::R8;
		mUTexture->mParameters = parameters;
		if (!mUTexture->init(errorState))
			return false;

		mVTexture = std::make_unique<RenderTexture2D>();
		mVTexture->mWidth = uvWidth;
		mVTexture->mHeight = uvHeight;
		mVTexture->mFormat = RenderTexture2D::EFormat::R8;
		mVTexture->mParameters = parameters;
		if (!mVTexture->init(errorState))
			return false;

		clearTextures();

		// Register with service
		mService.registerVideoPlayer(*this);

		return true;
	}


	void Video::clearTextures()
	{
		AVCodecContext& video_codec_context = mVideoState.getCodecContext();

		float yWidth = video_codec_context.width;
		float yHeight = video_codec_context.height;
		float uvWidth = video_codec_context.width * 0.5f;
		float uvHeight = video_codec_context.height * 0.5f;

		// YUV420p to RGB conversion uses an 'offset' value of (-0.0625, -0.5, -0.5) in the shader. 
		// This means that initializing the YUV planes to zero does not actually result in black output.
		// To fix this, we initialize the YUV planes to the negative of the offset
		std::vector<uint8_t> y_default_data(yWidth * yHeight, 16);

		// Initialize UV planes
		std::vector<uint8_t> uv_default_data(uvWidth * uvHeight, 127);

		mYTexture->update(y_default_data.data());
		mUTexture->update(uv_default_data.data());
		mVTexture->update(uv_default_data.data());
	}


	void Video::play(double startTimeSecs)
	{
		// Force a stop if video is currently playing
		stop(true);

		// Reset all state to make sure threads don't immediately exit from a previous 'stop' call
		mPlaying = true;
		mSystemClockSecs = sClockMax;
		mAudioClockSecs = sClockMax;
		mAudioDecodeClockSecs = sClockMax;
		clearTextures();

		seek(startTimeSecs);

		// It is important that the IOThread is started before the decode thread. The reason is that in startIOThread, we are
		// initializing synchronization primitives that are used in both IO thread and decode thread 
		startIOThread();

		mVideoState.startDecodeThread(std::bind(&Video::onClearVideoFrameQueue, this));
		
		if (isAudioEnabled())
			mAudioState.startDecodeThread(std::bind(&Video::onClearAudioFrameQueue, this));
	}


	void Video::stop(bool blocking)
	{
		if (!mPlaying)
			return;

		// It is important that the IO thread is stopped before the decode threads. The reason is that the IO thread may be in a seeking
		// state when trying to exit it. If the decode threads exit first, the IO thread needs to be able to deal with null pointers, etc.
		// By exiting the IO thread first, we don't have to have any special case handling in the seeking code for the decode thread exiting first.
		exitIOThread(blocking);

		mVideoState.exitDecodeThread(blocking);

		if (isAudioEnabled())
			mAudioState.exitDecodeThread(blocking);		

		mPlaying = false;

		clearPacketQueue();
		clearFrameQueue();

		mCurrentAudioFrame.free();
	}


	void Video::seek(double seconds)
	{
		mSeekTarget = -1;
		mSeekKeyframeTarget = -1;
		mSeekTargetSecs = seconds;
		setIOThreadState(IOThreadState::SeekRequest);
	}

	
	double Video::getCurrentTime() const
	{
		if (mIOThreadState != IOThreadState::Playing && mIOThreadState != IOThreadState::WaitingForEOF)
			return mSeekTargetSecs;
		else
			return isAudioEnabled() ? mAudioClockSecs : mSystemClockSecs;
	}


	void Video::clearPacketQueue()
	{
		mVideoState.clearPacketQueue();
		mAudioState.clearPacketQueue();
	}


	void Video::clearFrameQueue()
	{
		mVideoState.clearFrameQueue();
		mAudioState.clearFrameQueue();
	}


	void Video::onClearVideoFrameQueue()
	{
		// After clearing the frame queue, we also need to reset the video clock in order for playback to re-sync to possible new frames
		mSystemClockSecs = sClockMax;
	}


	void Video::onClearAudioFrameQueue()
	{
		// After clearing the audio frame queue, we need to reset the audio clocks, to ensure that the video playback does not block while waiting for the audio clock to advance
		mAudioDecodeClockSecs = sClockMax;
		mAudioClockSecs = sClockMax;
	}
	

	void Video::setErrorOccurred(const std::string& errorMessage)
	{
		mErrorMessage = errorMessage;
		stop(false);
	}


	Video::EProducePacketResult Video::ProducePacket(AVState* targetState)
	{
		// Read packets from the stream and push them onto the packet queue
		PacketWrapper packet;
		int result = av_read_frame(mFormatContext, packet.mPacket);

		VIDEO_DEBUG_LOG("read packet: pkt_pos: %d, dts: %d, pts: %d", packet.mPacket->pos, packet.mPacket->dts, packet.mPacket->pts);

		if (result == AVERROR_EOF)
		{
			// When end of file is reached, we put a special packet onto the queue that is actually consumed as a regular
			// packet by the decode thread. avcoded_receive_frame will decode it and return EOF when receiving. This lets
			// the decode thread know it reached the end of the stream. By doing it this way, we are sure that all frames
			// in the queue are properly processed. The decode thread will flush the codec at that point in time.
			mVideoState.addEndOfFilePacket(mExitIOThreadSignalled);
			if (isAudioEnabled())
				mAudioState.addEndOfFilePacket(mExitIOThreadSignalled);

			return EProducePacketResult::EndOfFile;
		}
		else if (result < 0)
		{
			setErrorOccurred(sErrorToString(result));
			return EProducePacketResult::Error;
		}

		EProducePacketResult packet_result = EProducePacketResult::GotUnknownPacket;
		if (mVideoState.matchesStream(*packet.mPacket))
		{
			packet_result = EProducePacketResult::GotVideoPacket;
			if (targetState == nullptr || targetState == &mVideoState)
			{
				if (mVideoState.addPacket(*packet.mPacket, mExitIOThreadSignalled))
					packet.mPacket = nullptr;
			}
		}
		else if (mAudioState.matchesStream(*packet.mPacket))
		{
			// Note that audio packets are always consumed to make sure that the stream progresses as it should, 
			// but they are only added when the client want to push the packets on the queue
			packet_result = EProducePacketResult::GotAudioPacket;
			if (targetState == nullptr || targetState == &mAudioState)
			{
				if (mAudioState.addPacket(*packet.mPacket, mExitIOThreadSignalled))
					packet.mPacket = nullptr;
			}
		}

		return packet_result;
	}


	void Video::finishSeeking(AVState& seekState, bool shouldDrainFrameQueue)
	{
		// It may be possible that after submitting a single packet, multiple frames are output on the decode thread. This can cause the decode
		// thread to block waiting for room to be available in the seek frame queue. Therefore we need to drain any additional frames produced.
		// Here we wait until the decode thread needs a new packet, this guarantees that no additional frames will follow.
		if (shouldDrainFrameQueue)
			seekState.drainSeekFrameQueue();

		// PTS could be exactly equal to target PTS, we may have reached EOF. We're done finding out target frame.
		setIOThreadState(IOThreadState::Playing);

		// Inform the decode thread that it should switch back to the regular frame queue
		mVideoState.addSeekEndPacket(mExitIOThreadSignalled, mSeekTargetSecs);
		if (isAudioEnabled())
			mAudioState.addSeekEndPacket(mExitIOThreadSignalled, mSeekTargetSecs);

		// Reset audio clock to something that won't drop all frames after we've finished seeking.
		// We'll rely on the audio callback to set this back to proper values once frames start being decoded
		mAudioClockSecs = -1;
		mAudioDecodeClockSecs = -1;

		mSeekState = nullptr;
	}


	void Video::setSeekTarget(AVState& seekState, double seekTargetSecs)
	{
		AVStream* stream = mFormatContext->streams[seekState.getStream()];

		// All timing information is relative to the stream start. If the stream start has no PTS value, we assume zero to be the start
		double stream_start_time = 0.0;
		if (stream->start_time != AV_NOPTS_VALUE)
			stream_start_time = stream->start_time * av_q2d(stream->time_base);

		mSeekTarget = std::round((seekTargetSecs - stream_start_time) / av_q2d(stream->time_base));
		mSeekKeyframeTarget = mSeekTarget;
	}


	bool Video::allocatePacket(uint64_t inPacketSize)
	{
		std::unique_lock<std::mutex> lock(mTotalPacketQueueSizeLock);
		mPacketQueueRoomAvailableCondition.wait(lock, [this, inPacketSize]() { return (mTotalPacketQueueSize + inPacketSize) < sMaxPacketQueueSizeInBytes || mExitIOThreadSignalled; });
		if (mExitIOThreadSignalled)
			return false;

		mTotalPacketQueueSize += inPacketSize;
		return true;
	}


	void Video::deallocatePacket(uint64_t inPacketSize)
	{
		std::unique_lock<std::mutex> lock(mTotalPacketQueueSizeLock);
		mTotalPacketQueueSize -= inPacketSize;

		mPacketQueueRoomAvailableCondition.notify_all();
	}


	void Video::setIOThreadState(IOThreadState threadState)
	{
		mIOThreadState = threadState;

		if (mIOThreadState == IOThreadState::WaitingForEOF)
		{
			// Once we enter waiting for EOF state, we need to reset the events used to wait for the frame queue to be empty,
			// to ensure that subsequent waits won't immediately fall through
			mVideoState.resetWaitForFrameQueueEmpty();
		}
		else if (mIOThreadState == IOThreadState::Playing)
		{
			// Once we enter playing state, we need to reset the events used to wait for EOF,
			// to ensure that subsequent waits won't immediately fall through
			mVideoState.resetEndOfFileProcessed();
			if (isAudioEnabled())
				mAudioState.resetEndOfFileProcessed();

			if (isAudioEnabled())
				mAudioState.resetWaitForFrameQueueEmpty();
		}
		else if (mIOThreadState == IOThreadState::SeekRequest)
		{
			// Once we enter the seek request state, we need to cancel any outstanding waits for EOF/frame queue,
			// to ensure that we don't wait for the consuming thread to have consumed all frames before we start the seek operation
			mVideoState.cancelWaitForEndOfFileProcessed();
			if (isAudioEnabled())
				mAudioState.cancelWaitForEndOfFileProcessed();

			mVideoState.cancelWaitForFrameQueueEmpty();
			if (isAudioEnabled())
				mAudioState.cancelWaitForFrameQueueEmpty();
		}
	}

	void Video::ioThread()
	{
		while (!mExitIOThreadSignalled)
		{
			switch (mIOThreadState)
			{
			case IOThreadState::Playing:
				{
					// read a frame from the stream and push a packet in the packet queue
					EProducePacketResult produce_packet_result = ProducePacket(nullptr);
					if (produce_packet_result == EProducePacketResult::EndOfFile)
					{
						if (mLoop)
						{
 							// We need to wait for all frames to be fully consumed. We don't want to start seeking before that, because otherwise
							// we'd clear the frame queue (in the seek start event), before the consumer has seen them. This looks like frames being dropped 
							// to the user.
							// We do this by first waiting for the EOF packet to be consumed by the decode thread. This way we know for sure that no more
							// frames will be produced. After waiting for the EOF event, we wait for all frames to be consumed. 
							// We first wait for the audio frame queue to be fully consumed and then transition to WaitingForEOF, where we wait for the video
							// to be consumed as well. The reason that this is separated into the WaitingForEOF state is to make sure that the audio thread
							// does not overwrite the audio clock as we reset it.
							if (mVideoState.waitForEndOfFileProcessed() && (!isAudioEnabled() || mAudioState.waitForEndOfFileProcessed()))
							{
								bool audioWaitCompleted = true;
								if (isAudioEnabled())
									audioWaitCompleted = mAudioState.waitForFrameQueueEmpty(mExitIOThreadSignalled);

								if (audioWaitCompleted)
									setIOThreadState(IOThreadState::WaitingForEOF);
							}

							continue;							
						}
						else
						{
							// We have reached end of the packet stream and we're not looping. Inform the decode 
							// threads that it does not need to expect any more packets.
							mVideoState.addIOFinishedPacket(mExitIOThreadSignalled);
							if (isAudioEnabled())
								mAudioState.addIOFinishedPacket(mExitIOThreadSignalled);

							return;
						}
					}
					else if (produce_packet_result == EProducePacketResult::Error)
					{
						return;
					}
					break;
				}

			case IOThreadState::WaitingForEOF:
				{
					// Reset the audio clock to max to ensure all remaining frames are consumed. This is necessary to deal with video files
					// where the audio stream is shorter than the video stream; the video frames after the end of the audio stream would otherwise never be consumed.
					mAudioDecodeClockSecs = sClockMax;
					mAudioClockSecs = sClockMax;

					if (mVideoState.waitForFrameQueueEmpty(mExitIOThreadSignalled))
						seek(0.0);

					break;
				}

			case IOThreadState::SeekRequest:
				{
					// Seeking stage 1: we've received a seek request. Make sure that we clear out any existing state (queues, codecs) and move the cursor in the stream

					// After seeking we should clear any queues. We will clear the packet queue here, the frame queue can only be cleared by the decode thread.
					clearPacketQueue();
					VIDEO_DEBUG_LOG("ioThread seek start, clear packet queue");

					// Add seek start packet to the queue to signal the decoder threads that they should switch their output to the seek queue, which will be consumed by this thread.
					// Note that it's important that we wait for the audio state first, because the video thread may rely on the audio clock being updated in order to progress.
					// For example, when the video frame queue is full, waiting for the seek start to be processed on the video state will block indefinitely,
					// because the audio clock won't progress, due to no packets being added to the audio state, because we're waiting for video here.
					if (isAudioEnabled())
						mAudioState.addSeekStartPacket(mExitIOThreadSignalled);
					
					mVideoState.addSeekStartPacket(mExitIOThreadSignalled);

					// Wait until the seek start is processed so that we are sure that there is not more state pending in the decode thread
					if (isAudioEnabled())
						mAudioState.waitSeekStartPacketProcessed();
					
					mVideoState.waitSeekStartPacketProcessed();					

					VIDEO_DEBUG_LOG("ioThread seek to %d", mSeekKeyframeTarget);

					// Use audio state to seek if there is an audio stream, otherwise seek by video (see documentation at top of file for information)
					mSeekState = isAudioEnabled() ? &mAudioState : &mVideoState;
					
					// We can only determine the seek target in stream units once we know the AVState we'll use to seek
					bool initialSeek = mSeekTarget == -1;
					if (initialSeek)
						setSeekTarget(*mSeekState, mSeekTargetSecs);

					// We can only seek by DTS, but we start out with a target PTS (in stream units). The DTS will be smaller or equal to the PTS. We search
					// backwards in the stream to the current keyframe target. Later we will test if that keyframes' PTS is actually smaller or equal to the
					// PTS that we are looking for.					
					int seek_result = av_seek_frame(mFormatContext, mSeekState->getStream(), mSeekKeyframeTarget, AVSEEK_FLAG_BACKWARD);

					// Seeking can fail for audio streams in some cases. One situation we've encountered was that the audio frames were not marked
					// as keyframes, so seeking will fail (as we're not using AVSEEK_FLAG_ANY). In these cases we try to fallback on seeking the video 
					// stream.
					if (seek_result < 0 && mSeekState == &mAudioState)
					{
						mSeekState = &mVideoState;

						// Reset the seek target to use the VideoState
						if (initialSeek)
							setSeekTarget(*mSeekState, mSeekTargetSecs);

						seek_result = av_seek_frame(mFormatContext, mSeekState->getStream(), mSeekKeyframeTarget, AVSEEK_FLAG_BACKWARD);
					}

					if (seek_result < 0)
					{
						Logger::error("Failed to seek to %f: %s", mSeekTargetSecs, sErrorToString(seek_result).c_str());
						setErrorOccurred(sErrorToString(seek_result));
						return;
					}

					setIOThreadState(IOThreadState::SeekingStartFrame);
					break;
				} 

			case IOThreadState::SeekingStartFrame:
				{
					EProducePacketResult seekPacketType = mSeekState == &mAudioState ? EProducePacketResult::GotAudioPacket : EProducePacketResult::GotVideoPacket;

					// Seeking stage 2: Push packets onto the queue and wait for a single frame to be received. If the PTS of that frame is greater than the PTS
					// we are looking for, we go back to stage 1 with a new seek (internal) target. Otherwise we continue the seeking process.

					// Receive single frame in lock-step
					EProducePacketResult produce_packet_result = EProducePacketResult::GotPacket;
					int receive_frame_result = 0;
					while ((receive_frame_result = mSeekState->waitForReceiveFrame()) == AVERROR(EAGAIN) && produce_packet_result != EProducePacketResult::EndOfFile && !mExitIOThreadSignalled)
					{
						assert(((uint8_t)produce_packet_result & (uint8_t)EProducePacketResult::GotPacket) != 0);
						do
						{
							produce_packet_result = ProducePacket(mSeekState);
						} while (produce_packet_result != seekPacketType && produce_packet_result != EProducePacketResult::EndOfFile);

						if (produce_packet_result == EProducePacketResult::Error)
							return;
					}

					// If we broke out of the receive frame loop because exit was signaled, just break out here (we can't make any assumptions about the state of the video frame queue)
					if (mExitIOThreadSignalled)
						break;

					// receive_frame_result is the result from the decode thread. When eof is reached, it means that all packets up until eof are processed by the decode thread. 
					// If receive_frame_result is AVERROR(EAGAIN) here, it means we're out of packets to produce while the decode thread still wants more of them. We stop seeking in
					// those cases as well.
					if (receive_frame_result == AVERROR_EOF || receive_frame_result == AVERROR(EAGAIN))
					{
						// When draining the frame queue we will wait for the next receive_frame. If an EOF is encountered, the decode thread will immediately go into receive_frame,
						// so it is safe to drain the frame queue in that case. In case of AVERROR(EGAIN), the decode thread will already have processed all packets and wait for the next
						// packet. As there is no more packet that we will push onto the queue, we would hang if we would drain the frame queue, so we skip draining the queue in that case.
						finishSeeking(*mSeekState, receive_frame_result != AVERROR(EAGAIN));
						break;
					}

					Frame seek_frame = mSeekState->popSeekFrame();
					VIDEO_DEBUG_LOG("seek start frame, pop frame: pkt_pos: %d, dts: %d, pts: %d", seek_frame.mFrame->pkt_pos, seek_frame.mFrame->pkt_dts, seek_frame.mFrame->pkt_pts);

					// Test if PTS > frame PTS and start new seek request if so. If this frame does not have a PTS, we don't know what the timestamp is, so we issue a new seek request. 
					// If we're at the beginning of the stream we never seek back any further
					if ((seek_frame.mFrame->best_effort_timestamp == AV_NOPTS_VALUE || seek_frame.mFrame->best_effort_timestamp > mSeekTarget) && seek_frame.mFirstPacketDTS != mFormatContext->streams[mSeekState->getStream()]->first_dts)
					{				
						// The new seek target is computed by targeting any packet *before* the smallest DTS that was used to construct this frame
						mSeekKeyframeTarget = seek_frame.mFirstPacketDTS - 1;

						setIOThreadState(IOThreadState::SeekRequest);
					}
					else if (seek_frame.mFrame->best_effort_timestamp < mSeekTarget && produce_packet_result != EProducePacketResult::EndOfFile)
					{
						// In case the target PTS > frame PTS and we haven't reached EOF, start stage 3 where we progress through frames until we reach target PTS
						setIOThreadState(IOThreadState::SeekingTargetFrame);
					}
					else
					{
						// If the frame we've found is an exact match with the PTS we're looking for, or we can't search further to the left (because the frame is at the start of the stream),
						// we finish the seek operation and resume normal play
						finishSeeking(*mSeekState, true);
					}
					seek_frame.free();
				}
				break;

			case IOThreadState::SeekingTargetFrame:
				{
					EProducePacketResult seekPacketType = mSeekState == &mAudioState ? EProducePacketResult::GotAudioPacket : EProducePacketResult::GotVideoPacket;

					// Seeking stage 3: The current frame PTS lies before the target PTS. Now we continue decoding frames until we decode
					// a frame that is greater or equal to the target PTS.
					while (true)
					{
						// Receive single frame in lock-step
						EProducePacketResult produce_packet_result = EProducePacketResult::GotPacket;
						int receive_frame_result = 0;
						while ((receive_frame_result = mSeekState->waitForReceiveFrame()) == AVERROR(EAGAIN) && produce_packet_result != EProducePacketResult::EndOfFile && !mExitIOThreadSignalled)
						{
							assert(((uint8_t)produce_packet_result & (uint8_t)EProducePacketResult::GotPacket) != 0);
							do
							{
								produce_packet_result = ProducePacket(mSeekState);
							} while (produce_packet_result != seekPacketType && produce_packet_result != EProducePacketResult::EndOfFile);

							if (produce_packet_result == EProducePacketResult::Error)
								return;
						}

						// If we broke out of the receive frame loop because exit was signaled, just break out here (we can't make any assumptions about the state of the video frame queue)
						if (mExitIOThreadSignalled)
							break;

						if (receive_frame_result == AVERROR_EOF || receive_frame_result == AVERROR(EAGAIN))
						{
							finishSeeking(*mSeekState, receive_frame_result != AVERROR(EAGAIN));
							break;
						}

						Frame seek_frame = mSeekState->popSeekFrame();
						if (seek_frame.mFrame->best_effort_timestamp >= mSeekTarget)
						{
							// If we've found a matching frame, finish the seek operation and resume normal play
							finishSeeking(*mSeekState, true);
							seek_frame.free();
							break;
						}
						seek_frame.free();
					}
					break;
				}
			}
		}
	}


	void Video::startIOThread()
	{
		exitIOThread(true);

		mExitIOThreadSignalled = false;
		mVideoState.notifyStartIOThread();
		mAudioState.notifyStartIOThread();

		mIOThread = std::thread(std::bind(&Video::ioThread, this));
	}


	void Video::exitIOThread(bool join)
	{
		mExitIOThreadSignalled = true;
		
		// Unlock I/O thread, it might be waiting for room to become available in the packet queue through AVState::addPacket
		mPacketQueueRoomAvailableCondition.notify_all();
		mVideoState.notifyExitIOThread();
		mAudioState.notifyExitIOThread();

		if (mIOThread.joinable() && join)
			mIOThread.join();
	}


	bool Video::getNextAudioFrame(const AudioFormat& targetAudioFormat)
	{
		// If there was a previous frame allocated, we have consumed it completely across audio callback and we 
		// can now destroy it as we're about to decode a new frame.
		if (mCurrentAudioFrame.mFrame != nullptr)
		{
			av_frame_unref(mCurrentAudioFrame.mFrame);
			av_frame_free(&mCurrentAudioFrame.mFrame);
		}

		mCurrentAudioFrame = mAudioState.popFrame();
		if (!mCurrentAudioFrame.isValid())
			return false;

		int buffer_size = av_samples_get_buffer_size(NULL, mCurrentAudioFrame.mFrame->channels, mCurrentAudioFrame.mFrame->nb_samples, (AVSampleFormat)mCurrentAudioFrame.mFrame->format, 1);

		AVFrame* frame = mCurrentAudioFrame.mFrame;
		int64_t dec_channel_layout = (frame->channel_layout && frame->channels == av_get_channel_layout_nb_channels(frame->channel_layout)) ?
			frame->channel_layout : av_get_default_channel_layout(frame->channels);

		// If the format of the audio frame does not match the target audio format, we need to allocate a resample context (once)
		if ((frame->format != (AVSampleFormat)targetAudioFormat.getSampleFormat() ||
			dec_channel_layout != targetAudioFormat.getChannelLayout() ||
			frame->sample_rate != targetAudioFormat.getSampleRate()) &&
			mAudioResampleContext == nullptr) 
		{
			mAudioResampleContext = swr_alloc_set_opts(NULL, targetAudioFormat.getChannelLayout(), (AVSampleFormat)targetAudioFormat.getSampleFormat(), targetAudioFormat.getSampleRate(), dec_channel_layout, (AVSampleFormat)frame->format, frame->sample_rate, 0, NULL);
			int result = swr_init(mAudioResampleContext);
			assert(result >= 0);
		}

		// Resample if the source and target formats do not match
		if (mAudioResampleContext != nullptr)
		{
			int target_num_channels = av_get_channel_layout_nb_channels(targetAudioFormat.getChannelLayout());

			const uint8_t **in = (const uint8_t **)frame->extended_data;
			int out_count = (int64_t)frame->nb_samples * targetAudioFormat.getSampleRate() / frame->sample_rate + 256;
			int out_size = av_samples_get_buffer_size(NULL, target_num_channels, out_count, (AVSampleFormat)targetAudioFormat.getSampleFormat(), 0);
			assert(out_size >= 0);
			int len2;

			mAudioResampleBuffer.resize(out_size);
			uint8_t* resample_buffer = mAudioResampleBuffer.data();
			uint8_t** out = &resample_buffer;
			len2 = swr_convert(mAudioResampleContext, out, out_count, in, frame->nb_samples);
			assert(len2 >= 0);
			assert(len2 != out_count);

			// Let 'current' audio buffer point to the resampled data
			mCurrentAudioBuffer = mAudioResampleBuffer.data();
			buffer_size = len2 * target_num_channels * av_get_bytes_per_sample((AVSampleFormat)targetAudioFormat.getSampleFormat());
		}
		else 
		{
			// We don't need to resample, let 'current' audio buffer point to the data in the frame directly
			mCurrentAudioBuffer = frame->data[0];
		}

		// We have a new frame, reset cursor and size
		mAudioFrameSize = buffer_size;
		mAudioFrameReadOffset = 0;
		
		// After decoding, we know the PTS of the decoded frame and we know its duration. So, we advance the audio decode clock to the end of the audio frame (in time)
		mAudioDecodeClockSecs = mCurrentAudioFrame.mPTSSecs + ((double)mCurrentAudioFrame.mFrame->nb_samples / mCurrentAudioFrame.mFrame->sample_rate);

		return true;
	}
	

	bool Video::OnAudioCallback(uint8_t* dataBuffer, int sizeInBytes, const AudioFormat& targetAudioFormat)
	{
		// If we've finished playback (or never started), don't try to fill the buffers
		if (!mPlaying || !isAudioEnabled())
			return false;

		// Here we are going to fill the audio buffer. The audio buffer has a certain fixed size,
		// the decoded audio buffer will most probably not match the target buffer size: it can be greater, 
		// equal or smaller than the buffer we need to fill. 

		uint8_t* dest = dataBuffer;
		int data_remaining = sizeInBytes;
		while (data_remaining > 0)
		{
			// Is the last frame we have have read fully copied to the target buffer? If so, we need to 
			// decode a new audio frame. Otherwise, we can continue with the existing decoded buffer and 
			// copy a part of the already decoded frame to the target buffer.
			// (The last decoded frame could also be the last frame from the previous audio callback).
			if (mAudioFrameReadOffset >= mAudioFrameSize)
				if (!getNextAudioFrame(targetAudioFormat))
					return false;

			int num_bytes_to_read = mAudioFrameSize - mAudioFrameReadOffset;
			num_bytes_to_read = std::min(num_bytes_to_read, data_remaining);

			uint8_t* source = mCurrentAudioBuffer + mAudioFrameReadOffset;
			memcpy(dest, source, num_bytes_to_read);

			data_remaining -= num_bytes_to_read;
			dest += num_bytes_to_read;
			mAudioFrameReadOffset += num_bytes_to_read;
		}

		// After copying data into the buffer, it will not immediately play; there is some time between the return of this callback and the audio actually playing.
		// However, we've already advanced the audio decode clock (in getNextAudioFrame) to the *end* of the audio frame, while it hasn't started playing yet.
		// Syncing video to the audio decode clock would therefore result in out-of-sync audio/video.
		//
		// To correct for this delay, we determine the 'actual' audio clock, by calculating the difference between the decoded time and the time of the first sample
		// that still needs to be played. 
		// Furthermore, we assume that the audio system in use will not immediately play the audio after returning from this function, 
		// due to double buffering (i.e. when this function returns, a previous buffer is still playing).
		// Therefore, we multiply the incoming buffer size by 2, to account for this double buffering. Note that this is a 'best guess'; the actual delay may be smaller or larger, 
		// but we can't know it exactly.
		uint64_t bytes_decoded = 2 * sizeInBytes + (mAudioFrameSize - mAudioFrameReadOffset);
		uint64_t bytes_per_sec = av_samples_get_buffer_size(NULL, av_get_channel_layout_nb_channels(targetAudioFormat.getChannelLayout()), targetAudioFormat.getSampleRate(), (AVSampleFormat)targetAudioFormat.getSampleFormat(), 1);
		double decoded_time = (double)bytes_decoded / (double)bytes_per_sec;

		// Set audio clock to actual 'play' time. We only do this if we're in playing state to work around a race condition
		// when a seek is started, which resets the audio clock. After the audio clock is reset, the audio callback may still be in progress,
		// which would set the audio clock back to an actual time, causing issues with seeking.
		// Note that this condition is potentially a race condition as well, but the scope for it is much smaller
		if (mIOThreadState == IOThreadState::Playing)
			mAudioClockSecs = mAudioDecodeClockSecs - decoded_time;

		return true;
	}


	bool Video::update(double deltaTime, utility::ErrorState& errorState)
	{
		if (!mPlaying)
			return true;

		// If the frame time spikes, make sure we re-sync to the first frame again, otherwise it may be possible that
		// the main thread is trying to catch up, but it never really can catch up
 		if (deltaTime > 1.0)
 			mSystemClockSecs = sClockMax;

		// Update system clock if it has been initialized
		if (mSystemClockSecs != sClockMax)
			mSystemClockSecs += (deltaTime * mSpeed);

		// Peek into the frame queue. If we have a frame and the PTS value of the first frame on
		// the FIFO queue has expired, we pop it. If there is no frame or the frame has not expired,
		// we return, effectively keeping the same contents in the existing textures.
		Frame cur_frame;
		{
			// This can be enabled to block waiting until data is available
			// mFrameDataAvailableCondition.wait(lock, [this]() { return !mFrameQueue.empty(); });

			if (mVideoState.isFinished())
			{
				stop(true);
				return errorState.check(mErrorMessage.empty(), mErrorMessage.c_str());
			}

			// Initialize the system clock to the first frame we see (if it has not been initialized yet)
			if (mSystemClockSecs == sClockMax)
			{
				Frame frame = mVideoState.peekFrame();
				if (frame.isValid())
					mSystemClockSecs = frame.mPTSSecs;
			}

			// If the video we're playing has an audio stream, we use the audio clock to present frames.
			// This makes sure that audio/video remain in sync. If there is no audio stream, we use the system clock.
			double display_clock = isAudioEnabled() ? mAudioClockSecs : mSystemClockSecs;

			// Try to get next frame to display (based on display clock)			
			cur_frame = mVideoState.tryPopFrame(display_clock);

			if (!cur_frame.isValid() || display_clock == sClockMax)
			{
				cur_frame.free();
				return true;
			}
		}

		// Copy data into texture
		mYTexture->update(cur_frame.mFrame->data[0], cur_frame.mFrame->linesize[0]);
		mUTexture->update(cur_frame.mFrame->data[1], cur_frame.mFrame->linesize[1]);
		mVTexture->update(cur_frame.mFrame->data[2], cur_frame.mFrame->linesize[2]);

		// Destroy frame that was allocated in the decode thread, after it has been processed
		cur_frame.free();

		return true;
	}
}
