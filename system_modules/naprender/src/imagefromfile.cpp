/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "imagefromfile.h"

// External Includes
#include <nap/logger.h>
#include <nap/core.h>
#include <renderservice.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ImageFromFile, "2D image that is loaded from disk and uploaded to the GPU. Holds the CPU (bitmap) and GPU (texture) data")
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Usage",					&nap::ImageFromFile::mUsage,			nap::rtti::EPropertyMetaData::Default, "How the texture is used at runtime (static, updated etc..)")
	RTTI_PROPERTY_FILELINK("ImagePath",		&nap::ImageFromFile::mImagePath, 		nap::rtti::EPropertyMetaData::Required, nap::rtti::EPropertyFileType::Image, "Path to the image on disk")
	RTTI_PROPERTY("GenerateLods",			&nap::ImageFromFile::mGenerateLods,		nap::rtti::EPropertyMetaData::Default, "If lower levels of detail (LODs) are auto-generated for the image")
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

		// Ensure hardware mip-mapping is supported
		if(!errorState.check(!mGenerateLods || mRenderService.getMipSupport(getBitmap().mSurfaceDescriptor),
			"%s: hardware mipmap generation not supported, consider disabling 'GenerateLods'", mID.c_str()))
			return false;

		// Initialize
		int lvl = mGenerateLods ? utility::computeMipLevel(getBitmap().mSurfaceDescriptor) : 1;
		return Texture2D::init(getBitmap().mSurfaceDescriptor, mUsage, lvl, getBitmap().getData(), 0, errorState);
	}
}
