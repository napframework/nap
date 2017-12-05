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
	ImageSequenceLayerInstance::ImageSequenceLayerInstance(ImageSequenceLayer& layer) :
		mLayer(&layer)
	{
	}


	bool ImageSequenceLayer::init(utility::ErrorState& errorState)
	{
		int file_index = 0;
		
		while (true)
		{
			const std::string filename = utility::stringFormat(mBaseFilename, file_index++);
			if (!utility::fileExists(filename))
				break;

			std::unique_ptr<Image> image = std::make_unique<Image>(filename);
			if (!errorState.check(image->init(errorState), "Failed to read image %s for image sequence %s", filename.c_str(), mID.c_str()))
				return false;

			mImages.emplace_back(std::move(image));
		}

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

		mCurrentIndex = ((int)(mCurrentTime / frame_time)) % mLayer->getNumImages();
	}
}