// Local Includes
#include "emographystress.h"

// nap::emography::EState enum definition 
RTTI_BEGIN_ENUM(nap::emography::EStressState)
	RTTI_ENUM_VALUE(nap::emography::EStressState::Under,	"Under"),
	RTTI_ENUM_VALUE(nap::emography::EStressState::Normal,	"Normal"),
	RTTI_ENUM_VALUE(nap::emography::EStressState::Over,		"Over"),
	RTTI_ENUM_VALUE(nap::emography::EStressState::Unknown,	"Unknown")
RTTI_END_ENUM

// nap::emography::Intensity class definition
RTTI_BEGIN_STRUCT(nap::emography::StressIntensity)
	RTTI_VALUE_CONSTRUCTOR(float)
	RTTI_PROPERTY("Value", &nap::emography::StressIntensity::mValue, nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT

RTTI_BEGIN_STRUCT(nap::emography::StressReading)
	RTTI_VALUE_CONSTRUCTOR(nap::emography::EStressState, float)
	RTTI_PROPERTY("State",		&nap::emography::StressReading::mState,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Intensity",	&nap::emography::StressReading::mIntensity, nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	namespace emography
	{
		StressIntensity::StressIntensity(float intensity) : mValue(intensity)	{ }

		StressReading::StressReading(EStressState state, float intensity) :
			mState(state), mIntensity(intensity)								{ }
	}
}
