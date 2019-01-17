#include "emographysnapshot.h"

// nap::emographysnapshot run time class definition 
RTTI_BEGIN_CLASS(nap::emography::StressStateReading)
	RTTI_CONSTRUCTOR(const nap::emography::EStressState&, const nap::TimeStamp&)
	RTTI_CONSTRUCTOR(const nap::emography::EStressState&)
	RTTI_PROPERTY("TimeStamp", &nap::emography::StressStateReading::mTimeStamp, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Object", &nap::emography::StressStateReading::mObject, nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT

RTTI_BEGIN_CLASS(nap::emography::StressIntensityReading)
	RTTI_CONSTRUCTOR(const nap::emography::StressIntensity&, const nap::TimeStamp&)
	RTTI_CONSTRUCTOR(const nap::emography::StressIntensity&)
	RTTI_PROPERTY("TimeStamp", &nap::emography::StressIntensityReading::mTimeStamp, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Object", &nap::emography::StressIntensityReading::mObject, nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT

RTTI_BEGIN_CLASS(nap::emography::StressStateReadingSummary)
	RTTI_PROPERTY("StartTime", &nap::emography::StressStateReadingSummary::mStartTime, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("EndTime", &nap::emography::StressStateReadingSummary::mEndTime, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Object", &nap::emography::StressStateReadingSummary::mObject, nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT

RTTI_BEGIN_CLASS(nap::emography::StressIntensityReadingSummary)
	RTTI_PROPERTY("StartTime", &nap::emography::StressIntensityReadingSummary::mStartTime, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("EndTime", &nap::emography::StressIntensityReadingSummary::mEndTime, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Object", &nap::emography::StressIntensityReadingSummary::mObject, nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT


//////////////////////////////////////////////////////////////////////////
