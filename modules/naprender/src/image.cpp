// Local Includes
#include "image.h"

// External Includes
#include <nap/logger.h>
#include <nap/fileutils.h>

RTTI_BEGIN_CLASS(nap::Image)
	RTTI_PROPERTY("mImagePath", 		&nap::Image::mImagePath, 			nap::rtti::EPropertyMetaData::FileLink | nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

namespace nap
{

	// Constructor
	Image::Image(const std::string& imgPath) { }


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
		
		// Convert and set texture parameters
		opengl::TextureParameters gl_params;
		convertTextureParameters(mParameters, gl_params);
		mImage->setTextureParameters(gl_params);

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
