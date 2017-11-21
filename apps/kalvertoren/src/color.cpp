#include "color.h"

RTTI_DEFINE_BASE(nap::Color)

RTTI_BEGIN_CLASS(nap::RGBColor8)
	RTTI_CONSTRUCTOR(nap::uint8, nap::uint8, nap::uint8)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::RGBAColor8)
	RTTI_CONSTRUCTOR(nap::uint8, nap::uint8, nap::uint8, nap::uint8)
RTTI_END_CLASS

namespace nap
{

}