#include "color.h"

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