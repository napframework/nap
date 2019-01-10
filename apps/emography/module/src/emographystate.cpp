// Local Includes
#include "emographystate.h"

// nap::emography::EState enum definition 
RTTI_BEGIN_ENUM(nap::emography::EState)
	RTTI_ENUM_VALUE(nap::emography::EState::Under,		"Under"),
	RTTI_ENUM_VALUE(nap::emography::EState::Normal,		"Normal"),
	RTTI_ENUM_VALUE(nap::emography::EState::Over,		"Over"),
	RTTI_ENUM_VALUE(nap::emography::EState::Unknown,	"Unknown")
RTTI_END_ENUM

// nap::emography::Intensity class definition
RTTI_BEGIN_STRUCT(nap::emography::Intensity)
	RTTI_VALUE_CONSTRUCTOR(float)
	RTTI_PROPERTY("Intensity", &nap::emography::Intensity::mIntensity, nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	namespace emography
	{
		Intensity::Intensity(float intensity) : mIntensity(intensity)	{ }


		bool Intensity::isValid()
		{
			return mIntensity >= 0.0f;
		}
	}
}
