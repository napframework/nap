#include "rendertexture2d.h"
#include "nap/core.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderTexture2D)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Width",		&nap::RenderTexture2D::mWidth, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Height",		&nap::RenderTexture2D::mHeight, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Format",		&nap::RenderTexture2D::mFormat, nap::rtti::EPropertyMetaData::Required)
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
		Texture2DSettings settings;
		settings.mWidth = mWidth;
		settings.mHeight = mHeight;
		settings.mDataType = ESurfaceDataType::BYTE;
		settings.mColorSpace = mColorSpace;

		switch (mFormat)
		{
		case ERenderTargetFormat::RGBA8:
			settings.mChannels = ESurfaceChannels::RGBA;
			break;
		case ERenderTargetFormat::R8:
			settings.mChannels = ESurfaceChannels::R;
			break;
		case ERenderTargetFormat::Depth:
			return true;
			//assert(false);
// 			settings.mFormat			= GL_DEPTH_COMPONENT;
// 			settings.mInternalFormat 	= GL_DEPTH_COMPONENT;
// 			settings.mType				= GL_FLOAT;
			break;
		}

		return initTexture(settings, errorState);
	}

}
