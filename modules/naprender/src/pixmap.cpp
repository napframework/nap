#include "pixmap.h"

// External includes
#include <nbitmaputils.h>
#include <utility/fileutils.h>
#include <rtti/typeinfo.h>
#include <basetexture2d.h>

RTTI_BEGIN_ENUM(nap::Pixmap::EChannels)
	RTTI_ENUM_VALUE(nap::Pixmap::EChannels::R,			"R"),
	RTTI_ENUM_VALUE(nap::Pixmap::EChannels::RGB,		"RGB"),
	RTTI_ENUM_VALUE(nap::Pixmap::EChannels::RGBA,		"RGBA"),
	RTTI_ENUM_VALUE(nap::Pixmap::EChannels::BGR,		"BGR"),
	RTTI_ENUM_VALUE(nap::Pixmap::EChannels::BGRA,		"BGRA")
RTTI_END_ENUM

RTTI_BEGIN_ENUM(nap::Pixmap::EDataType)
	RTTI_ENUM_VALUE(nap::Pixmap::EDataType::BYTE,		"Byte"),
	RTTI_ENUM_VALUE(nap::Pixmap::EDataType::USHORT,		"Short"),
	RTTI_ENUM_VALUE(nap::Pixmap::EDataType::FLOAT,		"Float")
RTTI_END_ENUM

// nap::bitmap run time class definition 
RTTI_BEGIN_CLASS(nap::Pixmap)
	RTTI_PROPERTY("Width",		&nap::Pixmap::mWidth,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Height",		&nap::Pixmap::mHeight,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Channels",	&nap::Pixmap::mChannels,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Type",		&nap::Pixmap::mType,			nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::PixmapFromFile)
	RTTI_PROPERTY("Path",		&nap::PixmapFromFile::mPath,	nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////

/**
* bitmapChannelMap
* Maps Bitmap channels to opengl bitmap channels
*/
using BitmapChannelMap = std::unordered_map<nap::Pixmap::EChannels, opengl::BitmapColorType>;
static const BitmapChannelMap bitmapChannelMap =
{
	{ nap::Pixmap::EChannels::R,					opengl::BitmapColorType::GREYSCALE },
	{ nap::Pixmap::EChannels::RGB,					opengl::BitmapColorType::RGB },
	{ nap::Pixmap::EChannels::RGBA,					opengl::BitmapColorType::RGBA },
	{ nap::Pixmap::EChannels::BGR,					opengl::BitmapColorType::BGR },
	{ nap::Pixmap::EChannels::BGRA,					opengl::BitmapColorType::BGRA }
};


static opengl::BitmapColorType getBitmapColorType(nap::Pixmap::EChannels colorType)
{
	auto it = bitmapChannelMap.find(colorType);
	assert(it != bitmapChannelMap.end());
	return it->second;
}


using BitmapDataTypeMap = std::unordered_map<nap::Pixmap::EDataType, opengl::BitmapDataType>;
static const BitmapDataTypeMap bitmapDataTypeMap =
{
	{ nap::Pixmap::EDataType::BYTE,					opengl::BitmapDataType::BYTE  },
	{ nap::Pixmap::EDataType::USHORT,				opengl::BitmapDataType::USHORT },
	{ nap::Pixmap::EDataType::FLOAT,				opengl::BitmapDataType::FLOAT },
};


static opengl::BitmapDataType getBitmapType(nap::Pixmap::EDataType dataType)
{
	auto it = bitmapDataTypeMap.find(dataType);
	assert(it != bitmapDataTypeMap.end());
	return it->second;
}

//////////////////////////////////////////////////////////////////////////
// Color creation functions / fill functions
//////////////////////////////////////////////////////////////////////////

/**
 * Helper function that creates a color based on the data associated with 
 * @param map the pixmap to get the color values from
 * @param x the x pixel coordinate value
 * @param y the y pixel coordinate value
 */
template<typename T>
static nap::BaseColor* createColor(const nap::Pixmap& map)
{
	switch (map.getBitmap().getNumberOfChannels())
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
static void fill(int x, int y, const nap::Pixmap& map, nap::BaseColor& outColor)
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
* @param map the pixmap to get the color values from
* @param x the x pixel coordinate value
* @param y the y pixel coordinate value
*/
template<typename T>
static nap::BaseColor* createColorData(const nap::Pixmap& map, int x, int y)
{
	switch (map.getBitmap().getNumberOfChannels())
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


static void convertPixmapSettings(const nap::Pixmap& resource, opengl::BitmapSettings& settings)
{
	settings.mDataType  = getBitmapType(resource.mType);
	settings.mColorType = getBitmapColorType(resource.mChannels);
	settings.mWidth = resource.mWidth;
	settings.mHeight = resource.mHeight;
}


namespace nap
{
	Pixmap::~Pixmap()			{ }

	bool Pixmap::init(utility::ErrorState& errorState)
	{
		// Create the settings we need to load the bitmap
		opengl::BitmapSettings settings;
		convertPixmapSettings(*this, settings);
		mBitmap.setSettings(settings);

		// Now allocate memory
		if (!errorState.check(mBitmap.allocateMemory(), "unable to allocate bitmap resource: %s", mID.c_str()))
			return false;

		// Store type of color
		onInit();

		return true;
	}


	bool Pixmap::initFromFile(const std::string& path, nap::utility::ErrorState& errorState)
	{
		if (!errorState.check(utility::fileExists(path), "unable to load image: %s, file does not exist: %s", path.c_str(), mID.c_str()))
			return false;

		// Load pixel data in to bitmap
		if (!errorState.check(opengl::loadBitmap(mBitmap, path, errorState), "Failed to load image %s, invalid bitmap", path.c_str()))
			return false;

		// Sync
		applySettingsFromBitmap();

		// Store type of color
		onInit();

		return true;
	}


	void Pixmap::initFromTexture(const nap::BaseTexture2D& texture)
	{
		const opengl::Texture2DSettings& settings = texture.getTexture().getSettings();

		// Get bitmap data type
		opengl::BitmapDataType  bitmap_type = opengl::getBitmapType(settings.mType);
		assert(bitmap_type != opengl::BitmapDataType::UNKNOWN);

		// Get bitmap color type
		opengl::BitmapColorType color_type = opengl::getColorType(settings.mFormat);
		assert(color_type != opengl::BitmapColorType::UNKNOWN);

		// Apply new settings
		mBitmap.setSettings(opengl::BitmapSettings(texture.getWidth(), texture.getHeight(), bitmap_type, color_type));

		// Sync
		applySettingsFromBitmap();

		// Store type of color
		onInit();

		// Now allocate
		mBitmap.allocateMemory();
	}


	std::unique_ptr<nap::BaseColor> Pixmap::makePixel() const
	{
		BaseColor* rvalue = nullptr;
		switch (mBitmap.getDataType())
		{
		case opengl::BitmapDataType::BYTE:
		{
			rvalue = createColor<uint8>(*this);
			break;
		}
		case opengl::BitmapDataType::FLOAT:
		{
			rvalue = createColor<float>(*this);
			break;
		}
		case opengl::BitmapDataType::USHORT:
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


	void Pixmap::getPixel(int x, int y, BaseColor& outPixel) const
	{
		switch (mBitmap.getDataType())
		{
		case opengl::BitmapDataType::BYTE:
		{
			fill<uint8>(x, y, *this, outPixel);
			break;
		}
		case opengl::BitmapDataType::FLOAT:
		{
			fill<float>(x, y, *this, outPixel);
			break;
		}
		case opengl::BitmapDataType::USHORT:
		{
			fill<uint16>(x, y, *this, outPixel);
			break;
		}
		default:
			assert(false);
			break;
		}
	}


	void Pixmap::setPixel(int x, int y, const BaseColor& color)
	{
		switch (mBitmap.getDataType())
		{
		case opengl::BitmapDataType::BYTE:
		{
			setPixelData<uint8>(x, y, color);
			break;
		}
		case opengl::BitmapDataType::FLOAT:
		{
			setPixelData<float>(x, y, color);
			break;
		}
		case opengl::BitmapDataType::USHORT:
		{
			setPixelData<uint16>(x, y, color);
			break;
		}
		default:
			assert(false);
			break;
		}
	}


	void Pixmap::applySettingsFromBitmap()
	{
		auto found_it = std::find_if(bitmapDataTypeMap.begin(), bitmapDataTypeMap.end(), [&](const auto& value)
		{
			return value.second == mBitmap.getDataType();
		});
		assert(found_it != bitmapDataTypeMap.end());
		mType = found_it->first;

		auto found_type = std::find_if(bitmapChannelMap.begin(), bitmapChannelMap.end(), [&](const auto& value)
		{
			return value.second == mBitmap.getColorType();
		});
		assert(found_type != bitmapChannelMap.end());
		mChannels = found_type->first;

		mWidth  = mBitmap.getWidth();
		mHeight = mBitmap.getHeight();
	}


	void Pixmap::onInit()
	{
		std::unique_ptr<BaseColor> temp_clr = makePixel();
		mColorType = temp_clr->get_type().get_raw_type();
		mValueType = temp_clr->getValueType();
	}

}


bool nap::PixmapFromFile::init(utility::ErrorState& errorState)
{
	return Pixmap::initFromFile(mPath, errorState);
}
