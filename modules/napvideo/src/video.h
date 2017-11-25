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

namespace opengl
{
	class Texture2D;
}

namespace nap
{
	class Texture2D;
	class VideoService;

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

		/**
		* @return The time stamp of the current video frame in seconds
		*/
		double getTimeStamp() const				{ return mPrevPTSSecs; }
			
		std::string mPath;				///< Path to the video to playback
		bool		mLoop = false;		///< If the video needs to loop
		float		mSpeed = 1.0f;		///< Video playback speed

	private:

		/**
		 * Thread that grabs packets as they are queued by the ioThread, decodes them and pushes them into the frame queue.
		 */
		void decodeThread();

		/**
		 * Calling this ensures that the thread is waked properly if in a wait state, and then exited. This function only
		 * causes the function to exit, it does not join the thread, so it is non-blocking.
		 */
		void exitDecodeThread();

		/**
		 * Thread reads packets from the stream and pushes them onto the packet queue.
		 */
		void ioThread();

		/**
		 * Calling this ensures that the thread is waked properly if in a wait state, and then exited. This function only
		 * causes the function to exit, it does not join the thread, so it is non-blocking.
		 */
		void exitIOThread();

		bool decodeFrame(AVFrame& frame);
		void clearPacketQueue();
		void clearFrameQueue();

	private:
		static const double		sVideoMax;

		std::unique_ptr<Texture2D> mYTexture;
		std::unique_ptr<Texture2D> mUTexture;
		std::unique_ptr<Texture2D> mVTexture;

		AVCodec*				mCodec = nullptr;
		AVCodecContext*			mCodecContext = nullptr;
		AVFormatContext*		mFormatContext = nullptr;
		int						mVideoStream = -1;				///< Specifies what stream in the file is the one containing video packets
		bool					mPlaying = false;				///< Set if playing. Should only be controlled from main thread
		int						mWidth = 0;						///< Width of the video, in pixels
		int						mHeight = 0;					///< Height of the video, in pixels
		float					mDuration = 0.0f;				///< Duration of the video in seconds
		double					mPrevPTSSecs = 0.0;				///< Stored timing information for the previous frame
		double					mVideoClockSecs = sVideoMax;	///< Clock that we use to synchronize the video to
		std::string				mErrorString;					///< If an error occurs, this is the string containing error information. If empty, no error occured.

		/**
		 * Frame as pushed in the frame queue.
		 */
		struct Frame
		{
			AVFrame*	mFrame;			///< Frame as decoded by the decode thread
			double		mPTSSecs;		///< When the frame needs to be displayed (absolute clock time)
		};

		// Producer/consumer variables for frame queue:
		std::queue<Frame>		mFrameQueue;						///< The frame queue as produced by the decodeThread and consumed by the main thread
		std::thread				mDecodeThread;						///< Decode thread
		std::mutex				mFrameQueueMutex;					///< Mutex protection for the frame queue
		std::condition_variable mFrameDataAvailableCondition;		///< Condition describing whether there is data in the frame queue to process
		std::condition_variable mFrameQueueRoomAvailableCondition;	///< Condition describing whether there is still room in the frame queue to add new frames

		// Producer/consumer variables for packet queue:
		std::queue<AVPacket*>	mPacketQueue;						///< The packet queue as produced by the ioThread and consumed by the decodeThread thread
		std::thread				mIOThread;							///< IO thread, reads packets
		std::mutex				mPacketQueueMutex;					///< Mutex protection for the packet queue
		std::condition_variable mPacketAvailableCondition;			///< Condition describing whether there is data in the packet queue to process
		std::condition_variable mPacketQueueRoomAvailableCondition;	///< Condition describing whether there is still room in the packet queue to add new packets

		bool					mExitIOThreadSignalled = false;		///< If this boolean is set, the IO thread will exit ASAP. This is used internally by exitIOThread and should not be used separately
		bool					mExitDecodeThreadSignalled = false;	///< If this boolean is set, the decode thread will exit ASAP. This is used internally by exitDecodeThread and should not be used separately
		bool					mPacketsFinished = false;			///< If this boolean is set, the IO thread has no more packets to read, either due to an error or due to an end of stream.
		bool					mFramesFinished = false;			///< If this boolean is set, the decode thread has no more frames to decode, either due to an error or due to an end of stream.

		int64_t					mSeekTarget = -1;					///< Seek target, in internal stream units (not secs)

		VideoService&			mService;							///< Video service that this object is registered with
	};

	// Object creator used for constructing the the OSC receiver
	using VideoObjectCreator = rtti::ObjectCreator<Video, VideoService>;
}
