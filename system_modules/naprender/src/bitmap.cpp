/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "bitmap.h"

// External includes
#include <utility/fileutils.h>
#include <rtti/typeinfo.h>
#include <texture.h>

#include "bitmapfilebuffer.h"
#include "copyimagedata.h"

#include <FreeImage.h>
#undef BYTE

// nap::bitmap run time class definition 
RTTI_BEGIN_CLASS(nap::Bitmap, "2D image resource without GPU representation")
	RTTI_PROPERTY("Settings",	&nap::Bitmap::mSurfaceDescriptor,	nap::rtti::EPropertyMetaData::Default, "Bitmap initialization settings")
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::BitmapFromFile, "2D image resource loaded from disk, without GPU representation")
	RTTI_PROPERTY("Path",		&nap::BitmapFromFile::mPath,		nap::rtti::EPropertyMetaData::Required, "Path to the image on disk")
RTTI_END_CLASS


//////////////////////////////////////////////////////////////////////////
// Color creation functions / fill functions
//////////////////////////////////////////////////////////////////////////

/**
 * Helper function that creates a color based on the data associated with 
 * @param map the bitmap to get the color values from
 * @param x the x pixel coordinate value
 * @param y the y pixel coordinate value
 */
template<typename T>
static nap::BaseColor* createColor(const nap::Bitmap& map)
{
	switch (map.getNumberOfChannels())
	{
	case 1:
	{
		nap::RColor<T>* color = new nap::RColor<T>();
		return color;
	}
	case 3:
	{
		nap::RGBColor<T>* color = new nap::RGBColor<T>();
		return color;
	}
	case 4:
	{
		nap::RGBAColor<T>* color = new nap::RGBAColor<T>();
		return color;
	}
	default:
		assert(false);
	}
	return nullptr;
}


/**
 * Helper function that fills outColor with the color values stored in the map
 * @param x the horizontal pixel coordinate
 * @param y the vertical pixel coordinate
 * @param outColor the associated pixel color values
 */
template<typename T>
static void fill(int x, int y, const nap::Bitmap& map, nap::BaseColor& outColor)
{
	assert(!(outColor.isPointer()));
	switch (outColor.getNumberOfChannels())
	{
	case 1:
	{
		nap::RColor<T>* clr = rtti_cast<nap::RColor<T>>(&outColor);
		assert(clr != nullptr);
		map.getColorValue<T>(x, y, nap::EColorChannel::Red, *clr);
		break;
	}
	case 3:
	{
		nap::RGBColor<T>* clr = rtti_cast<nap::RGBColor<T>>(&outColor);
		assert(clr != nullptr);
		map.getRGBColor<T>(x, y, *clr);
		break;
	}
	case 4:
	{
		nap::RGBAColor<T>* clr = rtti_cast<nap::RGBAColor<T>>(&outColor);
		assert(clr != nullptr);
		map.getRGBAColor<T>(x, y, *clr);
		break;
	}
	default:
		assert(false);
	}
}


/**
* Helper function that creates a color that stores the location of the color values
* @param map the bitmap to get the color values from
* @param x the x pixel coordinate value
* @param y the y pixel coordinate value
*/
template<typename T>
static nap::BaseColor* createColorData(const nap::Bitmap& map, int x, int y)
{
	switch (map.getNumberOfChannels())
	{
	case 1:
	{
		nap::RColor<T*>* color = new nap::RColor<T*>();
		map.getColorValueData<T>(x, y, nap::EColorChannel::Red, *color);
		return color;
	}
	case 3:
	{
		nap::RGBColor<T*>* color = new nap::RGBColor<T*>();
		map.getRGBColorData<T>(x, y, *color);
		return color;
	}
	case 4:
	{
		nap::RGBAColor<T*>* color = new nap::RGBAColor<T*>();
		map.getRGBAColorData<T>(x, y, *color);
		return color;
	}
	default:
		assert(false);
	}
	return nullptr;
}


namespace nap
{
	Bitmap::~Bitmap()			{ }


	bool Bitmap::init(utility::ErrorState& errorState)
	{
		if (!errorState.check(getWidth() > 0 && getHeight() > 0, "Invalid size specified for bitmap"))
			return false;

		updatePixelFormat();
		mData.resize(getSizeInBytes());

		return true;
	}


	bool Bitmap::initFromFile(const std::string& path, nap::utility::ErrorState& errorState)
	{
		BitmapFileBuffer file_buffer;
		if (!file_buffer.load(path, mSurfaceDescriptor, errorState))
		{
			errorState.fail("Failed to load bitmap from disk");
			return false;
		}

		updatePixelFormat();
		mData.resize(getSizeInBytes());
		copyImageData(
			(const uint8*)file_buffer.getData(), mSurfaceDescriptor.getPitch(), mSurfaceDescriptor.getChannels(),
			mData.data(), mSurfaceDescriptor.getPitch(), mSurfaceDescriptor.getChannels(),
			getWidth(), getHeight());

		return true;
	}


	void Bitmap::initFromDescriptor(const SurfaceDescriptor& surfaceDescriptor)
	{
		mSurfaceDescriptor = surfaceDescriptor;
		updatePixelFormat();
		uint64_t size = getSizeInBytes();
		mData.resize(size);
	}


	bool Bitmap::save(const std::string& path, utility::ErrorState& errorState)
	{
		// Check if the bitmap is allocated
		if (!errorState.check(!empty(), "Bitmap is not allocated"))
			return false;

		// Wrap the bitmap data and write to disk
		BitmapFileBuffer file_buffer{ *this, false };
		if (!file_buffer.save(path, errorState)) 
		{
			errorState.fail("Failed to write bitmap to disk");
			return false;
		}
		return true;
	}


	size_t Bitmap::getSizeInBytes() const
	{
		return mSurfaceDescriptor.getSizeInBytes();
	}


	std::unique_ptr<nap::BaseColor> Bitmap::makePixel() const
	{
		BaseColor* rvalue = nullptr;
		switch (mSurfaceDescriptor.getDataType())
		{
		case ESurfaceDataType::BYTE:
		{
			rvalue = createColor<uint8>(*this);
			break;
		}
		case ESurfaceDataType::FLOAT:
		{
			rvalue = createColor<float>(*this);
			break;
		}
		case ESurfaceDataType::USHORT:
		{
			rvalue = createColor<uint16>(*this);
			break;
		}
		default:
			assert(false);
			break;
		}
		assert(rvalue != nullptr);
		return std::unique_ptr<BaseColor>(rvalue);
	}


	void Bitmap::getPixel(int x, int y, BaseColor& outPixel) const
	{
		switch (mSurfaceDescriptor.getDataType())
		{
		case ESurfaceDataType::BYTE:
		{
			fill<uint8>(x, y, *this, outPixel);
			break;
		}
		case ESurfaceDataType::FLOAT:
		{
			fill<float>(x, y, *this, outPixel);
			break;
		}
		case ESurfaceDataType::USHORT:
		{
			fill<uint16>(x, y, *this, outPixel);
			break;
		}
		default:
			assert(false);
			break;
		}
	}


	void Bitmap::setPixel(int x, int y, const BaseColor& color)
	{
		switch (mSurfaceDescriptor.getDataType())
		{
		case ESurfaceDataType::BYTE:
		{
			setPixelData<uint8>(x, y, color);
			break;
		}
		case ESurfaceDataType::FLOAT:
		{
			setPixelData<float>(x, y, color);
			break;
		}
		case ESurfaceDataType::USHORT:
		{
			setPixelData<uint16>(x, y, color);
			break;
		}
		default:
			assert(false);
			break;
		}
	}


	void Bitmap::updatePixelFormat()
	{
		std::unique_ptr<BaseColor> temp_clr = makePixel();
		mColorType = temp_clr->get_type().get_raw_type();
		mValueType = temp_clr->getValueType();
	}
}


bool nap::BitmapFromFile::init(utility::ErrorState& errorState)
{
	return Bitmap::initFromFile(mPath, errorState);
}
