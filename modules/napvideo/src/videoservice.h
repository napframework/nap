#pragma once

// Nap Includes
#include <nap/service.h>

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

	class VideoResource
	{
	public:
		bool init(nap::utility::ErrorState& errorState);

		void update(double deltaTime);
		void play() { mPlaying = true; }

	public:
		std::string mPath;

		// Runtime data
		AVCodec*				mCodec = nullptr;
		AVCodecContext*			mContext = nullptr;
		AVFormatContext*		mFormatContext = nullptr;
		AVFrame*				mFrame = nullptr;
		int						mVideoStream = -1;
		bool					mPlaying = false;
		int						mWidth = 0;
		int						mHeight = 0;

		MemoryTextureResource2D& getYTexture() { return *mYTexture; }
		MemoryTextureResource2D& getUTexture() { return *mUTexture; }
		MemoryTextureResource2D& getVTexture() { return *mVTexture; }

	private:
		std::unique_ptr<MemoryTextureResource2D> mYTexture;
		std::unique_ptr<MemoryTextureResource2D> mUTexture;
		std::unique_ptr<MemoryTextureResource2D> mVTexture;
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