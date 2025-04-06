/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local includes
#include "cubemapfromfile.h"

// External includes
#include <nap/core.h>

// nap::cubemapfromfile run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::CubeMapFromFile, "Creates a cube map from a equirectangular image loaded from disk")
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY_FILELINK("ImagePath", &nap::CubeMapFromFile::mImagePath, nap::rtti::EPropertyMetaData::Required, nap::rtti::EPropertyFileType::Image, "Path to the equirectangular image on disk")
RTTI_END_CLASS

namespace nap
{
	CubeMapFromFile::CubeMapFromFile(Core& core) :
		EquiRectangularCubeMap(core), mEquiRectangularImage(core)
	{ }


	bool CubeMapFromFile::init(utility::ErrorState& errorState)
	{
		// Load & schedule upload equirectangular 2D texture 
		mEquiRectangularImage.mGenerateLods = false;
		mEquiRectangularImage.mImagePath = mImagePath;
		mEquiRectangularImage.mUsage = Texture2D::EUsage::Static;
		mEquiRectangularImage.mID = nap::utility::stringFormat("%s_source", mID.c_str());
		if (!mEquiRectangularImage.init(errorState))
			return false;

		// Schedule conversion
		return EquiRectangularCubeMap::init(mEquiRectangularImage, errorState);
	}
}
