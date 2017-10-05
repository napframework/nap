// Local Includes
#include "image.h"
#include "nbitmap.h"
#include "nbitmaputils.h"
#include "ntextureutils.h"

// External Includes
#include <nap/logger.h>
#include <nap/fileutils.h>

RTTI_BEGIN_CLASS(nap::Image)
	RTTI_PROPERTY("ImagePath", 			&nap::Image::mImagePath, 		nap::rtti::EPropertyMetaData::FileLink | nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Compressed",			&nap::Image::mCompressed,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{

	// Constructor
	Image::Image(const std::string& imgPath) { }


	bool Image::init(utility::ErrorState& errorState)
	{
		if (!errorState.check(!mImagePath.empty(), "Image path not set for ImageResource %s", mID.c_str()))
			return false;

		// Load pixel data in to bitmap
		if (!errorState.check(opengl::loadBitmap(mBitmap, mImagePath, errorState), "Failed to load image %s; invalid bitmap", mImagePath.c_str()))
			return false;
		
		// Get opengl settings from bitmap
		opengl::Texture2DSettings settings;
		if (!errorState.check(opengl::getSettingsFromBitmap(mBitmap, mCompressed, settings, errorState), "Unable to determine texture settings from bitmap %s", mImagePath.c_str()))
			return false;

		// Initialize texture from bitmap
		BaseTexture2D::init(settings);

		// Set data from bitmap
		getTexture().setData(mBitmap.getData());

		return true;
	}
}
