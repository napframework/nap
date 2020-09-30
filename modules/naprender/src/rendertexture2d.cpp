/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "rendertexture2d.h"
#include "nap/core.h"
#include "renderservice.h"

RTTI_BEGIN_ENUM(nap::RenderTexture2D::EFormat)
	RTTI_ENUM_VALUE(nap::RenderTexture2D::EFormat::RGBA8,	"RGBA8"),
	RTTI_ENUM_VALUE(nap::RenderTexture2D::EFormat::R8,		"R8")
RTTI_END_ENUM

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderTexture2D)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Width",		&nap::RenderTexture2D::mWidth,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Height",		&nap::RenderTexture2D::mHeight,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Format",		&nap::RenderTexture2D::mFormat,		nap::rtti::EPropertyMetaData::Required)
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

		// Figure out if the texture needs to be filled
		Texture2D::EClearMode clear_mode = mUsage == ETextureUsage::Static ? 
			Texture2D::EClearMode::DontClear :
			Texture2D::EClearMode::FillWithZero;

		// Initialize based on selected format
		switch (mFormat)
		{
			case RenderTexture2D::EFormat::RGBA8:
			{
				settings.mChannels = ESurfaceChannels::RGBA;
				return Texture2D::init(settings, false, clear_mode, errorState);
			}
			case RenderTexture2D::EFormat::R8:
			{
				settings.mChannels = ESurfaceChannels::R;
				return Texture2D::init(settings, false, clear_mode, errorState);
			}
			default:
			{
				errorState.fail("Unsupported format");
				return false;
			}
		}
	}
}
