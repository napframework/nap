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

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderTexture2D)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Width",		&nap::RenderTexture2D::mWidth,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Height",		&nap::RenderTexture2D::mHeight,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Format",		&nap::RenderTexture2D::mColorFormat,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("ColorSpace", &nap::RenderTexture2D::mColorSpace,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ClearColor", &nap::RenderTexture2D::mClearColor,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_ENUM(nap::DepthRenderTexture2D::EDepthFormat)
	RTTI_ENUM_VALUE(nap::DepthRenderTexture2D::EDepthFormat::D16,	"D16"),
	RTTI_ENUM_VALUE(nap::DepthRenderTexture2D::EDepthFormat::D32,	"D32")
RTTI_END_ENUM

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::DepthRenderTexture2D)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Fill",		&nap::DepthRenderTexture2D::mFill,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Width",		&nap::DepthRenderTexture2D::mWidth,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Height",		&nap::DepthRenderTexture2D::mHeight,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Format",		&nap::DepthRenderTexture2D::mDepthFormat,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("ColorSpace", &nap::DepthRenderTexture2D::mColorSpace,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ClearValue", &nap::DepthRenderTexture2D::mClearValue,	nap::rtti::EPropertyMetaData::Default)
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

		// Ensure texture can be used as an attachment for a render pass
		VkImageUsageFlags required_flags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		// Create render texture
		return Texture2D::init(settings, false, mClearColor.toVec4(), required_flags, errorState);
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

		const glm::vec4 clear_color = { mClearValue, mClearValue, mClearValue, mClearValue };

		// Ensure texture can be used as an attachment for a render pass
		VkImageUsageFlags required_flags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

		// Initialize based on selected format
		switch (mDepthFormat)
		{
		case DepthRenderTexture2D::EDepthFormat::D16:
		{
			settings.mDataType = ESurfaceDataType::USHORT;
			return Texture2D::init(settings, false, clear_color, required_flags, errorState);
		}
		case DepthRenderTexture2D::EDepthFormat::D32:
		{
			settings.mDataType = ESurfaceDataType::FLOAT;
			return Texture2D::init(settings, false, clear_color, required_flags, errorState);
		}
		default:
		{
			errorState.fail("Unsupported format");
			return false;
		}
		}
	}
}
