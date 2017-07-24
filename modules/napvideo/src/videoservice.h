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
		virtual bool init(nap::utility::ErrorState& errorState) override;

		void update(double deltaTime);
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
		void ioThread();

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
		bool					mExitThreadSignalled = false;
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