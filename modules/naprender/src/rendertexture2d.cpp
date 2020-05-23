#include "rendertexture2d.h"
#include "nap/core.h"
#include "renderservice.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderTexture2D)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Width",		&nap::RenderTexture2D::mWidth, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Height",		&nap::RenderTexture2D::mHeight, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("ColorSpace", &nap::RenderTexture2D::mColorSpace, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
	RenderTexture2D::RenderTexture2D(Core& core) :
		Texture2D(core)
	{
	}

	// Initializes 2D texture. 
	bool RenderTexture2D::init(utility::ErrorState& errorState)
	{
		SurfaceDescriptor settings;
		settings.mWidth = mWidth;
		settings.mHeight = mHeight;
		settings.mDataType = ESurfaceDataType::BYTE;
		settings.mColorSpace = mColorSpace;
		settings.mChannels = ESurfaceChannels::BGRA;

		return Texture2D::init(settings, false, VK_IMAGE_USAGE_SAMPLED_BIT, errorState);
	}
}
