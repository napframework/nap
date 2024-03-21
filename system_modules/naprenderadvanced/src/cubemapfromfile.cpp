/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "cubemapfromfile.h"
#include "renderadvancedservice.h"

// External includes
#include <nap/core.h>

// nap::cubemapfromfile run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::CubeMapFromFile)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY_FILELINK("ImagePath", &nap::CubeMapFromFile::mImagePath, nap::rtti::EPropertyMetaData::Required, nap::rtti::EPropertyFileType::Image)
	RTTI_PROPERTY("SampleShading", &nap::CubeMapFromFile::mSampleShading, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("GenerateLODs", &nap::CubeMapFromFile::mGenerateLODs, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	CubeMapFromFile::CubeMapFromFile(Core& core) :
		RenderTextureCube(core),
		mRenderAdvancedService(core.getService<RenderAdvancedService>()),
		mSourceImage(std::make_unique<Image>(core))
	{ }


	bool CubeMapFromFile::init(utility::ErrorState& errorState)
	{
		if (!RenderTextureCube::init(errorState))
			return false;

		mSourceImage->mUsage = EUsage::Static;
		if (!mSourceImage->getBitmap().initFromFile(mImagePath, errorState))
			return false;

		if (!mSourceImage->init(mSourceImage->getBitmap().mSurfaceDescriptor, false, mSourceImage->getBitmap().getData(), 0, errorState))
			return false;

		mRenderAdvancedService->registerCubeMap(*this);
		return true;
	}


	void CubeMapFromFile::onDestroy()
	{
		mRenderAdvancedService->removeCubeMap(*this);
	}
}
