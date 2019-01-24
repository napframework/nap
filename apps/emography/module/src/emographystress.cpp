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

// nap::emography::StressStateReading class definition
RTTI_BEGIN_CLASS(nap::emography::StressStateReading)
	RTTI_CONSTRUCTOR(const nap::emography::EStressState&, const nap::TimeStamp&)
	RTTI_CONSTRUCTOR(const nap::emography::EStressState&)
	RTTI_PROPERTY("Object", &nap::emography::StressStateReading::mObject, nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT

// nap::emography::StressStateReadingSummary class definition
RTTI_BEGIN_CLASS(nap::emography::StressStateReadingSummary)
	RTTI_CONSTRUCTOR(const nap::emography::ReadingBase&)
	RTTI_PROPERTY("Object", &nap::emography::StressStateReadingSummary::mObject, nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT

// nap::emography::StressIntensityReading class definition
RTTI_BEGIN_CLASS(nap::emography::StressIntensityReading)
	RTTI_CONSTRUCTOR(const nap::emography::StressIntensity&, const nap::TimeStamp&)
	RTTI_CONSTRUCTOR(const nap::emography::StressIntensity&)
	RTTI_PROPERTY("Object", &nap::emography::StressIntensityReading::mObject, nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT

// nap::emography::StressIntensityReadingSummary class definition
RTTI_BEGIN_CLASS(nap::emography::StressIntensityReadingSummary)
	RTTI_CONSTRUCTOR(const nap::emography::ReadingBase&)
	RTTI_PROPERTY("Object", &nap::emography::StressIntensityReadingSummary::mObject, nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	namespace emography
	{
		StressIntensity::StressIntensity(float intensity) : mValue(intensity)	{ }
	}
}
