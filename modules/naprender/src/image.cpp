// Local Includes
#include "image.h"

// External Includes
#include <nap/logger.h>
#include <nap/fileutils.h>


RTTI_BEGIN_CLASS(opengl::TextureParameters)
	RTTI_PROPERTY("mMinFilter",			&opengl::TextureParameters::minFilter,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("mMaxFilter",			&opengl::TextureParameters::maxFilter,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("mWrapVertical",		&opengl::TextureParameters::wrapVertical,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("mWrapHorizontal",	&opengl::TextureParameters::wrapHorizontal, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("mMaxLodLevel",		&opengl::TextureParameters::maxLodLevel,	nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_BASE_CLASS(nap::Texture)
	RTTI_PROPERTY("mParameters", 		&nap::Texture::mParameters,			nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS


RTTI_BEGIN_CLASS(nap::Image)
	RTTI_PROPERTY("mImagePath", 		&nap::Image::mImagePath, 			nap::rtti::EPropertyMetaData::FileLink | nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(opengl::Texture2DSettings)
	RTTI_PROPERTY("mInternalFormat",	&opengl::Texture2DSettings::internalFormat, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("mWidth",				&opengl::Texture2DSettings::width,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("mHeight",			&opengl::Texture2DSettings::height,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("mFormat",			&opengl::Texture2DSettings::format,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("mType",				&opengl::Texture2DSettings::type,			nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::MemoryTexture2D)
	RTTI_PROPERTY("mSettings",			&nap::MemoryTexture2D::mSettings, 	nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

namespace nap
{
	// Initializes 2D texture. Additionally a custom display name can be provided.
	bool MemoryTexture2D::init(utility::ErrorState& errorState)
	{
		// Create 2D texture
		mTexture = std::make_unique<opengl::Texture2D>();
		
		// Create the texture with the associated settings
		mTexture->setParameters(mParameters);
		mTexture->init();

		// Allocate the texture with the associated 2D image settings
 		mTexture->allocate(mSettings);

		return true;
	}


	// Returns 2D texture object
	const opengl::BaseTexture& MemoryTexture2D::getTexture() const
	{
		assert(mTexture != nullptr);
		return *mTexture;
	}


	const glm::vec2 MemoryTexture2D::getSize() const
	{
		return glm::vec2(mTexture->getSettings().width, mTexture->getSettings().height);
	}


	// Constructor
	Image::Image(const std::string& imgPath)
	{
	}


	// Load image if required and extract texture
	const opengl::BaseTexture& Image::getTexture() const
	{
		return getImage().getTexture();
	}


	bool Image::init(utility::ErrorState& errorState)
	{
		if (!errorState.check(!mImagePath.empty(), "Image path not set for ImageResource %s", mID.c_str()))
			return false;

		// Make texture and set associated texture parameters
		mImage = std::make_unique<opengl::Image>();
		mImage->setTextureParameters(mParameters);

		// Load the texture using associated settings
		if (!errorState.check(mImage->load(mImagePath), "Unable to load image from file %s for ImageResource %s", mImagePath.c_str(), mID.c_str()))
			return false;

		return true;
	}


	const opengl::Image& Image::getImage() const
	{
		assert(mImage != nullptr);
		return *mImage;
	}


	const glm::vec2 Image::getSize() const
	{
		return glm::vec2(mImage->getWidth(), mImage->getHeight());
	}

	
	// Non const getter, following:
	opengl::BaseTexture& Texture::getTexture()
	{
		return const_cast<opengl::BaseTexture&>(static_cast<const Texture&>(*this).getTexture());
	}
	
}
