#include "imagesequencelayer.h"

// External includes
#include <ntextureutils.h>
#include "image.h"

RTTI_BEGIN_CLASS(nap::ImageSequenceLayer)
	RTTI_PROPERTY("Images", &nap::ImageSequenceLayer::mImages, nap::rtti::EPropertyMetaData::Required | nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("FPS", &nap::ImageSequenceLayer::mFPS, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

namespace nap
{
	ImageSequenceLayerInstance::ImageSequenceLayerInstance(ImageSequenceLayer& layer) :
		mLayer(&layer)
	{
	}


	bool ImageSequenceLayer::init(utility::ErrorState& errorState)
	{
		if (!errorState.check(!mImages.empty(), "Image sequence %s has no images", mID))
			return false;

		return true;
	}


	std::unique_ptr<LayerInstance> ImageSequenceLayer::createInstance()
	{
		return std::make_unique<ImageSequenceLayerInstance>(*this);
	}


	void ImageSequenceLayerInstance::update(double deltaTime)
	{
		double frame_time = 1.0 / mLayer->mFPS;
		mCurrentTime += deltaTime;

		mCurrentIndex = ((int)(mCurrentTime / frame_time)) % mLayer->mImages.size();
	}
}