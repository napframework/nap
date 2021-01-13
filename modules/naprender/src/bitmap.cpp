/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "bitmap.h"

// External includes
#include <utility/fileutils.h>
#include <rtti/typeinfo.h>
#include <texture2d.h>
#include "copyimagedata.h"

// External includes
#include <FreeImage.h>

#undef BYTE

// nap::bitmap run time class definition 
RTTI_BEGIN_CLASS(nap::Bitmap)
	RTTI_PROPERTY("Settings",	&nap::Bitmap::mSurfaceDescriptor,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::BitmapFromFile)
	RTTI_PROPERTY("Path",		&nap::BitmapFromFile::mPath,	nap::rtti::EPropertyMetaData::Required)
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
		if (!errorState.check(utility::fileExists(path), "unable to load image: %s, file does not exist: %s", path.c_str(), mID.c_str()))
			return false;

		// Get format
		FREE_IMAGE_FORMAT fi_img_format = FreeImage_GetFIFFromFilename(path.c_str());
		if (!errorState.check(fi_img_format != FIF_UNKNOWN, "Unable to determine image format of file: %s", path.c_str()))
			return false;

		// Load
		FIBITMAP* fi_bitmap = FreeImage_Load(fi_img_format, path.c_str());
		if (!errorState.check(fi_bitmap != nullptr, "Unable to load bitmap: %s", path.c_str()))
		{
			FreeImage_Unload(fi_bitmap);
			return false;
		}

		// Get associated bitmap type for free image type
		FREE_IMAGE_TYPE fi_bitmap_type = FreeImage_GetImageType(fi_bitmap);

		ESurfaceDataType data_type;
		switch (fi_bitmap_type)
		{
		case FIT_BITMAP:
			data_type = ESurfaceDataType::BYTE;
			break;
		case FIT_UINT16:
		case FIT_RGB16:
		case FIT_RGBA16:
			data_type = ESurfaceDataType::USHORT;
			break;
		case FIT_FLOAT:
		case FIT_RGBF:
		case FIT_RGBAF:
			data_type = ESurfaceDataType::FLOAT;
			break;
		default:
			errorState.fail("Can't load bitmap from file; unknown pixel format");
			FreeImage_Unload(fi_bitmap);
			return false;
		}
		
		// Get color type
		FREE_IMAGE_COLOR_TYPE fi_bitmap_color_type = FreeImage_GetColorType(fi_bitmap);
		if (fi_bitmap_color_type == FIC_RGB)
		{
			FIBITMAP* converted_bitmap = FreeImage_ConvertTo32Bits(fi_bitmap);
			FreeImage_Unload(fi_bitmap);
			fi_bitmap = converted_bitmap;
			fi_bitmap_color_type = FIC_RGBALPHA;
		}

		// If we're dealing with an rgb or rgba map and a bitmap
		// The endian of the loaded free image map becomes important
		// If so we might have to swap the red and blue channels regarding the internal color representation
		bool swap = (fi_bitmap_type == FREE_IMAGE_TYPE::FIT_BITMAP && FI_RGBA_RED == 2);

		ESurfaceChannels channels;
		switch (fi_bitmap_color_type)
		{
		case FIC_MINISBLACK:
			channels = ESurfaceChannels::R;
			break;
		case FIC_RGBALPHA:
			channels = swap ? ESurfaceChannels::BGRA : ESurfaceChannels::RGBA;
			break;
		default:
			errorState.fail("Can't load bitmap from file; unknown pixel format");
			FreeImage_Unload(fi_bitmap);
			return false;
		}

		int width = FreeImage_GetWidth(fi_bitmap);
		int height = FreeImage_GetHeight(fi_bitmap);

		// copy image data
		mSurfaceDescriptor = SurfaceDescriptor(width, height, data_type, channels);
		updatePixelFormat();
		mData.resize(getSizeInBytes());
		copyImageData(FreeImage_GetBits(fi_bitmap), FreeImage_GetPitch(fi_bitmap), channels, mData.data(), mSurfaceDescriptor.getPitch(), mSurfaceDescriptor.getChannels(), getWidth(), getHeight());
		FreeImage_Unload(fi_bitmap);
		return true;
	}


	void Bitmap::initFromDescriptor(const SurfaceDescriptor& surfaceDescriptor)
	{
		mSurfaceDescriptor = surfaceDescriptor;
		updatePixelFormat();
		uint64_t size = getSizeInBytes();
		mData.resize(size);
	}


	bool Bitmap::writeToDisk(const std::string filename, utility::ErrorState& errorState)
	{
		// Check if image is allocated
		if (!errorState.check(!empty(), "Bitmap is not allocated"))
			return false;

		// Get format
		FREE_IMAGE_FORMAT fi_img_format = FreeImage_GetFIFFromFilename(filename.c_str());
		if (!errorState.check(fi_img_format != FIF_UNKNOWN, "Unable to determine image format"))
			return false;

		int bpp = mSurfaceDescriptor.getBytesPerPixel() * 8;
		int pitch = ((((bpp * getWidth()) + 31) / 32) * 4);

		// FreeImage_ConvertFromRawBits should be able to set R and G bytes accordingly
		// but it does not seem to work for me.
		FIBITMAP* fi_bitmap = FreeImage_ConvertFromRawBits(
			mData.data(), getWidth(), getHeight(), pitch, bpp,
			FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK
		);

		// Check if source and target channels match
		bool isLittleEndian = FI_RGBA_RED == 2;
		ESurfaceChannels targetChannels = isLittleEndian ? ESurfaceChannels::BGRA : ESurfaceChannels::RGBA;

		// Convert RGBA to BGRA or vice versa manually by looking up the internal bitmap
		if (mSurfaceDescriptor.getChannels() != targetChannels) {
			const uint8_t* source_line = mData.data();
			uint8_t* target_line = FreeImage_GetBits(fi_bitmap);

			// Get the amount of bytes every pixel occupies
			int source_stride = pitch / getWidth();
			int target_stride = source_stride;

			for (int y = 0; y < getHeight(); ++y)
			{
				const uint8_t* source_loc = source_line;
				uint8_t* target_loc = target_line;
				for (int x = 0; x < getWidth(); ++x)
				{
					*target_loc = *(source_loc+2); // B
					*(target_loc+2) = *source_loc; // R

					target_loc += target_stride;
					source_loc += source_stride;
				}
				source_line += pitch;
				target_line += pitch;
			}
		}
		else {
			memcpy(FreeImage_GetBits(fi_bitmap), mData.data(), getSizeInBytes());
		}

		// Screenshot output path
		const std::string screenshotDir = utility::joinPath({ utility::getExecutableDir(), "screenshots" }, utility::pathSep());
		if (!utility::dirExists(screenshotDir)) {
			utility::makeDirs(screenshotDir);
		}
		std::string outputPath = utility::joinPath({ screenshotDir, filename }, utility::pathSep());

		// Save and free
		if (!errorState.check(FreeImage_Save(fi_img_format, fi_bitmap, outputPath.c_str()), "Image could not be saved"))
			return false;

		FreeImage_Unload(fi_bitmap);
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
