/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "bitmaputils.h"
#include <utility/fileutils.h>
#include <datetime.h>

namespace nap
{
	namespace utility
	{
		FREE_IMAGE_TYPE getFIType(int bpp, int channels)
		{
			if (channels == 4) {
				return	(bpp == 32) ? FREE_IMAGE_TYPE::FIT_BITMAP :
						(bpp == 64) ? FREE_IMAGE_TYPE::FIT_RGBA16 :
						(bpp == 128) ? FREE_IMAGE_TYPE::FIT_RGBAF : FREE_IMAGE_TYPE::FIT_UNKNOWN;
			}
			if (channels == 1) {
				return	(bpp == 8) ? FREE_IMAGE_TYPE::FIT_BITMAP :
						(bpp == 16) ? FREE_IMAGE_TYPE::FIT_UINT16 :
						(bpp == 32) ? FREE_IMAGE_TYPE::FIT_FLOAT : FREE_IMAGE_TYPE::FIT_UNKNOWN;
			}
			return FREE_IMAGE_TYPE::FIT_UNKNOWN;
		}


		FREE_IMAGE_TYPE getFIType(ESurfaceDataType dataType, ESurfaceChannels channels)
		{
			switch (dataType) {
			case ESurfaceDataType::BYTE:
				return FREE_IMAGE_TYPE::FIT_BITMAP;
			case ESurfaceDataType::USHORT:
				return (channels != ESurfaceChannels::R) ? FREE_IMAGE_TYPE::FIT_RGBA16 : FREE_IMAGE_TYPE::FIT_UINT16;
			case ESurfaceDataType::FLOAT:
				return (channels != ESurfaceChannels::R) ? FREE_IMAGE_TYPE::FIT_RGBAF : FREE_IMAGE_TYPE::FIT_FLOAT;
			default: 
				return FREE_IMAGE_TYPE::FIT_UNKNOWN;
			}
		}


		bool getFIBitmapInfo(int bpp, int channels, uint32_t width, uint32_t height, FIBitmapInfo& outInfo)
		{
			// Do not support pixel formats under 8 bits
			if (bpp < 8) return false;

			int bytes_per_pixel = bpp / 8;
			outInfo.bpp = bpp;
			outInfo.pitch = width*bytes_per_pixel;
			outInfo.bytes = outInfo.pitch*height;
			outInfo.type = getFIType(bpp, channels);
			return true;
		}

		/**
		 * Write a bitmap to a specified location on disk
		 * @param fiBitmap a FreeImage bitmap handle
		 * @param fiType the FreeImage bitmap type
		 * @param path destination location on disk. Should include an extension (e.g. 'myBitmap.png')
		 * @errorState the error state
		 */
		bool writeToDisk(FIBITMAP* fiBitmap, FREE_IMAGE_TYPE fiType, const std::string& path, utility::ErrorState& errorState)
		{
			// Check if directory exists
			if (!errorState.check(utility::dirExists(utility::getFileDir(path)), "Directory does not exist: %s", path.c_str()))
				return false;

			// Get format
			std::string ext = utility::getFileExtension(path).c_str();
			FREE_IMAGE_FORMAT fi_img_format = FreeImage_GetFIFFromFormat(ext.c_str());
			if (!errorState.check(fi_img_format != FIF_UNKNOWN, "Unable to determine image format: %s", ext.c_str()))
				return false;

			// Convert to 24-bit for jpegs
			if (fi_img_format == FREE_IMAGE_FORMAT::FIF_JPEG) {
				if (!FreeImage_PreMultiplyWithAlpha(fiBitmap)) {
					errorState.fail("Can't premultiply with alpha");
					return false;
				}
				FIBITMAP* fi_bitmap_converted = FreeImage_ConvertTo24Bits(fiBitmap);
				if (!errorState.check(FreeImage_Save(fi_img_format, fi_bitmap_converted, path.c_str()), "Image could not be saved"))
					return false;

				FreeImage_Unload(fi_bitmap_converted);
			}
			else {
				if (!errorState.check(FreeImage_FIFSupportsWriting(fi_img_format) || FreeImage_FIFSupportsExportType(fi_img_format, fiType),
					utility::stringFormat("Cannot write bitmap data type to .%s", ext.c_str()))) {
					return false;
				}
				if (!errorState.check(FreeImage_Save(fi_img_format, fiBitmap, path.c_str()), "Image could not be saved"))
					return false;
			}
			return true;
		}
	}
}
