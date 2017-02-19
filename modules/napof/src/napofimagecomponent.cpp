#include <nap/logger.h>
#include "napofimagecomponent.h"

namespace nap
{
	std::unordered_map<std::string, std::shared_ptr<ofImage>> OFImageComponent::mImageCache;

	/**
	@brief Constructor -> connect
	**/
	OFImageComponent::OFImageComponent() 
	{ 
		mFile.valueChanged.connect(mFileChanged); 
	}


	/**
	@brief Destructor
	**/
	OFImageComponent::~OFImageComponent() {}


	/**
	@brief Loads the image
	**/
	void OFImageComponent::loadImage()
	{
		// Guard against an invalid filename
		if (mFile.getValue().empty()) return;

		if (!ofFile(mFile.getValue()).exists()) {
			Logger::warn(this->getName() + ": file does not exist: " + mFile.getValue());
			return;
		}
		Logger::debug("Loading: %s", mFile.getValue().c_str());

		if (useImageCache.getValueRef()) { // Use caching

			// Find existing image from cache
			auto it = mImageCache.find(mFile.getValueRef());
			if (it != mImageCache.end()) {
				mImage = it->second;
			} else {
				// Existing image not found in cache
				std::shared_ptr<ofImage> image = std::make_shared<ofImage>();

				if (!image->load(mFile.getValueRef())) {
					Logger::warn(this->getName() + ": unable to load image: " + mFile.getValue());
					return;
				}

				image->getTexture().enableMipmap();
				image->getTexture().generateMipmap();

				// Store locally accessible pointer to image
				mImage = image;
				// Store pointer copy in cache to let subsequent calls reuse the image
				mImageCache.insert({mFile.getValueRef(), std::move(image)});
			}

		} else {
			// Use no caching, just load it in the implicitly allocated ofImage

			ofImage& image = getImage();
			// Clear existing entry
			if (image.isAllocated()) image.clear();

			// Load in memory
			if (!image.load(mFile.getValue())) {
				Logger::fatal(this->getName() + ": unable to load image: " + mFile.getValue());
				return;
			}

			image.getTexture().enableMipmap();
		}
		texture.setValue(&getImage().getTexture());
		imageChanged.trigger(getImage());
	}

	ofImage& OFImageComponent::getImage() 
	{
		// Ensure we're returning a valid ofImage, this instance will only be used when caching is disabled
		if (mImage == nullptr) mImage = std::make_shared<ofImage>();
		return *mImage.get();
	}
}

RTTI_DEFINE(nap::OFImageComponent)
