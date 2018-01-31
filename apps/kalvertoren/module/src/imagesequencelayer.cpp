#include "imagesequencelayer.h"

// External includes
#include "texture2dfromfile.h"
#include <utility/fileutils.h>
#include "bitmaputils.h"

RTTI_BEGIN_CLASS(nap::ImageSequenceLayer)
	RTTI_PROPERTY("BaseFilename",	&nap::ImageSequenceLayer::mBaseFilename,	nap::rtti::EPropertyMetaData::Required | nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("FPS",			&nap::ImageSequenceLayer::mFPS,				nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ImageSequenceLayerInstance)
	RTTI_CONSTRUCTOR(nap::ImageSequenceLayer&)
RTTI_END_CLASS

namespace nap
{
	bool ImageSequenceLayer::init(utility::ErrorState& errorState)
	{
		int file_index = 0;
		
		while (true)
		{
			const std::string filename = utility::stringFormat(mBaseFilename, file_index++);
			if (!utility::fileExists(filename))
				break;

			std::unique_ptr<Bitmap> bitmap = std::make_unique<Bitmap>();
			if (!errorState.check(bitmap->initFromFile(filename, errorState), "Failed to read image %s in image sequence %s", filename.c_str(), mID.c_str()))
				return false;

			// Get opengl settings from bitmap
			opengl::Texture2DSettings cur_bitmap_settings;
			if (!errorState.check(getTextureSettingsFromBitmap(*bitmap, false,  cur_bitmap_settings, errorState), "Unable to determine texture settings from image %s in sequence %s", filename.c_str(), mID.c_str()))
				return false;

			// Verify the bitmap has the same settings as all other bitmaps
			if (!errorState.check(!mTextureSettings.isValid() || cur_bitmap_settings == mTextureSettings, "Image %s in image sequence %s has different texture settings than previous images; check dimensions and pixel format (RGB/RGBA)", filename.c_str(), mID.c_str()))
				return false;

			mTextureSettings = cur_bitmap_settings;

			mBitmaps.emplace_back(std::move(bitmap));
		}

		if (!errorState.check(!mBitmaps.empty(), "Image sequence %s has no images", mID.c_str()))
			return false;

		return true;
	}


	float ImageSequenceLayer::getLength() const
	{
		return static_cast<float>(getNumBitmaps()) / mFPS;
	}


	std::unique_ptr<LayerInstance> ImageSequenceLayer::createInstance()
	{
		return std::make_unique<ImageSequenceLayerInstance>(*this);
	}

	//////////////////////////////////////////////////////////////////////////

	ImageSequenceLayerInstance::ImageSequenceLayerInstance(ImageSequenceLayer& layer) :
		mLayer(&layer)
	{
		mCurrentFrameTexture = std::make_unique<Texture2D>();
		mCurrentFrameTexture->initTexture(layer.getTextureSettings());
	}

	void ImageSequenceLayerInstance::update(double deltaTime)
	{
		// Upload current texture data to the GPU if necessary
		if (mNextFrameIndex != mCurrentFrameIndex)
		{
			Bitmap& bitmap = mLayer->getBitmap(mNextFrameIndex);
			mCurrentFrameTexture->update(bitmap);
			mCurrentFrameIndex = mNextFrameIndex;
		}

		// Update current frame index
		double frame_time = 1.0 / mLayer->mFPS;
		mCurrentTime += deltaTime;

		// Calculate current / next texture index
		mNextFrameIndex = ((int)(mCurrentTime / frame_time)) % mLayer->getNumBitmaps();
		
		// Signal completion when the next frame we want to upload is the beginning of the sequence
		if (mNextFrameIndex != mCurrentFrameIndex && mCurrentFrameIndex == getNumBitmaps() - 1)
		{
			completed(*this);
		}
	}
}
