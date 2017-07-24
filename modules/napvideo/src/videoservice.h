#pragma once

// Nap Includes
#include <nap/service.h>
#include <queue>

namespace opengl
{
	class Texture2D;
}

struct AVPacket;
struct AVCodec;
struct AVCodecParserContext;
struct AVCodecContext;
struct AVFormatContext;
struct AVFrame;

namespace nap
{
	class MemoryTexture2D;

	class VideoResource : public rtti::RTTIObject
	{
		RTTI_ENABLE(rtti::RTTIObject)

	public:
		~VideoResource();
		virtual bool init(utility::ErrorState& errorState) override;

		bool update(double deltaTime, utility::ErrorState& errorState);
		void play();
		void stop();
	
		MemoryTexture2D& getYTexture() { return *mYTexture; }
		MemoryTexture2D& getUTexture() { return *mUTexture; }
		MemoryTexture2D& getVTexture() { return *mVTexture; }

		int getWidth() const { return mWidth; }
		int getHeight() const { return mHeight; }

		std::string mPath;

	private:
		void decodeThread();
		void exitDecodeThread();
		void ioThread();
		void exitIOThread();
		bool decodeFrame(AVFrame& frame);

	private:
		std::unique_ptr<MemoryTexture2D> mYTexture;
		std::unique_ptr<MemoryTexture2D> mUTexture;
		std::unique_ptr<MemoryTexture2D> mVTexture;

		AVCodec*				mCodec = nullptr;
		AVCodecContext*			mCodecContext = nullptr;
		AVFormatContext*		mFormatContext = nullptr;
		int						mVideoStream = -1;
		bool					mPlaying = false;
		int						mWidth = 0;
		int						mHeight = 0;
		double					mPrevPTSSecs = 0.0;
		double					mVideoClockSecs = DBL_MAX;

		std::string				mErrorString;

		struct Frame
		{
			AVFrame*	mFrame;
			double		mPTSSecs;
		};
		std::queue<Frame>		mFrameQueue;
		std::thread				mDecodeThread;
		std::mutex				mFrameQueueMutex;
		std::condition_variable mFrameDataAvailableCondition;
		std::condition_variable mFrameQueueRoomAvailableCondition;

		std::thread				mIOThread;
		std::mutex				mPacketQueueMutex;
		std::condition_variable mPacketAvailableCondition;
		std::condition_variable mPacketQueueRoomAvailableCondition;
		std::queue<AVPacket*>	mPacketQueue;
		bool					mExitIOThreadSignalled = false;
		bool					mExitDecodeThreadSignalled = false;
		bool					mPacketsFinished = false;
		bool					mFramesFinished = false;
	};

	/**
	 * 
	 */
	class VideoService : public Service
	{
		RTTI_ENABLE(Service)

	public:
		// Default constructor
		VideoService() = default;

		// Disable copy
		VideoService(const VideoService& that) = delete;
		VideoService& operator=(const VideoService&) = delete;
	
		bool init(nap::utility::ErrorState& errorState);
	};
}