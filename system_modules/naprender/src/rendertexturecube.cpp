/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "rendertexturecube.h"
#include "nap/core.h"
#include "renderservice.h"


RTTI_BEGIN_ENUM(nap::RenderTextureCube::EFormat)
	RTTI_ENUM_VALUE(nap::RenderTextureCube::EFormat::RGBA8, "RGBA8"),
	RTTI_ENUM_VALUE(nap::RenderTextureCube::EFormat::BGRA8, "BGRA8"),
	RTTI_ENUM_VALUE(nap::RenderTextureCube::EFormat::R8, "R8"),
	RTTI_ENUM_VALUE(nap::RenderTextureCube::EFormat::RGBA16, "RGBA16"),
	RTTI_ENUM_VALUE(nap::RenderTextureCube::EFormat::R16, "R16"),
	RTTI_ENUM_VALUE(nap::RenderTextureCube::EFormat::RGBA32, "RGBA32"),
	RTTI_ENUM_VALUE(nap::RenderTextureCube::EFormat::R32, "R32")
RTTI_END_ENUM

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderTextureCube, "Cubemap GPU color texture ")
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Width",		&nap::RenderTextureCube::mWidth,			nap::rtti::EPropertyMetaData::Required, "Width of a single cube face in texels")
	RTTI_PROPERTY("Height",		&nap::RenderTextureCube::mHeight,			nap::rtti::EPropertyMetaData::Required, "Height of a single cube face in texels")
	RTTI_PROPERTY("Format",		&nap::RenderTextureCube::mColorFormat,		nap::rtti::EPropertyMetaData::Required, "Texture color format")
	RTTI_PROPERTY("ColorSpace", &nap::RenderTextureCube::mColorSpace,		nap::rtti::EPropertyMetaData::Default,	"Texture color space")
	RTTI_PROPERTY("ClearColor", &nap::RenderTextureCube::mClearColor,		nap::rtti::EPropertyMetaData::Default,	"Initial clear color")
RTTI_END_CLASS

RTTI_BEGIN_ENUM(nap::DepthRenderTextureCube::EDepthFormat)
	RTTI_ENUM_VALUE(nap::DepthRenderTextureCube::EDepthFormat::D16, "D16"),
	RTTI_ENUM_VALUE(nap::DepthRenderTextureCube::EDepthFormat::D32, "D32")
RTTI_END_ENUM

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::DepthRenderTextureCube, "Cubemap GPU depth texture ")
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Width",		&nap::DepthRenderTextureCube::mWidth,		nap::rtti::EPropertyMetaData::Required, "Width of a single cube face in texels")
	RTTI_PROPERTY("Height",		&nap::DepthRenderTextureCube::mHeight,		nap::rtti::EPropertyMetaData::Required, "Height of a single cube face in texels")
	RTTI_PROPERTY("Format",		&nap::DepthRenderTextureCube::mDepthFormat, nap::rtti::EPropertyMetaData::Required,	"Texture depth format")
	RTTI_PROPERTY("ColorSpace", &nap::DepthRenderTextureCube::mColorSpace,	nap::rtti::EPropertyMetaData::Default,	"Texture color space")
	RTTI_PROPERTY("ClearColor", &nap::DepthRenderTextureCube::mClearValue,	nap::rtti::EPropertyMetaData::Default,	"Initial clear value")
RTTI_END_CLASS


namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// RenderTextureCube
	//////////////////////////////////////////////////////////////////////////

	RenderTextureCube::RenderTextureCube(Core& core) :
		TextureCube(core)
	{ }

	// Initializes Cube texture. 
	bool RenderTextureCube::init(utility::ErrorState& errorState)
	{
		SurfaceDescriptor settings;
		settings.mWidth = mWidth;
		settings.mHeight = mHeight;
		settings.mColorSpace = mColorSpace;

		// Initialize based on selected format
		switch (mColorFormat)
		{
			case RenderTextureCube::EFormat::RGBA8:
			{
				settings.mDataType = ESurfaceDataType::BYTE;
				settings.mChannels = ESurfaceChannels::RGBA;
				break;
			}
			case RenderTextureCube::EFormat::BGRA8:
			{
				settings.mDataType = ESurfaceDataType::BYTE;
				settings.mChannels = ESurfaceChannels::BGRA;
				break;
			}
			case RenderTextureCube::EFormat::R8:
			{
				settings.mDataType = ESurfaceDataType::BYTE;
				settings.mChannels = ESurfaceChannels::R;
				break;
			}
			case RenderTextureCube::EFormat::RGBA16:
			{
				settings.mDataType = ESurfaceDataType::USHORT;
				settings.mChannels = ESurfaceChannels::RGBA;
				break;
			}
			case RenderTextureCube::EFormat::R16:
			{
				settings.mDataType = ESurfaceDataType::USHORT;
				settings.mChannels = ESurfaceChannels::R;
				break;
			}
			case RenderTextureCube::EFormat::RGBA32:
			{
				settings.mDataType = ESurfaceDataType::FLOAT;
				settings.mChannels = ESurfaceChannels::RGBA;
				break;
			}
			case RenderTextureCube::EFormat::R32:
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

		// Ensure texture can be used as an attachment for a render pass
		VkImageUsageFlags required_flags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		// Create render texture
		return TextureCube::init(settings, mGenerateLODs, mClearColor.toVec4(), required_flags, errorState);
	}


	//////////////////////////////////////////////////////////////////////////
	// DepthRenderTextureCube
	//////////////////////////////////////////////////////////////////////////

	DepthRenderTextureCube::DepthRenderTextureCube(Core& core) :
		TextureCube(core)
	{ }

	// Initializes depth cube texture. 
	bool DepthRenderTextureCube::init(utility::ErrorState& errorState)
	{
		SurfaceDescriptor settings;
		settings.mWidth = mWidth;
		settings.mHeight = mHeight;
		settings.mColorSpace = mColorSpace;
		settings.mChannels = ESurfaceChannels::D;

		const glm::vec4 clear_color = { mClearValue, mClearValue, mClearValue, mClearValue };

		// Ensure texture can be used as an attachment for a render pass
		VkImageUsageFlags required_flags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

		// Initialize based on selected format
		switch (mDepthFormat)
		{
			case DepthRenderTextureCube::EDepthFormat::D16:
			{
				settings.mDataType = ESurfaceDataType::USHORT;
				return TextureCube::init(settings, false, clear_color, required_flags, errorState);
			}
			case DepthRenderTextureCube::EDepthFormat::D32:
			{
				settings.mDataType = ESurfaceDataType::FLOAT;
				return TextureCube::init(settings, false, clear_color, required_flags, errorState);
			}
			default:
			{
				errorState.fail("Unsupported format");
				return false;
			}
		}
	}
}
