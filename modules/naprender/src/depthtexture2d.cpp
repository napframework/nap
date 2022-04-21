/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "depthtexture2d.h"
#include "nap/core.h"
#include "renderservice.h"

RTTI_BEGIN_ENUM(nap::DepthTexture2D::EDepthFormat)
	RTTI_ENUM_VALUE(nap::DepthTexture2D::EDepthFormat::D8, "D8"),
	RTTI_ENUM_VALUE(nap::DepthTexture2D::EDepthFormat::D16, "D16"),
	RTTI_ENUM_VALUE(nap::DepthTexture2D::EDepthFormat::D32, "D32")
RTTI_END_ENUM

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::DepthTexture2D)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Fill",		&nap::DepthTexture2D::mFill,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Width",		&nap::DepthTexture2D::mWidth,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Height",		&nap::DepthTexture2D::mHeight,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Format",		&nap::DepthTexture2D::mFormat,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("ColorSpace", &nap::DepthTexture2D::mColorSpace,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ClearValue", &nap::DepthTexture2D::mClearValue,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
	DepthTexture2D::DepthTexture2D(Core& core) :
		Texture2D(core)
	{
	}

	// Initializes 2D texture. 
	bool DepthTexture2D::init(utility::ErrorState& errorState)
	{
		SurfaceDescriptor settings;
		settings.mWidth = mWidth;
		settings.mHeight = mHeight;
		settings.mColorSpace = mColorSpace;
		settings.mChannels = ESurfaceChannels::D;

		// Initialize based on selected format
		switch (mFormat)
		{
			case DepthTexture2D::EDepthFormat::D8:
			{
				settings.mDataType = ESurfaceDataType::BYTE;
				return Texture2D::init(settings, false, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, errorState);
			}
			case DepthTexture2D::EDepthFormat::D16:
			{
				settings.mDataType = ESurfaceDataType::USHORT;
				return Texture2D::init(settings, false, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, errorState);
			}
			case DepthTexture2D::EDepthFormat::D32:
			{
				settings.mDataType = ESurfaceDataType::FLOAT;
				return Texture2D::init(settings, false, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, errorState);
			}
			default:
			{
				errorState.fail("Unsupported format");
				return false;
			}
		}
	}
}
