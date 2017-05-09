// Local Includes
#include "imageresource.h"

// External Includes
#include <nap/logger.h>
#include <nap/fileutils.h>

RTTI_DEFINE_BASE(nap::TextureResource)

RTTI_BEGIN_CLASS(nap::ImageResource)
	RTTI_PROPERTY_FILE_LINK("mImagePath", &nap::ImageResource::mImagePath)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(opengl::Texture2DSettings)
	RTTI_PROPERTY("mLevel",				&opengl::Texture2DSettings::level)
	RTTI_PROPERTY("mInternalFormat",	&opengl::Texture2DSettings::internalFormat)
	RTTI_PROPERTY("mWidth",				&opengl::Texture2DSettings::width)
	RTTI_PROPERTY("mHeight",			&opengl::Texture2DSettings::height)
	RTTI_PROPERTY("mFormat",			&opengl::Texture2DSettings::format)
	RTTI_PROPERTY("mType",				&opengl::Texture2DSettings::type)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::MemoryTextureResource2D)
	RTTI_PROPERTY_REQUIRED("mSettings",			&nap::MemoryTextureResource2D::mSettings)
RTTI_END_CLASS

RTTI_DEFINE(nap::ImageResource)

namespace nap
{
	// Initializes 2D texture. Additionally a custom display name can be provided.
	bool MemoryTextureResource2D::init(ErrorState& errorState)
	{
		mPrevTexture = mTexture;
		mTexture = new opengl::Texture2D;
		mTexture->init();

		mTexture->allocate(mSettings);

		return true;
	}

	void MemoryTextureResource2D::finish(Resource::EFinishMode mode)
	{
		if (mode == Resource::EFinishMode::COMMIT)
		{
			if (mPrevTexture != nullptr)
			{
				delete mPrevTexture;
				mPrevTexture = nullptr;
			}
		}
		else
		{
			assert(mode == Resource::EFinishMode::ROLLBACK);
			delete mTexture;
			mTexture = mPrevTexture;
			mPrevTexture = nullptr;
		}
	}

	// Returns 2D texture object
	const opengl::BaseTexture& MemoryTextureResource2D::getTexture() const
	{
		assert(mTexture != nullptr);
		return *mTexture;
	}

	// Constructor
	ImageResource::ImageResource(const std::string& imgPath)
	{
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

	bool ImageResource::init(ErrorState& errorState)
	{
		if (!errorState.check(!mImagePath.empty(), "Imagepath not set"))
			return false;

		mPrevImage = mImage;
		mImage = new opengl::Image;

		if (!errorState.check(mImage->load(mImagePath), "Unable to load image from file"))
			return false;

		return true;
	}

	void ImageResource::finish(Resource::EFinishMode mode)
	{
		if (mode == Resource::EFinishMode::COMMIT)
		{
			if (mPrevImage != nullptr)
			{
				delete mPrevImage;
				mPrevImage = nullptr;
			}
		}
		else
		{
			assert(mode == Resource::EFinishMode::ROLLBACK);
			delete mImage;
			mImage = mPrevImage;
			mPrevImage = nullptr;
		}
	}

	const opengl::Image& ImageResource::getImage() const
	{
		assert(mImage != nullptr);
		return *mImage;
	}
	
	// Non const getter, following:
	opengl::BaseTexture& TextureResource::getTexture()
	{
		return const_cast<opengl::BaseTexture&>(static_cast<const TextureResource&>(*this).getTexture());
	}
	
}
