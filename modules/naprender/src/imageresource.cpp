// Local Includes
#include "imageresource.h"

// External Includes
#include <nap/logger.h>
#include <nap/fileutils.h>

namespace nap
{
	// Constructor
	ImageResource::ImageResource(const std::string& imgPath)
	{
		mImagePath = imgPath;
		mDisplayName = getFileNameWithoutExtension(imgPath);
		assert(mDisplayName != "");
	}


	// Load image if required and extract texture
	const opengl::BaseTexture& ImageResource::getTexture() const
	{
		return getImage().getTexture();
	}


	const std::string& ImageResource::getDisplayName() const
	{
		return mDisplayName;
	}


	const opengl::Image& ImageResource::getImage() const
	{
		if (!mLoaded)
		{
			if (!mImage.load(mImagePath))
			{
				nap::Logger::warn("unable to load image: %s", mImagePath.c_str());
			}
			mLoaded = true;
		}
		return mImage;
	}
	
	// Resource loader constructor
	ImageResourceLoader::ImageResourceLoader()
	{
		for (const auto& ext : getSupportedImgExtensions())
		{
			addFileExtension(ext);
		}
	}


	// Returns all supported image extensions
	const std::vector<std::string>& ImageResourceLoader::getSupportedImgExtensions()
	{
		static std::vector<std::string> extensions;
		if (extensions.empty())
		{
			extensions = std::vector<std::string>
			{
				"bmp",
				"dds",
				"raw",
				"ico",
				"jpg",
				"jpeg",
				"png",
				"tga",
				"tiff",
				"psd",
				"hdr",
				"exr",
				"gif",
			};
		}
		return extensions;
	}


	// Loads a resource
	std::unique_ptr<Resource> ImageResourceLoader::loadResource(const std::string& resourcePath) const
	{
		return std::make_unique<ImageResource>(resourcePath);
	}


	// Non const getter, following:
	opengl::BaseTexture& TextureResource::getTexture()
	{
		return const_cast<opengl::BaseTexture&>(static_cast<const TextureResource&>(*this).getTexture());
	}
	
}

RTTI_DEFINE(nap::TextureResource)
RTTI_DEFINE(nap::ImageResource)
RTTI_DEFINE(nap::ImageResourceLoader)
