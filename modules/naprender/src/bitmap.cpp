#include "bitmap.h"

// External includes
#include <bitmaputils.h>
#include <utility/fileutils.h>
#include <rtti/typeinfo.h>
#include <texture2d.h>

// External includes
#include <FreeImage.h>

#undef BYTE

RTTI_BEGIN_ENUM(nap::Bitmap::EChannels)
	RTTI_ENUM_VALUE(nap::Bitmap::EChannels::R,			"R"),
	RTTI_ENUM_VALUE(nap::Bitmap::EChannels::RGB,		"RGB"),
	RTTI_ENUM_VALUE(nap::Bitmap::EChannels::RGBA,		"RGBA"),
	RTTI_ENUM_VALUE(nap::Bitmap::EChannels::BGR,		"BGR"),
	RTTI_ENUM_VALUE(nap::Bitmap::EChannels::BGRA,		"BGRA")
RTTI_END_ENUM

RTTI_BEGIN_ENUM(nap::Bitmap::EDataType)
	RTTI_ENUM_VALUE(nap::Bitmap::EDataType::BYTE,		"Byte"),
	RTTI_ENUM_VALUE(nap::Bitmap::EDataType::USHORT,		"Short"),
	RTTI_ENUM_VALUE(nap::Bitmap::EDataType::FLOAT,		"Float")
RTTI_END_ENUM

// nap::bitmap run time class definition 
RTTI_BEGIN_CLASS(nap::Bitmap)
	RTTI_PROPERTY("Width",		&nap::Bitmap::mWidth,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Height",		&nap::Bitmap::mHeight,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Channels",	&nap::Bitmap::mChannels,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Type",		&nap::Bitmap::mType,			nap::rtti::EPropertyMetaData::Default)
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
		if (!errorState.check(mWidth > 0 && mHeight > 0, "Invalid size specified for bitmap"))
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

		switch (fi_bitmap_type)
		{
		case FIT_BITMAP:
			mType = Bitmap::EDataType::BYTE;
			break;
		case FIT_UINT16:
		case FIT_RGB16:
		case FIT_RGBA16:
			mType = Bitmap::EDataType::USHORT;
			break;
		case FIT_FLOAT:
		case FIT_RGBF:
		case FIT_RGBAF:
			mType = Bitmap::EDataType::FLOAT;
			break;
		default:
			errorState.fail("Can't load bitmap from file; unknown pixel format");
			FreeImage_Unload(fi_bitmap);
			return false;
		}
		
		// Get color type
		FREE_IMAGE_COLOR_TYPE fi_bitmap_color_type = FreeImage_GetColorType(fi_bitmap);

		// If we're dealing with an rgb or rgba map and a bitmap
		// The endian of the loaded free image map becomes important
		// If so we might have to swap the red and blue channels regarding the interal color representation
		bool swap = (fi_bitmap_type == FREE_IMAGE_TYPE::FIT_BITMAP && FI_RGBA_RED == 2);

		switch (fi_bitmap_color_type)
		{
		case FIC_MINISBLACK:
			mChannels = EChannels::R;
			break;
		case FIC_RGB:
			mChannels = swap ? EChannels::BGR : EChannels::RGB;
			break;
		case FIC_RGBALPHA:
			mChannels = swap ? EChannels::BGRA : EChannels::RGBA;
			break;
		default:
			errorState.fail("Can't load bitmap from file; unknown pixel format");
			FreeImage_Unload(fi_bitmap);
			return false;
		}

		mWidth = FreeImage_GetWidth(fi_bitmap);
		mHeight = FreeImage_GetHeight(fi_bitmap);

		updatePixelFormat();

		mData.resize(getSizeInBytes());
		setData(FreeImage_GetBits(fi_bitmap), FreeImage_GetPitch(fi_bitmap));

		FreeImage_Unload(fi_bitmap);

		return true;
	}

	void Bitmap::setData(uint8_t* source, unsigned int sourcePitch)
	{
		unsigned int target_pitch = mWidth * mChannelSize * mNumChannels;
		assert(target_pitch <= sourcePitch);

		// If the dest & source pitches are the same, we can do a straight memcpy (most common/efficient case)
		if (target_pitch == sourcePitch)
		{
			memcpy(mData.data(), source, getSizeInBytes());
			return;
		}

		// If the pitch of the source & destination buffers are different, we need to copy the image data line by line (happens for weirdly-sized images)
		uint8_t* source_line = (uint8_t*)source;
		uint8_t* target_line = (uint8_t*)mData.data();

		// Get the amount of bytes every pixel occupies
		int source_stride = sourcePitch / mWidth;
		int target_stride = target_pitch / mWidth;

		for (int y = 0; y < mHeight; ++y)
		{
			uint8_t* source_loc = source_line;
			uint8_t* target_loc = target_line;
			for (int x = 0; x < mWidth; ++x)
			{
				memcpy(target_loc, source_loc, target_stride);
				target_loc += target_stride;
				source_loc += source_stride;
			}

			//memcpy(dest_line, source_line, dest_pitch);
			source_line += sourcePitch;
			target_line += target_pitch;
		}
	}


	void Bitmap::initFromTexture(const opengl::Texture2DSettings& settings)
	{
		mWidth = settings.mWidth;
		mHeight = settings.mHeight;

		switch (settings.mType)
		{
		case GL_UNSIGNED_BYTE:
			mType = EDataType::BYTE;
			break;
		case GL_FLOAT:
			mType = EDataType::FLOAT;
			break;
		case GL_UNSIGNED_SHORT:
			mType = EDataType::USHORT;
			break;
		default:
			assert(false);
		}

		switch (settings.mFormat)
		{
		case GL_RED:
			mChannels = EChannels::R;
			break;
		case GL_RGB:
			mChannels = EChannels::RGB;
			break;
		case GL_RGBA:
			mChannels = EChannels::RGBA;
			break;
		case GL_BGR:
			mChannels = EChannels::BGR;
			break;
		case GL_BGRA:
			mChannels = EChannels::BGRA;
			break;
		default:
			assert(false);
		}

		updatePixelFormat();

		uint64_t size = getSizeInBytes();
		mData.resize(size);
	}


	size_t Bitmap::getSizeInBytes() const
	{
		return mWidth * mHeight * mNumChannels * mChannelSize;
	}


	std::unique_ptr<nap::BaseColor> Bitmap::makePixel() const
	{
		BaseColor* rvalue = nullptr;
		switch (mType)
		{
		case EDataType::BYTE:
		{
			rvalue = createColor<uint8>(*this);
			break;
		}
		case EDataType::FLOAT:
		{
			rvalue = createColor<float>(*this);
			break;
		}
		case EDataType::USHORT:
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
		switch (mType)
		{
		case EDataType::BYTE:
		{
			fill<uint8>(x, y, *this, outPixel);
			break;
		}
		case EDataType::FLOAT:
		{
			fill<float>(x, y, *this, outPixel);
			break;
		}
		case EDataType::USHORT:
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
		switch (mType)
		{
		case EDataType::BYTE:
		{
			setPixelData<uint8>(x, y, color);
			break;
		}
		case EDataType::FLOAT:
		{
			setPixelData<float>(x, y, color);
			break;
		}
		case EDataType::USHORT:
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
		switch (mType)
		{
		case EDataType::BYTE:
			mChannelSize = sizeof(unsigned char);
			break;
		case EDataType::FLOAT:
			mChannelSize = sizeof(float);
			break;
		case EDataType::USHORT:
			mChannelSize = sizeof(uint16_t);
			break;
		}

		switch (mChannels)
		{
		case EChannels::R:
			mNumChannels = 1;
			break;
		case EChannels::RGB:
		case EChannels::BGR:
			mNumChannels = 3;
			break;
		case EChannels::RGBA:
		case EChannels::BGRA:
			mNumChannels = 4;
			break;
		}

		std::unique_ptr<BaseColor> temp_clr = makePixel();
		mColorType = temp_clr->get_type().get_raw_type();
		mValueType = temp_clr->getValueType();
	}
}


bool nap::BitmapFromFile::init(utility::ErrorState& errorState)
{
	return Bitmap::initFromFile(mPath, errorState);
}
