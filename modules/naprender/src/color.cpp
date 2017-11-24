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
		float in  = inColor.getValue<float>(channel);
		assert(outColor.getValueType() == RTTI_OF(nap::uint8));
		uint8* out = static_cast<uint8*>(outColor.getData(channel));
		*out = static_cast<uint8>(math::clamp<float>(in, 0.0f, 1.0f) * static_cast<float>(math::max<uint8>()));
	}

	static void byteToFLoat(const BaseColor& inColor, BaseColor& outColor, int channel)
	{
		uint8 in = inColor.getValue<uint8>(channel);
		assert(outColor.getValueType() == RTTI_OF(float));
		float* out = static_cast<float*>(outColor.getData(channel));
		*out = static_cast<float>(in) / static_cast<float>(math::max<uint8>());
	}
}

void nap::BaseColor::convertColor(const BaseColor& from, BaseColor& to)
{
	assert(from.getNumberOfChannels() >= to.getNumberOfChannels());
	for (int i = 0; i < from.getNumberOfChannels(); i++)
	{
		byteToFLoat(from, to, i);
	}
}

void nap::BaseColor::convert(BaseColor& color) const
{
	BaseColor::convertColor(*this, color);
}
