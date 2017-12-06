#include "imagesequencelayer.h"

// External includes
#include <ntextureutils.h>
#include "image.h"
#include <utility/fileutils.h>

RTTI_BEGIN_CLASS(nap::ImageSequenceLayer)
	RTTI_PROPERTY("BaseFilename",	&nap::ImageSequenceLayer::mBaseFilename,	nap::rtti::EPropertyMetaData::Required | nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("FPS",			&nap::ImageSequenceLayer::mFPS,				nap::rtti::EPropertyMetaData::Required)
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
		double frame_time = 1.0 / mLayer->mFPS;
		mCurrentTime += deltaTime;

		// When the frame changes, update the GPU texture
		int new_frame_index = ((int)(mCurrentTime / frame_time)) % mLayer->getNumPixmaps();
		if (new_frame_index != mCurrentFrameIndex)
		{
			Pixmap& pixmap = mLayer->getPixmap(new_frame_index);
			mCurrentFrameTexture->getTexture().setData(pixmap.getBitmap().getData());
		}

		mCurrentFrameIndex = new_frame_index;
	}
}