#include "imagesequencelayer.h"

// External includes
#include <ntextureutils.h>
#include "image.h"
#include <utility/fileutils.h>

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
			const std::string filename = utility::stringFormat(mBaseFilename, file_index);
			if (!utility::fileExists(filename))
			{
				if (!errorState.check(file_index > 0, "first image in sequence: %s does not exist: %s", this->mID.c_str(), filename.c_str()))
					return false;
				break;
			}
			file_index++;

			std::unique_ptr<Pixmap> pixmap = std::make_unique<Pixmap>();
			if (!errorState.check(pixmap->initFromFile(filename, errorState), "Failed to read image %s in image sequence %s", filename.c_str(), mID.c_str()))
				return false;

			// Get opengl settings from pixmap
			opengl::Texture2DSettings cur_pixmap_settings;
			if (!errorState.check(opengl::getSettingsFromBitmap(pixmap->getBitmap(), false,  cur_pixmap_settings, errorState), "Unable to determine texture settings from image %s in sequence %s", filename.c_str(), mID.c_str()))
				return false;

			// Verify the pixmap has the same settings as all other pixmaps
			if (!errorState.check(!mTextureSettings.isValid() || cur_pixmap_settings == mTextureSettings, "Image %s in image sequence %s has different texture settings than previous images; check dimensions and pixel format (RGB/RGBA)", filename.c_str(), mID.c_str()))
				return false;

			mTextureSettings = cur_pixmap_settings;

			mPixmaps.emplace_back(std::move(pixmap));
		}

		if (!errorState.check(!mPixmaps.empty(), "Image sequence %s has no images", mID.c_str()))
			return false;

		return true;
	}


	float ImageSequenceLayer::getLength() const
	{
		return static_cast<float>(getNumPixmaps()) / mFPS;
	}


	std::unique_ptr<LayerInstance> ImageSequenceLayer::createInstance()
	{
		return std::make_unique<ImageSequenceLayerInstance>(*this);
	}

	//////////////////////////////////////////////////////////////////////////

	ImageSequenceLayerInstance::ImageSequenceLayerInstance(ImageSequenceLayer& layer) :
		mLayer(&layer)
	{
		mCurrentFrameTexture = std::make_unique<BaseTexture2D>();
		mCurrentFrameTexture->init(layer.getTextureSettings());
	}

	void ImageSequenceLayerInstance::update(double deltaTime)
	{
		// Upload current texture data to the GPU if necessary
		if (mNextFrameIndex != mCurrentFrameIndex)
		{
			Pixmap& pixmap = mLayer->getPixmap(mNextFrameIndex);
			mCurrentFrameTexture->getTexture().setData(pixmap.getBitmap().getData());
			mCurrentFrameIndex = mNextFrameIndex;
		}

		// Update current frame index
		double frame_time = 1.0 / mLayer->mFPS;
		mCurrentTime += deltaTime;

		// Calculate current / next texture index
		mNextFrameIndex = ((int)(mCurrentTime / frame_time)) % mLayer->getNumPixmaps();
		
		// Signal completion when the next frame we want to upload is the beginning of the sequence
		if (mNextFrameIndex != mCurrentFrameIndex && mCurrentFrameIndex == getNumPixmaps() - 1)
		{
			completed(*this);
		}
	}
}
