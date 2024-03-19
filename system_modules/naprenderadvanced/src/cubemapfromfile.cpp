#include "cubemapfromfile.h"

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
		mSourceImage(std::make_unique<Image>(core))
	{ }


	bool CubeMapFromFile::init(utility::ErrorState& errorState)
	{
		// Ensure the cube map is (re-)rendered by the render advanced service
		mDirty = true;

		mSourceImage->mUsage = EUsage::Static;
		if (!mSourceImage->getBitmap().initFromFile(mImagePath, errorState))
			return false;

		if (!mSourceImage->init(mSourceImage->getBitmap().mSurfaceDescriptor, false, mSourceImage->getBitmap().getData(), 0, errorState))
			return false;

		return RenderTextureCube::init(errorState);
	}
}
