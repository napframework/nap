#include "rendertexture2d.h"
#include "nap/core.h"
#include "renderservice.h"

RTTI_BEGIN_ENUM(nap::ERenderTargetFormat)
	RTTI_ENUM_VALUE(nap::ERenderTargetFormat::RGBA8,		"RGBA8"),
	RTTI_ENUM_VALUE(nap::ERenderTargetFormat::R8,			"R8"),	
	RTTI_ENUM_VALUE(nap::ERenderTargetFormat::Depth,		"Depth"),
	RTTI_ENUM_VALUE(nap::ERenderTargetFormat::Backbuffer,	"Backbuffer")
RTTI_END_ENUM

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
		SurfaceDescriptor settings;
		settings.mWidth = mWidth;
		settings.mHeight = mHeight;
		settings.mDataType = ESurfaceDataType::BYTE;
		settings.mColorSpace = mColorSpace;

		switch (mFormat)
		{
		case ERenderTargetFormat::Backbuffer:
			settings.mChannels = ESurfaceChannels::BGRA;
			return Texture2D::init(settings, false, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, errorState);
		case ERenderTargetFormat::Depth:
			settings.mChannels = ESurfaceChannels::Depth;
			settings.mDataType = ESurfaceDataType::FLOAT;
			return Texture2D::init(settings, false, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, errorState);
		case ERenderTargetFormat::RGBA8:
			settings.mChannels = ESurfaceChannels::RGBA;
			return Texture2D::init(settings, false, errorState);

		case ERenderTargetFormat::R8:
			settings.mChannels = ESurfaceChannels::R;
			return Texture2D::init(settings, false, errorState);
		}

		assert(false);
		return false;
	}
}
