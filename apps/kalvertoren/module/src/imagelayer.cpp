#include "imagelayer.h"

// External includes
#include "texture2dfromfile.h"
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
		mImage.mImagePath = mImagePath;

		if (!mImage.init(errorState))
			return false;

		// Make sure the amount of channels is > 3
		if (!errorState.check(mImage.getPixmap().getNumberOfChannels() >= 3, "color palette map: %s does not have 3 channels", mImagePath.c_str()))
			return false;

		return true;
	}

	std::unique_ptr<LayerInstance> ImageLayer::createInstance()
	{
		return std::make_unique<ImageLayerInstance>(*this);
	}
}