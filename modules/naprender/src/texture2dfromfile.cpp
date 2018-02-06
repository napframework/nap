// Local Includes
#include "texture2dfromfile.h"
#include "bitmaputils.h"

// External Includes
#include <nap/logger.h>

RTTI_BEGIN_CLASS(nap::Texture2DFromFile)
	RTTI_PROPERTY("ImagePath", 			&nap::Texture2DFromFile::mImagePath, 		nap::rtti::EPropertyMetaData::FileLink | nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Compressed",			&nap::Texture2DFromFile::mCompressed,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{

	// Constructor
	Texture2DFromFile::Texture2DFromFile(const std::string& imgPath) :
		mImagePath(imgPath)
	{
	}


	bool Texture2DFromFile::init(utility::ErrorState& errorState)
	{
		// Load pixel data in to bitmap
		if (!getBitmap().initFromFile(mImagePath, errorState))
			return false;

		return Texture2D::initFromBitmap(getBitmap(), mCompressed, errorState);
	}
}
