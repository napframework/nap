#include "color.h"
#include <mathutils.h>

RTTI_DEFINE_BASE(nap::BaseColor)

RTTI_BEGIN_ENUM(nap::EColorChannel)
	RTTI_ENUM_VALUE(nap::EColorChannel::Red,	"Red"),
	RTTI_ENUM_VALUE(nap::EColorChannel::Green,	"Green"),
	RTTI_ENUM_VALUE(nap::EColorChannel::Blue,	"Blue"),
	RTTI_ENUM_VALUE(nap::EColorChannel::Alpha,	"Red")
RTTI_END_ENUM

RTTI_BEGIN_CLASS(nap::RGBColor8)
	RTTI_CONSTRUCTOR(nap::uint8, nap::uint8, nap::uint8)
	RTTI_PROPERTY("Values", &nap::RGBColor8::mValues, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::RGBAColor8)
	RTTI_CONSTRUCTOR(nap::uint8, nap::uint8, nap::uint8, nap::uint8)
	RTTI_PROPERTY("Values", &nap::RGBAColor8::mValues, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::RGBColor16)
	RTTI_CONSTRUCTOR(nap::uint16, nap::uint16, nap::uint16)
	RTTI_PROPERTY("Values", &nap::RGBColor16::mValues, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::RGBAColor16)
	RTTI_CONSTRUCTOR(nap::uint16, nap::uint16, nap::uint16, nap::uint16)
	RTTI_PROPERTY("Values", &nap::RGBAColor16::mValues, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::RGBColorFloat)
	RTTI_CONSTRUCTOR(float, float, float)
	RTTI_PROPERTY("Values", &nap::RGBColorFloat::mValues, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::RGBAColorFloat)
	RTTI_CONSTRUCTOR(float, float, float, float)
	RTTI_PROPERTY("Values", &nap::RGBAColorFloat::mValues, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
	static void floatToByte(const BaseColor& inColor, BaseColor& outColor, int channel)
	{
		assert(inColor.getValueType()  == RTTI_OF(float));
		assert(outColor.getValueType() == RTTI_OF(nap::uint8));
		float in = *(static_cast<const float*>(inColor.getData(channel)));
		uint8* out = static_cast<uint8*>(outColor.getData(channel));
		*out = static_cast<uint8>(math::clamp<float>(in, 0.0f, 1.0f) * static_cast<float>(math::max<uint8>()));
	}

	static void floatToShort(const BaseColor& inColor, BaseColor& outColor, int channel)
	{
		assert(inColor.getValueType()  == RTTI_OF(float));
		assert(outColor.getValueType() == RTTI_OF(nap::uint16));
		float in = *(static_cast<const float*>(inColor.getData(channel)));
		uint16* out = static_cast<uint16*>(outColor.getData(channel));
		*out = static_cast<uint16>(math::clamp<float>(in, 0.0f, 1.0f) * static_cast<float>(math::max<uint16>()));
	}

	static void floatToFloat(const BaseColor& inColor, BaseColor& outColor, int channel)
	{
		assert(inColor.getValueType()  == RTTI_OF(float));
		assert(outColor.getValueType() == RTTI_OF(float));
		float in = *(static_cast<const float*>(inColor.getData(channel)));
		float* out = static_cast<float*>(outColor.getData(channel));
		*out = in;
	}

	static void byteToFLoat(const BaseColor& inColor, BaseColor& outColor, int channel)
	{
		assert(inColor.getValueType()  == RTTI_OF(uint8));
		assert(outColor.getValueType() == RTTI_OF(float));
		uint8 in = *(static_cast<const uint8*>(inColor.getData(channel)));
		float* out = static_cast<float*>(outColor.getData(channel));
		*out = static_cast<float>(in) / static_cast<float>(math::max<uint8>());
	}

	static void byteToShort(const BaseColor& inColor, BaseColor& outColor, int channel)
	{
		assert(inColor.getValueType()  == RTTI_OF(uint8));
		assert(outColor.getValueType() == RTTI_OF(uint16));
		uint8 in = *(static_cast<const uint8*>(inColor.getData(channel)));
		uint16* out = static_cast<uint16*>(outColor.getData(channel));
		*out = static_cast<uint16>(in) * (math::max<uint16>() / (uint16)math::max<uint8>());
	}

	static void byteToByte(const BaseColor& inColor, BaseColor& outColor, int channel)
	{
		assert(inColor.getValueType()  == RTTI_OF(uint8));
		assert(outColor.getValueType() == RTTI_OF(uint8));
		uint8 in = *(static_cast<const uint8*>(inColor.getData(channel)));
		uint8* out = static_cast<uint8*>(outColor.getData(channel));
		*out = in;
	}

	static void shortToFLoat(const BaseColor& inColor, BaseColor& outColor, int channel)
	{
		assert(inColor.getValueType() == RTTI_OF(uint16));
		assert(outColor.getValueType() == RTTI_OF(float));
		uint16 in = *(static_cast<const uint16*>(inColor.getData(channel)));
		float* out = static_cast<float*>(outColor.getData(channel));
		*out = static_cast<float>(in) / static_cast<float>(math::max<uint16>());
	}

	static void shortToByte(const BaseColor& inColor, BaseColor& outColor, int channel)
	{
		assert(inColor.getValueType() == RTTI_OF(uint16));
		assert(outColor.getValueType() == RTTI_OF(uint8));
		uint16 in = *(static_cast<const uint16*>(inColor.getData(channel)));
		uint8* out = static_cast<uint8*>(outColor.getData(channel));
		*out = static_cast<uint8>(in / (math::max<uint16>() / (uint16)math::max<uint8>()));
	}

	static void shortToShort(const BaseColor& inColor, BaseColor& outColor, int channel)
	{
		assert(inColor.getValueType() == RTTI_OF(uint16));
		assert(outColor.getValueType() == RTTI_OF(uint16));
		uint16 in = *(static_cast<const uint16*>(inColor.getData(channel)));
		uint16* out = static_cast<uint16*>(outColor.getData(channel));
		*out = in;
	}
}

void nap::BaseColor::convertColor(const BaseColor& from, BaseColor& to)
{
	assert(from.getNumberOfChannels() >= to.getNumberOfChannels());
	std::function<void(const BaseColor&, BaseColor&, int)> convert_func = nullptr;
	
	if (from.getValueType() == RTTI_OF(uint8))
	{
		if (to.getValueType() == RTTI_OF(float))
		{
			convert_func = &byteToFLoat;
		}
		else if (to.getValueType() == RTTI_OF(uint16))
		{
			convert_func = &byteToShort;
		}
		else if (to.getValueType() == RTTI_OF(uint8))
		{
			convert_func = &byteToByte;
		}
	}
	else if (from.getValueType() == RTTI_OF(float))
	{
		if (to.getValueType() == RTTI_OF(uint8))
		{
			convert_func = &floatToByte;
		}
		else if (to.getValueType() == RTTI_OF(uint16))
		{
			convert_func = &floatToFloat;
		}
		else if (to.getValueType() == RTTI_OF(float))
		{
			convert_func = &floatToFloat;
		}
	}
	else if (from.getValueType() == RTTI_OF(uint16))
	{
		if (to.getValueType() == RTTI_OF(uint8))
		{
			convert_func = &shortToByte;
		}
		else if (to.getValueType() == RTTI_OF(float))
		{
			convert_func = &shortToFLoat;
		}
		else if(to.getValueType() == RTTI_OF(uint16))
		{
			convert_func = shortToShort;
		}
	}

	// Perform conversion
	assert(convert_func != nullptr);
	for (int i = 0; i < from.getNumberOfChannels(); i++)
		convert_func(from, to, i);
}

void nap::BaseColor::convert(BaseColor& color) const
{
	BaseColor::convertColor(*this, color);
}
