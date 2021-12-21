/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "imguiicon.h"

// External Includes
#include <utility/fileutils.h>
#include <nap/core.h>
#include <imguiservice.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Icon)
	RTTI_CONSTRUCTOR(nap::IMGuiService&)
	RTTI_CONSTRUCTOR(nap::IMGuiService&, const std::string&)
	RTTI_PROPERTY("ImagePath", &nap::Icon::mImagePath, nap::rtti::EPropertyMetaData::Required | nap::rtti::EPropertyMetaData::FileLink)
RTTI_END_CLASS

namespace nap
{

	Icon::Icon(nap::IMGuiService& guiService) :
		mImage(guiService.getCore()),
		mGuiService(guiService)
	{ }


	Icon::Icon(nap::IMGuiService& guiService, const std::string& imagePath) :
		mImagePath(imagePath),
		mImage(guiService.getCore()),
		mGuiService(guiService)
	{ }


	bool Icon::init(utility::ErrorState& error)
	{
		// Ensure path exists
		if (!error.check(!mImagePath.empty(), "Missing icon path"))
			return false;

		mName = utility::stripFileExtension(utility::getFileName(mImagePath));
		mImage.mGenerateLods = true;
		mImage.mImagePath = mImagePath;
		mImage.mUsage = ETextureUsage::Static;
		return mImage.init(error);
	}


	ImTextureID Icon::getTextureHandle() const
	{
		return mGuiService.getTextureHandle(mImage);
	}

}
