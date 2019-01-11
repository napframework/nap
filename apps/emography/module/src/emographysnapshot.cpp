#include "emographysnapshot.h"

// nap::emographysnapshot run time class definition 
RTTI_BEGIN_STRUCT_NO_DEFAULT_CONSTRUCTOR(nap::emography::BaseSnapshot)
	RTTI_PROPERTY("TimeStamp",	&nap::emography::BaseSnapshot::mTimeStamp,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_STRUCT(nap::emography::StressSnapshot)
	RTTI_VALUE_CONSTRUCTOR(const nap::emography::StressReading&, const nap::TimeStamp&)
	RTTI_VALUE_CONSTRUCTOR(const nap::emography::StressReading&)
	RTTI_PROPERTY("Object", &nap::emography::StressSnapshot::mObject, nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	namespace emography
	{
		BaseSnapshot::BaseSnapshot() : mTimeStamp(utility::getCurrentTime())
		{
		}

		BaseSnapshot::BaseSnapshot(const TimeStamp& timestamp) : mTimeStamp(timestamp)
		{
		}

		BaseSnapshot::BaseSnapshot(TimeStamp&& timestamp) : mTimeStamp(std::move(timestamp))
		{
		}
	}
}