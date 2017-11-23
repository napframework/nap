#include "pixmap.h"

// External includes
#include <nbitmaputils.h>
#include <utility/fileutils.h>
#include <rtti/typeinfo.h>

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


using BitmapValueTypeMap = std::unordered_map<opengl::BitmapDataType, nap::rtti::TypeInfo>;
static BitmapValueTypeMap& getBitmapValueTypeMap()
{
	static BitmapValueTypeMap map;
	if (map.empty())
	{
		map.emplace(std::make_pair(opengl::BitmapDataType::BYTE,	RTTI_OF(nap::uint8)));
		map.emplace(std::make_pair(opengl::BitmapDataType::USHORT,	RTTI_OF(nap::uint16)));
		map.emplace(std::make_pair(opengl::BitmapDataType::FLOAT,	RTTI_OF(float)));
	}
	return map;
}

static opengl::BitmapDataType getBitmapType(nap::Pixmap::EDataType dataType)
{
	auto it = bitmapDataTypeMap.find(dataType);
	assert(it != bitmapDataTypeMap.end());
	return it->second;
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
		return true;
	}


	bool Pixmap::initFromFile(const std::string& path, nap::utility::ErrorState& errorState)
	{
		if (!errorState.check(utility::fileExists(path), "unable to load image: %s, file does not exist: %s", path.c_str(), mID.c_str()))
			return false;

		// Load pixel data in to bitmap
		if (!errorState.check(opengl::loadBitmap(mBitmap, path, errorState), "Failed to load image %s, invalid bitmap", path.c_str()))
			return false;

		// Extract the width, height, channels and type
		opengl::BitmapDataType bitmap_data_type = mBitmap.getDataType();
		auto found_it = std::find_if(bitmapDataTypeMap.begin(), bitmapDataTypeMap.end(), [&](const auto& value)
		{
			return value.second == bitmap_data_type;
		});
		assert(found_it != bitmapDataTypeMap.end());
		mType = found_it->first;

		opengl::BitmapColorType bitmap_color_type = mBitmap.getColorType();
		auto found_type = std::find_if(bitmapChannelMap.begin(), bitmapChannelMap.end(), [&](const auto& value)
		{
			return value.second == bitmap_color_type;
		});
		assert(found_type != bitmapChannelMap.end());
		mChannels = found_type->first;
		
		mWidth  = mBitmap.getWidth();
		mHeight = mBitmap.getHeight();

		return true;
	}
}


bool nap::PixmapFromFile::init(utility::ErrorState& errorState)
{
	return Pixmap::initFromFile(mPath, errorState);
}
