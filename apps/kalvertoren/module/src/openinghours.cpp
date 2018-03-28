#include "openinghours.h"

RTTI_BEGIN_STRUCT(nap::OpeningTime)
	RTTI_VALUE_CONSTRUCTOR(int, int)
	RTTI_PROPERTY("Hour",	&nap::OpeningTime::mHour,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Minute",	&nap::OpeningTime::mMinute,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT

// nap::openinghours run time class definition 
RTTI_BEGIN_CLASS(nap::OpeningHours)
	RTTI_PROPERTY("Monday",		&nap::OpeningHours::mMonday,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Tuesday",	&nap::OpeningHours::mTuesday,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Wednesday",	&nap::OpeningHours::mWednesday, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Thursday",	&nap::OpeningHours::mThursday,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Friday",		&nap::OpeningHours::mFriday,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Saturday",	&nap::OpeningHours::mSaturday,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Sunday",		&nap::OpeningHours::mSunday,	nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	OpeningHours::~OpeningHours()			{ }


	bool OpeningHours::init(utility::ErrorState& errorState)
	{
		return true;
	}
}