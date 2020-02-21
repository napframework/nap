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
		RGB			= 1,	///< RGB red, green and blue component
		RGBA		= 2,	///< RGBA red, green, blue and alpha component
		BGR			= 3,	///< BGR blue, green and red component
		BGRA		= 4		///< BGRA blue, green, red and alpha component
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
}
