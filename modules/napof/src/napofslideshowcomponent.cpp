// Local Includes
#include "napofslideshowcomponent.h"
#include "napofattributes.h"

// External Includes
#include <utils/nofUtils.h>

RTTI_DEFINE(nap::OFSlideshowComponent)

int posmod(int a, int b) { return (a%b+b)%b; }

namespace nap
{
    std::unordered_map<std::string, std::shared_ptr<ofImage>> OFSlideshowComponent::mImageCache;

	OFSlideshowComponent::~OFSlideshowComponent()
	{
		for (auto& image : mImages)
			image->clear();
	}


	int OFSlideshowComponent::getLeftImageIndex()
	{
		return posmod((int) floor(-mScrollPosition - 0.000001), getImageCount());
	}

	int OFSlideshowComponent::getRightImageIndex()
	{
		return posmod((int) floor(-mScrollPosition + 1 - .000001), getImageCount());
	}

	int OFSlideshowComponent::getCenterImageIndex() {
        return posmod((int) round(-mScrollPosition), getImageCount());
	}


	void OFSlideshowComponent::previous()
	{
        setTargetIndex(mTargetIndex-1);
	}


	void OFSlideshowComponent::next()
	{
        setTargetIndex(mTargetIndex+1);
	}

    void OFSlideshowComponent::setTargetIndex(int index) {
        mTargetIndex = index;
        mScrollTarget = -mTargetIndex;

		// emit signal
		slideChanged(index);
    }

	void OFSlideshowComponent::onUpdate()
	{
		if (getImageCount() == 0)
			return;
		
		double deltaTime = ofGetLastFrameTime();
		
		if (abs(mScrollTarget-mScrollPosition) > 0.0001)
			mScrollPosition = gSmoothDamp(mScrollPosition, mScrollTarget, mScrollVelocity,
				mScrollSmoothTime, mScrollMaxVelocity, deltaTime);

		// Update scroll position
		intersection.setValue(gWrap<float>(mScrollPosition, 0, 1));

		// Set proper images
// 
// 		Logger::info("Intersection: %s", to_string(intersection.getValue()).c_str());
// 		Logger::info(" Left: %s", to_string(getLeftImageIndex()).c_str());
// 		Logger::info("Right: %s", to_string(getRightImageIndex()).c_str());

		leftImageIndex.setValue(getLeftImageIndex());
		rightImageIndex.setValue(getRightImageIndex());

	}


	void OFSlideshowComponent::imageFilenamesChanged(const StringArray& filenames)
	{
		// clear all images and empty the array
		for (auto& image : mImages)
			image->clear();
		mImages.clear();

		for (auto& filename : filenames)
		{
			// Make sure the file exists and isn't empty
			if (filename.empty())
				return;

			auto it = mImageCache.find(filename);
			if (it != mImageCache.end())
			{
				// Image found in cache
				mImages.emplace_back(it->second);
			}
			else
			{
				// Load image and place in cache
				if (!ofFile(filename).exists())
				{
					Logger::fatal(this->getName() + ": file does not exist: " + filename);
					return;
				}

				Logger::debug("Loading: %s", filename.c_str());
				// Load in memory
				std::shared_ptr<ofImage> image = std::make_unique<ofImage>();

				if (!image->load(filename))
				{
					Logger::fatal(this->getName() + ": unable to load image: " + filename);
					continue;
				}

				mImageCache.insert({filename, image});
				mImages.emplace_back(image);
			}

		}

		leftImageIndex.setValue(0);
		rightImageIndex.setValue(0);

    }


	void OFSlideshowComponent::leftImageIndexChanged(const int& index)
	{
		if (index < 0 || index >= mImages.size())
		{
			Logger::warn(getName() + " image index out of bounds");
			leftTexture.setValue(nullptr);
			return;
		}

		auto newTex = &mImages[index]->getTexture();
		if (newTex == leftTexture.getValue())
			return;

		leftTexture.setValue(newTex);
	}


	void OFSlideshowComponent::rightImageIndexChanged(const int& index)
	{
		if (index < 0 || index >= mImages.size())
		{
			Logger::warn(getName() + " image index out of bounds");
			rightTexture.setValue(nullptr);
			return;
		}

		auto newTex = &mImages[index]->getTexture();
		if (newTex == rightTexture.getValue())
			return;

		rightTexture.setValue(newTex);
	}

    void OFSlideshowComponent::scroll(float distance) {
        // Scroll position is 'reverse'
        mScrollPosition -= distance;
    }


}