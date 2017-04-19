// Local Includes
#include "imageresource.h"

// External Includes
#include <nap/logger.h>
#include <nap/fileutils.h>

namespace nap
{
	// Initializes 2D texture. Additionally a custom display name can be provided.
	bool MemoryTextureResource2D::init(InitResult& initResult)
	{
		mTexture.init();

		opengl::Texture2DSettings settings;
		settings.level = mLevel.getValue();
		settings.internalFormat = mInternalFormat.getValue();
		settings.width = mWidth.getValue();
		settings.height = mHeight.getValue();
		settings.format = mFormat.getValue();
		settings.type = mType.getValue();
		mTexture.allocate(settings);

		return true;
	}

	// Returns 2D texture object
	const opengl::BaseTexture& MemoryTextureResource2D::getTexture() const
	{
		return mTexture;
	}

	// Constructor
	ImageResource::ImageResource(const std::string& imgPath)
	{
		//mImagePath = imgPath;
		mDisplayName = getFileNameWithoutExtension(imgPath);
		assert(mDisplayName != "");
	}


	// Load image if required and extract texture
	const opengl::BaseTexture& ImageResource::getTexture() const
	{
		return getImage().getTexture();
	}


	const std::string ImageResource::getDisplayName() const
	{
		return mDisplayName;
	}

	bool ImageResource::init(InitResult& initResult)
	{
		if (!initResult.check(!mImagePath.getValue().empty(), "Imagepath not set"))
			return false;

		if (!initResult.check(mImage.load(mImagePath), "Unable to load image from file"))
			return false;

		return true;
	}


	const opengl::Image& ImageResource::getImage() const
	{
		return mImage;
	}
	
	// Non const getter, following:
	opengl::BaseTexture& TextureResource::getTexture()
	{
		return const_cast<opengl::BaseTexture&>(static_cast<const TextureResource&>(*this).getTexture());
	}
	
}

RTTI_DEFINE_BASE(nap::TextureResource)
RTTI_DEFINE(nap::MemoryTextureResource2D)
RTTI_DEFINE(nap::ImageResource)
