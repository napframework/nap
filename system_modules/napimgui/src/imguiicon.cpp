/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "imguiicon.h"

// External Includes
#include <utility/fileutils.h>
#include <nap/core.h>
#include <imguiservice.h>
#include <FreeImage.h>
#include <bitmapfilebuffer.h>
#include <copyimagedata.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Icon)
	RTTI_CONSTRUCTOR(nap::IMGuiService&)
	RTTI_CONSTRUCTOR(nap::IMGuiService&, const std::string&)
	RTTI_PROPERTY("Invert",		&nap::Icon::mInvert,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ImagePath",	&nap::Icon::mImagePath, nap::rtti::EPropertyMetaData::Required | nap::rtti::EPropertyMetaData::FileLink)
RTTI_END_CLASS

namespace nap
{
	static bool invert(BitmapFileBuffer& source, utility::ErrorState& error)
	{
		static const std::array<FREE_IMAGE_COLOR_CHANNEL, 4> channels =
		{
			FICC_RED,
			FICC_GREEN,
			FICC_BLUE,
			FICC_BLACK
		};

		FIBITMAP* bitmap = reinterpret_cast<FIBITMAP*>(source.getHandle());
		for (FREE_IMAGE_COLOR_CHANNEL ch : channels)
		{
			// Get channel data if exists
			FIBITMAP* ch_data = FreeImage_GetChannel(bitmap, ch);
			if (ch_data == nullptr)
				continue;

			// Invert
			if (!error.check(FreeImage_Invert(ch_data) && FreeImage_SetChannel(bitmap, ch_data, ch),
				"Unable to invert channel: %d", (int)ch))
			{
				FreeImage_Unload(ch_data);
				return false;
			}
			FreeImage_Unload(ch_data);
		}
		return true;
	}


	Icon::Icon(nap::IMGuiService& guiService) :
		mTexture(guiService.getCore()),
		mGuiService(guiService)
	{ }


	Icon::Icon(nap::IMGuiService& guiService, const std::string& imagePath) :
		mImagePath(imagePath),
		mTexture(guiService.getCore()),
		mGuiService(guiService)
	{ }


	bool Icon::init(utility::ErrorState& error)
	{
		// Ensure path exists
		if (!error.check(!mImagePath.empty(), "Missing icon path"))
			return false;

		// Extract name
		mName = utility::stripFileExtension(utility::getFileName(mImagePath));

		// Load bitmap into memory
		BitmapFileBuffer file_buffer;
		SurfaceDescriptor descriptor;
		if (!file_buffer.load(mImagePath, descriptor, error))
			return false;

		// Invert colors if requested
		if (mInvert && !invert(file_buffer, error))
			return false;

		// Create 2D texture
		mTexture.mID = mName;
		return mTexture.init(descriptor, true, file_buffer.getData(), 0, error);
	}


	ImTextureID Icon::getTextureHandle() const
	{
		return mGuiService.getTextureHandle(mTexture);
	}

}
