#pragma once

#include <condition_variable>
#include <queue>
#include <thread>
#include <mutex>
#include <limits>
#include <nap/resource.h>
#include <rtti/factory.h>
#include <utility/autoresetevent.h>

struct AVPacket;
struct AVCodec;
struct AVCodecParserContext;
struct AVCodecContext;
struct AVFormatContext;
struct AVFrame;
struct SwrContext;
struct AVDictionary;

namespace opengl
{
	class Texture2D;
}

namespace nap
{
	class RenderTexture2D;
	class VideoService;
	class Video;

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
	struct Frame
	{
		bool isValid() const { return mFrame != nullptr; }
		void free();

		AVFrame*	mFrame = nullptr;	///< Frame as decoded by the decode thread
		double		mPTSSecs;			///< When the frame needs to be displayed (absolute clock time)
		int			mFirstPacketDTS;	///< First dts that was used to create this frame
	};

	/**
	 * Full state for either audio or video. Responsible for pulling packets from the packet queue and decoding them into frames.
	 */
	class AVState final
	{
	public:
		using OnClearFrameQueueFunction = std::function<void()>;

		/**
		 * Constructor
		 * @param video: video to play
		 * @param maxPacketQueueSize
		 */
		AVState(Video& video, int maxPacketQueueSize);

		/**
		 * Destructor
		 */
		~AVState();
		 
		/**
		 * Initializes stream and codec.
		 * @param stream Video or audio stream index.
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
		 * @param clearFrameQueueFunction An optional function that can be used to perform additional work when the frame queue is cleared.
		 */
		void startDecodeThread(const OnClearFrameQueueFunction& onClearFrameQueueFunction = OnClearFrameQueueFunction());

		/**
		 * Stops the decode thread and blocks waiting for it to exit if @join is true.
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
		 * @param exitIOThreadsignalled When I/O thread is in exit mode, this must be true.
		 */
		bool addPacket(AVPacket& packet, const bool& exitIOThreadSignalled);

		/**
		 * Blocks until the EOF packet that was added by addEndOfFilePacket is consumed by the decode thread.
		 */
		void waitForEndOfFileProcessed();

		/**
		 * Blocks until the 'seek start' packet that was added by addSeekStartPacket is consumed by the decode thread.
		 */
		void waitSeekStartPacketProcessed();
		
		/**
		 * Blocks until the decode thread has called avcoded_receive. This is used to perform lock-step production of packets and frames.
		 * @return Wether avcoded_receive requires a new packet to be pushed onto the packet queue.
		 */
		bool waitForReceiveFrame();

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
		
		// Producer/consumer variables for packet queue:
		std::queue<AVPacket*>		mPacketQueue;							///< The packet queue as produced by the ioThread and consumed by the decodeThread thread
		std::mutex					mPacketQueueMutex;						///< Mutex protection for the packet queue
		std::condition_variable		mPacketAvailableCondition;				///< Condition describing whether there is data in the packet queue to process
		std::condition_variable		mPacketQueueRoomAvailableCondition;		///< Condition describing whether there is still room in the packet queue to add new packets
		int							mMaxPacketQueueSize;

		bool						mFinishedProducingFrames = false;		///< If this boolean is set, the decode thread has no more frames to decode, either due to an error or due to an end of stream.

		std::unique_ptr<AVPacket>	mSeekStartPacket;						///< Specific 'command' packet to communicate 'seek start' to decode thread
		std::unique_ptr<AVPacket>	mSeekEndPacket;							///< Specific 'command' packet to communicate 'seek end' to decode thread
		std::unique_ptr<AVPacket>	mEndOfFilePacket;						///< Specific 'command' packet to communicate 'EndOfFile' to decode thread
		std::unique_ptr<AVPacket>	mIOFinishedPacket;						///< Specific 'command' packet to communicate 'I/O finished' to decode thread
		double						mSeekTargetSecs;						///< When a seek end packet is added, this value is set to the time in seconds of the seek target

		utility::AutoResetEvent		mEndOfFileProcessedEvent;				///< Event that is signaled when EndOfFile packet is consumed by decode thread
		utility::AutoResetEvent		mSeekStartProcessedEvent;				///< Event that is signaled when seek start packet is consumed by decode thread
		utility::AutoResetEvent		mReceiveFrameEvent;						///< Event that is signaled when avcodec_receive is called by decode thread
		bool						mReceiveFrameNeedsPacket = false;		///< Value set when avcoded_receive requires more packet to produce a frame

		double						mLastFramePTSSecs = 0.0;				///< The PTS of the last frame in seconds, used to 'guess' the PTS of a new frame if it's unknown.
		int							mFrameFirstPacketDTS = -INT_MAX;		///< Cached value for the first DTS that was used to produce the current frame
	};


	/**
	 * Video playback.
	 * Internally contains textures that have the contents for each frame. After calling update(), the texture is filled with
	 * the latest frame. update() is not blocking, internally the textures will only be updated when needed. The textures that
	 * are output are in the YUV format. Conversion to RGB can be done in a shader.
	 * The main thread will consume the frames when they are present in the frame queue and their timestamp has 'passed'.
	 */
	class NAPAPI Video final : public Resource
	{
		RTTI_ENABLE(Resource)

	public:
		/**
		 *	Constructor
		 */
		Video(VideoService& service);

		/**
		 * destructor
		 */
		virtual ~Video();

		/**
		 * Initializes the video. Finds decoder, sets up everything necessary to start playback.
		 * @param errorState Contains detailed information about errors if this function return false.
		 * @return True on success, false otherwise. 
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Updates the internal textures if a new frame has been decoded.
		 * @param errorState Contains detailed information about errors if this function return false.
		 * @return True on success, false otherwise.
		 */
		bool update(double deltaTime, utility::ErrorState& errorState);

		/**
		 * Starts playback of the video at the offset given by @startTimeSecs.
		 * @param startTimeSecs The offset in seconds to start the video at.
		 */
		void play(double startTimeSecs = 0.0);

		/**
		 * Check whether the video is currently playing
		 *
		 * @return True if the video is currently playing, false if not
		 */
		bool isPlaying() const { return mPlaying; }

		/**
		 * Stops playback of the video.
		 */
		void stop(bool blocking);

		/**
		 * Seeks within the video to the time provided. This can be called while playing.
		 * @param seconds: the time offset in seconds in the videp.
		 */
		void seek(double seconds);

		/**
		 * @return The current playback position, in seconds.
		 */
		double getCurrentTime() const;

		/**
		 * @return The duration of the video in seconds.
		 */
		double getDuration() const { return mDuration; }

		/**
		 * @return The Y texture as it is updated by update(). Initially, the texture is not initialized
		 * to zero, but to the 'black' equivalent in YUV space. The size of the Y texture is width * height.
		 */
		RenderTexture2D& getYTexture()			{ return *mYTexture; }

		/**
		 * @return The U texture as it is updated by update(). Initially, the texture is not initialized
		 * to zero, but to the 'black' equivalent in YUV space. The size of the Y texture is HALF the width * height.
		 */
		RenderTexture2D& getUTexture()			{ return *mUTexture; }

		/**
		 * @return The V texture as it is updated by update(). Initially, the texture is not initialized
		 * to zero, but to the 'black' equivalent in YUV space. The size of the V texture is HALF the width * height.
		 */
		RenderTexture2D& getVTexture()			{ return *mVTexture; }

		/**
		 * @return Width of the video, in pixels.
		 */
		int getWidth() const					{ return mWidth; }

		/**
		 * @return Height of the video, in pixels.
		 */
		int getHeight() const					{ return mHeight; }

		/**
		 * @return Whether this video has an audio stream.
		 */
		bool hasAudio() const					{ return mAudioState.getStream() != -1; }

		/**
		 * Function that needs to be called by the audio system on a fixed frequency to copy the audio data from the audio
		 * stream into the target buffer.
		 * @param dataBuffer The data buffer to fill.
		 * @param sizeInBytes Length of the data buffer, in bytes.
		 * @param targetAudioFormat The expected format of the audio data to put in dataBuffer.
		 */
		bool OnAudioCallback(uint8_t* dataBuffer, int sizeInBytes, const AudioFormat& targetAudioFormat);

		std::string mPath;				///< Path to the video to playback
		bool		mLoop = false;		///< If the video needs to loop
		float		mSpeed = 1.0f;		///< Video playback speed

	private:
		enum class EProducePacketResult : uint8_t
		{
			GotAudioPacket		= 1,													///< Received an audio packet
			GotVideoPacket		= 2,													///< Received a video packet
			GotUnknownPacket	= 4,													///< Received an unknown packet
			GotPacket			= GotAudioPacket | GotVideoPacket | GotUnknownPacket,	///< Received either an audio or video packet
			EndOfFile			= 8,													///< EndOfFile was reached, no packet was pushed
			Error				= 16													///< An error occurred during stream reading
		};

		/**
		 * Constructs AVState object and initializes codec and stream.
		 */
		static bool sInitAVState(AVState& destState, int streamIndex, const AVCodecContext& sourceCodecContext, AVDictionary*& options, utility::ErrorState& errorState);

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
		 * @inAddAudioPackets Set to true to push audio packets on the queue. Note that if this is false, the packets
		 *                    are still read from the stream, but just not pushed onto the queue.
		 * @return The result from reading packets from the stream. See @EProducePacketResult.
		 */
		EProducePacketResult ProducePacket(bool inAddAudioPackets);

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
		void finishSeeking();

	private:
		friend class AVState;

		enum class IOThreadState
		{
			Playing,					///< Regular playing state
			SeekRequest,				///< First stage of seeking, handling the request, finding target to seek to
			SeekingStartFrame,			///< Second stage of seeking, iterative seeking of start keyframe
			SeekingTargetFrame			///< Third stage of seeking, decoding up until the target PTS
		};

		static const double		sClockMax;

		std::unique_ptr<RenderTexture2D> mYTexture;
		std::unique_ptr<RenderTexture2D> mUTexture;
		std::unique_ptr<RenderTexture2D> mVTexture;

		AVFormatContext*		mFormatContext = nullptr;
		bool					mPlaying = false;							///< Set if playing. Should only be controlled from main thread
		int						mWidth = 0;									///< Width of the video, in pixels
		int						mHeight = 0;								///< Height of the video, in pixels
		float					mDuration = 0.0f;							///< Duration of the video in seconds
		double					mSystemClockSecs = sClockMax;				///< Clock that we use to synchronize the video to if there is no audio stream
		double					mAudioDecodeClockSecs = sClockMax;			///< Clock that indicates up to which time the audio thread has decoded frame
		double					mAudioClockSecs = sClockMax;				///< Clock that indicates the actual time of the *playing* audio

		std::string				mErrorMessage;								///< If an error occurs, this is the string containing error information. If empty, no error occured.
		
		AVState					mVideoState;								///< State containing all video decoding
		AVState					mAudioState;								///< State containing all audio decoding
		std::thread				mIOThread;									///< IO thread, reads packets and pushed onto frame queue

		bool					mExitIOThreadSignalled = false;				///< If this boolean is set, the IO thread will exit ASAP. This is used internally by exitIOThread and should not be used separately
		
		int64_t					mSeekKeyframeTarget = -1;					///< Seek target, in internal stream units (not secs). This value is used iteratively and can be different from the original seek target
		int64_t					mSeekTarget = -1;							///< Seek target as set by the user, in internal steram units (not secs)
		double					mSeekTargetSecs = 0.0f;						///< Seek target as set by the user, in secs.

		VideoService&			mService;									///< Video service that this object is registered with

		SwrContext*				mAudioResampleContext = nullptr;			///< Context used for resampling to the target audio format
		Frame					mCurrentAudioFrame;							///< Audio frame currently being decoded. A single audio frame can cross the boundaries of a single audio callback, so it is cached
		std::vector<uint8_t>	mAudioResampleBuffer;						///< Current resampled audio buffer. Can cross the boundaries of a single audio callback, so it is cached.
		uint8_t*				mCurrentAudioBuffer = nullptr;				///< Pointer to either the buffer in the current audio frame itself, or the resampled audio buffer.
		uint64_t				mAudioFrameReadOffset = 0;					///< Offset (cursor) into the current audio buffer that is decoded
		uint64_t				mAudioFrameSize = 0;						///< Size of the current decoded (and possible resampled) audio buffer, in bytes

		IOThreadState			mIOThreadState = IOThreadState::Playing;		///< FSM state of the I/O thread
	};

	// Object creator used for constructing the the OSC receiver
	using VideoObjectCreator = rtti::ObjectCreator<Video, VideoService>;
}
