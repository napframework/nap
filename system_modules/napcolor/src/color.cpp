/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "color.h"
#include <mathutils.h>

RTTI_DEFINE_BASE(nap::BaseColor)

RTTI_BEGIN_ENUM(nap::EColorChannel)
	RTTI_ENUM_VALUE(nap::EColorChannel::Red,	"Red"),
	RTTI_ENUM_VALUE(nap::EColorChannel::Green,	"Green"),
	RTTI_ENUM_VALUE(nap::EColorChannel::Blue,	"Blue"),
	RTTI_ENUM_VALUE(nap::EColorChannel::Alpha,	"Red")
RTTI_END_ENUM

RTTI_BEGIN_STRUCT(nap::RGBColor8)
	RTTI_VALUE_CONSTRUCTOR(nap::uint8, nap::uint8, nap::uint8)
	RTTI_VALUE_CONSTRUCTOR(const nap::BaseColor&)
	RTTI_PROPERTY("Values", &nap::RGBColor8::mValues, nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT

RTTI_BEGIN_STRUCT(nap::RGBAColor8)
	RTTI_VALUE_CONSTRUCTOR(nap::uint8, nap::uint8, nap::uint8, nap::uint8)
	RTTI_VALUE_CONSTRUCTOR(const nap::BaseColor&)
	RTTI_PROPERTY("Values", &nap::RGBAColor8::mValues, nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT

RTTI_BEGIN_STRUCT(nap::RGBColor16)
	RTTI_VALUE_CONSTRUCTOR(nap::uint16, nap::uint16, nap::uint16)
	RTTI_VALUE_CONSTRUCTOR(const nap::BaseColor&)
	RTTI_PROPERTY("Values", &nap::RGBColor16::mValues, nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT

RTTI_BEGIN_STRUCT(nap::RGBAColor16)
	RTTI_VALUE_CONSTRUCTOR(nap::uint16, nap::uint16, nap::uint16, nap::uint16)
	RTTI_VALUE_CONSTRUCTOR(const nap::BaseColor&)
	RTTI_PROPERTY("Values", &nap::RGBAColor16::mValues, nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT

RTTI_BEGIN_STRUCT(nap::RGBColorFloat)
	RTTI_VALUE_CONSTRUCTOR(float, float, float)
	RTTI_VALUE_CONSTRUCTOR(const nap::BaseColor&)
	RTTI_PROPERTY("Values", &nap::RGBColorFloat::mValues, nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT

RTTI_BEGIN_STRUCT(nap::RGBAColorFloat)
	RTTI_VALUE_CONSTRUCTOR(float, float, float, float)
	RTTI_VALUE_CONSTRUCTOR(const nap::BaseColor&)
	RTTI_PROPERTY("Values", &nap::RGBAColorFloat::mValues, nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT

RTTI_BEGIN_STRUCT(nap::RColor8)
	RTTI_VALUE_CONSTRUCTOR(nap::uint8)
	RTTI_VALUE_CONSTRUCTOR(const nap::BaseColor&)
	RTTI_PROPERTY("Values", &nap::RColor8::mValues, nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT

RTTI_BEGIN_STRUCT(nap::RColor16)
	RTTI_VALUE_CONSTRUCTOR(nap::uint16)
	RTTI_VALUE_CONSTRUCTOR(const nap::BaseColor&)
	RTTI_PROPERTY("Values", &nap::RColor16::mValues, nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT

RTTI_BEGIN_STRUCT(nap::RColorFloat)
	RTTI_VALUE_CONSTRUCTOR(float)
	RTTI_VALUE_CONSTRUCTOR(const nap::BaseColor&)
	RTTI_PROPERTY("Values", &nap::RColorFloat::mValues, nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT


RTTI_DEFINE_BASE(nap::RGBColorData8)
RTTI_DEFINE_BASE(nap::RGBAColorData8)
RTTI_DEFINE_BASE(nap::RColorData8)
RTTI_DEFINE_BASE(nap::RGBColorData16)
RTTI_DEFINE_BASE(nap::RGBAColorData16)
RTTI_DEFINE_BASE(nap::RColorData16)
RTTI_DEFINE_BASE(nap::RGBColorDataFloat)
RTTI_DEFINE_BASE(nap::RGBAColorDataFloat)
RTTI_DEFINE_BASE(nap::RColorDataFloat)


namespace nap
{
	/**
	 * Fetches the input and output colors that are used for color conversion
	 * This call works with colors that point to data in memory, ie: do not manage their own color data
	 * As well as for colors that do manage their own data
	 * @param inColor the input color to extract the input value from
	 * @param outColor the color to extract the output value from as a pointer
	 * @param channel the color channel to extract the data from
	 * @param out will hold the pointer to the location in memory that will receive the new color value
	 * @return the input color value @channel
	 */
	template<typename I, typename O>
	static I getColorIO(const BaseColor& inColor, BaseColor& outColor, int channel, O*& out)
	{
		I in(0);
		if (inColor.isPointer())
		{
			in = **((const I**)(inColor.getData(channel)));
		}
		else
		{
			in = *(static_cast<const I*>(inColor.getData(channel)));
		}

		O* outptr(nullptr);
		if (outColor.isPointer())
		{
			outptr = *((O**)(outColor.getData(channel)));
		}
		else
		{
			outptr = static_cast<O*>(outColor.getData(channel));
		}
		out = outptr;
		return in;
	}

	static void floatToByte(const BaseColor& inColor, BaseColor& outColor, int channel)
	{
		assert(inColor.getValueType()  == RTTI_OF(float));
		assert(outColor.getValueType() == RTTI_OF(nap::uint8));
		
		uint8* out(nullptr);
		float in = getColorIO<float, uint8>(inColor, outColor, channel, out);
		*out = static_cast<uint8>(math::clamp<float>(in, 0.0f, 1.0f) * static_cast<float>(math::max<uint8>()));
	}

	static void floatToShort(const BaseColor& inColor, BaseColor& outColor, int channel)
	{
		assert(inColor.getValueType()  == RTTI_OF(float));
		assert(outColor.getValueType() == RTTI_OF(nap::uint16));
		
		uint16* out(nullptr);
		float in = getColorIO<float, uint16>(inColor, outColor, channel, out);
		*out = static_cast<uint16>(math::clamp<float>(in, 0.0f, 1.0f) * static_cast<float>(math::max<uint16>()));
	}

	static void floatToFloat(const BaseColor& inColor, BaseColor& outColor, int channel)
	{
		assert(inColor.getValueType()  == RTTI_OF(float));
		assert(outColor.getValueType() == RTTI_OF(float));
		float* out(nullptr);
		float in = getColorIO<float, float>(inColor, outColor, channel, out);
		*out = in;
	}

	static void byteToFLoat(const BaseColor& inColor, BaseColor& outColor, int channel)
	{
		assert(inColor.getValueType()  == RTTI_OF(uint8));
		assert(outColor.getValueType() == RTTI_OF(float));

		float* out(nullptr);
		uint8 in = getColorIO<uint8, float>(inColor, outColor, channel, out);
		*out = static_cast<float>(in) / static_cast<float>(math::max<uint8>());
	}

	static void byteToShort(const BaseColor& inColor, BaseColor& outColor, int channel)
	{
		assert(inColor.getValueType()  == RTTI_OF(uint8));
		assert(outColor.getValueType() == RTTI_OF(uint16));

		uint16* out(nullptr);
		uint8 in = getColorIO<uint8, uint16>(inColor, outColor, channel, out);
		*out = static_cast<uint16>(in) * (math::max<uint16>() / (uint16)math::max<uint8>());
	}

	static void byteToByte(const BaseColor& inColor, BaseColor& outColor, int channel)
	{
		assert(inColor.getValueType()  == RTTI_OF(uint8));
		assert(outColor.getValueType() == RTTI_OF(uint8));
		
		uint8* out(nullptr);
		uint8 in = getColorIO<uint8, uint8>(inColor, outColor, channel, out);
		*out = in;
	}

	static void shortToFLoat(const BaseColor& inColor, BaseColor& outColor, int channel)
	{
		assert(inColor.getValueType()  == RTTI_OF(uint16));
		assert(outColor.getValueType() == RTTI_OF(float));
		
		float* out(nullptr);
		uint16 in = getColorIO<uint16, float>(inColor, outColor, channel, out);
		*out = static_cast<float>(in) / static_cast<float>(math::max<uint16>());
	}

	static void shortToByte(const BaseColor& inColor, BaseColor& outColor, int channel)
	{
		assert(inColor.getValueType()  == RTTI_OF(uint16));
		assert(outColor.getValueType() == RTTI_OF(uint8));

		uint8* out(nullptr);
		uint16 in = getColorIO<uint16, uint8>(inColor, outColor, channel, out);
		*out = static_cast<uint8>(in / (math::max<uint16>() / (uint16)math::max<uint8>()));
	}

	static void shortToShort(const BaseColor& inColor, BaseColor& outColor, int channel)
	{
		assert(inColor.getValueType()  == RTTI_OF(uint16));
		assert(outColor.getValueType() == RTTI_OF(uint16));

		uint16* out(nullptr);
		uint16 in = getColorIO<uint16, uint16>(inColor, outColor, channel, out);
		*out = in;
	}

	//////////////////////////////////////////////////////////////////////////
	// BaseColor
	//////////////////////////////////////////////////////////////////////////

	void nap::BaseColor::convertColor(const BaseColor& source, BaseColor& target)
	{
		// Get conversion function
		std::function<void(const BaseColor&, BaseColor&, int)> convert_func = getConverter(source, target);
		assert(convert_func != nullptr);

		// Convert
		int cc = math::min<int>(source.getNumberOfChannels(), target.getNumberOfChannels());
		for (int i = 0; i < cc; i++)
			convert_func(source, target, i);
	}


	void nap::BaseColor::convertColor(const BaseColor& source, BaseColor& target, const Converter& converter)
	{
		// Convert
		int cc = math::min<int>(source.getNumberOfChannels(), target.getNumberOfChannels());
		for (int i = 0; i < cc; i++)
			converter(source, target, i);
	}


	std::function<void(const nap::BaseColor&, nap::BaseColor&, int)> nap::BaseColor::getConverter(const BaseColor& target) const
	{
		return BaseColor::getConverter(*this, target);
	}


	void nap::BaseColor::convert(BaseColor& target) const
	{
		BaseColor::convertColor(*this, target);
	}


	BaseColor::Converter nap::BaseColor::getConverter(const BaseColor& source, const BaseColor& target)
	{
		using ColorConvertMap = std::unordered_map<nap::rtti::TypeInfo, std::unordered_map<rtti::TypeInfo, BaseColor::Converter>>;
		static ColorConvertMap convert_map =
		{
			{ RTTI_OF(uint8),	{ { RTTI_OF(float), &byteToFLoat },  { RTTI_OF(uint16), &byteToShort  }, { RTTI_OF(uint8),  &byteToByte   } } },
			{ RTTI_OF(float),	{ { RTTI_OF(uint8), &floatToByte },  { RTTI_OF(uint16), &floatToShort }, { RTTI_OF(float),  &floatToFloat } } },
			{ RTTI_OF(uint16),	{ { RTTI_OF(uint8), &shortToByte },  { RTTI_OF(float),  &shortToFLoat }, { RTTI_OF(uint16), &shortToShort } } },
		};

		// Get from
		auto it_from = convert_map.find(source.getValueType());
		if (it_from == convert_map.end())
			return nullptr;

		// Get to
		auto it_to = it_from->second.find(target.getValueType());
		return it_to != it_from->second.end() ? it_to->second : nullptr;
	}
}
