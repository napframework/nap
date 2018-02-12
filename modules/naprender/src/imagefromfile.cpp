// Local Includes
#include "imagefromfile.h"
#include "bitmaputils.h"

// External Includes
#include <nap/logger.h>

RTTI_BEGIN_CLASS(nap::ImageFromFile)
	RTTI_PROPERTY_FILELINK("ImagePath", &nap::ImageFromFile::mImagePath, 		nap::rtti::EPropertyMetaData::Required, nap::rtti::EPropertyFileType::Image)
	RTTI_PROPERTY("Compressed",			&nap::ImageFromFile::mCompressed,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{

	// Constructor
	ImageFromFile::ImageFromFile(const std::string& imgPath) :
		mImagePath(imgPath)
	{
	}


	bool ImageFromFile::init(utility::ErrorState& errorState)
	{
		// Load pixel data in to bitmap
		if (!getBitmap().initFromFile(mImagePath, errorState))
			return false;

		return Texture2D::initFromBitmap(getBitmap(), mCompressed, errorState);
	}
}
