/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "bitmapfilebuffer.h"

// Local includes
#include "copyimagedata.h"

// External includes
#include <utility/fileutils.h>
#include <nap/logger.h>
#include <FreeImage.h>
#undef BYTE

RTTI_BEGIN_ENUM(nap::BitmapFileBuffer::EImageFileFormat)
	RTTI_ENUM_VALUE(nap::BitmapFileBuffer::EImageFileFormat::PNG, "PNG"),
	RTTI_ENUM_VALUE(nap::BitmapFileBuffer::EImageFileFormat::JPEG, "JPEG"),
	RTTI_ENUM_VALUE(nap::BitmapFileBuffer::EImageFileFormat::TIFF, "TIFF"),
	RTTI_ENUM_VALUE(nap::BitmapFileBuffer::EImageFileFormat::BMP, "BMP")
RTTI_END_ENUM

//////////////////////////////////////////////////////////////////////////

/**
 * Lookup table for FreeImage file formats
 */
static const std::unordered_map<nap::BitmapFileBuffer::EImageFileFormat, FREE_IMAGE_FORMAT> sImageFileFormatMap
{
	{ nap::BitmapFileBuffer::EImageFileFormat::PNG,		FREE_IMAGE_FORMAT::FIF_PNG },
	{ nap::BitmapFileBuffer::EImageFileFormat::JPEG,	FREE_IMAGE_FORMAT::FIF_JPEG },
	{ nap::BitmapFileBuffer::EImageFileFormat::TIFF,	FREE_IMAGE_FORMAT::FIF_TIFF },
	{ nap::BitmapFileBuffer::EImageFileFormat::BMP,		FREE_IMAGE_FORMAT::FIF_BMP }
};


/**
* Get the FreeImage format that corresponds to the specifies EImageFileFormat
* @param fileFormat the image file format
* @return the corresponding FreeImage file format
*/
FREE_IMAGE_FORMAT getFreeImageFormat(nap::BitmapFileBuffer::EImageFileFormat fileFormat)
{
	auto it = sImageFileFormatMap.find(fileFormat);
	if (it != sImageFileFormatMap.end())
		return it->second;

	return FREE_IMAGE_FORMAT::FIF_UNKNOWN;
}


/**
* Get the FreeImage type associated with the specified surface data type and channels
* @param dataType surface data type
* @param channels surface channels
* @return the FreeImage type associated with this surface data type and channels
*/
FREE_IMAGE_TYPE getFreeImageType(nap::ESurfaceDataType dataType, nap::ESurfaceChannels channels)
{
	switch (dataType)
	{
	case nap::ESurfaceDataType::BYTE:
		return FREE_IMAGE_TYPE::FIT_BITMAP;
	case nap::ESurfaceDataType::USHORT:
		return (channels != nap::ESurfaceChannels::R) ? FREE_IMAGE_TYPE::FIT_RGBA16 : FREE_IMAGE_TYPE::FIT_UINT16;
	case nap::ESurfaceDataType::FLOAT:
		return (channels != nap::ESurfaceChannels::R) ? FREE_IMAGE_TYPE::FIT_RGBAF : FREE_IMAGE_TYPE::FIT_FLOAT;
	default:
		return FREE_IMAGE_TYPE::FIT_UNKNOWN;
	}
}


namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// BitmapFileBuffer
	//////////////////////////////////////////////////////////////////////////

	BitmapFileBuffer::BitmapFileBuffer()
	{}

	BitmapFileBuffer::BitmapFileBuffer(const Bitmap& bitmap, bool copyData)
	{
		// Ensure the bitmap is allocated
		assert(!bitmap.empty());

		utility::ErrorState error_state;
		if (!setData(bitmap.mSurfaceDescriptor, bitmap.getData(), copyData, error_state))
		{
			nap::Logger::error(error_state.toString());
			assert(false);
		}
	}

	BitmapFileBuffer::BitmapFileBuffer(const SurfaceDescriptor& surfaceDescriptor, const void* data, bool copyData)
	{
		utility::ErrorState error_state;
		if (!setData(surfaceDescriptor, data, copyData, error_state))
		{
			nap::Logger::error(error_state.toString());
			assert(false);
		}
	}


	BitmapFileBuffer::BitmapFileBuffer(const SurfaceDescriptor& surfaceDescriptor)
	{
		utility::ErrorState error_state;
		if (!allocate(surfaceDescriptor, error_state))
		{
			nap::Logger::error(error_state.toString());
			assert(false);
		}
	}


	bool BitmapFileBuffer::allocate(const SurfaceDescriptor& surfaceDescriptor, utility::ErrorState& errorState)
	{
		FREE_IMAGE_TYPE fi_image_type = getFreeImageType(surfaceDescriptor.mDataType, surfaceDescriptor.mChannels);
		int bpp = surfaceDescriptor.getBytesPerPixel() * 8;

		FIBITMAP* fi_bitmap = FreeImage_AllocateT(fi_image_type, surfaceDescriptor.mWidth, surfaceDescriptor.mHeight, bpp,
			FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK);

		if (!errorState.check(fi_bitmap != NULL, "Failed to allocate bitmap file buffer"))
			return false;

		mBitmapHandle = fi_bitmap;
		return true;
	}


	bool BitmapFileBuffer::setData(const SurfaceDescriptor& surfaceDescriptor, const void* data, bool copyData, utility::ErrorState& errorState)
	{
		// Get the FreeImage type
		FREE_IMAGE_TYPE fi_img_type = getFreeImageType(surfaceDescriptor.mDataType, surfaceDescriptor.mChannels);
		int bpp = surfaceDescriptor.getBytesPerPixel() * 8;
		int pitch = surfaceDescriptor.getPitch();

		// Wrap bitmap data with FIBITMAP header
		// Note that color masks are only supported for 16-bit RGBA images and ignored for any other color depth
		FIBITMAP* fi_bitmap = FreeImage_ConvertFromRawBitsEx(
			copyData, (uint8*)data, fi_img_type, surfaceDescriptor.getWidth(), surfaceDescriptor.getHeight(), pitch, bpp,
			FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK);

		if (!errorState.check(fi_bitmap != NULL, "Failed to allocate and create bitmap file buffer"))
			return false;

		mBitmapHandle = fi_bitmap;
		return true;
	}


	bool BitmapFileBuffer::load(const std::string& path, SurfaceDescriptor& outSurfaceDescriptor, utility::ErrorState& errorState)
	{
		if (!errorState.check(utility::fileExists(path), "File does not exist: %s", path.c_str()))
			return false;

		// Get format
		FREE_IMAGE_FORMAT fi_img_format = FreeImage_GetFIFFromFilename(path.c_str());
		if (!errorState.check(fi_img_format != FIF_UNKNOWN, "Unable to determine image format of file: %s", path.c_str()))
			return false;

		// Unload existing data
		if (mBitmapHandle != nullptr)
		{
			FIBITMAP* fi_bitmap = reinterpret_cast<FIBITMAP*>(mBitmapHandle);
			FreeImage_Unload(fi_bitmap);
		}

		// Load new data
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

		// Set the surface descriptor
		outSurfaceDescriptor.mWidth = FreeImage_GetWidth(fi_bitmap);
		outSurfaceDescriptor.mHeight = FreeImage_GetHeight(fi_bitmap);
		outSurfaceDescriptor.mDataType = data_type;
		outSurfaceDescriptor.mChannels = channels;

		mBitmapHandle = fi_bitmap;
		return true;
	}


	bool BitmapFileBuffer::save(const std::string& path, utility::ErrorState& errorState)
	{
		// Ensure the bitmap data is allocated
		if (!errorState.check(mBitmapHandle != nullptr, "Unallocated bitmap file buffer"))
			return false;

		// Cast to a FreeImage bitmap handle
		FIBITMAP* fi_bitmap = reinterpret_cast<FIBITMAP*>(mBitmapHandle);

		// Check if directory exists
		std::string path_filedir = utility::getFileDir(path);
		if (!utility::dirExists(path_filedir))
		{
			errorState.fail("Directory does not exist: %s", path.c_str());
			return false;
		}

		// Get free image format from file format
		std::string ext = utility::getFileExtension(path).c_str();

		FREE_IMAGE_FORMAT fi_img_format = FreeImage_GetFIFFromFormat(ext.c_str());
		if (!errorState.check(fi_img_format != FIF_UNKNOWN, "Unable to determine image format: %s", ext.c_str()))
			return false;

		// Convert to 24-bit for jpegs
		if (fi_img_format == FREE_IMAGE_FORMAT::FIF_JPEG)
		{
			if (!FreeImage_PreMultiplyWithAlpha(fi_bitmap))
			{
				errorState.fail("Can't premultiply with alpha");
				return false;
			}
			FIBITMAP* fi_bitmap_converted = FreeImage_ConvertTo24Bits(fi_bitmap);
			if (!errorState.check(FreeImage_Save(fi_img_format, fi_bitmap_converted, path.c_str()), "Image could not be saved"))
				return false;

			FreeImage_Unload(fi_bitmap_converted);
		}
		else
		{
			FREE_IMAGE_TYPE fi_type = FreeImage_GetImageType(fi_bitmap);
			if (!errorState.check(FreeImage_FIFSupportsWriting(fi_img_format) || FreeImage_FIFSupportsExportType(fi_img_format, fi_type),
				utility::stringFormat("Cannot write bitmap data type to .%s", ext.c_str())))
			{
				return false;
			}
			if (!errorState.check(FreeImage_Save(fi_img_format, fi_bitmap, path.c_str()), "Image could not be saved"))
				return false;
		}
		return true;
	}


	void* BitmapFileBuffer::getData()
	{
		FIBITMAP* fi_bitmap = reinterpret_cast<FIBITMAP*>(getHandle());
		return FreeImage_GetBits(fi_bitmap);
	}


	void* BitmapFileBuffer::getHandle()
	{
		assert(mBitmapHandle != nullptr);
		return mBitmapHandle;
	}


	void BitmapFileBuffer::release()
	{
		if (mBitmapHandle != nullptr)
		{
			FIBITMAP* fi_bitmap = reinterpret_cast<FIBITMAP*>(mBitmapHandle);
			FreeImage_Unload(fi_bitmap);

			mBitmapHandle = nullptr;
		}
	}


	BitmapFileBuffer::~BitmapFileBuffer()
	{
		release();
	}
}

