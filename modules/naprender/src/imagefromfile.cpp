// Local Includes
#include "imagefromfile.h"

// External Includes
#include <nap/logger.h>
#include <nap/core.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ImageFromFile)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY_FILELINK("ImagePath", &nap::ImageFromFile::mImagePath, 		nap::rtti::EPropertyMetaData::Required, nap::rtti::EPropertyFileType::Image)
	RTTI_PROPERTY("Compressed",			&nap::ImageFromFile::mCompressed,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
	ImageFromFile::ImageFromFile(Core& core) :
		Image(core)
	{
	}


	// Constructor
	ImageFromFile::ImageFromFile(Core& core, const std::string& imgPath) :
		Image(core),
		mImagePath(imgPath)
	{
	}


	bool ImageFromFile::init(utility::ErrorState& errorState)
	{
		// Load pixel data in to bitmap
		if (!getBitmap().initFromFile(mImagePath, errorState))
			return false;

		return Texture2D::init(getBitmap().mSurfaceDescriptor, true, getBitmap().getData(), errorState);
	}
}
