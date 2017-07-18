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
	class MemoryTextureResource2D;

	class VideoResource : public rtti::RTTIObject
	{
		RTTI_ENABLE(rtti::RTTIObject)

	public:
		virtual bool init(nap::utility::ErrorState& errorState) override;

		void update(double deltaTime);
		void play();

	private:
		void decodeThread();
		void ioThread();

	public:
		std::string mPath;

		MemoryTextureResource2D& getYTexture() { return *mYTexture; }
		MemoryTextureResource2D& getUTexture() { return *mUTexture; }
		MemoryTextureResource2D& getVTexture() { return *mVTexture; }

		int getWidth() const { return mWidth; }
		int getHeight() const { return mHeight; }

	private:
		std::unique_ptr<MemoryTextureResource2D> mYTexture;
		std::unique_ptr<MemoryTextureResource2D> mUTexture;
		std::unique_ptr<MemoryTextureResource2D> mVTexture;

		// Runtime data
		AVCodec*				mCodec = nullptr;
		AVCodecContext*			mContext = nullptr;
		AVFormatContext*		mFormatContext = nullptr;
		AVFrame*				mFrame = nullptr;
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