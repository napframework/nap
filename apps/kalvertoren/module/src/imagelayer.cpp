#include "imagelayer.h"

// External includes
#include "image.h"
#include "bitmaputils.h"

RTTI_BEGIN_CLASS(nap::ImageLayer)
	RTTI_PROPERTY("Path", &nap::ImageLayer::mImagePath, nap::rtti::EPropertyMetaData::Required | nap::rtti::EPropertyMetaData::FileLink)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ImageLayerInstance)
	RTTI_CONSTRUCTOR(nap::ImageLayer&)
RTTI_END_CLASS

namespace nap
{
	bool ImageLayer::init(utility::ErrorState& errorState)
	{
		if (!errorState.check(!mImagePath.empty(), "Image path not set for color palette %s", mID.c_str()))
			return false;

		// Load pixel data in to bitmap
		if (!mPixmap.initFromFile(mImagePath, errorState))
			return false;

		// Make sure the amount of channels is > 3
		if (!errorState.check(mPixmap.getNumberOfChannels() >= 3, "color palette map: %s does not have 3 channels", mImagePath.c_str()))
			return false;

		// Force parameters
		nap::TextureParameters parameters;
		parameters.mMaxFilter = EFilterMode::Nearest;
		parameters.mMinFilter = EFilterMode::Nearest;
		parameters.mMaxLodLevel = 0;
		parameters.mWrapHorizontal = EWrapMode::MirroredRepeat;
		parameters.mWrapVertical = EWrapMode::MirroredRepeat;
		mTexture.mParameters = parameters;
		mTexture.mUsage = opengl::ETextureUsage::Static;

		// Get opengl settings from bitmap
		opengl::Texture2DSettings settings;
		if (!errorState.check(getSettingsFromBitmap(mPixmap, false, settings, errorState), "Unable to determine texture settings from bitmap %s", mImagePath.c_str()))
			return false;

		// Initialize with settings
		mTexture.init(settings);

		// Set data
		mTexture.setData(mPixmap);

		return true;
	}

	std::unique_ptr<LayerInstance> ImageLayer::createInstance()
	{
		return std::make_unique<ImageLayerInstance>(*this);
	}
}