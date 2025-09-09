/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "rendertexture2d.h"
#include "nap/core.h"
#include "renderservice.h"

RTTI_BEGIN_ENUM(nap::RenderTexture2D::EFormat)
	RTTI_ENUM_VALUE(nap::RenderTexture2D::EFormat::RGBA8,	"RGBA8"),
	RTTI_ENUM_VALUE(nap::RenderTexture2D::EFormat::BGRA8,	"BGRA8"),
	RTTI_ENUM_VALUE(nap::RenderTexture2D::EFormat::R8,		"R8"),
	RTTI_ENUM_VALUE(nap::RenderTexture2D::EFormat::RGBA16,	"RGBA16"),
	RTTI_ENUM_VALUE(nap::RenderTexture2D::EFormat::R16,		"R16"),
	RTTI_ENUM_VALUE(nap::RenderTexture2D::EFormat::RGBA32,	"RGBA32"),
	RTTI_ENUM_VALUE(nap::RenderTexture2D::EFormat::R32,		"R32")
RTTI_END_ENUM

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderTexture2D, "Holds the color output from a render pass using a render target")
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Width",		&nap::RenderTexture2D::mWidth,			nap::rtti::EPropertyMetaData::Required, "Width of the texture in texels")
	RTTI_PROPERTY("Height",		&nap::RenderTexture2D::mHeight,			nap::rtti::EPropertyMetaData::Required, "Height of the texture in texels")
	RTTI_PROPERTY("Format",		&nap::RenderTexture2D::mColorFormat,	nap::rtti::EPropertyMetaData::Required, "Texture color format")
	RTTI_PROPERTY("ColorSpace", &nap::RenderTexture2D::mColorSpace,		nap::rtti::EPropertyMetaData::Default,	"Texture color space")
	RTTI_PROPERTY("ClearColor", &nap::RenderTexture2D::mClearColor,		nap::rtti::EPropertyMetaData::Default,	"Initial clear color")
	RTTI_PROPERTY("Usage",		&nap::RenderTexture2D::mUsage,			nap::rtti::EPropertyMetaData::Default,	"How the texture is used at runtime (internal, updated etc..)")
RTTI_END_CLASS

RTTI_BEGIN_ENUM(nap::DepthRenderTexture2D::EDepthFormat)
	RTTI_ENUM_VALUE(nap::DepthRenderTexture2D::EDepthFormat::D16,	"D16"),
	RTTI_ENUM_VALUE(nap::DepthRenderTexture2D::EDepthFormat::D32,	"D32")
RTTI_END_ENUM

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::DepthRenderTexture2D, "Holds the depth output from a render pass using a render target")
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Width",		&nap::DepthRenderTexture2D::mWidth,			nap::rtti::EPropertyMetaData::Required, "Width of the texture in texels")
	RTTI_PROPERTY("Height",		&nap::DepthRenderTexture2D::mHeight,		nap::rtti::EPropertyMetaData::Required, "Height of the texture in texels")
	RTTI_PROPERTY("Format",		&nap::DepthRenderTexture2D::mDepthFormat,	nap::rtti::EPropertyMetaData::Required, "Texture depth format")
	RTTI_PROPERTY("ColorSpace", &nap::DepthRenderTexture2D::mColorSpace,	nap::rtti::EPropertyMetaData::Default,	"Texture color space")
	RTTI_PROPERTY("ClearValue", &nap::DepthRenderTexture2D::mClearValue,	nap::rtti::EPropertyMetaData::Default,	"Initial clear value")
	RTTI_PROPERTY("Usage",		&nap::DepthRenderTexture2D::mUsage,			nap::rtti::EPropertyMetaData::Default,	"How the texture is used at runtime (internal, updated etc..)")
RTTI_END_CLASS


namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// RenderTexture2D
	//////////////////////////////////////////////////////////////////////////

	RenderTexture2D::RenderTexture2D(Core& core) :
		Texture2D(core)
	{ }

	// Initializes 2D texture. 
	bool RenderTexture2D::init(utility::ErrorState& errorState)
	{
		SurfaceDescriptor settings;
		settings.mWidth = mWidth;
		settings.mHeight = mHeight;
		settings.mColorSpace = mColorSpace;

		// Initialize based on selected format
		switch (mColorFormat)
		{
			case RenderTexture2D::EFormat::RGBA8:
			{
				settings.mDataType = ESurfaceDataType::BYTE;
				settings.mChannels = ESurfaceChannels::RGBA;
				break;
			}
			case RenderTexture2D::EFormat::BGRA8:
			{
				settings.mDataType = ESurfaceDataType::BYTE;
				settings.mChannels = ESurfaceChannels::BGRA;
				break;
			}
			case RenderTexture2D::EFormat::R8:
			{
				settings.mDataType = ESurfaceDataType::BYTE;
				settings.mChannels = ESurfaceChannels::R;
				break;
			}
			case RenderTexture2D::EFormat::RGBA16:
			{
				settings.mDataType = ESurfaceDataType::USHORT;
				settings.mChannels = ESurfaceChannels::RGBA;
				break;
			}
			case RenderTexture2D::EFormat::R16:
			{
				settings.mDataType = ESurfaceDataType::USHORT;
				settings.mChannels = ESurfaceChannels::R;
				break;
			}
			case RenderTexture2D::EFormat::RGBA32:
			{
				settings.mDataType = ESurfaceDataType::FLOAT;
				settings.mChannels = ESurfaceChannels::RGBA;
				break;
			}
			case RenderTexture2D::EFormat::R32:
			{
				settings.mDataType = ESurfaceDataType::FLOAT;
				settings.mChannels = ESurfaceChannels::R;
				break;
			}
			default:
			{
				errorState.fail("Unsupported format");
				return false;
			}
		}

		// Create render texture
		return Texture2D::init(settings, mUsage, 1, mClearColor.toVec4(), VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, errorState);
	}


	//////////////////////////////////////////////////////////////////////////
	// DepthRenderTexture2D
	//////////////////////////////////////////////////////////////////////////

	DepthRenderTexture2D::DepthRenderTexture2D(Core& core) :
		Texture2D(core)
	{ }

	// Initializes 2D texture. 
	bool DepthRenderTexture2D::init(utility::ErrorState& errorState)
	{
		SurfaceDescriptor settings;
		settings.mWidth = mWidth;
		settings.mHeight = mHeight;
		settings.mColorSpace = mColorSpace;
		settings.mChannels = ESurfaceChannels::D;

		// Initialize based on selected format
		const glm::vec4 clear_color = { mClearValue, mClearValue, mClearValue, mClearValue };
		switch (mDepthFormat)
		{
			case DepthRenderTexture2D::EDepthFormat::D16:
			{
				settings.mDataType = ESurfaceDataType::USHORT;
				return Texture2D::init(settings, mUsage, 1, clear_color, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, errorState);
			}
			case DepthRenderTexture2D::EDepthFormat::D32:
			{
				settings.mDataType = ESurfaceDataType::FLOAT;
				return Texture2D::init(settings, mUsage, 1, clear_color, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, errorState);
			}
			default:
			{
				errorState.fail("Unsupported format");
				return false;
			}
		}
	}
}
