#pragma once

#include <condition_variable>
#include <queue>
#include <thread>
#include <mutex>
#include <limits>
#include <rtti/rttiobject.h>
#include <rtti/factory.h>

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
	class Texture2D;
	class VideoService;
	class Video;

	struct AudioParams
	{
		int freq;
		int channels;
		int64_t channel_layout;
		int fmt;
		int frame_size;
		int bytes_per_sec;
	};

	/**
	* Frame as pushed in the frame queue.
	*/
	struct Frame
	{
		bool isValid() const { return mFrame != nullptr; }
		AVFrame*	mFrame = nullptr;	///< Frame as decoded by the decode thread
		double		mPTSSecs;			///< When the frame needs to be displayed (absolute clock time)
	};

	class AVState final
	{
	public:
		using DecodeFunction = std::function<int(AVCodecContext* /*avctx*/, AVFrame* /*frame*/, int* /*got_frame_ptr*/, const AVPacket* /*avpkt*/)>;

		AVState(Video& video) :
			mVideo(&video)
		{
		}

		~AVState();

		void close();

		bool isValid() const { return mStream != -1; }
		int getStream() const { return mStream; }
		void setStream(int stream) { mStream = stream; }

		void setCodec(AVCodec* codec, AVCodecContext* codecContext);

		void startDecodeThread(const DecodeFunction& decodeFunction);
		void exitDecodeThread(bool join);
		void decodeThread();
		bool decodeFrame(AVFrame& frame);
		bool isFinishedProducing() const { return mFinishedProducingFrames; }
		bool isFinishedConsuming() const;
		bool isFinished() const { return isFinishedProducing() && isFinishedConsuming(); }

		void notifyFinishedProducingPackets();
		void clearPacketQueue();
		void clearFrameQueue();

		bool matchesStream(const AVPacket& packet) const;
		bool addPacket(AVPacket* packet, const bool& exitIOThreadSignalled);

		Frame popFrame();
		Frame tryPopFrame(double pts);
		Frame peekFrame();

		void notifyPacketQueueRoomAvailable();

		AVCodec& getCodec() { return *mCodec; }
		AVCodecContext& getCodecContext() { return *mCodecContext; }

	private:
		Video*					mVideo;
		AVCodec*				mCodec = nullptr;
		AVCodecContext*			mCodecContext = nullptr;

		int						mStream = -1;							///< Specifies what stream in the file is the one containing video packets

		std::thread				mDecodeThread;							///< Video decode thread
		DecodeFunction			mDecodeFunction;
		bool					mExitDecodeThreadSignalled = false;		///< If this boolean is set, the decode thread will exit ASAP. This is used internally by exitDecodeThread and should not be used separately
		double					mPrevPTSSecs = 0.0;				///< Stored timing information for the previous frame

																// Producer/consumer variables for frame queue:
		std::queue<Frame>		mFrameQueue;							///< The frame queue as produced by the decodeThread and consumed by the main thread
		mutable std::mutex		mFrameQueueMutex;						///< Mutex protection for the frame queue
		std::condition_variable mFrameDataAvailableCondition;			///< Condition describing whether there is data in the frame queue to process
		std::condition_variable mFrameQueueRoomAvailableCondition;		///< Condition describing whether there is still room in the frame queue to add new frames
																		// Producer/consumer variables for packet queue:
		std::queue<AVPacket*>	mPacketQueue;							///< The packet queue as produced by the ioThread and consumed by the decodeThread thread
		std::mutex				mPacketQueueMutex;						///< Mutex protection for the packet queue
		std::condition_variable mPacketAvailableCondition;				///< Condition describing whether there is data in the packet queue to process
		std::condition_variable mPacketQueueRoomAvailableCondition;		///< Condition describing whether there is still room in the packet queue to add new packets

		bool					mIOFinishedProducingPackets = false;	///< IO thread has finished producing packets
		bool					mFinishedProducingFrames = false;		///< If this boolean is set, the decode thread has no more frames to decode, either due to an error or due to an end of stream.
	};


	/**
	 * Video playback.
	 * Internally contains textures that have the contents for each frame. After calling update(), the texture is filled with
	 * the latest frame. update() is not blocking, internally the textures will only be updated when needed. The textures that
	 * are output are in the YUV format. Conversion to RGB can be done in a shader.
	 * 
	 * Internally, the producer/consumer pattern is used for the reading of packets and the decoding of packets:
	 * The ioThread (the producer) will read packets from the stream and push them onto a frame queue. The frame queue is consumed by the decode
	 * thread (which is both a consumer (=> packets) and a producer (=> frames)). The frame thread decodes the frame (which internally will spawn more 
	 * thread to decode the package) and pushes the frame onto the frame queue. 
	 * The main thread will consume the frames when they are present in the frame queue and their timestamp has 'passed'.
	 */
	class NAPAPI Video final : public rtti::RTTIObject
	{
		RTTI_ENABLE(rtti::RTTIObject)

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
		void stop();

		/**
		 * Seeks within the video to the time provided. This can be called while playing.
		 * @param seconds: the time offset in seconds in the videp.
		 */
		void seek(double seconds);

		/**
		 * @return The current playback position, in seconds.
		 */
		double getCurrentTime() const			{ return mVideoClockSecs; }

		/**
		 * @return The Y texture as it is updated by update(). Initially, the texture is not initialized
		 * to zero, but to the 'black' equivalent in YUV space. The size of the Y texture is width * height.
		 */
		Texture2D& getYTexture()			{ return *mYTexture; }

		/**
		 * @return The U texture as it is updated by update(). Initially, the texture is not initialized
		 * to zero, but to the 'black' equivalent in YUV space. The size of the Y texture is HALF the width * height.
		 */
		Texture2D& getUTexture()			{ return *mUTexture; }

		/**
		 * @return The V texture as it is updated by update(). Initially, the texture is not initialized
		 * to zero, but to the 'black' equivalent in YUV space. The size of the V texture is HALF the width * height.
		 */
		Texture2D& getVTexture()			{ return *mVTexture; }

		/**
		 * @return Width of the video, in pixels.
		 */
		int getWidth() const					{ return mWidth; }

		/**
		 * @return Height of the video, in pixels.
		 */
		int getHeight() const					{ return mHeight; }

		/**
		* @return The duration of the video in seconds.
		*/
		double getDuration() const				{ return mDuration; }

		void OnAudioCallback(uint8_t* stream, int len, const AudioParams& audioHwParams);

		std::string mPath;				///< Path to the video to playback
		bool		mLoop = false;		///< If the video needs to loop
		float		mSpeed = 1.0f;		///< Video playback speed

	private:
		bool SDLInit(nap::utility::ErrorState& errorState);

		static int audio_open(void *opaque, int64_t wanted_channel_layout, int wanted_nb_channels, int wanted_sample_rate, AudioParams& audio_hw_params);
		static bool sInitCodec(AVState& destState, const AVCodecContext& sourceCodecContext, AVDictionary*& options, utility::ErrorState& errorState);

		void onFinishedProducingPackets();

		/**
		 * Thread that grabs packets as they are queued by the ioThread, decodes them and pushes them into the frame queue.
		 */
		void decodeVideoThread();

		/**
		* Thread that grabs packets as they are queued by the ioThread, decodes them and pushes them into the frame queue.
		*/
		void decodeAudioThread();

		/**
		 * Thread reads packets from the stream and pushes them onto the packet queue.
		 */
		void ioThread();

		void startIOThread();

		/**
		 * Calling this ensures that the thread is waked properly if in a wait state, and then exited. This function only
		 * causes the function to exit, it does not join the thread, so it is non-blocking.
		 */
		void exitIOThread(bool join);

		bool decodeVideoFrame(AVFrame& frame);
		bool decodeAudioFrame(AVFrame& frame);
		void getNextAudioFrame(const AudioParams& audioHwParams);

		void clearPacketQueue();
		void clearVideoFrameQueue();
		void clearAudioFrameQueue();

		friend class AVState;
		bool hasErrorOccurred() const;
		void setErrorOccurred(const std::string& errorMessage);


	private:
		static const double		sVideoMax;

		std::unique_ptr<Texture2D> mYTexture;
		std::unique_ptr<Texture2D> mUTexture;
		std::unique_ptr<Texture2D> mVTexture;

		AVFormatContext*		mFormatContext = nullptr;
		bool					mPlaying = false;				///< Set if playing. Should only be controlled from main thread
		int						mWidth = 0;						///< Width of the video, in pixels
		int						mHeight = 0;					///< Height of the video, in pixels
		float					mDuration = 0.0f;				///< Duration of the video in seconds
		double					mVideoClockSecs = sVideoMax;	///< Clock that we use to synchronize the video to
		std::string				mErrorMessage;					///< If an error occurs, this is the string containing error information. If empty, no error occured.
		bool					mErrorOccurred = false;
		
		AVState					mVideoState;
		AVState					mAudioState;
		std::thread				mIOThread;									///< IO thread, reads packets

		bool					mExitIOThreadSignalled = false;				///< If this boolean is set, the IO thread will exit ASAP. This is used internally by exitIOThread and should not be used separately
		
		int64_t					mSeekTarget = -1;							///< Seek target, in internal stream units (not secs)

		VideoService&			mService;									///< Video service that this object is registered with

		SwrContext*				mAudioResampleContext = nullptr;
		Frame					mCurrentAudioFrame;
		uint8_t*				mCurrentAudioBuffer = nullptr;
		std::vector<uint8_t>	mAudioResampleBuffer;
		uint64_t				mAudioFrameReadPtr = 0;
		uint64_t				mAudioFrameSize = 0;
	};

	// Object creator used for constructing the the OSC receiver
	using VideoObjectCreator = rtti::ObjectCreator<Video, VideoService>;
}
