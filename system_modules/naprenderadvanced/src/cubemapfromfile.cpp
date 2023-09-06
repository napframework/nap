#include "cubemapfromfile.h"

// External includes
#include <nap/core.h>

// nap::cubemapfromfile run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::CubeMapFromFile)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("SourceTexture", &nap::CubeMapFromFile::mSourceTexture, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("SampleShading", &nap::CubeMapFromFile::mSampleShading, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	CubeMapFromFile::CubeMapFromFile(Core& core) :
		RenderTextureCube(core) { }


	bool CubeMapFromFile::init(utility::ErrorState& errorState)
	{
		return RenderTextureCube::init(errorState);
	}
}
