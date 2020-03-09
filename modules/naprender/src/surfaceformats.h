#pragma once

#include <functional>

namespace nap
{
	/**
	 *	Supported bitmap data types
	 */
	enum class ESurfaceDataType : int
	{
		BYTE		= 0,	///< 08 bit bitmap
		USHORT		= 1,	///< 16 bit bitmap
		FLOAT		= 2		///< 32 bit bitmap
	};

	/**
	 *	Supported bitmap color types
	 */
	enum class ESurfaceChannels : int
	{
		R			= 0,	///< R red component
		RGBA		= 1,	///< RGBA red, green, blue and alpha component
		BGRA		= 2		///< BGRA blue, green, red and alpha component
	};

	enum class EColorSpace : int
	{
		Linear,				///< Linear color space
		sRGB				///< Non-linear, sRGB color space
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
