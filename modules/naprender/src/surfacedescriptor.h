#pragma once

#include "rtti/typeinfo.h"

// External Includes

namespace nap
{
	/**
	 *	Supported surface data types
	 */
	enum class ESurfaceDataType : int
	{
		BYTE		= 0,	///< 08 bit bitmap
		USHORT		= 1,	///< 16 bit bitmap
		FLOAT		= 2		///< 32 bit bitmap
	};

	/**
	 *	Supported number of surface channels
	 */
	enum class ESurfaceChannels : int
	{
		R			= 0,	///< R red component
		RGBA		= 1,	///< RGBA red, green, blue and alpha component
		BGRA		= 2		///< BGRA blue, green, red and alpha component
	};

	/**
	 *	Supported surface color spaces
	 */
	enum class EColorSpace : int
	{
		Linear,				///< Linear color space
		sRGB				///< Non-linear, sRGB color space
	};

	/**
	 * Used to describe the data of all 2D surfaces, including 2DTextures and Bitmaps.
	 */
	struct NAPAPI SurfaceDescriptor
	{
		RTTI_ENABLE()

	public:
		SurfaceDescriptor() = default;
        virtual ~SurfaceDescriptor() = default;
		SurfaceDescriptor(uint32_t width, uint32_t height, ESurfaceDataType dataType, ESurfaceChannels channels);
		SurfaceDescriptor(uint32_t width, uint32_t height, ESurfaceDataType dataType, ESurfaceChannels channels, EColorSpace colorSpace);

		/**
		 * @return width of the surface in pixels or texels.
		 */
		int getWidth() const											{ return mWidth; }

		/**
		 * @return height of the surface in pixels or texels.
		 */
		int getHeight() const											{ return mHeight; }
		
		/**
		 * @return The size in bytes of 1 row of pixels
		 */
		int getPitch() const;
		
		/**
		 * @return number of color channels
		 */
		int getNumChannels() const;

		/**
		 * @return Size in bytes of a single channel: 1 for 8 bit, 2 for short and 4 for float.
		 */
		int getChannelSize() const;
		
		/**
		 * @return Total number of bytes for a single pixel
		 */
		int getBytesPerPixel() const;

		/**
		 * @return total size in bytes for the entire surface
		 */
		uint64_t getSizeInBytes() const;

		/**
		 * @return Type of surface: Byte, Short, Float etc.
		 */
		ESurfaceDataType getDataType() const							{ return mDataType; }

		/**
		 * @return total number of channels, 3 for RGB, 4 for RGBA etc.
		 */
		ESurfaceChannels getChannels() const							{ return mChannels; }

		/**
		 * @return surface color space, defaults to Linear.
		 */
		EColorSpace getColorSpace() const								{ return mColorSpace; }

		/**
		 * @return if the surface is initialized and valid.
		 */
		bool isValid() const { return mWidth != 0 && mHeight != 0; }

		bool operator==(const SurfaceDescriptor& other) const;
		bool operator!=(const SurfaceDescriptor& other) const;

	public:
		uint32_t			mWidth = 0;									///< Property: 'Width' specifies the width of the texture
		uint32_t			mHeight = 0;								///< Property: 'Height' specifies the height of the texture
		ESurfaceDataType	mDataType = nap::ESurfaceDataType::BYTE;	///< Property: 'DataType' specifies the amount of bytes in a single channel
		ESurfaceChannels	mChannels = nap::ESurfaceChannels::BGRA;	///< Property: 'Channels' specifies the channels and their order
		EColorSpace			mColorSpace = EColorSpace::Linear;			///< Property: 'ColorSpace' specifies linear or SRGB space. Only applicable to BYTE datatypes
	};
}

namespace std
{
	template <>
	struct hash<nap::ESurfaceChannels>
	{
		size_t operator()(const nap::ESurfaceChannels& v) const
		{
			return hash<int>()(static_cast<int>(v));
		}
	};

	template <>
	struct hash<nap::ESurfaceDataType>
	{
		size_t operator()(const nap::ESurfaceDataType& v) const
		{
			return hash<int>()(static_cast<int>(v));
		}
	};

	template <>
	struct hash<nap::EColorSpace>
	{
		size_t operator()(const nap::EColorSpace& v) const
		{
			return hash<int>()(static_cast<int>(v));
		}
	};
}