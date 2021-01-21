/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <utility/dllexport.h>
#include <utility/errorstate.h>
#include <FreeImage.h>

// Local Includes
#include "surfacedescriptor.h"

namespace nap
{
	namespace utility
	{
		/**
		* @param dataType surface data type
		* @param channels surface channels
		* @return the FreeImage type associated with this surface description
		*/
		FREE_IMAGE_TYPE NAPAPI getFIType(ESurfaceDataType dataType, ESurfaceChannels channels);

		/**
		* Writes a FreeImage bitmap to disk
		* @param fiBitmap the bitmap handle
		* @param fiType the free image format that determines the target extension
		* @param path the path including filename and image extension of the output file e.g. "targetFolder/MyOutputFile.png"
		* @param errorState error state
		* @return true if successful
		*/
		bool NAPAPI writeToDisk(FIBITMAP* fiBitmap, FREE_IMAGE_TYPE fiType, const std::string& path, utility::ErrorState& errorState);
	}
}
