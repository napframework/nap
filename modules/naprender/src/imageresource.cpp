// Local Includes
#include "imageresource.h"

// External Includes
#include <nap/logger.h>
#include <nap/fileutils.h>

RTTI_DEFINE_BASE(nap::TextureResource)

RTTI_BEGIN_CLASS(nap::ImageResource)
	RTTI_PROPERTY("mImagePath", 		&nap::ImageResource::mImagePath, 			nap::rtti::EPropertyMetaData::FileLink | nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(opengl::Texture2DSettings)
	RTTI_PROPERTY("mLevel",				&opengl::Texture2DSettings::level,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("mInternalFormat",	&opengl::Texture2DSettings::internalFormat, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("mWidth",				&opengl::Texture2DSettings::width,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("mHeight",			&opengl::Texture2DSettings::height,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("mFormat",			&opengl::Texture2DSettings::format,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("mType",				&opengl::Texture2DSettings::type,			nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::MemoryTextureResource2D)
	RTTI_PROPERTY("mSettings",			&nap::MemoryTextureResource2D::mSettings, 	nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_DEFINE(nap::ImageResource)

namespace nap
{
	// Initializes 2D texture. Additionally a custom display name can be provided.
	bool MemoryTextureResource2D::init(utility::ErrorState& errorState)
	{
		mTexture = std::make_unique<opengl::Texture2D>();
		mTexture->init();

 		mTexture->allocate(mSettings);

		return true;
	}


	// Returns 2D texture object
	const opengl::BaseTexture& MemoryTextureResource2D::getTexture() const
	{
		assert(mTexture != nullptr);
		return *mTexture;
	}


	const glm::vec2 MemoryTextureResource2D::getSize() const
	{
		return glm::vec2(mTexture->getSettings().width, mTexture->getSettings().height);
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

	bool ImageResource::init(utility::ErrorState& errorState)
	{
		if (!errorState.check(!mImagePath.empty(), "Imagepath not set"))
			return false;

		mImage = std::make_unique<opengl::Image>();

		if (!errorState.check(mImage->load(mImagePath), "Unable to load image from file"))
			return false;

		return true;
	}

	const opengl::Image& ImageResource::getImage() const
	{
		assert(mImage != nullptr);
		return *mImage;
	}


	const glm::vec2 ImageResource::getSize() const
	{
		return glm::vec2(mImage->getWidth(), mImage->getHeight());
	}

	
	// Non const getter, following:
	opengl::BaseTexture& TextureResource::getTexture()
	{
		return const_cast<opengl::BaseTexture&>(static_cast<const TextureResource&>(*this).getTexture());
	}
	
}
