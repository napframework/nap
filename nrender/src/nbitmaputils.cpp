#include "nbitmaputils.h"
#include "nglutils.h"
#include "utility/errorstate.h"
#include <assert.h>
#include <algorithm>

namespace opengl
{
	/**
	* freeImageTypeMap
	*
	* Holds all bitmap to free image type conversions
	* Note that every 8 bit image is a FIT_BITMAP in freeImage
	*/
	using FreeImageTypeMap = std::unordered_map<BitmapDataType, std::vector<FREE_IMAGE_TYPE>>;
	static const FreeImageTypeMap freeImageTypeMap =
	{
		{ BitmapDataType::BYTE,		{ FIT_BITMAP }},
		{ BitmapDataType::USHORT,	{ FIT_UINT16, FIT_RGB16, FIT_RGBA16}},
		{ BitmapDataType::FLOAT,	{ FIT_FLOAT, FIT_RGBF, FIT_RGBAF}}
	};


	/**
	 * freeImageColorMap
	 *
	 * Holds all bitmap color to free image color type conversions
	 * If there are multiple vector entries, the second entry is always the little endian version
	 * TODO: Figure out something nicer for this lookup
	 */
	using FreeImageColorMap = std::unordered_map<FREE_IMAGE_COLOR_TYPE, BitmapColorType>;
	static const FreeImageColorMap freeImageColorMap
	{
		{ FIC_MINISBLACK,	BitmapColorType::GREYSCALE },
		{ FIC_RGB,			BitmapColorType::RGB },
		{ FIC_RGBALPHA,		BitmapColorType::RGBA },
		{ FIC_PALETTE,		BitmapColorType::INDEXED}
	};


	//Returns the associated Bitmap Data Type for the queried free image type
	opengl::BitmapDataType getBitmapType(FREE_IMAGE_TYPE type)
	{
		auto found_it = std::find_if(freeImageTypeMap.begin(), freeImageTypeMap.end(), [&](const auto& value)
		{
			bool found(false);
			for (const auto& v : value.second)
			{
				if (v == type)
				{
					found = true;
					break;
				}
			}
			return found;
		});
		return found_it == freeImageTypeMap.end() ? BitmapDataType::UNKNOWN : (*found_it).first;
	}


	/**
	 * getColorType
	 *
	 * Figures out what type of color belongs to the free image color type
	 * A bitmap loaded with free image can have a different channel ordering based on the endianness of the system
	 * For that reason it's important to know the image's data type before returning the color type to use
	 * If the free image bitmap is of type FIT_BITMAP and has an RGB or RGBA color scheme
	 * The channels might be ordered differently base don little / big endian
	 * This function compensates for that ordering and returns the correct color type based on the difference in architecture
	 */
	opengl::BitmapColorType getColorType(FREE_IMAGE_COLOR_TYPE colorType, FREE_IMAGE_TYPE dataType)
	{
		// Find based on color key
		auto it = freeImageColorMap.find(colorType);

		// If no match was found return
		if (it == freeImageColorMap.end())
			return BitmapColorType::UNKNOWN;

		// If we're dealing with an rgb or rgba map and a bitmap
		// The endian of the loaded free image map becomes important
		// If so we might have to swap the red and blue channels regarding the interal color representation
		bool swap = (dataType == FREE_IMAGE_TYPE::FIT_BITMAP && FI_RGBA_RED == 2);

		// Based on type, swap if necessary
		BitmapColorType return_type = it->second;
		switch (it->second)
		{
		case BitmapColorType::RGB:
			return_type = swap ? BitmapColorType::BGR : BitmapColorType::RGB;
			break;
		case BitmapColorType::RGBA:
			return_type = swap ? BitmapColorType::BGRA : BitmapColorType::RGBA;
			break;
		default:
			break;
		}

		// Swapping is of no importance
		return return_type;
	}


	// Loads bitmap from image file path
	Bitmap* loadBitmap(const std::string& imgPath, nap::utility::ErrorState& errorState)
	{
		// Create a new bitmap
		opengl::Bitmap* nbitmap = new Bitmap();
		if (!loadBitmap(*nbitmap, imgPath, errorState))
		{
			delete nbitmap;
			return nullptr;
		}
		return nbitmap;
	}


	// loads a bitmap from file
	bool loadBitmap(Bitmap& bitmap, const std::string& imgPath, nap::utility::ErrorState& errorState)
	{
		// Get format
		FREE_IMAGE_FORMAT fi_img_format = FreeImage_GetFIFFromFilename(imgPath.c_str());
		if (!errorState.check(fi_img_format != FIF_UNKNOWN, "Unable to determine image format of file: %s", imgPath.c_str()))
			return false;

		// Load
		FIBITMAP* fi_bitmap = FreeImage_Load(fi_img_format, imgPath.c_str());
		if (!errorState.check(fi_bitmap != nullptr, "Unable to load bitmap: %s", imgPath.c_str()))
		{
			FreeImage_Unload(fi_bitmap);
			return false;
		}

		// Get associated bitmap type for free image type
		FREE_IMAGE_TYPE fi_bitmap_type = FreeImage_GetImageType(fi_bitmap);
		BitmapDataType bitmap_type = getBitmapType(fi_bitmap_type);
		if (!errorState.check(bitmap_type != BitmapDataType::UNKNOWN, "Unable to find matching bitmap type for free image type: %d", static_cast<int>(fi_bitmap_type)))
		{
			FreeImage_Unload(fi_bitmap);
			return false;
		}

		// Get color type
		FREE_IMAGE_COLOR_TYPE fi_bitmap_color_type = FreeImage_GetColorType(fi_bitmap);
		BitmapColorType bitmap_color = getColorType(fi_bitmap_color_type, fi_bitmap_type);
		if (!errorState.check(bitmap_color != BitmapColorType::UNKNOWN, "Unable to find matching bitmap color type for free image type: %d", static_cast<int>(fi_bitmap_color_type)))
		{
			FreeImage_Unload(fi_bitmap);
			return false;
		}
		
		// Copy free image data in our own bitmap container
		uint8_t* fi_data = FreeImage_GetBits(fi_bitmap);		
		assert(fi_data != nullptr);		
		bool success = bitmap.copyData(FreeImage_GetWidth(fi_bitmap), FreeImage_GetHeight(fi_bitmap), bitmap_type, bitmap_color, fi_data, FreeImage_GetPitch(fi_bitmap));
		assert(success);

		FreeImage_Unload(fi_bitmap);
		return success;
	}
}