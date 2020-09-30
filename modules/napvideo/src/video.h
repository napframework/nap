/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <condition_variable>
#include <queue>
#include <thread>
#include <mutex>
#include <climits>
#include <cassert>
#include <nap/resource.h>
#include <rtti/factory.h>
#include <utility/autoresetevent.h>
#include <nap/signalslot.h>

struct AVPacket;
struct AVCodec;
struct AVCodecParserContext;
struct AVCodecContext;
struct AVFormatContext;
struct AVFrame;
struct SwrContext;
struct AVDictionary;
struct AVStream;

namespace nap
{
	// Forward Declares
	class Video;
	namespace audio
	{
		class VideoNode;
	}

	/**
	 * Description of an audio format for converting source data to a target format.
	 */
	class AudioFormat
	{
	public:
		enum class EChannelLayout : uint8_t
		{
			Mono,
			Stereo,
			_2Point1,
			_2_1,
			Surround,
			_3Point1,
			_4Point0,
			_4Point1,
			_2_2,
			Quad,
			_5Point0,
			_5Point1,
			_5Point0_Back,
			_5Point1_Back,
			_6Point0,
			_6Point0_Front,
			Hexagonal,
			_6Point1,
			_6Point1_Back,
			_6Point1_Front,
			_7Point0,
			_7Point0_Front,
			_7Point1,
			_7Point1_Wide,
			_7Point1_Wide_Back,
			Octagonal,
			Hexadecagonal,
			Stereo_Downmix
		};

		/**
		 * Video sample format
		 */
		enum class ESampleFormat 
		{
			U8,         ///< unsigned 8 bits
			S16,        ///< signed 16 bits
			S32,        ///< signed 32 bits
			FLT,		///< float
			DBL,        ///< double
			S64			///< signed 64 bits
		};

		/**
		 * Constructor taking explicit channel layout.
		 */
		AudioFormat(EChannelLayout channelLayout, ESampleFormat sampleFormat, int sampleRate);

		/**
		 * Constructor taking number of channels. Uses default channel layout for this number of channels.
		 */
		AudioFormat(int numChannels, ESampleFormat sampleFormat, int sampleRate);

		/**
		 * @return samplerate, in hz.
		 */
		int getSampleRate() const { return mSampleRate; }

		/**
		 * @return ffmpeg-style channel layout (not EChannelLayout)
		 */
		int64_t getChannelLayout() const { return mChannelLayout; }

		/**
		 * @return ffmpeg-style sample format (not ESampleFormat)
		 */
		int getSampleFormat() const { return mSampleFormat; }

	private:
		int			mSampleRate;			///< Samplerate, in hz. For instance, 48000
		int64_t		mChannelLayout;			///< One of AV_CH_LAYOUT_xxx defines as defined in channel_layout.h
		int			mSampleFormat;			///< One of the value of the AVSampleFormat enum, as defined in samplefmt.h
	};


	/**
	 * Frame as pushed in the frame queue.
	 */
	struct NAPAPI Frame
	{
		bool isValid() const { return mFrame != nullptr; }
		void free();

		AVFrame*	mFrame = nullptr;		///< Frame as decoded by the decode thread
		double		mPTSSecs = 0.0;			///< When the frame needs to be displayed (absolute clock time)
		int			mFirstPacketDTS = 0;	///< First dts that was used to create this frame
	};


	/**
	 * Full state for either audio or video. 
	 * Responsible for pulling packets from the packet queue and decoding them into frames.
	 */
	class NAPAPI AVState final
	{
	public:
		using OnClearFrameQueueFunction = std::function<void()>;

		/**
		 * Constructor
		 * @param video: video to play
		 */
		AVState(Video& video);

		/**
		 * Destructor
		 */
		~AVState();
		 
		/**
		 * Initializes stream and codec.
		 * @param stream Video or audio stream index.
		 * @param codec codec to use
		 * @param codecContext context associated with the codec
		 */
		void init(int stream, AVCodec* codec, AVCodecContext* codecContext);

		/**
		 * Destroys codec.
		 */
		void close();

		/**
 		 * @return whether object is initialized.
		 */
		bool isValid() const { return mStream != -1; }

		/**
		 * @return Index of this stream.
		 */
		int getStream() const { return mStream; }

		/**
		 * Spawns the decode thread.
		 * @param onClearFrameQueueFunction An optional function that can be used to perform additional work when the frame queue is cleared.
		 */
		void startDecodeThread(const OnClearFrameQueueFunction& onClearFrameQueueFunction = OnClearFrameQueueFunction());

		/**
		 * Stops the decode thread and blocks waiting for it to exit if join is true.
		 * @param join If true, the function blocks until the thread is exited, otherwise false.
		 */
		void exitDecodeThread(bool join);

		/**
		 * @return True when there are no more frames to produce (there are no more packets to decode) and all frames have been
		 * consumed by the client.
		 */
		bool isFinished() const;

		/**
		 * Clears frame queue.
		 */
		void clearFrameQueue();

		/**
		 * Clears packet queue.
		 */
		void clearPacketQueue();

		/**
		 * @return true when the packet belongs to the stream for this AVState.
		 */
		bool matchesStream(const AVPacket& packet) const;

		/**
		 * Adds the 'seek start' packet to the packet queue. This functions as a command to the decode thread.
		 */
		bool addSeekStartPacket(const bool& exitIOThreadSignalled);

		/**
		 * Adds the 'seek end' packet to the packet queue. This functions as a command to the decode thread.
		 */
		bool addSeekEndPacket(const bool& exitIOThreadSignalled, double seekTargetSecs);

		/**
		 * Adds the 'end of file' packet to the packet queue. The end of file packet is a special packet
		 * with a nullptr for data.
		 */
		bool addEndOfFilePacket(const bool& exitIOThreadSignalled);

		/**
		 * Adds the 'I/O finished' packet to the packet queue. This functions as a command to the decode thread.
		 */
		bool addIOFinishedPacket(const bool& exitIOThreadSignalled);

		/**
		 * Adds a packet to the packet queue.
		 * @param packet The packet to add.
		 * @param exitIOThreadSignalled When I/O thread is in exit mode, this must be true.
		 * @return if packet was added
		 */
		bool addPacket(AVPacket& packet, const bool& exitIOThreadSignalled);

		/**
		 * Wait for the frame to be empty
		 * @param exitIOThreadSignalled When I/O thread is in exit mode, this must be true.
		 * @return if the queue emptied.
		 */
		bool waitForFrameQueueEmpty(bool& exitIOThreadSignalled);

		/**
		 * Cancel any outstanding waits for the frame queue to be empty
		 */
		void cancelWaitForFrameQueueEmpty();

		/**
		 * Reset the state of the frame queue so that subsequent waits will wait normally
		 */
		void resetWaitForFrameQueueEmpty();

		/**
		 * Blocks until the EOF packet that was added by addEndOfFilePacket is consumed by the decode thread.
		 */
		bool waitForEndOfFileProcessed();

		/**
		 * Cancel any outstanding waits for end of file to be processed
		 */
		void cancelWaitForEndOfFileProcessed();

		/**
		 * Reset the state of the end of file processed event so that subsequent waits will wait normally
		 */
		void resetEndOfFileProcessed();

		/**
		 * Blocks until the 'seek start' packet that was added by addSeekStartPacket is consumed by the decode thread.
		 */
		void waitSeekStartPacketProcessed();
		
		/**
		 * Blocks until the decode thread has called avcoded_receive. This is used to perform lock-step production of packets and frames.
		 * @return Result of avcoded_receive_frame
		 */
		int waitForReceiveFrame();

		/**
		 * Consumes all frames in the frame queue until the decode thread needs new packets.
		 */
		void drainSeekFrameQueue();

		/**
		 * Block until a frame can be popped from the queue.
		 * @return The next frame on the queue. If the thread was exited during this operation, an empty frame is returned.
		 */
		Frame popFrame();

		/**
		 * Non-blocking popping of the next frame. The pts is checked to see if the next frame is equal or greater than the pts passed. If so,
		 * the frame is popped. Otherwise, an empty frame is returned.
		 * @param pts The 'current time' that is tested against the frames' PTS.
		 */
		Frame tryPopFrame(double pts);

		/**
		 * @return If there was a frame on the queue, returns it without removing it from the queue.
		 */
		Frame peekFrame();

		/**
		 * @return If there was a frame on the 'seek' frame queue, returns it without removing it from the queue.
		 */
		Frame popSeekFrame();

		/**
		 * Prepares for start of I/O thread, reset synchronization primitives.
		 */
		void notifyStartIOThread();

		/**
		 * Unblocks synchronization primitives for succesful exit of I/O thread.
		 */
		void notifyExitIOThread();

		/**
		 * @return audio or video codec used to decode packets into frames.
		 */
		AVCodec& getCodec() { return *mCodec; }

		/**
		 * @return audio or video codec context used to decode packets into frames.
		 */
		AVCodecContext& getCodecContext() { return *mCodecContext; }

	private:
		enum class EDecodeFrameResult
		{
			GotFrame,
			EndOfFile,
			Exit
		};

		void decodeThread();
		EDecodeFrameResult decodeFrame(AVFrame& frame, int& frameFirstPacketDTS);
		AVPacket* popPacket();
		void clearFrameQueue(std::queue<Frame>& frameQueue, bool emitCallback);

	private:
		using FrameQueue = std::queue<Frame>;

		Video*						mVideo;
		AVCodec*					mCodec = nullptr;
		AVCodecContext*				mCodecContext = nullptr;

		int							mStream = -1;							///< Specifies what stream in the file is the one containing video packets

		std::thread					mDecodeThread;							///< Video decode thread
		OnClearFrameQueueFunction	mOnClearFrameQueueFunction;				///< Callback that is called when the frame queue is cleared
		bool						mExitDecodeThreadSignalled = false;		///< If this boolean is set, the decode thread will exit ASAP. This is used internally by exitDecodeThread and should not be used separately

		FrameQueue					mFrameQueue;							///< The frame queue as produced by the decodeThread and consumed by the main thread
		FrameQueue					mSeekFrameQueue;						///< The frame queue as produced by the decodeThread and consumed by the main thread
		FrameQueue*					mActiveFrameQueue = &mFrameQueue;		
		mutable std::mutex			mFrameQueueMutex;						///< Mutex protection for the frame queue
		std::condition_variable		mFrameDataAvailableCondition;			///< Condition describing whether there is data in the frame queue to process
		std::condition_variable		mFrameQueueRoomAvailableCondition;		///< Condition describing whether there is still room in the frame queue to add new frames
		bool						mCancelWaitFrameQueueEmpty = false;		///< Whether the wait for the frame queue to be empty should be cancelled

		// Producer/consumer variables for packet queue:
		std::queue<AVPacket*>		mPacketQueue;							///< The packet queue as produced by the ioThread and consumed by the decodeThread thread
		std::mutex					mPacketQueueMutex;						///< Mutex protection for the packet queue
		std::condition_variable		mPacketAvailableCondition;				///< Condition describing whether there is data in the packet queue to process
		
		bool						mFinishedProducingFrames = false;		///< If this boolean is set, the decode thread has no more frames to decode, either due to an error or due to an end of stream.

		std::unique_ptr<AVPacket>	mSeekStartPacket;						///< Specific 'command' packet to communicate 'seek start' to decode thread
		std::unique_ptr<AVPacket>	mSeekEndPacket;							///< Specific 'command' packet to communicate 'seek end' to decode thread
		std::unique_ptr<AVPacket>	mEndOfFilePacket;						///< Specific 'command' packet to communicate 'EndOfFile' to decode thread
		std::unique_ptr<AVPacket>	mIOFinishedPacket;						///< Specific 'command' packet to communicate 'I/O finished' to decode thread
		double						mSeekTargetSecs;						///< When a seek end packet is added, this value is set to the time in seconds of the seek target

		utility::AutoResetEvent		mEndOfFileProcessedEvent;				///< Event that is signaled when EndOfFile packet is consumed by decode thread
		utility::AutoResetEvent		mSeekStartProcessedEvent;				///< Event that is signaled when seek start packet is consumed by decode thread
		utility::AutoResetEvent		mReceiveFrameEvent;						///< Event that is signaled when avcodec_receive is called by decode thread
		int							mReceiveFrameResult = 0;				///< Value set when avcoded_receive requires more packet to produce a frame

		double						mLastFramePTSSecs = 0.0;				///< The PTS of the last frame in seconds, used to 'guess' the PTS of a new frame if it's unknown.
		int							mFrameFirstPacketDTS = -INT_MAX;		///< Cached value for the first DTS that was used to produce the current frame
	};


	/**
	 * Decodes a video using FFMPEG in the background.
	 * It is NOT recommended to manually (at run-time) create a nap::Video. Use the nap::VideoPlayer instead.
	 * The nap::VideoPlayer consumes frames, produced by a video, when they are ready to be presented.
	 * This object does not contain any textures, only FFMPEG related content.
	 *
	 * Call init() after construction and start() / stop() afterwards. 
	 * On initialization the video file is opened, streams are extracted and the video / audio state is initialized.
	 * A video stream is required, the audio stream is optional. Use a nap::VideoAudioComponent to decode and output the audio stream.
	 * On start() the IO and decode threads are spawned. On stop() the IO and decode threads are stopped.
	 * The av and codec contexts are freed on destruction. 
	 * Keeping the format and codec contexts in memory ensures the loaded video can be started and stopped fast.
	 */
	class NAPAPI Video final
	{
	public:
		/**
		 * @param path the video file on disk
		 */
		Video(const std::string& path);

		// Destructor
		virtual ~Video();

		/**
		 * Copy is not allowed
		 */
		Video(Video&) = delete;

		/**
		 * Copy assignment is not allowed
		 */
		Video& operator=(const Video&) = delete;

		/**
		 * Move is not allowed
		 */
		Video(Video&&) = delete;

		/**
		 * Move assignment is not allowed
		 */
		Video& operator=(Video&&) = delete;

		/**
		 * Initializes the video. Finds decoder, sets up everything necessary to start playback.
		 * @param errorState Contains detailed information about errors if this function return false.
		 * @return True on success, false otherwise. 
		 */
		virtual bool init(utility::ErrorState& errorState);

		/**
		 * Returns a newly decoded frame if available, an invalid frame otherwise.
		 * Always call .free() after processing frame content! This is a non-blocking call.
		 * @param deltaTime time in seconds in between calls.
		 * @return a newly decoded frame if available, an invalid frame otherwise.
		 */
		Frame update(double deltaTime);

		/**
		 * Starts playback of the video at the given time in seconds.
		 * This will spawn the video IO and decode threads in the background.
		 * Video is stopped before being started.
		 * @param time the offset in seconds to start the video at.
		 */
		void play(double time = 0.0);

		/**
		 * Check whether the video is currently playing
		 * @return True if the video is currently playing, false if not
		 */
		bool isPlaying() const					{ return mPlaying; }

		/**
		 * Stops playback of the video.
		 * The video IO and decode threads are stopped.
		 * @param blocking if the calling thread waits for the IO and decode thread to stop.
		 */
		void stop(bool blocking);

		/**
		 * Seeks within the video to the time provided. This can be called during playback.
		 * @param seconds: video offset in seconds.
		 */
		void seek(double seconds);

		/**
		 * @return The current playback position, in seconds.
		 */
		double getCurrentTime() const;

		/**
		 * @return The duration of the video in seconds.
		 */
		double getDuration() const				{ return mDuration; }

		/**
		 * @return Width of the video, in pixels.
		 */
		int getWidth() const					{ return mWidth; }

		/**
		 * @return Height of the video, in pixels.
		 */
		int getHeight() const					{ return mHeight; }

		/**
		 * @return path to the video file on disk
		 */
		const std::string& getPath() const		{ return mPath; }

		/**
		 * @return Whether this video has an audio stream.
		 */
		bool hasAudio() const					{ return mAudioState.isValid(); }

		/**
		 * Returns if audio decoding and playback is enabled.
		 * This is the case when there is an audio stream available and audio decoding is explicitly enabled.
		 * @return whether audio decoding and playback is enabled. 
		 */
		bool audioEnabled() const				{ return hasAudio() && mDecodeAudio; }

		bool		mLoop = false;				///< If the video needs to loop
		float		mSpeed = 1.0f;				///< Video playback speed
        
        nap::Signal<Video&> mDestructedSignal; ///< This signal will be emitted before the Video resource is destructed

	private:

		friend class audio::VideoNode;
		friend class AVState;

		enum class EProducePacketResult : uint8_t
		{
			GotAudioPacket		= 1,													///< Received an audio packet
			GotVideoPacket		= 2,													///< Received a video packet
			GotUnknownPacket	= 4,													///< Received an unknown packet
			GotPacket			= GotAudioPacket | GotVideoPacket | GotUnknownPacket,	///< Received either an audio or video packet
			EndOfFile			= 8,													///< EndOfFile was reached, no packet was pushed
			Error				= 16													///< An error occurred during stream reading
		};

		enum class IOThreadState
		{
			WaitingForEOF,				///< Waiting for EOF to be processed state
			Playing,					///< Regular playing state
			SeekRequest,				///< First stage of seeking, handling the request, finding target to seek to
			SeekingStartFrame,			///< Second stage of seeking, iterative seeking of start keyframe
			SeekingTargetFrame			///< Third stage of seeking, decoding up until the target PTS
		};

		/**
		 * Constructs AVState object and initializes codec and stream.
		 */
		static bool sInitAVState(AVState& destState, const AVStream& stream, AVDictionary*& options, utility::ErrorState& errorState);

		/**
		 * Thread reads packets from the stream and pushes them onto the packet queue.
		 */
		void ioThread();
		
		/**
		 * Starts the I/O thread;
		 */
		void startIOThread();

		/**
		 * Reads packet from the stream and pushes packets onto the packet queue.
		 * @targetState The state for which packets should be pushed onto the packet queue. If null, the packets are pushed for all states. Note that packets
		 *                    are still read from the stream, but just not pushed onto the queue.
		 * @return The result from reading packets from the stream. See @EProducePacketResult.
		 */
		EProducePacketResult ProducePacket(AVState* targetState);

		/**
		 * Calling this ensures that the thread is waked properly if in a wait state, and then exited. 
		 * @param join Select whether this function blocks until the thread is exited. If false, the function is non-blocking.
		 */
		void exitIOThread(bool join);

		/**
		 * Pops frame from the frame queue and performs samplerate conversion to the target format.
		 * @param targetAudioFormat audio format to convert to.
		 */
		bool getNextAudioFrame(const AudioFormat& targetAudioFormat);

		/**
		 * Clears packet queue for both audio and video state.
		 */
		void clearPacketQueue();

		/**
		 * Clears frame queue for both audio and video state.
		 */
		void clearFrameQueue();
		
		/**
		 * Callback that is called when the video frame queue is cleared.
		 */
		void onClearVideoFrameQueue();

		/**
		 * Callback that is called when the audio frame queue is cleared.
		 */
		void onClearAudioFrameQueue();

		/**
		 * Called by functions on critical error. Stops execution of video.
		 */
		void setErrorOccurred(const std::string& errorMessage);

		/**
		 * Finishes the seeking operation and returns back to playing state.
		 */
		void finishSeeking(AVState& seekState, bool shouldDrainFrameQueue);

		/**
		 * Set the internal seek target to the specified number of seconds in the timebase of the provided AVState
		 */
		void setSeekTarget(AVState& seekState, double seekTargetSecs);

		/**
		 * Set the state of the IO thread
		 * @param threadState the new io thread state
		 */
		void setIOThreadState(IOThreadState threadState);

		/**
		 * Allocate a packet of specified size. Will block if the max size is reached
		 * @param inPacketSize The size (in bytes) to allocate.
		 */
		bool allocatePacket(uint64_t inPacketSize);

		/**
		 * Deallocate a packet of specified size
		 * @param inPacketSize The size (in bytes) to deallocate.
		 */
		void deallocatePacket(uint64_t inPacketSize);

		/**
		 * Set whether audio decoding and playback is enabled for the video. If enabled, video will be synced to the audio clock.
		 * Enabling this requires that somebody calls OnAudioCallback() to advance the audio clock; this is not handled internally.
		 *
		 * Note that this can only be changed *before* play() is called!
		 * @param enabled if audio decoding and playback is enabled.
		 */
		void decodeAudioStream(bool enabled);

		/**
		 * Function that needs to be called by the audio system on a fixed frequency to copy the audio data from the audio
		 * stream into the target buffer.
		 * @param dataBuffer The data buffer to fill.
		 * @param sizeInBytes Length of the data buffer, in bytes.
		 * @param targetAudioFormat The expected format of the audio data to put in dataBuffer.
		 * @return true if copy succeeded, false otherwise.
		 */
		bool OnAudioCallback(uint8_t* dataBuffer, int sizeInBytes, const AudioFormat& targetAudioFormat);

	private:
		std::string				mPath;										
		static const double		sClockMax;
		AVFormatContext*		mFormatContext = nullptr;
		bool					mPlaying = false;							///< Set if playing. Should only be controlled from main thread
		int						mWidth = 0;									///< Width of the video, in pixels
		int						mHeight = 0;								///< Height of the video, in pixels
		float					mDuration = 0.0f;							///< Duration of the video in seconds
		double					mSystemClockSecs = sClockMax;				///< Clock that we use to synchronize the video to if there is no audio stream
		double					mAudioDecodeClockSecs = -1;					///< Clock that indicates up to which time the audio thread has decoded frame
		double					mAudioClockSecs = -1;						///< Clock that indicates the actual time of the *playing* audio

		std::string				mErrorMessage;								///< If an error occurs, this is the string containing error information. If empty, no error occured.
		
		std::mutex				mTotalPacketQueueSizeLock;					///< Lock guarding the current packet queue size
		std::condition_variable	mPacketQueueRoomAvailableCondition;			///< Condition used to wait for room to be available in the packet queue
		uint64_t				mTotalPacketQueueSize = 0;					///< Total packet size in use

		AVState					mVideoState;								///< State containing all video decoding
		AVState					mAudioState;								///< State containing all audio decoding
		AVState*				mSeekState = nullptr;						///< The AVState that we use to seek against
		std::thread				mIOThread;									///< IO thread, reads packets and pushed onto frame queue

		bool					mExitIOThreadSignalled = false;				///< If this boolean is set, the IO thread will exit ASAP. This is used internally by exitIOThread and should not be used separately
		
		int64_t					mSeekKeyframeTarget = -1;					///< Seek target, in internal stream units (not secs). This value is used iteratively and can be different from the original seek target
		int64_t					mSeekTarget = -1;							///< Seek target as set by the user, in internal steram units (not secs)
		double					mSeekTargetSecs = 0.0f;						///< Seek target as set by the user, in secs.

		SwrContext*				mAudioResampleContext = nullptr;			///< Context used for resampling to the target audio format
		Frame					mCurrentAudioFrame;							///< Audio frame currently being decoded. A single audio frame can cross the boundaries of a single audio callback, so it is cached
		std::vector<uint8_t>	mAudioResampleBuffer;						///< Current resampled audio buffer. Can cross the boundaries of a single audio callback, so it is cached.
		uint8_t*				mCurrentAudioBuffer = nullptr;				///< Pointer to either the buffer in the current audio frame itself, or the resampled audio buffer.
		uint64_t				mAudioFrameReadOffset = 0;					///< Offset (cursor) into the current audio buffer that is decoded
		uint64_t				mAudioFrameSize = 0;						///< Size of the current decoded (and possible resampled) audio buffer, in bytes
		bool					mDecodeAudio = false;						///< Whether audio is enabled or not
		IOThreadState			mIOThreadState = IOThreadState::Playing;	///< FSM state of the I/O thread
	};
}
