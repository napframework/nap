#include "emographysnapshot.h"

// nap::emographysnapshot run time class definition 
RTTI_BEGIN_CLASS(nap::emography::ReadingBase)
	RTTI_PROPERTY("TimeStamp", &nap::emography::ReadingBase::mTimeStamp, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("NumSecondsActive", &nap::emography::ReadingBase::mNumSecondsActive, nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT

RTTI_BEGIN_CLASS(nap::emography::StressStateReading)
	RTTI_CONSTRUCTOR(const nap::emography::EStressState&, const nap::TimeStamp&)
	RTTI_CONSTRUCTOR(const nap::emography::EStressState&)
	RTTI_PROPERTY("Object", &nap::emography::StressStateReading::mObject, nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT

RTTI_BEGIN_CLASS(nap::emography::StressIntensityReading)
	RTTI_CONSTRUCTOR(const nap::emography::StressIntensity&, const nap::TimeStamp&)
	RTTI_CONSTRUCTOR(const nap::emography::StressIntensity&)
	RTTI_PROPERTY("Object", &nap::emography::StressIntensityReading::mObject, nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT


//////////////////////////////////////////////////////////////////////////
