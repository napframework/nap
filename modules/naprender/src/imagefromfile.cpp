/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "imagefromfile.h"

// External Includes
#include <nap/logger.h>
#include <nap/core.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ImageFromFile)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY_FILELINK("ImagePath",		&nap::ImageFromFile::mImagePath, 		nap::rtti::EPropertyMetaData::Required, nap::rtti::EPropertyFileType::Image)
	RTTI_PROPERTY("GenerateLods",			&nap::ImageFromFile::mGenerateLods,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
	ImageFromFile::ImageFromFile(Core& core) :
		Image(core)
	{ }


	// Constructor
	ImageFromFile::ImageFromFile(Core& core, const std::string& imgPath) :
		Image(core),
		mImagePath(imgPath)
	{ }


	bool ImageFromFile::init(utility::ErrorState& errorState)
	{
		// Load pixel data in to bitmap
		if (!getBitmap().initFromFile(mImagePath, errorState))
			return false;

		// Create 2D texture
		return Texture2D::init(getBitmap().mSurfaceDescriptor, mGenerateLods, getBitmap().getData(), 0, errorState);
	}
}
